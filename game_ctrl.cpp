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
//				player
// --------------------------------------------------------------------

CPlayers Players;

CPlayers::CPlayers () {
	int i;
	for (i=0; i<MAX_PLAYERS; i++) {
		plyr[i].name = "";
		plyr[i].funlocked = "";
//		plyr[i].score = "";
	}
	numPlayers = 1;
	currPlayer = 0;
}

CPlayers::~CPlayers () {}

void CPlayers::LoadParams () {
	CSPList list(MAX_PLAYERS);
	string line;

	numPlayers = 1;
	plyr[0].funlocked = "";
	plyr[0].name = "Racer";

	string playerfile = param.config_dir + SEP + "players";
	if (FileExists (playerfile.c_str()) == false) {
		Players.SaveParams ();
		return; 
	}
	if (list.Load (playerfile) == false) {
		Players.SaveParams ();
		return; 
	}
	for (int i=0; i<list.Count(); i++) {
		line = list.Line(i);
		plyr[i].name = SPStrN (line, "name", "Unknown");
		plyr[i].funlocked = SPStrN (line, "unlocked", "");
	}
}

void CPlayers::SaveParams () {
	string playerfile = param.config_dir + SEP + "players";
	CSPList list(MAX_PLAYERS);
	string item = "";
	for (int i=0; i<numPlayers; i++) {
		item = "[name]" + plyr[i].name;
		item += "[unlocked]";
		item += plyr[i].funlocked;
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

CControl *CPlayers::GetControl () {
	return &plyr[currPlayer].ctrl;
}

CControl *CPlayers::GetControl (int player) {
	if (player < 0 || player >= numPlayers) return NULL;
	return &plyr[player].ctrl;
}





