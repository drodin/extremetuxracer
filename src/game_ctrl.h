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

#ifndef GAME_CTRL_H
#define GAME_CTRL_H

#include "bh.h"
#include "keyframe.h"
#include "spx.h"
#include <unordered_map>


enum TFrameType {
	START,
	FINISH,
	WONRACE,
	LOSTRACE,
	NUM_FRAME_TYPES
};


class TTexture;

struct TRace {
	TCourse* course;
	std::size_t light;
	int snow;
	int wind;
	TVector3i herrings;
	TVector3d time;
	std::size_t music_theme;

	TRace(TCourse* course_, std::size_t light_, int snow_, int wind_, const TVector3i& herrings_, const TVector3d& time_, std::size_t music_theme_)
		: course(course_), light(light_), snow(snow_), wind(wind_), herrings(herrings_), time(time_), music_theme(music_theme_)
	{}
};

struct TCup {
	std::string cup;
	std::string name;
	std::string desc;
	std::vector<TRace*> races;
	bool Unlocked;

	TCup(const std::string& cup_, const std::string& name_, const std::string& desc_)
		: cup(cup_), name(name_), desc(desc_), Unlocked(false)
	{}
};

struct TEvent {
	std::string name;
	std::vector<TCup*> cups;

	explicit TEvent(const std::string& name_)
		: name(name_)
	{}
};

class CEvents {
private:
	std::unordered_map<std::string, std::size_t> RaceIndex;
	std::unordered_map<std::string, std::size_t> CupIndex;
	std::unordered_map<std::string, std::size_t> EventIndex;
public:
	std::vector<TRace> RaceList;
	std::vector<TCup> CupList;
	std::vector<TEvent> EventList;
	bool LoadEventList();
	std::size_t GetRaceIdx(const std::string& race) const;
	std::size_t GetCupIdx(const std::string& cup) const;
	std::size_t GetEventIdx(const std::string& event) const;
	const std::string& GetCup(std::size_t event, std::size_t cup) const;
	const std::string& GetCupTrivialName(std::size_t event, std::size_t cup) const;

	void MakeUnlockList(const std::string& unlockstr);
	bool IsUnlocked(std::size_t event, std::size_t cup) const;
};

extern CEvents Events;

// --------------------------------------------------------------------
//				player
// --------------------------------------------------------------------

struct TAvatar {
	std::string filename;
	TTexture* texture;

	TAvatar(const std::string& filename_, TTexture* texture_)
		: filename(filename_), texture(texture_)
	{}
};

struct TPlayer {
	std::string name;
	CControl *ctrl;
	std::string funlocked;
	const TAvatar* avatar;

	TPlayer(const std::string& name_ = emptyString, const TAvatar* avatar_ = nullptr)
		: name(name_), ctrl(nullptr), avatar(avatar_)
	{}
};

class CPlayers {
private:
	std::vector<TPlayer> plyr;
	void SetDefaultPlayers();
	std::vector<TAvatar> avatars;

	const TAvatar* FindAvatar(const std::string& name) const;
public:
	~CPlayers();

	TPlayer* GetPlayer(std::size_t index) { return &plyr[index]; }
	void AddPassedCup(const std::string& cup);
	void AddPlayer(const std::string& name, const std::string& avatar);
	bool LoadPlayers();
	void SavePlayers() const;
	void ResetControls();
	void AllocControl(std::size_t player);
	bool LoadAvatars();
	std::size_t numAvatars() const { return avatars.size(); }
	std::size_t numPlayers() const { return plyr.size(); }

	TTexture* GetAvatarTexture(std::size_t avatar) const;
	const std::string& GetDirectAvatarName(std::size_t avatar) const;
};

extern CPlayers Players;

// -------------------------------- characters ------------------------

struct TCharacter {
	std::string name;
	std::string dir;
	TTexture* preview;
	CCharShape *shape;
	CKeyframe frames[NUM_FRAME_TYPES];
	int type;
	bool finishframesok;

	CKeyframe* GetKeyframe(TFrameType frametype);
};

class CCharacter {
public:
	std::vector<TCharacter> CharList;

	~CCharacter();

	bool LoadCharacterList();
	void FreeCharacterPreviews();
};

extern CCharacter Char;


#endif
