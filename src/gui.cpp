/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "gui.h"
#include "textures.h"
#include "font.h"
#include "ogl.h"
#include "winsys.h"
#include <vector>


#define CURSOR_SIZE 10

static std::vector<TWidget*> Widgets;
static int lock_focussed = -1;
static int focussed = -1;
static bool locked_LR = false;
static bool locked_UD = false;

static TWidget* AddWidget(TWidget* widget) {
	if (Widgets.size() == focussed) {
		widget->focus = true;
		widget->Focussed();
	}
	Widgets.push_back(widget);
	return widget;
}

static bool Inside(int x, int y, const TRect& Rect) {
	return (x >= Rect.left
	        && x <= Rect.left + Rect.width
	        && y >= Rect.top
	        && y <= Rect.top + Rect.height);
}

TWidget::TWidget(int x, int y, int width, int height, bool interactive_)
	: active(true)
	, visible(true)
	, interactive(interactive_)
	, focus(false) {
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
	bool ofocus = focus;
	focus = interactive && active && visible && Inside(x, y, mouseRect);
	if (ofocus != focus)
		Focussed();
}


TLabel::TLabel(const sf::String& string, int x, int y, const sf::Color& color)
	: TWidget(x, y, 0, 0, false)
	, text(string, FT.getCurrentFont(), FT.GetSize()) {
	if (x == CENTER)
		text.setPosition((Winsys.resolution.width - text.getLocalBounds().width) / 2, y);
	else
		text.setPosition(x, y);
	text.setFillColor(color);
	text.setOutlineColor(color);
}

void TLabel::Focussed(bool masterFocus) {
	focus = masterFocus && active;
	if (focus) {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
	} else {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
	}
}

void TLabel::Draw() const {
	Winsys.draw(text);
}

sf::Vector2f TLabel::GetSize() const {
	return sf::Vector2f(text.getLocalBounds().width, text.getLocalBounds().height);

}

TLabel* AddLabel(const sf::String& string, int x, int y, const sf::Color& color) {
	return static_cast<TLabel*>(AddWidget(new TLabel(string, x, y, color)));
}


TFramedText::TFramedText(int x, int y, int width, int height, int line, const sf::Color& backcol, const sf::String& string, unsigned int ftsize, bool borderFocus_)
	: TWidget(x, y, width, height, false)
	, frame(sf::Vector2f(width - line * 2, height - line * 2))
	, text(string, FT.getCurrentFont(), ftsize)
	, borderFocus(borderFocus_) {
	text.setPosition(x + line + 20, y + line);
	if (!borderFocus) {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
	} else {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
	}
	frame.setPosition(x + line, y + line);
	frame.setOutlineThickness(line);
	frame.setFillColor(backcol);
	frame.setOutlineColor(colWhite);
}

void TFramedText::Activated() {
	if (!active) {
		text.setFillColor(colLGrey);
		text.setOutlineColor(colLGrey);
	} else if (borderFocus || focus) {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
	} else {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
	}
}

void TFramedText::Focussed(bool masterFocus) {
	focus = masterFocus && active;
	if (focus) {
		frame.setOutlineColor(colDYell);
		if (!borderFocus) {
			text.setFillColor(colDYell);
			text.setOutlineColor(colDYell);
		}
	} else {
		frame.setOutlineColor(colWhite);
		if (!borderFocus) {
			text.setFillColor(colWhite);
			text.setOutlineColor(colWhite);
		}
	}
}

void TFramedText::Draw() const {
	Winsys.draw(frame);
	Winsys.draw(text);
}

TFramedText* AddFramedText(int x, int y, int width, int height, int line, const sf::Color& backcol, const sf::String& text, unsigned int ftsize, bool borderFocus) {
	return static_cast<TFramedText*>(AddWidget(new TFramedText(x, y, width, height, line, backcol, text, ftsize, borderFocus)));
}

TTextButton::TTextButton(int x, int y, const sf::String& text_, int ftsize)
	: TWidget(x, y, 0, 0)
	, text(text_, FT.getCurrentFont(), ftsize) {
	if (ftsize < 0) text.setCharacterSize(FT.AutoSizeN(4));

	int len = text.getLocalBounds().width;
	if (x == CENTER) position.x = (Winsys.resolution.width - len) / 2;
	text.setPosition(position.x, position.y);
	int offs = ftsize / 5;
	mouseRect.left = position.x-20;
	mouseRect.top = position.y+offs;
	mouseRect.width = len+40;
	mouseRect.height = ftsize+offs;
}

