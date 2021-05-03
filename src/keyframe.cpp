/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "keyframe.h"
#include "course.h"
#include "spx.h"
#include "tux.h"
#include "game_ctrl.h"
#include "physics.h"
#include <algorithm>
#include <iterator>

static const int numJoints = 19;

// The jointnames are shown on the tools screen and define the
// possible rotations. A joint can be rotated around 3 axis, so
// a joint can contain up to 3 joinnames.
static const std::string jointnames[numJoints] = {
	"time","pos.x","pos.y","pos.z","yaw","pitch","roll","neck","head",
	"l_shldr","r_shldr","l_arm","r_arm",
	"l_hip","r_hip","l_knee","r_knee","l_ankle","r_ankle"
};

// The highlightnames must be official joint identifiers, defined in
// the character description. They are used to find the port nodes
// for highlighting
static const std::string highlightnames[numJoints] = {
	"","","","","","","","neck","head",
	"left_shldr","right_shldr","left_shldr","right_shldr",
	"left_hip","right_hip","left_knee","right_knee","left_ankle","right_ankle"
};

CKeyframe TestFrame;

CKeyframe::CKeyframe() {
	keytime = 0;
	active = false;
	loaded = false;
	heightcorr = 0;
	keyidx = 0;
}

double CKeyframe::interp(double frac, double v1, double v2) {
	return frac * v1 + (1.0 - frac) * v2;
}

void CKeyframe::Init(const TVector3d& ref_position, double height_correction) {
	if (!loaded) return;
	g_game.character->shape->ResetNode("head");
	g_game.character->shape->ResetNode("neck");
	refpos = ref_position;
	heightcorr = height_correction;
	active = true;
	keyidx = 0;
	keytime = 0;
}

void CKeyframe::Init(const TVector3d& ref_position, double height_correction, CCharShape *shape) {
	if (!loaded) return;
	shape->ResetNode("head");
	shape->ResetNode("neck");
	refpos = ref_position;
	heightcorr = height_correction;
	active = true;
	keyidx = 0;
	keytime = 0;

}

void CKeyframe::InitTest(const TVector3d& ref_position, CCharShape *shape) {
	if (!loaded) return;
	shape->ResetNode("head");
	shape->ResetNode("neck");
	refpos = ref_position;
	heightcorr = 0.0;
	active = true;
	keyidx = 0;
	keytime = 0;
}

void CKeyframe::Reset() {
	loaded = false;
	active = false;
	loadedfile = "";
	keytime = 0;
	frames.clear();
}

bool CKeyframe::Load(const std::string& dir, const std::string& filename) {
	if (loaded && loadedfile == filename) return true;
	CSPList list;

	if (list.Load(dir, filename)) {
		frames.resize(list.size());
		std::size_t i = 0;
		for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
			frames[i].val[0] = SPFloatN(*line, "time", 0);
			TVector3d posit = SPVector3d(*line, "pos");
			frames[i].val[1] = posit.x;
			frames[i].val[2] = posit.y;
			frames[i].val[3] = posit.z;
			frames[i].val[4] = SPFloatN(*line, "yaw", 0);
			frames[i].val[5] = SPFloatN(*line, "pitch", 0);
			frames[i].val[6] = SPFloatN(*line, "roll", 0);
			frames[i].val[7] = SPFloatN(*line, "neck", 0);
			frames[i].val[8] = SPFloatN(*line, "head", 0);
			TVector2d pp = SPVector2d(*line, "sh");
			frames[i].val[9] = pp.x;
			frames[i].val[10] = pp.y;
			pp = SPVector2d(*line, "arm");
			frames[i].val[11] = pp.x;
			frames[i].val[12] = pp.y;
			pp = SPVector2d(*line, "hip");
			frames[i].val[13] = pp.x;
			frames[i].val[14] = pp.y;
			pp = SPVector2d(*line, "knee");
			frames[i].val[15] = pp.x;
			frames[i].val[16] = pp.y;
			pp = SPVector2d(*line, "ankle");
			frames[i].val[17] = pp.x;
			frames[i].val[18] = pp.y;
		}
		loaded = true;
		loadedfile = filename;
		return true;
	} else {
		Message("keyframe not found:", filename);
		loaded = false;
		return false;
	}
}

// there are more possibilities for rotating the parts of the body,
// that will be implemented later

