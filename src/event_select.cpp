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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "event_select.h"
#include "gui.h"
#include "font.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "game_ctrl.h"
#include "translation.h"
#include "event.h"
#include "game_type_select.h"
#include "winsys.h"

CEventSelect EventSelect;

static TUpDown* event;
static TUpDown* cup;
static TWidget* textbuttons[2];
static TLabel* selectEvent;
static TLabel* selectCup;
static TFramedText* selectedEvent;
static TFramedText* selectedCup;
static TLabel* cupLocked;

void EnterEvent() {
	g_game.game_type = CUPRACING;
	g_game.cup = Events.EventList[event->GetValue()].cups[cup->GetValue()];
	State::manager.RequestEnterState(Event);
}

void CEventSelect::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case sf::Keyboard::Escape:
			State::manager.RequestEnterState(GameTypeSelect);
			break;
		case sf::Keyboard::Q:
			State::manager.RequestQuit();
			break;
		case sf::Keyboard::Return:
			if (textbuttons[1]->focussed()) State::manager.RequestEnterState(GameTypeSelect);
			else if (Events.IsUnlocked(event->GetValue(), cup->GetValue())) EnterEvent();
			break;
		case sf::Keyboard::U:
			param.ui_snow = !param.ui_snow;
			break;
		default:
			KeyGUI(key, release);
	}
}

void CEventSelect::Mouse(int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* clicked = ClickGUI(x, y);
		if (textbuttons[0] == clicked) {
			if (Events.IsUnlocked(event->GetValue(), cup->GetValue()))
				EnterEvent();
		} else if (textbuttons[1] == clicked)
			State::manager.RequestEnterState(GameTypeSelect);
	}
}

void CEventSelect::Motion(int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow(cursor_pos);
}

void CEventSelect::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);

	int framewidth = 500 * Winsys.scale;
	int frameheight = 50 * Winsys.scale;
	TArea area = AutoAreaN(30, 80, framewidth);
	int frametop1 = AutoYPosN(35);
	int frametop2 = AutoYPosN(50);

	ResetGUI();
	event = AddUpDown(area.right+8, frametop1, 0, (int)Events.EventList.size() - 1, 0);
	cup = AddUpDown(area.right + 8, frametop2, 0, (int)Events.EventList[0].cups.size() - 1, 0);

	int siz = FT.AutoSizeN(5);

	float len = FT.GetTextWidth(Trans.Text(9));
	textbuttons[0] = AddTextButton(Trans.Text(9), area.right-len-50, AutoYPosN(70), siz);
	textbuttons[1] = AddTextButton(Trans.Text(8), area.left+50, AutoYPosN(70), siz);
	SetFocus(textbuttons[0]);

	FT.AutoSizeN(3);
	selectEvent = AddLabel(Trans.Text(6), area.left, AutoYPosN(30), colWhite);
	selectCup = AddLabel(Trans.Text(7), area.left, AutoYPosN(45), colWhite);
	cupLocked = AddLabel(Trans.Text(10), CENTER, AutoYPosN(58), colLGrey);

	FT.AutoSizeN(4);
	selectedEvent = AddFramedText(area.left, frametop1, framewidth, frameheight, 3, colMBackgr, "", FT.GetSize(), true);
	selectedCup = AddFramedText(area.left, frametop2, framewidth, frameheight, 3, colMBackgr, "", FT.GetSize(), true);

	Events.MakeUnlockList(g_game.player->funlocked);
	Music.Play(param.menu_music, true);
}

void CEventSelect::Loop(float timestep) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(timestep);
		draw_ui_snow();
	}

	DrawGUIBackground(Winsys.scale);

	cupLocked->SetVisible(Events.IsUnlocked(event->GetValue(), cup->GetValue()) == false);

	selectedEvent->Focussed(event->focussed());
	selectedEvent->SetString(Events.EventList[event->GetValue()].name);

	selectedCup->SetActive(Events.IsUnlocked(event->GetValue(), cup->GetValue()));
	selectedCup->Focussed(cup->focussed());
	selectedCup->SetString(Events.GetCupTrivialName(event->GetValue(), cup->GetValue()));

	textbuttons[0]->SetActive(Events.IsUnlocked(event->GetValue(), cup->GetValue()));
	DrawGUI();

	Winsys.SwapBuffers();
}
