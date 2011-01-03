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

#include "game_type_select.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "gui.h"
#include "particles.h"
#include "font.h"
#include "translation.h"

static int scope = 0;
static TVector2 cursor_pos = {0, 0};

void EnterPractice () {
	g_game.game_type = PRACTICING;	
	Winsys.SetMode (RACE_SELECT);
}

void QuitGameType (int sc) {
	switch (sc) {
		case 0: Winsys.SetMode (EVENT_SELECT); break;
		case 1: EnterPractice (); break;
		case 2: Winsys.SetMode (GAME_CONFIG); break;
		case 5: Winsys.SetMode (CREDITS); break;
		case 4: Winsys.SetMode (HELP); break;
		case 3: Winsys.SetMode (SCORE); break;
		case 6: Winsys.Quit (); break;
	};
}

static void GameSelectMouseFunc (int button, int state, int x, int y) {
	int focus, dr;
	if (state == 1) {
		GetFocus (x, y, &focus, &dr);
		QuitGameType (focus);
	}
}

void GameSelectKeys (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;

	switch (key) {
		case SDLK_u: param.ui_snow = !param.ui_snow; break;	
		case 27: Winsys.Quit (); break;
		case 274: if (scope < 6) scope++; break;
		case 273: if (scope > 0) scope--; break;
		case 13: QuitGameType (scope); break;
		case SDLK_TAB: scope++; if (scope > 6) scope = 0; break;
		case SDLK_w: Music.FreeMusics (); break;
	}	
}

void GameSelectMotionFunc (int x, int y) {
    TVector2 old_pos;
 	int sc, dir;
    
	if (Winsys.ModePending()) return;
	GetFocus (x, y, &sc, &dir);
	if (sc >= 0) scope = sc;
	y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);

    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

// ====================================================================

static void GameSelectInit (void) {
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	scope = 0;
	
	ResetWidgets ();
	int top = AutoYPosN (40);
	int siz = FT.AutoSizeN (6);
	int dist = FT.AutoDistanceN (2);
	AddTextButton (Trans.Text(1), CENTER, top, 0, siz);
	AddTextButton (Trans.Text(2), CENTER, top + dist, 1, siz);
	AddTextButton (Trans.Text(3), CENTER, top + dist * 2, 2, siz);
	AddTextButton (Trans.Text(4), CENTER, top + dist * 5, 5, siz);
	AddTextButton (Trans.Text(43), CENTER, top + dist * 4, 4, siz);
	AddTextButton ("Highscore list", CENTER, top + dist * 3, 3, siz);
	AddTextButton (Trans.Text(5), CENTER, top + dist * 6, 6, siz);

	Music.Play (param.menu_music, -1);
	g_game.loopdelay = 10;
}

static void GameSelectLoop (double time_step) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;

	check_gl_error();
	Music.Update ();    
    set_gl_options (GUI);
    ClearRenderContext ();
    SetupGuiDisplay ();
    
	if (param.ui_snow) {
		update_ui_snow (time_step);
		draw_ui_snow();
    }

	Tex.Draw (T_TITLE, CENTER, AutoYPosN (5), param.scale);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	PrintTextButton (0, scope);
	PrintTextButton (1, scope);
	PrintTextButton (2, scope);
	PrintTextButton (3, scope);
	PrintTextButton (4, scope);
	PrintTextButton (5, scope);
	PrintTextButton (6, scope);

	if (param.ice_cursor) DrawCursor ();
	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

static void GameSelectTerm (void) {
}

void game_type_select_register () {
	Winsys.SetModeFuncs (GAME_TYPE_SELECT, GameSelectInit, GameSelectLoop, GameSelectTerm,
 		GameSelectKeys, GameSelectMouseFunc, GameSelectMotionFunc, NULL, NULL, NULL);
}

