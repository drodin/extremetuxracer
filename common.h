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

#ifndef COMMON_H
#define COMMON_H

#include "bh.h"

using namespace std;

// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

extern const TColor colWhite;
extern const TColor colDYell;
extern const TColor colYellow;
extern const TColor colLYell;
extern const TColor colOrange;
extern const TColor colRed;
extern const TColor colGrey;
extern const TColor colLGrey;
extern const TColor colDGrey;
extern const TColor colBlack;
extern const TColor colBlue;
extern const TColor colLBlue;
extern const TColor colDBlue;
extern const TColor colBackgr;
extern const TColor colDBackgr;
extern const TColor colDDBackgr;
extern const TColor colMBackgr;
extern const TColor colLBackgr;
extern const TColor colMess;
extern const TColor colSky;

TColor	MakeColor (double r, double g, double b, double a);
TColor3 MakeColor3 (double r, double g, double b);

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------
// some simple functions to print out values on the 
// terminal. Only used for development. 

void	PrintInt (const int val);
void	PrintInt (char *s, const int val);
void	PrintStr (const char *val);
void	PrintString (string s);
void	PrintFloat (const float val);
void	PrintDouble (const double val);
void	PrintFloat8 (const float val);
void	PrintFloat (char *s, const float val);
void	PrintBool (const bool val);
void	PrintPointer (void *p);
void	PrintVector (const TVector3 v);
void	PrintVector4 (const TVector4 v);
void    PrintColor (const TColor);
void	PrintVector2 (const TVector2 v);
void	PrintVector (char *s, const TVector3 v);

void	PrintMatrix (TMatrix mat);
void	PrintMatrixGL (TMatrixGL glmat);
void	PrintQuaternion (TQuaternion q);

void	PrintIndex3 (TIndex3 idx);
void	PrintIndex4 (TIndex4 idx);

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool    FileExists (const char *filename);
bool    DirExists (const char *dirname);

// the following functions should work on Windows, too
// though they are not particularly for Window
bool	FileExistsWin (const char *filename);
bool    DirExistsWin (const char *dirname);

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

// at the moment the messages are written on the terminal.
// It's intended to write them in a file, so the user can see what was wrong
// and perhaps report errors
void    Message (const char *msg, const char *desc);
void    Message (const char *msg);
void	MessageN (string a, string b);

// --------------------------------------------------------------------
//				other stuff
// --------------------------------------------------------------------

void GetTimeComponents (double time, int *min, int *sec, int *hundr);

#endif 


