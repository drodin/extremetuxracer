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
static int curr_focus = 0;
static TCup2 *CupList;
static int curr_cup = 0;
static int last_cup;
static TVector2 cursor_pos = {0, 0};

void EnterEvent () {
	g_game.game_type = CUPRACING;
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
static TArea area;
static int framewidth, frameheight, frametop1, frametop2;

void EventSelectInit () {
	Winsys.ShowCursor (!param.ice_cursor);    
	EventList = Events.EventList;
	curr_event = 0;
	last_event = Events.numEvents - 1;
	CupList = Events.CupList;
	curr_cup = 0;
	last_cup = EventList[curr_event].num_cups - 1;
	curr_focus = 1;

	framewidth = 500 * param.scale;
	frameheight = 50 * param.scale;
	area = AutoAreaN (30, 80, framewidth);
	frametop1 = AutoYPosN (35);
	frametop2 = AutoYPosN (50);

	ResetWidgets ();
	AddArrow (area.right+8, frametop1, 0, 0);
	AddArrow (area.right+8, frametop1+18, 1, 0);
	AddArrow (area.right+8, frametop2, 0, 1);
	AddArrow (area.right+8, frametop2+18, 1, 1);

	int siz = FT.AutoSizeN (5);
	AddTextButton (Trans.Text(9), area.left+50, AutoYPosN (70), 2, siz);
	double len = FT.GetTextWidth (Trans.Text(8));
	AddTextButton (Trans.Text(8), area.right-len-50, AutoYPosN (70), 3, siz);

	Events.MakeUnlockList (Players.GetCurrUnlocked());
	Music.Play (param.menu_music, -1);
	g_game.loopdelay = 20;
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

	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), param.scale);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	
//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top, 
//			0, colMBackgr, colBlack, 0.2);

	FT.AutoSizeN (3);
	FT.SetColor (colWhite);
	FT.DrawString (area.left, AutoYPosN (30), Trans.Text (6));
	FT.DrawString (area.left,AutoYPosN (45), Trans.Text (7));
	if (Events.IsUnlocked (curr_event, curr_cup) == false) {
		FT.SetColor (colLGrey);
 		FT.DrawString (CENTER, AutoYPosN (58), Trans.Text (10));
	}

	FT.AutoSizeN (4);

	if (curr_focus == 0) col = colDYell; else col = colWhite;
	DrawFrameX (area.left, frametop1, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (colDYell);
	FT.DrawString (area.left + 20, frametop1, EventList[curr_event].name);

	if (curr_focus == 1) col = colDYell; else col = colWhite;
	DrawFrameX (area.left, frametop2, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	if (Events.IsUnlocked (curr_event, curr_cup)) FT.SetColor (colDYell); 
		else FT.SetColor (colLGrey);
	FT.DrawString (area.left + 20, frametop2, Events.GetCupTrivialName (curr_event, curr_cup));

	PrintArrow (0, (curr_event > 0));	
	PrintArrow (1, (curr_event < last_event));
	PrintArrow (2, (curr_cup > 0));	
	PrintArrow (3, (curr_cup < last_cup));

	if (Events.IsUnlocked (curr_event, curr_cup)) PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	if (param.ice_cursor) DrawCursor ();
    SDL_GL_SwapBuffers ();
}

void EventSelectTerm () {
}

void event_select_register() {
	Winsys.SetModeFuncs (EVENT_SELECT, EventSelectInit, EventSelectLoop, EventSelectTerm,
 		EventSelectKeys, EventSelectMouseFunc, EventSelectMotionFunc, NULL, NULL, NULL);
}

