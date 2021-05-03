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

#include "textures.h"
#include "spx.h"
#include "winsys.h"
#include "ogl.h"
#include "gui.h"
#include <cctype>


static const GLshort fullsize_texture[] = {
	0, 1,
	1, 1,
	1, 0,
	0, 0
};


// --------------------------------------------------------------------
//				class TTexture
// --------------------------------------------------------------------

bool TTexture::Load(const std::string& filename, bool repeatable) {
	texture.setSmooth(true);
	texture.setRepeated(repeatable);
	return texture.loadFromFile(filename);
}

bool TTexture::Load(const std::string& dir, const std::string& filename, bool repeatable) {
	return Load(MakePathStr(dir, filename), repeatable);
}

void TTexture::Bind() {
	sf::Texture::bind(&texture);
}

void TTexture::Draw() {
	GLint w, h;

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLint vtx[] = {
		0, 0,
		w, 0,
		w, h,
		0, h
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::Draw(int x, int y, float size) {
	GLint w, h;
	GLfloat width, height, top, bott, left, right;

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	width  = w * size;
	height = h * size;
	top = Winsys.resolution.height - y;
	bott = top - height;

	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLfloat vtx[] = {
		left, bott,
		right, bott,
		right, top,
		left, top
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::Draw(int x, int y, float width, float height) {
	GLfloat top, bott, left, right;

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

	top = Winsys.resolution.height - y;
	bott = top - height;
	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLfloat vtx[] = {
		left, bott,
		right, bott,
		right, top,
		left, top
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::DrawFrame(int x, int y, int w, int h, int frame, const sf::Color& col) {
	if (w < 1) w = texture.getSize().x;
	if (h < 1) h = texture.getSize().y;

	if (frame > 0)
		DrawFrameX(x - frame, y - frame, w + 2 * frame, h + 2 * frame, frame, colTransp, col, 1.f);

	sf::Sprite temp(texture);
	temp.setPosition(x, y);
	temp.setScale((float) w / (float) texture.getSize().x, (float) h / (float) texture.getSize().y);
	Winsys.draw(temp);
}

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

CTexture Tex;

CTexture::CTexture() {
}

CTexture::~CTexture() {
	FreeTextureList();
}

bool CTexture::LoadTextureList() {
	FreeTextureList();
	CSPList list;
	if (list.Load(param.tex_dir, "textures.lst")) {
		for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
			int id = SPIntN(*line, "id", -1);
			CommonTex.resize(std::max(CommonTex.size(), (std::size_t)id+1));
			std::string texfile = SPStrN(*line, "file");
			bool rep = SPBoolN(*line, "repeat", false);
			if (id >= 0) {
				CommonTex[id] = new TTexture();
				CommonTex[id]->Load(param.tex_dir, texfile, rep);
			} else Message("wrong texture id in textures.lst");
		}
	} else {
		Message("failed to load common textures");
		return false;
	}
	return true;
}

void CTexture::FreeTextureList() {
	for (std::size_t i=0; i<CommonTex.size(); i++) {
		delete CommonTex[i];
	}
	CommonTex.clear();
}

TTexture* CTexture::GetTexture(std::size_t idx) const {
	if (idx >= CommonTex.size()) return nullptr;
	return CommonTex[idx];
}

const sf::Texture& CTexture::GetSFTexture(std::size_t idx) const {
	return CommonTex[idx]->texture;
}

bool CTexture::BindTex(std::size_t idx) {
	if (idx >= CommonTex.size()) return false;
	CommonTex[idx]->Bind();
	return true;
}

// ---------------------------- Draw ----------------------------------

void CTexture::Draw(std::size_t idx) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw();
}

void CTexture::Draw(std::size_t idx, int x, int y, float size) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw(x, y, size);
}

void CTexture::Draw(std::size_t idx, int x, int y, int width, int height) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw(x, y, width, height);
}

void CTexture::DrawFrame(std::size_t idx, int x, int y, double w, double h, int frame, const sf::Color& col) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->DrawFrame(x, y, w, h, frame, col);
}

// -------------------------- numeric strings -------------------------

void CTexture::DrawNumChr(char c, int x, int y, int w, int h) {
	int idx;
	if (std::isdigit((unsigned char)c))
		idx = c - '0';
	else if (c == ':')
		idx = 10;
	else if (c == ' ')
		idx = 11;
	else
		return;

	// texture coords
	float texw = 22.0 / 256.0;
	float texleft = idx * texw;
	float texright = (idx + 1) * texw;

	const GLfloat tex[] = {
		texleft, 1,
		texright, 1,
		texright, 0,
		texleft, 0
	};
	const GLfloat vtx[] = {
		static_cast<GLfloat>(x),
		static_cast<GLfloat>(Winsys.resolution.height - y - h),
		static_cast<GLfloat>(x + w * 0.9f),
		static_cast<GLfloat>(Winsys.resolution.height - y - h),
		static_cast<GLfloat>(x + w * 0.9f),
		static_cast<GLfloat>(Winsys.resolution.height - y),
		static_cast<GLfloat>(x),
		static_cast<GLfloat>(Winsys.resolution.height - y)
	};

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void CTexture::DrawNumStr(const std::string& s, int x, int y, float size, const sf::Color& col) {
	if (!BindTex(NUMERIC_FONT)) {
		Message("DrawNumStr: missing texture");
		return;
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	int qw = (int)(22 * size);
	int qh = (int)(32 * size);

	glColor(col);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (std::size_t i=0; i < s.size(); i++) {
		DrawNumChr(s[i], x + (int)i*qw, y, qw, qh);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
