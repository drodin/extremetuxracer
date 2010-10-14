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

static TVector2 cursor_pos = {0, 0};
static int pass = 0;
				  
void SplashKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (key == 27) Winsys.Quit ();
	else Winsys.SetMode (GAME_TYPE_SELECT);
}

void SplashMouseFunc (int button, int state, int x, int y) {
	if (state == 1) Winsys.SetMode (GAME_TYPE_SELECT);
}

void SplashMotionFunc (int x, int y ){
    TVector2 old_pos;
	if (Winsys.ModePending ()) return; 
    y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}
void SplashInit (void) {  
	Winsys.ShowCursor (false);
	init_ui_snow (); 
	Music.Play ("start1", -1);
	pass = 0;
}

void SplashLoop (double timestep ){
	int ww = param.x_resolution;
	int hh = param.y_resolution;
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();
		
	if (param.ui_snow && pass > 0) {
		update_ui_snow (timestep);
		draw_ui_snow();
    }

	Tex.Draw (TEXLOGO, -1, 60, 1);
	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);

	FT.SetColor (colDYell);
	FT.SetSize (AutoFtSize ());
	int top = AutoYPos (350);
	if (pass == 0) {
		// this must be written in English:
		FT.DrawText (CENTER, top, "loading resources");
		FT.DrawText (CENTER, top + AutoDistance(), "please wait ...");
	} else {
		// this can already displayed in special languages:
		FT.DrawString (CENTER, top + AutoDistance(), Trans.Text (0));
	}

    Winsys.SwapBuffers();

	if (pass == 0) {
		Trans.LoadLanguages ();
		Trans.LoadTranslations (param.language);
		LoadCreditList ();
		LoadTux2 ();		
		Course.LoadObjectTypes (); 
		Course.LoadTerrainTypes ();
		Env.LoadEnvironmentList ();
		Course.LoadCourseList ();
		Events.LoadEventList ();
		Players.LoadParams ();
		pass = 1;
		SDL_Delay (100);
	}
} 

void SplashTerm () {
	Music.Halt ();
}

void splash_screen_register() {
	Winsys.SetModeFuncs (SPLASH, SplashInit, SplashLoop, SplashTerm,
 		SplashKeys, SplashMouseFunc, SplashMotionFunc, NULL, NULL);
}
