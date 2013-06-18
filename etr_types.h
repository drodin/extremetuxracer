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

#ifndef M_PI
#	define M_PI 3.1415926535
#endif

#ifndef EPS
#	define EPS 1.0e-13
#endif

#define MAG_SQD(vec) ((vec).x * (vec).x + (vec).y * (vec).y + (vec).z * (vec).z)


enum Orientation {
	OR_TOP = 0,			// top-orientated menu widgets
	OR_BOTTOM = 1		// bottom-orientated
};

struct TVector2	{
	double x, y;
	TVector2(double _x = 0.0, double _y = 0.0)
		: x(_x), y(_y)
	{}
};
struct TVector3 : public TVector2 {
	double z;
	TVector3(double _x = 0.0, double _y = 0.0, double _z = 0.0)
		: TVector2(_x, _y), z(_z)
	{}
};
struct TVector4 : public TVector3 {
	double w;
	TVector4(double _x = 0.0, double _y = 0.0, double _z = 0.0, double _w = 0.0)
		: TVector3(_x, _y, _z), w(_w)
	{}
};

struct TIndex2 {
	int i, j;
	TIndex2(int i_ = 0, int j_ = 0)
		: i(i_), j(j_)
	{}
};
struct TIndex3 : public TIndex2 {
	int k;
	TIndex3(int i_ = 0, int j_ = 0, int k_ = 0)
		: TIndex2(i_, j_), k(k_)
	{}
};
struct TIndex4 : public TIndex3 {
	int l;
	TIndex4(int i_ = 0, int j_ = 0, int k_ = 0, int l_ = 0)
		: TIndex3(i_, j_, k_), l(l_)
	{}
};

struct TColor3 {
	double r, g, b;
	TColor3(double r_ = 0, double g_ = 0, double b_ = 0)
		: r(r_), g(g_), b(b_)
	{}
};
struct TColor : public TColor3 {
	double a;
	TColor(double r_ = 0, double g_ = 0, double b_ = 0, double a_ = 0)
		: TColor3(r_, g_, b_), a(a_)
	{}
};

typedef double TMatrix[4][4];
typedef TVector4 TQuaternion;

struct TPlane		{ TVector3 nml; double d; };
struct TPolygon		{ int num_vertices; int *vertices; };
struct TSphere		{ double radius; int divisions; };
struct TRay			{ TVector3 pt; TVector3 vec; };

struct TPolyhedron {
    size_t num_vertices;
    size_t num_polygons;
    TVector3 *vertices;
    TPolygon *polygons;
};

struct TRect {
	int left;
	int top;
	int width;
	int height;
};

struct TArea {
	int left;
	int right;
	int top;
	int bottom;
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

struct TCup2;

struct TGameData {
	TToolMode toolmode;
	double time_step;
	TGameType game_type;
	bool force_treemap;
	int treesize;
	int treevar;
	int argument;
	string group_arg;
	string dir_arg;
	string file_arg;
	int loopdelay;
	bool finish;
	bool use_keyframe;
	double finish_brake;

	// course and race params
	size_t player_id;
	size_t start_player;
	TCup2* cup;
	size_t race_id;
	bool mirror_id;
	size_t char_id;
	size_t course_id;
	size_t location_id;
	size_t light_id;
	int snow_id;
	int wind_id;
	size_t theme_id;

	// requirements
	TIndex3 herring_req;	// 3 levels of needed herrings
	TVector3 time_req;		// 3 levels of allowed time

	// race results (better in player.ctrl ?)
	double time;			// reached time
	int score;				// reached score
	int herring;			// catched herrings during the race
	int race_result;		// tuxlifes, only for a single race, see game_ctrl
    bool raceaborted;
};

class CControl;

#endif
