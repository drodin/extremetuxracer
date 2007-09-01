/* 
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
 
#include "alignment.h"
 
namespace pp{

Alignment::Alignment(float horizontal, float vertical)
 : m_horizontal(horizontal),
   m_vertical(vertical)
{	
	checkHorizontal();
	checkVertical();
}

void
Alignment::checkHorizontal()
{
	if(m_horizontal>1.0f){
		m_horizontal = 1.0f;
	}else if(m_horizontal<0.0f){
		m_horizontal = 0.0f;
	}		
}
	
void
Alignment::checkVertical()
{
	if(m_vertical>1.0f){
		m_vertical = 1.0f;
	}else if(m_vertical<0.0f){
		m_vertical = 0.0f;
	}
}

void
Alignment::setHorizontal(float horizontal)
{
	m_horizontal = horizontal;
	checkHorizontal();
}

void
Alignment::setVertical(float vertical)
{
	m_vertical = vertical;
	checkVertical();	
}

void
Alignment::set(float horizontal, float vertical)
{
	setHorizontal(horizontal);
	setVertical(vertical);	
}	

pp::Vec2d
Alignment::alignPosition( const pp::Vec2d &position,
						  const pp::Vec2d &size) const
{
	return pp::Vec2d(
		position.x - (m_horizontal*size.x),
		position.y - (m_vertical*size.y)	
	);	
}

	
} //namespace pp
