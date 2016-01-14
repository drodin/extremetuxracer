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

#include "bh.h"
#include "textures.h"
#include "ogl.h"
#include "splash_screen.h"
#include "audio.h"
#include "font.h"
#include "tools.h"
#include "ogl_test.h"
#include "winsys.h"
#include <iostream>
#include <ctime>
#include <cstring>

TGameData g_game;

void InitGame(int argc, char **argv) {
	g_game.toolmode = NONE;
	g_game.argument = 0;
	if (argc == 4) {
		if (std::strcmp("--char", argv[1]) == 0)
			g_game.argument = 4;
		Tools.SetParameter(argv[2], argv[3]);
	} else if (argc == 2) {
		if (std::strcmp(argv[1], "9") == 0)
			g_game.argument = 9;
	}

	g_game.player = nullptr;
	g_game.start_player = 0;
	g_game.course = nullptr;
	g_game.mirrorred = false;
	g_game.character = nullptr;
	g_game.location_id = 0;
	g_game.light_id = 0;
	g_game.snow_id = 0;
	g_game.cup = 0;
	g_game.theme_id = 0;
	g_game.force_treemap = false;
	g_game.treesize = 3;
	g_game.treevar = 3;
}

int main(int argc, char **argv) {
	cout << "\n----------- Extreme Tux Racer " ETR_VERSION_STRING " ----------------";
	cout << "\n----------- (C) 2010-2016 Extreme Tuxracer Team  --------\n\n";

	srand(time(nullptr));
	InitConfig();
	InitGame(argc, argv);
	Winsys.Init();
	InitOpenglExtensions();

	// For checking the joystick and the OpgenGL version (the info is written on the console):
	//Winsys.PrintJoystickInfo();
	//PrintGLInfo ();

	// theses resources must or should be loaded before splashscreen starts
	if (!Tex.LoadTextureList()) {
		Winsys.Quit();
		return -1;
	}
	FT.LoadFontlist();
	FT.SetFontFromSettings();
	Music.LoadMusicList();
	Music.SetVolume(param.music_volume);

	switch (g_game.argument) {
		case 0:
			State::manager.Run(SplashScreen);
			break;
		case 4:
			g_game.toolmode = TUXSHAPE;
			State::manager.Run(Tools);
			break;
		case 9:
			State::manager.Run(OglTest);
			break;
	}

	Winsys.Quit();

	return 0;
}
