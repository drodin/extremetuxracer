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

#define MAX_PLAYERS 8

typedef struct {
	string name;
	CControl ctrl;
	string funlocked;
} TPlayer;

class CPlayers {
private:
	TPlayer plyr[MAX_PLAYERS];
	int numPlayers;
 	int currPlayer;
public:
	CPlayers ();
	~CPlayers ();

	string GetCurrUnlocked ();
	void AddPassedCup (string cup);
	void LoadParams ();
	void SaveParams ();
	CControl *GetControl (); // current player
	CControl *GetControl (int player); 	
};

extern CPlayers Players;

#endif
