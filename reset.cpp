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

#include "reset.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "tux.h"
#include "env.h"
#include "hud.h"
#include "course.h"
#include "track_marks.h"
#include "particles.h"
#include "game_ctrl.h"

#define BLINK_IN_PLACE_TIME 0.5
#define TOTAL_RESET_TIME 1.0

static double reset_start_time;
static bool position_reset;


//=====================================================================
void reset_init (void) {
    reset_start_time = Winsys.ClockTime ();
    position_reset = false;
}

void reset_loop (double time_step) {
    int width, height;
	CControl *ctrl = Players.GetControl (0);
    double elapsed_time = Winsys.ClockTime () - reset_start_time;
    double course_width, course_length;
    static bool tux_visible = true; 
    static int tux_visible_count = 0;
	TObjectType	*object_types;
    TItem *item_locs;
    int  i, first_reset, last_reset, num_item_types;
    int best_loc;

    width = param.x_resolution;
    height = param.y_resolution;

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
		object_types = Course.ObjTypes;
		item_locs  = Course.NocollArr;
		num_item_types = Course.numObjTypes;
		first_reset = 0;
		last_reset = 0;
		for  (i = 0; i < num_item_types; i++) {
		    if (object_types[i].reset_point == true) {
				last_reset = first_reset + object_types[i].num_items - 1;
				break;
		    } else {
				first_reset += object_types[i].num_items;
		    }
		} // for

		if (last_reset == 0) {
		    Course.GetDimensions (&course_width, &course_length);
		    ctrl->cpos.x = course_width/2.0;
		    ctrl->cpos.z = min(ctrl->cpos.z + 10, -1.0);
		} else {
		    best_loc = -1;
		    for  (i = first_reset; i <= last_reset; i++) {
				if (item_locs[i].pt.z > ctrl->cpos.z) { 
				    if (best_loc == -1 || item_locs[i].pt.z < item_locs[best_loc].pt.z) {
						best_loc = i;
				    } // if
				} // if
		    } // for

		    if  (best_loc == -1) {
				Course.GetDimensions (&course_width, &course_length);
				ctrl->cpos.x = course_width/2.0;
				ctrl->cpos.z = min (ctrl->cpos.z + 10, -1.0);
		    } else if  (item_locs[best_loc].pt.z <= ctrl->cpos.z) {
				Course.GetDimensions (&course_width, &course_length);
				ctrl->cpos.x = course_width/2.0;
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

    if (tux_visible) Tux.Draw ();

    if (++tux_visible_count > 3) {
		tux_visible = (bool) !tux_visible;
		tux_visible_count = 0;
    }

    DrawHud (ctrl);
    Reshape (width, height);
    Winsys.SwapBuffers ();
    g_game.time += time_step;

    if (elapsed_time > TOTAL_RESET_TIME) {
	Winsys.SetMode (RACING);
    }
} 

void reset_register() {
	Winsys.SetModeFuncs 
		(RESET, reset_init, reset_loop, NULL, NULL, NULL, NULL, NULL, NULL);
}


