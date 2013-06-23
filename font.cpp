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
#include "ft_font.h"
#include "spx.h"
#include "winsys.h"

#define USE_UNICODE 1

// --------------------------------------------------------------------
// First some common function used for textboxes and called by
// CFont::MakeLineList. This bundle of functions generates
// a vector<string> from a textstring and adapts the lines to the textbox

static void MakeWordList (vector<string>& wordlist, const char *s) {
	size_t start = 0;
	for(size_t i = 0; s[i] != '\0'; i++) {
		if(s[i] == ' ')
		{
			if(i != start)
				wordlist.push_back(string(s+start, i-start));
			while(s[i+1] == ' ')
				i++;
			start = i+1;
		}
	}
	if(s[start] != '0')
		wordlist.push_back(string(s+start));
}

static size_t MakeLine (size_t first, const vector<string>& wordlist, vector<string>& linelist, float width) {
	if (first >= wordlist.size()) return wordlist.size()-1;

	size_t last = first;
	float lng = 0;

	float spacelng = FT.GetTextWidth("a a") - FT.GetTextWidth("aa");;
	while(last < wordlist.size()) {
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
		if(j < last)
			line += ' ';
	}
	linelist.push_back(line);
	return last-1;
}



// --------------------------------------------------------------------
//				CFont
// --------------------------------------------------------------------

CFont FT;

CFont::CFont () {
	forientation = OR_TOP;

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

void CFont::Clear () {
	for(size_t i = 0; i < fonts.size(); i++)
		delete fonts[i];
	fonts.clear();
	fontindex.clear();
}

// --------------------------------------------------------------------
//				private
// --------------------------------------------------------------------

wstring CFont::UnicodeStr (const char *s) {
	size_t len = strlen (s);
	wstring res;
	res.resize(len);

	for (size_t i=0, j=0; i < len; ++i, ++j) {
		wchar_t ch = ((const unsigned char *)s)[i];
		if (ch >= 0xF0) {
			ch  =  (wchar_t) (s[i] & 0x07)   << 18;
			ch |=  (wchar_t) (s[++i] & 0x3F) << 12;
			ch |=  (wchar_t) (s[++i] & 0x3F) << 6;
			ch |=  (wchar_t) (s[++i] & 0x3F);
		} else
			if (ch >= 0xE0) {
				ch  =  (wchar_t) (s[i] & 0x0F)   << 12;
				ch |=  (wchar_t) (s[++i] & 0x3F) << 6;
			ch |=  (wchar_t) (s[++i] & 0x3F);
			} else
				if (ch >= 0xC0) {
					ch  =  (wchar_t) (s[i] & 0x1F) << 6;
					ch |=  (wchar_t) (s[++i] & 0x3F);
				}
				res[j] = ch;
	}
	return res;
}

// --------------------------------------------------------------------
//					public
// --------------------------------------------------------------------

int CFont::LoadFont (const string& name, const char *path) {
	fonts.push_back(new FTGLPixmapFont (path));
//	fonts.push_back(new FTGLTextureFont (path));
	if (fonts.back()->Error()) {
		Message ("Failed to open font");
		return -1;
	}
	fonts.back()->FaceSize (18);
	fonts.back()->CharMap (ft_encoding_unicode);

	fontindex[name] = fonts.size()-1;
	return (int)fonts.size()-1;
}

int CFont::LoadFont (const string& name, const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadFont (name, path.c_str());
}

bool CFont::LoadFontlist () {
	CSPList list(MAX_FONTS);
	if (!list.Load ( param.font_dir, "fonts.lst")) return false;
	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		string fontfile = SPStrN (line, "file", "");
		string name = SPStrN (line, "name", "");

		int ftidx = LoadFont (name, param.font_dir.c_str(), fontfile.c_str());
		if (ftidx < 0) {
			Message ("couldn't load font", name);
		}
	}
	return true;
}

size_t CFont::GetFontIdx (const string &name) const {
	return fontindex.at(name);
}

void CFont::SetProps (const string &fontname, float size, const TColor& col) {
	SetProps(fontname, size);
	curr_col  = col;
}

void CFont::SetProps (const string &fontname, float size) {
	if (!fontname.empty()) curr_font = (int)GetFontIdx (fontname);
	if (size > 0) curr_size = size;
}

void CFont::SetColor (float r, float g, float b, float a) {
	curr_col.r = r;
	curr_col.g = g;
	curr_col.b = b;
	curr_col.a = a;
}

void CFont::SetColor (const TColor& col) {
	curr_col = col;
}

void CFont::SetSize (float size) {
	curr_size = size;
}

void CFont::SetFont (const string& fontname) {
	try {
		curr_font = (int)fontindex[fontname];
	} catch(...) {
		curr_font = -1;
	}

	if (fontname == "pc20") curr_fact = 1.25;
		else curr_fact = 1.0;
}

// -------------------- auto ------------------------------------------

int CFont::AutoSizeN (int rel_val) {
	double size = (rel_val + 2) * 4;
	size *= curr_fact;
	size *= Winsys.scale;
	SetSize (size);
	return (int)size;
}

