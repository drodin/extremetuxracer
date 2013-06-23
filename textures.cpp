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

#include "textures.h"
#include "spx.h"
#include "course.h"
#include "winsys.h"
#include <fstream>
#include <cctype>
#include <cstdio>

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
		delete[] data;
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
	data  = new unsigned char[pitch * ny];

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
	data  = new unsigned char[nx * ny * depth];

	glReadBuffer (GL_FRONT);

	for (int i=0; i<viewport[3]; i++) {
		glReadPixels (viewport[0], viewport[1] + viewport[3] - 1 - i,
			viewport[2], 1, GL_RGB, GL_UNSIGNED_BYTE, data + viewport[2] * i * 3);
	}

	return true;
}

void CImage::ReadFrameBuffer_TGA () {
	nx = Winsys.resolution.width;
	ny = Winsys.resolution.height;
	depth = 3;

	DisposeData ();
	data  = new unsigned char[nx * ny * depth];

	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGR, GL_UNSIGNED_BYTE, data);
}

void CImage::ReadFrameBuffer_BMP () {
	nx = Winsys.resolution.width;
	ny = Winsys.resolution.height;
	depth = 4;

	DisposeData ();
	data  = new unsigned char[nx * ny * depth];
	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGRA, GL_UNSIGNED_BYTE, data);
}

// ---------------------------

