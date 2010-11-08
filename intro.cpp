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

#include "audio.h"
#include "keyframe.h"
#include "course_render.h"
#include "ogl.h"
#include "view.h"
#include "tux.h"
#include "env.h"
#include "hud.h"
#include "course.h"
#include "track_marks.h"
#include "particles.h"
#include "game_ctrl.h"

void IntroKeys (unsigned int key, bool special, bool release, int x, int y);

void abort_intro (CControl *ctrl) {
	TVector2 start_pt = Course.GetStartPoint ();
    Winsys.SetMode (RACING);
    ctrl->orientation_initialized = false;
    ctrl->view_init = false;
    ctrl->cpos.x = start_pt.x;
    ctrl->cpos.z = start_pt.y;
}

// =================================================================
void intro_init(void) {
    int i, num_items;
    TItem *item_locs;

    CControl *ctrl = Players.GetControl (0);
    TVector2 start_pt = Course.GetStartPoint ();
	ctrl->orientation_initialized = false;
    ctrl->view_init = false;
    ctrl->cpos.x = start_pt.x;
    ctrl->cpos.z = start_pt.y;

	TuxStart.Init (ctrl->cpos, -0.05);

	// reset of result values
    g_game.herring = 0;
    g_game.score = 0;
    g_game.time = 0.0;
	g_game.race_result = -1; 
	g_game.raceaborted = false;

    ctrl->Init ();

    ctrl->cvel = MakeVector (0, 0, 0);
    clear_particles();
    set_view_mode (ctrl, ABOVE);
    update_view (ctrl, EPS); 

    num_items = Course.numNocoll;
    item_locs = Course.NocollArr;
    for (i = 0; i < num_items; i++) {
		if (item_locs[i].collectable != -1) {
		    item_locs[i].collectable = 1;
		}
    }

	InitSnow (ctrl);
	InitWind ();
		
	Music.PlayTheme (g_game.theme_id, MUS_RACING);
    param.show_hud = true;	
	g_game.loopdelay = 1;
}

void intro_loop (double time_step) {
	int width, height;
	CControl *ctrl = Players.GetControl (0);

    width = param.x_resolution;
    height = param.y_resolution;
    check_gl_error();

	if (TuxStart.active) TuxStart.Update (time_step, ctrl);
	else Winsys.SetMode (RACING);
    ClearRenderContext ();

	Env.SetupFog ();

    update_view (ctrl, time_step);
    SetupViewFrustum (ctrl);

	Music.Update ();    
    Env.DrawSkybox (ctrl->viewpos);

	Env.DrawFog ();
	Env.SetupLight ();
    RenderCourse ();
    DrawTrackmarks ();
    DrawTrees ();
	
	UpdateWind (time_step, ctrl);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);

	Char.Draw (g_game.char_id);
    DrawHud (ctrl);

    Reshape (width, height);
    Winsys.SwapBuffers ();

} 

void IntroTerm () {
}

// -----------------------------------------------------------------------

void IntroKeys (unsigned int key, bool special, bool release, int x, int y) {
	CControl *ctrl = Players.GetControl (0);
    if (release) return;
    abort_intro (ctrl);
}


void intro_register() {
	Winsys.SetModeFuncs (INTRO, intro_init, intro_loop, IntroTerm,
 		IntroKeys, NULL, NULL, NULL, NULL);
}

