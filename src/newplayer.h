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

#ifndef NEW_PLAYER_H
#define NEW_PLAYER_H

#include "bh.h"
#include "states.h"

class CNewPlayer : public State {
	void Enter();
	void Loop(double time_step);
	void Keyb_spec(SDL_keysym sym, bool release);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
};

extern CNewPlayer NewPlayer;

#endif
