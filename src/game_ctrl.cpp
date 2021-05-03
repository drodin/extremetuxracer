/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#include "game_ctrl.h"
#include "spx.h"
#include "course.h"
#include "env.h"
#include "audio.h"
#include "textures.h"
#include "tux.h"
#include "physics.h"

// --------------------------------------------------------------------
//				administration of events and cups
// --------------------------------------------------------------------

CEvents Events;

bool CEvents::LoadEventList() {
	CSPList list;

	if (!list.Load(param.common_course_dir, "events.lst")) {
		Message("could not load events.lst");
		return false;
	}

	// pass 1: races
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		int type = SPIntN(*line, "struct", -1);
		if (type == 0) {
			RaceList.emplace_back(Course.GetCourse(SPStrN(*line, "group"), SPStrN(*line, "course")),
			                      Env.GetLightIdx(SPStrN(*line, "light")),
			                      SPIntN(*line, "snow", 0),
			                      SPIntN(*line, "wind", 0),
			                      SPVector3i(*line, "herring"),
			                      SPVector3d(*line, "time"),
			                      Music.GetThemeIdx(SPStrN(*line, "theme", "normal")));
		}
	}
	list.MakeIndex(RaceIndex, "race");

	// pass 2: cups
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		int type = SPIntN(*line, "struct", -1);
		if (type == 1) {
			CupList.emplace_back(SPStrN(*line, "cup", errorString),
			                     SPStrN(*line, "name", "unknown"),
			                     SPStrN(*line, "desc", emptyString));
			int num = SPIntN(*line, "num", 0);
			CupList.back().races.resize(num);
			for (int ii=0; ii<num; ii++) {
				std::string race = SPStrN(*line, Int_StrN(ii+1));
				CupList.back().races[ii] = &RaceList[GetRaceIdx(race)];
			}
		}
	}
	list.MakeIndex(CupIndex, "cup");

	// pass 3: events
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		int type = SPIntN(*line, "struct", -1);
		if (type == 2) {
			EventList.emplace_back(SPStrN(*line, "name", "unknown"));
			int num = SPIntN(*line, "num", 0);
			EventList.back().cups.resize(num);
			for (int ii=0; ii<num; ii++) {
				std::string cup = SPStrN(*line, Int_StrN(ii+1));
				EventList.back().cups[ii] = &CupList[GetCupIdx(cup)];
			}
		}
	}
	list.MakeIndex(EventIndex, "event");

	return true;
}

std::size_t CEvents::GetRaceIdx(const std::string& race) const {
	return RaceIndex.at(race);
}

std::size_t CEvents::GetCupIdx(const std::string& cup) const {
	return CupIndex.at(cup);
}

std::size_t CEvents::GetEventIdx(const std::string& event) const {
	return EventIndex.at(event);
}

const std::string& CEvents::GetCup(std::size_t event, std::size_t cup) const {
	if (event >= EventList.size()) return errorString;
	if (cup >= EventList[event].cups.size()) return errorString;
	return EventList[event].cups[cup]->cup;
}

const std::string& CEvents::GetCupTrivialName(std::size_t event, std::size_t cup) const {
	if (event >= EventList.size()) return errorString;
	if (cup >= EventList[event].cups.size()) return errorString;
	return EventList[event].cups[cup]->name;
}

void CEvents::MakeUnlockList(const std::string& unlockstr) {
	for (std::size_t event=0; event<EventList.size(); event++) {
		for (std::size_t cup=0; cup<EventList[event].cups.size(); cup++) {
			EventList[event].cups[cup]->Unlocked = false;
		}
	}
	for (std::size_t event=0; event<EventList.size(); event++) {
		for (std::size_t cup=0; cup<EventList[event].cups.size(); cup++) {
			const std::string& cp = GetCup(event, cup);
			bool passed = SPosN(unlockstr, cp) != std::string::npos;
			if (cup < 1) EventList[event].cups[0]->Unlocked = true;
			if (passed) {
				EventList[event].cups[cup]->Unlocked = true;
				if (cup+1 < EventList[event].cups.size())
					EventList[event].cups[cup+1]->Unlocked = true;
			}
		}
	}
}

bool CEvents::IsUnlocked(std::size_t event, std::size_t cup) const {
	if (event >= EventList.size()) return false;
	if (cup >= EventList[event].cups.size()) return false;
	return EventList[event].cups[cup]->Unlocked;
}

// --------------------------------------------------------------------
//				player administration
// --------------------------------------------------------------------

CPlayers Players;

CPlayers::~CPlayers() {
	ResetControls();
	for (std::size_t i = 0; i < avatars.size(); i++)
		delete avatars[i].texture;
}

void CPlayers::AddPlayer(const std::string& name, const std::string& avatar) {
	plyr.emplace_back(name, FindAvatar(avatar));
}

void CPlayers::SetDefaultPlayers() {
	plyr.emplace_back("Racer", FindAvatar("avatar01.png"));
	plyr.emplace_back("Bunny", FindAvatar("avatar02.png"));
}

