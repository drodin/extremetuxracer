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
#include <map>

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

#define BF_TYPE 0x4D42             // "MB"

struct TTgaHeader {
    char tfType;
    char tfColorMapType;
    char tfImageType;
    char tfColorMapSpec[5];
    short tfOrigX;
    short tfOrigY;
    short tfWidth;
    short tfHeight;
    char tfBpp;
    char tfImageDes;
};

struct TBmpHeader {
    unsigned short  bfType;           // identifier of bmp format
    unsigned long   bfSize;           // size of file, including the headers
    unsigned short  bfReserved1;      // reserved, always 0
    unsigned short  bfReserved2;      // reserved, always 0
    unsigned long   bfOffBits;        // offset to bitmap data
};

struct TBmpInfo {
    unsigned long   biSize;           // size of info header, normally 40
    long            biWidth;          // width
    long            biHeight;         // height
    unsigned short  biPlanes;         // number of color planes, normally 1
    unsigned short  biBitCount;       // Number of bits per pixel (8 * depth)
    unsigned long   biCompression;    // type of compression, normally 0 = no compr.
    unsigned long   biSizeImage;      // size of data
    long            biXPelsPerMeter;  // normally 0
    long            biYPelsPerMeter;  // normally 0
    unsigned long   biClrUsed;        // normally 0
    unsigned long   biClrImportant;   // normally 0
};


// --------------------------------------------------------------------
//				class CImage
// --------------------------------------------------------------------

class CImage {
private:
public:
    CImage ();
	~CImage ();

	unsigned char *data;
	int nx;
    int ny;
    int depth;
    int pitch;

	void DisposeData ();

	// load:
	bool LoadPng (const char *filepath, bool mirroring);
	bool LoadPng (const char *dir, const char *filepath, bool mirroring);

	// write:
	bool ReadFrameBuffer_PPM ();
	void ReadFrameBuffer_TGA ();
	void ReadFrameBuffer_BMP ();
	void WritePPM (const char *filepath);
	void WritePPM (const char *dir, const char *filename);
	void WriteTGA (const char *filepath);
	void WriteTGA (const char *dir, const char *filename);

	// versions with explicite header
	void WriteTGA_H (const char *filepath);
	void WriteTGA_H (const char *dir, const char *filename);
	void WriteBMP (const char *filepath);
	void WriteBMP (const char *dir, const char *filename);
};

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

class TTexture {
	TTexture(const TTexture&);
	TTexture& operator=(const TTexture&);

	GLuint id;
public:

	TTexture() : id(0) {}
	~TTexture();
	bool Load(const string& filename);
	bool Load(const string& dir, const string& filename);
	bool LoadMipmap(const string& filename, bool repeatable);
	bool LoadMipmap(const string& dir, const string& filename, bool repeatable);

	void Bind();
	void Draw();
	void Draw(int x, int y, float size, Orientation orientation);
	void Draw(int x, int y, float width, float height, Orientation orientation);
	void DrawFrame(int x, int y, double w, double h, int frame, const TColor& col);
};

class CTexture {
private:
	vector<TTexture*> CommonTex;
	map<string, TTexture*> Index;
	Orientation forientation;

	void DrawNumChr (char c, int x, int y, int w, int h, const TColor& col);
public:
    CTexture ();
	~CTexture ();
	void LoadTextureList ();
	void FreeTextureList ();

	TTexture* GetTexture (size_t idx) const;
	TTexture* GetTexture (const string& name) const;
	bool BindTex (size_t idx);
	bool BindTex (const string& name);

	void Draw (size_t idx);
	void Draw (const string& name);

	void Draw (size_t idx, int x, int y, float size);
	void Draw (const string& name, int x, int y, float size);

	void Draw (size_t idx, int x, int y, int width, int height);
	void Draw (const string& name, int x, int y, int width, int height);

	void DrawFrame (size_t idx, int x, int y, double w, double h, int frame, const TColor& col);
	void DrawFrame (const string& name, int x, int y, double w, double h, int frame, const TColor& col);

	void SetOrientation (Orientation orientation);
	void DrawNumStr (const char *s, int x, int y, float size, const TColor& col);
};

extern CTexture Tex;

void ScreenshotN ();
void TGAScreenshot2 (const char *destFile);
void BMPScreenshot (const char *destFile);


#endif
