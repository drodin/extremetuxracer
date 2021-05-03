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

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "bh.h"

struct TParam {
	// defined at runtime:
	std::string config_dir;
	std::string data_dir;
	std::string save_dir;
	std::string common_course_dir;
	std::string obj_dir;
	std::string terr_dir;
	std::string char_dir;
	std::string env_dir2;
	std::string tex_dir;
	std::string sounds_dir;
	std::string music_dir;
	std::string screenshot_dir;
	std::string font_dir;
	std::string trans_dir;
	std::string player_dir;
	std::string configfile;

	// ------------------------------------
	// main config params:
	std::size_t	res_type;
	uint32_t	framerate;
	int			perf_level;
	std::size_t	language;
	int			sound_volume;
	int			music_volume;

	int		forward_clip_distance;
	int		backward_clip_distance;
	int		fov;
	int		bpp_mode;
	int		tree_detail_distance;
	int		tux_sphere_divisions;
	int		tux_shadow_sphere_divisions;
	int		course_detail_level; // only for quadtree

	int		use_papercut_font;
	bool	ice_cursor;
	bool	full_skybox;
	bool	use_quad_scale;			// scaling type for menus
	bool	fullscreen;

	std::string	menu_music;
	std::string	credits_music;
	std::string	config_music;

	// these params are not saved in options file
	TViewMode view_mode;
	bool	ui_snow;
	bool	display_fps;
	bool	show_hud;
};

void InitConfig();
void SaveConfigFile();

extern TParam param;

#endif
