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
#include "course.h"
#include "spx.h"
#include "game_ctrl.h"

// The jointnames are shown on the tools screen and define the 
// possible rotations. A joint can be rotated around 3 axis, so 
// a joint can contain up to 3 joinnames. 
string jointnames[MAX_KEY_FRAMES] =
	{"time","pos.x","pos.y","pos.z","yaw","pitch","roll","neck","head",
	"l_shldr","r_shldr","l_arm","r_arm",
	"l_hip","r_hip","l_knee","r_knee","l_ankle","r_ankle",
	"","","","","","","","","","","","",""};

// The highlightnames must be official joint identifiers, defined in
// the character description. They are used to find the port nodes 
// for highlighting
string highlightnames[MAX_KEY_FRAMES] =
	{"","","","","","","","neck","head",
	"left_shldr","right_shldr","left_shldr","right_shldr",
	"left_hip","right_hip","left_knee","right_knee","left_ankle","right_ankle",
	"","","","","","","","","","","","",""};

int numJoints = 19;

CKeyframe TestFrame;

CKeyframe::CKeyframe () {
	for (int i=0; i<=MAX_KEY_FRAMES; i++) {
		frames[i] = NULL;
	}
	keytime = 0;
	numFrames = 0;
	active = false;
	loaded = false;
	heightcorr = 0;
	keyidx = 0;
}

CKeyframe::~CKeyframe () {}

double CKeyframe::interp (double frac, double v1, double v2) {
    return frac * v1 + (1.0 - frac) * v2;
} 

void CKeyframe::Init (TVector3 ref_position, double height_correction) {
	if (!loaded) return;
	CCharShape *shape = Char.GetShape (g_game.char_id);
    shape->ResetNode ("head");
    shape->ResetNode ("neck");
	refpos = ref_position;
	heightcorr = height_correction;
	active = true;
	keyidx = 0;
	keytime = 0;
}

void CKeyframe::Init (TVector3 ref_position, double height_correction, CCharShape *shape) {
	if (!loaded) return;
    shape->ResetNode ("head");
    shape->ResetNode ("neck");
	refpos = ref_position;
	heightcorr = height_correction;
	active = true;
	keyidx = 0;
	keytime = 0;

}

void CKeyframe::InitTest (TVector3 ref_position, CCharShape *shape) {
	if (!loaded) return;
    shape->ResetNode ("head");
    shape->ResetNode ("neck");
	refpos = ref_position;
	heightcorr = 0.0;
	active = true;
	keyidx = 0;
	keytime = 0;
}

void CKeyframe::Reset () {
	loaded = false;
	active = false;
	loadedfile = "";
	keytime = 0;
	numFrames = 0;

	for (int i=0; i<=MAX_KEY_FRAMES; i++) {
		if (frames[i] != NULL) {
			free (frames[i]);
			frames[i] = NULL;
		}
	}
}

bool CKeyframe::Load (string dir, string filename) {
	if (loaded && loadedfile == filename) return true;
	CSPList list (1000);
	int i;
	string line;
	TVector2 pp;
	numFrames = 0;
	TVector3 posit;

	if (list.Load (dir, filename)) {
		for (i=0; i<list.Count(); i++) {
			line = list.Line (i);
			frames[numFrames] = new (TKeyframe2);
			frames[numFrames]->val[0] = SPFloatN (line, "time", 0);		
			posit = SPVector3N (line, "pos", MakeVector (0, 0, 0));
			frames[numFrames]->val[1] = posit.x;
			frames[numFrames]->val[2] = posit.y;
			frames[numFrames]->val[3] = posit.z;
			frames[numFrames]->val[4] = SPFloatN (line, "yaw", 0);
			frames[numFrames]->val[5] = SPFloatN (line, "pitch", 0);
			frames[numFrames]->val[6] = SPFloatN (line, "roll", 0);
			frames[numFrames]->val[7] = SPFloatN (line, "neck", 0);
			frames[numFrames]->val[8] = SPFloatN (line, "head", 0);
			pp = SPVector2N (line, "sh", MakeVector2 (0, 0));
			frames[numFrames]->val[9] = pp.x;
			frames[numFrames]->val[10] = pp.y;
			pp = SPVector2N (line, "arm", MakeVector2 (0, 0));
			frames[numFrames]->val[11] = pp.x;
			frames[numFrames]->val[12] = pp.y;
			pp = SPVector2N (line, "hip", MakeVector2 (0, 0));
			frames[numFrames]->val[13] = pp.x;
			frames[numFrames]->val[14] = pp.y;
			pp = SPVector2N (line, "knee", MakeVector2 (0, 0));
			frames[numFrames]->val[15] = pp.x;
			frames[numFrames]->val[16] = pp.y;
			pp = SPVector2N (line, "ankle", MakeVector2 (0, 0));
			frames[numFrames]->val[17] = pp.x;
			frames[numFrames]->val[18] = pp.y;

			numFrames++;
		}
		loaded = true;
		loadedfile = filename;
		return true;
	} else {
		MessageN ("keyframe not found:", filename);
		loaded = false;
		return false;
	}	
}

