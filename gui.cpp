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

#include "gui.h"
#include "textures.h"
#include "font.h"
#include <list>
#include <vector>

#define CURSOR_SIZE 10

static int cursor_x = -100;
static int cursor_y = -100;

static list<TMouseRect> MouseRects;
static vector<TArrow> Arrows;
static int selArrow = -1;
static int selType = -1;
static vector<TTextButton> TextButtons;
static vector<TIconButton> IconButtons;
static vector<TCheckbox> Checkboxes;

void ResetWidgets () {
	MouseRects.clear();
	Arrows.clear();
	TextButtons.clear();
	IconButtons.clear();
	Checkboxes.clear();
}

void DrawCursor () {
	Tex.Draw (MOUSECURSOR, cursor_x, cursor_y, 
		CURSOR_SIZE  * (double)param.x_resolution / 14000);
}

void AddMouseRect (int left, int top, int width, int height,
		int focus, int dir, size_t arrnr, TWidgetType type) {
	MouseRects.push_back(TMouseRect());
	MouseRects.back().rect.left = left;
	MouseRects.back().rect.top = top;
	MouseRects.back().rect.width = width;
	MouseRects.back().rect.height = height;
	MouseRects.back().focus = focus;
	MouseRects.back().dir = dir;
	MouseRects.back().arrnr = arrnr;
	MouseRects.back().type = type;
}

void AddArrow (int x, int y, int dir, int focus) {
	Arrows.push_back(TArrow());
	Arrows.back().x = x;
	Arrows.back().y = y;
	Arrows.back().dir = dir;
	Arrows.back().focus = focus;
	AddMouseRect (x, y, 32, 16, focus, dir, Arrows.size()-1, W_ARROW);
}

void AddTextButton (const string& text, int x, int y, int focus, double ftsize) {
	TextButtons.push_back(TTextButton());
	TextButtons.back().y = y;
	TextButtons.back().text = text;
	TextButtons.back().focus = focus;

	if (ftsize < 0) ftsize = FT.AutoSizeN (4);
	
	TextButtons.back().ftsize = ftsize;	
	FT.SetSize (ftsize);
	double len = FT.GetTextWidth (text);
	if (x == CENTER) x = (int)((param.x_resolution - len) / 2);
	TextButtons.back().x = x;
	int offs = (int)(ftsize / 5);
	AddMouseRect (x-20, y+offs, (int)len + 40, (int)(ftsize+offs), focus, 0, 
		TextButtons.size()-1, W_TEXTBUTTON);
}

void AddTextButtonN (const string& text, int x, int y, int focus, int rel_ftsize) {
	double siz = FT.AutoSizeN (rel_ftsize);
	AddTextButton (text, x, y, focus, siz);
}

void PrintTextButton (int nr, int focus) {
	if (nr >= TextButtons.size()) return;

	if (focus == TextButtons[nr].focus)
		FT.SetColor (colDYell);
	else
		FT.SetColor (colWhite);
	FT.SetSize (TextButtons[nr].ftsize);
	FT.DrawString (TextButtons[nr].x, TextButtons[nr].y, TextButtons[nr].text);
}

void AddCheckbox (int x, int y, int focus, int width, const string& tag) {
	Checkboxes.push_back(TCheckbox());
	Checkboxes.back().x = x;
	Checkboxes.back().y = y;
	Checkboxes.back().focus = focus;
	Checkboxes.back().width = width;
	Checkboxes.back().tag = tag;
	AddMouseRect (x+width-32, y, 32, 32, focus, 0, Checkboxes.size()-1, W_CHECKBOX);
}

void PrintCheckbox (int nr, int focus, bool state) {
	if (nr >= Checkboxes.size()) return;

	TCheckbox *box = &Checkboxes[nr];
	Tex.Draw (CHECKBOX, box->x + box->width - 32, box->y, 1.0);
	if (state)
		Tex.Draw (CHECKMARK_SMALL, box->x + box->width - 32, box->y, 1.0);
	if (focus == box->focus)
		FT.SetColor (colDYell);
	else
		FT.SetColor (colWhite);
	FT.DrawString (box->x, box->y, box->tag);
}

void AddIconButton (int x, int y, int focus, GLuint texid, double size) {
	IconButtons.push_back(TIconButton());
	IconButtons.back().x = x;
	IconButtons.back().y = y;
	IconButtons.back().focus = focus;
	IconButtons.back().texid = texid;
	IconButtons.back().size = size;	
	AddMouseRect (x, y, 32, 32, focus, 0, IconButtons.size()-1, W_ICONBUTTON);
}

