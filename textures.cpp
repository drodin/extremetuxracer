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
#include "course.h"
// --------------------------------------------------------------------
//				class CImage
// --------------------------------------------------------------------

CImage::CImage () { 
	data = NULL;
	nx = 0;	
    ny = 0; 
    depth = 0;
    pitch = 0;
}

CImage::~CImage () {
	DisposeData ();
}

void CImage::DisposeData () {
	if (data != NULL) {
		free (data);
		data = NULL;
	}
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
	DisposeData ();
	data  = (unsigned char *) malloc (pitch * ny * sizeof (unsigned char));

   	if (SDL_MUSTLOCK (sdlImage)) {
   	    if (SDL_LockSurface (sdlImage) < 0) {
			SDL_FreeSurface (sdlImage);
	   		Message ("mustlock error");
			return false;
		};
   	}

	sdlData = (unsigned char *) sdlImage->pixels;

	if (mirroring) {
		for (int y=0; y<ny; y++) {
			memcpy(data + y*pitch, sdlData + (ny-1-y)*pitch, pitch);
		}
	} else {
		memcpy(data, sdlData, ny*pitch);
	}

	if (SDL_MUSTLOCK (sdlImage)) SDL_UnlockSurface (sdlImage);
	SDL_FreeSurface (sdlImage);
	return true;
}

bool CImage::LoadPng (const char *dir, const char *filename, bool mirroring) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadPng (path.c_str(), mirroring);
}

// ------------------ read framebuffer --------------------------------

bool CImage::ReadFrameBuffer_PPM () {
	int viewport[4];
	glGetIntegerv (GL_VIEWPORT, viewport);
	
	nx = viewport[2];
	ny = viewport[3];
	depth = 3;

	DisposeData ();
	data  = (unsigned char *) malloc (nx * ny * depth * sizeof (unsigned char));
	
	glReadBuffer (GL_FRONT);
	
	for (int i=0; i<viewport[3]; i++){
		glReadPixels (viewport[0], viewport[1] + viewport[3] - 1 - i,
			viewport[2], 1, GL_RGB, GL_UNSIGNED_BYTE, data + viewport[2] * i * 3);
	}
	
	return true;
}

void CImage::ReadFrameBuffer_TGA () {
	nx = param.x_resolution;
	ny = param.y_resolution;
	depth = 3;

	DisposeData ();
	data  = (unsigned char *) malloc (nx * ny * depth * sizeof (unsigned char));

	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGR, GL_UNSIGNED_BYTE, data);	
}

void CImage::ReadFrameBuffer_BMP () {
	nx = param.x_resolution;
	ny = param.y_resolution;
	depth = 4;

	DisposeData ();
	data  = (unsigned char *) malloc (nx * ny * depth * sizeof (unsigned char));
	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGRA, GL_UNSIGNED_BYTE, data);	
}

// ---------------------------

