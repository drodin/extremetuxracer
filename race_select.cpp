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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "race_select.h"
#include "ogl.h"
#include "textures.h"
#include "particles.h"
#include "audio.h"
#include "env.h"
#include "course.h"
#include "gui.h"
#include "font.h"
#include "translation.h"
#include "spx.h"
#include "game_type_select.h"
#include "loading.h"
#include "winsys.h"

CRaceSelect RaceSelect;

static TUpDown* course;
static TIconButton* light;
static TIconButton* snow;
static TIconButton* wind;
static TIconButton* mirror;
static TIconButton* random_btn;
static TWidget* textbuttons[2];
static string info;

static TCourse *CourseList;

static void UpdateInfo() {
	info = "";
	if(mirror->focus && mirror->GetValue() < 2) {
		info = Trans.Text(69 + mirror->GetValue());
	}
	else if(light->focus && light->GetValue() < 4) {
		info = Trans.Text(71 + light->GetValue());
	}
	else if(snow->focus && snow->GetValue() < 4) {
		info = Trans.Text(75 + snow->GetValue());
	}
	else if(wind->focus && wind->GetValue() < 4) {
		info = Trans.Text(79 + wind->GetValue());
	}
	else if(random_btn->focus) {
		info = Trans.Text(83);
	}
}

void SetRaceConditions (void) {
	g_game.mirror_id = mirror->GetValue() != 0;
	g_game.light_id = light->GetValue();
	g_game.snow_id = snow->GetValue();
	g_game.wind_id = wind->GetValue();

	g_game.course_id = course->GetValue();
	g_game.theme_id = CourseList[course->GetValue()].music_theme;
	g_game.game_type = PRACTICING;
	State::manager.RequestEnterState (Loading);
}

void CRaceSelect::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);

	UpdateInfo ();
}

void CRaceSelect::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		ClickGUI(x, y);

		if(textbuttons[0]->focussed())
			SetRaceConditions ();
		else if(textbuttons[1]->focussed())
			State::manager.RequestEnterState (GameTypeSelect);

		if(random_btn->focussed()) {
			mirror->SetValue(IRandom (0, 1));
			light->SetValue(IRandom (0, 3));
			snow->SetValue(IRandom (0, 3));
			wind->SetValue(IRandom (0, 3));
		}

		UpdateInfo ();
	}
}

void CRaceSelect::Keyb(unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	KeyGUI(key, 0, release);
	switch (key) {
		case SDLK_ESCAPE: State::manager.RequestEnterState (GameTypeSelect); break;
		case SDLK_u: param.ui_snow = !param.ui_snow; break;
		case SDLK_t: g_game.force_treemap = !g_game.force_treemap; break;
		case SDLK_c: g_game.treesize++;
				if (g_game.treesize > 5) g_game.treesize = 1; break;
		case SDLK_v: g_game.treevar++;
				if (g_game.treevar > 5) g_game.treevar = 1; break;
		case SDLK_RETURN: if (textbuttons[1]->focussed())
					 State::manager.RequestEnterState (GameTypeSelect);
				 else
					 SetRaceConditions (); break;
	}
}

// --------------------------------------------------------------------
static TArea area;
static int framewidth, frameheight, frametop;
static int prevtop, prevwidth, prevheight;
static int icontop, iconsize, iconspace, iconleft, iconsumwidth;
static int boxleft, boxwidth;

void CRaceSelect::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);
	Music.Play (param.menu_music, -1);

	CourseList = &Course.CourseList[0];

	framewidth = 550 * Winsys.scale;
	frameheight = 50 * Winsys.scale;
	frametop = AutoYPosN (30);

	area = AutoAreaN (30, 80, framewidth);
	prevtop = AutoYPosN (50);
	prevheight = 144 * Winsys.scale;
	prevwidth = 192 * Winsys.scale;
	boxwidth = framewidth - prevwidth - 20;
	boxleft = area.right - boxwidth;
	icontop = AutoYPosN (40);
	iconsize = 32 * Winsys.scale;
	iconspace = (int)((iconsize+6) * 1.5);
	iconsumwidth = iconspace * 4 + iconsize;
	iconleft = (Winsys.resolution.width - iconsumwidth) / 2;

	ResetGUI ();

	course = AddUpDown(area.left + framewidth + 8, frametop, 0, (int)Course.CourseList.size() - 1, (int)g_game.course_id);

	light = AddIconButton (iconleft, icontop, Tex.GetTexture (LIGHT_BUTT), iconsize, 3, (int)g_game.light_id);
	snow = AddIconButton (iconleft + iconspace, icontop, Tex.GetTexture (SNOW_BUTT), iconsize, 3, g_game.snow_id);
	wind = AddIconButton (iconleft + iconspace*2, icontop, Tex.GetTexture (WIND_BUTT), iconsize, 3, g_game.wind_id);
	mirror = AddIconButton (iconleft + iconspace*3, icontop, Tex.GetTexture (MIRROR_BUTT), iconsize, 1, (int)g_game.mirror_id);
	random_btn = AddIconButton (iconleft + iconspace*4, icontop, Tex.GetTexture (RANDOM_BUTT), iconsize, 0, 0);
	int siz = FT.AutoSizeN (5);
	double len1 = FT.GetTextWidth (Trans.Text(13));
	textbuttons[0] = AddTextButton (Trans.Text(13), area.right-len1-50, AutoYPosN (80), siz);
	textbuttons[1] = AddTextButton (Trans.Text(8), area.left + 50, AutoYPosN (80), siz);

	g_game.loopdelay = 20;
}

void CRaceSelect::Loop(double timestep) {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;
	TColor col;

	check_gl_error();
	ScopedRenderMode rm(GUI);
    ClearRenderContext ();
	SetupGuiDisplay ();

	Music.Update ();
	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow ();
	}

	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, colBlack, 0.2);

	// course selection
	if (course->focussed()) col = colDYell; else col = colWhite;
	DrawFrameX (area.left, frametop, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.AutoSizeN (4);
	FT.SetColor (colDYell);
	FT.DrawString (area.left+20, frametop, CourseList[course->GetValue()].name);

	if(CourseList[course->GetValue()].preview)
		CourseList[course->GetValue()].preview->DrawFrame(area.left + 3, prevtop, prevwidth, prevheight, 3, colWhite);

	DrawFrameX (area.right-boxwidth, prevtop-3, boxwidth, prevheight+6, 3, colBackgr, colWhite, 1.0);
	FT.AutoSizeN (2);
	FT.SetColor (colWhite);
	int dist = FT.AutoDistanceN (0);
	for (size_t i=0; i<CourseList[course->GetValue()].num_lines; i++) {
		FT.DrawString (boxleft+8, prevtop+i*dist, CourseList[course->GetValue()].desc[i]);
	}

	FT.DrawString (CENTER, prevtop + prevheight + 10, "Author:  " + CourseList[course->GetValue()].author);

	FT.DrawString (CENTER, AutoYPosN (45), info);

	if (g_game.force_treemap) {
		FT.AutoSizeN (4);
		static const string forcetrees = "Load trees.png";
		string sizevar = "Size: ";
		sizevar += Int_StrN (g_game.treesize);
		sizevar += " Variation: ";
		sizevar += Int_StrN (g_game.treevar);
		FT.SetColor (colYellow);
		FT.DrawString (CENTER, AutoYPosN (85), forcetrees);
		FT.DrawString (CENTER, AutoYPosN (90), sizevar);
	}

	DrawGUI();

    Winsys.SwapBuffers();
}
