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
#include "spx.h"
#include <iostream>
#include <cerrno>

// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

const TColor colWhite =		{1.0, 1.0, 1.0, 1.0};
const TColor colDYell =		{1.0, 0.8, 0.0, 1.0};
const TColor colDDYell =	{0.8, 0.6, 0.0, 1.0};
const TColor colYellow =	{1.0, 1.0, 0.0, 1.0};
const TColor colLYell = 	{1.0, 1.0, 0.4, 1.0};
const TColor colOrange =	{1.0, 0.5, 0.0, 1.0};
const TColor colLRed =		{1.0, 0.3, 0.3, 1.0};
const TColor colRed =		{1.0, 0.0, 0.0, 1.0};
const TColor colDRed =		{0.8, 0.0, 0.0, 1.0};
const TColor colGrey =		{0.5, 0.5, 0.5, 1.0};
const TColor colLGrey =		{0.7, 0.7, 0.7, 1.0};
const TColor colDGrey =		{0.3, 0.3, 0.3, 1.0};
const TColor colBlack =		{0.0, 0.0, 0.0, 1.0};	
const TColor colBlue =		{0.0, 0.0, 1.0, 1.0};	
const TColor colLBlue =		{0.5, 0.7, 1.0, 1.0};	
const TColor colDBlue =		{0.0, 0.0, 0.6, 1.0};	
const TColor colLBackgr =	{0.5, 0.7, 0.9, 1.0};
const TColor colBackgr =	{0.4, 0.6, 0.8, 1.0};
const TColor colMBackgr =	{0.35, 0.5, 0.7, 1.0};
const TColor colDBackgr =	{0.2, 0.3, 0.6, 1.0};
const TColor colDDBackgr =	{0.13, 0.2, 0.4, 1.0};
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
	cout << "Integer: " << val << '\n';
}

void PrintInt (const string& s, const int val) {
	cout << s << val << endl;
}

void PrintStr (const char *val) {
	cout << val << '\n';
}

void PrintString (const string& s) {
	cout << s << endl;
}

void PrintFloat (const float val) {
	cout.precision(4);
	cout << val << '\n';
}

void PrintDouble (const double val) {
	cout.precision(4);
	cout << val << '\n';
}

void PrintFloat (char *s, const float val) {
	cout.precision(5);
	cout << s << ' ' << val << '\n';
}

void PrintFloat8 (const float val) {
	cout.precision(9);
	cout << val << '\n';
}

void PrintBool (const bool val) {
	if (val == true) cout <<"bool: true\n";
	else cout << "bool: false\n";
}

void PrintPointer (void *p) {
	if (p == NULL) cout << "Pointer: NULL\n";
	else cout << "Pointer: " << p << '\n';
}

void PrintVector4 (const TVector4& v) {
	cout.precision(3);
	cout << v.x << "  " << v.y << "  " << v.z << "  " << v.w << '\n';
}

void PrintColor (const TColor& v) {
	cout.precision(3);
	cout << v.r << "  " << v.g << "  " << v.b << '\n';
}

void PrintVector2 (const TVector2& v) {
	cout.precision(3);
	cout << v.x << "  " << v.y << '\n';
}

void PrintVector (const TVector3& v) {
	cout.precision(5);
	cout << v.x << "  " << v.y << "  " << v.z << '\n';
}

void PrintIndex3 (const TIndex3& idx) {
	cout << idx.i << ' ' << idx.j << ' ' << idx.k << '\n';
}

void PrintIndex4 (const TIndex4& idx) {
	cout << idx.i << ' ' << idx.j << ' ' << idx.k << ' ' << idx.l << '\n';
}

void PrintVector (char *s, const TVector3& v) {
	cout << s << ' ';
	PrintVector(v);
}

void PrintMatrix (TMatrix mat) {
	cout << '\n';
	cout.precision(3);
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			if (mat[i][j]>=0) cout << ' ';
			cout << "  " << mat[i][j];
		}
		cout << '\n';
	}
	cout << '\n';
}

void PrintQuaternion (const TQuaternion& q) {
	cout.precision(5);
	cout << "Quaternion: " << q.x << "  " << q.y << "  " << q.z << "  " << q.w << '\n';
}

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

static CSPList msg_list (100);

void InitMessages () {}

void SaveMessages () {
	msg_list.Save (param.config_dir, "messages");
}

void Message (const char *msg, const char *desc) {
	if (strlen(msg)<1 && strlen(desc)<1) {
		cout << '\n';
		return;
	}

	string aa = msg;
	string bb = desc;
	cout << aa << "  " << bb << '\n';
	msg_list.Add (aa + bb);
}

void Message (const char *msg) {
	if (strlen(msg)<1) {
		cout << '\n';
		return;
	}
	cout << msg << '\n';
	msg_list.Add (msg);
}

void Message (const string& a, const string& b) {
	cout << a << ' ' << b << endl;
	msg_list.Add (a + b);
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

bool FileExists (const string& filename) {
	return FileExists (filename.c_str());
}

bool FileExists (const string& dir, const string& filename) {
	return FileExists (dir + SEP + filename);
}

#ifndef OS_WIN32_MSC
bool DirExists (const char *dirname) {
	DIR *xdir;
    if ((xdir = opendir (dirname)) == 0) 
		return ((errno != ENOENT) && (errno != ENOTDIR));
    if (closedir (xdir) != 0) Message ("Couldn't close directory", dirname);
    return true;
}
#else
bool DirExists (const char *dirname) {
	DWORD typ = GetFileAttributesA(dirname);
	if(typ == INVALID_FILE_ATTRIBUTES)
		return false; // Doesn't exist

	return (typ & FILE_ATTRIBUTE_DIRECTORY) != 0; // Is directory?
}
#endif

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

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents (double time, int *min, int *sec, int *hundr) {
    *min = (int) (time / 60);
    *sec = ((int) time) % 60;
    *hundr = ((int) (time * 100 + 0.5) ) % 100;
}

/*
   struct tm {
     int tm_sec;  
     int tm_min;  
     int tm_hour;	// 0..23
	 int tm_mday;	// day in month 1..31
	 int tm_mon;	// 0..11
     int tm_year;	// year (sice 1900)
     int tm_wday;	// weekday 0..6, start with sunday
     int tm_yday;   // day in the year
     int tm_isdst;  // not zero in case of US sommertime converting 
   };
*/ 

void GetTestTime () {
	time_t rawtime;			// seconds since 1. January 1970
	struct tm * timeinfo;	// see above

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	PrintInt (timeinfo->tm_year + 1900);
	PrintStr (asctime (timeinfo));
}

string GetTimeString1 () {
	time_t rawtime;	
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);
//	line = Int_StrN (timeinfo->tm_year-100);
	string line = Int_StrN (timeinfo->tm_mon + 1);
	line += "_" + Int_StrN (timeinfo->tm_mday);
	line += "_" + Int_StrN (timeinfo->tm_hour);
	line += Int_StrN (timeinfo->tm_min);
	line += Int_StrN (timeinfo->tm_sec);
	return line;
}

// --------------------------------------------------------------------
//				FILE, read and write
// --------------------------------------------------------------------

size_t write_word (FILE *fp, uint16_t w) {
	return fwrite(&w, 2, 1, fp);
}

size_t write_dword(FILE *fp, uint32_t dw) {
	return fwrite(&dw, 4, 1, fp);
}

size_t write_long (FILE *fp, int32_t l) {
	return fwrite(&l, 4, 1, fp);
}
