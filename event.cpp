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

#include "event.h"
#include "ogl.h"
#include "audio.h"
#include "particles.h"
#include "textures.h"
#include "gui.h"
#include "course.h"
#include "env.h"
#include "spx.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"

// ready: 0 - racing  1 - ready with success  2 - ready with failure
static int ready = 0; 						// indicates if last race is done
static int curr_focus = 0;
static TCup2 *ecup = 0;						
static TRace2 *eraces[MAX_RACES_PER_CUP];	
static int ecourseidx[MAX_RACES_PER_CUP];	
static int curr_race = 0;
static int curr_bonus = 0;
static int xleft, ytop, ytop2;
static TVector2 cursor_pos = {0, 0};

void StartRace () {
	if (ready > 0) {
		Winsys.SetMode (EVENT_SELECT);
		return;
	}
	g_game.mirror_id = 0;
	g_game.course_id = eraces[curr_race]->course;
	g_game.light_id = eraces[curr_race]->light;
	g_game.snow_id = eraces[curr_race]->snow;
	g_game.wind_id = eraces[curr_race]->wind;
	g_game.herring_req = eraces[curr_race]->herrings;
	g_game.time_req = eraces[curr_race]->time;
	g_game.numraces = ecup->num_races;
	g_game.GameType = CUPRACING;
	Winsys.SetMode (LOADING); 
}

void EventKeys (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
	case 13: 
		if (curr_focus < 1 && ready < 1) StartRace (); 
		else Winsys.SetMode (EVENT_SELECT);
		break;
	case 27:
		Winsys.SetMode (EVENT_SELECT);
		break;
	case SDLK_TAB:
		if (ready > 0) {
			curr_focus = 2;
		} else {
			if (curr_focus < 1) curr_focus = 1; else curr_focus = 0;
		}
		break;
	case SDLK_LEFT: if (curr_focus < 1) curr_focus = 1; break; 
	case SDLK_RIGHT: if (curr_focus == 1) curr_focus = 0; break; 
	case SDLK_u: param.ui_snow = !param.ui_snow; break;
	}
}

void EventMouseFunc (int button, int state, int x, int y ) {
	int foc, dr;
	if (state != 1) return;

	GetFocus (x, y, &foc, &dr);
	if (ready < 1) {
		if (foc >= 0) {
			if (foc == 0) StartRace ();
			else Winsys.SetMode (EVENT_SELECT);
		}
	} else Winsys.SetMode (EVENT_SELECT);
}

void EventMotionFunc (int x, int y ){
	TVector2 old_pos;
 	int dir, foc;
	if (Winsys.ModePending ()) return;
	GetFocus (x, y, &foc, &dir); // necessary for drawing the cursor
	if (foc >= 0) curr_focus = foc;
	y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);

    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

void InitCupRacing () {
	ecup = &Events.CupList[g_game.cup_id];
	for (int i=0; i<ecup->num_races; i++) {
		eraces[i] = &Events.RaceList[ecup->races[i]];	// pointer to race struct
		ecourseidx[i] = eraces[i]->course;				// idx of course list
	}
	curr_race = 0;
	curr_bonus = ecup->num_races;
	ready = 0;
	curr_focus = 0;
}

void UpdateCupRacing () {
	int lastrace = ecup->num_races - 1;
	curr_bonus += g_game.race_result;
	if (g_game.race_result >= 0) {
		if (curr_race < lastrace) curr_race++; else ready = 1;
	} else {
		if (curr_bonus <= 0) ready = 2;
	}
	if (ready == 1) {
		Players.AddPassedCup (ecup->cup);
		Players.SaveParams ();
	}
}

// --------------------------------------------------------------------

