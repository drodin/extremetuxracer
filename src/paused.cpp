/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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

#include "paused.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "env.h"
#include "hud.h"
#include "track_marks.h"
#include "particles.h"
#include "textures.h"
#include "game_ctrl.h"
#include "tux.h"
#include "racing.h"
#include "winsys.h"
#include "physics.h"

CPaused Paused;

static bool sky = true;
static bool fog = true;
static bool terr = true;
static bool trees = true;

void CPaused::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case sf::Keyboard::S:
			Winsys.TakeScreenshot();
			break;
		case sf::Keyboard::F5:
			sky = !sky;
			break;
		case sf::Keyboard::F6:
			fog = !fog;
			break;
		case sf::Keyboard::F7:
			terr = !terr;
			break;
		case sf::Keyboard::F8:
			trees = !trees;
			break;
		default:
			State::manager.RequestEnterState(Racing);
	}
}

void CPaused::Mouse(int button, int state, int x, int y) {
	State::manager.RequestEnterState(Racing);
}

// ====================================================================

void CPaused::Loop(float time_step) {
	CControl *ctrl = g_game.player->ctrl;
	int width = Winsys.resolution.width;
	int height = Winsys.resolution.height;

	ClearRenderContext();
	Env.SetupFog();
	update_view(ctrl, 0);
	SetupViewFrustum(ctrl);

	if (sky) Env.DrawSkybox(ctrl->viewpos);
	if (fog) Env.DrawFog();
	Env.SetupLight();
	if (terr) RenderCourse();
	DrawTrackmarks();
	if (trees) DrawTrees();

	DrawSnow(ctrl);

	if (param.perf_level > 2) draw_particles(ctrl);
	g_game.character->shape->Draw();

	DrawHud(ctrl);
	Reshape(width, height);
	Winsys.SwapBuffers();
}
