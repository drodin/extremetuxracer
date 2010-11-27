/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#include "score.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "tux.h"
#include "env.h"
#include "particles.h"
#include "credits.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"

CScore Score;

CScore::CScore () {}

void CScore::ResetScorelist (int list_idx) {
	TScoreList *list = &Scorelist[list_idx];
	for (int i=0; i<MAX_SCORES; i++) {
		list->scores[i].player = "";
		list->scores[i].points = 0;
		list->scores[i].herrings = 0;
		list->scores[i].time = 0.0;
	}
	list->numScores = 0;
}

void CScore::AddScore (int list_idx, TScore score) {
	if (list_idx < 0 || list_idx >= MAX_COURSES) return;
	if (score.points < 1) return;

	TScoreList *list = &Scorelist[list_idx];
	int num = list->numScores;
	int i;
	int pos = 0;
	int lastpos = num-1;
	int val = score.points;

	if (num == 0) {
		list->scores[0] = score;
		list->numScores++;
	} else if (num == MAX_SCORES) {
		while (val <= list->scores[pos].points && pos < num) pos++;
		if (pos == lastpos) {
			list->scores[pos] = score;
		} else if (pos < lastpos) {
			for (i=lastpos; i>pos; i--) list->scores[i] = list->scores[i-1];
			list->scores[pos] = score;
		}
	} else {
		while (val <= list->scores[pos].points && pos < num) pos++;
		for (i=num; i>pos; i--) list->scores[i] = list->scores[i-1];
		list->scores[pos] = score;
		list->numScores++;
	}
}

void CScore::PrintScorelist (int list_idx) {
	if (list_idx < 0 || list_idx >= MAX_COURSES) return;
	TScoreList *list = &Scorelist[list_idx];
	string line;

	if (list->numScores < 1) {
		PrintString ("no entries in this score list");
	} else {
		for (int i=0; i<list->numScores; i++) {
			line = "player: " + list->scores[i].player;
			line += " points: " + Int_StrN (list->scores[i].points);
			line += " herrings: " + Int_StrN (list->scores[i].herrings);
			line += " time: " + Float_StrN (list->scores[i].time, 2);
			PrintString (line);
		}
	}
}

void CScore::SetScorelist (int list_idx) {
	if (list_idx < 0 || list_idx >= MAX_COURSES) return;
	ResetScorelist (list_idx);
}

TScoreList *CScore::GetScorelist (int list_idx) {
	if (list_idx < 0 || list_idx >= MAX_COURSES) return NULL;
	return &Scorelist[list_idx];
}

bool CScore::SaveHighScore () {
	CSPList splist (520);
	string line;
	int li, sc, num;
	string coursedir;

	TCourse *courselist = Course.CourseList;
	TScoreList *lst;
	TScore score;
	
	for (li=0; li<MAX_COURSES; li++) {
		lst = &Scorelist[li];
		if (lst != NULL) {
			coursedir = courselist[li].dir;
			num = lst->numScores;
			if (num > 0) {
				for (sc=0; sc<num; sc++) {
					score = lst->scores[sc];
					line = "*[course] " + courselist[li].dir;
					line += " [plyr] " + score.player;
					line += " [pts] " + Int_StrN (score.points);
					line += " [herr] " + Int_StrN (score.herrings);
					line += " [time] " + Float_StrN (score.time, 1);
					splist.Add (line);
				}
			}
		}
	}
	if (!splist.Save (param.config_dir, "highscore")) {
		Message ("could not save highscore list");
		return false;
	}
	return true;
}

bool CScore::LoadHighScore () {
	CSPList list (520);
	string line, course;
	int i, cidx;
	TScore score;
	
	if (!list.Load (param.config_dir, "highscore")) {
		Message ("could not load highscore list");
		return false;
	}
	for (i=0; i<MAX_COURSES; i++) ResetScorelist (i);

	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		course = SPStrN (line, "course", "unknown");
		cidx = Course.GetCourseIdx (course);

		score.player = SPStrN (line, "plyr", "unknown");
		score.points = SPIntN (line, "pts", 0);
		score.herrings = SPIntN (line, "herr", 0);
		score.time = SPFloatN (line, "time", 0);
			
		Score.AddScore (cidx, score);
	}
	return true;
}

// immmer in init
int CScore::CalcRaceResult () {
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

	TempScore.points = g_game.score;
	TempScore.herrings = g_game.herring;
	TempScore.time = g_game.time;
	TempScore.player = Players.GetName (g_game.player_id);
	AddScore (g_game.course_id, TempScore);

	return g_game.race_result;
}

// --------------------------------------------------------------------
//				score screen
// --------------------------------------------------------------------

