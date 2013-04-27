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

using namespace std;

// ----- elementary string functions ----------------------------------
char	*NewStr (const char *s);
string   MakePathStr  (string src, string add);
void     SInsertN     (string &s, unsigned int pos, const string ins);
void     SDeleteN     (string &s, unsigned int pos, int count);
int      SPosN        (string &s, const string find);
void     STrimLeftN   (string &s);
void     STrimRightN  (string &s);
void     STrimN       (string &s);

// ----- conversion functions -----------------------------------------
void     Int_StrN     (string &s, const int val);
string   Int_StrN     (const int val);
string   Int_StrN     (const int val, const int count);
void     Float_StrN   (string &s, const float val, const int count);
string   Float_StrN   (const float val, const int count);
string   Vector_StrN  (const TVector3 v, const int count);
int      Str_IntN     (const string &s, const int def);
bool     Str_BoolN    (const string &s, const bool def);
bool     Str_BoolNX   (const string &s, const bool def);
float    Str_FloatN   (const string &s, const float def);
TVector2 Str_Vector2N (const string &s, const TVector2 def);
TVector2 Str_Vector2N (const string &s);
TVector3 Str_Vector3N (const string &s, const TVector3 def);
TVector3 Str_Vector3N (const string &s);
TVector4 Str_Vector4N (const string &s, const TVector4 def);
TColor   Str_ColorN   (const string &s, const TColor def);
TColor3  Str_Color3N  (const string &s, const TColor3 def);
void     Str_ArrN     (const string &s, float *arr, int count, float def);
string   Bool_StrN    (const bool val);
TIndex3  Str_Index3N  (const string &s, const TIndex3 def);

// ----- SP functions for parsing lines --------------------------------
bool     SPExistsN    (string &s, const string &tag);
int      SPPosN       (string &s, const string &tag);

string   SPItemN      (string &s, const string &tag);
void     SPItemN      (string &s, const string &tag, string &item);
string   SPStrN       (string &s, const string &tag, const string def);
int      SPIntN       (string &s, const string &tag, const int def);
bool     SPBoolN      (string &s, const string &tag, const bool def);
bool     SPBoolNX     (string &s, const string &tag, const bool def);
float    SPFloatN     (string &s, const string &tag, const float def);
TVector2 SPVector2N   (string &s, const string &tag, const TVector2 def);
TVector3 SPVector3N   (string &s, const string &tag, const TVector3 def);
TIndex3  SPIndex3N    (string &s, const string &tag, const TIndex3 def);
TVector2 SPVector2N   (string &s, const string &tag);
TVector3 SPVector3N   (string &s, const string &tag);
TVector4 SPVector4N   (string &s, const string &tag, const TVector4 def);
TColor   SPColorN     (string &s, const string &tag, const TColor def);
TColor3  SPColor3N    (string &s, const string &tag, const TColor3 def);
TColor3  SPColor3N    (string &s, const string &tag);
void     SPArrN       (string &s, const string &tag, float *arr, int count, float def);
int      SPEnumN      (string &s, const string &tag, int def);

// ----- compatibility ------------------------------------------------
void     Int_CharN    (char *s, const int val);
void     SPCharN      (string &s, const string &tag, char *result);

// ----- making SP strings --------------------------------------------
void     SPAddIntN    (string &s, const string &tag, const int val);
void     SPAddFloatN  (string &s, const string &tag, const float val, int count);
void     SPAddStrN    (string &s, const string &tag, const string &val);
void     SPAddVec2N   (string &s, const string &tag, const TVector2 val, int count);
void     SPAddVec3N   (string &s, const string &tag, const TVector3 val, int count);
void     SPAddIndx3N  (string &s, const string &tag, const TIndex3 val);
void     SPAddIndx4N  (string &s, const string &tag, const TIndex4 val);
void	 SPAddBoolN   (string &s, const string &tag, const bool val);

// ----- manipulating SP strings --------------------------------------
void     SPSetIntN    (string &s, const string &tag, const int val);
void     SPSetFloatN  (string &s, const string &tag, const float val, int count);
void     SPSetStrN    (string &s, const string &tag, const string &val);

// --------------------------------------------------------------------
//		 string list
// --------------------------------------------------------------------

class CSPList  {
private:
	string *flines;
	int fmax;
	int fcount;
	int *fflag;
	int fnewlineflag;
public:
	CSPList (int maxlines, int newlineflag = 0);
	~CSPList ();

	string Line (int idx);
	const char *LineC (int idx);
	int  Count ();
	void Clear ();
	void Add (string line);
	void Add (string line, int flag);
	void Append (string line, int idx);
	void Print ();
	bool Load (const string &filepath);
	bool Load (string dir, string filename);
	bool Save (const string &filepath);
	bool Save (string dir, string filename);

	int Flag (int idx);
	void SetFlag (int idx, int flag); 
	void MakeIndex (string &index, const string &tag);
};

void SetEnum (string s);

#endif


