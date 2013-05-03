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

CGameConfig GameConfig;
static TVector2 cursor_pos = {0, 0};
static string res_names[NUM_RESOLUTIONS];

static bool curr_fullscreen = false;
static bool prev_fullscreen;
static int  curr_res = 0;
static int  prev_res;
static int  curr_focus = 0;
static int  curr_mus_vol = 20;
static int  curr_sound_vol = 100;
static int  curr_detail_level = 5;
static size_t  curr_language = 0;
static bool paramchanged = false;
static TLang *LangList;
static size_t lastLang = 0;

void RestartSDL () {
	// first close the resources that must be restored when
	// starting a new VideoMode. That are all resourcse controlled
	// by SDL or OpenGL
	Winsys.CloseJoystick ();		// controlled by SDL
	Tex.FreeTextureList ();			// textures are controlled by OpenGL
	Course.FreeCourseList ();		// contains the preview textures
	Char.FreeCharacterPreviews ();	// they are not reloaded !!!
	Audio.Close ();					// frees music and sound as well (SDL_mixer)
	SDL_Quit ();					// SDL main

	// second restore the freed resources
	Winsys.Init ();					// includes SetVideoMode 
 	Audio.Open ();					// clear, it has been closed before
	Sound.LoadSoundList ();			// all sounds must loaded again
	Music.LoadMusicList (); 		// same with music pieces
	Tex.LoadTextureList ();			// common textures
	Course.LoadCourseList ();		// only for the previews
	Course.ResetCourse ();			// the current course must be freed and reset

	// and some start settings which refer to the restored resources,
	// probably it's not necessary.
	Music.SetVolume (param.music_volume);
	g_game.course_id = 0;
	g_game.cup = 0;
	g_game.race_id = 0;
}

void SetConfig () {
	if (paramchanged) {	
		if (curr_res != prev_res || curr_fullscreen != prev_fullscreen) {
			// these changes require a new VideoMode
			param.res_type = curr_res;
			param.fullscreen = curr_fullscreen;

			#if defined (OS_WIN32_MINGW)
				// on windows we need to free and restore a lot of resources
				if (!param.restart_on_res_change) RestartSDL ();
			#elif defined (OS_LINUX)
				// on linux the resources seem to be kept at new VideoMode
				if (curr_res > 0) Winsys.SetupVideoMode (curr_res);
				else RestartSDL ();
			#endif
		}
		
		// the followind config params don't require a new VideoMode
		// they only must stored in the param structure (and saved)
		param.music_volume = curr_mus_vol;
		Music.SetVolume (param.music_volume);
		param.sound_volume = curr_sound_vol;
		param.perf_level = curr_detail_level;
		Winsys.SetFonttype ();
		if (param.language != curr_language) {
			param.language = curr_language;
			Trans.LoadTranslations (curr_language);
		}
		SaveConfigFile ();
	}
	State::manager.RequestEnterState(*State::manager.PreviousState());
}

void ChangeRes (int val) {
	curr_res += val;
	if (curr_res < 0) curr_res = 0;
	if (curr_res >= NUM_RESOLUTIONS) curr_res = NUM_RESOLUTIONS-1;
	paramchanged = true; 
}

void ToggleFullscreen () {
	curr_fullscreen = !curr_fullscreen; 
	paramchanged = true; 
}

void ChangeMusVol (int val) {
	curr_mus_vol += val;
	if (curr_mus_vol < 0) curr_mus_vol = 0;
	if (curr_mus_vol > 120) curr_mus_vol = 120;
	paramchanged = true; 
}

void ChangeSoundVol (int val) {
	curr_sound_vol += val;
	if (curr_sound_vol < 0) curr_sound_vol = 0;
	if (curr_sound_vol > 120) curr_sound_vol = 120;
	paramchanged = true; 
}

void ChangeDetail (int val) {
	curr_detail_level += val;
	if (curr_detail_level < 1) curr_detail_level = 1;
	if (curr_detail_level > 3) curr_detail_level = 3;
	paramchanged = true; 
}

