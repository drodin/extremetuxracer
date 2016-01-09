/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef CREDITS_H
#define CREDITS_H

#include "bh.h"
#include "states.h"
#include <forward_list>

#define MAX_CREDITS 64

struct TCredits {
	string text;
	float size;
	int offs;
	int font;
	int col;
};

class CCredits : public State {
	forward_list<TCredits> CreditList;

	void DrawCreditsText(float time_step);
	void Enter();
	void Exit();
	void Loop(float time_step);
	void Keyb(sf::Keyboard::Key key, bool release, int x, int y);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
	void LoadCreditList();
};

extern CCredits Credits;

#endif
