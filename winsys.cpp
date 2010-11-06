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

#include "winsys.h"
#include "ogl.h"
#include "audio.h"
#include "game_ctrl.h"
#include "font.h"

#define USE_JOYSTICK true

CWinsys Winsys;

CWinsys::CWinsys () {
	screen = NULL;
	for (int i=0; i<NUM_GAME_MODES; i++) {
		modefuncs[i].init   = NULL;
		modefuncs[i].loop   = NULL;
		modefuncs[i].term   = NULL;
		modefuncs[i].keyb   = NULL;
		modefuncs[i].mouse  = NULL;
		modefuncs[i].motion = NULL;
		modefuncs[i].jaxis  = NULL;
		modefuncs[i].jbutt  = NULL;
	}
	new_mode = NO_MODE;
	lasttick = 0;

	joystick = NULL;
	numJoysticks = 0;
	joystick_active = false;

	auto_x_resolution = 640;
	auto_y_resolution = 480;
}

CWinsys::~CWinsys () {}

void CWinsys::GetDefaultVideoMode () {
    Uint32 video_flags = SDL_OPENGL; 
    if (param.fullscreen) video_flags |= SDL_FULLSCREEN;

	if ((screen = SDL_SetVideoMode (0, 0, 0, video_flags)) == NULL) return;
	SDL_Surface *surf = SDL_GetVideoSurface ();
	auto_x_resolution = surf->w;
	auto_y_resolution = surf->h;
}

void CWinsys::SetupVideoMode () {
    int bpp = 0;
    int width = 640; 
	int height = 480;
    Uint32 video_flags = SDL_OPENGL;
//	if (param.resizable) video_flags |= SDL_RESIZABLE; 
    if (param.fullscreen) video_flags |= SDL_FULLSCREEN;

    switch (param.bpp_mode ) {
		case 0:	bpp = 0; break;
		case 1:	bpp = 16; break;
		case 2:	bpp = 32; break;
    default: param.bpp_mode = 0; bpp = 0;
    }

	int type = param.res_type;
	if (type > 2 || type < 0) type = 0;
	switch (type) {
		case 0: 
			width = auto_x_resolution;
			height = auto_y_resolution;
			break;
		case 1: width = 800; height = 600; break;
		case 2: width = 1024; height = 768; break;
	}
	if ((screen = SDL_SetVideoMode (width, height, bpp, video_flags)) == NULL) {
		Message ("Couldn't initialize video",  SDL_GetError()); 	
	}
	param.x_resolution = width;
	param.y_resolution = height;
}

void CWinsys::InitJoystick () {
    if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) < 0) {
		Message ("Could not initialize SDL_joystick: %s", SDL_GetError());
		return;
	}
	numJoysticks = SDL_NumJoysticks ();
	if (numJoysticks < 1) {
		joystick = NULL;
		return;		
	}	
	SDL_JoystickEventState (SDL_ENABLE);
	joystick = SDL_JoystickOpen (0);	// first stick with number 0
    if (joystick == NULL) {
		Message ("Cannot open joystick %s", SDL_GetError ());
		return;
    }
	joystick_active = true;
}

void CWinsys::Init (int *argc, char **argv) {
    Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER;
    if (SDL_Init (sdl_flags) < 0) Message ("Could not initialize SDL");
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

	#if defined (USE_STENCIL_BUFFER)
	    SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
	#endif
	
	GetDefaultVideoMode ();
	SetupVideoMode ();
	Reshape (param.x_resolution, param.y_resolution);

    SDL_WM_SetCaption ("","");
	KeyRepeat (false);
	if (USE_JOYSTICK) InitJoystick ();
}

void CWinsys::KeyRepeat (bool repeat) {
	if (repeat) 
		SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	else SDL_EnableKeyRepeat (0, 0);
}

void CWinsys::SetFonttype () {
	if (param.use_papercut_font > 0) {
		FT.SetFont ("pc20");
	} else {
		FT.SetFont ("bold");
	}
}

void CWinsys::Quit () {
	if (joystick_active) SDL_JoystickClose (joystick);	
	Audio.Close ();
	SDL_Quit ();
	Players.SaveParams ();
    exit (0);
}

void CWinsys::PrintJoystickInfo () {
	if (joystick_active == false) {
		Message ("No joystick found");
		return;
	}
	PrintStr ("");
	PrintStr (SDL_JoystickName (0));
	int num_buttons = SDL_JoystickNumButtons (joystick);
	printf ("Joystick has %d button%s\n", num_buttons, num_buttons == 1 ? "" : "s");
	int num_axes = SDL_JoystickNumAxes (joystick);
	printf ("Joystick has %d ax%ss\n\n", num_axes, num_axes == 1 ? "i" : "e");
}

