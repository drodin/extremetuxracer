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
#include <vector>
#include <map>

class TTexture;


struct TFog {
	bool is_on;
	GLint mode;
	float start;
	float end;
	float height;
	float color[4];
	TColor part_color;
};

struct TLight {
	bool is_on;
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];

	void Enable(GLenum num) const;
};

struct TEnvironment {
	string name;
	bool high_res;
};

class CEnvironment {
private:
	size_t EnvID;
	TTexture* Skybox;
	vector<TEnvironment> locs;
	string lightcond[4];
	TLight default_light;
	TLight lights[4];
	TFog fog;
	TFog default_fog;

	map<string, size_t> EnvIndex;
	map<string, size_t> LightIndex;

	void ResetSkybox ();
	void LoadSkybox(const string& EnvDir, bool high_res);
	void LoadSkyboxSide(size_t index, const string& EnvDir, const string& name, bool high_res);
	void ResetLight ();
	void LoadLight (const string& EnvDir);
	void ResetFog ();
	void Reset ();
	string GetDir (size_t location, size_t light) const;
public:
	CEnvironment ();
	bool LoadEnvironmentList ();
	void LoadEnvironment (size_t loc, size_t light);
	void DrawSkybox (const TVector3d& pos);
	void SetupLight ();
	void SetupFog ();
	void DrawFog ();
	const TColor& ParticleColor () const { return fog.part_color; }
	size_t GetEnvIdx (const string& tag) const;
	size_t GetLightIdx (const string& tag) const;
};

extern CEnvironment Env;

#endif
