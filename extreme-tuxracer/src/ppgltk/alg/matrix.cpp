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
 
#include "matrix.h"
#include "defs.h"
#include <math.h>

namespace pp {
	
Matrix::Matrix(const Quat quat)
{
    data[0][0] = 1.0 - 2.0 * ( quat.y * quat.y + quat.z * quat.z );
    data[1][0] =       2.0 * ( quat.x * quat.y - quat.w * quat.z );
    data[2][0] =       2.0 * ( quat.x * quat.z + quat.w * quat.y );

    data[0][1] =       2.0 * ( quat.x * quat.y + quat.w * quat.z );
    data[1][1] = 1.0 - 2.0 * ( quat.x * quat.x + quat.z * quat.z );
    data[2][1] =       2.0 * ( quat.y * quat.z - quat.w * quat.x );

	data[0][2] =       2.0 * ( quat.x * quat.z - quat.w * quat.y );
    data[1][2] =       2.0 * ( quat.y * quat.z + quat.w * quat.x );
    data[2][2] = 1.0 - 2.0 * ( quat.x * quat.x + quat.y * quat.y );

    data[3][0] = data[3][1] = data[3][2] = 0.0;
    data[0][3] = data[1][3] = data[2][3] = 0.0;
    data[3][3] = 1.0;
}

void
Matrix::makeIdentity(void)
{
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			data[i][j]=(i==j);	
		}
	}
}	

void
Matrix::makeRotation(const double angle, const char axis)
{
	double sinv = sin( ANGLES_TO_RADIANS( angle ) );
    double cosv = cos( ANGLES_TO_RADIANS( angle ) );

    makeIdentity();

    switch( axis ) {
    case 'x':
        data[1][1] = cosv;
        data[2][1] = -sinv;
        data[1][2] = sinv;
        data[2][2] = cosv;
        break;

    case 'y':
        data[0][0] = cosv;
        data[2][0] = sinv;
        data[0][2] = -sinv;
        data[2][2] = cosv;
        break;

    case 'z': 
        data[0][0] = cosv;
        data[1][0] = -sinv;
        data[0][1] = sinv;
        data[1][1] = cosv;
        break;

    default:
		{
        //code_not_reached();  /* shouldn't get here */
		}
    }
}

void
Matrix::makeTranslation(const double x, const double y, const double z)
{
	makeIdentity();
    data[3][0] = x;
    data[3][1] = y;
    data[3][2] = z;
}

void
Matrix::makeScaling(const double x, const double y, const double z )
{
	makeIdentity();
    data[0][0] = x;
    data[1][1] = y;
    data[2][2] = z;
}

void
Matrix::makeRotationAboutVector(const Vec3d u, const double angle )
{
	Matrix rx,irx, ry, iry;
	
    double a = u.x;
    double b = u.y;
    double c = u.z;
	double d = sqrt( b*b + c*c );

    if ( d < EPS ) {
        if ( a < 0 ) 
            makeRotation( -angle, 'x' );
        else
            makeRotation( angle, 'x' );
        return;
    } 

    rx.makeIdentity();
    irx.makeIdentity();
    ry.makeIdentity();
    iry.makeIdentity();

    rx.data[1][1] = c/d;
    rx.data[2][1] = -b/d;
    rx.data[1][2] = b/d;
    rx.data[2][2] = c/d;

    irx.data[1][1] = c/d;
    irx.data[2][1] = b/d;
    irx.data[1][2] = -b/d;
    irx.data[2][2] = c/d;

    ry.data[0][0] = d;
    ry.data[2][0] = -a;
    ry.data[0][2] = a;
    ry.data[2][2] = d;

    iry.data[0][0] = d;
    iry.data[2][0] = a;
    iry.data[0][2] = -a;
    iry.data[2][2] = d;

    makeRotation( angle, 'z' );

    *this=(*this)*ry;
    *this=(*this)*rx;
    *this=iry*(*this);
    *this=irx*(*this);
}


void
Matrix::transpose(const Matrix& matrix)
{
	for( int i= 0 ; i< 4 ; i++ ){
		for( int j= 0 ; j< 4 ; j++ ){
	    	data[j][i] = matrix.data[i][j];
		}
	}
}

Matrix
Matrix::operator*(const Matrix matrix) const
{
	Matrix ret;
 
    for( int i= 0 ; i< 4 ; i++ )
	for( int j= 0 ; j< 4 ; j++ )
	    ret.data[j][i]=
		data[0][i] * matrix.data[j][0] +
		data[1][i] * matrix.data[j][1] +
		data[2][i] * matrix.data[j][2] +
		data[3][i] * matrix.data[j][3];

   	return ret;
}

Vec3d
Matrix::transformVector(const Vec3d v) const
{   
	return Vec3d(
		v.x * data[0][0] + v.y * data[1][0] + v.z * data[2][0],
    	v.x * data[0][1] + v.y * data[1][1] + v.z * data[2][1],
    	v.x * data[0][2] + v.y * data[1][2] + v.z * data[2][2]
	);
}
	
		
Vec3d
Matrix::transformPoint(const Vec3d p) const
{
	return Vec3d(
    	p.x * data[0][0] + p.y * data[1][0] + p.z * data[2][0] + data[3][0],
    	p.x * data[0][1] + p.y * data[1][1] + p.z * data[2][1] + data[3][1],
    	p.x * data[0][2] + p.y * data[1][2] + p.z * data[2][2] + data[3][2]
    );
}

void
Matrix::makeChangeOfBasisMatrix(Matrix& mat,
				Matrix& invMat,
				const Vec3d w1,
				const Vec3d w2,
				const Vec3d w3)
{
	mat.makeIdentity();
    mat.data[0][0] = w1.x;
    mat.data[0][1] = w1.y;
    mat.data[0][2] = w1.z;
    mat.data[1][0] = w2.x;
    mat.data[1][1] = w2.y;
    mat.data[1][2] = w2.z;
    mat.data[2][0] = w3.x;
    mat.data[2][1] = w3.y;
    mat.data[2][2] = w3.z;

    invMat.makeIdentity();
    invMat.data[0][0] = w1.x;
    invMat.data[1][0] = w1.y;
    invMat.data[2][0] = w1.z;
    invMat.data[0][1] = w2.x;
    invMat.data[1][1] = w2.y;
    invMat.data[2][1] = w2.z;
    invMat.data[0][2] = w3.x;
    invMat.data[1][2] = w3.y;
    invMat.data[2][2] = w3.z;
}

} //namespace pp
