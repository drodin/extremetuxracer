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
#include "spx.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "event_select.h"
#include "game_over.h"
#include "game_config.h"
#include "loading.h"
#include "winsys.h"

CEvent Event;

// ready: 0 - racing  1 - ready with success  2 - ready with failure
static int ready = 0; 						// indicates if last race is done
static TWidget* curr_focus = 0;
static TCup2 *ecup = 0;
static size_t curr_race = 0;
static size_t curr_bonus = 0;
static TVector2 cursor_pos(0, 0);
static TWidget* textbuttons[3];

void StartRace () {
	if (ready > 0) {
		State::manager.RequestEnterState (EventSelect);
		return;
	}
	g_game.mirror_id = false;
	g_game.course_id = ecup->races[curr_race]->course;
	g_game.theme_id = ecup->races[curr_race]->music_theme;
	g_game.light_id = ecup->races[curr_race]->light;
	g_game.snow_id = ecup->races[curr_race]->snow;
	g_game.wind_id = ecup->races[curr_race]->wind;
	g_game.herring_req = ecup->races[curr_race]->herrings;
	g_game.time_req = ecup->races[curr_race]->time;
	g_game.game_type = CUPRACING;
	State::manager.RequestEnterState (Loading);
}

void CEvent::Keyb (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
	case 13:
		if (curr_focus == textbuttons[0] && ready < 1) StartRace ();
		else State::manager.RequestEnterState (EventSelect);
		break;
	case 27:
		State::manager.RequestEnterState (EventSelect);
		break;
	case SDLK_TAB:
		if (ready > 0) {
			curr_focus = textbuttons[2];
		} else {
			if (curr_focus == textbuttons[0]) curr_focus = textbuttons[1]; else curr_focus = textbuttons[0];
		}
		break;
	case SDLK_LEFT: if (curr_focus == textbuttons[0]) curr_focus = textbuttons[1]; break;
	case SDLK_RIGHT: if (curr_focus == textbuttons[1]) curr_focus = textbuttons[0]; break;
	case SDLK_u: param.ui_snow = !param.ui_snow; break;
	}
}

void CEvent::Mouse (int button, int state, int x, int y) {
	if (state != 1) return;

	TWidget* clicked = ClickGUI(x, y);
	if(clicked == textbuttons[0]) {
		if (ready < 1)
			StartRace ();
	} else if(clicked == textbuttons[1])
		State::manager.RequestEnterState (EventSelect);
}

