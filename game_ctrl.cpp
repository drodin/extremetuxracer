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

CEvents::CEvents () {
	numRaces = 0;
	RaceIndex = "";
	numCups = 0;
	CupIndex = "";
	numEvents = 0;
	EventIndex = "";
}

CEvents::~CEvents () {}

bool CEvents::LoadEventList () {
	CSPList list(256);
	int i, ii, type;
	string line, item, race, cup;
	int num;

	if (!list.Load (param.common_course_dir, "events.lst")) {
		Message ("could not load events.lst");
		return false;
	}

	// pass 1: races
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		type = SPIntN (line, "struct", -1);
		if (type == 0) {
			if (numRaces < MAX_RACES2) {
				RaceList[numRaces].race = SPStrN (line, "race", "error");
				item = SPStrN (line, "course", "");
				RaceList[numRaces].course = Course.GetCourseIdx (item);
				item = SPStrN (line, "light", "");
				RaceList[numRaces].light = Env.GetLightIdx (item);
				RaceList[numRaces].snow = SPIntN (line, "snow", 0);
				RaceList[numRaces].wind = SPIntN (line, "wind", 0);
				RaceList[numRaces].time = SPVector3N (line, "time", MakeVector (0, 0, 0));
				RaceList[numRaces].herrings = SPIndex3N (line, "herring", MakeIndex3 (0, 0, 0));
				RaceList[numRaces].music_theme = Music.GetThemeIdx (SPStrN (line, "theme", "normal"));
				numRaces++;
			}
		}
	}
	list.MakeIndex (RaceIndex, "race");

	// pass 2: cups
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		type = SPIntN (line, "struct", -1);
		if (type == 1) {
			if (numCups < MAX_CUPS2) {
				CupList[numCups].cup = SPStrN (line, "cup", "error");
				CupList[numCups].name = SPStrN (line, "name", "unknown");
				CupList[numCups].desc = SPStrN (line, "desc", "unknown");
				num = SPIntN (line, "num", 0);
				CupList[numCups].num_races = num;
				for (ii=0; ii<num; ii++) {
					race = SPStrN (line, Int_StrN (ii+1), "");
					CupList[numCups].races[ii] = GetRaceIdx (race);
				}
				numCups++;
			}
		}
	}
	list.MakeIndex (CupIndex, "cup");

	// pass 3: events
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		type = SPIntN (line, "struct", -1);
		if (type == 2) {
			if (numEvents < MAX_EVENTS2) {
				EventList[numEvents].name = SPStrN (line, "name", "unknown");
				num = SPIntN (line, "num", 0);
				EventList[numEvents].num_cups = num;
				for (ii=0; ii<num; ii++) {
					cup = SPStrN (line, Int_StrN (ii+1), "");
					EventList[numEvents].cups[ii] = GetCupIdx (cup);
				}
				numEvents++;
			}
		}
	}
	list.MakeIndex (EventIndex, "event");

	return true;
}

int CEvents::GetRaceIdx (string race) {
	return SPIntN (RaceIndex, race, 0);
}

int CEvents::GetCupIdx (string cup) {
	return SPIntN (CupIndex, cup, 0);
}

int CEvents::GetEventIdx (string event) {
	return SPIntN (EventIndex, event, 0);
}

string CEvents::GetCup (int event, int cup) {
	if (event < 0 || event >= numEvents) return "error";
	if (cup < 0 || cup >= EventList[event].num_cups) return "error";
	int ci = EventList[event].cups[cup];
	return CupList[ci].cup;
}

string CEvents::GetCupTrivialName (int event, int cup) {
	if (event < 0 || event >= numEvents) return "error";
	if (cup < 0 || cup >= EventList[event].num_cups) return "error";
	int ci = EventList[event].cups[cup];
	return CupList[ci].name;
}

