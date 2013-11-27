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

#ifndef GAME_CTRL_H
#define GAME_CTRL_H

#include "bh.h"
#include "keyframe.h"
#include "spx.h"
#include <map>


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
	size_t light;
	int snow;
	int wind;
	TVector3i herrings;
	TVector3d time;
	size_t music_theme;

	TRace(TCourse* course_, size_t light_, int snow_, int wind_, const TVector3i& herrings_, const TVector3d& time_, size_t music_theme_)
		: course(course_), light(light_), snow(snow_), wind(wind_), herrings(herrings_), time(time_), music_theme(music_theme_)
	{}
};

struct TCup {
	string cup;
	string name;
	string desc;
	vector<TRace*> races;
	bool Unlocked;

	TCup(const string& cup_, const string& name_, const string& desc_)
		: cup(cup_), name(name_), desc(desc_), Unlocked(false)
	{}
};

struct TEvent {
	string name;
	vector<TCup*> cups;

	TEvent(const string& name_)
		: name(name_)
	{}
};

class CEvents {
private:
	map<string, size_t> RaceIndex;
	map<string, size_t> CupIndex;
	map<string, size_t> EventIndex;
public:
	vector<TRace> RaceList;
	vector<TCup> CupList;
	vector<TEvent> EventList;
	bool LoadEventList ();
	size_t GetRaceIdx (const string& race) const;
	size_t GetCupIdx (const string& cup) const;
	size_t GetEventIdx (const string& event) const;
	const string& GetCup (size_t event, size_t cup) const;
	const string& GetCupTrivialName (size_t event, size_t cup) const;

	void MakeUnlockList (const string& unlockstr);
	bool IsUnlocked (size_t event, size_t cup) const;
};

extern CEvents Events;

// --------------------------------------------------------------------
//				player
// --------------------------------------------------------------------

#define MAX_PLAYERS 16
#define MAX_AVATARS 32

struct TAvatar {
	string filename;
	TTexture* texture;

	TAvatar(const string& filename_, TTexture* texture_)
		: filename(filename_), texture(texture_)
	{}
};

struct TPlayer {
	string name;
	CControl *ctrl;
	string funlocked;
	const TAvatar* avatar;

	TPlayer(const string& name_ = emptyString, const TAvatar* avatar_ = NULL)
		: name(name_), ctrl(NULL), avatar(avatar_)
	{}
};

class CPlayers {
private:
	vector<TPlayer> plyr;
	void SetDefaultPlayers ();
	vector<TAvatar> avatars;

	const TAvatar* FindAvatar(const string& name);
public:
	~CPlayers();

	TPlayer* GetPlayer(size_t index) { return &plyr[index]; }
	void AddPassedCup (const string& cup);
	void AddPlayer (const string& name, const string& avatar);
	bool LoadPlayers ();
	void SavePlayers () const;
	void ResetControls ();
	void AllocControl (size_t player);
	void LoadAvatars ();
	size_t numAvatars() const { return avatars.size(); }
	size_t numPlayers() const { return plyr.size(); }

	TTexture* GetAvatarTexture (size_t avatar) const;
	const string& GetDirectAvatarName(size_t avatar) const;
};

extern CPlayers Players;

// -------------------------------- characters ------------------------
#define MAX_CHARACTERS 16

struct TCharacter {
	int type;
	string name;
	string dir;
	TTexture* preview;
	CCharShape *shape;
	CKeyframe frames[NUM_FRAME_TYPES];
	bool finishframesok;

	CKeyframe* GetKeyframe(TFrameType type);
};

class CCharacter {
public:
	vector<TCharacter> CharList;

	~CCharacter();

	void LoadCharacterList ();
	void FreeCharacterPreviews ();
};

extern CCharacter Char;


#endif