void CEvent::Motion (int x, int y) {
	TWidget* foc = MouseMoveGUI(x, y);
	if (foc != 0) curr_focus = foc;
	y = param.y_resolution - y;
    TVector2 old_pos = cursor_pos;
    cursor_pos = TVector2(x, y);

    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

void InitCupRacing () {
	ecup = g_game.cup;
	curr_race = 0;
	curr_bonus = ecup->races.size();
	ready = 0;
	curr_focus = 0;
}

void UpdateCupRacing () {
	size_t lastrace = ecup->races.size() - 1;
	curr_bonus += g_game.race_result;
	if (g_game.race_result >= 0) {
		if (curr_race < lastrace) curr_race++; else ready = 1;
	} else {
		if (curr_bonus == 0) ready = 2;
	}
	if (ready == 1) {
		Players.AddPassedCup (ecup->cup);
		Players.SavePlayers ();
	}
}

// --------------------------------------------------------------------

static TArea area;
static int messtop, messtop2;
static int bonustop, framewidth, frametop, framebottom;
static int dist, texsize;

void CEvent::Enter () {
	Winsys.ShowCursor (!param.ice_cursor);

	if (State::manager.PreviousState() == &GameOver) UpdateCupRacing ();
		else InitCupRacing ();

	framewidth = 500;
	frametop = AutoYPosN (45);
	area = AutoAreaN (30, 80, framewidth);
	messtop = AutoYPosN (50);
	messtop2 = AutoYPosN (60);
	bonustop = AutoYPosN (35);
	texsize = 32 * param.scale;
	if (texsize < 32) texsize = 32;
	dist = texsize + 2 * 4;
	framebottom = frametop + (int)ecup->races.size() * dist + 10;

	ResetGUI ();
	int siz = FT.AutoSizeN (5);
	textbuttons[1] = AddTextButton (Trans.Text(8), area.left + 100, AutoYPosN (80), siz);
	double len = FT.GetTextWidth (Trans.Text(13));
	textbuttons[0] = AddTextButton (Trans.Text(13), area.right -len - 100, AutoYPosN (80), siz);
	textbuttons[2] = AddTextButton (Trans.Text(15), CENTER, AutoYPosN (80), siz);

	Music.Play (param.menu_music, -1);
	if (ready < 1) curr_focus = textbuttons[0]; else curr_focus = textbuttons[2];
	g_game.loopdelay = 20;
}

int resultlevel (size_t num, size_t numraces) {
	if (num < 1) return 0;
	int q = (int)((num - 0.01) / numraces);
	return q + 1;
}

void CEvent::Loop (double timestep) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;

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

	if (ready == 0) {			// cup not finished
		FT.AutoSizeN (6);
		FT.SetColor (colWhite);
		FT.DrawString (CENTER, AutoYPosN (25), ecup->name);

		DrawBonusExt (bonustop, (int)ecup->races.size(), curr_bonus);

		DrawFrameX (area.left, frametop, framewidth,
			(int)ecup->races.size() * dist + 20, 3, colBackgr, colWhite, 1);

		for (size_t i=0; i<ecup->races.size(); i++) {
			FT.AutoSizeN (3);

			int y = frametop + 10 + (int)i * dist;
			if (i == curr_race)
				FT.SetColor (colDYell);
			else
				FT.SetColor (colWhite);
			FT.DrawString (area.left + 29, y, Course.CourseList[ecup->races[i]->course].name);
			Tex.Draw (CHECKBOX, area.right -54, y, texsize, texsize);
			if (curr_race > i) Tex.Draw (CHECKMARK, area.right-50, y + 4, 0.8);
		}

		FT.AutoSizeN (3);
		int ddd = FT.AutoDistanceN (1);
		FT.SetColor (colDBlue);
		string info = Trans.Text(11);
		info += "   " + Int_StrN (ecup->races[curr_race]->herrings.i);
		info += "   " + Int_StrN (ecup->races[curr_race]->herrings.j);
		info += "   " + Int_StrN (ecup->races[curr_race]->herrings.k);
		FT.DrawString (CENTER, framebottom+15, info);

		info = Trans.Text(12);
		info += "   " + Float_StrN (ecup->races[curr_race]->time.x, 0);
		info += "   " + Float_StrN (ecup->races[curr_race]->time.y, 0);
		info += "   " + Float_StrN (ecup->races[curr_race]->time.z, 0);
		info += "  " + Trans.Text(14);
		FT.DrawString (CENTER, framebottom+15+ddd, info);

	} else if (ready == 1) {		// cup successfully finished
		FT.AutoSizeN (5);
		FT.SetColor (colWhite);
		FT.DrawString (CENTER, messtop, Trans.Text(16));
		DrawBonusExt (bonustop, (int)ecup->races.size(), curr_bonus);
		int res = resultlevel(curr_bonus, ecup->races.size());
		FT.DrawString (CENTER, messtop2, Trans.Text(17) + "  "+Int_StrN (res));
	} else if (ready == 2) {		// cup finished but failed
		FT.AutoSizeN (5);
		FT.SetColor (colLRed);
		FT.DrawString (CENTER, messtop, Trans.Text(18));
		DrawBonusExt (bonustop, ecup->races.size(), curr_bonus);
		FT.DrawString (CENTER, messtop2, Trans.Text(19));
	}

	textbuttons[0]->SetVisible(ready < 1);
	textbuttons[1]->SetVisible(ready < 1);
	textbuttons[2]->SetVisible(!(ready < 1));

	DrawGUI ();
    Winsys.SwapBuffers();
}
