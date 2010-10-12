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
				  
void HelpKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (key == 27) Winsys.SetMode (GAME_TYPE_SELECT);
}

void HelpMouseFunc (int button, int state, int x, int y) {
	if (state == 1) Winsys.SetMode (GAME_TYPE_SELECT);
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
	Music.Play ("credits2", -1);

	xleft1 = 20; 
	xleft2 = (int)(param.x_resolution / 2) + 20;
	ytop = 30;
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

	double offs = AutoFtSize (25);
	FT.SetSize (AutoFtSize (28));
	FT.SetColor (colWhite);
	FT.DrawString (xleft1, ytop, Trans.Text (57));
	FT.SetSize (AutoFtSize (22));
	FT.DrawString (xleft1+20, ytop + 50, Trans.Text(44));
	FT.DrawString (xleft1+20, ytop + 50 + offs, Trans.Text(45));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 2, Trans.Text(46));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 3, Trans.Text(47));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 4, Trans.Text(48));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 5, Trans.Text(49));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 6, Trans.Text(50));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 7, Trans.Text(51));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 8,Trans.Text(52));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 9, Trans.Text(53));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 10, Trans.Text(54));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 11, Trans.Text(55));
	FT.DrawString (xleft1+20, ytop + 50 + offs * 12, Trans.Text(56));
		

    Winsys.SwapBuffers();

} 

void HelpTerm () {}

void RegisterKeyInfo () {
	Winsys.SetModeFuncs (HELP, HelpInit, HelpLoop, HelpTerm,
 		HelpKeys, HelpMouseFunc, HelpMotionFunc, NULL, NULL);
}
