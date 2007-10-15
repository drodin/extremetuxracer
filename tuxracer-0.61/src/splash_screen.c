/* 
 * Tux Racer 
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

#include "tuxracer.h"
#include "audio.h"
#include "keyboard.h"
#include "gl_util.h"
#include "fps.h"
#include "loop.h"
#include "render_util.h"
#include "textures.h"
#include "multiplayer.h"
#include "ui_mgr.h"
#include "ui_snow.h"

#define COORD_OFFSET_AMT -0.5
static const colour_t background_colour = { 0.48, 0.63, 0.90, 1.0 };
static char *logo_bindings[4] = { "splash_screen_tl",
				  "splash_screen_bl",
				  "splash_screen_tr",
				  "splash_screen_br" };

static void goto_next_mode()
{
    int i;
    set_game_mode( GAME_TYPE_SELECT );

    /* 
     * Free textures
     */
    for (i=0; i<sizeof(logo_bindings)/sizeof(logo_bindings[0]); i++) {
	unbind_texture( logo_bindings[i] );
    }
    flush_textures();
    winsys_post_redisplay();
}

static void splash_screen_mouse_func( int button, int state, int x, int y )
{
    goto_next_mode();
}

void splash_screen_init(void) 
{
    init_ui_snow();

    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( splash_screen_mouse_func );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );

    play_music( "splash_screen" );

    reshape( getparam_x_resolution(), getparam_y_resolution() );
}


static void draw_logo()
{
    GLuint texid[4];
    int xoffsets[4] = { -1, -1, 0, 0 };
    int yoffsets[4] = { 0, -1, 0, -1 };
    point2d_t ll, ur;
    GLint w, h;
    int i;

    glEnable( GL_TEXTURE_2D );

    for (i=0; i<4; i++) {
	if ( ! get_texture_binding( logo_bindings[i], &texid[i] ) ) {
	    return;
	}
    }

    glColor4f( 1.0, 1.0, 1.0, 1.0 );

    for (i=0; i<4; i++) {
	glBindTexture( GL_TEXTURE_2D, texid[i] );

	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h );

	ll.x = getparam_x_resolution()/2 + xoffsets[i]*w;
	ll.y = getparam_y_resolution()/2 + yoffsets[i]*h;
	ur.x = ll.x + w;
	ur.y = ll.y + h;


	glBegin( GL_QUADS );
	{
	    glTexCoord2f( 0.0, 0.0 );
	    glVertex2f( ll.x, ll.y );
	    glTexCoord2f( 1.0, 0.0 );
	    glVertex2f( ur.x, ll.y );
	    glTexCoord2f( 1.0, 1.0 );
	    glVertex2f( ur.x, ur.y );
	    glTexCoord2f( 0.0, 1.0 );
	    glVertex2f( ll.x, ur.y );
	}
	glEnd();
    }
}


void splash_screen_loop( scalar_t time_step )
{
    check_gl_error();

    update_audio();

    clear_rendering_context();

    set_gl_options( GUI );

    ui_setup_display();

    if (getparam_ui_snow()) {
	update_ui_snow( time_step, False );
	draw_ui_snow();
    }

    draw_logo();

    ui_draw();

    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
} 

START_KEYBOARD_CB( splash_screen_cb )
{
    goto_next_mode();
}
END_KEYBOARD_CB

START_KEYBOARD_CB( toggle_snow )
{
    if ( release ) return;

    setparam_ui_snow( (bool_t) !getparam_ui_snow() );
}
END_KEYBOARD_CB

void splash_screen_register()
{
    int status = 0;

    status |= add_keymap_entry( SPLASH, DEFAULT_CALLBACK, NULL, NULL, 
				splash_screen_cb );

    status |= add_keymap_entry( ALL_MODES, FIXED_KEY, "tab", NULL, 
				toggle_snow );

    check_assertion( status == 0, "out of keymap entries" );

    register_loop_funcs( SPLASH, splash_screen_init, splash_screen_loop, 
			 NULL );

}
