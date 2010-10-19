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

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "bh.h"

#define CONFIG_DIR ".etr"
#define CONFIG_FILE "options"
#define PLAYER_FILE "players"

using namespace std;

typedef struct {
	// defined at runtime:
	string	prog_dir;
	string	config_dir;
    string	data_dir;
    string	common_course_dir;
	string	common_cup_dir;
    string	obj_dir;
    string	terr_dir;
    string	char_dir;
	string  env_dir2;
    string	tex_dir;
	string	sounds_dir;
	string  music_dir;
	string	screenshot_dir;
	string	font_dir;
	string  keyframe_dir;
	string  trans_dir;
	string  configfile;
	int 	x_resolution;
	int 	y_resolution;

	// ------------------------------------
	// main config params:
	bool	fullscreen;  
	int		res_type;
	int		perf_level;
	int		language;
    int		sound_volume;
    int		music_volume;

    int		forward_clip_distance;
    int		backward_clip_distance;
    int		fov;
    int		bpp_mode;
    int		tree_detail_distance;
    int		tux_sphere_divisions;
    int		tux_shadow_sphere_divisions;
    int		course_detail_level; // only for quadtree
	int		audio_freq;
	int		audio_buffer_size;

	int		use_papercut_font;
	bool	ice_cursor;
	bool	full_skybox;

	// toggle params, not saved in options file
    bool	ui_snow;
    bool	display_fps;
	bool	show_hud;
    int		view_mode;
	bool	force_music_loop;
} TParam;

void InitConfig (char *arg0);

extern TParam param;

// ********************************************************************
//			configuration screen
// ********************************************************************

void RegisterGameConfig ();

#endif 

