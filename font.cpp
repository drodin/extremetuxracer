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

#include "font.h"
#include "spx.h"

#define USE_UNICODE true

// --------------------------------------------------------------------
// First some common function used for textboxes and called by
// CFont::MakeLineList. This bundle of functions generates
// a SPList from a textstring and adapts the lines to the textbox

void MakeWordList (CSPList *wordlist, const char *s) {
	char dest[2000];
	char val[16];
	dest[0]=0;
	int ch = 0;
	int wd = 1;
	dest[ch] = '['; ch++;
	dest[ch] = '0'; ch++;
	dest[ch] = ']'; ch++;

	for (unsigned int i=0; i<strlen(s); i++) {
		if (s[i] == ' ') {
			Int_CharN (val, wd); 
			wd++;
			dest[ch] = '['; 
			ch++;
			for (unsigned int j=0; j<strlen(val); j++) {
				dest[ch] = val[j]; 
				ch++;
			} 
			dest[ch] = ']'; 
			ch++;
		} else {
			dest[ch] = s[i]; 
			ch++;
		}
	}
	dest[ch] = 0;

	string entry, istr;
	string dest2 (dest);
//	string *dest2 = new string (dest);
	for (int i=0; i<wd; i++) {
		istr = Int_StrN (i);
		entry = SPStrN (dest2, istr, "");	
		entry += ' ';
		wordlist->Add (entry);
	}
}

float GetWordLng (const char *word) {
	return FT.GetTextWidth (word) + 4;		// +4: some space reserve at EOL
}

int MakeLine (int first, CSPList *wordlist, CSPList *linelist, float width) {
	if (first >= wordlist->Count()) return 999;
	float wordlng;

	int last = first-1;
	float lng = 0;

	wordlng = GetWordLng (wordlist->LineC(first));
	bool ready = false;
	do {
		last++;
		lng += wordlng;
		if (last >= wordlist->Count()-1) ready = true;
		if (!ready) {
			wordlng = GetWordLng (wordlist->LineC (last+1));
			if (!ready && lng + wordlng >= width) ready = true;  
		}
	} while (!ready);

	string line = "";
	for (int j=first; j<=last; j++) line += wordlist->Line(j);
	linelist->Add (line);
	return last;
}



// --------------------------------------------------------------------
//				CFont
// --------------------------------------------------------------------

CFont FT;	

CFont::CFont () {
	for (int i=0; i<MAX_FONTS; i++) fonts[i] = NULL;
	numFonts = 0;
	fontindex = "";
	forientation = OR_TOP;

	// setting default values
	curr_col.r = 0.0;	// default color: black
	curr_col.g = 0.0;
	curr_col.b = 0.0;
	curr_col.a = 1.0;	// default: no transparency
	curr_size   = 20;	// default size: 20 px
}

CFont::~CFont () {
	for (int i=0; i<MAX_FONTS; i++) {
		if (fonts[i] != NULL) delete (fonts[i]);
	}
	numFonts = 0;
	fontindex = "";
}	

// --------------------------------------------------------------------
//				private
// --------------------------------------------------------------------

wchar_t *CFont::UnicodeStr (const char *s) {
	wchar_t ch;
	int len = strlen (s);
	wchar_t *res = new wchar_t [len+1];
	int j=0;
		
	for (int i=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)s)[i];
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
	res[j] = 0;
	return res;
}

void CFont::UnicodeStr (wchar_t *buff, const char *string) {
	wchar_t ch;
	int len = strlen(string);
	int j=0;
		
	for (int i=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)string)[i];
		if ( ch >= 0xF0 ) {
			ch  =  (wchar_t)(string[i]&0x07)   << 18;
			ch |=  (wchar_t)(string[++i]&0x3F) << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xE0 ) {
			ch  =  (wchar_t)(string[i]&0x0F)   << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xC0 ) {
			ch  =  (wchar_t)(string[i]&0x1F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		}
		buff[j] = ch;
	}
	buff[j] = 0;
}

// --------------------------------------------------------------------
//					public
// --------------------------------------------------------------------

