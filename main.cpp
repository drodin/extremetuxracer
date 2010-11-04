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

#include "bh.h"
#include "textures.h"
#include "ogl.h"
#include "splash_screen.h"
#include "intro.h"
#include "racing.h"
#include "game_over.h"
#include "paused.h"
#include "reset.h"
#include "audio.h"
#include "game_type_select.h"
#include "race_select.h"
#include "event_select.h"
#include "credits.h"
#include "loading.h"
#include "course.h"
#include "event.h"
#include "font.h"
#include "translation.h"
#include "help.h"
#include "tools.h"
#include "tux.h"

TGameData g_game;

void InitGame (int argc, char **argv) {
	g_game.toolmode = NONE;

	if (argv[1] != NULL) g_game.argument = Str_IntN (argv[1], 0);
		else g_game.argument = 0;

//	if (argv[1] != NULL) PrintStr (argv[1]);
//	if (argv[2] != NULL) PrintStr (argv[2]);

	g_game.secs_since_start = 0;
	g_game.course_id = 0;
	g_game.mirror_id = 0;
	g_game.char_id = 0;
	g_game.location_id = 0;
	g_game.light_id = 0;
	g_game.snow_id = 0;
	g_game.cup_id = 0;
	g_game.race_id = 0;
	g_game.theme_id = 0;
	g_game.force_treemap = 0;
	g_game.treesize = 3;
	g_game.treevar = 3;
}

// ====================================================================
// 					main
// ====================================================================

int main( int argc, char **argv ) {
	// ****************************************************************
	printf ("\n----------- Extreme Tux Racer " VERSION " ----------------");
    printf ("\n----------- (C) 2010 Extreme Tuxracer Team  --------\n\n ");

	srand (time (NULL));
	InitConfig (argv[0]);
	InitGame (argc, argv);
	Winsys.Init (&argc, argv);
    InitOpenglExtensions ();

	// for checking the joystick and the OpgenGL version (the info is
	// written on the console):
	//	Winsys.PrintJoystickInfo ();
	//	PrintGLInfo ();

//	string uu = "hhh";
//	if (uu == "hhh") PrintStr ("+++"); else PrintStr ("---");
//	return 0;

	// register loop functions
    splash_screen_register();
    intro_register();
    racing_register();
    game_over_register();
    paused_register();
    reset_register();
    game_type_select_register();
	RegisterGameConfig ();
    event_select_register();
	event_register ();
    RaceSelectRegister();
    credits_register();
    loading_register();
	RegisterKeyInfo ();
	RegisterToolFuncs ();

	// theses resources must or should be loaded before splashscreen starts
 	Course.MakeStandardPolyhedrons ();
	Tex.LoadTextureList ();
	FT.LoadFontlist ();
	Winsys.SetFonttype ();
	Audio.Open ();
	Sound.LoadSoundList ();
	Music.LoadMusicList ();
	Music.SetVolume (param.music_volume);

	g_game.mode = NO_MODE;

	switch (g_game.argument) {
		case 0: Winsys.SetMode (SPLASH); break;
		case 2: 
			if (TestChar.Load (param.char_dir, "test.lst", true) == false) Winsys.Quit();
			g_game.toolmode = TUXSHAPE; 
			Winsys.SetMode (TOOLS); 
			break;
	}

 	Winsys.EventLoop ();
    return 0;
} 

