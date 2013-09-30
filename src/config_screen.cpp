/* --------------------------------------------------------------------
EXTREME TUXRACER

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

/*
If you want to add a new option, do this:
First add the option to the TParam struct (game_config.h).

Then edit the below functions:

- LoadConfigFile. Use
	SPIntN for integer and boolean values
	SPStrN for strings.
	The first value is always 'line', the second defines the tag within the
	brackets [ ], and the last value is the default.

- SetConfigDefaults. These values are used as long as no options file exists.
	It's a good idea to use the same values as the defaults in LoadConfigFile.

- SaveConfigFile. See the other entries; it should be self-explanatory.
	If an options file exists, you will have to change any value at runtime
	on the configuration screen to overwrite the file. Then you will see the
	new entry.
*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "config_screen.h"
#include "spx.h"
#include "translation.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "gui.h"
#include "textures.h"
#include "font.h"
#include "translation.h"
#include "course.h"
#include "game_ctrl.h"
#include "winsys.h"

CGameConfig GameConfig;
static string res_names[NUM_RESOLUTIONS];

static TLang *LangList;
static TCheckbox* fullscreen;
static TUpDown* language;
static TUpDown* resolution;
static TUpDown* mus_vol;
static TUpDown* sound_vol;
static TUpDown* detail_level;
static TWidget* textbuttons[2];


void SetConfig () {
	if (mus_vol->GetValue() != param.music_volume ||
	        sound_vol->GetValue() != param.sound_volume ||
	        language->GetValue() != param.language ||
	        resolution->GetValue() != param.res_type ||
	        detail_level->GetValue() != param.perf_level ||
	        fullscreen->checked != param.fullscreen) {

		if (resolution->GetValue() != param.res_type || fullscreen->checked != param.fullscreen) {
			// these changes require a new VideoMode
			param.res_type = resolution->GetValue();
#ifdef _WIN32
			if (fullscreen->checked == param.fullscreen)
				Winsys.SetupVideoMode(param.res_type);
			param.fullscreen = fullscreen->checked;
#else
			param.fullscreen = fullscreen->checked;
			Winsys.SetupVideoMode(param.res_type);
#endif
		}

		// the followind config params don't require a new VideoMode
		// they only must stored in the param structure (and saved)
		param.music_volume = mus_vol->GetValue();
		Music.SetVolume (param.music_volume);
		param.sound_volume = sound_vol->GetValue();
		param.perf_level = detail_level->GetValue();
		Winsys.SetFonttype ();
		if (param.language != language->GetValue()) {
			param.language = language->GetValue();
			Trans.LoadTranslations (param.language);
		}
		SaveConfigFile ();
	}
	State::manager.RequestEnterState(*State::manager.PreviousState());
}

void CGameConfig::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;

	if (key != SDLK_UP && key != SDLK_DOWN)
		KeyGUI(key, 0, release);
	switch (key) {
		case SDLK_q:
			State::manager.RequestQuit();
			break;
		case SDLK_ESCAPE:
			State::manager.RequestEnterState (*State::manager.PreviousState());
			break;
		case SDLK_RETURN:
			if (textbuttons[0]->focussed())
				State::manager.RequestEnterState (*State::manager.PreviousState());
			else if (textbuttons[1]->focussed())
				SetConfig ();
			break;
		case SDLK_UP:
			DecreaseFocus();
			break;
		case SDLK_DOWN:
			IncreaseFocus();
			break;
	}
}

void CGameConfig::Mouse (int button, int state, int x, int y) {
	if (state == 1) {
		TWidget* focussed = ClickGUI(x, y);

		if (focussed == textbuttons[0])
			State::manager.RequestEnterState (*State::manager.PreviousState());
		else if (focussed == textbuttons[1])
			SetConfig ();
	}
}

void CGameConfig::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

// ------------------ Init --------------------------------------------

static TArea area;
static int dd;

void CGameConfig::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);
	Winsys.KeyRepeat (true);

	LangList = &Trans.languages[0];

	for (int i=0; i<NUM_RESOLUTIONS; i++) res_names[i] = Winsys.GetResName (i);

	int framewidth = 550 * Winsys.scale;
	area = AutoAreaN (30, 80, framewidth);
	FT.AutoSizeN (4);
	dd = FT.AutoDistanceN (3);
	if (dd < 36) dd = 36;
	int rightpos = area.right -48;

	ResetGUI ();
	fullscreen = AddCheckbox (area.left, area.top, framewidth-16, Trans.Text(31));
	fullscreen->checked = param.fullscreen;

	resolution = AddUpDown(rightpos, area.top+dd*1, 0, NUM_RESOLUTIONS-1, (int)param.res_type);
	mus_vol = AddUpDown(rightpos, area.top+dd*2, 0, 120, param.music_volume);
	sound_vol = AddUpDown(rightpos, area.top+dd*3, 0, 120, param.sound_volume);
	detail_level = AddUpDown(rightpos, area.top+dd*4, 1, 3, param.perf_level);
	language = AddUpDown(rightpos, area.top+dd*5, 0, (int)Trans.languages.size() - 1, (int)param.language);

	int siz = FT.AutoSizeN (5);
	textbuttons[0] = AddTextButton (Trans.Text(28), area.left+50, AutoYPosN (80), siz);
	double len = FT.GetTextWidth (Trans.Text(8));
	textbuttons[1] = AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (80), siz);

	Music.Play (param.config_music, -1);
}

void CGameConfig::Loop (double time_step) {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;

	Music.Update ();

	check_gl_error();
	Music.Update ();
	ScopedRenderMode rm(GUI);
	ClearRenderContext ();
	SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow (time_step);
		draw_ui_snow();
	}

	Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, colBlack, 0.2);

	FT.AutoSizeN (4);

	if (resolution->focussed()) FT.SetColor (colDYell);
	else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd, Trans.Text(32));
	if (mus_vol->focussed()) FT.SetColor (colDYell);
	else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*2, Trans.Text(33));
	if (sound_vol->focussed()) FT.SetColor (colDYell);
	else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*3, Trans.Text(34));
	if (detail_level->focussed()) FT.SetColor (colDYell);
	else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*4, Trans.Text(36));
	if (language->focussed()) FT.SetColor (colDYell);
	else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*5, Trans.Text(35));

	FT.SetColor (colWhite);
	FT.DrawString (area.left+240, area.top + dd, res_names[resolution->GetValue()]);
	FT.DrawString (area.left+240, area.top + dd*2, Int_StrN (mus_vol->GetValue()));
	FT.DrawString (area.left+240, area.top + dd*3, Int_StrN (sound_vol->GetValue()));
	FT.DrawString (area.left+240, area.top + dd*4, Int_StrN (detail_level->GetValue()));
	FT.DrawString (area.left+240, area.top + dd*5, LangList[language->GetValue()].language);

#if defined (_WIN32)
	if (fullscreen->checked != param.fullscreen) {
		FT.SetColor (colDYell);
		FT.AutoSizeN (4);
		FT.DrawString (CENTER, AutoYPosN (68), Trans.Text(84));
		FT.DrawString (CENTER, AutoYPosN (72), Trans.Text(85));
	} else {
		FT.SetColor (colLGrey);
		FT.AutoSizeN (3);
		FT.DrawString (CENTER, AutoYPosN (68), Trans.Text(41));
		FT.DrawString (CENTER, AutoYPosN (72), Trans.Text(42));
	}
#else
	FT.SetColor (colWhite);
	FT.AutoSizeN (3);
	FT.DrawString (CENTER, AutoYPosN (68), Trans.Text(41));
	FT.DrawString (CENTER, AutoYPosN (72), Trans.Text(42));
#endif

	DrawGUI();

	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

void CGameConfig::Exit() {
	Winsys.KeyRepeat (false);
}
