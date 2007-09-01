/* 
 * PPGLTK
 *
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

#include "vec3d.h"

#include <math.h>

namespace pp{

Vec3d::Vec3d()
 : x(0.0), y(0.0), z(0.0)
{
}
	
Vec3d::Vec3d(const double x, const double y, const double z)
 : x(x), y(y), z(z)
{
}

Vec3d::Vec3d(const double *v)
 : x(v[0]), y(v[1]), z(v[2])
{
}


double 
Vec3d::normalize()
{
    double len = length();
	if (len>0.0)
	{
		x /= len;
		y /= len;
		z /= len;
	}                
	return(len);
};

double
Vec3d::length() const
{
	return sqrt(x*x+y*y+z*z);
}

double
Vec3d::length2() const
{
	return x*x+y*y+z*z;
}

Vec3d
operator+(const Vec3d& vec1,const Vec3d& vec2)
{
	return Vec3d(vec1.x+vec2.x,vec1.y+vec2.y,vec1.z+vec2.z);
}

Vec3d
operator-(const Vec3d& vec1,const Vec3d& vec2)
{
	return Vec3d(vec1.x-vec2.x,vec1.y-vec2.y,vec1.z-vec2.z);
}

Vec3d
operator*(const Vec3d& vec, const double scalar)
{
	return Vec3d(vec.x*scalar,vec.y*scalar,vec.z*scalar);
}

Vec3d
operator*(const double scalar, const Vec3d& vec)
{
	return Vec3d(vec.x*scalar,vec.y*scalar,vec.z*scalar);
}

double
operator*(const Vec3d& vec1,const Vec3d& vec2)
{
	return vec1.x*vec2.x+vec1.y*vec2.y+vec1.z*vec2.z;
}

Vec3d 
operator^(const Vec3d& vec1,const Vec3d& vec2)
{
	return Vec3d(
		vec1.y * vec2.z - vec1.z * vec2.y,
		vec1.z * vec2.x - vec1.x * vec2.z,
		vec1.x * vec2.y - vec1.y * vec2.x
	);
}

} //namespace pp
