/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2013 Extreme Tux Racer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "common.h"
#include "matrices.h"
#include <algorithm>


template<int ix, int iy>
const TMatrix<ix, iy>& TMatrix<ix, iy>::getIdentity() {
	static bool b = false;
	static TMatrix<ix, iy> mat;
	if (!b) {
		mat.SetIdentity();
		b = true;
	}
	return mat;
}
template const TMatrix<3, 3>& TMatrix<3, 3>::getIdentity();
template const TMatrix<4, 4>& TMatrix<4, 4>::getIdentity();

template<int ix, int iy>
void TMatrix<ix, iy>::SetIdentity() {
	for (int i = 0; i < ix; i++)
		for (int j = 0; j < iy; j++)
			_data[i][j] = (i == j);
}
template void TMatrix<3, 3>::SetIdentity();
template void TMatrix<4, 4>::SetIdentity();



template<int ix, int iy>
void TMatrix<ix, iy>::SetRotationMatrix(double angle, char axis) {
	double sinv, cosv;
	sinv = std::sin(ANGLES_TO_RADIANS(angle));
	cosv = std::cos(ANGLES_TO_RADIANS(angle));

	SetIdentity();

	switch (axis) {
		case 'x':
			_data[1][1] = cosv;
			_data[2][1] = -sinv;
			_data[1][2] = sinv;
			_data[2][2] = cosv;
			break;

		case 'y':
			_data[0][0] = cosv;
			_data[2][0] = sinv;
			_data[0][2] = -sinv;
			_data[2][2] = cosv;
			break;

		case 'z':
			_data[0][0] = cosv;
			_data[1][0] = -sinv;
			_data[0][1] = sinv;
			_data[1][1] = cosv;
			break;
	}
}
template void TMatrix<3, 3>::SetRotationMatrix(double angle, char axis);
template void TMatrix<4, 4>::SetRotationMatrix(double angle, char axis);

template<int ix, int iy>
void TMatrix<ix, iy>::SetTranslationMatrix(double x, double y, double z) {
	SetIdentity();
	_data[3][0] = x;
	_data[3][1] = y;
	_data[3][2] = z;
}
template void TMatrix<4, 4>::SetTranslationMatrix(double x, double y, double z);

template<int ix, int iy>
void TMatrix<ix, iy>::SetScalingMatrix(double x, double y, double z) {
	SetIdentity();
	_data[0][0] = x;
	_data[1][1] = y;
	_data[2][2] = z;
}
template void TMatrix<3, 3>::SetScalingMatrix(double x, double y, double z);
template void TMatrix<4, 4>::SetScalingMatrix(double x, double y, double z);

template<int ix, int iy>
TMatrix<ix, iy>::TMatrix(const TVector3d& w1, const TVector3d& w2, const TVector3d& w3) {
	SetIdentity();
	_data[0][0] = w1.x;
	_data[0][1] = w1.y;
	_data[0][2] = w1.z;
	_data[1][0] = w2.x;
	_data[1][1] = w2.y;
	_data[1][2] = w2.z;
	_data[2][0] = w3.x;
	_data[2][1] = w3.y;
	_data[2][2] = w3.z;
}
template TMatrix<3, 3>::TMatrix(const TVector3d& w1, const TVector3d& w2, const TVector3d& w3);
template TMatrix<4, 4>::TMatrix(const TVector3d& w1, const TVector3d& w2, const TVector3d& w3);

template<int ix, int iy>
TMatrix<ix, iy> TMatrix<ix, iy>::GetTransposed() const {
	TMatrix<ix, iy> r;

	for (int i = 0; i < ix; i++)
		for (int j = 0; j < iy; j++)
			r._data[j][i] = _data[i][j];

	return r;
}
template TMatrix<3, 3> TMatrix<3, 3>::GetTransposed() const;
template TMatrix<4, 4> TMatrix<4, 4>::GetTransposed() const;

template<>
TMatrix<3, 3> operator*<3, 3>(const TMatrix<3, 3>& l, const TMatrix<3, 3>& r) {
	TMatrix<3, 3> ret;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			ret[j][i] = l[0][i] * r[j][0] +
			            l[1][i] * r[j][1] +
			            l[2][i] * r[j][2];

	return ret;
}
template<>
TMatrix<4, 4> operator*<4, 4>(const TMatrix<4, 4>& l, const TMatrix<4, 4>& r) {
	TMatrix<4, 4> ret;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret[j][i] = l[0][i] * r[j][0] +
			            l[1][i] * r[j][1] +
			            l[2][i] * r[j][2] +
			            l[3][i] * r[j][3];

	return ret;
}
