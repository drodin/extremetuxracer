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
#include "tux.h"
#include "racing.h"
#include "winsys.h"
#include "physics.h"

#define BLINK_IN_PLACE_TIME 0.5
#define TOTAL_RESET_TIME 1.0

CReset Reset;

static sf::Clock reset_timer;
static bool position_reset;


//=====================================================================
void CReset::Enter() {
	reset_timer.restart();
	position_reset = false;
}

void CReset::Loop(float time_step) {
	CControl *ctrl = g_game.player->ctrl;
	float elapsed_time = reset_timer.getElapsedTime().asSeconds();
	static bool tux_visible = true;
	static int tux_visible_count = 0;

	ClearRenderContext();
	Env.SetupFog();
	ctrl->UpdatePlayerPos(EPS);
	update_view(ctrl, EPS);
	SetupViewFrustum(ctrl);
	Env.DrawSkybox(ctrl->viewpos);
	Env.DrawFog();
	Env.SetupLight();
	RenderCourse();
	DrawTrackmarks();
	DrawTrees();

	if (elapsed_time > BLINK_IN_PLACE_TIME && !position_reset) {
		// Determine optimal location for reset
		int best_loc = -1;
		for (std::size_t i = 0; i < Course.NocollArr.size(); i++) {
			if (Course.NocollArr[i].type.reset_point && Course.NocollArr[i].pt.z > ctrl->cpos.z) {
				if (best_loc == -1 || Course.NocollArr[i].pt.z < Course.NocollArr[best_loc].pt.z) {
					best_loc = (int)i;
				}
			}
		}

		if (best_loc == -1) { // Fallback in case there are no reset points
			ctrl->cpos.x = Course.GetDimensions().x/2.0;
			ctrl->cpos.z = std::min(ctrl->cpos.z + 10, -1.0);
		} else if (Course.NocollArr[best_loc].pt.z <= ctrl->cpos.z) {
			ctrl->cpos.x = Course.GetDimensions().x/2.0;
			ctrl->cpos.z = std::min(ctrl->cpos.z + 10, -1.0);
		} else {
			ctrl->cpos.x = Course.NocollArr[best_loc].pt.x;
			ctrl->cpos.z = Course.NocollArr[best_loc].pt.z;
		}

		ctrl->view_init = false;
		ctrl->Init();
		position_reset = true;
	}

	if (tux_visible) g_game.character->shape->Draw();

	if (++tux_visible_count > 3) {
		tux_visible = (bool) !tux_visible;
		tux_visible_count = 0;
	}

	DrawHud(ctrl);
	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
	g_game.time += time_step;

	if (elapsed_time > TOTAL_RESET_TIME) {
		State::manager.RequestEnterState(Racing);
	}
}
