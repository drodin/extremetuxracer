/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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
/* --------------------------------------------------------------------
An name convention: 
"lang" means the short identifier, e.g. "en_GB"
"language" means the language name, e.g. "English"
---------------------------------------------------------------------*/

#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "bh.h"
#include <vector>

#define MAX_LANGUAGES 32
#define MAX_COMMON_TEXTS 128


struct TLang {
	string lang;
	string language;
};

class CTranslation {
private:
	string texts[MAX_COMMON_TEXTS];
	string LangIndex;
	bool languages_ok;
public:
	vector<TLang> languages;

	CTranslation ();

	void LoadLanguages ();
	size_t GetLangIdx (const string& lang) const;
	string GetLanguage (size_t idx) const;
	string GetLanguage (const string& lang) const;
	void SetDefaultTranslations ();
	string Text (size_t idx) const;
	void LoadTranslations (size_t langidx);
};

extern CTranslation Trans;


#endif
