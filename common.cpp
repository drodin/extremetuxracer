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

#include "common.h"

// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

const TColor colWhite =		{1.0, 1.0, 1.0, 1.0};
const TColor colDYell =		{1.0, 0.8, 0.0, 1.0};
const TColor colYellow =	{1.0, 1.0, 0.0, 1.0};
const TColor colLYell = 	{1.0, 1.0, 0.4, 1.0};
const TColor colOrange =	{1.0, 0.5, 0.0, 1.0};
const TColor colRed   =		{1.0, 0.0, 0.0, 1.0};
const TColor colGrey  =		{0.5, 0.5, 0.5, 1.0};
const TColor colLGrey  =	{0.7, 0.7, 0.7, 1.0};
const TColor colDGrey  =	{0.3, 0.3, 0.3, 1.0};
const TColor colBlack =		{0.0, 0.0, 0.0, 1.0};	
const TColor colBlue =		{0.0, 0.0, 1.0, 1.0};	
const TColor colLBlue =		{0.5, 0.7, 0.9, 1.0};	
const TColor colDBlue =		{0.0, 0.0, 0.6, 1.0};	
const TColor colLBackgr =	{0.5, 0.7, 0.9, 1.0};
const TColor colBackgr =	{0.4, 0.6, 0.8, 1.0};
const TColor colMBackgr =	{0.35, 0.5, 0.7, 1.0};
const TColor colDBackgr =	{0.2, 0.3, 0.6, 1.0};
const TColor colMess =		{0.3, 0.3, 0.7, 1.0};
const TColor colSky =		{0.82, 0.86, 0.88, 1.0};

TColor MakeColor (double r, double g, double b, double a) {
	TColor res;
	res.r = r;
	res.g = g;
	res.b = b;
	res.a = a;
	return res;
}

TColor3 MakeColor3 (double r, double g, double b) {
	TColor3 res;
	res.r = r;
	res.g = g;
	res.b = b;
	return res;
}

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------

void PrintInt (const int val) {
	printf ("Integer: %i \n", val );
}

void PrintInt (char *s, const int val) {
	char ss[128];
	strcpy (ss, s);
	strcat (ss," %i \n");
	printf (ss, val);
}

void PrintStr (const char *val) {
	printf ("%s \n", val);
}

void PrintString (string s) {
	cout << s << endl;
}

void PrintFloat (const float val) {
	printf ("%.3f \n", val);
}

void PrintDouble (const double val) {
	printf ("%.3f \n", val);
}

void PrintFloat (char *s, const float val) {
	char ss[128];
	strcpy (ss, s);
	strcat (ss," %.4f \n");
	printf (ss, val);
}

void PrintFloat8 (const float val) {
	printf ("%.8f \n", val);
}

void PrintBool (const bool val) {
	if (val == true) printf ("bool: true\n");
	else printf ("bool: false\n");
}

void PrintPointer (void *p) {
	if (p == NULL) printf ("Pointer: NULL\n");
	else printf ("Pointer: %p \n", &p);
}

void PrintVector4 (const TVector4 v) {
	printf ("%.2f  %.2f  %.2f  %.2f \n", v.x, v.y, v.z, v.w); 
}

void PrintColor (const TColor v) {
	printf ("%.2f  %.2f  %.2f  %.2f \n", v.r, v.g, v.b, v.a); 
}

void PrintVector2 (const TVector2 v) {
	printf ("%.2f  %.2f \n", v.x, v.y); 
}

void PrintVector (const TVector3 v) {
	printf ("%.4f  %.4f  %.4f \n", v.x, v.y, v.z); 
}

void PrintIndex3 (TIndex3 idx) {
	printf ("%i %i %i \n", idx.i, idx.j, idx.k);
}

void PrintIndex4 (TIndex4 idx) {
	printf ("%i %i %i %i \n", idx.i, idx.j, idx.k, idx.l);
}

void PrintVector (char *s, const TVector3 v) {
	char ss[128];
	strcpy (ss, s);
	strcat (ss, " %.4f  %.4f  %.4f \n");
	printf (ss, v.x, v.y, v.z); 
}

void PrintMatrix (TMatrix mat) {
	printf ("\n");
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) 
			if (mat[i][j]<0) printf ("  %.2f", mat[i][j]);
			else printf ("   %.2f", mat[i][j]);
		printf ("\n");
	}
	printf ("\n");
}

void PrintMatrixGL (TMatrixGL glmat) {
	printf ("\n");
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) 
			if (glmat[i*4+j]<0) printf ("  %.2f", glmat[i*4+j]);
			else printf ("   %.2f", glmat[i*4+j]);
		printf ("\n");
	}
	printf ("\n");
}

void PrintQuaternion (TQuaternion q) {
	printf ("Quaternion: %.4f  %.4f  %.4f  %.4f\n", q.x, q.y, q.z, q.w); 
}

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

