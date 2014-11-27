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

#ifndef TEXTURES_H
#define TEXTURES_H

#include "bh.h"
#include <vector>

#define TEXLOGO 0
#define SNOW_START 1
#define SNOW_TRACK 2
#define SNOW_STOP 3
#define T_TITLE 4
#define T_TITLE_SMALL 5
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
	bool Load(const string& filename, bool repeatable = false);
	bool Load(const string& dir, const string& filename, bool repeatable = false);
	bool Load(const string& dir, const char* filename, bool repeatable = false) { return Load(dir, string(filename), repeatable); }

	void Bind();
	void Draw();
	void Draw(int x, int y, float size, Orientation orientation);
	void Draw(int x, int y, float width, float height, Orientation orientation);
	void DrawFrame(int x, int y, int w, int h, int frame, const sf::Color& col);
};

class CTexture {
private:
	vector<TTexture*> CommonTex;
	Orientation forientation;

	void DrawNumChr(char c, int x, int y, int w, int h);
public:
	CTexture();
	~CTexture();
	bool LoadTextureList();
	void FreeTextureList();

	TTexture* GetTexture(size_t idx) const;
	const sf::Texture& GetSFTexture(size_t idx) const;
	bool BindTex(size_t idx);

	void Draw(size_t idx);
	void Draw(size_t idx, int x, int y, float size);
	void Draw(size_t idx, int x, int y, int width, int height);

	void DrawFrame(size_t idx, int x, int y, double w, double h, int frame, const sf::Color& col);

	void SetOrientation(Orientation orientation);
	void DrawNumStr(const string& s, int x, int y, float size, const sf::Color& col);
};

extern CTexture Tex;

void ScreenshotN();


#endif
