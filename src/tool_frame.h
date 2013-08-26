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

#ifndef TOOL_FRAME_H
#define TOOL_FRAME_H

#include "bh.h"

void InitFrameTools ();
void SingleFrameKeys (unsigned int key, bool special, bool release, int x, int y);
void SingleFrameMouse (int button, int state, int x, int y);
void SingleFrameMotion (int x, int y);
void RenderSingleFrame (double timestep);

// --------------------------------------------------------------------
//				frame sequence
// --------------------------------------------------------------------

void SequenceKeys (unsigned int key, bool special, bool release, int x, int y);
void SequenceMouse (int button, int state, int x, int y);
void SequenceMotion (int x, int y);
void RenderSequence (double timestep);

#endif

