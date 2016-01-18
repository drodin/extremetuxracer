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

const string emptyString = "";
const string errorString = "error";

// --------------------------------------------------------------------
//					elementary string functions
// --------------------------------------------------------------------

string MakePathStr(const string& src, const string& add) {
	string res = src;
	res += SEP;
	res += add;
	return res;
}

void SInsertN(string &s, size_t pos, const string& ins) {
	if (pos > s.size()) pos = s.size();
	s.insert(pos, ins);
}

void SDeleteN(string &s, size_t pos, size_t count) {
	if (pos > s.size()) pos = s.size();
	s.erase(pos, count);
}

size_t SPosN(const string &s, const string& find) {
	return s.find(find);
}

void STrimLeftN(string &s) {
	size_t i = s.find_first_not_of(" \t");
	if (i > 0)
		SDeleteN(s, 0, i);
}

void STrimRightN(string &s) {
	size_t i = s.find_last_not_of(" \t");
	if (i != s.size()-1)
		s.erase(i+1);
}

void STrimN(string &s) {
	STrimLeftN(s);
	STrimRightN(s);
}

// --------------------------------------------------------------------
//				conversion functions
// --------------------------------------------------------------------

string Int_StrN(const int val) {
	return std::to_string(val);
}

string Int_StrN(const int val, const streamsize count) {
	ostringstream os;
	os << setw(count) << setfill('0') << val;
	return os.str();
}

string Float_StrN(const float val, const streamsize count) {
	ostringstream os;
	os << setprecision(count) << fixed << val;
	return os.str();
}

string Vector_StrN(const TVector3d& v, const streamsize count) {
	string res = Float_StrN(v.x, count);
	res += ' ' + Float_StrN(v.y, count);
	res += ' ' + Float_StrN(v.z, count);
	return res;
}

string Bool_StrN(const bool val) {
	if (val == true) return "true";
	else return "false";
}

int Str_IntN(const string &s, const int def) {
	int val;
	istringstream is(s);
	is >> val;
	if (is.fail()) return def;
	else return val;
}

bool Str_BoolN(const string &s, const bool def) {
	if (s == "0" || s == "false")
		return false;
	if (s == "1" || s == "true")
		return true;
	return Str_IntN(s, (int)def) != 0; // Try to parse as int
}

float Str_FloatN(const string &s, const float def) {
	float val;
	istringstream is(s);
	is >> val;
	if (is.fail()) return def;
	else return val;
}

template<typename T>
TVector2<T> Str_Vector2(const string &s, const TVector2<T> &def) {
	T x, y;
	istringstream is(s);
	is >> x >> y;
	if (is.fail()) return def;
	else return TVector2<T>(x, y);
}
template TVector2<double> Str_Vector2(const string &s, const TVector2<double> &def);
template TVector2<int> Str_Vector2(const string &s, const TVector2<int> &def);

template<typename T>
TVector3<T> Str_Vector3(const string &s, const TVector3<T> &def) {
	T x, y, z;
	istringstream is(s);
	is >> x >> y >> z;
	if (is.fail()) return def;
	else return TVector3<T>(x, y, z);
}
template TVector3<double> Str_Vector3(const string &s, const TVector3<double> &def);
template TVector3<int> Str_Vector3(const string &s, const TVector3<int> &def);

template<typename T>
TVector4<T> Str_Vector4(const string &s, const TVector4<T> &def) {
	T x, y, z, w;
	istringstream is(s);
	is >> x >> y >> z >> w;
	if (is.fail()) return def;
	else return TVector4<T>(x, y, z, w);
}
template TVector4<double> Str_Vector4(const string &s, const TVector4<double> &def);
template TVector4<int> Str_Vector4(const string &s, const TVector4<int> &def);


sf::Color Str_ColorN(const string &s, const sf::Color &def) {
	float r, g, b, a;
	istringstream is(s);
	is >> r >> g >> b >> a;
	if (is.fail()) return def;
	else return sf::Color(r * 255, g * 255, b * 255, a * 255);
}

TColor3 Str_Color3N(const string &s, const TColor3 &def) {
	int r, g, b;
	istringstream is(s);
	is >> r >> g >> b;
	if (is.fail()) return def;
	else return TColor3(r, g, b);
}

void Str_ArrN(const string &s, float *arr, size_t count, float def) {
	istringstream is(s);
	for (size_t i = 0; i < count; i++)
		is >> arr[i];
	if (is.fail())
		for (size_t i=0; i<count; i++) arr[i] = def;
}

// --------------------------------------------------------------------
//				SP functions for parsing lines
// --------------------------------------------------------------------

static string SPItemN(const string &s, const string &tag) {
	if (s.empty() || tag.empty()) return "";

	string tg = '[' + tag + ']';
	size_t i = SPosN(s, tg);
	if (i == string::npos) return "";
	size_t ii = i + tg.size();
	string item;
	while (ii < s.size() && s[ii] != '[' && s[ii] != '#') {
		item += s[ii];
		ii++;
	}
	return item;
}

string SPStrN(const string &s, const string &tag, const string& def) {
	string item = SPItemN(s, tag);
	if (item.empty()) return def;
	STrimN(item);
	return item;
}

string SPStrN(const string &s, const char* tag, const char* def) {
	string item = SPItemN(s, tag);
	if (item.empty()) return def;
	STrimN(item);
	return item;
}

