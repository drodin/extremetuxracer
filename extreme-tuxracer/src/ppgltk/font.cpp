/* 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
/*
 * Note:
 * The way we bind fonts to other fonts is a dirty hack and
 * lacks resource management / refpointers.
 * Therefore we don't delete mp_font in the destructor.
 * This isn't a real memleak because we don't delete fonts
 * in current ppracer version.
 * 
 * This will changed with the new resource manager that
 * isn't ready yet.
 */ 

#include "font.h"

#ifdef _WIN32
	#include <WTypes.h>
#endif

#include <GL/gl.h>
#include <iostream>

#include "FT/FTGLTextureFont.h"

namespace pp{
	
Font::Font(const char *fileName, unsigned int size, const pp::Color &color)
 : mp_font(NULL),
   m_color(color)
{
	mp_font = new FTGLTextureFont(fileName);
	
	mp_font->FaceSize(size);
	mp_font->CharMap(ft_encoding_unicode);	
}

Font::Font(FTFont *font, const pp::Color &color)
 : mp_font(font),
   m_color(color)
{
}

Font::~Font()
{
	/* we don't have a refpointer yet...	
	if(mp_font!=NULL){
		delete mp_font;
	}
	*/
}
	
void
Font::draw(const char *string, float x, float y)
{
	const wchar_t* u_string;
	u_string = Font::utf8ToUnicode(string);	

	glPushMatrix();
	{
		glColor4dv( (double*)&m_color );
		glTranslatef(x, y, 0);
		mp_font->Render(u_string);
	}
	glPopMatrix();

	delete u_string;
}

void
Font::draw(const char* string, pp::Vec2d position)
{
	draw(string, position.x, position.y);
}

void
Font::draw(const wchar_t *string, float x, float y)
{
	glPushMatrix();
	{
		glColor4dv( (double*)&m_color );
		glTranslatef(x, y, 0);
		mp_font->Render(string);
	}
	glPopMatrix();
}

void
Font::draw(const wchar_t *string, pp::Vec2d position)
{
	draw(string, position.x, position.y);
}

float
Font::ascender()
{
	return mp_font->Ascender();
}

float
Font::descender()
{
	return mp_font->Descender();
}

float
Font::advance(const char* string)
{
	const wchar_t* u_string;
	u_string = Font::utf8ToUnicode(string);	
	
	float adv = mp_font->Advance(u_string);
	
	delete u_string;
	return adv;	
}

float
Font::advance(const wchar_t* string)
{
	return mp_font->Advance(string);	
}

pp::Color&
Font::getColor()
{
	return m_color;
}

FTFont*
Font::getFTFont()
{
	return mp_font;
}
	

//static stuff

std::map<std::string, pp::Font*> Font::sm_bindings;

bool
Font::registerFont(const char *binding, const char *fileName, unsigned int size, const pp::Color &color)
{
	//todo: check wether the font alrady exists	
	sm_bindings[binding] = new pp::Font(fileName, size, color);
	return true;
}

bool
Font::bindFont(const char *binding, const char *fontName)
{
	pp::Font *font = pp::Font::get(fontName);
	if(font==NULL) return false;
		
	sm_bindings[binding] = new pp::Font(font->getFTFont(), font->getColor());
	return true;
}	

bool
Font::bindFont(const char *binding, const char *fontName, const pp::Color &color)
{
	pp::Font *font = pp::Font::get(fontName);
	if(font==NULL) return false;
		
	sm_bindings[binding] = new pp::Font(font->getFTFont(), color);
	return true;
}

Font*
Font::get(const char* binding)
{
	if(binding==NULL){
		return NULL;
	}
	
	std::map<std::string, pp::Font*>::iterator it;
	it = sm_bindings.find(binding);
	if (it != sm_bindings.end()){
		return (*it).second;
	}else{
		return NULL;
	}	
}

void
Font::draw(const char* binding, const char *string, float x, float y)
{
	pp::Font* font = pp::Font::get(binding);
	
	if(font!=NULL){	
		font->draw(string, x, y);
	}
}

wchar_t*
Font::utf8ToUnicode(const char* string)
{
	wchar_t ch;
	int len = strlen(string);
	wchar_t *u_string = new wchar_t[len+1];
	int j=0;
		
	for (int i=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)string)[i];
		if ( ch >= 0xF0 ) {
			ch  =  (wchar_t)(string[i]&0x07) << 18;
			ch |=  (wchar_t)(string[++i]&0x3F) << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xE0 ) {
			ch  =  (wchar_t)(string[i]&0x0F) << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xC0 ) {
			ch  =  (wchar_t)(string[i]&0x1F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		}
		u_string[j] = ch;
	}
	u_string[j] = 0;
	
	return u_string;
}

void
Font::utf8ToUnicode(wchar_t* buff, const char* string)
{
	wchar_t ch;
	int len = strlen(string);
	int j=0;
		
	for (int i=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)string)[i];
		if ( ch >= 0xF0 ) {
			ch  =  (wchar_t)(string[i]&0x07) << 18;
			ch |=  (wchar_t)(string[++i]&0x3F) << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xE0 ) {
			ch  =  (wchar_t)(string[i]&0x0F) << 12;
			ch |=  (wchar_t)(string[++i]&0x3F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		} else
		if ( ch >= 0xC0 ) {
			ch  =  (wchar_t)(string[i]&0x1F) << 6;
			ch |=  (wchar_t)(string[++i]&0x3F);
		}
		buff[j] = ch;
	}
	buff[j] = 0;
}


} //namepsace pp
