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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "mathlib.h"
#include <cstdlib>
#include <algorithm>


TVector3d ProjectToPlane (const TVector3d& nml, const TVector3d& v) {
	double dotProd = DotProduct (nml, v);
	TVector3d nmlComp = dotProd * nml;

	return v - nmlComp;
}


TVector3d TransformVector(const TMatrix mat, const TVector3d& v) {
	TVector3d r;
	r.x = v.x * mat[0][0] + v.y * mat[1][0] + v.z * mat[2][0];
	r.y = v.x * mat[0][1] + v.y * mat[1][1] + v.z * mat[2][1];
	r.z = v.x * mat[0][2] + v.y * mat[1][2] + v.z * mat[2][2];
	return r;
}

TVector3d TransformNormal(const TVector3d& n, const TMatrix mat) {
	TVector3d r;
	r.x = n.x * mat[0][0] + n.y * mat[0][1] + n.z * mat[0][2];
	r.y = n.x * mat[1][0] + n.y * mat[1][1] + n.z * mat[1][2];
	r.z = n.x * mat[2][0] + n.y * mat[2][1] + n.z * mat[2][2];
	return r;
}

TVector3d TransformPoint(const TMatrix mat, const TVector3d& p) {
	TVector3d r;
	r.x = p.x * mat[0][0] + p.y * mat[1][0] + p.z * mat[2][0];
	r.y = p.x * mat[0][1] + p.y * mat[1][1] + p.z * mat[2][1];
	r.z = p.x * mat[0][2] + p.y * mat[1][2] + p.z * mat[2][2];
	r.x += mat[3][0];
	r.y += mat[3][1];
	r.z += mat[3][2];
	return r;
}

bool IntersectPlanes (const TPlane& s1, const TPlane& s2, const TPlane& s3, TVector3d *p) {
	double A[3][4];
	double x[3];
	double retval;

	A[0][0] =  s1.nml.x;
	A[0][1] =  s1.nml.y;
	A[0][2] =  s1.nml.z;
	A[0][3] = -s1.d;

	A[1][0] =  s2.nml.x;
	A[1][1] =  s2.nml.y;
	A[1][2] =  s2.nml.z;
	A[1][3] = -s2.d;

	A[2][0] =  s3.nml.x;
	A[2][1] =  s3.nml.y;
	A[2][2] =  s3.nml.z;
	A[2][3] = -s3.d;

	retval = Gauss ((double*) A, 3, x);

	if (retval != 0) {
		return false;
	} else {
		p->x = x[0];
		p->y = x[1];
		p->z = x[2];
		return true;
	}
}

double DistanceToPlane (const TPlane& plane, const TVector3d& pt) {
	return
	    DotProduct(plane.nml, pt) +
	    plane.d;
}

void MakeIdentityMatrix(TMatrix h) {
	for (int i= 0; i< 4; i++)
		for (int j= 0; j< 4; j++)
			h[i][j]= (i==j);
}


void MultiplyMatrices (TMatrix ret, const TMatrix mat1, const TMatrix mat2) {
	TMatrix r;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			r[j][i] = mat1[0][i] * mat2[j][0] +
			          mat1[1][i] * mat2[j][1] +
			          mat1[2][i] * mat2[j][2] +
			          mat1[3][i] * mat2[j][3];

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret[i][j] = r[i][j];
}


void TransposeMatrix (const TMatrix mat, TMatrix trans) {
	TMatrix r;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j< 4; j++)
			r[j][i] = mat[i][j];

	for (int i = 0; i< 4; i++)
		for (int j = 0; j< 4; j++)
			trans[i][j] = r[i][j];
}

void MakeRotationMatrix (TMatrix mat, double angle, char axis) {
	double sinv, cosv;
	sinv = sin (ANGLES_TO_RADIANS (angle));
	cosv = cos (ANGLES_TO_RADIANS (angle));

	MakeIdentityMatrix (mat);

	switch (axis) {
		case 'x':
			mat[1][1] = cosv;
			mat[2][1] = -sinv;
			mat[1][2] = sinv;
			mat[2][2] = cosv;
			break;

		case 'y':
			mat[0][0] = cosv;
			mat[2][0] = sinv;
			mat[0][2] = -sinv;
			mat[2][2] = cosv;
			break;

		case 'z':
			mat[0][0] = cosv;
			mat[1][0] = -sinv;
			mat[0][1] = sinv;
			mat[1][1] = cosv;
			break;
	}
}

