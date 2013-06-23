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

void EnterPractice () {
	g_game.game_type = PRACTICING;
	State::manager.RequestEnterState (RaceSelect);
}

void QuitGameType () {
	if(textbuttons[0]->focussed())
		State::manager.RequestEnterState (EventSelect);
	if(textbuttons[1]->focussed())
		EnterPractice ();
	if(textbuttons[2]->focussed())
		State::manager.RequestEnterState (GameConfig);
	if(textbuttons[3]->focussed())
		State::manager.RequestEnterState (Score);
	if(textbuttons[4]->focussed())
		State::manager.RequestEnterState (Help);
	if(textbuttons[5]->focussed())
		State::manager.RequestEnterState (Credits);
	if(textbuttons[6]->focussed())
		State::manager.RequestQuit();
}

void CGameTypeSelect::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		ClickGUI(x, y);
		QuitGameType();
	}
}

void CGameTypeSelect::Keyb (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;

	KeyGUI(key, 0, release);
	switch (key) {
		case SDLK_u: param.ui_snow = !param.ui_snow; break;
		case SDLK_ESCAPE: State::manager.RequestQuit(); break;
		case SDLK_DOWN: IncreaseFocus(); break;
		case SDLK_UP: DecreaseFocus(); break;
		case SDLK_RETURN: QuitGameType(); break;
		case SDLK_w: Music.FreeMusics(); break;
	}
}

void CGameTypeSelect::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

// ====================================================================

void CGameTypeSelect::Enter () {
	Winsys.ShowCursor (!param.ice_cursor);

	ResetGUI ();
	int top = AutoYPosN (40);
	int siz = FT.AutoSizeN (6);
	int dist = FT.AutoDistanceN (2);
	textbuttons[0] = AddTextButton (Trans.Text(1), CENTER, top, siz);
	textbuttons[1] = AddTextButton (Trans.Text(2), CENTER, top + dist, siz);
	textbuttons[2] = AddTextButton (Trans.Text(3), CENTER, top + dist * 2, siz);
	textbuttons[3] = AddTextButton (Trans.Text(62), CENTER, top + dist * 3, siz);
	textbuttons[4] = AddTextButton (Trans.Text(43), CENTER, top + dist * 4, siz);
	textbuttons[5] = AddTextButton (Trans.Text(4), CENTER, top + dist * 5, siz);
	textbuttons[6] = AddTextButton (Trans.Text(5), CENTER, top + dist * 6, siz);

	Music.Play (param.menu_music, -1);
	g_game.loopdelay = 10;
}

void CGameTypeSelect::Loop (double time_step) {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;

	check_gl_error();
	Music.Update ();
    ScopedRenderMode rm(GUI);
    ClearRenderContext ();
    SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow (time_step);
		draw_ui_snow();
    }

	Tex.Draw (T_TITLE, CENTER, AutoYPosN (5), Winsys.scale);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	DrawGUI();

	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}
