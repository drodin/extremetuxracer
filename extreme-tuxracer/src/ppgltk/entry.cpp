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

#include "entry.h"

#include "ui_theme.h"
#include "render_util.h"
#include "alg/glhelper.h"
#include "alg/defs.h"

namespace pp {
		
Entry::Entry( pp::Vec2d pos, pp::Vec2d size,
			 const char *binding, const char *content )
 : mp_font(NULL)
{
	m_position = pos;
	m_size = size;
	m_content = content;
	
	mp_font = pp::Font::get(binding);
	
	m_editable=true;
	m_maxchars=8;
	
	setSensitive(true);
}
	
void
Entry::setMaxChars(unsigned int maxchars)
{
	m_maxchars = MAX(1,maxchars);	
}


void
Entry::draw()
{
	if(m_hasFocus){
		ppGL::draw::rect(theme.highlight,
				m_position,
				m_position+m_size);	
	}else{	
		ppGL::draw::rect(theme.foreground,
				m_position,
				m_position+m_size);
	}
	ppGL::draw::rect(theme.background,
				m_position+theme.border,
				m_position+m_size-theme.border);
	
	mp_font->draw(m_content.c_str(), m_position+theme.textPadding);
}

void
Entry::performClickAction()
{
	setFocus(true);
}

void
Entry::setContent(std::string string)
{
	m_content = string;
}

std::string&
Entry::getContent()
{
	return m_content;
}



bool
Entry::keyboardEvent(SDLKey key, SDLMod mod, bool release)
{
	if(!release){
		if(m_editable){
			if( islower(key) ){
				if(m_content.size()<m_maxchars){
					if(mod & KMOD_SHIFT){
						m_content+=toupper(key);
					}else{
						m_content+=key;
					}
				}
			}else if( isdigit(key) ){
				if(m_content.size()<m_maxchars){
					m_content+=key;
				}
			}else if( key == SDLK_BACKSPACE){
				if(!m_content.empty()){
					std::string::iterator it = m_content.end();
					it--;
					m_content.erase(it);	
					//i'm stupid... or g++-2.95?
					//content.erase( --m_content.end() );
				}
			}
		}else{
			signalKeyPressed.Emit(this,key);
		}
	}
	return true;
}

} //namespace pp