void MakeTranslationMatrix (TMatrix mat, double x, double y, double z) {
	MakeIdentityMatrix (mat);
	mat[3][0] = x;
	mat[3][1] = y;
	mat[3][2] = z;
}

void MakeScalingMatrix (TMatrix mat, double x, double y, double z) {
	MakeIdentityMatrix (mat);
	mat[0][0] = x;
	mat[1][1] = y;
	mat[2][2] = z;
}

void MakeBasisMat (TMatrix mat, const TVector3d& w1, const TVector3d& w2, const TVector3d& w3) {
	MakeIdentityMatrix (mat);
	mat[0][0] = w1.x;
	mat[0][1] = w1.y;
	mat[0][2] = w1.z;
	mat[1][0] = w2.x;
	mat[1][1] = w2.y;
	mat[1][2] = w2.z;
	mat[2][0] = w3.x;
	mat[2][1] = w3.y;
	mat[2][2] = w3.z;
}

void MakeBasismatrix_Inv (TMatrix mat, TMatrix invMat,
                          const TVector3d& w1, const TVector3d& w2, const TVector3d& w3) {
	MakeIdentityMatrix (mat);
	mat[0][0] = w1.x;
	mat[0][1] = w1.y;
	mat[0][2] = w1.z;
	mat[1][0] = w2.x;
	mat[1][1] = w2.y;
	mat[1][2] = w2.z;
	mat[2][0] = w3.x;
	mat[2][1] = w3.y;
	mat[2][2] = w3.z;

	MakeIdentityMatrix (invMat);
	invMat[0][0] = w1.x;
	invMat[1][0] = w1.y;
	invMat[2][0] = w1.z;
	invMat[0][1] = w2.x;
	invMat[1][1] = w2.y;
	invMat[2][1] = w2.z;
	invMat[0][2] = w3.x;
	invMat[1][2] = w3.y;
	invMat[2][2] = w3.z;
}

void RotateAboutVectorMatrix (TMatrix mat, const TVector3d& u, double angle) {
	TMatrix rx, irx, ry, iry;

	double a = u.x;
	double b = u.y;
	double c = u.z;

	double d = sqrt (b*b + c*c);

	if (d < EPS) {
		if (a < 0)
			MakeRotationMatrix (mat, -angle, 'x');
		else
			MakeRotationMatrix (mat, angle, 'x');
		return;
	}

	MakeIdentityMatrix (rx);
	MakeIdentityMatrix (irx);
	MakeIdentityMatrix (ry);
	MakeIdentityMatrix (iry);

	rx[1][1] = c/d;
	rx[2][1] = -b/d;
	rx[1][2] = b/d;
	rx[2][2] = c/d;

	irx[1][1] = c/d;
	irx[2][1] = b/d;
	irx[1][2] = -b/d;
	irx[2][2] = c/d;

	ry[0][0] = d;
	ry[2][0] = -a;
	ry[0][2] = a;
	ry[2][2] = d;

	iry[0][0] = d;
	iry[2][0] = a;
	iry[0][2] = -a;
	iry[2][2] = d;

	MakeRotationMatrix (mat, angle, 'z');

	MultiplyMatrices (mat, mat, ry);
	MultiplyMatrices (mat, mat, rx);
	MultiplyMatrices (mat, iry, mat);
	MultiplyMatrices (mat, irx, mat);
}

TQuaternion MultiplyQuaternions (const TQuaternion& q, const TQuaternion& r) {
	TQuaternion res;
	res.x = q.y * r.z - q.z * r.y + r.w * q.x + q.w * r.x;
	res.y = q.z * r.x - q.x * r.z + r.w * q.y + q.w * r.y;
	res.z = q.x * r.y - q.y * r.x + r.w * q.z + q.w * r.z;
	res.w = q.w * r.w - q.x * r.x - q.y * r.y - q.z * r.z;
	return res;
}

