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

#include "racing.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/alg/defs.h"

#include "keyframe.h"
#include "course_render.h"
#include "gl_util.h"
#include "fps.h"
#include "loop.h"
#include "render_util.h"
#include "view.h"
#include "model_hndl.h"
#include "tux_shadow.h"
#include "phys_sim.h"
#include "part_sys.h"
#include "screenshot.h"
#include "fog.h"
#include "viewfrustum.h"
#include "track_marks.h"
#include "hud.h"
#include "joystick.h"
#include "snow.h"
#include "game_config.h"
#include "winsys.h"

#include "game_mgr.h"
#include "bench.h"

#include "ppgltk/ppgltk.h"


/* Time constant for automatic steering centering (s) */
#define TURN_DECAY_TIME_CONSTANT 0.5

/* Maximum time over which jump can be charged */
#define MAX_JUMP_AMT 1.0

/* Time constant for automatic rolling centering (s) */
#define ROLL_DECAY_TIME_CONSTANT 0.2

#define JUMP_CHARGE_DECAY_TIME_CONSTANT 0.1

/* If too high off the ground, tux flaps instead of jumping */
#define JUMP_MAX_START_HEIGHT 0.30

extern terrain_tex_t terrain_texture[NUM_TERRAIN_TYPES];
extern unsigned int num_terrains;

Racing::Racing()
{
	m_rightTurn = false;
	m_leftTurn = false;
	m_trickModifier = false;
	m_paddling = false;
	m_charging = false;
	m_braking = false;
	
    // Initialize view
    if ( getparam_view_mode() < 0 || 
		getparam_view_mode() >= NUM_VIEW_MODES ) 
    {
		setparam_view_mode( ABOVE );
    }
    set_view_mode( players[0], (view_mode_t)getparam_view_mode() );

    // We need to reset controls here since callbacks won't have been
    //   called in paused mode. This results in duplication between this
    //   code and init_physical_simulation.  Oh well. 

    players[0].control.turn_fact = 0.0;
    players[0].control.turn_animation = 0.0;
    players[0].control.is_braking = false;
    players[0].control.is_paddling = false;
    players[0].control.jumping = false;
    players[0].control.jump_charging = false;
	players[0].max_speed = 0;

    // Set last_terrain to a value not used below
    m_lastTerrain = 0;
    
    if ( GameMode::prevmode != PAUSED ) {
		init_physical_simulation();
    }

    gameMgr->abortRace(false);
	
	init_snow(players[0].view.pos);
		
    play_music( "racing" );	
}


Racing::~Racing()
{
	//todo: stop all sound which are specified by the used theme
	halt_sound( "flying_sound" );
    halt_sound( "rock_sound" );
    halt_sound( "ice_sound" );
    halt_sound( "snow_sound" );
    break_track_marks();
}

