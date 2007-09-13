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

#ifndef _LOOP_H_
#define _LOOP_H_

#include "SDL.h"
#include "translation.h"

// we include this here because this headers
// are used in nearly all modes
#include "pp_types.h"
#include "gl_util.h"
#include "ppgltk/ui_mgr.h"
#include "ppgltk/ui_theme.h"
#include "render_util.h"
#include "game_config.h"


/// Game state
typedef enum {
    ALL_MODES = -2,
    NO_MODE = -1,
    SPLASH = 0,
    GAME_TYPE_SELECT,
    EVENT_SELECT,
	EVENT_RACE_SELECT,
    RACE_SELECT,
    LOADING,
    INTRO,
    RACING,
    GAME_OVER,
    PAUSED,
    RESET,
    CREDITS,
	CONFIGURATION,
	CONFIG_GRAPHICS,
	CONFIG_VIDEO,
	CONFIG_AUDIO,
	CONFIG_KEYBOARD,
	CONFIG_JOYSTICK,
	QUIT,
	BENCHMARK,
    NUM_GAME_MODES,
    HIGHSCORE
} game_mode_t;

void set_game_mode( game_mode_t mode );
void main_loop();
bool is_mode_change_pending();

void loop_mouse_func (int button, int state, int x, int y);
void loop_mouse_motion_func( int x, int y );
void loop_keyboard_func(SDLKey key, SDLMod mod, bool release, int x, int y);



class GameMode
{

protected:
	/// custom function for drawing snow in the ui
	void drawSnow( float timeStep, bool windy = false );
	
public:
	GameMode();
	virtual ~GameMode(){}; 
	virtual void loop(float timeStep) = 0;
	
		
	virtual bool keyboardEvent(SDLKey key, bool release){return false;};
	virtual bool keyPressEvent(SDLKey key){return false;};
	virtual bool keyReleaseEvent(SDLKey key){return false;};
	
	virtual bool mouseButtonEvent(int button, int x, int y, bool pressed){return false;};
	virtual bool mouseButtonPressEvent(int button, int x, int y){return false;};
	virtual bool mouseButtonReleaseEvent(int button, int x, int y){return false;};
	
	static GameMode* currentMode;
	static game_mode_t mode;
	static game_mode_t prevmode;
};

#endif // _LOOP_H_
