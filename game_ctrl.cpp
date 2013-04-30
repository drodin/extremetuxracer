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

#include "game_ctrl.h"
#include "spx.h"
#include "course.h"
#include "env.h"
#include "audio.h"
#include "textures.h"

// --------------------------------------------------------------------
//				administration of events and cups
// --------------------------------------------------------------------

CEvents Events;

bool CEvents::LoadEventList () {
	CSPList list(256);

	if (!list.Load (param.common_course_dir, "events.lst")) {
		Message ("could not load events.lst");
		return false;
	}

	// pass 1: races
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		int type = SPIntN (line, "struct", -1);
		if (type == 0) {
			RaceList.push_back(TRace2());
			RaceList.back().race = SPStrN (line, "race", "error");
			string item = SPStrN (line, "course", "");
			RaceList.back().course = Course.GetCourseIdx (item);
			item = SPStrN (line, "light", "");
			RaceList.back().light = Env.GetLightIdx (item);
			RaceList.back().snow = SPIntN (line, "snow", 0);
			RaceList.back().wind = SPIntN (line, "wind", 0);
			RaceList.back().time = SPVector3N (line, "time", MakeVector (0, 0, 0));
			RaceList.back().herrings = SPIndex3N (line, "herring", MakeIndex3 (0, 0, 0));
			RaceList.back().music_theme = Music.GetThemeIdx (SPStrN (line, "theme", "normal"));
		}
	}
	list.MakeIndex (RaceIndex, "race");

	// pass 2: cups
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		int type = SPIntN (line, "struct", -1);
		if (type == 1) {
			CupList.push_back(TCup2());
			CupList.back().cup = SPStrN (line, "cup", "error");
			CupList.back().name = SPStrN (line, "name", "unknown");
			CupList.back().desc = SPStrN (line, "desc", "unknown");
			int num = SPIntN (line, "num", 0);
			CupList.back().num_races = num;
			for (int ii=0; ii<num; ii++) {
				string race = SPStrN (line, Int_StrN (ii+1), "");
				CupList.back().races[ii] = &RaceList[GetRaceIdx(race)];
			}
		}
	}
	list.MakeIndex (CupIndex, "cup");

	// pass 3: events
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		int type = SPIntN (line, "struct", -1);
		if (type == 2) {
			EventList.push_back(TEvent2());
			EventList.back().name = SPStrN (line, "name", "unknown");
			int num = SPIntN (line, "num", 0);
			EventList.back().num_cups = num;
			for (int ii=0; ii<num; ii++) {
				string cup = SPStrN (line, Int_StrN (ii+1), "");
				EventList.back().cups[ii] = &CupList[GetCupIdx(cup)];
			}
		}
	}
	list.MakeIndex (EventIndex, "event");

	return true;
}

int CEvents::GetRaceIdx (const string& race) const {
	return SPIntN (RaceIndex, race, 0);
}

int CEvents::GetCupIdx (const string& cup) const {
	return SPIntN (CupIndex, cup, 0);
}

int CEvents::GetEventIdx (const string& event) const {
	return SPIntN (EventIndex, event, 0);
}

string CEvents::GetCup (size_t event, size_t cup) const {
	if (event < 0 || event >= EventList.size()) return "error";
	if (cup < 0 || cup >= EventList[event].num_cups) return "error";
	return EventList[event].cups[cup]->cup;
}

string CEvents::GetCupTrivialName (size_t event, size_t cup) const {
	if (event < 0 || event >= EventList.size()) return "error";
	if (cup < 0 || cup >= EventList[event].num_cups) return "error";
	return EventList[event].cups[cup]->name;
}

void CEvents::MakeUnlockList (string unlockstr) {
	for (int event=0; event<EventList.size(); event++) {
		for (int cup=0; cup<EventList[event].num_cups; cup++) {
			EventList[event].cups[cup]->Unlocked = false;
		}
	}
	for (int event=0; event<EventList.size(); event++) {
		for (int cup=0; cup<EventList[event].num_cups; cup++) {
			string cp = GetCup (event, cup);
			bool passed = SPosN (unlockstr, cp) != string::npos;
			if (cup < 1) EventList[event].cups[0]->Unlocked = true;
			if (passed) {
				EventList[event].cups[cup]->Unlocked = true;
				if(cup+1 < EventList[event].num_cups)
					EventList[event].cups[cup+1]->Unlocked = true;
			}
		}
	}
}

