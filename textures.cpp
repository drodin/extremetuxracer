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

#include "textures.h"
#include "spx.h"
#include <fstream>

// --------------------------------------------------------------------
//				class CImage
// --------------------------------------------------------------------

CImage::CImage () { 
	status = false; 
}

CImage::~CImage () {
	if (status) free (data);
}

bool CImage::LoadPng (const char *filepath, bool mirroring) {
	SDL_Surface *sdlImage;
	unsigned char *sdlData;
	
	sdlImage = IMG_Load (filepath);
   	if (sdlImage == 0) {
   		Message ("could not load image", filepath);
		return false;
   	}

	nx    = sdlImage->w;
	ny    = sdlImage->h;
	depth = sdlImage->format->BytesPerPixel;
	pitch = sdlImage->pitch;
	data  = (unsigned char *) malloc (pitch * ny * sizeof (unsigned char));

   	if (SDL_MUSTLOCK (sdlImage)) {
   	    if (SDL_LockSurface (sdlImage) < 0) {
	   		Message ("mustlock error");
			return false;
		};
   	}

	sdlData = (unsigned char *) sdlImage->pixels;

	// Um die Texturkoordinaten im gleichen Drehsinn wie die Vertexkoordinaten 
	// eingeben zu können, kann das Bild in y-Richtung gespiegelt werden.
	if (mirroring) {
		for (int y=0; y<ny; y++) {
			for (int x=0; x<pitch; x++) {
				data [y * pitch + x] = sdlData [(ny-1-y) * pitch + x];	
			}
		}
	} else {
		for (int y=0; y<ny; y++) {
			for (int x=0; x<pitch; x++) {
				data [y * pitch + x] = sdlData [y * pitch + x];	
			}
		}
	}

	if (SDL_MUSTLOCK (sdlImage)) SDL_UnlockSurface (sdlImage);
	SDL_FreeSurface (sdlImage);

	status = true;	
	return true;
}

bool CImage::LoadPng (const char *dir, const char *filename, bool mirroring) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadPng (path.c_str(), mirroring);
}

void CImage::WritePPM (const char *filepath) {
	if (!status) return;
	std::ofstream file;
	file.open (filepath);

	file << "P6\n# A Raw PPM file"
		 << "\n# width\n" << nx
		 << "\n# height\n" << ny
		 << "\n# max component value\n255"<< std::endl;

	file.write ((const char*) data, nx * ny * depth);
	file.close ();
}

void CImage::WritePPM (const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	WritePPM (path.c_str());
}

bool CImage::LoadFrameBuffer () {
	int viewport[4];
	glGetIntegerv (GL_VIEWPORT, viewport);
	glReadBuffer (GL_FRONT);
	
	nx = viewport[2];
	ny = viewport[3];
	depth = 3;
	data = new unsigned char [nx * ny * depth];
	
	for (int i=0; i<viewport[3]; i++){
		glReadPixels (viewport[0], viewport[1] + viewport[3] - 1 - i,
			viewport[2], 1, GL_RGB, GL_UNSIGNED_BYTE, data + viewport[2] * i * 3);
	}

	status = true;
	return true;
}

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

CTexture Tex;

CTexture::CTexture () {
	for (int i=0; i<MAX_COMMON_TEX; i++) CommonTex[i] = 0;
	numTextures = 0;
	TextureIndex = "";	
	forientation = OR_TOP;
}

CTexture::~CTexture () {}

int CTexture::LoadTexture (const char *filename) {
    CImage texImage;
	GLuint texid;

	if (texImage.LoadPng (filename, true) == false) return 0;
	glGenTextures (1, &texid);
	glBindTexture (GL_TEXTURE_2D, texid);		
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4); 
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	GLenum format;
	if (texImage.depth == 3) format = GL_RGB; 
		else format = GL_RGBA;

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	glTexImage2D 
		(GL_TEXTURE_2D, 0, texImage.depth, texImage.nx,
		texImage.ny, 0, format, GL_UNSIGNED_BYTE, texImage.data);

    return texid;    
}

int CTexture::LoadTexture (const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadTexture (path.c_str());
}

