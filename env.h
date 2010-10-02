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

#ifndef ENV_H
#define ENV_H

#include "bh.h"

#define MAX_LOCATIONS 32

typedef struct {
	string name;
} TLocInfo;

typedef struct {
	string name;
} TLightCond;

typedef struct {
    bool is_on;
	GLint mode;
    float start;
    float end;
	float height;
    float color[4];
	float part_color[4]; 
} TFog;

typedef struct {
	bool is_on;
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];	
} TLight;

class CEnvironment {
private:
	int EnvID;
	GLuint Skybox[6];
	TLocInfo locs [MAX_LOCATIONS];
	int numLocs;
	TLightCond lightcond[4];
	TLight default_light;
	TLight lights[4];
	TFog fog;
	TFog default_fog;

	TPlane leftclip; 
	TPlane rightclip;
	TPlane farclip;
	TPlane bottomclip;

	string EnvIndex;
	string LightIndex;

	void ResetSkybox ();
	void LoadSkybox ();
	void ResetLight ();
	void LoadLight ();
	void ResetFog ();
	string EnvDir;
	void Reset ();
	string GetDir (int location, int light);
public:
	CEnvironment ();
	~CEnvironment ();
	bool LoadEnvironmentList ();
	bool LoadEnvironment (int loc, int light);
	void DrawSkybox (TVector3 pos);
	void SetupLight ();
	void SetupFog ();
	void DrawFog ();
	TColor ParticleColor ();
	int GetEnvIdx (const string tag);
	int GetLightIdx (const string tag);
};

extern CEnvironment Env;

#endif