void TTextButton::Focussed() {
	if (focus) {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
	} else {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
	}
}

void TTextButton::Draw() const {
	Winsys.draw(text);
}

TTextButton* AddTextButton(const sf::String& text, int x, int y, int ftsize) {
	return static_cast<TTextButton*>(AddWidget(new TTextButton(x, y, text, ftsize)));
}

TTextButton* AddTextButtonN(const sf::String& text, int x, int y, int rel_ftsize) {
	unsigned int siz = FT.AutoSizeN(rel_ftsize);
	return AddTextButton(text, x, y, siz);
}


TTextField::TTextField(int x, int y, int width, int height, const sf::String& text_)
	: TWidget(x, y, width, height)
	, text(text_, FT.getCurrentFont(), FT.AutoSizeN(5))
	, frame(sf::Vector2f(width-6.f, height-6.f))
	, cursorShape(sf::Vector2f(2.f, 30.f * Winsys.scale))
	, maxLng(32)
	, time(0.0)
	, cursor(false) {
	text.setPosition(mouseRect.left + 20, mouseRect.top);
	cursorShape.setFillColor(colYellow);
	frame.setPosition(x + 3, y + 3);
	frame.setOutlineThickness(3);
	frame.setFillColor(colMBackgr);
	frame.setOutlineColor(colWhite);
	SetCursorPos(0);
}

void TTextField::Draw() const {
	Winsys.draw(frame);
	Winsys.draw(text);
	if (cursor && focus)
		Winsys.draw(cursorShape);
}

void TTextField::TextEnter(char c) {
	if (c != '\b') {
		sf::String string = text.getString();
		string.insert(cursorPos, c);
		text.setString(string);
		SetCursorPos(cursorPos+1);
	}
}

void TTextField::SetCursorPos(std::size_t new_pos) {
	cursorPos = new_pos;
	cursorShape.setPosition(text.findCharacterPos(cursorPos).x, mouseRect.top + 9);
}

void TTextField::Focussed() {
	if (focus) {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
		frame.setOutlineColor(colDYell);
	} else {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
		frame.setOutlineColor(colWhite);
	}
}

bool TTextField::Click(int x, int y) {
	if (TWidget::Click(x, y)) {
		cursorPos = 0;
		float first = text.findCharacterPos(cursorPos).x;
		for (;;) {
			float second = text.findCharacterPos(cursorPos + 1).x;
			if ((first + second) / 2.f >= x || cursorPos >= text.getString().getSize())
				break;
			cursorPos++;
			first = second;
		}
		cursorShape.setPosition(text.findCharacterPos(cursorPos).x, mouseRect.top + 9);
		return true;
	}
	return false;
}

static void eraseFromText(sf::Text& text, std::size_t pos) {
	sf::String str = text.getString();
	str.erase(pos, 1);
	text.setString(str);
}
void TTextField::Key(sf::Keyboard::Key key, bool released) {
	switch (key) {
		case sf::Keyboard::Delete:
			if (cursorPos < text.getString().getSize()) eraseFromText(text, cursorPos);
			break;
		case sf::Keyboard::BackSpace:
			if (cursorPos > 0) { eraseFromText(text, cursorPos-1); SetCursorPos(cursorPos - 1); }
			break;
		case sf::Keyboard::Right:
			if (cursorPos < text.getString().getSize()) SetCursorPos(cursorPos + 1);
			break;
		case sf::Keyboard::Left:
			if (cursorPos > 0) SetCursorPos(cursorPos - 1);
			break;
		case sf::Keyboard::Home:
			SetCursorPos(0);
			break;
		case sf::Keyboard::End:
			SetCursorPos(text.getString().getSize());
			break;
		default:
			break;
	}
}

void TTextField::UpdateCursor(float timestep) {
	time += timestep;
	if (time > CRSR_PERIODE) {
		time = 0;
		cursor = !cursor;
	}
}

TTextField* AddTextField(const sf::String& text, int x, int y, int width, int height) {
	locked_LR = true;
	return static_cast<TTextField*>(AddWidget(new TTextField(x, y, width, height, text)));
}

