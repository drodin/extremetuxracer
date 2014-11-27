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


class CFont {
private:
	vector<sf::Font*> fonts;
	map<string, size_t> fontindex;
	Orientation forientation;

	int    curr_font;
	sf::Color curr_col;
	float  curr_size;
	float  curr_fact;		// the length factor

	void DrawText(float x, float y, const sf::String& text, size_t font, float size) const;
	void GetTextSize(const sf::String& text, float &x, float &y, size_t font, float size) const;
public:
	CFont();
	~CFont();

	void Clear();
	int  LoadFont(const string& name, const string& dir, const string& filename);
	int  LoadFont(const string& name, const string& path);
	bool LoadFontlist();
	size_t GetFontIdx(const string &name) const;
	const sf::Font& getCurrentFont() const { return *fonts[curr_font]; }
	float GetSize() const { return curr_size; }

	// properties
	void SetProps(const string &fontname, float size, const sf::Color& col);
	void SetProps(const string &fontname, float size);
	void SetColor(const sf::Color& col) { curr_col = col; }
	void SetSize(float size) { curr_size = size; }
	void SetFont(const string& fontname);
	void SetFontFromSettings();

	// auto
	int AutoSizeN(int rel_val);	// rel_val = relative size, return: autosize
	int AutoDistanceN(int rel_val);	// rel_val = relative dist

	// draw
	void DrawString(float x, float y, const sf::String &s) const; // sf::String class
	void DrawString(float x, float y, const sf::String &s, const string &fontname, float size) const;

	// metrics
	void  GetTextSize(const sf::String& text, float &x, float &y) const;
	void  GetTextSize(const sf::String& text, float &x, float &y, const string &fontname, float size) const;
	float GetTextWidth(const sf::String& text) const;
	float GetTextWidth(const sf::String& text, const string &fontname, float size) const;

	float CenterX(const char *text) const;
	void  SetOrientation(Orientation orientation) { forientation = orientation; }

	vector<string> MakeLineList(const char *source, float width);
};

extern CFont FT;

#endif
