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

static const TVector3 NullVec(0.0, 0.0, 0.0);
static const TVector3 GravVec(0.0, -1.0, 0.0);

// --------------------------------------------------------------------
//			vector and matrix
// --------------------------------------------------------------------

double		VectorLength (const TVector3 &v);

TVector3	ScaleVector (double s, const TVector3& v);
TVector3	AddVectors (const TVector3& v1, const TVector3& v2);
TVector3	SubtractVectors (const TVector3& v1, const TVector3& v2);
double		NormVector (TVector3 &v);

double		DotProduct (const TVector3& v1, const TVector3& v2);
TVector3	CrossProduct (const TVector3& u, const TVector3& v);

TVector3	ProjectToPlane (const TVector3& nml, const TVector3& v);
TVector3	TransformVector (const TMatrix mat, const TVector3& v);
TVector3	TransformNormal (const TVector3& n, const TMatrix mat);	// not used ?
TVector3	TransformPoint (const TMatrix mat, const TVector3& p);
TPlane		MakePlane (double nx, double ny, double nz, double d);
bool		IntersectPlanes (const TPlane& s1, const TPlane& s2, const TPlane& s3, TVector3 *p);
double		DistanceToPlane (const TPlane& plane, const TVector3& pt);

void MakeIdentityMatrix (TMatrix h);
void MakeRotationMatrix (TMatrix mat, double angle, char axis);
void MakeTranslationMatrix (TMatrix mat, double x, double y, double z);
void MakeScalingMatrix (TMatrix mat, double x, double y, double z);

void MultiplyMatrices (TMatrix ret, const TMatrix mat1, const TMatrix mat2);
void TransposeMatrix (const TMatrix mat, TMatrix trans);
void MakeBasisMat (TMatrix mat,	const TVector3& w1, const TVector3& w2, const TVector3& w3);
void MakeBasismatrix_Inv (TMatrix mat, TMatrix invMat, const TVector3& w1, const TVector3& w2, const TVector3& w3);
void RotateAboutVectorMatrix (TMatrix mat, const TVector3& u, double angle);

TQuaternion AddQuaternions (const TQuaternion& q, const TQuaternion& r);		// not used?
TQuaternion MultiplyQuaternions (const TQuaternion& q, const TQuaternion& r);
TQuaternion ScaleQuaternion (double s, const TQuaternion& q);
TQuaternion ConjugateQuaternion (const TQuaternion& q);
void		MakeMatrixFromQuaternion (TMatrix mat, const TQuaternion& q);
TQuaternion MakeQuaternionFromMatrix (const TMatrix mat);
TQuaternion MakeRotationQuaternion (const TVector3& s, const TVector3& t);
TQuaternion InterpolateQuaternions (const TQuaternion& q, TQuaternion r, double t);
TVector3	RotateVector (const TQuaternion& q, const TVector3& v);

bool		IntersectPolygon (const TPolygon& p, TVector3 *v);
bool		IntersectPolyhedron (const TPolyhedron& p);
TVector3	MakeNormal (const TPolygon& p, TVector3 *v);
TPolyhedron	CopyPolyhedron (const TPolyhedron& ph);
void		FreePolyhedron (const TPolyhedron& ph);
void		TransPolyhedron (const TMatrix mat, const TPolyhedron& ph);

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
};

TOdeSolver NewOdeSolver23 ();

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
