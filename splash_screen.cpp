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

#include "splash_screen.h"
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
static int pass = 0;

static int curr_focus = 0;
static TCharacter *CharList;
static int curr_character = 0;
static int last_character;
static int xleft, ytop;

void ChangeCharSelection (int focus, int dir) {
	if (dir == 0) {
		switch (focus) {
			case 0:	if (curr_character > 0) curr_character--; break;
		}
	} else {
		switch (focus) {
			case 0:	if (curr_character < last_character) curr_character++; break;
		}
	}
}
				  
void SplashKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case 27: Winsys.Quit (); break;
		case 13: 
			g_game.char_id = curr_character; 
			Winsys.SetMode (GAME_TYPE_SELECT); 
			break;
		case SDLK_TAB: if (curr_focus < 1) curr_focus++; else curr_focus = 0; break;
		case SDLK_DOWN: ChangeCharSelection (curr_focus, 1); break;
		case SDLK_UP: ChangeCharSelection (curr_focus, 0); break;
	}
}

void SplashMouseFunc (int button, int state, int x, int y) {
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		switch (foc) {
			case 0: ChangeCharSelection (foc, dir); break;
			case 1: 
				g_game.char_id = curr_character; 
				Winsys.SetMode (GAME_TYPE_SELECT); 
				break;
			case 8: Winsys.SetMode (GAME_TYPE_SELECT); break;
		}
	}
}

void SplashMotionFunc (int x, int y ){
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


void SplashInit (void) {  
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);
	pass = 0;


	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (230);
	ResetWidgets ();
	AddArrow (xleft + 470, ytop, 0, 0);
	AddArrow (xleft + 470, ytop+18, 1, 0);
	AddTextButton ("Enter", -1, ytop + 280, 1, -1);

	curr_focus = 0;
	g_game.loopdelay = 20;
}

void SplashLoop (double timestep ){
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	int top;
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();
	TColor col;
		
	if (param.ui_snow && pass > 0) {
		update_ui_snow (timestep);
		draw_ui_snow();
    }

	if (pass == 0) {
		Tex.Draw (TEXLOGO, -1, 60, 1);
		FT.SetColor (colDYell);
		FT.SetSize (AutoFtSize ());
		top = AutoYPos (350);
		FT.DrawText (CENTER, top, "loading resources");
		FT.DrawText (CENTER, top + AutoDistance(), "please wait ...");
	} else {
		Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
		Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
		Tex.Draw (TOP_LEFT, 0, 0, 1);
		Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
		Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);


		if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
		FT.SetColor (colWhite);
		FT.DrawString (xleft, ytop-40, "Select your player name:");
		FT.DrawString (xleft+260, ytop-40, "Select a character:");

		// char selection
		if (curr_focus == 0) col = colDYell; else col = colWhite;
		DrawFrameX (xleft+260, ytop-4, 200, 44, 3, colMBackgr, col, 1.0);
		if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
		FT.SetColor (colDYell);
		FT.DrawString (xleft+280, ytop, CharList[curr_character].name);
		Tex.DrawDirectFrame (CharList[curr_character].preview, xleft + 300, ytop + 65, 
			128, 128, 3, colWhite);

		// player selection
		if (curr_focus == 2) col = colDYell; else col = colWhite;
		DrawFrameX (xleft, ytop-4, 200, 44, 3, colMBackgr, col, 1.0);
		FT.DrawString (xleft+20, ytop, "Little Bear");
		Tex.DrawDirectFrame (Tex.TexID (44), xleft + 30, ytop + 65, 
			128, 128, 3, colWhite);

		FT.SetColor (colWhite);
		FT.DrawString (-1, ytop+240, "Register a new player");

		PrintArrow (0, (curr_character > 0));	
		PrintArrow (1, (curr_character < last_character));
		PrintTextButton (0, curr_focus);

	}
	if (param.ice_cursor) DrawCursor ();
    Winsys.SwapBuffers();

	// =========================== pass 0 =============================

	if (pass == 0) {
		Trans.LoadLanguages ();
		Trans.LoadTranslations (param.language);
		LoadCreditList ();
		Char.LoadCharacterList ();
		Course.LoadObjectTypes (); 
		Course.LoadTerrainTypes ();
		Env.LoadEnvironmentList ();
		Course.LoadCourseList ();
		Events.LoadEventList ();
		Players.LoadParams ();
		pass = 1;

		CharList = Char.CharList;
		last_character = Char.numCharacters - 1;


		SDL_Delay (100);
	}
} 

void SplashTerm () {
}

void splash_screen_register() {
	Winsys.SetModeFuncs (SPLASH, SplashInit, SplashLoop, SplashTerm,
 		SplashKeys, SplashMouseFunc, SplashMotionFunc, NULL, NULL);
}
