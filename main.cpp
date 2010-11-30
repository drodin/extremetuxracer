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
#include "regist.h"
#include "keyframe.h"
#include "newplayer.h"
#include "score.h"

TGameData g_game;

void InitGame (int argc, char **argv) {
	g_game.toolmode = NONE;
	g_game.argument = 0;
	if (argc == 4) {
		g_game.group_arg = argv[1];
		if (g_game.group_arg == "--char") g_game.argument = 1;
		g_game.dir_arg = argv[2];
		g_game.file_arg = argv[3];
	} 

	g_game.secs_since_start = 0;
	g_game.player_id = 0;
	g_game.start_player = 0;
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
	g_game.loopdelay = 1;
}

// ====================================================================
// 					main
// ====================================================================

#if defined ( OS_WIN32_MINGW )
	#undef main
#endif
int main( int argc, char **argv ) {
	// ****************************************************************
	printf ("\n----------- Extreme Tux Racer " VERSION " ----------------");
    printf ("\n----------- (C) 2010 Extreme Tuxracer Team  --------\n\n ");

	srand (time (NULL));
	InitConfig (argv[0]);
	InitGame (argc, argv);
	Winsys.Init ();
    InitOpenglExtensions ();

	// for checking the joystick and the OpgenGL version (the info is
	// written on the console):
	//	Winsys.PrintJoystickInfo ();
	//	PrintGLInfo ();

	// register loop functions
    splash_screen_register();
	regist_register ();
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
	NewPlayerRegister ();
	RegisterScoreFunctions ();

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
		case 1: 
			g_game.toolmode = TUXSHAPE; 
			Winsys.SetMode (TOOLS); 
			break;
	}

 	Winsys.EventLoop ();
    return 0;
} 

