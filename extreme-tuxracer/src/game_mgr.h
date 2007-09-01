/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef _GAME_MGR_H_
#define _GAME_MGR_H_


#include "course_mgr.h"
#include "player.h"

class GameMgr
{
	std::list<CourseData>::iterator mi_currentRace;
	
	bool m_raceAborted;
	bool m_raceWon;
	bool m_cupWon;
	bool m_eventWon;
	
	double m_lastTicks;
		
public:
	GameMgr();

	// should be private
	// needs some love in event_select.cpp
	std::list<EventData>::iterator currentEvent;
	std::list<CupData>::iterator currentCup;


	difficulty_level_t difficulty;
	double time;
	double airbornetime;
	double timeStep;
	int numPlayers;

	void setupEventAndCup( std::list<EventData>::iterator event,
						std::list<CupData>::iterator cup);

	void setCurrentRace(std::list<CourseData>::iterator race);
	
	inline EventData& getCurrentEvent(){return *currentEvent;};
	inline CupData& getCurrentCup(){return *currentCup;};
	inline CourseData& getCurrentRace(){return *mi_currentRace;};

	typedef enum{
		PRACTICING,
		EVENT
	}gametype_t;

	gametype_t gametype;
	void reset(gametype_t type);
	
	inline bool wasRaceAborted(){return m_raceAborted;};
	inline void abortRace(bool abort=true){m_raceAborted=abort;};
	
	void updatePlayersScores();

	unsigned int calculateScore(double time,
								int herring,
								int health);

	bool updateCurrentRaceData();
	bool wasRaceWon();
	bool wasCupWon();
	bool wasEventWon();
	
	void resetTimeStep();
	void updateTimeStep();
	inline double getTimeStep(){return timeStep;};	
};

extern GameMgr* gameMgr;

#endif // _GAME_MGR_H_
