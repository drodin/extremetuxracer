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

#ifndef ETR_TYPES_H
#define ETR_TYPES_H

#include "vectors.h"

enum Orientation {
	OR_TOP = 0,			// top-orientated menu widgets
	OR_BOTTOM = 1		// bottom-orientated
};

struct TColor3 {
	uint8_t r, g, b;
	TColor3(uint8_t r_ = 0, uint8_t g_ = 0, uint8_t b_ = 0)
		: r(r_), g(g_), b(b_)
	{}
};

enum TToolMode {
	NONE,
	TUXSHAPE,
	KEYFRAME,
	TREEGEN,
	LEARN,
};

enum TGameType {
	PRACTICING,
	CUPRACING
};

enum TViewMode {
	BEHIND,
	FOLLOW,
	ABOVE,
	NUM_VIEW_MODES
};

struct TCup;
struct TPlayer;
struct TCourse;
struct TRace;
struct TCharacter;

struct TGameData {
	TToolMode toolmode;
	float time_step;
	TGameType game_type;
	bool force_treemap;
	int treesize;
	int treevar;
	int argument;
	bool finish;
	bool use_keyframe;
	double finish_brake;

	// course and race params
	TPlayer* player;
	TCharacter* character;
	size_t start_player;
	TCup* cup;
	TRace* race; // Only valid if not in practice mode
	TCourse* course;
	bool mirrorred;
	size_t location_id;
	size_t light_id;
	int snow_id;
	int wind_id;
	size_t theme_id;

	// race results (better in player.ctrl ?)
	float time;				// reached time
	int score;				// reached score
	int herring;			// catched herrings during the race
	int race_result;		// tuxlifes, only for a single race, see game_ctrl
	bool raceaborted;
};

class CControl;

#endif
