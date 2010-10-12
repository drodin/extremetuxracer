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

#include "event_select.h"
#include "gui.h"
#include "font.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "course.h"
#include "textures.h"
#include "game_ctrl.h"
#include "translation.h"

static TEvent2 *EventList;
static int last_event;
static int curr_event = 0;
static int xleft, ytop, ytop2;
static int curr_focus = 0;
static TCup2 *CupList;
static int curr_cup = 0;
static int last_cup;
static TVector2 cursor_pos = {0, 0};

void EnterEvent () {
	g_game.GameType = CUPRACING;
	g_game.cup_id = EventList[curr_event].cups[curr_cup];
	g_game.race_id = 0;
	Winsys.SetMode (EVENT);
}

void ChangeEventSelection (int focus, int dir) {
	if (focus != 0) return;
	if (dir == 0 && curr_event > 0) {
		curr_event--;
		curr_cup = 0;
		last_cup = EventList[curr_event].num_cups - 1;
	}
	if (dir != 0 && curr_event < last_event) {
		curr_event++;
		curr_cup = 0;
		last_cup = EventList[curr_event].num_cups - 1;
	}
}

void ChangeCupSelection (int focus, int dir) {
	if (focus != 1) return;
	if (dir == 0 && curr_cup > 0) curr_cup--;
	if (dir != 0 && curr_cup < last_cup) curr_cup++;
}

void EventSelectKeys (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
		case 27: Winsys.SetMode (GAME_TYPE_SELECT); break;
		case SDLK_TAB: if (curr_focus < 3) curr_focus++; else curr_focus = 0; 
			if (Events.IsUnlocked (curr_event, curr_cup) == false && curr_focus == 2)
			curr_focus = 3;
			break;
		case SDLK_q: Winsys.Quit (); break;
		case 13: if (curr_focus == 3) Winsys.SetMode (GAME_TYPE_SELECT);
			else if (Events.IsUnlocked (curr_event, curr_cup)) EnterEvent(); break;
		case SDLK_DOWN: 
			if (curr_focus == 0) ChangeEventSelection (curr_focus, 1); 
			else ChangeCupSelection (curr_focus, 1); 
			break;
		case SDLK_UP: if (curr_focus == 0) ChangeEventSelection (curr_focus, 0); 
			else ChangeCupSelection (curr_focus, 0); break;
		case SDLK_u: param.ui_snow = !param.ui_snow; break;
	}
}

void EventSelectMouseFunc (int button, int state, int x, int y ) {
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		switch (foc) {
			case 0: ChangeEventSelection (foc, dir); break;
			case 1: ChangeCupSelection (foc, dir); break;
			case 2: if (Events.IsUnlocked (curr_event, curr_cup)) EnterEvent(); break;
			case 3: Winsys.SetMode (GAME_TYPE_SELECT); break;
		}
	}
}

void EventSelectMotionFunc (int x, int y ){
	TVector2 old_pos;
 	int foc, dir;
	if (Winsys.ModePending ()) return;
	GetFocus (x, y, &foc, &dir);
	if (foc >= 0) curr_focus = foc;
	   
	y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);

    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

// --------------------------------------------------------------------

void EventSelectInit () {
	Winsys.ShowCursor (!param.ice_cursor);    
	EventList = Events.EventList;
	curr_event = 0;
	last_event = Events.numEvents - 1;
	CupList = Events.CupList;
	curr_cup = 0;
	last_cup = EventList[curr_event].num_cups - 1;

	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (210);
	ytop2 = AutoYPos (320);
	ResetWidgets ();
	AddArrow (xleft + 470, ytop, 0, 0);
	AddArrow (xleft + 470, ytop+18, 1, 0);
	AddArrow (xleft + 470, ytop2, 0, 1);
	AddArrow (xleft + 470, ytop2+18, 1, 1);
	AddTextButton (Trans.Text (9), xleft + 300, ytop + 200, 2, -1);
	AddTextButton (Trans.Text (8), xleft + 100, ytop + 200, 3, -1);

	Events.MakeUnlockList (Players.GetCurrUnlocked());
	Music.Play ("start_screen", -1);
}

void EventSelectLoop (double timestep) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	TColor col;
		
	check_gl_error();
   	set_gl_options (GUI );
	Music.Update ();    
    ClearRenderContext ();
	SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow ();
	}

	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
	FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop-40, Trans.Text (6));
	FT.DrawString (xleft, ytop2-40, Trans.Text (7));
	if (Events.IsUnlocked (curr_event, curr_cup) == false) {
		FT.SetColor (colLGrey);
		FT.DrawString (CENTER, ytop2+60, Trans.Text (10));
	}

	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);

	if (curr_focus == 0) col = colDYell; else col = colWhite;
	DrawFrameX (xleft, ytop-4, 460, 44, 3, colMBackgr, col, 1.0);
	FT.SetColor (colDYell);
	FT.DrawString (xleft+20, ytop, EventList[curr_event].name);

	if (curr_focus == 1) col = colDYell; else col = colWhite;
	DrawFrameX (xleft, ytop2-4, 460, 44, 3, colMBackgr, col, 1.0);
	if (Events.IsUnlocked (curr_event, curr_cup)) FT.SetColor (colDYell); 
		else FT.SetColor (colLGrey);
	FT.DrawString (xleft+20, ytop2, Events.GetCupTrivialName (curr_event, curr_cup));

	PrintArrow (0, (curr_event > 0));	
	PrintArrow (1, (curr_event < last_event));
	PrintArrow (2, (curr_cup > 0));	
	PrintArrow (3, (curr_cup < last_cup));
	if (Events.IsUnlocked (curr_event, curr_cup)) PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	if (param.ice_cursor) DrawCursor ();
    SDL_GL_SwapBuffers ();
}

void EventSelectTerm () {}

void event_select_register() {
	Winsys.SetModeFuncs (EVENT_SELECT, EventSelectInit, EventSelectLoop, EventSelectTerm,
 		EventSelectKeys, EventSelectMouseFunc, EventSelectMotionFunc, NULL, NULL);
}

