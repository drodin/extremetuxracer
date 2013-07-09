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

#include "help.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "font.h"
#include "gui.h"
#include "translation.h"
#include "game_type_select.h"
#include "winsys.h"

CHelp Help;

static int xleft1, xleft2, ytop;

void CHelp::Keyb(unsigned int key, bool special, bool release, int x, int y) {
	State::manager.RequestEnterState (GameTypeSelect);
}

void CHelp::Mouse(int button, int state, int x, int y) {
	if (state == 1) State::manager.RequestEnterState (GameTypeSelect);
}

void CHelp::Motion(int x, int y) {
	if (param.ui_snow) push_ui_snow (cursor_pos);
}

void CHelp::Enter() {
	Winsys.ShowCursor (false);
	Music.Play (param.credits_music, -1);

	xleft1 = 40;
	xleft2 = (int)(Winsys.resolution.width / 2) + 20;
	ytop = AutoYPosN (15);
}

void CHelp::Loop(double timestep) {
	Music.Update ();
	check_gl_error();
    ClearRenderContext ();
	ScopedRenderMode rm(GUI);
    SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow (timestep);
		draw_ui_snow();
    }

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

	FT.DrawString (CENTER, AutoYPosN (90), Trans.Text(65));
    Winsys.SwapBuffers();
}

void CHelp::Exit() {
	Music.Halt();
}
