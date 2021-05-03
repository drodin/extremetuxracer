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

#ifndef BH_H
#define BH_H

// --------------------------------------------------------------------
//			global and or system-dependant includes
// --------------------------------------------------------------------

#include <cstdint>
#include <cstddef>
#include <string>

#include <GL/gl.h>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#ifndef HAVE_CONFIG_H
#	ifdef _WIN32 // Windows platform
#		ifdef _MSC_VER // MSVC compiler
#			define OS_WIN32_MSC
#		else // Assume MinGW compiler
#			define OS_WIN32_MINGW
#		endif
#	else // Assume Unix platform (Linux, Mac OS X, BSD, ...)
#		ifdef __APPLE__
#			define OS_MAC
#		elif defined(__linux__)
#			define OS_LINUX
#		endif
#	endif
#endif // CONFIG_H

#if defined OS_WIN32_MSC // Windows platform
#	include <windows.h>
#	include "glext.h"
#	pragma warning (disable:4244)
#	pragma warning (disable:4305)
#	define SEP "\\"
#	undef DrawText
#	undef GetObject
#	if _MSC_VER < 1900 // VS 2013 or older
#		define constexpr
#	endif
#elif defined OS_WIN32_MINGW
#	include <dirent.h>
#	include <GL/glext.h>
#	define SEP "/"
#else // Assume Unix platform (Linux, Mac OS X, BSD, ...)
#	include <unistd.h>
#	include <sys/types.h>
#	include <pwd.h>
#	include <dirent.h>
#	include <sys/time.h>
#	include <GL/glx.h>
#	define SEP "/"
#endif


#define USE_STENCIL_BUFFER

#include "version.h"
#define WINDOW_TITLE "Extreme Tux Racer " ETR_VERSION_STRING

#include "etr_types.h"
#include "common.h"
#include "game_config.h"

extern TGameData g_game;

#endif // BH_H
