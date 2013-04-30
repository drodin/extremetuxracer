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
#include "tux.h"
#include "keyframe.h"

#define MAX_RACES_PER_CUP 6
#define MAX_CUPS_PER_EVENT 12

struct TRace2 {
	string race;
	int course;
	int light;
	int snow;
	int wind;
	TIndex3 herrings;
	TVector3 time;
	int music_theme;
};

struct TCup2 {
	string cup;
	string name;
	string desc;
	int num_races;
	TRace2* races[MAX_RACES_PER_CUP];
	bool Unlocked;
};

struct TEvent2 {
	string name;
	int num_cups;
	TCup2* cups[MAX_CUPS_PER_EVENT];
};

class CEvents {
private:
	string RaceIndex;
	string CupIndex;
	string EventIndex;
public:
	vector<TRace2> RaceList;
	vector<TCup2> CupList;
	vector<TEvent2> EventList;
	bool LoadEventList ();
	int GetRaceIdx (const string& race) const;
	int GetCupIdx (const string& cup) const;
	int GetEventIdx (const string& event) const;
	string GetCup (size_t event, size_t cup) const;
	string GetCupTrivialName (size_t event, size_t cup) const;

	void MakeUnlockList (string unlockstr);
	bool IsUnlocked (size_t event, size_t cup) const;
};

extern CEvents Events;

// --------------------------------------------------------------------
//				player
// --------------------------------------------------------------------

#define MAX_PLAYERS 16
#define MAX_AVATARS 32

struct TPlayer {
	string name;
	CControl *ctrl;
	string funlocked;
	GLuint texid;
	string avatar;
};

typedef struct {
	string filename;
	GLuint texid;
} TAvatar;

class CPlayers {
private:
	vector<TPlayer> plyr;
 	int currPlayer;
 	void SetDefaultPlayers ();
	string AvatarIndex;
	vector<TAvatar> avatars;
public:
	CPlayers ();

	string GetCurrUnlocked ();
	void AddPassedCup (const string& cup);
	void AddPlayer (const string& name, const string& avatar);
	bool LoadPlayers ();
	void SavePlayers ();
	CControl *GetCtrl (); // current player
	CControl *GetCtrl (int player); 	
	string GetName (int player) const;
	void ResetControls ();
	void AllocControl (int player);
	void LoadAvatars ();
	size_t numAvatars() const { return avatars.size(); }
	size_t numPlayers() const { return plyr.size(); }
	
	GLuint GetAvatarID (size_t player) const;
	GLuint GetAvatarID (const string& filename) const;
	GLuint GetDirectAvatarID (size_t avatar) const;
	string GetDirectAvatarName (size_t avatar) const;
};

extern CPlayers Players;

// -------------------------------- characters ------------------------
#define MAX_CHARACTERS 16

struct TCharacter {
	int type;
	string name;
	string dir;
	GLuint preview;
	CCharShape *shape;
	CKeyframe frames[NUM_FRAME_TYPES];
	bool finishframesok;
};

class CCharacter {
private:
	int curr_character; 
public:
	CCharacter ();

	vector<TCharacter> CharList;

	void Draw (size_t idx);
	CCharShape *GetShape (size_t idx);
	void LoadCharacterList ();
 	void FreeCharacterPreviews ();

	CKeyframe *GetKeyframe (size_t idx, TFrameType type);
};

extern CCharacter Char;


#endif
