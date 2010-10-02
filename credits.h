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

#define MAX_CREDITS 64

typedef struct {
	string text;
	int offs;
	int font;
	double size;
	int col;
} TCredits;

void credits_register();
void LoadCreditList ();

#endif
