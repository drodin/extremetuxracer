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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "spx.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

const std::string emptyString = "";
const std::string errorString = "error";

// --------------------------------------------------------------------
//					elementary string functions
// --------------------------------------------------------------------

std::string MakePathStr(const std::string& src, const std::string& add) {
	std::string res = src;
	res += SEP;
	res += add;
	return res;
}

void SInsertN(std::string &s, std::size_t pos, const std::string& ins) {
	if (pos > s.size()) pos = s.size();
	s.insert(pos, ins);
}

void SDeleteN(std::string &s, std::size_t pos, std::size_t count) {
	if (pos > s.size()) pos = s.size();
	s.erase(pos, count);
}

std::size_t SPosN(const std::string &s, const std::string& find) {
	return s.find(find);
}

void STrimLeftN(std::string &s) {
	std::size_t i = s.find_first_not_of(" \t");
	if (i > 0)
		SDeleteN(s, 0, i);
}

void STrimRightN(std::string &s) {
	std::size_t i = s.find_last_not_of(" \t");
	if (i != s.size()-1)
		s.erase(i+1);
}

void STrimN(std::string &s) {
	STrimLeftN(s);
	STrimRightN(s);
}

// --------------------------------------------------------------------
//				conversion functions
// --------------------------------------------------------------------

std::string Int_StrN(const int val) {
	return std::to_string(val);
}

std::string Int_StrN(const int val, const std::streamsize count) {
	std::ostringstream os;
	os << std::setw(count) << std::setfill('0') << val;
	return os.str();
}

std::string Float_StrN(const float val, const std::streamsize count) {
	std::ostringstream os;
	os << std::setprecision(count) << std::fixed << val;
	return os.str();
}

std::string Vector_StrN(const TVector3d& v, const std::streamsize count) {
	std::string res = Float_StrN(v.x, count);
	res += ' ' + Float_StrN(v.y, count);
	res += ' ' + Float_StrN(v.z, count);
	return res;
}

std::string Bool_StrN(const bool val) {
	if (val == true) return "true";
	else return "false";
}

int Str_IntN(const std::string &s, const int def) {
	int val;
	std::istringstream is(s);
	is >> val;
	if (is.fail()) return def;
	else return val;
}

bool Str_BoolN(const std::string &s, const bool def) {
	if (s == "0" || s == "false")
		return false;
	if (s == "1" || s == "true")
		return true;
	return Str_IntN(s, (int)def) != 0; // Try to parse as int
}

float Str_FloatN(const std::string &s, const float def) {
	float val;
	std::istringstream is(s);
	is >> val;
	if (is.fail()) return def;
	else return val;
}

template<typename T>
TVector2<T> Str_Vector2(const std::string &s, const TVector2<T> &def) {
	T x, y;
	std::istringstream is(s);
	is >> x >> y;
	if (is.fail()) return def;
	else return TVector2<T>(x, y);
}
template TVector2<double> Str_Vector2(const std::string &s, const TVector2<double> &def);
template TVector2<int> Str_Vector2(const std::string &s, const TVector2<int> &def);

template<typename T>
TVector3<T> Str_Vector3(const std::string &s, const TVector3<T> &def) {
	T x, y, z;
	std::istringstream is(s);
	is >> x >> y >> z;
	if (is.fail()) return def;
	else return TVector3<T>(x, y, z);
}
template TVector3<double> Str_Vector3(const std::string &s, const TVector3<double> &def);
template TVector3<int> Str_Vector3(const std::string &s, const TVector3<int> &def);

template<typename T>
TVector4<T> Str_Vector4(const std::string &s, const TVector4<T> &def) {
	T x, y, z, w;
	std::istringstream is(s);
	is >> x >> y >> z >> w;
	if (is.fail()) return def;
	else return TVector4<T>(x, y, z, w);
}
template TVector4<double> Str_Vector4(const std::string &s, const TVector4<double> &def);
template TVector4<int> Str_Vector4(const std::string &s, const TVector4<int> &def);


sf::Color Str_ColorN(const std::string &s, const sf::Color &def) {
	float r, g, b, a;
	std::istringstream is(s);
	is >> r >> g >> b >> a;
	if (is.fail()) return def;
	else return sf::Color(r * 255, g * 255, b * 255, a * 255);
}

