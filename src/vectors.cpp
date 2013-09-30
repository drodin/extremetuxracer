/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010-2013 Extreme Tuxracer Team

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

#include "vectors.h"


// Instanciate only functions we actually need
template<>
double TVector3<double>::Norm() {
	double square = x*x + y*y + z*z;
	if (square == 0.0) return 0.0;
	double denom = sqrt(square);
	*this *= 1.0 / denom;
	return denom;
}

double DotProduct(const TVector3d& v1, const TVector3d& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
TVector3d CrossProduct(const TVector3d& u, const TVector3d& v) {
	return TVector3d(
	           u.y * v.z - u.z * v.y,
	           u.z * v.x - u.x * v.z,
	           u.x * v.y - u.y * v.x);
}