int SPIntN(const string &s, const string &tag, const int def) {
	return (Str_IntN(SPItemN(s, tag), def));
}

bool SPBoolN(const string &s, const string &tag, const bool def) {
	string item = SPItemN(s, tag);
	STrimN(item);
	return Str_BoolN(item, def);
}

float SPFloatN(const string &s, const string &tag, const float def) {
	return (Str_FloatN(SPItemN(s, tag), def));
}

template<typename T>
TVector2<T> SPVector2(const string &s, const string &tag, const TVector2<T>& def) {
	return (Str_Vector2(SPItemN(s, tag), def));
}
template TVector2<int> SPVector2(const string &s, const string &tag, const TVector2<int>& def);
template TVector2<double> SPVector2(const string &s, const string &tag, const TVector2<double>& def);

template<typename T>
TVector3<T> SPVector3(const string &s, const string &tag, const TVector3<T>& def) {
	return (Str_Vector3(SPItemN(s, tag), def));
}
template TVector3<int> SPVector3(const string &s, const string &tag, const TVector3<int>& def);
template TVector3<double> SPVector3(const string &s, const string &tag, const TVector3<double>& def);

template<typename T>
TVector4<T> SPVector4(const string &s, const string &tag, const TVector4<T>& def) {
	return (Str_Vector4(SPItemN(s, tag), def));
}
template TVector4<int> SPVector4(const string &s, const string &tag, const TVector4<int>& def);
template TVector4<double> SPVector4(const string &s, const string &tag, const TVector4<double>& def);

sf::Color SPColorN(const string &s, const string &tag, const sf::Color& def) {
	return (Str_ColorN(SPItemN(s, tag), def));
}

TColor3 SPColor3N(const string &s, const string &tag, const TColor3& def) {
	return (Str_Color3N(SPItemN(s, tag), def));
}

void SPArrN(const string &s, const string &tag, float *arr, size_t count, float def) {
	Str_ArrN(SPItemN(s, tag), arr, count, def);
}

size_t SPPosN(const string &s, const string &tag) {
	string tg = '[' + tag + ']';
	return SPosN(s, tg);
}

// ------------------ add ---------------------------------------------

void SPAddIntN(string &s, const string &tag, const int val) {
	s += '[';
	s += tag;
	s += ']';
	s += Int_StrN(val);
}

void SPAddFloatN(string &s, const string &tag, const float val, size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += Float_StrN(val, count);
}

void SPAddStrN(string &s, const string &tag, const string &val) {
	s += '[';
	s += tag;
	s += ']';
	s += val;
}

void SPAddVec2N(string &s, const string &tag, const TVector2d &val, size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Float_StrN(val.x, count);
	s += ' ';
	s += Float_StrN(val.y, count);
}

void SPAddVec3N(string &s, const string &tag, const TVector3d &val, size_t count) {
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

void SPAddBoolN(string &s, const string &tag, const bool val) {
	s += '[';
	s += tag;
	s += ']';
	if (val == true) s += "true";
	else s+= "false";
}

// --------------------------------------------------------------------

void SPSetIntN(string &s, const string &tag, const int val) {
	size_t pos = SPPosN(s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN(s, tag);
		if (item.size() != string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, Int_StrN(val));
	} else SPAddIntN(s, tag, val);
}

void SPSetFloatN(string &s, const string &tag, const float val, size_t count) {
	size_t pos = SPPosN(s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN(s, tag);
		if (item.size() != string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, Float_StrN(val, count));
	} else SPAddFloatN(s, tag, val, count);
}

void SPSetStrN(string &s, const string &tag, const string &val) {
	size_t pos = SPPosN(s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN(s, tag);
		if (item.size() != string::npos) SDeleteN(s, ipos, item.size());
		SInsertN(s, ipos, val);
	} else SPAddStrN(s, tag, val);
}

// --------------------------------------------------------------------
//					class CSPList
// --------------------------------------------------------------------


CSPList::CSPList(bool newlineflag) {
	fnewlineflag = newlineflag;
}

void CSPList::Add(const string& line) {
	push_back(line);
}

void CSPList::Add(string&& line) {
	push_back(line);
}

void CSPList::Print() const {
	for (const_iterator line = cbegin(); line != cend(); ++line)
		cout << *line << endl;
}

bool CSPList::Load(const string &filepath) {
	std::ifstream tempfile(filepath.c_str());

	if (!tempfile) {
		Message("CSPList::Load - unable to open " + filepath);
		return false;
	} else {
		bool backflag = false;
		string line;

		while (getline(tempfile, line)) {
			// delete new line char if in string
			size_t npos = line.rfind('\n');
			if (npos != string::npos) SDeleteN(line, npos, 1);

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
			} else {
				Message("CSPList::Load - not enough lines");
				return false;
			}
		}
		return true;
	}
}

bool CSPList::Load(const string& dir, const string& filename) {
	return Load(dir + SEP + filename);
}

bool CSPList::Save(const string &filepath) const {
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

bool CSPList::Save(const string& dir, const string& filename) const {
	return Save(dir + SEP + filename);
}

void CSPList::MakeIndex(map<string, size_t>& index, const string &tag) {
	index.clear();
	size_t idx = 0;

	for (const_iterator line = cbegin(); line != cend(); ++line) {
		string item = SPItemN(*line, tag);
		STrimN(item);
		if (!item.empty()) {
			index[item] = idx;
			idx++;
		}
	}
}
