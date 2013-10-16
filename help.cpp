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

#include "help.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "font.h"
#include "gui.h"
#include "translation.h"

static int xleft1, xleft2, ytop;
static TVector2 cursor_pos = {0, 0};

#ifdef __QNX__
const char website[] = "http://extremetuxracer.drodin.com";
#endif
				  
void HelpKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (key == 27) Winsys.SetMode (GAME_TYPE_SELECT);
}

void HelpMouseFunc (int button, int state, int x, int y) {
#ifndef __QNX__
	if (state == 1) Winsys.SetMode (GAME_TYPE_SELECT);
#else
	int foc, dir;
	if (state == 1) {
		GetFocus (x, y, &foc, &dir);
		if (foc == 0) navigator_invoke(website, NULL);
		else Winsys.SetMode (GAME_TYPE_SELECT);
	}
#endif
}

void HelpMotionFunc (int x, int y ){
    TVector2 old_pos;
	if (Winsys.ModePending ()) return; 
    y = param.y_resolution - y;
    old_pos = cursor_pos;
    cursor_pos = MakeVector2 (x, y);
    if  (old_pos.x != x || old_pos.y != y) {
		if (param.ui_snow) push_ui_snow (cursor_pos);
    }
}

void HelpInit (void) {  
	Winsys.ShowCursor (false);
	init_ui_snow (); 
	Music.Play (param.credits_music, -1);

	xleft1 = 40; 
	xleft2 = (int)(param.x_resolution / 2) + 20;
	ytop = AutoYPosN (15);
	ResetWidgets ();
#ifdef __QNX__
	int siz = FT.AutoSizeN (5);
	AddTextButton (website, param.x_resolution/2 - 120, AutoYPosN (85), 0, siz);
#endif
}

void HelpLoop (double timestep ){
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();
		
	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow();
    }

#ifndef __QNX__
	FT.AutoSizeN (4);
	FT.SetColor (colWhite);
	FT.DrawString (xleft1, AutoYPosN (5), Trans.Text (57));

	FT.AutoSizeN (3);
	int offs = FT.AutoDistanceN (2);
	FT.DrawString (xleft1, ytop, Trans.Text(44));
	FT.DrawString (xleft1, ytop + offs, Trans.Text(45));
	FT.DrawString (xleft1, ytop + offs * 2, Trans.Text(46));
	FT.DrawString (xleft1, ytop + offs * 3, Trans.Text(47));
	FT.DrawString (xleft1, ytop + offs * 4, Trans.Text(48));
	FT.DrawString (xleft1, ytop + offs * 5, Trans.Text(49));
	FT.DrawString (xleft1, ytop + offs * 6, Trans.Text(50));
	FT.DrawString (xleft1, ytop + offs * 7, Trans.Text(51));
	FT.DrawString (xleft1, ytop + offs * 8,Trans.Text(52));
	FT.DrawString (xleft1, ytop + offs * 9, Trans.Text(53));
	FT.DrawString (xleft1, ytop + offs * 10, Trans.Text(54));
	FT.DrawString (xleft1, ytop + offs * 11, Trans.Text(55));
	FT.DrawString (xleft1, ytop + offs * 12, Trans.Text(56));

	FT.DrawString (CENTER, AutoYPosN (90), "Press any key to return to the main menu");
#else
	int ww = param.x_resolution;
	int hh = param.y_resolution;

	Tex.Draw (TEXLOGO, CENTER, 20, param.scale);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	FT.SetColor (colWhite);
	FT.AutoSizeN (4);
	int top = AutoYPosN (47);
	int dist = FT.AutoDistanceN (2);
	FT.DrawText (CENTER, top, "Control Tux with your tablet's accelerometer.");
	FT.DrawText (CENTER, top+dist, "Tilt tablet left/right to turn respectively.");
	FT.DrawText (CENTER, top+dist*2, "Lean forward to accelerate, backward to brake.");
	FT.DrawText (CENTER, top+dist*3, "Touch screen and hold to get power, release to jump.");
	FT.DrawText (CENTER, top+dist*4, "Swipe down to abort race or go to previous menu.");
	/*
	FT.SetColor (colDYell);
	FT.AutoSizeN (6);
	FT.DrawText (CENTER, AutoYPosN (85), "Loading resources, please wait ...");
	*/
	FT.AutoSizeN (5);
	FT.DrawText (param.x_resolution/2 - 300, AutoYPosN (85), "More info at:");
	PrintTextButton (0, 0);
#endif
    Winsys.SwapBuffers();

} 

void HelpTerm () {
	Music.Halt();
}

void RegisterKeyInfo () {
	Winsys.SetModeFuncs (HELP, HelpInit, HelpLoop, HelpTerm,
 		HelpKeys, HelpMouseFunc, HelpMotionFunc, NULL, NULL, NULL);
}