void CEvents::MakeUnlockList (string unlockstr) {
	int event, cup, passed;
	string cp;
	for (event=0; event<numEvents; event++) {
		for (cup=0; cup<EventList[event].num_cups; cup++) {
			Unlocked[event][cup] = false;
		}
	}
	for (event=0; event<numEvents; event++) {
		for (cup=0; cup<EventList[event].num_cups; cup++) {
			cp = GetCup (event, cup);
			passed = SPosN (unlockstr, cp);
			if (cup < 1) Unlocked[event][0] = true;
			if (passed >= 0) {
				Unlocked[event][cup] = true;
				Unlocked[event][cup+1] = true;
			}
		}
	}
}

bool CEvents::IsUnlocked (int event, int cup) {
	if (event < 0 || event >= numEvents) return false;
	if (cup < 0 || cup >= EventList[event].num_cups) return false;
	return Unlocked[event][cup];
}

// --------------------------------------------------------------------
//				player administration
// --------------------------------------------------------------------

CPlayers Players;

CPlayers::CPlayers () {
	int i;
	for (i=0; i<MAX_PLAYERS; i++) {
		plyr[i].name = "";
		plyr[i].avatar = "";
		plyr[i].ctrl = NULL;
		plyr[i].texid = 0;
		plyr[i].funlocked = "";
	}
	currPlayer = 0;
	AvatarIndex = "";
	numAvatars = 0;
	for (i=0; i<MAX_AVATARS; i++) {
		avatars[i].filename = "";
		avatars[i].texid = 0;
	}
}

CPlayers::~CPlayers () {}

void CPlayers::AddPlayer (string name, string avatar) {
	if (numPlayers >= MAX_PLAYERS) {
		Message ("maximum of players reached");
		return;
	}
	plyr[numPlayers].name = name;
	plyr[numPlayers].avatar = avatar;
	plyr[numPlayers].texid = SPIntN (AvatarIndex, plyr[numPlayers].avatar, 0);
	plyr[numPlayers].funlocked = "";
	numPlayers++;
}

void CPlayers::SetDefaultPlayers () {
	plyr[0].funlocked = "";
	plyr[0].name = "Racer";
	plyr[0].avatar = "avatar01.png";
	plyr[0].texid = SPIntN (AvatarIndex, plyr[0].avatar, 0);

	plyr[1].funlocked = "";
	plyr[1].name = "Bunny";
	plyr[1].avatar = "avatar02.png";
	plyr[1].texid = SPIntN (AvatarIndex, plyr[1].avatar, 0);
	numPlayers = 2;
}

