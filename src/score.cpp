/* --------------------------------------------------------------------
EXTREME TUXRACER

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

int CScore::AddScore(const std::string& group, const std::string& course, TScore&& score) {
	if (score.points < 1) return 999;

	TScoreList *list = &Scorelist[group][course];
	int num = list->numScores;
	int pos = 0;
	int lastpos = num-1;
	int val = score.points;

	if (num == 0) {
		list->scores[0] = score;
		list->numScores++;
	} else if (num == MAX_SCORES) {
		while (pos < num && val <= list->scores[pos].points) pos++;
		if (pos == lastpos) {
			list->scores[pos] = score;
		} else if (pos < lastpos) {
			for (int i=lastpos; i>pos; i--) list->scores[i] = list->scores[i-1];
			list->scores[pos] = score;
		}
	} else {
		while (pos < num && val <= list->scores[pos].points) pos++;
		for (int i=num; i>pos; i--) list->scores[i] = list->scores[i-1];
		list->scores[pos] = score;
		list->numScores++;
	}
	return pos;
}

// for testing:
void CScore::PrintScorelist(const std::string& group, const std::string& course) const {
	const TScoreList *list = &Scorelist.at(group).at(course);

	if (list->numScores < 1) {
		PrintStr("no entries in this score list");
	} else {
		for (int i=0; i<list->numScores; i++) {
			std::string line = "player: " + list->scores[i].player;
			line += " points: " + Int_StrN(list->scores[i].points);
			line += " herrings: " + Int_StrN(list->scores[i].herrings);
			line += " time: " + Float_StrN(list->scores[i].time, 2);
			PrintString(line);
		}
	}
}

const TScoreList* CScore::GetScorelist(const std::string& group, const std::string& course) const {
	try {
		return &Scorelist.at(group).at(course);
	} catch (...) {
		return nullptr;
	}
}

bool CScore::SaveHighScore() const {
	CSPList splist;

	for (std::unordered_map<std::string, std::unordered_map<std::string, TScoreList>>::const_iterator i = Scorelist.cbegin(); i != Scorelist.cend(); ++i) {
		for (std::unordered_map<std::string, TScoreList>::const_iterator j = i->second.cbegin(); j != i->second.cend(); ++j) {
			const TScoreList *list = &j->second;

			int num = list->numScores;
			if (num > 0) {
				for (int sc = 0; sc<num; sc++) {
					const TScore& score = list->scores[sc];
					std::string line = "*[group] " + i->first;
					line += " [course] " + j->first;
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
	CSPList list;

	if (!list.Load(param.config_dir, "highscore")) {
		Message("could not load highscore list");
		return false;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		std::string group = SPStrN(*line, "group", "default");
		std::string course = SPStrN(*line, "course", "unknown");
		try {
			AddScore(group, course, TScore(
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

	return AddScore(Course.currentCourseList->name, g_game.course->dir, TScore(g_game.player->name, g_game.score, g_game.herring, g_game.time));
}

// --------------------------------------------------------------------
//				score screen
// --------------------------------------------------------------------

static CCourseList *CourseList;
static int prevGroup = 0;
static TUpDown* course;
static TUpDown* courseGroup;
static TWidget* textbutton;
static TFramedText* courseName;
static TFramedText* courseGroupName;
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
		default:
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
	int frametop = AutoYPosN(28);
	area = AutoAreaN(30, 80, framewidth);
	FT.AutoSizeN(3);
	linedist = FT.AutoDistanceN(1);
	listtop = AutoYPosN(46);
	dd1 = 50 * Winsys.scale;
	dd2 = 115 * Winsys.scale;
	dd3 = 250 * Winsys.scale;
	dd4 = 375 * Winsys.scale;

	CourseList = &Course.CourseLists["default"];

	ResetGUI();
	courseGroup = AddUpDown(area.right + 8, frametop, 0, (int)Course.CourseLists.size() - 1, 0);
	course = AddUpDown(area.right + 8, frametop + frameheight + 20, 0, (int)CourseList->size() - 1, 0);
	int siz = FT.AutoSizeN(5);
	textbutton = AddTextButton(Trans.Text(64), CENTER, AutoYPosN(85), siz);

	FT.AutoSizeN(7);
	headline = AddLabel(Trans.Text(62), CENTER, AutoYPosN(18), colWhite);

	FT.AutoSizeN(4);
	courseGroupName = AddFramedText(area.left, frametop - 2, framewidth, frameheight, 3, colMBackgr, "default", FT.GetSize(), true);
	courseName = AddFramedText(area.left, frametop - 2 + frameheight + 20, framewidth, frameheight, 3, colMBackgr, "", FT.GetSize(), true);
}

void CScore::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}

	DrawGUIBackground(Winsys.scale);

	if (courseGroup->GetValue() != prevGroup) {
		prevGroup = courseGroup->GetValue();
		CourseList = Course.getGroup((std::size_t)courseGroup->GetValue());
		course->SetValue(0);
		course->SetMaximum((int)CourseList->size() - 1);
		courseGroupName->SetString(CourseList->name);
	}

	courseGroupName->Focussed(courseGroup->focussed());

	courseName->Focussed(course->focussed());
	courseName->SetString((*CourseList)[course->GetValue()].name);

	const TScoreList *list = Score.GetScorelist(CourseList->name, (*CourseList)[course->GetValue()].dir);

	FT.SetColor(colWhite);
	FT.AutoSizeN(3);
	if (list != nullptr && list->numScores > 0) {
		for (int i=0; i<std::min(MAX_SCORES, list->numScores); i++) {
			int y = listtop + i*linedist;
			FT.DrawString(area.left, y, Trans.Text(99+i));
			FT.DrawString(area.left + dd1, y, Int_StrN(list->scores[i].points));
			FT.DrawString(area.left + dd2, y, list->scores[i].player);
			FT.DrawString(area.left + dd3, y,
			              Int_StrN(list->scores[i].herrings) + "  " + Trans.Text(97));
			FT.DrawString(area.left + dd4, y,
			              Float_StrN(list->scores[i].time, 1) + "  " + Trans.Text(98));
		}
	} else
		FT.DrawString(CENTER, area.top + 140, Trans.Text(63));

	DrawGUI();

	Winsys.SwapBuffers();
}
