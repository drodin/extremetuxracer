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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "score.h"
#include "ogl.h"
#include "audio.h"
#include "gui.h"
#include "particles.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "course.h"
#include "spx.h"
#include "winsys.h"

CScore Score;

int CScore::AddScore(const TCourse* course, TScore&& score) {
	size_t list_idx = Course.GetCourseIdx(course);
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
void CScore::PrintScorelist(size_t list_idx) const {
	if (list_idx >= Scorelist.size()) return;
	const TScoreList *list = &Scorelist[list_idx];

	if (list->numScores < 1) {
		PrintStr("no entries in this score list");
	} else {
		for (int i=0; i<list->numScores; i++) {
			string line = "player: " + list->scores[i].player;
			line += " points: " + Int_StrN(list->scores[i].points);
			line += " herrings: " + Int_StrN(list->scores[i].herrings);
			line += " time: " + Float_StrN(list->scores[i].time, 2);
			PrintString(line);
		}
	}
}

const TScoreList *CScore::GetScorelist(size_t list_idx) const {
	if (list_idx >= Scorelist.size()) return nullptr;
	return &Scorelist[list_idx];
}

bool CScore::SaveHighScore() const {
	CSPList splist((int)Scorelist.size()*MAX_SCORES);

	const CCourseList* courses = &Course.CourseLists["default"]; // TODO: Save Highscore of all groups
	for (size_t li=0; li<Scorelist.size(); li++) {
		const TScoreList* lst = &Scorelist[li];
		if (lst != nullptr) {
			int num = lst->numScores;
			if (num > 0) {
				for (int sc=0; sc<num; sc++) {
					const TScore& score = lst->scores[sc];
					string line = "*[group] " + courses->name;
					line += " [course] " + (*courses)[li].dir;
					line += " [plyr] " + score.player;
					line += " [pts] " + Int_StrN(score.points);
					line += " [herr] " + Int_StrN(score.herrings);
					line += " [time] " + Float_StrN(score.time, 1);
					splist.Add(line);
				}
			}
		}
	}
	if (!splist.Save(param.config_dir, "highscore")) {
		Message("could not save highscore list");
		return false;
	}
	return true;
}

bool CScore::LoadHighScore() {
	CSPList list(520);

	if (Course.currentCourseList)
		Scorelist.resize(Course.currentCourseList->size());

	if (!list.Load(param.config_dir, "highscore")) {
		Message("could not load highscore list");
		return false;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		string group = SPStrN(*line, "group", "default");
		string course = SPStrN(*line, "course", "unknown");
		try {
			TCourse* cidx = Course.GetCourse(group, course);

			AddScore(cidx, TScore(
			             SPStrN(*line, "plyr", "unknown"),
			             SPIntN(*line, "pts", 0),
			             SPIntN(*line, "herr", 0),
			             SPFloatN(*line, "time", 0)));
		} catch (std::exception&)
		{ }
	}
	return true;
}

int CScore::CalcRaceResult() {
	g_game.race_result = -1;
	if (g_game.game_type == CUPRACING) {
		if (g_game.time <= g_game.race->time.x &&
		        g_game.herring >= g_game.race->herrings.x) g_game.race_result = 0;
		if (g_game.time <= g_game.race->time.y &&
		        g_game.herring >= g_game.race->herrings.y) g_game.race_result = 1;
		if (g_game.time <= g_game.race->time.z &&
		        g_game.herring >= g_game.race->herrings.z) g_game.race_result = 2;
	}

	int herringpt = g_game.herring * 10;
	double timept = Course.GetDimensions().y - (g_game.time * 10);
	g_game.score = (int)(herringpt + timept);
	if (g_game.score < 0) g_game.score = 0;

	return AddScore(g_game.course, TScore(g_game.player->name, g_game.score, g_game.herring, g_game.time));
}

// --------------------------------------------------------------------
//				score screen
// --------------------------------------------------------------------

static CCourseList *CourseList;
static TUpDown* course;
static TWidget* textbutton;
static TFramedText* courseName;
static TLabel* headline;

void CScore::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	KeyGUI(key, release);
	if (release) return;
	switch (key) {
		case sf::Keyboard::Escape:
			State::manager.RequestEnterState(*State::manager.PreviousState());
			break;
		case sf::Keyboard::Q:
			State::manager.RequestQuit();
			break;
		case sf::Keyboard::S:
			Score.SaveHighScore();
			break;
		case sf::Keyboard::L:
			Score.LoadHighScore();
			break;
		case sf::Keyboard::Return:
			State::manager.RequestEnterState(*State::manager.PreviousState());
			break;
	}
}

void CScore::Mouse(int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* clicked = ClickGUI(x, y);
		if (clicked == textbutton)
			State::manager.RequestEnterState(*State::manager.PreviousState());
	}
}

void CScore::Motion(int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow(cursor_pos);
}

static TArea area;
static int linedist, listtop;
static int dd1, dd2, dd3, dd4;

void CScore::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);
	Music.Play(param.menu_music, true);

	int framewidth = 550 * Winsys.scale;
	int frameheight = 50 * Winsys.scale;
	int frametop = AutoYPosN(32);
	area = AutoAreaN(30, 80, framewidth);
	FT.AutoSizeN(3);
	linedist = FT.AutoDistanceN(1);
	listtop = AutoYPosN(44);
	dd1 = 50 * Winsys.scale;
	dd2 = 115 * Winsys.scale;
	dd3 = 250 * Winsys.scale;
	dd4 = 375 * Winsys.scale;

	CourseList = Course.currentCourseList;

	ResetGUI();
	course = AddUpDown(area.right + 8, frametop, 0, (int)Course.currentCourseList->size()-1, 0);
	int siz = FT.AutoSizeN(5);
	textbutton = AddTextButton(Trans.Text(64), CENTER, AutoYPosN(80), siz);

	FT.AutoSizeN(7);
	headline = AddLabel(Trans.Text(62), CENTER, AutoYPosN(22), colWhite);

	FT.AutoSizeN(4);
	courseName = AddFramedText(area.left, frametop-2, framewidth, frameheight, 3, colMBackgr, "", FT.GetSize(), true);
}

const string ordinals[10] =
{"1:st", "2:nd", "3:rd", "4:th", "5:th", "6:th", "7:th", "8:th", "9:th", "10:th"};

void CScore::Loop(float timestep) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(timestep);
		draw_ui_snow();
	}

	DrawGUIBackground(Winsys.scale);

	courseName->Focussed(course->focussed());
	courseName->SetString((*CourseList)[course->GetValue()].name);

	const TScoreList *list = Score.GetScorelist(course->GetValue());

	FT.SetColor(colWhite);
	if (list != nullptr) {
		FT.AutoSizeN(3);
		if (list->numScores < 1) {
			FT.DrawString(CENTER, area.top + 140, Trans.Text(63));
		} else {
			for (int i=0; i<min(MAX_SCORES, list->numScores); i++) {
				int y = listtop + i*linedist;
				FT.DrawString(area.left, y, ordinals[i]);
				FT.DrawString(area.left + dd1, y, Int_StrN(list->scores[i].points));
				FT.DrawString(area.left + dd2, y, list->scores[i].player);
				FT.DrawString(area.left + dd3, y,
				              Int_StrN(list->scores[i].herrings) + "  herrings");
				FT.DrawString(area.left + dd4, y,
				              Float_StrN(list->scores[i].time, 1) + "  sec");
			}
		}
	} else Message("score list out of range");

	DrawGUI();

	Winsys.SwapBuffers();
}
