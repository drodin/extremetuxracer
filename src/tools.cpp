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
#include <GL/glu.h>

CGluCamera GluCamera;

CCamera::CCamera() {
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

void CCamera::XMove(GLfloat step) {
	zview += (float)std::sin(-vhead * 3.14 / 180) * step;
	xview += (float)std::cos(-vhead * 3.14 / 180) * step;
}

void CCamera::YMove(GLfloat step) {
	yview += step;
}

void CCamera::ZMove(GLfloat step) {
	xview += (float)std::sin(vhead * 3.14 / 180) * step;
	zview += (float)std::cos(vhead * 3.14 / 180) * step;
}

void CCamera::RotateHead(GLfloat step) {
	vhead += step;
}

void CCamera::RotatePitch(GLfloat step) {
	vpitch += step;
}

void CCamera::Update(float timestep) {
	if (fore)		ZMove(-2 * timestep);
	if (back)		ZMove(2 * timestep);
	if (left)		XMove(-1 * timestep);
	if (right)		XMove(1 * timestep);
	if (up)			YMove(1 * timestep);
	if (down)		YMove(-1 * timestep);
	if (headleft)	RotateHead(5 * timestep);
	if (headright)	RotateHead(-5 * timestep);
	if (pitchup)	RotatePitch(-2 * timestep);
	if (pitchdown)	RotatePitch(2 * timestep);

	glLoadIdentity();
	glRotatef(-vpitch, 1.0, 0.0, 0.0);
	glRotatef(-vhead, 0.0, 1.0, 0.0);
	glTranslatef(-xview, -yview, -zview);
}


CGluCamera::CGluCamera() {
	angle = 0.0;
	distance = 3.0;
	turnright = false;
	turnleft = false;
	nearer = false;
	farther = false;
}

void CGluCamera::Update(float timestep) {
	if (turnright) angle += timestep * 2000;
	if (turnleft) angle -= timestep * 2000;
	if (nearer) distance -= timestep * 100;
	if (farther) distance += timestep * 100;
	double xx = distance * std::sin(angle * M_PI / 180);
	double zz = distance * std::sin((90 - angle) * M_PI / 180);
	glLoadIdentity();
	gluLookAt(xx, 0, zz, 0, 0, 0, 0, 1, 0);
}

// --------------------------------------------------------------------
//				tools
// --------------------------------------------------------------------

CTools Tools;

static bool finalstage = false;
static bool charchanged = false;
static bool framechanged = false;
static std::string char_dir;
static std::string char_file;
static std::string frame_file;

static const TLight toollight = {
	{0.45f, 0.53f, 0.75f, 1.f},
	{1.f,   0.9f,  1.f,   1.f},
	{0.6f,  0.6f,  0.6f,  1.f},
	{1.f,   2.f,   2.f,   0.f},
	true
};
static int tool_mode = 0;

void DrawQuad(float x, float y, float w, float h, float scrheight, const sf::Color& col, int frame) {
	glDisable(GL_TEXTURE_2D);
	glColor(col);
	const GLfloat vtx[] = {
		x - frame, scrheight - y - h - frame,
		x + w + frame, scrheight - y - h - frame,
		x + w + frame, scrheight - y + frame,
		x - frame, scrheight - y + frame
	};
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
}

void DrawChanged() {
	DrawQuad(Winsys.resolution.width - 120, 10, 100, 22, Winsys.resolution.height, colRed, 0);
	FT.SetProps("normal", 18, colBlack);
	FT.DrawString(Winsys.resolution.width - 110, 8, "changed");
}

void SetToolLight() {
	toollight.Enable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
}

void QuitTool() {
	if (!charchanged && !framechanged) State::manager.RequestQuit();
	else finalstage = true;
}

void SetToolMode(int newmode) {
	if (newmode == tool_mode) return;
	if (newmode > 2) tool_mode = 0;
	else tool_mode = newmode;
	switch (tool_mode) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
	}
}

bool CharHasChanged() {return charchanged;}
bool FrameHasChanged() {return framechanged;}

bool ToolsFinalStage() {
	return finalstage;
}

void SetCharChanged(bool val) {
	charchanged = val;
}

void SetFrameChanged(bool val) {
	framechanged = val;
}

void SaveToolCharacter() {
	if (!charchanged) return;
	TestChar.SaveCharNodes(char_dir, char_file);
	charchanged = false;
}

void ReloadToolCharacter() {
	TestChar.Load(char_dir, char_file, true);
	charchanged = false;
}

void SaveToolFrame() {
	if (!framechanged) return;
	TestFrame.SaveTest(char_dir, frame_file);
	framechanged = false;
}

void CTools::SetParameter(const std::string& dir, const std::string& file) {
	char_dir = param.char_dir + SEP + dir;
	char_file = "shape.lst";
	frame_file = file;
}

void CTools::Enter() {
	if (TestChar.Load(char_dir, char_file, true) == false) {
		Message("could not load 'shape.lst'");
		Winsys.Terminate();
	}
	if (TestFrame.Load(char_dir, frame_file) == false) {
		Message("could not load 'frame.lst'");
		Winsys.Terminate();
	}
	charchanged = false;
	framechanged = false;

	InitCharTools();
	InitFrameTools();
}

void CTools::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	switch (tool_mode) {
		case 0:
			CharKeys(key, release, x, y);
			break;
		case 1:
			SingleFrameKeys(key, release, x, y);
			break;
		case 2:
			SequenceKeys(key, release, x, y);
			break;
	}
}

void CTools::Mouse(int button, int state, int x, int y) {
	switch (tool_mode) {
		case 0:
			CharMouse(button, state, x, y);
			break;
		case 1:
			SingleFrameMouse(button, state, x, y);
			break;
		case 2:
			SequenceMouse(button, state, x, y);
			break;
	}
}

void CTools::Motion(int x, int y) {
	switch (tool_mode) {
		case 0:
			CharMotion(x, y);
			break;
		case 1:
			SingleFrameMotion(x, y);
			break;
		case 2:
			SequenceMotion(x, y);
			break;
	}
}

void CTools::Loop(float time_step) {
	switch (tool_mode) {
		case 0:
			RenderChar(time_step);
			break;
		case 1:
			RenderSingleFrame(time_step);
			break;
		case 2:
			RenderSequence(time_step);
			break;
	}
}
