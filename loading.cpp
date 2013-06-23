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
	Winsys.ShowCursor (false);
    Music.Play ("loading", -1);
}

void CLoading::Loop(double time_step) {
	TCourse *CourseList = &Course.CourseList[0];
	int ww = Winsys.resolution.width;
    int hh = Winsys.resolution.height;
	string msg = Trans.Text(29);
	msg += " " + CourseList[g_game.course_id].name;

	check_gl_error ();
	ScopedRenderMode rm(GUI);
    ClearRenderContext ();
    SetupGuiDisplay ();

    if (param.ui_snow) {
		update_ui_snow (time_step);
		draw_ui_snow ();
    }

	Tex.Draw (TEXLOGO, CENTER, 40, 0.7);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	FT.SetColor (colDYell);
	FT.AutoSizeN (5);
	FT.DrawString (CENTER, AutoYPosN (60), msg);
	FT.SetColor (colWhite);
	FT.DrawString (CENTER, AutoYPosN (70), Trans.Text (30));
	Winsys.SwapBuffers ();

	Course.LoadCourse (g_game.course_id);
	g_game.location_id = Course.GetEnv ();
	Env.LoadEnvironment (g_game.location_id, g_game.light_id);
    State::manager.RequestEnterState (Intro);
}

void CLoading::Exit() {
	Music.Halt ();
}
