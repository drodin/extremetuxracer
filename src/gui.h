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


struct TRect {
	int left;
	int top;
	int width;
	int height;
};

struct TArea {
	int left;
	int right;
	int top;
	int bottom;
};


class TTexture;

class TWidget {
protected:
	TRect mouseRect;
	TVector2i position;
	bool active;
	bool visible;
	bool interactive;
public:
	bool focus;

	TWidget(int x, int y, int width, int height, bool interactive_ = true);
	virtual ~TWidget() {}
	virtual void Draw() const = 0;
	virtual bool Click(int x, int y);
	virtual void TextEnter(char text) {}
	virtual void Key(sf::Keyboard::Key key, bool released) {}
	virtual void MouseMove(int x, int y);
	virtual void Focussed() {}
	virtual void Activated() {}
	bool focussed() const { return focus; }
	void SetActive(bool a) { active = a; if (!a) focus = false; Activated(); }
	void SetVisible(bool v) { visible = v; if (!v) focus = false; }
	bool GetActive() const { return active; }
	bool GetVisible() const { return visible; }
	bool GetInteractive() const { return interactive; }
};

class TLabel : public TWidget {
	sf::Text text;
public:
	TLabel(const sf::String& string, int x, int y, const sf::Color& color);
	void Focussed(bool masterFocus);
	void Draw() const;
	sf::Vector2f GetSize() const;
};
TLabel* AddLabel(const sf::String& string, int x, int y, const sf::Color& color);

class TFramedText : public TWidget {
	sf::RectangleShape frame;
	sf::Text text;
	bool borderFocus;
public:
	TFramedText(int x, int y, int width, int height, int line, const sf::Color& backcol, const sf::String& string, float ftsize, bool borderFocus_ = false);
	void Focussed(bool masterFocus);
	void Activated();
	void Draw() const;
	void SetString(const sf::String& string) { text.setString(string); }
};
TFramedText* AddFramedText(int x, int y, int width, int height, int line, const sf::Color& backcol, const sf::String& text, float ftsize, bool borderFocus = false);

class TTextButton : public TWidget {
	sf::Text text;
public:
	TTextButton(int x, int y, const sf::String& text_, float ftsize);
	void Focussed();
	void Draw() const;
};
TTextButton* AddTextButton(const sf::String& text, int x, int y, float ftsize);
TTextButton* AddTextButtonN(const sf::String& text, int x, int y, int rel_ftsize);

class TTextField : public TWidget {
	sf::Text text;
	sf::RectangleShape frame;
	sf::RectangleShape cursorShape;
	size_t cursorPos;
	size_t maxLng;
	float time;
	bool cursor;

	void SetCursorPos(size_t new_pos);
public:
	TTextField(int x, int y, int width, int height, const sf::String& text_);
	void Draw() const;
	void TextEnter(char text);
	void Key(sf::Keyboard::Key key, bool released);
	void Focussed();
	void UpdateCursor(float timestep);
	const sf::String& Text() const { return text.getString(); }
};
TTextField* AddTextField(const sf::String& text, int x, int y, int width, int height);

class TCheckbox : public TWidget {
	sf::Text text;
	sf::Sprite back, checkmark;
public:
	bool checked;

	TCheckbox(int x, int y, int width_, const sf::String& tag_);
	void Draw() const;
	void SetPosition(int x, int y);
	void SetChecked(bool c) { checked = c; }
	void Focussed();
	bool Click(int x, int y);
	void Key(sf::Keyboard::Key key, bool released);
};
TCheckbox* AddCheckbox(int x, int y, int width, const sf::String& tag);

class TIconButton : public TWidget {
	sf::Sprite sprite;
	sf::RectangleShape frame;
	double size;
	int maximum;
	int value;
public:
	TIconButton(int x, int y, const sf::Texture& texture, double size_, int max_, int value_);
	int GetValue() const { return value; }
	void SetValue(int _value);
	void Draw() const;
	void Focussed();
	bool Click(int x, int y);
	void Key(sf::Keyboard::Key key, bool released);
};
TIconButton* AddIconButton(int x, int y, const sf::Texture& texture, double size, int maximum, int value);

class TArrow : public TWidget {
	sf::Sprite sprite;
	void SetTexture();
public:
	TArrow(int x, int y, bool down_);
	bool down;
	void Activated();
	void Focussed();
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
	void Key(sf::Keyboard::Key key, bool released);
	void MouseMove(int x, int y);
};
TUpDown* AddUpDown(int x, int y, int minimum, int maximum, int value, int distance = 2);

// --------------------------------------------------------------------

void DrawGUI();
TWidget* ClickGUI(int x, int y);
TWidget* MouseMoveGUI(int x, int y);
TWidget* KeyGUI(sf::Keyboard::Key key, bool released);
TWidget* TextEnterGUI(char text);
void SetFocus(TWidget* widget);
void IncreaseFocus();
void DecreaseFocus();
void ResetGUI();

// --------------------------------------------------------------------

void DrawFrameX(int x, int y, int w, int h, int line,
                const sf::Color& backcol, const sf::Color& framecol, double transp);
void DrawBonusExt(int y, size_t numraces, size_t num);
void DrawGUIBackground(float scale);
void DrawGUIFrame();
void DrawCursor();

// --------------------------------------------------------------------

int AutoYPosN(double percent);
TArea AutoAreaN(double top_perc, double bott_perc, unsigned int w);

#endif
