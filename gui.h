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

#ifndef GUI_H
#define GUI_H

#include "bh.h"

enum TWidgetType {
	W_ARROW,
	W_TEXTBUTTON,
	W_ICONBUTTON,
	W_CHECKBOX
};

struct TMouseRect {
	TRect rect;
	int focus;
	int dir;
	size_t arrnr;
	TWidgetType type;
};

struct TArrow {
	int x;
	int y;
	int dir;
	int focus;
};

struct TTextButton {
	int x;
	int y;
	int font;
 	string text;
	int focus;
	double ftsize;	// font height
};

struct TIconButton {
	int x;
	int y;
	int focus;
	double size;
	GLuint texid;
};

struct TCheckbox {
	int x;
	int y;
	int focus;
	int width;
	string tag;
};

void GetFocus (int x, int y, int *focus, int *dir);
void ResetWidgets ();

void AddArrow (int x, int y, int dir, int focus);
void PrintArrow (int nr, bool active);
void AddTextButton (const string& text, int x, int y, int focus, double ftsize);
void AddTextButtonN (const string& text, int x, int y, int focus, int rel_ftsize);
void PrintTextButton (int nr, int focus);
void AddIconButton (int x, int y, int focus, GLuint texid, double size);
void PrintIconButton (int nr, int focus, int state);
void AddCheckbox (int x, int y, int focus, int width, const string& tag);
void PrintCheckbox (int nr, int focus, bool state);

void DrawFrameX (int x, int y, int w, int h, int line, 
			const TColor& backcol, const TColor& framecol, double transp);
void DrawLevel (int x, int y, int level, double fact);
void DrawBonus (int x, int y, int max, int num);
void DrawBonusExt (int y, int numraces, int num);
void DrawCursor ();

// --------------------------------------------------------------------

int AutoYPosN (double percent);
TArea AutoAreaN (double top_perc, double bott_perc, int w);

#endif
