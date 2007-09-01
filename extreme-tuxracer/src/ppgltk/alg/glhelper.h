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
 
#ifndef UTILS_DRAW_H
#define UTILS_DRAW_H

#include "color.h"
#include "vec2d.h"

namespace ppGL {

namespace draw {
	
void rect(pp::Color color, pp::Vec2d startPos, pp::Vec2d endPos);
	
void rect(const char* binding, pp::Vec2d startPos, pp::Vec2d size);	
	
	
} //namespace draw
	
} //namespace ppGL




#endif // GLUTILS_DRAW_H
