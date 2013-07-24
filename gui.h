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


#define CENTER -1
#define CRSR_PERIODE 0.4


class TTexture;

class TWidget {
protected:
	TRect mouseRect;
	TVector2 position;
	bool active;
	bool visible;
public:
	bool focus;

	TWidget(int x, int y, int width, int height);
	virtual void Draw() const = 0;
	virtual bool Click(int x, int y);
	virtual void Key(unsigned int key, unsigned int mod, bool released) {}
	virtual void MouseMove(int x, int y);
	bool focussed() const { return focus; }
	void SetActive(bool a) { active = a; if(!a) focus = false; }
	void SetVisible(bool v) { visible = v; if(!v) focus = false; }
	bool GetActive() const { return active; }
	bool GetVisible() const { return visible; }
};

class TTextButton : public TWidget {
	string text;
	double ftsize;	// font height
public:
	TTextButton(int x, int y, const string& text_, double ftsize_);
	void Draw() const;
};
TTextButton* AddTextButton (const string& text, int x, int y, double ftsize);
TTextButton* AddTextButtonN (const string& text, int x, int y, int rel_ftsize);

class TTextField : public TWidget {
	string text;
	size_t cursorPos;
	size_t maxLng;
	double time;
	bool cursor;
public:
	TTextField(int x, int y, int width, int height, const string& text_);
	void Draw() const;
	void Key(unsigned int key, unsigned int mod, bool released);
	void UpdateCursor(double timestep);
	const string& Text() const { return text; }
};
TTextField* AddTextField(const string& text, int x, int y, int width, int height);

class TCheckbox : public TWidget {
	string tag;
	int width;
public:
	bool checked;

	TCheckbox(int x, int y, int width_, const string& tag_)
		: TWidget(x, y, 32, 32)
		, tag(tag_)
		, width(width_)
		, checked(false)
	{
		mouseRect.left = x+width-32;
	}
	void Draw() const;
	bool Click(int x, int y);
	void Key(unsigned int key, unsigned int mod, bool released);
};
TCheckbox* AddCheckbox (int x, int y, int width, const string& tag);

class TIconButton : public TWidget {
	double size;
	TTexture* texture;
	int maximum;
	int value;
public:
	TIconButton(int x, int y, TTexture* texture_, double size_, int max_, int value_)
		: TWidget(x, y, 32, 32)
		, size(size_)
		, texture(texture_)
		, maximum(max_)
		, value(value_)
	{}
	int GetValue() const { return value; }
	void SetValue(int _value);
	void Draw() const;
	bool Click(int x, int y);
	void Key(unsigned int key, unsigned int mod, bool released);
};
TIconButton* AddIconButton (int x, int y, TTexture* texture, double size, int maximum, int value);

class TArrow : public TWidget {
public:
	TArrow(int x, int y, bool down_)
		: TWidget(x, y, 32, 16)
		, down(down_)
	{}
	bool down;
	void Draw() const;
};
TArrow* AddArrow(int x, int y, bool down);

class TUpDown : public TWidget {
	TArrow up;
	TArrow down;
	int value;
	int minimum;
	int maximum;
public:
	TUpDown(int x, int y, int min_, int max_, int value_, int distance);
	int GetValue() const { return value; }
	void SetValue(int value_);
	void SetMinimum(int min_);
	void SetMaximum(int max_);
	void Draw() const;
	bool Click(int x, int y);
	void Key(unsigned int key, unsigned int mod, bool released);
	void MouseMove(int x, int y);
};
TUpDown* AddUpDown(int x, int y, int minimum, int maximum, int value, int distance = 2);

// --------------------------------------------------------------------

void DrawGUI();
TWidget* ClickGUI(int x, int y);
TWidget* MouseMoveGUI(int x, int y);
TWidget* KeyGUI(unsigned int key, unsigned int spec, bool released);
void SetFocus(TWidget* widget);
void IncreaseFocus();
void DecreaseFocus();
void ResetGUI();

// --------------------------------------------------------------------

void DrawFrameX (int x, int y, int w, int h, int line,
			const TColor& backcol, const TColor& framecol, double transp);
void DrawLevel (int x, int y, int level, double fact);
void DrawBonus (int x, int y, size_t max, size_t num);
void DrawBonusExt (int y, size_t numraces, size_t num);
void DrawCursor ();

// --------------------------------------------------------------------

int AutoYPosN (double percent);
TArea AutoAreaN (double top_perc, double bott_perc, int w);

#endif
