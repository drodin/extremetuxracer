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

#include "loading.h"

#include "ppgltk/ui_mgr.h"
#include "ppgltk/audio/audio.h"

#include "game_config.h"
#include "gl_util.h"
#include "fps.h"
#include "render_util.h"
#include "phys_sim.h"
#include "view.h"
#include "course_render.h"
#include "loop.h"
#include "fog.h"
#include "viewfrustum.h"
#include "course_mgr.h"

#include "ppgltk/ui_snow.h"
#include "ppgltk/ui_theme.h"

#include "joystick.h"
#include "part_sys.h"

#include "game_mgr.h"

Loading::Loading()
{
    int width = getparam_x_resolution();
    int height = getparam_y_resolution();
	
	pp::Vec2d pos(width/2,height/2);	
	mp_loadingLbl = new pp::Label(pos,"loading",_("Loading, Please Wait..."));
	mp_loadingLbl->alignment.center();
	mp_loadingLbl->alignment.middle();
	
	m_loadedCondition = (race_conditions_t)-1;
	play_music( "loading" );
}

Loading::~Loading()
{
	delete mp_loadingLbl;
}

void
Loading::loop(float timeStep)
{
	int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    update_audio();

    clear_rendering_context();

    set_gl_options( GUI );

    UIMgr.setupDisplay();

	drawSnow(timeStep, gameMgr->getCurrentRace().windy);
	
    theme.drawMenuDecorations();

    reshape( width, height );

	UIMgr.draw();	
	
    winsys_swap_buffers();
	

    if ( m_loadedCourse.empty() ||
		m_loadedCourse != gameMgr->getCurrentRace().course ||
		m_loadedCondition != gameMgr->getCurrentRace().condition ) 
    {
	// Load the course
	load_course( gameMgr->getCurrentRace().course );

	m_loadedCourse = gameMgr->getCurrentRace().course;
	m_loadedCondition = gameMgr->getCurrentRace().condition;
    }

    set_course_mirroring( gameMgr->getCurrentRace().mirrored );

    // We're done here, enter INTRO mode
    set_game_mode( INTRO );	
}
