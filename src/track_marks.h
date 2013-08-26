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

// This code is from Tuxracer 0.61. It works but it is only suitable for
// snow tracks. To make the code usable for other terrains (mud, snowy ice etc.)
// it must be rewritten. Trial code exists for Bunny Hill.

#ifndef TRACK_MARKS_H
#define TRACK_MARKS_H

#include "bh.h"

void init_track_marks();
void break_track_marks();

void SetTrackIDs(int id1, int id2, int id3);
void UpdateTrackmarks(const CControl *ctrl);
void DrawTrackmarks();

#endif
