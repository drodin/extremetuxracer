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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "gui.h"
#include "textures.h"
#include "font.h"
#include "winsys.h"
#include <list>
#include <vector>


#define CURSOR_SIZE 10

static vector<TWidget*> Widgets;

static bool Inside (int x, int y, const TRect& Rect) {
	return (x >= Rect.left
		&& x <= Rect.left + Rect.width
		&& y >= Rect.top
		&& y <= Rect.top + Rect.height);
}

TWidget::TWidget(int x, int y, int width, int height)
	: active(true)
	, visible(true)
	, focus(false)
{
	mouseRect.top = y;
	mouseRect.left = x;
	mouseRect.height = height;
	mouseRect.width = width;
	position.x = x;
	position.y = y;
}

bool TWidget::Click(int x, int y) {
	return active && visible && Inside(x, y, mouseRect);
}

void TWidget::MouseMove(int x, int y) {
	focus = active && visible && Inside(x, y, mouseRect);
}

TTextButton::TTextButton(int x, int y, const string& text_, double ftsize_)
	: TWidget(x, y, 0, 0)
	, text(text_)
	, ftsize(ftsize_)
{
	if (ftsize < 0) ftsize = FT.AutoSizeN (4);

	double len = FT.GetTextWidth (text);
	if (x == CENTER) position.x = (int)((Winsys.resolution.width - len) / 2);
	int offs = (int)(ftsize / 5);
	mouseRect.left = position.x-20;
	mouseRect.top = position.y+offs;
	mouseRect.width = len+40;
	mouseRect.height = ftsize+offs;
}

void TTextButton::Draw() const {
	if (focus)
		FT.SetColor (colDYell);
	else
		FT.SetColor (colWhite);
	FT.SetSize (ftsize);
	FT.DrawString (position.x, position.y, text);
}

TTextButton* AddTextButton (const string& text, int x, int y, double ftsize) {
	Widgets.push_back(new TTextButton(x, y, text, ftsize));
	return static_cast<TTextButton*>(Widgets.back());
}

TTextButton* AddTextButtonN (const string& text, int x, int y, int rel_ftsize) {
	double siz = FT.AutoSizeN (rel_ftsize);
	return AddTextButton (text, x, y, siz);
}


TTextField::TTextField(int x, int y, int width, int height, const string& text_)
	: TWidget(x, y, width, height)
	, text(text_)
	, cursorPos(0)
	, maxLng(32)
	, time(0.0)
	, cursor(false)
{
}

void TTextField::Draw() const {
	const TColor& col = focus?colDYell:colWhite;
	FT.SetColor (col);
	DrawFrameX (mouseRect.left, mouseRect.top, mouseRect.width, mouseRect.height, 3, colMBackgr, col, 1.0);
	FT.AutoSizeN (5);
	FT.DrawString (mouseRect.left+20, mouseRect.top, text);

	if (cursor && focus) {
		double x = mouseRect.left+20+1;
		if (cursorPos != 0) {
			string temp = text.substr (0, cursorPos);
			x += FT.GetTextWidth (temp);
		}
		double w = 3;
		double h = 26*Winsys.scale;
		double scrheight = Winsys.resolution.height;

		glDisable (GL_TEXTURE_2D);
		glColor4f (colYellow.r, colYellow.g, colYellow.b, colYellow.a);
		glBegin (GL_QUADS);
			glVertex2f (x,   scrheight-mouseRect.top-h-9);
			glVertex2f (x+w, scrheight-mouseRect.top-h-9);
			glVertex2f (x+w, scrheight-mouseRect.top-9);
			glVertex2f (x,   scrheight-mouseRect.top-9);
		glEnd();
		glEnable (GL_TEXTURE_2D);
	}
}

