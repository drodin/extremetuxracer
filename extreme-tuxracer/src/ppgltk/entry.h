/* 
 * PPRacer 
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
 
#ifndef _PP_ENTRY_H_
#define _PP_ENTRY_H_

#include "widget.h"
#include "font.h"

#include <string>

namespace pp {

class Entry : public Widget {
	
	pp::Font *mp_font;
	std::string m_content;
	unsigned int m_maxchars;
	bool m_editable;
	
	void performClickAction();
	
public:
	Entry( pp::Vec2d pos, pp::Vec2d size,
			 const char *binding, const char *content="" );

	void setMaxChars(unsigned int maxchars);

	void draw();
	bool keyboardEvent(SDLKey key, SDLMod mod, bool release);

	void setContent(std::string string);
	std::string& getContent();

	void setEditable(bool editable){m_editable=editable;};

	//signals
	pp::Signal2<pp::Entry*,SDLKey> signalKeyPressed;

};
	
} //namespace pp

#endif // _PP_ENTRY_H_