void PrintIconButton (int nr, int focus, int state) {
	if (state < 0 || state >= 4) return;
	if (nr >= IconButtons.size()) return;

	TColor framecol = colWhite;
	if (focus == IconButtons[nr].focus) framecol = colDYell;

	int size = (int)IconButtons[nr].size;
	int line = 3;
	int framesize = size + 2 * line; 
 	int t = param.y_resolution - IconButtons[nr].y;
	int y = t - size;
	int x = IconButtons[nr].x;
	int r = x + size;

	DrawFrameX (IconButtons[nr].x-line, IconButtons[nr].y-line, 
				framesize, framesize, line, colBlack, framecol, 1.0);

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, IconButtons[nr].texid);
    glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin (GL_QUADS);
	switch (state) {
		case 0:
			glTexCoord2f (0, 0.5); glVertex2f (x,y);
			glTexCoord2f (0.5, 0.5); glVertex2f (r,y);
			glTexCoord2f (0.5, 1.0); glVertex2f (r,t);
			glTexCoord2f (0, 1.0); glVertex2f (x,t);
		break;
		case 1:
			glTexCoord2f (0.5, 0.5); glVertex2f (x,y);
			glTexCoord2f (1.0, 0.5); glVertex2f (r,y);
			glTexCoord2f (1.0, 1.0); glVertex2f (r,t);
			glTexCoord2f (0.5, 1.0); glVertex2f (x,t);
		break;
		case 2:
			glTexCoord2f (0, 0); glVertex2f (x,y);
			glTexCoord2f (0.5, 0); glVertex2f (r,y);
			glTexCoord2f (0.5, 0.5); glVertex2f (r,t);
			glTexCoord2f (0, 0.5); glVertex2f (x,t);
		break;
		case 3:
			glTexCoord2f (0.5, 0); glVertex2f (x,y);
			glTexCoord2f (1.0, 0); glVertex2f (r,y);
			glTexCoord2f (1.0, 0.5); glVertex2f (r,t);
			glTexCoord2f (0.5, 0.5); glVertex2f (x,t);
		break;
	}
	glEnd ();
}

void DrawArrow (int x, int y, int dir, bool active, int sel) {
	double textl[6] = {0.5, 0.0, 0.5, 0.5, 0.0, 0.5};		
	double textr[6] = {1.0, 0.5, 1.0, 1.0, 0.5, 1.0};
	double texbl[6] = {0.25, 0.25, 0.75, 0.00, 0.00, 0.50};
	double texbr[6] = {0.50, 0.50, 1.00, 0.25, 0.25, 0.75};
    double texleft, texright, textop, texbottom;
    TVector2 bl, tr;

	int type;	 
	if (active) type = 3 * dir + 1 + sel; else type = 3 * dir;
	
	bl.x = x;
	bl.y = param.y_resolution - y - 16;
	tr.x = x + 32;
	tr.y = param.y_resolution - y;
		
	texleft = textl[type];
	texright = textr[type];
	texbottom = texbl[type];	
	textop = texbr[type];
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D );
	Tex.BindTex (LB_ARROWS);
	glColor4f (1.0, 1.0, 1.0, 1.0);
	
	glBegin( GL_QUADS );
	    glTexCoord2f (texleft, texbottom); 
		glVertex2f (bl.x, bl.y );
	    glTexCoord2f (texright, texbottom); 
		glVertex2f (tr.x, bl.y );
	    glTexCoord2f (texright, textop); 
		glVertex2f (tr.x, tr.y );
	    glTexCoord2f (texleft, textop); 
		glVertex2f (bl.x, tr.y );
	glEnd();
}


// active is true if the arrow can be clicked on. If the value has reached the end
// of range, active must be set to false
// nr is the index in arrowarray Arrows
void PrintArrow (int nr, bool active) {
	int sel = 0;
	if (nr >= Arrows.size()) return;
	if ((nr == selArrow) && (selType == W_ARROW) && active) sel = 1;
	DrawArrow (Arrows[nr].x, Arrows[nr].y, Arrows[nr].dir, active, sel);		
}

static bool Inside (int x, int y, const TMouseRect& Rect) {
	if (x >= Rect.rect.left
		&& x <= Rect.rect.left + Rect.rect.width
		&& y >= Rect.rect.top
		&& y <= Rect.rect.top + Rect.rect.height) {
		return true;
	} else return false;
}

void GetFocus (int x, int y, int *focus, int *dir) {
	cursor_x = x;
	cursor_y = y;
	for (list<TMouseRect>::const_iterator i = MouseRects.begin(); i != MouseRects.end(); ++i) {
		if (Inside (x, y, *i)) {
			*focus = i->focus;
			*dir = i->dir;
			selArrow = i->arrnr;
			selType = i->type;
			return;
		}
	}
	*focus = -1;
	*dir = -1;
	selArrow = -1;
	selType = -1;
}

