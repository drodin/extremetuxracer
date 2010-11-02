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

const TVector3 NullVec = {0.0, 0.0, 0.0};
const TVector3 GravVec = {0.0, -1.0, 0.0};

// --------------------------------------------------------------------
//			vector and matrix
// --------------------------------------------------------------------

TVector3	MakeVector (double x, double y, double z);
TVector2	MakeVector2 (double x, double y);
TVector3	MakeVector3 (double x, double y, double z);
TVector4	MakeVector4 (float x, float y, float z, float w);
TIndex2		MakeIndex2 (int i, int j);
TIndex3		MakeIndex3 (int i, int j, int k);

TVector3	ScaleVector (double s, TVector3 v);
TVector3	AddVectors (TVector3 v1, TVector3 v2);
TVector3	SubtractVectors (TVector3 v1, TVector3 v2);
double		NormVector (TVector3 *v);
double		NormVectorN (TVector3 &v);	// new version with reference

double		DotProduct (TVector3 v1, TVector3 v2);
TVector3	CrossProduct (TVector3 u, TVector3 v);

TVector3	ProjectToPlane (TVector3 nml, TVector3 v);
TVector3	TransformVector (TMatrix mat, TVector3 v);
TVector3	TransformNormal (TVector3 n, TMatrix mat);	// not used ?
TVector3	TransformPoint (TMatrix mat, TVector3 p);
TPlane		MakePlane (double nx, double ny, double nz, double d);
bool		IntersectPlanes (TPlane s1, TPlane s2, TPlane s3, TVector3 *p);
double		DistanceToPlane (TPlane plane, TVector3 pt);

void MakeIdentityMatrix (TMatrix h);
void MakeRotationMatrix (TMatrix mat, double angle, char axis);
void MakeTranslationMatrix (TMatrix mat, double x, double y, double z);
void MakeScalingMatrix (TMatrix mat, double x, double y, double z);

void MultiplyMatrices (TMatrix ret, TMatrix mat1, TMatrix mat2);
void TransposeMatrix (TMatrix mat, TMatrix trans);
void MakeBasisMat (TMatrix mat,	TVector3 w1, TVector3 w2, TVector3 w3);
void MakeBasismatrix_Inv (TMatrix mat, TMatrix invMat, TVector3 w1, TVector3 w2, TVector3 w3);
void RotateAboutVectorMatrix (TMatrix mat, TVector3 u, double angle);

TQuaternion MakeQuaternion (double x, double y, double z, double w);
TQuaternion AddQuaternions (TQuaternion q, TQuaternion r);		// not used?
TQuaternion MultiplyQuaternions (TQuaternion q, TQuaternion r);	// not used?
TQuaternion ScaleQuaternion (double s, TQuaternion q);
TQuaternion ConjugateQuaternion (TQuaternion q);
void 		MakeMatrixFromQuaternion (TMatrix mat, TQuaternion q);
TQuaternion MakeQuaternionFromMatrix (TMatrix mat);
TQuaternion MakeRotationQuaternion (TVector3 s, TVector3 t);
TQuaternion InterpolateQuaternions (TQuaternion q, TQuaternion r, double t);
TVector3	RotateVector (TQuaternion q, TVector3 v);

bool		IntersectPolygon (TPolygon p, TVector3 *v);
bool		IntersectPolyhedron (TPolyhedron p);
TVector3	MakeNormal (TPolygon p, TVector3 *v);
TPolyhedron	CopyPolyhedron (TPolyhedron ph);
void		FreePolyhedron (TPolyhedron ph) ;
void		TransPolyhedron (TMatrix mat, TPolyhedron ph);

// --------------------------------------------------------------------
//				ode solver
// --------------------------------------------------------------------

typedef char		TOdeData;
typedef TOdeData*	(*PNewOdeData) ();
typedef int 		(*PNumEstimates) ();
typedef void 		(*PInitOdeData) (TOdeData *, double init_val, double h);
typedef double 		(*PNextTime) (TOdeData *, int step);
typedef double 		(*PNextValue) (TOdeData *, int step);
typedef void 		(*PUpdateEstimate) (TOdeData *, int step, double val);
typedef double 		(*PFinalEstimate) (TOdeData *);
typedef double 		(*PEstimateError) (TOdeData *);
typedef double 		(*PTimestepExponent) ();

typedef struct {
    PNewOdeData			NewOdeData;
    PNumEstimates		NumEstimates;
    PInitOdeData		InitOdeData;
    PNextTime			NextTime;
    PNextValue			NextValue;
    PUpdateEstimate		UpdateEstimate;
    PFinalEstimate		FinalEstimate;
    PEstimateError		EstimateError;
    PTimestepExponent	TimestepExponent;
} TOdeSolver;

typedef struct {
    double k[4];
    double init_val;
    double h;
} TOdeData23;

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