void EventInit () {
	Winsys.ShowCursor (!param.ice_cursor);    
	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (210);
	ytop2 = ytop + 80;

	if (g_game.prev_mode == GAME_OVER) UpdateCupRacing ();
		else InitCupRacing ();

	ResetWidgets ();
	AddTextButton (Trans.Text(13), xleft + 300, ytop + ecup->num_races * 35 + 160, 0, -1);
	AddTextButton (Trans.Text(8), xleft + 100, ytop + ecup->num_races * 35 + 160, 1, -1);
	AddTextButton (Trans.Text(15), CENTER, ytop + ecup->num_races * 35 + 160, 2, -1);
	if (param.force_music_loop == false) Music.Play ("menu", -1);
	if (ready < 1) curr_focus = 0; else curr_focus = 2;
}

int resultlevel (int num, int numraces) {
	if (num < 1) return 0;
	int q = (int)((num - 0.01) / numraces);
	return q + 1;
} 

void EventLoop (double timestep) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;
 	int i;
	TColor col;
	int y;
	string info;

	check_gl_error();
	set_gl_options (GUI );
	Music.Update ();    
	if (param.force_music_loop == true) Music.Play ("menu", -1);
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

	if (ready == 0) {			// cup not finished
		FT.SetSize (AutoFtSize());
		FT.SetColor (colWhite);
		FT.DrawString (CENTER, ytop - 60, ecup->name);

		DrawBonusExt (ytop+20, ecup->num_races, curr_bonus);

		// race list:
		DrawFrameX (xleft, ytop2, 500, ecup->num_races * 35 + 20, 3, colBackgr, colWhite, 1);
		for (i=0; i<ecup->num_races; i++) {
			if (i == curr_race) col = colDYell; else col = colWhite;
			if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
				
			y = ytop2 + 10 + i * 35;
			FT.SetColor (col);
			FT.DrawString (xleft + 29, y, Course.CourseList[ecourseidx[i]].name);
			Tex.Draw (CHECKBOX, xleft + 440, y, 1.0);
			if (curr_race > i) Tex.Draw (CHECKMARK, xleft + 444, y + 4, 0.8);
		}			

		FT.SetColor (colWhite);
		info = Trans.Text(11);
		info += "   " + Int_StrN (eraces[curr_race]->herrings.i);
		info += "   " + Int_StrN (eraces[curr_race]->herrings.j);
		info += "   " + Int_StrN (eraces[curr_race]->herrings.k);
		FT.DrawString (xleft+30, ytop2 + ecup->num_races * 35 + 25, info);

		info = Trans.Text(12);
		info += "   " + Float_StrN (eraces[curr_race]->time.x, 0);
		info += "   " + Float_StrN (eraces[curr_race]->time.y, 0);
		info += "   " + Float_StrN (eraces[curr_race]->time.z, 0);
		info += "  " + Trans.Text(14);
		FT.DrawString (xleft+280, ytop2 + ecup->num_races * 35 + 25, info);

	} else if (ready == 1) {		// cup successfully finished
		FT.SetColor (colWhite);
		FT.DrawString (CENTER, ytop-20, Trans.Text(16));
		DrawBonusExt (ytop+60, ecup->num_races, curr_bonus);
		int res = resultlevel(curr_bonus, ecup->num_races);
		if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (20);
		FT.DrawString (CENTER, ytop + 120, Trans.Text(17) + "  "+Int_StrN (res));
	} else if (ready == 2) {		// cup finished but failed
		FT.SetColor (colRed);
		FT.DrawString (CENTER, ytop-20, Trans.Text(18));
		DrawBonusExt (ytop+60, ecup->num_races, curr_bonus);
		if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (20);
		FT.DrawString (CENTER, ytop + 120, Trans.Text(19));
	}
	if (ready < 1) {
		PrintTextButton (0, curr_focus);
		PrintTextButton (1, curr_focus);
	} else PrintTextButton (2, curr_focus);

	if (param.ice_cursor) DrawCursor ();
    SDL_GL_SwapBuffers ();
}

void EventTerm () {
}

void event_register() {
	Winsys.SetModeFuncs (EVENT, EventInit, EventLoop, EventTerm,
 		EventKeys, EventMouseFunc, EventMotionFunc, NULL, NULL);
}