TColor3 Str_Color3N(const std::string &s, const TColor3 &def) {
	int r, g, b;
	std::istringstream is(s);
	is >> r >> g >> b;
	if (is.fail()) return def;
	else return TColor3(r, g, b);
}

void Str_ArrN(const std::string &s, float *arr, std::size_t count, float def) {
	std::istringstream is(s);
	for (std::size_t i = 0; i < count; i++)
		is >> arr[i];
	if (is.fail())
		for (std::size_t i=0; i<count; i++) arr[i] = def;
}

// --------------------------------------------------------------------
//				SP functions for parsing lines
// --------------------------------------------------------------------

static std::string SPItemN(const std::string &s, const std::string &tag) {
	if (s.empty() || tag.empty()) return "";

	std::string tg = '[' + tag + ']';
	std::size_t i = SPosN(s, tg);
	if (i == std::string::npos) return "";
	std::size_t ii = i + tg.size();
	std::string item;
	while (ii < s.size() && s[ii] != '[' && s[ii] != '#') {
		item += s[ii];
		ii++;
	}
	return item;
}

std::string SPStrN(const std::string &s, const std::string &tag, const std::string& def) {
	std::string item = SPItemN(s, tag);
	if (item.empty()) return def;
	STrimN(item);
	return item;
}

std::string SPStrN(const std::string &s, const char* tag, const char* def) {
	std::string item = SPItemN(s, tag);
	if (item.empty()) return def;
	STrimN(item);
	return item;
}

int SPIntN(const std::string &s, const std::string &tag, const int def) {
	return (Str_IntN(SPItemN(s, tag), def));
}

bool SPBoolN(const std::string &s, const std::string &tag, const bool def) {
	std::string item = SPItemN(s, tag);
	STrimN(item);
	return Str_BoolN(item, def);
}

float SPFloatN(const std::string &s, const std::string &tag, const float def) {
	return (Str_FloatN(SPItemN(s, tag), def));
}

template<typename T>
TVector2<T> SPVector2(const std::string &s, const std::string &tag, const TVector2<T>& def) {
	return (Str_Vector2(SPItemN(s, tag), def));
}
template TVector2<int> SPVector2(const std::string &s, const std::string &tag, const TVector2<int>& def);
template TVector2<double> SPVector2(const std::string &s, const std::string &tag, const TVector2<double>& def);

template<typename T>
TVector3<T> SPVector3(const std::string &s, const std::string &tag, const TVector3<T>& def) {
	return (Str_Vector3(SPItemN(s, tag), def));
}
template TVector3<int> SPVector3(const std::string &s, const std::string &tag, const TVector3<int>& def);
template TVector3<double> SPVector3(const std::string &s, const std::string &tag, const TVector3<double>& def);

template<typename T>
TVector4<T> SPVector4(const std::string &s, const std::string &tag, const TVector4<T>& def) {
	return (Str_Vector4(SPItemN(s, tag), def));
}
template TVector4<int> SPVector4(const std::string &s, const std::string &tag, const TVector4<int>& def);
template TVector4<double> SPVector4(const std::string &s, const std::string &tag, const TVector4<double>& def);

sf::Color SPColorN(const std::string &s, const std::string &tag, const sf::Color& def) {
	return (Str_ColorN(SPItemN(s, tag), def));
}

TColor3 SPColor3N(const std::string &s, const std::string &tag, const TColor3& def) {
	return (Str_Color3N(SPItemN(s, tag), def));
}

void SPArrN(const std::string &s, const std::string &tag, float *arr, std::size_t count, float def) {
	Str_ArrN(SPItemN(s, tag), arr, count, def);
}

std::size_t SPPosN(const std::string &s, const std::string &tag) {
	std::string tg = '[' + tag + ']';
	return SPosN(s, tg);
}

// ------------------ add ---------------------------------------------

void SPAddIntN(std::string &s, const std::string &tag, const int val) {
	s += '[';
	s += tag;
	s += ']';
	s += Int_StrN(val);
}

void SPAddFloatN(std::string &s, const std::string &tag, const float val, std::size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += Float_StrN(val, count);
}

void SPAddStrN(std::string &s, const std::string &tag, const std::string &val) {
	s += '[';
	s += tag;
	s += ']';
	s += val;
}

