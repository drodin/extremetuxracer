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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "states.h"
#include "ogl.h"
#include "winsys.h"

State::Manager State::manager(Winsys);

State::Manager::~Manager() {
	if(current)
		current->Exit();
}

void State::Manager::Run(State& entranceState) {
	current = &entranceState;
	current->Enter();
	while (!quit) {
		PollEvent();
		if(next)
			EnterNextState();
		CallLoopFunction();
		SDL_Delay(g_game.loopdelay);
	}
	current->Exit();
	previous = current;
	current = NULL;
}

void State::Manager::EnterNextState() {
	current->Exit();
	previous = current;
	current = next;
	next = NULL;
	current->Enter();
}

void State::Manager::PollEvent() {
	SDL_Event event;
	unsigned int key;
	int x, y;

	while (SDL_PollEvent (&event)) {
		if (!next) {
			switch (event.type) {
				case SDL_KEYDOWN:
					SDL_GetMouseState(&x, &y);
					key = event.key.keysym.sym;
					current->Keyb(key, key >= 256, false, x, y);
					current->Keyb_spec(event.key.keysym, false);
					break;

				case SDL_KEYUP:
					SDL_GetMouseState(&x, &y);
					key = event.key.keysym.sym;
					current->Keyb(key, key >= 256, true, x, y);
					current->Keyb_spec(event.key.keysym, true);
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					current->Mouse(event.button.button, event.button.state, event.button.x, event.button.y);
					break;

				case SDL_MOUSEMOTION: {
					TVector2 old = cursor_pos;
					cursor_pos.x = event.motion.x;
					cursor_pos.y = event.motion.y;
					current->Motion(event.motion.x-old.x, event.motion.y-old.y);
					break;
				}

				case SDL_JOYAXISMOTION:
					if (Winsys.joystick_isActive()) {
						unsigned int axis = event.jaxis.axis;
						if (axis < 2) {
							float val = (float)event.jaxis.value / 32768;
							current->Jaxis(axis, val);
						}
					}
					break;
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					if (Winsys.joystick_isActive()) {
						current->Jbutt(event.jbutton.button, event.jbutton.state);
					}
					break;

				case SDL_VIDEORESIZE:
					if(Winsys.resolution.width != event.resize.w || Winsys.resolution.height != event.resize.h) {
						Winsys.resolution.width = event.resize.w;
						Winsys.resolution.height = event.resize.h;
						Winsys.SetupVideoMode (param.res_type);
						Reshape(event.resize.w, event.resize.h);
					}
					break;

				case SDL_QUIT:
					quit = true;
					break;
			}
		}
	}
}

void State::Manager::CallLoopFunction() {
	float cur_time = SDL_GetTicks() * 1.e-3;
	g_game.time_step = cur_time - clock_time;
	if (g_game.time_step < 0.0001) g_game.time_step = 0.0001;
	clock_time = cur_time;

	current->Loop(g_game.time_step);
}