TCheckbox::TCheckbox(int x, int y, int width, const sf::String& tag_)
	: TWidget(x, y, 32 * Winsys.scale / 0.8f, 32 * Winsys.scale / 0.8f)
	, text(tag_, FT.getCurrentFont(), FT.GetSize())
	, back(Tex.GetSFTexture(CHECKBOX))
	, checkmark(Tex.GetSFTexture(CHECKMARK_SMALL))
	, checked(false) {
	text.setPosition(x, y);
	back.setPosition(x + width - 32, y);
	checkmark.setPosition(x + width - 32, y);
	mouseRect.left = x + width - 32;
	back.setScale(Winsys.scale / 0.8f, Winsys.scale / 0.8f);
	checkmark.setScale(Winsys.scale / 0.8f, Winsys.scale / 0.8f);
}

void TCheckbox::SetPosition(int x, int y) {
	text.setPosition(x, y);
	back.setPosition(x, y);
	checkmark.setPosition(x, y);
}

void TCheckbox::Focussed() {
	if (focus) {
		text.setFillColor(colDYell);
		text.setOutlineColor(colDYell);
	} else {
		text.setFillColor(colWhite);
		text.setOutlineColor(colWhite);
	}
}

void TCheckbox::Draw() const {
	Winsys.draw(back);
	if (checked)
		Winsys.draw(checkmark);
	Winsys.draw(text);
}

bool TCheckbox::Click(int x, int y) {
	if (active && visible && Inside(x, y, mouseRect)) {
		checked = !checked;
		return true;
	}
	return false;
}

void TCheckbox::Key(sf::Keyboard::Key key, bool released) {
	if (released) return;

	if (key == sf::Keyboard::Space || key == sf::Keyboard::Return) {
		checked = !checked;
	}
}

TCheckbox* AddCheckbox(int x, int y, int width, const sf::String& tag) {
	return static_cast<TCheckbox*>(AddWidget(new TCheckbox(x, y, width, tag)));
}

TIconButton::TIconButton(int x, int y, const sf::Texture& texture, float size_, int max_, int value_)
	: TWidget(x, y, 32, 32)
	, sprite(texture)
	, frame(sf::Vector2f(size_, size_))
	, size(size_)
	, maximum(max_)
	, value(value_) {
	sprite.setScale(size / (texture.getSize().x / 2.f), size / (texture.getSize().y / 2.f));
	sprite.setPosition(x, y);
	frame.setPosition(x, y);
	frame.setOutlineColor(colWhite);
	frame.setOutlineThickness(3.f);
	SetValue(value_);
}

void TIconButton::SetValue(int _value) {
	value = _value;
	if (value > maximum)
		value = 0;
	else if (value < 0)
		value = maximum;

	sf::Vector2u texSize = sprite.getTexture()->getSize();
	switch (value) {
		case 0:
			sprite.setTextureRect(sf::IntRect(0, 0, texSize.x / 2, texSize.y / 2));
			break;
		case 1:
			sprite.setTextureRect(sf::IntRect(texSize.x / 2, 0, texSize.x / 2, texSize.y / 2));
			break;
		case 2:
			sprite.setTextureRect(sf::IntRect(0, texSize.y / 2, texSize.x / 2, texSize.y / 2));
			break;
		case 3:
			sprite.setTextureRect(sf::IntRect(texSize.x / 2, texSize.y / 2, texSize.x / 2, texSize.y / 2));
			break;
	}
}

void TIconButton::Draw() const {
	Winsys.draw(frame);
	Winsys.draw(sprite);
}

void TIconButton::Focussed() {
	if (focus)
		frame.setOutlineColor(colDYell);
	else
		frame.setOutlineColor(colWhite);
}

bool TIconButton::Click(int x, int y) {
	if (Inside(x, y, mouseRect)) {
		SetValue(value + 1);
		return true;
	}
	return false;
}

void TIconButton::Key(sf::Keyboard::Key key, bool released) {
	if (released) return;

	if (key == sf::Keyboard::Down) { // Arrow down/left
		SetValue(value - 1);
	} else if (key == sf::Keyboard::Up) { // Arrow up/right
		SetValue(value + 1);
	}
}

TIconButton* AddIconButton(int x, int y, const sf::Texture& texture, float size, int maximum, int value) {
	locked_UD = true;
	return static_cast<TIconButton*>(AddWidget(new TIconButton(x, y, texture, size, maximum, value)));
}

