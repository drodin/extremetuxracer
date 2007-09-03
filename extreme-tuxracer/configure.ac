dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/etracer.h)

dnl Check system type
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE(etracer, "SVN Development")
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX

dnl Check for ranlib
AC_PROG_RANLIB

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_CHECK_FUNCS(getcwd gettimeofday strdup)

dnl Check for non-standard floating point functions and headers
AC_CHECK_FUNCS(finite isnan)
AC_CHECK_FUNCS(_finite _isnan)
AC_CHECK_HEADERS(ieeefp.h)

dnl --------------------------------------------------------------------------
dnl General options
dnl --------------------------------------------------------------------------
TR_CPPFLAGS=""
TR_CFLAGS=""
TR_CXXFLAGS=""
TR_LIBS=""

case "$host" in
i*86-*-*) TR_CFLAGS="$TR_CFLAGS";
          TR_CXXFLAGS="$TR_CXXFLAGS";;
alpha*-*-linux-*) TR_CFLAGS="$TR_CFLAGS -mieee";;
powerpc-*-*) TR_CFLAGS="$TR_CFLAGS -fsigned-char";
             R_CXXFLAGS="$TR_CXXFLAGS -fsigned-char";;
esac

#AC_ARG_ENABLE(debug,     [  --enable-debug          Produce an executable with debugging symbols], 
#			 [TR_CFLAGS="-g -Wall"; TR_CXXFLAGS="-g -Wall"],
#			 [TR_CPPFLAGS="$TR_CPPFLAGS -DTUXRACER_NO_ASSERT=1"])


AC_ARG_ENABLE(debug,     [  --enable-debug          Produce an executable with debugging symbols])

if test "x$enable_debug" = "xyes" ; then
	TR_CFLAGS="-g -Wall"
	TR_CXXFLAGS="-g -Wall"
else
	TR_CPPFLAGS="$TR_CPPFLAGS -DTUXRACER_NO_ASSERT=1"
fi

AC_ARG_ENABLE(pedantic,  [  --enable-pedantic       Enable -pedantic (and -ansi for C) when compiling], TR_CFLAGS="$TR_CFLAGS -ansi -pedantic"; TR_CXXFLAGS="$TR_CXXFLAGS -pedantic")

AC_ARG_ENABLE(profile,[  --enable-profile        Produce a profiled executable],  [TR_CFLAGS="$TR_CFLAGS -pg"; TR_CXXFLAGS="$TR_CXXFLAGS -pg"])


dnl --------------------------------------------------------------------------
dnl Check if compiling for Win32
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING([for Win32 platform])
AC_EGREP_CPP(yes,
[ 
#if defined(WIN32) || defined(__CYGWIN__)
  yes
#endif
], is_win32=yes, is_win32=no )
AC_MSG_RESULT([$is_win32])

dnl Add libs required for Windows
dnl I don't use -mwindows since I like having the console around
dnl If the console annoys you then remove "-luser32 -lgdi32" and replace
dnl with "-mwindows"
if test "x$is_win32" = "xyes" ; then
    TR_LIBS="$TR_LIBS -luser32 -lgdi32"
fi

dnl --------------------------------------------------------------------------
dnl Check for X  
dnl Note that we only need libs (so that glut links properly on all systems).
dnl --------------------------------------------------------------------------
if test "x$is_win32" = "xno" ; then
    AC_PATH_XTRA
    if test "x$no_x" = "x" ; then
	TR_LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXi -lXext -lXmu -lXt $X_EXTRA_LIBS $TR_LIBS"
    fi
fi

dnl --------------------------------------------------------------------------
dnl Check for some common libraries
dnl --------------------------------------------------------------------------
AC_CHECK_LIB( dl, main, TR_LIBS="$TR_LIBS -ldl" )
AC_CHECK_LIB(  m, main, TR_LIBS="$TR_LIBS -lm" )



#--------------------------------------------------------------------
# Find and load the tclConfig.sh file
#--------------------------------------------------------------------

AC_ARG_WITH(tcl,[  --with-tcl=PATH       directory with Tcl config file: tclConfig.sh], tcl=$withval, tcl=yes)
AC_ARG_WITH(tcl-inc,[  --with-tcl-inc=PATH       directory with Tcl header files])
AC_ARG_WITH(tcl-lib-name,[  --with-tcl-lib-name=NAME       TCL library name])

