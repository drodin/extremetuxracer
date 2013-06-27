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
#include <map>


enum TFrameType {
	START,
	FINISH,
	WONRACE,
	LOSTRACE,
	NUM_FRAME_TYPES
};


class TTexture;

struct TRace2 {
	string race;
	size_t course;
	size_t light;
	int snow;
	int wind;
	TIndex3 herrings;
	TVector3 time;
	size_t music_theme;
};

struct TCup2 {
	string cup;
	string name;
	string desc;
	vector<TRace2*> races;
	bool Unlocked;
};

struct TEvent2 {
	string name;
	vector<TCup2*> cups;
};

class CEvents {
private:
	map<string, size_t> RaceIndex;
	map<string, size_t> CupIndex;
	map<string, size_t> EventIndex;
public:
	vector<TRace2> RaceList;
	vector<TCup2> CupList;
	vector<TEvent2> EventList;
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
};

struct TPlayer {
	string name;
	CControl *ctrl;
	string funlocked;
	TTexture* texture;
	string avatar;
};

class CPlayers {
private:
	vector<TPlayer> plyr;
	void SetDefaultPlayers ();
	map<string, size_t> AvatarIndex;
	vector<TAvatar> avatars;
public:
	~CPlayers();

	const string& GetCurrUnlocked () const;
	void AddPassedCup (const string& cup);
	void AddPlayer (const string& name, const string& avatar);
	bool LoadPlayers ();
	void SavePlayers () const;
	CControl *GetCtrl (size_t player);
	const CControl *GetCtrl (size_t player) const;
	const string& GetName (size_t player) const;
	void ResetControls ();
	void AllocControl (size_t player);
	void LoadAvatars ();
	size_t numAvatars() const { return avatars.size(); }
	size_t numPlayers() const { return plyr.size(); }

	TTexture* GetAvatarTexture (size_t avatar) const;
	const string& GetDirectAvatarName (size_t avatar) const;
};

extern CPlayers Players;

// -------------------------------- characters ------------------------
#define MAX_CHARACTERS 16

class CCharShape;

struct TCharacter {
	int type;
	string name;
	string dir;
	TTexture* preview;
	CCharShape *shape;
	CKeyframe frames[NUM_FRAME_TYPES];
	bool finishframesok;
};

class CCharacter {
public:
	vector<TCharacter> CharList;

	~CCharacter();

	void Draw (size_t idx);
	CCharShape *GetShape (size_t idx);
	void LoadCharacterList ();
	void FreeCharacterPreviews ();

	CKeyframe *GetKeyframe (size_t idx, TFrameType type);
};

extern CCharacter Char;


#endif
