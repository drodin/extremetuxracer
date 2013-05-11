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

#ifndef WINSYS_H
#define WINSYS_H

#include "bh.h"

#define NUM_RESOLUTIONS 10

class CWinsys {
private:
	// time
	float lasttick;
	float elapsed_time;
	int remain_ticks;

	// joystick
	SDL_Joystick *joystick;	
	int numJoysticks;
	bool joystick_active;

	// sdl window
 	TScreenRes resolution[NUM_RESOLUTIONS];
	int auto_x_resolution;
	int auto_y_resolution;
	SDL_Surface *screen;
	TScreenRes MakeRes (int width, int height);
	double CalcScreenScale ();
public:
	CWinsys ();

	// sdl window
	TScreenRes GetResolution (size_t idx);
	string GetResName (size_t idx);
	void Init ();
	void SetupVideoMode (TScreenRes resolution);
	void SetupVideoMode (size_t idx);
	void SetupVideoMode (int width, int height);
	void KeyRepeat (bool repeat);
	void SetFonttype ();
	void PrintJoystickInfo ();
	void ShowCursor (bool visible) {SDL_ShowCursor (visible);}
	void SwapBuffers () {SDL_GL_SwapBuffers ();}
	void Quit ();
	void Terminate ();
	void InitJoystick ();
	void CloseJoystick ();
	bool joystick_isActive() const { return joystick_active; }
	double ClockTime () {return SDL_GetTicks() * 1.e-3; } 
//	SDL_Surface *GetSurfaceData ();
	unsigned char *GetSurfaceData ();
};

extern CWinsys Winsys;

#endif
