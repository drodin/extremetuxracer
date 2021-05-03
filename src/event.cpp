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

#include "event.h"
#include "ogl.h"
#include "audio.h"
#include "particles.h"
#include "textures.h"
#include "gui.h"
#include "course.h"
#include "spx.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "event_select.h"
#include "game_over.h"
#include "game_config.h"
#include "loading.h"
#include "winsys.h"

CEvent Event;

// ready: 0 - racing  1 - ready with success  2 - ready with failure
static int ready = 0; // indicates if last race is done
static TCup *ecup = 0;
static std::size_t curr_race = 0;
static std::size_t curr_bonus = 0;
static TWidget* textbuttons[3];
static TLabel* headline;
static TLabel* info1;
static TLabel* info2;

void StartRace() {
	if (ready > 0) {
		State::manager.RequestEnterState(EventSelect);
		return;
	}
	g_game.mirrorred = false;
	g_game.course = ecup->races[curr_race]->course;
	g_game.theme_id = ecup->races[curr_race]->music_theme;
	g_game.light_id = ecup->races[curr_race]->light;
	g_game.snow_id = ecup->races[curr_race]->snow;
	g_game.wind_id = ecup->races[curr_race]->wind;
	g_game.race = ecup->races[curr_race];
	g_game.game_type = CUPRACING;
	State::manager.RequestEnterState(Loading);
}

void CEvent::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case sf::Keyboard::Return:
			if (textbuttons[0]->focussed() && ready < 1) StartRace();
			else State::manager.RequestEnterState(EventSelect);
			break;
		case sf::Keyboard::Escape:
			State::manager.RequestEnterState(EventSelect);
			break;
		case sf::Keyboard::U:
			param.ui_snow = !param.ui_snow;
			break;
		default:
			KeyGUI(key, release);
	}
}

void CEvent::Mouse(int button, int state, int x, int y) {
	if (state != 1) return;

	TWidget* clicked = ClickGUI(x, y);
	if (clicked == textbuttons[0]) {
		if (ready < 1)
			StartRace();
	} else if (clicked == textbuttons[1] || clicked == textbuttons[2])
		State::manager.RequestEnterState(EventSelect);
}

void CEvent::Motion(int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow(cursor_pos);
}

void InitCupRacing() {
	ecup = g_game.cup;
	curr_race = 0;
	curr_bonus = ecup->races.size();
	ready = 0;
}

void UpdateCupRacing() {
	std::size_t lastrace = ecup->races.size() - 1;
	curr_bonus += g_game.race_result;
	if (g_game.race_result >= 0) {
		if (curr_race < lastrace) curr_race++;
		else ready = 1;
	} else {
		if (curr_bonus == 0) ready = 2;
	}
	if (ready == 1) {
		Players.AddPassedCup(ecup->cup);
		Players.SavePlayers();
	}
}

// --------------------------------------------------------------------

static TArea area;
static int messtop, messtop2;
static int bonustop, framewidth, frametop;
static int dist, texsize;

void CEvent::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);

	if (State::manager.PreviousState() == &GameOver) UpdateCupRacing();
	else InitCupRacing();

	framewidth = 500*Winsys.scale;
	frametop = AutoYPosN(45);
	area = AutoAreaN(30, 80, framewidth);
	messtop = AutoYPosN(50);
	messtop2 = AutoYPosN(60);
	bonustop = AutoYPosN(35);
	texsize = 32 * Winsys.scale / 0.8f;
	dist = texsize + 2 * 4;
	int framebottom = frametop + (int) ecup->races.size() * dist + 10;

	ResetGUI();
	unsigned int siz = FT.AutoSizeN(5);
	textbuttons[1] = AddTextButton(Trans.Text(8), area.left + 100, AutoYPosN(80), siz);
	int len = FT.GetTextWidth(Trans.Text(13));
	textbuttons[0] = AddTextButton(Trans.Text(13), area.right -len - 100, AutoYPosN(80), siz);
	textbuttons[2] = AddTextButton(Trans.Text(15), CENTER, AutoYPosN(80), siz);
	SetFocus((ready >= 1) ? textbuttons[2] : textbuttons[0]);

	FT.AutoSizeN(6);
	headline = AddLabel(ecup->name, CENTER, AutoYPosN(25), colWhite);

	FT.AutoSizeN(3);
	int ddd = FT.AutoDistanceN(1);
	sf::String info = Trans.Text(11);
	info += "   " + Int_StrN(ecup->races[curr_race]->herrings.x);
	info += "   " + Int_StrN(ecup->races[curr_race]->herrings.y);
	info += "   " + Int_StrN(ecup->races[curr_race]->herrings.z);
	info1 = AddLabel(info, CENTER, framebottom + 15, colDBlue);

	info = Trans.Text(12);
	info += "   " + Int_StrN((int)ecup->races[curr_race]->time.x);
	info += "   " + Int_StrN((int)ecup->races[curr_race]->time.y);
	info += "   " + Int_StrN((int)ecup->races[curr_race]->time.z);
	info += "  " + Trans.Text(14);
	info2 = AddLabel(info, CENTER, framebottom + 15 + ddd, colDBlue);

	headline->SetVisible(ready == 0);
	info1->SetVisible(ready == 0);
	info2->SetVisible(ready == 0);

	Music.Play(param.menu_music, true);
}

int resultlevel(std::size_t num, std::size_t numraces) {
	if (num < 1) return 0;
	int q = (int)((num - 0.01) / numraces);
	return q + 1;
}

void CEvent::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}
	DrawGUIBackground(Winsys.scale);

	if (ready == 0) {			// cup not finished
		DrawBonusExt(bonustop, (int)ecup->races.size(), curr_bonus);

		DrawFrameX(area.left, frametop, framewidth,
		           (int)ecup->races.size() * dist + 20, 3, colBackgr, colWhite, 1);

		TCheckbox checkbox(area.right - 50, frametop, texsize, "");
		for (std::size_t i=0; i<ecup->races.size(); i++) {
			FT.AutoSizeN(4);

			int y = frametop + 10 + (int)i * dist;
			if (i == curr_race)
				FT.SetColor(colDYell);
			else
				FT.SetColor(colWhite);
			FT.DrawString(area.left + 29, y, ecup->races[i]->course->name);
			checkbox.SetPosition(area.right - 50*Winsys.scale/0.8f, y + 4);
			checkbox.SetChecked(curr_race > i);
			checkbox.Draw();
		}
	} else if (ready == 1) {		// cup successfully finished
		FT.AutoSizeN(5);
		FT.SetColor(colWhite);
		FT.DrawString(CENTER, messtop, Trans.Text(16));
		DrawBonusExt(bonustop, (int)ecup->races.size(), curr_bonus);
		int res = resultlevel(curr_bonus, ecup->races.size());
		FT.DrawString(CENTER, messtop2, Trans.Text(17) + " " + Int_StrN(res));
	} else if (ready == 2) {		// cup finished but failed
		FT.AutoSizeN(5);
		FT.SetColor(colLRed);
		FT.DrawString(CENTER, messtop, Trans.Text(18));
		DrawBonusExt(bonustop, ecup->races.size(), curr_bonus);
		FT.DrawString(CENTER, messtop2, Trans.Text(19));
	}

	textbuttons[0]->SetVisible(ready < 1);
	textbuttons[1]->SetVisible(ready < 1);
	textbuttons[2]->SetVisible(!(ready < 1));
	textbuttons[0]->SetActive(ready < 1);
	textbuttons[1]->SetActive(ready < 1);
	textbuttons[2]->SetActive(!(ready < 1));

	DrawGUI();
	Winsys.SwapBuffers();
}
