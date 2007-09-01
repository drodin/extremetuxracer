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

#ifndef _PP_FONT_H_
#define _PP_FONT_H_

#include <map>
#include <string>

#include "FT/FTFont.h"
#include "alg/color.h"
#include "alg/vec2d.h"

namespace pp{
	
class Font
{
private:
	FTFont *mp_font;
	pp::Color m_color;

public:	
	Font(const char *fileName, unsigned int size, const pp::Color &color);

	Font(FTFont *font, const pp::Color &color);

	~Font();

	///draws the utf8 string at position x,y
	void draw(const char *string, float x, float y);

	///draws the utf8 string at the specified position 
	void draw(const char *string, pp::Vec2d position);

	///draws the unicode string at position x,y 
	void draw(const wchar_t *string, float x, float y);

	///draws the unicode string at the specified position
	void draw(const wchar_t *string, pp::Vec2d position);

	float ascender();
	float descender();
	float advance(const char* string);
	float advance(const wchar_t* string);

	pp::Color& getColor();
	FTFont* getFTFont();
	
//static stuff	
private:
	
	/// stores the font bindings
	static std::map<std::string, pp::Font*> sm_bindings;

public:
	
	/// loads and registers the ttf file to the specified binding
	static bool registerFont(const char *binding, const char *fileName, unsigned int size, const pp::Color &color);
	
	/// bind font to the specified font
	static bool bindFont(const char *binding, const char *fontName);
	
	/// bind font to the specified font with a different color
	static bool bindFont(const char *binding, const char *fontName, const pp::Color& color);

	/// returns a pointer to a font
	static Font* get(const char *binding);
	
	static void draw(const char *binding, const char *string, float x, float y);

	/// returns the unicode of the utf8 string.
	static wchar_t* utf8ToUnicode(const char* string);

	/// fills the buff array with the unicode string.
	static void utf8ToUnicode(wchar_t* buff, const char* string);
};

} //namepsace pp

#endif // _PP_FONT_H_
