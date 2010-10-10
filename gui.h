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

#define MAX_MOUSERECTS 32
#define MAX_ARROWS 16
#define MAX_TEXTBUTTONS 16
#define MAX_ICONBUTTONS 8
#define MAX_CHECKBOXES 16

#define CENTER -1
#define FIT -1

typedef enum {
	W_ARROW,
	W_TEXTBUTTON,
	W_ICONBUTTON,
	W_CHECKBOX
} TWidgetType;

typedef struct {
	TRect rect;
	int focus;
	int dir;
	int arrnr;
	TWidgetType type;
} TMouseRect;

typedef struct {
	int x;
	int y;
	int dir;
	int focus;
} TArrow;

typedef struct {
	int x;
	int y;
	int font;
 	string text;
	int focus;
	double ftsize;	// font height
} TTextButton;

typedef struct {
	int x;
	int y;
	int focus;
	double size;
	GLuint texid;
} TIconButton;

typedef struct {
	int x;
	int y;
	int focus;
	int width;
	string tag;
} TCheckbox;

void GetFocus (int x, int y, int *focus, int *dir);
void ResetWidgets ();

void AddArrow (int x, int y, int dir, int focus);
void PrintArrow (int nr, bool active);
void AddTextButton (const char *text, int x, int y, int focus, double ftsize);
void PrintTextButton (int nr, int focus);
void AddIconButton (int x, int y, int focus, GLuint texid, double size);
void PrintIconButton (int nr, int focus, int state);
void AddCheckbox (int x, int y, int focus, int width, const string tag);
void PrintCheckbox (int nr, int focus, bool state);

void DrawFrameX (int x, int y, int w, int h, int line, 
			TColor backcol, TColor framecol, double transp);
void DrawLevel (int x, int y, int level, double fact);
void DrawBonus (int x, int y, int max, int num);
void DrawBonusExt (int y, int numraces, int num);
void DrawCursor ();

double AutoFtSize ();
int AutoXSize (int size);
int AutoYSize (int size);
int AutoXPos (int x);
int AutoYPos (int y);
double AutoDistance ();

#endif
