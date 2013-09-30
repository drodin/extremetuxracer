/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#ifndef SPX_H
#define SPX_H

#include "bh.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

extern const string emptyString;
extern const string errorString;

// ----- elementary string functions ----------------------------------
string   MakePathStr  (const string& src, const string& add);
void     SInsertN     (string &s, size_t pos, const string& ins);
void     SDeleteN     (string &s, size_t pos, size_t count);
size_t   SPosN        (const string &s, const string& find);
void     STrimLeftN   (string &s);
void     STrimRightN  (string &s);
void     STrimN       (string &s);

// ----- conversion functions -----------------------------------------
void     Int_StrN     (string &s, const int val);
string   Int_StrN     (const int val);
string   Int_StrN     (const int val, const streamsize count);
void     Float_StrN   (string &s, const float val, const streamsize count);
string   Float_StrN   (const float val, const streamsize count);
string   Vector_StrN  (const TVector3d& v, const streamsize count);
int      Str_IntN     (const string &s, const int def);
bool     Str_BoolN    (const string &s, const bool def);
float    Str_FloatN   (const string &s, const float def);
TVector2d Str_Vector2N (const string &s, const TVector2d& def);
TVector3d Str_Vector3N (const string &s, const TVector3d& def);
TVector4d Str_Vector4N (const string &s, const TVector4d& def);
TColor   Str_ColorN   (const string &s, const TColor& def);
TColor3  Str_Color3N  (const string &s, const TColor3& def);
void     Str_ArrN     (const string &s, float *arr, size_t count, float def);
string   Bool_StrN    (const bool val);
TIndex3  Str_Index3N  (const string &s, const TIndex3& def);

// ----- SP functions for parsing lines --------------------------------
bool     SPExistsN    (const string &s, const string &tag);
size_t   SPPosN       (const string &s, const string &tag);

string   SPItemN      (const string &s, const string &tag);
string   SPStrN       (const string &s, const string &tag, const string& def = emptyString);
int      SPIntN       (const string &s, const string &tag, const int def);
bool     SPBoolN      (const string &s, const string &tag, const bool def);
float    SPFloatN     (const string &s, const string &tag, const float def);
TVector2d SPVector2N   (const string &s, const string &tag, const TVector2d& def);
TVector3d SPVector3N   (const string &s, const string &tag, const TVector3d& def);
TIndex3  SPIndex3N    (const string &s, const string &tag, const TIndex3& def);
TVector4d SPVector4N   (const string &s, const string &tag, const TVector4d& def);
TColor   SPColorN     (const string &s, const string &tag, const TColor& def);
TColor3  SPColor3N    (const string &s, const string &tag, const TColor3& def);
void     SPArrN       (const string &s, const string &tag, float *arr, size_t count, float def);

// ----- making SP strings --------------------------------------------
void     SPAddIntN    (string &s, const string &tag, const int val);
void     SPAddFloatN  (string &s, const string &tag, const float val, size_t count);
void     SPAddStrN    (string &s, const string &tag, const string &val);
void     SPAddVec2N   (string &s, const string &tag, const TVector2d& val, size_t count);
void     SPAddVec3N   (string &s, const string &tag, const TVector3d& val, size_t count);
void     SPAddIndx3N  (string &s, const string &tag, const TIndex3& val);
void     SPAddIndx4N  (string &s, const string &tag, const TIndex4& val);
void	 SPAddBoolN   (string &s, const string &tag, const bool val);

// ----- manipulating SP strings --------------------------------------
void     SPSetIntN    (string &s, const string &tag, const int val);
void     SPSetFloatN  (string &s, const string &tag, const float val, size_t count);
void     SPSetStrN    (string &s, const string &tag, const string &val);

// --------------------------------------------------------------------
//		 string list
// --------------------------------------------------------------------

class CSPList {
private:
	vector<string> flines;
	size_t fmax;
	bool fnewlineflag;
public:
	CSPList (size_t maxlines, bool newlineflag = false);

	const string& Line (size_t idx) const;
	size_t Count () const { return flines.size(); }
	void Clear () { flines.clear(); }
	void Add (const string& line);
	void AddLine();
	void Append (const string& line, size_t idx);
	void Print () const;
	bool Load (const string &filepath);
	bool Load (const string& dir, const string& filename);
	bool Save (const string &filepath) const;
	bool Save (const string& dir, const string& filename) const;

	void MakeIndex (map<string, size_t>& index, const string &tag);
};

#endif