void CImage::WritePPM (const char *filepath) {
	if (data == NULL) return;
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

void CImage::WriteTGA (const char *filepath) {
	if (data == NULL) return;
	std::ofstream out(filepath, std::ios_base::out|std::ios_base::binary);
	short TGAhead[] = {0, 2, 0, 0, 0, 0, nx, ny, 24};

	out.write(reinterpret_cast<char*>(&TGAhead), sizeof(TGAhead));
	out.write(reinterpret_cast<char*>(data), 3 * nx * ny);
}

void CImage::WriteTGA (const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	WriteTGA (path.c_str());
}

void CImage::WriteTGA_H (const char *filepath) {
	if (data == NULL) return;
	TTgaHeader header;

	header.tfType = 0;
    header.tfColorMapType = 0;
    header.tfImageType = 2;
    for (int i=0; i<5; i++) header.tfColorMapSpec[i] = 0;
    header.tfOrigX = 0;
    header.tfOrigY = 0;
    header.tfWidth = param.x_resolution;
    header.tfHeight = param.y_resolution;
    header.tfBpp = 24;
    header.tfImageDes = 0;

	std::ofstream out(filepath, std::ios_base::out|std::ios_base::binary);
	out.write(reinterpret_cast<char*>(&header), sizeof(TTgaHeader));
	out.write(reinterpret_cast<char*>(data), 3 * nx * ny);
}

void CImage::WriteTGA_H (const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	WriteTGA_H (path.c_str());
}

void CImage::WriteBMP (const char *filepath) {
	if (data == NULL) return;
	TBmpInfo info;
    FILE *fp;       
    int  infosize; 
	unsigned int bitsize;   

	info.biSize = 40;
	info.biWidth = nx;
	info.biHeight = ny;
	info.biPlanes = 1;
	info.biBitCount = 8 * depth;
	info.biCompression = 0;
	info.biSizeImage = nx * ny * depth; 
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter= 0;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

    if ((fp = fopen (filepath, "wb")) == NULL) {
		Message ("could not open bmp file", filepath);
		return;
	}

	int imgsize = info.biSizeImage;
	int width = info.biWidth;
	int height = info.biHeight;
	int bitcnt = info.biBitCount; // 24 or 32

	// (width * bitcnt + 7) / 8 = width * depth
    if (imgsize == 0) bitsize = (width * bitcnt + 7) / 8 * height;
    else bitsize = imgsize;

    infosize = info.biSize; // 40
	if (infosize != 40 || info.biCompression != 0) {
		Message ("wrong bmp header");
		fclose(fp);
		return;
	}

    write_word  (fp, 0x4D42); 
    write_dword (fp, 14 + infosize + bitsize);    
    write_word  (fp, 0);        
    write_word  (fp, 0);        
    write_dword (fp, 54);

    write_dword (fp, info.biSize);
    write_long  (fp, info.biWidth);
    write_long  (fp, info.biHeight);
    write_word  (fp, info.biPlanes);
    write_word  (fp, info.biBitCount);
    write_dword (fp, info.biCompression);
    write_dword (fp, info.biSizeImage);
    write_long  (fp, info.biXPelsPerMeter);
    write_long  (fp, info.biYPelsPerMeter);
    write_dword (fp, info.biClrUsed);
    write_dword (fp, info.biClrImportant);

    if (fwrite (data, 1, bitsize, fp) != bitsize) {
		Message ("error on writing bmp data");
        fclose (fp);
        return;
    }

    fclose(fp);
    return;
}

void CImage::WriteBMP (const char *dir, const char *filename) {
	string path = dir;
	path += SEP;
	path += filename;
	WriteBMP (path.c_str());
}

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

CTexture Tex;

CTexture::CTexture () {
	forientation = OR_TOP;
}

CTexture::~CTexture () {
	FreeTextureList();
}

int CTexture::LoadTexture (const string& filename) {
    CImage texImage;
	GLuint texid;

	if (texImage.LoadPng (filename.c_str(), true) == false) return 0;
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

	texImage.DisposeData();
    return texid;    
}

int CTexture::LoadTexture (const string& dir, const string& filename) {
	return LoadTexture(dir + SEP + filename);
}

int CTexture::LoadMipmapTexture (const string& filename, bool repeatable) {
    CImage texImage;
	GLuint texid;

	if (texImage.LoadPng (filename.c_str(), true) == false) return 0;
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
	
	texImage.DisposeData();
    return texid;    
}

int CTexture::LoadMipmapTexture (const string& dir, const string& filename, bool repeatable) {
	return LoadMipmapTexture(dir + SEP + filename, repeatable);
}

void CTexture::LoadTextureList () {
	TextureIndex = "";
	CSPList list (200);
	if (list.Load (param.tex_dir, "textures.lst")) {
		for (int i=0; i<list.Count(); i++) {
			string line = list.Line (i);
			string name = SPStrN (line, "name", "");
			int id = SPIntN (line, "id", -1);
			CommonTex.resize(max(CommonTex.size(), (size_t)id+1));
			string texfile = SPStrN (line, "file", "");
			bool rep = SPIntN (line, "repeat", 0) != 0;
			if (id >= 0) {
				if (rep) CommonTex[id] =
					LoadMipmapTexture (param.tex_dir, texfile, rep);
				else CommonTex[id] = 
					LoadTexture (param.tex_dir, texfile);
				if (CommonTex[id] > 0) {
					TextureIndex = TextureIndex + "[" + name + "]" + Int_StrN (CommonTex[id]);
				}
			} else Message ("wrong texture id in textures.lst");	
		}
	} else Message ("failed to load common textures");
}

void CTexture::FreeTextureList () {
	for (int i=0; i<CommonTex.size(); i++) {
		if (CommonTex[i] > 0) {
			glDeleteTextures (1, &CommonTex[i]);
		}
	}
	TextureIndex = "";
	CommonTex.clear();
}

GLuint CTexture::TexID (int idx) const {
	if (idx >= CommonTex.size() || idx < 0) return 0;
	return CommonTex[idx];
}

GLuint CTexture::TexID (const string& name) const {
	return SPIntN (TextureIndex, name, 0);
}

bool CTexture::BindTex (int idx) {
	if (idx < 0 || idx >= CommonTex.size()) return false;
	glBindTexture (GL_TEXTURE_2D, CommonTex[idx]);
	return true;
}

bool CTexture::BindTex (const string& name) {
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

void CTexture::Draw (const string& name) {
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

void CTexture::Draw (const string& name, int x, int y, float size) {
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

void CTexture::Draw (const string& name, int x, int y, int width, int height) {
	DrawDirect (TexID (name), x, y, width, height);
}

void CTexture::DrawDirectFrame (GLuint texid, int x, int y, double w, double h, int frame, const TColor& col) {
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

void CTexture::DrawFrame (int idx, int x, int y, double w, double h, int frame, const TColor& col) {
	DrawDirectFrame (TexID (idx), x, y, w, h, frame, col);
}

void CTexture::DrawFrame (const string& name, int x, int y, double w, double h, int frame, const TColor& col) {
	DrawDirectFrame (TexID (name), x, y, w, h, frame, col);
}

void CTexture::SetOrientation (Orientation orientation) {
	forientation = orientation;
}

// -------------------------- numeric strings -------------------------

void CTexture::DrawNumChr (char c, int x, int y, int w, int h, const TColor& col) {
	int idx;
	if(isdigit(c)) {
		char chrname[2] = {c, '\0'};
		idx = atoi(chrname);
	}
	else if(c == ':')
		idx = 10;
	else if(c == ' ')
		idx = 11;
	else
		return;

    TVector2 bl, tr;
	// quad coords
	bl.x = x;
	bl.y = param.y_resolution - y - h;
	tr.x = x + w * 0.9;
	tr.y = param.y_resolution - y;

	// texture coords
	float texw = 22.0 / 256.0;
	float texleft = idx * texw;
	float texright = (idx + 1) * texw;

	glColor4f (col.r, col.g, col.b, col.a);
	glBegin (GL_QUADS);
	    glTexCoord2f (texleft, 0); glVertex2f (bl.x, bl.y);
	    glTexCoord2f (texright, 0); glVertex2f (tr.x, bl.y);
	    glTexCoord2f (texright, 1); glVertex2f (tr.x, tr.y);
	    glTexCoord2f (texleft, 1); glVertex2f (bl.x, tr.y);
	glEnd();
} 

void CTexture::DrawNumStr (const char *s, int x, int y, float size, const TColor& col) {
	if (!BindTex ("ziff032")) {
		Message ("DrawNumStr: missing texture");
		return;
	}
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
	int qw = (int)(22 * size);
	int qh = (int)(32 * size);

	for (size_t i=0; s[i] != '\0'; i++) {
		DrawNumChr (s[i], x + i*qw, y, qw, qh, col); 
	}
}

// --------------------------------------------------------------------
//				screenshot
// --------------------------------------------------------------------

// 0 ppm, 1 tga, 2 tga_header, 3 bmp
#define SCREENSHOT_PROC 3

void ScreenshotN () {
	CImage image;
	string path = param.screenshot_dir;
	path += SEP;
	path += Course.CourseList[g_game.course_id].dir;
	path += "_";
	path += GetTimeString1 ();
	int type = SCREENSHOT_PROC;

	switch (type) {
		case 0:
			path += ".ppm";
			image.ReadFrameBuffer_PPM ();
			image.WritePPM (path.c_str());
			image.DisposeData ();
			break;
		case 1:
			path += ".tga";
			image.ReadFrameBuffer_TGA ();
			image.WriteTGA (path.c_str());
			image.DisposeData ();
			break;
		case 2:
			path += ".tga";
			image.ReadFrameBuffer_TGA ();
			image.WriteTGA_H (path.c_str());
			image.DisposeData ();
			break;
		case 3:
			path += ".bmp";
			image.ReadFrameBuffer_BMP ();
			image.WriteBMP (path.c_str());
			image.DisposeData ();
			break;
	}
}
