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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "translation.h"
#include "spx.h"
#include "course.h"

CTranslation Trans;

// if anything is wrong with an translation, the program will fall back
// to these defaults (only the wrong items)
void CTranslation::SetDefaultTranslations() {
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
	texts[25] = "Race aborted";
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
	texts[42] = "file 'options.txt' and read the documentation.";

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

	texts[58] = "Select your player name:";
	texts[59] = "Select a character:";
	texts[60] = "Enter";
	texts[61] = "Register a new player";

	texts[62] = "Highscore list";
	texts[63] = "No entries for this race";
	texts[64] = "Back";

	texts[65] = "Press any key to return to the main menu";

	texts[66] = "Enter a name for the new player and select an avatar:";

	texts[67] = "Loading resources,";
	texts[68] = "please wait ...";

	texts[69] = "Mirror track: Off";
	texts[70] = "Mirror track: On";

	texts[71] = "Light: Sunny";
	texts[72] = "Light: Cloudy";
	texts[73] = "Light: Evening";
	texts[74] = "Light: Night";

	texts[75] = "Snow: No";
	texts[76] = "Snow: A little";
	texts[77] = "Snow: Some";
	texts[78] = "Snow: A lot";

	texts[79] = "Wind: No";
	texts[80] = "Wind: Breeze";
	texts[81] = "Wind: Strong";
	texts[82] = "Wind: Blustery";

	texts[83] = "Randomize settings";

	texts[84] = "Score";
	texts[85] = "Herring";
	texts[86] = "Time";
	texts[87] = "Path length";
	texts[88] = "Average speed";
	texts[89] = "Position";
	texts[90] = "in highscore list";

	texts[91] = "Author";

	texts[92] = "Loading courses failed.";
	texts[93] = "Loading characters failed.";
	texts[94] = "Loading environments failed.";
	texts[95] = "Loading terrains failed.";
	texts[96] = "Loading avatars failed.";

	texts[97] = "herrings";
	texts[98] = "sec";

	texts[99] = "1st";
	texts[100] = "2nd";
	texts[101] = "3rd";
	texts[102] = "4th";
	texts[103] = "5th";
	texts[104] = "6th";
	texts[105] = "7th";
	texts[106] = "8th";
	texts[107] = "9th";
	texts[108] = "10th";

	texts[109] = "Unknown";
	texts[110] = "auto";
}

const sf::String& CTranslation::Text(std::size_t idx) const {
	static const sf::String empty;
	if (idx >= NUM_COMMON_TEXTS) return empty;
	return texts[idx];
}

static std::wstring UnicodeStr(const std::string& s) {
	std::size_t len = s.length();
	std::wstring res;
	res.resize(len);

	for (std::size_t i = 0, j = 0; i < len; ++i, ++j) {
		wchar_t ch = (unsigned char)s[i];
		if (ch >= 0xF0) {
			ch = (wchar_t)(s[i] & 0x07) << 18;
			ch |= (wchar_t)(s[++i] & 0x3F) << 12;
			ch |= (wchar_t)(s[++i] & 0x3F) << 6;
			ch |= (wchar_t)(s[++i] & 0x3F);
		} else if (ch >= 0xE0) {
			ch = (wchar_t)(s[i] & 0x0F) << 12;
			ch |= (wchar_t)(s[++i] & 0x3F) << 6;
			ch |= (wchar_t)(s[++i] & 0x3F);
		} else if (ch >= 0xC0) {
			ch = (wchar_t)(s[i] & 0x1F) << 6;
			ch |= (wchar_t)(s[++i] & 0x3F);
		}
		res[j] = ch;
	}
	return res;
}

void CTranslation::LoadLanguages() {
	CSPList list;

	if (!list.Load(param.trans_dir, "languages.lst")) {
		Message("could not load language list");
		return;
	}

	languages.resize(list.size()+1);
	languages[0].lang = "en_GB";
	languages[0].language = "English";
	std::size_t i = 1;
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
		languages[i].lang = SPStrN(*line, "lang", "en_GB");
		languages[i].language = UnicodeStr(SPStrN(*line, "language", "English"));
	}

	if (param.language == std::string::npos)
		param.language = GetSystemDefaultLangIdx();
}

const sf::String& CTranslation::GetLanguage(std::size_t idx) const {
	static const sf::String error = "error";
	if (idx >= languages.size()) return error;
	return languages[idx].language;
}

void CTranslation::LoadTranslations(std::size_t langidx) {
	SetDefaultTranslations();
	if (langidx == 0 || langidx >= languages.size()) return;

	CSPList list;
	std::string filename = languages[langidx].lang + ".lst";
	if (!list.Load(param.trans_dir, filename)) {
		Message("could not load translations list:", filename);
		return;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		int idx = SPIntN(*line, "idx", -1);
		if (idx >= 0 && idx < NUM_COMMON_TEXTS) {
			texts[idx] = UnicodeStr(SPStrN(*line, "trans", texts[idx]));
		}
	}
}

void CTranslation::ChangeLanguage(std::size_t langidx) {
	LoadTranslations(langidx);

	// The course description and name translations are stored in the course files.
	Course.FreeCourseList();
	Course.LoadCourseList();
}

std::string CTranslation::GetSystemDefaultLang() {
#ifdef _WIN32
	wchar_t buf[10] = {0};
	GetUserDefaultLocaleName(buf, 10);
	char buf2[10] = {0};
	WideCharToMultiByte(CP_ACP, 0, buf, -1, buf2, 10, nullptr, nullptr);
	std::string ret = buf2;
	while (ret.find('-') != std::string::npos)
		ret[ret.find('-')] = '_';
	return ret;
#else
	return "";
#endif
}

std::size_t CTranslation::GetSystemDefaultLangIdx() const {
	std::string name = GetSystemDefaultLang();
	return GetLangIdx(name);
}

std::size_t CTranslation::GetLangIdx(const std::string& lang) const {
	for (std::size_t i = 0; i < languages.size(); i++)
		if (languages[i].lang == lang)
			return i;
	return 0;
}