TQuaternion ConjugateQuaternion (const TQuaternion& q) {
	TQuaternion res(
	    -1 * q.x,
	    -1 * q.y,
	    -1 * q.z,
	    q.w);

	return res;
}

void MakeMatrixFromQuaternion (TMatrix mat, const TQuaternion& q) {
	mat[0][0] = 1.0 - 2.0 *  (q.y * q.y + q.z * q.z);
	mat[1][0] =       2.0 *  (q.x * q.y - q.w * q.z);
	mat[2][0] =       2.0 *  (q.x * q.z + q.w * q.y);

	mat[0][1] =       2.0 *  (q.x * q.y + q.w * q.z);
	mat[1][1] = 1.0 - 2.0 *  (q.x * q.x + q.z * q.z);
	mat[2][1] =       2.0 *  (q.y * q.z - q.w * q.x);

	mat[0][2] =       2.0 *  (q.x * q.z - q.w * q.y);
	mat[1][2] =       2.0 *  (q.y * q.z + q.w * q.x);
	mat[2][2] = 1.0 - 2.0 *  (q.x * q.x + q.y * q.y);

	mat[3][0] = mat[3][1] = mat[3][2] = 0.0;
	mat[0][3] = mat[1][3] = mat[2][3] = 0.0;
	mat[3][3] = 1.0;
}

TQuaternion MakeQuaternionFromMatrix (const TMatrix m) {
	TQuaternion res;
	double  tr, s, q[4];

	static int nxt[3] = {1, 2, 0};

	tr = m[0][0] + m[1][1] + m[2][2];

	if (tr > 0.0) {
		s = sqrt (tr + 1.0);
		res.w = 0.5 * s;
		s = 0.5 / s;
		res.x = (m[1][2] - m[2][1]) * s;
		res.y = (m[2][0] - m[0][2]) * s;
		res.z = (m[0][1] - m[1][0]) * s;
	} else {
		int i = 0;
		if (m[1][1] > m[0][0]) i = 1;
		if (m[2][2] > m[i][i]) i = 2;
		int j = nxt[i];
		int k = nxt[j];

		s = sqrt (m[i][i] - m[j][j] - m[k][k] + 1.0);

		q[i] = s * 0.5;

		if (s != 0.0) s = 0.5 / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;

		res.x = q[0];
		res.y = q[1];
		res.z = q[2];
		res.w = q[3];
	}

	return res;
}

TQuaternion MakeRotationQuaternion (const TVector3d& s, const TVector3d& t) {
	TVector3d u = CrossProduct (s, t);
	double sin2phi = u.Norm();

	if (sin2phi < EPS) {
		return TQuaternion (0., 0., 0., 1.);
	} else {
		double cos2phi = DotProduct (s, t);

		double sinphi = sqrt ( (1 - cos2phi) / 2.0);
		double cosphi = sqrt ( (1 + cos2phi) / 2.0);

		return TQuaternion(
		           sinphi * u.x,
		           sinphi * u.y,
		           sinphi * u.z,
		           cosphi);
	}
}

TQuaternion InterpolateQuaternions (const TQuaternion& q, TQuaternion r, double t) {
	double cosphi = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;

	if (cosphi < 0.0) {
		cosphi = -cosphi;
		r.x = -r.x;
		r.y = -r.y;
		r.z = -r.z;
		r.w = -r.w;
	}

	double scale0, scale1;
	if (1.0 - cosphi > EPS) {
		double phi = acos (cosphi);
		double sinphi = sin (phi);
		scale0 = sin (phi *  (1.0 - t)) / sinphi;
		scale1 = sin (phi * t) / sinphi;
	} else {
		scale0 = 1.0 - t;
		scale1 = t;
	}

	return scale0 * q + scale1 * r;
}

TVector3d RotateVector (const TQuaternion& q, const TVector3d& v) {
	TQuaternion p(v.x, v.y, v.z, 1.0);

	TQuaternion qs(-q.x, -q.y, -q.z, q.w);

	TQuaternion res_q = MultiplyQuaternions (q, MultiplyQuaternions (p, qs));

	return res_q;
}

