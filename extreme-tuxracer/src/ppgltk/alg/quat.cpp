/* 
 * PPGLTK
 *
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

#include "quat.h"
#include "defs.h"

#include <math.h>

namespace pp {

	
Quat::Quat(const double x, const double y, const double z, const double w)
 : x(x), y(y), z(z), w(w)
{
}	
	
Quat::Quat(const Vec3d& s, const Vec3d& t)
{
    Vec3d u = s^t;
	if (u.normalize() < EPS ){
		x=0.0;
		y=0.0;
		z=0.0;
		w=1.0;
    }else{
		double cos2phi = s*t;
		double sinphi = sqrt( ( 1 - cos2phi ) / 2.0 );
		double cosphi = sqrt( ( 1 + cos2phi ) / 2.0 );

		x = sinphi * u.x;
		y = sinphi * u.y;
		z = sinphi * u.z;
		w = cosphi;
    }
}

// source:
// http://www.gamasutra.com/features/19980703/quaternions_01.htm
//
Quat::Quat(const Matrix matrix)
{
    static int nxt[3] = {1, 2, 0};
    double tr = matrix.data[0][0] + matrix.data[1][1] + matrix.data[2][2];

    // check the diagonal
   	if (tr > 0.0) {
		double s = sqrt (tr + 1.0);
		w = 0.5 * s;
		s = 0.5 / s;
		x = (matrix.data[1][2] - matrix.data[2][1]) * s;
		y = (matrix.data[2][0] - matrix.data[0][2]) * s;
		z = (matrix.data[0][1] - matrix.data[1][0]) * s;
    } else {                
		// diagonal is negative
		int i = 0;
		if (matrix.data[1][1] > matrix.data[0][0]) i = 1;
		if (matrix.data[2][2] > matrix.data[i][i]) i = 2;
		int j = nxt[i];
		int k = nxt[j];

		double s = sqrt (matrix.data[i][i] - matrix.data[j][j] - matrix.data[k][k] + 1.0);
        
		double q[4];		
		q[i] = s * 0.5;
                             
		if (s != 0.0) s = 0.5 / s;

		q[3] = (matrix.data[j][k] - matrix.data[k][j]) * s;
		q[j] = (matrix.data[i][j] + matrix.data[j][i]) * s;
		q[k] = (matrix.data[i][k] + matrix.data[k][i]) * s;

		x = q[0];
		y = q[1];
		z = q[2];
		w = q[3];
	}
}



void
Quat::set(const double x, const double y, const double z, const double w){
	this->x=x;
	this->y=y;
	this->z=z;
	this->w=w;
}


Quat
Quat::conjugate(void) const
{
	return Quat(-x, -y, -z, w);	
}

Vec3d
Quat::rotate( const Vec3d& v ) const
{
    Quat p(v.x,v.y,v.z,1.0);
    Quat res_q = (*this)*(p*conjugate());
    
	return Vec3d(res_q.x,res_q.y,res_q.z);
}

Quat
Quat::operator*(const Quat& quat) const{
	return Quat(
		y * quat.z - z * quat.y + quat.w * x + w * quat.x,
		z * quat.x - x * quat.z + quat.w * y + w * quat.y,
		x * quat.y - y * quat.x + quat.w * z + w * quat.z,
		w * quat.w - x * quat.x - y * quat.y - z * quat.z
	);
}



Quat 
Quat::interpolate(const Quat& q, Quat r,double t )
{
	Quat res;
    double cosphi;
    double sinphi;
    double phi;
    double scale0, scale1;

    cosphi = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;

    // adjust signs (if necessary) 
    if ( cosphi < 0.0 ) {
	cosphi = -cosphi;
	r.x = -r.x;
	r.y = -r.y;
	r.z = -r.z;
	r.w = -r.w;
    }

    if ( 1.0 - cosphi > EPS ) {
	// standard case -- slerp 
	phi = acos( cosphi );
	sinphi = sin( phi );
	scale0 = sin( phi * ( 1.0 - t ) ) / sinphi;
	scale1 = sin( phi * t ) / sinphi;
    } else {
	// use linear interpolation to avoid division by zero
	scale0 = 1.0 - t;
	scale1 = t;
    }

    res.x = scale0 * q.x + scale1 * r.x; 
    res.y = scale0 * q.y + scale1 * r.y; 
    res.z = scale0 * q.z + scale1 * r.z; 
    res.w = scale0 * q.w + scale1 * r.w; 

    return res;
}
	
} //namespace pp
