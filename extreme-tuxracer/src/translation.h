/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _TRANSLATION_H_
#define _TRANSLATION_H_

#include "etracer.h"

#include <string>
#include <map>
#include <list>


typedef struct{
	std::string language; 
	std::string name;
}language_t;

/// Class for handling translations
class Translation
{
	
	/// the map for storing the translation strings
	std::map<std::string, std::string> m_translations;	
	std::list<language_t> m_languages;
	
public:
	Translation();
	
	void load(const char* language);
	const char* getTranslation(const char* string);
	void setTranslation(const char* language, const char* name);
	void addLanguage(const char* language, const char* name);
	void getLanguages();
	std::list<language_t> LanguageList() {return m_languages;}


};

extern Translation translation;

///a gettext like macro that returns the translated string
#define _(string) translation.getTranslation(string)

void register_translation_callbacks( Tcl_Interp *ip );

#endif // _TRANSLATION_H_
