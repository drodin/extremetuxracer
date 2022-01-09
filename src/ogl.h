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

#ifndef OGL_H
#define OGL_H

#include "bh.h"

#define FAR_CLIP_FUDGE_AMOUNT 5
#define NEAR_CLIP_DIST 0.1

enum TRenderMode {
	GUI,
	GAUGE_BARS,
	TEXFONT,
	COURSE,
	TREES,
	PARTICLES,
	TUX,
	TUX_SHADOW,
	SKY,
	FOG_PLANE,
	TRACK_MARKS,
	RM_UNINITIALIZED = -1
};


#undef GL_EXT_compiled_vertex_array

extern PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
extern PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;

void check_gl_error();
void InitOpenglExtensions();
void PrintGLInfo();

void set_material_diffuse(const sf::Color& diffuse_colour);
void set_material(const sf::Color& diffuse_colour,
                  const sf::Color& specular_colour,
                  float specular_exp);


void ResetRenderMode();
void PushRenderMode(TRenderMode mode);
void PopRenderMode();

struct ScopedRenderMode {
	explicit ScopedRenderMode(TRenderMode mode) {
		PushRenderMode(mode);
	}
	~ScopedRenderMode() {
		PopRenderMode();
	}
};

void ClearRenderContext();
void ClearRenderContext(const sf::Color& col);
void Setup2dScene();
void Reshape(int w, int h);

void glColor(const sf::Color& col);
void glColor(const sf::Color& col, uint8_t alpha);

void glTranslate(const TVector3d& vec);

void glNormal3(const TVector3d& vec);
void glVertex3(const TVector3d& vec);
void glTexCoord2(const TVector2d& vec);

void glLoadMatrix(const TMatrix<4, 4>& mat);
void glMultMatrix(const TMatrix<4, 4>& mat);


#endif
