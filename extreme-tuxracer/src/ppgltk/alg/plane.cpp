/* 
 * Copyright (C) 2004-2005 Volker Stroebel <mmv1@planetpenguin.de>
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

#include "plane.h"

#include "gaus.h"



#include <math.h>

namespace pp{

Plane::Plane( const double x, const double y, const double z, const double d ){
	nml.x=x;
	nml.y=y;
	nml.z=z;
	this->d=d;
}

double
Plane::distance(const Vec3d& point) const
{
	return 	nml.x * point.x +
			nml.y * point.y +
			nml.z * point.z +
			d;	
}


bool
Plane::intersect( const Plane& s1, const Plane& s2, const Plane& s3, Vec3d *p )
{
	double A[3][4];
    double x[3];
    double retval;

    A[0][0] =  s1.nml.x;
    A[0][1] =  s1.nml.y;
    A[0][2] =  s1.nml.z;
    A[0][3] = -s1.d;

    A[1][0] =  s2.nml.x;
    A[1][1] =  s2.nml.y;
    A[1][2] =  s2.nml.z;
    A[1][3] = -s2.d;

    A[2][0] =  s3.nml.x;
    A[2][1] =  s3.nml.y;
    A[2][2] =  s3.nml.z;
    A[2][3] = -s3.d;

    retval = gauss( (double*) A, 3, x);

    if ( retval != 0 ) {
	/* Matrix is singular */
	return false;
    } else {
	/* Solution found */
	p->x = x[0];
	p->y = x[1];
	p->z = x[2];
	return true;
    }
}



} //namespace pp
