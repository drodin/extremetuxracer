/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
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
 
#include "stuff.h" 

#include "SDL.h"
 
pp::Vec3d
projectIntoPlane(const pp::Vec3d nml, const pp::Vec3d v)
{
	return v-nml*v*nml;
}

void
getTimeComponents(	float time, 
					int& minutes,
					int& seconds,
					int& hundredths )
{
    minutes = (int) (time / 60);
    seconds = ((int) time) % 60;
    hundredths = ((int) (time * 100 + 0.5) ) % 100;
}

float
getClockTime()
{
    return SDL_GetTicks()/1000.0;
}