bool CPlayers::LoadPlayers () {
	CSPList list(MAX_PLAYERS);
	string line;
	int active;

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

	numPlayers = 0;
	g_game.start_player = 0;
	for (int i=0; i<list.Count(); i++) {
		line = list.Line(i);
		plyr[numPlayers].name = SPStrN (line, "name", "unknown");
		plyr[numPlayers].funlocked = SPStrN (line, "unlocked", "");
		plyr[numPlayers].avatar = SPStrN (line, "avatar", "");
		plyr[numPlayers].texid = SPIntN (AvatarIndex, plyr[numPlayers].avatar, 0);
		active = SPIntN (line, "active", 0);
		if (active > 0) g_game.start_player = numPlayers;
		numPlayers++;
	}
	if (numPlayers < 1) {
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
	for (int i=0; i<numPlayers; i++) {
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

void CPlayers::AddPassedCup (string cup) {
	if (SPIntN (plyr[currPlayer].funlocked, cup, -1) > 0) return;
	plyr[currPlayer].funlocked += " ";
	plyr[currPlayer].funlocked += cup;
}

CControl *CPlayers::GetCtrl () {
	return plyr[currPlayer].ctrl;
}

CControl *CPlayers::GetCtrl (int player) {
	if (player < 0 || player >= numPlayers) return NULL;
	return plyr[player].ctrl;
}

string CPlayers::GetName (int player) {
	if (player < 0 || player >= numPlayers) return "";
	return plyr[player].name;
}

void CPlayers::ResetControls () {
	for (int i=0; i<MAX_PLAYERS; i++) {
		if (plyr[i].ctrl != NULL) {
			free (plyr[i].ctrl);
			plyr[i].ctrl = NULL;
		}
	}
}

// called in module regist.cpp:
void CPlayers::AllocControl (int player) {
	if (player < 0 || player >= numPlayers) return;
	if (plyr[player].ctrl != NULL) return;
	plyr[player].ctrl = new (CControl);
}

// ----------------------- avatars ------------------------------------

void CPlayers::LoadAvatars () {
	CSPList list (MAX_AVATARS);
	int i;
	string line, filename;
	GLuint texid;

	if (!list.Load (param.player_dir, "avatars.lst")) {
		Message ("could not load avators.lst");
		return;
	}

	AvatarIndex = "";
	numAvatars = 0;
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		filename = SPStrN (line, "file", "unknown");
		texid = Tex.LoadTexture (param.player_dir, filename);
		if (texid > 0) {
			avatars[numAvatars].filename = filename;
			avatars[numAvatars].texid = texid;
			AvatarIndex += "[" + filename + "]";
			AvatarIndex += Int_StrN (texid);
			numAvatars++;
		}
	}
}

GLuint CPlayers::GetAvatarID (int player) {
	if (player < 0 || player >= numPlayers) return 0;
	return plyr[player].texid;
}

GLuint CPlayers::GetAvatarID (string filename) {
	return SPIntN (AvatarIndex, filename, 0);
}

GLuint CPlayers::GetDirectAvatarID (int avatar) {
	if (avatar < 0 || avatar >= numAvatars) return 0;
	return avatars[avatar].texid;
}

string CPlayers::GetDirectAvatarName (int avatar) {
	if (avatar < 0 || avatar >= numAvatars) return "";
	return avatars[avatar].filename;
}

// ********************************************************************
//				Character Administration
// ********************************************************************

CCharacter Char;

CCharacter::CCharacter () {
	for (int i=0; i<MAX_CHARACTERS; i++) {
		CharList[i].name = "";
		CharList[i].dir = "";
		CharList[i].preview = 0;
		CharList[i].shape = NULL;
	}
	numCharacters = 0;
	curr_character = 0;
}

CCharacter::~CCharacter () {}

static string char_type_index = "[spheres]0[3d]1";

void CCharacter::LoadCharacterList () {
	CSPList list (MAX_CHARACTERS+2);
	string line, typestr, charpath, previewfile;
	int i;
	GLuint texid;

	if (!list.Load (param.char_dir, "characters.lst")) {
		Message ("could not load characters.lst");
		return;
	}

	numCharacters = 0;
	TCharacter *ch;
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		CharList[i].name = SPStrN (line, "name", "");
		CharList[i].dir = SPStrN (line, "dir", "");
		typestr = SPStrN (line, "type", "unknown");
		CharList[i].type = SPIntN (char_type_index, typestr, -1);

		charpath = param.char_dir + SEP + CharList[i].dir;
		if (DirExists (charpath.c_str())) {
			previewfile = charpath + SEP + "preview.png";
			texid = Tex.LoadMipmapTexture (previewfile.c_str(), 0);
			if (texid < 1) {
				Message ("could not load previewfile of character");					
//				texid = Tex.TexID (NO_PREVIEW);
			}
			
			ch = &CharList[i];
			ch->preview = texid;

			ch->shape = new CCharShape;
			if (ch->shape->Load (charpath, "shape.lst", false) == false) {
				free (ch->shape);
				ch->shape = NULL;
				Message ("could not load character shape");
			} else numCharacters++;

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

void CCharacter::Draw (int idx) {
	if (idx < 0 || idx >= numCharacters) return;
	CharList[idx].shape->Draw ();
}

CCharShape *CCharacter::GetShape (int idx) {
	if (idx < 0 || idx >= numCharacters) return NULL;
	return CharList[idx].shape;
}

CKeyframe *CCharacter::GetKeyframe (int idx, TFrameType type) {
	if (type < 0 || type >= NUM_FRAME_TYPES) return NULL;
	if (idx < 0 || idx >= numCharacters) return NULL;
	return &CharList[idx].frames[type];
}