void ChangeLanguage (int val) {
	curr_language += val;
	if (curr_language < 0) curr_language = 0;
	if (curr_language > lastLang) curr_language = lastLang;
	paramchanged = true; 
}

void CGameConfig::Keyb (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
		case SDLK_q: State::manager.RequestQuit(); break;
		case 27: State::manager.RequestEnterState (*State::manager.PreviousState()); break;
		case SDLK_TAB: if (curr_focus < 7) curr_focus++; else curr_focus = 0; break;
		case 13: 
			switch (curr_focus) {
				case 6: State::manager.RequestEnterState (*State::manager.PreviousState()); break;
				case 7: SetConfig (); break;
				default: SetConfig (); break;
			} break;
		case 276: 
			switch (curr_focus) {
				case 0: ToggleFullscreen (); break; 
				case 1: ChangeRes (-1); break;
				case 2: ChangeMusVol (-1); break;
				case 3: ChangeSoundVol (-1); break;
				case 4: ChangeDetail (-1); break;
				case 5: ChangeLanguage (-1); break;
			} break;
		case 275: 
			switch (curr_focus) {
				case 0: ToggleFullscreen (); break; 
				case 1: ChangeRes (1); break;
				case 2: ChangeMusVol (1); break;
				case 3: ChangeSoundVol (1); break;
				case 4: ChangeDetail (1); break;
				case 5: ChangeLanguage (1); break;
			} break;
		case 273: if (curr_focus > 0) curr_focus--; break;
		case 274: if (curr_focus < 7) curr_focus++; break;
	}
}

void ChangeConfigSelection (int focus, int dir) {
	if (dir == 0) {
		switch (focus) {
			case 1: ChangeRes (1); break;
			case 2: ChangeMusVol (1); break;
			case 3: ChangeSoundVol (1); break;
			case 4: ChangeDetail (1); break;
			case 5: ChangeLanguage (-1); break;
		}
	} else {
		switch (focus) {
			case 1: ChangeRes (-1); break;
			case 2: ChangeMusVol (-1); break;
			case 3: ChangeSoundVol (-1); break;
			case 4: ChangeDetail (-1); break;
			case 5: ChangeLanguage (1); break;
		}
	}
}

void CGameConfig::Mouse (int button, int state, int x, int y) {
	int focus, dr;
	if (state == 1) {
		GetFocus (x, y, &focus, &dr);
		switch (focus) {
			case 0: ToggleFullscreen (); break; 
			case 1: ChangeConfigSelection (focus, dr); break;
			case 2: ChangeConfigSelection (focus, dr); break;
			case 3: ChangeConfigSelection (focus, dr); break;
			case 4: ChangeConfigSelection (focus, dr); break;
			case 5: ChangeConfigSelection (focus, dr); break;
			case 6: State::manager.RequestEnterState (*State::manager.PreviousState()); break;
			case 7: SetConfig (); break;
		}
	}
}

