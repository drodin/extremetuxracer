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

#ifndef BH_H
#define BH_H

// --------------------------------------------------------------------
//		compiler flags
// --------------------------------------------------------------------

// later these flags should set by a build system (on Linux)
#define HAVE_SDL_MIXER
#define HAVE_SDL 
#define STDC_HEADERS 		
#define TIME_WITH_SYS_TIME	
#define HAVE_FINITE 
#define HAVE_GETCWD 
#define HAVE_GETTIMEOFDAY 
#define HAVE_ISNAN 
#define HAVE_STRDUP 
#define HAVE_GL_GLEXT_H 
#define HAVE_GL_GLX_H 
#define HAVE_SYS_TIME_H 
#define USE_STENCIL_BUFFER

// --------------------------------------------------------------------
//			includes
// --------------------------------------------------------------------

#define OS_LINUX
// #define OS_WIN32_MINGW
// #define OS_WIN32_NATIVE
// #define OS_WIN32_MSC

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <float.h>
#include <sys/stat.h>

#include <string>
#include <iostream>

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL/SDL.h"
#include "SDL/SDL_joystick.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"

#if defined ( OS_LINUX )
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
	#include <dirent.h>
	#include <sys/time.h>
	#include <GL/glx.h>
	#define SEP "/"
#elif defined ( OS_WIN32_MINGW )
	#define SEP "/"
	#include <dirent.h>
	#include <GL/glext.h>
#elif defined ( OS_WIN32_MSC )
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
	#pragma warning (disable:4244)	
	#pragma warning (disable:4305)	
	#pragma warning (disable:4761)	
	#define SEP "\\"
#elif defined ( OS_WIN32_NATIVE )
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
	#define SEP "\\"
#endif

// --------------------------------------------------------------------
//			defines
// --------------------------------------------------------------------

#if defined (WIN32)
#	define SEP "\\"
#else
#	define SEP "/"
#endif

#ifndef S_ISDIR
#   define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif 

#if defined( HAVE_FINITE )
#   define FINITE(x) (finite(x))
#elif defined( HAVE__FINITE )
#   define FINITE(x) (_finite(x))
#elif defined( HAVE_ISNAN )
#   define FINITE(x) (!isnan(x))
#elif defined( HAVE__ISNAN )
#   define FINITE(x) (!_isnan(x))
#else
#   error "You don't have finite(), _finite(), isnan(), or _isnan() on your system!"
#endif

using namespace std;

#include "etr_types.h"
#include "mathlib.h"
#include "common.h"
#include "game_config.h"
#include "winsys.h"
#include "physics.h"

extern TGameData g_game;

#endif
