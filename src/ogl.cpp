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

#include "ogl.h"
#include "spx.h"
#include "winsys.h"
#include <GL/glu.h>
#include <stack>

static const struct {
	const char* name;
	GLenum value;
	GLenum type;
} gl_values[] = {
	{ "maximum lights", GL_MAX_LIGHTS, GL_INT },
	{ "modelview stack depth", GL_MAX_MODELVIEW_STACK_DEPTH, GL_INT },
	{ "projection stack depth", GL_MAX_PROJECTION_STACK_DEPTH, GL_INT },
	{ "max texture size", GL_MAX_TEXTURE_SIZE, GL_INT },
	{ "double buffering", GL_DOUBLEBUFFER, GL_UNSIGNED_BYTE },
	{ "red bits", GL_RED_BITS, GL_INT },
	{ "green bits", GL_GREEN_BITS, GL_INT },
	{ "blue bits", GL_BLUE_BITS, GL_INT },
	{ "alpha bits", GL_ALPHA_BITS, GL_INT },
	{ "depth bits", GL_DEPTH_BITS, GL_INT },
	{ "stencil bits", GL_STENCIL_BITS, GL_INT }
};

void check_gl_error() {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		const char* errstr = (const char*)gluErrorString(error);
		Message ("OpenGL Error: ", errstr ? errstr : "");
	}
}

PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p = NULL;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p = NULL;

typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))();
void InitOpenglExtensions () {
	get_gl_proc_fptr_t get_gl_proc;

	get_gl_proc = (get_gl_proc_fptr_t) SDL_GL_GetProcAddress;

	if (get_gl_proc) {
		glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC)
		                    (*get_gl_proc)((GLubyte*) "glLockArraysEXT");
		glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC)
		                      (*get_gl_proc)((GLubyte*) "glUnlockArraysEXT");

		if (glLockArraysEXT_p != NULL && glUnlockArraysEXT_p != NULL) {

		} else {
			Message ("GL_EXT_compiled_vertex_array extension NOT supported");
			glLockArraysEXT_p = NULL;
			glUnlockArraysEXT_p = NULL;
		}
	} else {
		Message ("No function available for obtaining GL proc addresses");
	}

}

void PrintGLInfo () {
	Message ("Gl vendor: ", (char*)glGetString (GL_VENDOR));
	Message ("Gl renderer: ", (char*)glGetString (GL_RENDERER));
	Message ("Gl version: ", (char*)glGetString (GL_VERSION));
	string extensions = (char*)glGetString (GL_EXTENSIONS);
	Message ("");
	Message ("Gl extensions:");
	Message ("");

	size_t oldpos = 0;
	size_t pos;
	while ((pos = extensions.find(' ', oldpos)) != string::npos) {
		string s = extensions.substr(oldpos, pos-oldpos);
		Message(s);
		oldpos = pos+1;
	}
	Message(extensions.substr(oldpos));

	Message ("");
	for (int i=0; i<(int)(sizeof(gl_values)/sizeof(gl_values[0])); i++) {
		switch (gl_values[i].type) {
			case GL_INT: {
				GLint int_val;
				glGetIntegerv (gl_values[i].value, &int_val);
				string ss = Int_StrN (int_val);
				Message (gl_values[i].name, ss);
				break;
			}
			case GL_FLOAT: {
				GLfloat float_val;
				glGetFloatv (gl_values[i].value, &float_val);
				string ss = Float_StrN (float_val, 2);
				Message (gl_values[i].name, ss);
				break;
			}
			case GL_UNSIGNED_BYTE: {
				GLboolean boolean_val;
				glGetBooleanv (gl_values[i].value, &boolean_val);
				string ss = Int_StrN (boolean_val);
				Message (gl_values[i].name, ss);
				break;
			}
			default:
				Message ("");
		}
	}
}

