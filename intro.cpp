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

#include "intro.h"
#include "audio.h"
#include "course_render.h"
#include "ogl.h"
#include "view.h"
#include "env.h"
#include "hud.h"
#include "course.h"
#include "track_marks.h"
#include "particles.h"
#include "game_ctrl.h"
#include "racing.h"
#include "winsys.h"
#include "physics.h"

CIntro Intro;
static CKeyframe *startframe;

void abort_intro (CControl *ctrl) {
	TVector2 start_pt = Course.GetStartPoint ();
    State::manager.RequestEnterState (Racing);
    ctrl->orientation_initialized = false;
    ctrl->view_init = false;
    ctrl->cpos.x = start_pt.x;
    ctrl->cpos.z = start_pt.y;
}

// =================================================================
void CIntro::Enter() {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);
    TVector2 start_pt = Course.GetStartPoint ();
	ctrl->orientation_initialized = false;
    ctrl->view_init = false;
    ctrl->cpos.x = start_pt.x;
    ctrl->cpos.z = start_pt.y;

	startframe = Char.GetKeyframe (g_game.char_id, START);
	if (startframe->loaded) {
//		startframe->Init (ctrl->cpos, -0.05);
		CCharShape *sh = Char.GetShape (g_game.char_id);
		startframe->Init (ctrl->cpos, -0.05, sh);
	}

	// reset of result values
    g_game.herring = 0;
    g_game.score = 0;
    g_game.time = 0.0;
	g_game.race_result = -1;
	g_game.raceaborted = false;

    ctrl->Init ();

    ctrl->cvel = TVector3(0, 0, 0);
    clear_particles();
    set_view_mode (ctrl, ABOVE);
	SetCameraDistance (4.0);
	SetStationaryCamera (false);
	update_view (ctrl, EPS);
    size_t num_items = Course.NocollArr.size();
    TItem* item_locs = &Course.NocollArr[0];
    for (size_t i = 0; i < num_items; i++) {
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

void CIntro::Loop (double time_step) {
	CControl *ctrl = Players.GetCtrl (g_game.player_id);
    int width = Winsys.resolution.width;
    int height = Winsys.resolution.height;
    check_gl_error();

	if (startframe->active) {
		startframe->Update (time_step);
	} else State::manager.RequestEnterState (Racing);

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

	UpdateWind (time_step);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);

	Char.Draw (g_game.char_id);
    DrawHud (ctrl);

    Reshape (width, height);
    Winsys.SwapBuffers ();

}
// -----------------------------------------------------------------------

void CIntro::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	CControl *ctrl = Players.GetCtrl (g_game.player_id);
    if (release) return;
    abort_intro (ctrl);
}
