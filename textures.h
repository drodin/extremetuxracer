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

#define MAX_COMMON_TEX 128

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
#define ENV_MAP 24
#define T_ENERGY_MASK 25
#define T_MASK_OUTLINE 26
#define NUMERIC_FONT 27
#define MIRROR_BUTT 28
#define CHAR_BUTT 29
#define RANDOM_BUTT 30
#define T_YELLHERRING 31
#define T_TIME 32
#define STARS 33
#define HERRING_ICON2 34
#define SPEED_KNOB 35
#define CUPICON 36
#define CHECKBOX 37
#define CHECKMARK_SMALL 38
#define CHECKMARK 39

#define T_WIDGETS 40
#define T_SNOW1 41
#define T_SNOW2 42
#define T_SNOW3 43
#define WORD_SPACE 6
#define LETTER_SPACE -3



// --------------------------------------------------------------------
//				class CImage
// --------------------------------------------------------------------

class CImage {
private:
	bool status;
public:
    CImage ();
	~CImage ();
	bool LoadPng (const char *filepath, bool mirroring);
	bool LoadPng (const char *dir, const char *filepath, bool mirroring);
	void WritePPM (const char *filepath);
	void WritePPM (const char *dir, const char *filename);
	bool LoadFrameBuffer ();
		
    int nx;	
    int ny; 
    int depth;
    int pitch;
    unsigned char *data;
};

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

#define TEX_WIDGETS 0
#define TEX_NUMERIC_FONT 1
#define TEX_LOGO 2

class CTexture {
private:
	GLuint CommonTex [MAX_COMMON_TEX];
	int numTextures;
	string TextureIndex;
	int forientation;
	
	void DrawNumChr (char c, int x, int y, int w, int h, TColor col);
public:
    CTexture ();
	~CTexture ();
	int LoadTexture (const char *filename);
	int LoadTexture (const char *dir, const char *filename);
	int LoadTexture (const string dir, const string filename);
	int LoadMipmapTexture (const char *filename, bool repeatable);
	int LoadMipmapTexture (const char *dir, const char *filename, bool repeatable);
	void LoadTextureList ();
	void FreeTextureList ();

	GLuint TexID (int idx);
	GLuint TexID (string name);
	bool BindTex (int idx);
	bool BindTex (string name);

	void DrawDirect (GLuint texid);
	void Draw (int idx);
	void Draw (string name);

	void DrawDirect (GLuint texid, int x, int y, float size);
	void Draw (int idx, int x, int y, float size);
	void Draw (string name, int x, int y, float size);

	void DrawDirect (GLuint texid, int x, int y, float width, float height);
	void Draw (int idx, int x, int y, int width, int height);
	void Draw (string name, int x, int y, int width, int height);

	void DrawDirectFrame (GLuint texid, int x, int y, double w, double h, int frame, TColor col);
	void DrawFrame (int idx, int x, int y, double w, double h, int frame, TColor col);
	void DrawFrame (string name, int x, int y, double w, double h, int frame, TColor col);

	void SetOrientation (int orientation);
	void DrawNumStr (const char *s, int x, int y, float size, TColor col);
};

extern CTexture Tex;

void ScreenshotN ();


#endif