void CKeyframe::InterpolateKeyframe(std::size_t idx, double frac, CCharShape *shape) {
	double vv;
	vv = interp(frac, frames[idx].val[4], frames[idx+1].val[4]);
	shape->RotateNode("root", 2, vv);

	vv = interp(frac, frames[idx].val[5], frames[idx+1].val[5]);
	shape->RotateNode("root", 1, vv);

	vv = interp(frac, frames[idx].val[6], frames[idx+1].val[6]);
	shape->RotateNode("root", 3, vv);

	vv = interp(frac, frames[idx].val[7], frames[idx+1].val[7]);
	shape->RotateNode("neck", 3, vv);

	vv = interp(frac, frames[idx].val[8], frames[idx+1].val[8]);
	shape->RotateNode("head", 2, vv);

	vv = interp(frac, frames[idx].val[9], frames[idx+1].val[9]);
	shape->RotateNode("left_shldr", 3, vv);

	vv = interp(frac, frames[idx].val[10], frames[idx+1].val[10]);
	shape->RotateNode("right_shldr", 3, vv);

	vv = interp(frac, frames[idx].val[11], frames[idx+1].val[11]);
	shape->RotateNode("left_shldr", 2, vv);

	vv = interp(frac, frames[idx].val[12], frames[idx+1].val[12]);
	shape->RotateNode("right_shldr", 2, vv);

	vv = interp(frac, frames[idx].val[13], frames[idx+1].val[13]);
	shape->RotateNode("left_hip", 3, vv);

	vv = interp(frac, frames[idx].val[14], frames[idx+1].val[14]);
	shape->RotateNode("right_hip", 3, vv);

	vv = interp(frac, frames[idx].val[15], frames[idx+1].val[15]);
	shape->RotateNode("left_knee", 3, vv);

	vv = interp(frac, frames[idx].val[16], frames[idx+1].val[16]);
	shape->RotateNode("right_knee", 3, vv);

	vv = interp(frac, frames[idx].val[17], frames[idx+1].val[17]);
	shape->RotateNode("left_ankle", 3, vv);

	vv = interp(frac, frames[idx].val[18], frames[idx+1].val[18]);
	shape->RotateNode("right_ankle", 3, vv);
}

void CKeyframe::CalcKeyframe(std::size_t idx, CCharShape *shape, const TVector3d& refpos_) const {
	double vv;
	TVector3d pos;

	pos.x = frames[idx].val[1] + refpos_.x;
	pos.z = frames[idx].val[3] + refpos_.z;
	pos.y = refpos_.y;

	shape->ResetRoot();
	shape->ResetJoints();
	shape->TranslateNode(0, pos);

	vv = frames[idx].val[4];
	shape->RotateNode("root", 2, vv);

	vv = frames[idx].val[5];
	shape->RotateNode("root", 1, vv);

	vv = frames[idx].val[6];
	shape->RotateNode("root", 3, vv);

	vv = frames[idx].val[7];
	shape->RotateNode("neck", 3, vv);

	vv = frames[idx].val[8];
	shape->RotateNode("head", 2, vv);

	vv = frames[idx].val[9];
	shape->RotateNode("left_shldr", 3, vv);

	vv = frames[idx].val[10];
	shape->RotateNode("right_shldr", 3, vv);

	vv = frames[idx].val[11];
	shape->RotateNode("left_shldr", 2, vv);

	vv = frames[idx].val[12];
	shape->RotateNode("right_shldr", 2, vv);

	vv = frames[idx].val[13];
	shape->RotateNode("left_hip", 3, vv);

	vv = frames[idx].val[14];
	shape->RotateNode("right_hip", 3, vv);

	vv = frames[idx].val[15];
	shape->RotateNode("left_knee", 3, vv);

	vv = frames[idx].val[16];
	shape->RotateNode("right_knee", 3, vv);

	vv = frames[idx].val[17];
	shape->RotateNode("left_ankle", 3, vv);

	vv = frames[idx].val[18];
	shape->RotateNode("right_ankle", 3, vv);
}

void CKeyframe::Update(float timestep) {
	if (!loaded) return;
	if (!active) return;

	keytime += timestep;
	if (keytime >= frames[keyidx].val[0]) {
		keyidx++;
		keytime = 0;
	}

	if (keyidx >= frames.size()-1 || frames.size() < 2) {
		active = false;
		return;
	}

	double frac;
	TVector3d pos;
	CCharShape *shape = g_game.character->shape;

	if (std::fabs(frames[keyidx].val[0]) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx].val[0] - keytime) / frames[keyidx].val[0];

	pos.x = interp(frac, frames[keyidx].val[1], frames[keyidx+1].val[1]) + refpos.x;
	pos.z = interp(frac, frames[keyidx].val[3], frames[keyidx+1].val[3]) + refpos.z;
	pos.y = interp(frac, frames[keyidx].val[2], frames[keyidx+1].val[2]);
	pos.y += Course.FindYCoord(pos.x, pos.z);

	shape->ResetRoot();
	shape->ResetJoints();

	g_game.player->ctrl->cpos = pos;
	double disp_y = pos.y + TUX_Y_CORR + heightcorr;
	shape->ResetNode(0);
	shape->TranslateNode(0, TVector3d(pos.x, disp_y, pos.z));
	InterpolateKeyframe(keyidx, frac, shape);
}

