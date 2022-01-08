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

#include "help.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "font.h"
#include "gui.h"
#include "translation.h"
#include "winsys.h"

CHelp Help;

#define TEXT_LINES 13
TLabel* headline;
TLabel* texts[TEXT_LINES];
TLabel* footnote;

#if defined(MOBILE)
#define LICENSE_LINES 11
#elif defined(MACOS)
#define LICENSE_LINES 7
#endif

#if defined(MOBILE) || defined(MACOS)
sf::String license[LICENSE_LINES] = {
#ifdef MOBILE
	"Controls:",
	"Touch screen center to prepare for jump, release to jump.",
	"Enable touch paddle/brake controls in Configuration.",
	"",
#endif
	"Privacy policy:",
	"We don't collect any personal data from our apps.",
	"",
	"License:",
	"This application is released under the GPLv2 license",
	"The full source code is available at:",
	"https://github.com/drodin/extremetuxracer",
};
TLabel* labels[LICENSE_LINES];
#endif

void CHelp::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	State::manager.RequestEnterState(*State::manager.PreviousState());
}

void CHelp::Mouse(int button, int state, int x, int y) {
	if (state == 1) State::manager.RequestEnterState(*State::manager.PreviousState());
}

void CHelp::Motion(int x, int y) {
	if (param.ui_snow) push_ui_snow(cursor_pos);
}

void CHelp::Enter() {
	ResetGUI();
	Winsys.ShowCursor(false);
	Music.Play(param.credits_music, true);

#ifndef MOBILE
	int ytop = AutoYPosN(15);
#else
	int ytop = AutoYPosN(3);
#endif

	int xleft1 = 40;

	FT.AutoSizeN(4);
#ifndef MOBILE
	headline = AddLabel(Trans.Text(57), xleft1, AutoYPosN(5), colWhite);

	FT.AutoSizeN(3);
	int offs = FT.AutoDistanceN(2);
	for (int i = 0; i < TEXT_LINES; i++)
		texts[i] = AddLabel(Trans.Text(44 + i), xleft1, ytop + offs*i, colWhite);

	footnote = AddLabel(Trans.Text(65), CENTER, AutoYPosN(90), colWhite);
#endif
#if defined(MACOS)
	xleft1 = Winsys.resolution.width / 2;
#elif defined(IOS)
	xleft1 = Winsys.resolution.width / 20;
#endif
#if defined(MOBILE) || defined(MACOS)
	int loffs = FT.AutoDistanceN(2);
	for (int i = 0; i < LICENSE_LINES; i++)
		labels[i] = AddLabel(license[i], xleft1, ytop + loffs*i, colWhite);
#endif
}

void CHelp::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}

	DrawGUI();
	Winsys.SwapBuffers();
}
