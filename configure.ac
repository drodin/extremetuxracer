#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([etr], [0.6.99], [https://sourceforge.net/apps/phpbb/extremetuxracer])
AC_CONFIG_SRCDIR([src/gui.cpp])
AC_CONFIG_HEADERS([etr_config.h])

AM_INIT_AUTOMAKE([dist-xz dist-zip])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_FT2

CXXFLAGS="$CXXFLAGS $FT2_CFLAGS"
LIBS="$LIBS $FT2_LIBS"

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

#
case "$host_os" in
  *linux*)
     AC_DEFINE([OS_LINUX], [1], [Linux build]) ;;
  *mingw32*)
     AC_DEFINE([OS_WIN32_MINGW], [1], [MinGW build]) ;;
  *darwin*)
     AC_DEFINE([OS_MAC], [1], [Mac OS X build]) ;;
esac

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor getcwd memmove memset mkdir pow sqrt strchr strdup])

PKG_CHECK_MODULES([SFML_AUDIO], [sfml-audio],
[
  CFLAGS="${CFLAGS} $SFML_AUDIO_CFLAGS"
  LIBS="${LIBS} $SFML_AUDIO_LIBS"
],
[AC_MSG_ERROR([sfml-audio not found])])

PKG_CHECK_MODULES([SFML_WINDOW], [sfml-window],
[
  CFLAGS="${CFLAGS} $SFML_WINDOW_CFLAGS"
  LIBS="${LIBS} $SFML_WINDOW_LIBS"
],
[AC_MSG_ERROR([sfml-window not found])])

PKG_CHECK_MODULES([SFML_GRAPHICS], [sfml-graphics],
[
  CFLAGS="${CFLAGS} $SFML_GRAPHICS_CFLAGS"
  LIBS="${LIBS} $SFML_GRAPHICS_LIBS"
],
[AC_MSG_ERROR([sfml-graphics not found])])

AC_CHECK_LIB([GL], [glBegin], [],
[AC_MSG_ERROR([libGL not found])])

LIBS="$LIBS -lGL"

AC_CHECK_LIB([GLU], [gluBeginCurve], [],
[AC_MSG_ERROR([libGLU not found])])

LIBS="$LIBS -lGLU"

CPPFLAGS="${CPPFLAGS} -DETR_DATA_DIR=\\\"$datadir\\\""

# Request c++11 compatibility
CXXFLAGS="${CXXFLAGS} -std=c++11"

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        build/Makefile
        build/Installer/Makefile
        data/Makefile
        data/char/Makefile
        data/char/beastie/Makefile
        data/char/boris/Makefile
        data/char/samuel/Makefile
        data/char/trixi/Makefile
        data/char/tux/Makefile
        data/courses/Makefile
        data/courses/default/Makefile
        data/courses/default/bumpy_ride/Makefile
        data/courses/default/bunny_hill/Makefile
        data/courses/default/challenge_one/Makefile
        data/courses/default/chinese_wall/Makefile
        data/courses/default/chragis_gagiwaetter/Makefile
        data/courses/default/course/Makefile
        data/courses/default/downhill_fear/Makefile
        data/courses/default/explore_mountains/Makefile
        data/courses/default/frozen_lakes/Makefile
        data/courses/default/frozen_river/Makefile
        data/courses/default/hippo_run/Makefile
        data/courses/default/holygrail/Makefile
        data/courses/default/in_search_of_vodka/Makefile
        data/courses/default/keep_country_tidy/Makefile
        data/courses/default/milos_castle/Makefile
        data/courses/default/path_of_daggers/Makefile
        data/courses/default/penguins_cant_fly/Makefile
        data/courses/default/quiet_river/Makefile
        data/courses/default/secret_valleys/Makefile
        data/courses/default/this_means_something/Makefile
        data/courses/default/tux_at_home/Makefile
        data/courses/default/twisty_slope/Makefile
        data/courses/default/wild_mountains/Makefile
        data/env/Makefile
        data/env/etr/Makefile
        data/env/etr/cloudy/Makefile
        data/env/etr/evening/Makefile
        data/env/etr/night/Makefile
        data/env/etr/sunny/Makefile
        data/env/tuxracer/Makefile
        data/env/tuxracer/cloudy/Makefile
        data/env/tuxracer/evening/Makefile
        data/env/tuxracer/night/Makefile
        data/env/tuxracer/sunny/Makefile
        data/fonts/Makefile
        data/music/Makefile
        data/objects/Makefile
        data/players/Makefile
        data/sounds/Makefile
        data/terrains/Makefile
        data/textures/Makefile
        data/translations/Makefile
        doc/Makefile
        resources/Makefile
        resources/gui/Makefile
])
AC_OUTPUT
