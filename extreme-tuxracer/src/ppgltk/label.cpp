/* 
 * PPRacer 
 * Copyright (C) 2005 Volker Stroebel <volker@planetpenguin.de>
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
 
#include "label.h"

namespace pp {

Label::Label( pp::Vec2d pos, const char *binding, const char *text )
 : mp_insensitiveFont(NULL),
   m_text(text)
{
	m_position = pos;
	mp_font = pp::Font::get(binding);

	if(mp_font){
		m_size.x = mp_font->advance(text);
		m_size.y = mp_font->ascender();	
	}
}

void
Label::setText(const char* text)
{
	m_text=text;
	if(mp_font){
		m_size.x = mp_font->advance(text);
	}
}

void
Label::setInsensitiveFont(const char* binding)
{
	mp_insensitiveFont = pp::Font::get(binding);	
}
	
void
Label::draw()
{
	pp::Font *font = mp_font;
	
	if(!m_sensitive && mp_insensitiveFont){
		font = mp_insensitiveFont;
	}
	
	if(font){
		pp::Vec2d pos = alignment.alignPosition(m_position,m_size);
		font->draw(m_text.c_str(),pos);		
	}
}
	
} //namespace pp