int CFont::LoadFont (string name, const char *path) {
	if (numFonts >= MAX_FONTS) return -1;	
	fonts[numFonts] = new FTGLPixmapFont (path);
	if (fonts[numFonts]->Error()) {
		Message ("Failed to open font");
		return -1;
	}
	fonts[numFonts]->FaceSize (18);
	fonts[numFonts]->CharMap (ft_encoding_unicode);

	fontindex = fontindex + "[" + name + "]" + Int_StrN (numFonts);	
	numFonts++;
	return numFonts-1;
}

int CFont::LoadFont (string name, const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadFont (name, path.c_str());
}

bool CFont::LoadFontlist () {
	string fontfile, name, line;
	int ftidx = -1;

	CSPList list(MAX_FONTS);
	if (!list.Load ( param.font_dir, "fonts.lst")) return false;
	for (int i=0; i<list.Count(); i++) {
		line = list.Line(i);
		fontfile = SPStrN (line, "file", "");
		name = SPStrN (line, "name", "");

		ftidx = LoadFont (name, param.font_dir.c_str(), fontfile.c_str());		
		if (ftidx < 0) {
			Message ("couldn't load font", name.c_str());
		}
	}
	return true;
}

int CFont::GetFontIdx (const string &name) {
	int idx = SPIntN (fontindex, name, -1);
	return idx;
}

void CFont::SetProps (const string &fontname, float size, TColor col) {
	if (fontname.size() > 0) curr_font = GetFontIdx (fontname);
	if (size > 0) curr_size = size;
	curr_col  = col;	
}

void CFont::SetProps (const string &fontname, float size) {
	if (fontname.size() > 0) curr_font = GetFontIdx (fontname);
	if (size > 0) curr_size = size;
}

void CFont::SetColor (float r, float g, float b, float a) {
	curr_col.r = r;
	curr_col.g = g;
	curr_col.b = b;
	curr_col.a = a;
}	

void CFont::SetColor (TColor col) { 
	curr_col = col; 
}

void CFont::SetSize (float size) { 
	curr_size = size; 
}

void CFont::SetFont (string fontname) {
	int idx = SPIntN (fontindex, fontname, -1);
	curr_font = idx;
}

// -------------------- draw (x, y, text) -----------------------------

