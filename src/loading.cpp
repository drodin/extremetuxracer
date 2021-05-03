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

#include "loading.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "font.h"
#include "particles.h"
#include "course.h"
#include "env.h"
#include "translation.h"
#include "gui.h"
#include "intro.h"
#include "winsys.h"

CLoading Loading;

// ====================================================================
void CLoading::Enter() {
	Winsys.ShowCursor(false);
	Music.Play("loading", true);
}

void CLoading::Loop(float time_step) {
	ScopedRenderMode rm(GUI);
	Winsys.clear();

	if (param.ui_snow) {
		update_ui_snow(time_step);
		draw_ui_snow();
	}

	sf::Sprite logo(Tex.GetSFTexture(TEXLOGO));
	logo.setScale(0.35f, 0.35f);
	logo.setPosition((Winsys.resolution.width - logo.getTextureRect().width*0.35f) / 2, 40);
	Winsys.draw(logo);
	DrawGUIFrame();

	FT.SetColor(colDYell);
	FT.AutoSizeN(5);
	FT.DrawString(CENTER, AutoYPosN(60), Trans.Text(29) + " '" + g_game.course->name + '\'');
	FT.SetColor(colWhite);
	FT.DrawString(CENTER, AutoYPosN(70), Trans.Text(30));
	Winsys.SwapBuffers();

	Course.LoadCourse(g_game.course);
	g_game.location_id = Course.GetEnv();
	Env.LoadEnvironment(g_game.location_id, g_game.light_id);
	State::manager.RequestEnterState(Intro);
}
