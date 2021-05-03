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

#ifndef TEXTURES_H
#define TEXTURES_H

#include "bh.h"
#include <vector>

#define TEXLOGO 0
#define SNOW_START 1
#define SNOW_TRACK 2
#define SNOW_STOP 3
#define T_TITLE 4
#define HERRING_ICON 7
#define GAUGE_OUTLINE 8
#define GAUGE_ENERGY 9
#define GAUGE_SPEED 10
#define LB_ARROWS 11
#define SPEEDMETER 12
#define LIGHT_BUTT 14
#define SNOW_BUTT 15
#define WIND_BUTT 16
#define BOTTOM_LEFT 17
#define BOTTOM_RIGHT 18
#define TOP_LEFT 19
#define TOP_RIGHT 20
#define TUXBONUS 21
#define MOUSECURSOR 22
#define SNOW_PART 23
#define T_ENERGY_MASK 25
#define T_MASK_OUTLINE 26
#define NUMERIC_FONT 27
#define MIRROR_BUTT 28
#define CHAR_BUTT 29
#define RANDOM_BUTT 30
#define T_TIME 32
#define SPEED_KNOB 35
#define CHECKBOX 37
#define CHECKMARK_SMALL 38
#define T_SNOW1 41
#define T_SNOW2 42
#define T_SNOW3 43


// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

class TTexture {
	sf::Texture texture;
	friend class CTexture;
public:
	bool Load(const std::string& filename, bool repeatable = false);
	bool Load(const std::string& dir, const std::string& filename, bool repeatable = false);
	bool Load(const std::string& dir, const char* filename, bool repeatable = false) { return Load(dir, std::string(filename), repeatable); }

	void Bind();
	void Draw();
	void Draw(int x, int y, float size);
	void Draw(int x, int y, float width, float height);
	void DrawFrame(int x, int y, int w, int h, int frame, const sf::Color& col);
};

class CTexture {
private:
	std::vector<TTexture*> CommonTex;

	void DrawNumChr(char c, int x, int y, int w, int h);
public:
	CTexture();
	~CTexture();
	bool LoadTextureList();
	void FreeTextureList();

	TTexture* GetTexture(std::size_t idx) const;
	const sf::Texture& GetSFTexture(std::size_t idx) const;
	bool BindTex(std::size_t idx);

	void Draw(std::size_t idx);
	void Draw(std::size_t idx, int x, int y, float size);
	void Draw(std::size_t idx, int x, int y, int width, int height);

	void DrawFrame(std::size_t idx, int x, int y, double w, double h, int frame, const sf::Color& col);

	void DrawNumStr(const std::string& s, int x, int y, float size, const sf::Color& col);
};

extern CTexture Tex;


#endif
