/* 
 * Copyright (C) 2004-2005 Volker Stroebel <mmv1@planetpenguin.de>
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
 
#ifndef _PLANE_H
#define _PLANE_H

#include "vec3d.h"

///
namespace pp {

///
class Plane
{
public:
	Plane(void){}
	Plane(const double x, const double y, const double z, const double d);

	double distance(const Vec3d& point) const;
	static bool intersect( const Plane& s1, const Plane& s2, const Plane& s3, Vec3d *p );
		
	Vec3d nml;
    double d;
};

} //namespace pp

#endif // PLANE_H
