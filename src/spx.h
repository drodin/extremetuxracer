/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#ifndef SPX_H
#define SPX_H

#include "bh.h"
#include <string>
#include <list>
#include <unordered_map>

extern const std::string emptyString;
extern const std::string errorString;

// ----- elementary string functions ----------------------------------
std::string MakePathStr(const std::string& src, const std::string& add);
void        SInsertN(std::string &s, std::size_t pos, const std::string& ins);
void        SDeleteN(std::string &s, std::size_t pos, std::size_t count);
std::size_t SPosN(const std::string &s, const std::string& find);
void        STrimLeftN(std::string &s);
void        STrimRightN(std::string &s);
void        STrimN(std::string &s);

// ----- conversion functions -----------------------------------------
std::string Int_StrN(const int val);
std::string Int_StrN(const int val, const std::streamsize count);
std::string Float_StrN(const float val, const std::streamsize count);
std::string Bool_StrN(const bool val);
std::string Vector_StrN(const TVector3d& v, const std::streamsize count);
int         Str_IntN(const std::string &s, const int def);
bool        Str_BoolN(const std::string &s, const bool def);
float       Str_FloatN(const std::string &s, const float def);
template<typename T>
TVector2<T> Str_Vector2(const std::string &s, const TVector2<T>& def);
template<typename T>
TVector3<T> Str_Vector3(const std::string &s, const TVector3<T>& def);
template<typename T>
TVector4<T> Str_Vector4(const std::string &s, const TVector4<T>& def);
sf::Color   Str_ColorN(const std::string &s, const sf::Color& def);
sf::Color   Str_Color3N(const std::string &s, const sf::Color& def);
void        Str_ArrN(const std::string &s, float *arr, std::size_t count, float def);

// ----- SP functions for parsing lines --------------------------------
std::size_t SPPosN(const std::string &s, const std::string &tag);

std::string SPStrN(const std::string &s, const std::string &tag, const std::string& def = emptyString);
std::string SPStrN(const std::string &s, const char* tag, const char* def);
int         SPIntN(const std::string &s, const std::string &tag, const int def);
bool        SPBoolN(const std::string &s, const std::string &tag, const bool def);
float       SPFloatN(const std::string &s, const std::string &tag, const float def);
template<typename T>
TVector2<T> SPVector2(const std::string &s, const std::string &tag, const TVector2<T>& def);
static inline TVector2d SPVector2d(const std::string &s, const std::string &tag) { return SPVector2(s, tag, NullVec2); }
static inline TVector2i SPVector2i(const std::string &s, const std::string &tag) { return SPVector2(s, tag, NullVec2i); }
template<typename T>
TVector3<T> SPVector3(const std::string &s, const std::string &tag, const TVector3<T>& def);
static inline TVector3d SPVector3d(const std::string &s, const std::string &tag) { return SPVector3(s, tag, NullVec3); }
static inline TVector3i SPVector3i(const std::string &s, const std::string &tag) { return SPVector3(s, tag, NullVec3i); }
template<typename T>
TVector4<T> SPVector4(const std::string &s, const std::string &tag, const TVector4<T>& def);
static inline TVector4d SPVector4d(const std::string &s, const std::string &tag) { return SPVector4(s, tag, NullVec4); }
static inline TVector4i SPVector4i(const std::string &s, const std::string &tag) { return SPVector4(s, tag, NullVec4i); }
sf::Color SPColorN(const std::string &s, const std::string &tag, const sf::Color& def);
sf::Color SPColor3N(const std::string &s, const std::string &tag, const sf::Color& def);
void      SPArrN(const std::string &s, const std::string &tag, float *arr, std::size_t count, float def);

// ----- making SP strings --------------------------------------------
void     SPAddIntN(std::string &s, const std::string &tag, const int val);
void     SPAddFloatN(std::string &s, const std::string &tag, const float val, std::size_t count);
void     SPAddStrN(std::string &s, const std::string &tag, const std::string &val);
void     SPAddVec2N(std::string &s, const std::string &tag, const TVector2d& val, std::size_t count);
void     SPAddVec3N(std::string &s, const std::string &tag, const TVector3d& val, std::size_t count);
void     SPAddBoolN(std::string &s, const std::string &tag, const bool val);

// ----- manipulating SP strings --------------------------------------
void     SPSetIntN(std::string &s, const std::string &tag, const int val);
void     SPSetFloatN(std::string &s, const std::string &tag, const float val, std::size_t count);
void     SPSetStrN(std::string &s, const std::string &tag, const std::string &val);

// --------------------------------------------------------------------
//		 string list
// --------------------------------------------------------------------

class CSPList : public std::list<std::string> {
private:
	bool fnewlineflag;
public:
	explicit CSPList(bool newlineflag = false);

	void Add(const std::string& line = emptyString);
	void Add(std::string&& line);
	void Print() const;
	bool Load(const std::string &filepath);
	bool Load(const std::string& dir, const std::string& filename);
	bool Save(const std::string &filepath) const;
	bool Save(const std::string& dir, const std::string& filename) const;

	void MakeIndex(std::unordered_map<std::string, std::size_t>& index, const std::string &tag);
};

#endif