TArrow::TArrow(int x, int y, bool down_)
	: TWidget(x, y, 32 * Winsys.scale / 0.8f, 16 * Winsys.scale / 0.8f)
	, sprite(Tex.GetSFTexture(LB_ARROWS))
	, down(down_) {
	sprite.setPosition(x, y);
	sprite.setScale(Winsys.scale / 0.8f, Winsys.scale / 0.8f);

	SetTexture();
}

void TArrow::Focussed() {
	SetTexture();
}

void TArrow::Activated() {
	SetTexture();
}

void TArrow::SetTexture() {
	static const float textl[6] = { 0.5f, 0.f, 0.5f, 0.5f, 0.f, 0.5f };
	static const float texbr[6] = { 0.5f, 0.5f, 0.f, 0.75f, 0.75f, 0.25f };

	int type = 0;
	if (active)
		type = 1;
	if (focus)
		type++;
	if (down)
		type += 3;

	sf::Vector2u texSize = sprite.getTexture()->getSize();
	sprite.setTextureRect(sf::IntRect(textl[type] * texSize.x, texbr[type] * texSize.y, texSize.x / 2, texSize.y / 4));
}

void TArrow::Draw() const {
	Winsys.draw(sprite);
}

TArrow* AddArrow(int x, int y, bool down) {
	return static_cast<TArrow*>(AddWidget(new TArrow(x, y, down)));
}


TUpDown::TUpDown(int x, int y, int min_, int max_, int value_, int distance, bool swapArrows_)
	: TWidget(x, y, 32 * Winsys.scale / 0.8f, (32 + distance)*Winsys.scale / 0.8f)
	, up(x, y + (16 + distance)*Winsys.scale / 0.8f, true)
	, down(x, y, false)
	, higher(swapArrows_ ? up : down)
	, lower(swapArrows_ ? down : up)
	, value(value_)
	, minimum(min_)
	, maximum(max_)
	, swapArrows(swapArrows_) {
	lower.SetActive(value < maximum);
	higher.SetActive(value > minimum);
}

void TUpDown::Draw() const {
	up.Draw();
	down.Draw();
}

bool TUpDown::Click(int x, int y) {
	if (active && visible && lower.Click(x, y)) {
		value++;
		higher.SetActive(true);
		if (value == maximum)
			lower.SetActive(false);
		return true;
	}
	if (active && visible && higher.Click(x, y)) {
		lower.SetActive(true);
		value--;
		if (value == minimum)
			down.SetActive(false);
		return true;
	}
	return false;
}

void TUpDown::Key(sf::Keyboard::Key key, bool released) {
	if (released) return;

	if ((!swapArrows && key == sf::Keyboard::Up) || (swapArrows && key == sf::Keyboard::Down)) { // Arrow up
		if (value > minimum) {
			value--;
			lower.SetActive(true);
			if (value == minimum)
				higher.SetActive(false);
		}
	} else if ((!swapArrows && key == sf::Keyboard::Down) || (swapArrows && key == sf::Keyboard::Up)) { // Arrow down
		if (value < maximum) {
			value++;
			higher.SetActive(true);
			if (value == maximum)
				lower.SetActive(false);
		}
	}
}

void TUpDown::MouseMove(int x, int y) {
	bool ofocus = focus;
	focus = active && visible && Inside(x, y, mouseRect);
	if (ofocus != focus)
		Focussed();
	up.MouseMove(x, y);
	down.MouseMove(x, y);
}

void TUpDown::SetValue(int value_) {
	value = clamp(minimum, value_, maximum);
	lower.SetActive(value < maximum);
	higher.SetActive(value > minimum);
}
void TUpDown::SetMinimum(int min_) {
	minimum = min_;
	value = clamp(minimum, value, maximum);
	lower.SetActive(value < maximum);
	higher.SetActive(value > minimum);
}
void TUpDown::SetMaximum(int max_) {
	maximum = max_;
	value = clamp(minimum, value, maximum);
	lower.SetActive(value < maximum);
	higher.SetActive(value > minimum);
}

TUpDown* AddUpDown(int x, int y, int minimum, int maximum, int value, int distance, bool swapArrows) {
	locked_UD = true;
	return static_cast<TUpDown*>(AddWidget(new TUpDown(x, y, minimum, maximum, value, distance, swapArrows)));
}

