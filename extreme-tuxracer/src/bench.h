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


#ifndef _BENCH_H_
#define _BENCH_H_

#include "loop.h"

#include "ppgltk/alg/vec2d.h"

#include <string>


class Benchmark : public GameMode
{	
	static std::string sm_course;
	static double sm_frames;
	static double sm_oldFrames;
	static int sm_framesCounter;
	static int sm_maxFrames;
	static int sm_fc;
	static pp::Vec2d sm_pos;
	static double sm_timeStep;
	static race_conditions_t sm_condition;
	
public:
	Benchmark();
	~Benchmark();

	void loop(float timeStep);

	typedef enum{
		NONE=0,
		AUTO=1,
		DEMO,
		PAUSED		
	}mode_t;

	static void setCourse(const char* course);
	static void setMaxTime(double time);
	static void setMaxFrames(int frames);
	static void setPosition(pp::Vec2d &position);
	static pp::Vec2d& getPosition(); 
	static Benchmark::mode_t getMode();
	static void setMode(Benchmark::mode_t);
	static void setTimeStep(double timeStep);
	static double getTimeStep(); 
	static void setRaceCondition(int condition);

	static void updateFPS(double fps);
	static void displayState();
	
private:
	static mode_t sm_mode;

};

#endif // _BENCH_H_
