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
#include "particles.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "course.h"
#include "spx.h"
#include "game_type_select.h"

CScore Score;

int CScore::AddScore (size_t list_idx, const TScore& score) {
	if (list_idx >= Scorelist.size()) return 999;
	if (score.points < 1) return 999;

	TScoreList *list = &Scorelist[list_idx];
	int num = list->numScores;
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
			for (int i=lastpos; i>pos; i--) list->scores[i] = list->scores[i-1];
			list->scores[pos] = score;
		}
	} else {
		while (val <= list->scores[pos].points && pos < num) pos++;
		for (int i=num; i>pos; i--) list->scores[i] = list->scores[i-1];
		list->scores[pos] = score;
		list->numScores++;
	}
	return pos;
}

// for testing:
void CScore::PrintScorelist (size_t list_idx) {
	if (list_idx >= Scorelist.size()) return;
	TScoreList *list = &Scorelist[list_idx];

	if (list->numScores < 1) {
		PrintString ("no entries in this score list");
	} else {
		for (int i=0; i<list->numScores; i++) {
			string line = "player: " + list->scores[i].player;
			line += " points: " + Int_StrN (list->scores[i].points);
			line += " herrings: " + Int_StrN (list->scores[i].herrings);
			line += " time: " + Float_StrN (list->scores[i].time, 2);
			PrintString (line);
		}
	}
}

TScoreList *CScore::GetScorelist (size_t list_idx) {
	if (list_idx >= Scorelist.size()) return NULL;
	return &Scorelist[list_idx];
}

bool CScore::SaveHighScore () {
	CSPList splist ((int)Scorelist.size()*MAX_SCORES);

	for (size_t li=0; li<Scorelist.size(); li++) {
		TScoreList* lst = &Scorelist[li];
		if (lst != NULL) {
			int num = lst->numScores;
			if (num > 0) {
				for (int sc=0; sc<num; sc++) {
					TScore score = lst->scores[sc];
					string line = "*[course] " + Course.CourseList[li].dir;
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
	
	if (!list.Load (param.config_dir, "highscore")) {
		Message ("could not load highscore list");
		return false;
	}

	TScore score;

	Scorelist.resize(Course.CourseList.size());
	for (size_t i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		string course = SPStrN (line, "course", "unknown");
		size_t cidx = Course.GetCourseIdx (course);

		score.player = SPStrN (line, "plyr", "unknown");
		score.points = SPIntN (line, "pts", 0);
		score.herrings = SPIntN (line, "herr", 0);
		score.time = SPFloatN (line, "time", 0);
			
		AddScore (cidx, score);
	}
	return true;
}

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

	return AddScore (g_game.course_id, TempScore);
}

// --------------------------------------------------------------------
//				score screen
// --------------------------------------------------------------------

static TVector2 cursor_pos(0, 0);
static TCourse *CourseList;
static TUpDown* course;
static TWidget* textbutton;

void CScore::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	KeyGUI(key, release);
	if (release) return;
	switch (key) {
		case 27: State::manager.RequestEnterState (GameTypeSelect); break;
		case SDLK_q: State::manager.RequestQuit(); break;
		case SDLK_s: Score.SaveHighScore (); break;
		case SDLK_l: Score.LoadHighScore (); break;
		case 13: State::manager.RequestEnterState (GameTypeSelect); break;
	}
}

void CScore::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* clicked = ClickGUI(x, y);
		if (clicked == textbutton)
			State::manager.RequestEnterState (GameTypeSelect); 
	}
}

void CScore::Motion (int x, int y ){
	MouseMoveGUI(x, y);
	y = param.y_resolution - y;

    TVector2 old_pos = cursor_pos;
    cursor_pos = TVector2(x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

static TArea area;
static int framewidth, frameheight, frametop;
static int linedist, listtop;
static int dd1, dd2, dd3, dd4;

void CScore::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);    
	Winsys.KeyRepeat (true);
	Music.Play (param.menu_music, -1);

	framewidth = 550 * param.scale;
	frameheight = 50 * param.scale;
	frametop = AutoYPosN (32);
	area = AutoAreaN (30, 80, framewidth);
	FT.AutoSizeN (3);
	linedist = FT.AutoDistanceN (1);
	listtop = AutoYPosN (44);
	dd1 = 50 * param.scale;
	dd2 = 115 * param.scale;
	dd3 = 250 * param.scale;
	dd4 = 375 * param.scale;

	CourseList = &Course.CourseList[0];

	ResetGUI ();
	course = AddUpDown(area.right + 8, frametop, 0, (int)Course.CourseList.size()-1, 0);
	int siz = FT.AutoSizeN (5);
	textbutton = AddTextButton (Trans.Text(64), CENTER, AutoYPosN (80), siz);

	g_game.loopdelay = 1;
}

const string ordinals[10] = 
	{"1:st", "2:nd", "3:rd", "4:th", "5:th", "6:th", "7:th", "8:th", "9:th", "10:th"};

void CScore::Loop (double timestep ){
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
	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), param.scale);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top, 
//			0, colMBackgr, colBlack, 0.2);

	FT.AutoSizeN (7);
	FT.SetColor (colWhite);
	FT.DrawString (CENTER, AutoYPosN (22), Trans.Text(62));

	DrawFrameX (area.left, frametop, framewidth, frameheight, 3, colMBackgr, colDYell, 1.0);
	FT.AutoSizeN (5);
	FT.SetColor (colWhite);
	FT.DrawString (area.left+20, frametop, CourseList[course->GetValue()].name);

	TScoreList *list = Score.GetScorelist (course->GetValue());

	FT.SetColor (colWhite);
	if (list != NULL) {
		FT.AutoSizeN (3);
		if (list->numScores < 1) {
			FT.DrawString (CENTER, area.top + 140, Trans.Text(63));
		} else {
			if (list->numScores > MAX_SCORES) list->numScores = MAX_SCORES;
			for (int i=0; i<list->numScores; i++) {
				int y = listtop + i*linedist;
				FT.DrawString (area.left, y, ordinals[i]);
				FT.DrawString (area.left + dd1, y, Int_StrN (list->scores[i].points));
				FT.DrawString (area.left + dd2, y, list->scores[i].player);
				FT.DrawString (area.left + dd3, y, 
					Int_StrN (list->scores[i].herrings) + "  herrings");
				FT.DrawString (area.left + dd4, y, 
					Float_StrN (list->scores[i].time, 1) + "  sec");
			}
		}
	} else Message ("score list out of range");

	DrawGUI();

    Winsys.SwapBuffers();
}
