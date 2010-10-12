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

#include "game_config.h"
#include "spx.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "gui.h"
#include "textures.h"
#include "font.h"
#include "translation.h"

TParam param;


void LoadConfigFile () {
	CSPList list(4);
	string line;
	if (!list.Load (param.configfile)) {
		Message ("Could not load 'options'");
		return;
	}

	for (int i=0; i<list.Count(); i++) {
		line = list.Line(i);

		param.fullscreen = SPIntN (line, "fullscreen", 0);
		param.res_type = SPIntN (line, "res_type", 0);
		param.perf_level = SPIntN (line, "detail_level", 0);
		param.language = SPIntN (line, "language", 0);
		param.sound_volume = SPIntN (line, "sound_volume", 100);
		param.music_volume = SPIntN (line, "music_volume", 20);

		param.forward_clip_distance = SPIntN (line, "forward_clip_distance", 75);
		param.backward_clip_distance = SPIntN (line, "backward_clip_distance", 20);
		param.fov = SPIntN (line, "fov", 60);
		param.bpp_mode = SPIntN (line, "bpp_mode", 1);
		param.tree_detail_distance = SPIntN (line, "tree_detail_distance", 20);
		param.tux_sphere_divisions = SPIntN (line, "tux_sphere_divisions", 10);
		param.tux_shadow_sphere_divisions = SPIntN (line, "tux_shadow_sphere_div", 3);
		param.course_detail_level = SPIntN (line, "course_detail_level", 75);

		param.use_papercut_font = SPIntN (line, "use_papercut_font", true);
		param.ice_cursor = SPIntN (line, "ice_cursor", true);
		param.full_skybox = SPIntN (line, "full_skybox", false);

	}		
}

void SetConfigDefaults () {
	param.fullscreen = true;
	param.res_type = 2; // 0=auto / 1=800x600 / 2=1024x768
 	param.perf_level = 3;	// detail level
	param.language = 0;
	param.sound_volume = 100;
	param.music_volume = 20;

	param.forward_clip_distance = 75;
	param.backward_clip_distance = 20;
	param.fov = 60;
	param.bpp_mode = 1;
	param.tree_detail_distance = 20;
	param.tux_sphere_divisions = 10;
	param.tux_shadow_sphere_divisions = 3;
	param.course_detail_level = 75;

	param.use_papercut_font = 1;
	param.ice_cursor = true;
	param.full_skybox = false;
}

void AddItem (CSPList *list, string tag, string content) {
	string item = "[" +tag + "] " + content;
	list->Add (item);
}

void AddIntItem (CSPList *list, string tag, int val) {
	string vs = Int_StrN (val);
	AddItem (list, tag, vs);
}

void SaveConfigFile () {
	CSPList liste(128);
	
	AddIntItem (&liste, "fullscreen", param.fullscreen);
	AddIntItem (&liste, "res_type", param.res_type);
	AddIntItem (&liste, "detail_level", param.perf_level);
	AddIntItem (&liste, "language", param.language);
    AddIntItem (&liste, "sound_volume", param.sound_volume);
    AddIntItem (&liste, "music_volume", param.music_volume);

	AddIntItem (&liste, "forward_clip_distance", param.forward_clip_distance);
	AddIntItem (&liste, "backward_clip_distance", param.backward_clip_distance);
	AddIntItem (&liste, "fov", param.fov);
	AddIntItem (&liste, "bpp_mode", param.bpp_mode);
	AddIntItem (&liste, "tree_detail_distance", param.tree_detail_distance);
	AddIntItem (&liste, "tux_sphere_divisions", param.tux_sphere_divisions);
	AddIntItem (&liste, "tux_shadow_sphere_div", param.tux_shadow_sphere_divisions);
	AddIntItem (&liste, "course_detail_level", param.course_detail_level);

	AddIntItem (&liste, "use_papercut_font", param.use_papercut_font);
	AddIntItem (&liste, "ice_cursor", param.ice_cursor);
	AddIntItem (&liste, "full_skybox", param.full_skybox);
	liste.Save (param.configfile);		
}

// --------------------------------------------------------------------

