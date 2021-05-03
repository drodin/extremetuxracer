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

#define TColor(r, g, b, a) sf::Color(static_cast<sf::Uint8>(r*255), static_cast<sf::Uint8>(g*255), static_cast<sf::Uint8>(b*255), static_cast<sf::Uint8>(a*255))
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

const sf::Color colBronze   = sf::Color(205, 127, 50,  255);
const sf::Color colSilver   = sf::Color(192, 192, 192, 255);
const sf::Color colGold     = sf::Color(255, 215, 0,   255);
const sf::Color colGreen    = sf::Color(0,   128, 0,   255);

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------

void PrintInt(const int val) {
	std::cout << "Integer: " << val << '\n';
}

void PrintInt(const std::string& s, const int val) {
	std::cout << s << val << std::endl;
}

void PrintStr(const char *val) {
	std::cout << val << '\n';
}

void PrintString(const std::string& s) {
	std::cout << s << std::endl;
}

void PrintDouble(const double val) {
	std::cout.precision(4);
	std::cout << val << '\n';
}

void PrintVector4(const TVector4d& v) {
	std::cout.precision(3);
	std::cout << v.x << "  " << v.y << "  " << v.z << "  " << v.w << '\n';
}

void PrintColor(const sf::Color& c) {
	std::cout.precision(3);
	std::cout << c.r << "  " << c.g << "  " << c.b << '\n';
}

void PrintVector2(const TVector2d& v) {
	std::cout.precision(3);
	std::cout << v.x << "  " << v.y << '\n';
}

void PrintVector(const TVector3d& v) {
	std::cout.precision(5);
	std::cout << v.x << "  " << v.y << "  " << v.z << '\n';
}

template<int x, int y>
void PrintMatrix(const TMatrix<x, y>& mat) {
	std::cout << '\n';
	std::cout.precision(3);
	for (int i=0; i<x; i++) {
		for (int j=0; j<y; j++) {
			if (mat[i][j]>=0) std::cout << ' ';
			std::cout << "  " << mat[i][j];
		}
		std::cout << '\n';
	}
	std::cout << '\n';
}
template void PrintMatrix<4, 4>(const TMatrix<4, 4>& mat);
template void PrintMatrix<3, 3>(const TMatrix<3, 3>& mat);

void PrintQuaternion(const TQuaternion& q) {
	std::cout.precision(5);
	std::cout << "Quaternion: " << q.x << "  " << q.y << "  " << q.z << "  " << q.w << '\n';
}

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

static CSPList msg_list;

void SaveMessages() {
	msg_list.Save(param.config_dir, "messages");
}

void Message(const char *msg, const char *desc) {
	if (*msg == 0 && *desc == 0) {
		std::cout << '\n';
		return;
	}

	std::string aa = msg;
	std::string bb = desc;
	std::cout << aa << "  " << bb << '\n';
	msg_list.Add(aa + bb);
}

void Message(const char *msg) {
	std::cout << msg << '\n';
	if (*msg != 0)
		msg_list.Add(msg);
}

void Message(const std::string& a, const std::string& b) {
	std::cout << a << ' ' << b << std::endl;
	msg_list.Add(a + b);
}

void Message(const std::string& msg) {
	std::cout << msg << std::endl;
	msg_list.Add(msg);
}

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool FileExists(const std::string& filename) {
	struct stat stat_info;
	if (stat(filename.c_str(), &stat_info) != 0) {
		if (errno != ENOENT) Message("couldn't stat", filename);
		return false;
	} else return true;
}

bool FileExists(const std::string& dir, const std::string& filename) {
	return FileExists(MakePathStr(dir, filename));
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

std::string GetTimeString() {
	std::time_t rawtime;
	std::time(&rawtime);
	struct std::tm* timeinfo = std::localtime(&rawtime);

	std::string line = Int_StrN(timeinfo->tm_mon + 1);
	line += '_' + Int_StrN(timeinfo->tm_mday);
	line += '_' + Int_StrN(timeinfo->tm_hour);
	line += Int_StrN(timeinfo->tm_min);
	line += Int_StrN(timeinfo->tm_sec);
	return line;
}
