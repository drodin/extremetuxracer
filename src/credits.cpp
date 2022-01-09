/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2010 Extreme Tux Racer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "credits.h"
#include "audio.h"
#include "ogl.h"
#include "particles.h"
#include "textures.h"
#include "font.h"
#include "gui.h"
#include "spx.h"
#include "winsys.h"

#define TOP_Y 165
#define BOTT_Y 64
#define FADE 50
#define OFFS_SCALE_FACTOR 1.2f

CCredits Credits;


static float y_offset = 0;
static bool moving = true;
sf::VertexArray arr(sf::TriangleStrip, 12);
sf::RenderStates* states;

void CCredits::LoadCreditList() {
	CSPList list;

	if (!list.Load(param.data_dir, "credits.lst")) {
		Message("could not load credits list");
		return;
	}

	int old_offs = 0;
	CreditList.reserve(list.size());
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		std::string version = "";
		TCredits& credit = CreditList.emplace_back();
		std::string temp = SPStrN(*line, "text");
		credit.text = sf::String::fromUtf8(temp.cbegin(), temp.cend());

		if (version.empty()) {
			version = SPStrN(*line, "version");
			if (!version.empty())
				credit.text += " " ETR_VERSION_STRING;
		}

		int offset = SPFloatN(*line, "offs", 0) * OFFS_SCALE_FACTOR * Winsys.scale;
		if (line != list.cbegin()) credit.offs = old_offs + offset;
		else credit.offs = offset;

		credit.col = SPIntN(*line, "col", 0);
		credit.size = SPFloatN(*line, "size", 1.f);

		old_offs = credit.offs;
	}
}

void CCredits::DrawCreditsText(float time_step) {
	int h = Winsys.resolution.height;
	float offs = 0.f;
	if (moving) y_offset += time_step * 30;

	sf::Text text;
	text.setFont(FT.getCurrentFont());
	for (std::vector<TCredits>::const_iterator i = CreditList.begin(); i != CreditList.end(); ++i) {
		offs = h - TOP_Y - y_offset + i->offs;
		if (offs > h || offs < TOP_Y - 100.f) // Draw only visible lines
			continue;

		if (i->col == 0) {
			text.setFillColor(colWhite);
			text.setOutlineColor(colWhite);
		} else {
			text.setFillColor(colDYell);
			text.setOutlineColor(colDYell);
		}
		text.setCharacterSize(FT.AutoSizeN(i->size)+1);
		text.setString(i->text);
		text.setPosition((Winsys.resolution.width - text.getLocalBounds().width) / 2, offs);
		Winsys.draw(text);
	}

	Winsys.draw(arr, *states);

	if (offs < TOP_Y) y_offset = 0;
}

void CCredits::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case sf::Keyboard::M:
			moving = !moving;
			break;
		case sf::Keyboard::U:
			param.ui_snow = !param.ui_snow;
			break;
		default:
			State::manager.RequestEnterState(*State::manager.PreviousState());
	}
}

void CCredits::Mouse(int button, int state, int x, int y) {
	if (state == 1) State::manager.RequestEnterState(*State::manager.PreviousState());
}

void CCredits::Motion(int x, int y) {
	if (param.ui_snow) push_ui_snow(cursor_pos);
}

void CCredits::Enter() {
	LoadCreditList();

	Music.Play(param.credits_music, true);
	y_offset = 0;
	moving = true;

	float w = Winsys.resolution.width;
	float h = Winsys.resolution.height;
	arr[0] = sf::Vertex(sf::Vector2f(0, 0), colBackgr);
	arr[1] = sf::Vertex(sf::Vector2f(w, 0), colBackgr);

	arr[2] = sf::Vertex(sf::Vector2f(0, TOP_Y), colBackgr);
	arr[3] = sf::Vertex(sf::Vector2f(w, TOP_Y), colBackgr);

	arr[4] = sf::Vertex(sf::Vector2f(0, TOP_Y + FADE), colTBackr);
	arr[5] = sf::Vertex(sf::Vector2f(w, TOP_Y + FADE), colTBackr);

	arr[6] = sf::Vertex(sf::Vector2f(0, h - BOTT_Y), colTBackr);
	arr[7] = sf::Vertex(sf::Vector2f(w, h - BOTT_Y), colTBackr);

	arr[8] = sf::Vertex(sf::Vector2f(0, h - BOTT_Y + FADE), colBackgr);
	arr[9] = sf::Vertex(sf::Vector2f(w, h - BOTT_Y + FADE), colBackgr);

	arr[10] = sf::Vertex(sf::Vector2f(0, h), colBackgr);
	arr[11] = sf::Vertex(sf::Vector2f(w, h), colBackgr);

	states = new sf::RenderStates(sf::BlendAlpha);
}

void CCredits::Exit() {
	delete states;
	states = nullptr;
	CreditList.clear();
}

void CCredits::Loop(float time_step) {
	check_gl_error();
	ClearRenderContext();
	Winsys.clear();

	DrawCreditsText(time_step);
	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}
	DrawGUIBackground(Winsys.scale);

	Winsys.SwapBuffers();
}
