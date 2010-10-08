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

typedef struct{
	string language; 
	string name;
} TLanguage;

class CTranslation {
private:	
	/// the map for storing the translation strings
	map < string, string > m_translations;	
	list < TLanguage > m_languages;	
public:
	CTranslation ();
	
	void Lload (const char *language);
	const char* getTranslation (const char *string);
	void setTranslation (const char *language, const char *name);
	void addLanguage (const char *language, const char *name);
	bool getLanguages ();
	list < TLanguage > LanguageList () {return m_languages;}
};

extern CTranslation Trans;

/// a gettext like macro that returns the translated string
#define _(string) translation.getTranslation (string)

//void register_translation_callbacks( Tcl_Interp *ip );


#endif 