static int curr_focus = 0;
static TVector2 cursor_pos = {0, 0};
static int xleft, ytop;
static TCourse *CourseList;
static int lastCourse = 0;
static int curr_course = 0;

void ChangeScoreSelection (int focus, int dir) {
	if (dir == 0) {
		if (curr_course > 0) curr_course--;
	} else {
		if (curr_course < lastCourse) curr_course++;
	}
}

static TScore aaa;
void ScoreKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case 27: Winsys.SetMode (GAME_TYPE_SELECT); break;
		case SDLK_q: Winsys.Quit (); break;
		case SDLK_DOWN: ChangeScoreSelection (curr_focus, 1); break;
		case SDLK_UP: ChangeScoreSelection (curr_focus, 0); break;
		case SDLK_LEFT: ChangeScoreSelection (curr_focus, 0); break;
		case SDLK_RIGHT: ChangeScoreSelection (curr_focus, 1); break;
		case SDLK_s: Score.SaveHighScore (); break;
		case SDLK_l: Score.LoadHighScore (); break;

	// for testing:
		case SDLK_F12: 
			aaa.player = Players.GetName (g_game.player_id);
			aaa.points = IRandom (1, 10000);
			aaa.herrings = IRandom (15, 70);
			aaa.time = XRandom (20, 160);
			Score.AddScore (curr_course, aaa);
			break;		
	}
}

void ScoreMouseFunc (int button, int state, int x, int y) {
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		ChangeScoreSelection (foc, dir);
	}
}

void ScoreMotionFunc (int x, int y ){
    TVector2 old_pos;
 	int sc, dir;
	if (Winsys.ModePending ()) return; 

	GetFocus (x, y, &sc, &dir);
	if (sc >= 0) curr_focus = sc;
	y = param.y_resolution - y;

    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

void ScoreInit (void) {  
	Winsys.ShowCursor (!param.ice_cursor);    
	Winsys.KeyRepeat (true);
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);

	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (150);

	CourseList = Course.CourseList;
	lastCourse = Course.numCourses - 1;
	curr_course = g_game.course_id;

	ResetWidgets ();
	AddArrow (xleft + 470, ytop+80, 0, 0);
	AddArrow (xleft + 470, ytop+98, 1, 0);

	g_game.loopdelay = 1;
}

const string ordinals[10] = 
	{"1:st", "2:nd", "3:rd", "4:th", "5:th", "6:th", "7:th", "8:th", "9:th", "10:th"};

void ScoreLoop (double timestep ){
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();
	update_ui_snow (timestep);
	draw_ui_snow();

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);

	if (param.use_papercut_font > 0) FT.SetSize (42); else FT.SetSize (33);
	FT.SetColor (colWhite);
	FT.DrawString (-1, ytop, "Highscore list");

	DrawFrameX (xleft, ytop+76, 460, 44, 3, colMBackgr, colDYell, 1.0);
	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	FT.SetColor (colWhite);
	FT.DrawString (xleft+20, ytop+80, CourseList[curr_course].name);

	PrintArrow (0, (curr_course > 0));	
	PrintArrow (1, (curr_course < lastCourse));

	TScoreList *list = Score.GetScorelist (curr_course);
	string line;
	int y;

	FT.SetColor (colWhite);
	if (list != NULL) {
		if (param.use_papercut_font > 0) FT.SetSize (22); else FT.SetSize (16);
		if (list->numScores < 1) {
			FT.DrawString (-1, ytop + 140, "No entries for this race");
		} else {
			if (list->numScores > MAX_SCORES) list->numScores = MAX_SCORES;
			for (int i=0; i<list->numScores; i++) {
				y = ytop + i*30 + 140;
				FT.DrawString (xleft, y, ordinals[i]);
				FT.DrawString (xleft+50, y, Int_StrN (list->scores[i].points));
				FT.DrawString (xleft + 115, y, list->scores[i].player);
				FT.DrawString (xleft + 250, y, 
					Int_StrN (list->scores[i].herrings) + "  herrings");
				FT.DrawString (xleft + 375, y, 
					Float_StrN (list->scores[i].time, 1) + "  sec");
			}
		}
	} else Message ("score list out of range");

	if (param.ice_cursor) DrawCursor ();
    Winsys.SwapBuffers();
} 

void ScoreTerm () {}

void RegisterScoreFunctions () {
	Winsys.SetModeFuncs (SCORE,  ScoreInit,  ScoreLoop,  ScoreTerm,
 		 ScoreKeys,  ScoreMouseFunc,  ScoreMotionFunc, NULL, NULL, NULL);
}



