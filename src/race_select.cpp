/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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

static TUpDown* courseGroup;
static TUpDown* course;
static TFramedText* courseGroupName;
static TFramedText* courseName;
static TIconButton* light;
static TIconButton* snow;
static TIconButton* wind;
static TIconButton* mirror;
static TIconButton* random_btn;
static TWidget* textbuttons[2];
static sf::String info;
static int prevGroup = 0;

static void UpdateInfo() {
	if (mirror->focus && mirror->GetValue() < 2)
		info = Trans.Text(69 + mirror->GetValue());
	else if (light->focus && light->GetValue() < 4)
		info = Trans.Text(71 + light->GetValue());
	else if (snow->focus && snow->GetValue() < 4)
		info = Trans.Text(75 + snow->GetValue());
	else if (wind->focus && wind->GetValue() < 4)
		info = Trans.Text(79 + wind->GetValue());
	else if (random_btn->focus)
		info = Trans.Text(83);
	else
		info = "";
}

void SetRaceConditions() {
	g_game.mirrorred = mirror->GetValue() != 0;
	g_game.light_id = light->GetValue();
	g_game.snow_id = snow->GetValue();
	g_game.wind_id = wind->GetValue();

	g_game.course = &(*Course.currentCourseList)[course->GetValue()];
	g_game.theme_id = (*Course.currentCourseList)[course->GetValue()].music_theme;
	g_game.game_type = PRACTICING;
	State::manager.RequestEnterState(Loading);
}

void CRaceSelect::Motion(int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow(cursor_pos);

	UpdateInfo();
}

void CRaceSelect::Mouse(int button, int state, int x, int y) {
	if (state == 1) {
		ClickGUI(x, y);

		if (textbuttons[0]->focussed())
			SetRaceConditions();
		else if (textbuttons[1]->focussed())
			State::manager.RequestEnterState(GameTypeSelect);

		if (random_btn->focussed()) {
			mirror->SetValue(IRandom(0, 1));
			light->SetValue(IRandom(0, 3));
			snow->SetValue(IRandom(0, 3));
			wind->SetValue(IRandom(0, 3));
		}

		UpdateInfo();
	}
}

void CRaceSelect::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	KeyGUI(key, release);
	UpdateInfo();
	switch (key) {
		case sf::Keyboard::Escape:
			State::manager.RequestEnterState(GameTypeSelect);
			break;
		case sf::Keyboard::U:
			param.ui_snow = !param.ui_snow;
			break;
		case sf::Keyboard::T:
			g_game.force_treemap = !g_game.force_treemap;
			break;
		case sf::Keyboard::C:
			g_game.treesize++;
			if (g_game.treesize > 5) g_game.treesize = 1;
			break;
		case sf::Keyboard::V:
			g_game.treevar++;
			if (g_game.treevar > 5) g_game.treevar = 1;
			break;
		case sf::Keyboard::Return:
			if (textbuttons[1]->focussed())
				State::manager.RequestEnterState(GameTypeSelect);
			else
				SetRaceConditions();
			break;
		default:
			break;
	}
}

// --------------------------------------------------------------------
static TArea area;
static int framewidth, frameheight, frametop;
static int prevtop, prevwidth, prevheight;
static int boxleft, boxwidth;
static int infotop;

