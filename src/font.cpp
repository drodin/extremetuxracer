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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "font.h"
#include "spx.h"
#include "ogl.h"
#include "winsys.h"
#include "gui.h"

#define USE_UNICODE 1

// --------------------------------------------------------------------
// First some common function used for textboxes and called by
// CFont::MakeLineList. This bundle of functions generates
// a vector<string> from a textstring and adapts the lines to the textbox

static void MakeWordList(vector<string>& wordlist, const char *s) {
	size_t start = 0;
	for (size_t i = 0; s[i] != '\0'; i++) {
		if (s[i] == ' ') {
			if (i != start)
				wordlist.emplace_back(s + start, i - start);
			while (s[i+1] == ' ')
				i++;
			start = i+1;
		}
	}
	if (s[start] != '0')
		wordlist.emplace_back(s + start);
}

static size_t MakeLine(size_t first, const vector<string>& wordlist, vector<string>& linelist, float width) {
	if (first >= wordlist.size()) return wordlist.size()-1;

	size_t last = first;
	float lng = 0;

	float spacelng = FT.GetTextWidth("a a") - FT.GetTextWidth("aa");
	while (last < wordlist.size()) {
		float wordlng = FT.GetTextWidth(wordlist[last]);
		lng += wordlng;
		lng += spacelng;
		if (lng >= width && first != last) // If first == last, we write beyond line
			break;
		last++;
	}

	string line;
	for (size_t j=first; j<last; j++) {
		line += wordlist[j];
		if (j < last)
			line += ' ';
	}
	linelist.push_back(line);
	return last-1;
}



// --------------------------------------------------------------------
//				CFont
// --------------------------------------------------------------------

CFont FT;

CFont::CFont() {
	// setting default values
	curr_col.r = 0.0;	// default color: black
	curr_col.g = 0.0;
	curr_col.b = 0.0;
	curr_col.a = 1.0;	// default: no transparency
	curr_size   = 20;	// default size: 20 px
	curr_fact   = 0;
	curr_font   = 0;
}

CFont::~CFont() {
	Clear();
}

void CFont::Clear() {
	for (size_t i = 0; i < fonts.size(); i++)
		delete fonts[i];
	fonts.clear();
	fontindex.clear();
}

// --------------------------------------------------------------------
//					public
// --------------------------------------------------------------------

int CFont::LoadFont(const string& name, const string& path) {
	fonts.push_back(new sf::Font());
	if (!fonts.back()->loadFromFile(path)) {
		Message("Failed to open font");
		return -1;
	}

	fontindex[name] = fonts.size()-1;
	return (int)fonts.size()-1;
}

int CFont::LoadFont(const string& name, const string& dir, const string& filename) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadFont(name, path);
}

bool CFont::LoadFontlist() {
	CSPList list;
	if (!list.Load(param.font_dir, "fonts.lst")) {
		fonts.push_back(new sf::Font()); // Insert an empty font, otherwise ETR will crash
		return false;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		string fontfile = SPStrN(*line, "file");
		string name = SPStrN(*line, "name");

		int ftidx = LoadFont(name, param.font_dir, fontfile);
		if (ftidx < 0) {
			Message("couldn't load font", name);
		}
	}
	return true;
}

size_t CFont::GetFontIdx(const string &name) const {
	return fontindex.at(name);
}

void CFont::SetProps(const string &fontname, float size, const sf::Color& col) {
	SetProps(fontname, size);
	curr_col  = col;
}

void CFont::SetProps(const string &fontname, float size) {
	curr_font = (int)GetFontIdx(fontname);
	curr_size = size;
}

void CFont::SetFont(const string& fontname) {
	try {
		curr_font = (int)fontindex[fontname];
	} catch (...) {
		curr_font = -1;
	}

	if (fontname == "pc20") curr_fact = 1.25;
	else curr_fact = 1.0;
}

void CFont::SetFontFromSettings() {
	if (param.use_papercut_font > 0)
		SetFont("pc20");
	else
		SetFont("bold");
}

// -------------------- auto ------------------------------------------

int CFont::AutoSizeN(int rel_val) {
	float size = (rel_val + 2) * 4;
	size *= curr_fact;
	size *= Winsys.scale;
	SetSize(size);
	return (int)size;
}

int CFont::AutoDistanceN(int rel_val) {
	float fact = (rel_val + 5) * 0.2;
	float dist = curr_size * fact;
	return (int) dist;
}

// -------------------- draw (x, y, text) -----------------------------

void CFont::DrawText(float x, float y, const sf::String& text, size_t font, float size) const {
	if (font >= fonts.size()) return;

	sf::Text temp(text, *fonts[font], size);
	if (x == CENTER)
		x = (Winsys.resolution.width - temp.getLocalBounds().width) / 2;
	temp.setPosition(x, y);
	temp.setColor(curr_col);
	Winsys.draw(temp);
}

void CFont::DrawString(float x, float y, const sf::String &s) const {
	DrawText(x, y, s, curr_font, curr_size);
}

void CFont::DrawString(float x, float y, const sf::String& s, const string &fontname, float size) const {
	DrawText(x, y, s, GetFontIdx(fontname), size);
}


// --------------------- metrics --------------------------------------

void CFont::GetTextSize(const sf::String& text, float &x, float &y, size_t font, float size) const {
	if (font >= fonts.size()) { x = 0; y = 0; return; }

	sf::Text temp(text, *fonts[font], size);
	x = temp.getGlobalBounds().width;
	y = temp.getGlobalBounds().height;
}

void CFont::GetTextSize(const sf::String& text, float &x, float &y, const string &fontname, float size) const {
	GetTextSize(text, x, y, GetFontIdx(fontname), size);
}

void CFont::GetTextSize(const sf::String& text, float &x, float &y) const {
	GetTextSize(text, x, y, curr_font, curr_size);
}

float CFont::GetTextWidth(const sf::String& text) const {
	float x, y;
	GetTextSize(text, x, y, curr_font, curr_size);
	return x;
}

float CFont::GetTextWidth(const sf::String& text, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx(fontname);
	float x, y;
	GetTextSize(text, x, y, temp_font, size);
	return x;
}

vector<string> CFont::MakeLineList(const char *source, float width) {
	vector<string> wordlist;
	MakeWordList(wordlist, source);
	vector<string> linelist;

	for (size_t last = 0; last < wordlist.size();)
		last = MakeLine(last, wordlist, linelist, width)+1;

	return linelist;
}
