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

static int curr_focus = 0;
static int curr_light = 0;
static int lastLight = 3;
static int curr_snow = 0;
static int lastSnow = 3;
static int curr_wind = 0;
static int lastWind = 3;
static int curr_mirror = 0;
static int lastMirror = 1;
static int curr_random = 0;
static int lastRandom = 1;
static int curr_char = 0;
static int lastChar = 3;
static int curr_course = 0;
static int lastCourse;
static TVector2 cursor_pos = {0, 0};

static TCourse *CourseList;
static int xleft, ytop;

void SetRaceConditions (void) {
	if (curr_random > 0) {
		g_game.mirror_id = IRandom (0, lastMirror);
		g_game.char_id = IRandom (0, lastChar);
		g_game.light_id = IRandom (0, lastLight);
		g_game.snow_id = IRandom (0, lastSnow);
		g_game.wind_id = IRandom (0, lastWind);
		curr_random = 0;
	} else {
		g_game.mirror_id = curr_mirror;
		g_game.char_id = curr_char;
		g_game.light_id = curr_light;
		g_game.snow_id = curr_snow;
		g_game.wind_id = curr_wind;
	}
	g_game.course_id = curr_course;
	g_game.theme_id = CourseList[curr_course].music_theme;
	g_game.game_type = PRACTICING;
	Winsys.SetMode (LOADING); 
}

void ChangeSelection (int focus, int dir) {
	if (dir == 0) {
		switch (focus) {
			case 0:	if (curr_course > 0) curr_course--; break;
			case 1: if (curr_light > 0) curr_light--; break;
			case 2: if (curr_snow > 0) curr_snow--; break;
			case 3: if (curr_wind > 0) curr_wind--; break;	
			case 4: if (curr_mirror > 0) curr_mirror--; break;	
			case 5: if (curr_random > 0) curr_random--; break;	
			case 6: if (curr_char > 0) curr_char--; break;	
		}
	} else {
		switch (focus) {
			case 0:	if (curr_course < lastCourse) curr_course++; break;
			case 1: if (curr_light < lastLight) curr_light++; break;
			case 2: if (curr_snow < lastSnow) curr_snow++; break;
			case 3: if (curr_wind < lastWind) curr_wind++; break;	
			case 4: if (curr_mirror < lastMirror) curr_mirror++; break;	
			case 5: if (curr_random < lastRandom) curr_random++; break;	
			case 6: if (curr_char < lastChar) curr_char++; break;	
		}
	}
}

static void RaceSelectMotionFunc (int x, int y) {
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

static void RaceSelectMouseFunc (int button, int state, int x, int y ){
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		switch (foc) {
			case 0: ChangeSelection (foc, dir); break;
			case 1: if (curr_light < lastLight) curr_light++; else curr_light = 0; break;
			case 2: if (curr_snow < lastSnow) curr_snow++; else curr_snow = 0; break;
			case 3: if (curr_wind < lastWind) curr_wind++; else curr_wind = 0; break;
			case 4: if (curr_mirror < lastMirror) curr_mirror++; else curr_mirror = 0; break;
			case 5: if (curr_random < lastRandom) curr_random++; else curr_random = 0; break;
			case 6: if (curr_char < lastChar) curr_char++; else curr_char = 0; break;
			case 7: SetRaceConditions (); break;
			case 8: Winsys.SetMode (GAME_TYPE_SELECT); break;
		}
	}
}

static void RaceSelectKeys 
		(unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case 27: Winsys.SetMode (GAME_TYPE_SELECT); break;
		case SDLK_TAB: if (curr_focus < 8) curr_focus++; else curr_focus = 0; break;
		case SDLK_DOWN: ChangeSelection (curr_focus, 1); break;
		case SDLK_UP: ChangeSelection (curr_focus, 0); break;
		case SDLK_LEFT: ChangeSelection (curr_focus, 0); break;
		case SDLK_RIGHT: ChangeSelection (curr_focus, 1); break;
		case SDLK_u: param.ui_snow = !param.ui_snow; break;
		case SDLK_t: g_game.force_treemap = !g_game.force_treemap; break;
		case SDLK_c: g_game.treesize++; 
				if (g_game.treesize > 5) g_game.treesize = 1; break;
		case SDLK_v: g_game.treevar++; 
				if (g_game.treevar > 5) g_game.treevar = 1; break;
		case 13: if (curr_focus == 8) Winsys.SetMode (GAME_TYPE_SELECT);
				 else SetRaceConditions ();	break;
	}
}