void DrawFrameX (int x, int y, int w, int h, int line, 
		const TColor& backcol, const TColor& framecol, double transp) {
	double yy = param.y_resolution - y - h;
 
	if (x < 0) x = (param.x_resolution -w) / 2;
	glPushMatrix();
	glDisable (GL_TEXTURE_2D);
    
	glColor4f (framecol.r, framecol.g, framecol.b, transp); 
	glTranslatef (x, yy, 0);
	glBegin (GL_QUADS );
	    glVertex2f (0, 0 );
	    glVertex2f (w, 0 );
	    glVertex2f (w, h );
	    glVertex2f (0, h );
	glEnd();

	glColor4f (backcol.r, backcol.g, backcol.b, transp);
	glBegin (GL_QUADS );
	    glVertex2f (0 + line, 0 + line );
	    glVertex2f (w - line, 0 + line );
	    glVertex2f (w - line, h - line );
	    glVertex2f (0 + line, h - line );
	glEnd();

	glEnable (GL_TEXTURE_2D);
    glPopMatrix();
}

void DrawLevel (int x, int y, int level, double fact) {
    TVector2 bl, tr;
	double lev[4] = {0.0, 0.75, 0.5, 0.25}; 
	double bott, top;
	
	bl.x = x;
	bl.y = param.y_resolution - y - 32;
	tr.x = x + 95;
	tr.y = param.y_resolution - y;

	bott = lev[level];
	top = bott + 0.25;
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (STARS);
	glColor4f (1.0, 1.0, 1.0, 1.0);
	
	glBegin( GL_QUADS );
	    glTexCoord2f (0, bott); 
		glVertex2f (bl.x, bl.y );
	    glTexCoord2f (0.75, bott); 
		glVertex2f (tr.x, bl.y );
	    glTexCoord2f (0.75, top); 
		glVertex2f (tr.x, tr.y );
	    glTexCoord2f (0, top); 
		glVertex2f (bl.x, tr.y );
	glEnd();
}

void DrawBonus (int x, int y, int max, int num) {
    TVector2 bl, tr;
	double bott, top;
	int i;
		
	bl.y = param.y_resolution - y - 32;
	tr.y = param.y_resolution - y;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (TUXBONUS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	for (i=0; i<max; i++) {
		bl.x = x + i * 40;
		tr.x = bl.x + 32;
	
		if (i<num) bott = 0.5; else bott = 0.0;
		top = bott + 0.5;
		
		glBegin( GL_QUADS );
		    glTexCoord2f (0, bott); 
			glVertex2f (bl.x, bl.y );
		    glTexCoord2f (1, bott); 
			glVertex2f (tr.x, bl.y );
	    	glTexCoord2f (1, top); 
			glVertex2f (tr.x, tr.y );
		    glTexCoord2f (0, top); 
			glVertex2f (bl.x, tr.y );
		glEnd();
	}
}

void DrawBonusExt (int y, int numraces, int num) {
    TVector2 bl, tr;
	double bott, top;
	int i;

	TColor col1 = MakeColor (0.3, 0.5, 0.7, 1);
	TColor col2 = MakeColor (0.45, 0.65, 0.85, 1);
	TColor col3 = MakeColor (0.6, 0.8, 1.0, 1);
	TColor gold = MakeColor (1, 1, 0, 1);

	int lleft[3];
	int majr, minr;

	int maxtux = numraces * 3;
	if (num > maxtux) return;

	int framewidth = numraces * 40 + 8;
	int totalwidth = framewidth * 3 + 8;
	int xleft = (param.x_resolution - totalwidth) / 2;
	lleft[0] = xleft;
	lleft[1] = xleft + framewidth + 4;
	lleft[2] = xleft + framewidth + framewidth + 8;

	DrawFrameX (lleft[0], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX (lleft[1], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX (lleft[2], y, framewidth, 40, 1, col2, colBlack, 1);
	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
	bl.y = param.y_resolution - y - 32 -4;
	tr.y = param.y_resolution - y - 0 -4;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (TUXBONUS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	for (i=0; i<maxtux; i++) {
		majr = (int)(i/numraces);
		minr = i - majr * numraces;
		if (majr > 2) majr = 2;
		bl.x = lleft[majr] + minr * 40 + 6;
		tr.x = bl.x + 32;

		// with tux outlines:
		// if (i<num) bott = 0.5; else bott = 0.0;
		// top = bott + 0.5;
		if (i<num) {
			bott = 0.5;
			top = 1.0;
			glBegin (GL_QUADS);
				glTexCoord2f (0, bott); glVertex2f (bl.x, bl.y);
				glTexCoord2f (1, bott); glVertex2f (tr.x, bl.y);
				glTexCoord2f (1, top);  glVertex2f (tr.x, tr.y);
				glTexCoord2f (0, top);  glVertex2f (bl.x, tr.y);
			glEnd();
		}
	}
}

// ------------------ new ---------------------------------------------

int AutoYPosN (double percent) {
	double hh = (double)param.y_resolution;
	double po = hh * percent / 100;
	return (int)(po);
}

TArea AutoAreaN (double top_perc, double bott_perc, int w) {
	TArea res;
	res.top = AutoYPosN (top_perc);
	res.bottom = AutoYPosN (bott_perc);
 	if (w > param.x_resolution) w = param.x_resolution;
	double left = (param.x_resolution - w) / 2;
	res.left = (int) left;
	res.right = param.x_resolution - res.left;
	return res;
}
