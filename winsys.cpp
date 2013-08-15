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

#include "winsys.h"
#include "ogl.h"
#include "audio.h"
#include "game_ctrl.h"
#include "font.h"
#include "score.h"
#include "textures.h"
#include "spx.h"
#include "course.h"
#include <iostream>

#define USE_JOYSTICK true

TVector2 cursor_pos(0, 0);

CWinsys Winsys;

CWinsys::CWinsys ()
	: auto_resolution(800, 600) {
	screen = NULL;

	joystick = NULL;
	numJoysticks = 0;
	joystick_active = false;

	resolutions[0] = TScreenRes(0, 0);
	resolutions[1] = TScreenRes(800, 600);
	resolutions[2] = TScreenRes(1024, 768);
	resolutions[3] = TScreenRes(1152, 864);
	resolutions[4] = TScreenRes(1280, 960);
	resolutions[5] = TScreenRes(1280, 1024);
	resolutions[6] = TScreenRes(1360, 768);
	resolutions[7] = TScreenRes(1400, 1050);
	resolutions[8] = TScreenRes(1440, 900);
	resolutions[9] = TScreenRes(1680, 1050);
}

const TScreenRes& CWinsys::GetResolution (size_t idx) const {
	if (idx >= NUM_RESOLUTIONS) return auto_resolution;
	return resolutions[idx];
}

string CWinsys::GetResName (size_t idx) const {
	if (idx >= NUM_RESOLUTIONS) return "800 x 600";
	if (idx == 0) return ("auto");
	string line = Int_StrN (resolutions[idx].width);
	line += " x " + Int_StrN (resolutions[idx].height);
	return line;
}

double CWinsys::CalcScreenScale () const {
	if (resolution.height < 768) return 0.78;
	else if (resolution.height == 768) return 1.0;
	else return (resolution.height / 768);
}

void CWinsys::SetupVideoMode (const TScreenRes& resolution_) {
	int bpp = 0;
	Uint32 video_flags = SDL_OPENGL;
	if (param.fullscreen) video_flags |= SDL_FULLSCREEN;
	switch (param.bpp_mode) {
		case 0:
			bpp = 0;
			break;
		case 1:
			bpp = 16;
			break;
		case 2:
			bpp = 32;
			break;
		default:
			param.bpp_mode = 0;
			bpp = 0;
	}
	if ((screen = SDL_SetVideoMode
	              (resolution_.width, resolution_.height, bpp, video_flags)) == NULL) {
		Message ("couldn't initialize video",  SDL_GetError());
		Message ("set to 800 x 600");
		screen = SDL_SetVideoMode (800, 600, bpp, video_flags);
		param.res_type = 1;
		SaveConfigFile ();
	}
	SDL_Surface *surf = SDL_GetVideoSurface ();
	resolution.width = surf->w;
	resolution.height = surf->h;
	if (resolution.width == 0 && resolution.height == 0) {
		auto_resolution = resolution;
	}
	scale = CalcScreenScale ();
	if (param.use_quad_scale) scale = sqrt (scale);
}

void CWinsys::SetupVideoMode (size_t idx) {
	SetupVideoMode (GetResolution(idx));
}

void CWinsys::SetupVideoMode (int width, int height) {
	SetupVideoMode (TScreenRes(width, height));
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

void CWinsys::Init () {
	Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER;
	if (SDL_Init (sdl_flags) < 0) Message ("Could not initialize SDL");
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

#if defined (USE_STENCIL_BUFFER)
	SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
#endif

	SetupVideoMode (GetResolution (param.res_type));
	Reshape (resolution.width, resolution.height);

	SDL_WM_SetCaption (WINDOW_TITLE, WINDOW_TITLE);
	KeyRepeat (false);
	if (USE_JOYSTICK) InitJoystick ();
//	SDL_EnableUNICODE (1);
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

void CWinsys::CloseJoystick () {
	if (joystick_active) SDL_JoystickClose (joystick);
}

void CWinsys::Quit () {
	CloseJoystick ();
	Score.SaveHighScore ();
	SaveMessages ();
	Audio.Close ();		// frees music and sound as well
	FT.Clear ();
	if (g_game.argument < 1) Players.SavePlayers ();
	SDL_Quit ();
}

void CWinsys::Terminate () {
	Quit();
	exit(0);
}

void CWinsys::PrintJoystickInfo () const {
	if (joystick_active == false) {
		Message ("No joystick found");
		return;
	}
	PrintStr ("");
	PrintStr (SDL_JoystickName (0));
	int num_buttons = SDL_JoystickNumButtons (joystick);
	cout << "Joystick has " << num_buttons << " button" << (num_buttons == 1 ? "" : "s") << '\n';
	int num_axes = SDL_JoystickNumAxes (joystick);
	cout << "Joystick has " << num_axes << " ax" << (num_axes == 1 ? "i" : "e") << "s\n\n";
}

unsigned char *CWinsys::GetSurfaceData () const {
	return (unsigned char*)screen->pixels;
}