void set_material (const TColor& diffuse_colour, const TColor& specular_colour, float specular_exp) {
	GLfloat mat_amb_diff[4] = {
		static_cast<GLfloat>(diffuse_colour.r),
		static_cast<GLfloat>(diffuse_colour.g),
		static_cast<GLfloat>(diffuse_colour.b),
		static_cast<GLfloat>(diffuse_colour.a)
	};
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);

	GLfloat mat_specular[4] = {
		static_cast<GLfloat>(specular_colour.r),
		static_cast<GLfloat>(specular_colour.g),
		static_cast<GLfloat>(specular_colour.b),
		static_cast<GLfloat>(specular_colour.a)
	};
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, specular_exp);

	glColor(diffuse_colour);
}
void ClearRenderContext () {
	glDepthMask (GL_TRUE);
	glClearColor (colBackgr.r, colBackgr.g, colBackgr.b, colBackgr.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ClearRenderContext (const TColor& col) {
	glDepthMask (GL_TRUE);
	glClearColor (col.r, col.g, col.b, col.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SetupGuiDisplay () {
	static const float offset = 0.f;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, Winsys.resolution.width, 0, Winsys.resolution.height, -1.0, 1.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (offset, offset, -1.0);
	glColor4f (1.0, 1.0, 1.0, 1.0);
}

void Reshape (int w, int h) {
	glViewport (0, 0, (GLint) w, (GLint) h );
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	double far_clip_dist = param.forward_clip_distance + FAR_CLIP_FUDGE_AMOUNT;
	gluPerspective (param.fov, (double)w/h, NEAR_CLIP_DIST, far_clip_dist );
	glMatrixMode (GL_MODELVIEW);
}
// ====================================================================
//					GL options
// ====================================================================

TRenderMode currentMode = RM_UNINITIALIZED;
void set_gl_options (TRenderMode mode) {
	currentMode = mode;
	switch (mode) {
		case GUI:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			glDisable (GL_FOG);
			break;

		case GAUGE_BARS:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

			glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			break;

		case TEXFONT:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case COURSE:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
			glEnable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LEQUAL);

			glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			break;

		case TREES:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glEnable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

			glAlphaFunc (GL_GEQUAL, 0.5);
			break;

		case PARTICLES:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glEnable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

			glAlphaFunc (GL_GEQUAL, 0.5);
			break;

		case SKY:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_FALSE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case FOG_PLANE:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case TUX:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glEnable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case TUX_SHADOW:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_COLOR_MATERIAL);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
#ifdef USE_STENCIL_BUFFER
			glDisable (GL_CULL_FACE);
			glEnable (GL_STENCIL_TEST);
			glDepthMask (GL_FALSE);

			glStencilFunc (GL_EQUAL, 0, ~0);
			glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
#else
			glEnable (GL_CULL_FACE);
			glDisable (GL_STENCIL_TEST);
			glDepthMask (GL_TRUE);
#endif
			break;

		case TRACK_MARKS:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_COLOR_MATERIAL);
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
			glDepthMask (GL_FALSE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LEQUAL);
			break;

		default:
			Message ("not a valid render mode");
	}
}
/* defined but not used
    case TEXT:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

	case SPLASH_SCREEN:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case PARTICLE_SHADOWS:
        glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case OVERLAYS:
	    glEnable (GL_TEXTURE_2D);
    	glDisable (GL_DEPTH_TEST);
	    glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glEnable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
    	glAlphaFunc (GL_GEQUAL, 0.5);
    break;
*/

static stack<TRenderMode> modestack;
void PushRenderMode(TRenderMode mode) {
	if (currentMode != mode)
		set_gl_options(mode);
	modestack.push(mode);
}

void PopRenderMode() {
	TRenderMode mode = modestack.top();
	modestack.pop();
	if (!modestack.empty() && modestack.top() != mode)
		set_gl_options(modestack.top());
}


void glColor(const TColor& col) {
	glColor4d(col.r, col.g, col.b, col.a);
}

void glColor(const TColor& col, double alpha) {
	glColor4d(col.r, col.g, col.b, alpha);
}

void glTranslate(const TVector3& vec) {
	glTranslated(vec.x, vec.y, vec.z);
}

void glNormal3(const TVector3& vec) {
	glNormal3d(vec.x, vec.y, vec.z);
}

void glVertex3(const TVector3& vec) {
	glVertex3d(vec.x, vec.y, vec.z);
}

void glTexCoord2(const TVector2& vec) {
	glTexCoord2d(vec.x, vec.y);
}
