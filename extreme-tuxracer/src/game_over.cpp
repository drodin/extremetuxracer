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

#include "game_over.h"

#include "ppgltk/ui_mgr.h"
#include "ppgltk/font.h"
#include "ppgltk/audio/audio.h"

#include "game_config.h"
#include "gl_util.h"
#include "fps.h"
#include "render_util.h"
#include "phys_sim.h"
#include "view.h"
#include "course_render.h"
#include "model_hndl.h"
#include "tux_shadow.h"
#include "loop.h"
#include "fog.h"
#include "viewfrustum.h"
#include "hud.h"
#include "joystick.h"
#include "part_sys.h"

#include "stuff.h"
#include "game_mgr.h"
#include "highscore.h"

#include "bench.h"

GameOver::GameOver()
{
	halt_sound( "flying_sound" );
    halt_sound( "rock_sound" );
    halt_sound( "ice_sound" );
    halt_sound( "snow_sound" );

	if( Benchmark::getMode() != Benchmark::NONE ){
		Benchmark::displayState();
		winsys_exit( 0 );
	}
		
    play_music( "game_over" );

    m_aborted = gameMgr->wasRaceAborted();

	if ( !m_aborted ) {
		gameMgr->updatePlayersScores();
    }
		
    if ( gameMgr->gametype!=GameMgr::PRACTICING ) {
		m_bestScore = gameMgr->updateCurrentRaceData();		
		
		if(!gameMgr->wasRaceWon()){
			players[0].decLives();
		}
    }else{
		if ( !m_aborted ) {
			m_bestScore = players[0].updateOpenCourseData(
									gameMgr->getCurrentRace().name,
									gameMgr->time,
									players[0].herring,
									players[0].score);
			
			if(m_bestScore) players[0].saveData();
		}
	}
	
	{  
	pp::Vec3d dir = players[0].vel;
	int speed = int(dir.normalize());
	//set max_speed
	if (speed > players[0].max_speed) players[0].max_speed=int(speed);
	}
	
	int width = getparam_x_resolution();
    int height = getparam_y_resolution();
	
	pp::Vec2d pos(width/2, height/2 +200);
	
	if ( gameMgr->wasRaceAborted() ) {
		mp_raceOverLbl = new pp::Label(pos,"race_over",_("Race aborted"));
		mp_raceOverLbl->alignment.center();
    }else{	
		mp_raceOverLbl = new pp::Label(pos,"race_over",_("Race Over"));
		mp_raceOverLbl->alignment.center();
	
		char buff[BUFF_LEN];
		int minutes, seconds, hundredths;

		getTimeComponents( gameMgr->time, minutes, seconds, hundredths );
		sprintf( buff, _("Time: %02d:%02d.%02d"), minutes, seconds, hundredths );	
		pos.y-=100;
		mp_timeLbl = new pp::Label(pos,"race_stats", buff);
		mp_timeLbl->alignment.center();
	
		sprintf( buff, _("Herring: %3d"), players[0].herring );
		pos.y-=30;
		mp_herringLbl = new pp::Label(pos,"race_stats",buff);
		mp_herringLbl->alignment.center();
	
		sprintf( buff, _("Score: %6d"), players[0].score );
		pos.y-=30;
		mp_scoreLbl = new pp::Label(pos,"race_stats",buff);
		mp_scoreLbl->alignment.center();
	
		int speed = int((double)players[0].max_speed * M_PER_SEC_TO_KM_PER_H);
		sprintf( buff, _("Max speed: %3d km/h"), speed);
		pos.y-=30;
		mp_maxspeedLbl = new pp::Label(pos,"race_stats",buff);
		mp_maxspeedLbl->alignment.center();
	
		double percent = (gameMgr->airbornetime / gameMgr->time) * 100.0;
		sprintf( buff, _("Was flying: %.01f %% of time"), percent);
		pos.y-=30;
		mp_flyingLbl = new pp::Label(pos,"race_stats",buff);
		mp_flyingLbl->alignment.center();
		
		char buff2[50];
		snprintf(buff2, 50, "");
		if(highscore::useHighscore) {
			int pos = Highscore->addScore(gameMgr->getCurrentRace().name,players[0].name,players[0].score);
			if(pos != -1)
				snprintf(buff2, 50, _("You made it to the %s place in the highscore!"),highscore::posToStr(pos).c_str());
		}
		pos.y-=30;
		mp_highscoreLbl = new pp::Label(pos,"race_stats",buff2);
		mp_highscoreLbl->alignment.center();
	
		const char *string="";
	
		if ( gameMgr->gametype==GameMgr::PRACTICING){
			if(m_bestScore){
				string = _("You beat your best score!");
			}
		} else if(gameMgr->wasEventWon()){
			string = _("Congratulations! You won the event!");
		} else if(gameMgr->wasCupWon()){
			string = _("Congratulations! You won the cup!");
		} else if(gameMgr->wasRaceWon()){
			string = _("You advanced to the next race!");
		} else {
			string = _("You didn't advance.");
		}	
	
		pos.y-=30;
		mp_resultsLbl = new pp::Label(pos,"race_stats",string);
		mp_resultsLbl->alignment.center();
	}
}

GameOver::~GameOver()
{
	delete mp_raceOverLbl;
	
	if ( !gameMgr->wasRaceAborted() ) {
		delete mp_timeLbl;
		delete mp_herringLbl;
		delete mp_scoreLbl;
		delete mp_maxspeedLbl;
		delete mp_flyingLbl;
		delete mp_resultsLbl;
		delete mp_highscoreLbl;
	}
}

void
GameOver::loop(float timeStep)
{
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    /* Check joystick */
    if ( is_joystick_active() ) {
	update_joystick();

	if ( is_joystick_continue_button_down() )
	{
	    if ( gameMgr->gametype != GameMgr::PRACTICING ) {
			set_game_mode( EVENT_RACE_SELECT );
		}else{
			set_game_mode( RACE_SELECT );
		}
	    winsys_post_redisplay();
	    return;
	}
    }
    fpsCounter.update();

    update_audio();

    clear_rendering_context();

    fogPlane.setup();
    update_player_pos( players[0], 0 );
    update_view( players[0], 0 );

    setup_view_frustum( players[0], NEAR_CLIP_DIST, 
			getparam_forward_clip_distance() );

    draw_sky(players[0].view.pos);
    draw_fog_plane();

    set_course_clipping( true );
    set_course_eye_point( players[0].view.pos );
    setup_course_lighting();
    render_course();
    draw_trees();
    if ( getparam_draw_particles() ) {
		draw_particles( players[0] );
    }

    ModelHndl->draw_tux();
    draw_tux_shadow();
    set_gl_options( GUI );

    UIMgr.setupDisplay();
	UIMgr.draw();
		
    HUD1.draw(players[0]);
    reshape( width, height );

    winsys_swap_buffers();
}

bool
GameOver::mouseButtonEvent(int button, int x, int y, bool pressed)

{
	if ( gameMgr->gametype!=GameMgr::PRACTICING ) {
		set_game_mode( EVENT_RACE_SELECT );
	}else{
		set_game_mode( RACE_SELECT );
	}
	winsys_post_redisplay();
	return true;
}

bool
GameOver::keyPressEvent(SDLKey key)
{
	if ( gameMgr->gametype!=GameMgr::PRACTICING ) {
		set_game_mode( EVENT_RACE_SELECT );
	}else{
		set_game_mode( RACE_SELECT );
	}
	winsys_post_redisplay();
	return true;
}
