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

#include "hud.h"
#include "ogl.h"
#include "textures.h"
#include "spx.h"
#include "particles.h"
#include "font.h"
#include "course.h"
#include "physics.h"
#include "winsys.h"


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

static const GLfloat energy_background_color[] = { 0.2, 0.2, 0.2, 0.0 };
static const GLfloat energy_foreground_color[] = { 0.54, 0.59, 1.00, 0.5 };
static const GLfloat speedbar_background_color[] = { 0.2, 0.2, 0.2, 0.0 };
static const GLfloat hud_white[] = { 1.0, 1.0, 1.0, 1.0 };

static const TColor text_colour(0, 0, 0, 1);

static void draw_time() {
    int min, sec, hundr;
    GetTimeComponents (g_game.time, &min, &sec, &hundr);
	string timestr = Int_StrN (min, 2);
	string secstr = Int_StrN (sec, 2);
	string hundrstr = Int_StrN (hundr, 2);

	timestr += ":";
	timestr += secstr;

	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (timestr.c_str(), 20, 10, 1, colWhite);
		Tex.DrawNumStr (hundrstr.c_str(), 136, 10, 0.7, colWhite);
	} else {

	/*
		glEnable (GL_LINE_SMOOTH);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glLineWidth (1.5);
	*/
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		Tex.Draw (T_TIME, 10, 12, 1);
		FT.SetColor (colDYell);
		FT.SetSize (32);
		FT.DrawString (160, 6, hundrstr);
		FT.SetSize (42);
		FT.DrawString (70, 10, timestr);
	}
}

static void draw_herring_count (int herring_count) {
	string hcountstr = Int_StrN (herring_count, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (hcountstr.c_str(), Winsys.resolution.width - 90, 10, 1, colWhite);
		Tex.Draw (HERRING_ICON, Winsys.resolution.width-160, -20, 1);
	} else {
		FT.SetColor (colDYell);
		FT.DrawString ( Winsys.resolution.width - 90, 10, hcountstr);
		Tex.Draw (T_YELLHERRING, Winsys.resolution.width-160, 12, 1);
	}
}

TVector2 calc_new_fan_pt (double angle) {
    TVector2 pt;
    pt.x = ENERGY_GAUGE_CENTER_X + cos (ANGLES_TO_RADIANS (angle)) * SPEEDBAR_OUTER_RADIUS;
    pt.y = ENERGY_GAUGE_CENTER_Y + sin (ANGLES_TO_RADIANS (angle)) * SPEEDBAR_OUTER_RADIUS;
    return pt;
}

void start_tri_fan() {
    glBegin (GL_TRIANGLE_FAN);
    glVertex2f (ENERGY_GAUGE_CENTER_X,
		ENERGY_GAUGE_CENTER_Y);
    TVector2 pt = calc_new_fan_pt (SPEEDBAR_BASE_ANGLE);
    glVertex2f (pt.x, pt.y);
}

void draw_partial_tri_fan (double fraction) {
    bool trifan = false;

	double angle = SPEEDBAR_BASE_ANGLE +
		(SPEEDBAR_MAX_ANGLE - SPEEDBAR_BASE_ANGLE) * fraction;

    int divs = (int)((SPEEDBAR_BASE_ANGLE - angle) * CIRCLE_DIVISIONS / 360.0);
    double cur_angle = SPEEDBAR_BASE_ANGLE;
    double angle_incr = 360.0 / CIRCLE_DIVISIONS;

    for (int i=0; i<divs; i++) {
		if (!trifan) {
		    start_tri_fan();
	    	trifan = true;
		}
		cur_angle -= angle_incr;
		TVector2 pt = calc_new_fan_pt (cur_angle);
		glVertex2f (pt.x, pt.y);
    }

    if (cur_angle > angle + EPS) {
		cur_angle = angle;
		if (!trifan) {
		    start_tri_fan();
	    	trifan = true;
		}
		TVector2 pt = calc_new_fan_pt (cur_angle);
		glVertex2f (pt.x, pt.y);
    }

    if (trifan) {
		glEnd();
		trifan = false;
    }
}

