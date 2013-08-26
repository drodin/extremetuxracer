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

#ifndef TOOLS_H
#define TOOLS_H

#include "bh.h"
#include "states.h"

class CCamera {
private:
	GLfloat xview;  // x-Position der Kamera
	GLfloat yview;  // Y-Position der Kamera
	GLfloat zview;  // z-Position der Kamera
	GLfloat vhead;  // heading - Rundumsicht
	GLfloat vpitch; // pitch - Drehung nach oben/unten

	void XMove (GLfloat step);
	void YMove (GLfloat step);
	void ZMove (GLfloat step);
	void RotateHead (GLfloat step);
	void RotatePitch (GLfloat step);
public:
	CCamera ();

	bool fore;
	bool back;
	bool left;
	bool right;
	bool up;
	bool down;
	bool headleft;
	bool headright;
	bool pitchup;
	bool pitchdown;
	void Update (float timestep);
};

// ---------------------------------------------------------------
// CGluCamera works with gluLookAt but is reduced to a simple
// go-around-camera that ist strictly focused on an object in
// identity position (0,0,0).
class CGluCamera {
private:
public:
	CGluCamera ();
	double distance;
	double angle;
	void Update (double timestep);

	bool turnright;
	bool turnleft;
	bool nearer;
	bool farther;
};

extern CGluCamera GluCamera;

// --------------------------------------------------------------------

void SetToolLight ();
void QuitTool ();
void SetToolMode (int newmode);
bool ToolsFinalStage ();
void SetCharChanged (bool val);
void SetFrameChanged (bool val);
bool CharHasChanged ();
bool FrameHasChanged ();
void SaveToolCharacter ();
void SaveToolFrame ();
void ReloadToolCharacter ();
void DrawChanged ();


class CTools : public State {
	void Loop(double time_step);
	void Keyb(unsigned int key, bool special, bool release, int x, int y);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
	void Enter();
};

extern CTools Tools;


#endif
