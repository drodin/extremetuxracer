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

#include "loop.h"
#include "game_config.h"

#include "ppgltk/ui_snow.h"
#include "ppgltk/alg/defs.h"

#include "SDL.h"


#include "render_util.h"
#include "splash_screen.h"
#include "game_type_select.h"
#include "credits.h"
#include "race_select.h"
#include "loading.h"
#include "intro.h"
#include "racing.h"
#include "paused.h"
#include "game_over.h"
#include "reset.h"
#include "event_select.h"
#include "event_race_select.h"
#include "bench.h"
#include "highscore.h"

#include "configuration.h"
#include "graphicsconfig.h"
#include "videoconfig.h"
#include "audioconfig.h"
#include "keyboardconfig.h"
#include "joystickconfig.h"

#include "game_mgr.h"


GameMode* GameMode::currentMode = NULL;

game_mode_t GameMode::mode;
game_mode_t GameMode::prevmode;

static game_mode_t new_mode = NO_MODE;

void set_game_mode( game_mode_t mode ) 
{
    new_mode = mode;
}

void main_loop()
{
	if ( getparam_capture_mouse() ) {
		const int w = getparam_x_resolution();
		const int h = getparam_y_resolution();
		pp::Vec2d pos = UIMgr.getMousePosition();

		// Flip y coordinates
		pos.y = h - pos.y;

		if ( pos.x < 0 ) {
		    pos.x = 0;
		}
		if ( pos.x > w-1 ) {
		    pos.x = w-1;
		}
		if ( pos.y < 0 ) {
		    pos.y = 0;
		}
		if ( pos.y > h-1 ) {
		    pos.y = h-1;
		}

		winsys_warp_pointer( int(pos.x), int(pos.y) );
    }
	
	if ( GameMode::mode != new_mode ) {
		
		if (GameMode::currentMode!=NULL){
			delete GameMode::currentMode;
		}
		GameMode::prevmode = GameMode::mode;

		switch(new_mode){
			case SPLASH:
				GameMode::currentMode = new SplashScreen();
				break;			
			case GAME_TYPE_SELECT:
				GameMode::currentMode = new GameTypeSelect();
				break;
			case CREDITS:
				GameMode::currentMode = new Credits();
				break;
			case CONFIGURATION:
				GameMode::currentMode = new Configuration();
				break;
			case RACE_SELECT:
				GameMode::currentMode = new RaceSelect();
				break;
			case LOADING:
				GameMode::currentMode = new Loading();
				break;
			case INTRO:
				GameMode::currentMode = new Intro();
				break;
			case RACING:
				GameMode::currentMode = new Racing();
				break;
			case PAUSED:
				GameMode::currentMode = new Paused();
				break;
			case GAME_OVER:
				GameMode::currentMode = new GameOver();
				break;			
			case RESET:
				GameMode::currentMode = new Reset();
				break;
			case EVENT_SELECT:
				GameMode::currentMode = new EventSelect();
				break;
			case CONFIG_GRAPHICS:
				GameMode::currentMode = new GraphicsConfig();
				break;
			case CONFIG_VIDEO:
				GameMode::currentMode = new VideoConfig();
				break;
			case CONFIG_AUDIO:
				GameMode::currentMode = new AudioConfig();
				break;
			case CONFIG_KEYBOARD:
				GameMode::currentMode = new KeyboardConfig();
				break;
			case CONFIG_JOYSTICK:
				GameMode::currentMode = new JoystickConfig();
				break;
			case EVENT_RACE_SELECT:
				GameMode::currentMode = new EventRaceSelect();
				break;
			case BENCHMARK:
				GameMode::currentMode = new Benchmark();
			case HIGHSCORE:
				GameMode::currentMode = new HighscoreShow();
				break;
			default:{}
				//todo: add fallback			
		}
		
		GameMode::mode = new_mode;
	
		// Reset time step clock so that there isn't a sudden
		//  jump when we start the new mode 
		gameMgr->resetTimeStep();
	}
	
	gameMgr->updateTimeStep();
	
	if ( Benchmark::getTimeStep() >0.0 ){
		GameMode::currentMode->loop(Benchmark::getTimeStep());
	}else{	
		GameMode::currentMode->loop( gameMgr->getTimeStep() );
	}
}

/*---------------------------------------------------------------------------*/
/*! 
  Returns true if a mode change will occur the next time main_loop() runs.

  \author  jfpatry
*/
bool is_mode_change_pending()
{
    return bool(GameMode::mode != new_mode);
}

GameMode::GameMode()
{
}

	
void
GameMode::drawSnow( float timeStep, bool windy )
{
	// check wether ui snow is 
	if(getparam_ui_snow()){
		// update and draw snow
		update_ui_snow( timeStep, windy );
		draw_ui_snow();
    }
}

void loop_mouse_func (int button, int state, int x, int y)
{
	if(GameMode::currentMode!=NULL){
		bool pressed = state && SDL_PRESSED;
		if(GameMode::currentMode->mouseButtonEvent(button,x,y,pressed)) return;
		else if(pressed){
			if(GameMode::currentMode->mouseButtonPressEvent(button,x,y)) return;
		}else{
			if(GameMode::currentMode->mouseButtonReleaseEvent(button,x,y)) return;
		}	
		UIMgr.mouseEvent(button,state,x,y);
	}
}

void loop_mouse_motion_func( int x, int y )
{
	UIMgr.motionEvent(x,y);
}

void loop_keyboard_func(SDLKey key, SDLMod mod, bool release, int x, int y)
{
	if(GameMode::currentMode!=NULL){		
		if (key < SDLK_UP) {
            if ( isalpha( key ) ) {
                key = SDLKey(tolower( key ));
            }
        }
		
		if(GameMode::currentMode->keyboardEvent(key,release)) return;
		else if(release){
			if(GameMode::currentMode->keyReleaseEvent(key)) return;
		}else{
			if(GameMode::currentMode->keyPressEvent(key)) return;
		}
		UIMgr.keyboardEvent(key,mod,release);
	}
}
