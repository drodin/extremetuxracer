/* --------------------------------------------------------------------
EXTREME TUXRACER

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

/*
If you want to add a new option, do this:
First add the option to the TParam struct (game_config.h).

Then edit the below functions:

- LoadConfigFile. Use
	SPIntN for integer and boolean values
	SPStrN for strings.
	The first value is always 'line', the second defines the tag within the
	brackets [ ], and the last value is the default.

- SetConfigDefaults. These values are used as long as no options.txt or options file exists.
	It's a good idea to use the same values as the defaults in LoadConfigFile.

- SaveConfigFile. See the other entries; it should be self-explanatory.
	If an options.txt or options file exists, you will have to change any value at runtime
	on the configuration screen to overwrite the file. Then you will see the
	new entry.
*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "game_config.h"
#include "spx.h"
#include "translation.h"
#include <sstream>
#include <sys/stat.h>

TParam param;


void LoadConfigFile() {
	CSPList list;
	if (!list.Load(param.configfile)) {
		Message("Could not load 'options.txt'");
		return;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		param.fullscreen = SPBoolN(*line, "fullscreen", false);
		param.res_type = SPIntN(*line, "res_type", 0);
		param.perf_level = SPIntN(*line, "detail_level", 3);
		param.language = Trans.GetLangIdx(SPStrN(*line, "language", "EN_en"));
		param.sound_volume = SPIntN(*line, "sound_volume", 90);
		param.music_volume = SPIntN(*line, "music_volume", 20);

		param.framerate = SPIntN(*line, "framerate", 60);

		param.forward_clip_distance = SPIntN(*line, "forward_clip_distance", 75);
		param.backward_clip_distance = SPIntN(*line, "backward_clip_distance", 20);
		param.fov = SPIntN(*line, "fov", 60);
		param.bpp_mode = SPIntN(*line, "bpp_mode", 0);
		param.tree_detail_distance = SPIntN(*line, "tree_detail_distance", 20);
		param.tux_sphere_divisions = SPIntN(*line, "tux_sphere_divisions", 10);
		param.tux_shadow_sphere_divisions = SPIntN(*line, "tux_shadow_sphere_div", 3);
		param.course_detail_level = SPIntN(*line, "course_detail_level", 75);

		param.use_papercut_font = SPIntN(*line, "use_papercut_font", 1);
		param.ice_cursor = SPIntN(*line, "ice_cursor", 1) != 0;
		param.full_skybox = SPBoolN(*line, "full_skybox", false);
		param.use_quad_scale = SPBoolN(*line, "use_quad_scale", false);

		param.menu_music = SPStrN(*line, "menu_music", "start_1");
		param.credits_music = SPStrN(*line, "credits_music", "credits_1");
		param.config_music = SPStrN(*line, "config_music", "options_1");
	}
}

void SetConfigDefaults() {
	param.fullscreen = true;
	param.res_type = 0; // 0=auto / 1=800x600 / 2=1024x768 ...
	param.perf_level = 3;	// detail level
	param.language = std::string::npos; // If language is set to npos, ETR will try to load default system language
	param.sound_volume = 90;
	param.music_volume = 20;

	// ---------------------------------------

	param.framerate = 60;

	param.forward_clip_distance = 75;
	param.backward_clip_distance = 20;
	param.fov = 60;
	param.bpp_mode = 0;
	param.tree_detail_distance = 20;
	param.tux_sphere_divisions = 10;
	param.tux_shadow_sphere_divisions = 3;
	param.course_detail_level = 75;

	param.use_papercut_font = 1;
	param.ice_cursor = true;
	param.full_skybox = false;
	param.use_quad_scale = false;

	param.menu_music = "start_1";
	param.credits_music = "credits_1";
	param.config_music = "options_1";
}


template<typename T>
void AddItem(CSPList &list, const std::string& tag, T content) {
	std::ostringstream os;
	os << "  [" << tag << "] " << content;
	list.Add(os.str());
}

void AddComment(CSPList &list, const std::string& comment) {
	std::string line = "# " + comment;
	list.Add(line);
}

void SaveConfigFile() {
	CSPList liste;

	liste.Add("# ------------------------------------------------------------------");
	liste.Add("#   The first group of params can be adjusted ");
	liste.Add("#   on the configuration screen, too");
	liste.Add("# ------------------------------------------------------------------");
	liste.Add();

	AddComment(liste, "Full-screen mode [0...1]");
	AddItem(liste, "fullscreen", param.fullscreen);
	liste.Add();

	AddComment(liste, "Screen resolution [0...9]");
	AddComment(liste, "0 = auto, 1 = 800x600, 2 = 1024x768");
	AddComment(liste, "3 = 1152x864, 4 = 1280x960, 5 = 1280x1024");
	AddComment(liste, "6 = 1360x768, 7 = 1400x1050, 8 = 1440x900, 9=1680x1050");
	AddItem(liste, "res_type", param.res_type);
	liste.Add();

	AddComment(liste, "Level of details [1...4]");
	AddComment(liste, "1 = best performance, 4 = best appearance");
	AddItem(liste, "detail_level", param.perf_level);
	liste.Add();

	AddComment(liste, "Language code");
	AddComment(liste, "en_GB = English etc.");
	AddItem(liste, "language", Trans.languages[param.language].lang);
	liste.Add();

	AddComment(liste, "Sound volume [0...100]");
	AddComment(liste, "Sounds are the terrain effects or the pickup noise.");
	AddItem(liste, "sound_volume", param.sound_volume);
	liste.Add();

	AddComment(liste, "Volume of the background music [0...100]");
	AddItem(liste, "music_volume", param.music_volume);
	liste.Add();

	liste.Add("# ------------------------------------------------------------------");
	liste.Add("#   The second group of params must be adjusted in this file.");
	liste.Add("# ------------------------------------------------------------------");
	liste.Add();

	AddComment(liste, "Framerate limit");
	AddComment(liste, "0 = unlimited, default: 60");
	AddItem(liste, "framerate", param.framerate);
	liste.Add();

	AddComment(liste, "Forward clipping distance");
	AddComment(liste, "Controls how far ahead of the camera the course");
	AddComment(liste, "is rendered. Larger values mean that more of the course is");
	AddComment(liste, "rendered, resulting in slower performance. Decreasing this ");
	AddComment(liste, "value is an effective way to improve framerates.");
	AddItem(liste, "forward_clip_distance", param.forward_clip_distance);
	liste.Add();

	AddComment(liste, "Backward clipping distance");
	AddComment(liste, "Some objects aren't yet clipped to the view frustum, ");
	AddComment(liste, "so this value is used to control how far up the course these ");
	AddComment(liste, "objects are drawn.");
	AddItem(liste, "backward_clip_distance", param.backward_clip_distance);
	liste.Add();

	AddComment(liste, "Field of View of the camera");
	AddItem(liste, "fov", param.fov);
	liste.Add();

	AddComment(liste, "Bpp mode - bits per pixel");
	AddComment(liste, "Controls the color depth of the OpenGL window");
	AddComment(liste, "16 = 16 bpp, 32 = 32 bpp,");
	AddComment(liste, "0 (or any other) = use current bpp setting of operating system,");
	AddItem(liste, "bpp_mode", param.bpp_mode);
	liste.Add();

	AddComment(liste, "Tree detail distance");
	AddComment(liste, "Controls how far up the course the trees are drawn crosswise.");
	AddItem(liste, "tree_detail_distance", param.tree_detail_distance);
	liste.Add();

	AddComment(liste, "Tux sphere divisions");
	AddComment(liste, "Controls how detailed the character is drawn");
	AddItem(liste, "tux_sphere_divisions", param.tux_sphere_divisions);
	liste.Add();

	AddComment(liste, "Tux shadow sphere divisions");
	AddComment(liste, "The same but for the shadow of the character");
	AddItem(liste, "tux_shadow_sphere_div", param.tux_shadow_sphere_divisions);
	liste.Add();

	AddComment(liste, "Detail level of the course");
	AddComment(liste, "This param is used for the quadtree and controls the");
	AddComment(liste, "LOD of the algorithm. ");
	AddItem(liste, "course_detail_level", param.course_detail_level);
	liste.Add();

	AddComment(liste, "Font type [0...2]");
	AddComment(liste, "0 = always arial-like font,");
	AddComment(liste, "1 = papercut font on the menu screens");
	AddComment(liste, "2 = papercut font for the hud display, too");
	AddItem(liste, "use_papercut_font", param.use_papercut_font);
	liste.Add();

	AddComment(liste, "Cursor type [0...1]");
	AddComment(liste, "0 = normal cursor (arrow), 1 = icicle");
	AddItem(liste, "ice_cursor", (int)param.ice_cursor);
	liste.Add();

	AddComment(liste, "Draw full skybox [0...1]");
	AddComment(liste, "A normal skybox consists of 6 textures. In Tuxracer");
	AddComment(liste, "3 textures are invisible (top, bottom and back).");
	AddComment(liste, "These textures needn't be drawn.");
	AddItem(liste, "full_skybox", param.full_skybox);
	liste.Add();

	AddComment(liste, "Select the music:");
	AddComment(liste, "(the racing music is defined by a music theme)");
	AddItem(liste, "menu_music", param.menu_music);
	AddItem(liste, "credits_music", param.credits_music);
	AddItem(liste, "config_music", param.config_music);
	liste.Add();

	AddComment(liste, "Use sqare root of scale factors for menu screens [0...1]");
	AddComment(liste, "Experimental: these factors reduce the effect of screen scaling.");
	AddComment(liste, "The widgets are closer to their default sizes.");
	AddItem(liste, "use_quad_scale", param.use_quad_scale);
	liste.Add();

	// ---------------------------------------
	liste.Save(param.config_dir + SEP "options.txt");
}

// --------------------------------------------------------------------

void InitConfig() {
	int config_exist = 0;

#if defined (OS_WIN32_MINGW) || defined (OS_WIN32_MSC)
	// the progdir is always the current dir
	param.config_dir = "config";
	param.data_dir = "data";
	param.save_dir = "data";
	param.configfile = param.config_dir + SEP "options.txt";
	if (FileExists(param.configfile)) {
		config_exist = 1;
	}
#else /* WIN32 */

