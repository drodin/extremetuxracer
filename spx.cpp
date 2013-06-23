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

string MakePathStr (const string& src, const string& add) {
	string res = src;
	res += SEP;
	res += add;
	return res;
}

void SInsertN (string &s, size_t pos, const string& ins) {
	if (pos > s.size()) pos = s.size();
	s.insert (pos, ins);
}

void SDeleteN (string &s, size_t pos, size_t count) {
	if (pos > s.size()) pos = s.size();
	s.erase (pos, count);
}

size_t SPosN (const string &s, const string& find) {
	return s.find (find);
}

void STrimLeftN (string &s) {
	size_t i = s.find_first_not_of(" \t");
	if (i > 0)
		SDeleteN (s, 0, i);
}

void STrimRightN (string &s) {
	size_t i = s.find_last_not_of(" \t");
	if(i != s.size()-1)
		s.erase (i+1);
}

void STrimN (string &s) {
	STrimLeftN (s);
	STrimRightN (s);
}

// --------------------------------------------------------------------
//				conversion functions
// --------------------------------------------------------------------

void Int_StrN (string &s, const int val) {
	ostringstream os;
	os << val;
	s = os.str();
}

string Int_StrN (const int val) {
	ostringstream os;
	os << val;
	return os.str();
}

string Int_StrN (const int val, const streamsize count) {
	ostringstream os;
	os << setw(count) << setfill('0') << val;
	return os.str();
}

void Float_StrN (string &s, const float val, const streamsize count) {
	ostringstream os;
	os << setprecision(count) << fixed << val;
	s = os.str();
}

string Float_StrN (const float val, const streamsize count) {
	ostringstream os;
	os << setprecision(count) << fixed << val;
	return os.str();
}

string Vector_StrN (const TVector3& v, const streamsize count) {
	string res = Float_StrN (v.x, count);
	res += ' ' + Float_StrN (v.y, count);
	res += ' ' + Float_StrN (v.z, count);
	return res;
}

int Str_IntN (const string &s, const int def) {
	int val;
	istringstream is(s);
	is >> val;
	if (is.fail()) return def; else return val;
}

bool Str_BoolN (const string &s, const bool def) {
	if(s == "0" || s == "false")
		return false;
	if(s == "1" || s == "true")
		return true;
	return Str_IntN(s, (int)def) != 0; // Try to parse as int
}

float Str_FloatN (const string &s, const float def) {
	float val;
	istringstream is(s);
	is >> val;
	if (is.fail()) return def; else return val;
}

TVector2 Str_Vector2N (const string &s, const TVector2 &def) {
	float x, y;
	istringstream is(s);
	is >> x >> y;
	if (is.fail()) return def;
	else return TVector2 (x, y);
}

TVector3 Str_Vector3N (const string &s, const TVector3 &def) {
	float x, y, z;
	istringstream is(s);
	is >> x >> y >> z;
	if (is.fail()) return def;
	else return TVector3 (x, y, z);
}

TIndex3 Str_Index3N (const string &s, const TIndex3 &def) {
	int i, j, k;
	istringstream is(s);
	is >> i >> j >> k;
	if (is.fail()) return def;
	else return TIndex3 (i, j, k);
}

TVector4 Str_Vector4N (const string &s, const TVector4 &def) {
	float x, y, z, w;
	istringstream is(s);
	is >> x >> y >> z >> w;
	if (is.fail()) return def;
	else return TVector4 (x, y, z, w);
}

TColor Str_ColorN (const string &s, const TColor &def) {
	float r, g, b, a;
	istringstream is(s);
	is >> r >> g >> b >> a;
	if (is.fail()) return def;
	else return TColor(r, g, b, a);
}

TColor3 Str_Color3N (const string &s, const TColor3 &def) {
	float r, g, b;
	istringstream is(s);
	is >> r >> g >> b;
	if (is.fail()) return def;
	else return TColor3(r, g, b);
}

void Str_ArrN (const string &s, float *arr, size_t count, float def) {
	istringstream is(s);
	for(size_t i = 0; i < count; i++)
		is >> arr[i];
	if (is.fail())
		for (size_t i=0; i<count; i++) arr[i] = def;
}

string Bool_StrN (const bool val) {
	if (val == true) return "true"; else return "false";
}

