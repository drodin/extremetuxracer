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
#include "ft_font.h"
#include "spx.h"

// --------------------------------------------------------------------
//		CFont
// --------------------------------------------------------------------

#define MAX_FONTS 36

class CFont {
private:
	FTFont *fonts[MAX_FONTS];
	int numFonts;
	string fontindex;
	int forientation;

	int    curr_font;
	TColor curr_col;
	float  curr_size;
	float  curr_fact;		// the length factor

	wstring UnicodeStr  (const char* s);

public:
	CFont ();

	void Clear ();
	int  LoadFont (string name, const char *dir, const char *filename);
	int  LoadFont (string name, const char *path);
	bool LoadFontlist ();
	int  GetFontIdx (const string &name);

	// properties
 	void SetProps   (const string &fontname, float size, TColor col);
	void SetProps   (const string &fontname, float size);
	void SetColor   (float r, float g, float b, float a);
	void SetColor   (TColor col);
	void SetSize    (float size);
	void SetFont    (string fontname);

	// auto
	int AutoSizeN     (int rel_val);	// rel_val = relative size, return: autosize
	int AutoDistanceN (int rel_val);	// rel_val = relative dist
	
	// draw
	void DrawText   (float x, float y, const char *text);		// normal char*
	void DrawText   (float x, float y, const wchar_t *text);	// wide char*
	void DrawString (float x, float y, const string &s);		// string class


	void DrawText   (float x, float y, const char *text, const string &fontname, float size);
	void DrawText   (float x, float y, const wchar_t *text, const string &fontname, float size);
	void DrawString (float x, float y, const string &s, const string &fontname, float size);

	// metrics
	void  GetTextSize  (const char *text, float &x, float &y);
	void  GetTextSize  (const char *text, float &x, float &y, const string &fontname, float size);
	float GetTextWidth (const char *text);
	float GetTextWidth (const string text);
	float GetTextWidth (const wchar_t *text);
	float GetTextWidth (const char *text, const string &fontname, float size);
	float GetTextWidth (const wchar_t *text, const string &fontname, float size);

	float CenterX        (const char *text);
	void  SetOrientation (int orientation);

	void MakeLineList (const char *source, CSPList *dest, float width);
};

extern CFont FT;

#endif