void TTextField::Key(unsigned int key, unsigned int mod, bool released) {
	if (islower (key)) {
		if (text.size() < maxLng) {
			if (mod & KMOD_SHIFT) text.insert(cursorPos, 1, toupper(key));
			else text.insert(cursorPos, 1, key);
			cursorPos++;
		}
	} else if (isdigit (key)) {
		if (text.size() < maxLng) {
			text.insert(cursorPos, 1, key);
			cursorPos++;
		}
	} else {
		switch (key) {
			case SDLK_DELETE: if (cursorPos < text.size()) text.erase (cursorPos, 1); break;
			case SDLK_BACKSPACE: if (cursorPos > 0) { text.erase (cursorPos-1, 1); cursorPos--; } break;
			case SDLK_RIGHT: if (cursorPos < text.size()) cursorPos++; break;
			case SDLK_LEFT: if (cursorPos > 0) cursorPos--; break;
			case SDLK_HOME: cursorPos = 0; break;
			case SDLK_END: cursorPos = text.size(); break;
			case SDLK_SPACE: text.insert(cursorPos, 1, 21); cursorPos++; break;
		}
	}
}

void TTextField::UpdateCursor(double timestep) {
	time += timestep;
	if (time > CRSR_PERIODE) {
		time = 0;
		cursor = !cursor;
	}
}

TTextField* AddTextField(const string& text, int x, int y, int width, int height) {
	Widgets.push_back(new TTextField(x, y, width, height, text));
	return static_cast<TTextField*>(Widgets.back());
}

void TCheckbox::Draw () const {
	Tex.Draw (CHECKBOX, position.x + width - 32, position.y, 1.0);
	if (checked)
		Tex.Draw (CHECKMARK_SMALL, position.x + width - 32, position.y, 1.0);
	if (focus == focus)
		FT.SetColor (colDYell);
	else
		FT.SetColor (colWhite);
	FT.DrawString (position.x, position.y, tag);
}

bool TCheckbox::Click(int x, int y) {
	if(active && visible && Inside(x, y, mouseRect)) {
		checked = !checked;
		return true;
	}
	return false;
}

void TCheckbox::Key(unsigned int key, unsigned int mod, bool released) {
	if(released) return;

	if(key == SDLK_SPACE || key == SDLK_RIGHT || key == SDLK_LEFT) {
		checked = !checked;
	}
}

TCheckbox* AddCheckbox (int x, int y, int width, const string& tag) {
	Widgets.push_back(new TCheckbox(x, y, width, tag));
	return static_cast<TCheckbox*>(Widgets.back());
}

void TIconButton::SetValue (int _value) {
	value = _value;
	if(value > maximum)
		value = maximum;
}

