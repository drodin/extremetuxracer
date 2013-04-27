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
#include "tux.h"

#define MAX_KEY_FRAMES 128
#define MAX_FRAME_VALUES 32

typedef struct {
	double val[MAX_FRAME_VALUES];
} TKeyframe2;

class CKeyframe {
private:
	double keytime;
	TKeyframe2 *frames[MAX_KEY_FRAMES];
	TVector3 refpos;
	double heightcorr;
	int keyidx;
	string loadedfile;
	TKeyframe2 clipboard;

	double interp (double frac, double v1, double v2);
	void InterpolateKeyframe (int idx, double frac, CCharShape *shape);

	// test and editing
	void ResetFrame2 (TKeyframe2 *frame);
public:
	CKeyframe ();
	int numFrames;
	string jointname;
	bool loaded;

	bool active;
	void Init (TVector3 ref_position, double height_correction);
	void Init (TVector3 ref_position, double height_correction, CCharShape *shape);
	void InitTest (TVector3 ref_position, CCharShape *shape);
	void Reset ();
	void Update (double timestep, CControl *ctrl);
	void UpdateTest (double timestep, CCharShape *shape);
	bool Load (string dir, string filename);
	void CalcKeyframe (int idx, CCharShape *shape, TVector3 refpos);

	// test and editing
	TKeyframe2 *GetFrame (int idx);
	string GetHighlightName (int idx);
	string GetJointName (int idx);
	int GetNumJoints ();
	void SaveTest (string dir, string filename);
	void CopyFrame (int prim_idx, int sec_idx);
	void AddFrame ();
	int  DeleteFrame (int idx);
	void InsertFrame (int idx);
	void CopyToClipboard (int idx);
	void PasteFromClipboard (int idx);
	void ClearFrame (int idx);
};

extern CKeyframe TestFrame;

#endif