// --------------------------------------------------------------------
//				SP functions for parsing lines
// --------------------------------------------------------------------

string SPItemN (const string &s, const string &tag) {
	if (s.empty() || tag.empty()) return "";

	string tg = '[' + tag + ']';
	size_t i = SPosN (s, tg);
	if (i == string::npos) return "";
	size_t ii = i + tg.size();
	string item;
	while (ii < s.size() && s[ii] != '[' && s[ii] != '#') {
		item += s[ii];
		ii++;
	}
	return item;
}

string SPStrN (const string &s, const string &tag, const string& def) {
	string item = SPItemN (s, tag);
	if (item.empty()) return def;
	STrimN (item);
	return item;
}

int SPIntN (const string &s, const string &tag, const int def) {
	return (Str_IntN (SPItemN (s, tag), def));
}

bool SPBoolN (const string &s, const string &tag, const bool def) {
	string item = SPItemN (s, tag);
	STrimN (item);
	return Str_BoolN (item, def);
}

float SPFloatN (const string &s, const string &tag, const float def) {
	return (Str_FloatN (SPItemN (s, tag), def));
}

TVector2 SPVector2N (const string &s, const string &tag, const TVector2& def) {
	return (Str_Vector2N (SPItemN (s, tag), def));
}

TVector3 SPVector3N (const string &s, const string &tag, const TVector3& def) {
	return (Str_Vector3N (SPItemN (s, tag), def));
}

TIndex3 SPIndex3N (const string &s, const string &tag, const TIndex3& def) {
	return (Str_Index3N (SPItemN (s, tag), def));
}

TVector4 SPVector4N (const string &s, const string &tag, const TVector4& def) {
	return (Str_Vector4N (SPItemN (s, tag), def));
}

TColor SPColorN (const string &s, const string &tag, const TColor& def) {
	return (Str_ColorN (SPItemN (s, tag), def));
}

TColor3 SPColor3N (const string &s, const string &tag, const TColor3& def) {
	return (Str_Color3N (SPItemN (s, tag), def));
}

void SPArrN (const string &s, const string &tag, float *arr, size_t count, float def) {
	Str_ArrN (SPItemN (s, tag), arr, count, def);
}

bool SPExistsN  (const string &s, const string &tag) {
	string tg = '[' + tag + ']';
	size_t i = SPosN (s, tg);
	return i != string::npos;
}

size_t SPPosN (const string &s, const string &tag) {
	string tg = '[' + tag + ']';
	return SPosN (s, tg);
}

// ------------------ add ---------------------------------------------

void SPAddIntN (string &s, const string &tag, const int val) {
	s += '[';
	s += tag;
	s += ']';
	s += Int_StrN (val);
}

void SPAddFloatN (string &s, const string &tag, const float val, size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += Float_StrN (val, count);
}

void SPAddStrN (string &s, const string &tag, const string &val) {
	s += '[';
	s += tag;
	s += ']';
	s += val;
}

void SPAddVec2N (string &s, const string &tag, const TVector2 &val, size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Float_StrN (val.x, count);
	s += ' ';
	s += Float_StrN (val.y, count);
}

void SPAddVec3N (string &s, const string &tag, const TVector3 &val, size_t count) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Float_StrN (val.x, count);
	s += ' ';
	s += Float_StrN (val.y, count);
	s += ' ';
	s += Float_StrN (val.z, count);
}

void SPAddIndx3N  (string &s, const string &tag, const TIndex3 &val) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Int_StrN (val.i);
	s += ' ';
	s += Int_StrN (val.j);
	s += ' ';
	s += Int_StrN (val.k);
}

void SPAddIndx4N  (string &s, const string &tag, const TIndex4 &val) {
	s += '[';
	s += tag;
	s += ']';
	s += ' ';
	s += Int_StrN (val.i);
	s += ' ';
	s += Int_StrN (val.j);
	s += ' ';
	s += Int_StrN (val.k);
	s += ' ';
	s += Int_StrN (val.l);
}

void SPAddBoolN (string &s, const string &tag, const bool val) {
	s += '[';
	s += tag;
	s += ']';
	if (val == true) s += "true"; else s+= "false";
}

// --------------------------------------------------------------------

