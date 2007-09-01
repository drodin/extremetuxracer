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

#include "joystick.h"
#include "game_config.h"

#if defined( HAVE_SDL_JOYSTICKOPEN )

#include "SDL.h"
#include "SDL_joystick.h"

static SDL_Joystick *joystick = NULL;
static int num_buttons = 0;
static int num_axes = 0;

void init_joystick()
{
    if(getparam_disable_joystick()) return;
	
	int num_joysticks = 0;
    char *js_name;

    /* Initialize SDL SDL joystick module */
    if ( SDL_Init( SDL_INIT_JOYSTICK ) < 0 ) {
	handle_error( 1, "Couldn't initialize SDL: %s", SDL_GetError() );
    }

    num_joysticks = SDL_NumJoysticks();

    print_debug( DEBUG_JOYSTICK, "Found %d joysticks", num_joysticks );

    if ( num_joysticks == 0 ) {
	joystick = NULL;
	return;
    }

    js_name = (char*) SDL_JoystickName( 0 );

    print_debug( DEBUG_JOYSTICK, "Using joystick `%s'", js_name );

    joystick = SDL_JoystickOpen( 0 );

    if ( joystick == NULL ) {
	print_debug( DEBUG_JOYSTICK, "Cannot open joystick" );
	return;
    }

    /* Get number of buttons */
    num_buttons = SDL_JoystickNumButtons( joystick );
    print_debug( DEBUG_JOYSTICK, "Joystick has %d button%s", 
		 num_buttons, num_buttons == 1 ? "" : "s" );

    /* Get number of axes */
    num_axes = SDL_JoystickNumAxes( joystick );
    print_debug( DEBUG_JOYSTICK, "Joystick has %d ax%ss", 
		 num_axes, num_axes == 1 ? "i" : "e" );

}

bool is_joystick_active()
{
	if(getparam_disable_joystick()) return false;

	return (bool) ( joystick != NULL );
	
}

void update_joystick()
{
	if(getparam_disable_joystick()) return;
    SDL_JoystickUpdate();
}

double get_joystick_x_axis()
{
    if(getparam_disable_joystick()) return 0.0;

	
	static bool warning_given = false;
    int axis;

    check_assertion( joystick != NULL,
		     "joystick is null" );

    axis = getparam_joystick_x_axis();

    /* Make sure axis is valid */
    if ( axis >= num_axes || axis < 0 ) {

	if ( !warning_given ) {
	    print_warning( IMPORTANT_WARNING, 
			   "joystick x axis mapped to axis %d "
			   "but joystick only has %d axes", axis, num_axes );
	    warning_given = true;
	}

	return 0.0;
    }

    return SDL_JoystickGetAxis( joystick, axis )/32768.0;
}

double get_joystick_y_axis()
{
	if(getparam_disable_joystick()) return 0.0;
	
    static bool warning_given = false;
    int axis;

    check_assertion( joystick != NULL,
		     "joystick is null" );

    axis = getparam_joystick_y_axis();

    /* Make sure axis is valid */
    if ( axis >= num_axes || axis < 0 ) {

	if ( !warning_given ) {
	    print_warning( IMPORTANT_WARNING, 
			   "joystick y axis mapped to axis %d "
			   "but joystick only has %d axes", axis, num_axes );
	    warning_given = true;
	}

	return 0.0;
    }

    return SDL_JoystickGetAxis( joystick, getparam_joystick_y_axis() )/32768.0;
}

bool is_joystick_button_down( int button ) 
{
    if(getparam_disable_joystick()) return false;
	
	static bool warning_given = false;

    check_assertion( joystick != NULL,
		     "joystick is null" );

    check_assertion( button >= 0, "button is negative" );

    if ( button >= num_buttons ) {
	if ( !warning_given ) {
	    print_warning( IMPORTANT_WARNING,
			   "state of button %d requested, but "
			   "joystick only has %d buttons.  Further warnings "
			   "of this type will be suppressed", 
			   button, num_buttons );
	    warning_given = true;
	}
	return false;
    }

    return (bool) SDL_JoystickGetButton( joystick, button );
}

bool is_joystick_continue_button_down()
{
	if(getparam_disable_joystick()) return false;
	
    if ( joystick == NULL ) {
	return false;
    }

    if ( getparam_joystick_continue_button() < 0 ) {
	return false;
    }

    return is_joystick_button_down( getparam_joystick_continue_button() );
}

int get_joystick_down_button()
{
    for(int i=0; i<num_buttons; i++){
		if(is_joystick_button_down(i)){
			return i;			
		}	
	}
	return -1;
}






#else

/* Stub functions */

void init_joystick()
{
}

bool is_joystick_active()
{
    return false;
}

void update_joystick()
{
}

double get_joystick_x_axis()
{
    return 0.0;
}

double get_joystick_y_axis()
{
    return 0.0;
}

bool is_joystick_button_down( int button ) 
{
    return false;
}

bool is_joystick_continue_button_down()
{
    return false;
}

int get_joystick_down_button()
{
    return -1;
}


#endif

/* EOF */
