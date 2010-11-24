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

#include "paused.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "tux.h"
#include "env.h"
#include "hud.h"
#include "track_marks.h"
#include "particles.h"
#include "textures.h"
#include "game_ctrl.h"

static bool sky = true;
static bool fog = true;
static bool terr = true;

void PausedKeys (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
		case SDLK_s: ScreenshotN (); break;
		case SDLK_F5: sky = !sky; break;
		case SDLK_F6: fog = !fog; break;
		case SDLK_F7: terr = !terr; break;
		default: Winsys.SetMode (RACING);
	}
}

static void PausedMouseFunc (int button, int state, int x, int y){
    Winsys.SetMode (RACING);
}

void PausedSetupDisplay () {
    double offset = 0.0;

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, param.x_resolution,  0, param.y_resolution, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef (offset, offset, -1.0);
    glColor4f (1.0, 1.0, 1.0, 1.0);
}

// ====================================================================

void paused_init (void) {}

void paused_loop (double time_step) {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);
    int width, height;
    width = param.x_resolution;
    height = param.y_resolution;
    check_gl_error();

	Music.Update ();
	ClearRenderContext ();
    Env.SetupFog ();
    ctrl->UpdatePlayerPos (0);
    update_view (ctrl, 0);
    SetupViewFrustum (ctrl);

    if (sky) Env.DrawSkybox (ctrl->viewpos);
    if (fog) Env.DrawFog ();
	Env.SetupLight ();
    if (terr) RenderCourse();
    DrawTrackmarks ();
    DrawTrees();
	
	UpdateWind (time_step, ctrl);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);

    if (param.perf_level > 2) draw_particles (ctrl);
	Char.Draw (g_game.char_id);

    set_gl_options (GUI);
	SetupGuiDisplay ();
	PausedSetupDisplay ();
	DrawHud (ctrl);
    Reshape (width, height);
    Winsys.SwapBuffers ();
} 

void paused_register() {
	Winsys.SetModeFuncs (PAUSED, paused_init, paused_loop, NULL,
 		PausedKeys, PausedMouseFunc, NULL, NULL, NULL, NULL);
}
