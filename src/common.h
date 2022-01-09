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

#ifndef COMMON_H
#define COMMON_H

#include "bh.h"
#include "matrices.h"


#define clamp(minimum, x, maximum) (std::max(std::min(x, maximum), minimum))

#ifndef M_PI
#	define M_PI 3.1415926535
#endif

#ifndef EPS
#	define EPS 1.0e-13
#endif

#define ANGLES_TO_RADIANS(x) (M_PI / 180.0 * (x))
#define RADIANS_TO_ANGLES(x) (180.0 / M_PI * (x))

#define MAG_SQD(vec) ((vec).x * (vec).x + (vec).y * (vec).y + (vec).z * (vec).z)


// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

#define colTransp sf::Color::Transparent
#define colWhite  sf::Color::White
#define colBlack  sf::Color::Black
#define colRed    sf::Color::Red
#define colYellow sf::Color::Yellow
#define colBlue   sf::Color::Blue
extern const sf::Color colTBackr;
extern const sf::Color colDYell;
extern const sf::Color colDDYell;
extern const sf::Color colLYell;
extern const sf::Color colOrange;
extern const sf::Color colLRed;
extern const sf::Color colDRed;
extern const sf::Color colGrey;
extern const sf::Color colLGrey;
extern const sf::Color colDGrey;
extern const sf::Color colLBlue;
extern const sf::Color colDBlue;
extern const sf::Color colBackgr;
extern const sf::Color colDBackgr;
extern const sf::Color colDDBackgr;
extern const sf::Color colMBackgr;
extern const sf::Color colLBackgr;
extern const sf::Color colMess;
extern const sf::Color colSky;
extern const sf::Color colBronze;
extern const sf::Color colSilver;
extern const sf::Color colGold;
extern const sf::Color colGreen;


// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------
// some simple functions to print out values on the
// terminal. Only used for development.
void	PrintInt(const int val);
void	PrintInt(const std::string& s, const int val);
void	PrintStr(const char *val);
void	PrintString(const std::string& s);
void	PrintDouble(const double val);
void	PrintVector(const TVector3d& v);
void	PrintVector4(const TVector4d& v);
void	PrintColor(const sf::Color& c);
void	PrintVector2(const TVector2d& v);

template<int x, int y>
void	PrintMatrix(const TMatrix<x, y>& mat);
void	PrintQuaternion(const TQuaternion& q);

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool	FileExists(const std::string& filename);
bool	FileExists(const std::string& dir, const std::string& filename);
bool	DirExists(const char *dirname);

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

void	Message(const char *msg, const char *desc);
void	Message(const char *msg);
void	Message(const std::string& a, const std::string& b);
void	Message(const std::string& msg);
void	SaveMessages();

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents(double time, int *min, int *sec, int *hundr);
std::string GetTimeString();


#endif