int CTexture::LoadMipmapTexture (const char *filename, bool repeatable) {
    CImage texImage;
	GLuint texid;

	if (texImage.LoadPng (filename, true) == false) return 0;
	glGenTextures (1, &texid);
	glBindTexture (GL_TEXTURE_2D, texid);		
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
 
   if  (repeatable) {
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

	GLenum format;
	if (texImage.depth == 3) format = GL_RGB; 
	else format = GL_RGBA;

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); 

	gluBuild2DMipmaps 
		(GL_TEXTURE_2D, texImage.depth, texImage.nx,
		texImage.ny, format, GL_UNSIGNED_BYTE, texImage.data);

    return texid;    
}

int CTexture::LoadMipmapTexture (const char *dir, const char *filename, bool repeatable) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadMipmapTexture (path.c_str(), repeatable);
}

void CTexture::LoadTextureList () {
	int rep, id;
	string texfile, line, name;

	CSPList list (200);
	if (list.Load (param.tex_dir, "textures.lst")) {
		for (int i=0; i<list.Count(); i++) {
			line = list.Line (i);
			name = SPStrN (line, "name", "");		
			id = SPIntN (line, "id", 0);
			texfile = SPStrN (line, "file", "");
			rep = SPIntN (line, "repeat", 0);
			if (id >= 0 && id < MAX_COMMON_TEX) {
				if (rep>0) CommonTex[id] = 
					LoadMipmapTexture (param.tex_dir.c_str(), texfile.c_str(), rep);
				else CommonTex[id] = 
					LoadTexture (param.tex_dir.c_str(), texfile.c_str());
				if (CommonTex[id] > 0) {
					TextureIndex = TextureIndex + "[" + name + "]" + Int_StrN (CommonTex[id]);
					numTextures++;
				}
			} else Message ("wrong texture id in textures.lst");	
	}
	} else Message ("failed to load common textures");
}

GLuint CTexture::TexID (int idx) {
	if (idx >= MAX_COMMON_TEX || idx < 0) return 0;
	return CommonTex[idx];
}

GLuint CTexture::TexID (string name) {
	return SPIntN (TextureIndex, name, 0);
}

bool CTexture::BindTex (int idx) {
	if (idx < 0 || idx >= MAX_COMMON_TEX) return false;
	glBindTexture (GL_TEXTURE_2D, CommonTex[idx]);
	return true;
}

bool CTexture::BindTex (string name) {
	GLuint id = SPIntN (TextureIndex, name, 0);
	if (id == 0) return false;
	glBindTexture (GL_TEXTURE_2D, id);
	return true;
}

// ---------------------------- Draw ----------------------------------

void CTexture::DrawDirect (GLuint texid) {
	GLint w, h;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, texid);

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	    glTexCoord2f (0, 0); glVertex2f (0, 0);
	    glTexCoord2f (1, 0); glVertex2f (w, 0);
	    glTexCoord2f (1, 1); glVertex2f (w, h);
	    glTexCoord2f (0, 1); glVertex2f (0, h);
	glEnd();
}

void CTexture::Draw (int idx) {
	DrawDirect (TexID (idx));
}

void CTexture::Draw (string name) {
	DrawDirect (TexID (name));
}

void CTexture::DrawDirect (GLuint texid, int x, int y, float size) {
	GLint w, h;
	GLfloat width, height, top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, texid);

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	width  = w * size;
	height = h * size;

	if (forientation == OR_TOP) {
		top = param.y_resolution - y;
		bott = top - height;

	} else {
		bott = y;
		top = bott + height;
    }
	if (x >= 0) left = x; else left = (param.x_resolution - width) / 2;
	right = left + width;

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	    glTexCoord2f (0, 0); glVertex2f (left, bott);
	    glTexCoord2f (1, 0); glVertex2f (right, bott);
	    glTexCoord2f (1, 1); glVertex2f (right, top);
	    glTexCoord2f (0, 1); glVertex2f (left, top);
	glEnd();
}

void CTexture::Draw (int idx, int x, int y, float size) {
	DrawDirect (TexID (idx), x, y, size);
}

void CTexture::Draw (string name, int x, int y, float size) {
	DrawDirect (TexID (name), x, y, size);
}

void CTexture::DrawDirect (GLuint texid, int x, int y, float width, float height) {
	GLint w, h;
	GLfloat top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, texid);

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	if (forientation == OR_TOP) {
		top = param.y_resolution - y;
		bott = top - height;
	} else {
		bott = y;
		top = bott + height;
    }
	if (x >= 0) left = x; else left = (param.x_resolution - width) / 2;
	right = left + width;

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	    glTexCoord2f (0, 0); glVertex2f (left, bott);
	    glTexCoord2f (1, 0); glVertex2f (right, bott);
	    glTexCoord2f (1, 1); glVertex2f (right, top);
	    glTexCoord2f (0, 1); glVertex2f (left, top);
	glEnd();
}

