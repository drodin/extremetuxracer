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

#include "newplayer.h"
#include "particles.h"
#include "audio.h"
#include "gui.h"
#include "ogl.h"
#include "textures.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "regist.h"
#include "winsys.h"
#include <cctype>

CNewPlayer NewPlayer;

static TVector2 cursor_pos(0, 0);
static TUpDown* avatar;
static TWidget* textbuttons[2];
static TTextField* textfield;

void QuitAndAddPlayer () {
	if (textfield->Text().size () > 0)
		Players.AddPlayer (textfield->Text(), Players.GetDirectAvatarName(avatar->GetValue()));
	State::manager.RequestEnterState (Regist);
}

void CNewPlayer::Keyb_spec (SDL_keysym sym, bool release) {
	if (release) return;

	KeyGUI(sym.sym, sym.mod, release);
	switch (sym.sym) {
		case 27: State::manager.RequestEnterState (Regist); break;
		case 13:
			if (textbuttons[0]->focussed()) State::manager.RequestEnterState (Regist);
			else QuitAndAddPlayer ();
			break;
	}
}

void CNewPlayer::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* clicked = ClickGUI(x, y);

		if(clicked == textbuttons[0])
			State::manager.RequestEnterState (Regist);
		else if(clicked == textbuttons[1])
			QuitAndAddPlayer();
	}
}

void CNewPlayer::Motion (int x, int y) {
	MouseMoveGUI(x, y);
	y = param.y_resolution - y;

    TVector2 old_pos = cursor_pos;
    cursor_pos = TVector2(x, y);
    if (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

static int prevleft, prevtop, prevwidth, prevoffs;

void CNewPlayer::Enter() {
	Winsys.KeyRepeat (true);
	Winsys.ShowCursor (!param.ice_cursor);
	Music.Play (param.menu_music, -1);

	g_game.loopdelay = 10;
	int framewidth = 400 * param.scale;
	int frameheight = 50 * param.scale;
	int frametop = AutoYPosN (38);
	TArea area = AutoAreaN (30, 80, framewidth);
	prevleft = area.left;
	prevtop = AutoYPosN (52);
	prevwidth = 75 * param.scale;
	prevoffs = 80;

	ResetGUI();

	avatar = AddUpDown (area.left + prevwidth + prevoffs + 8, prevtop, 0, (int)Players.numAvatars() - 1, 0, prevwidth - 34);
	int siz = FT.AutoSizeN (5);
	textbuttons[0] = AddTextButton (Trans.Text(8), area.left+50, AutoYPosN (70), siz);
	double len = FT.GetTextWidth (Trans.Text(15));
	textbuttons[1] = AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (70), siz);

	textfield = AddTextField("", area.left, frametop, framewidth, frameheight);
}

void CNewPlayer::Loop(double timestep) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	TColor col;

	Music.Update ();
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();

	update_ui_snow (timestep);
	draw_ui_snow();

	textfield->UpdateCursor(timestep);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, col, 0.2);

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), param.scale);

	FT.SetColor (colWhite);
	FT.AutoSizeN (4);
	FT.DrawString (CENTER, AutoYPosN (30), Trans.Text(66));

	if (avatar->focussed()) col = colDYell; else col = colWhite;
	Players.GetAvatarTexture(avatar->GetValue())->DrawFrame(
		prevleft + prevoffs, prevtop, prevwidth, prevwidth, 2, col);

	DrawGUI();
    Winsys.SwapBuffers();
}
