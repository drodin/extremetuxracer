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

#include "game_type_select.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "gui.h"
#include "particles.h"
#include "font.h"
#include "credits.h"
#include "translation.h"
#include "event_select.h"
#include "race_select.h"
#include "config_screen.h"
#include "help.h"
#include "score.h"
#include "winsys.h"

CGameTypeSelect GameTypeSelect;

static TTextButton* textbuttons[7];
static sf::Sprite logo;

void EnterPractice() {
	g_game.game_type = PRACTICING;
	State::manager.RequestEnterState(RaceSelect);
}

void QuitGameType() {
	if (textbuttons[0]->focussed())
		State::manager.RequestEnterState(EventSelect);
	if (textbuttons[1]->focussed())
		EnterPractice();
	if (textbuttons[2]->focussed())
		State::manager.RequestEnterState(GameConfig);
	if (textbuttons[3]->focussed())
		State::manager.RequestEnterState(Score);
	if (textbuttons[4]->focussed())
		State::manager.RequestEnterState(Help);
	if (textbuttons[5]->focussed())
		State::manager.RequestEnterState(Credits);
	if (textbuttons[6]->focussed())
		State::manager.RequestQuit();
}

void CGameTypeSelect::Mouse(int button, int state, int x, int y) {
	if (state == 1) {
		ClickGUI(x, y);
		QuitGameType();
	}
}

void CGameTypeSelect::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;

	switch (key) {
		case sf::Keyboard::U:
			param.ui_snow = !param.ui_snow;
			break;
		case sf::Keyboard::Escape:
			State::manager.RequestQuit();
			break;
		case sf::Keyboard::Return:
			QuitGameType();
			break;
		case sf::Keyboard::W:
			Music.FreeMusics();
			break;
		default:
			KeyGUI(key, release);
			break;
	}
}

void CGameTypeSelect::Motion(int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow(cursor_pos);
}

// ====================================================================

void CGameTypeSelect::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);

	ResetGUI();
	int top = AutoYPosN(40);
	unsigned int siz = FT.AutoSizeN(6);
	int dist = FT.AutoDistanceN(2);
	textbuttons[0] = AddTextButton(Trans.Text(1), CENTER, top, siz);
	textbuttons[1] = AddTextButton(Trans.Text(2), CENTER, top + dist, siz);
	textbuttons[2] = AddTextButton(Trans.Text(3), CENTER, top + dist * 2, siz);
	textbuttons[3] = AddTextButton(Trans.Text(62), CENTER, top + dist * 3, siz);
	textbuttons[4] = AddTextButton(Trans.Text(43), CENTER, top + dist * 4, siz);
	textbuttons[5] = AddTextButton(Trans.Text(4), CENTER, top + dist * 5, siz);
	textbuttons[6] = AddTextButton(Trans.Text(5), CENTER, top + dist * 6, siz);
	logo.setTexture(Tex.GetSFTexture(T_TITLE));
	logo.setScale(Winsys.scale, Winsys.scale);
	logo.setPosition((Winsys.resolution.width - logo.getTextureRect().width) / 2, (5));

	Music.Play(param.menu_music, true);
}

void CGameTypeSelect::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}

	Winsys.draw(logo);
	DrawGUIFrame();
	DrawGUI();

	Winsys.SwapBuffers();
}