if test "x$with_tcl_inc" != "x" && test "x$with_tcl_lib_name" != "x"; then
    TCL_CFLAGS="-I$with_tcl_inc"
    AC_MSG_NOTICE(["Using custom TCL headers directory: $with_tcl_inc"])
    TCL_LIBS="-l$with_tcl_lib_name"
    AC_MSG_NOTICE(["Using custom TCL Library: $with_tcl_lib_name"])
    customtcl="yes"
else
    AC_MSG_CHECKING(for location of tclConfig.sh)

    dirs="$tcl /lib /usr/lib /usr/lib/tcl8.4 /usr/tcl/lib /usr/local/lib /usr/local/tcl-8.4/lib /usr/local/tcl/lib /opt/lib /   mingw/lib /usr/lib/tcl8.3"

    found=0
    if test "$tcl" != "no"; then
    	tcl=no
    	for try in $dirs; do
    		if test -f $try/tclConfig.sh; then		
    			found=1
    			. $try/tclConfig.sh
    			tcl=yes
                customtcl="no"
                if test "x$with_tcl_lib_name" != "x"; then
                    TCL_LIBS="-l$with_tcl_lib_name"
                    AC_MSG_NOTICE(["Using custom TCL Library: $with_tcl_lib_name"])
                else
                    TCL_LIBS="$TCL_LIB_SPEC $TCL_LIBS"
                fi

                if test "x$with_tcl_inc" != "x"; then
                    TCL_CFLAGS="-I$with_tcl_inc"
                    AC_MSG_NOTICE(["Using custom TCL headers directory: $with_tcl_inc"])
                else
                    TCL_CFLAGS="-I$TCL_INC_DIR $TCL_INCLUDE_SPEC"
                fi

    			AC_MSG_RESULT($try/tclConfig.sh)
    			break
    		fi
    	done
    	if test "$found" -eq 0 ; then
    		AC_MSG_ERROR([tclConfig.sh not found - use the --with-tcl option])
    	fi
    fi
fi

dnl --------------------------------------------------------------------------
dnl Check for SDL
dnl --------------------------------------------------------------------------

#SDL_CONFIG="sdl-config"

if test "x$enable_sdl" != "xno" ; then

    SDL_mixer='no'

    SDL_VERSION=1.2.0
    AM_PATH_SDL($SDL_VERSION, have_SDL=yes, have_SDL=no)

    if test "x$have_SDL" = "xyes" ; then
	TR_CPPFLAGS="$TR_CPPFLAGS"
	TR_CFLAGS="$TR_CFLAGS $SDL_CFLAGS"
	TR_CXXFLAGS="$TR_CXXFLAGS $SDL_CFLAGS"
	TR_LIBS="$TR_LIBS $SDL_LIBS"
	SDL_VERSION=`$SDL_CONFIG --version`

	dnl Check for joystick support in SDL
	saved_LIBS="$LIBS"
	LIBS="$saved_LIBS $TR_LIBS"

	AC_CHECK_FUNCS( SDL_JoystickOpen, 
			have_SDL_joystick=yes, have_SDL_joystick=no )

	if test "x$have_SDL_joystick" = "xno" ; then
	    echo "*** This version of SDL doesn't have joystick support."
	    echo "*** Configuring without joystick support."
	fi

	dnl Check for SDL_mixer
	AC_CHECK_LIB( SDL_mixer, Mix_OpenAudio, 
		      have_SDL_mixer=yes, have_SDL_mixer=no )

	if test "x$have_SDL_mixer" = "xyes" ; then 
	    TR_CPPFLAGS="$TR_CPPFLAGS -DHAVE_SDL_MIXER=1" 
	    TR_LIBS="$TR_LIBS -lSDL_mixer"
	else
	    echo "*** SDL_mixer not found.  Configuring without audio support."
	fi

	LIBS="$saved_LIBS"
    else
	AC_MSG_ERROR([SDL not found - use the --with-sdl-* options])
    fi

fi

