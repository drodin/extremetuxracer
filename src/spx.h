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
#include <list>
#include <map>

extern const string emptyString;
extern const string errorString;

// ----- elementary string functions ----------------------------------
string   MakePathStr(const string& src, const string& add);
void     SInsertN(string &s, size_t pos, const string& ins);
void     SDeleteN(string &s, size_t pos, size_t count);
size_t   SPosN(const string &s, const string& find);
void     STrimLeftN(string &s);
void     STrimRightN(string &s);
void     STrimN(string &s);

// ----- conversion functions -----------------------------------------
string   Int_StrN(const int val);
string   Int_StrN(const int val, const streamsize count);
string   Float_StrN(const float val, const streamsize count);
string   Bool_StrN(const bool val);
string   Vector_StrN(const TVector3d& v, const streamsize count);
int      Str_IntN(const string &s, const int def);
bool     Str_BoolN(const string &s, const bool def);
float    Str_FloatN(const string &s, const float def);
template<typename T>
TVector2<T> Str_Vector2(const string &s, const TVector2<T>& def);
template<typename T>
TVector3<T> Str_Vector3(const string &s, const TVector3<T>& def);
template<typename T>
TVector4<T> Str_Vector4(const string &s, const TVector4<T>& def);
sf::Color   Str_ColorN(const string &s, const sf::Color& def);
TColor3  Str_Color3N(const string &s, const TColor3& def);
void     Str_ArrN(const string &s, float *arr, size_t count, float def);

// ----- SP functions for parsing lines --------------------------------
size_t   SPPosN(const string &s, const string &tag);

string   SPStrN(const string &s, const string &tag, const string& def = emptyString);
string   SPStrN(const string &s, const char* tag, const char* def);
int      SPIntN(const string &s, const string &tag, const int def);
bool     SPBoolN(const string &s, const string &tag, const bool def);
float    SPFloatN(const string &s, const string &tag, const float def);
template<typename T>
TVector2<T> SPVector2(const string &s, const string &tag, const TVector2<T>& def);
static inline TVector2d SPVector2d(const string &s, const string &tag) { return SPVector2(s, tag, NullVec2); }
static inline TVector2i SPVector2i(const string &s, const string &tag) { return SPVector2(s, tag, NullVec2i); }
template<typename T>
TVector3<T> SPVector3(const string &s, const string &tag, const TVector3<T>& def);
static inline TVector3d SPVector3d(const string &s, const string &tag) { return SPVector3(s, tag, NullVec3); }
static inline TVector3i SPVector3i(const string &s, const string &tag) { return SPVector3(s, tag, NullVec3i); }
template<typename T>
TVector4<T> SPVector4(const string &s, const string &tag, const TVector4<T>& def);
static inline TVector4d SPVector4d(const string &s, const string &tag) { return SPVector4(s, tag, NullVec4); }
static inline TVector4i SPVector4i(const string &s, const string &tag) { return SPVector4(s, tag, NullVec4i); }
sf::Color SPColorN(const string &s, const string &tag, const sf::Color& def);
TColor3   SPColor3N(const string &s, const string &tag, const TColor3& def);
void      SPArrN(const string &s, const string &tag, float *arr, size_t count, float def);

// ----- making SP strings --------------------------------------------
void     SPAddIntN(string &s, const string &tag, const int val);
void     SPAddFloatN(string &s, const string &tag, const float val, size_t count);
void     SPAddStrN(string &s, const string &tag, const string &val);
void     SPAddVec2N(string &s, const string &tag, const TVector2d& val, size_t count);
void     SPAddVec3N(string &s, const string &tag, const TVector3d& val, size_t count);
void     SPAddBoolN(string &s, const string &tag, const bool val);

// ----- manipulating SP strings --------------------------------------
void     SPSetIntN(string &s, const string &tag, const int val);
void     SPSetFloatN(string &s, const string &tag, const float val, size_t count);
void     SPSetStrN(string &s, const string &tag, const string &val);

// --------------------------------------------------------------------
//		 string list
// --------------------------------------------------------------------

class CSPList : public std::list<string> {
private:
	size_t fmax;
	bool fnewlineflag;
public:
	CSPList(size_t maxlines, bool newlineflag = false);

	void Add(const string& line = emptyString);
	void Add(string&& line);
	void Print() const;
	bool Load(const string &filepath);
	bool Load(const string& dir, const string& filename);
	bool Save(const string &filepath) const;
	bool Save(const string& dir, const string& filename) const;

	void MakeIndex(map<string, size_t>& index, const string &tag);
};

#endif