// ------------ modes -------------------------------------------------

void CWinsys::SetModeFuncs (
		TGameMode mode, TInitFuncN init, TLoopFuncN loop, TTermFuncN term,
		TKeybFuncN keyb, TMouseFuncN mouse, TMotionFuncN motion,
		TJAxisFuncN jaxis, TJButtFuncN jbutt) {
    modefuncs[mode].init   = init;
   	modefuncs[mode].loop   = loop;
    modefuncs[mode].term   = term;
    modefuncs[mode].keyb   = keyb;
   	modefuncs[mode].mouse  = mouse;
    modefuncs[mode].motion = motion;
    modefuncs[mode].jaxis  = jaxis;
    modefuncs[mode].jbutt  = jbutt;
}

void CWinsys::IdleFunc () {}

bool CWinsys::ModePending () {
	return g_game.mode != new_mode;
}

void CWinsys::PollEvent () {
    SDL_Event event; 
    unsigned int key, axis;
    int x, y;
	float val;

	while (SDL_PollEvent (&event)) {
		if (ModePending()) {
			IdleFunc ();
    	} else {
			switch (event.type) {
				case SDL_KEYDOWN:
				if (modefuncs[g_game.mode].keyb) {
					SDL_GetMouseState (&x, &y);
					key = event.key.keysym.sym; 
					(modefuncs[g_game.mode].keyb) (key, key >= 256, false, x, y);
				}
				break;
	
				case SDL_KEYUP:
				if (modefuncs[g_game.mode].keyb) {
					SDL_GetMouseState (&x, &y);
					key = event.key.keysym.sym; 
					(modefuncs[g_game.mode].keyb)  (key, key >= 256, true, x, y);
				}
				break;
	
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				if (modefuncs[g_game.mode].mouse) {
					(modefuncs[g_game.mode].mouse) 
						(event.button.button, event.button.state,
						event.button.x, event.button.y);
				}
				break;
	
				case SDL_MOUSEMOTION:
					if (modefuncs[g_game.mode].motion) 
					(modefuncs[g_game.mode].motion) (event.motion.x, event.motion.y);
				break;

				case SDL_JOYAXISMOTION:  
				if (joystick_active) {
					axis = event.jaxis.axis;
					if (modefuncs[g_game.mode].jaxis && axis < 2) {
						val = (float)event.jaxis.value / 32768;
							(modefuncs[g_game.mode].jaxis) (axis, val);
					}
				}
				break; 

				case SDL_JOYBUTTONDOWN:  
				case SDL_JOYBUTTONUP:  
				if (joystick_active) {
					if (modefuncs[g_game.mode].jbutt) {
						(modefuncs[g_game.mode].jbutt) 
							(event.jbutton.button, event.jbutton.state);
					}
				}
				break;

				case SDL_VIDEORESIZE:
					param.x_resolution = event.resize.w;
					param.y_resolution = event.resize.h;
					SetupVideoMode ();
					Reshape (event.resize.w, event.resize.h);
				break;
			
				case SDL_QUIT: 
					Quit ();
				break;
			}
    	}
	}
}

void CWinsys::ChangeMode () {
	// this function is called when new_mode is set
	// terminate function of previous mode
	if (g_game.mode >= 0 &&  modefuncs[g_game.mode].term != 0) 
	    (modefuncs[g_game.mode].term) ();
	g_game.prev_mode = g_game.mode;

	// init function of new mode
	if (modefuncs[new_mode].init != 0) {
		(modefuncs[new_mode].init) ();
		clock_time = SDL_GetTicks() * 1.e-3;
	}

	g_game.mode = new_mode;
	// new mode is now the current mode.
}

void CWinsys::CallLoopFunction () {
		cur_time = SDL_GetTicks() * 1.e-3;
		g_game.time_step = cur_time - clock_time;
		if (g_game.time_step < 0.0001) g_game.time_step = 0.0001;
		clock_time = cur_time;

		if (modefuncs[g_game.mode].loop != 0) 
			(modefuncs[g_game.mode].loop) (g_game.time_step);	
}

void CWinsys::EventLoop () {
    while (true) {
		PollEvent ();
	    if (ModePending()) ChangeMode ();
		CallLoopFunction ();
		SDL_Delay (g_game.loopdelay);
    }
}



