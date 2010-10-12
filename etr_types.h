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

#define COURSE_VERTEX(x,y) MakeVector ( (double)(x)/(nx-1.)*course_width, \
                       ELEV((x),(y)), -(double)(y)/(ny-1.)*course_length ) 

#ifndef MIN
#	define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#	define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef ROUND_TO_NEAREST
#   define ROUND_TO_NEAREST(x) ((int) ((x)+0.5))
#endif

#define ANGLES_TO_RADIANS(x) (M_PI / 180.0 * (x) )
#define RADIANS_TO_ANGLES(x) (180.0 / M_PI * (x) )

#ifndef M_PI
#   define M_PI 3.1415926535
#endif

#ifndef EPS
#	define EPS 1.0e-13
#endif

#define OR_TOP 0			// top-orientated menu widgets
#define OR_BOTTOM 1			// bottom-orientated

#define TUX_WIDTH 0.45 			
#define MAX_ROLL_ANGLE 30
#define BRAKING_ROLL_ANGLE 55
#define BUFF_LEN 1024

typedef double scalar_t;

typedef struct {double x, y;}			TVector2;
typedef struct {double x, y, z;}		TVector3;
typedef struct {double x, y, z, w;} 	TVector4;

typedef struct {int i, j;}				TIndex2;
typedef struct {int i, j, k;}			TIndex3;
typedef struct {int i, j, k, l;}		TIndex4;

typedef struct {double r, g, b, a;}		TColor;
typedef struct {double r, g, b;}		TColor3;
typedef struct {double x, y, z;}		TTuple;
typedef struct {double a, b, c, d;}		TTuple4;

typedef double							TMatrix[4][4]; 
typedef double							TMatrixGL[16];
typedef struct {double x, y, z, w;}		TQuaternion;

typedef struct {TVector3 nml; double d;}			TPlane;
typedef struct {int num_vertices; int *vertices;} 	TPolygon;
typedef struct {double radius; int divisions;}		TSphere;
typedef struct {TVector3 pt; TVector3 vec;}			TRay;

typedef struct {
    int num_vertices;
    int num_polygons;
    TVector3 *vertices;
    TPolygon *polygons;
} TPolyhedron;

typedef struct {
    TColor diffuse;
    TColor specular_colour;
    double specular_exp;
} TMaterial;

typedef struct {
    double time;
    TVector3 pos;
    double yaw;      
    double pitch;    
    double l_shldr;
    double r_shldr;
    double l_hip;
    double r_hip;
} key_frame_t; 

typedef struct {
	TVector3 pt;
    double height;
    double diam;
    int tree_type;
} TCollidable;

typedef struct {
	TVector3 pt;
    double height;
    double diam;
    int item_type;
    int collectable;
    bool drawable;
} TItem; 

typedef struct {
	int left;
	int top;
	int width;
	int height;
} TRect;

typedef enum {
    NO_MODE,
    SPLASH,
    GAME_TYPE_SELECT,
    EVENT_SELECT,
	EVENT,
    RACE_SELECT,
    LOADING,
    INTRO,
    RACING,
    GAME_OVER,
    PAUSED,
    RESET,
    CREDITS,
	GAME_CONFIG,
	HELP,
    NUM_GAME_MODES
} TGameMode;

typedef enum {
	PRACTICING,
	CUPRACING,
	TRAINING
} TGameType;

typedef enum {
    BEHIND,
    FOLLOW,
    ABOVE,
    NUM_VIEW_MODES
} TViewMode;

typedef struct {	
    TGameMode mode;              
    TGameMode prev_mode;         
	double time_step;            
    double secs_since_start;  
	double fps;
	int timesteps;
	TGameType GameType;
	bool force_treemap;
	int treesize;
	int treevar;

	// course and race params 
	int cup_id;
	int race_id;
	int numraces;
	int mirror_id;                           
	int char_id;                           
	int course_id;
	int location_id;
	int light_id;
	int snow_id;
	int wind_id;

	// requirements
	TIndex3 herring_req;	// 3 levels of needed herrings
	TVector3 time_req;		// 3 levels of allowed time

	// race results (better in player.ctrl ?)
	double time;			// reached time
	int score;				// reached score
	int herring;			// catched herrings during the race
	int race_result;		// tuxlifes, only for a single race, see game_ctrl
    bool raceaborted;        
} TGameData;

#endif