// ------------------ Elementary drawing ---------------------------------------------

void DrawFrameX(int x, int y, int w, int h, int line, const sf::Color& backcol, const sf::Color& framecol, float transp) {
	x += line;
	y += line;
	w -= line * 2;
	h -= line * 2;
	sf::RectangleShape shape(sf::Vector2f(w, h));
	shape.setPosition(x, y);
	shape.setOutlineThickness(line);
	shape.setFillColor(sf::Color(backcol.r, backcol.g, backcol.b, backcol.a * transp));
	shape.setOutlineColor(sf::Color(framecol.r, framecol.g, framecol.b, framecol.a * transp));
	Winsys.draw(shape);
}

void DrawBonusExt(int y, std::size_t numraces, std::size_t num) {
	std::size_t maxtux = numraces * 3;
	if (num > maxtux) return;

	static const sf::Color col2(115, 166, 217);

	int lleft[3];

	int framewidth = (int)numraces * 40 + 8;
	int totalwidth = framewidth * 3 + 8;
	int xleft = (Winsys.resolution.width - totalwidth) / 2;
	lleft[0] = xleft;
	lleft[1] = xleft + framewidth + 4;
	lleft[2] = xleft + framewidth + framewidth + 8;

	DrawFrameX(lleft[0], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX(lleft[1], y, framewidth, 40, 1, col2, colBlack, 1);
	DrawFrameX(lleft[2], y, framewidth, 40, 1, col2, colBlack, 1);

	static sf::Sprite tuxbonus(Tex.GetSFTexture(TUXBONUS));
	sf::Vector2u size = tuxbonus.getTexture()->getSize();
	tuxbonus.setTextureRect(sf::IntRect(0, 0, size.x, size.y/2));

	for (std::size_t i=0; i<maxtux; i++) {
		std::size_t majr = (i/numraces);
		std::size_t minr = i - majr * numraces;
		if (majr > 2) majr = 2;
		int x = lleft[majr] + (int)minr * 40 + 6;

		if (i<num) {
			tuxbonus.setPosition(x, y + 4);
			Winsys.draw(tuxbonus);
		}
	}
}

void DrawGUIFrame() {
	static sf::Sprite bottom_left(Tex.GetSFTexture(BOTTOM_LEFT));
	static sf::Sprite bottom_right(Tex.GetSFTexture(BOTTOM_RIGHT));
	static sf::Sprite top_left(Tex.GetSFTexture(TOP_LEFT));
	static sf::Sprite top_right(Tex.GetSFTexture(TOP_RIGHT));

	bottom_left.setPosition(0, Winsys.resolution.height - bottom_left.getTexture()->getSize().y);
	bottom_right.setPosition(Winsys.resolution.width - bottom_right.getTexture()->getSize().x, Winsys.resolution.height - bottom_right.getTexture()->getSize().y);
	top_right.setPosition(Winsys.resolution.width - top_right.getTexture()->getSize().x, 0);

	Winsys.draw(bottom_left);
	Winsys.draw(bottom_right);
	Winsys.draw(top_left);
	Winsys.draw(top_right);
}

void DrawGUIBackground(float scale) {
	DrawGUIFrame();

	static sf::Sprite logo(Tex.GetSFTexture(T_TITLE));
	scale *= 0.5f;
	logo.setScale(scale, scale);
	logo.setPosition((Winsys.resolution.width - logo.getTextureRect().width*scale)/2, 5);
	Winsys.draw(logo);
}

void DrawCursor() {
	static sf::Sprite s(Tex.GetSFTexture(MOUSECURSOR));
	static bool init = false;
	if (!init) {
		s.setScale((double) Winsys.resolution.width / 1400, (double) Winsys.resolution.width / 1400);
		init = true;
	}
	s.setPosition(cursor_pos.x, cursor_pos.y);
	Winsys.draw(s);
}


// ------------------ Main GUI functions ---------------------------------------------

void DrawGUI() {
	for (std::size_t i = 0; i < Widgets.size(); i++)
		if (Widgets[i]->GetVisible())
			Widgets[i]->Draw();
	if (param.ice_cursor)
		DrawCursor();
}

TWidget* ClickGUI(int x, int y) {
	TWidget* clicked = nullptr;
	for (std::size_t i = 0; i < Widgets.size(); i++) {
		if (Widgets[i]->Click(x, y)) {
			clicked = Widgets[i];
			lock_focussed = focussed;
		}
	}
	return clicked;
}

TWidget* MouseMoveGUI(int x, int y) {
	if (x != 0 || y != 0) {
		focussed = -1;
		for (std::size_t i = 0; i < Widgets.size(); i++) {
			Widgets[i]->MouseMove(cursor_pos.x, cursor_pos.y);
			if (Widgets[i]->focussed())
				focussed = (int)i;
		}
	}
	if (focussed == -1) {
		focussed = lock_focussed;
		if (focussed != -1) {
			Widgets[focussed]->focus = true;
			Widgets[focussed]->Focussed();
		}
		return 0;
	}

	return Widgets[focussed];
}

TWidget* KeyGUI(sf::Keyboard::Key key, bool released) {
	if (!released) {
		switch (key) {
			case sf::Keyboard::Tab:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
					DecreaseFocus();
				else
					IncreaseFocus();
				break;
			case sf::Keyboard::Up:
				if (!locked_UD)
					DecreaseFocus();
				break;
			case sf::Keyboard::Left:
				if (!locked_LR)
					DecreaseFocus();
				break;
			case sf::Keyboard::Down:
				if (!locked_UD)
					IncreaseFocus();
				break;
			case sf::Keyboard::Right:
				if (!locked_LR)
					IncreaseFocus();
				break;
			default:
				break;
		}
	}
	if (focussed == -1)
		return 0;
	Widgets[focussed]->Key(key, released);
	return Widgets[focussed];
}

TWidget* TextEnterGUI(char text) {
	if (focussed == -1)
		return 0;
	Widgets[focussed]->TextEnter(text);
	return Widgets[focussed];
}

void SetFocus(TWidget* widget) {
	if (!widget)
		focussed = -1;
	else
		for (std::size_t i = 0; i < Widgets.size(); i++) {
			if (Widgets[i] == widget) {
				Widgets[i]->focus = true;
				Widgets[i]->Focussed();
				focussed = (int)i;
				break;
			} else if (Widgets[i]->focus) {
				Widgets[i]->focus = false;
				Widgets[i]->Focussed();
			}
		}
}

void IncreaseFocus() {
	if (focussed >= 0) {
		Widgets[focussed]->focus = false;
		Widgets[focussed]->Focussed();
	}

	focussed++;
	if (focussed >= (int)Widgets.size())
		focussed = 0;
	int end = focussed;
	// Select only active widgets
	do {
		if (Widgets[focussed]->GetActive() && Widgets[focussed]->GetInteractive())
			break;

		focussed++;
		if (focussed >= (int)Widgets.size())
			focussed = 0;
	} while (end != focussed);

	if (focussed >= 0) {
		Widgets[focussed]->focus = true;
		Widgets[focussed]->Focussed();
	}
	lock_focussed = focussed;
}
void DecreaseFocus() {
	if (focussed >= 0) {
		Widgets[focussed]->focus = false;
		Widgets[focussed]->Focussed();
	}

	if (focussed > 0)
		focussed--;
	else
		focussed = (int)Widgets.size()-1;
	int end = focussed;
	// Select only active widgets
	do {
		if (Widgets[focussed]->GetActive() && Widgets[focussed]->GetInteractive())
			break;

		if (focussed > 0)
			focussed--;
		else
			focussed = (int)Widgets.size()-1;
	} while (end != focussed);

	if (focussed >= 0) {
		Widgets[focussed]->focus = true;
		Widgets[focussed]->Focussed();
	}
	lock_focussed = focussed;
}

void ResetGUI() {
	for (std::size_t i = 0; i < Widgets.size(); i++)
		delete Widgets[i];
	Widgets.clear();
	focussed = 0;
	lock_focussed = -1;
	locked_LR = locked_UD = false;
}

// ---------------------------------------------------------------

int AutoYPosN(int percent) {
	return Winsys.resolution.height * percent / 100;
}

TArea AutoAreaN(int top_perc, int bott_perc, unsigned int w) {
	TArea res;
	res.top = AutoYPosN(top_perc);
	res.bottom = AutoYPosN(bott_perc);
	if (w > Winsys.resolution.width) w = Winsys.resolution.width;
	res.left = (Winsys.resolution.width - w) / 2;
	res.right = Winsys.resolution.width - res.left;
	return res;
}
