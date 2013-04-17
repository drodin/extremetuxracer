#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([etr], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([gui.cpp])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

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
     CXXFLAGS="$CXXFLAGS -DOS_LINUX" ;;
esac

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor getcwd memmove memset mkdir pow sqrt strchr])

AM_PATH_SDL([1.0.0])
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# TODO: Proper checks instead of hardcoding
LDFLAGS="$LDFLAGS -lGL -lGLU -lSDL_image -lSDL_mixer"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
