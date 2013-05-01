/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
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

#ifndef STATES_H
#define STATES_H

#include "bh.h"


class CWinsys;

class State {
	State(const State&);
	State& operator=(const State&);
protected:
	State() {}
public:
	class Manager {
		friend class State;

		CWinsys& Winsys;
		State* previous;
		State* current;
		State* next;
		bool quit;
		float clock_time;
		Manager(CWinsys& winsys) : Winsys(winsys), previous(NULL), current(NULL), next(NULL), quit(false), clock_time(0.f) {}
		Manager(const Manager&);
		Manager& operator=(const Manager&);
		~Manager();

		void PollEvent();
		void CallLoopFunction();
		void EnterNextState();
	public:
		void RequestEnterState(State& state) { next = &state; }
		void RequestQuit() { quit = true; }
		void Run(State& entranceState);
		State* PreviousState() { return previous; }
		State* CurrentState() { return current; }
	};
	static Manager manager;

	virtual void Enter() {}
	virtual void Loop(double time_step) {}
	virtual void Keyb(unsigned int key, bool special, bool release, int x, int y) {}
	virtual void Mouse(int button, int state, int x, int y) {}
	virtual void Motion(int x, int y) {}
	virtual void Jaxis(int axis, double value) {}
	virtual void Jbutt(int button, int state) {}
	virtual void Keyb_spec(SDL_keysym sym, bool release) {}
	virtual void Exit() {}
};

#endif