void CGameConfig::Motion (int x, int y) {
 	int sc, dir;
	
	GetFocus (x, y, &sc, &dir);
	if (sc >= 0) curr_focus = sc;
	y = param.y_resolution - y;

    TVector2 old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

// ------------------ Init --------------------------------------------

static TArea area;
static int framewidth, frameheight;
static int dd, rightpos;

void CGameConfig::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);    
	Winsys.KeyRepeat (true);

	LangList = &Trans.languages[0];
	lastLang = Trans.languages.size() - 1;

	for (int i=0; i<NUM_RESOLUTIONS; i++) res_names[i] = Winsys.GetResName (i);
 
	paramchanged = false;

	// read the start params:
	curr_res = param.res_type;
	prev_res = param.res_type;
	curr_fullscreen = param.fullscreen;
	prev_fullscreen = param.fullscreen;
	curr_mus_vol = param.music_volume;
	curr_sound_vol = param.sound_volume;
	curr_detail_level = param.perf_level;
	curr_language = param.language;
	if (curr_language > lastLang) curr_language = lastLang;

	framewidth = 550 * param.scale;
	frameheight = 50 * param.scale;
	area = AutoAreaN (30, 80, framewidth);
	FT.AutoSizeN (4);
	dd = FT.AutoDistanceN (3);
	if (dd < 36) dd = 36;
	rightpos = area.right -48;

	ResetWidgets ();
	AddCheckbox (area.left, area.top, 0, framewidth-16, Trans.Text(31));
	AddArrow (rightpos, area.top+dd*1, 0, 1);
	AddArrow (rightpos, area.top+dd*1+18, 1, 1);
	AddArrow (rightpos, area.top+dd*2, 0, 2);
	AddArrow (rightpos, area.top+dd*2+18, 1, 2);
	AddArrow (rightpos, area.top+dd*3, 0, 3);
	AddArrow (rightpos, area.top+dd*3+18, 1, 3);
	AddArrow (rightpos, area.top+dd*4, 0, 4);
	AddArrow (rightpos, area.top+dd*4+18, 1, 4);
	AddArrow (rightpos, area.top+dd*5, 0, 5);
	AddArrow (rightpos, area.top+dd*5+18, 1, 5);	

	int siz = FT.AutoSizeN (5);
	AddTextButton (Trans.Text(28), area.left+50, AutoYPosN (80), 6, siz);
	double len = FT.GetTextWidth (Trans.Text(8));
	AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (80), 7, siz);

	curr_focus = 0;
	Music.Play (param.config_music, -1);
}

void CGameConfig::Loop (double time_step) {
	int ww = param.x_resolution;
	int hh = param.y_resolution;

	Music.Update ();    
			
	check_gl_error();
	Music.Update ();    
    set_gl_options (GUI);
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
	PrintCheckbox (0, curr_focus, curr_fullscreen);

	if (curr_focus == 1) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd, Trans.Text(32));
	if (curr_focus == 2) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*2, Trans.Text(33));
	if (curr_focus == 3) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*3, Trans.Text(34));
	if (curr_focus == 4) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*4, Trans.Text(36));
	if (curr_focus == 5) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top + dd*5, Trans.Text(35));

	FT.SetColor (colWhite);
	FT.DrawString (area.left+240, area.top + dd, res_names[curr_res]);
	FT.DrawString (area.left+240, area.top + dd*2, Int_StrN (curr_mus_vol));
	FT.DrawString (area.left+240, area.top + dd*3, Int_StrN (curr_sound_vol));
	FT.DrawString (area.left+240, area.top + dd*4, Int_StrN (curr_detail_level));
	FT.DrawString (area.left+240, area.top + dd*5, LangList[curr_language].language);

	PrintArrow (0, (curr_res < (NUM_RESOLUTIONS-1)));
	PrintArrow (1, (curr_res > 0));	
	PrintArrow (2, (curr_mus_vol < 120));
	PrintArrow (3, (curr_mus_vol > 0));	
	PrintArrow (4, (curr_sound_vol < 120));
	PrintArrow (5, (curr_sound_vol > 0));	
	PrintArrow (6, (curr_detail_level < 3));
	PrintArrow (7, (curr_detail_level > 1));	
	PrintArrow (8, (curr_language > 0));	
 	PrintArrow (9, (curr_language < lastLang));
	
	PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	#if defined (OS_WIN32_MINGW)
		if ((curr_res != prev_res || curr_fullscreen != prev_fullscreen) &&
		param.restart_on_res_change) {
			FT.SetColor (colDYell);
			FT.AutoSizeN (4);
			FT.DrawString (CENTER, AutoYPosN (68), "The video adjustments have changed,");
			FT.DrawString (CENTER, AutoYPosN (72), "You need to restart the game");
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

	if (param.ice_cursor) DrawCursor ();
	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

void CGameConfig::Exit() {
	Winsys.KeyRepeat (false);
}