// there are more possibilities for rotating the parts of the body,
// that will be implemented later

void CKeyframe::InterpolateKeyframe (int idx, double frac, CCharShape *shape) {

	double vv;
    vv = interp (frac, frames[idx]->val[4], frames[idx+1]->val[4]);
    shape->RotateNode ("root", 2, vv);

    vv = interp (frac, frames[idx]->val[5], frames[idx+1]->val[5]);
	shape->RotateNode ("root", 1, vv);

    vv = interp (frac, frames[idx]->val[6], frames[idx+1]->val[6]);
    shape->RotateNode ("root", 3, vv);

    vv = interp (frac, frames[idx]->val[7], frames[idx+1]->val[7]);
    shape->RotateNode ("neck", 3, vv);

    vv = interp (frac, frames[idx]->val[8], frames[idx+1]->val[8]);
    shape->RotateNode ("head", 2, vv);

    vv = interp (frac, frames[idx]->val[9], frames[idx+1]->val[9]);
    shape->RotateNode ("left_shldr", 3, vv);

    vv = interp (frac, frames[idx]->val[10], frames[idx+1]->val[10]);
    shape->RotateNode ("right_shldr", 3, vv);

    vv = interp (frac, frames[idx]->val[11], frames[idx+1]->val[11]);
    shape->RotateNode ("left_shldr", 2, vv);

    vv = interp (frac, frames[idx]->val[12], frames[idx+1]->val[12]);
    shape->RotateNode ("right_shldr", 2, vv);

    vv = interp (frac, frames[idx]->val[13], frames[idx+1]->val[13]);
    shape->RotateNode ("left_hip", 3, vv);

    vv = interp (frac, frames[idx]->val[14], frames[idx+1]->val[14]);
    shape->RotateNode ("right_hip", 3, vv);

    vv = interp (frac, frames[idx]->val[15], frames[idx+1]->val[15]);
    shape->RotateNode ("left_knee", 3, vv);

    vv = interp (frac, frames[idx]->val[16], frames[idx+1]->val[16]);
    shape->RotateNode ("right_knee", 3, vv);

    vv = interp (frac, frames[idx]->val[17], frames[idx+1]->val[17]);
    shape->RotateNode ("left_ankle", 3, vv);

    vv = interp (frac, frames[idx]->val[18], frames[idx+1]->val[18]);
    shape->RotateNode ("right_ankle", 3, vv);
}

void CKeyframe::CalcKeyframe (int idx, CCharShape *shape, TVector3 refpos) {
	double vv;
	TVector3 pos;

    pos.x = frames[idx]->val[1] + refpos.x;
    pos.z = frames[idx]->val[3] + refpos.z;
	pos.y = refpos.y;

	shape->ResetRoot ();
	shape->ResetJoints ();
    shape->TranslateNode (0, MakeVector (pos.x, pos.y, pos.z));

    vv = frames[idx]->val[4];
    shape->RotateNode ("root", 2, vv);

    vv = frames[idx]->val[5];
	shape->RotateNode ("root", 1, vv);

    vv = frames[idx]->val[6];
	shape->RotateNode ("root", 3, vv);

    vv = frames[idx]->val[7];
    shape->RotateNode ("neck", 3, vv);

    vv = frames[idx]->val[8];
    shape->RotateNode ("head", 2, vv);

    vv = frames[idx]->val[9];
    shape->RotateNode ("left_shldr", 3, vv);

    vv = frames[idx]->val[10];
    shape->RotateNode ("right_shldr", 3, vv);

    vv = frames[idx]->val[11];
    shape->RotateNode ("left_shldr", 2, vv);

    vv = frames[idx]->val[12];
    shape->RotateNode ("right_shldr", 2, vv);

    vv = frames[idx]->val[13];
    shape->RotateNode ("left_hip", 3, vv);

    vv = frames[idx]->val[14];
    shape->RotateNode ("right_hip", 3, vv);

    vv = frames[idx]->val[15];
    shape->RotateNode ("left_knee", 3, vv);

    vv = frames[idx]->val[16];
    shape->RotateNode ("right_knee", 3, vv);

    vv = frames[idx]->val[17];
    shape->RotateNode ("left_ankle", 3, vv);

    vv = frames[idx]->val[18];
    shape->RotateNode ("right_ankle", 3, vv);
}