void draw_gauge (double speed, double energy) {
	static const GLfloat xplane[4] = {1.0 / GAUGE_IMG_SIZE, 0.0, 0.0, 0.0 };
	static const GLfloat yplane[4] = {0.0, 1.0 / GAUGE_IMG_SIZE, 0.0, 0.0 };

	ScopedRenderMode rm(GAUGE_BARS);

	if (Tex.GetTexture (GAUGE_ENERGY) == NULL) return;
	if (Tex.GetTexture (GAUGE_SPEED) == NULL) return;
	if (Tex.GetTexture (GAUGE_OUTLINE) == NULL) return;

	Tex.BindTex (GAUGE_ENERGY);
    glTexGenfv (GL_S, GL_OBJECT_PLANE, xplane);
    glTexGenfv (GL_T, GL_OBJECT_PLANE, yplane);

    glPushMatrix();
	glTranslatef (Winsys.resolution.width - GAUGE_WIDTH, 0, 0);
	Tex.BindTex (GAUGE_ENERGY);
	double y = ENERGY_GAUGE_BOTTOM + energy * ENERGY_GAUGE_HEIGHT;

	glColor4fv (energy_background_color);
	glBegin (GL_QUADS);
	    glVertex2f (0.0, y);
	    glVertex2f (GAUGE_IMG_SIZE, y);
	    glVertex2f (GAUGE_IMG_SIZE, GAUGE_IMG_SIZE);
	    glVertex2f (0.0, GAUGE_IMG_SIZE);
	glEnd ();

	glColor4fv (energy_foreground_color);
	glBegin (GL_QUADS);
	    glVertex2f (0.0, 0.0);
	    glVertex2f (GAUGE_IMG_SIZE, 0.0);
	    glVertex2f (GAUGE_IMG_SIZE, y);
	    glVertex2f (0.0, y);
	glEnd ();

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

	glColor4fv (speedbar_background_color);
	Tex.BindTex (GAUGE_SPEED);
	draw_partial_tri_fan (1.0);
	glColor4fv (hud_white);
	draw_partial_tri_fan (min (1.0, speedbar_frac));

	glColor4fv (hud_white);
	Tex.BindTex (GAUGE_OUTLINE);
	glBegin (GL_QUADS);
	    glVertex2f (0.0, 0.0);
	    glVertex2f (GAUGE_IMG_SIZE, 0.0);
	    glVertex2f (GAUGE_IMG_SIZE, GAUGE_IMG_SIZE);
	    glVertex2f (0.0, GAUGE_IMG_SIZE);
	glEnd();
    glPopMatrix();
}

