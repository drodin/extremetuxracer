/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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

#ifndef MATHLIB_H
#define MATHLIB_H

#include "bh.h"
#include "matrices.h"

static const TVector3d GravVec(0.0, -1.0, 0.0);

// --------------------------------------------------------------------
//			vector and matrix
// --------------------------------------------------------------------

TVector3d	ProjectToPlane(const TVector3d& nml, const TVector3d& v);
TVector3d	TransformVector(const TMatrix<4, 4>& mat, const TVector3d& v);
TVector3d	TransformNormal(const TVector3d& n, const TMatrix<4, 4>& mat);	// not used ?
TVector3d	TransformPoint(const TMatrix<4, 4>& mat, const TVector3d& p);
bool		IntersectPlanes (const TPlane& s1, const TPlane& s2, const TPlane& s3, TVector3d *p);
double		DistanceToPlane (const TPlane& plane, const TVector3d& pt);

TMatrix<4, 4> RotateAboutVectorMatrix(const TVector3d& u, double angle);

TQuaternion MultiplyQuaternions (const TQuaternion& q, const TQuaternion& r);
TQuaternion ConjugateQuaternion (const TQuaternion& q);
TMatrix<4, 4> MakeMatrixFromQuaternion(const TQuaternion& q);
TQuaternion MakeQuaternionFromMatrix(const TMatrix<4, 4>& mat);
TQuaternion MakeRotationQuaternion (const TVector3d& s, const TVector3d& t);
TQuaternion InterpolateQuaternions (const TQuaternion& q, TQuaternion r, double t);
TVector3d	RotateVector (const TQuaternion& q, const TVector3d& v);

bool		IntersectPolygon (const TPolygon& p, TVector3d *v);
bool		IntersectPolyhedron (const TPolyhedron& p);
TVector3d	MakeNormal (const TPolygon& p, TVector3d *v);
TPolyhedron	CopyPolyhedron (const TPolyhedron& ph);
void		FreePolyhedron (const TPolyhedron& ph);
void		TransPolyhedron(const TMatrix<4, 4>& mat, const TPolyhedron& ph);

// --------------------------------------------------------------------
//				ode solver
// --------------------------------------------------------------------

struct TOdeData {
	double k[4];
	double init_val;
	double h;
};

typedef int			(*PNumEstimates) ();
typedef void		(*PInitOdeData) (TOdeData *, double init_val, double h);
typedef double		(*PNextTime) (TOdeData *, int step);
typedef double		(*PNextValue) (TOdeData *, int step);
typedef void		(*PUpdateEstimate) (TOdeData *, int step, double val);
typedef double		(*PFinalEstimate) (TOdeData *);
typedef double		(*PEstimateError) (TOdeData *);
typedef double		(*PTimestepExponent) ();

struct TOdeSolver {
	PNumEstimates		NumEstimates;
	PInitOdeData		InitOdeData;
	PNextTime			NextTime;
	PNextValue			NextValue;
	PUpdateEstimate		UpdateEstimate;
	PFinalEstimate		FinalEstimate;
	PEstimateError		EstimateError;
	PTimestepExponent	TimestepExponent;
	TOdeSolver();
};

// --------------------------------------------------------------------
//			special
// --------------------------------------------------------------------

int Gauss (double *matrix, int n, double *soln);
double LinearInterp (const double x[], const double y[], double val, int n);

double	XRandom (float min, float max);
double	FRandom ();
int		IRandom (int min, int max);
int		ITrunc (int val, int base);
int		IFrac (int val, int base);

#endif