void CKeyframe::Update (double timestep, CControl *ctrl) {
	if (!loaded) return;
    double frac;
    TVector3 pos;
	CCharShape *shape = Char.GetShape (g_game.char_id);

	if (!active) return;
    keytime += timestep;
	if (keytime >= frames[keyidx]->val[0]) {
		keyidx++;
		keytime = 0;
	}

    if  (keyidx >= numFrames-1 || numFrames < 2) {
		active = false;
        return;
    } 

    if  (fabs (frames[keyidx]->val[0]) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx]->val[0] - keytime) / frames[keyidx]->val[0];

    pos.x = interp (frac, frames[keyidx]->val[1], frames[keyidx+1]->val[1]) + refpos.x;
    pos.z = interp (frac, frames[keyidx]->val[3], frames[keyidx+1]->val[3]) + refpos.z;
    pos.y = interp (frac, frames[keyidx]->val[2], frames[keyidx+1]->val[2]);
    pos.y += Course.FindYCoord (pos.x, pos.z);

	shape->ResetRoot ();
	shape->ResetJoints ();

    Players.GetCtrl (g_game.player_id)->cpos = pos;
    double disp_y = pos.y + TUX_Y_CORR + heightcorr; 
    shape->ResetNode (0);
    shape->TranslateNode (0, MakeVector (pos.x, disp_y, pos.z));
	InterpolateKeyframe (keyidx, frac, shape);
}

void CKeyframe::UpdateTest (double timestep, CCharShape *shape) {
    double frac;
    TVector3 pos;

	if (!active) return;
    keytime += timestep;
	if (keytime >= frames[keyidx]->val[0]) {
		keyidx++;
		keytime = 0;
	}
	
    if  (keyidx >= numFrames-1 || numFrames < 2) {
		active = false;
        return;
    } 

    if  (fabs (frames[keyidx]->val[0]) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx]->val[0] - keytime) / frames[keyidx]->val[0];

    pos.x = interp (frac, frames[keyidx]->val[1], frames[keyidx+1]->val[1]) + refpos.x;
    pos.z = interp (frac, frames[keyidx]->val[3], frames[keyidx+1]->val[3]) + refpos.z;
    pos.y = interp (frac, frames[keyidx]->val[2], frames[keyidx+1]->val[2]);

	shape->ResetRoot ();
	shape->ResetJoints ();
    shape->TranslateNode (0, MakeVector (pos.x, pos.y, pos.z));
	InterpolateKeyframe (keyidx, frac, shape);
}

void CKeyframe::ResetFrame2 (TKeyframe2 *frame) {
	for (int i=1; i<32; i++) frame->val[i] = 0.0;
	frame->val[0] = 0.5; // time
}

TKeyframe2 *CKeyframe::GetFrame (int idx) {
	if (idx < 0 || idx >= numFrames) return NULL;
	return frames[idx];
}

string CKeyframe::GetJointName (int idx) {
	if (idx < 0 || idx >= numJoints) return "";
	return jointnames[idx];
}

string CKeyframe::GetHighlightName (int idx) {
	if (idx < 0 || idx >= numJoints) return "";
	return highlightnames[idx];
}

int CKeyframe::GetNumJoints () {
	return numJoints;
}