void CRaceSelect::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);
	Music.Play(param.menu_music, true);

	framewidth = 550 * Winsys.scale;
	frameheight = 50 * Winsys.scale;
	frametop = AutoYPosN(30);

	area = AutoAreaN(30, 80, framewidth);
	prevheight = 144 * Winsys.scale;
	prevwidth = 192 * Winsys.scale;
	boxwidth = framewidth - prevwidth - 20;
	boxleft = area.right - boxwidth;
	int icontop = frametop + 2*(frameheight + 20) - 5;
	int iconsize = 32 * Winsys.scale;
	int iconspace = (int)((iconsize + 6) * 1.5);
	int iconsumwidth = iconspace * 4 + iconsize;
	int iconleft = (Winsys.resolution.width - iconsumwidth) / 2;
	infotop = icontop + iconsize + 6;
	prevtop = infotop + 35*Winsys.scale;
	ResetGUI();

	light = AddIconButton(iconleft, icontop, Tex.GetSFTexture(LIGHT_BUTT), iconsize, 3, (int)g_game.light_id);
	snow = AddIconButton(iconleft + iconspace, icontop, Tex.GetSFTexture(SNOW_BUTT), iconsize, 3, g_game.snow_id);
	wind = AddIconButton(iconleft + iconspace * 2, icontop, Tex.GetSFTexture(WIND_BUTT), iconsize, 3, g_game.wind_id);
	mirror = AddIconButton(iconleft + iconspace * 3, icontop, Tex.GetSFTexture(MIRROR_BUTT), iconsize, 1, (int)g_game.mirrorred);
	random_btn = AddIconButton(iconleft + iconspace * 4, icontop, Tex.GetSFTexture(RANDOM_BUTT), iconsize, 0, 0);
	int siz = FT.AutoSizeN(5);
	int len1 = FT.GetTextWidth(Trans.Text(13));
	textbuttons[0] = AddTextButton(Trans.Text(13), area.right-len1-50, AutoYPosN(85), siz);
	textbuttons[1] = AddTextButton(Trans.Text(8), area.left + 50, AutoYPosN(85), siz);
	FT.AutoSizeN(4);

	courseGroup = AddUpDown(area.left + framewidth + 8, frametop, 0, (int)Course.CourseLists.size() - 1, prevGroup);
	courseGroupName = AddFramedText(area.left, frametop, framewidth, frameheight, 3, colMBackgr, Course.currentCourseList->name, FT.GetSize(), true);
	course = AddUpDown(area.left + framewidth + 8, frametop + frameheight + 20, 0, (int)Course.currentCourseList->size() - 1, g_game.course ? (int)Course.GetCourseIdx(g_game.course) : 0);
	courseName = AddFramedText(area.left, frametop + frameheight + 20, framewidth, frameheight, 3, colMBackgr, "", FT.GetSize(), true);

	SetFocus(course);
}

void CRaceSelect::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}

	DrawGUIBackground(Winsys.scale);

	if (courseGroup->GetValue() != prevGroup) {
		prevGroup = courseGroup->GetValue();
		Course.currentCourseList = Course.getGroup((std::size_t)courseGroup->GetValue());
		g_game.course = nullptr;
		course->SetValue(0);
		course->SetMaximum((int)Course.currentCourseList->size()-1);
		courseGroupName->SetString(Course.currentCourseList->name);
	}
	// selected course
	courseGroupName->Focussed(courseGroup->focussed());
	courseName->Focussed(course->focussed());
	courseName->SetString((*Course.currentCourseList)[course->GetValue()].name);

	if ((*Course.currentCourseList)[course->GetValue()].preview)
		(*Course.currentCourseList)[course->GetValue()].preview->DrawFrame(area.left + 3, prevtop, prevwidth, prevheight, 3, colWhite);

	DrawFrameX(area.right-boxwidth, prevtop-3, boxwidth, prevheight+6, 3, colBackgr, colWhite, 1.f);
	FT.AutoSizeN(2);
	FT.SetColor(colWhite);
	int dist = FT.AutoDistanceN(0);
	for (std::size_t i = 0; i<(*Course.currentCourseList)[course->GetValue()].num_lines; i++) {
		FT.DrawString(boxleft + 8, prevtop + i*dist, (*Course.currentCourseList)[course->GetValue()].desc[i]);
	}

	FT.DrawString(CENTER, prevtop + prevheight + 10, Trans.Text(91) + ":  " + (*Course.currentCourseList)[course->GetValue()].author);

	FT.DrawString(CENTER, infotop, info);

	if (g_game.force_treemap) {
		FT.AutoSizeN(4);
		static const sf::String forcetrees = "Load trees.png";
		std::string sizevar = "Size: ";
		sizevar += Int_StrN(g_game.treesize);
		sizevar += " Variation: ";
		sizevar += Int_StrN(g_game.treevar);
		FT.SetColor(colYellow);
		FT.DrawString(CENTER, AutoYPosN(85), forcetrees);
		FT.DrawString(CENTER, AutoYPosN(90), sizevar);
	}

	DrawGUI();

	Winsys.SwapBuffers();
}
