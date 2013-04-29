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
	languages_ok = false;
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

	texts[43] = "Help";
	texts[44] = "1, 2, 3 - change view mode";
	texts[45] = "F - hide/show fps display";
	texts[46] = "H - hide/show hud display";
	texts[47] = "S - screenshot";
	texts[48] = "U - toggle ui snow";
	texts[49] = "P - set pause mode";
	texts[50] = "T - trick";
	texts[51] = "ESC - abort Race";
	texts[52] = "SPACE - jump";
	texts[53] = "CRSR Left - turn left";
	texts[54] = "CRSR Right - turn right";
	texts[55] = "CRSR Up - accelerate";
	texts[56] = "CRSR down - brake";
	texts[57] = "Keyboard functions";
}

string CTranslation::Text (int idx) const {
	if (idx < 0 || idx >= MAX_COMMON_TEXTS) return "";
	return texts[idx];
}

void CTranslation::LoadLanguages () {
	CSPList list (MAX_LANGUAGES);

	languages_ok = false;
	if (!list.Load (param.trans_dir, "languages.lst")) {
		Message ("could not load language list");
		return;
	}

	languages.resize(list.Count());
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line(i);
		languages[i].lang = SPStrN (line, "lang", "en_GB");
		languages[i].language = SPStrN (line, "language", "English");
		LangIndex += "[" + languages[i].lang + "]";
		LangIndex += Int_StrN (i);
	}
	if (languages.size() > 0) languages_ok = true;
}

int CTranslation::GetLangIdx (const string& lang) const {
	return SPIntN (LangIndex, lang, 0);
}

string CTranslation::GetLanguage (int idx) const {
	if (idx < 0 || idx >= languages.size()) return "error";
	return languages[idx].language;
}

string CTranslation::GetLanguage (const string& lang) const {
	return GetLanguage (GetLangIdx (lang));
}

void CTranslation::LoadTranslations (int langidx) {
	SetDefaultTranslations ();
	if (!languages_ok) return;
	if (langidx < 0 || langidx >= languages.size()) return;

	CSPList list(MAX_COMMON_TEXTS + 3);
	string filename = languages[langidx].lang + ".lst";
	if (!list.Load (param.trans_dir, filename)) {
		Message ("could not load translations list:", filename.c_str());
		return;
	}

	for (int i=0; i<list.Count(); i++) {
		string line = list.Line(i);
		int idx = SPIntN (line, "idx", -1);
		if (idx >= 0 && idx < MAX_COMMON_TEXTS) {
			texts[idx] = SPStrN (line, "trans", texts[idx]);
		} 
	}
}
