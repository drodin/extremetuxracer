/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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

#include "paused.h"

#include "ppgltk/ui_mgr.h"
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
#include "part_sys.h"
#include "joystick.h"
#include "course_mgr.h"

#include "game_mgr.h"

#include "bench.h"

Paused::Paused()
{
	int centerX = getparam_x_resolution() / 2;
	int centerY = getparam_y_resolution() / 2;
	
	mp_backgroundFrm = new pp::Frame(pp::Vec2d(centerX-120,centerY-100),
								  pp::Vec2d(240,200));
	
	mp_pausedLbl = new pp::Label(pp::Vec2d(centerX,centerY+90),"paused",_("Paused"));
	mp_pausedLbl->alignment.center();
	mp_pausedLbl->alignment.top();
	

	mp_configBtn = new pp::Button(pp::Vec2d(centerX-150,centerY-15),
				     	pp::Vec2d(300, 40),
				     	"button_label",
				     	_("Configuration") );
	
    mp_configBtn->setHilitFontBinding("button_label_hilit");
	mp_configBtn->signalClicked.Connect(pp::CreateSlot(this,&Paused::configuration));
	
	mp_resumeBtn = new pp::Button(pp::Vec2d(centerX-150,centerY-55),
				     	pp::Vec2d(300, 40),
				     	"button_label",
				     	_("Resume") );
	
    mp_resumeBtn->setHilitFontBinding("button_label_hilit");
	mp_resumeBtn->signalClicked.Connect(pp::CreateSlot(this,&Paused::resume));

	mp_quitBtn = new pp::Button(pp::Vec2d(centerX-150,centerY-95),
				     	pp::Vec2d(300, 40),
				     	"button_label",
				     	_("Quit") );
	
    mp_quitBtn->setHilitFontBinding("button_label_hilit");
	mp_quitBtn->signalClicked.Connect(pp::CreateSlot(this,&Paused::quit));

	play_music( "paused" );
}

Paused::~Paused()
{
	delete mp_configBtn;
	delete mp_resumeBtn;
	delete mp_quitBtn;
	delete mp_backgroundFrm;
	delete mp_pausedLbl;
}
	
void
Paused::loop(float timeStep)
{
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    // Check joystick 
    if ( is_joystick_active() ) {
		update_joystick();

		if ( is_joystick_continue_button_down() )
		{
			set_game_mode( RACING );
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

    draw_sky( players[0].view.pos );

    draw_fog_plane( );

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

    HUD1.draw(players[0]);
	
	if(Benchmark::getMode()!=Benchmark::PAUSED){
    	UIMgr.draw();
	}    	
	reshape( width, height );
    winsys_swap_buffers();
}

bool
Paused::keyPressEvent(SDLKey key)
{
	if(key=='q') {
		Paused::quit();
	 } else {
		if(Benchmark::getMode() == Benchmark::PAUSED){
			set_game_mode( GAME_OVER );
		}else{
			set_game_mode( RACING );
		}
	}
	winsys_post_redisplay();
	return true;
}

void
Paused::resume()
{
	set_game_mode( RACING );
	winsys_post_redisplay();
}

void
Paused::quit()
{
	gameMgr->abortRace();
    set_game_mode( GAME_OVER );
}

void
Paused::configuration()
{
	set_game_mode( CONFIGURATION );
	UIMgr.setDirty();
}
