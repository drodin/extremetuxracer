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

#ifndef _COURSE_MGR_H
#define _COURSE_MGR_H

#include "etracer.h"
#include "pp_types.h"

#include <list>
#include <string>

class CourseData;

struct CupData {
    std::string name;
	std::string icon;
    std::list<CourseData> raceList;
};

struct EventData {
    std::string name;
	std::string icon;
    std::list<CupData> cupList;
};

class CourseData
{
public:
	CourseData(){};
	~CourseData(){};

	std::string course;
	std::string name;
	std::string description;
	std::string contributed;	
		
	double par_time;
    int herring_req[DIFFICULTY_NUM_LEVELS];
	double time_req[DIFFICULTY_NUM_LEVELS];
	double score_req[DIFFICULTY_NUM_LEVELS];
	bool mirrored;
	race_conditions_t condition;
	bool windy;
	bool snowing;
};

extern std::list<CourseData> openCourseList;
extern std::list<EventData> eventList;

void init_course_manager();
void register_course_manager_callbacks( Tcl_Interp *ip );

#endif // _COURSE_MGR_H
