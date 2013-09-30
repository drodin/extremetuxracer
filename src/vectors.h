/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef GEOMETRY_TYPES_H
#define GEOMETRY_TYPES_H

#include <cmath>


template<typename T>
struct TVector2 {
	T x, y;
	TVector2(T _x = 0.0, T _y = 0.0)
		: x(_x), y(_y)
	{}
	double Length() const {
		return sqrt(static_cast<double>(x*x + y*y));
	}
	double Norm();
	TVector2<T>& operator*=(T f) {
		x *= f;
		y *= f;
		return *this;
	}
	TVector2<T>& operator+=(const TVector2<T>& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	TVector2<T>& operator-=(const TVector2<T>& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
};

template<typename T>
struct TVector3 : public TVector2<T> {
	T z;
	TVector3(T _x = 0.0, T _y = 0.0, T _z = 0.0)
		: TVector2(_x, _y), z(_z)
	{}
	double Length() const {
		return sqrt(static_cast<double>(x*x + y*y + z*z));
	}
	double Norm();
	TVector3<T>& operator*=(T f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	TVector3<T>& operator+=(const TVector3<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	TVector3<T>& operator-=(const TVector3<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
};

template<typename T>
struct TVector4 : public TVector3<T> {
	T w;
	TVector4(T _x = 0.0, T _y = 0.0, T _z = 0.0, T _w = 0.0)
		: TVector3(_x, _y, _z), w(_w)
	{}
	double Length() const {
		return sqrt(static_cast<double>(x*x + y*y + z*z + w*w));
	}
	double Norm();
	TVector4<T>& operator*=(T f) {
		x *= f;
		y *= f;
		z *= f;
		w *= f;
		return *this;
	}
	TVector4<T>& operator+=(const TVector4<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}
	TVector4<T>& operator-=(const TVector4<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}
};

typedef TVector4<double> TVector4d;
typedef TVector3<double> TVector3d;
typedef TVector2<double> TVector2d;
typedef TVector4<int> TVector4i;
typedef TVector3<int> TVector3i;
typedef TVector2<int> TVector2i;
typedef TVector4d TQuaternion;

template<typename T>
TVector2<T> operator*(T f, const TVector2<T>& v) {
	return TVector2<typename T>(v.x*f, v.y*f);
}
template<typename T>
TVector3<T> operator*(T f, const TVector3<T>& v) {
	return TVector3<typename T>(v.x*f, v.y*f, v.z*f);
}
template<typename T>
TVector4<T> operator*(T f, const TVector4<T>& v) {
	return TVector4<typename T>(v.x*f, v.y*f, v.z*f, v.w*f);
}

template<typename T>
TVector2<T> operator+(const TVector2<T>& l, const TVector2<T>& r) {
	return TVector2<typename T>(l.x + r.x, l.y + r.y);
}
template<typename T>
TVector3<T> operator+(const TVector3<T>& l, const TVector3<T>& r) {
	return TVector3<typename T>(l.x + r.x, l.y + r.y, l.z + r.z);
}
template<typename T>
TVector4<T> operator+(const TVector4<T>& l, const TVector4<T>& r) {
	return TVector4<typename T>(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
}

template<typename T>
TVector2<T> operator-(const TVector2<T>& l, const TVector2<T>& r) {
	return TVector2<typename T>(l.x - r.x, l.y - r.y);
}
template<typename T>
TVector3<T> operator-(const TVector3<T>& l, const TVector3<T>& r) {
	return TVector3<typename T>(l.x - r.x, l.y - r.y, l.z - r.z);
}
template<typename T>
TVector4<T> operator-(const TVector4<T>& l, const TVector4<T>& r) {
	return TVector4<typename T>(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
}

double    DotProduct(const TVector3d& v1, const TVector3d& v2);
TVector3d CrossProduct(const TVector3d& u, const TVector3d& v);


#endif
