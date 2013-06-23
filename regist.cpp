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

#include "regist.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "particles.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "game_type_select.h"
#include "newplayer.h"
#include "winsys.h"

CRegist Regist;

static TWidget* textbuttons[2];
static TUpDown* player;
static TUpDown* character;

void QuitRegistration () {
	Players.ResetControls ();
	Players.AllocControl (player->GetValue());
	g_game.player_id = player->GetValue();

	g_game.char_id = character->GetValue();
	State::manager.RequestEnterState (GameTypeSelect);
}

void CRegist::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	TWidget* focussed = KeyGUI(key, 0, release);
	if (release) return;
	switch (key) {
		case SDLK_ESCAPE: State::manager.RequestQuit(); break;
		case SDLK_RETURN:
			if (focussed == textbuttons[1]) {
				g_game.player_id = player->GetValue();
				State::manager.RequestEnterState (NewPlayer);
			} else QuitRegistration ();	break;
	}
}

void CRegist::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* focussed = ClickGUI(x, y);
		if(focussed == textbuttons[0])
			QuitRegistration ();
		else if(focussed == textbuttons[1]) {
			g_game.player_id = player->GetValue();
			State::manager.RequestEnterState (NewPlayer);
		}
	}
}

void CRegist::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

static int framewidth, frameheight, arrowwidth, sumwidth;
static TArea area;
static double scale, texsize;

void CRegist::Enter (void) {
	Winsys.ShowCursor (!param.ice_cursor);
	Music.Play (param.menu_music, -1);

	scale = Winsys.scale;
	framewidth = (int)(scale * 280);
	frameheight = (int)(scale * 50);
	arrowwidth = 50;
	sumwidth = framewidth * 2 + arrowwidth * 2;
	area = AutoAreaN (30, 80, sumwidth);
	texsize = 128 * scale;

	ResetGUI ();
	player = AddUpDown(area.left + framewidth + 8, area.top, 0, (int)Players.numPlayers() - 1, (int)g_game.start_player);
	character = AddUpDown(area.left + framewidth * 2 + arrowwidth + 8, area.top, 0, (int)Char.CharList.size() - 1, 0);
	int siz = FT.AutoSizeN (5);
	textbuttons[0] = AddTextButton (Trans.Text(60), CENTER, AutoYPosN (62), siz);
	textbuttons[1] = AddTextButton (Trans.Text(61), CENTER, AutoYPosN (70), siz);

	g_game.loopdelay = 10;
	if(Char.CharList.empty())
		Winsys.Terminate(); // Characters are necessary - ETR is unusable otherwise
}

void CRegist::Loop (double timestep) {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;
	Music.Update ();
	check_gl_error();
    ClearRenderContext ();
    ScopedRenderMode rm(GUI);
    SetupGuiDisplay ();
	TColor col;

	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow();
	}

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), scale);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, col, 0.2);

	FT.AutoSizeN (3);
	FT.SetColor (colWhite);
	int top = AutoYPosN (24);
	FT.DrawString (area.left, top, Trans.Text(58));
	FT.DrawString (area.left + framewidth + arrowwidth, top, Trans.Text(59));

	FT.AutoSizeN (4);
	if (player->focussed()) col = colDYell; else col = colWhite;
	DrawFrameX (area.left, area.top, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString (area.left + 20, area.top, Players.GetName (player->GetValue()));
	Players.GetAvatarTexture(player->GetValue())->DrawFrame(
		area.left + 60, AutoYPosN (40), texsize, texsize, 3, colWhite);

	if (character->focussed()) col = colDYell; else col = colWhite;
	DrawFrameX (area.left + framewidth + arrowwidth, area.top,
		framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString (area.left + framewidth + arrowwidth + 20,
		area.top, Char.CharList[character->GetValue()].name);
	if(Char.CharList[character->GetValue()].preview != NULL)
		Char.CharList[character->GetValue()].preview->DrawFrame(
			area.right - texsize - 60 - arrowwidth,
			AutoYPosN (40), texsize, texsize, 3, colWhite);


	FT.SetColor (colWhite);
	DrawGUI();

    Winsys.SwapBuffers();
}
