/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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
	State(const State&) = delete;
	State& operator=(const State&) = delete;
protected:
	State() = default;
public:
	class Manager {
		friend class State;

		CWinsys& Winsys;
		State* previous;
		State* current;
		State* next;
		sf::Clock timer;
		bool quit;
		explicit Manager(CWinsys& winsys) : Winsys(winsys), previous(nullptr), current(nullptr), next(nullptr), quit(false) {}
		Manager(const Manager&);
		Manager& operator=(const Manager&) = delete;
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
	virtual void Loop(float time_step) {}
	virtual void Keyb(sf::Keyboard::Key key, bool release, int x, int y) {}
	virtual void Mouse(int button, int state, int x, int y) {}
	virtual void Motion(int x, int y) {}
	virtual void Jaxis(int axis, float value) {}
	virtual void Jbutt(int button, bool pressed) {}
	virtual void TextEntered(char text) {}
	virtual void Exit() {}
};

#endif
