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

#include "ogl_test.h"
#include "ogl.h"
#include "font.h"
#include "env.h"
#include "gui.h"
#include "winsys.h"

COglTest OglTest;

static const TLight light = {
	true,
	{0.45, 0.53, 0.75, 1.0},
	{1.0, 0.9, 1.0, 1.0},
	{0.6, 0.6, 0.6, 1.0},
	{1, 2, 2, 0.0}
};


void SetTestLight () {
	light.Enable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
}


void COglTest::Keyb(unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case SDLK_ESCAPE:
			State::manager.RequestQuit();
			break;
	}
}

void COglTest::Enter() {
	Winsys.KeyRepeat (true);
	g_game.loopdelay = 1;
}

void COglTest::Loop(double timestep) {
	check_gl_error();

	// ------------- 3d scenery ---------------------------------------
	ScopedRenderMode rm(TUX);
	ClearRenderContext (colDDBackgr);

	glLoadIdentity ();
	glPushMatrix ();
	SetTestLight ();

	/*
		glTranslatef (xposition, yposition, zposition);
		glRotatef (xrotation, 1, 0, 0);
		glRotatef (yrotation, 0, 1, 0);
		glRotatef (zrotation, 0, 0, 1);
	*/
	glPopMatrix ();

	// --------------- 2d screen --------------------------------------
	SetupGuiDisplay ();
	ScopedRenderMode rm2(TEXFONT);
	FT.SetFont ("bold");
	FT.SetSize (24);
	FT.SetColor (colWhite);
	FT.DrawString (CENTER, 10, "Test screen");
	Reshape (Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers ();
}
