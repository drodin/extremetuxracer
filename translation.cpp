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

CTranslation::CTranslation () {
	int i;
	for (i=0; i<MAX_LANGUAGES; i++) {
		languages[i].lang = "";
		languages[i].language = "";
	}
	numLanguages = 0;
	LangIndex = "";
	languages_ok = false;
	for (i=0; i<MAX_COMMON_TEXTS; i++) texts[i] = "";
}
// if anything is wrong with an translation, the program will fall back
// to these defaults (only the wrong items)
void CTranslation::SetDefaultTranslations () {
	texts[0] = "Press any key to start";
	texts[1] = "Enter an event";
	texts[2] = "Practice";
	texts[3] = "Configuration";
	texts[4] = "Credits";
	texts[5] = "Quit";
	texts[6] = "Select an event";
	texts[7] = "Select a cup";
	texts[8] = "Back";
	texts[9] = "Continue";
	texts[10] = "You cannot enter this cup yet";
	texts[11] = "Herring:";
	texts[12] = "Time:";
	texts[13] = "Race!";
	texts[14] = "seconds";
	texts[15] = "Ok";
	texts[16] = "Congratulations! You won the cup!";
	texts[17] = "You have reached level";
	texts[18] = "Sorry, you didn't advance";
	texts[19] = "You don't have any lives left";
	texts[20] = "Select a race";
	texts[21] = "Failed, -1 Tuxlive";
	texts[22] = "Success, +/- 0 Tuxlive";
	texts[23] = "Success, +1 Tuxlive";
	texts[24] = "Success, +2 Tuxlive";
	texts[25] = "Race aborted [trans]";
	texts[26] = "Score:";
	texts[27] = "points";

	texts[28] = "Cancel"; 
	texts[29] = "Loading";
	texts[30] = "Please wait ...";
	texts[31] = "Fullscreen:";
	texts[32] = "Resolution:";
	texts[33] = "Music volume:";
	texts[34] = "Sound volume:";
	texts[35] = "Language:";
	texts[36] = "Level of detail:";
	texts[37] = "Contributed by:";
	texts[38] = "Event:";
	texts[39] = "Cup:";
	texts[40] = "Race Over";
	
	texts[41] = "For more configuration options, please edit the";
	texts[42] = "file 'options.lst' and read the documentation.";
}

string CTranslation::Text (int idx) {
	if (idx < 0 || idx >= MAX_COMMON_TEXTS) return "";
	return texts[idx];
}

void CTranslation::LoadLanguages () {
	CSPList list (MAX_LANGUAGES);
	int i;
	string line, lang, language;

	languages_ok = false;
	if (!list.Load (param.trans_dir, "languages.lst")) {
		Message ("could not load language list");
		return;
	}

	numLanguages = 0;
	for (i=0; i<list.Count(); i++) {
		line = list.Line(i);
		languages[numLanguages].lang = SPStrN (line, "lang", "en_GB"); 
		languages[numLanguages].language = SPStrN (line, "language", "English"); 
		LangIndex += "[" + languages[numLanguages].lang + "]";
		LangIndex += Int_StrN (numLanguages);
		numLanguages++;
	}
	if (numLanguages > 0) languages_ok = true;
}

int CTranslation::GetLangIdx (string lang) {
	return SPIntN (LangIndex, lang, 0);
}

string CTranslation::GetLanguage (int idx) {
	if (idx < 0 || idx >= numLanguages) return "error";
	return languages[idx].language;
}

string CTranslation::GetLanguage (string lang) {
	return GetLanguage (GetLangIdx (lang));
}

void CTranslation::LoadTranslations (int langidx) {
	CSPList list(MAX_COMMON_TEXTS + 3);
	int i, idx;
	string line;

	SetDefaultTranslations ();
	if (!languages_ok) return;
	if (langidx < 0 || langidx >= numLanguages) return;

	string filename = languages[langidx].lang + ".lst";;
	if (!list.Load (param.trans_dir, filename)) {
		Message ("could not load translations list:", filename.c_str());
		return;
	}

	for (i=0; i<list.Count(); i++) {
		line = list.Line(i);
		idx = SPIntN (line, "idx", -1);
		if (idx >= 0 && idx < MAX_COMMON_TEXTS) {
			texts[idx] = SPStrN (line, "trans", texts[idx]);
		} 
	}



}


/*
const char *CTranslation::getTranslation (const char* string) {
	std::map<std::string, std::string>::iterator it;
	
	if ((it = m_translations.find(string)) != m_translations.end()) {
		return (*it).second.c_str();
	}else{
		return string;
	}
}

void CTranslation::setTranslation (const char* string, const char* translation) {
	m_translations[string] = translation;
}
*/


/*
static int pp_translate_string_cb (ClientData cd, Tcl_Interp *ip,  int argc, CONST84 char *argv[]) {
	if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <string> <translated string>",
			 (char *)0 );
        return TCL_ERROR;
    } 
	
	translation.setTranslation(argv[1],argv[2]);
	
	return TCL_OK;

}
*/


/*
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


	char buff[BUFF_LEN];
	sprintf (buff, "%s/translations/languages.lst", param.trans_dir.c_str());
	if ( Tcl_EvalFile( tclInterp, buff ) != TCL_OK) {
		std::cerr << " error evalating language-settings file " << buff
				<< " : " << Tcl_GetStringResult (tclInterp ) << std::endl;
	}

}


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


void CTranslation::addLanguage (const char* language, const char* name) {
	TLanguage lang;
	lang.language = language;
	lang.name=name;
	m_languages.push_back(lang);
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

