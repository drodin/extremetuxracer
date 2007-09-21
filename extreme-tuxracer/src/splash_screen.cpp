/* 
 * ETRacer 
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

#include "splash_screen.h"

#include "gl_util.h"
#include "textures.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/alg/glhelper.h"
 
SplashScreen::SplashScreen()
{		
	pp::Vec2d pos(getparam_x_resolution()/2, 100); //Coordinates are from bottom-left corner of the screen
	
	mp_anykeyLbl = new pp::Label(pos,"event_and_cup_label",_("PRESS ANY KEY TO START"));

	mp_anykeyLbl->alignment.center();  //use the horizontal center of the text as the placement point
	mp_anykeyLbl->alignment.top(); //use the top of the text as the placement point
	
  play_music( "splash_screen" );
}

SplashScreen::~SplashScreen()
{
	// there is no need to keep the logo in memory
	if(getparam_y_resolution()>800) {
		unbind_texture( "splash_screen" );
	} else {
		unbind_texture( "splash_screen" );
	}
	flush_textures();
	
	delete mp_anykeyLbl;
}

void
SplashScreen::loop(float timeStep)
{
	update_audio();

  clear_rendering_context();
  set_gl_options( GUI );
  UIMgr.setupDisplay();

	drawSnow(timeStep);

	{//should this be created in the splash screen constructor?
		
		if(getparam_y_resolution()>760) {
			pp::Vec2d pos(getparam_x_resolution()/2 - 256, getparam_y_resolution()/2 - 256); //centered
			pp::Vec2d size(512, 512);
			ppGL::draw::rect("splash_screen", pos, size);
		} else {
			pp::Vec2d pos(getparam_x_resolution()/2 - 256, getparam_y_resolution()/2 - 256/2); //centered
			pp::Vec2d size(512, 256);
			ppGL::draw::rect("splash_screen_small", pos, size);
		}
	}	

  UIMgr.draw();

  reshape( getparam_x_resolution(), getparam_y_resolution() );
  winsys_swap_buffers();	
}


bool
SplashScreen::mouseButtonReleaseEvent(int button, int x, int y)
{
	set_game_mode( GAME_TYPE_SELECT );
	winsys_post_redisplay();
	return true;
}

bool
SplashScreen::keyReleaseEvent(SDLKey key)
{
	set_game_mode( GAME_TYPE_SELECT );
	winsys_post_redisplay();
	return true;
}
