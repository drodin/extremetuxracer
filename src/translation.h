/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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

#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "bh.h"
#include <vector>

#define NUM_COMMON_TEXTS 111

/* --------------------------------------------------------------------
Name convention:
"lang" means the short identifier, e.g. "en_GB"
"language" means the language name, e.g. "English"
---------------------------------------------------------------------*/

struct TLang {
	std::string lang;
	sf::String language;
};

class CTranslation {
private:
	sf::String texts[NUM_COMMON_TEXTS];
public:
	std::vector<TLang> languages;

	void LoadLanguages();
	const sf::String& GetLanguage(std::size_t idx) const;
	void SetDefaultTranslations();
	const sf::String& Text(std::size_t idx) const;
	void LoadTranslations(std::size_t langidx);
	void ChangeLanguage(std::size_t langidx);
	static std::string GetSystemDefaultLang();
	std::size_t GetSystemDefaultLangIdx() const;
	std::size_t GetLangIdx(const std::string& lang) const;
};

extern CTranslation Trans;


#endif