void SPSetIntN (string &s, const string &tag, const int val) {
	size_t pos = SPPosN (s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN (s, tag);
		if (item.size() != string::npos) SDeleteN (s, ipos, item.size());
		SInsertN (s, ipos, Int_StrN (val));
	} else SPAddIntN (s, tag, val);
}

void SPSetFloatN (string &s, const string &tag, const float val, size_t count) {
	size_t pos = SPPosN (s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN (s, tag);
		if (item.size() != string::npos) SDeleteN (s, ipos, item.size());
		SInsertN (s, ipos, Float_StrN (val, count));
	} else SPAddFloatN (s, tag, val, count);
}

void SPSetStrN (string &s, const string &tag, const string &val) {
	size_t pos = SPPosN (s, tag);
	if (pos != string::npos) {
		size_t ipos = pos + tag.size() + 2;
		string item = SPItemN (s, tag);
		if (item.size() != string::npos) SDeleteN (s, ipos, item.size());
		SInsertN (s, ipos, val);
	} else SPAddStrN (s, tag, val);
}

// --------------------------------------------------------------------
//					class CSPList
// --------------------------------------------------------------------


CSPList::CSPList (size_t maxlines, bool newlineflag) {
	fmax = maxlines;
	fnewlineflag = newlineflag;
}

const string& CSPList::Line (size_t idx) const {
	if (idx >= flines.size()) return emptyString;
	return flines[idx].first;
}

void CSPList::Clear () {
	flines.clear();
}

void CSPList::Add (const string& line) {
	if (flines.size() < fmax) {
		flines.push_back(make_pair(line, 0));
	}
}

void CSPList::Add (const string& line, int flag) {
	if (flines.size() < fmax) {
		flines.push_back(make_pair(line, flag));
	}
}

void CSPList::Append (const string& line, size_t idx) {
	if (idx >= flines.size()) return;
	flines[idx].first += line;
}

void CSPList::SetFlag (size_t idx, int flag) {
	if (idx >= flines.size()) return;
	flines[idx].second= flag;
}

int CSPList::Flag (size_t idx) const {
	if (idx >= flines.size()) return 0;
	return flines[idx].second;
}

void CSPList::Print () const {
	for (size_t i=0; i<flines.size(); i++) cout << flines[i].first << endl;
}

bool CSPList::Load (const string &filepath) {
	std::ifstream tempfile(filepath.c_str());
	string line;

	bool backflag = false;
	if (!tempfile) {
		Message ("CSPList::Load - unable to open " + filepath, "");
		return false;
	} else {
		while (getline(tempfile, line)) {

			// delete new line char if in string
			size_t npos = line.rfind ('\n');
			if (npos >= 0) SDeleteN (line, npos, 1);

			bool valid = true;
			if (line.empty()) valid = false;	// empty line
			else if (line[0] == '#') valid = false;	// comment line

			if (valid) {
				if (flines.size() < fmax) {
					if (!fnewlineflag) {
						if (line[0] == '*' || flines.empty()) Add (line);
						else Append (line, flines.size()-1);
					} else {
						bool fwdflag;
						if (line[line.length()-1] == '\\') {
							SDeleteN (line, line.length()-1, 1);
							fwdflag = true;
						} else {
							fwdflag = false;
						}

						if (backflag == false) Add (line);
						else Append (line, flines.size()-1);

						backflag = fwdflag;
					}
				} else {
					Message ("CSPList::Load - not enough lines","");
					return false;
				}
			}
		}
		return true;
	}
}

bool CSPList::Load (const string& dir, const string& filename) {
	return Load (dir + SEP + filename);
}

bool CSPList::Save (const string &filepath) const {
	std::ofstream tempfile(filepath.c_str());
	if (!tempfile) {
		Message ("CSPList::Save - unable to open " + filepath, "");
		return false;
	} else {
		for (size_t i=0; i<flines.size(); i++) {
			tempfile << flines[i].first << '\n';
		}
		return true;
	}
}

bool CSPList::Save (const string& dir, const string& filename) const {
	return Save (dir + SEP + filename);
}

void CSPList::MakeIndex (map<string, size_t>& index, const string &tag) {
	index.clear();
	size_t idx = 0;

	for (size_t i=0; i<flines.size(); i++) {
		string item = SPItemN (flines[i].first, tag);
		STrimN (item);
		if (!item.empty()) {
			index[item] = idx;
			idx++;
		}
	}
}
