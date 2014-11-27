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
#include <sys/stat.h>
#include <iostream>
#include <cerrno>
#include <ctime>

// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

#define TColor(r, g, b, a) sf::Color(r*255, g*255, b*255, a*255)
const sf::Color colDYell =		TColor(1.0, 0.8, 0.0, 1.0);
const sf::Color colDDYell =		TColor(0.8, 0.6, 0.0, 1.0);
const sf::Color colLYell =		TColor(1.0, 1.0, 0.4, 1.0);
const sf::Color colOrange =		TColor(1.0, 0.5, 0.0, 1.0);
const sf::Color colLRed =		TColor(1.0, 0.3, 0.3, 1.0);
const sf::Color colDRed =		TColor(0.8, 0.0, 0.0, 1.0);
const sf::Color colGrey =		TColor(0.5, 0.5, 0.5, 1.0);
const sf::Color colLGrey =		TColor(0.7, 0.7, 0.7, 1.0);
const sf::Color colDGrey =		TColor(0.3, 0.3, 0.3, 1.0);
const sf::Color colLBlue =		TColor(0.5, 0.7, 1.0, 1.0);
const sf::Color colDBlue =		TColor(0.0, 0.0, 0.6, 1.0);
const sf::Color colLBackgr =	TColor(0.5, 0.7, 0.9, 1.0);
const sf::Color colBackgr =		TColor(0.4, 0.6, 0.8, 1.0);
const sf::Color colMBackgr =	TColor(0.35, 0.5, 0.7, 1.0);
const sf::Color colDBackgr =	TColor(0.2, 0.3, 0.6, 1.0);
const sf::Color colDDBackgr =	TColor(0.13, 0.2, 0.4, 1.0);
const sf::Color colTBackr =		TColor(0.4, 0.6, 0.8, 0.0);
const sf::Color colMess =		TColor(0.3, 0.3, 0.7, 1.0);
const sf::Color colSky =		TColor(0.82, 0.86, 0.88, 1.0);

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------

void PrintInt(const int val) {
	cout << "Integer: " << val << '\n';
}

void PrintInt(const string& s, const int val) {
	cout << s << val << endl;
}

void PrintStr(const char *val) {
	cout << val << '\n';
}

void PrintString(const string& s) {
	cout << s << endl;
}

void PrintDouble(const double val) {
	cout.precision(4);
	cout << val << '\n';
}

void PrintVector4(const TVector4d& v) {
	cout.precision(3);
	cout << v.x << "  " << v.y << "  " << v.z << "  " << v.w << '\n';
}

void PrintColor(const sf::Color& v) {
	cout.precision(3);
	cout << v.r << "  " << v.g << "  " << v.b << '\n';
}

void PrintVector2(const TVector2d& v) {
	cout.precision(3);
	cout << v.x << "  " << v.y << '\n';
}

void PrintVector(const TVector3d& v) {
	cout.precision(5);
	cout << v.x << "  " << v.y << "  " << v.z << '\n';
}

template<int x, int y>
void PrintMatrix(const TMatrix<x, y>& mat) {
	cout << '\n';
	cout.precision(3);
	for (int i=0; i<x; i++) {
		for (int j=0; j<y; j++) {
			if (mat[i][j]>=0) cout << ' ';
			cout << "  " << mat[i][j];
		}
		cout << '\n';
	}
	cout << '\n';
}
template void PrintMatrix<4, 4>(const TMatrix<4, 4>& mat);
template void PrintMatrix<3, 3>(const TMatrix<3, 3>& mat);

void PrintQuaternion(const TQuaternion& q) {
	cout.precision(5);
	cout << "Quaternion: " << q.x << "  " << q.y << "  " << q.z << "  " << q.w << '\n';
}

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

static CSPList msg_list(100);

void SaveMessages() {
	msg_list.Save(param.config_dir, "messages");
}

void Message(const char *msg, const char *desc) {
	if (*msg == 0 && *desc == 0) {
		cout << '\n';
		return;
	}

	string aa = msg;
	string bb = desc;
	cout << aa << "  " << bb << '\n';
	msg_list.Add(aa + bb);
}

void Message(const char *msg) {
	cout << msg << '\n';
	if (*msg != 0)
		msg_list.Add(msg);
}

void Message(const string& a, const string& b) {
	cout << a << ' ' << b << endl;
	msg_list.Add(a + b);
}

void Message(const string& msg) {
	cout << msg << endl;
	msg_list.Add(msg);
}

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool FileExists(const string& filename) {
	struct stat stat_info;
	if (stat(filename.c_str(), &stat_info) != 0) {
		if (errno != ENOENT) Message("couldn't stat", filename);
		return false;
	} else return true;
}

bool FileExists(const string& dir, const string& filename) {
	return FileExists(dir + SEP + filename);
}

#ifndef OS_WIN32_MSC
bool DirExists(const char *dirname) {
	DIR *xdir;
	if ((xdir = opendir(dirname)) == 0)
		return ((errno != ENOENT) && (errno != ENOTDIR));
	if (closedir(xdir) != 0) Message("Couldn't close directory", dirname);
	return true;
}
#else
bool DirExists(const char *dirname) {
	DWORD typ = GetFileAttributesA(dirname);
	if (typ == INVALID_FILE_ATTRIBUTES)
		return false; // Doesn't exist

	return (typ & FILE_ATTRIBUTE_DIRECTORY) != 0; // Is directory?
}
#endif

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents(double time, int *min, int *sec, int *hundr) {
	*min = (int)(time / 60);
	*sec = ((int) time) % 60;
	*hundr = ((int)(time * 100 + 0.5)) % 100;
}

string GetTimeString() {
	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo = localtime(&rawtime);

	string line = Int_StrN(timeinfo->tm_mon + 1);
	line += '_' + Int_StrN(timeinfo->tm_mday);
	line += '_' + Int_StrN(timeinfo->tm_hour);
	line += Int_StrN(timeinfo->tm_min);
	line += Int_StrN(timeinfo->tm_sec);
	return line;
}
