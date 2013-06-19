/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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

#ifndef FONT_H
#define FONT_H

#include "bh.h"
#include <vector>
#include <map>

// --------------------------------------------------------------------
//		CFont
// --------------------------------------------------------------------

#define MAX_FONTS 36

class FTFont;
class CSPList;


class CFont {
private:
	vector<FTFont*> fonts;
	map<string, size_t> fontindex;
	Orientation forientation;

	int    curr_font;
	TColor curr_col;
	float  curr_size;
	float  curr_fact;		// the length factor

	static wstring UnicodeStr(const char* s);
	void DrawText(float x, float y, const char *text, size_t font, float size) const;
	void DrawText(float x, float y, const wchar_t *text, size_t font, float size) const;
	void GetTextSize(const char *text, float &x, float &y, size_t font, float size) const;
	void GetTextSize(const wchar_t *text, float &x, float &y, size_t font, float size) const;

public:
	CFont ();
	~CFont ();

	void Clear ();
	int  LoadFont (const string& name, const char *dir, const char *filename);
	int  LoadFont (const string& name, const char *path);
	bool LoadFontlist ();
	size_t GetFontIdx (const string &name) const;

	// properties
	void SetProps   (const string &fontname, float size, const TColor& col);
	void SetProps   (const string &fontname, float size);
	void SetColor   (float r, float g, float b, float a);
	void SetColor   (const TColor& col);
	void SetSize    (float size);
	void SetFont    (const string& fontname);

	// auto
	int AutoSizeN     (int rel_val);	// rel_val = relative size, return: autosize
	int AutoDistanceN (int rel_val);	// rel_val = relative dist

	// draw
	void DrawText   (float x, float y, const char *text) const;		// normal char*
	void DrawText   (float x, float y, const wchar_t *text) const;	// wide char*
	void DrawString (float x, float y, const string &s) const;		// string class
	void DrawString (float x, float y, const wstring &s) const;		// wstring class


	void DrawText   (float x, float y, const char *text, const string &fontname, float size) const;
	void DrawText   (float x, float y, const wchar_t *text, const string &fontname, float size) const;
	void DrawString (float x, float y, const string &s, const string &fontname, float size) const;
	void DrawString (float x, float y, const wstring &s, const string &fontname, float size) const;

	// metrics
	void  GetTextSize  (const char *text, float &x, float &y) const;
	void  GetTextSize  (const char *text, float &x, float &y, const string &fontname, float size) const;
	float GetTextWidth (const char *text) const;
	float GetTextWidth (const string& text) const;
	float GetTextWidth (const wchar_t *text) const;
	float GetTextWidth (const char *text, const string &fontname, float size) const;
	float GetTextWidth (const wchar_t *text, const string &fontname, float size) const;

	float CenterX        (const char *text) const;
	void  SetOrientation (Orientation orientation);

	vector<string> MakeLineList (const char *source, float width);
};

extern CFont FT;

#endif
