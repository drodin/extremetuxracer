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

#include "splash_screen.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "course.h"
#include "env.h"
#include "particles.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "score.h"
#include "regist.h"
#include "winsys.h"

CSplashScreen SplashScreen;
sf::Text* Failure = nullptr;
sf::String reason;


void CSplashScreen::Enter() {
	Winsys.ShowCursor(!param.ice_cursor);
	Music.Play(param.menu_music, true);
}

void CSplashScreen::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();
	Trans.LoadTranslations(param.language);  // Before first texts are being displayed

	sf::Sprite logo(Tex.GetSFTexture(TEXLOGO));
	logo.setScale(Winsys.scale/2.f, Winsys.scale/2.f);
	logo.setPosition((Winsys.resolution.width - logo.getTextureRect().width*(Winsys.scale / 2)) / 2, 60);

	if (!Failure) {
		FT.AutoSizeN(6);
		sf::Text t1(Trans.Text(67), FT.getCurrentFont(), FT.GetSize());
		int top = AutoYPosN(60);
		t1.setPosition((Winsys.resolution.width - t1.getLocalBounds().width) / 2, top);
		sf::Text t2(Trans.Text(68), FT.getCurrentFont(), FT.GetSize());
		int dist = FT.AutoDistanceN(3);
		t2.setPosition((Winsys.resolution.width - t2.getLocalBounds().width) / 2, top + dist);

		Winsys.draw(t1);
		Winsys.draw(t2);
	} else {
		Winsys.draw(*Failure);
	}
	Winsys.draw(logo);
	Winsys.SwapBuffers();

	if (!Failure) {
		init_ui_snow();

		Course.MakeStandardPolyhedrons();
		Sound.LoadSoundList();
		if (!Char.LoadCharacterList())
			reason += Trans.Text(93) + "\n";
		Course.LoadObjectTypes();
		if (!Course.LoadTerrainTypes())
			reason += Trans.Text(95) + "\n";
		if (Env.LoadEnvironmentList()) {
			if (Course.LoadCourseList()) {
				Score.LoadHighScore();  // after LoadCourseList !!!
				Events.LoadEventList();

				if (Players.LoadAvatars()) {  // before LoadPlayers !!!
					Players.LoadPlayers();
				} else
					reason += Trans.Text(96) + "\n";
			} else
				reason += Trans.Text(92) + "\n";
		} else
			reason += Trans.Text(94) + "\n";

		if (reason.isEmpty())
			State::manager.RequestEnterState(Regist);
		else { // Failure
			FT.AutoSizeN(6);
			int top = AutoYPosN(60);
			Failure = new sf::Text(reason, FT.getCurrentFont(), FT.GetSize());
			Failure->setFillColor(colDRed);
			Failure->setOutlineColor(colDRed);
			Failure->setPosition((Winsys.resolution.width - Failure->getLocalBounds().width) / 2, top);
		}
	}
}