bool CEvents::IsUnlocked (size_t event, size_t cup) const {
	if (event < 0 || event >= EventList.size()) return false;
	if (cup < 0 || cup >= EventList[event].num_cups) return false;
	return EventList[event].cups[cup]->Unlocked;
}

// --------------------------------------------------------------------
//				player administration
// --------------------------------------------------------------------

CPlayers Players;

CPlayers::CPlayers () {
	currPlayer = 0;
}

void CPlayers::AddPlayer (const string& name, const string& avatar) {
	plyr.push_back(TPlayer());
	plyr.back().name = name;
	plyr.back().avatar = avatar;
	plyr.back().texid = SPIntN (AvatarIndex, plyr.back().avatar, 0);
	plyr.back().funlocked = "";
	plyr.back().ctrl = NULL;
}

void CPlayers::SetDefaultPlayers () {
	plyr.resize(2);
	plyr[0].funlocked = "";
	plyr[0].name = "Racer";
	plyr[0].avatar = "avatar01.png";
	plyr[0].texid = SPIntN (AvatarIndex, plyr[0].avatar, 0);
	plyr[0].ctrl = NULL;

	plyr[1].funlocked = "";
	plyr[1].name = "Bunny";
	plyr[1].avatar = "avatar02.png";
	plyr[1].texid = SPIntN (AvatarIndex, plyr[1].avatar, 0);
	plyr[1].ctrl = NULL;
}

bool CPlayers::LoadPlayers () {
	CSPList list(MAX_PLAYERS);

	if (FileExists (param.config_dir, "players") == false) {
		SetDefaultPlayers ();
		Message ("file 'players' does not exist, set default players");
		return false; 
	}

	if (list.Load (param.config_dir, "players") == false) {
		SetDefaultPlayers ();
		Message ("coule not load players list, set default players");
		return false; 
	}

	g_game.start_player = 0;
	plyr.resize(list.Count());
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line(i);
		plyr[i].name = SPStrN (line, "name", "unknown");
		plyr[i].funlocked = SPStrN (line, "unlocked", "");
		plyr[i].avatar = SPStrN (line, "avatar", "");
		plyr[i].texid = SPIntN (AvatarIndex, plyr[i].avatar, 0);
		plyr[i].ctrl = NULL;
		int active = SPIntN (line, "active", 0);
		if (active > 0) g_game.start_player = plyr.size()-1;
	}
	if (plyr.empty()) {
		SetDefaultPlayers ();
		Message ("player file doesn't contain a player, set default players");
		return false;
	}
	return true;
}

void CPlayers::SavePlayers () {
	string playerfile = param.config_dir + SEP + "players";
	CSPList list(MAX_PLAYERS);
	string item = "";
	for (int i=0; i<plyr.size(); i++) {
		item = "*[name]" + plyr[i].name;
		item +="[avatar]" + plyr[i].avatar;
		item += "[unlocked]" + plyr[i].funlocked;
		if (i == g_game.player_id) item += "[active]1";
			else item += "[active]0";
		list.Add (item);
	}
	list.Save (playerfile);
}

string CPlayers::GetCurrUnlocked (){
	return plyr[currPlayer].funlocked;
}

void CPlayers::AddPassedCup (const string& cup) {
	if (SPIntN (plyr[currPlayer].funlocked, cup, -1) > 0) return;
	plyr[currPlayer].funlocked += " ";
	plyr[currPlayer].funlocked += cup;
}

CControl *CPlayers::GetCtrl () {
	return plyr[currPlayer].ctrl;
}

CControl *CPlayers::GetCtrl (int player) {
	if (player < 0 || player >= plyr.size()) return NULL;
	return plyr[player].ctrl;
}

string CPlayers::GetName (int player) const {
	if (player < 0 || player >= plyr.size()) return "";
	return plyr[player].name;
}

void CPlayers::ResetControls () {
	for (int i=0; i<plyr.size(); i++) {
		if (plyr[i].ctrl != NULL) {
			free (plyr[i].ctrl);
			plyr[i].ctrl = NULL;
		}
	}
}

// called in module regist.cpp:
void CPlayers::AllocControl (int player) {
	if (player < 0 || player >= plyr.size()) return;
	if (plyr[player].ctrl != NULL) return;
	plyr[player].ctrl = new (CControl);
}