void
Racing::loop(float timeStep)
{
	int width, height;
    bool joy_left_turn = false;
    bool joy_right_turn = false;
    double joy_turn_fact = 0.0;
    bool joy_paddling = false;
    bool joy_braking = false;
    bool joy_tricks = false;
    bool joy_charging = false;
    bool airborne;
    pp::Vec3d dir;
    float speed;
    float terrain_weights[NUM_TERRAIN_TYPES];
    int new_terrain = 0;
    int slide_volume;
	unsigned int i;

	if (Benchmark::getMode() == Benchmark::AUTO){
		m_paddling = true;
	}	
	
    dir = players[0].vel;
    speed = dir.normalize();
	
	//set max_speed
	if (speed > players[0].max_speed) players[0].max_speed=int(speed);

	
    airborne = (bool) ( players[0].pos.y > ( find_y_coord(players[0].pos.x, 
						       players[0].pos.z) + 
					  JUMP_MAX_START_HEIGHT ) );

    width = getparam_x_resolution();
    height = getparam_y_resolution();

    fpsCounter.update();

    update_audio();

    clear_rendering_context();

    fogPlane.setup();

    // Joystick

    if ( is_joystick_active() ) {
	float joy_x;
	float joy_y;

	update_joystick();

	joy_x = get_joystick_x_axis();
	joy_y = get_joystick_y_axis();

	if ( joy_x > 0.1 ) {
	    joy_right_turn = true;
	    joy_turn_fact = joy_x;
	} else if ( joy_x < -0.1 ) {
	    joy_left_turn = true;
	    joy_turn_fact = joy_x;
	}

	if ( getparam_joystick_brake_button() >= 0 ) {
	    joy_braking = 
		is_joystick_button_down( getparam_joystick_brake_button() );
	} 
	if ( !joy_braking ) {
	    joy_braking = (bool) ( joy_y > 0.5 );
	}

	if ( getparam_joystick_paddle_button() >= 0 ) {
	    joy_paddling = 
		is_joystick_button_down( getparam_joystick_paddle_button() );
	}
	if ( !joy_paddling ) {
	    joy_paddling = (bool) ( joy_y < -0.5 );
	}

	if ( getparam_joystick_jump_button() >= 0 ) {
	    joy_charging = 
		is_joystick_button_down( getparam_joystick_jump_button() );
	}

	if ( getparam_joystick_trick_button() >= 0 ) {
	    joy_tricks = 
		is_joystick_button_down( getparam_joystick_trick_button() );
	}
    }

    // Update braking 
    players[0].control.is_braking = (bool) ( m_braking || joy_braking );

    if ( airborne ) {
	new_terrain = (1<<4);

	// Tricks
	if ( m_trickModifier || joy_tricks ) {
	    if ( m_leftTurn || joy_left_turn ) {
		players[0].control.barrel_roll_left = true;
	    }
	    if ( m_rightTurn || joy_right_turn ) {
		players[0].control.barrel_roll_right = true;
	    }
	    if ( m_paddling || joy_paddling ) {
		players[0].control.front_flip = true;
	    }
	    if ( players[0].control.is_braking ) {
		players[0].control.back_flip = true;
	    }
	}

		for(i=0;i<num_terrains;i++){
			if ( !terrain_texture[i].sound.empty() && terrain_texture[i].soundactive==true) {
				halt_sound( terrain_texture[i].sound.c_str() );
				terrain_texture[i].soundactive=false;
			}
		}
		
    } else {

	get_surface_type(players[0].pos.x, players[0].pos.z, terrain_weights);
	

    //Play sliding sound
		
		slide_volume = int(MIN( (((pow(players[0].control.turn_fact, 2)*128)) +
			 (players[0].control.is_braking?128:0) +
			 (players[0].control.jumping?128:0) +
			 20) *
			(speed/10), 128 ));
		
		for(i=0;i<num_terrains;i++){
			if ( !terrain_texture[i].sound.empty() ) {
				if (terrain_weights[i] > 0 ){
					set_sound_volume(terrain_texture[i].sound.c_str(), int(slide_volume * terrain_weights[i]));
					if (terrain_texture[i].soundactive==false){
						play_sound(terrain_texture[i].sound.c_str() , -1 );
						terrain_texture[i].soundactive=true;
					}
				} else if (terrain_texture[i].soundactive==true){
					halt_sound( terrain_texture[i].sound.c_str() );
					terrain_texture[i].soundactive=false;
				}
			}
		}
		
			
    }

    // Jumping

    calcJumpAmt( timeStep );

    if ( ( m_charging || joy_charging ) && 
	 !players[0].control.jump_charging && !players[0].control.jumping ) 
    {
		players[0].control.jump_charging = true;
		m_chargeStartTime = gameMgr->time;
    }

    if ( ( !m_charging && !joy_charging ) && players[0].control.jump_charging ) {
		players[0].control.jump_charging = false;
		players[0].control.begin_jump = true;
    }

 
    // Turning 

    if ( ( m_leftTurn || joy_left_turn )  ^ (m_rightTurn || joy_right_turn ) ) {
	bool turning_left = (bool) ( m_leftTurn || joy_left_turn );

	if ( joy_left_turn || joy_right_turn ) {
	    players[0].control.turn_fact = joy_turn_fact;
	} else {
	    players[0].control.turn_fact = (turning_left?-1:1);
	}

	players[0].control.turn_animation += (turning_left?-1:1) *
	    0.15 * timeStep / 0.05;
	players[0].control.turn_animation = 
	    MIN(1.0, MAX(-1.0, players[0].control.turn_animation));
    } else {
	players[0].control.turn_fact = 0;

	// Decay turn animation
	if ( timeStep < ROLL_DECAY_TIME_CONSTANT ) {
	    players[0].control.turn_animation *= 
		1.0 - timeStep/ROLL_DECAY_TIME_CONSTANT;
	} else {
	    players[0].control.turn_animation = 0.0;
	}
    }

    
    
    //Paddling
    if ( ( m_paddling || joy_paddling ) && players[0].control.is_paddling == false ) {
		players[0].control.is_paddling = true;
		players[0].control.paddle_time = gameMgr->time;
    }

    
   	//Play flying sound

    if (new_terrain & (1<<4)) {
		set_sound_volume("flying_sound", int(MIN(128, speed*2)));
		if (!(m_lastTerrain & (1<<4))) {
	 	   play_sound( "flying_sound", -1 );
		}
	    } else {
		if (m_lastTerrain & (1<<4)) {
		    halt_sound( "flying_sound" );
		}
	}

  	m_lastTerrain = new_terrain; 

	//Tricks
    if ( players[0].control.barrel_roll_left || players[0].control.barrel_roll_right ) {
	players[0].control.barrel_roll_factor += 
		( players[0].control.barrel_roll_left ? -1 : 1 ) * 0.15 * timeStep / 0.05;
	if ( (players[0].control.barrel_roll_factor  > 1) ||
	     (players[0].control.barrel_roll_factor  < -1) ) {
	    players[0].control.barrel_roll_factor = 0;
	    players[0].control.barrel_roll_left = players[0].control.barrel_roll_right = false;
	}
    }
    if ( players[0].control.front_flip || players[0].control.back_flip ) {
	players[0].control.flip_factor += 
		( players[0].control.back_flip ? -1 : 1 ) * 0.15 * timeStep / 0.05;
	if ( (players[0].control.flip_factor  > 1) ||
	     (players[0].control.flip_factor  < -1) ) {
	    players[0].control.flip_factor = 0;
	    players[0].control.front_flip = players[0].control.back_flip = false;
	}
    }

    update_player_pos( players[0], timeStep );
	 
	//Track Marks
    add_track_mark( players[0] );


    update_view( players[0], timeStep );

    setup_view_frustum( players[0], NEAR_CLIP_DIST, 
			getparam_forward_clip_distance() );

    draw_sky(players[0].view.pos);

    draw_fog_plane();

    set_course_clipping( true );
    set_course_eye_point( players[0].view.pos );
    setup_course_lighting();
    render_course();
	
	
	//Draw snow
	update_snow( timeStep, false, players[0].view.pos );
	draw_snow(players[0].view.pos);
	
    draw_trees();
	
    if ( getparam_draw_particles() ) {
	update_particles( timeStep );
	draw_particles( players[0] );
    }

    ModelHndl->draw_tux();
    draw_tux_shadow();

    HUD1.draw(players[0]);
	
	
    reshape( width, height );

    winsys_swap_buffers();

    gameMgr->time += timeStep;
	if (airborne) gameMgr->airbornetime += timeStep;
		
	if(Benchmark::getMode() == Benchmark::PAUSED){
		set_game_mode(PAUSED);
	}
}

