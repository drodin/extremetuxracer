#ifndef GL_H
#define GL_H

#include "glu.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#define GL_QUADS 0x10000
#define GL_POLYGON 0x10001

#define GL_DOUBLEBUFFER 0x0C32

#define GL_QUAD_STRIP GL_TRIANGLE_STRIP
#define GL_CLAMP GL_CLAMP_TO_EDGE

void qglBegin(GLenum mode);void glDrawBuffer(GLenum mode);
void qglEnd(void);
void qglDrawArrays(GLenum mode, GLint first, GLsizei count);
void qglColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void qglColor4fv(GLfloat *v);
void qglTexCoord2f(GLfloat s, GLfloat t);
void qglTexCoord2fv(GLfloat *v);
void qglTexGenfv( GLenum coord, GLenum pname, const GLfloat * param );
void qglNormal3f(GLfloat x, GLfloat y, GLfloat z);
void qglNormal3fv(GLfloat *v);
void qglVertex3f(GLfloat x, GLfloat y, GLfloat z);
void qglVertex3fv(GLfloat *v);
void qglBindTexture(GLenum target, GLuint texture);
void qglTexGeni(GLenum coord , GLenum pname , GLint param );

void qglEnable(GLenum target);
void qglDisable(GLenum target);
void qglCallList(GLuint list);

void qglVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void qglColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void qglTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void qglNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);

void qglDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void qglEnableClientState (GLenum array);
void qglDisableClientState (GLenum array);
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void glMultMatrixd(const double * m);
void glLoadMatrixd(const double * m);
void rotate2D(float angle, float pointOfRotationX, float pointOfRotationY);

void qglReadBuffer(GLenum mode);

void qglTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLint qgluBuild2DMipmaps(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * data);
void qglGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params );
void qglDeleteTextures(GLsizei n, const GLuint *textures);

#define GL_S 0
#define GL_T 1

#define GL_OBJECT_PLANE 0
#define GL_OBJECT_LINEAR 1
#define GL_SPHERE_MAP 2

#define GL_TEXTURE_GEN_MODE 0

#define GL_TEXTURE_GEN_S 0xdeadbeef
#define GL_TEXTURE_GEN_T 0xdeadbee2

#define GL_TEXTURE_1D				0x0DE0
#define GL_PROXY_TEXTURE_1D			0x8063
#define GL_TEXTURE_WIDTH			0x1000
#define GL_TEXTURE_HEIGHT			0x1001

#define glBegin qglBegin
#define glEnd qglEnd
#define glDrawArrays qglDrawArrays
#define glColor4f qglColor4f
#define glColor4fv qglColor4fv
#define glTexCoord2f qglTexCoord2f
#define glTexCoord2fv qglTexCoord2fv
#define glVertex3f qglVertex3f
#define glVertex3fv qglVertex3fv
#define glNormal3f qglNormal3f
#define glNormal3fv qglNormal3fv

#define glCallList qglCallList
#define glTexGenfv qglTexGenfv
#define glEnable qglEnable
#define glDisable qglDisable

#define glTexGeni qglTexGeni
#define glVertexPointer qglVertexPointer
#define glTexCoordPointer qglTexCoordPointer
#define glColorPointer qglColorPointer

#define glDrawElements qglDrawElements
#define glEnableClientState qglEnableClientState
#define glDisableClientState qglDisableClientState
#define glNormalPointer qglNormalPointer

#define glOrtho glOrthof
#define glReadBuffer qglReadBuffer

#define glTexImage2D qglTexImage2D
#define gluBuild2DMipmaps qgluBuild2DMipmaps
#define glGetTexLevelParameteriv qglGetTexLevelParameteriv
#define glDeleteTextures qglDeleteTextures

#define glVertex2f(x, y)	qglVertex3f(x, y, 0.0)
#define glTexCoord2d(x, y)	glTexCoord2f(x, y)

static inline void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    glColor4f(r, g, b, 1.0);
}
	
static inline void glColor4dv(const double *v)
{
	glColor4f((float)v[0], (float)v[1], (float)v[2], (float)v[3]);
}


//NEW
static inline void glColor4ubv(const GLubyte *v)
{
    glColor4ub(v[0], v[1], v[2], v[3]);
}

static inline void glMaterialiv(GLenum face, GLenum pname, const GLint *v)
{
    GLfloat params[4] = {
            static_cast<GLfloat>(v[0])/INT_MAX,
            static_cast<GLfloat>(v[1])/INT_MAX,
            static_cast<GLfloat>(v[2])/INT_MAX,
            static_cast<GLfloat>(v[3])/INT_MAX
    };

    glMaterialfv(face, pname, params);
}

static inline void glNormal3i(GLint nx, GLint ny, GLint nz)
{
    qglNormal3f((float)nx, (float)ny, (float)nz);
}

static inline void glVertex3d(double x, double y, double z)
{
    qglVertex3f((float)x, (float)y, (float)z);
}

static inline void glNormal3d(double x, double y, double z)
{
    qglNormal3f((float)x, (float)y, (float)z);
}

static inline void glTranslated(double x, double y, double z)
{
    glTranslatef((float)x, (float)y, (float)z);
}
//end NEW

static inline void glColor3dv(const double *v)	
{
	glColor3f((float)v[0], (float)v[1], (float)v[2]);
}

static inline void glVertex2dv(const double *v)	
{
	glVertex2f((float)v[0], (float)v[1]);
}

static inline void glTexCoord2dv(const double *v)	
{
    GLfloat coord[2] = { (float)v[0], (float)v[1] };
	glTexCoord2fv(coord);
}

//FIXME : Not sure
static inline void glFogi( GLenum pname, GLint param ) 
{
	glFogf(pname,(GLfloat) param);
}
	
static inline void glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	glRectf(x1, y1, x2, y2);
}

#endif