dnl --------------------------------------------------------------------------
dnl Check for GL library
dnl --------------------------------------------------------------------------
AC_ARG_WITH(gl-libs,      [  --with-gl-libs=DIR      GL/MesaGL library location])
AC_ARG_WITH(gl-lib-name,  [  --with-gl-lib-name=NAME GL library name])

if test "x$with_gl_libs" = "x" ; then
    GL_LDOPTS=""
else
    GL_LDOPTS="-L$with_gl_libs"
fi

saved_LIBS="$LIBS"

AC_DEFUN( CHECK_FOR_GL_LIB, [
    AC_MSG_CHECKING([for $GL_LIB_NAME library])
    LIBS="$saved_LIBS $TR_LIBS $GL_LDOPTS -l$GL_LIB_NAME"
    AC_TRY_LINK( , , have_GL=yes, have_GL=no)
    AC_MSG_RESULT([$have_GL])

    if test "x$have_GL" = "xyes" ; then
	TR_LIBS="$TR_LIBS $GL_LDOPTS -l$GL_LIB_NAME"

    else
	dnl Try with -lpthread

	AC_MSG_CHECKING([for $GL_LIB_NAME library (with pthreads)])
	LIBS="$saved_LIBS $TR_LIBS $GL_LDOPTS -l$GL_LIB_NAME -lpthread"
	AC_TRY_LINK( , , have_GL=yes, have_GL=no)
	AC_MSG_RESULT([$have_GL])

	if test "x$have_GL" = "xyes" ; then
	    TR_LIBS="$TR_LIBS $GL_LDOPTS -l$GL_LIB_NAME -lpthread"
	fi
    fi
])

AC_DEFUN( FIND_GL_LIB, [
    if test "x$with_gl_lib_name" = "x" ; then
        GL_LIB_NAME="GL"
    else
        GL_LIB_NAME="$with_gl_lib_name"
    fi

    CHECK_FOR_GL_LIB

    if test "x$have_GL" = "xno" -a "x$with_gl_lib_name" = "x" ; then
        GL_LIB_LIST=`grep -v -E "^$GL_LIB_NAME\$" <<EOF
GL
MesaGL
opengl32
EOF
`

        for GL_LIB_NAME in $GL_LIB_LIST ; do
            CHECK_FOR_GL_LIB

            if test "x$have_GL" = "xyes" ; then
                break;
            fi
        done
    fi
])

FIND_GL_LIB

if test "x$have_GL" = "xno" -a "x$GL_LDOPTS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenGL libraries installed in the standard"
    echo "*** location (/usr/lib).  I'll check in /usr/X11R6/lib, since"
    echo "*** many distributions (incorrectly) put OpenGL libs there."
    GL_LDOPTS="-L/usr/X11R6/lib"
    FIND_GL_LIB
fi

if test "x$have_GL" = "xno" ; then
    AC_MSG_ERROR([Cannot find GL library])
fi

#dnl Check for glXGetProcAddressARB
#AC_CHECK_FUNCS( glXGetProcAddressARB, has_glx_get_proc=yes,
#                has_glx_get_proc=no )

LIBS="$saved_LIBS"

dnl --------------------------------------------------------------------------
dnl Check for GLU library
dnl --------------------------------------------------------------------------

AC_ARG_WITH(glu-lib-name, [  --with-glu-lib-name=NAME GLU library name])

AC_DEFUN( CHECK_FOR_GLU_LIB, [
    AC_MSG_CHECKING([for $GLU_LIB_NAME library])
    LIBS="$saved_LIBS $TR_LIBS -l$GLU_LIB_NAME"
    AC_TRY_LINK( , , have_GLU=yes, have_GLU=no)
    AC_MSG_RESULT([$have_GLU])
])

if test "x$with_glu_lib_name" = "x" ; then
    GLU_LIB_NAME="GLU"
else
    GLU_LIB_NAME="$with_glu_lib_name"
fi

saved_LIBS="$LIBS"

CHECK_FOR_GLU_LIB

