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

static int scope = 0;
static TVector2 cursor_pos = {0, 0};

void EnterPractice () {
	g_game.GameType = PRACTICING;	
	Winsys.SetMode (RACE_SELECT);
}

void QuitGameType (int sc) {
	switch (sc) {
		case 0: Winsys.SetMode (EVENT_SELECT); break;
		case 1: EnterPractice (); break;
		case 2: Winsys.SetMode (GAME_CONFIG); break;
		case 3: Winsys.SetMode (CREDITS); break;
		case 4: Winsys.Quit (); break;
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
		case 274: if (scope < 4) scope++; break;
		case 273: if (scope > 0) scope--; break;
		case 13: QuitGameType (scope); break;
		case SDLK_TAB: scope++; if (scope > 4) scope = 0; break;
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
	scope = 1;
	
	ResetWidgets ();
	double top = (double)(param.y_resolution * 0.4);
	double dist = AutoDistance ();
	AddTextButton ("Enter an event", CENTER, top, 0, FIT);
	AddTextButton ("Practice", CENTER, top + dist, 1, FIT);
	AddTextButton ("Configuration", CENTER, top + dist * 2, 2, FIT);
	AddTextButton ("Credits", CENTER, top + dist * 3, 3, FIT);
	AddTextButton ("Quit", CENTER, top + dist * 4, 4, FIT);
	Music.Play ("start1", -1);
}

static void GameSelectLoop (double time_step) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;

	Music.Update ();    
			
	check_gl_error();
	Music.Update ();    
    set_gl_options (GUI);
    ClearRenderContext ();
    SetupGuiDisplay ();
    
	if (param.ui_snow) {
		update_ui_snow (time_step);
		draw_ui_snow();
    }

	Tex.Draw (T_TITLE, -1, 20, 0.8);
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
	    
	if (param.ice_cursor) DrawCursor ();
	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

static void GameSelectTerm (void) {}

void game_type_select_register () {
	Winsys.SetModeFuncs (GAME_TYPE_SELECT, GameSelectInit, GameSelectLoop, GameSelectTerm,
 		GameSelectKeys, GameSelectMouseFunc, GameSelectMotionFunc, NULL, NULL);
}

