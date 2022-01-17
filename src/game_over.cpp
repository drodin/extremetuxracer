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

#include "game_over.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "env.h"
#include "hud.h"
#include "track_marks.h"
#include "particles.h"
#include "gui.h"
#include "font.h"
#include "spx.h"
#include "game_ctrl.h"
#include "translation.h"
#include "score.h"
#include "race_select.h"
#include "event.h"
#include "winsys.h"
#include "physics.h"
#include "tux.h"

CGameOver GameOver;

static CKeyframe *final_frame;
static int highscore_pos = MAX_SCORES;

void QuitGameOver() {
	if (g_game.game_type == PRACTICING) {
		State::manager.RequestEnterState(RaceSelect);
	} else {
		State::manager.RequestEnterState(Event);
	}
}

void CGameOver::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	if (key == sf::Keyboard::Enter || key == sf::Keyboard::Escape) QuitGameOver();
}

void CGameOver::Mouse(int button, int state, int x, int y) {
	if (state) return;
	QuitGameOver();
}


void GameOverMessage(const CControl *ctrl) {
	int fwidth = 500 * Winsys.scale;

	int leftframe = (Winsys.resolution.width - fwidth) / 2;

	const sf::Color& backcol = colWhite;
	static const sf::Color framecol(178, 178, 255);

	if (param.use_papercut_font > 0) FT.AutoSizeN(6);
	else FT.AutoSizeN(5);
	if (g_game.raceaborted) {
		DrawFrameX(leftframe, AutoYPosN(10), fwidth, AutoYPosN(10), 4, backcol, framecol, 0.5f);
		FT.SetColor(colDBlue);
		FT.DrawString(CENTER, AutoYPosN(10), Trans.Text(25));
	} else {
		int firstMarker = leftframe + 60 * Winsys.scale;
		int secondMarker = leftframe + 310 * Winsys.scale;
    DrawFrameX(leftframe, AutoYPosN(10), fwidth, AutoYPosN(30), 4, backcol, framecol, 0.5f);

		if (param.use_papercut_font > 0) FT.AutoSizeN(4);
		else FT.AutoSizeN(3);
		if (g_game.race_result >= 0 || g_game.game_type != CUPRACING) FT.SetColor(colDBlue);
		else FT.SetColor(colDRed);

		sf::String line = Trans.Text(84) + ":  ";
		FT.DrawString(firstMarker, AutoYPosN(12), line);
		line = Int_StrN(g_game.score);
		line += "  pts";
		FT.DrawString(secondMarker, AutoYPosN(12), line);

		line = Trans.Text(85) + ":  ";
		FT.DrawString(firstMarker, AutoYPosN(17), line);
		line = Int_StrN(g_game.herring);
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Int_StrN(g_game.race->herrings.x);
			line += ')';
		}
		FT.DrawString(secondMarker, AutoYPosN(17), line);

		line = Trans.Text(86) + ":  ";
		FT.DrawString(firstMarker, AutoYPosN(22), line);
		line = Float_StrN(g_game.time, 2);
		line += "  s";
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Float_StrN(g_game.race->time.x, 2);
			line += ')';
		}
		FT.DrawString(secondMarker, AutoYPosN(22), line);

		line = Trans.Text(87) + ":  ";
		FT.DrawString(firstMarker, AutoYPosN(27), line);
		line = Float_StrN(ctrl->way, 2);
		line += "  m";
		FT.DrawString(secondMarker, AutoYPosN(27), line);

		line = Trans.Text(88) + ":  ";
		FT.DrawString(firstMarker, AutoYPosN(32), line);
		line = Float_StrN(ctrl->way / g_game.time * 3.6, 2);
		line += "  km/h";
		FT.DrawString(secondMarker, AutoYPosN(32), line);

		if (param.use_papercut_font > 0) FT.AutoSizeN(6);
		else FT.AutoSizeN(5);
		if (g_game.game_type == CUPRACING) {
			FT.DrawString(CENTER, AutoYPosN(40), Trans.Text(22 + g_game.race_result)); // Text IDs 21 - 24; race_results is in [-1; 2]
		} else {
			if (highscore_pos < MAX_SCORES) {
				line = Trans.Text(89) + ' ';
				line += Int_StrN(highscore_pos + 1);
				line += ' ' + Trans.Text(90);
				FT.DrawString(CENTER, AutoYPosN(40), line);
			}
		}
	}
}

// =========================================================================
void CGameOver::Enter() {
	if (!g_game.raceaborted) highscore_pos = Score.CalcRaceResult();

	if (g_game.game_type == CUPRACING) {
		if (g_game.race_result >= 0) {
			Music.PlayTheme(g_game.theme_id, MUS_WONRACE);
		} else {
			Music.PlayTheme(g_game.theme_id, MUS_LOSTRACE);
		}
	} else {
		if (g_game.raceaborted) {
			Music.PlayTheme(g_game.theme_id, MUS_LOSTRACE);
		} else {
			Music.PlayTheme(g_game.theme_id, MUS_WONRACE);
		}
	}


	if (g_game.raceaborted || !g_game.use_keyframe) {
		final_frame = nullptr;
	} else {
		if (g_game.game_type == CUPRACING) {
			if (g_game.race_result < 0)
				final_frame = g_game.character->GetKeyframe(LOSTRACE);
			else final_frame = g_game.character->GetKeyframe(WONRACE);
		} else final_frame = g_game.character->GetKeyframe(FINISH);

		if (!g_game.raceaborted) {
			const CControl *ctrl = g_game.player->ctrl;
			final_frame->Init(ctrl->cpos, -0.18);
		}
	}
	SetStationaryCamera(true);
}


void CGameOver::Loop(float time_step) {
	CControl *ctrl = g_game.player->ctrl;
	int width = Winsys.resolution.width;
	int height = Winsys.resolution.height;

	ClearRenderContext();
	Env.SetupFog();

	update_view(ctrl, 0);

	if (final_frame != nullptr) final_frame->Update(time_step);

	SetupViewFrustum(ctrl);
	Env.DrawSkybox(ctrl->viewpos);
	Env.DrawFog();
	Env.SetupLight();

	RenderCourse();
	DrawTrackmarks();
	DrawTrees();

	UpdateWind(time_step);
	UpdateSnow(time_step, ctrl);
	DrawSnow(ctrl);

	g_game.character->shape->Draw();

	{
		ScopedRenderMode rm(GUI);
		if (final_frame != nullptr) {
			if (!final_frame->active) GameOverMessage(ctrl);
		} else GameOverMessage(ctrl);
	}
	DrawHud(ctrl);
	Reshape(width, height);
	Winsys.SwapBuffers();
}