void InitConfig (char *arg0) {
#if defined (WIN32)
	// the progdir is always the current dir
	param.config_dir = "config";
	param.data_dir = "data";
	param.screenshot_dir = param.data_dir + SEP + "screenshots";
#else
	char buff[256];
	if (strcmp (arg0, "./etr") == 0) { 		// start from work directory
		char *s = getcwd (buff, 256);
		if (s==NULL) {};
	} else {								// start with full path
		strcpy (buff, arg0);		
		if (strlen (buff) > 5) buff[strlen(buff)-3] = 0;
	} 
	param.prog_dir = buff;
	struct passwd *pwent = getpwuid (getuid ());
	param.config_dir = pwent->pw_dir;
	param.config_dir += SEP;
	param.config_dir += CONFIG_DIR;
	// or: param.config_dir = param.prog_dir + SEP + "config";
    if (!DirExists (param.config_dir.c_str()))
		mkdir (param.config_dir.c_str(), 0775); 
	param.data_dir = param.prog_dir + SEP + "data";
	param.screenshot_dir = param.data_dir + SEP + "screenshots";
#endif

	param.obj_dir = param.data_dir + SEP + "objects";
	param.env_dir2 = param.data_dir + SEP + "env";
	param.char_dir = param.data_dir + SEP + "char";
	param.keyframe_dir = param.char_dir + SEP + "keyframes";
	param.terr_dir = param.data_dir + SEP + "terrains";
	param.tex_dir = param.data_dir + SEP + "textures";
	param.common_course_dir = param.data_dir + SEP + "courses";
	param.common_cup_dir = param.data_dir + SEP + "cups";
	param.sounds_dir = param.data_dir + SEP + "sounds";
	param.music_dir = param.data_dir + SEP + "music";
	param.font_dir = param.data_dir + SEP + "fonts";
	param.trans_dir = param.data_dir + SEP + "translations";
	param.configfile = param.config_dir + SEP + "options";

	param.ui_snow = true;
	param.view_mode = 1;
	param.display_fps = false;
	param.show_hud = true;

	if (FileExists (param.configfile.c_str())) {
//		SetConfigDefaults ();
		LoadConfigFile ();
	} else {
		SetConfigDefaults ();
		SaveConfigFile ();
	}
	string playerfile = param.config_dir + SEP + PLAYER_FILE;
	if (FileExists (playerfile.c_str())) {
	} else {
	}
}

// ********************************************************************
//			configuration screen
// ********************************************************************

#define NUM_RES 3
//#define NUM_LANG 1

static TVector2 cursor_pos = {0, 0};
static string resolution[NUM_RES];
//static string languages[NUM_LANG];
static int xleft, ytop;

static bool curr_fullscreen = false;
static int  curr_res = 0;
static int  curr_focus = 0;
static int  curr_mus_vol = 20;
static int  curr_sound_vol = 100;
static int  curr_detail_level = 5;
static int  curr_language = 0;
static bool screenchanged = false;
static bool paramchanged = false;
static TLang *LangList;
static int lastLang = 0;

void SetConfig () {
	if (paramchanged) {	
		if (screenchanged) {
			param.res_type = curr_res;
			param.fullscreen = curr_fullscreen;
			Winsys.SetupVideoMode ();
		}
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
	Winsys.SetMode (g_game.prev_mode);
}

void ChangeRes (int val) {
	curr_res += val;
	if (curr_res < 0) curr_res = 2;
	if (curr_res > 2) curr_res = 0;
	paramchanged = true; 
	screenchanged = true;
}

void ToggleFullscreen () {
	curr_fullscreen = !curr_fullscreen; 
	paramchanged = true; 
	screenchanged = true; 
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

void GameConfigKeys (unsigned int key, bool special, bool release, int x, int y) {
    if (release) return;
	switch (key) {
		case SDLK_q: Winsys.Quit (); break;
		case 27: Winsys.SetMode (g_game.prev_mode); break;
		case SDLK_TAB: if (curr_focus < 7) curr_focus++; else curr_focus = 0; break;
		case 13: 
			switch (curr_focus) {
				case 6: Winsys.SetMode (g_game.prev_mode); break;
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
			case 1: ChangeRes (-1); break;
			case 2: ChangeMusVol (-1); break;
			case 3: ChangeSoundVol (-1); break;
			case 4: ChangeDetail (-1); break;
			case 5: ChangeLanguage (-1); break;
		}
	} else {
		switch (focus) {
			case 1: ChangeRes (1); break;
			case 2: ChangeMusVol (1); break;
			case 3: ChangeSoundVol (1); break;
			case 4: ChangeDetail (1); break;
			case 5: ChangeLanguage (1); break;
		}
	}
}

void GameConfigMouseFunc (int button, int state, int x, int y) {
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
			case 6: Winsys.SetMode (g_game.prev_mode); break;
			case 7: SetConfig (); break;
		}
	}
}

