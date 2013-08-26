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

#ifndef PAUSED_H
#define PAUSED_H

#include "bh.h"
#include "states.h"

class CPaused : public State {
	void Loop(double time_step);
	void Keyb(unsigned int key, bool special, bool release, int x, int y);
	void Mouse(int button, int state, int x, int y);
public:
};

extern CPaused Paused;

#endif
