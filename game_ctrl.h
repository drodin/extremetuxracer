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

#define MAX_RACES2 256
#define MAX_CUPS2 64
#define MAX_EVENTS2 16
#define MAX_RACES_PER_CUP 6
#define MAX_CUPS_PER_EVENT 12

typedef struct {
	string race;
	int course;
	int light;
	int snow;
	int wind;
	TIndex3 herrings;
	TVector3 time;
	int music_theme;
} TRace2;

typedef struct {
	string cup;
	string name;
	string desc;
	int num_races;
	int races[MAX_RACES_PER_CUP];
} TCup2;

typedef struct {
	string name;
	int num_cups;
	int cups[MAX_CUPS_PER_EVENT];
} TEvent2;

class CEvents {
private:
	string RaceIndex;
	string CupIndex;
	string EventIndex;
	bool Unlocked [MAX_EVENTS2][MAX_CUPS_PER_EVENT+1];
public:
	CEvents ();
	~CEvents ();
	TRace2 RaceList[MAX_RACES2];
	int numRaces;
	TCup2 CupList[MAX_CUPS2];
	int numCups;
	TEvent2 EventList[MAX_EVENTS2];
	int numEvents;
	bool LoadEventList ();
	int GetRaceIdx (string race);
	int GetCupIdx (string cup);
	int GetEventIdx (string event);
	string GetCup (int event, int cup);
	string GetCupTrivialName (int event, int cup);

	void MakeUnlockList (string unlockstr);
	bool IsUnlocked (int event, int cup);
};

extern CEvents Events;

// --------------------------------------------------------------------
//				player
// --------------------------------------------------------------------

#define MAX_PLAYERS 16
#define MAX_AVATARS 32

typedef struct {
	string name;
	CControl *ctrl;
	string funlocked;
	GLuint texid;
	string avatar;
} TPlayer;

typedef struct {
	string filename;
	GLuint texid;
} TAvatar;

class CPlayers {
private:
	TPlayer plyr[MAX_PLAYERS];
 	int currPlayer;
 	void SetDefaultPlayers ();
	string AvatarIndex;
	TAvatar avatars[MAX_AVATARS];
public:
	CPlayers ();
	~CPlayers ();
	int numPlayers;
	int numAvatars;

	string GetCurrUnlocked ();
	void AddPassedCup (string cup);
	void AddPlayer (string name, string avatar);
	bool LoadPlayers ();
	void SavePlayers ();
	CControl *GetCtrl (); // current player
	CControl *GetCtrl (int player); 	
	string GetName (int player);
	void ResetControls ();
	void AllocControl (int player);
	void LoadAvatars ();
	
	GLuint GetAvatarID (int player);
	GLuint GetAvatarID (string filename);
	GLuint GetDirectAvatarID (int avatar);
	string GetDirectAvatarName (int avatar);
};

extern CPlayers Players;

// -------------------------------- characters ------------------------
#define MAX_CHARACTERS 16

typedef struct {
	int type;
	string name;
	string dir;
	GLuint preview;
	CCharShape *shape;
	CKeyframe frames[NUM_FRAME_TYPES];
	bool finishframesok;
} TCharacter;

class CCharacter {
private:
	int curr_character; 
public:
	CCharacter ();
	~CCharacter ();

	TCharacter CharList [MAX_CHARACTERS];
	int numCharacters;

	void Draw (int idx);
	CCharShape *GetShape (int idx);
	void LoadCharacterList ();
 	void FreeCharacterPreviews ();

	CKeyframe *GetKeyframe (int idx, TFrameType type);
};

extern CCharacter Char;


#endif
