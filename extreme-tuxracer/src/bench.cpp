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
 
#include "bench.h"

#include "game_mgr.h"

Benchmark::mode_t Benchmark::sm_mode = Benchmark::NONE;

std::string Benchmark::sm_course;
double Benchmark::sm_frames=0.0;
double Benchmark::sm_oldFrames=0.0;
int Benchmark::sm_framesCounter=0;
int Benchmark::sm_maxFrames=0;
int Benchmark::sm_fc=0;
pp::Vec2d Benchmark::sm_pos;
double Benchmark::sm_timeStep=-1;
race_conditions_t Benchmark::sm_condition=RACE_CONDITIONS_SUNNY;
	
Benchmark::Benchmark()
{
	
	
}

Benchmark::~Benchmark()
{
		
	
}

void
Benchmark::loop(float timeStep)
{
	static std::list<CourseData>::iterator it;
	
	it = openCourseList.begin();

	for (it=openCourseList.begin(); 
		it != openCourseList.end();
		it++)
	{
		if((*it).course == sm_course) break;			
	}
	
	if( it != openCourseList.end() ){
		gameMgr->setCurrentRace(it);
		gameMgr->getCurrentRace().condition = sm_condition;
	}else{
		std::cout << "Benchmark error: unable to set course: " 
					<< sm_course << std::endl;
		exit(0);
	}
	
	set_game_mode( LOADING );
}

void
Benchmark::setCourse(const char* course)
{
	sm_course = course;	
	if(sm_mode == Benchmark::NONE){
		sm_mode = Benchmark::DEMO;
	}
}

void
Benchmark::setMaxFrames(int frames)
{
	sm_maxFrames = frames;
}

void
Benchmark::setPosition(pp::Vec2d &position)
{
	sm_pos = position;
}

pp::Vec2d&
Benchmark::getPosition()
{
	return sm_pos;
}

void
Benchmark::setMode(Benchmark::mode_t mode)
{
	sm_mode = mode;
}

Benchmark::mode_t
Benchmark::getMode()
{
	return sm_mode;
}

void
Benchmark::setTimeStep(double timeStep)
{
	sm_timeStep = timeStep;
}

double
Benchmark::getTimeStep()
{
	return sm_timeStep;
}

void
Benchmark::setRaceCondition(int condition)
{
	if (condition > 0 && 
		condition < RACE_CONDITIONS_NUM_CONDITIONS)
	{
		sm_condition = (race_conditions_t) condition;
	}
}

void
Benchmark::updateFPS(double fps)
{
	sm_frames+=fps;
	sm_framesCounter++;
	
	if(sm_fc>=2){
		sm_fc=0;
		std::cout << "FPS: "<< (sm_frames-sm_oldFrames)/3 << std::endl;
		sm_oldFrames = sm_frames;		
	}else{
		sm_fc++;
	}
		
	if( sm_maxFrames > 0 && sm_frames >= sm_maxFrames ){
		set_game_mode(GAME_OVER);		
	}
}

void
Benchmark::displayState()
{
	std::cout << std::endl;
	std::cout << "Frames: "
			<< sm_frames << std::endl;
	
	std::cout << "Average FPS: "
			<< sm_frames/sm_framesCounter << std::endl;
}