void SPAddVec2N(std::string &s, const std::string &tag, const TVector2d &val, std::size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Float_StrN(val.x, count);
	s += ' ';
	s += Float_StrN(val.y, count);
}

void SPAddVec3N(std::string &s, const std::string &tag, const TVector3d &val, std::size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Float_StrN(val.x, count);
	s += ' ';
	s += Float_StrN(val.y, count);
	s += ' ';
	s += Float_StrN(val.z, count);
}

void SPAddBoolN(std::string &s, const std::string &tag, const bool val) {
	s += '[';
	s += tag;
	s += ']';
	if (val == true) s += "true";
	else s+= "false";
}

// --------------------------------------------------------------------

void SPSetIntN(std::string &s, const std::string &tag, const int val) {
	std::size_t pos = SPPosN(s, tag);
	if (pos != std::string::npos) {
		std::size_t ipos = pos + tag.size() + 2;
		std::string item = SPItemN(s, tag);
		if (item.size() != std::string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, Int_StrN(val));
	} else SPAddIntN(s, tag, val);
}

void SPSetFloatN(std::string &s, const std::string &tag, const float val, std::size_t count) {
	std::size_t pos = SPPosN(s, tag);
	if (pos != std::string::npos) {
		std::size_t ipos = pos + tag.size() + 2;
		std::string item = SPItemN(s, tag);
		if (item.size() != std::string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, Float_StrN(val, count));
	} else SPAddFloatN(s, tag, val, count);
}

void SPSetStrN(std::string &s, const std::string &tag, const std::string &val) {
	std::size_t pos = SPPosN(s, tag);
	if (pos != std::string::npos) {
		std::size_t ipos = pos + tag.size() + 2;
		std::string item = SPItemN(s, tag);
		if (item.size() != std::string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, val);
	} else SPAddStrN(s, tag, val);
}

// --------------------------------------------------------------------
//					class CSPList
// --------------------------------------------------------------------


CSPList::CSPList(bool newlineflag) {
	fnewlineflag = newlineflag;
}

void CSPList::Add(const std::string& line) {
	push_back(line);
}

void CSPList::Add(std::string&& line) {
	push_back(line);
}

void CSPList::Print() const {
	for (const_iterator line = cbegin(); line != cend(); ++line)
		std::cout << *line << std::endl;
}

bool CSPList::Load(const std::string &filepath) {
	std::ifstream tempfile(filepath.c_str());

	if (!tempfile) {
		Message("CSPList::Load - unable to open " + filepath);
		return false;
	} else {
		bool backflag = false;
		std::string line;

		while (getline(tempfile, line)) {
			// delete new line char if in string
			std::size_t npos = line.rfind('\n');
			if (npos != std::string::npos) SDeleteN(line, npos, 1);

			bool valid = true;
			if (line.empty()) valid = false;	// empty line
			else if (line[0] == '#') valid = false;	// comment line

			if (valid) {
				if (!fnewlineflag) {
					if (line[0] == '*' || empty()) Add(line);
					else back() += line;
				} else {
					bool fwdflag = false;
					if (line.back() == '\\') {
						SDeleteN(line, line.length()-1, 1);
						fwdflag = true;
					}

					if (backflag == false) Add(line);
					else back() += line;

					backflag = fwdflag;
				}
			}
		}
		return true;
	}
}

bool CSPList::Load(const std::string& dir, const std::string& filename) {
	return Load(dir + SEP + filename);
}

bool CSPList::Save(const std::string &filepath) const {
	std::ofstream tempfile(filepath.c_str());
	if (!tempfile) {
		Message("CSPList::Save - unable to open " + filepath);
		return false;
	} else {
		for (const_iterator line = cbegin(); line != cend(); ++line) {
			tempfile << *line << '\n';
		}
		return true;
	}
}

bool CSPList::Save(const std::string& dir, const std::string& filename) const {
	return Save(dir + SEP + filename);
}

void CSPList::MakeIndex(std::map<std::string, std::size_t>& index, const std::string &tag) {
	index.clear();
	std::size_t idx = 0;

	for (const_iterator line = cbegin(); line != cend(); ++line) {
		std::string item = SPItemN(*line, tag);
		STrimN(item);
		if (!item.empty()) {
			index[item] = idx;
			idx++;
		}
	}
}