// --------------------------------------------------------------------

void RaceSelectInit (void) {
	Winsys.ShowCursor (!param.ice_cursor);    
	Music.Play (param.menu_music, -1);
	
	CourseList = Course.CourseList;
	lastCourse = Course.numCourses - 1;

	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (170);
	ResetWidgets ();
	AddArrow (xleft + 470, ytop, 0, 0);
	AddArrow (xleft + 470, ytop+18, 1, 0);

	int buttoffs = 80; // defines x position of first button
	AddIconButton (xleft + buttoffs + 10, ytop + 55, 1, Tex.TexID (LIGHT_BUTT), 32); 
	AddIconButton (xleft + buttoffs + 60, ytop + 55, 2, Tex.TexID (SNOW_BUTT), 32); 
	AddIconButton (xleft + buttoffs + 110, ytop + 55, 3, Tex.TexID (WIND_BUTT), 32); 
	AddIconButton (xleft + buttoffs + 160, ytop + 55, 4, Tex.TexID (MIRROR_BUTT), 32); 
	AddIconButton (xleft + buttoffs + 210, ytop + 55, 5, Tex.TexID (RANDOM_BUTT), 32); 
	AddIconButton (xleft + buttoffs + 260, ytop + 55, 6, Tex.TexID (CHAR_BUTT), 32); 

	AddTextButton (Trans.Text(13), xleft + 300, ytop + 280, 7, -1);
	AddTextButton (Trans.Text(8), xleft + 100, ytop + 280, 8, -1);
	curr_focus = 0;
}

void RaceSelectLoop (double timestep){
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	TColor col;
		
	check_gl_error();
   	set_gl_options (GUI );
    ClearRenderContext ();
	SetupGuiDisplay ();

	Music.Update ();    

	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow ();
	}

	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	
	// course selection
	if (curr_focus == 0) col = colDYell; else col = colWhite;
	DrawFrameX (xleft, ytop-4, 460, 44, 3, colMBackgr, col, 1.0);
	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	FT.SetColor (colDYell);
	FT.DrawString (xleft+20, ytop, CourseList[curr_course].name);
	
	Tex.DrawDirectFrame (CourseList[curr_course].preview, xleft + 3, ytop + 105, 
		192, 144, 3, colWhite);
	DrawFrameX (xleft+210, ytop+102, 250, 150, 3, colBackgr, colWhite, 1.0);
	if (param.use_papercut_font > 0) FT.SetSize (18); else FT.SetSize (12);
	FT.SetColor (colWhite);
	for (int i=0; i<CourseList[curr_course].num_lines; i++) {
		FT.DrawString (xleft+220, ytop+105+i*16, CourseList[curr_course].desc[i]);
	}

	PrintArrow (0, (curr_course > 0));	
	PrintArrow (1, (curr_course < lastCourse));

	PrintIconButton (0, curr_focus, curr_light);
	PrintIconButton (1, curr_focus, curr_snow);
	PrintIconButton (2, curr_focus, curr_wind);
	PrintIconButton (3, curr_focus, curr_mirror);
	PrintIconButton (4, curr_focus, curr_random);
	PrintIconButton (5, curr_focus, curr_char);

	PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (20);
	string forcetrees = "Load trees.png";
	string sizevar = "Size: ";
	sizevar += Int_StrN (g_game.treesize);
	sizevar += " Variation: ";
	sizevar += Int_StrN (g_game.treevar);
	if (g_game.force_treemap) {
		FT.SetColor (colYellow);
		FT.DrawString (-1, ytop + 340, forcetrees);
		FT.DrawString (-1, ytop + 380, sizevar);
	}

	if (param.ice_cursor) DrawCursor ();
    SDL_GL_SwapBuffers ();
} 

void RaceSelectTerm () {}

void RaceSelectRegister () {
	Winsys.SetModeFuncs (RACE_SELECT, RaceSelectInit, RaceSelectLoop, RaceSelectTerm,
 		RaceSelectKeys, RaceSelectMouseFunc, RaceSelectMotionFunc, NULL, NULL);
}
