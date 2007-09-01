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
 
#ifndef _PP_ALIGNMENT_H
#define _PP_ALIGNMENT_H

#include "alg/vec2d.h"

namespace pp{

///A class for storing the alignment of an UI widget
class Alignment
{
private:
	float m_horizontal;
	float m_vertical;
	
	///ensure 0.0f <= m_horizontal <= 1.0f 
	void checkHorizontal();

	///ensure 0.0f <= m_vertical <= 1.0f 
	void checkVertical();

public:
	Alignment(float horizontal=0.0f, float vertical=0.0f);
	
	void left()		{ m_horizontal=0.0f; }; ///horizontal align left
	void right()	{ m_horizontal=1.0f; }; ///horizontal align right
	void center()	{ m_horizontal=0.5f; }; ///horizontal allign center
	void bottom()	{ m_vertical=0.0f; };   ///vertical align bottom
	void top()		{ m_vertical=1.0f; };   ///vertical align top
	void middle()	{ m_vertical=0.5f; };	///vertical align middle

	void setHorizontal(float horizontal);
	void setVertical(float vertical);
	void set(float horizontal, float vertical);	
	
	pp::Vec2d alignPosition(const pp::Vec2d &position, const pp::Vec2d &size) const;
	
};

	
} //namespace pp

#endif // PP_ALIGNMENT_H_