void GameConfigMotionFunc (int x, int y) {

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

// ------------------ Init --------------------------------------------

void GameConfigInit (void) {
	Winsys.ShowCursor (!param.ice_cursor);    
	Winsys.KeyRepeat (true);
	init_ui_snow (); 

	LangList = Trans.languages;
	lastLang = Trans.numLanguages - 1;

	SDL_Surface *surf = 0;
	surf = SDL_GetVideoSurface ();

 	resolution[0] = "auto";
	resolution[1] = "800 x 600";
	resolution[2] = "1024 x 768";

	xleft = (param.x_resolution - 400) / 2;
	ytop = AutoYPos (150);
	ResetWidgets ();
	screenchanged = false;
	paramchanged = false;

	// read the start params:
	curr_res = param.res_type;
	curr_fullscreen = param.fullscreen;
	curr_mus_vol = param.music_volume;
	curr_sound_vol = param.sound_volume;
	curr_detail_level = param.perf_level;
	curr_language = param.language;
	if (curr_language > lastLang) curr_language = lastLang;

	AddCheckbox (xleft, ytop, 0, 400, Trans.Text(31));
	AddArrow (xleft + 400 - 32, ytop+36, 0, 1);
	AddArrow (xleft + 400 - 32, ytop+36+18, 1, 1);
	AddArrow (xleft + 400 - 32, ytop+72, 0, 2);
	AddArrow (xleft + 400 - 32, ytop+72+18, 1, 2);
	AddArrow (xleft + 400 - 32, ytop+108, 0, 3);
	AddArrow (xleft + 400 - 32, ytop+108+18, 1, 3);
	AddArrow (xleft + 400 - 32, ytop+144, 0, 4);
	AddArrow (xleft + 400 - 32, ytop+144+18, 1, 4);
	AddArrow (xleft + 400 - 32, ytop+180, 0, 5);
	AddArrow (xleft + 400 - 32, ytop+180+18, 1, 5);	
	AddTextButton (Trans.Text(28), xleft+70, ytop+320, 6, -1);
	AddTextButton (Trans.Text(15), xleft+300, ytop+320, 7, -1);

	curr_focus = 0;
 	Music.Play ("options1", -1);
}

void GameConfigLoop (double time_step) {
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

	Tex.Draw (T_TITLE_SMALL, -1, 20, 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	if (param.use_papercut_font > 0) FT.SetSize (28); else FT.SetSize (22);
	PrintCheckbox (0, curr_focus, curr_fullscreen);

	if (curr_focus == 1) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop + 36, Trans.Text(32));
	if (curr_focus == 2) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop + 72, Trans.Text(33));
	if (curr_focus == 3) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop + 108, Trans.Text(34));
	if (curr_focus == 4) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop + 144, Trans.Text(36));
	if (curr_focus == 5) FT.SetColor (colDYell); else FT.SetColor (colWhite);
	FT.DrawString (xleft, ytop + 180, Trans.Text(35));

	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (14);
	FT.SetColor (colWhite);
	FT.DrawString (xleft+240, ytop + 40, resolution[curr_res]);
	FT.DrawString (xleft+240, ytop + 76, Int_StrN (curr_mus_vol));
	FT.DrawString (xleft+240, ytop + 112, Int_StrN (curr_sound_vol));
	FT.DrawString (xleft+240, ytop + 148, Int_StrN (curr_detail_level));
	FT.DrawString (xleft+240, ytop + 184, LangList[curr_language].language);

	PrintArrow (0, (curr_res > 0));	
	PrintArrow (1, (curr_res < (NUM_RES-1)));
	PrintArrow (2, (curr_mus_vol > 0));	
	PrintArrow (3, (curr_mus_vol < 120));
	PrintArrow (4, (curr_sound_vol > 0));	
	PrintArrow (5, (curr_sound_vol < 120));
	PrintArrow (6, (curr_detail_level > 1));	
	PrintArrow (7, (curr_detail_level < 3));
	PrintArrow (8, (curr_language > 0));	
 	PrintArrow (9, (curr_language < lastLang));
	
	PrintTextButton (0, curr_focus);
	PrintTextButton (1, curr_focus);

	FT.SetColor (colWhite);
	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (14);
	FT.DrawString (xleft, ytop+240, Trans.Text(41));
 	FT.DrawString (xleft, ytop+262, Trans.Text(42));

	if (param.ice_cursor) DrawCursor ();
	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

void GameConfigTerm (void) {
	Winsys.KeyRepeat (false);
}

void RegisterGameConfig () {
	Winsys.SetModeFuncs (GAME_CONFIG, GameConfigInit, GameConfigLoop, GameConfigTerm,
 		GameConfigKeys, GameConfigMouseFunc, GameConfigMotionFunc, NULL, NULL);
}

