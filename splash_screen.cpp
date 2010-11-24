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

#include "splash_screen.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "course.h"
#include "tux.h"
#include "env.h"
#include "particles.h"
#include "credits.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"

static int xleft, ytop;

void SplashInit (void) {  
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);

	xleft = (param.x_resolution - 500) / 2;
	ytop = AutoYPos (230);
	g_game.loopdelay = 10;
}

void SplashLoop (double timestep ){
	int top;
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();

	Tex.Draw (TEXLOGO, -1, 60, 1);
	FT.SetColor (colDYell);
	FT.SetSize (AutoFtSize ());
	top = AutoYPos (350);
	FT.DrawText (CENTER, top, "Loading resources,");
	FT.DrawText (CENTER, top + AutoDistance(), "please wait ...");

	if (param.ice_cursor) DrawCursor ();
    Winsys.SwapBuffers();


	Trans.LoadLanguages ();
	Trans.LoadTranslations (param.language);
	LoadCreditList ();
	Char.LoadCharacterList ();
	Course.LoadObjectTypes (); 
	Course.LoadTerrainTypes ();
	Env.LoadEnvironmentList ();
	Course.LoadCourseList ();
	Events.LoadEventList ();
	Players.LoadAvatars (); // before LoadPlayers !!!
	Players.LoadPlayers ();

	SDL_Delay (100);
	Winsys.SetMode (REGIST);
} 

void SplashTerm () {
}

void splash_screen_register() {
	Winsys.SetModeFuncs (SPLASH, SplashInit, SplashLoop, SplashTerm,
	NULL, NULL, NULL, NULL, NULL, NULL);
}
