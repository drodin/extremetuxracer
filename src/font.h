/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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

#ifndef FONT_H
#define FONT_H

#include "bh.h"
#include <vector>
#include <unordered_map>

// --------------------------------------------------------------------
//		CFont
// --------------------------------------------------------------------


class CFont {
private:
	std::vector<sf::Font*> fonts;
	std::unordered_map<std::string, std::size_t> fontindex;

	int curr_font;
	sf::Color curr_col;
	unsigned int curr_size;
	float curr_fact;		// the length factor

	void DrawText(float x, float y, const sf::String& text, std::size_t font, unsigned int size) const;
	void GetTextSize(const sf::String& text, float &x, float &y, std::size_t font, unsigned int size) const;
public:
	CFont();
	~CFont();

	void Clear();
	int  LoadFont(const std::string& name, const std::string& dir, const std::string& filename);
	int  LoadFont(const std::string& name, const std::string& path);
	bool LoadFontlist();
	std::size_t GetFontIdx(const std::string &name) const;
	const sf::Font& getCurrentFont() const { return *fonts[curr_font]; }
	unsigned int GetSize() const { return curr_size; }

	// properties
	void SetProps(const std::string &fontname, unsigned int size, const sf::Color& col);
	void SetProps(const std::string &fontname, unsigned int size);
	void SetColor(const sf::Color& col) { curr_col = col; }
	void SetSize(unsigned int size) { curr_size = size; }
	void SetFont(const std::string& fontname);
	void SetFontFromSettings();

	// auto
	unsigned int AutoSizeN(int rel_val);	// rel_val = relative size, return: autosize
	int AutoDistanceN(int rel_val) const;	// rel_val = relative dist

	// draw
	void DrawString(float x, float y, const sf::String &s) const; // sf::String class
	void DrawString(float x, float y, const sf::String &s, const std::string &fontname, unsigned int size) const;

	// metrics
	void  GetTextSize(const sf::String& text, float &x, float &y) const;
	void  GetTextSize(const sf::String& text, float &x, float &y, const std::string &fontname, unsigned int size) const;
	float GetTextWidth(const sf::String& text) const;
	float GetTextWidth(const sf::String& text, const std::string &fontname, unsigned int size) const;

	static std::vector<std::string> MakeLineList(const char *source, float width);
};

extern CFont FT;

#endif