void TIconButton::Draw () const {
	TColor framecol = colWhite;
	if (focus) framecol = colDYell;

	int line = 3;
	int framesize = size + 2 * line;
	int t = Winsys.resolution.height - position.y;
	int y = t - size;
	int x = position.x;
	int r = x + size;

	DrawFrameX (position.x-line, position.y-line,
				framesize, framesize, line, colBlack, framecol, 1.0);

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	texture->Bind();
    glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin (GL_QUADS);
	switch (value) {
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

bool TIconButton::Click(int x, int y) {
	if(Inside(x, y, mouseRect)) {
		value++;
		if(value > maximum)
			value = 0;
		return true;
	}
	return false;
}

void TIconButton::Key(unsigned int key, unsigned int mod, bool released) {
	if(released) return;

	if(key == SDLK_DOWN || key == SDLK_LEFT) { // Arrow down/left
		value--;
		if(value < 0)
			value = maximum;
	} else if(key == SDLK_UP || key == SDLK_RIGHT) { // Arrow up/right
		value++;
		if(value > maximum)
			value = 0;
	}
}

TIconButton* AddIconButton(int x, int y, TTexture* texture, double size, int maximum, int value) {
	Widgets.push_back(new TIconButton(x, y, texture, size, maximum, value));
	return static_cast<TIconButton*>(Widgets.back());
}

void TArrow::Draw() const {
	static const double textl[6] = {0.5, 0.0, 0.5, 0.5, 0.0, 0.5};
	static const double textr[6] = {1.0, 0.5, 1.0, 1.0, 0.5, 1.0};
	static const double texbl[6] = {0.25, 0.25, 0.75, 0.00, 0.00, 0.50};
	static const double texbr[6] = {0.50, 0.50, 1.00, 0.25, 0.25, 0.75};
    TVector2 bl, tr;

	int type = 0;
	if (active)
		type = 1;
	if(focus)
		type++;
	if(down)
		type += 3;

	bl.x = position.x;
	bl.y = Winsys.resolution.height - position.y - 16;
	tr.x = position.x + 32;
	tr.y = Winsys.resolution.height - position.y;

	double texleft = textl[type];
	double texright = textr[type];
	double texbottom = texbl[type];
	double textop = texbr[type];

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (LB_ARROWS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin( GL_QUADS );
		glTexCoord2f (texleft, texbottom);
		glVertex2f (bl.x, bl.y);
		glTexCoord2f (texright, texbottom);
		glVertex2f (tr.x, bl.y);
		glTexCoord2f (texright, textop);
		glVertex2f (tr.x, tr.y);
		glTexCoord2f (texleft, textop);
		glVertex2f (bl.x, tr.y);
	glEnd();
}

TArrow* AddArrow(int x, int y, bool down) {
	Widgets.push_back(new TArrow(x, y, down));
	return static_cast<TArrow*>(Widgets.back());
}


TUpDown::TUpDown(int x, int y, int min_, int max_, int value_, int distance)
	: TWidget(x, y, 32, 32+distance)
	, up(x, y+16+distance, true)
	, down(x, y, false)
	, value(value_)
	, minimum(min_)
	, maximum(max_)
{
	up.SetActive(value < maximum);
	down.SetActive(value > minimum);
}

void TUpDown::Draw() const {
	up.Draw();
	down.Draw();
}

bool TUpDown::Click(int x, int y) {
	if(active && visible && up.Click(x, y)) {
		value++;
		down.SetActive(true);
		if(value == maximum)
			up.SetActive(false);
		return true;
	}
	if(active && visible && down.Click(x, y)) {
		up.SetActive(true);
		value--;
		if(value == minimum)
			down.SetActive(false);
		return true;
	}
	return false;
}

void TUpDown::Key(unsigned int key, unsigned int mod, bool released) {
	if(released) return;

	if(key == SDLK_UP || key == SDLK_RIGHT) { // Arrow down/left
		if(value > minimum) {
			value--;
			up.SetActive(true);
			if(value == minimum)
				down.SetActive(false);
		}
	} else if(key == SDLK_DOWN || key == SDLK_LEFT) { // Arrow up/right
		if(value < maximum) {
			value++;
			down.SetActive(true);
			if(value == maximum)
				up.SetActive(false);
		}
	}
}

void TUpDown::MouseMove(int x, int y) {
	focus = active && visible &&Inside(x, y, mouseRect);
	up.MouseMove(x, y);
	down.MouseMove(x, y);
}

void TUpDown::SetValue(int value_) {
	value = clamp(minimum, value_, maximum);
	up.SetActive(value < maximum);
	down.SetActive(value > minimum);
}
void TUpDown::SetMinimum(int min_) {
	minimum = min_; value = clamp(minimum, value, maximum);
	up.SetActive(value < maximum);
	down.SetActive(value > minimum);
}
void TUpDown::SetMaximum(int max_) {
	maximum = max_; value = clamp(minimum, value, maximum);
	up.SetActive(value < maximum);
	down.SetActive(value > minimum);
}

TUpDown* AddUpDown(int x, int y, int minimum, int maximum, int value, int distance) {
	Widgets.push_back(new TUpDown(x, y, minimum, maximum, value, distance));
	return static_cast<TUpDown*>(Widgets.back());
}

// ------------------ Elementary drawing ---------------------------------------------

void DrawFrameX (int x, int y, int w, int h, int line,
		const TColor& backcol, const TColor& framecol, double transp) {
	double yy = Winsys.resolution.height - y - h;

	if (x < 0) x = (Winsys.resolution.width -w) / 2;
	glPushMatrix();
	glDisable (GL_TEXTURE_2D);

	glColor4f (framecol.r, framecol.g, framecol.b, transp);
	glTranslatef (x, yy, 0);
	glBegin (GL_QUADS);
	    glVertex2f (0, 0);
	    glVertex2f (w, 0);
	    glVertex2f (w, h);
	    glVertex2f (0, h);
	glEnd();

	glColor4f (backcol.r, backcol.g, backcol.b, transp);
	glBegin (GL_QUADS);
	    glVertex2f (0 + line, 0 + line);
	    glVertex2f (w - line, 0 + line);
	    glVertex2f (w - line, h - line);
	    glVertex2f (0 + line, h - line);
	glEnd();

	glEnable (GL_TEXTURE_2D);
    glPopMatrix();
}

void DrawLevel (int x, int y, int level, double fact) {
    TVector2 bl, tr;
	static const double lev[4] = {0.0, 0.75, 0.5, 0.25};

	bl.x = x;
	bl.y = Winsys.resolution.height - y - 32;
	tr.x = x + 95;
	tr.y = Winsys.resolution.height - y;

	double bott = lev[level];
	double top = bott + 0.25;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (STARS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin( GL_QUADS );
	    glTexCoord2f (0, bott);
		glVertex2f (bl.x, bl.y);
	    glTexCoord2f (0.75, bott);
		glVertex2f (tr.x, bl.y);
	    glTexCoord2f (0.75, top);
		glVertex2f (tr.x, tr.y);
	    glTexCoord2f (0, top);
		glVertex2f (bl.x, tr.y);
	glEnd();
}

void DrawBonus (int x, int y, size_t max, size_t num) {
    TVector2 bl, tr;

	bl.y = Winsys.resolution.height - y - 32;
	tr.y = Winsys.resolution.height - y;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (TUXBONUS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	for (size_t i=0; i<max; i++) {
		bl.x = x + i * 40;
		tr.x = bl.x + 32;

		double bott = 0.0;
		if (i<num) bott = 0.5;
		double top = bott + 0.5;

		glBegin( GL_QUADS );
		    glTexCoord2f (0, bott);
			glVertex2f (bl.x, bl.y);
		    glTexCoord2f (1, bott);
			glVertex2f (tr.x, bl.y);
	    	glTexCoord2f (1, top);
			glVertex2f (tr.x, tr.y);
		    glTexCoord2f (0, top);
			glVertex2f (bl.x, tr.y);
		glEnd();
	}
}

void DrawBonusExt (int y, size_t numraces, size_t num) {
	size_t maxtux = numraces * 3;
	if (num > maxtux) return;

    TVector2 bl, tr;

	//TColor col1 = {0.3, 0.5, 0.7, 1};
	TColor col2(0.45, 0.65, 0.85, 1);
	//TColor col3 = {0.6, 0.8, 1.0, 1};
	//TColor gold = {1, 1, 0, 1};

	int lleft[3];

	int framewidth = (int)numraces * 40 + 8;
	int totalwidth = framewidth * 3 + 8;
	int xleft = (Winsys.resolution.width - totalwidth) / 2;
	lleft[0] = xleft;
	lleft[1] = xleft + framewidth + 4;
	lleft[2] = xleft + framewidth + framewidth + 8;

	DrawFrameX (lleft[0], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX (lleft[1], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX (lleft[2], y, framewidth, 40, 1, col2, colBlack, 1);
	if (param.use_papercut_font > 0) FT.SetSize (20); else FT.SetSize (15);
	bl.y = Winsys.resolution.height - y - 32 -4;
	tr.y = Winsys.resolution.height - y - 0 -4;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	Tex.BindTex (TUXBONUS);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	for (size_t i=0; i<maxtux; i++) {
		size_t majr = (i/numraces);
		size_t minr = i - majr * numraces;
		if (majr > 2) majr = 2;
		bl.x = lleft[majr] + (int)minr * 40 + 6;
		tr.x = bl.x + 32;

		// with tux outlines:
		// if (i<num) bott = 0.5; else bott = 0.0;
		// top = bott + 0.5;
		if (i<num) {
			double bott = 0.5;
			double top = 1.0;
			glBegin (GL_QUADS);
				glTexCoord2f (0, bott); glVertex2f (bl.x, bl.y);
				glTexCoord2f (1, bott); glVertex2f (tr.x, bl.y);
				glTexCoord2f (1, top);  glVertex2f (tr.x, tr.y);
				glTexCoord2f (0, top);  glVertex2f (bl.x, tr.y);
			glEnd();
		}
	}
}

void DrawCursor () {
	Tex.Draw (MOUSECURSOR, cursor_pos.x, cursor_pos.y,
		CURSOR_SIZE  * (double)Winsys.resolution.width / 14000);
}


// ------------------ Main GUI functions ---------------------------------------------

void DrawGUI() {
	for(size_t i = 0; i < Widgets.size(); i++)
		if(Widgets[i]->GetVisible())
			Widgets[i]->Draw();
	if(param.ice_cursor)
		DrawCursor ();
}

TWidget* ClickGUI(int x, int y) {
	TWidget* clicked = NULL;
	for(size_t i = 0; i < Widgets.size(); i++)
		if(Widgets[i]->Click(x, y))
			clicked = Widgets[i];
	return clicked;
}

static int focussed = -1;
TWidget* MouseMoveGUI(int x, int y) {
	if(x != 0 || y != 0) {
		focussed = -1;
		for(size_t i = 0; i < Widgets.size(); i++) {
			Widgets[i]->MouseMove(cursor_pos.x, cursor_pos.y);
			if(Widgets[i]->focussed())
				focussed = (int)i;
		}
	}
	if(focussed == -1)
		return 0;

	return Widgets[focussed];
}

TWidget* KeyGUI(unsigned int key, unsigned int mod, bool released) {
	if(!released) {
		switch (key) {
			case SDLK_TAB:
				IncreaseFocus();
				break;
			default:
				break;
		}
	}
	if(focussed == -1)
		return 0;
	Widgets[focussed]->Key(key, mod, released);
	return Widgets[focussed];
}

void SetFocus(TWidget* widget) {
	if(!widget)
		focussed = -1;
	else
		for(int i = 0; i < (int)Widgets.size(); i++)
			if(Widgets[i] == widget) {
				focussed = i;
				break;
			}
}

void IncreaseFocus() {
	if(focussed >= 0)
		Widgets[focussed]->focus = false;

	focussed++;
	if(focussed >= (int)Widgets.size())
		focussed = 0;
	int end = focussed;
	// Select only active widgets
	do {
		if(Widgets[focussed]->GetActive())
			break;

		focussed++;
		if(focussed >= (int)Widgets.size())
			focussed = 0;
	} while(end != focussed);

	if(focussed >= 0)
		Widgets[focussed]->focus = true;
}
void DecreaseFocus() {
	if(focussed >= 0)
		Widgets[focussed]->focus = false;

	if(focussed > 0)
		focussed--;
	else
		focussed = (int)Widgets.size()-1;
	int end = focussed;
	// Select only active widgets
	do {
		if(Widgets[focussed]->GetActive())
			break;

		if(focussed > 0)
			focussed--;
		else
			focussed = (int)Widgets.size()-1;
	} while(end != focussed);

	if(focussed >= 0)
		Widgets[focussed]->focus = true;
}

void ResetGUI () {
	for(size_t i = 0; i < Widgets.size(); i++)
		delete Widgets[i];
	Widgets.clear();
	focussed = 0;
}

// ------------------ new ---------------------------------------------

int AutoYPosN (double percent) {
	double hh = (double)Winsys.resolution.height;
	double po = hh * percent / 100;
	return (int)(po);
}

TArea AutoAreaN (double top_perc, double bott_perc, int w) {
	TArea res;
	res.top = AutoYPosN (top_perc);
	res.bottom = AutoYPosN (bott_perc);
	if (w > Winsys.resolution.width) w = Winsys.resolution.width;
	double left = (Winsys.resolution.width - w) / 2;
	res.left = (int) left;
	res.right = Winsys.resolution.width - res.left;
	return res;
}
