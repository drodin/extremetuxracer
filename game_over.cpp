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
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "tux.h"
#include "env.h"
#include "hud.h"
#include "track_marks.h"
#include "particles.h"
#include "textures.h"
#include "course.h"
#include "gui.h"
#include "font.h"
#include "spx.h"
#include "game_ctrl.h"
#include "translation.h"

static TVector2 cursor_pos = {0, 0 };

void CalcScore (CControl *ctrl){
    double par_time;
	double w, l;
	
	Course.GetDimensions (&w, &l);
	par_time = l * 0.03;
    g_game.score = max (0, (int)(100 * (par_time - g_game.time) 
		+ 200 * g_game.herring) );
}

void QuitGameOver () {
    if (g_game.game_type == PRACTICING) {
		Winsys.SetMode (RACE_SELECT);
	} else {
		Winsys.SetMode (EVENT);
	}
}

void GameOverKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	if (key == 13 || key == 27) QuitGameOver ();
}

static void mouse_cb (int button, int state, int x, int y) {
	QuitGameOver ();
}

void CalcRaceResult () {
	g_game.race_result = -1;
	if (g_game.time <= g_game.time_req.x && 
		g_game.herring >= g_game.herring_req.i) g_game.race_result = 0;
	if (g_game.time <= g_game.time_req.y && 
		g_game.herring >= g_game.herring_req.j) g_game.race_result = 1;
	if (g_game.time <= g_game.time_req.z && 
		g_game.herring >= g_game.herring_req.k) g_game.race_result = 2;

	double ll, ww;
	Course.GetDimensions (&ww, &ll);
	double herringpt = g_game.herring * 10;
	double timept = ll - (g_game.time * 10);
	g_game.score = (int)(herringpt + timept);
	if (g_game.score < 0) g_game.score = 0;
}

void GameOverMotionFunc  (int x, int y) {
    TVector2 old_pos;
    
	if (Winsys.ModePending ()) return;
    y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y); 
    if  (old_pos.x != x || old_pos.y != y)  {
    }
}

void GameOverMessage () {
	int hh = param.y_resolution;
	char buff[64];
    CControl *ctrl = Players.GetControl (0);
	int fheight = 75;
	int fwidth = 500;
	char s[128];

	DrawFrameX (CENTER, hh-90, fwidth, fheight, 2, colWhite, colSky, 0.4);
	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);

	if (!g_game.raceaborted) {
		if (g_game.game_type == PRACTICING) {
			CalcScore (ctrl);
			sprintf (buff, "%d", g_game.score);
			strcpy (s, "Score:  ");
			strcat (s, buff);
			FT.SetColor (colDBlue);
			FT.DrawText (CENTER, hh-90+18, s);
		} else if (g_game.game_type == TRAINING) {

		} else if (g_game.game_type == CUPRACING) {
			if (g_game.race_result < 0) FT.SetColor (colRed); 
				else FT.SetColor (colDBlue);

			switch (g_game.race_result) {
				case -1: 
					FT.DrawString (CENTER, hh-90+8, Trans.Text(21));
					break;
				case 0: 
					FT.DrawString (CENTER, hh-90+4, Trans.Text(22));
					FT.DrawString (CENTER, hh-90+36,  Trans.Text(26) + 
						"  " + Int_StrN (g_game.score) + " " + Trans.Text(27));
					break;
				case 1: 
					FT.DrawString (CENTER, hh-90+4, Trans.Text(23));
					FT.DrawString (CENTER, hh-90+36, Trans.Text(26) +
						+ "  " + Int_StrN (g_game.score) +" " + Trans.Text(27));
					break;
				case 2: 
					FT.DrawString (CENTER, hh-90+4,  Trans.Text(24));
					FT.DrawString (CENTER, hh-90+36, Trans.Text (26) +
						+ "  " + Int_StrN (g_game.score) + " " + Trans.Text(27));
					break;
			}
		}
	} else {
		FT.SetColor (colDBlue);
		FT.DrawString (CENTER, hh-90+18, Trans.Text(25));
	}
}

// =========================================================================
void GameOverInit (void) {
	Sound.HaltAll ();
	CalcRaceResult ();
	if (g_game.game_type == CUPRACING) {
		if (g_game.race_result >= 0) {
			Music.PlayTheme (g_game.theme_id, MUS_WONRACE);
		} else {
			Music.PlayTheme (g_game.theme_id, MUS_LOSTRACE);
		}
	} else {
		if (g_game.raceaborted) {
			Music.PlayTheme (g_game.theme_id, MUS_LOSTRACE);
		} else {
			Music.PlayTheme (g_game.theme_id, MUS_WONRACE);
		}
	}
}

void GameOverLoop (double time_step) {
    CControl *ctrl = Players.GetControl (0);
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
    Env.DrawSkybox (ctrl->viewpos);
    Env.DrawFog ();
	Env.SetupLight ();	// and fog

    RenderCourse ();
    DrawTrackmarks ();
    DrawTrees ();

	UpdateWind (time_step, ctrl);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);
	
	DrawTux2 ();

    set_gl_options (GUI); 
	SetupGuiDisplay ();
    GameOverMessage ();
	DrawHud (ctrl);
    Reshape (width, height);
    Winsys.SwapBuffers ();
} 

void GameOverTerm () {
}


void game_over_register() {
	Winsys.SetModeFuncs (GAME_OVER, GameOverInit, GameOverLoop, GameOverTerm,
 		GameOverKeys, mouse_cb, GameOverMotionFunc, NULL, NULL);
}


