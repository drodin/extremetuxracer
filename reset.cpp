/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "reset.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "env.h"
#include "hud.h"
#include "course.h"
#include "track_marks.h"
#include "game_ctrl.h"
#include "racing.h"
#include "winsys.h"
#include "physics.h"

#define BLINK_IN_PLACE_TIME 0.5
#define TOTAL_RESET_TIME 1.0

CReset Reset;

static double reset_start_time;
static bool position_reset;


//=====================================================================
void CReset::Enter() {
    reset_start_time = Winsys.ClockTime ();
    position_reset = false;
}

void CReset::Loop(double time_step) {
	CControl *ctrl = Players.GetCtrl (g_game.player_id);
    double elapsed_time = Winsys.ClockTime () - reset_start_time;
    static bool tux_visible = true;
    static int tux_visible_count = 0;

    check_gl_error();
	ClearRenderContext ();
    Env.SetupFog ();
    ctrl->UpdatePlayerPos (EPS);
    update_view (ctrl, EPS);
    SetupViewFrustum (ctrl);
    Env.DrawSkybox (ctrl->viewpos);
    Env.DrawFog ();
	Env.SetupLight ();	// and fog
    RenderCourse();
    DrawTrackmarks ();
    DrawTrees ();

    if ((elapsed_time > BLINK_IN_PLACE_TIME) && (!position_reset)) {
		TObjectType* object_types = &Course.ObjTypes[0];
		TItem* item_locs  = &Course.NocollArr[0];
		size_t num_item_types = Course.ObjTypes.size();
		size_t first_reset = 0;
		size_t last_reset = 0;
		for (size_t i = 0; i < num_item_types; i++) {
		    if (object_types[i].reset_point == true) {
				last_reset = first_reset + object_types[i].num_items - 1;
				break;
		    } else {
				first_reset += object_types[i].num_items;
		    }
		} // for

		if (last_reset == 0) {
		    ctrl->cpos.x = Course.GetDimensions().x/2.0;
		    ctrl->cpos.z = min(ctrl->cpos.z + 10, -1.0);
		} else {
			int best_loc = -1;
		    for (size_t i = first_reset; i <= last_reset; i++) {
				if (item_locs[i].pt.z > ctrl->cpos.z) {
				    if (best_loc == -1 || item_locs[i].pt.z < item_locs[best_loc].pt.z) {
						best_loc = (int)i;
				    } // if
				} // if
		    } // for

		    if (best_loc == -1) {
				ctrl->cpos.x = Course.GetDimensions().x/2.0;
				ctrl->cpos.z = min (ctrl->cpos.z + 10, -1.0);
		    } else if (item_locs[best_loc].pt.z <= ctrl->cpos.z) {
				ctrl->cpos.x = Course.GetDimensions().x/2.0;
				ctrl->cpos.z = min (ctrl->cpos.z + 10, -1.0);
		    } else {
				ctrl->cpos.x = item_locs[best_loc].pt.x;
				ctrl->cpos.z = item_locs[best_loc].pt.z;
		    } // if
		}

		ctrl->view_init = false;
		ctrl->Init ();
		position_reset = true;
    } // if elapsed time

    if (tux_visible) Char.Draw (g_game.char_id);

    if (++tux_visible_count > 3) {
		tux_visible = (bool) !tux_visible;
		tux_visible_count = 0;
    }

    DrawHud (ctrl);
	Reshape (Winsys.resolution.width, Winsys.resolution.height);
    Winsys.SwapBuffers ();
    g_game.time += time_step;

    if (elapsed_time > TOTAL_RESET_TIME) {
		State::manager.RequestEnterState (Racing);
    }
}
