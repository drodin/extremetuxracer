/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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
	{0.45f, 0.53f, 0.75f, 1.f},
	{1.f,   0.9f,  1.f,   1.f},
	{0.6f,  0.6f,  0.6f,  1.f},
	{1.f,   2.f,   2.f,   0.f},
	true
};


void SetTestLight() {
	light.Enable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
}


void COglTest::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	if (key == sf::Keyboard::Escape) {
		State::manager.RequestQuit();
	}
}

void COglTest::Loop(float time_step) {
	// ------------- 3d scenery ---------------------------------------
	ScopedRenderMode rm(TUX);
	ClearRenderContext(colDDBackgr);

	glLoadIdentity();
	glPushMatrix();
	SetTestLight();

	/*
		glTranslatef (xposition, yposition, zposition);
		glRotatef (xrotation, 1, 0, 0);
		glRotatef (yrotation, 0, 1, 0);
		glRotatef (zrotation, 0, 0, 1);
	*/
	glPopMatrix();

	// --------------- 2d screen --------------------------------------
	Setup2dScene();
	ScopedRenderMode rm2(TEXFONT);
	FT.SetProps("bold", 24, colWhite);
	FT.DrawString(CENTER, 10, "Test screen");
	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
}
