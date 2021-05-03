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

#ifndef ENV_H
#define ENV_H

#include "bh.h"
#include <vector>
#include <unordered_map>

class TTexture;


struct TFog {
	GLint mode;
	float start;
	float end;
	float height;
	float color[4];
	sf::Color part_color;
	bool is_on;
};

struct TLight {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float position[4];
	bool is_on;

	void Enable(GLenum num) const;
};

struct TEnvironment {
	std::string name;
	bool high_res;
};

class CEnvironment {
private:
	std::size_t EnvID;
	TTexture* Skybox;
	std::vector<TEnvironment> locs;
	std::string lightcond[4];
	TLight default_light;
	TLight lights[4];
	TFog fog;
	TFog default_fog;

	std::unordered_map<std::string, std::size_t> EnvIndex;
	std::unordered_map<std::string, std::size_t> LightIndex;

	void ResetSkybox();
	void LoadSkybox(const std::string& EnvDir, bool high_res);
	void LoadSkyboxSide(std::size_t index, const std::string& EnvDir, const std::string& name, bool high_res);
	void ResetLight();
	void LoadLight(const std::string& EnvDir);
	void ResetFog();
	void Reset();
	std::string GetDir(std::size_t location, std::size_t light) const;
public:
	CEnvironment();
	bool LoadEnvironmentList();
	void LoadEnvironment(std::size_t loc, std::size_t light);
	void DrawSkybox(const TVector3d& pos) const;
	void SetupLight();
	void SetupFog();
	void DrawFog() const;
	const sf::Color& ParticleColor() const { return fog.part_color; }
	std::size_t GetEnvIdx(const std::string& tag) const;
	std::size_t GetLightIdx(const std::string& tag) const;
};

extern CEnvironment Env;

#endif
