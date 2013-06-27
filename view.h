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

void set_view_mode (CControl *ctrl, TViewMode mode);
void update_view (CControl *ctrl, double dt);

void SetStationaryCamera (bool stat); // 0 follow, 1 stationary
void IncCameraDistance (double timestep);
void SetCameraDistance (double val);

// ------------- viewfrustum ------------------------------------------

enum clip_result_t {
    NoClip,
    SomeClip,
    NotVisible
};

void SetupViewFrustum (const CControl *ctrl);
clip_result_t clip_aabb_to_view_frustum (const TVector3& min, const TVector3& max);

const TPlane& get_far_clip_plane();
const TPlane& get_left_clip_plane();
const TPlane& get_right_clip_plane();
const TPlane& get_bottom_clip_plane();

#endif
