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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "common.h"
#include "spx.h"
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <ctime>

// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

const TColor colWhite		(1.0, 1.0, 1.0, 1.0);
const TColor colDYell		(1.0, 0.8, 0.0, 1.0);
const TColor colDDYell		(0.8, 0.6, 0.0, 1.0);
const TColor colYellow		(1.0, 1.0, 0.0, 1.0);
const TColor colLYell		(1.0, 1.0, 0.4, 1.0);
const TColor colOrange		(1.0, 0.5, 0.0, 1.0);
const TColor colLRed		(1.0, 0.3, 0.3, 1.0);
const TColor colRed			(1.0, 0.0, 0.0, 1.0);
const TColor colDRed		(0.8, 0.0, 0.0, 1.0);
const TColor colGrey		(0.5, 0.5, 0.5, 1.0);
const TColor colLGrey		(0.7, 0.7, 0.7, 1.0);
const TColor colDGrey		(0.3, 0.3, 0.3, 1.0);
const TColor colBlack		(0.0, 0.0, 0.0, 1.0);
const TColor colBlue		(0.0, 0.0, 1.0, 1.0);
const TColor colLBlue		(0.5, 0.7, 1.0, 1.0);
const TColor colDBlue		(0.0, 0.0, 0.6, 1.0);
const TColor colLBackgr		(0.5, 0.7, 0.9, 1.0);
const TColor colBackgr		(0.4, 0.6, 0.8, 1.0);
const TColor colMBackgr		(0.35, 0.5, 0.7, 1.0);
const TColor colDBackgr		(0.2, 0.3, 0.6, 1.0);
const TColor colDDBackgr	(0.13, 0.2, 0.4, 1.0);
const TColor colMess		(0.3, 0.3, 0.7, 1.0);
const TColor colSky			(0.82, 0.86, 0.88, 1.0);

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

void PrintFloat (const char *s, const float val) {
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

void PrintVector (const char *s, const TVector3& v) {
	cout << s << ' ';
	PrintVector(v);
}

void PrintMatrix (const TMatrix mat) {
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
	if (*msg == 0 && *desc == 0) {
		cout << '\n';
		return;
	}

	string aa = msg;
	string bb = desc;
	cout << aa << "  " << bb << '\n';
	msg_list.Add (aa + bb);
}

void Message (const char *msg) {
	cout << msg << '\n';
	if (*msg != 0)
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

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents (double time, int *min, int *sec, int *hundr) {
    *min = (int) (time / 60);
    *sec = ((int) time) % 60;
    *hundr = ((int) (time * 100 + 0.5) ) % 100;
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
