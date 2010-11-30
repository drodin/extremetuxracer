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
#include "keyframe.h"
#include "score.h"

static TVector2 cursor_pos = {0, 0 };
static CKeyframe *final_frame;
static int highscore_pos = 999;

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

void GameOverMotionFunc  (int x, int y) {
    TVector2 old_pos;
    
	if (Winsys.ModePending ()) return;
    y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y); 
    if  (old_pos.x != x || old_pos.y != y)  {
    }
}

void DrawMessageFrame (float x, float y, float w, float h, int line, 
		TColor backcol, TColor framecol, float transp) {

	float yy = param.y_resolution - y - h; 
	if (x < 0) 	x = (param.x_resolution - w) / 2;

	glPushMatrix();
	glDisable (GL_TEXTURE_2D);
    
	glColor4f (framecol.r, framecol.g, framecol.b, transp); 
	glTranslatef (x, yy, 0);
	glBegin (GL_QUADS );
	    glVertex2f (0, 0 );
	    glVertex2f (w, 0 );
	    glVertex2f (w, h );
	    glVertex2f (0, h );
	glEnd();

	glColor4f (backcol.r, backcol.g, backcol.b, transp);
	glBegin (GL_QUADS );
	    glVertex2f (0 + line, 0 + line );
	    glVertex2f (w - line, 0 + line );
	    glVertex2f (w - line, h - line );
	    glVertex2f (0 + line, h - line );
	glEnd();

	glEnable (GL_TEXTURE_2D);
    glPopMatrix();
}


void GameOverMessage (CControl *ctrl) {
	int fwidth = 500;

	string line;
	float leftframe = (param.x_resolution - fwidth) / 2;
	float topframe = 80;

	TColor backcol = MakeColor (1, 1, 1, 1);
	TColor framecol = MakeColor (0.7, 0.7, 1, 1);

	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	if (g_game.raceaborted) {
		DrawMessageFrame (leftframe, topframe, fwidth, 100, 4, backcol, framecol, 0.5);
		FT.SetColor (colDBlue);
		FT.DrawString (CENTER, topframe+30, Trans.Text(25));
	} else {
		DrawMessageFrame (leftframe, topframe, fwidth, 210, 4, backcol, framecol, 0.5);
		string line;

		if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (14);
		if (g_game.race_result >= 0 || g_game.game_type != CUPRACING) FT.SetColor (colDBlue);
			else FT.SetColor (colDRed);

		line = "Score:  ";
		FT.DrawString (leftframe+80, topframe+15, line);
		line = Int_StrN (g_game.score);
		line += "  pts";
		FT.DrawString (leftframe+240, topframe+15, line);

		line = "Herring:  ";
		FT.DrawString (leftframe+80, topframe+40, line);
		line = Int_StrN (g_game.herring);
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Int_StrN (g_game.herring_req.i);
			line += ")";
		}
		FT.DrawString (leftframe+240, topframe+40, line);

		line = "Time:  ";
		FT.DrawString (leftframe+80, topframe+65, line);
		line = Float_StrN (g_game.time, 2);
		line += "  s";
		if (g_game.game_type == CUPRACING) {
			line += "  (";
			line += Float_StrN (g_game.time_req.x, 2);
			line += ")";
		}
		FT.DrawString (leftframe+240, topframe+65, line);
		
		line = "Path length:  ";
		FT.DrawString (leftframe+80, topframe+90, line);
		line = Float_StrN (ctrl->way, 2);
		line += "  m";
		FT.DrawString (leftframe+240, topframe+90, line);
			
		line = "Average speed:  ";
		FT.DrawString (leftframe+80, topframe+115, line);		
		line = Float_StrN (ctrl->way / g_game.time * 3.6, 2);
		line += "  km/h"; 
		FT.DrawString (leftframe+240, topframe+115, line);		

		if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
		if (g_game.game_type == CUPRACING) {
			switch (g_game.race_result) {
				case -1: FT.DrawString (CENTER, topframe+150, Trans.Text(21)); break;
				case 0: FT.DrawString (CENTER, topframe+150, Trans.Text(22)); break;
				case 1: FT.DrawString (CENTER, topframe+150, Trans.Text(23)); break;
				case 2: FT.DrawString (CENTER, topframe+150,  Trans.Text(24)); break;
			}
		} else {
			if (highscore_pos < 5) {
				line = "Position ";
				line += Int_StrN (highscore_pos + 1);
				line += " in highscore list";
				FT.DrawString (CENTER, topframe+150, line);
			}
		}
	}
}

// =========================================================================
void GameOverInit (void) {
	Sound.HaltAll ();

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

	if (!g_game.raceaborted) highscore_pos = Score.CalcRaceResult ();

	if (g_game.raceaborted || !g_game.use_keyframe) {
		final_frame = NULL;
	} else {
		if (g_game.game_type == CUPRACING) {
			if (g_game.race_result < 0) final_frame = 
				Char.GetKeyframe (g_game.char_id, LOSTRACE); 
				else final_frame = Char.GetKeyframe (g_game.char_id, WONRACE);
		} else final_frame = Char.GetKeyframe (g_game.char_id, FINISH);

		if (!g_game.raceaborted) {
			CControl *ctrl = Players.GetCtrl (g_game.player_id);
			final_frame->Init (ctrl->cpos, -0.18);
		}
	}
	SetStationaryCamera (true);
}


void GameOverLoop (double time_step) {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);
    int width, height;
    width = param.x_resolution;
    height = param.y_resolution;
    check_gl_error();

	Music.Update ();

	ClearRenderContext ();
    Env.SetupFog ();

	update_view (ctrl, 0); 

	if (final_frame != NULL) final_frame->Update (time_step, ctrl);

    SetupViewFrustum (ctrl);
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

    set_gl_options (GUI); 
	SetupGuiDisplay ();
	if (final_frame != NULL) {
		if (!final_frame->active) GameOverMessage (ctrl);
	} else GameOverMessage (ctrl);
	DrawHud (ctrl);
    Reshape (width, height);
    Winsys.SwapBuffers ();
} 

void GameOverTerm () {
}


void game_over_register() {
	Winsys.SetModeFuncs (GAME_OVER, GameOverInit, GameOverLoop, GameOverTerm,
 		GameOverKeys, mouse_cb, GameOverMotionFunc, NULL, NULL, NULL);
}