void CFont::DrawText (float x, float y, const char *text) {
	float left;
	if (numFonts < 1) return;
	if (curr_font < 0 || curr_font >= numFonts) return;

	glPushMatrix();
	fonts[curr_font]->FaceSize ((int)curr_size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);
	
	if (x >= 0) left = x; 
	else left = (param.x_resolution - GetTextWidth (text)) / 2;
	if (left < 0) left = 0;

	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(param.y_resolution - curr_size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	if (USE_UNICODE) fonts[curr_font]->Render (UnicodeStr (text));
		else fonts[curr_font]->Render (text);
	glPopMatrix();
}

void CFont::DrawText (float x, float y, const wchar_t *text) {
	float left;
	if (numFonts < 1) return;
	if (curr_font < 0 || curr_font >= numFonts) return;

	glPushMatrix();
	fonts[curr_font]->FaceSize ((int)curr_size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);

	if (x >= 0) left = x; 
	else left = (param.x_resolution - GetTextWidth (text)) / 2;
	if (left < 0) left = 0;

	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(param.y_resolution - curr_size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	fonts[curr_font]->Render (text);
	glPopMatrix();
}

void CFont::DrawString (float x, float y, const string &s) {
	DrawText (x, y, s.c_str());
}

void CFont::DrawText 
		(float x, float y, const char *text, const string &fontname, float size) {
	float left;
	if (numFonts < 1) return;
	int temp_font = GetFontIdx (fontname);
	if (temp_font < 0 || temp_font >= numFonts) return;

	glPushMatrix();
	fonts[temp_font]->FaceSize ((int)size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);

	if (x >= 0) left = x; 
	else left = (param.x_resolution - GetTextWidth (text, fontname, size)) / 2;
	if (left < 0) left = 0;


	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(param.y_resolution - size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	if (USE_UNICODE) fonts[temp_font]->Render (UnicodeStr (text));
		else fonts[temp_font]->Render (text);
	glPopMatrix();
}

void CFont::DrawText 
		(float x, float y, const wchar_t *text, const string &fontname, float size) {
	float left;
	if (numFonts < 1) return;
	int temp_font = GetFontIdx (fontname);
	if (temp_font < 0 || temp_font >= numFonts) return;

	glPushMatrix();
	fonts[temp_font]->FaceSize ((int)size);
	glColor4f (curr_col.r, curr_col.g, curr_col.b, curr_col.a);

	if (x >= 0) left = x; 
	else left = (param.x_resolution - GetTextWidth (text, fontname, size)) / 2;
	if (left < 0) left = 0;

	if (forientation == OR_TOP) {
		glRasterPos2i ((int)left, (int)(param.y_resolution - size - y));
	} else {
		glRasterPos2i ((int)left, (int)y);
	}

	fonts[temp_font]->Render (text);
	glPopMatrix();
}

void CFont::DrawString (
		float x, float y, const string &s, const string &fontname, float size) {
	DrawText (x, y, s.c_str(), fontname, size);
}

// --------------------- metrics --------------------------------------

void CFont::GetTextSize (const char *text, float &x, float &y) {
	if (numFonts < 1)  { x = 0; y = 0; return; }
	if (curr_font < 0 || curr_font >= numFonts) { x = 0; y = 0; return; }

	float llx, lly, llz, urx, ury, urz;
	fonts[curr_font]->FaceSize ((int)curr_size);
	if (USE_UNICODE) fonts[curr_font]->BBox (UnicodeStr(text), llx, lly, llz, urx, ury, urz);
		else fonts[curr_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	x = urx - llx;
	y = ury - lly;
}

void CFont::GetTextSize (const char *text, float &x, float &y, const string &fontname, float size) {
	if (numFonts < 1)  { x = 0; y = 0; return; }
	int temp_font = GetFontIdx (fontname);
	if (temp_font < 0 || temp_font >= numFonts) { x = 0; y = 0; return; }

	float llx, lly, llz, urx, ury, urz;
	fonts[temp_font]->FaceSize ((int)size);
	if (USE_UNICODE) fonts[temp_font]->BBox (UnicodeStr(text), llx, lly, llz, urx, ury, urz);
		else fonts[temp_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	x = urx - llx;
	y = ury - lly;
}

float CFont::GetTextWidth (const char *text) {
	if (numFonts < 1)  { return 0.0; }
	if (curr_font < 0 || curr_font >= numFonts) { return 0.0; }

	float llx, lly, llz, urx, ury, urz;
	fonts[curr_font]->FaceSize ((int)curr_size);
	if (USE_UNICODE) fonts[curr_font]->BBox (UnicodeStr (text), llx, lly, llz, urx, ury, urz);
		else fonts[curr_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	return urx - llx;
}

float CFont::GetTextWidth (const wchar_t *text) {
	if (numFonts < 1)  { return 0.0; }
	if (curr_font < 0 || curr_font >= numFonts) { return 0.0; }

	float llx, lly, llz, urx, ury, urz;
	fonts[curr_font]->FaceSize ((int)curr_size);
	fonts[curr_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	return urx - llx;
}

float CFont::GetTextWidth (const char *text, const string &fontname, float size) {
	if (numFonts < 1) return 0.0; 
	int temp_font = GetFontIdx (fontname);
	if (temp_font < 0 || temp_font >= numFonts) return 0.0;

	float llx, lly, llz, urx, ury, urz;
	fonts[temp_font]->FaceSize ((int)size);
	if (USE_UNICODE) fonts[temp_font]->BBox (UnicodeStr (text), llx, lly, llz, urx, ury, urz);
		else fonts[temp_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	return urx - llx;
}

float CFont::GetTextWidth (const wchar_t *text, const string &fontname, float size) {
	if (numFonts < 1) return 0.0; 
	int temp_font = GetFontIdx (fontname);
	if (temp_font < 0 || temp_font >= numFonts) return 0.0;

	float llx, lly, llz, urx, ury, urz;
	fonts[temp_font]->FaceSize ((int)size);
	fonts[temp_font]->BBox (text, llx, lly, llz, urx, ury, urz);
	return urx - llx;
}

float CFont::CenterX (const char *text) {
	return (param.x_resolution - GetTextWidth (text)) / 2;
}

void CFont::SetOrientation (int orientation) {
	forientation = orientation;
}

void CFont::MakeLineList (const char *source, CSPList *line_list, float width) {
	CSPList wordlist(1000);
	MakeWordList (&wordlist, source);

	int last = -1;
	do { last = MakeLine (last+1, &wordlist, line_list, width); } 
	while (last < wordlist.Count()-1);
}

