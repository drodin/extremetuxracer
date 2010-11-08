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

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "bh.h"

#define MAX_KEY_FRAMES 128

typedef struct {
    TVector3 pos;
    double	time;
    double	yaw;      
    double	pitch;    
    double	l_shldr;
    double	r_shldr;
    double	l_hip;
    double	r_hip;
	double	l_knee;
	double	r_knee;
	double	l_ankle;
	double	r_ankle;
	double	neck;
	double	head;
} TKeyframe; 

class CKeyframe {
private:
	double keytime;
	int numFrames;
	TKeyframe frames[MAX_KEY_FRAMES];
	TVector3 refpos;
	double heightcorr;
	int keyidx;

	double interp (double frac, double v1, double v2);
	void InterpolateKeyframe (int idx, double frac);
public:
	CKeyframe ();
	~CKeyframe ();

	bool active;
	bool loaded;
	string loadedfile;
	void Init (TVector3 ref_position, double height_correction);
	void Reset ();
	void Update (double timestep, CControl *ctrl);
	void TestUpdate (double timestep);
	void Load (string filename);
};

extern CKeyframe TuxStart;
extern CKeyframe TuxLostrace;
extern CKeyframe TuxWonrace;
extern CKeyframe TuxFinish;
extern CKeyframe TuxTest;

#endif