void CImage::WritePPM (const char *filepath) {
	if (data == NULL) return;
	std::ofstream file(filepath);

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
    header.tfWidth = Winsys.resolution.width;
    header.tfHeight = Winsys.resolution.height;
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
//				class TTexture
// --------------------------------------------------------------------

TTexture::~TTexture() {
	glDeleteTextures (1, &id);
}

bool TTexture::Load(const string& filename) {
    CImage texImage;

	if (texImage.LoadPng (filename.c_str(), true) == false)
		return false;
	glGenTextures (1, &id);
	glBindTexture (GL_TEXTURE_2D, id);
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
    return true;
}
bool TTexture::Load(const string& dir, const string& filename) {
	return Load(dir + SEP + filename);
}

bool TTexture::LoadMipmap(const string& filename, bool repeatable) {
    CImage texImage;
	if (texImage.LoadPng (filename.c_str(), true) == false)
		return false;

	glGenTextures (1, &id);
	glBindTexture (GL_TEXTURE_2D, id);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

   if (repeatable) {
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
    return true;
}
bool TTexture::LoadMipmap(const string& dir, const string& filename, bool repeatable) {
	return LoadMipmap(dir + SEP + filename, repeatable);
}

void TTexture::Bind() {
	glBindTexture (GL_TEXTURE_2D, id);
}

void TTexture::Draw() {
	GLint w, h;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, id);

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

void TTexture::Draw(int x, int y, float size, Orientation orientation) {
	GLint w, h;
	GLfloat width, height, top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, id);

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	width  = w * size;
	height = h * size;

	if (orientation == OR_TOP) {
		top = Winsys.resolution.height - y;
		bott = top - height;

	} else {
		bott = y;
		top = bott + height;
    }
	if (x >= 0) left = x; else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	    glTexCoord2f (0, 0); glVertex2f (left, bott);
	    glTexCoord2f (1, 0); glVertex2f (right, bott);
	    glTexCoord2f (1, 1); glVertex2f (right, top);
	    glTexCoord2f (0, 1); glVertex2f (left, top);
	glEnd();
}

void TTexture::Draw(int x, int y, float width, float height, Orientation orientation) {
	GLint w, h;
	GLfloat top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture (GL_TEXTURE_2D, id);

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	if (orientation == OR_TOP) {
		top = Winsys.resolution.height - y;
		bott = top - height;
	} else {
		bott = y;
		top = bott + height;
    }
	if (x >= 0) left = x; else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

    glColor4f (1.0, 1.0, 1.0, 1.0);
	glBegin (GL_QUADS);
	    glTexCoord2f (0, 0); glVertex2f (left, bott);
	    glTexCoord2f (1, 0); glVertex2f (right, bott);
	    glTexCoord2f (1, 1); glVertex2f (right, top);
	    glTexCoord2f (0, 1); glVertex2f (left, top);
	glEnd();
}

void TTexture::DrawFrame(int x, int y, double w, double h, int frame, const TColor& col) {
	if (id < 1)
		return;

    GLint ww = GLint (w);
	GLint hh = GLint (h);
	GLint xx = x;
	GLint yy = Winsys.resolution.height - hh - y;

	glBindTexture (GL_TEXTURE_2D, id);

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

void CTexture::LoadTextureList () {
	FreeTextureList();
	CSPList list (200);
	if (list.Load (param.tex_dir, "textures.lst")) {
		for (size_t i=0; i<list.Count(); i++) {
			const string& line = list.Line(i);
			string name = SPStrN (line, "name", "");
			int id = SPIntN (line, "id", -1);
			CommonTex.resize(max(CommonTex.size(), (size_t)id+1));
			string texfile = SPStrN (line, "file", "");
			bool rep = SPBoolN (line, "repeat", false);
			if (id >= 0) {
				CommonTex[id] = new TTexture();
				if (rep)
					CommonTex[id]->LoadMipmap(param.tex_dir, texfile, rep);
				else
					CommonTex[id]->Load(param.tex_dir, texfile);

				Index[name] = CommonTex[id];
			} else Message ("wrong texture id in textures.lst");
		}
	} else Message ("failed to load common textures");
}

void CTexture::FreeTextureList () {
	for (size_t i=0; i<CommonTex.size(); i++) {
		delete CommonTex[i];
	}
	CommonTex.clear();
	Index.clear();
}

TTexture* CTexture::GetTexture (size_t idx) const {
	if (idx >= CommonTex.size() || idx < 0) return NULL;
	return CommonTex[idx];
}

TTexture* CTexture::GetTexture (const string& name) const {
	return Index.at(name);
}

bool CTexture::BindTex (size_t idx) {
	if (idx < 0 || idx >= CommonTex.size()) return false;
	CommonTex[idx]->Bind();
	return true;
}

bool CTexture::BindTex (const string& name) {
	try {
		Index.at(name)->Bind();
	} catch(...) {
		return false;
	}
	return true;
}

// ---------------------------- Draw ----------------------------------

void CTexture::Draw (size_t idx) {
	if(CommonTex.size() > idx)
		CommonTex[idx]->Draw();
}

void CTexture::Draw (const string& name) {
	Index[name]->Draw();
}

void CTexture::Draw (size_t idx, int x, int y, float size) {
	if(CommonTex.size() > idx)
		CommonTex[idx]->Draw(x, y, size, forientation);
}

void CTexture::Draw (const string& name, int x, int y, float size) {
	Index[name]->Draw(x, y, size, forientation);
}

void CTexture::Draw (size_t idx, int x, int y, int width, int height) {
	if(CommonTex.size() > idx)
		CommonTex[idx]->Draw (x, y, width, height, forientation);
}

void CTexture::Draw (const string& name, int x, int y, int width, int height) {
	Index[name]->Draw (x, y, width, height, forientation);
}

void CTexture::DrawFrame (size_t idx, int x, int y, double w, double h, int frame, const TColor& col) {
	if(CommonTex.size() > idx)
		CommonTex[idx]->DrawFrame (x, y, w, h, frame, col);
}

void CTexture::DrawFrame (const string& name, int x, int y, double w, double h, int frame, const TColor& col) {
	Index[name]->DrawFrame (x, y, w, h, frame, col);
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
	bl.y = Winsys.resolution.height - y - h;
	tr.x = x + w * 0.9;
	tr.y = Winsys.resolution.height - y;

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
		DrawNumChr (s[i], x + (int)i*qw, y, qw, qh, col);
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
