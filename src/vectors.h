/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef VECTORS_H
#define VECTORS_H

#include <cmath>


template<typename T>
struct TVector2 {
	T x, y;
	constexpr explicit TVector2(T _x = (T)0, T _y = (T)0)
		: x(_x), y(_y)
	{}
	constexpr double Length() const {
		return std::hypot(x, y);
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
struct TVector3 {
	T x, y, z;
	constexpr explicit TVector3(T _x = (T)0, T _y = (T)0, T _z = (T)0)
		: x(_x), y(_y), z(_z)
	{}
	constexpr double Length() const {
		return std::sqrt(static_cast<double>(x*x + y*y + z*z));
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
struct TVector4 {
	T x, y, z, w;
	constexpr explicit TVector4(T _x = (T)0, T _y = (T)0, T _z = (T)0, T _w = (T)0)
		: x(_x), y(_y), z(_z), w(_w)
	{}
	constexpr double Length() const {
		return std::sqrt(static_cast<double>(x*x + y*y + z*z + w*w));
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
constexpr TVector2<T> operator*(T f, const TVector2<T>& v) {
	return TVector2<T>(v.x*f, v.y*f);
}
template<typename T>
constexpr TVector3<T> operator*(T f, const TVector3<T>& v) {
	return TVector3<T>(v.x*f, v.y*f, v.z*f);
}
template<typename T>
constexpr TVector4<T> operator*(T f, const TVector4<T>& v) {
	return TVector4<T>(v.x*f, v.y*f, v.z*f, v.w*f);
}

template<typename T>
constexpr TVector2<T> operator+(const TVector2<T>& l, const TVector2<T>& r) {
	return TVector2<T>(l.x + r.x, l.y + r.y);
}
template<typename T>
constexpr TVector3<T> operator+(const TVector3<T>& l, const TVector3<T>& r) {
	return TVector3<T>(l.x + r.x, l.y + r.y, l.z + r.z);
}
template<typename T>
constexpr TVector4<T> operator+(const TVector4<T>& l, const TVector4<T>& r) {
	return TVector4<T>(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
}

template<typename T>
constexpr TVector2<T> operator-(const TVector2<T>& l, const TVector2<T>& r) {
	return TVector2<T>(l.x - r.x, l.y - r.y);
}
template<typename T>
constexpr TVector3<T> operator-(const TVector3<T>& l, const TVector3<T>& r) {
	return TVector3<T>(l.x - r.x, l.y - r.y, l.z - r.z);
}
template<typename T>
constexpr TVector4<T> operator-(const TVector4<T>& l, const TVector4<T>& r) {
	return TVector4<T>(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
}

template<typename T>
constexpr TVector2<T> operator-(const TVector2<T>&r) {
	return TVector2<T>(-r.x, -r.y);
}
template<typename T>
constexpr TVector3<T> operator-(const TVector3<T>& r) {
	return TVector3<T>(-r.x, -r.y, -r.z);
}
template<typename T>
constexpr TVector4<T> operator-(const TVector4<T>& r) {
	return TVector4<T>(-r.x, -r.y, -r.z, -r.w);
}

template<typename T>
constexpr double DotProduct(const TVector3<T>& v1, const TVector3<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
template<typename T>
constexpr double DotProduct(const TVector4<T>& v1, const TVector4<T>& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

TVector3d CrossProduct(const TVector3d& u, const TVector3d& v);


extern const TVector2d NullVec2;
extern const TVector3d NullVec3;
extern const TVector4d NullVec4;
extern const TVector2i NullVec2i;
extern const TVector3i NullVec3i;
extern const TVector4i NullVec4i;


#endif
