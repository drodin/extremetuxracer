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

#include "keyframe.h"
#include "tux.h"
#include "course.h"
#include "spx.h"
#include "game_ctrl.h"

CKeyframe TuxStart;		// keyframe before starting
CKeyframe TuxFailure;	// in challenge mode: race was not successful
CKeyframe TuxSuccess;	// in challenge mode: successful
CKeyframe TuxFinal;		// in training mode after reaching the finish line
CKeyframe TuxTest;

CKeyframe::CKeyframe () {
	keytime = 0;
	numFrames = 0;
	active = false;
	loaded = false;
}

CKeyframe::~CKeyframe () {}

double CKeyframe::interp (double frac, double v1, double v2) {
    return frac * v1 + (1.0 - frac) * v2;
} 

void CKeyframe::Init (TVector3 ref_pos) {
    ResetCharNode2 ("head");
    ResetCharNode2 ("neck");
	refpos = ref_pos;
	active = true;
	keyidx = 0;
	keytime = 0;
}

void CKeyframe::Reset () {
	loaded = false;
	active = false;
}

void CKeyframe::Load (string filename) {
	CSPList list (1000);
	int i;
	string line;
	TVector2 pp;
	numFrames = 0;
	if (list.Load (param.keyframe_dir, filename)) {
		for (i=0; i<list.Count(); i++) {
			line = list.Line (i);
			frames[numFrames].time    = SPFloatN   (line, "time", 0);		
			frames[numFrames].pos	  = SPVector3N (line, "pos", MakeVector (0, 0, 0));
			frames[numFrames].yaw     = SPFloatN   (line, "yaw", 0);
			frames[numFrames].pitch   = SPFloatN   (line, "pitch", 0);
			frames[numFrames].neck    = SPFloatN   (line, "neck", 0);
			frames[numFrames].head    = SPFloatN   (line, "head", 0);
			pp = SPVector2N (line, "sh", MakeVector2 (0, 0));
			frames[numFrames].l_shldr = pp.x;
			frames[numFrames].r_shldr = pp.y;
			pp = SPVector2N (line, "hip", MakeVector2 (0, 0));
			frames[numFrames].l_hip   = pp.x;
			frames[numFrames].r_hip   = pp.y;
			pp = SPVector2N (line, "knee", MakeVector2 (0, 0));
			frames[numFrames].l_knee  = pp.x;
			frames[numFrames].r_knee  = pp.y;
			pp = SPVector2N (line, "ankle", MakeVector2 (0, 0));
			frames[numFrames].l_ankle = pp.x;
			frames[numFrames].r_ankle = pp.y;
			numFrames++;
		}
		loaded = true;
	} else {
		MessageN ("keyframe not found:", filename);
		loaded = false;
	}	
}

// there are more possibilities for rotating the parts of the body,
// that will be implemented later

void CKeyframe::InterpolateKeyframe (int idx, double frac) {
	double v;
    v = interp (frac, frames[idx].yaw, frames[idx+1].yaw);
    RotateCharNode2 ("root", 2, v);

    v = interp (frac, frames[idx].pitch, frames[idx+1].pitch);
    RotateCharNode2 ("root", 1, v);

    v = interp (frac, frames[idx].neck, frames[idx+1].neck);
    RotateCharNode2 ("neck", 3, v);

    v = interp (frac, frames[idx].head, frames[idx+1].head);
    RotateCharNode2 ("head", 2, v);

    v = interp (frac, frames[idx].l_shldr, frames[idx+1].l_shldr);
    RotateCharNode2 ("left_shoulder", 3, v);

    v = interp (frac, frames[idx].r_shldr, frames[idx+1].r_shldr);
    RotateCharNode2 ("right_shoulder", 3, v);

    v = interp (frac, frames[idx].l_hip, frames[idx+1].l_hip);
    RotateCharNode2 ("left_hip", 3, v);

    v = interp (frac, frames[idx].r_hip, frames[idx+1].r_hip);
    RotateCharNode2 ("right_hip", 3, v);

    v = interp (frac, frames[idx].l_knee, frames[idx+1].l_knee);
    RotateCharNode2 ("left_knee", 3, v);

    v = interp (frac, frames[idx].r_knee, frames[idx+1].r_knee);
    RotateCharNode2 ("right_knee", 3, v);

    v = interp (frac, frames[idx].l_ankle, frames[idx+1].l_ankle);
    RotateCharNode2 ("left_ankle", 3, v);

    v = interp (frac, frames[idx].r_ankle, frames[idx+1].r_ankle);
    RotateCharNode2 ("right_ankle", 3, v);
}

void CKeyframe::Update (double timestep, CControl *ctrl) {
    double frac;
    TVector3 pos;

	if (!active) return;
    keytime += timestep;
	if (keytime >= frames[keyidx].time) {
		keyidx++;
		keytime = 0;
	}
	
    if  (keyidx >= numFrames-1 || numFrames < 2) {
		active = false;
        return;
    } 

    if  (fabs (frames[keyidx].time) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx].time - keytime) / frames[keyidx].time;

    pos.x = interp (frac, frames[keyidx].pos.x, frames[keyidx+1].pos.x) + refpos.x;
    pos.z = interp (frac, frames[keyidx].pos.z, frames[keyidx+1].pos.z) + refpos.z;
    pos.y = interp (frac, frames[keyidx].pos.y, frames[keyidx+1].pos.y);
    pos.y += Course.FindYCoord (pos.x, pos.z);
    Players.GetControl(0)->SetTuxPosition (pos);

	ResetTuxRoot2 ();
	ResetTuxJoints2 ();

    double disp_y = pos.y + TUX_Y_CORR; 
    ResetCharNode2 ("root");
    TranslateCharNode2 ("root", MakeVector (pos.x, disp_y, pos.z));
	InterpolateKeyframe (keyidx, frac);
}

void CKeyframe::TestUpdate (double timestep) {
    double frac;
    TVector3 pos;

	if (!active) return;
    keytime += timestep;
	if (keytime >= frames[keyidx].time) {
		keyidx++;
		keytime = 0;
	}
	
    if  (keyidx >= numFrames-1 || numFrames < 2) {
		active = false;
        return;
    } 

    if  (fabs (frames[keyidx].time) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx].time - keytime) / frames[keyidx].time;

    pos.x = interp (frac, frames[keyidx].pos.x, frames[keyidx+1].pos.x);
    pos.z = interp (frac, frames[keyidx].pos.z, frames[keyidx+1].pos.z);
    pos.y = interp (frac, frames[keyidx].pos.y, frames[keyidx+1].pos.y);

	ResetTuxRoot2 ();
	ResetTuxJoints2 ();
	InterpolateKeyframe (keyidx, frac);
}

