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

static int xleft, ytop;
static int curr_focus = 0;
static TVector2 cursor_pos = {0, 0};
static string name;
static int posit;
static float curposit;
static int maxlng = 32;
static bool crsrvisible = true;
static float crsrtime = 0;
static int curr_avatar = 0;
static int last_avatar = 0;

#define CRSR_PERIODE 0.4

void DrawCrsr (float x, float y, int pos, double timestep) {
	if (crsrvisible) {
		float w = 3;
		float h = 26;
		TColor col = MakeColor (1, 1, 0, 1);
		float scrheight = param.y_resolution;

		glDisable (GL_TEXTURE_2D);
		glColor4f (col.r, col.g, col.b, col.a);
		glBegin (GL_QUADS);
			glVertex2f (x,   scrheight-y-h);
			glVertex2f (x+w, scrheight-y-h);
			glVertex2f (x+w, scrheight-y);
			glVertex2f (x,   scrheight-y);
		glEnd();
		glEnable (GL_TEXTURE_2D);
	}
	crsrtime += timestep;
	if (crsrtime > CRSR_PERIODE) {
		crsrtime = 0;
		crsrvisible = !crsrvisible;
	}
}

int len (const string s) {return s.size(); }

void CalcCursorPos () {
	int le = len (name);
	if (posit == 0) curposit = 0;
	if (posit > le) posit = le;
	string temp = name.substr (0, posit);
	curposit = FT.GetTextWidth (temp);
}

void NameInsert (string ss) {
	int le = len (name);
	if (posit > le) posit = le;
	name.insert (posit, ss);	
}

void NameInsert (char *ss) {
	int le = len (name);
	if (posit > le) posit = le;
	name.insert (posit, ss);	
}

void NameInsert (char c) {
	char temp[2];
	temp[0] = c;
	temp[1] = 0;
	NameInsert (temp);
}

void NameDelete (int po) {
	int le = len (name);
	if (po > le) po = le; 
	name.erase (po, 1);
}

void QuitAndAddPlayer () {
	if (name.size () > 0)
		Players.AddPlayer (name, Players.GetDirectAvatarName (curr_avatar));
	Winsys.SetMode (REGIST);
}

// NewPlayerKeys not used any longer, see NewPlayerKeySpec instead
void NewPlayerKeys (unsigned int key, bool special, bool release, int x, int y) {}

void ChangeAvatarSelection (int focus, int dir) {
	if (dir == 0) {
		switch (focus) {
			case 0:	if (curr_avatar > 0) curr_avatar--; break;
		}
	} else {
		switch (focus) {
			case 0:	if (curr_avatar < last_avatar) curr_avatar++; break;
		}
	}
}

/*
typedef struct{
  Uint8 scancode;
  SDLKey sym;
  SDLMod mod;
  Uint16 unicode;
} SDL_keysym;*/

void NewPlayerKeySpec (SDL_keysym sym, bool release) {
	if (release) return;
	unsigned int key = sym.sym;
	unsigned int mod = sym.mod;

	crsrtime = 0;
	crsrvisible = true;

	if (islower (key)) {
		if (len (name) < maxlng) {
			if (mod & KMOD_SHIFT) NameInsert (toupper (key));
			else NameInsert (key);
			posit++;
		}
	} else if (isdigit (key)) {
		if (len (name) < maxlng) {
			NameInsert (key);
			posit++;
		}
	} else {
		switch (key) {
			case 127: if (posit < len(name)) NameDelete (posit); break;
			case 8: if (posit > 0) NameDelete (posit-1); posit--; break;
			case 27: Winsys.SetMode (REGIST); break;
			case 13: 
				if (curr_focus == 1) Winsys.SetMode (REGIST);
				else QuitAndAddPlayer (); 
				break;
			case SDLK_RIGHT: if (posit < len(name)) posit++; break;
			case SDLK_LEFT: if (posit > 0) posit--; break;
			case 278: posit = 0; break;
			case 279: posit = len (name); break;
			
			case 32: NameInsert (32); posit++; break;
			case SDLK_UP: if (curr_avatar>0) curr_avatar--; break;
			case SDLK_DOWN: if (curr_avatar<last_avatar) curr_avatar++; break;
			case SDLK_TAB: curr_focus++; if (curr_focus>2) curr_focus =0; break;
		}
	}
}

void NewPlayerMouseFunc (int button, int state, int x, int y) {
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		switch (foc) {
			case 0: ChangeAvatarSelection (foc, dir); break;
			case 1: Winsys.SetMode (REGIST); break;
			case 2: QuitAndAddPlayer(); break;
		}
	}
}

void NewPlayerMotionFunc (int x, int y ){
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

void NewPlayerInit (void) {  
	Winsys.KeyRepeat (true);
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);

	xleft = (param.x_resolution - 400) / 2;
	ytop = AutoYPos (200);
	g_game.loopdelay = 10;
	name = "";	
	posit = 0;

	ResetWidgets ();
	AddArrow (xleft + 340, ytop+165, 0, 0);
	AddArrow (xleft + 340, ytop+183, 1, 0);
	AddTextButton (Trans.Text(8), xleft + 100, ytop + 280, 1, -1);
	AddTextButton (Trans.Text(15), xleft + 300, ytop + 280, 2, -1);

	last_avatar = Players.numAvatars - 1;
	curr_focus = 0;
}

void NewPlayerLoop (double timestep ){
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

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);

	FT.SetColor (colWhite);
	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (20);
	FT.DrawString (-1, ytop, "Enter a name for the new player:");
	if (param.use_papercut_font > 0) FT.SetSize (24); else FT.SetSize (16);
	FT.DrawString (xleft + 70, ytop+160, "Select an avatar:");

	DrawFrameX (xleft, ytop+60, 400, 44, 3, colMBackgr, colWhite, 1.0);
	FT.DrawString (xleft+20, ytop+64, name);
 	CalcCursorPos ();
	DrawCrsr (xleft+20+curposit+1, ytop+69, 0, timestep);

	if (curr_focus == 0) col = colDYell; else col = colWhite;
	Tex.DrawDirectFrame (Players.GetDirectAvatarID (curr_avatar), 
			xleft+250, ytop + 145, 75, 75, 2, col);

	FT.SetColor (colWhite);
	PrintArrow (0, (curr_avatar > 0));	
 	PrintArrow (1, (curr_avatar < last_avatar));
	PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	if (param.ice_cursor) DrawCursor ();
    Winsys.SwapBuffers();
} 

void NewPlayerTerm () {
	Winsys.SetFonttype ();
}

void NewPlayerRegister() {
	Winsys.SetModeFuncs (NEWPLAYER, NewPlayerInit, NewPlayerLoop, NewPlayerTerm,
	NULL, NewPlayerMouseFunc, NewPlayerMotionFunc, NULL, NULL, NewPlayerKeySpec);
}
