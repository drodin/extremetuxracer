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

#ifndef VIEW_H
#define VIEW_H

#include "bh.h"
#include "tux.h"

void set_view_mode (CControl *ctrl, TViewMode mode );
TViewMode get_view_mode (CControl *ctrl );
void update_view (CControl *ctrl, double dt );
void setup_view_matrix (CControl *ctrl );

// ------------- viewfrustum ------------------------------------------

typedef enum {
    NoClip,
    SomeClip,
    NotVisible
} clip_result_t;

void SetupViewFrustum (CControl *ctrl);
clip_result_t clip_aabb_to_view_frustum (TVector3 min, TVector3 max );

TPlane get_far_clip_plane();
TPlane get_left_clip_plane();
TPlane get_right_clip_plane();
TPlane get_bottom_clip_plane();

#endif
