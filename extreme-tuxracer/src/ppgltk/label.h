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
 
#ifndef _PP_LABEL_H
#define _PP_LABEL_H

#include "widget.h"
#include "font.h"

#include <string>

namespace pp {

class Label : public Widget {
	
	pp::Font *mp_font;
	pp::Font *mp_insensitiveFont;
	std::string m_text;
public:
	Label( pp::Vec2d pos, const char *binding, const char *text );
	void draw();

	void setText(const char* text);
	void setInsensitiveFont(const char* binding);
};
	
} //namespace pp

#endif // _PP_LABEL_H