if test "x$have_GLU" = "xno" -a "x$with_glu_lib_name" = "x"; then
    GLU_LIB_LIST=`grep -v -E "^$GLU_LIB_NAME\$" <<EOF
GLU
MesaGLU
glu32
EOF
`

    for GLU_LIB_NAME in $GLU_LIB_LIST ; do
        CHECK_FOR_GLU_LIB

        if test "x$have_GLU" = "xyes" ; then
            break
        fi
    done
fi

if test "x$have_GLU" = "xno" ; then
    AC_MSG_ERROR([Cannot find GLU library])
fi

LIBS="$saved_LIBS"
TR_LIBS="$TR_LIBS -l$GLU_LIB_NAME"


dnl --------------------------------------------------------------------------
dnl Check for OpenGL headers
dnl --------------------------------------------------------------------------

AC_ARG_WITH(gl-inc,   [  --with-gl-inc=DIR       OpenGL header file location])

if test "x$with_gl_inc" = "x" ; then
    GL_CPPFLAGS=""
else 
    GL_CPPFLAGS="-I$with_gl_inc"
fi

dnl check for gl.h
saved_CPPFLAGS="$CPPFLAGS"

AC_DEFUN( CHECK_FOR_GL_H, [
    CPPFLAGS="$saved_CPPFLAGS $GL_CPPFLAGS $TR_CPPFLAGS"

    AC_MSG_CHECKING([for GL/gl.h])
    AC_TRY_CPP( [ #include <GL/gl.h> ], have_gl_h=yes, have_gl_h=no )
    AC_MSG_RESULT([$have_gl_h])
])

CHECK_FOR_GL_H

if test "x$have_gl_h" = "xno" -a "x$GL_CPPFLAGS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenGL headers installed in the standard"
    echo "*** location (/usr/include).  I'll check in /usr/X11R6/include, since"
    echo "*** many distributions (incorrectly) put OpenGL headers there."
    GL_CPPFLAGS="-I/usr/X11R6/include"
    CHECK_FOR_GL_H
fi

if test "x$have_gl_h" = "xno" ; then
    AC_MSG_ERROR([Cannot find GL/gl.h])
fi

if test "x$has_glx_get_proc" = "xyes" ; then 

    dnl Check for glx.h
    AC_CHECK_HEADERS([GL/glx.h], have_glx_h=yes, have_glx_h=no )

    if test "x$have_glx_h" = "xno" ; then
	AC_MSG_ERROR([Cannot find GL/glx.h])
    fi

    dnl Check that glXGetProcAddressARB is defined in glx.h
    AC_MSG_CHECKING([whether glx.h defines glXGetProcAddressARB]);
    AC_TRY_COMPILE( [#include <GL/glx.h>], [ char *foo = (char*) glXGetProcAddressARB; ],
                    glx_h_up_to_date=yes, glx_h_up_to_date=no )
    AC_MSG_RESULT([$glx_h_up_to_date])

    if test "x$glx_h_up_to_date" = "xno" ; then
        AC_MSG_ERROR([Your copy of glx.h is out of date.  You can get a more recent copy from the latest Mesa distribution (http://mesa3d.sourceforge.net).])
    fi
fi

dnl Check for glext.h 
AC_CHECK_HEADERS( "GL/glext.h", have_glext_h=yes, have_glext_h=no ,[#include <GL/gl.h>])

if test "x$have_glext_h" = "xno" ; then
    AC_MSG_ERROR([
*****************************************************************
You are missing the file glext.h.
You can get a copy at
http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
Place it in the directory /usr/include/GL.
*****************************************************************])
fi

dnl Check that glext.h version is recent enough
AC_MSG_CHECKING([whether glext.h is recent enough]);
AC_EGREP_CPP(yes,
[
#include <GL/glext.h>
#if defined(GL_GLEXT_VERSION) && GL_GLEXT_VERSION >= 6
 yes
#endif
], is_glext_h_recent=yes, is_glext_h_recent=no )
AC_MSG_RESULT([$is_glext_h_recent])

if test "x$is_glext_h_recent" = "xno" ; then
    AC_MSG_ERROR([
*****************************************************************
Your copy of glext.h is out of date.
You can get a copy of the latest version at
http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
Place it in the directory /usr/include/GL.
*****************************************************************])
fi

CPPFLAGS="$saved_CPPFLAGS" 
TR_CPPFLAGS="$TR_CPPFLAGS $GL_CPPFLAGS"



PKG_CONFIG="pkg-config"

dnl --------------------------------------------------------------------------
dnl Check for libpng
dnl --------------------------------------------------------------------------
  if test x$with_libpng != xno && test -z "$LIBPNG"; then
    AC_MSG_CHECKING(for libpng12)
    if $PKG_CONFIG --exists libpng12 ; then
        AC_MSG_RESULT(yes)
        PNG='png'
		PNG_DEP_CFLAGS_PACKAGES=libpng12
		PNG_LIBS=`$PKG_CONFIG --libs libpng12`
		PNG_CXXFLAGS=`$PKG_CONFIG --cflags libpng12`
		PNG_VERSION=`$PKG_CONFIG --modversion libpng12`
    else
      AC_MSG_RESULT(no)
      AC_CHECK_LIB(png, png_read_info,
        [AC_CHECK_HEADER(png.h,
          png_ok=yes,
          png_ok=no)],
        AC_MSG_ERROR(*** PNG loader will not be built (PNG library not found) ***), -lz -lm)
      if test "$png_ok" = yes; then
        AC_MSG_CHECKING([for png_structp in png.h])
        AC_TRY_COMPILE([#include <png.h>],
          [png_structp pp; png_infop info; png_colorp cmap; png_create_read_struct;],
          png_ok=yes,
          png_ok=no)
        AC_MSG_RESULT($png_ok)
        if test "$png_ok" = yes; then
          PNG='png'; PNG_LIBS='-lpng -lz'
		  PNG_VERSION='Unknown'
        else
          AC_MSG_ERROR(*** PNG loader will not be built (PNG library is too old) ***)
        fi
      else
       AC_MSG_ERROR(*** PNG loader will not be built (PNG header file not found) ***)
      fi
    fi
  fi

  if test x$with_libpng != xno && test -z "$PNG_LIBS"; then
     AC_MSG_ERROR([
*** Checks for PNG loader failed.])
  fi


dnl --------------------------------------------------------------------------
dnl Check for Freetype library
dnl --------------------------------------------------------------------------

AC_CHECK_FT2

dnl --------------------------------------------------------------------------
dnl Other options
dnl --------------------------------------------------------------------------

dnl Specify data directory
ppdatadir="$datadir/etracer"

AC_ARG_WITH(data-dir,[  --with-data-dir=PATH    Default ppracer data directory], ppdatadir=$withval)

AC_SUBST(ppdatadir)

AC_SUBST(TR_CPPFLAGS)
AC_SUBST(TR_CFLAGS)
AC_SUBST(TR_CXXFLAGS)
AC_SUBST(TR_LIBS)

CPPFLAGS="$CPPFLAGS $TR_CPPFLAGS"
CFLAGS="$CFLAGS $TR_CFLAGS"
CXXFLAGS="$CXXFLAGS $TR_CXXFLAGS $TCL_CFLAGS $PNG_CXXFLAGS $FT2_CFLAGS"
LIBS="$LIBS $TR_LIBS $TCL_LIBS $PNG_LIBS $FT2_LIBS"

AC_OUTPUT(Makefile src/Makefile data/Makefile)

echo 
echo Extreme Tux Racer $VERSION
echo
echo libpng12 ........... $PNG_VERSION

if test "$customtcl" = "yes"; then
    echo tcl include path ... $with_tcl_inc 
    echo tcl library name ... $with_tcl_lib_name
else
    echo tcl ................ $TCL_MAJOR_VERSION.$TCL_MINOR_VERSION$TCL_PATCH_LEVEL
fi

echo SDL ................ $SDL_VERSION

if test "x$have_SDL_mixer" = "xno" ; then
echo SDL_Mixer........... no
else
echo SDL_Mixer........... yes
fi

if test "x$have_SDL_joystick" = "xno" ; then
echo Joystick support ... no
else
echo Joystick support ... yes
fi
echo OpenGL support ..... $GL_LIB_NAME
echo GLU support ........ $GLU_LIB_NAME

echo
echo The binary will be installed in $prefix/bin
echo The data files will be installed in $ppdatadir
echo
echo configure complete, now type \'make\' and pray.
echo