void
Racing::calcJumpAmt( double time_step )
{
    if ( players[0].control.jump_charging ) {
		players[0].control.jump_amt = MIN( 
	    MAX_JUMP_AMT, gameMgr->time - m_chargeStartTime );
    } else if ( players[0].control.jumping ) {
		players[0].control.jump_amt *= 
	    ( 1.0 - ( gameMgr->time - players[0].control.jump_start_time ) / 
	      JUMP_FORCE_DURATION );
    } else {
		players[0].control.jump_amt = 0;
    }
}


bool
Racing::keyboardEvent(SDLKey key, bool release)
{
	if(key==getparam_turn_left_key()){
		m_leftTurn = (bool) !release;
		return true;
	}else if(key==getparam_turn_right_key()){
		m_rightTurn = (bool) !release;
		return true;
	}else if(key==getparam_paddle_key()){
		m_paddling = (bool) !release;
		return true;
	}else if(key==getparam_brake_key()){
		m_braking = (bool) !release;
		return true;
	}else if(key==getparam_trick_modifier_key()){
		m_trickModifier = (bool) !release;
		return true;
	}else if(key==getparam_jump_key()){
		m_charging = (bool) !release;
		return true;
	}else{
		return false;
	}
}

bool
Racing::keyPressEvent(SDLKey key)
{
	switch(key){
		case 'q':
			gameMgr->abortRace();
	    		set_game_mode( GAME_OVER );
	    		return true;
		case SDLK_ESCAPE: 
			set_game_mode( PAUSED );
			return true;	
		case '1':
    		set_view_mode( players[0], ABOVE );
    		setparam_view_mode( ABOVE );
			return true;
		case '2':
			set_view_mode( players[0], FOLLOW );
			setparam_view_mode( FOLLOW );
			return true;
		case '3':
			set_view_mode( players[0], BEHIND );
			setparam_view_mode( BEHIND );
			return true;	
		case 's':
    		screenshot();
			return true;
		case 'p':
			set_game_mode( PAUSED );
			return true;
		default:
			if(key==getparam_reset_key()){
				set_game_mode( RESET );
				return true;
			}
	}
		
	return false;
}