void CTexture::Draw (int idx, int x, int y, int width, int height) {
	DrawDirect (TexID (idx), x, y, width, height);
}

void CTexture::Draw (string name, int x, int y, int width, int height) {
	DrawDirect (TexID (name), x, y, width, height);
}

void CTexture::DrawDirectFrame (GLuint texid, int x, int y, double w, double h, int frame, TColor col) {
    GLint ww = GLint (w);
	GLint hh = GLint (h);
	GLint xx = x;
	GLint yy = param.y_resolution - hh - y;

	if (texid < 1) return;
	glBindTexture (GL_TEXTURE_2D, texid);

	if (frame > 0) {
		if (w < 1) glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
		if (h < 1) glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &hh);

	   glColor4f (col.r, col.g, col.b, 1.0);
		
		glDisable (GL_TEXTURE_2D);
		glBegin (GL_QUADS);
			glVertex2f (xx - frame, yy - frame);
		    glVertex2f (xx + ww + frame, yy - frame);
		    glVertex2f (xx + ww + frame, yy + hh + frame);
		    glVertex2f (xx - frame, yy + hh + frame);
		glEnd();
		glEnable (GL_TEXTURE_2D);
	}

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
		glTexCoord2f (0, 0); glVertex2f (xx, yy);
	    glTexCoord2f (1, 0); glVertex2f (xx + ww, yy);
	    glTexCoord2f (1, 1); glVertex2f (xx + ww, yy + hh);
	    glTexCoord2f (0, 1); glVertex2f (xx, yy + hh);
	glEnd();
}

void CTexture::DrawFrame (int idx, int x, int y, double w, double h, int frame, TColor col) {
	DrawDirectFrame (TexID (idx), x, y, w, h, frame, col);
}

void CTexture::DrawFrame (string name, int x, int y, double w, double h, int frame, TColor col) {
	DrawDirectFrame (TexID (name), x, y, w, h, frame, col);
}

void CTexture::SetOrientation (int orientation) {
	forientation = orientation;
}

// -------------------------- numeric strings -------------------------

void CTexture::DrawNumChr (char c, int x, int y, int w, int h, TColor col) {
	static string numidxstr = "[0]0[1]1[2]2[3]3[4]4[5]5[6]6[7]7[8]8[9]9[:]10[ ]11";
    float texleft, texright, texw;
	char chrname[2]; 	
    TVector2 bl, tr;
	chrname[0]= c;
	chrname[1]= 0;

	int idx = SPIntN (numidxstr, chrname, 11);	

	// quad coords
	bl.x = x;
	bl.y = param.y_resolution - y - h;
	tr.x = x + w * 0.9;
	tr.y = param.y_resolution - y;

	// texture coords
	texw = 22.0 / 256.0;
	texleft = idx * texw;
	texright = (idx + 1) * texw;

	glColor4f (col.r, col.g, col.b, col.a);
	glBegin (GL_QUADS);
	    glTexCoord2f (texleft, 0); glVertex2f (bl.x, bl.y);
	    glTexCoord2f (texright, 0); glVertex2f (tr.x, bl.y);
	    glTexCoord2f (texright, 1); glVertex2f (tr.x, tr.y);
	    glTexCoord2f (texleft, 1); glVertex2f (bl.x, tr.y);
	glEnd();
} 

void CTexture::DrawNumStr (const char *s, int x, int y, float size, TColor col) {
	if (!BindTex ("ziff032")) {
		Message ("DrawNumStr: missing texture");
		return;
	}
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	int qw = (int)(22 * size);
	int qh = (int)(32 * size);

	for (int i=0; i<int(strlen(s)); i++) {
		DrawNumChr (s[i], x + i*qw, y, qw, qh, col); 
	}
}

// --------------------------------------------------------------------
//				screenshot
// --------------------------------------------------------------------

void ScreenshotN () {
	CImage image;

	string path = param.screenshot_dir;
	path += SEP;
	path += "shot_";
	path += Int_StrN (IRandom (0, 999999), 6);
	path += ".ppm";

	image.LoadFrameBuffer ();
	image.WritePPM (path.c_str());
} 





























