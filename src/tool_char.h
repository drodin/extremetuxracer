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

#ifndef TOOL_CHAR_H
#define TOOL_CHAR_H

#include "bh.h"

struct TCharAction;

void InitCharTools ();
void CharKeys (unsigned int key, bool special, bool release, int x, int y);
void CharMouse (int button, int state, int x, int y);
void CharMotion (int x, int y);
void RenderChar (double timestep);
void StoreAction (TCharAction *act);

#endif