bool CPlayers::LoadPlayers() {
	if (FileExists(param.config_dir, "players") == false) {
		SetDefaultPlayers();
		Message("file 'players' does not exist, set default players");
		return false;
	}

	CSPList list;
	if (list.Load(param.config_dir, "players") == false) {
		SetDefaultPlayers();
		Message("could not load players list, set default players");
		return false;
	}

	g_game.start_player = 0;
	plyr.resize(list.size());
	std::size_t i = 0;
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
		plyr[i].name = SPStrN(*line, "name", "unknown");
		plyr[i].funlocked = SPStrN(*line, "unlocked");
		plyr[i].avatar = FindAvatar(SPStrN(*line, "avatar"));
		plyr[i].ctrl = nullptr;
		int active = SPIntN(*line, "active", 0);
		if (active > 0) g_game.start_player = plyr.size()-1;
	}
	if (plyr.empty()) {
		SetDefaultPlayers();
		Message("player file doesn't contain a player, set default players");
		return false;
	}
	return true;
}

void CPlayers::SavePlayers() const {
	std::string playerfile = param.config_dir + SEP "players";
	CSPList list;
	for (std::size_t i=0; i<plyr.size(); i++) {
		std::string item = "*[name]" + plyr[i].name;
		item +="[avatar]" + plyr[i].avatar->filename;
		item += "[unlocked]" + plyr[i].funlocked;
		if (&plyr[i] == g_game.player) item += "[active]1";
		else item += "[active]0";
		list.Add(item);
	}
	list.Save(playerfile);
}

const TAvatar* CPlayers::FindAvatar(const std::string& name) const {
	for (std::size_t i = 0; i < avatars.size(); i++)
		if (avatars[i].filename == name)
			return &avatars[i];
	return 0;
}

void CPlayers::AddPassedCup(const std::string& cup) {
	if (SPIntN(g_game.player->funlocked, cup, -1) > 0) return;
	g_game.player->funlocked += ' ';
	g_game.player->funlocked += cup;
}

void CPlayers::ResetControls() {
	for (std::size_t i=0; i<plyr.size(); i++) {
		delete plyr[i].ctrl;
		plyr[i].ctrl = nullptr;
	}
}

// called in module regist.cpp:
void CPlayers::AllocControl(std::size_t player) {
	if (player >= plyr.size()) return;
	if (plyr[player].ctrl != nullptr) return;
	plyr[player].ctrl = new CControl;
}

// ----------------------- avatars ------------------------------------

bool CPlayers::LoadAvatars() {
	CSPList list;

	if (!list.Load(param.player_dir, "avatars.lst")) {
		Message("could not load avators.lst");
		return false;
	}

	avatars.reserve(list.size());
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		std::string filename = SPStrN(*line, "file", "unknown");
		TTexture* texture = new TTexture();
		if (texture && texture->Load(param.player_dir, filename)) {
			avatars.emplace_back(filename, texture);
		} else
			delete texture;
	}
	return true;
}

TTexture* CPlayers::GetAvatarTexture(std::size_t avatar) const {
	if (avatar >= avatars.size()) return 0;
	return avatars[avatar].texture;
}

const std::string& CPlayers::GetDirectAvatarName(std::size_t avatar) const {
	if (avatar >= avatars.size()) return emptyString;
	return avatars[avatar].filename;
}

// ********************************************************************
//				Character Administration
// ********************************************************************

CKeyframe* TCharacter::GetKeyframe(TFrameType frametype) {
	if (frametype < 0 || frametype >= NUM_FRAME_TYPES) return nullptr;
	return &frames[frametype];
}


CCharacter Char;

static const std::string char_type_index = "[spheres]0[3d]1";

CCharacter::~CCharacter() {
	for (std::size_t i = 0; i < CharList.size(); i++) {
		delete CharList[i].preview;
		delete CharList[i].shape;
	}
}

bool CCharacter::LoadCharacterList() {
	CSPList list;

	if (!list.Load(param.char_dir, "characters.lst")) {
		Message("could not load characters.lst");
		return false;
	}

	CharList.resize(list.size());
	std::size_t i = 0;
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
		CharList[i].name = SPStrN(*line, "name");
		CharList[i].dir = SPStrN(*line, "dir");
		std::string typestr = SPStrN(*line, "type", "unknown");
		CharList[i].type = SPIntN(char_type_index, typestr, -1);

		std::string charpath = MakePathStr(param.char_dir, CharList[i].dir);
		if (DirExists(charpath.c_str())) {
			std::string previewfile = charpath + SEP "preview.png";

			TCharacter* ch = &CharList[i];
			ch->preview = new TTexture();
			if (!ch->preview->Load(previewfile, false)) {
				Message("could not load previewfile of character");
//				texid = Tex.TexID (NO_PREVIEW);
			}

			ch->shape = new CCharShape;
			if (ch->shape->Load(charpath, "shape.lst", false) == false) {
				delete ch->shape;
				ch->shape = nullptr;
				Message("could not load character shape");
			}

			ch->frames[0].Load(charpath, "start.lst");
			ch->finishframesok = true;
			ch->frames[1].Load(charpath, "finish.lst");
			if (ch->frames[1].loaded == false) ch->finishframesok = false;
			ch->frames[2].Load(charpath, "wonrace.lst");
			if (ch->frames[2].loaded == false) ch->finishframesok = false;
			ch->frames[3].Load(charpath, "lostrace.lst");
			if (ch->frames[3].loaded == false) ch->finishframesok = false;
		}
	}
	return !CharList.empty();
}

void CCharacter::FreeCharacterPreviews() {
	for (std::size_t i=0; i<CharList.size(); i++) {
		delete CharList[i].preview;
		CharList[i].preview = 0;
	}
}
