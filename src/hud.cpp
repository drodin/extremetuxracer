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

#include "hud.h"
#include "ogl.h"
#include "textures.h"
#include "spx.h"
#include "particles.h"
#include "font.h"
#include "course.h"
#include "physics.h"
#include "winsys.h"
#include "game_ctrl.h"
#include <algorithm>


#define GAUGE_IMG_SIZE 128
#define ENERGY_GAUGE_BOTTOM 3.0
#define ENERGY_GAUGE_HEIGHT 103.0
#define ENERGY_GAUGE_CENTER_X 71.0
#define ENERGY_GAUGE_CENTER_Y 55.0
#define GAUGE_WIDTH 128.0
#define SPEEDBAR_OUTER_RADIUS  (ENERGY_GAUGE_CENTER_X)
#define SPEEDBAR_BASE_ANGLE 225
#define SPEEDBAR_MAX_ANGLE 45
#define SPEEDBAR_GREEN_MAX_SPEED  (MAX_PADDLING_SPEED * 3.6)
#define SPEEDBAR_YELLOW_MAX_SPEED 100
#define SPEEDBAR_RED_MAX_SPEED 160
#define SPEEDBAR_GREEN_FRACTION 0.5
#define SPEEDBAR_YELLOW_FRACTION 0.25
#define SPEEDBAR_RED_FRACTION 0.25
#define CIRCLE_DIVISIONS 10

static const GLubyte energy_background_color[]   = { 51,  51,  51, 0 };
static const GLubyte energy_foreground_color[]   = { 138, 150, 255, 128 };
static const GLubyte speedbar_background_color[] = { 51,  51,  51, 0 };
static const GLubyte hud_white[]                 = { 255, 255, 255, 255 };

static void draw_time(double time, sf::Color color) {
	Tex.Draw(T_TIME, 10, 10, 1);

	int min, sec, hundr;
	GetTimeComponents(time, &min, &sec, &hundr);
	std::string timestr = Int_StrN(min, 2);
	std::string secstr = Int_StrN(sec, 2);
	std::string hundrstr = Int_StrN(hundr, 2);

	timestr += ':';
	timestr += secstr;

	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr(timestr, 50, 12, 1, color);
		Tex.DrawNumStr(hundrstr, 170, 12, 0.7f, color);
	} else {
		Winsys.beginSFML();
		FT.SetColor(color);
		FT.SetSize(30);
		FT.DrawString(138, 3, hundrstr);
		FT.SetSize(42);
		FT.DrawString(53, 3, timestr);
		Winsys.endSFML();
	}
}

static void draw_herring_count(int herring_count, sf::Color color) {
	Tex.Draw(HERRING_ICON, Winsys.resolution.width - 59, 12, 1);

	std::string hcountstr = Int_StrN(herring_count, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr(hcountstr, Winsys.resolution.width - 130, 12, 1, color);
	} else {
		Winsys.beginSFML();
		FT.SetColor(color);
		FT.DrawString(Winsys.resolution.width - 125, 3, hcountstr);
		Winsys.endSFML();
	}
}

TVector2d calc_new_fan_pt(double angle) {
	return TVector2d(
	           ENERGY_GAUGE_CENTER_X + std::cos(ANGLES_TO_RADIANS(angle)) * SPEEDBAR_OUTER_RADIUS,
	           ENERGY_GAUGE_CENTER_Y + std::sin(ANGLES_TO_RADIANS(angle)) * SPEEDBAR_OUTER_RADIUS);
}

void draw_partial_tri_fan(double fraction) {
	double angle = SPEEDBAR_BASE_ANGLE +
	               (SPEEDBAR_MAX_ANGLE - SPEEDBAR_BASE_ANGLE) * fraction;

	int divs = (int)((SPEEDBAR_BASE_ANGLE - angle) * CIRCLE_DIVISIONS / 360.0) + 1;
	double cur_angle = SPEEDBAR_BASE_ANGLE;
	double angle_incr = 360.0 / CIRCLE_DIVISIONS;

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(ENERGY_GAUGE_CENTER_X,
	           ENERGY_GAUGE_CENTER_Y);

	for (int i=0; i<divs; i++) {
		TVector2d pt = calc_new_fan_pt(cur_angle);
		glVertex2f(pt.x, pt.y);
		cur_angle -= angle_incr;
	}

	if (cur_angle+angle_incr > angle + EPS) {
		cur_angle = angle;
		TVector2d pt = calc_new_fan_pt(cur_angle);
		glVertex2f(pt.x, pt.y);
	}

	glEnd();
}