// --------------------------------------------------------------------
//				 Gauss
// --------------------------------------------------------------------

bool order (double *matrix, int n, int pivot);
void elim (double *matrix, int n, int pivot);
void backsb (double *matrix, int n, double *soln);

int Gauss(double *matrix, int n, double *soln) {
	int pivot = 0;
	bool error = false;

	while ((pivot<(n-1)) && (!error)) {
		if (!(error = order(matrix,n,pivot))) {
			elim(matrix,n,pivot);
			pivot++;
		}
	}
	if (error) {
		return 1;
	} else {
		backsb(matrix, n, soln);
	}
	return 0;
}

bool order (double *matrix, int n, int pivot) {
	bool error = false;

	int rmax = pivot;

	for (int row=pivot+1; row<n; row++) {
		if (fabs(*(matrix+row*(n+1)+pivot)) > fabs(*(matrix+rmax*(n+1)+pivot)))
			rmax = row;
	}

	if (fabs(*(matrix+rmax*(n+1)+pivot)) < EPS)
		error = true;
	else if (rmax != pivot) {
		for (int k=0; k<(n+1); k++) {
			double temp = *(matrix+rmax*(n+1)+k);
			*(matrix+rmax*(n+1)+k) = *(matrix+pivot*(n+1)+k);
			*(matrix+pivot*(n+1)+k) = temp;
		}
	}
	return error;
}

void elim (double *matrix, int n, int pivot) {
	for (int row = pivot+1; row < n; row++) {
		double factor = (*(matrix+row*(n+1)+pivot))/(*(matrix+pivot*(n+1)+pivot));
		*(matrix+row*(n+1)+pivot)=0.0;
		for (int col=pivot+1l; col<n+1; col++) {
			*(matrix+row*(n+1)+col) = *(matrix+row*(n+1)+col) -
			                          (*(matrix+pivot*(n+1)+col))*factor;
		}
	}
}


void backsb (double *matrix, int n, double *soln) {
	for (int row = n-1; row >=0; row--) {
		for (int col = n-1; col >= row+1; col--) {
			*(matrix+row*(n+1)+(n)) = *(matrix+row*(n+1)+n) -
			                          (*(soln+col))*(*(matrix+row*(n+1)+col));
		}
		*(soln+row) = (*(matrix+row*(n+1)+n))/(*(matrix+row*(n+1)+row));
	}
}

// ***************************************************************************
// ***************************************************************************

bool IntersectPolygon (const TPolygon& p, TVector3d *v) {
	TRay ray;
	double d, s, nuDotProd;
	double distsq;

	TVector3d nml = MakeNormal (p, v);
	ray.pt = TVector3d(0., 0., 0.);
	ray.vec = nml;

	nuDotProd = DotProduct (nml, ray.vec);
	if (fabs(nuDotProd) < EPS)
		return false;

	d = - DotProduct(nml, v[p.vertices[0]]);

	if (fabs (d) > 1) return false;

	for (int i=0; i < p.num_vertices; i++) {
		TVector3d *v0, *v1;

		v0 = &v[p.vertices[i]];
		v1 = &v[p.vertices[ (i+1) % p.num_vertices ]];

		TVector3d edge_vec = *v1 - *v0;
		double edge_len = edge_vec.Norm();

		double t = - DotProduct (*v0, edge_vec);

		if (t < 0) {
			distsq = MAG_SQD (*v0);
		} else if (t > edge_len) {
			distsq = MAG_SQD (*v1);
		} else {
			*v0 += t * edge_vec;
			distsq = MAG_SQD (*v0);
		}

		if (distsq <= 1) return true;
	}

	s = - (d + DotProduct (nml, ray.pt)) / nuDotProd;
	TVector3d pt = ray.pt + s * ray.vec;

	for (int i=0; i < p.num_vertices; i++) {
		TVector3d edge_nml = CrossProduct (nml,
		                                   v[p.vertices[ (i+1) % p.num_vertices ]] - v[p.vertices[i]]);

		double wec = DotProduct (pt - v[p.vertices[i]], edge_nml);
		if (wec < 0) return false;
	}
	return true;
}