// ----------------------- avatars ------------------------------------

void CPlayers::LoadAvatars () {
	CSPList list (MAX_AVATARS);

	if (!list.Load (param.player_dir, "avatars.lst")) {
		Message ("could not load avators.lst");
		return;
	}

	AvatarIndex = "";
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		string filename = SPStrN (line, "file", "unknown");
		GLuint texid = Tex.LoadTexture (param.player_dir, filename);
		if (texid > 0) {
			avatars.push_back(TAvatar());
			avatars.back().filename = filename;
			avatars.back().texid = texid;
			AvatarIndex += "[" + filename + "]";
			AvatarIndex += Int_StrN (texid);
		}
	}
}

GLuint CPlayers::GetAvatarID (size_t player) const {
	if (player < 0 || player >= plyr.size()) return 0;
	return plyr[player].texid;
}

GLuint CPlayers::GetAvatarID (const string& filename) const {
	return SPIntN (AvatarIndex, filename, 0);
}

GLuint CPlayers::GetDirectAvatarID (size_t avatar) const {
	if (avatar < 0 || avatar >= avatars.size()) return 0;
	return avatars[avatar].texid;
}

string CPlayers::GetDirectAvatarName (size_t avatar) const {
	if (avatar < 0 || avatar >= avatars.size()) return "";
	return avatars[avatar].filename;
}

// ********************************************************************
//				Character Administration
// ********************************************************************

CCharacter Char;

CCharacter::CCharacter () {
	curr_character = 0;
}

static string char_type_index = "[spheres]0[3d]1";

void CCharacter::LoadCharacterList () {
	CSPList list (MAX_CHARACTERS);

	if (!list.Load (param.char_dir, "characters.lst")) {
		Message ("could not load characters.lst");
		return;
	}

	CharList.resize(list.Count());
	for (int i=0; i<list.Count(); i++) {
		string line = list.Line (i);
		CharList[i].name = SPStrN (line, "name", "");
		CharList[i].dir = SPStrN (line, "dir", "");
		string typestr = SPStrN (line, "type", "unknown");
		CharList[i].type = SPIntN (char_type_index, typestr, -1);

		string charpath = param.char_dir + SEP + CharList[i].dir;
		if (DirExists (charpath.c_str())) {
			string previewfile = charpath + SEP + "preview.png";
			GLuint texid = Tex.LoadMipmapTexture (previewfile, 0);
			if (texid < 1) {
				Message ("could not load previewfile of character");					
//				texid = Tex.TexID (NO_PREVIEW);
			}
			
			TCharacter* ch = &CharList[i];
			ch->preview = texid;

			ch->shape = new CCharShape;
			if (ch->shape->Load (charpath, "shape.lst", false) == false) {
				free (ch->shape);
				ch->shape = NULL;
				Message ("could not load character shape");
			}

			ch->frames[0].Load (charpath, "start.lst"); 
			ch->finishframesok = true;
			ch->frames[1].Load (charpath, "finish.lst"); 
			if (ch->frames[1].loaded == false) ch->finishframesok = false;
			ch->frames[2].Load (charpath, "wonrace.lst"); 
			if (ch->frames[2].loaded == false) ch->finishframesok = false;
			ch->frames[3].Load (charpath, "lostrace.lst"); 
			if (ch->frames[3].loaded == false) ch->finishframesok = false;
		}
	}
}

void CCharacter::FreeCharacterPreviews () {
	for (int i=0; i<CharList.size(); i++) {
		if (CharList[i].preview > 0) {
			glDeleteTextures (1, &CharList[i].preview);
			CharList[i].preview = 0;
		}
	}
}

void CCharacter::Draw (size_t idx) {
	if (idx < 0 || idx >= CharList.size()) return;
	CharList[idx].shape->Draw ();
}

CCharShape *CCharacter::GetShape (size_t idx) {
	if (idx < 0 || idx >= CharList.size()) return NULL;
	return CharList[idx].shape;
}

CKeyframe *CCharacter::GetKeyframe (size_t idx, TFrameType type) {
	if (type < 0 || type >= NUM_FRAME_TYPES) return NULL;
	if (idx < 0 || idx >= CharList.size()) return NULL;
	return &CharList[idx].frames[type];
}