void Message (const char *msg, const char *desc) {
	if (strlen(msg)<1 && strlen(desc)<1) {
		printf (" \n");
		return;
	}
	char item[128];
	strcpy (item, msg);
	strcat (item, "  ");
	strcat (item, desc);
	printf ("%s \n", item);
}

void Message (const char *msg) {
	if (strlen(msg)<1) {
		printf (" \n");
		return;
	}
	char item[128];
	strcpy (item, msg);
	printf ("%s \n", item);
}

void MessageN (string a, string b) {
	cout << a << " " << b << endl;
}
 
// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool FileExists (const char *filename) {
    struct stat stat_info;
    if (stat (filename, &stat_info) != 0) {
		if (errno != ENOENT) Message ("couldn't stat ", filename);
		return false;
    } else return true;
}

bool DirExists (const char *dirname) {
    DIR *xdir;
    if ((xdir = opendir (dirname)) == 0) 
		return ((errno != ENOENT) && (errno != ENOTDIR));
    if (closedir (xdir) != 0) Message ("Couldn't close directory", dirname);
    return true;
}

bool FileExistsWin (const char *filename) {
	FILE *xfile = fopen (filename, "r");
    if (xfile == NULL) return false;
	if (fclose (xfile) != 0) Message ("error closing file", filename);
	return true;
}

bool DirExistsWin (const char *dirname) {
    char curdir[1000];
    if (getcwd (curdir, 1000) == 0)
		Message ("DirExistsWin: getcwd failed");

    if (chdir (dirname) == -1) return false;
    if (chdir (curdir) == -1) {
		Message ("Couldn't access directory", curdir);
		return false;
	}
    return true;
}

void GetTimeComponents (double time, int *min, int *sec, int *hundr) {
    *min = (int) (time / 60);
    *sec = ((int) time) % 60;
    *hundr = ((int) (time * 100 + 0.5) ) % 100;
}

void DrawStdSphere (int num_divisions) {
    double theta, phi, d_theta, d_phi, eps, twopi;
    double x, y, z;
    int div = num_divisions;
    eps = 1e-15;
    twopi = M_PI * 2.0;
    d_theta = d_phi = M_PI / div;
    for  (phi = 0.0; phi + eps < M_PI; phi += d_phi) {
	double cos_theta, sin_theta;
	double sin_phi, cos_phi;
	double sin_phi_d_phi, cos_phi_d_phi;
	sin_phi = sin (phi);
	cos_phi = cos (phi);
	sin_phi_d_phi = sin (phi + d_phi);
	cos_phi_d_phi = cos (phi + d_phi);
	if  (phi <= eps) {
		glBegin (GL_TRIANGLE_FAN);
        	glNormal3f (0.0, 0.0, 1.0); 
        	glVertex3f (0.0, 0.0, 1.0);
			for  (theta = 0.0; theta + eps < twopi; theta += d_theta) {
				sin_theta = sin (theta);
				cos_theta = cos (theta);
				x = cos_theta * sin_phi_d_phi;
				y = sin_theta * sin_phi_d_phi;
				z = cos_phi_d_phi;
				glNormal3f (x, y, z); 
				glVertex3f (x, y, z);
			} 
			x = sin_phi_d_phi;
			y = 0.0;
			z = cos_phi_d_phi;
			glNormal3f (x, y, z);
			glVertex3f (x, y, z);
		glEnd ();

	} else if  (phi + d_phi + eps >= M_PI) {
            glBegin (GL_TRIANGLE_FAN);
                glNormal3f (0.0, 0.0, -1.0);
                glVertex3f (0.0, 0.0, -1.0);
                for  (theta = twopi; theta - eps > 0; theta -= d_theta) {
		    sin_theta = sin (theta);
		    cos_theta = cos (theta);
                    x = cos_theta * sin_phi;
                    y = sin_theta * sin_phi;
                    z = cos_phi;
                    glNormal3f (x, y, z);
                    glVertex3f (x, y, z);
                } 
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
                glNormal3f (x, y, z);
                glVertex3f (x, y, z);
            glEnd();
        } else {
            glBegin (GL_TRIANGLE_STRIP);
                for  (theta = 0.0; theta + eps < twopi; theta += d_theta) {
		    sin_theta = sin (theta);
		    cos_theta = cos (theta);
                    x = cos_theta * sin_phi;
                    y = sin_theta * sin_phi;
                    z = cos_phi;
                    glNormal3f (x, y, z);
                    glVertex3f (x, y, z);
                    x = cos_theta * sin_phi_d_phi;
                    y = sin_theta * sin_phi_d_phi;
                    z = cos_phi_d_phi;
                    glNormal3f (x, y, z);
                    glVertex3f (x, y, z);
                } 
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
                glNormal3f (x, y, z);
                glVertex3f (x, y, z);
                x = sin_phi_d_phi;
                y = 0.0;
                z = cos_phi_d_phi;
                glNormal3f (x, y, z);
                glVertex3f (x, y, z);
            glEnd();
        } 
    } 
} 