void draw_gauge(double speed, double energy) {
	static const GLfloat xplane[4] = {1.f / GAUGE_IMG_SIZE, 0.f, 0.f, 0.f };
	static const GLfloat yplane[4] = {0.f, 1.f / GAUGE_IMG_SIZE, 0.f, 0.f };

	ScopedRenderMode rm(GAUGE_BARS);

	if (Tex.GetTexture(GAUGE_ENERGY) == nullptr) return;
	if (Tex.GetTexture(GAUGE_SPEED) == nullptr) return;
	if (Tex.GetTexture(GAUGE_OUTLINE) == nullptr) return;

	Tex.BindTex(GAUGE_ENERGY);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, xplane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, yplane);

	glPushMatrix();
	glTranslatef(Winsys.resolution.width - GAUGE_WIDTH, 0, 0);
	Tex.BindTex(GAUGE_ENERGY);
	float y = ENERGY_GAUGE_BOTTOM + energy * ENERGY_GAUGE_HEIGHT;

	const GLfloat vtx1 [] = {
		0.f, y,
		GAUGE_IMG_SIZE, y,
		GAUGE_IMG_SIZE, GAUGE_IMG_SIZE,
		0.f, GAUGE_IMG_SIZE
	};
	const GLfloat vtx2 [] = {
		0.f, 0.f,
		GAUGE_IMG_SIZE, 0.f,
		GAUGE_IMG_SIZE, y,
		0.f, y
	};
	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4ubv(energy_background_color);
	glVertexPointer(2, GL_FLOAT, 0, vtx1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glColor4ubv(energy_foreground_color);
	glVertexPointer(2, GL_FLOAT, 0, vtx2);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	double speedbar_frac = 0.0;

	if (speed > SPEEDBAR_GREEN_MAX_SPEED) {
		speedbar_frac = SPEEDBAR_GREEN_FRACTION;

		if (speed > SPEEDBAR_YELLOW_MAX_SPEED) {
			speedbar_frac += SPEEDBAR_YELLOW_FRACTION;
			if (speed > SPEEDBAR_RED_MAX_SPEED) {
				speedbar_frac += SPEEDBAR_RED_FRACTION;
			} else {
				speedbar_frac += (speed - SPEEDBAR_YELLOW_MAX_SPEED) /
				                 (SPEEDBAR_RED_MAX_SPEED - SPEEDBAR_YELLOW_MAX_SPEED) * SPEEDBAR_RED_FRACTION;
			}
		} else {
			speedbar_frac += (speed - SPEEDBAR_GREEN_MAX_SPEED) /
			                 (SPEEDBAR_YELLOW_MAX_SPEED - SPEEDBAR_GREEN_MAX_SPEED) * SPEEDBAR_YELLOW_FRACTION;
		}
	} else {
		speedbar_frac +=  speed/SPEEDBAR_GREEN_MAX_SPEED * SPEEDBAR_GREEN_FRACTION;
	}

	glColor4ubv(speedbar_background_color);
	Tex.BindTex(GAUGE_SPEED);
	draw_partial_tri_fan(1.0);
	glColor4ubv(hud_white);
	draw_partial_tri_fan(std::min(1.0, speedbar_frac));

	glColor4ubv(hud_white);
	Tex.BindTex(GAUGE_OUTLINE);
	static const GLshort vtx3 [] = {
		0, 0,
		GAUGE_IMG_SIZE, 0,
		GAUGE_IMG_SIZE, GAUGE_IMG_SIZE,
		0, GAUGE_IMG_SIZE
	};
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_SHORT, 0, vtx3);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
}

