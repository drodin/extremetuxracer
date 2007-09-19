/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *  
 * Copyright (C) 1999-2001 Jasmin F. Patry
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


#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "course_mgr.h"

#include <list>
#include <map>
#include <string>
#include <fstream>


class PlayerCourseData
{
public:
	PlayerCourseData();	

	bool update(double time,int herring,int score, bool won=true);

	bool won;
    double time;
    int herring;
    int score;
};


class PlayerCupData
{
	std::map<std::string,PlayerCourseData> courses;
public:
	PlayerCupData();

	bool won;

	bool getCupCourseData(std::string course,
					PlayerCourseData& data);
	bool updateCupCourseData(std::string course, 
					double time, 
					int herring,
					int score,
					bool won);

	void saveData(std::ofstream& sfile);
	void loadData(std::ifstream& sfile);
};

class PlayerEventData
{
	std::map<std::string,PlayerCupData> cups;	
public:
	PlayerEventData();

	bool won;

	bool isCupWon(std::string cup);
	bool getCupCourseData(std::string cup,
					std::string course,
					PlayerCourseData& data);
	bool updateCupCourseData(std::string cup,
					std::string course, 
					double time, 
					int herring,
					int score,
					bool won);
	void setCupComplete(std::string cup);

	void clearCupData(std::string cup);

	void saveData(std::ofstream& sfile);
	void loadData(std::ifstream& sfile);
};


class Player
{
	std::map<std::string,PlayerEventData> events;
	
	std::map<std::string,PlayerCourseData> courses;
	
	int m_lives;
	
public:
	Player();
	~Player();

	static const int FILE_VERSION=2;

	bool isCupComplete(std::string event,
						std::string cup);
	bool isFirstIncompleteCup( std::list<EventData>::iterator event,
						std::list<CupData>::iterator cup);
	
	bool getCupCourseData(std::string event,
					std::string cup,
					std::string course,
					PlayerCourseData& data);

	bool updateCupCourseData(std::string event,
					std::string cup,
					std::string course, 
					double time, 
					int herring,
					int score,
					bool won);

	void setCupComplete(std::string event, std::string cup);
	void setEventComplete(std::string event);
	
	void clearCupData(std::string event, std::string cup);
	

	bool getOpenCourseData(std::string course,
								PlayerCourseData& data);

	bool updateOpenCourseData(std::string course, 
								double time, 
								int herring,
								int score);

	void resetLives();
	int getLives();
	void decLives();
	void incLives();
	
	
	bool saveData();
	bool loadData();
	
	
	/// name of player 
	std::string name;
	
	// for future use
	int health;
	
	int herring;
	
	int score;
	
	int max_speed;

		

    /// current position
	pp::Vec3d pos;   
	
    /// current velocity
	pp::Vec3d vel;
	
	/// current orientation
	pp::Quat orientation; 
	
    /// is orientation initialized?
	bool orientation_initialized;
	
    /// vector sticking out of bellybutton (assuming on back)
	pp::Vec3d plane_nml;
	
	/// vector sticking out of feet
	pp::Vec3d direction; 
	
    /// net force on player 
	pp::Vec3d net_force;
	
    /// terrain force on player
	pp::Vec3d normal_force;  
	
    /// is plyr in the air?
	bool airborne;
	
    /// has plyr collided with obstacle?
	bool collision;
	
	/// player control data
	control_t control;

    /// player's view point
	view_t view;
	
	float getCoursePercentage();	    
};

///global array of players
extern Player players[1];

#endif // _PLAYER_H_
