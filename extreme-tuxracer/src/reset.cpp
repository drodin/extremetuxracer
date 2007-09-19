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

#include "reset.h"


#include "game_config.h"
#include "ppgltk/audio/audio.h"
#include "ppgltk/alg/defs.h"

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
#include "course_load.h"
#include "winsys.h"

#include "stuff.h"

#include "game_mgr.h"

#include "ppgltk/ppgltk.h"


#define BLINK_IN_PLACE_TIME 0.5
#define TOTAL_RESET_TIME 1.0

Reset::Reset()
 : m_textColor(0.0, 0.0, 0.0),
   m_resetStartTime(getClockTime()),
   m_positionReset(false)
{
}

Reset::~Reset()
{
	
}

void
Reset::loop(float timeStep)
{
	int width, height;
    float elapsed_time = getClockTime() - m_resetStartTime;
    float course_width, course_length;
    static bool tux_visible = true; 
    static int tux_visible_count = 0;
    item_type_t  *item_types;
    Item       *item_locs;
    int  i, first_reset, last_reset, num_item_types;
    int best_loc;

    width = getparam_x_resolution();
    height = getparam_y_resolution();

    fpsCounter.update();

    update_audio();

    clear_rendering_context();

    fogPlane.setup();

    update_player_pos( players[0], EPS );
	
    update_view( players[0], EPS );

    setup_view_frustum( players[0], NEAR_CLIP_DIST, 
			getparam_forward_clip_distance() );

    draw_sky(players[0].view.pos);

    draw_fog_plane();

    set_course_clipping( true );
    set_course_eye_point( players[0].view.pos );
    setup_course_lighting();
    render_course();
    draw_trees();

    if ((elapsed_time > BLINK_IN_PLACE_TIME) && (!m_positionReset)) {
	item_types = get_item_types();
	item_locs  = get_item_locs();
	num_item_types = get_num_item_types();
	first_reset = 0;
	last_reset = 0;
	for ( i = 0; i < num_item_types; i++ ) {
	    if (item_types[i].reset_point == true) {
		last_reset = first_reset + item_types[i].num_items - 1;
		break;
	    } else {
		first_reset += item_types[i].num_items;
	    }
	}

	if (last_reset == 0) {
	    // didn't find a reset point item type 
	    get_course_dimensions( &course_width, &course_length );
	    players[0].pos.x = course_width/2.0;
	    players[0].pos.z = MIN(players[0].pos.z + 10, -1.0);
	} else {
	    // BFI 
	    best_loc = -1;
	    for ( i = first_reset; i <= last_reset; i++) {
		if (item_locs[i].ray.pt.z > players[0].pos.z ) { 
		    if (best_loc == -1 || 
			item_locs[i].ray.pt.z < item_locs[best_loc].ray.pt.z)
		    {
			best_loc = i;
		    }
		}
	    }

	    if ( best_loc == -1 ) {
		get_course_dimensions( &course_width, &course_length );
		players[0].pos.x = course_width/2.0;
		players[0].pos.z = MIN(players[0].pos.z + 10, -1.0);
	    } else if ( item_locs[best_loc].ray.pt.z <= players[0].pos.z ) {
		get_course_dimensions( &course_width, &course_length );
		players[0].pos.x = course_width/2.0;
		players[0].pos.z = MIN(players[0].pos.z + 10, -1.0);
	    } else {
		players[0].pos.x = item_locs[best_loc].ray.pt.x;
		players[0].pos.z = item_locs[best_loc].ray.pt.z;
	    }
	}

	// Re-initialize the camera 
	players[0].view.initialized = false;

	init_physical_simulation();
	m_positionReset = true;
    }

    if (tux_visible) { 
	ModelHndl->draw_tux();
	draw_tux_shadow();
    } 
    if (++tux_visible_count > 3) {
	tux_visible = (bool) !tux_visible;
	tux_visible_count = 0;
    }

    HUD1.draw(players[0]);

    reshape( width, height );

    winsys_swap_buffers();

    gameMgr->time += timeStep;

    if (elapsed_time > TOTAL_RESET_TIME) {
		set_game_mode( RACING );
		winsys_post_redisplay();
    }	
}
