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

#ifndef MIN
#	define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#	define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#define clamp(minimum, x, maximum) (max(min(x, maximum), minimum))

#ifndef ROUND_TO_NEAREST
#   define ROUND_TO_NEAREST(x) ((int) ((x)+0.5))
#endif

#define ANGLES_TO_RADIANS(x) (M_PI / 180.0 * (x) )
#define RADIANS_TO_ANGLES(x) (180.0 / M_PI * (x) )

#ifndef M_PI
#   define M_PI 3.1415926535
#endif

#define MAG_SQD(vec) ((vec).x * (vec).x + (vec).y * (vec).y + (vec).z * (vec).z )

#ifndef EPS
#	define EPS 1.0e-13
#endif

enum Orientation {
	OR_TOP = 0,			// top-orientated menu widgets
	OR_BOTTOM = 1		// bottom-orientated
};

#define TUX_WIDTH 0.45
#define MAX_ROLL_ANGLE 30
#define BRAKING_ROLL_ANGLE 55

#define CENTER -1
#define FIT -1

struct TVector2		{ double x, y; };
struct TVector3		{ double x, y, z; };
struct TVector4		{ double x, y, z, w; };

struct TIndex2		{ int i, j; };
struct TIndex3		{ int i, j, k; };
struct TIndex4		{ int i, j, k, l; };

struct TColor		{ double r, g, b, a; };
struct TColor3		{ double r, g, b; };

typedef double TMatrix[4][4];
struct TQuaternion	{ double x, y, z, w; };

struct TPlane		{ TVector3 nml; double d; };
struct TPolygon		{ int num_vertices; int *vertices; };
struct TSphere		{ double radius; int divisions; };
struct TRay			{ TVector3 pt; TVector3 vec; };
struct TScreenRes	{ int width, height; };

struct TPolyhedron {
    size_t num_vertices;
    size_t num_polygons;
    TVector3 *vertices;
    TPolygon *polygons;
};

struct TMaterial {
    TColor diffuse;
    TColor specular_colour;
    double specular_exp;
};

struct key_frame_t {
    double time;
    TVector3 pos;
    double yaw;
    double pitch;
    double l_shldr;
    double r_shldr;
    double l_hip;
    double r_hip;
};

struct TCollidable {
	TVector3 pt;
    double height;
    double diam;
    int tree_type;
};

struct TItem {
	TVector3 pt;
    double height;
    double diam;
    int item_type;
    int collectable;
    bool drawable;
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
	CUPRACING,
	TRAINING
};

enum TViewMode {
    BEHIND,
    FOLLOW,
    ABOVE,
    NUM_VIEW_MODES
};

enum TFrameType {
	START,
	FINISH,
	WONRACE,
	LOSTRACE,
	NUM_FRAME_TYPES
};

struct TCup2;

struct TGameData {
	TToolMode toolmode;
	double time_step;
    double secs_since_start;
	double fps;
	int timesteps;
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
	int location_id;
	int light_id;
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

#endif
