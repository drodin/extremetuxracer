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

#include "tools.h"
#include "tux.h"
#include "ogl.h"
#include "font.h"
#include "textures.h"
#include "keyframe.h"
#include "tool_frame.h"
#include "tool_char.h"
#include "env.h"
#include "winsys.h"

CGluCamera GluCamera;

CCamera::CCamera () {
	xview = 0;
	yview = 0;
	zview = 4;
	vhead = 0;
	vpitch = 0;

	fore = false;
	back = false;
	left = false;
	right = false;
	up = false;
	down = false;
	headleft = false;
	headright = false;
	pitchup = false;
	pitchdown = false;
}

void CCamera::XMove (GLfloat step) {
	zview += (float)sin(-vhead * 3.14 / 180) * step;
	xview += (float)cos(-vhead * 3.14 / 180) * step;
}

void CCamera::YMove (GLfloat step) {
	yview += step;
}

void CCamera::ZMove (GLfloat step) {
	xview += (float)sin (vhead * 3.14 / 180) * step;
	zview += (float)cos (vhead * 3.14 / 180) * step;
}

void CCamera::RotateHead (GLfloat step) {
	vhead += step;
}

void CCamera::RotatePitch (GLfloat step) {
	vpitch += step;
}

void CCamera::Update (float timestep) {
	if (fore)		ZMove (-2 * timestep);
	if (back)		ZMove (2 * timestep);
	if (left)		XMove (-1 * timestep);
	if (right)		XMove (1 * timestep);
	if (up)			YMove (1 * timestep);
	if (down)		YMove (-1 * timestep);
	if (headleft)	RotateHead (5 * timestep);
	if (headright)	RotateHead (-5 * timestep);
	if (pitchup)	RotatePitch (-2 * timestep);
	if (pitchdown)	RotatePitch (2 * timestep);

	glLoadIdentity ();
	glRotatef (-vpitch, 1.0, 0.0 , 0.0);
	glRotatef (-vhead, 0.0, 1.0 , 0.0);
	glTranslatef (-xview, -yview, -zview);
}


CGluCamera::CGluCamera () {
	angle = 0.0;
	distance = 3.0;
	turnright = false;
	turnleft = false;
	nearer = false;
	farther = false;
}

void CGluCamera::Update (double timestep) {
	if (turnright) angle += timestep * 2000;
	if (turnleft) angle -= timestep * 2000;
	if (nearer) distance -= timestep * 100;
	if (farther) distance += timestep * 100;
	double xx = distance * sin (angle * M_PI / 180);
	double zz = distance * sin ((90 - angle) * M_PI / 180);
	glLoadIdentity ();
	gluLookAt (xx, 0, zz, 0, 0, 0, 0, 1, 0);
}

// --------------------------------------------------------------------
//				tools
// --------------------------------------------------------------------

CTools Tools;

static bool finalstage = false;
static bool charchanged = false;
static bool framechanged = false;
static string char_dir;
static string char_file;
static string frame_file;

static float tdef_amb[]  = {0.45, 0.53, 0.75, 1.0};
static float tdef_diff[] = {1.0, 0.9, 1.0, 1.0};
static float tdef_spec[] = {0.6, 0.6, 0.6, 1.0};
static float tdef_pos[]  = {1, 2, 2, 0.0};
static TLight toollight;
static int tool_mode = 0;

void DrawQuad (float x, float y, float w, float h,
		float scrheight, const TColor& col, int frame) {
	glDisable (GL_TEXTURE_2D);
    glColor4f (col.r, col.g, col.b, col.a);
	glBegin (GL_QUADS);
		glVertex2f (x-frame,   scrheight-y-h-frame);
		glVertex2f (x+w+frame, scrheight-y-h-frame);
		glVertex2f (x+w+frame, scrheight-y+frame);
		glVertex2f (x-frame,   scrheight-y+frame);
	glEnd();
	glEnable (GL_TEXTURE_2D);
}

