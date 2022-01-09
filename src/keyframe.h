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

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "bh.h"
#include <vector>

#define MAX_FRAME_VALUES 32

class CCharShape;

struct TKeyframe {
	double val[MAX_FRAME_VALUES];
	TKeyframe() {
		for (int i = 1; i < 32; i++) val[i] = 0.0;
		val[0] = 0.5; // time
	}
};

class CKeyframe {
private:
	std::vector<TKeyframe> frames;
	TVector3d refpos;
	std::string loadedfile;
	TKeyframe clipboard;
	double keytime;
	double heightcorr;
	std::size_t keyidx;

	static double interp(double frac, double v1, double v2);
	void InterpolateKeyframe(std::size_t idx, double frac, CCharShape *shape);

	// test and editing
	void ResetFrame2(TKeyframe *frame);
public:
	CKeyframe();
	bool loaded;
	bool active;

	void Init(const TVector3d& ref_position, double height_correction);
	void Init(const TVector3d& ref_position, double height_correction, CCharShape *shape);
	void InitTest(const TVector3d& ref_position, CCharShape *shape);
	void Reset();
	void Update(float timestep);
	void UpdateTest(float timestep, CCharShape *shape);
	bool Load(const std::string& dir, const std::string& filename);
	void CalcKeyframe(std::size_t idx, CCharShape *shape, const TVector3d& refpos_) const;

	// test and editing
	TKeyframe *GetFrame(std::size_t idx);
	static const std::string& GetHighlightName(std::size_t idx);
	static const std::string& GetJointName(std::size_t idx);
	static int GetNumJoints();
	void SaveTest(const std::string& dir, const std::string& filename) const;
	void CopyFrame(std::size_t prim_idx, std::size_t sec_idx);
	void AddFrame();
	std::size_t  DeleteFrame(std::size_t idx);
	void InsertFrame(std::size_t idx);
	void CopyToClipboard(std::size_t idx);
	void PasteFromClipboard(std::size_t idx);
	void ClearFrame(std::size_t idx);
	std::size_t numFrames() const { return frames.size(); }
};

extern CKeyframe TestFrame;

#endif