bool IntersectPolyhedron (const TPolyhedron& p) {
	bool hit = false;
	for (size_t i=0; i<p.num_polygons; i++) {
		hit = IntersectPolygon (p.polygons[i], p.vertices);
		if (hit == true) break;
	}
	return hit;
}

TVector3d MakeNormal (const TPolygon& p, TVector3d *v) {
	TVector3d v1 = v[p.vertices[1]] - v[p.vertices[0]];
	TVector3d v2 = v[p.vertices[p.num_vertices-1]] - v[p.vertices[0]];
	TVector3d normal = CrossProduct (v1, v2);

	normal.Norm();
	return normal;
}


TPolyhedron CopyPolyhedron (const TPolyhedron& ph) {
	TPolyhedron newph = ph;
	newph.vertices = new TVector3d[ph.num_vertices];
	copy_n(ph.vertices, ph.num_vertices, newph.vertices);
	return newph;
}

void FreePolyhedron (const TPolyhedron& ph) {
	delete[] ph.vertices;
}

void TransPolyhedron (const TMatrix mat, const TPolyhedron& ph) {
	for (size_t i=0; i<ph.num_vertices; i++)
		ph.vertices[i] = TransformPoint (mat, ph.vertices[i]);
}

// --------------------------------------------------------------------
//					ode solver
// --------------------------------------------------------------------

const double ode23_time_step_mat[] = { 0., 1./2., 3./4., 1. };
const double ode23_coeff_mat[][4] = {
	{0.0, 1./2.,   0.0,  2./9.},
	{0.0,   0.0, 3./4.,  1./3.},
	{0.0,   0.0,   0.0,  4./9.},
	{0.0,   0.0,   0.0,    0.0}
};

const double ode23_error_mat[] = {-5./72., 1./12., 1./9., -1./8. };
const double ode23_time_step_exp = 1./3.;

int ode23_NumEstimates() {return 4; }

void ode23_InitOdeData (TOdeData *data, double init_val, double h) {
	data->init_val = init_val;
	data->h = h;
}

double ode23_NextTime(TOdeData *data, int step) {
	return ode23_time_step_mat[step] * data->h;
}

double ode23_NextValue (TOdeData *data, int step) {
	double val = data->init_val;

	for (int i=0; i<step; i++)
		val += ode23_coeff_mat[i][step] * data->k[i];
	return val;
}

void ode23_UpdateEstimate(TOdeData *data, int step, double val) {
	data->k[step] = data->h * val;
}

double ode23_FinalEstimate(TOdeData *data) {
	double val = data->init_val;

	for (int i=0; i<3; i++)
		val += ode23_coeff_mat[i][3] * data->k[i];
	return val;
}

double ode23_EstimateError(TOdeData *data) {
	double err=0.;

	for (int i=0; i<4; i++)
		err += ode23_error_mat[i] * data->k[i];
	return fabs(err);
}

double ode23_TimestepExponent() {
	return ode23_time_step_exp;
}

TOdeSolver::TOdeSolver() {
	NumEstimates = ode23_NumEstimates;
	InitOdeData = ode23_InitOdeData;
	NextTime = ode23_NextTime;
	NextValue = ode23_NextValue;
	UpdateEstimate = ode23_UpdateEstimate;
	FinalEstimate = ode23_FinalEstimate;
	EstimateError = ode23_EstimateError;
	TimestepExponent = ode23_TimestepExponent;
}

double LinearInterp (const double x[], const double y[], double val, int n) {
	int i;
	double m, b;

	if (val < x[0]) i = 0;
	else if (val >= x[n-1]) i = n-2;
	else for (i=0; i<n-1; i++) if (val < x[i+1]) break;

	m = (y[i+1] - y[i]) / (x[i+1] - x[i]);
	b = y[i] - m * x[i];

	return m * val + b;
}

double XRandom (float min, float max) {
	return (double)rand () / RAND_MAX * (max - min) + min;
}

double FRandom () {
	return (double)rand () / RAND_MAX;
}

int IRandom (int min, int max) {
	return min + rand()%(max-min+1);
}

int ITrunc (int val, int base) {
	return (int)(val / base);
}

int IFrac (int val, int base) {
	return val - ITrunc (val, base) * base;
}