void CKeyframe::UpdateTest(float timestep, CCharShape *shape) {
	if (!active) return;

	keytime += timestep;
	if (keytime >= frames[keyidx].val[0]) {
		keyidx++;
		keytime = 0;
	}

	if (keyidx >= frames.size()-1 || frames.size() < 2) {
		active = false;
		return;
	}

	double frac;
	TVector3d pos;

	if (std::fabs(frames[keyidx].val[0]) < 0.0001) frac = 1.0;
	else frac = (frames[keyidx].val[0] - keytime) / frames[keyidx].val[0];

	pos.x = interp(frac, frames[keyidx].val[1], frames[keyidx+1].val[1]) + refpos.x;
	pos.z = interp(frac, frames[keyidx].val[3], frames[keyidx+1].val[3]) + refpos.z;
	pos.y = interp(frac, frames[keyidx].val[2], frames[keyidx+1].val[2]);

	shape->ResetRoot();
	shape->ResetJoints();
	shape->TranslateNode(0, pos);
	InterpolateKeyframe(keyidx, frac, shape);
}

void CKeyframe::ResetFrame2(TKeyframe *frame) {
	std::fill_n(frame->val + 1, MAX_FRAME_VALUES - 1, 0.0);
	frame->val[0] = 0.5; // time
}

TKeyframe *CKeyframe::GetFrame(std::size_t idx) {
	if (idx >= frames.size()) return nullptr;
	return &frames[idx];
}

const std::string& CKeyframe::GetJointName(std::size_t idx) {
	if (idx >= numJoints) return emptyString;
	return jointnames[idx];
}

const std::string& CKeyframe::GetHighlightName(std::size_t idx) {
	if (idx >= numJoints) return emptyString;
	return highlightnames[idx];
}

int CKeyframe::GetNumJoints() {
	return numJoints;
}

void CKeyframe::SaveTest(const std::string& dir, const std::string& filename) const {
	CSPList list;

	for (std::size_t i=0; i<frames.size(); i++) {
		const TKeyframe* frame = &frames[i];
		std::string line = "*[time] " + Float_StrN(frame->val[0], 1);
		line += " [pos] " + Float_StrN(frame->val[1], 2);
		line += " " + Float_StrN(frame->val[2], 2);
		line += " " + Float_StrN(frame->val[3], 2);
		if (frame->val[4] != 0) line += " [yaw] " + Int_StrN((int)frame->val[4]);
		if (frame->val[5] != 0) line += " [pitch] " + Int_StrN((int)frame->val[5]);
		if (frame->val[6] != 0) line += " [roll] " + Int_StrN((int)frame->val[6]);
		if (frame->val[7] != 0) line += " [neck] " + Int_StrN((int)frame->val[7]);
		if (frame->val[8] != 0) line += " [head] " + Int_StrN((int)frame->val[8]);

		double ll = frame->val[9];
		double rr = frame->val[10];
		if (ll != 0 || rr != 0)
			line += " [sh] " + Int_StrN((int)ll) + ' ' + Int_StrN((int)rr);

		ll = frame->val[11];
		rr = frame->val[12];
		if (ll != 0 || rr != 0)
			line += " [arm] " + Int_StrN((int)ll) + ' ' + Int_StrN((int)rr);

		ll = frame->val[13];
		rr = frame->val[14];
		if (ll != 0 || rr != 0)
			line += " [hip] " + Int_StrN((int)ll) + ' ' + Int_StrN((int)rr);

		ll = frame->val[15];
		rr = frame->val[16];
		if (ll != 0 || rr != 0)
			line += " [knee] " + Int_StrN((int)ll) + ' ' + Int_StrN((int)rr);

		ll = frame->val[17];
		rr = frame->val[18];
		if (ll != 0 || rr != 0)
			line += " [ankle] " + Int_StrN((int)ll) + ' ' + Int_StrN((int)rr);

		list.Add(line);
	}
	list.Save(dir, filename);
}

void CKeyframe::CopyFrame(std::size_t prim_idx, std::size_t sec_idx) {
	TKeyframe *ppp = &frames[prim_idx];
	TKeyframe *sss = &frames[sec_idx];
	std::copy_n(ppp->val, MAX_FRAME_VALUES, sss->val);
}

void CKeyframe::AddFrame() {
	frames.emplace_back();
}

std::size_t CKeyframe::DeleteFrame(std::size_t idx) {
	if (frames.size() < 2) return idx;
	if (idx > frames.size() - 1) return 0;

	std::vector<TKeyframe>::iterator i = frames.begin();
	std::advance(i, idx);
	frames.erase(i);
	return std::max(idx, frames.size() - 2);
}

void CKeyframe::InsertFrame(std::size_t idx) {
	if (idx > frames.size() - 1) return;

	std::vector<TKeyframe>::iterator i = frames.begin();
	std::advance(i, idx);
	frames.emplace(i);
}

void CKeyframe::CopyToClipboard(std::size_t idx) {
	if (idx >= frames.size()) return;
	std::copy_n(frames[idx].val, MAX_FRAME_VALUES, clipboard.val);
}

void CKeyframe::PasteFromClipboard(std::size_t idx) {
	if (idx >= frames.size()) return;
	std::copy_n(clipboard.val, MAX_FRAME_VALUES, frames[idx].val);
}

void CKeyframe::ClearFrame(std::size_t idx) {
	if (idx >= frames.size()) return;
	ResetFrame2(&frames[idx]);
}
