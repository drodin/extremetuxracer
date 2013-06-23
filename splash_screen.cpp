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
#include "score.h"
#include "regist.h"
#include "winsys.h"

CSplashScreen SplashScreen;

void CSplashScreen::Keyb(unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case SDLK_ESCAPE: State::manager.RequestQuit(); break;
		case SDLK_RETURN: State::manager.RequestEnterState (Regist); break;
	}
}


void CSplashScreen::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);
	init_ui_snow ();
	Music.Play (param.menu_music, -1);
	g_game.loopdelay = 10;
}

void CSplashScreen::Loop(double timestep) {
	Music.Update ();
	check_gl_error();
    ClearRenderContext ();
	ScopedRenderMode rm(GUI);
    SetupGuiDisplay ();
	Trans.LoadLanguages ();
	Trans.LoadTranslations (param.language); // Before first texts are being displayed

//	FT.SetFont ("normal");
	Tex.Draw (TEXLOGO, CENTER, 60, Winsys.scale);
	FT.SetColor (colDYell);
	FT.AutoSizeN (6);
	int top = AutoYPosN (60);
	int dist = FT.AutoDistanceN (3);
	FT.DrawString (CENTER, top, Trans.Text(67));
	FT.DrawString (CENTER, top+dist, Trans.Text(68));

    Winsys.SwapBuffers();
	Course.MakeStandardPolyhedrons ();
	Sound.LoadSoundList ();
	Credits.LoadCreditList ();
	Char.LoadCharacterList ();
	Course.LoadObjectTypes ();
	Course.LoadTerrainTypes ();
	Env.LoadEnvironmentList ();
	Course.LoadCourseList ();
	Score.LoadHighScore (); // after LoadCourseList !!!
	Events.LoadEventList ();
	Players.LoadAvatars (); // before LoadPlayers !!!
	Players.LoadPlayers ();

	State::manager.RequestEnterState (Regist);
}