#if 0
	char buff[256];

	if (std::strcmp(arg0, "./etr") == 0) {		// start from work directory
		char *s = getcwd(buff, 256);
		if (s==nullptr) {};
	} else {								// start with full path
		std::strcpy(buff, arg0);
		if (std::strlen(buff) > 5) {
			buff[std::strlen(buff)-3] = 0;
		}
	}

	param.prog_dir = buff;
#endif /* 0 */

	struct passwd *pwent = getpwuid(getuid());

	param.config_dir = pwent->pw_dir;
	param.config_dir += SEP;
	param.config_dir += ".etr";
	// or: param.config_dir = param.prog_dir + SEP "config";
	if (!DirExists(param.config_dir.c_str())) {
		mkdir(param.config_dir.c_str(), 0775);
	}
	param.data_dir = ETR_DATA_DIR;
	param.data_dir += SEP;
	param.data_dir += "etr";
	param.save_dir = param.config_dir;
	param.configfile = param.config_dir + SEP "options.txt";
	if (FileExists(param.configfile)) {
		config_exist = 1;
	} else {
		// Old name as fallback
		param.configfile = param.config_dir + SEP "options";
		if (FileExists(param.configfile)) {
			config_exist = 2;
		}
	}
#endif /* WIN32 */

	param.screenshot_dir = param.save_dir + SEP "screenshots";
	param.obj_dir = param.data_dir + SEP "objects";
	param.env_dir2 = param.data_dir + SEP "env";
	param.char_dir = param.data_dir + SEP "char";
	param.terr_dir = param.data_dir + SEP "terrains";
	param.tex_dir = param.data_dir + SEP "textures";
	param.common_course_dir = param.data_dir + SEP "courses";
	param.sounds_dir = param.data_dir + SEP "sounds";
	param.music_dir = param.data_dir + SEP "music";
	param.font_dir = param.data_dir + SEP "fonts";
	param.trans_dir = param.data_dir + SEP "translations";
	param.player_dir = param.data_dir + SEP "players";

	param.ui_snow = true;
	param.view_mode = FOLLOW;
	param.display_fps = false;
	param.show_hud = true;

	if (config_exist > 0) {
		Trans.LoadLanguages();
		LoadConfigFile();
	} else {
		SetConfigDefaults();
		Trans.LoadLanguages();
	}
}
