/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 * 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "winsys.h"
#include "game_config.h"
#include "loop.h"
#include "render_util.h"
#include "highscore.h"
#include "player.h"

#include "ppgltk/audio/audio.h"

/* Windowing System Abstraction Layer */
/* Abstracts creation of windows, handling of events, etc. */

#if defined( HAVE_SDL_MIXER )
#   include "SDL_mixer.h"
#endif

static SDL_Surface *screen = NULL;

static bool redisplay = false;


/*---------------------------------------------------------------------------*/
/*! 
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay() 
{
    redisplay = true;
}


/*---------------------------------------------------------------------------*/
/*! 
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    SDL_GL_SwapBuffers();
}


/*---------------------------------------------------------------------------*/
/*! 
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    SDL_WarpMouse( x, y );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets up the SDL OpenGL rendering context
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void setup_sdl_video_mode()
{
    Uint32 video_flags = SDL_OPENGL; 
    int bpp = 0;
    int width, height;

    if ( getparam_fullscreen() ) {
		video_flags |= SDL_FULLSCREEN;
    } else {
		video_flags |= SDL_RESIZABLE;
    }
	
#if SDL_VERSION_ATLEAST(1,2,6)
	if(getparam_enable_fsaa()){
		//enable FSAA
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, getparam_multisamples());
	}
#endif
	
	if(getparam_stencil_buffer()){
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	}

    switch ( getparam_bpp_mode() ) {
    case 0:
		/* Use current bpp */
		bpp = 0;
	break;

    case 1:
		/* 16 bpp */
		bpp = 16;
	break;

    case 2:
		/* 32 bpp */
		bpp = 32;
	break;

    default:
		setparam_bpp_mode( 0 );
		bpp = getparam_bpp_mode();
    }

    width = getparam_x_resolution();
	
	if(getparam_x_resolution_half_width()){
		width /=2;		
	}
	
    height = getparam_y_resolution();

    if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags ) ) ==  NULL ){
    	handle_system_error( 1, "Couldn't initialize video: %s", 
			     SDL_GetError() );
    }
	
	glViewport(0,0,width,height);

}


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the OpenGL rendering context, and creates a window (or 
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title, 
		  char *icon_title )
{
    Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;

    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
	handle_error( 1, "Couldn't initialize SDL: %s", SDL_GetError() );
    }


    /* 
     * Init video 
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    setup_sdl_video_mode();

    SDL_WM_SetCaption( window_title, icon_title );

}


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
    SDL_Quit();
}

/*---------------------------------------------------------------------------*/
/*! 
  Enables/disables key repeat messages from being generated
  \return  
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool enabled )
{
    if ( enabled ) {
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY,
			     SDL_DEFAULT_REPEAT_INTERVAL );
    } else {
	SDL_EnableKeyRepeat( 0, 0 );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool visible )
{
    SDL_ShowCursor( visible );
}

/*---------------------------------------------------------------------------*/
/*! 
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
	SDL_Event event; 
    int x, y;

    while (true) {

	SDL_LockAudio();
	SDL_UnlockAudio();

	while ( SDL_PollEvent( &event ) ) {
	    
	    switch ( event.type ) {
	    case SDL_KEYDOWN:
		    SDL_GetMouseState( &x, &y );
			loop_keyboard_func( event.key.keysym.sym,
						event.key.keysym.mod,
				      	false,
				      	x, y );
			break;

	    case SDL_KEYUP:
		    SDL_GetMouseState( &x, &y );
		    loop_keyboard_func( event.key.keysym.sym,
				      	event.key.keysym.mod,
				      	true,
				      	x, y );
		break;

	    case SDL_MOUSEBUTTONDOWN:
	    case SDL_MOUSEBUTTONUP:
			loop_mouse_func( event.button.button,
				   event.button.state,
				   event.button.x,
				   event.button.y );
		break;

	    case SDL_MOUSEMOTION:
		if ( event.motion.state ) {
		    /* buttons are down */
			loop_mouse_motion_func( event.motion.x,
					event.motion.y );
		} else {
		    /* no buttons are down */
			loop_mouse_motion_func( event.motion.x,
						event.motion.y );
		}
		break;

	    case SDL_VIDEORESIZE:
		setup_sdl_video_mode();
		    reshape( event.resize.w,
				     event.resize.h );
		break;
			
		case SDL_QUIT:
			winsys_exit(0);		
			break;
		
	    }

	    SDL_LockAudio();
	    SDL_UnlockAudio();
	}

	if ( redisplay ) {
	    redisplay = false;
	}
	main_loop();

	/* Delay for 1 ms.  This allows the other threads to do some
	   work (otherwise the audio thread gets starved). */
	SDL_Delay(1);

    }

    /* Never exits */
    code_not_reached();
}


/*---------------------------------------------------------------------------*/
/*! 
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
extern void cleanup(void);

void winsys_exit( int code )
{
    players[0].saveData();
    Highscore->saveData();
    cleanup();
    
    exit( code );
}