void CKeyframe::SaveTest (string dir, string filename) {
	CSPList list (100);
	string line;
	TKeyframe2 *frame;
	double ll, rr;

	for (int i=0; i<numFrames; i++) {
		frame = frames[i];
		line = "*[time] " + Float_StrN (frame->val[0], 1);
		line += " [pos] " + Float_StrN (frame->val[1], 2);
		line += " " + Float_StrN (frame->val[2], 2);
		line += " " + Float_StrN (frame->val[3], 2);
		if (frame->val[4] != 0) line += " [yaw] " + Float_StrN (frame->val[4], 0);
		if (frame->val[5] != 0) line += " [pitch] " + Float_StrN (frame->val[5], 0);
		if (frame->val[6] != 0) line += " [roll] " + Float_StrN (frame->val[6], 0);
		if (frame->val[7] != 0) line += " [neck] " + Float_StrN (frame->val[7], 0);
		if (frame->val[8] != 0) line += " [head] " + Float_StrN (frame->val[8], 0);

		ll = frame->val[9];
		rr = frame->val[10];
		if (ll != 0 || rr != 0) 
			line += " [sh] " + Float_StrN (ll, 0) + " " + Float_StrN (rr, 0);

		ll = frame->val[11];
		rr = frame->val[12];
		if (ll != 0 || rr != 0) 
			line += " [arm] " + Float_StrN (ll, 0) + " " + Float_StrN (rr, 0);

		ll = frame->val[13];
		rr = frame->val[14];
		if (ll != 0 || rr != 0) 
			line += " [hip] " + Float_StrN (ll, 0) + " " + Float_StrN (rr, 0);

		ll = frame->val[15];
		rr = frame->val[16];
		if (ll != 0 || rr != 0) 
			line += " [knee] " + Float_StrN (ll, 0) + " " + Float_StrN (rr, 0);

		ll = frame->val[17];
		rr = frame->val[18];
		if (ll != 0 || rr != 0) 
			line += " [ankle] " + Float_StrN (ll, 0) + " " + Float_StrN (rr, 0);

		list.Add (line);
	}
	list.Save (dir, filename);
}

void CKeyframe::CopyFrame (int prim_idx, int sec_idx) {
	TKeyframe2 *ppp = frames[prim_idx];
	TKeyframe2 *sss = frames[sec_idx];
	for (int i=0; i<MAX_FRAME_VALUES; i++) sss->val[i] = ppp->val[i];
}

void CKeyframe::AddFrame () {
	if (numFrames >= MAX_KEY_FRAMES) return;
	if (frames[numFrames] == NULL) frames[numFrames] = new (TKeyframe2);
	ResetFrame2 (frames[numFrames]);
	numFrames++;
}

int CKeyframe::DeleteFrame (int idx) {
	if (numFrames < 2) return idx;
	int lastframe = numFrames-1;
	if (idx < 0 || idx > lastframe) return 0;
	
	if (idx == lastframe) {
		free (frames[lastframe]);
		frames[lastframe] = NULL;
		numFrames--;
		return numFrames-1;
	
	} else {
		for (int i=idx; i<lastframe-1; i++) CopyFrame (i+1, i);
		free (frames[lastframe]);
		frames[lastframe] = NULL;
		numFrames--;
		return idx;
	}
}

void CKeyframe::InsertFrame (int idx) {
	if (numFrames >= MAX_KEY_FRAMES) return;
	int lastframe = numFrames-1;
	if (idx < 0 || idx > lastframe) return;

	if (frames[numFrames] == NULL) frames[numFrames] = new (TKeyframe2);
	
	for (int i=numFrames; i>idx; i--) CopyFrame (i-1, i);
	ResetFrame2 (frames[idx]);
	numFrames++;
}	

void CKeyframe::CopyToClipboard (int idx) {
	if (idx < 0 || idx >= numFrames) return;
	for (int i=0; i<MAX_FRAME_VALUES; i++) clipboard.val[i] = frames[idx]->val[i];
}

void CKeyframe::PasteFromClipboard (int idx) {
	if (idx < 0 || idx >= numFrames) return;
	for (int i=0; i<MAX_FRAME_VALUES; i++) frames[idx]->val[i] = clipboard.val[i];
}

void CKeyframe::ClearFrame (int idx) {
	if (idx < 0 || idx >= numFrames) return;
	ResetFrame2 (frames[idx]);
}


