/* 
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
 
#ifndef _QUAT_H
#define _QUAT_H


#include "vec3d.h"
#include "matrix.h"

///
namespace pp {

class Matrix;

///Quaternion
class Quat
{
public:
	double x;
	double y;
	double z;
	double w;

	Quat(void){};
	Quat(const double x, const double y, const double z, const double w);	
	Quat(const Vec3d& s, const Vec3d& t);
	Quat(const Matrix matrix);
	
	
	void set(const double x, const double y, const double z, const double w);	

	Quat conjugate(void) const;
	Vec3d rotate( const Vec3d& v ) const;
	
	Quat operator*(const Quat& quat) const;

	static Quat interpolate(const Quat& q, Quat r,double t );		
};


} //namespace pp

#endif // QUAT_H
