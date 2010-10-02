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

#ifndef PARTICLES_H
#define PARTICLES_H

#include "bh.h"

// --------------------------------------------------------------------
//					snow for menu screens
// --------------------------------------------------------------------

void init_ui_snow (void );
void update_ui_snow (double time_step);
void push_ui_snow (TVector2 pos );
void draw_ui_snow (void );
void make_ui_snow (TVector2 pos );
void reset_ui_snow_cursor_pos (TVector2 pos );

// --------------------------------------------------------------------
//					snow particles during race
// --------------------------------------------------------------------

void create_new_particles (TVector3 loc, TVector3 vel, int num );
void update_particles (double time_step );
void clear_particles ();
void draw_particles (CControl *ctrl );
void generate_particles (CControl *ctrl, double dtime, TVector3 pos, double speed);

// --------------------------------------------------------------------
//					snow flakes for short distances
// --------------------------------------------------------------------

#define MAX_FLAKEAREAS 6

typedef struct {
    TVector3 pt;
    float   size;
    TVector3 vel;
    TVector2 tex_min;
    TVector2 tex_max;
} TFlake;

typedef struct {
	int num_flakes;
	float xrange;
	float ytop;
	float yrange;
	float zback;
	float zrange;
	float minSize;
	float maxSize;
	float speed;
	bool  rotate_flake;
	
	float left;
	float right;
	float bottom;
	float top;
	float front;
	float back;

	TFlake *flakes;
} TFlakeArea;

class CFlakes {
private:
	TVector3 snow_lastpos;
	TFlakeArea areas[MAX_FLAKEAREAS];
	int numAreas;
	void MakeSnowFlake (int ar, int i);
	void GenerateSnowFlakes (CControl *ctrl);
	void UpdateAreas (CControl *ctrl);
	void DrawArea (int ar, CControl *ctrl);
	void CreateArea (
		int num_flakes, 
		float xrange,
		float ytop,
		float yrange,
		float zback,
		float zrange,
		float minSize,
		float maxSize,
		float speed,
		bool  rotate);
public:
	CFlakes ();
	~CFlakes ();
	void Init (int grade, CControl *ctrl);
	void Reset ();
	void Update (double timestep, CControl *ctrl);
	void Draw (CControl *ctrl);
};

// --------------------------------------------------------------------
//					snow clouds for medium distances
// --------------------------------------------------------------------
// intended for future versions

// --------------------------------------------------------------------
//					snow curtains for greater distances
// --------------------------------------------------------------------

#define MAX_CURTAIN_COLS 16
#define MAX_CURTAIN_ROWS 8
#define MAX_CURTAINS 6

typedef struct {
    TVector3 pt;
	float angle;
	float height;
	float zrandom;
} TCurtainElement;

class CCurtain {
private:
	TCurtainElement curtains[MAX_CURTAINS][MAX_CURTAIN_COLS][MAX_CURTAIN_ROWS];

	int numCols[MAX_CURTAINS];
	int numRows[MAX_CURTAINS];
	float zdist[MAX_CURTAINS];
	float size[MAX_CURTAINS];
	float speed[MAX_CURTAINS];
	float angledist[MAX_CURTAINS];
	float startangle[MAX_CURTAINS];
	float lastangle[MAX_CURTAINS];
	float minheight[MAX_CURTAINS];
	bool enabled[MAX_CURTAINS];
	int texture[MAX_CURTAINS];

	int chg[MAX_CURTAINS][MAX_CURTAIN_ROWS];	// for each row

	void GenerateCurtain (
		int nr,	
		int num_rows,
		float z_dist,
		float tex_size,
		float base_speed,
		float start_angle,
		float min_height,
		int curt_texture);

	void SetStartParams (CControl *ctrl);
	void CurtainVec (float angle, float zdist, float &x, float &z);
public:
	CCurtain ();
	~CCurtain ();
	void Init (CControl *ctrl);
	void Update (float timestep, CControl *ctrl);
	void Draw (CControl *ctrl);
	void Reset ();
};

// --------------------------------------------------------------------
//					wind
// --------------------------------------------------------------------

typedef struct {
	float minSpeed;
	float maxSpeed;
	float minChange;
	float maxChange;

	float minAngle;
	float maxAngle;
	float minAngleChange;
	float maxAngleChange;

	float topSpeed;
	float topProbability;
	float nullProbability;
} TWindParams;

class CWind {
private:
	bool windy;
	float CurrTime;
	TWindParams params;
	// if dest > curr state the modes are 1, otherwise 0
	int SpeedMode;
	int AngleMode;

	float WSpeed;
	float WAngle;
	TVector3 WVector;

	float DestSpeed;
	float DestAngle;
	float WindChange;
	float AngleChange;

	void SetParams (int grade);
	void CalcDestSpeed ();
	void CalcDestAngle ();
public:
	CWind ();
	~CWind ();

	void Update (float timestep);
	void Init (int wind_id);
	bool Windy () { return windy; }
	float Angle () { return WAngle; }
	float Speed () { return WSpeed; }
	TVector3 WindDrift () { return WVector; }
};

extern CWind Wind;

// --------------------------------------------------------------------
// 			Acess functions
// --------------------------------------------------------------------

void InitSnow (CControl *ctrl);
void UpdateSnow (double timestep, CControl *ctrl);
void DrawSnow (CControl *ctrl);
void InitWind ();
void UpdateWind (double timestep, CControl *ctrl);

#endif