void DrawChanged () {
	DrawQuad (Winsys.resolution.width - 120, 10, 100, 22, Winsys.resolution.height, colRed, 0);
	FT.SetFont ("normal");
	FT.SetSize (18);
	FT.SetColor (colBlack);
	FT.DrawString (Winsys.resolution.width - 110, 8, "changed");
}

void InitToolLight () {
	toollight.is_on = true;
	for (int i=0; i<4; i++) {
		toollight.ambient[i]  = tdef_amb[i];
		toollight.diffuse[i]  = tdef_diff[i];
		toollight.specular[i] = tdef_spec[i];
		toollight.position[i] = tdef_pos[i];
	}
}

void SetToolLight () {
	glLightfv (GL_LIGHT0, GL_POSITION, toollight.position);
	glLightfv (GL_LIGHT0, GL_AMBIENT, toollight.ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, toollight.diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, toollight.specular);
	glEnable  (GL_LIGHT0);
	glEnable  (GL_LIGHTING);
}

void QuitTool () {
	if (!charchanged && !framechanged) State::manager.RequestQuit();
	else finalstage = true;
}

void SetToolMode (int newmode) {
	if (newmode == tool_mode) return;
	if (newmode > 2) tool_mode = 0; else tool_mode = newmode;
	switch (tool_mode) {
		case 0: break;
		case 1: break;
		case 2: break;
	}
}

bool CharHasChanged () {return charchanged;}
bool FrameHasChanged () {return framechanged;}

bool ToolsFinalStage () {
	return finalstage;
}

void SetCharChanged (bool val) {
	charchanged = val;
}

void SetFrameChanged (bool val) {
	framechanged = val;
}

void SaveToolCharacter () {
	if (!charchanged) return;
	TestChar.SaveCharNodes (char_dir, char_file);
	charchanged = false;
}

void ReloadToolCharacter () {
	TestChar.Load (char_dir, char_file, true);
	charchanged = false;
}

void SaveToolFrame () {
	if (!framechanged) return;
	TestFrame.SaveTest (char_dir, frame_file);
	framechanged = false;
}

void CTools::Enter() {
	char_dir = param.char_dir + SEP + g_game.dir_arg;
	char_file = "shape.lst";
	frame_file = g_game.file_arg;

	if (TestChar.Load (char_dir, char_file, true) == false) {
		Message ("could not load 'shape.lst'");
		Winsys.Terminate();
	}
	if (TestFrame.Load (char_dir, frame_file) == false) {
		Message ("could not load 'frame.lst'");
		Winsys.Terminate();
	}
	charchanged = false;
	framechanged = false;

	InitToolLight ();
	InitCharTools ();
	InitFrameTools ();

	Winsys.KeyRepeat (true);
	g_game.loopdelay = 1;
}

void CTools::Keyb(unsigned int key, bool special, bool release, int x, int y) {
		switch (tool_mode) {
			case 0: CharKeys (key, special, release, x, y); break;
			case 1: SingleFrameKeys (key, special, release, x, y); break;
			case 2: SequenceKeys (key, special, release, x, y); break;
		}
}

void CTools::Mouse(int button, int state, int x, int y) {
	switch (tool_mode) {
		case 0: CharMouse (button, state, x, y); break;
		case 1: SingleFrameMouse (button, state, x, y); break;
		case 2: SequenceMouse (button, state, x, y); break;
	}
}

void CTools::Motion(int x, int y) {
	switch (tool_mode) {
		case 0: CharMotion (x, y); break;
		case 1: SingleFrameMotion (x, y); break;
		case 2: SequenceMotion (x, y); break;
	}
}

void CTools::Loop(double timestep) {
	switch (tool_mode) {
		case 0: RenderChar (timestep); break;
		case 1: RenderSingleFrame (timestep); break;
		case 2: RenderSequence (timestep); break;
	}
}
