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

#include "regist.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "course.h"
#include "tux.h"
#include "env.h"
#include "particles.h"
#include "credits.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"

static TVector2 cursor_pos = {0, 0};
static int curr_focus = 0;
static TCharacter *CharList;
static int curr_character = 0;
static int last_character;
static int xleft, ytop;
static int curr_player = 0;
static int last_player;

void QuitRegistration () {
	Players.ResetControls ();
	Players.AllocControl (curr_player);
	g_game.player_id = curr_player;

	g_game.char_id = curr_character;
	Winsys.SetMode (GAME_TYPE_SELECT);
}

void ChangeRegistSelection (int focus, int dir) {
	if (dir == 0) {
		switch (focus) {
			case 0:	if (curr_player > 0) curr_player--; break;
			case 1:	if (curr_character > 0) curr_character--; break;
		}
	} else {
		switch (focus) {
			case 0:	if (curr_player < last_player) curr_player++; break;
			case 1:	if (curr_character < last_character) curr_character++; break;
		}
	}
}
				  
void RegistKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case 27: Winsys.Quit (); break;
		case 13: 
			if (curr_focus == 3) {
				Winsys.SetMode (NEWPLAYER);
			} else QuitRegistration ();	break;
		case SDLK_TAB: 
			curr_focus++; 
			if (curr_focus > 3) curr_focus = 0; 
			break;
		case SDLK_DOWN: ChangeRegistSelection (curr_focus, 1); break;
		case SDLK_UP: ChangeRegistSelection (curr_focus, 0); break;
	}
}

void RegistMouseFunc (int button, int state, int x, int y) {
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		switch (foc) {
			case 0: ChangeRegistSelection (foc, dir); break;
			case 1: ChangeRegistSelection (foc, dir); break;
			case 2: QuitRegistration (); break;
			case 3: Winsys.SetMode (NEWPLAYER); break;
		}
	}
}

void RegistMotionFunc (int x, int y ){
    TVector2 old_pos;
 	int sc, dir;
	if (Winsys.ModePending ()) return; 

	GetFocus (x, y, &sc, &dir);
	if (sc >= 0) curr_focus = sc;
	y = param.y_resolution - y;

    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

void RegistInit (void) {  
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);

	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (230);
	ResetWidgets ();
	AddArrow (xleft + 210, ytop, 0, 0);
	AddArrow (xleft + 210, ytop+18, 1, 0);
	AddArrow (xleft + 470, ytop, 0, 1);
	AddArrow (xleft + 470, ytop+18, 1, 1);
	AddTextButton ("Enter", -1, ytop + 240, 2, -1);
	AddTextButton ("Register a new player", -1, ytop + 290, 3, -1);

	curr_focus = 0;
	g_game.loopdelay = 10;
	CharList = Char.CharList;
	last_character = Char.numCharacters - 1;
	last_player = Players.numPlayers - 1;
	if (g_game.prev_mode == NEWPLAYER) curr_player = last_player; 
		else curr_player = g_game.start_player;
}

void RegistLoop (double timestep ){
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();
	TColor col;
		
	update_ui_snow (timestep);
	draw_ui_snow();

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);

	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
	FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop-40, "Select your player name:");
	FT.DrawString (xleft+260, ytop-40, "Select a character:");

	// player selection
	if (curr_focus == 0) col = colDYell; else col = colWhite;
	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	DrawFrameX (xleft, ytop-4, 200, 44, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString (xleft+20, ytop, Players.GetName (curr_player));
	Tex.DrawDirectFrame (Players.GetAvatarID (curr_player), 
		xleft + 30, ytop + 65, 128, 128, 3, colWhite);

	// char selection
	if (curr_focus == 1) col = colDYell; else col = colWhite;
	DrawFrameX (xleft+260, ytop-4, 200, 44, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString (xleft+280, ytop, CharList[curr_character].name);
	Tex.DrawDirectFrame (CharList[curr_character].preview, xleft + 300, ytop + 65, 
		128, 128, 3, colWhite);

	FT.SetColor (colWhite);
	PrintArrow (0, (curr_player > 0));	
 	PrintArrow (1, (curr_player < last_player));
	PrintArrow (2, (curr_character > 0));	
	PrintArrow (3, (curr_character < last_character));
	PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	if (param.ice_cursor) DrawCursor ();
    Winsys.SwapBuffers();
} 

void RegistTerm () {
}

void regist_register() {
	Winsys.SetModeFuncs (REGIST,  RegistInit,  RegistLoop,  RegistTerm,
 		 RegistKeys,  RegistMouseFunc,  RegistMotionFunc, NULL, NULL, NULL);
}