void DrawSpeed (double speed) {
	string speedstr = Int_StrN ((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (speedstr.c_str(),
			Winsys.resolution.width - 85, Winsys.resolution.height-74, 1, colWhite);
	} else {
		FT.SetColor (colDDYell);
		FT.DrawString (Winsys.resolution.width-82, Winsys.resolution.height-80, speedstr);
	}
}

void DrawWind (double dir, double speed) {
	Tex.Draw (SPEEDMETER, 10, Winsys.resolution.height - 150, 1.0);
	glPushMatrix ();
    glDisable (GL_TEXTURE_2D);

	glColor4f (1, 0, 0, 0.5);
	glTranslatef (82, 77, 0);
	glRotatef (dir, 0, 0, 1);
	glBegin (GL_QUADS);
	    glVertex2f (-3, 0.0);
	    glVertex2f (3, 0.0);
	    glVertex2f (3, -speed);
	    glVertex2f (-3, -speed);
	glEnd();
	glPopMatrix ();
	Tex.Draw (SPEED_KNOB, 74, Winsys.resolution.height - 84, 1.0);
}

void DrawWind2 (float dir, float speed, const CControl *ctrl) {
	if (g_game.wind_id < 1) return;

	Tex.Draw (SPEEDMETER, 0, Winsys.resolution.height-140, 1.0);
    glDisable (GL_TEXTURE_2D);


	float alpha, red, blue, len;
	len = 45;
	if (speed <= 50) {
		alpha = speed / 50;
		red = 0;
	} else {
		alpha = 1.0;
		red = (speed - 50) / 50;
	}
	blue = 1.0 - red;

	glPushMatrix ();
	glColor4f (red, 0, blue, alpha);
	glTranslatef (72, 66, 0);
	glRotatef (dir, 0, 0, 1);
	glBegin (GL_QUADS);
	    glVertex2f (-5, 0.0);
	    glVertex2f (5, 0.0);
	    glVertex2f (5, -len);
	    glVertex2f (-5, -len);
	glEnd();
	glPopMatrix ();

	// direction indicator
	TVector3 movdir = ctrl->cvel;
	NormVector (movdir);
	float dir_angle = atan (movdir.x / movdir.z) * 57.3;

	glPushMatrix ();
	glColor4f (0, 0.5, 0, 1.0);
	glTranslatef (72, 66, 0);
	glRotatef (dir_angle + 180, 0, 0, 1);
	glBegin (GL_QUADS);
	    glVertex2f (-2, 0.0);
	    glVertex2f (2, 0.0);
	    glVertex2f (2, -50);
	    glVertex2f (-2, -50);
	glEnd();
	glPopMatrix ();

    glEnable (GL_TEXTURE_2D);

	Tex.Draw (SPEED_KNOB, 64, Winsys.resolution.height - 74, 1.0);
	string windstr = Int_StrN ((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (windstr.c_str(), 130, Winsys.resolution.height - 55, 1, colWhite);
	} else {
		FT.SetColor (colBlue);
		FT.DrawString (130, Winsys.resolution.height - 55, windstr);
	}
}

const  int   maxFrames = 50;
static int   numFrames = 0;
static float averagefps = 0;
static float sumTime = 0;

void DrawFps () {
	if (numFrames >= maxFrames) {
		averagefps = 1 / sumTime * maxFrames;
		numFrames = 0;
		sumTime = 0;
	} else {
		sumTime += g_game.time_step;
		numFrames++;
	}
	if (averagefps < 1) return;

	if (param.display_fps) {
		string fpsstr = Float_StrN (averagefps, 0);
		if (param.use_papercut_font < 2) {
			Tex.DrawNumStr (fpsstr.c_str(), (Winsys.resolution.width - 60) / 2, 10, 1, colWhite);
		} else {
			if (averagefps >= 35)
				FT.SetColor (colWhite);
			else
				FT.SetColor (colRed);
			FT.DrawString ((Winsys.resolution.width - 60) / 2, 10, fpsstr);
		}
	}
}

void DrawPercentBar (float fact, float x, float y) {
	Tex.BindTex (T_ENERGY_MASK);
	glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
		glTexCoord2f (0, 0); glVertex2f (x, y);
	    glTexCoord2f (1, 0); glVertex2f (x+32, y);
	    glTexCoord2f (1, fact); glVertex2f (x+32, y+fact*128);
	    glTexCoord2f (0, fact); glVertex2f (x, y+fact*128);
	glEnd();
}

void DrawCoursePosition (const CControl *ctrl) {
	double fact = ctrl->cpos.z / Course.GetPlayDimensions().y;
	if (fact > 1.0) fact = 1.0;
    glEnable (GL_TEXTURE_2D);
	DrawPercentBar (-fact, Winsys.resolution.width - 48, 280-128);
	Tex.Draw (T_MASK_OUTLINE, Winsys.resolution.width - 48, Winsys.resolution.height - 280, 1.0);
}

// -------------------------------------------------------
void DrawHud (const CControl *ctrl) {
	if (!param.show_hud)
		return;

	TVector3 vel = ctrl->cvel;
	double speed = NormVector (vel);
    SetupGuiDisplay ();

    draw_gauge (speed * 3.6, ctrl->jump_amt);
	ScopedRenderMode rm(TEXFONT);
	glColor4f (1, 1, 1, 1);
    draw_time();
    draw_herring_count (g_game.herring);
    DrawSpeed (speed * 3.6);
	DrawFps ();
	DrawCoursePosition (ctrl);
	if (g_game.wind_id > 0) DrawWind2 (Wind.Angle (), Wind.Speed (), ctrl);
}
