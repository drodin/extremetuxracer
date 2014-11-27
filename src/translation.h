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

#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "bh.h"
#include <vector>

#define MAX_LANGUAGES 32
#define NUM_COMMON_TEXTS 97
#define MAX_COMMON_TEXT_LINES NUM_COMMON_TEXTS*2

/* --------------------------------------------------------------------
Name convention:
"lang" means the short identifier, e.g. "en_GB"
"language" means the language name, e.g. "English"
---------------------------------------------------------------------*/

struct TLang {
	string lang;
	sf::String language;
};

class CTranslation {
private:
	sf::String texts[NUM_COMMON_TEXTS];
public:
	vector<TLang> languages;

	void LoadLanguages();
	const sf::String& GetLanguage(size_t idx) const;
	void SetDefaultTranslations();
	const sf::String& Text(size_t idx) const;
	void LoadTranslations(size_t langidx);
	void ChangeLanguage(size_t langidx);
	static string GetSystemDefaultLang();
	size_t GetSystemDefaultLangIdx() const;
	size_t GetLangIdx(const string& lang) const;
};

extern CTranslation Trans;


#endif
