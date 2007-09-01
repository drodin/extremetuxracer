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
 
#ifndef _MATRIX_H
#define _MATRIX_H


#include "vec3d.h"
#include "quat.h"

namespace pp {

///
class Quat;

///
class Matrix
{
public:
	double data[4][4];
	Matrix(void){};
	Matrix(const Quat quat);	

	Matrix operator*(const Matrix matrix) const;
		
	void makeIdentity(void);
	void makeRotation(const double angle, const char axis);
	void makeTranslation(const double x, const double y, const double z);
	void makeScaling(const double x, const double y, const double z );
	void makeRotationAboutVector(const Vec3d u, const double angle );

	void transpose(const Matrix& mat);
		
	Vec3d  transformVector(const Vec3d v) const;
	Vec3d  transformPoint(const Vec3d p) const;
		
	static void makeChangeOfBasisMatrix(Matrix& mat,
				Matrix& invMat,
				const Vec3d w1,
				const Vec3d w2,
				const Vec3d w3);	
};


} //namespace pp

#endif // MATRIX_H