int CFont::AutoDistanceN (int rel_val) {
	double fact = (rel_val + 5) * 0.2;
	double dist = curr_size * fact;
	return (int) dist;
}

// -------------------- draw (x, y, text) -----------------------------

void CFont::DrawText(float x, float y, const char *text, size_t font, float size) const {
	if (font >= fonts.size()) return;

	glPushMatrix();
	fonts[font]->FaceSize ((int)size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);

	float left;
	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - GetTextWidth (text)) / 2;
	if (left < 0) left = 0;

	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(Winsys.resolution.height - curr_size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	fonts[font]->Render (text);
	glPopMatrix();
}

void CFont::DrawText(float x, float y, const wchar_t *text, size_t font, float size) const {
	if (font >= fonts.size()) return;

	glPushMatrix();
	fonts[font]->FaceSize ((int)size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);

	float left;
	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - GetTextWidth (text)) / 2;
	if (left < 0) left = 0;

	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(Winsys.resolution.height - curr_size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	fonts[font]->Render (text);
	glPopMatrix();
}

void CFont::DrawText (float x, float y, const char *text) const {
#if USE_UNICODE
	DrawString(x, y, UnicodeStr(text));
#else
	DrawText(x, y, text, curr_font);
#endif
}

void CFont::DrawText (float x, float y, const wchar_t *text) const {
	DrawText(x, y, text, curr_font, curr_size);
}

void CFont::DrawString (float x, float y, const string &s) const {
	DrawText (x, y, s.c_str());
}

void CFont::DrawString (float x, float y, const wstring &s) const {
	DrawText (x, y, s.c_str());
}

void CFont::DrawText(float x, float y, const char *text, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx (fontname);
#if USE_UNICODE
	DrawText(x, y, UnicodeStr(text).c_str(), temp_font, size);
#else
	DrawText(x, y, text, temp_font, size);
#endif
}

void CFont::DrawText
		(float x, float y, const wchar_t *text, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx (fontname);
	DrawText(x, y, text, temp_font, size);
}

void CFont::DrawString (
		float x, float y, const string &s, const string &fontname, float size) const {
	DrawText (x, y, s.c_str(), fontname, size);
}

void CFont::DrawString (
		float x, float y, const wstring &s, const string &fontname, float size) const {
	DrawText (x, y, s.c_str(), fontname, size);
}

// --------------------- metrics --------------------------------------

void CFont::GetTextSize (const wchar_t *text, float &x, float &y, size_t font, float size) const {
	if (font >= fonts.size()) { x = 0; y = 0; return; }

	float llx, lly, llz, urx, ury, urz;
	fonts[font]->FaceSize ((int)size);
	fonts[font]->BBox (text, llx, lly, llz, urx, ury, urz);
	x = urx - llx;
	y = ury - lly;
}

void CFont::GetTextSize (const char *text, float &x, float &y, size_t font, float size) const {
#if USE_UNICODE
	GetTextSize(UnicodeStr(text).c_str(), x, y, font, size);
#else
	if (font >= fonts.size()) { x = 0; y = 0; return; }

	float llx, lly, llz, urx, ury, urz;
	fonts[font]->FaceSize ((int)size);
	fonts[font]->BBox (text, llx, lly, llz, urx, ury, urz);
	x = urx - llx;
	y = ury - lly;
#endif
}

void CFont::GetTextSize (const char *text, float &x, float &y) const {
	GetTextSize(text, x, y, curr_font, curr_size);
}

void CFont::GetTextSize (const char *text, float &x, float &y, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx (fontname);
	GetTextSize(text, x, y, temp_font, size);
}

float CFont::GetTextWidth (const char *text) const {
	float x, y;
	GetTextSize(text, x, y, curr_font, curr_size);
	return x;
}

float CFont::GetTextWidth (const string& text) const {
	return GetTextWidth (text.c_str());
}

float CFont::GetTextWidth (const wchar_t *text) const {
	float x, y;
	GetTextSize(text, x, y, curr_font, curr_size);
	return x;
}

float CFont::GetTextWidth (const char *text, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx (fontname);
	float x, y;
	GetTextSize(text, x, y, temp_font, size);
	return x;
}

float CFont::GetTextWidth (const wchar_t *text, const string &fontname, float size) const {
	size_t temp_font = GetFontIdx (fontname);
	float x, y;
	GetTextSize(text, x, y, temp_font, size);
	return x;
}

float CFont::CenterX (const char *text) const {
	return (Winsys.resolution.width - GetTextWidth (text)) / 2.0;
}

void CFont::SetOrientation (Orientation orientation) {
	forientation = orientation;
}

vector<string> CFont::MakeLineList (const char *source, float width) {
	vector<string> wordlist;
	MakeWordList(wordlist, source);
	vector<string> linelist;

	for(size_t last = 0; last < wordlist.size();)
		last = MakeLine(last, wordlist, linelist, width)+1;

	return linelist;
}
