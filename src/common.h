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


#ifndef MIN
#	define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#	define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#define clamp(minimum, x, maximum) (max(min(x, maximum), minimum))

#ifndef ROUND_TO_NEAREST
#	define ROUND_TO_NEAREST(x) ((int) ((x)+0.5))
#endif

#define ANGLES_TO_RADIANS(x) (M_PI / 180.0 * (x) )
#define RADIANS_TO_ANGLES(x) (180.0 / M_PI * (x) )

#define TUX_WIDTH 0.45
#define MAX_ROLL_ANGLE 30
#define BRAKING_ROLL_ANGLE 55


// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

extern const TColor colWhite;
extern const TColor colDYell;
extern const TColor colDDYell;
extern const TColor colYellow;
extern const TColor colLYell;
extern const TColor colOrange;
extern const TColor colLRed;
extern const TColor colRed;
extern const TColor colDRed;
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

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------
// some simple functions to print out values on the
// terminal. Only used for development.
void	PrintInt (const int val);
void	PrintInt (const string& s, const int val);
void	PrintStr (const char *val);
void	PrintString (const string& s);
void	PrintFloat (const float val);
void	PrintDouble (const double val);
void	PrintFloat8 (const float val);
void	PrintFloat (const char *s, const float val);
void	PrintBool (const bool val);
void	PrintPointer (void *p);
void	PrintVector (const TVector3& v);
void	PrintVector4 (const TVector4& v);
void    PrintColor (const TColor& c);
void	PrintVector2 (const TVector2& v);
void	PrintVector (const char *s, const TVector3& v);

void	PrintMatrix (const TMatrix mat);
void	PrintQuaternion (const TQuaternion& q);

void	PrintIndex3 (const TIndex3& idx);
void	PrintIndex4 (const TIndex4& idx);

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool    FileExists (const char *filename);
bool	FileExists (const string& filename);
bool	FileExists (const string& dir, const string& filename);
bool    DirExists (const char *dirname);

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

void    Message (const char *msg, const char *desc);
void    Message (const char *msg);
void	Message (const string& a, const string& b);
void	SaveMessages ();

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents (double time, int *min, int *sec, int *hundr);
string GetTimeString1 ();

size_t write_word (FILE *fp, uint16_t w);
size_t write_dword (FILE *fp, uint32_t dw);
size_t write_long (FILE *fp, int32_t l);

#endif