void DrawSpeed(double speed) {
	std::string speedstr = Int_StrN((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr(speedstr,
		               Winsys.resolution.width - 87, Winsys.resolution.height-73, 1, colWhite);
	} else {
		Winsys.beginSFML();
		FT.SetColor(colDDYell);
		FT.DrawString(Winsys.resolution.width - 82, Winsys.resolution.height - 80, speedstr);
		Winsys.endSFML();
	}
}

void DrawWind(float dir, float speed, const CControl *ctrl) {
	if (g_game.wind_id < 1) return;

	static const int texHeight = Tex.GetSFTexture(SPEEDMETER).getSize().y;
	static const int texWidth = Tex.GetSFTexture(SPEEDMETER).getSize().x;

	Tex.Draw(SPEEDMETER, 5, Winsys.resolution.height-5-texHeight, 1.0);
	glDisable(GL_TEXTURE_2D);


	float alpha, red, blue;
	if (speed <= 50) {
		alpha = speed / 50;
		red = 0;
	} else {
		alpha = 1.f;
		red = (speed - 50) / 50;
	}
	blue = 1.f - red;

	glPushMatrix();
	glColor4f(red, 0, blue, alpha);
	glTranslatef(5 + texWidth / 2, 5 + texHeight / 2, 0);
	glRotatef(dir, 0, 0, 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	static const int len = 45;
	static const GLshort vtx1 [] = {
		-5, 0,
		    5, 0,
		    5, -len,
		    - 5, -len
	    };
	glVertexPointer(2, GL_SHORT, 0, vtx1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// direction indicator
	float dir_angle = RADIANS_TO_ANGLES(std::atan2(ctrl->cvel.x, ctrl->cvel.z));

	glColor4f(0, 0.5, 0, 1.0);
	glRotatef(dir_angle - dir, 0, 0, 1);
	static const GLshort vtx2 [] = {
		-2, 0,
		    2, 0,
		    2, -50,
		    -2, -50
	    };
	glVertexPointer(2, GL_SHORT, 0, vtx2);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	Tex.Draw(SPEED_KNOB, 5 + texWidth / 2 - 8, Winsys.resolution.height - 5 - texWidth / 2 - 8, 1.0);
	std::string windstr = Int_StrN((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr(windstr, 120, Winsys.resolution.height - 45, 1, colWhite);
	} else {
		Winsys.beginSFML();
		FT.SetColor(colDDYell);
		FT.DrawString(120, Winsys.resolution.height - 50, windstr);
		Winsys.endSFML();
	}
}

void DrawFps() {
	const  int   maxFrames = 50;
	static int   numFrames = 0;
	static float averagefps = 0;
	static float sumTime = 0;

	if (!param.display_fps)
		return;

	if (numFrames >= maxFrames) {
		averagefps = 1 / sumTime * maxFrames;
		numFrames = 0;
		sumTime = 0;
	} else {
		sumTime += g_game.time_step;
		numFrames++;
	}
	if (averagefps < 1) return;

	std::string fpsstr = Int_StrN((int)averagefps);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr(fpsstr, (Winsys.resolution.width - 60) / 2, 10, 1, colWhite);
	} else {
		Winsys.beginSFML();
		if (averagefps >= 35)
			FT.SetColor(colWhite);
		else
			FT.SetColor(colRed);
		FT.DrawString(-1, 3, fpsstr);
		Winsys.endSFML();
	}
}

void DrawPercentBar(float fact, float x, float y) {
	Tex.BindTex(T_ENERGY_MASK);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	const GLfloat tex[] = {
		0, 1,
		1, 1,
		1, 1 - fact,
		0, 1 - fact
	};
	const GLfloat vtx[] = {
		x, y,
		x + 32, y,
		x + 32, y + fact * 128,
		x, y + fact * 128
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void DrawCoursePosition(const CControl *ctrl) {
	double fact = ctrl->cpos.z / Course.GetPlayDimensions().y;
	if (fact > 1.0) fact = 1.0;
	glEnable(GL_TEXTURE_2D);
	DrawPercentBar(-fact, Winsys.resolution.width - 48, 280-128);
	Tex.Draw(T_MASK_OUTLINE, Winsys.resolution.width - 48, Winsys.resolution.height - 280, 1.0);
}

// -------------------------------------------------------
void DrawHud(const CControl *ctrl) {
	if (!param.show_hud)
		return;

	double speed = ctrl->cvel.Length();
	Setup2dScene();

	draw_gauge(speed * 3.6, ctrl->jump_amt);
	ScopedRenderMode rm(TEXFONT);

	if (g_game.game_type == CUPRACING) {
		if (g_game.time < g_game.race->time.z)
			draw_time(g_game.race->time.z - g_game.time, colGold);
		else if (g_game.time < g_game.race->time.y)
			draw_time(g_game.race->time.y - g_game.time, colSilver);
		else if (g_game.time < g_game.race->time.x)
			draw_time(g_game.race->time.x - g_game.time, colBronze);
		else
			draw_time(g_game.time, colDRed);

		if (g_game.herring < g_game.race->herrings.x)
			draw_herring_count(g_game.race->herrings.x - g_game.herring, colBronze);
		else if (g_game.herring < g_game.race->herrings.y)
			draw_herring_count(g_game.race->herrings.y - g_game.herring, colSilver);
		else if (g_game.herring < g_game.race->herrings.z)
			draw_herring_count(g_game.race->herrings.z - g_game.herring, colGold);
		else
			draw_herring_count(g_game.herring, colGreen);
	} else {
		draw_time(g_game.time, param.use_papercut_font < 2 ? colWhite : colDYell);
		draw_herring_count(g_game.herring, param.use_papercut_font < 2 ? colWhite : colDYell);
	}

	DrawSpeed(speed * 3.6);
	DrawFps();
	DrawCoursePosition(ctrl);
	DrawWind(Wind.Angle(), Wind.Speed(), ctrl);
}
