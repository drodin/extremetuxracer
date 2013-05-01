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
#include "score.h"
#include "textures.h"
#include "spx.h"
#include "course.h"

#define USE_JOYSTICK true

CWinsys Winsys;

CWinsys::CWinsys () {
	screen = NULL;
	lasttick = 0;

	joystick = NULL;
	numJoysticks = 0;
	joystick_active = false;

 	resolution[0] = MakeRes (0, 0);
	resolution[1] = MakeRes (800, 600);
	resolution[2] = MakeRes (1024, 768);
	resolution[3] = MakeRes (1152, 864);
	resolution[4] = MakeRes (1280, 960);
	resolution[5] = MakeRes (1280, 1024);
	resolution[6] = MakeRes (1360, 768);
	resolution[7] = MakeRes (1400, 1050);
	resolution[8] = MakeRes (1440, 900);
	resolution[9] = MakeRes (1680, 1050);
	
	auto_x_resolution = 800;
	auto_y_resolution = 600;

	lasttick = 0;
	elapsed_time = 0;
	remain_ticks = 0;
}

TScreenRes CWinsys::MakeRes (int width, int height) {
	TScreenRes res;
	res.width = width;
	res.height = height;
	return res;
}

TScreenRes CWinsys::GetResolution (int idx) {
	if (idx < 0 || idx >= NUM_RESOLUTIONS) return MakeRes (800, 600);
	return resolution[idx];
}

string CWinsys::GetResName (int idx) {
	if (idx < 0 || idx >= NUM_RESOLUTIONS) return "800 x 600";
	if (idx == 0) return ("auto");
	string line = Int_StrN (resolution[idx].width);
	line += " x " + Int_StrN (resolution[idx].height);
	return line;
}

double CWinsys::CalcScreenScale () {
	double hh = (double)param.y_resolution;
	if (hh < 768) return 0.78; 
	else if (hh == 768) return 1.0;
	else return (hh / 768);
}

/*
typedef struct SDL_Surface {
    Uint32 flags;                           // Read-only 
    SDL_PixelFormat *format;                // Read-only 
    int w, h;                               // Read-only 
    Uint16 pitch;                           // Read-only 
    void *pixels;                           // Read-write 
    SDL_Rect clip_rect;                     // Read-only 
    int refcount;                           // Read-mostly
} SDL_Surface;
*/

void CWinsys::SetupVideoMode (TScreenRes resolution) {
    int bpp = 0;
    Uint32 video_flags = SDL_OPENGL;
    if (param.fullscreen) video_flags |= SDL_FULLSCREEN;
	switch (param.bpp_mode ) {
		case 0:	bpp = 0; break;
		case 1:	bpp = 16; break;
		case 2:	bpp = 32; break;
		default: param.bpp_mode = 0; bpp = 0;
    }
	if ((screen = SDL_SetVideoMode 
	(resolution.width, resolution.height, bpp, video_flags)) == NULL) {
		Message ("couldn't initialize video",  SDL_GetError()); 
		Message ("set to 800 x 600");
		screen = SDL_SetVideoMode (800, 600, bpp, video_flags);
		param.res_type = 1;
		SaveConfigFile ();
	}
	SDL_Surface *surf = SDL_GetVideoSurface ();
	param.x_resolution = surf->w;
	param.y_resolution = surf->h;
	if (resolution.width == 0 && resolution.height == 0) {
		auto_x_resolution = param.x_resolution;
		auto_y_resolution = param.y_resolution;
	}
 	param.scale = CalcScreenScale ();
	if (param.use_quad_scale) param.scale = sqrt (param.scale);
}

void CWinsys::SetupVideoMode (int idx) {
	if (idx < 0 || idx >= NUM_RESOLUTIONS) SetupVideoMode (MakeRes (800, 600));
	else SetupVideoMode (resolution[idx]);
}

void CWinsys::SetupVideoMode (int width, int height) {
	SetupVideoMode (MakeRes (width, height));
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
	Reshape (param.x_resolution, param.y_resolution);

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
	Tex.FreeTextureList ();
	Course.FreeCourseList ();
	Course.ResetCourse ();
	SaveMessages ();
	Audio.Close ();		// frees music and sound as well
	FT.Clear ();
	if (g_game.argument < 1) Players.SavePlayers ();
	Score.SaveHighScore ();
	SDL_Quit ();
}

void CWinsys::Terminate () {
	Quit();
	exit(0);
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

unsigned char *CWinsys::GetSurfaceData () {
	return (unsigned char*)screen->pixels;
}
