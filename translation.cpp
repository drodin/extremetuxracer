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

#include "translation.h"
#include "spx.h"

CTranslation Trans;

CTranslation::CTranslation () {}

bool CTranslation::getLanguages () {
	CSPList list(100);
	string line;

	if (!list.Load (param.trans_dir, "languages.lst")) {
		Message ("could not load languages");	
		return false;	
	}

	for (int i=0; i<list.Count(); i++) {
		line = list.Line (i);
	}

	return true;

//	pp_translate_language "de_DE" "Deutsch"

/*
	char buff[BUFF_LEN];
	sprintf (buff, "%s/translations/languages.lst", param.trans_dir.c_str());
	if ( Tcl_EvalFile( tclInterp, buff ) != TCL_OK) {
		std::cerr << " error evalating language-settings file " << buff
				<< " : " << Tcl_GetStringResult (tclInterp ) << std::endl;
	}
*/
}

/*
/// load translation for the specified language into the map
void CTranslation::Load (const char* language) {

	m_translations.clear();		
	char buff[BUFF_LEN];
	sprintf(buff, "%s/translations/%s.tcl", getparam_data_dir(), language);	

	if ( Tcl_EvalFile( tclInterp, buff ) != TCL_OK ) {
		std::cerr << "error evalating language file " << buff
				<< " : " << Tcl_GetStringResult( tclInterp ) << std::endl;
	}	
}

/// return translation for string
const char *CTranslation::getTranslation (const char* string) {
	std::map<std::string, std::string>::iterator it;
	
	if((it=m_translations.find(string))!=m_translations.end()){
		return (*it).second.c_str();
	}else{
		return string;
	}
}

/// set translation string
void CTranslation::setTranslation (const char* string, const char* translation) {
	m_translations[string]=translation;
}

void CTranslation::addLanguage (const char* language, const char* name) {
	TLanguage lang;
	lang.language = language;
	lang.name=name;
	m_languages.push_back(lang);
}

static int pp_translate_string_cb (ClientData cd, Tcl_Interp *ip, 
  int argc, CONST84 char *argv[]) {
	if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <string> <translated string>",
			 (char *)0 );
        return TCL_ERROR;
    } 
	
	translation.setTranslation(argv[1],argv[2]);
	
	return TCL_OK;
}

static int pp_translate_language_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[])
{
	if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <language> <language-name>",
			 (char *)0 );
        return TCL_ERROR;
    } 
	
	translation.addLanguage(argv[1],argv[2]);
	
	return TCL_OK;
}

void register_translation_callbacks( Tcl_Interp *ip ){
    Tcl_CreateCommand (ip, "pp_translate_string", pp_translate_string_cb,   0,0);
    Tcl_CreateCommand (ip, "pp_translate_language", pp_translate_language_cb,   0,0);
}

*/

