/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* This file is complex.  However, the ultimate purpose is to make
   adding new configuration parameters easy.  Here's what you need to
   do to add a new parameter:

   1. Choose a name and type for the parameter.  By convention,
   parameters have lower case names and words_are_separated_like_this.
   Possible types are bool, int, char, and string.  (Nothing is ruling
   out floating point types; I just haven't needed them.)  As an
   example in the subsequent steps, suppose we wish to add a parameter
   foo_bar of type string.

   2. Add a field for the parameter to the 'params' struct defined
   below.  In our example, we would add the line
       struct param foo_bar;
   to the definition of struct params.

   Note that the order of the entries in this struct determines the
   order that the parameters will appear in the configuration file.

   3. Initialize and assign a default value to the parameter in the
   init_game_configuration() function.  The INIT_PARAM_<TYPE> macros
   will do this for you.  In our example, we would add the line
       INIT_PARAM_STRING( foo_bar, "baz" )
   to assign a default value of "baz" to the parameter foo_bar.

   4. Create the getparam/setparam functions for the parameter.  This
   is done using the FN_PARAM_<TYPE> macros.  In our example, we would
   add the line 
       FN_PARAM_STRING( foo_bar )
   somewhere in the top-level scope of this file (to keep things neat
   group it with the other definitions).  The will create
   getparam_foo_bar() and setparam_foo_bar() functions that can be
   used to query the value of the parameter.

   5. Create the prototypes for the getparam/setparam functions.  This
   is done in game_config.h using the PROTO_PARAM_<TYPE> macros.  In
   our example, we would add the line
       PROTO_PARAM_STRING( foo_bar );
   to game_config.h.

   6. You're done!  */

#include "file_util.h"
#include "game_config.h"
#include "string_util.h"
#include "course_mgr.h"
#include "winsys.h"
#include "ppgltk/audio/audio.h"


#if defined( WIN32 )
#  define OLD_CONFIG_FILE "etracer.cfg"
#else
#  define OLD_CONFIG_FILE ".etracer"
#endif /* defined( WIN32 ) */

#if defined( WIN32 )
#  define CONFIG_DIR "config"
#  define CONFIG_FILE "options.txt"
#else
#  define CONFIG_DIR ".etracer"
#  define CONFIG_FILE "options"
#endif /* defined( WIN32 ) */

#ifndef DATA_DIR
#  if defined( WIN32 )
#    define DATA_DIR "."
#  else
#    define DATA_DIR PP_DATADIR
#  endif /* defined( WIN32 ) */
#endif




static const char* sp_config_file=NULL;


/* Identifies the parameter type */
typedef enum {
    PARAM_STRING,
    PARAM_CHAR,
    PARAM_INT,
    PARAM_BOOL
} param_type;

/* Stores the value for all types */
typedef union {
    char* string_val;
    char  char_val;
    int   int_val;
    bool bool_val;
} param_val;

/* Stores state for each parameter */
struct param {
    int loaded;
    char *name;
    param_type type;
    param_val val;
    param_val deflt;
    char *comment;
};

/*
 * These macros are used to initialize parameter values
 */

#define INIT_PARAM( nam, val, typename, commnt ) \
   Params.nam.loaded = false; \
   Params.nam.name = #nam; \
   Params.nam.deflt.typename ## _val  = val; \
   Params.nam.comment = commnt;

#define INIT_PARAM_STRING( nam, val, commnt ) \
   INIT_PARAM( nam, val, string, commnt ); \
   Params.nam.type = PARAM_STRING;

#define INIT_PARAM_CHAR( nam, val, commnt ) \
   INIT_PARAM( nam, val, char, commnt ); \
   Params.nam.type = PARAM_CHAR;

#define INIT_PARAM_INT( nam, val, commnt ) \
   INIT_PARAM( nam, val, int, commnt ); \
   Params.nam.type = PARAM_INT;

#define INIT_PARAM_BOOL( nam, val, commnt ) \
   INIT_PARAM( nam, val, bool, commnt ); \
   Params.nam.type = PARAM_BOOL;


/*
 * These functions are used to get and set parameter values
 */

void fetch_param_string( struct param *p )
{
    const char *val;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    if ( val == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( val );
    }
    p->loaded = true;

}

void set_param_string( struct param *p, CONST84 char *new_val )
{
    const char *ret;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    if ( p->loaded ) {
	free( p->val.string_val );
    }
    ret = Tcl_SetVar( tclInterp, p->name, new_val, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( new_val );
    }
    p->loaded = true;

}

void fetch_param_char( struct param *p )
{
    const char *str_val;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL || str_val[0] == '\0' ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = str_val[0];
    }
    p->loaded = true;
}

void set_param_char( struct param *p, char new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    buff[0] = new_val;
    buff[1] = '\0';

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_int( struct param *p )
{
    CONST84 char *str_val;
    int val;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );
    
    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL 
	 || Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR  ) 
    {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = val;
    }
    p->loaded = true;
}

void set_param_int( struct param *p, int new_val )
{
    char buff[30];
    const char *ret;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_bool( struct param *p )
{
    CONST84 char *str_val;
    int val;
    bool no_val = false;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL ) {
	no_val = true;
    } else if ( strcmp( str_val, "false" ) == 0 ) {
	p->val.bool_val = false;
    } else if ( strcmp( str_val, "true" ) == 0 ) {
	p->val.bool_val = true;
    } else if ( Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR ) {
	no_val = true;
    } else {
	p->val.bool_val = (val == 0) ? false : true ;
    }

    if ( no_val ) {
	p->val.bool_val = p->deflt.bool_val;
    }

    p->loaded = true;
}

void set_param_bool( struct param *p, bool new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val ? 1 : 0 );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.bool_val = p->deflt.bool_val;
    } else {
	p->val.bool_val = new_val;
    }
    p->loaded = true;
}


/*
 * Creates set/get functions for each parameter
 */
#define FN_PARAM( name, typename, type ) \
    type getparam_ ## name() { \
        if ( !Params.name.loaded ) { \
            fetch_param_ ## typename( &( Params.name ) ); \
        } \
        return Params.name.val.typename ## _val; \
    } \
    void setparam_ ## name( type val) { \
        set_param_ ## typename( &( Params.name ), val ); } 

#define FN_PARAM_STRING( name ) \
    FN_PARAM( name, string, char* )

#define FN_PARAM_CHAR( name ) \
    FN_PARAM( name, char, char )

#define FN_PARAM_INT( name ) \
    FN_PARAM( name, int, int )

#define FN_PARAM_BOOL( name ) \
    FN_PARAM( name, bool, bool )


/*
 * Main parameter struct
 */
struct params {
    struct param data_dir;
    struct param fullscreen;
    struct param x_resolution;
    struct param y_resolution;
    struct param x_resolution_half_width;		
    struct param bpp_mode;
    struct param capture_mouse; 
    struct param force_window_position;
    struct param quit_key;
    struct param turn_left_key;
    struct param turn_right_key;
    struct param trick_modifier_key;
    struct param brake_key;
    struct param paddle_key;
    struct param jump_key;
    struct param reset_key;
    struct param follow_view_key;
    struct param behind_view_key;
    struct param above_view_key;
    struct param view_mode; /* coresponds to view_mode_t */
    struct param screenshot_key;
    struct param pause_key;

    struct param joystick_paddle_button;
    struct param joystick_brake_button;
    struct param joystick_jump_button;
    struct param joystick_trick_button;
    struct param joystick_continue_button;
    struct param joystick_x_axis;
    struct param joystick_y_axis;
	struct param disable_joystick;

    struct param no_audio;
    struct param sound_enabled;
    struct param music_enabled;
    struct param sound_volume; /* 0-128 */
    struct param music_volume; /* 0-128 */
    struct param audio_freq_mode; /* 0 = 11025, 
				     1 = 22050, 
				     2 = 44100 */
    struct param audio_format_mode; /* 0 = 8 bits, 
				       1 = 16 bits */
    struct param audio_stereo; 
    struct param audio_buffer_size; 

    struct param display_fps;
	struct param display_course_percentage;	
	struct param course_detail_level;
    struct param forward_clip_distance;
    struct param backward_clip_distance;
    struct param tree_detail_distance;
    struct param terrain_blending;
    struct param perfect_terrain_blending;
    struct param terrain_envmap;
    struct param disable_fog;
		
    struct param stencil_buffer;
	struct param enable_fsaa;	
	struct param multisamples;
		
	struct param always_save_event_race_data;
		
	struct param draw_tux_shadow;	
    struct param tux_sphere_divisions;
    struct param tux_shadow_sphere_divisions;
    struct param draw_particles;
    struct param track_marks;
    struct param ui_snow;
    struct param nice_fog;
    struct param use_cva;
    struct param cva_hack;
    struct param use_sphere_display_list;
    struct param do_intro_animation;
    struct param mipmap_type; /* 0 = GL_NEAREST,
				 1 = GL_LINEAR,
				 2 = GL_NEAREST_MIPMAP_NEAREST,
				 3 = GL_LINEAR_MIPMAP_NEAREST,
				 4 = GL_NEAREST_MIPMAP_LINEAR,
				 5 = GL_LINEAR_MIPMAP_LINEAR
			      */
    struct param ode_solver; /* 0 = Euler,
				1 = ODE23,
				2 = ODE45
			     */
    struct param fov; 
    struct param debug; 
    struct param warning_level; 
    struct param write_diagnostic_log;
	struct param disable_collision_detection;
	struct param ui_language;
	struct param disable_videomode_autodetection;			
};

static struct params Params;


/*
 * Initialize parameter data
 */

void init_game_configuration()
{
    INIT_PARAM_STRING( 
	data_dir, DATA_DIR, 
	"# The location of the ET Racer data files" );

	INIT_PARAM_BOOL( 
	stencil_buffer, false, 
	"# Set this to true to activate the stencil buffer" );
	
	INIT_PARAM_BOOL( 
	enable_fsaa, false, 
	"# Set this to true to activate FSAA" );

	INIT_PARAM_INT( 
	multisamples, 2,
	"# Set multisamples for FSAA" );
	
    INIT_PARAM_BOOL( 
	draw_tux_shadow, false, 
	"# Set this to true to display Tux's shadow.  Note that this is a \n"
	"# hack and is quite expensive in terms of framerate.\n"
	"# [EXPERT] This looks better if your card has a stencil buffer; \n"
	"# if compiling use the --enable-stencil-buffer configure option \n"
	"# to enable the use of the stencil buffer" );
	
	

    INIT_PARAM_BOOL( 
	draw_particles, true,
	"# Controls the drawing of snow particles that are kicked up as Tux\n"
	"# turns and brakes.  Setting this to false should help improve \n"
	"# performance." );

    INIT_PARAM_INT( 
	tux_sphere_divisions, 15,
	"# [EXPERT] Higher values result in a more finely subdivided mesh \n"
	"# for Tux, and vice versa.  If you're experiencing low framerates,\n"
	"# try lowering this value." );

    INIT_PARAM_INT( 
	tux_shadow_sphere_divisions, 3,
	"# [EXPERT] The level of subdivision of Tux's shadow." );

    INIT_PARAM_BOOL( 
	nice_fog, false,
	"# [EXPERT] If true, then the GL_NICEST hint will be used when\n"
	"# rendering fog.  On some cards, setting this to false may improve\n"
	"# performance.");

    INIT_PARAM_BOOL( 
	use_sphere_display_list, true,
	"# [EXPERT]  Mesa 3.1 sometimes renders Tux strangely when display \n"
	"# lists are used.  Setting this to false should solve the problem \n"
	"# at the cost of a few Hz." );

    INIT_PARAM_BOOL( 
	display_fps, false,
	"# Set this to true to display the current framerate in Hz." );

    INIT_PARAM_BOOL( 
	display_course_percentage, true,
	"# Set this to true to display a progressbar of \n"
	"# the course percentage." );

    INIT_PARAM_INT( 
	x_resolution, 800,
	"# The horizontal size of the Tux Racer window" );

    INIT_PARAM_INT( 
	y_resolution, 600,
	"# The vertical size of the Tux Racer window" );

	INIT_PARAM_BOOL( 
	x_resolution_half_width, false, 
	"# Set this to true to use only half of the resolution width" );

    INIT_PARAM_BOOL( 
	capture_mouse, false,
	"# If true, then the mouse will not be able to leave the \n"
	"# Tux Racer window.\n"
	"# If you lose keyboard focus while running Tux Racer, try setting\n"
	"# this to true." );

    INIT_PARAM_BOOL( 
	do_intro_animation, true,
	"# If false, then the introductory animation sequence will be skipped." 
	);

    INIT_PARAM_INT( 
	mipmap_type, 5,
	"# [EXPERT] Allows you to control which type of texture\n"
	"# interpolation/mipmapping is used when rendering textures.  The\n"
	"# values correspond to the following OpenGL settings:\n"
	"#\n"
        "#  0: GL_NEAREST\n"
        "#  1: GL_LINEAR\n"
        "#  2: GL_NEAREST_MIPMAP_NEAREST\n"
	"#  3: GL_LINEAR_MIPMAP_NEAREST\n"
        "#  4: GL_NEAREST_MIPMAP_LINEAR\n"
        "#  5: GL_LINEAR_MIPMAP_LINEAR\n"
	"#\n"
	"# On some cards, you may be able to improve performance by\n"
        "# decreasing this number, at the cost of lower image quality." );

    INIT_PARAM_BOOL( 
	fullscreen, true,
	"# If true then the game will run in full-screen mode." );

    INIT_PARAM_INT( 
	bpp_mode, 0,
	"# Controls how many bits per pixel are used in the game.\n"
	"# Valid values are:\n"
	"#\n"
	"#  0: Use current bpp setting of operating system\n"
	"#  1: 16 bpp\n"
	"#  2: 32 bpp\n"
	"# Note that some cards (e.g., Voodoo1, Voodoo2, Voodoo3) only support\n"
	"# 16 bits per pixel." );

    INIT_PARAM_BOOL( 
	force_window_position, false ,
	"# If true, then the Tux Racer window will automatically be\n"
	"# placed at (0,0)" );

    INIT_PARAM_INT( 
	ode_solver, 2 ,
	"# Selects the ODE (ordinary differential equation) solver.  \n"
	"# Possible values are:\n"
	"#\n"
	"#   0: Modified Euler     (fastest but least accurate)\n"
        "#   1: Runge-Kutta (2,3)\n"
	"#   2: Runge-Kutta (4,5)  (slowest but most accurate)" );

    INIT_PARAM_STRING( 
	quit_key, "q escape" ,
	"# Key binding for quitting a race" );
    INIT_PARAM_INT( 
	turn_left_key, SDLK_LEFT ,
	"# Key binding for turning left" );
    INIT_PARAM_INT( 
	turn_right_key, SDLK_RIGHT ,
	"# Key binding for turning right" );
    INIT_PARAM_INT( 
	trick_modifier_key, 't' ,
	"# Key binding for doing tricks" );
    INIT_PARAM_INT( 
	brake_key, SDLK_DOWN ,
	"# Key binding for braking" );
    INIT_PARAM_INT( 
	paddle_key, SDLK_UP ,
	"# Key binding for paddling (on the ground) and flapping (in the air)" 
	);
    INIT_PARAM_STRING( 
	follow_view_key, "1" ,
	"# Key binding for the \"Follow\" camera mode" );
    INIT_PARAM_STRING( 
	behind_view_key, "2" ,
	"# Key binding for the \"Behind\" camera mode" );
    INIT_PARAM_STRING( 
	above_view_key, "3" ,
	"# Key binding for the \"Above\" camera mode" );
    INIT_PARAM_INT( 
	view_mode, 1 ,
	"# Default view mode. Possible values are\n" 
	"#\n"
	"#   0: Behind\n"
	"#   1: Follow\n"
	"#   2: Above" );
    INIT_PARAM_STRING( 
	screenshot_key, "=" ,
	"# Key binding for taking a screenshot" );
    INIT_PARAM_STRING( 
	pause_key, "p" ,
	"# Key binding for pausing the game" );
    INIT_PARAM_INT( 
	reset_key, 'r' ,
	"# Key binding for resetting the player position" );
    INIT_PARAM_INT( 
	jump_key, 'e' ,
	"# Key binding for jumping" );

    INIT_PARAM_INT( 
	joystick_paddle_button, 0 ,
	"# Joystick button for paddling (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_brake_button, 2 ,
	"# Joystick button for braking (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_jump_button, 3 ,
	"# Joystick button for jumping (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_trick_button, 1 ,
	"# Joystick button for doing tricks (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_continue_button, 0 ,
	"# Joystick button for moving past intro, paused, and \n"
	"# game over screens (numbering starts at 0)" );
    
    INIT_PARAM_INT(
	joystick_x_axis, 0 ,
	"# Joystick axis to use for turning (numbering starts at 0)" );

    INIT_PARAM_INT(
	joystick_y_axis, 1 ,
	"# Joystick axis to use for paddling/braking (numbering starts at 0)" );
   
	INIT_PARAM_BOOL(
	disable_joystick, false ,
	"# Disables the joystick support" );

    INIT_PARAM_INT( 
	fov, 60 ,
	"# [EXPERT] Sets the camera field-of-view" );
    INIT_PARAM_STRING( 
	debug, "" ,
	"# [EXPERT] Controls the Tux Racer debugging modes" );
    INIT_PARAM_INT( 
	warning_level, 100 ,
	"# [EXPERT] Controls the Tux Racer warning messages" );
    INIT_PARAM_INT( 
	forward_clip_distance, 100 ,
	"# Controls how far ahead of the camera the course\n"
	"# is rendered.  Larger values mean that more of the course is\n"
	"# rendered, resulting in slower performance. Decreasing this \n"
	"# value is an effective way to improve framerates." );
    INIT_PARAM_INT( 
	backward_clip_distance, 10 ,
	"# [EXPERT] Some objects aren't yet clipped to the view frustum, \n"
	"# so this value is used to control how far up the course these \n"
	"# objects are drawn." );
    INIT_PARAM_INT( 
	tree_detail_distance, 20 ,
	"# [EXPERT] Controls the distance at which trees are drawn with \n"
	"# two rectangles instead of one." );
    INIT_PARAM_BOOL( 
	terrain_blending, true ,
	"# Controls the blending of the terrain textures.  Setting this\n"
	"# to false will help improve performance." );
    INIT_PARAM_BOOL( 
	perfect_terrain_blending, false ,
	"# [EXPERT] If true, then terrain triangles with three different\n"
	"# terrain types at the vertices will be blended correctly\n"
	"# (instead of using a faster but imperfect approximation)." );
    INIT_PARAM_BOOL( 
	terrain_envmap, true ,
	"# If true, then the ice will be drawn with an \"environment map\",\n"
	"# which gives the ice a shiny appearance.  Setting this to false\n"
	"# will help improve performance." );
    INIT_PARAM_BOOL( 
	disable_fog, false ,
	"# If true, then fog will be turned off.  Some Linux drivers for the\n"
	"# ATI Rage128 seem to have a bug in their fog implementation which\n"
	"# makes the screen nearly pure white when racing; if you experience\n"
	"# this problem then set this variable to true." );
    INIT_PARAM_BOOL( 
	use_cva, true ,
	"# [EXPERT] If true, then compiled vertex arrays will be used when\n"
	"# drawing the terrain.  Whether or not this helps performance\n"
	"# is driver- and card-dependent." );
    INIT_PARAM_BOOL( 
	cva_hack, true ,
	"# Some card/driver combinations render the terrrain incorrectly\n"
	"# when using compiled vertex arrays.  This activates a hack \n"
	"# to work around that problem." );
    INIT_PARAM_INT( 
	course_detail_level, 75 ,
	"# [EXPERT] This controls how accurately the course terrain is \n"
	"# rendered. A high value results in greater accuracy at the cost of \n"
	"# performance, and vice versa.  This value can be decreased and \n"
	"# increased in 10% increments at runtime using the F9 and F10 keys.\n"
	"# To better see the effect, activate wireframe mode using the F11 \n"
	"# key (this is a toggle)." );
    INIT_PARAM_BOOL( 
	no_audio, false ,
	"# If true, then audio in the game is completely disabled." );
    INIT_PARAM_BOOL( 
	sound_enabled, true ,
	"# Use this to turn sound effects on and off." );
    INIT_PARAM_BOOL( 
	music_enabled, true ,
	"# Use this to turn music on and off." );
    INIT_PARAM_INT( 
	sound_volume, 64 ,
	"# This controls the sound volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	music_volume, 127 ,
	"# This controls the music volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	audio_freq_mode, 1 ,
	"# The controls the frequency of the audio.  Valid values are:\n"
	"# \n"
	"#   0: 11025 Hz\n"
	"#   1: 22050 Hz\n"
	"#   2: 44100 Hz" );
    INIT_PARAM_INT( 
	audio_format_mode, 1 ,
	"# This controls the number of bits per sample for the audio.\n"
	"# Valid values are:\n"
	"#\n"
	"#   0: 8 bits\n"
	"#   1: 16 bits" );
    INIT_PARAM_BOOL( 
	audio_stereo, true ,
	"# Audio will be played in stereo of true, and mono if false" );
    INIT_PARAM_INT( 
	audio_buffer_size, 2048 ,
	"# [EXPERT] Controls the size of the audio buffer.  \n"
	"# Increase the buffer size if you experience choppy audio\n" 
	"# (at the cost of greater audio latency)" );
    INIT_PARAM_BOOL( 
	track_marks, true ,
	"# If true, then the players will leave track marks in the snow." );
    INIT_PARAM_BOOL( 
	ui_snow, true ,
	"# If true, then the ui screens will have falling snow." );

    INIT_PARAM_BOOL( 
	write_diagnostic_log, false ,
	"# If true, then a file called diagnostic_log.txt will be generated\n" 
	"# which you should attach to any bug reports you make.\n"
	"# To generate the file, set this variable to \"true\", and\n"
	"# then run the game so that you reproduce the bug, if possible."
	);
	
    INIT_PARAM_BOOL( 
	always_save_event_race_data, false ,
	"# only for cheating purpose"
	);	
	
	INIT_PARAM_BOOL( 
	disable_collision_detection, false ,
	"# If true, collision detection with tree models is disabled"
	);
	
	INIT_PARAM_BOOL( 
	disable_videomode_autodetection, false, 
	"# Set this to true disable the autodetection\n"
	"# for available video modes." );
		
	INIT_PARAM_STRING( 
	ui_language, "en_GB" ,
	"# set the language for the ui"
	);
	
}


/* 
 * Create the set/get functions for parameters
 */

FN_PARAM_STRING( data_dir )
FN_PARAM_BOOL( draw_tux_shadow )
FN_PARAM_BOOL( draw_particles )
FN_PARAM_INT( tux_sphere_divisions )
FN_PARAM_INT( tux_shadow_sphere_divisions )
FN_PARAM_BOOL( nice_fog )
FN_PARAM_BOOL( use_sphere_display_list )
FN_PARAM_BOOL( display_fps )
FN_PARAM_BOOL( display_course_percentage )
FN_PARAM_INT( x_resolution )
FN_PARAM_INT( y_resolution )
FN_PARAM_BOOL( x_resolution_half_width )
FN_PARAM_BOOL( capture_mouse )
FN_PARAM_BOOL( do_intro_animation )
FN_PARAM_INT( mipmap_type )
FN_PARAM_BOOL( fullscreen )
FN_PARAM_INT( bpp_mode )
FN_PARAM_BOOL( force_window_position )
FN_PARAM_INT( ode_solver )
FN_PARAM_STRING( quit_key )

FN_PARAM_INT( turn_left_key )
FN_PARAM_INT( turn_right_key )
FN_PARAM_INT( trick_modifier_key )
FN_PARAM_INT( brake_key )
FN_PARAM_INT( paddle_key )
FN_PARAM_STRING( above_view_key )
FN_PARAM_STRING( behind_view_key )
FN_PARAM_STRING( follow_view_key )
FN_PARAM_INT( view_mode )
FN_PARAM_STRING( screenshot_key )
FN_PARAM_STRING( pause_key )
FN_PARAM_INT( reset_key )
FN_PARAM_INT( jump_key )
FN_PARAM_INT( joystick_jump_button )
FN_PARAM_INT( joystick_brake_button )
FN_PARAM_INT( joystick_paddle_button )
FN_PARAM_INT( joystick_trick_button )
FN_PARAM_INT( joystick_continue_button )
FN_PARAM_INT( joystick_x_axis )
FN_PARAM_INT( joystick_y_axis )
FN_PARAM_BOOL ( disable_joystick )
FN_PARAM_INT( fov )
FN_PARAM_STRING( debug )
FN_PARAM_INT( warning_level )
FN_PARAM_INT( forward_clip_distance )
FN_PARAM_INT( backward_clip_distance )
FN_PARAM_INT( tree_detail_distance )
FN_PARAM_INT( course_detail_level )
FN_PARAM_BOOL( terrain_blending )
FN_PARAM_BOOL( perfect_terrain_blending )
FN_PARAM_BOOL( terrain_envmap )
FN_PARAM_BOOL( disable_fog )
FN_PARAM_BOOL( use_cva )
FN_PARAM_BOOL( cva_hack )
FN_PARAM_BOOL( track_marks )
FN_PARAM_BOOL( ui_snow )

FN_PARAM_BOOL( no_audio )
FN_PARAM_BOOL( sound_enabled )
FN_PARAM_BOOL( music_enabled )
FN_PARAM_INT( sound_volume )
FN_PARAM_INT( music_volume )
FN_PARAM_INT( audio_freq_mode )
FN_PARAM_INT( audio_format_mode )
FN_PARAM_BOOL( audio_stereo )
FN_PARAM_INT( audio_buffer_size )
FN_PARAM_BOOL( write_diagnostic_log )

FN_PARAM_BOOL( stencil_buffer )
FN_PARAM_BOOL( enable_fsaa )
FN_PARAM_INT( multisamples )

FN_PARAM_BOOL( always_save_event_race_data )
FN_PARAM_BOOL( disable_collision_detection )
FN_PARAM_BOOL( disable_videomode_autodetection )

FN_PARAM_STRING( ui_language )


/*
 * Functions to read and write the configuration file
 */

int get_old_config_file_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( OLD_CONFIG_FILE ) +1 > len ) {
	return 1;
    }
    strcpy( buff, OLD_CONFIG_FILE );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( OLD_CONFIG_FILE ) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, OLD_CONFIG_FILE );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_dir_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( CONFIG_DIR ) +1 > len ) {
	return 1;
    }
    strcpy( buff, CONFIG_DIR );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( CONFIG_DIR) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, CONFIG_DIR );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_file_name( char *buff, unsigned int len )
{
    if (get_config_dir_name( buff, len ) != 0) {
	return 1;
    }
    if ( strlen( buff ) + strlen( CONFIG_FILE ) +2 > len ) {
	return 1;
    }

#if defined( WIN32 ) 
    strcat( buff, "\\" );
#else
    strcat( buff, "/" );
#endif /* defined( WIN32 ) */

    strcat( buff, CONFIG_FILE);
    return 0;
}

void clear_config_cache()
{
    struct param *parm;
    unsigned int i;

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	parm->loaded = false;
    }
}

void read_config_file(std::string& file)
{
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];

    clear_config_cache();

	if( !file.empty()){
		if ( Tcl_EvalFile( tclInterp, FUCKTCL file.c_str() ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", file.c_str(),
			      Tcl_GetStringResult( tclInterp ) );
	    }	
		sp_config_file = file.c_str();	
		return;
	}else{
		sp_config_file = NULL;
	}
	
    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
		return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
		return;
    }

	

    if ( dir_exists( config_dir ) ) {
	if ( file_exists( config_file ) ) {
	    /* File exists -- let's try to evaluate it. */
	    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", config_file,
			      Tcl_GetStringResult( tclInterp ) );
	    }
	}
	return;
    }

    /* File does not exist -- look for old version */
    if ( get_old_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( !file_exists( config_file ) ) {
	return;
    }
    /* Old file exists -- let's try to evaluate it. */
    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
	handle_error( 1, "error evalating deprecated %s: %s", config_file,
		      Tcl_GetStringResult( tclInterp ) );
    } else {
	/* Remove old file and save info in new file location */
	remove(config_file);
	write_config_file();
    }
}

void write_config_file()
{
    FILE *config_stream;
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];
    struct param *parm;
    unsigned int i;
	
	if(sp_config_file==NULL){

    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
	return;
    }

    if ( !dir_exists( config_dir ) ) {

#if defined(WIN32) && !defined(__CYGWIN__)
	if (mkdir( config_dir ) != 0) {
	    return;
	}
#else
	if (mkdir( config_dir, 0775) != 0) {
	    return;
	}
#endif

    }

    config_stream = fopen( config_file, "w" );
	if ( config_stream == NULL ) {
	print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       config_file, strerror(errno) );
	return;
    }
	
	}else{
		std::cout << "Writing to custom config file: "
				  << sp_config_file << std::endl;
		config_stream = fopen( sp_config_file, "w" );
		if ( config_stream == NULL ) {
			print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       sp_config_file, strerror(errno) );
			return;
    	}
	}
	
    fprintf( config_stream, 
	     "# PP Racer " VERSION " configuration file\n"
	     "#\n"
	);

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	if ( parm->comment != NULL ) {
	    fprintf( config_stream, "\n# %s\n#\n%s\n#\n", 
		     parm->name, parm->comment );
	}
	switch ( parm->type ) {
	case PARAM_STRING:
	    fetch_param_string( parm );
	    fprintf( config_stream, "set %s \"%s\"\n",
		     parm->name, parm->val.string_val );
	    break;
	case PARAM_CHAR:
	    fetch_param_char( parm );
	    fprintf( config_stream, "set %s %c\n",
		     parm->name, parm->val.char_val );
	    break;
	case PARAM_INT:
	    fetch_param_int( parm );
	    fprintf( config_stream, "set %s %d\n",
		     parm->name, parm->val.int_val );
	    break;
	case PARAM_BOOL:
	    fetch_param_bool( parm );
	    fprintf( config_stream, "set %s %s\n",
		     parm->name, parm->val.bool_val ? "true" : "false" );
	    break;
	default:
	    code_not_reached();
	}
    }

    if ( fclose( config_stream ) != 0 ) {
	perror( "fclose" );
    }
}

/*
 * Tcl callback to allow reading of game configuration variables from Tcl.
 */
static int get_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int num_params;
    struct param *parm;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Get value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	fetch_param_string( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( parm->val.string_val, -1 ) );
	break;

    case PARAM_CHAR:
	fetch_param_char( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( &parm->val.char_val, 1 ) );
	break;

    case PARAM_INT:
	fetch_param_int( parm );
	Tcl_SetObjResult( ip, Tcl_NewIntObj( parm->val.int_val ) );
	break;

    case PARAM_BOOL:
	fetch_param_bool( parm );
	Tcl_SetObjResult( ip, Tcl_NewBooleanObj( parm->val.bool_val ) );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

/* 
 * Tcl callback to allow setting of game configuration variables from Tcl.
 */
static int set_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int tmp_int;
    int num_params;
    struct param *parm;

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name> <value>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Set value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	set_param_string( parm, argv[2] ); 
	break;

    case PARAM_CHAR:
	if ( strlen( argv[2] ) > 1 ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a single character", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_char( parm, argv[2][0] );
	break;

    case PARAM_INT:
	if ( Tcl_GetInt( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be an integer", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_int( parm, tmp_int );
	break;

    case PARAM_BOOL:
	if ( Tcl_GetBoolean( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a boolean", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	check_assertion( tmp_int == 0 || tmp_int == 1, 
			 "invalid boolean value" );
	set_param_bool( parm, (bool) tmp_int );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

void register_game_config_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_get_param", get_param_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_set_param", set_param_cb,   0,0);
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         <S:update-report send-all="true" xmlns:S="svn:"><S:src-path>https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer</S:src-path><S:target-revision>65</S:target-revision><S:entry rev="63" ></S:entry></S:update-report>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     ‰S[]Â ‹@Ø	Àt‹@ÃU‹ìƒÄôSVW3Ò‰Uü‹Ø3ÀUhv¬@ dÿ0d‰ EüP‰]ôÆEøUô3É¸ğ  èhÁÿÿ‹Mü²¸È¢@ èAßÿÿè¼„ÿÿ3ÀZYYd‰h}¬@ Eüè‹‡ÿÿÃééƒÿÿëğ_^[‹å]ÃSVWU‹è¡(I ‹pN…ö|"F3Û‹Ó¡(I è.  ‹ø‹Õ‹Çè~ÿÿ„Àu(CNuá‹Õ¡,I ‹ÿQH‹Ø…Û|‹Ó¡,I ‹ÿQ‹øë3ÿ‹Ç]_^[ÃSV‹ğ‹Æè•ÿÿÿ‹Ø…Ûu‹Æèÿÿÿ‹Ã^[Ã@ U‹ìÄğşÿÿSVW3É‰Mğ‰Uü3ÒUhº­@ dÿ2d‰"èq}ÿÿ‰Eôë`‹EôèĞşÿÿ‹ğ…ötG¿K…Û|?CÇEø    ‹Eø‹|†•ğşÿÿ‹Çè>}ÿÿ•ğşÿÿEğè°‡ÿÿ‹Eğ‹Uüè¡¼ÿÿ…Àt%ÿEøKuÉ‹EôèO}ÿÿ‰Eô‹Eô=Ü¤@ u–‹EüèBÿÿÿ‹ø3ÀZYYd‰hÁ­@ EğèG†ÿÿÃé¥‚ÿÿëğ‹Ç_^[‹å]Ã‹ÀU‹ìÄôşÿÿSVW3Ò‰Uü‹Ø¾(I 3ÀUh†®@ dÿ0d‰ ëp•üşÿÿ‹ÃèŸ|ÿÿ•üşÿÿEüè‡ÿÿ‹Eüèmşÿÿ…Àt/‹Eü‰…ôşÿÿÆ…øşÿÿ…ôşÿÿPj ¹ğ  ²¸¢@ èwŞÿÿèÒ‚ÿÿ‹Ó‹èå  ûÜ¤@ t‹Ãèz|ÿÿ‹Ø‹Ó‹è¯  @t„3ÀZYYd‰h®@ Eüè{…ÿÿÃéÙÿÿëğ_^[‹å]ÃU‹ìSV‹Ê‹ˆISyù‹Ä‹ò…ö|F‹Ø‹èÿÿÿƒÃNuó‹uø‹]ü‹å]Ã                        ø®@    Ä@ ,,@ à*@ +@ L+@ 	TIntConst‹ÀU‹ì„ÒtƒÄğè~ÿÿ‰H‹M‰H‹M‰H„Òt
d    ƒÄ]Â SVW‹ù‹ò‹ØVW‹Ë²¸ø®@ è·ÿÿÿ‹Ğ¡0I èß  _^[Ã@ U‹ìQSV‹Ú‹ğj
‹ÆèÅ‡ÿÿP¡I Pèı¨ÿÿ‹Ğ…Ò•À„ÀtWRè©ÿÿVj
‹I ²¸P©@ è/!  ‰Eü3ÀUhØ¯@ dÿ0d‰ ‹‹Eüèl  ‰3ÀZYYd‰hß¯@ ‹Eüè{ÿÿÃé‡€ÿÿëğ°^[Y]Ã‹À‹dI ¡hI è@  ²¸\¤@ è,{ÿÿ£dI Ã‹ÀSV¡dI ‹XK…Û|C3ö‹Ö¡dI èª  ‹ÿRFKuë^[Ã‹À¡dI è{ÿÿ¡hI èD  £dI ¡hI ‹PJ¡hI è  Ã@ U‹ìÄüşÿÿSVW3Ò‰Uü‹ğ3ÀUhï°@ dÿ0d‰ 3ÛşØª@ tF‹E;pøt>‹EP‹Æè6zÿÿè½ÿÿÿY‹Ø•üşÿÿ‹Æèåyÿÿ•üşÿÿEüèW„ÿÿ‹Eü‹UƒÂüè‰şÿÿ
Ø3ÀZYYd‰hö°@ EüèƒÿÿÃépÿÿëğ‹Ã_^[‹å]ÃU‹ìƒÄô‰Uø‰Eüè×şÿÿ3ÀUhI±@ dÿ0d‰ U‹Eüètyÿÿè;ÿÿÿYˆE÷èÖşÿÿ3ÀZYYd‰hP±@ èğşÿÿÃéÿÿëóŠE÷‹å]ÃVW‰Æ‰×‰Ê1ÀƒâÁéó§u‰Ñó¦u@_^Ã‹ÀSVW‹ò‹Ø‹ÃèN  ‹ø‹ÆèE  ;øu‹Ãè:  ‹È‹V‹Cè±ÿÿÿ„Àu3Àë°_^[Ã@ U‹ìQS‹Úh   ‹È²¸hª@ èz   ‰Eü3ÀUh²@ dÿ0d‰ ‹Ó‹EüèÇ>  3ÀZYYd‰h
²@ ‹EüèVyÿÿÃé\~ÿÿëğ[Y]Ã‹ÀU‹ìƒÄôSVW‹ò‹ØÆEÿ ‹Ãètxÿÿ‹ø‹Æèkxÿÿ;ø…®   ‹Ãèp  ‹ø‹Æèg  ;ø…–   ²¸ü¨@ èÓxÿÿ‰Eø3ÀUhà²@ dÿ0d‰ ‹Ó‹Eøè@ÿÿÿ²¸ü¨@ è¬xÿÿ‰Eô3ÀUhÃ²@ dÿ0d‰ ‹Ö‹Eôèÿÿÿ‹Uô‹EøèÎşÿÿˆEÿ3ÀZYYd‰hÊ²@ ‹Eôè–xÿÿÃéœ}ÿÿëğ3ÀZYYd‰hç²@ ‹EøèyxÿÿÃé}ÿÿëğŠEÿ_^[‹å]Ã@ S‹Ø‹Ë²¸P£@ èGÙÿÿè~ÿÿ[Ã¸ğ  èŞÿÿÿÃSV‹Ú‹ğ‹Æè;   „Ût‹Æè<zÿÿ‹Æ^[Ã@ SVW‹ú‹Ø‹s;su‹Ã‹ÿR‹C‰<°ÿC‹Æ_^[Ã@ S‹Ø3Ò‹Ãè  3Ò‹ÃèÛ  [ÃSV‹ò‹Ø…ö|;s|‹Ã‹ÿÿK‹C;ğ}+Æ‹ÈÁá‹C°‹CD°èˆsÿÿ^[ÃèSÿÿÿÃ‹ÀS‹Ø‹C;Cu‹Ã‹ÿR‹Ã[Ã‹ÀSV‹ò‹Ø…ö|;s|‹Ã‹ÿ‹C‹°^[Ã‹À‹Hƒù~º   ëƒù~º   ëº   Ñè5  ÃS3ÉëA;H}‹X;‹uò;HuƒÉÿ‹Á[ÃSVW‹ù‹ò‹Ø…ö|;s~‹Ã‹ÿ‹C;Cu‹Ã‹ÿR‹C;ğ}+Æ‹ÈÁá‹CT°‹C°èµrÿÿ‹C‰<°ÿC_^[Ã‹PJè;ÿÿÿÃ‹ÀSVWU‹ñ‹ú‹Ø;÷t.…ö|;s|‹Ã‹ÿ‹×‹Ãèÿÿÿ‹è‹×‹Ãè«şÿÿ‹Í‹Ö‹Ãèdÿÿÿ]_^[Ã@ SVW‹ù‹ò‹Ø…ö|;s|‹Ã‹ÿ‹C‰<°_^[Ã‹ÀSV‹Ø‹Ãèÿÿÿ‹ğƒşÿt	‹Ö‹ÃèYşÿÿ‹Æ^[ÃSV‹ğ‹^Kƒû |‹Ó‹Æèšşÿÿ…Àu	‹Ó‹Æè1şÿÿKƒûÿuä^[ÃSV‹ò‹Ø;s|şÿÿÿ~‹Ã‹ÿ;st‹ÖÁâCèıpÿÿ‰s^[Ã@ SV‹ò‹Ø…ö|şÿÿÿ~‹Ã‹ÿ;s~	‹Ö‹Ãè¦ÿÿÿ‹C;ğ~‹S‚‹Î+È‹ÁÁà3É’èlsÿÿ‰s^[Ã‹ÀSV‹Ú‹ğ3Ò‹Æè=   3Ò‹Æèduÿÿ„Ût‹Æèywÿÿ‹Æ^[Ã¹¸ğ  ²¸£@ èCÖÿÿè{ÿÿÃ‹È;Ğ}‹Ê‹ÁÃSVWUQ‹ò‹Ø;s„Š   …ö}‹Ãè¿ÿÿÿF H…ÀyƒÀÁø‹øÁç‹CƒÀ H…ÀyƒÀÁø‹èÁå;ïtO3À‰$…ÿt‹ÇèÒoÿÿ‰$‹$3É‹×è¯rÿÿ…ít&ƒ<$ t‹×‹Åèxÿÿÿ‹È‹$‹Cè“pÿÿ‹Õ‹Cè±oÿÿ‹$‰C‰sZ]_^[Ã‹À;Ps‹@Ét«Ã³Ãƒú ŒÿÿÿPRQBè6ÿÿÿYZXëÚÃSVWQ‹Ø‹CƒÀ H…ÀyƒÀÁøH…À|M@‰$3ö‹C‹`·@ ;°t3‹C‹<°3À‹Ğ€úwƒâ£×r‹ÖÁâƒàĞ‹Â‹S;Â|‹Âë@< uÕFÿ$u¹‹CZ_^[Ã   ÿÿÿÿ…Òt’‹ÿÃ3Òè   Ã‹ÀU‹ìÄìşÿÿSVW3É‰Mü‹ò‹Ø3ÀUh.¸@ dÿ0d‰ …öt•üşÿÿ‹èôrÿÿ•üşÿÿEüèf}ÿÿëEüºD¸@ èË|ÿÿ‹Eü‰…ìşÿÿÆ…ğşÿÿ•üşÿÿ‹èºrÿÿ…üşÿÿ‰…ôşÿÿÆ…øşÿÿ…ìşÿÿPj¹ ğ  ²¸4g@ è©Ôÿÿèyÿÿ3ÀZYYd‰h5¸@ EüèÓ{ÿÿÃé1xÿÿëğ_^[‹å]Ãÿÿÿÿ   nil ’è*ÿÿÿÃÃ@ SV‹Ú‹ğÇF   ƒ~ t‹ÆèÎ   ‹Fèârÿÿ„Ût‹Æèëtÿÿ‹Æ^[Ã‹À‹È²‹@ÿPÃU‹ìQSVW‹ò‰Eü‹ÆºL¥@ èôrÿÿ„Àth‹Eüèp   3ÀUh¹@ dÿ0d‰ ‹Eüèn   ‹ÆèÛ   ‹ØK…Û|C3ÿ‹×‹ÆèĞ   P‹Eüè—ÿÿÿZ‹ÿQGKuä3ÀZYYd‰h"¹@ ‹Eüè”   ÃéNwÿÿëğ‹Ö‹EüèBşÿÿ_^[Y]Ãÿ@Ãƒx u3Ò‹ÿQÃ‹ÀU‹ìQS‰Eü‹Eü‹@ƒx ~M‹EüèĞÿÿÿ3ÀUh–¹@ dÿ0d‰ ë‹Ãè!ûÿÿèäqÿÿ‹Eü‹Xƒ{ è3ÀZYYd‰h¹@ ‹Eüè   ÃéÉvÿÿëğ[Y]Ã@ ÿHè€ÿÿÿÃ@ ‹@‹@Ã‹@èúÿÿÃ@ Ã@ SVW‹ú‹Ø‹Ã‹ÿR‹ğ‹Ï‹Ö‹Ã‹ÿSL‹Æ_^[Ã@ SVW‹ù‹Ø‹Ã‹ÿQ,‹ğ‹Ï‹Ö‹Ã‹ÿS ‹Æ_^[Ã@ ‹ÿQ,Ã‹ÀU‹ìƒÄôSVW3É‰Mô‰Uø‰Eü3ÀUh¿º@ dÿ0d‰ ‹Eüèş   3ÀUh¢º@ dÿ0d‰ ‹Eø‹ÿR‹ğN…ö|-F3ÛMô‹Ó‹Eø‹8ÿW‹EôP‹Ó‹Eø‹ÿQ‹È‹EüZ‹8ÿW0CNuÖ3ÀZYYd‰h©º@ ‹EüèW  Ãé½uÿÿëğ3ÀZYYd‰hÆº@ EôèByÿÿÃé uÿÿëğ_^[‹å]Ã@ U‹ìQS‹Ú‰Eü‹ÃºÀ¥@ èºpÿÿ„ÀtE‹EüèN   3ÀUh(»@ dÿ0d‰ ‹Eü‹ÿR8‹Ó‹Eü‹ÿQ43ÀZYYd‰h9»@ ‹EüèÑ   Ãé7uÿÿëğ‹Ó‹Eüè+üÿÿ[Y]Ã@ S‹Øƒ{ u	²‹Ã‹ÿQ(ÿC[ÃU‹ìSV‹E‹@ü‹p…öt,³‹E‹ÆºÀ¥@ è%pÿÿ„Àt'‹E‹Ö‹E‹@øè}   ‹Ø€óë‹E‹@ø‹ÿR…ÀŸÃ‹Ã^[]ÃU‹ìƒÄøS‰Uü‰Eø‹EøPh¤Â@ ‹EøPhÆ@ Uè‡ÿÿÿY‹Èºğ»@ ‹Eü‹ÿ[YY]Ã   ÿÿÿÿ   Strings ÿHƒx u3Ò‹ÿQ(Ã@ U‹ìƒÄìSVW3É‰Mğ‰Mì‰Uø‰Eü3ÀUh¡¼@ dÿ0d‰ ÆE÷ ‹Eü‹ÿR‹Ø‹Eø‹ÿR;Øu;‹óN…ö|0F3ÛMğ‹Ó‹Eü‹8ÿW‹EğPMì‹Ó‹Eø‹8ÿW‹UìXèzÿÿuCNuÓÆE÷3ÀZYYd‰h¨¼@ Eìº   è€wÿÿÃé¾sÿÿëëŠE÷_^[‹å]Ã‹ÀU‹ìƒÄğSVW3Û‰]ğ‰]ô‰Mü‹ò‹Ø3ÀUhU½@ dÿ0d‰ Mô‹Ö‹Ã‹8ÿW‹Ö‹Ã‹ÿQ‰EøMğ‹Uü‹Ã‹8ÿW‹Mğ‹Ö‹Ã‹8ÿW‹Uü‹Ã‹ÿQ‹È‹Ö‹Ã‹0ÿV ‹Mô‹Uü‹Ã‹0ÿV‹Mø‹Uü‹Ã‹ÿS 3ÀZYYd‰h\½@ Eğº   èÌvÿÿÃé
sÿÿëë_^[‹å]ÃU‹ìÄğÿÿPƒÄèSVW3É‰èïÿÿ‰Uø‰Eü3ÀUh²¾@ dÿ0d‰ ‹Eü‹ÿR‹ğƒşu+èïÿÿ3Ò‹Eü‹ÿSƒ½èïÿÿ u‹EøºÈ¾@ èÏvÿÿéÏ   ìïÿÿN…öŒ¬   F‰uìÇEğ    ƒ}ğ tÆ,Cèïÿÿ‹Uğ‹Eü‹0ÿV‹…èïÿÿè.yÿÿ‰Eô3ö3À‹}ôë"„ÀuŠ€ú v
€ú"t€ú,uƒÆ°€?"uFFG€? uÙó•ìïÿÿÂ   ;òs6„ÀtÆ"C‹}ôë€ú"uÆ"CŠˆCGŠ„Òuë„ÀtÆ"CÿEğÿMì…_ÿÿÿ…ìïÿÿ‹Ë+È‹Ğ‹EøèGvÿÿ3ÀZYYd‰h¹¾@ …èïÿÿèOuÿÿÃé­qÿÿëí_^[‹å]Ãÿÿÿÿ   ""  3ÀÃU‹ìj SVW‹Ø3ÀUh¿@ dÿ0d‰ Uü‹Ã‹ÿQ‹Eüè>xÿÿè©µÿÿ‹Ø3ÀZYYd‰h¿@ EüèêtÿÿÃéHqÿÿëğ‹Ã_^[Y]Ã‹ÀU‹ìƒÄèSVW3É‰Mè‰Mì‰Uø‰Eü3ÀUhÿ¿@ dÿ0d‰ ‹Eü‹ÿR‰Eğ3Û‹uğN…ö|)FÇEô    Mè‹Uô‹Eü‹8ÿW‹EèèôuÿÿƒÀØÿEôNuß‹Eø‹Ë3ÒèIuÿÿ‹]ø‹‹uğN…ö|@FÇEô    Mì‹Uô‹Eü‹8ÿW‹Eìè²uÿÿ‹ø…ÿt‹Ó‹Eì‹ÏèdgÿÿßÆCÆ
CÿEôNuÈ3ÀZYYd‰hÀ@ Eèº   è"tÿÿÃé`pÿÿëë_^[‹å]Ã@ U‹ìƒÄôSVW3É‰Mô‰Uø‰Eü3ÀUh}À@ dÿ0d‰ ‹Eü‹ÿR‹ØK…Û|#C3öMô‹Ö‹Eü‹8ÿW‹Eô‹Uøè¬ªÿÿ…ÀtFKuàƒÎÿ3ÀZYYd‰h„À@ Eôè„sÿÿÃéâoÿÿëğ‹Æ_^[‹å]Ã@ SVWU‹ê‹ø‹Ç‹ÿR‹ØK…Û|C3ö‹Ö‹Ç‹ÿQ;ètFKuïƒÎÿ‹Æ]_^[ÃU‹ìQSVW‰Mü‹Ú‹ğ‹Mü‹Ó‹Æ‹8ÿWL‹M‹Ó‹Æ‹ÿS _^[Y]Â ‹ÀU‹ìQS‹Øj ‹Ê¸\¨@ ²èÍ  ‰Eü3ÀUh<Á@ dÿ0d‰ ‹Uü‹Ã‹ÿQT3ÀZYYd‰hCÁ@ ‹EüèjÿÿÃé#oÿÿëğ[Y]ÃU‹ìƒÄøSVW3É‰Mø‹Ú‰Eü3ÀUhæÁ@ dÿ0d‰ ‹EüèÏùÿÿ3ÀUhÉÁ@ dÿ0d‰ ‹ÃèN
  ‹ğ‹Ãè)
  +ğEø‹Î3ÒèCsÿÿ‹Uø‹Î‹Ã‹ÿ‹Uø‹Eü‹ÿQ$3ÀZYYd‰hĞÁ@ ‹Eüè0úÿÿÃé–nÿÿëğ3ÀZYYd‰híÁ@ EøèrÿÿÃéynÿÿëğ_^[YY]ÃU‹ìƒÄøSVW3Û‰]ø‰Mü‹ò‹Ø3ÀUh`Â@ dÿ0d‰ ;uüt.Mø‹Ö‹Ã‹8ÿW‹Ö‹Ã‹ÿQ‹ø‹Ö‹Ã‹ÿQ<W‹Mø‹Uü‹Ãèzşÿÿ3ÀZYYd‰hgÂ@ Eøè¡qÿÿÃéÿmÿÿëğ_^[YY]Ã‹ÀSVWU‹é‹ò‹Ø‹Ö‹Ã‹ÿQ‹ø‹Ö‹Ã‹ÿQ<W‹Í‹Ö‹Ãè*şÿÿ]_^[ÃÃ@ U‹ìj SVW‹Ú‹ğ3ÀUh
Ã@ dÿ0d‰ ‹Ãè  ‹Æ‹ÿR8ëUü‹ÃèX(  ‹Uü‹Æ‹ÿQ,‹Ãè  „Àtá‹Ãèà  3ÀZYYd‰hÃ@ Eüè÷pÿÿÃéUmÿÿëğ_^[Y]ÃU‹ìQS‹Øhÿÿ  ‹Ê¸\¨@ ²è¦
  ‰Eü3ÀUhcÃ@ dÿ0d‰ ‹Uü‹Ã‹ÿQ`3ÀZYYd‰hjÃ@ ‹EüèögÿÿÃéülÿÿëğ[Y]Ã‹ÀU‹ìj SVW‹ò‹Ø3ÀUhÅÃ@ dÿ0d‰ ‹Ã‹ÿRD‹ĞEüè‘qÿÿ‹EüèÑqÿÿ‹È‹Uü‹Æè‘  3ÀZYYd‰hÌÃ@ Eüè<pÿÿÃéšlÿÿëğ_^[Y]Ã‹ÀU‹ìÄğÿÿPƒÄøSVW3É‰Mø‹Ú‰Eü3ÀUhîÄ@ dÿ0d‰ ‹Eüè<÷ÿÿ3öUhÑÄ@ dÿ6d‰&‹Eü‹ÿR8‹Ãèsÿÿ‹Ğ…øïÿÿ¹ÿ  èÙ®ÿÿøïÿÿëCŠ„Àt< võ€; tp‹Ó€;"u‹ÃCë€ù"uC€;"uŠˆ@CŠ„ÉuéëCŠ< v<,uõ‹Ã‹È+ÊEøèWpÿÿ‹Uø‹Eü‹ÿQ,ëCŠ„Àt< võŠ„Àt<,u CëCŠ„Àt–< võë3ÀZYYd‰hØÄ@ ‹Eüè(÷ÿÿÃékÿÿëğ3ÀZYYd‰hõÄ@ EøèoÿÿÃéqkÿÿëğ_^[‹å]ÃU‹ìj SVW‹ò‹Ø3ÀUh@Å@ dÿ0d‰ Eü‹Öèpÿÿ‹Uü‹Ã‹ÿQ$3ÀZYYd‰hGÅ@ EüèÁnÿÿÃékÿÿëğ_^[Y]Ã@ U‹ìƒÄøSVW3É‰Mø‹Ú‰Eü3ÀUhÆ@ dÿ0d‰ ‹EüèÇõÿÿ3ÒUhéÅ@ dÿ2d‰"‹Eü‹ÿR8…Ût@ë9‹ÃëCŠ„Òt
€ê
t€êuï‹Ë+ÈUø’è)oÿÿ‹Uø‹Eü‹ÿQ,€;uC€;
uC€; uÂ3ÀZYYd‰hğÅ@ ‹EüèöÿÿÃévjÿÿëğ3ÀZYYd‰hÆ@ EøèûmÿÿÃéYjÿÿëğ_^[YY]ÃÃ@ U‹ìƒÄôSVW3É‰Mô‰Uø‰Eü3ÀUhÆ@ dÿ0d‰ ‹EøèR/  ‹Eü‹ÿR‹ØK…Û|C3öMô‹Ö‹Eü‹8ÿW‹Uô‹EøèÜ:  FKuä‹Eøè$/  3ÀZYYd‰h•Æ@ EôèsmÿÿÃéÑiÿÿëğ_^[‹å]ÃSV‹Ú‹ğ3À‰F‰F3À‰F ‰F$‹F…Àt‹V‹È¸|¦@ ’èosÿÿ3À‰F3Ò‹Æèm  „Ût‹ÆèŠfÿÿ‹Æ^[ÃSVWQ‹ò‹Ø€{ u‹C‰$ë&‹Ì‹Ö‹Ã‹8ÿWp„ÀtŠC,rşÈtë
¸ğ  èĞëÿÿ‹Î‹$‹Ãèx  ‹$Z_^[Ãƒx ufƒx t
‹È‹Ğ‹AÿQÃƒx ufƒx" t
‹È‹Ğ‹A$ÿQ ÃS‹Øƒ{ t,‹Ã‹ÿRl‹C‹Kº|¦@ è°rÿÿ3À‰C3Ò‹Ãè®  ‹Ã‹ÿRh[ÃSV‹ò‹Ø…ö|;s|èXëÿÿ‹Ã‹ÿRl‹Cğº|¦@ èõrÿÿÿK‹C;ğ}+Æ‹ÈÁá‹Cğ‹CDğèJ_ÿÿ‹Ã‹ÿRh^[ÃSVW‹ù‹ò‹Ø…ö|;s}	…ÿ|;{|èôêÿÿ‹Ã‹ÿRl‹Ï‹Ö‹Ãè   ‹Ã‹ÿRh_^[Ã@ S‹XÓ‹@È‹
‹‰‰‹J‹X‰Z‰H[ÃSVWUƒÄô‰L$‰$‹èÆD$ 3ö‹}O;ş|47Ñë‹E‹Ø‹$èz¢ÿÿ…À}së‹ûO…ÀuÆD$€}t‹ó;ş}Ì‹D$‰0ŠD$ƒÄ]_^[Ã@ SVW‹ù‹ò‹Ø…ö|;s|è1êÿÿ‹Ç‹S‹òè¬kÿÿ_^[Ã‹@ÃSV‹ò‹Ø…ö|;s|èêÿÿ‹C‹Dğ^[Ã‹À‹Hƒù~º   ëƒù~º   ëº   Ñè  ÃSVQ‹ò‹Ø€{ u‹Ö‹ÃèÂöÿÿ‰$ë‹Ì‹Ö‹Ã‹ÿSp„ÀuÇ$ÿÿÿÿ‹$Z^[ÃSVW‹ù‹ò‹Ø€{ t
¸ğ  èkéÿÿ…ö|;s~èuéÿÿ‹Ï‹Ö‹Ãè   _^[Ã‹ÀSVW‹ù‹ò‹Ø‹Ã‹ÿRl‹C;Cu‹ÃèIÿÿÿ‹C;ğ}+Æ‹ÈÁá‹CTğ‹CğèQ]ÿÿ‹Cğ‹ğ3À‰3À‰F‹Æ‹×èOjÿÿÿC‹Ã‹ÿRh_^[ÃSVW‹ù‹ò‹Ø€{ t
¸ğ  èËèÿÿ…ö|;s|èÕèÿÿ‹Ã‹ÿRl‹Cğ‹×èjÿÿ‹Ã‹ÿRh_^[Ã‹ÀSVW‹ù‹ò‹Ø…ö|;s|èèÿÿ‹Ã‹ÿRl‹C‰|ğ‹Ã‹ÿRh_^[ÃU‹ìƒÄôSVW3Û‰]ô‰Mø‰Uü‹ø3ÀUh6Ë@ dÿ0d‰ ‹]ü‹uøEô‹UüUøÑê‹O‹ÑèÌiÿÿëC‹G‹Ø‹Uôè/ ÿÿ…À|íëN‹G‹ğ‹Uôè ÿÿ…Àí;ó|‹Î‹Ó‹Çè7ıÿÿCN;ó}Ä;uü~‹Î‹Uü‹Çèpÿÿÿ‰]ü;]ø|Œ3ÀZYYd‰h=Ë@ EôèËhÿÿÃé)eÿÿëğ_^[‹å]ÃSV‹ò‹Ø‹ÖÁâCè![ÿÿ‰s^[Ã@ SV‹Ú‹ğ:^t„Ût‹Æ‹ÿRtˆ^^[Ã„Òt‹ÿRlÃ‹ÿRhÃS‹Ø€{ u!ƒ{~‹Ã‹ÿRl‹KI3Ò‹ÃèÙşÿÿ‹Ã‹ÿRh[ÃSf¹ 3Ò‹ÿS[Ã‹ÀS3É‹ÿS[Ã‹ÀSVW‹Øf¹ 3Ò‹Ã‹0ÿV‹ğf¹ 3Ò‹Ã‹8ÿW‹ø3É‹Ö‹Ã‹ÿS‹Ç_^[ÃSVW‹Ù‹ú‹ğ…Ût$‹×‹Ë‹Æ‹0ÿ;Øt¹ğ  ²¸H¢@ èÀÿÿèãdÿÿ_^[Ã@ SVW‹Ù‹ú‹ğ…Ût%‹×‹Ë‹Æ‹0ÿV;Øt¹ğ  ²¸ˆ¢@ èß¿ÿÿèªdÿÿ_^[Ã‹ÀU‹ìƒÄìSVW‹Ù‰Uø‰Eü…Ûu3Ò‹Eøè1ÿÿÿ‹Eøè5ÿÿÿ‹Ø‰]ôû ğ  ~	ÇEğ ğ  ë‰]ğ‹EğèˆYÿÿ‰Eì3ÀUhÍ@ dÿ0d‰ …Ût-;]ğ~‹uğë‹ó‹}ì‹×‹Î‹Eøèÿÿÿ‹×‹Î‹EüèDÿÿÿ+Ş…ÛuÓ3ÀZYYd‰h"Í@ ‹Uğ‹EìèFYÿÿÃéDcÿÿëí‹Eô_^[‹å]ÃU‹ìƒÄøS‹Úh   ‹È²¸ ª@ è  ‰Eø3ÀUh|Í@ dÿ0d‰ ‹Ó‹Eøè‘  ‰Eü3ÀZYYd‰hƒÍ@ ‹Eøèİ]ÿÿÃéãbÿÿëğ‹Eü[YY]Ã„ÒtƒÄğè˜_ÿÿ‰H„Òt
d    ƒÄÃ‹À‹@èÀ ÿÿƒøÿu3ÀÃ‹@èÜ ÿÿƒøÿu3ÀÃ·É‹@èõ ÿÿÃU‹ìƒÄôSVW„ÒtƒÄğèC_ÿÿ‹ñˆUÿ‹Ø‹}·Ç=ÿÿ  u5‹ÆèH ÿÿ‹ø‰{…ÿ}[‰uôÆEøEôPj ¹ğ  ²¸„¡@ è¾ÿÿèèbÿÿë6·×‹ÆèĞŸÿÿ‰Cƒ{ }#‰uôÆEøEôPj ¹ğ  ²¸È¡@ èU¾ÿÿè°bÿÿ€}ÿ t
d    ƒÄ‹Ã_^[‹å]Â SV‹Ú‹ğ‹F…À|è> ÿÿ„Ût‹ÆèÇ^ÿÿ‹Æ^[Ã‹À‰P‰HÃSVW‹Ø‹{…ÿ|$…É| ‹s+÷…ö~;Î}‹ñ‹CÇ‹ÎèYXÿÿsë3ö‹Æ_^[Ãfƒér	tfÿÉtë‰PëPëP‰P‹@ÃS‹H…Ét‹X‡Ú‹Ãèıÿÿ[Ã‹ÀU‹ìQS‹Øhÿÿ  ‹Ê¸\¨@ ²è–şÿÿ‰Eü3ÀUhsÏ@ dÿ0d‰ ‹Uü‹Ãè³ÿÿÿ3ÀZYYd‰hzÏ@ ‹Eüèæ[ÿÿÃéì`ÿÿëğ[Y]Ã‹ÀSV‹Ú‹ğ‹Æè   3Ò‹Æè¶[ÿÿ„Ût‹ÆèË]ÿÿ‹Æ^[Ã‹ÀS‹Ø3Ò‹Ãè   3À‰C3À‰C[ÃSQ‰$‹Ø‹Ô‹Ã‹ÿQ‹Ğ‹K‹ÃèĞşÿÿ‹$‰CZ[Ã@ SV‹ò‹Ø‹Ãè³ÿÿÿ‹Ö‹ÃèÂÿÿÿ‰s^[ÃSVWƒ: ~‹
Áÿ  á àÿÿ‰
‹p‹Î‹:‹X;ûtRƒ: u‹ÆèZ’ÿÿ3ÉëB…Ûu·  I ‹è’ÿÿ‹Èë·  I ‹‹Æè’ÿÿ‹È…Éu¹ğ  ²¸@¡@ èÕ»ÿÿè `ÿÿ‹Á_^[Ã‹ÀSVWU‹ñ‹ê‹Ø‹C…À|8…ö|4‹øş…ÿ~,;{~;{~	‹×‹Ãèÿÿÿ‰{‹SS‹Å‹ÎèrVÿÿ‰{‹Æë3À]_^[ÃU‹ìSVW„ÒtƒÄğèJ\ÿÿ‹ñ‹Ú‹ø3Ò‹Çè;Zÿÿ‹EP‹Eè7fÿÿ‹È‹Ö‹ÇèŒ   „Ût
d    ƒÄ‹Ç_^[]Â U‹ìƒÄôSVW3À‰Eü3ÀUhŠÑ@ dÿ0d‰ EüP‹E‹@ü‰EôÆEøUô3É¸
ğ  èTœÿÿ‹Mü²¸£@ è-ºÿÿè¨_ÿÿ3ÀZYYd‰h‘Ñ@ EüèwbÿÿÃéÕ^ÿÿëğ_^[‹å]ÃU‹ìQSVW‰Mü‹ò‹Ø‹EP‹EüPVèÈ†ÿÿ‹ø‰{…ÿuUèaÿÿÿY‹CPVèVˆÿÿ‹ø‰{…ÿuUèGÿÿÿY‹CPVèôˆÿÿP‹CPèJˆÿÿ‹Ğ‹ÃYè°üÿÿ_^[Y]Â SV‹Ú‹ğ‹FèNÿÿ‹FPè‰†ÿÿ3Ò‹Æè(Yÿÿ„Ût‹Æè=[ÿÿ‹Æ^[Ã¹ğ  ²¸@¡@ èºÿÿèÒ^ÿÿÃU‹ìSVW„ÒtƒÄğèÒZÿÿ‹Ú‹ø‹u‰O‹ÆèÙSÿÿ‰G‰w„Ût
d    ƒÄ‹Ç_^[]Â SV‹Ú‹ğ‹F…Àt‹Vè¿Sÿÿ„Ût‹ÆèÀZÿÿ‹Æ^[Ã@     èÒ@     Ó@         ,Ó@    Ä@ ,,@ à*@ +@ L+@         |@    |@     7Ó@      	FInstance    FInstanceRoot
TPropFixup Ü¤@ Øª@ @ U‹ìSV„ÒtƒÄğèÛYÿÿ‹Ú‹ğ‰N‹E‰F‹E‰FF‹Uèİ`ÿÿF‹UèÒ`ÿÿ„Ût
d    ƒÄ‹Æ^[]Â U‹ìS‹Ø‹E‹@ü‹Óèuàÿÿ…À}‹E‹@ø‹Óèdàÿÿ…À|‹E‹@ü‹Óèoßÿÿ[]ÃU‹ìS‹Ø‹E‹@ü‹Óè=àÿÿƒøÿt‹U‹Rü’èˆßÿÿ‹E‹@ø‹Óèàÿÿ…À}‹E‹@ø‹Óè*ßÿÿ[]Ã@ U‹ìƒÄøSVWƒ=`I  „4  ¡lI ƒx %  ²¸\¤@ èëVÿÿ‰Eü3ÀUhWÕ@ dÿ0d‰ ²¸\¤@ èÎVÿÿ‰Eø3ÀUhÕ@ dÿ0d‰ 3öëq‹Ö¡lI èQßÿÿ‹Ø‹Cÿ`I ‹ø…ÿu‹S‹CèõÇÿÿ…Àt:…ÿt‹S‹Çè·1  ‹È‹S‹CèFÈÿÿU‹CèÍşÿÿY‹Ö¡lI è¤Şÿÿ‹Ãè}VÿÿëU‹CèæşÿÿYF¡lI ;p|…3ÀZYYd‰hÕ@ ‹EøèQVÿÿÃéW[ÿÿëğ‹Eü‹XK…Û|'C3ö‹Ö‹Eüè­Şÿÿ‹ø‹ÇºØª@ èkVÿÿ„Àt€gFKuÜ3ÀZYYd‰h^Õ@ ‹EüèVÿÿÃé[ÿÿëğ_^[YY]Ã@ SVWU‹ê‹ø¡lI ‹XKƒû |B‹Ó¡lI èJŞÿÿ‹ğ…ÿt;~u%…ít‹V‹Åèy”ÿÿ…Àu‹Ó¡lI èÅİÿÿ‹ÆèUÿÿKƒûÿu¾]_^[Ã@ S‹Ø‹Ë²¸H¢@ ès¶ÿÿè>[ÿÿ[Ã¸ğ  èŞÿÿÿÃ¸ğ  èÒÿÿÿÃSèbÅÿÿ‹ØƒûÿuèÖÿÿÿ‹Ã[Ã‹ÀSVW‹Ú‹ğ‹V+Vf¹ ‹F‹8ÿW3Ò‹ÆèZüÿÿ„Ût‹Æè/Wÿÿ‹Æ_^[ÃSV‹Ú‹ğ‹Æè?  :ØtÿN‹Æè5  è|ÿÿÿ^[ÃU‹ìSV‹ò‹Ø‹SP‹Æè¡“ÿÿ…Àu‹Ó‹EÿUCPèq]ÿÿ^[]Â @ U‹ìƒÄôS‹Ú‰Eü‹Eü‹PP‹Ãèk“ÿÿ…À…§   ‹EüèÏ  <
t‹EüÿH‹EüèÁ  ‹EüÆ@Lèÿÿÿ²¸ü¨@ èETÿÿ‰Eø3ÀUhN×@ dÿ0d‰ Uô¹   ‹Eüèü  ‹Uô‹EøèÑøÿÿ‹Eø‹P‹Mô‹Eüèà  ‹EüÆ@L‹Uø‹EÿU3ÀZYYd‰hU×@ ‹EøèTÿÿÃéYÿÿëğ‹EüƒÀPè \ÿÿ[‹å]Â S‹Ø‹Ãè  „À”ÀÿK[ÃSQÆ$ fƒxF tT‹Ø‹Ê‹Ğ‹CHÿSDŠ$Z[ÃSVWÄøşÿÿ‹ñ‹ú‹ØD$‹Ö¹ÿ   èMaÿÿT$‹è’Tÿÿ‰$ƒ<$ ”D$fƒ{6 tTD$P‹Î‹Ó‹C8ÿS4€|$ tèëıÿÿ‹$Ä  _^[Ã‹ÀU‹ìQSVW‰Eü‹Eü‹@,…Àtd3ÒUhuØ@ dÿ2d‰"‹pN…ö|1F3ÿ‹×‹Eü‹@,è–Ûÿÿ‹Ø‹S‹Eü‹@è.  ‹È‹S‹Cè©ÄÿÿGNuÒ3ÀZYYd‰h|Ø@ ‹Eüè<   ÃéêWÿÿëğ_^[Y]Ã‹ÀSV‹Ø‹s‹Æè&óÿÿ‹Ğ‹C+C+Ğ‹Æè%óÿÿ3À‰C3À‰C^[ÃSVW‹Ø‹C,…Àt+‹pN…ö|F3ÿ‹×‹C,èÛÿÿè‚RÿÿGNuí‹C,èvRÿÿ3À‰C,_^[ÃS‹Ø‹Ãèš  ÿK[Ãè“  èæüÿÿÃVWS‰×‰Ë‰Æë6‹N+Nw
‰ğè2   ‹N9Ùr‰ÙV)Ë‹FFN‰Æ‰ÊÁéüó¥‰Ñƒáó¤^	ÛuÆ[_^Ã@ SV‹Ø‹S‹K‹C‹0ÿ‹ğ‰s…öu¹ğ  ²¸H¢@ èÍ²ÿÿè˜Wÿÿ3À‰C^[ÃS‹Ø‹Ãèú  <	”À[Ã@ U‹ìƒÄøSVW3Ò‰Uø‹Ø3ÀUhÚ@ dÿ0d‰ ²‹Ãè}üÿÿUÿ¹   ‹Ãè2ÿÿÿŠEÿ<tÿKUø‹Ãè
  èñûÿÿUÿ¹   ‹Ãè
ÿÿÿ3ÀZYYd‰hÚ@ EøèíYÿÿÃéKVÿÿëğŠEÿ_^[YY]Ã@ U‹ìQSVW‰Uü‹Ø3ö‹EüèêŞÿÿ3ÀUhÓÚ@ dÿ0d‰ ë]‹Ãè—şÿÿş,s‹Ãèş  ‹ğë‹EüèŞÿÿ‹Eüè:ßÿÿ;ğ}ì‹Ö‹Eüè4ßÿÿ‹ø‹Ãè?  ë	‹×‹ÃèŒ  ‹ÃèÉüÿÿ„Àtì‹Ãè*  F‹Ãè¶üÿÿ„Àt˜‹Ãè  3ÀZYYd‰hÚÚ@ ‹EüèÒŞÿÿÃéŒUÿÿëğ_^[Y]ÃU‹ì‹Eö@ÿu	‹Eƒxø t3À]Ã°]Ã@ U‹ìSV‹ğ3ÛèŠ®ÿÿºXc@ èˆPÿÿ„Àt;‹EPè»ÿÿÿY„Àt‹è'Pÿÿ3À‰‹E‹@ô3Òè  èQ®ÿÿ‹P‹E‹@ô‹ÿQ‹Ø‹Ã^[]Ã@ U‹ìSVW3ÀUh°Û@ dÿ0d‰ ‹E‹Pì‹E‹@ô‹@è|Ñÿÿ‹U‹Rô‹J$²ÿP$‹U‰Bğ‹E‹@ğ€H3ÀZYYd‰ë#é»Rÿÿ‹EP‹EƒÀğè<ÿÿÿY„ÀuèjUÿÿè¹Uÿÿ_^[]ÃU‹ìSVW3ÀUh%Ü@ dÿ0d‰ ‹E‹@ô‹P(‹E‹@ğf»ûÿèüOÿÿ‹EHè‹E‹Pğ‹E‹@ô‹ÿS3ÀZYYd‰ë#éFRÿÿ‹EP‹EƒÀğèÇşÿÿY„ÀuèõTÿÿèDUÿÿ_^[]Ã@ U‹ìƒÄøSVW3ÀUh½Ü@ dÿ0d‰ ‹E‹Pè‹E‹@ô‹@èí)  ‹U‰Bğ‹Eƒxğ u)‹E‹@è‰EøÆEüEøPj ¹ğ  ²¸H¢@ è°ÿÿèiTÿÿ3ÀZYYd‰ë#é®Qÿÿ‹EP‹EƒÀğè/şÿÿY„Àuè]Tÿÿè¬Tÿÿ_^[YY]ÃU‹ìƒÄäSVW3É‰Mì‰Mè‰Uø‰Eô3ÀUhŞ@ dÿ0d‰ MäUÿ‹EôèÁ  Uì‹EôèÎ  Uè‹EôèÃ  ‹Eø‰Eğƒ}ğ uöEÿt	UèÿÿÿYëUèşÿÿYƒ}ğ „    3ÀUhÛİ@ dÿ0d‰ ‹Eğ€HöEÿuUèZşÿÿYƒ}ğ u
3ÀZYYd‰ën‹Eğ€H‹Uô‹Eğ‹ÿQ‹Eğ€`ıöEÿt‹Eô‹@(‹Mä‹Uğf»üÿè=Nÿÿ‹Eô‹@0‹UğègÕÿÿ3ÀZYYd‰ë"éPÿÿUèúüÿÿY„Àt‹EğèeMÿÿè@SÿÿèSÿÿ3ÀZYYd‰hŞ@ Eèº   è	VÿÿÃéGRÿÿëë‹Eğ_^[‹å]Ã@ U‹ìQS‹Ú‰Eü‹Eüƒx, uO²¸\¤@ èåLÿÿ‹Uü‰B,3ÀUh‡Ş@ dÿ0d‰ ‹Ó‹Eüè3   ‹Eüè“ùÿÿ3ÀZYYd‰h˜Ş@ ‹Eüè*úÿÿÃéØQÿÿëğ‹Ó‹Eüè   [Y]ÃU‹ìƒÄôSVW‹ò‰Eüë
‹Ö‹Eüèn  ‹Eüèªøÿÿ„Àtê‹Eüè
  ‹Eü‹@(‰Eø‹Eü‹@$‰Eô‹Æf»ıÿèMÿÿ‹Uü‰B(3ÀUh^ß@ dÿ0d‰ ‹Æf»şÿèúLÿÿ‹ø‹Eü‰x$…ÿu‹Eü‹@‹Uü‰B$ë
3Ò‹Eüè¾ıÿÿ‹Eüè6øÿÿ„Àtê‹Eüè–  3ÀZYYd‰heß@ ‹Eø‹Uü‰B(‹Eô‹Uü‰B$ÃéQÿÿëæ_^[‹å]ÃSƒÄğ‹Ø‹Ãè  <u‹Ô¹
   ‹Ãè}ùÿÿëÿK‹ÃèÉ   ‰D$ÛD$Û<$›Û,$ƒÄ[ÃSVQ‹ò‹Ø‹ÃèÖ  ,tşÈt=şÈtG,tQë]‹Ô¹   ‹Ãè.ùÿÿ3ÉŠ$‹Æ3ÒèøTÿÿ‹ÆèUWÿÿ‹Ğ3ÉŠ$‹Ãèùÿÿë/‹Æº<à@ è‰Tÿÿë!‹ÆºLà@ è{Tÿÿë‹Æº\à@ èmTÿÿëè²õÿÿZ^[Ã  ÿÿÿÿ   False   ÿÿÿÿ   True    ÿÿÿÿ   nil SƒÄø‹Ø‹Ãè#  ,t
şÈt şÈt6ëDT$¹   ‹Ãè}øÿÿ¾D$‰$ë/T$¹   ‹Ãècøÿÿ¿D$‰$ë‹Ô¹   ‹ÃèKøÿÿëèõÿÿ‹$YZ[Ã²èqõÿÿÃ3ÒèiõÿÿÃSVW‹ù‹ò‹Ø á@ ˆ‹Ãèù÷ÿÿ$ğ<ğu‹Ãè
  $ˆöt	‹ÃèPÿÿÿ‰_^[Ã      U‹ìÄäşÿÿSVW3Ò‰Uü‹Ø3ÀUhùá@ dÿ0d‰ Eüè»Rÿÿ‹E‹@üºØª@ èKJÿÿ„ÀtEü‹U‹Rü‹Rè*Sÿÿƒ}ü u!•üşÿÿ‹E‹@ü‹ èIÿÿ•üşÿÿEüèSÿÿ‹Eü‰…äşÿÿÆ…èşÿÿ‹E‹@ø‰…ìşÿÿÆ…ğşÿÿ‹C‰…ôşÿÿÆ…øşÿÿ…äşÿÿPj¹ğ  ²¸H¢@ èŞªÿÿè9Oÿÿ3ÀZYYd‰h â@ EüèRÿÿÃéfNÿÿëğ_^[‹å]ÃU‹ì‹E‹@ôè~
  ¸ğ  è¨óÿÿ]Ã‹ÀU‹ìƒÄìSVW3É‰Mø‰Uü‰Eô3ÀUhåã@ dÿ0d‰ 3ÒUhšã@ dÿ2d‰"Uø‹Eôè•  3ÒUhsã@ dÿ2d‰"»   ‹EøèúRÿÿ‹ğ‹Eü‰Eğ‹EôÆ@L‹ûëC;ó|
‹Eø€|ÿ.uñ‹EôƒÀPP‹Ë+Ï‹×‹EøèÇTÿÿ;ó|`‹Eğ‹ èiIÿÿ‹Uô‹RPèŞ¸ÿÿ‹ø…ÿu‹Eôè$öÿÿ3À‰Eì‹€8u‹×‹Eğè¦¹ÿÿ‰Eì‹EìºÜ¤@ è¢Hÿÿ„ÀuUè ÿÿÿY‹Eì‰EğCétÿÿÿ‹Eğ‹ è	Iÿÿ‹Uô‹RPè~¸ÿÿ‹ø…ÿt‹Ï‹Uğ‹Eôè  ë*‹EôÆ@L ‹Uô‹Eğ‹ÿQ‹EôÆ@L‹EôƒxP t‹Eôè“õÿÿ3ÀZYYd‰ëé¼Kÿÿ   Xc@ „ã@ Uè’ıÿÿYèüMÿÿ3ÀZYYd‰ë5é•Kÿÿ   Xc@ «ã@ ‰Ã‹Eô€xL t‹S‹Eô‹ÿQ„ÀuènMÿÿè½Mÿÿ3ÀZYYd‰hìã@ EøèPÿÿÃézLÿÿëğ_^[‹å]ÃU‹ìƒÄğSVW‰Mô‰Uø‰Eü¡0I ‹pN…ö|<F3ÿ‹×¡0I è²Ïÿÿ‹Ø‹Eø‹ ;CuUğ‹EôÿS„Àt‹Mğ‹Uø‹Eüè»¸ÿÿë	GNuÇèŒñÿÿ_^[‹å]ÃU‹ìj j j SVW‹Ù‰Uü‹ø3ÀUh,å@ dÿ0d‰ Eøè{OÿÿEô‹ÓèPÿÿ‹Ó¸Då@ èÁSÿÿ‹ğ…öt&EøP‹ÎIº   ‹ÃèÄRÿÿEôPV¹ÿÿÿ‹Ãè±Rÿÿ‹E‹@ü‹@P‹EüP‹EøP‹EôP‹Ï²¸èÒ@ èYîÿÿ‹Øƒ}ø u‹E‹@ü‹@,‹Óè5Îÿÿë‹Ó¡lI è'Îÿÿ3ÀZYYd‰h3å@ Eôº   èõNÿÿÃé3Kÿÿëë_^[‹å]Ã  ÿÿÿÿ   .   U‹ìƒÄìSVW3Û‰]ì‹Ù‰Uø‰Eü3ÀUhtç@ dÿ0d‰ ‹óƒ~ u
¸ğ  èJğÿÿ‹63ÀŠƒø
‡Ñ  ÿ$…”å@ ^ç@ Àå@ æ@ æ@ Cæ@ aæ@ ~æ@ ™æ@ şæ@ ^ç@ aæ@ ‹Eüè$óÿÿ<uUì‹EüèÕùÿÿ‹Mì‹Ó‹Eøèşÿÿéu  ‹Eüèoúÿÿ‹È‹Ó‹Eøè·ÿÿé\  ‹Eüè–óÿÿ3ÉŠÈ‹Ó‹Eøèì¶ÿÿéA  Uì‹Eüè„ùÿÿ‹Uì‹ÆèÆïÿÿ‹È‹Ó‹EøèÆ¶ÿÿé  ‹Eüè!ùÿÿƒÄôÛ<$›‹Ó‹Eøè¹ÿÿéı   Uì‹EüèÄ  ‹Mì‹Ó‹EøèG¸ÿÿéà   ‹Ö‹Eüè  ‹È‹Ó‹Eøèp¶ÿÿéÅ   ‹EüèKòÿÿ,tşÈtë7‹Eüèİ  3É‹Ó‹EøèE¶ÿÿéš   ‹EüèÄ  ‹Ó‹Eøè¾µÿÿ‹Ğ‹EüèHóÿÿë|UUì‹Eüè¾øÿÿ‹Mì‹Ó‹EøèaıÿÿYë`‹Eüèæñÿÿ<u‹Eüè~  ¹4I ‹Ó‹Eøè¹ÿÿë;Uì‹Eüè~øÿÿ‹Mì‹Eü‹P‹Eü‹0ÿV‰Eğ‹Eü‹@‰Eôƒ}ğ tMğ‹Ó‹Eøèâ¸ÿÿ3ÀZYYd‰h{ç@ EìèLÿÿÃéëHÿÿëğ_^[‹å]Ã‹ÀU‹ìƒÄğSVW‹Ú‹ø3ö‹ÃèfLÿÿƒ=`I  t9‹Ã‹×èèLÿÿë"FS‰}ğÆEô‰uøÆEü Uğ¹   ¸ìç@ è´ÿÿ‹ÿ`I …ÀuÒ_^[‹å]Ã ÿÿÿÿ   %s_%d   U‹ìƒÄäSVW3É‰Mè‰Mä‰Uø‰Eü3ÀUhê@ dÿ0d‰ ‹Eüè¯  3À‰Eô3ÀUhÅé@ dÿ0d‰ MğUï‹Eüèšøÿÿƒ}ø u5Uè‹Eüè¡  ‹Eèè‰Äÿÿ3É²ÿP$‰EôUè‹Eüè„  ‹Uè‹Eô‹ÿQëH‹Eø‰EôUè‹Eüèf  ‹Eôö@tUè‹EüèR  ë!Uä‹EüèE  ‹EäUèèÊşÿÿ‹Uè‹Eô‹ÿQ‹Eô‹Uü‰Bƒ=dI  t¡dI ‹Uü‰B0ë²¸\¤@ è@Bÿÿ‹Uü‰B03ÀUh¯é@ dÿ0d‰ ‹Eü‹P‹Eü‹@0è#Êÿÿ‹Eü‹@‹Uü‰B$‹Eü‹@€H‹Eü‹@€H‹Uü‹Eü‹@‹ÿQ‹Eü‹@€`ıƒ=dI  u/‹Eü‹@0‹XK…Û|!CÇEğ    ‹Uğ‹Eü‹@0èZÊÿÿ‹ÿRÿEğKuç3ÀZYYd‰h¶é@ ƒ=dI  u‹Eü‹@0è²Aÿÿ‹Eü3Ò‰P0Ãé°FÿÿëÜèYêÿÿ3ÀZYYd‰ë'é¦Dÿÿ3Ò‹Eøè”ëÿÿƒ}ø u‹EôèvAÿÿèQGÿÿè Gÿÿ3ÀZYYd‰hê@ Eäº   èJÿÿÃéXFÿÿëë‹Eô_^[‹å]ÃU‹ìƒÄøSVW3É‰Mø‹Ú‰Eü3ÀUhÀê@ dÿ0d‰ 3ÀUh“ê@ dÿ0d‰ ‹EüèA  <tèˆëÿÿ‹Ãè¹°ÿÿ‹p3ÛUø‹Eüè…   ƒ}ø t‹Uø‹Æèyëÿÿƒøwà«ÃëÛ3ÀZYYd‰ëéØCÿÿ‹Eüè  è“FÿÿèâFÿÿ3ÀZYYd‰hÇê@ EøèAIÿÿÃéŸEÿÿëğ‹Ã_^[YY]ÃQ‹Ô¹   è+îÿÿ‹$;$I t
¸	ğ  èÖêÿÿZÃSVQ‹ò‹Ø‹Ô¹   ‹Ãèÿíÿÿ3ÉŠ$‹Æ3ÒèÉIÿÿ‹Æè&Lÿÿ‹Ğ3ÉŠ$‹ÃèÜíÿÿZ^[ÃSVQ‹ò‹Ø3À‰$‹ÃèM   ,t,të ‹Ô¹   ‹Ãè­íÿÿë‹Ô¹   ‹Ãèíÿÿëènêÿÿ‹Æ‹$3ÒèbIÿÿ‹‹$‹Ãè~íÿÿZ^[Ã‹ÀQ‹Ô¹   èkíÿÿŠ$ZÃ‹ÀU‹ìj SVW‹Ø3ÀUhâë@ dÿ0d‰ Uü‹Ãè.ÿÿÿƒ}ü uğ3ÀZYYd‰héë@ EüèHÿÿÃé}Dÿÿëğ_^[Y]ÃU‹ìS‹]ƒÃüë‹è‘   ‹è^ëÿÿ„Àtî‹è¿ôÿÿ[]ÃU‹ìÄ ÿÿÿS‹Ø…Û~?û   ~• ÿÿÿ‹E‹@ü¹   èÂìÿÿë   ë• ÿÿÿ‹E‹@ü‹Ëè§ìÿÿ3Û…ÛÁ[‹å]ÃU‹ìQUü‹E‹@ü¹   è…ìÿÿ‹EP‹Eüè‰ÿÿÿYY]ÃU‹ìƒÄøSVW3Ò‰Uø‰Eü3ÀUhuí@ dÿ0d‰ ‹EüèÕşÿÿƒàƒø‡˜   ÿ$…Îì@ _í@  í@ 	í@ í@ %í@ 3í@ Aí@ Aí@ _í@ _í@ Ní@ Wí@ ë_UèêşÿÿYëVU¸   èÿÿÿYëHU¸   èöşÿÿYë:U¸   èèşÿÿYë,U¸
   èÚşÿÿYëUø‹Eüè¨ıÿÿëUèÿÿÿYë‹EüèEşÿÿ3ÀZYYd‰h|í@ EøèŒFÿÿÃéêBÿÿëğ_^[YY]ÃU‹ìj SVW‹Ø3ÀUhÃí@ dÿ0d‰ Uü‹ÃèNıÿÿ‹Ãèçşÿÿ3ÀZYYd‰hÊí@ Eüè>FÿÿÃéœBÿÿëğ_^[Y]ÃU‹ìƒÄôSVW3É‰Mô‹Ø3ÀUhaî@ dÿ0d‰ „Òt*MøUÿ‹ÃèİòÿÿUô‹ÃèëüÿÿUô‹Ãèáüÿÿë‹Ãèhÿÿÿ‹ÃèEéÿÿ„Àtî‹Ãè¦òÿÿë	²‹Ãè—ÿÿÿ‹Ãè(éÿÿ„Àtì‹Ãè‰òÿÿ3ÀZYYd‰hhî@ Eôè EÿÿÃéşAÿÿëğ_^[‹å]ÃSVW‹ù‹òfƒx> tW‹Ø‹Î‹Ğ‹C@ÿS<‹‹Æ‹ÿQ_^[ÃSV‹Ú‹ğ‹Æè  3Ò‹ÆèŞãÿÿ„Ût‹Æè³>ÿÿ‹Æ^[Ã‹À‹@,èpÄÿÿÃ@ U‹ìS‹Ø„Ét‹ÃèÇ  ‹Ó‹EÿU[]Â ‹ÀU‹ìS‹Ø„Ét‹Ãè§  ÿuÿu‹Ãè¢   []Â S‹Ø‹CèÜÿÿC[Ãè  Ã‹ÀSV‹ò‹Ø‹Cè‚Üÿÿ;Æ	‹SĞ;ò~‹Ãèê   ‹Ö‹CètÜÿÿ^[Ã+ğ‰s^[ÃVWS‰Ö‰Ë‰Çë6‹O+Ow
‰øèº   ‹O9Ùr‰Ù)ËW‹GGO‰Ç‰ÊÁéüó¥‰Ñƒáó¤_	ÛuÆ[_^Ã@ U‹ìƒÄøS‹Ø²¸ü¨@ èo;ÿÿ‰Eü3ÀUh$ğ@ dÿ0d‰ ‹Uü‹EÿU²
‹ÃèÀ  ‹EüèèÛÿÿ‰EøUø¹   ‹Ãèbÿÿÿ‹Eü‹P‹Mø‹ÃèRÿÿÿ3ÀZYYd‰h+ğ@ ‹Eüè5;ÿÿÃé;@ÿÿëğ[YY]Â ‹ÀS‹Ø‹S‹K‹CèûÛÿÿ3À‰C[Ã„Òt²	èM  Ã²èE  ÃU‹ìj SVW‹Ú‹ğ3ÀUh¤ğ@ dÿ0d‰ Eü‹ÓèŒDÿÿ‹Uü‹Æèº  3ÀZYYd‰h«ğ@ Eüè]CÿÿÃé»?ÿÿëğ_^[Y]Ã@ SVWU‹ê‹Ø²‹Ãèß  ‹ÅèäÈÿÿ‹ğN…ö|'F3ÿ‹Ãè·  ‹×‹ÅèÒÈÿÿ‹Ğ‹Ãèù  ‹Ãè¦  GNuÜ‹Ãè›  ]_^[Ã‹ÀU‹ìQSVW‰Eü‹E‹@ü‹@,‹XK…Û|(C3ÿ‹E‹@ü‹@,‹×èÂÿÿ‹ğ‹Uü‹FèÙxÿÿ…ÀtGKuÛ3ö‹Æ_^[Y]Ã@ U‹ìQS‹Ú‰Eü€K‹Eüƒx, tU‹CèÿÿÿY‹Uü‰B‹Uü‹Ã‹ÿQ €cû[Y]Ã@ U‹ìÄÜşÿÿSVW3É‰Üşÿÿ‰Uø‰Eü3ÀUhIô@ dÿ0d‰ •àşÿÿ‹Eø‹ èÔ8ÿÿ3ÛŠàşÿÿ‹Eø‹@è•CÿÿØCƒÃƒÃ‹Eü‹@‹Uü+B;Ø~‹Eüè4şÿÿ‹Eüèıÿÿ‰Eô Xô@ ˆEã‹Eüƒx t€Mã‹Eü‹X,…Ût1‹C‹Uü;B0~&‹Eüƒx t‹Eü‹P0‹Eü‹@,è…Áÿÿ‹Uü;Bt€Mã‹Eü‹H4ŠUã‹Eüè8  •àşÿÿ‹Eø‹ è$8ÿÿ•àşÿÿ…Üşÿÿè“Bÿÿ‹•Üşÿÿ‹Eüèe  ‹Eø‹P‹EüèW  ‹Eüè_üÿÿ‰Eğ‹Eü‹@,…Àt ‹@‹Uü;B0~‹Eüƒx t‹Eüÿ@0‹Eüÿ@4‹Uø‹Eüèÿ  ‹Eüè«  ‹Eü‹@,‰Eì‹Eü‹@0‰Eè‹Eü‹@4‰Eä3ÀUhïó@ dÿ0d‰ ‹Eü3Ò‰P,‹Eü3Ò‰P0‹Eü3Ò‰P4‹Eü€x  …‡   3ÀUh¿ó@ dÿ0d‰ ‹Eü‹X…Ût:‹ÃºØª@ è=8ÿÿ„Àt*²¸\¤@ è½7ÿÿ‹Uü‰B,‹EüPhÀî@ ‹Eü‹@f»ÿÿèo8ÿÿ‹EüPhXñ@ ‹Eøf»ÿÿèZ8ÿÿ3ÀZYYd‰hÆó@ ‹Eü‹@,èš7ÿÿÃé <ÿÿëí3ÀZYYd‰höó@ ‹Eì‹Uü‰B,‹Eè‹Uü‰B0‹Eä‹Uü‰B4Ãép<ÿÿëİ‹Eüè  ‹Eø‹Uü;Bt' \ô@ :Eãu‹Eüèõúÿÿ‹UğƒÂ;Âu‹Uô‹Eüèøúÿÿ3ÀZYYd‰hPô@ …Üşÿÿè¸?ÿÿÃé<ÿÿëí_^[‹å]Ã        U‹ìS‹Ø²‹Ãè5  U¹
   ‹Ãèâúÿÿ[]Â SV‹ò‹Øºüô@ ‹Æè†uÿÿ…Àu²‹Ãè  ^[Ãºõ@ ‹Æèjuÿÿ…Àu²	‹Ãèå  ë-ºõ@ ‹ÆèOuÿÿ…Àu²‹ÃèÊ  ë²‹Ãè¿  ‹Ö‹Ãè  ^[Ã   ÿÿÿÿ   False   ÿÿÿÿ   True    ÿÿÿÿ   nil SQ‰$‹Øƒ<$€| ƒ<$²‹Ãèh  ‹Ô¹   ‹ÃèúÿÿZ[Ã<$ €ÿÿ|"<$ÿ  ²‹Ãè<  ‹Ô¹   ‹Ãèêùÿÿë²‹Ãè#  ‹Ô¹   ‹ÃèÑùÿÿZ[Ã‹À²è	  Ã3Òè  ÃSVQ‹ñˆ$‹Ø äõ@ :$t(Š$ğˆD$T$¹   ‹Ãèùÿÿö$t	‹Ö‹Ãè@ÿÿÿZ^[Ã    U‹ìƒÄğSVW‰Uø‰Eü‹Eø‹ è'6ÿÿè¥ÿÿ¿@‰Eôƒ}ô    ‹EôÁàè&0ÿÿ‰Eğ3ÀUh‘ö@ dÿ0d‰ ‹Eø‹ èë5ÿÿ‹UğèÏ¥ÿÿ‹uôN…ö|(F3ÿ‹Eğ‹¸‹Ó‹Eøè¦ÿÿ„Àt‹Ë‹Uø‹Eüè§	  GNuÛ3ÀZYYd‰h˜ö@ ‹UôÁâ‹EğèĞ/ÿÿÃéÎ9ÿÿëê‹Uü‹Eø‹ÿQ_^[‹å]Ã‹ÀU‹ìS‹E‹@üƒx t0‹E‹@øèÕ3ÿÿ‹Ø‹E‹@ü‹@èÅ3ÿÿ;Øt‹E‹@ø‹U‹Rü;Bt3À[]Ã°[]Ã‹ÀU‹ìj SVW3ÀUhC÷@ dÿ0d‰ Eü‹U‹RôƒÂè>ÿÿ‹Uü‹E‹@üèw	  3ÀZYYd‰hJ÷@ Eüè¾<ÿÿÃé9ÿÿëğ_^[Y]ÃU‹ìj SVW‹ğ3ÀUhÔ÷@ dÿ0d‰ ‹E‹@ğè¥£ÿÿ‹x‹E‹@ü²è!
  3Û‹Ãƒøw£ÆsMü‹Ó‹Çè‡£ÿÿ‹Uü‹E‹@üèU	  Cƒû uÔ‹E‹@ü3ÒèB	  3ÀZYYd‰hÛ÷@ Eüè-<ÿÿÃé‹8ÿÿëğ_^[Y]Ã@ U‹ìƒÄôSVW3É‰Mô‰Uø‰Eü3ÀUhqø@ dÿ0d‰ ¡0I ‹pN…ö|:F3ÿ‹×¡0I è²»ÿÿ‹Ø‹Eü;CuUô‹EøÿS„Àt‹E‹@ü‹Uôè=üÿÿëGNuÉ‹E‹@ü‹UøèÅüÿÿ3ÀZYYd‰hxø@ Eôè;ÿÿÃéî7ÿÿëğ_^[‹å]ÃU‹ìj SVW‹Ø3ÀUhù@ dÿ0d‰ ‹EPèWşÿÿYEü‹U‹Rü‹R(èá;ÿÿ3ÀUhûø@ dÿ0d‰ ‹E‹@üƒÀ(è1;ÿÿ‹E‹@ü‹ÓèØ÷ÿÿ3ÀZYYd‰hù@ ‹E‹@üƒÀ(‹UüèV;ÿÿÃéd7ÿÿëç3ÀZYYd‰hù@ Eüèé:ÿÿÃéG7ÿÿëğ_^[Y]Ã@ U‹ì‹E‹@PèuıÿÿY„Àt%‹E‹@‹Pô‹E‹@‹@ü‹@è>£ÿÿ‹U;Bü”À]Ã‹E‹@ü‹U‹R‹Rô;B”À]ÃU‹ìƒÄøSVW3À‰Eø3ÀUhBú@ dÿ0d‰ ‹E‹Pô‹E‹@øèï¢ÿÿ‰EüUèzÿÿÿY„Àuy‹EPè<ıÿÿY‹E‹@ğŠ şÈtşÈt!şÈt<,t)ëU‹EP‹E‹@ô‹ ‹UüèùıÿÿYë>‹Uü‹E‹@üèdöÿÿë.‹EP‹EüèFıÿÿYëMø‹E‹@ğ‹Uüè¡ÿÿ‹Uø‹E‹@üèXúÿÿ3ÀZYYd‰hIú@ Eøè¿9ÿÿÃé6ÿÿëğ_^[YY]ÃU‹ì‹E‹@PèMüÿÿY„Àt*‹E‹@‹Pô‹E‹@‹@ü‹@èŠ¤ÿÿ‹EÛhöŞÙßà”À]Ã‹EÛhöØ¤ú@ ßà”À]Ã      U‹ìƒÄô‹E‹Pô‹E‹@øèI¤ÿÿÛ}ö›Uè‡ÿÿÿY„Àu ‹EPè!üÿÿYf‹EşPÿuúÿuö‹E‹@üèrùÿÿ‹å]Ã‹ÀU‹ìj SVW3ÀUhkû@ dÿ0d‰ ‹E‹@Pè–ûÿÿY„Àt0Mü‹E‹@‹Pô‹E‹@‹@ü‹@èP£ÿÿ‹Uü‹E‹@üè>;ÿÿ”Ãë
‹Eƒxü ”Ã3ÀZYYd‰hrû@ Eüè–8ÿÿÃéô4ÿÿëğ‹Ã_^[Y]Ã‹ÀU‹ìj SVW3ÀUhßû@ dÿ0d‰ Mü‹E‹Pô‹E‹@øèâ¢ÿÿUèHÿÿÿY„Àu‹EPè>ûÿÿY‹E‹@ü‹Uüè  3ÀZYYd‰hæû@ Eüè"8ÿÿÃé€4ÿÿëğ_^[Y]ÃU‹ìS3Û‹E‹@Pè®úÿÿY„Àtl‹E‹@‹Pô‹E‹@‹@ü‹@èw ÿÿ‹Ø…ÛtL‹C‹U‹R‹Rü;B$u;‹Eƒxü t2‹E‹@ü‹@‹U‹R‹Rü;Bu‹E‹@ü‹P‹Cè·mÿÿ…Àu‹E‹Xü‹E;Xü”À[]ÃU‹ìSVW‹ú‹Ø‹E‹@‹@ü‹@‹s;Æu‹Ç‹Sèñ7ÿÿë$…ötÿvhØü@ ÿs‹Çº   èt9ÿÿë‹Çè77ÿÿ_^[]Ã  ÿÿÿÿ   .   U‹ì3ÉQQQQSVW3ÀUh¿ş@ dÿ0d‰ ‹E‹Pô‹E‹@øèŸÿÿ‰Eüƒ}ü u'UèÖşÿÿY„Àu‹EPèÔùÿÿY‹E‹@ü²èr  ém  ‹EüºÜ¤@ è\.ÿÿ„À„X  ‹EüºØª@ èG.ÿÿ„ÀtNUè‰şÿÿY„À…8  UUğ‹EüèÿÿÿY‹UğEôè7ÿÿƒ}ô „  ‹EPèaùÿÿY‹E‹@ü‹UôèŞöÿÿéù   ‹EüºL¥@ èè-ÿÿ„ÀtF‹EPèçøÿÿY„Àt&‹E‹Pô‹E‹@ü‹@è¶ÿÿ‹Ğ‹Eüè(´ÿÿ„À…´   ‹EP‹Eüè„úÿÿYé¢   ‹E‹@ü‹XEø‹U‹Rü‹R(èx6ÿÿ‹E‹@üÿp(Eğ‹U‹RôƒÂèê6ÿÿÿuğhØş@ ‹E‹@üƒÀ(º   èã7ÿÿ‹EPèRøÿÿY„Àt‹E‹Pô‹E‹@ü‹@è!ÿÿ‹U‹Rü‰B‹E‹@ü‹Uüè^÷ÿÿ‹E‹@ü‰X‹E‹@üƒÀ(‹Uøè¬5ÿÿ3ÀZYYd‰hÆş@ Eğº   èb5ÿÿÃé 1ÿÿëë_^[‹å]Ã   ÿÿÿÿ   .   U‹ìÄøşÿÿSV3Û‹E‹@Pè·÷ÿÿY„Àt Mø‹E‹@‹Pô‹E‹@‹@ü‹@èé ÿÿ‹]ø‹E;Xøt5‹E‹pø…öt'øşÿÿ‹E‹Ö‹E‹@‹@ü‹@‹ èZ-ÿÿ€½øşÿÿ t3Àë°^[‹å]ÃU‹ìÄôşÿÿSVW3À‰Eô3ÀUh A dÿ0d‰ Mø‹E‹Pô‹E‹@øèm ÿÿUèCÿÿÿY„ÀuT‹EPèQ÷ÿÿY‹]ø…Ûu‹E‹@ü²èè  ë4ôşÿÿ‹E‹@ü‹@‹ ‹ÓèÎ,ÿÿ•ôşÿÿEôè<5ÿÿ‹Uô‹E‹@üè’ôÿÿ3ÀZYYd‰h A Eôèù3ÿÿÃéW0ÿÿëğ_^[‹å]Ã‹ÀU‹ìƒÄğ‰Mô‰Uø‰Eü‹Eôƒx tn‹ ‰Eğ‹Eğ¶ ƒø
w^ÿ$…G A  A s A s A s A | A … A s A  A — A  A … A UèÿøÿÿYë"Uè&úÿÿYëUèñúÿÿYëUèHüÿÿYëUèÃşÿÿY‹å]Ã‹ÀU‹ìj SVW‹ò‹Ø3ÀUhó A dÿ0d‰ ‹S(EüèË3ÿÿEü‹Öè©4ÿÿ‹Uü‹Ãè#   3ÀZYYd‰hú A Eüè3ÿÿÃél/ÿÿëğ_^[Y]ÃSVQ‹ò‹Ø‹Æèf4ÿÿ‰$<$ÿ   ~Ç$ÿ   ‹Ô¹   ‹Ãè1îÿÿ‹Æè6ÿÿ‹Ğ‹$‹ÃèîÿÿZ^[Ã‹ÀSVQ‹ò‹Ø‹Æè4ÿÿ‰$<$ÿ   ²‹Ãè9   ‹Ô¹   ‹Ãèçíÿÿë²‹Ãè    ‹Ô¹   ‹ÃèÎíÿÿ‹Ö‹$‹ÃèÂíÿÿZ^[Ã‹ÀQˆ$‹Ô¹   è¬íÿÿZÃ‹ÀSVW„ÒtƒÄğèi+ÿÿ‹ñ‹Ú‹ø øA ˆG…öt	‹×‹Æèn  „Ût
d    ƒÄ‹Ç_^[Ã     SVWUQˆ$‹Ø‹C…Àt/‹pN…ö|F3ÿ‹×‹Cè²±ÿÿ±‹Ó‹(ÿUGNué‹Cè!)ÿÿ3À‰C‹ÃèÁ  ‹Ãè  ‹C…Àt‹Óè@  €<$ t‹Ãè+ÿÿ‹ÃZ]_^[Ã@ SV‹ò‹Ø‹C…Àt;Ft6ƒ{ u²¸\¤@ èœ(ÿÿ‰C‹Ö‹Cè{±ÿÿ…À}‹Ö‹Cè‰°ÿÿ‹Ó‹Æè¸ÿÿÿ^[ÃSV‹ò‹Ø‹Æè—İÿÿf‰C^[ÃSV‹ò‹Ø‹Æèƒİÿÿf‰C^[Ã·@’è2òÿÿÃ·@’è&òÿÿÃSV‹ò‹Øƒ{ u²¸\¤@ è(ÿÿ‰C‹Ö‹Cè°ÿÿ‰^^[ÃS‹Ø3À‰B‹CèÌ±ÿÿ‹Cƒx u
è(ÿÿ3À‰C[Ã@ SVW‹Ú‹ğ‹CP3É‹Ó‹Æ‹8ÿW‹Ó‹Æèÿÿÿ²‹ÃèÀ  öFt	²‹Ãèq  3É‹Ó‹Æ‹ÿS_^[Ã‹ÀSVW‹Ú‹ğ±‹Ó‹Æ‹8ÿW3Ò‹Ãè…  ‹Ó‹Æèlÿÿÿ3Ò‹Ãè3  j ‹K‹Ó‹Æ‹ÿS_^[ÃSV‹Øëè­°ÿÿ‹ğ‹Ö‹Ãè:ÿÿÿ²‹Æ‹ÿQü‹C…Àuà^[Ã@ SVW‹ØöCu)€K‹C…Àt‹pN…ö|F3ÿ‹×‹Cè¥¯ÿÿèĞÿÿÿGNuí_^[ÃSVWUƒÄøˆL$‰$‹Ø‹C…Àt"€|$u‹$è¢°ÿÿ‹Cƒx u
èè&ÿÿ3À‰C‹C…Àt%‹pN…ö|F3ÿ‹×‹CèC¯ÿÿŠL$‹$‹(ÿUGNuæYZ]_^[ÃU‹ì‹Eƒxü t‹E‹@øf‹@‹U‹Rüf;B•À]Ã‹E‹@øfƒx •À]Ã@ U‹ì‹Eƒxü t‹E‹@øf‹@‹U‹Rüf;B•À]Ã‹E‹@øfƒx •À]Ã@ U‹ìƒÄøSV‹Ú‰Eø‹C‰Eü‹EøPh¼A ‹EøPhäA UèeÿÿÿY‹ÈºˆA ‹Ã‹0ÿ‹EøPhĞA ‹EøPhğA UèwÿÿÿY‹Èº˜A ‹Ã‹ÿ^[YY]Ã   ÿÿÿÿ   Left    ÿÿÿÿ   Top 3ÀÃU‹ì]Â 3ÀÃÃ@ Ã@ 3ÀÃÃ@ S‹Ø€K@[Ã@ S‹Ø€c¿[Ã@ S‹Ø€cş[Ã@ ’èFØÿÿÃ’‹ÿQÃU‹ìƒÄôSVW‰Mü‹ú‹Ø‹u…ÿt>‹Ö‹Eüèdÿÿ…Àt0‹Ö‹ÃèJ   …Àt#‰uôÆEøEôPj ¹ğ  ²¸¤@ è|†ÿÿè×*ÿÿöCtƒ{ tV‹Mü‹×‹C‹ÿS_^[‹å]Â SVWUQ‰$‹øƒ<$ t3ƒ t-‹G‹XK…Û|"C3í‹Õ‹Gè>­ÿÿ‹ğ‹$‹Fèycÿÿ…ÀtEKuá3ö‹ÆZ]_^[Ã@ SVWƒÄø‹ò‹Ø‹C‹Öè¸/ÿÿtl…öt,‹Æè»eÿÿ„Àu!‰4$ÆD$Tj ¹ğ  ²¸¤@ èÃ…ÿÿè*ÿÿ‹C…ÀtV‹K‹Ó‹8ÿWëV‹K3Ò‹Ã‹8ÿW3Ò‹Ãè  ‹Ö‹Ãè   ²‹Ãèş   YZ_^[ÃSV‹ò‹ØC‹Öè -ÿÿ^[Ã‹P…Òtƒz t
‹R’è²¬ÿÿÃƒÈÿÃSV‹ò‹Øƒ{ u
¸ğ  èn«ÿÿ‹Ö‹CèD¬ÿÿ^[Ã‹P…Òt‹BÃ3ÀÃ‹ÀSVW‹ò‹Ø‹C…Àt?‹@‹Óè`¬ÿÿ…À|1‹S‹z‹W…ö}3ö;Ö‹òN;Æt‹×’è–«ÿÿ‹C‹@‹Ë‹ÖèK¬ÿÿ_^[Ã@ SVWU‹Ú‹ø„Ût€Oë€gï‹Çèÿÿÿ‹èM…í|E3ö‹Ö‹ÇèHÿÿÿ‹ÓèÉÿÿÿFMuì]_^[ÃSVWÄ ÿÿÿ‹Ú‹ø‹w…öt(‹Ä‹W¹ÿ   è­0ÿÿ‹Ô‹Æè|$ÿÿ…Àt„Ût‰8ë3Ò‰Ä   _^[Ã‹ÀSVW¿0I ‹‹XK…Û|C3ö‹Ö‹è6«ÿÿèµ"ÿÿFKuî‹èª"ÿÿ_^[Ã‹À¡(I èš"ÿÿ¡,I è"ÿÿè³ÿÿÿ3Ò3Àè’Ìÿÿ¡lI èx"ÿÿ¡hI èn"ÿÿÃ¸<I è6Aÿÿ²¸\¤@ è."ÿÿ£(I ²¸Ğ¦@ è"ÿÿ£,I ²¸\¤@ è"ÿÿ£0I ²¸\¤@ èû!ÿÿ£lI è¥Oÿÿ£\I ²¸\¤@ èà!ÿÿ£hI Ã‹À¡pI èö!ÿÿÃ¸HI è¾@ÿÿ3À£pI Ã‹À                        ¬	A    Xc@ ,,@ à*@ +@ L+@ 
EMenuError
TMenuBreak       ¸	A mbNonembBreak
mbBarBreak	TShortCut    ÿÿ      d
A ¨
A         ~
A 
A ˜   Øª@ ,,@ à*@ +@ ¸A H¸@ ˜A d·@ ÔA <A àA ¸A ğA èA xA 0A ¨A         |@     |@ 8    ÿÿøÿ÷ÿüÿûÿÜA ˜A ÜA A ´A 	TMenuItem	TMenuItem4
A X«@  Menus ¸	A .  ÿàA       €     Break|@    ÿğA       €   € Caption @ (  ÿA       €     Checked @ *  ÿ0A       €     Default @ )  ÿ A       €    EnabledH@ -  ÿA       €     
GroupIndexÔ @ 4  ÿ4  ÿ      €     HelpContext|@ 8  ÿ8  ÿ      €   €	 Hint @ +  ÿ A       €    
 	RadioItemì	A @8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-12T07:44:01.776333Z
63
cpicon92


svn:special svn:externals svn:needs-lock










incomplete
14c2f07d-8935-0410-8134-90508e68d282

mkinstalldirs
file
63



2007-12-31T17:50:58.000000Z
792922784eade1d03ecb1b33ba8bb7c3
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

configure
file
63



2007-12-31T17:50:58.000000Z
5853a1d5ee2190aa8719836cf82201e6
2007-09-22T05:12:27.337331Z
24
botsnlinux
has-props

etracericons.zip
file
63



2007-12-31T17:50:58.000000Z
ebc3ab7d7eed3ae3aea01dd8d4d43f07
2007-12-29T04:55:05.087774Z
52
cpicon92
has-props

Makefile.in
file
63



2007-12-31T17:50:58.000000Z
6a53c6d2136399671bce5bfe46dd97c0
2007-09-03T03:13:45.611773Z
5
botsnlinux

AUTHORS
file
63



2007-12-31T17:50:58.000000Z
7170cde0a73ba562ce009a62a1f2c0ae
2007-09-22T12:24:39.066583Z
29
hamishmorrison

depcomp
file
63



2007-12-31T17:50:58.000000Z
7e26ecc61d5c27c50d334ebe19d5ef06
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

ChangeLog
file
63



2007-12-31T17:50:58.000000Z
f72b5a783bf9a486d6669b817edcb048
2007-09-21T18:50:23.813603Z
14
Torandi

src
dir

config.guess
file
63



2007-12-31T17:50:58.000000Z
510429fd06b6d6ed70295881a9f7a811
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

etracericon.svg
file
63



2007-12-31T17:50:58.000000Z
10196636249783d69051cbf65f061d00
2007-12-29T04:55:05.087774Z
52
cpicon92

config.sub
file
63



2007-12-31T17:50:58.000000Z
58587de484be089541f508f26f0ebc59
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

README
file
63



2007-12-31T17:50:58.000000Z
d41d8cd98f00b204e9800998ecf8427e
2007-09-01T16:38:12.025871Z
2
botsnlinux

contrib
dir

config.h.in
file
63



2007-12-31T17:50:58.000000Z
f8962f020179de9c3e4e1931dc937c71
2007-09-03T03:13:45.611773Z
5
botsnlinux

configure.ac
file
63



2007-12-31T17:50:58.000000Z
1d488474794f5100400c6a65b19718a7
2007-09-03T03:13:45.611773Z
5
botsnlinux

autom4te.cache
dir

doc
dir

INSTALL
file
63



2007-12-31T17:50:58.000000Z
d964bafa888407605e61b31bb2133ed6
2007-09-21T20:45:39.865882Z
17
botsnlinux

COPYING
file
63



2007-12-31T17:50:58.000000Z
94d55d512a9ba36caa9b7df079bae19f
2007-09-01T16:38:12.025871Z
2
botsnlinux

data
dir

Makefile.am
file
63



2007-12-31T17:50:58.000000Z
0bab2800872107e20939a5e9f6930a90
2007-09-01T16:38:12.025871Z
2
botsnlinux

missing
file
63



2007-12-31T17:50:58.000000Z
0a5e8ed778878a2656e3ea2313dac833
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

NEWS
file
63



2007-12-31T17:50:58.000000Z
d41d8cd98f00b204e9800998ecf8427e
2007-09-01T16:38:12.025871Z
2
botsnlinux

aclocal.m4
file
63



2007-12-31T17:50:58.000000Z
c2f06b7cb64ef54dd5a40b3979f3adcc
2007-09-03T03:13:45.611773Z
5
botsnlinux

install-sh
file
63



2007-12-31T17:50:58.000000Z
d4c3da374db4aa2301a21ab3e51ddb21
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ^[ÃSVW‹Ú‹ğ:^-t‹~D…ÿt‹Ö‹Çè¬  ‹Ğ‹Ë‹Çèiıÿÿˆ^-_^[Ã‹À‹P<…Òu3ÀÃ‹BÃ‹ÀSV‹ò‹Øƒ{< uèÿòÿÿ‹Ö‹C<èu—ÿÿ^[Ã‹Àf‰P@²‹ÿQ(ÃˆP,²‹ÿQ(ÃƒÊÿ‹HD…Ét‹Ğ‹Áè=  ‹Ğ‹ÂÃSVW‹ò‹Ø‹CD…Àt4èÿÿÿ…ö}3ö;Æ‹ğN‹ÃèÁÿÿÿ;ğt‹{D‹Ó‹Çè5  ‹Ë‹Ö‹Çè¶   _^[Ã‹ÀU‹ìQSV‰Eü‹EüèGÿÿÿ‹ØK…Û|C3ö‹Ö‹EüèCÿÿÿ‹Ğ‹EÿUFKuê^[Y]Â SV‹ñ‹Ú‹Ãº4
A è’ÿÿ‹Öègÿÿÿ^[ÃSVWU‹Ú‹è:]*tJ„Ût:ƒ}D t4‹EDèåşÿÿ‹øO…ÿ|%G3ö‹Ö‹EDèáşÿÿ€x* t‹Ö‹EDèÑşÿÿÆ@* FOuŞˆ]*²‹Å‹ÿQ(]_^[ÃSVWU‹ñ‹ú‹Øƒ~D t¹6ğ  ²¸¬	A èŸnÿÿèjÿÿƒ{< u²¸\¤@ èlÿÿ‰C<‹ïM…í|.‹C<;h}&‹Õ‹C<èô•ÿÿŠ@-:F-v‹Õ‹C<èâ•ÿÿŠP-‹ÆèşÿÿŠN-‹×‹ÃèŒûÿÿ‹Î‹×‹C<è(–ÿÿ‰^D‰Œ   Ç†ˆ   PA ƒ{$ t‹Ãè$ûÿÿ²‹Ã‹ÿQ(]_^[Ã‹ÀSVW‹ò‹Ø…ö|ƒ{< t‹ÃèØıÿÿ;ğ|èïğÿÿ‹Ö‹C<èe•ÿÿ‹ø‹Ö‹C<èı”ÿÿ3À‰GD3À‰‡ˆ   ‰‡Œ   ƒ{$ t‹Ãè½úÿÿ²‹Ã‹ÿQ(_^[Ã€x) tfƒ¸’    t‹È‹Ğ‹”   ÿ‘   Ã@ VƒÉÿ‹p<…öt	‹Æè>•ÿÿ‹È‹Á^ÃSV‹ò‹Ø‹ÃèCıÿÿ‹Ğ‹Î‹Ãèşÿÿ^[ÃS‹Ø‹ÃèÂÿÿÿƒøÿu¹7ğ  ²¸¬	A è,mÿÿè÷ÿÿ‹Ğ‹Ãèÿÿÿ[ÃSfƒ¸Š    t‹Ø‹Ê‹Ğ‹ƒŒ   ÿ“ˆ   [ÃSV‹Ø„Étƒ{$ t‹Ãèóùÿÿ‹CD…Àt3É‹ÓèÛÿÿÿ^[Ã‹s‹Æº`A èÿÿ„Àt‹Æè^  ^[Ã@ ‹PD…Òtƒº„    t‹‚„   Ã‹ÂÃ‹ÀSV‹ò‹Ø‹CD…Àt‹Óè8ÿÿÿ…öt5‹Æº˜A èÈÿÿ„Àt‹F ‹Óèşşÿÿë‹Æº4
A è¬ÿÿ„Àt	‹Ó‹Æèãşÿÿ^[Ã:P+t
ˆP+²‹ÿQ(ÃU‹ìQSVW„ÒtƒÄğè	ÿÿ‹ùˆUÿ‹Ø‹Ë²¸4
A è<õÿÿ‹ğ‰s ‰Œ   ‹‹@(‰†ˆ   ‰„   ‹Ï3Ò‹ÃèUáÿÿ€}ÿ t
d    ƒÄ‹Ã_^[Y]ÃSV‹Ú‹ğ‹F èÎ
ÿÿ3Ò‹Æèiáÿÿ„Ût‹ÆèÎÿÿ‹Æ^[ÃU‹ìSÿuÿu‹@ f»ÿÿèFÿÿ[]Â ‹@ è|ùÿÿÃ@ SVW‹ú‹ğ‹×‹F f»üÿèÿÿ_^[Ã@ U‹ìQSVW‹ØÆEÿ ‹E€xÿ u‹E‹@ø·S0;Ât+‹E€xÿu‹E‹@ø;C$t‹E€xÿu‹E‹@ø·S@;Âu‹E‰XôÆEÿë2‹Ãèêúÿÿ‹ğN…ö|$F3ÿ‹EP‹×‹Ãèãúÿÿè‚ÿÿÿY„ÀtÆEÿëGNußŠEÿ_^[Y]Ã‹ÀU‹ìƒÄôSˆMÿ‰Uø3Ò‰Uô¹è A ‹P ‹Zd‹Â‘‹ÓèÉòÿÿ‹Eô[‹å]ÃVQˆ$3ö±€<$ t3É·Òè·ÿÿÿë‹@D…Àtƒx4 tó…Àt‹p4‹ÆZ^ÃS3Û·Ò3Éèÿÿÿ…Àt‹ÿR,³‹Ã[ÃS3Û±èvÿÿÿ…Àt‹ÿR,³‹Ã[Ã@ U‹ìQSVW‹ØÆEÿ‹CD…Àt‹URèãÿÿÿYˆEÿ€}ÿu\€{) tR3ÀUh"A dÿ0d‰ ‹Ã‹ÿR,‹E‹@üf‹@@‹Uf;BútÆEÿ3ÀZYYd‰ë éìÿÿ‹E‹Pô¡(I èÄ# èóÿÿëÆEÿ ŠEÿ_^[Y]ÃU‹ìƒÄôS‹Ú‰Eô3À‹Uôƒz$ tj3ÀŠCf‰Eújèu<ÿÿf…À}fEú  jèc<ÿÿf…À}fEú @öC tfEú €3Û·Uú±‹Eôèxşÿÿ‰Eüƒ}ü tU‹EüèÿÿÿY‹Ø€ûtÖƒ}ü •À[‹å]Ã‹ÀU‹ìƒÄôSVW‰Eü‹EüPè<ÿÿ‰Eô‹E˜ şÿÿ‹EƒÀû‰Eø3öë~h   ‹Eø+ÃPSV‹EüPèø;ÿÿ‹ÃèíNÿÿ‹Øh   V‹EüPèØ;ÿÿ‹øf÷Ç tºô#A ‹Ãè?Oÿÿ‹Øf÷Ç@ tºø#A ‹Ãè*Oÿÿ‹Øf÷Ç tºü#A ‹ÃèOÿÿ‹Øº $A ‹ÃèOÿÿ‹ØF;uô}	;]ø‚tÿÿÿ_^[‹å]Ã   $   @   #   ;   U‹ìÄ şÿÿSV‹ğ3ÛÆ… şÿÿ ƒ~$ tU‹Æ‹ÿR,èÿÿÿYƒ~( t‹F(èüÿÿ• şÿÿèUOÿÿ…Àt³F(• şÿÿ¹   èüÿÿ‹Ã^[‹å]Ã‰P$è”ÿÿÿÃ@ Ã@ :P4tˆP4‹P$…Òtj j h°  Rè<ÿÿÃS‹Øƒ{$ t‹Ãè\ÿÿÿ„Àt	‹C$Pè¿9ÿÿ[Ã…Òt‹@ ‹R èôÿÿÃ3Ò‹@ èvôÿÿÃ…Òt‹H ‹Id;J u
3Ò‹@ è[ôÿÿÃ‹ÀSV‹Ø3É3Ò‹Ã‹0ÿV(‹C$…Àtj j h°  Pè<ÿÿ^[Ã‹P0…Òt‹ÂÃè™ûÿÿÃ                        d%A    \¤@ ,,@ à*@ +@ ³@ ´³@ ô³@ 
TPopupListU‹ìƒÄôSVW‹ò‰Eü3ÉUhm'A dÿ1d‰!‹ƒèS„ü   -¾   tƒètMƒè„‚   é˜  ‹Eü‹XK…ÛŒ‰  C3ÿ‹×‹Eüèÿÿf‹Vèüÿÿ„Àt3ÀZYYd‰é  GKuØéY  ‹Eü‹XK…ÛŒJ  C3ÿ‹×‹EüèÈÿÿ‹Vèìûÿÿ„Àt3ÀZYYd‰é_  GKuÙé  ÆEû öFtÆEû‹Eü‹XK…Û|;C3ÿ‹×‹Eüè€ÿÿ·VŠMûè ûÿÿ…Àt‹P8¡(I è$ 3ÀZYYd‰é  GKuÈ3Ò¡(I èş# é¶   ‹F‰Eô‹Eü‹XK…ÛŒ¡   C3ÿ‹×‹Eüèÿÿ‹ÿR,‹Uô;Bu‹×‹Eüèÿÿ‹Uôf‹R±èÖúÿÿ…Àu‹×‹EüèìŒÿÿ‹Uôf‹R3Éèºúÿÿ‹,I ‹R@…Òud    ƒÄëuö‚  t‹Èº   ¡(I èg! ë‹Ğ¡(I èM! 3ÀZYYd‰ëCGK…bÿÿÿ‹FP‹FP‹P‹Eü‹@Pè¼6ÿÿ‰F3ÀZYYd‰ëéşÿÿ‹Uü¡(I èÙ è
ÿÿ_^[‹å]ÃSV‹ò‹Øƒ{ uShp%A èuÌ  ‰C‹Ö‹Ãè‰‹ÿÿ^[Ã‹ÀS‹Ø‹ÃèFÿÿƒ{ u‹CèğÌ  [Ã‹ÀSV„ÒtƒÄğèRÿÿ‹Ú‹ğ3Ò‹Æè)øÿÿ‹F ‰°”   Ç€   X(A ¡(I ‹@ ‰F$ÆF-‹Ö¡ÀI èsÿÿÿ„Ût
d    ƒÄ‹Æ^[ÃSV‹Ú‹ğ‹Ö¡ÀI èvÿÿÿ3Ò‹Æè5øÿÿ„Ût‹Æèÿÿ‹Æ^[Ãfƒx6 t‹È‹A8ÿQ4Ã‹@ ‹@4Ã‹@ ‰P4ÃSVW‹ù‹ò‹Ø‹Ó‹ÃèÎÿÿÿj ¡ÀI ‹@Pj WV3ÀŠC,f‹EĞI fƒÈ·ÀP‹C èñÿÿPè@9ÿÿ_^[ÃU‹ìQSVW3ÀUh)A dÿ0d‰ ¸4
A ‰EüEü3Òè­…ÿÿè¸æÿÿ²¸ ¤@ è8ÿÿ£¼I ²¸\%A è'ÿÿ£ÀI 3ÀZYYd‰h#)A ÃéCÿÿëø_^[Y]Ã@                         `)A    äc@ ,,@ à*@ T@ H@ EOutOfResources                        ¤)A    Xc@ ,,@ à*@ +@ L+@ EInvalidOperation‹ÀTCursor €ÿÿÿ  ‹À                        *A    Ä@ ,,@ à*@ +@ L+@ è9A :A 0:A ¸9A À9A 8:A <:A TDragObject                        x*A     *A ,,@ à*@ +@ L+@ ä:A 0;A H;A ¸9A À9A ğ:A ;A TDragControlObject        è*A             Ø*A @   –A ,,@ à*@ +@ ¨AA H¸@ P¸@ d·@ ˜±A „±A ĞAA TControlCanvasTControlCanvasÀ*A <–A  Controls  @ TAlign       +A alNonealTopalBottomalLeftalRightalClient	TDragMode       T+A dmManualdmAutomatic@ 	TTabOrderÿÿÿÿÿ  
TCaption‹ÀTMouseEvent  SenderTObject ButtonTMouseButton ShiftTShiftState XInteger YInteger@ TMouseMoveEvent  SenderTObject ShiftTShiftState XInteger YInteger	TKeyEvent  SenderTObjectKeyWord ShiftTShiftState‹ÀTKeyPressEvent  SenderTObjectKeyCharTDragOverEvent  SenderTObject SourceTObject XInteger YInteger State
TDragStateAcceptBoolean‹ÀTDragDropEvent  SenderTObject SourceTObject XInteger YIntegerTStartDragEvent  SenderTObject
DragObjectTDragObjectTEndDragEvent  SenderTObject TargetTObject XInteger YInteger    x.A ”/A         Š.A ˆ/A ¨   Øª@ ´RA à*@ +@ üBA H¸@ |SA d·@ ÔA xDA DA ¬HA ğA èA tBA ØFA <FA (MA @CA ôCA ”IA RA  NA üNA EA àNA         |@ T   * ¡ 	  °°°°°	°#°
°°°°öÿõÿôÿóÿòÿñÿğÿïÿøÿîÿ÷ÿíÿìÿëÿêÿûÿéÿèÿçÿxTA ÄTA ¬UA üUA àTA ÔUA $VA tVA WA WA ¸WA ØWA XA 8XA @XA HXA ˆXA PXA lXA ¤XA ¬XA ÄXA ÜXA ˜GA ÌSA èSA ˆPA ˜PA QA øPA DCA PCA 4UA HCA TA LVA °VA xCA TCA MA RA ÔPA TControl@ TControl$.A X«@  Controls 4@ $  ÿtEA       €   € Left4@ (  ÿ”EA       €   € Top4@ ,  ÿ´EA       €   € Width4@ 0  ÿÔEA       €   € Height¸)A L  ÿLA       €     Cursor|@ T  ÿT  ÿ      €   € Hint@         Ø2A         $1A Ê2A ü   $.A ÜjA à*@ +@ \YA H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA äXA ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA  bA èdA ¨cA „eA ,eA ìlA ¸~A A F  N  9 - + ,  G      / .  ‚ „    .°°°°°°°°°°°°"°°°°$°%°&°'°/° ½½½½½,°öÿæÿåÿÿÿäÿãÿâÿêÿüÿéÿáÿ oA LoA loA oA àoA  pA  pA DpA hpA ŒpA °pA ìpA |qA œqA °qA ˆsA ¨sA TtA ttA uA 0uA ¤uA ÄuA øuA vA LvA hvA èvA wA ¬oA ÀoA ĞoA |wA wA ˜wA  wA ¨wA  xA @xA hxA ¤xA äxA $yA ØwA ,wA DwA PwA \wA LyA TyA \yA dyA lyA `zA 8{A Œ{A Ø{A 4|A ÄyA €}A ”rA °rA ”…A ÌrA ÈsA ”tA ŒvA ä…A  A ÌƒA TWinControl‹ÀTWinControl¨0A ”/A 	 Controls Ô @ Ğ  ÿĞ  ÿ      €     HelpContext        Ä3A         ¬3A ´3A ¬   $.A ´RA à*@ +@ x†A H¸@ |SA d·@ ÔA xDA DA ¬HA ğA èA $†A ØFA <FA (MA @CA ôCA ”IA RA  NA üNA EA àNA  ‡A   ¤†A TGraphicControlTGraphicControlT3A ”/A  Controls  ‹À        ¸4A          4A ¨4A    ¨0A ÜjA à*@ +@ @ŠA H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA ì‰A ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA  bA èdA ¨cA „eA ,eA tŠA ¸~A A ÈŠA   lŠA TCustomControlTCustomControl 4A Ø2A 	 Controls  @         °5A         œ5A ¤5A     4A ÜjA à*@ +@ @ŠA H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA ‡A ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA „‡A èdA ¨cA „eA ,eA tŠA ¸~A A °‡A <‰A DˆA  °°ˆA THintWindowTHintWindow5A ¸4A 	 Controls  ‹ÀU‹ìQS]ü¡ØI ‹U‰À   ¡ØI ‹€¨   Pjü‹EPè«+ÿÿjğ‹EPè¸)ÿÿ©   @tjô‹EPè¦)ÿÿ…Àu‹EPjô‹EPè{+ÿÿ¡ØI P·ÈI P‹EPè4+ÿÿ¡ØI P·ÆI P‹EPè+ÿÿÿuÿuÿuÿu‹ØI ÇØI     ÿ¨   ‰Eü‹[Y]Â SVQ‹Ø3ö…Ût%TSèN)ÿÿ…Àtè-"ÿÿ;$u·ÈI PSèÒ(ÿÿ‹ğ‹ÆZ^[Ã‹ÀSV‹(I ‹s …ötQRPVèX*ÿÿ^[Ã3À^[Ãÿÿÿÿ	   crDefault   ÿÿÿÿ   crArrow ÿÿÿÿ   crCross ÿÿÿÿ   crIBeam ÿÿÿÿ
   crSizeNESW  ÿÿÿÿ   crSizeNS    ÿÿÿÿ
   crSizeNWSE  ÿÿÿÿ   crSizeWE    ÿÿÿÿ	   crUpArrow   ÿÿÿÿ   crHourGlass ÿÿÿÿ   crDrag  ÿÿÿÿ   crNoDrop    ÿÿÿÿ   crHSplit    ÿÿÿÿ   crVSplit    ÿÿÿÿ   crMultiDrag ÿÿÿÿ	   crSQLWait   ÿÿÿÿ   crNo    ÿÿÿÿ
   crAppStart  ÿÿÿÿ   crHelp  ÿÿÿÿ   crSize  SVW‹òº   ¿ÜI ¿;Áu³‹Æ‹WèÖûşÿëƒÇJuå3Û‹Ã_^[Ã‹ÀSVWU‹ê‹ø¾   »àI ‹×‹èu1ÿÿ…Àu°¿Sü‰U ëƒÃNuâ3À]_^[ÃSVW‹ú‹ğ‹Ö¸9A è…ÿşÿ‹Ø…Ûu‹Ç‹Öè®ûşÿëW‹ËIº   ‹Æè€şşÿ_^[Ãÿÿÿÿ   |   SVW‹ú‹ğ‹Ö¸H9A èAÿşÿ‹Ø…Ûu‹Ç‹ÖèjûşÿëWS¹ÿÿÿ‹Æè<şşÿ_^[Ãÿÿÿÿ   |   è‹%ÿÿè6ıÿÿ…Àtƒ=|I  t‹|I ;B u¡|I ÃS‹Øèp'ÿÿ3À£|I …Ût,‹Ãº¨0A èòşÿ„Àuƒ{  t‰|I ‹[ ‹ÃèËG  Pè¡'ÿÿ[Ã@ ¡I Ã‹ÀSVÄ ÿÿÿ‹ò‹Ø‹Ô‹èËğşÿ‹Ô‹ÆèBûşÿÄ   ^[Ã3ÀÃSPh@:A è º  ‹ØSèT'ÿÿ‹Ã[ÃS‹Úèà&ÿÿ‹Ãè©º  [Ã@ U‹ì„Òtf¸ôÿëf¸óÿ]Â @ U‹ì]Â Ã@ Ã@ U‹ìƒÄøSVW‹Ú3ÀUh:A dÿ0d‰ ‹-   tƒèt&ë+Uø‹CèH(ÿÿEøP¡àI Pèa#ÿÿEøè  ë°èx  3ÀZYYd‰ëéÍóşÿƒ=ÌI  t3ÀèY  è€öşÿèÏöşÿ_^[YY]Ã„ÒtƒÄğè`òşÿ‰H„Òt
d    ƒÄÃ‹À‹@‹ÿR4Ã@ S‹Ø‹C‹ÿR4…Àt‹C‹ÿR4è4S  [Ã‹ÀS‹Ø‹C‹ÿR4…Àt‹C‹ÿR4èüR  [Ã‹ÀU‹ì„Òt	‹@f‹@Nëf¸óÿ]Â U‹ìQSVW‰Mü‹ú‹ğ€} u‹Ff»óÿè˜ğşÿ‹EP‹Mü‹×‹Ff»ñÿèƒğşÿ_^[Y]Â @ U‹ìƒÄìS‰Mü3É…Àt/‹M‹‰]ì‹Y‰]ğ‹M‰Mø‹Mü‰MôMìQƒâRh/°  Pèo%ÿÿ‹È‹Á[‹å]Â SQ‹Ø…Ût'TSè$ÿÿ…Àtèìÿÿ;$u·ÆI PSè‘#ÿÿ…Àu3ÀZ[Ã°Z[Ã@ Sÿpÿ0è-&ÿÿ‹Ø…Ût‹Ãè¬ÿÿÿ„ÀuSèV#ÿÿ‹Ø…Ûué‹Ã[ÃSV‹ò‹Ø‹ÃèÇÿÿÿ‰j S‹‹ĞI ²è/ÿÿÿ^[ÃS‹Ø3Àƒ=ØI  t#¡ØI PhìI ‹ĞI ‹Ó¡ÜI èÿÿÿ÷ØÀ÷Ø[ÃSVWQ‹Ø€=øI  u'¡äI +™3Â+Âƒø}¡èI +C™3Â+ÂƒøŒ(  €=øI  u&ƒ=üI  t¡èI Pè9"ÿÿ‹Ğ‹äI ¡üI èP  ÆøI ‹Ô‹Ãè3ÿÿÿ‹ğ;5ØI t-°èFÿÿÿ‰5ØI ‹$£ÜI ‹‰ìI ‹C‰ğI 3Àè ÿÿÿ‹‰ìI ‹C‰ğI ‹CP°èÿÿÿ‹Ğ‹¡ĞI ‹8ÿW‹øƒ=üI  tj…ötöF5 tA‹×¡üI è%O  ¡üI €xD u‹CPè!ÿÿ‹Ğ‹¡üI èWO  ëC‹K‹¡üI è*P  ë2¡üI è’P  ¿×¡,I è1õ  Pèƒ#ÿÿë¿×¡,I èõ  Pèn#ÿÿZ_^[ÃS‹Ú£ĞI 3À£ØI häI è÷ ÿÿèê ÿÿ£ôI ˆøI ¡ĞI ‹ÿ£üI ¡ĞI è·ûÿÿ£àI €=øI  t&ƒ=üI  t¡èI PèÅ ÿÿ‹Ğ‹äI ¡üI è—N  €=øI  t
¸äI èşÿÿ[Ã‹ÀU‹ìƒÄøSVWˆUÿ‹ğ‰5ÌI 3ÀUhï>A dÿ0d‰ 3À‰EøÆÔI  Uø‹Æf»ğÿèDíşÿƒ}ø u‹Î²¸\*A èôûÿÿ‰EøÆÔI ŠUÿ‹Eøèÿÿÿ3ÀZYYd‰ëé|ïşÿ3À£ÌI è8òşÿè‡òşÿ_^[YY]ÃU‹ìƒÄôSV‹Ø3À‰Eü3À£ÌI 3ÒUhk@A dÿ2d‰"‹àI ¡ĞI èÄúÿÿ¡ĞI ‰Eüƒ=üI  t¡üI èıN  ë¡ôI Pèô!ÿÿ3ÒUhE@A dÿ2d‰"‹5ØI ‹Æº$.A èìşÿ„ÀtMôºìI ‹Æè¹  ë‹ìI ‰Eô‹ğI ‰Eø€=øI  t°èüÿÿ„Àt„Ûu3Àë°‹Ø3À£ĞI °„Ûu°3Ò‰ìI 3Ò‰ğI 3Ò‰Uô3Ò‰Uø‹ØI RhìI ‹Mü‹Ğ¡ÜI èuûÿÿ‹EøPS‹Mô‹ØI ‹Eü‹ÿS3À£ØI 3ÀZYYd‰hL@A 3À£ĞI Ãéğşÿëñ3ÀZYYd‰hr@A €=ÔI  t‹EüèîêşÿÃéôïşÿëç^[‹å]Ãƒ=ĞI  t3Àè„şÿÿ3À£ÌI ÃSVÿpÿ0è¨!ÿÿ‹Ø3ö…Ût‹Ãèáõÿÿ‹ğ…öuSèÍÿÿ‹Ø…Ûuç‹Æ^[Ã‹ÀSVWUƒÄø‹Ú‹ø3ö‹Çè¸ÿÿÿ‹è…ít‹õ‹Ì‹×‹Åèi  ‹Ô‹Ë‹Åèr'  …Àt‹ğ‹ÆYZ]_^[Ã@ SV‹ò‹Øƒ; u²¸\¤@ èêşÿ‰‹‹Öèrÿÿ^[Ã@ S‹Ø‹èÊsÿÿ‹ƒx u‹èêşÿ3À‰[ÃSVWƒÄø‹ù‹ò‹ØTSè)ÿÿj ‹D$+ÇP‹D$+ÆPSèÓÿÿYZ_^[Ã3Ò¡ I èHrÿÿè—   Ã‹Àëèåÿÿÿ¡ I ƒx ğÃSV‹Ú‹ğ‹Æès   3Ò‹Æèf  „Ût‹Æè£ëşÿ‹Æ^[Ã‹ÀS‹Øƒ{4 u	‹ÃèÈn  [Ãƒ{8 u2¡ I ‹@‹ I ;Buè€ÿÿÿS<‹C4‹ÿQ0‰C8‹Ó¡ I èqÿÿ‹S8‹Ãèˆn  [Ã‹ÀS‹Øƒ{8 t'3Ò‹Ãèrn  ‹Ó¡ I è¾rÿÿ‹C8P‹C<Pè¡ÿÿ3À‰C8[Ã‹ÀSV‹ò‹Ø;s4t
‹Ãèºÿÿÿ‰s4^[ÃU‹ìQSV„ÒtƒÄğèªêşÿˆUÿ‹Ø3Ò‹Ãè$¿ÿÿf¡øBA f‰C4²¸l“A èÒ]  ‹ğ‰sD‰^ÇFKA ÇCH  €ÆC7ÆC8ÆC9ÆC:ÆC^ÆC= fÇCNôÿ€}ÿ t
d    ƒÄ‹Ã^[Y]Ã   ª   SV‹Ú‹ğ‹Ö¡(I è6ö  ‹FDèBèşÿ‹F@è¶1ÿÿ3Ò‹Æ‹ÿQ<3Ò‹ÆèÌ¾ÿÿ„Ût‹Æè1êşÿ‹Æ^[Ã3ÀÃ3ÀÃƒx  •ÀÃ‹@ ÃVW‹ú‹ğ‹Çº¨0A è:èşÿ„Àt	‹×‹Æ‹ÿQ<_^Ã‹ÀSVWƒÄøˆ$‹ğÆD$ ‹Æf»ïÿèmèşÿ‹ø…ÿtOT$‹Æ‹ÿQ0‹ØŠ$4ƒàPWSèVÿÿ‹øSèÿÿ…Àt‹Æ‹ÿRDjWSè:ÿÿSèôÿÿS‹D$PèÿÿÆD$ŠD$YZ_^[Ã‹ÀˆP<Ã‹P …Òt’è+  Ã‹ÀVWU‹ú‹ğ€N6‹o(‹Åº¨0A è~çşÿ„Àt	‹Õ‹Æ‹ÿQ<‹×‹Æè¨Áÿÿ€f6÷ƒ~  t0j 3Éº	°  ‹Æè†  j 3Éº°  ‹Æèv  j 3Éº#°  ‹Æèf  ]_^Ã‹ÀSVW‹Ù‹ò‹ø‹Ë‹Ö‹Çè°¿ÿÿ;wPu€ûu	3Ò‹Çè­  _^[ÃSVQ‹ğŠF;:ĞtMˆ$ˆV;öFuAŠ$H,’À‹Ê€Áı€é’Á:Áu)Š$„Àt",t„Òt€êt‹F0P‹F,P‹N(‹V$‹Æ‹ÿSL‹ÆèôşÿÿZ^[ÃU‹ìSVW‹ù‹ò‹Ø;s$u;{(u‹E;C,u‹E;C0t<3ÉŠS7‹ÃèÚ  ‰s$‰{(‹E‰C,‹E‰C0‹Ã‹ÿRDj 3ÉºG   ‹Ãèu  ‹Ãèşÿÿ_^[]Â @ SV‹ğ‹F,P‹F0P‹N(‹Æ‹ÿSL€N\^[Ã@ SV‹ğ‹F,P‹F0P‹Ê‹Æ‹V$‹ÿSL€N\^[ÃSV‹ğR‹F0P‹N(‹Æ‹V$‹ÿSL€N\^[Ã@ SV‹ğ‹F,PR‹N(‹Æ‹V$‹ÿSL€N\^[Ã@ ‹H$‰
‹H(‰J‹H$H,‰J‹H(H0‰JÃ‹ÀSVWU‹ğ‹B‹:+ÇP‹B‹j+ÅP‹Í‹×‹Æ‹ÿSL]_^[Ã3É‰
3É‰J‹H,‰J‹@0‰BÃ‹ÀSƒÄğ‹Ø‹Ô‹Ã‹ÿQ,‹D$ƒÄ[ÃSVƒÄø‹ò‹Ø‹Ãè   ‹Ğ‹Ì‹Æè1eÿÿ‹Ô‹Ãè¼  YZ^[Ã@ SƒÄğ‹Ø‹Ô‹Ã‹ÿQ,‹D$ƒÄ[ÃSVƒÄø‹ò‹Ø‹Ãè”ÿÿÿ‹Ì‹Öèïdÿÿ‹Ô‹Ãèz  YZ^[ÃSVƒÄø‹ò‹Øƒ{  u$‹C‰$ÆD$Tj ¹/ğ  ²¸¤)A è¶Eÿÿèêşÿ‹Ö‹C ‹ÿQ(‹C$‹C(FYZ^[Ã@ SVWƒÄø‹ù‹ò‹Ø‹Ô‹Ã‹ÿQ(‹$‰‹FD$‰GYZ_^[ÃSVWƒÄø‹ù‹ò‹Ø‹Ô‹Ã‹ÿQ(‹+$‰‹F+D$‰GYZ_^[ÃS‹Úè„®  …Àt‹ÓèáÕ  [Ã@ SVWUƒÄğ‰$‹ú‹ğ;<$„ã   öFtŠ^\ëŠ¤HA öÃt‹$PW‹F$Pè‰ÿÿ‹èë‹n$öÃt‹$PW‹F(Pèoÿÿ‰D$ë‹F(‰D$öÃtöF5u‹$PW‹F$F,PèFÿÿ+Å‰D$ë‹F,‰D$öÃt!öF5u‹$PW‹F(F0Pèÿÿ+D$‰D$ë‹F0‰D$‹D$P‹D$P‹L$‹Õ‹Æ‹(ÿUL€~9 u#öÃt‹$PW‹^D‹ÃèZ  PèÓÿÿ‹Ğ‹ÃèªZ   ¨HA ˆF\ƒÄ]_^[Ã         U‹ìƒÄøSVW3É‰Mø‰Uü‹ø3ÀUh;IA dÿ0d‰ öG4 t4Uø‹Çè£  ‹Uø‹Gèœíşÿu‹w…öt‹Æº$.A è£âşÿ„Àt
öFt3Ûë³‹Uü‹Çè¡½ÿÿ„Ût
‹Uü‹Çè‹  3ÀZYYd‰hBIA EøèÆêşÿÃé$çşÿëğ_^[YY]Ã@ SVWƒÄè‹ò<$¥¥‹ØT$‹Ã‹ÿQ,‹C,+D$$P‹C0+D$D$P‹K(‹S$‹Ã‹ÿSLƒÄ_^[Ã@ SV‹ò‹Ø‹C ;ğt5;Øu¹Cğ  ²¸¤)A è–Bÿÿèaçşÿ‹C …Àt‹Óèƒ  …öt	‹Ó‹ÆèÚ  ^[Ã@ SVQˆ$‹ğŠF7:$t(‹Æf»èÿè	âşÿŠ$ˆF7j 3Éº°  ‹ÆèË  ‹ÆèäùÿÿZ^[Ã:P8tˆP8j 3Éº°  èª  Ãj 3Éº   èš  ÃRº   è  Ã‰PP…Òt’è¸ÿÿÃ‹ÀS‹ØR3É‹Ãº   èj  j 3Éº°  ‹ÃèZ  [ÃSVW‹ú‹ğ‹Æè¢ÿÿÿ‹Ø‹Ç‹Ë3ÒèEêşÿ…ÛtK‹‹Æè•ÿÿÿ_^[ÃU‹ìj SVW‹ò‹Ø3ÀUhKA dÿ0d‰ Uü‹Ãè¬ÿÿÿ‹Eü‹Öè¦ëşÿt‹ÆèQìşÿ‹Ğ‹Ãèlÿÿÿ3ÀZYYd‰hKA EüèûèşÿÃéYåşÿëğ_^[Y]ÃS‹ØÆC9 ‹CDèqW  ;CXt€K\‹CDè`W  ‰CXj 3Éº°  ‹Ãè‘  [Ã@ ‹@D‹ÿQÃ@ Š@94Ã‹ÀŠ@^4Ã‹À:P9tˆP9ƒx  tj 3Éº°  èT  Ã@ :P]tˆP]Æ@^ j 3Éº"°  è6  Ã:P^tˆP^ƒx  tj 3Éº#°  è  Ã@ ;PHt‰PHÆ@: j 3Éº°  èú  ÃŠ@:4Ã‹À:P:tˆP:ƒx  tj 3Éº	°  èÔ  Ã@ f;PLtf‰PLj 3Éº°  è¸  Ã@ S‹Øè íÿÿ;Ø”À[ÃSV‹Ú‹ğ‹Æèãÿÿÿ:Øt„Ût	‹Æè$íÿÿë3Àèíÿÿ^[ÃSV‹ğ²‹Æf»éÿè“ßşÿ^[ÃSV‹ğ3Ò‹Æf»éÿèßşÿ^[ÃSVW‹ò‹Ø‹C …Àtl‹€°   ‹Óè}gÿÿ…À|[‹S ‹º°   ‹W…ö}3ö;Ö‹òN;Æt>‹×’è°fÿÿ‹C ‹€°   ‹Ë‹Öèbgÿÿ±ŠS7‹Ãè.  ‹ÃèG©  ö@6t²f»êÿèßşÿ_^[Ã‹À‹H …Ét„Òt‹‘°   ‹RJèfÿÿÿë3Òè]ÿÿÿÃSVƒÄø‹Øƒ{  u$‹C‰$ÆD$Tj ¹/ğ  ²¸¤)A èh?ÿÿèÃãşÿ‹C ‹ÿQ0‹ğj ‹C(P‹C$PVèÑÿÿ‹C0P‹C,Pj j Vèÿÿ‹ÆYZ^[ÃU‹ìƒÄÜSVWÆEÿ‹E‹@ü‹@ ‹¸°   ‹E‹Pü‹Çègfÿÿ‹Ø…Û~HK‹Ó‹Çèfÿÿ‹ğöF4@t2UÜ‹ÆèøÿÿEÜP‹EƒÀìPEìPè!ÿÿ‹EƒÀìPEìPè±ÿÿ…Àu…Û¸ÆEÿ ŠEÿ_^[‹å]Ã@ U‹ìƒÄèSˆMë‹Ú‰Eü„Ûu‹Eüö@th‹Eüö@5u_‹Eüƒx  tV‹Eü‹@ èÃ4  „ÀtGUì‹Eüè˜÷ÿÿ€}ë u‹Eü‹@ ö@4@uUèÿÿÿY„Àt3Àë°ƒàPEìP‹Eü‹@ èæ2  Pè|ÿÿ[‹å]Ã@ ö@4@•ÁŠP7èeÿÿÿÃ3Òè%ûÿÿÃS‹Ø‹C …Àt‹Ó‹ÿQtöCtöC5t	²‹Ãèşúÿÿ[ÃV‹p …öt‹Æ‹ÿRP^Ã@ ‹ÿRHÃ‹ÀU‹ìƒÄøS‰Eü‹Eü€x7 u‹Eüö@„Ğ   ‹Eüö@5…Ã   ‹Eüƒx  „¶   ‹Eü‹@ èÏ3  „À„£   ‹Eüö@4@„†   ‹Eü‹@ è2  Pè¥ÿÿ‰Eø3ÉUhÕOA dÿ1d‰!‹Uü‹R(‹Ê‹]üK0Q‹Eü‹@$‹È‹]üK,QRP‹EøPèóÿÿ‹Mü‹Uø‹Eü‹@ èf  3ÀZYYd‰hìOA ‹EøP‹Eü‹@ èª1  Pè ÿÿÃéŠàşÿëã‹Eü‹ÿRD‹Eü‹ÿRP[YY]Ã@ SVƒÄğ‹Ú‹ğ‹ÆºäA è—Ûşÿ„Àt¹Nğ  ²¸¤)A è.<ÿÿèùàşÿƒ=ÌI  uU‰5ÌI öF6t8TèÆÿÿL$‹Ô‹Æè	÷ÿÿ‹D$‰$‹D$‰D$‹ÄèoÿÿP3Éº  ‹Æèh  ;5ÌI u	‹Ó‹ÆèÿíÿÿƒÄ^[ÃÃ@ ;ÌI ”ÀÃ‹ÀU‹ìSV‹uÆfƒ¸‚    tQ‹MQŠMQV‹Ø‹Ê‹Ğ‹ƒ„   ÿ“€   ëÆ ^[]Â U‹ìSfƒxz tQ‹MQ‹Ø‹Ê‹Ğ‹C|ÿSx[]Â @ Sfƒ¸Š    t‹Ê‹Ø‹Ğ‹ƒŒ   ÿ“ˆ   [ÃU‹ìSfƒ¸’    tQ‹MQ‹Ø‹Ê‹Ğ‹ƒ”   ÿ“   []Â ‹ÀSVWUƒÄğ‰$‹ø‹$‹[‹s‹î‹Åº\*A è9Úşÿ„Àt‹uL$‹Ó‹ÇèÙõÿÿ‹$Š@‹Ğ€êrt0ëD‹T$R‹T$PD$P‹L$‹Ö‹Çf»òÿèTÚşÿ3ÀŠD$‹$‰Bë‹D$P‹L$‹Ö‹Çf»çÿè0ÚşÿƒÄ]_^[ÃU‹ìƒÄğ‰Uğ‰Mô‹U‰Uø3Ò‰Uü…ÀtUğ‹ÿQ@‹Eü‹å]Â Ã@ SVW‹ò‹ØöCt(‹Ãèğ£  ‹ø…ÿtƒ¿,   t‹Î‹‡,  ‹Ó‹8ÿ„Àur‹=   |`=	  YöC4€u-  t
ƒètƒèuƒ.‹-   tHtHt,Htë+‹Î‹Ó¡(I è›ø  ë€{<u²‹ÃèZıÿÿë€K6ë€c6ş‹Ö‹ÃèÙşÿ_^[ÃSVW‹ò‹Ø‹ƒètJHtHt*ëW‹C@…Àt‹øë¿(SA ‹NI‹F‹×è$ ÿÿècÿÿ‰Fë/‹{@…ÿu3À‰Fë!‹ÇèIÿÿ‰Fë‹Fè”!ÿÿ‹ø‹C@è¶!ÿÿ‰{@_^[Ã       SV‹ò‹Ø‹ÆèS†ÿÿˆC=^[ÃŠ@=’èÿÿÃ‹ÀU‹ì‹E‹@ü‹@…Àt‹UŠ@=‹U‹Rø:B=•À]Ã‹E‹@øŠ@=]ÃU‹ìƒÄøS‰Uü‰Eø‹EøPh,SA ‹EøPh@SA Uè«ÿÿÿY‹ÈºÀSA ‹Eü‹ÿ[YY]Ã   ÿÿÿÿ	   IsControl   fƒ¸š    t‹È‹Ğ‹œ   ÿ‘˜   Ãfƒ¸¢    t‹È‹Ğ‹¤   ÿ‘    ÃU‹ìQSˆMÿfƒxb tŠMÿQ‹MQ‹MQ‹Ø‹Ê‹Ğ‹CdÿS`[Y]Â @ U‹ìQSVWˆMÿ‹ú‹ğöF5u&¿GP¿G
Pf‹Gè   ‹È
MŠUÿ‹Æf»íÿè’×şÿ_^[Y]Â ‹ÀSV‹ò‹Ø‹Ó‹Ãèùòÿÿ‹Ö‹Ã‹ÿQğöC4t	²‹Ãè•÷ÿÿöC4t€K6 ÀTA P‹Ö3É‹Ãèzÿÿÿ^[Ã       SV‹ò‹Ø‹Ó‹Ãè­òÿÿ‹Ö‹Ã‹ÿQğ^[ÃSVW‹ú‹ğ‹Ö‹Æèòÿÿ‹×‹Æ‹ÿQğöF4t	²‹Æè,÷ÿÿöF4t‹Æf»ôÿèçÖşÿ 0UA P‹×3É‹Æè
ÿÿÿ_^[Ã  @   ‹@PÃSVWƒÄì‰$‹øöGu\‹÷…ötV‹Æf»îÿè¦Öşÿ‹Ø…Ût>€{- t83Ò‹Çèòÿÿ‰s0T$‹$è>ÿÿT$L$‹Çè›ñÿÿ‹L$‹T$‹Ã‹ÿS0ë‹v …öuªƒÄ_^[ÃSV‹ò‹Ø‹Ö‹Ã‹ÿQğ ĞUA P‹Ö±‹Ãèhşÿÿ^[Ã     SV‹ò‹Ø‹Ö‹Ã‹ÿQğ øUA P‹Ö±‹Ãè@şÿÿ^[Ã @   SV‹ò‹Ø‹Ö‹Ã‹ÿQğ  VA P‹Ö±‹Ãèşÿÿ^[Ã     SV‹ò‹Ø‹Ö‹Ã‹ÿQğ HVA P‹Ö±‹Ãèğıÿÿ^[Ã @   U‹ìQSˆUÿfƒxj tQ‹UR‹Ø‹ĞŠMÿ‹ClÿSh[Y]Â SVW‹ú‹ğ‹×‹Æ‹ÿQğöF5u¿G
Pf‹GèD  ‹Ğ¿O‹Æf»ìÿèWÕşÿ_^[Ã@ U‹ìQSˆMÿfƒxr tŠMÿQ‹MQ‹MQ‹Ø‹Ê‹Ğ‹CtÿSp[Y]Â @ SVWQˆ$‹ú‹ğöF5u$¿GP¿G
Pf‹GèØ  ‹ÈŠT$‹Æf»ëÿèëÔşÿZ_^[Ã‹ÀSVWƒÄè‹ú‹ğ‹×‹Æ‹ÿQğöF4t	3Ò‹ÆèöôÿÿöF6t:€f6ı‹Ô‹Gèjÿÿÿt$ÿt$T$‹Æ‹ÿQ,D$Pèe	ÿÿ…Àt‹Æf»õÿè‚Ôşÿ‹×3É‹ÆèWÿÿÿƒÄ_^[ÃSV‹ò‹Ø‹Ö‹Ã‹ÿQğ‹Ö±‹Ãè6ÿÿÿ‹V‹Ãè„ıÿÿ^[ÃSV‹ò‹Ø‹Ö‹Ã‹ÿQğ‹Ö±‹Ãèÿÿÿ^[Ã@ V‹ğ‹Æ‹ÿQğ‹Æè;ôÿÿ„Àt3Ò‹Æè>ôÿÿöF6tjÿ3Éº  ‹ÆèÌùÿÿ^Ã‹Àö@tö@5t€x7 tö@4@u3Éë±²èßõÿÿÃ‹À‹ÿRDÃ‹À‹ÿRDÃ‹À‹ÿRDÃ‹ÀS‹Ø€{: t‹C ‹PH‹Ãè^óÿÿÆC:[ÃS‹Ø€{^ t‹C ŠP]‹ÃèóÿÿÆC^[ÃS‹Ø€{9 t‹C ‹PD‹Ãè®òÿÿÆC9[ÃÇB   Ã‹P …ÒtP3É‹Âº°  èùÿÿÃ‹P …ÒtP3É‹Âº°  èşøÿÿÃ3À‰BÃ‹ÀU‹ìQSV„ÒtƒÄğè:ÔşÿˆUÿ‹Ø3Ò‹ÃèpéÿÿShôgA èMš  ‰ƒ¨   ²¸t•A è[L  ‹ğ‰³¼   ‹SH‹ÆèM  ÆƒÆ   ÇƒÈ   ÿÿÿÿ€}ÿ t
d    ƒÄ‹Ã^[Y]ÃSVQˆ$‹Ø‹Ãè™ªÿÿƒ{  t	²‹Ãè‚  ƒ»À    t‹Ã‹ÿRh‹ÃèS  …Àt)‹ĞJ‹Ãè  ‹ğ‹Ö‹Ãè¾  ²‹Æ‹ÿQü‹Ãè*  …Àu×‹ƒ¼   èÑşÿ‹ƒ¨   …Àtèš  3Ò‹Ãèéÿÿ€<$ t‹Ãè~Óşÿ‹ÃZ^[ÃU‹ìƒÄôSV‰Eü‹Eüƒ¸´    „°   ²¸\¤@ èÑşÿ‰Eô3ÉUh¸ZA dÿ1d‰!‹Eü‹€´   ‹@‰Eø‹Uø‹Eôè>[ÿÿ‹]øK…Û|0C3ö‹Eü‹€´   ‹ÖèsYÿÿ‹È   …Ò|;Uø}
‹È‹EôèfZÿÿFKuÓ‹]øK…Û|C3ö‹Ö‹EôèAYÿÿ…Àt‹Öè'  FKuç3ÀZYYd‰h¿ZA ‹Eôè¡ĞşÿÃé§Õşÿëğ^[‹å]Ã@ U‹ìQS‹Ú‰Eü‹EüèŞ  3ÀUh[A dÿ0d‰ ‹Ó‹Eüèéÿÿ3ÀZYYd‰h[A ‹Eüè¹  ÃéWÕşÿëğ‹Eüèİşÿÿ‹Eüƒx  tj 3Éº°  ‹Eüè§öÿÿ‹Eüè‹  [Y]Ã@ U‹ìQSˆMÿ3ÉŠ]ÿşËtşËtşËt$şËt+ë:‹@(;B(œÁë/‹H(H0‹B(B0;ÈŸÁë‹@$;B$œÁë‹H$H,‹B$B,;ÈŸÁ‹Á[Y]Ã…À}‹ÂÃU‹ìSVW‹ğ‹ÂşÈtşÈtşÈt"şÈt+ë5‹E‹@ü‹N0Hë'‹E‹@ü‹N0)Hë‹E‹@ü‹N,ë‹E‹@ü‹N,)H3ÀŠÂƒø‡0  ÿ$…\A 6]A %\A e\A ¢\A Ş\A ]A ‹E‹@ü‹@‹U‹Zü‹+Ã‹V,è`ÿÿÿP‹F0P‹E‹Hü‹I+N0‹E‹@ü‹Ó‹Æ‹ÿSLéÑ   ‹E‹@ü‹@‹U‹Zü‹+Ã‹V,è ÿÿÿP‹F0P‹E‹Hü‹I‹E‹@ü‹Ó‹Æ‹ÿSLé”   ‹^,S‹E‹@ü‹@‹U‹zü‹+Ç‹V0èŞşÿÿP‹E‹@ü‹Ï‹E‹Pü‹+Ó‹Æ‹ÿSLëX‹F,P‹E‹@ü‹@‹U‹Zü‹[+Ã‹V0è¢şÿÿP‹E‹@ü‹Ë‹E‹Pü‹R‹Æ‹ÿSLë‹E‹@üPèÿÿ…Àu‹E‹Pü‹ÆèŞèÿÿ_^[]ÃU‹ìƒÄøSVWˆEÿ‹E‹@øèVÿÿ‹Eƒxô tC‹E‹@ô€x7 u‹E‹@ôö@t+‹E‹@ôö@5u‹E‹@ôŠ@;:Eÿu‹E‹Pô‹E‹@øè™Uÿÿ‹E‹@ğè:  ‹øO…ÿ||GÇEø    ‹E‹@ğ‹Uøèí  ‹ØŠC;:EÿuV€{7 uöCtJöC5uD‹E;Xôt<3öëF‹E‹@ø;p}‹E‹@ø‹ÖèÌUÿÿ‹ĞŠMÿ‹Ãè,ıÿÿ„Àt×‹E‹@ø‹Ë‹ÖèVÿÿÿEøOuŒ‹E‹@ø‹xO…ÿ|)GÇEø    ‹EP‹E‹@ø‹UøèUÿÿŠUÿèKıÿÿYÿEøOuß_^[YY]ÃU‹ìSV³‹E‹@ğèf  ‹ğNƒş |‹E‹@ğ‹Öè!  €x; uNƒşÿuç3Û‹Ã^[]ÃU‹ìƒÄğ‰Mü‰Uô‰EğUè¯ÿÿÿY„Àtg²¸\¤@ èbÌşÿ‰Eø3ÀUh"_A dÿ0d‰ U°èUşÿÿYU°èLşÿÿYU°èCşÿÿYU°è:şÿÿYU°è1şÿÿY3ÀZYYd‰h)_A ‹Eøè7ÌşÿÃé=Ñşÿëğ‹å]Ã@ U‹ìƒÄìS‹Ú‰Eü‹EüèÌ#  „Àth‹Eüfƒ¸Ì    t	‹Eü€H6ëR‹EüèR   3ÀUh©_A dÿ0d‰ Uì‹Eü‹ÿQ,Mì‹Ó‹Eü‹ÿST3ÀZYYd‰h°_A ‹Eü€`6ï‹Eüè   Ãé¶Ğşÿëé[‹å]Ã@ fÿ€Ì   ÃfÿˆÌ   fƒ¸Ì    uö@6tè   Ã@ 3ÒèIÿÿÿÃë‹R …Òt;Âuõ…Ò•ÀÃSV‹Ú‹ğ‹Æè–  …Àt	‹Ë‹Öèº  ^[Ã@ SV‹Ú‹ğ…Ût<‹Ãº¨0A ènËşÿ„Àt†´   ‹ÓèÅàÿÿ†¸   ‹Óè¸àÿÿë†°   ‹Óè©àÿÿ‰s ^[Ã@ SV‹Ú‹ğ‹Ãº¨0A è&Ëşÿ„Àt†¸   ‹Óè¥àÿÿ†´   ‹Óè˜àÿÿë†°   ‹Óè‰àÿÿ3À‰C ^[ÃSV‹Ú‹ğj‹Ëº,°  ‹Æèñÿÿ‹Ó‹ÆèMÿÿÿöC6utj 3Éº	°  ‹Ãèóğÿÿj 3Éº°  ‹Ãèãğÿÿj 3Éº#°  ‹ÃèÓğÿÿ‹Ãº¨0A èÊşÿ„Àtj 3Éº°  ‹Ãè³ğÿÿ‹Æè˜  ë‹ÆèÛ!  „Àt‹Ã‹ÿRD‹Ó‹Æèçıÿÿ^[ÃSVW‹Ú‹ğ‹Ãº¨0A èAÊşÿ„Àt²‹û‹Çèşÿÿ‹ÇèÓ  ë‹Æè’!  „Àt3ÉŠS7‹Ãè†ìÿÿ‹Ó‹ÆèÑşÿÿj ‹Ëº,°  ‹Æè1ğÿÿ‹Æè2şÿÿ_^[Ã‹ÀV‹°°   …öt‹Në3É;Ê~	‹ÆèRÿÿ^Ã+Ñ‹€´   èôQÿÿ^Ã‹À3Ò‹ˆ°   …ÉtQ‹ˆ´   …ÉtQ‹ÂÃSVWU‹ò‹è‹ÅèÑÿÿÿ‹ØK…Û|C3ÿ‹×‹Åèÿÿÿ‹Ö‹ÿQ@ƒ~ uGKuæ]_^[ÃƒÄğ·Ò‰$3Ò‰T$3Ò‰T$3Ò‰T$‹Ôè¦ÿÿÿƒÄÃ‹ÀSV‹ñ‹Ú…öt.C$PV¡I Pènüşÿ…ÀuC$PVj è^üşÿ‹C$%¿ÿÿƒÈ‰C$^[ÃU‹ìÄüşÿÿSVW3É‰Mü‹Ú‹ğ3ÀUh™cA dÿ0d‰ ‹Ã3ÉºŒ   ècÆşÿ‹F@‰ÇC   DöF4tK   öFu€~8 uK   €¾Ä    tK   ‹F$‰C‹F(‰C‹F,‰C‹F0‰C3À‰C‹~ …ÿt
‹Çè@  ‰CÇC$   ÇC(^@ h   j èışÿ‰C<3À‰C@•üşÿÿ‹è6Çşÿ•üşÿÿEüè¨Ñşÿ‹UüCLè½ÿÿ3ÀZYYd‰h cA EüèhĞşÿÃéÆÌşÿëğ_^[‹å]ÃSÄDÿÿÿ‹Ø‹Ô‹Ã‹ÿQ\ƒ|$ u9öD$@t2‹C‰„$´   Æ„$¸   „$´   Pj ¹/ğ  ²¸¤)A èÇ(ÿÿè"Íşÿ‹D$(‰ƒ¬   „$Œ   PD$PP¡I PèĞúşÿ÷ØÀ÷Ø„ÀtºÔ5A ;”$   tR„Àt¡I PD$PPèÔışÿÇD$(Ô5A ¡I ‰D$4D$L‰D$HD$$Pèqüşÿf…Àu¹,ğ  ²¸`)A èË'ÿÿè–Ìşÿ‰ØI ‹Ô‹Ã‹ÿQ`ƒ»À    u¹-ğ  ²¸`)A è'ÿÿèhÌşÿ‹C@èÿÿ3À‰C@‹ÃèT  j‹CDè®<  ‹Èº0   ‹ÃèøìÿÿÄ¼   [ÃSV‹Ú‹ğ‹C P¡I Pj ‹CP‹CP‹CP‹CP‹CP‹CP‹PCLP‹CPèâøşÿ‰†À   ^[ÃSVW‹Ø‹Ãèøäÿÿ‹ğƒş}¸€eA è_ÿÿ‰C@ëFè2ÿÿ‹ø‰{@‹Çè:ÿÿ‹È‹×‹ÃèÓäÿÿè"Üÿÿ‹Ã‹ÿRh_^[Ã       ‹€À   Pè´øşÿÃ@ SVW‹Ú‹ø‹‡´   ‹ÓètNÿÿ‹ğF‹‡´   ‹XK+Ş|C‹‡´   ‹ÖèNÿÿ‹€À   …ÀuFKuå3À_^[ÃSV‹Øƒ»À    uZ‹Ã‹ÿRd‹óV·ÈI P‹ƒÀ   PèuûşÿV·ÆI P‹ƒÀ   Pè`ûşÿ‹s …öt jj j j j ‹Ó‹Æè^ÿÿÿP‹ƒÀ   Pèyûşÿ^[Ã‹ÀSVW‹Øƒ»À    t2‹ƒ´   …Àt!‹pN…ö|F3ÿ‹×‹ƒ´   è\MÿÿèËÿÿÿGNuê‹Ã‹ÿRl_^[ÃSV‹ğƒ¾À    t0‹Æè  ‹Ø‹ÆèŸÿÿÿ‹Æè  „Ûtƒ¾À    t‹†À   Pè›úşÿ^[ÃU‹ìƒÄøSVW‰Eü‹Eü€x7 u‹Eüö@t‹Eüö@5u	‹Eüö@6t3Àë°ˆEû€}û tE‹Eüƒ¸À    u‹Eü‹ÿRX‹Eü‹€´   …Àt$‹XK…Û|C3ö‹Ö‹Eü‹€´   èLÿÿè€ÿÿÿFKuç‹Eüƒ¸À    t`‹EüŠ€Ç   :EûtRŠEû‹Uüˆ‚Ç   3ÀUhŸgA dÿ0d‰ j 3Éº°  ‹EüèCêÿÿ3ÀZYYd‰ëéÌÆşÿŠEû4‹Uüˆ‚Ç   èÉşÿèĞÉşÿ_^[YY]ÃS‹Ø‹Ãè>  ‹Ğ…Òt‹Ã;Ğt‹@ €¸Ç    t;Ğuğ‹ÃèÛşÿÿ[ÃU‹ìQSVW‰Eü3ÀUhKhA dÿ0d‰ 3ÀUh:hA dÿ0d‰ ‹Eü‹ÿQ@3ÀZYYd‰hAhA è`Ùÿÿèw_  Ãé%Èşÿëî3ÀZYYd‰ëé Æşÿ‹Uü¡(I èûİ  è*Éşÿ_^[Y]ÃSVWUƒÄäˆ$‹ê‹ø‹‡°   …À„˜   ‹pNƒş Œ‹   ‹Ö‹‡°   è6Kÿÿ‹ØL$‹U+S(‹E +C$èCÿÿÿt$ÿt$T$‹Ã‹ÿQ,D$Pèøşÿ…Àt<öCt€{7 u<öC5t6€{7 t$ŠC8
$tD$èÑùşÿP3Éº
°  ‹ÃèÊèÿÿ…ÀuNƒşÿ…uÿÿÿ3Û‹ÃƒÄ]_^[ÃSVWƒÄğ‹ú‹ğ‹Æè?  ‹Øèœõşÿ;Øu3Ûƒ=|I  t-¡|I ;p u#‹|I ëT$‹GèOùşÿT$3É‹Æèîşÿÿ‹Ø3À…Ût+¿G+C$‰$¿G
+C(‰D$‹Äè2ùşÿP‹O‹‹Ãè-èÿÿ°ƒÄ_^[ÃSVWƒÄğ‹ò‹Ø‹=„   tiƒèt.HtRƒè„¸   éë    ÿÿÿƒè	‚˜   	ÿÿÿƒè
réÎ   ‹Ãè	Œ  ‹ø…ÿ„½   ‹Ó‹Çèn°  „À„µ   é§   öC6 …¦   é˜   ‹Ö‹ÃèÍçÿÿƒ~ÿ…   T$‹FèkøşÿT$‹Ì‹ÃèöÜÿÿ‹Ô3É‹Ãèÿıÿÿ…ÀtfÇF   ë]‹Ö‹Ãè©şÿÿ„ÀuPëE‹Ãè æÿÿ„ÀuCë8‹Ãèß  ‹øè<ôşÿ;øu&ƒ=|I  t¡|I ;X uj 3Éº   ¡|I èçÿÿ‹Ö‹Ãè5çÿÿƒÄ_^[Ã‹ÀSVWU‹ò‹Ø‹«À   …í„   ‹>‹ÇÎşÿÿƒèrDÿÿƒèrëY‹^S‹FPÇ ¼  WSèöşÿ‰Fëe‹CD‹@è¿4  P‹FPèòşÿ‹ƒ¼   èö:  è¥4  P‹FPè¿ñşÿ‹ƒ¼   è;  ‰Fë&‹FP‹FP‹PU‹ƒ¬   Pè/òşÿ‰Fë	‹Ö‹Ãè)çÿÿ]_^[ÃSVW‹ò3ÛèğÊÿÿ‹ø…ÿt‹FP‹N‹Â ¼  ‹Çè æÿÿ‰F³‹Ã_^[ÃU‹ìƒÄ°SVW‰Uø‰Eü‹]ø‹[…ÛuE°P‹Eüè  Pèñşÿ‹Ø3ÉUhİlA dÿ1d‰!‹/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* This file is complex.  However, the ultimate purpose is to make
   adding new configuration parameters easy.  Here's what you need to
   do to add a new parameter:

   1. Choose a name and type for the parameter.  By convention,
   parameters have lower case names and words_are_separated_like_this.
   Possible types are bool, int, char, and string.  (Nothing is ruling
   out floating point types; I just haven't needed them.)  As an
   example in the subsequent steps, suppose we wish to add a parameter
   foo_bar of type string.

   2. Add a field for the parameter to the 'params' struct defined
   below.  In our example, we would add the line
       struct param foo_bar;
   to the definition of struct params.

   Note that the order of the entries in this struct determines the
   order that the parameters will appear in the configuration file.

   3. Initialize and assign a default value to the parameter in the
   init_game_configuration() function.  The INIT_PARAM_<TYPE> macros
   will do this for you.  In our example, we would add the line
       INIT_PARAM_STRING( foo_bar, "baz" )
   to assign a default value of "baz" to the parameter foo_bar.

   4. Create the getparam/setparam functions for the parameter.  This
   is done using the FN_PARAM_<TYPE> macros.  In our example, we would
   add the line 
       FN_PARAM_STRING( foo_bar )
   somewhere in the top-level scope of this file (to keep things neat
   group it with the other definitions).  The will create
   getparam_foo_bar() and setparam_foo_bar() functions that can be
   used to query the value of the parameter.

   5. Create the prototypes for the getparam/setparam functions.  This
   is done in game_config.h using the PROTO_PARAM_<TYPE> macros.  In
   our example, we would add the line
       PROTO_PARAM_STRING( foo_bar );
   to game_config.h.

   6. You're done!  */

#include "file_util.h"
#include "game_config.h"
#include "string_util.h"
#include "course_mgr.h"
#include "winsys.h"
#include "ppgltk/audio/audio.h"


#if defined( WIN32 )
#  define OLD_CONFIG_FILE "etracer.cfg"
#else
#  define OLD_CONFIG_FILE ".etracer"
#endif /* defined( WIN32 ) */

#if defined( WIN32 )
#  define CONFIG_DIR "config"
#  define CONFIG_FILE "options.txt"
#else
#  define CONFIG_DIR ".etracer"
#  define CONFIG_FILE "options"
#endif /* defined( WIN32 ) */

#ifndef DATA_DIR
#  if defined( WIN32 )
#    define DATA_DIR "."
#  else
#    define DATA_DIR PP_DATADIR
#  endif /* defined( WIN32 ) */
#endif




static const char* sp_config_file=NULL;


/* Identifies the parameter type */
typedef enum {
    PARAM_STRING,
    PARAM_CHAR,
    PARAM_INT,
    PARAM_BOOL
} param_type;

/* Stores the value for all types */
typedef union {
    char* string_val;
    char  char_val;
    int   int_val;
    bool bool_val;
} param_val;

/* Stores state for each parameter */
struct param {
    int loaded;
    char *name;
    param_type type;
    param_val val;
    param_val deflt;
    char *comment;
};

/*
 * These macros are used to initialize parameter values
 */

#define INIT_PARAM( nam, val, typename, commnt ) \
   Params.nam.loaded = false; \
   Params.nam.name = #nam; \
   Params.nam.deflt.typename ## _val  = val; \
   Params.nam.comment = commnt;

#define INIT_PARAM_STRING( nam, val, commnt ) \
   INIT_PARAM( nam, val, string, commnt ); \
   Params.nam.type = PARAM_STRING;

#define INIT_PARAM_CHAR( nam, val, commnt ) \
   INIT_PARAM( nam, val, char, commnt ); \
   Params.nam.type = PARAM_CHAR;

#define INIT_PARAM_INT( nam, val, commnt ) \
   INIT_PARAM( nam, val, int, commnt ); \
   Params.nam.type = PARAM_INT;

#define INIT_PARAM_BOOL( nam, val, commnt ) \
   INIT_PARAM( nam, val, bool, commnt ); \
   Params.nam.type = PARAM_BOOL;


/*
 * These functions are used to get and set parameter values
 */

void fetch_param_string( struct param *p )
{
    const char *val;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    if ( val == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( val );
    }
    p->loaded = true;

}

void set_param_string( struct param *p, CONST84 char *new_val )
{
    const char *ret;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    if ( p->loaded ) {
	free( p->val.string_val );
    }
    ret = Tcl_SetVar( tclInterp, p->name, new_val, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( new_val );
    }
    p->loaded = true;

}

void fetch_param_char( struct param *p )
{
    const char *str_val;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL || str_val[0] == '\0' ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = str_val[0];
    }
    p->loaded = true;
}

void set_param_char( struct param *p, char new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    buff[0] = new_val;
    buff[1] = '\0';

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_int( struct param *p )
{
    CONST84 char *str_val;
    int val;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );
    
    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL 
	 || Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR  ) 
    {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = val;
    }
    p->loaded = true;
}

void set_param_int( struct param *p, int new_val )
{
    char buff[30];
    const char *ret;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_bool( struct param *p )
{
    CONST84 char *str_val;
    int val;
    bool no_val = false;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL ) {
	no_val = true;
    } else if ( strcmp( str_val, "false" ) == 0 ) {
	p->val.bool_val = false;
    } else if ( strcmp( str_val, "true" ) == 0 ) {
	p->val.bool_val = true;
    } else if ( Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR ) {
	no_val = true;
    } else {
	p->val.bool_val = (val == 0) ? false : true ;
    }

    if ( no_val ) {
	p->val.bool_val = p->deflt.bool_val;
    }

    p->loaded = true;
}

void set_param_bool( struct param *p, bool new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val ? 1 : 0 );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.bool_val = p->deflt.bool_val;
    } else {
	p->val.bool_val = new_val;
    }
    p->loaded = true;
}


/*
 * Creates set/get functions for each parameter
 */
#define FN_PARAM( name, typename, type ) \
    type getparam_ ## name() { \
        if ( !Params.name.loaded ) { \
            fetch_param_ ## typename( &( Params.name ) ); \
        } \
        return Params.name.val.typename ## _val; \
    } \
    void setparam_ ## name( type val) { \
        set_param_ ## typename( &( Params.name ), val ); } 

#define FN_PARAM_STRING( name ) \
    FN_PARAM( name, string, char* )

#define FN_PARAM_CHAR( name ) \
    FN_PARAM( name, char, char )

#define FN_PARAM_INT( name ) \
    FN_PARAM( name, int, int )

#define FN_PARAM_BOOL( name ) \
    FN_PARAM( name, bool, bool )


/*
 * Main parameter struct
 */
struct params {
    struct param data_dir;
    struct param fullscreen;
    struct param x_resolution;
    struct param y_resolution;
    struct param x_resolution_half_width;		
    struct param bpp_mode;
    struct param capture_mouse; 
    struct param force_window_position;
    struct param quit_key;
    struct param turn_left_key;
    struct param turn_right_key;
    struct param trick_modifier_key;
    struct param brake_key;
    struct param paddle_key;
    struct param jump_key;
    struct param reset_key;
    struct param follow_view_key;
    struct param behind_view_key;
    struct param above_view_key;
    struct param view_mode; /* coresponds to view_mode_t */
    struct param screenshot_key;
    struct param pause_key;

    struct param joystick_paddle_button;
    struct param joystick_brake_button;
    struct param joystick_jump_button;
    struct param joystick_trick_button;
    struct param joystick_continue_button;
    struct param joystick_x_axis;
    struct param joystick_y_axis;
	struct param disable_joystick;

    struct param no_audio;
    struct param sound_enabled;
    struct param music_enabled;
    struct param sound_volume; /* 0-128 */
    struct param music_volume; /* 0-128 */
    struct param audio_freq_mode; /* 0 = 11025, 
				     1 = 22050, 
				     2 = 44100 */
    struct param audio_format_mode; /* 0 = 8 bits, 
				       1 = 16 bits */
    struct param audio_stereo; 
    struct param audio_buffer_size; 

    struct param display_fps;
	struct param display_course_percentage;	
	struct param course_detail_level;
    struct param forward_clip_distance;
    struct param backward_clip_distance;
    struct param tree_detail_distance;
    struct param terrain_blending;
    struct param perfect_terrain_blending;
    struct param terrain_envmap;
    struct param disable_fog;
		
    struct param stencil_buffer;
	struct param enable_fsaa;	
	struct param multisamples;
		
	struct param always_save_event_race_data;
		
	struct param draw_tux_shadow;	
    struct param tux_sphere_divisions;
    struct param tux_shadow_sphere_divisions;
    struct param draw_particles;
    struct param track_marks;
    struct param ui_snow;
    struct param nice_fog;
    struct param use_cva;
    struct param cva_hack;
    struct param use_sphere_display_list;
    struct param do_intro_animation;
    struct param mipmap_type; /* 0 = GL_NEAREST,
				 1 = GL_LINEAR,
				 2 = GL_NEAREST_MIPMAP_NEAREST,
				 3 = GL_LINEAR_MIPMAP_NEAREST,
				 4 = GL_NEAREST_MIPMAP_LINEAR,
				 5 = GL_LINEAR_MIPMAP_LINEAR
			      */
    struct param ode_solver; /* 0 = Euler,
				1 = ODE23,
				2 = ODE45
			     */
    struct param fov; 
    struct param debug; 
    struct param warning_level; 
    struct param write_diagnostic_log;
	struct param disable_collision_detection;
	struct param ui_language;
	struct param disable_videomode_autodetection;			
};

static struct params Params;


/*
 * Initialize parameter data
 */

void init_game_configuration()
{
    INIT_PARAM_STRING( 
	data_dir, DATA_DIR, 
	"# The location of the ET Racer data files" );

	INIT_PARAM_BOOL( 
	stencil_buffer, false, 
	"# Set this to true to activate the stencil buffer" );
	
	INIT_PARAM_BOOL( 
	enable_fsaa, false, 
	"# Set this to true to activate FSAA" );

	INIT_PARAM_INT( 
	multisamples, 2,
	"# Set multisamples for FSAA" );
	
    INIT_PARAM_BOOL( 
	draw_tux_shadow, false, 
	"# Set this to true to display Tux's shadow.  Note that this is a \n"
	"# hack and is quite expensive in terms of framerate.\n"
	"# [EXPERT] This looks better if your card has a stencil buffer; \n"
	"# if compiling use the --enable-stencil-buffer configure option \n"
	"# to enable the use of the stencil buffer" );
	
	

    INIT_PARAM_BOOL( 
	draw_particles, true,
	"# Controls the drawing of snow particles that are kicked up as Tux\n"
	"# turns and brakes.  Setting this to false should help improve \n"
	"# performance." );

    INIT_PARAM_INT( 
	tux_sphere_divisions, 15,
	"# [EXPERT] Higher values result in a more finely subdivided mesh \n"
	"# for Tux, and vice versa.  If you're experiencing low framerates,\n"
	"# try lowering this value." );

    INIT_PARAM_INT( 
	tux_shadow_sphere_divisions, 3,
	"# [EXPERT] The level of subdivision of Tux's shadow." );

    INIT_PARAM_BOOL( 
	nice_fog, false,
	"# [EXPERT] If true, then the GL_NICEST hint will be used when\n"
	"# rendering fog.  On some cards, setting this to false may improve\n"
	"# performance.");

    INIT_PARAM_BOOL( 
	use_sphere_display_list, true,
	"# [EXPERT]  Mesa 3.1 sometimes renders Tux strangely when display \n"
	"# lists are used.  Setting this to false should solve the problem \n"
	"# at the cost of a few Hz." );

    INIT_PARAM_BOOL( 
	display_fps, false,
	"# Set this to true to display the current framerate in Hz." );

    INIT_PARAM_BOOL( 
	display_course_percentage, true,
	"# Set this to true to display a progressbar of \n"
	"# the course percentage." );

    INIT_PARAM_INT( 
	x_resolution, 800,
	"# The horizontal size of the Tux Racer window" );

    INIT_PARAM_INT( 
	y_resolution, 600,
	"# The vertical size of the Tux Racer window" );

	INIT_PARAM_BOOL( 
	x_resolution_half_width, false, 
	"# Set this to true to use only half of the resolution width" );

    INIT_PARAM_BOOL( 
	capture_mouse, false,
	"# If true, then the mouse will not be able to leave the \n"
	"# Tux Racer window.\n"
	"# If you lose keyboard focus while running Tux Racer, try setting\n"
	"# this to true." );

    INIT_PARAM_BOOL( 
	do_intro_animation, true,
	"# If false, then the introductory animation sequence will be skipped." 
	);

    INIT_PARAM_INT( 
	mipmap_type, 5,
	"# [EXPERT] Allows you to control which type of texture\n"
	"# interpolation/mipmapping is used when rendering textures.  The\n"
	"# values correspond to the following OpenGL settings:\n"
	"#\n"
        "#  0: GL_NEAREST\n"
        "#  1: GL_LINEAR\n"
        "#  2: GL_NEAREST_MIPMAP_NEAREST\n"
	"#  3: GL_LINEAR_MIPMAP_NEAREST\n"
        "#  4: GL_NEAREST_MIPMAP_LINEAR\n"
        "#  5: GL_LINEAR_MIPMAP_LINEAR\n"
	"#\n"
	"# On some cards, you may be able to improve performance by\n"
        "# decreasing this number, at the cost of lower image quality." );

    INIT_PARAM_BOOL( 
	fullscreen, true,
	"# If true then the game will run in full-screen mode." );

    INIT_PARAM_INT( 
	bpp_mode, 0,
	"# Controls how many bits per pixel are used in the game.\n"
	"# Valid values are:\n"
	"#\n"
	"#  0: Use current bpp setting of operating system\n"
	"#  1: 16 bpp\n"
	"#  2: 32 bpp\n"
	"# Note that some cards (e.g., Voodoo1, Voodoo2, Voodoo3) only support\n"
	"# 16 bits per pixel." );

    INIT_PARAM_BOOL( 
	force_window_position, false ,
	"# If true, then the Tux Racer window will automatically be\n"
	"# placed at (0,0)" );

    INIT_PARAM_INT( 
	ode_solver, 2 ,
	"# Selects the ODE (ordinary differential equation) solver.  \n"
	"# Possible values are:\n"
	"#\n"
	"#   0: Modified Euler     (fastest but least accurate)\n"
        "#   1: Runge-Kutta (2,3)\n"
	"#   2: Runge-Kutta (4,5)  (slowest but most accurate)" );

    INIT_PARAM_STRING( 
	quit_key, "q escape" ,
	"# Key binding for quitting a race" );
    INIT_PARAM_INT( 
	turn_left_key, SDLK_LEFT ,
	"# Key binding for turning left" );
    INIT_PARAM_INT( 
	turn_right_key, SDLK_RIGHT ,
	"# Key binding for turning right" );
    INIT_PARAM_INT( 
	trick_modifier_key, 't' ,
	"# Key binding for doing tricks" );
    INIT_PARAM_INT( 
	brake_key, SDLK_DOWN ,
	"# Key binding for braking" );
    INIT_PARAM_INT( 
	paddle_key, SDLK_UP ,
	"# Key binding for paddling (on the ground) and flapping (in the air)" 
	);
    INIT_PARAM_STRING( 
	follow_view_key, "1" ,
	"# Key binding for the \"Follow\" camera mode" );
    INIT_PARAM_STRING( 
	behind_view_key, "2" ,
	"# Key binding for the \"Behind\" camera mode" );
    INIT_PARAM_STRING( 
	above_view_key, "3" ,
	"# Key binding for the \"Above\" camera mode" );
    INIT_PARAM_INT( 
	view_mode, 1 ,
	"# Default view mode. Possible values are\n" 
	"#\n"
	"#   0: Behind\n"
	"#   1: Follow\n"
	"#   2: Above" );
    INIT_PARAM_STRING( 
	screenshot_key, "=" ,
	"# Key binding for taking a screenshot" );
    INIT_PARAM_STRING( 
	pause_key, "p" ,
	"# Key binding for pausing the game" );
    INIT_PARAM_INT( 
	reset_key, 'r' ,
	"# Key binding for resetting the player position" );
    INIT_PARAM_INT( 
	jump_key, 'e' ,
	"# Key binding for jumping" );

    INIT_PARAM_INT( 
	joystick_paddle_button, 0 ,
	"# Joystick button for paddling (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_brake_button, 2 ,
	"# Joystick button for braking (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_jump_button, 3 ,
	"# Joystick button for jumping (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_trick_button, 1 ,
	"# Joystick button for doing tricks (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_continue_button, 0 ,
	"# Joystick button for moving past intro, paused, and \n"
	"# game over screens (numbering starts at 0)" );
    
    INIT_PARAM_INT(
	joystick_x_axis, 0 ,
	"# Joystick axis to use for turning (numbering starts at 0)" );

    INIT_PARAM_INT(
	joystick_y_axis, 1 ,
	"# Joystick axis to use for paddling/braking (numbering starts at 0)" );
   
	INIT_PARAM_BOOL(
	disable_joystick, false ,
	"# Disables the joystick support" );

    INIT_PARAM_INT( 
	fov, 60 ,
	"# [EXPERT] Sets the camera field-of-view" );
    INIT_PARAM_STRING( 
	debug, "" ,
	"# [EXPERT] Controls the Tux Racer debugging modes" );
    INIT_PARAM_INT( 
	warning_level, 100 ,
	"# [EXPERT] Controls the Tux Racer warning messages" );
    INIT_PARAM_INT( 
	forward_clip_distance, 100 ,
	"# Controls how far ahead of the camera the course\n"
	"# is rendered.  Larger values mean that more of the course is\n"
	"# rendered, resulting in slower performance. Decreasing this \n"
	"# value is an effective way to improve framerates." );
    INIT_PARAM_INT( 
	backward_clip_distance, 10 ,
	"# [EXPERT] Some objects aren't yet clipped to the view frustum, \n"
	"# so this value is used to control how far up the course these \n"
	"# objects are drawn." );
    INIT_PARAM_INT( 
	tree_detail_distance, 20 ,
	"# [EXPERT] Controls the distance at which trees are drawn with \n"
	"# two rectangles instead of one." );
    INIT_PARAM_BOOL( 
	terrain_blending, true ,
	"# Controls the blending of the terrain textures.  Setting this\n"
	"# to false will help improve performance." );
    INIT_PARAM_BOOL( 
	perfect_terrain_blending, false ,
	"# [EXPERT] If true, then terrain triangles with three different\n"
	"# terrain types at the vertices will be blended correctly\n"
	"# (instead of using a faster but imperfect approximation)." );
    INIT_PARAM_BOOL( 
	terrain_envmap, true ,
	"# If true, then the ice will be drawn with an \"environment map\",\n"
	"# which gives the ice a shiny appearance.  Setting this to false\n"
	"# will help improve performance." );
    INIT_PARAM_BOOL( 
	disable_fog, false ,
	"# If true, then fog will be turned off.  Some Linux drivers for the\n"
	"# ATI Rage128 seem to have a bug in their fog implementation which\n"
	"# makes the screen nearly pure white when racing; if you experience\n"
	"# this problem then set this variable to true." );
    INIT_PARAM_BOOL( 
	use_cva, true ,
	"# [EXPERT] If true, then compiled vertex arrays will be used when\n"
	"# drawing the terrain.  Whether or not this helps performance\n"
	"# is driver- and card-dependent." );
    INIT_PARAM_BOOL( 
	cva_hack, true ,
	"# Some card/driver combinations render the terrrain incorrectly\n"
	"# when using compiled vertex arrays.  This activates a hack \n"
	"# to work around that problem." );
    INIT_PARAM_INT( 
	course_detail_level, 75 ,
	"# [EXPERT] This controls how accurately the course terrain is \n"
	"# rendered. A high value results in greater accuracy at the cost of \n"
	"# performance, and vice versa.  This value can be decreased and \n"
	"# increased in 10% increments at runtime using the F9 and F10 keys.\n"
	"# To better see the effect, activate wireframe mode using the F11 \n"
	"# key (this is a toggle)." );
    INIT_PARAM_BOOL( 
	no_audio, false ,
	"# If true, then audio in the game is completely disabled." );
    INIT_PARAM_BOOL( 
	sound_enabled, true ,
	"# Use this to turn sound effects on and off." );
    INIT_PARAM_BOOL( 
	music_enabled, true ,
	"# Use this to turn music on and off." );
    INIT_PARAM_INT( 
	sound_volume, 64 ,
	"# This controls the sound volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	music_volume, 127 ,
	"# This controls the music volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	audio_freq_mode, 1 ,
	"# The controls the frequency of the audio.  Valid values are:\n"
	"# \n"
	"#   0: 11025 Hz\n"
	"#   1: 22050 Hz\n"
	"#   2: 44100 Hz" );
    INIT_PARAM_INT( 
	audio_format_mode, 1 ,
	"# This controls the number of bits per sample for the audio.\n"
	"# Valid values are:\n"
	"#\n"
	"#   0: 8 bits\n"
	"#   1: 16 bits" );
    INIT_PARAM_BOOL( 
	audio_stereo, true ,
	"# Audio will be played in stereo of true, and mono if false" );
    INIT_PARAM_INT( 
	audio_buffer_size, 2048 ,
	"# [EXPERT] Controls the size of the audio buffer.  \n"
	"# Increase the buffer size if you experience choppy audio\n" 
	"# (at the cost of greater audio latency)" );
    INIT_PARAM_BOOL( 
	track_marks, true ,
	"# If true, then the players will leave track marks in the snow." );
    INIT_PARAM_BOOL( 
	ui_snow, true ,
	"# If true, then the ui screens will have falling snow." );

    INIT_PARAM_BOOL( 
	write_diagnostic_log, false ,
	"# If true, then a file called diagnostic_log.txt will be generated\n" 
	"# which you should attach to any bug reports you make.\n"
	"# To generate the file, set this variable to \"true\", and\n"
	"# then run the game so that you reproduce the bug, if possible."
	);
	
    INIT_PARAM_BOOL( 
	always_save_event_race_data, false ,
	"# only for cheating purpose"
	);	
	
	INIT_PARAM_BOOL( 
	disable_collision_detection, false ,
	"# If true, collision detection with tree models is disabled"
	);
	
	INIT_PARAM_BOOL( 
	disable_videomode_autodetection, false, 
	"# Set this to true disable the autodetection\n"
	"# for available video modes." );
		
	INIT_PARAM_STRING( 
	ui_language, "en_GB" ,
	"# set the language for the ui"
	);
	
}


/* 
 * Create the set/get functions for parameters
 */

FN_PARAM_STRING( data_dir )
FN_PARAM_BOOL( draw_tux_shadow )
FN_PARAM_BOOL( draw_particles )
FN_PARAM_INT( tux_sphere_divisions )
FN_PARAM_INT( tux_shadow_sphere_divisions )
FN_PARAM_BOOL( nice_fog )
FN_PARAM_BOOL( use_sphere_display_list )
FN_PARAM_BOOL( display_fps )
FN_PARAM_BOOL( display_course_percentage )
FN_PARAM_INT( x_resolution )
FN_PARAM_INT( y_resolution )
FN_PARAM_BOOL( x_resolution_half_width )
FN_PARAM_BOOL( capture_mouse )
FN_PARAM_BOOL( do_intro_animation )
FN_PARAM_INT( mipmap_type )
FN_PARAM_BOOL( fullscreen )
FN_PARAM_INT( bpp_mode )
FN_PARAM_BOOL( force_window_position )
FN_PARAM_INT( ode_solver )
FN_PARAM_STRING( quit_key )

FN_PARAM_INT( turn_left_key )
FN_PARAM_INT( turn_right_key )
FN_PARAM_INT( trick_modifier_key )
FN_PARAM_INT( brake_key )
FN_PARAM_INT( paddle_key )
FN_PARAM_STRING( above_view_key )
FN_PARAM_STRING( behind_view_key )
FN_PARAM_STRING( follow_view_key )
FN_PARAM_INT( view_mode )
FN_PARAM_STRING( screenshot_key )
FN_PARAM_STRING( pause_key )
FN_PARAM_INT( reset_key )
FN_PARAM_INT( jump_key )
FN_PARAM_INT( joystick_jump_button )
FN_PARAM_INT( joystick_brake_button )
FN_PARAM_INT( joystick_paddle_button )
FN_PARAM_INT( joystick_trick_button )
FN_PARAM_INT( joystick_continue_button )
FN_PARAM_INT( joystick_x_axis )
FN_PARAM_INT( joystick_y_axis )
FN_PARAM_BOOL ( disable_joystick )
FN_PARAM_INT( fov )
FN_PARAM_STRING( debug )
FN_PARAM_INT( warning_level )
FN_PARAM_INT( forward_clip_distance )
FN_PARAM_INT( backward_clip_distance )
FN_PARAM_INT( tree_detail_distance )
FN_PARAM_INT( course_detail_level )
FN_PARAM_BOOL( terrain_blending )
FN_PARAM_BOOL( perfect_terrain_blending )
FN_PARAM_BOOL( terrain_envmap )
FN_PARAM_BOOL( disable_fog )
FN_PARAM_BOOL( use_cva )
FN_PARAM_BOOL( cva_hack )
FN_PARAM_BOOL( track_marks )
FN_PARAM_BOOL( ui_snow )

FN_PARAM_BOOL( no_audio )
FN_PARAM_BOOL( sound_enabled )
FN_PARAM_BOOL( music_enabled )
FN_PARAM_INT( sound_volume )
FN_PARAM_INT( music_volume )
FN_PARAM_INT( audio_freq_mode )
FN_PARAM_INT( audio_format_mode )
FN_PARAM_BOOL( audio_stereo )
FN_PARAM_INT( audio_buffer_size )
FN_PARAM_BOOL( write_diagnostic_log )

FN_PARAM_BOOL( stencil_buffer )
FN_PARAM_BOOL( enable_fsaa )
FN_PARAM_INT( multisamples )

FN_PARAM_BOOL( always_save_event_race_data )
FN_PARAM_BOOL( disable_collision_detection )
FN_PARAM_BOOL( disable_videomode_autodetection )

FN_PARAM_STRING( ui_language )


/*
 * Functions to read and write the configuration file
 */

int get_old_config_file_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( OLD_CONFIG_FILE ) +1 > len ) {
	return 1;
    }
    strcpy( buff, OLD_CONFIG_FILE );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( OLD_CONFIG_FILE ) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, OLD_CONFIG_FILE );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_dir_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( CONFIG_DIR ) +1 > len ) {
	return 1;
    }
    strcpy( buff, CONFIG_DIR );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( CONFIG_DIR) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, CONFIG_DIR );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_file_name( char *buff, unsigned int len )
{
    if (get_config_dir_name( buff, len ) != 0) {
	return 1;
    }
    if ( strlen( buff ) + strlen( CONFIG_FILE ) +2 > len ) {
	return 1;
    }

#if defined( WIN32 ) 
    strcat( buff, "\\" );
#else
    strcat( buff, "/" );
#endif /* defined( WIN32 ) */

    strcat( buff, CONFIG_FILE);
    return 0;
}

void clear_config_cache()
{
    struct param *parm;
    unsigned int i;

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	parm->loaded = false;
    }
}

void read_config_file(std::string& file)
{
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];

    clear_config_cache();

	if( !file.empty()){
		if ( Tcl_EvalFile( tclInterp, FUCKTCL file.c_str() ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", file.c_str(),
			      Tcl_GetStringResult( tclInterp ) );
	    }	
		sp_config_file = file.c_str();	
		return;
	}else{
		sp_config_file = NULL;
	}
	
    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
		return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
		return;
    }

	

    if ( dir_exists( config_dir ) ) {
	if ( file_exists( config_file ) ) {
	    /* File exists -- let's try to evaluate it. */
	    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", config_file,
			      Tcl_GetStringResult( tclInterp ) );
	    }
	}
	return;
    }

    /* File does not exist -- look for old version */
    if ( get_old_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( !file_exists( config_file ) ) {
	return;
    }
    /* Old file exists -- let's try to evaluate it. */
    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
	handle_error( 1, "error evalating deprecated %s: %s", config_file,
		      Tcl_GetStringResult( tclInterp ) );
    } else {
	/* Remove old file and save info in new file location */
	remove(config_file);
	write_config_file();
    }
}

void write_config_file()
{
    FILE *config_stream;
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];
    struct param *parm;
    unsigned int i;
	
	if(sp_config_file==NULL){

    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
	return;
    }

    if ( !dir_exists( config_dir ) ) {

#if defined(WIN32) && !defined(__CYGWIN__)
	if (mkdir( config_dir ) != 0) {
	    return;
	}
#else
	if (mkdir( config_dir, 0775) != 0) {
	    return;
	}
#endif

    }

    config_stream = fopen( config_file, "w" );
	if ( config_stream == NULL ) {
	print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       config_file, strerror(errno) );
	return;
    }
	
	}else{
		std::cout << "Writing to custom config file: "
				  << sp_config_file << std::endl;
		config_stream = fopen( sp_config_file, "w" );
		if ( config_stream == NULL ) {
			print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       sp_config_file, strerror(errno) );
			return;
    	}
	}
	
    fprintf( config_stream, 
	     "# PP Racer " VERSION " configuration file\n"
	     "#\n"
	);

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	if ( parm->comment != NULL ) {
	    fprintf( config_stream, "\n# %s\n#\n%s\n#\n", 
		     parm->name, parm->comment );
	}
	switch ( parm->type ) {
	case PARAM_STRING:
	    fetch_param_string( parm );
	    fprintf( config_stream, "set %s \"%s\"\n",
		     parm->name, parm->val.string_val );
	    break;
	case PARAM_CHAR:
	    fetch_param_char( parm );
	    fprintf( config_stream, "set %s %c\n",
		     parm->name, parm->val.char_val );
	    break;
	case PARAM_INT:
	    fetch_param_int( parm );
	    fprintf( config_stream, "set %s %d\n",
		     parm->name, parm->val.int_val );
	    break;
	case PARAM_BOOL:
	    fetch_param_bool( parm );
	    fprintf( config_stream, "set %s %s\n",
		     parm->name, parm->val.bool_val ? "true" : "false" );
	    break;
	default:
	    code_not_reached();
	}
    }

    if ( fclose( config_stream ) != 0 ) {
	perror( "fclose" );
    }
}

/*
 * Tcl callback to allow reading of game configuration variables from Tcl.
 */
static int get_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int num_params;
    struct param *parm;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Get value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	fetch_param_string( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( parm->val.string_val, -1 ) );
	break;

    case PARAM_CHAR:
	fetch_param_char( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( &parm->val.char_val, 1 ) );
	break;

    case PARAM_INT:
	fetch_param_int( parm );
	Tcl_SetObjResult( ip, Tcl_NewIntObj( parm->val.int_val ) );
	break;

    case PARAM_BOOL:
	fetch_param_bool( parm );
	Tcl_SetObjResult( ip, Tcl_NewBooleanObj( parm->val.bool_val ) );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

/* 
 * Tcl callback to allow setting of game configuration variables from Tcl.
 */
static int set_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int tmp_int;
    int num_params;
    struct param *parm;

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name> <value>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Set value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	set_param_string( parm, argv[2] ); 
	break;

    case PARAM_CHAR:
	if ( strlen( argv[2] ) > 1 ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a single character", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_char( parm, argv[2][0] );
	break;

    case PARAM_INT:
	if ( Tcl_GetInt( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be an integer", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_int( parm, tmp_int );
	break;

    case PARAM_BOOL:
	if ( Tcl_GetBoolean( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a boolean", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	check_assertion( tmp_int == 0 || tmp_int == 1, 
			 "invalid boolean value" );
	set_param_bool( parm, (bool) tmp_int );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

void register_game_config_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_get_param", get_param_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_set_param", set_param_cb,   0,0);
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         /* 
 * Extreme Tux Racer (Not Copyrighted)
 * 
 * based on Planet Penguin Racer, Open Racer, and Tux Racer: 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "credits.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/font.h"


#define CREDITS_MAX_Y -140
#define CREDITS_MIN_Y 64

typedef struct {
    pp::Font *font;
	char *binding;
    char *text;
} credit_line_t;

static credit_line_t credit_lines[] = 
{
	{ NULL, "credits_h1", "Extreme Tux Racer" },
    { NULL, "credits_text", "Version 0.4" },
    { NULL, "credits_text", "on the web at: www.extremetuxracer.com" },
    { NULL, "credits_text", "" },	
	{ NULL, "credits_text", "Extreme Tux Racer is in the process of being" },	
	{ NULL, "credits_text", "completely rewritten. All versions after 0.35," },	
	{ NULL, "credits_text", "including this one, won't have major code changes" },	
	{ NULL, "credits_text", "because the programmers are busy making a new, improved game." },		
	{ NULL, "credits_text", "" },		
	{ NULL, "credits_text", "Extreme Tux Racer is based on:" },
	{ NULL, "credits_text", "PlanetPenguin Racer," },
	{ NULL, "credits_text", "Open Racer," },
	{ NULL, "credits_text", "and Tux Racer" },
	    { NULL, "credits_text", "" },
		{ NULL, "credits_text", "Some courses may have been modified from their" },
		{ NULL, "credits_text", "original versions to fit the requirements of the game." },		
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Development" },
    { NULL, "credits_text", "Volker StrÃ¶bel" },
    { NULL, "credits_text", "Steven Bell" },
    { NULL, "credits_text", "Hamish Morrison" },
    { NULL, "credits_text", "Andreas Tarandi" },
	{ NULL, "credits_text_small", "from Sunspire Studios:" },
    { NULL, "credits_text", "Patrick \"Pog\" Gilhuly" },
    { NULL, "credits_text", "Eric \"Monster\" Hall" },
    { NULL, "credits_text", "Rick Knowles" },
    { NULL, "credits_text", "Vincent Ma" },
    { NULL, "credits_text", "Jasmin Patry" },
    { NULL, "credits_text", "Mark Riddell" },
		{ NULL, "credits_text", "" },
		{ NULL, "credits_h2", "Non-Rewrite Version Organization" },
		{ NULL, "credits_text", "Christian Picon" },	
		{ NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Translators" },
	{ NULL, "credits_text", "Nicosmos (French)" },
	{ NULL, "credits_text", "teksturi (Finnish)" },
	{ NULL, "credits_text", "arith, spacedwarv, and JoyFM (German)" },
	{ NULL, "credits_text", "spectrum (Italian)" },
	{ NULL, "credits_text", "Andreas Tarandi and pingvin (Swedish)" },
    	{ NULL, "credits_text", "Asciimonster (Dutch)" },
    	{ NULL, "credits_text", "woody (Polish)" },
    	{ NULL, "credits_text", "ttsmj (Slovak)" },		
    	{ NULL, "credits_text_small", "other (incomplete) translations are from the PPRacer Project" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Graphics" },
    { NULL, "credits_text", "Nicosmos (Logo, Hud Graphics)" },
    { NULL, "credits_text", "Christian Picon (Objects, Skyboxes)" },
    { NULL, "credits_text", "Reinhard Niehoff (Trees)" },	
    { NULL, "credits_text", "Daniel Poeira and Christian Picon (Font)" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Music" },
    { NULL, "credits_text", "Joseph Toscano" },
	    { NULL, "credits_text_small", "'Race 1'" },
		{ NULL, "credits_text_small", "'Menu'" },
	    { NULL, "credits_text_small", "'Options'" },	
	    { NULL, "credits_text_small", "'Won Race'" },		
    { NULL, "credits_text", "Christian Picon" },
	    { NULL, "credits_text_small", "'Credits Ballad'" },	
		    { NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Misc. PPRacer Contributors" },
	{ NULL, "credits_text", "Peter Reichel" },
	{ NULL, "credits_text", "Teemu Vesala" },
	{ NULL, "credits_text", "Theo Snelleman" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Thanks To" },
    { NULL, "credits_text_small", "(for Exteme Tux Racer)" },
    { NULL, "credits_text", "Ranger (for hosting the website)" },
    { NULL, "credits_text", "Slythfox (for making the website)" },
    { NULL, "credits_text", "The Course Creators" },	
    { NULL, "credits_text", "The OpenRacer project" },
    { NULL, "credits_text", "Reinhard Niehoff (for helping with code modification)" },
    { NULL, "credits_text", "All the people on the forum." },
    { NULL, "credits_text", "Larry Ewing (creator of Tux)" },
    { NULL, "credits_text_small", "(for Tux Racer)" },
    { NULL, "credits_text", "Thatcher Ulrich" },
    { NULL, "credits_text", "Steve Baker" },
    { NULL, "credits_text", "Ingo Ruhnke" },
    { NULL, "credits_text", "James Barnard" },
    { NULL, "credits_text", "Alan Levy" },
    { NULL, "credits_text", "" },
  { NULL, "credits_text_small", "Tux Racer is a trademark of Jasmin F. Patry" },
	{ NULL, "credits_text_small", "Extreme Tux Racer is licenced under the GPL." },
	{ NULL, "credits_text_small", "We grant use of the name 'Extreme Tux Racer' to any forks or continuations. " },
	{ NULL, "credits_text_small", "" },
	{ NULL, "credits_text_small", "PlanetPenguin Racer is Copyright Â© 2005 Volker Stroebel" },
  { NULL, "credits_text_small", "Tux Racer and the Tux Racer Name are Copyright Â© 2001 Jasmin F. Patry" },
};

Credits::Credits()
 : m_yOffset(0.0)
{
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
		credit_lines[i].font = pp::Font::get(credit_lines[i].binding);		
	}
	
  play_music( "credits_screen" );
}


Credits::~Credits()
{	
}

void
Credits::loop(float timeStep)
{
	int width, height;

  width = getparam_x_resolution();
  height = getparam_y_resolution();

  update_audio();

  clear_rendering_context();

  set_gl_options( GUI );

  UIMgr.setupDisplay();

  drawText( timeStep );

	drawSnow(timeStep);

	theme.drawMenuDecorations();

  UIMgr.draw();

  reshape( width, height );

  winsys_swap_buffers();
}


void
Credits::drawText( float timeStep )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    float y;

    m_yOffset += timeStep * 30;
    y = CREDITS_MIN_Y + m_yOffset;

	//loop through all credit lines
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
	    credit_line_t line = credit_lines[i];

		//get the font and sizes for the binding
		//pp::Font *font = pp::Font::get(line.binding);
		float width = line.font->advance(line.text);
		float desc = line.font->descender();
		float asc = line.font->ascender();
		
		//draw the line on the screen
		line.font->draw(line.text, w/2 - width/2, y);

		//calculate the y value for the next line
		y-=asc-desc;
	}

	//if the last string reaches the top, reset the y offset
    if ( y > h+CREDITS_MAX_Y ) {
		m_yOffset = 0;
    }

    // Draw strips at the top and bottom to clip out text 
    glDisable( GL_TEXTURE_2D );

    glColor4dv( (double*)&theme.background );

    glRectf( 0, 0, w, CREDITS_MIN_Y );

    glBegin( GL_QUADS );
    {
	glVertex2f( 0, CREDITS_MIN_Y );
	glVertex2f( w, CREDITS_MIN_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( w, CREDITS_MIN_Y + 30 );
	glVertex2f( 0, CREDITS_MIN_Y + 30 );
    }
    glEnd();

    glColor4dv( (double*)&theme.background );

    glRectf( 0, h+CREDITS_MAX_Y, w, h );

    glBegin( GL_QUADS );
    {
	glVertex2f( w, h+CREDITS_MAX_Y );
	glVertex2f( 0, h+CREDITS_MAX_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( 0, h+CREDITS_MAX_Y - 30 );
	glVertex2f( w, h+CREDITS_MAX_Y - 30 );
    }
    glEnd();

    glColor4f( 1, 1, 1, 1 );

    glEnable( GL_TEXTURE_2D );
}

bool
Credits::mouseButtonReleaseEvent(int button, int x, int y)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}

bool
Credits::keyReleaseEvent(SDLKey key)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 è‡   „Àt€»  uöƒ  tÆ$ë
Æ$ ëÆ$fƒ»f   t‹Ì‹Ó‹ƒh  ÿ“d  €<$ t=¡(I ;X(u¡(I èñ  ë'€<$u	‹Ãè&  ë€<$u²‹Ãèmèÿÿë‹Ãè\  Z[ÃSVWQ‹Ø€»  u+Æ$ ‹Ãè"ãÿÿ‹ğN…ö|F3ÿ‹×‹Ãè[ãÿÿèÎÿÿÿ„Àt"GNuêÆ$fƒ»n   t‹Ì‹Ó‹ƒp  ÿ“l  Š$Z_^[ÃU‹ìƒÄøSVW‰Eü3ÀUhh-B dÿ0d‰ ÆEû ‹Eüèzÿÿÿ„Àt&ÆEû‹Eüfƒ¸f   tMû‹]ü‹Uü‹ƒh  ÿ“d  ŠEû,rşÈtë‹Eü3Ò‰(  ë‹Eüè†  3ÀZYYd‰ë"éşÿ‹Eü3Ò‰(  ‹Uü¡(I èÓ  èşÿ_^[YY]Ã@ 3Òè½İÿÿÃS‹Ø²‹Ãè°İÿÿ‹Ãè­ÿÿ[Ã@ €¸   u'€x7 t€x8 u¹.ğ  ²¸¤)A èr^şÿè=şÿèÜîÿÿÃ@ U‹ìƒÄäS‰Eü»(I è|ÿÿ‹Eü€x7 u!‹Eü€x8 t‹Eüö€  u‹Eü€¸  u¹2ğ  ²¸¤)A è^şÿèàşÿè›0şÿ…Àtj j jèŒ0şÿPèŞ2şÿè‘2şÿ‹Eü€ˆ  èj0şÿ‰Eä¡|I ‰Eğ¡,I ‹@L‰Eì¡,I ‹Uü‰PL¡,I f‹@(f‰Eê3Ò¡,I èˆ  3Àè‘¿ÿÿ‰Eô3ÒUhÒ/B dÿ2d‰"‹EüèØşÿÿ3ÒUh{/B dÿ2d‰"j j h °  ‹Eüè•RÿÿPèK2şÿ‹Eü3Ò‰(  ‹èY  ‹€x| t‹EüÇ€(     ë‹Eüƒ¸(   t‹EüèÆıÿÿ‹Eü‹€(  …ÀtÁ‰Eøj j h°  ‹Eüè1RÿÿPèç1şÿ‹Eüè#Rÿÿ‹Øèx/şÿ;Øt3À‰Eä3ÀZYYd‰h‚/B ‹EüèşÿÿÃéä şÿëğ3ÀZYYd‰hÙ/B f‹Uê¡,I è  ‹EôèK¿ÿÿ¡,I ‹Uì‰PLƒ}ä t	‹EäPè1şÿ‹Eğ£|I ‹Eü€   ÷Ãé şÿë¶‹Eø[‹å]Ã@ S‹Øj j h!°  ‹ÃèQÿÿPèÇ0şÿ[ÃU‹ìj SVW3ÀUhO0B dÿ0d‰ Eü‹UƒÂ¹    è3şÿ‹Uü‹E‹ÿQ,»   3ÀZYYd‰hV0B Eüè²şÿÃé şÿëğ‹Ã_^[Y]Â U‹ìQSV„ÒtƒÄğè¾üıÿˆUÿ‹Ø3Ò‹Ãè8Ñşÿ‹ÃèM  ²¸Ğ¦@ èúıÿ‰C ²¸\¤@ èúıÿ‰C,²¸\¤@ èúıÿ‰C0j èQ.şÿ‹ğ‹C Ph 0B j Vè>+şÿjZVèn+şÿ‰C$Vj è0şÿ€}ÿ t
d    ƒÄ‹Ã^[Y]ÃSV‹Ú‹ğ‹F0èRúıÿ‹F,èJúıÿ‹F èBúıÿ‹Æè  3Ò‹ÆèÖĞşÿ„Ût‹Æè;üıÿ‹Æ^[Ã‹Àjèu.şÿÃj èm.şÿÃ‹@,è„‚şÿÃ@ ‹@,‹@ÃS‹Ø‹C@;CHt‰CHfƒ{Z t‹Ó‹C\ÿSX‹C<;CDt‰CDfƒ{R t‹Ó‹CTÿSP[Ã@ ‹@,è˜şÿÃ@ S‹Ø‹C,èUƒşÿ‹C,ƒx u¡(I ƒx` t¡(I ‹@`èØVÿÿ[Ã‹ÀSVWU‹èh   j è~.şÿ‰E8»ìÿÿÿ¾äI ƒûï|ƒûô‹=I ë3ÿ‹PWèT.şÿ‹È‹Ó‹Åè­   CƒÆƒûÿuÏ]_^[Ã@ SVW‹ø‹_4…Ût)‹Cƒøô~…À~	‹CPèß+şÿ‹3º   ‹Ãèôıÿ‹Ş…Ûu×h   j è÷-şÿ‹W8;ÂtRè²+şÿ_^[Ã‹ÀSVW‹ø‹_43öë‹ó‹…Ût;Suó…Ût$‹CPè†+şÿ…öu‹‰G4ë‹‰º   ‹ÃèŸóıÿ_^[Ã@ SVW‹ù‹ò‹Ø¸   èmóıÿ‹S4‰‰p‰x‰C4_^[Ã@ ‹@0èÜ€şÿÃ@ ‹@0‹@Ã3Éƒúÿt‹H4ë‹	…Ét;Quõ…Éu‹H8ë‹I‹ÁÃ‹ÀSVWUƒÄø‹ò‹Øf;s(tsf‰s(f…öuZTè¸+şÿÿt$ÿt$èë.şÿ‹ø…ÿtAj Wè…,şÿ‹èèv%şÿ;èu.‹D$P‹D$Ph„   Wè­-şÿ‹Øfº ‹Ãè¸.şÿPWj Wè–-şÿë¿Ö‹ÃèZÿÿÿPè¬-şÿYZ]_^[ÃSVW‹ù‹ò‹Ø…öu…ÿuh   j è,şÿ‰C8ë"‰{8ëƒşÿt‹Ö‹Ãè“şÿÿ…ÿt‹Ï‹Ö‹ÃèÌşÿÿ_^[ÃSV‹ò‹Ø…Ûtƒ{T u‹[ ë‹Æ‹STèu şÿ^[Ã…Ûuä‹ÆèÓÿıÿ^[Ãë‹@ …Àt€x] tó…Àtö@t3ÀÃU‹ìƒ=(I  t
¡(I è~  ]Â ‹ÀU‹ìƒÄøS»(I hè  èN$şÿPè¨-şÿƒ; të‹ƒx@ tãEøPèj*şÿEøèîÿÿ…ÀuÎ‹è£  ëÅ[YY]Â ‹ÀU‹ìSVW‹u‹]V‹EPS¡TI PèÇ(şÿ‹ø…Û|ƒ=(I  t‹Ö¡(I èt  ‹Ç_^[]Â @ Q¡(I €x uGƒ=TI  uèÎ#şÿPj ¸¸4B Pjè¦,şÿ£TI ƒ=XI  uTj j hh4B hè  j èò"şÿ£XI ZÃ@ ƒ=TI  t¡TI Pè¬,şÿ3À£TI ƒ=XI  tj ¡XI Pèo%şÿ3À£XI Ã@ ƒÄøÇ$   j D$Pj jHèJ,şÿ…Àt‹D$÷ØÀ÷ØYZÃ3ÀYZÃƒÄøÇ$   ƒà‰D$j D$Pj jIè,şÿYZÃSVW‹ú‹ğè ÿÿÿ‹Ø„Ût3ÀèÃÿÿÿWVèè+şÿ„Ût°è±ÿÿÿ_^[ÃTApplication    U‹ìÄüşÿÿSV„ÒtƒÄğèõöıÿˆUÿ‹Ø3Ò‹ÃèoËşÿ²¸\¤@ èÛôıÿ‰Cp²¸\¤@ èÌôıÿ‰ƒ€   3À‰C@3À‰C`ÇC<  €ÇCTô  ÇCX2   ÇC\Ä	  ÆCd ÆC}²¸”™A è£ÿÿ‹ğ‰sxh|7B ¡I Pè³)şÿ‹Ğ‹Æèv¦ÿÿ‹Cx‰XÇ@<JB h   …ÿşÿÿP¡I Pè^"şÿ…ÿşÿÿP…ÿşÿÿPè«)şÿ…ÿşÿÿ²\èJ=şÿ…ÀtP…ÿşÿÿè ;şÿ…ÿşÿÿ².è=şÿ…ÀtÆ  …ÿşÿÿ@PèC&şÿCl•ÿşÿÿ¹   èşıÿ€=4I  u‹Ãè°   ÆC9ÆC:€}ÿ t
d    ƒÄ‹Ã^[‹å]ÃMAINICON    SV‹Ú‹ğÆF} ‹Æè·  3Ò‹ÆèZÊşÿVh ;B ‹Æè  ‹F …Àt'€~~ t!€=ÄI  tj jh€   Pè`)şÿ‹F Pèg&şÿ‹F$…Àtè¼ÿÿèîºÿÿ‹†€   è_óıÿ„Ût‹Æèhõıÿ‹Æ^[Ã@ SƒÄØ‹Ø€{~ …#  Sh¤;B è5»ÿÿ‰C$T¡TI P¡I Pè°&şÿ…Àu/¡I £@I h0I èˆ(şÿf…Àu¹,ğ  ²¸`)A èâSşÿè­øıÿj è>'şÿÑøyƒĞ Pjè/'şÿÑøyƒĞ Pj j j j ¡I Pj ‹Clè“şıÿ‹Ğ¹  Ê”¡TI è:*şÿ‰C Clè?ûıÿÆC~º	   ‹C èıÿÿ‹C$Pjü‹C PèË(şÿ€=ÄI  t‹Ãè  Pjh€   ‹C Pè2(şÿj ‹C PèŸ&şÿ‹Øj h0ğ  Sè%şÿj h ğ  Sèû$şÿ€=ÄI  tj hğ  Sèå$şÿƒÄ([ÃS;P(u3É‰H(;P,u3É‰H,‹,I ;Q<u‹,I 3Û‰Y<‹,I ;Q@u‹,I 3Û‰Y@‹,I ;QLu‹,I 3Û‰YL;P@u3Ò‰P@¡,I è­÷ÿÿ[Ã@ U‹ìSVW‹]¾   jSèò%şÿ‹(I ;B u;jìSèç%şÿ¨t(¡(I ‹x(…ÿt‹Çè„Gÿÿ;Øt¡(I ‹@p‹Óè1yşÿë‹E‰3ö‹Æ_^[]Â @ SVQ‹Ø¡(I ƒx  t|ƒ{t us‹C ‰$‹ÄPh´9B è_$şÿ‹Cpƒx tWj‹D$Pèb%şÿ‰$jì‹D$Pè[%şÿ¨tÇ$şÿÿÿ‹Cp‹pNƒş |%jj j j j ‹D$P‹Ö‹Cpè;yşÿPè'şÿNƒşÿuÛÿCtZ^[ÃSV‹ğ¡(I ƒx  t?ÿNtƒ~t u6‹Fp‹XKƒû |"jj j j j jÿ‹Ó‹FpèîxşÿPèĞ&şÿKƒûÿuŞ‹Fpèfxşÿ^[Ã@ S3Û‹ƒêu€x9 tè›Kşÿ‹Ã[Ã@ U‹ì‹E‹@ø‹@P‹E‹@ø‹@P‹E‹@ø‹ P‹E‹@ü‹@ PèÑ"şÿ‹U‹Rø‰B]Ã‹ÀU‹ìƒÄÀSEÀP‹E‹@ü‹@ Pè"şÿ‹Ø‹E‹@üè‘  Pj j SèÚ"şÿEÀP‹E‹@ü‹@ Pè÷"şÿ[‹å]Ã‹ÀU‹ìƒÄøSVW‰Uø‰Eü3ÒUhô@B dÿ2d‰"‹Eø3Ò‰P‹Eü‹€€   ‹XK…Û|2C3ö‹Eü‹€€   ‹Öèçwşÿ‹È‹Uø‹Aÿ„Àt3ÀZYYd‰é  FKuÑ‹Uø‹Eüèéşÿÿ‹]ø‹‹Ã=  _„½   ƒø-„X  ƒè„w  ƒè„õ  ƒè„  H„Ñ   é†  ƒè„Û   H„j  ƒè„a  ƒè„$  é_  =°  4„¸  Îşÿÿƒè‚  -Ç®  „8  H„Y  ƒè„4  é$  -°  „h  ƒè„›  H„«  ƒè„Õ  éû  ‹Eø‹@%ğÿ  - ğ  t	-   të‹Eüè!  éÚ  ‹Eüè\  éÍ  UèùıÿÿYéÁ  ‹Eü‹@(…À„³  è¨îÿÿé©  ƒ=I  Œœ  ƒ=HI  „  ÿHI é„  ‹Eü‹@ Pè²"şÿ…ÀtUèÜıÿÿYéh  Uè”ıÿÿYé\  ‹EøÇ '   UèıÿÿYéG  ‹Eüèe  ‹Uø‰Bé4  j j h°  ‹Eü‹@ Pèù"şÿUèKıÿÿYé  Uè?ıÿÿY‹Eø‹@÷ØÀ÷Ø‹UüˆB}‹Eøƒx t"‹Eüè«üÿÿj j h °  ‹Eü‹@ Pè®"şÿéÏ  ‹Eüèùûÿÿj j h°  ‹Eü‹@ PèŒ"şÿé­  ‹Eøƒx t1‹Eüè^üÿÿ‹Eü‹€„   …Àtè°ÿÿ‹Eü3Ò‰„   Uè«üÿÿYés  UèŸüÿÿY‹Eüƒ¸„    u‹Eü‹@ è§¯ÿÿ‹Uü‰‚„   ‹Eüèvûÿÿé?  ‹uø‹vV‹Eø‹@P‹EøÃ ¼  S‹EøVèh"şÿ‹Uø‰Bé  ‹Eøƒx „  èkşÿéü  ‹Eü‹X(…Û„î  ƒ»$   „á  ‹ÃèhBÿÿPè&!şÿ…À„Ì  ‹Uø‹Eü‹@(‹€$  èvãşÿ„À„°  ‹EøÇ@   é¡  ‹Eü‹@(…À„“  ‹ğ‹ÆèBÿÿ…À„‚  ‹Æè	BÿÿPèÇ şÿ…À„m  ‹ÆèôAÿÿPèº şÿ…À„X  ÆxI  è†şÿ‹Ø‹ÆèÑAÿÿPè·!şÿ‹Eø‹@P‹Mø‹Iº  ‹ÆèÿÿSè˜!şÿÆxI ‹EøÇ@   é  ‹Eüfƒ¸¾    „÷   ‹Mü‹Uü‹À   ÿ‘¼   éà   ‹Eüfƒ¸¶    „Ï   ‹Mü‹Uü‹¸   ÿ‘´   é¸   ‹Eü‹@ Pèæşÿ…À…¤   èÙşÿ‹Uü;B …“   3Àè.¯ÿÿ…À„„   Pèø şÿë|‹Mø‹I‹Uøf‹R‹Eüèe  ëe‹Eøƒx u‹Eø‹@ÿpÿ0‹EüèË  ëG‹Eø‹@ÿpÿ0‹Eüè  ë2‹Eøƒxu‹Eü‹€ˆ   ‹Uø‰Bë‹Eø‹@‹Uü‰‚ˆ   ëUè3úÿÿY3ÀZYYd‰ëéwíıÿ‹Uü‹EüèT  èƒğıÿ_^[YY]Ã‹@xè°™ÿÿ…Àuh   j èDşÿÃ@ S‹Ø‹C Pèìşÿ…Àu5‹ÃèÙøÿÿ‹C Pè  şÿº   ‹C è‹ôÿÿfƒ»Î    t‹Ó‹ƒĞ   ÿ“Ì   [ÃSV‹Ø‹C Pè£şÿ…ÀtN‹C Pè¾şÿº	   ‹C èIôÿÿ‹Ãè
ùÿÿ¡,I ‹p<…öt‹ÆèÃ?ÿÿPè©şÿfƒ»Ö    t‹Ó‹ƒØ   ÿ“Ô   ^[Ã‹ÀSV‹Ø‹C …Àt+Pècşÿ‹ğ…öt;s tVèJşÿ…ÀtVè8şÿ…ÀtVè^şÿ^[Ã@ SVÄ ÿÿÿ‹ò‹Ø€{~ t h   D$P‹C Pè«şÿ‹È‹Ô‹Æèœòıÿë
‹Æ‹SlèDòıÿÄ   ^[Ã@ SV‹ò‹Ø€{~ t‹ÆèÉôıÿP‹C PèKşÿ^[ÃCl‹ÖèÊñıÿ^[Ã@ SVW‹ú‹Ø3À‹³ˆ   …ötWVèvşÿ÷ØÀ÷Ø_^[ÃSVW‹ú‹Ø3À‹s(…öt:€¾  u1‹,I ƒz@ t%‹,I ‹R@€º  uW‹†0  Pèşÿ÷ØÀ÷Ø_^[Ã‹ÀSVWU‹ê‹ğ3Û‹}ÿ   |Gÿ  ?èºşÿ…Àu6‹} ‹F(…Àt;¸0  uè;>ÿÿ‹ø‹EP‹EP‹E ¼  PWèŞşÿ…Àt³‹Ã]_^[ÃSVWU‹ú‹è3Û‹u`…öt‹×‹Æ‹ÿ‘„   „Àt‹ÅèÄ  ‹Ã]_^[ÃSVƒÄà‹ğ3Ûjj j j D$Pèşÿ…À„…   ³ƒ|$txÆ$ fƒ¾–    t‹ÌT$‹†˜   ÿ–”   T$‹Æèyÿÿÿ„ÀuM€<$ uGT$‹Æè°şÿÿ„Àu8T$‹Æèñşÿÿ„Àu)T$‹Æènşÿÿ„ÀuD$PèàşÿD$PèşÿëÆF|‹ÃƒÄ ^[ÃS‹Ø‹ÃèFÿÿÿ„Àuõ[ÃS‹Ø‹Ãè6ÿÿÿ„Àu‹Ãèo  [ÃU‹ìS‹Ø€{~ u‹C …Àt@URj h °  Pè¤şÿë-‹ƒ€   èoşÿ¸   è¡áıÿ‹U‰‹U‰P‹Ğ‹ƒ€   èynşÿ[]Â U‹ìSVW‹Ø€{~ u‹C …Àt]URjh °  PèJşÿëJ‹ƒ€   ‹xO…ÿ|<G3ö‹Ö‹ƒ€   èÊnşÿ‹;Uu!‹U;Puº   è=áıÿ‹Ö‹ƒ€   èHnşÿëFOuÇ_^[]Â @ ƒ= I  tÿ I ÃU‹ìƒÄøSVW‰Mü‹ò‹Ø‹ÆÿPô‰Eø‹Eü‹Uø‰3ÀUh”EB dÿ0d‰ ‹Ë3Ò‹Eø‹0ÿV$3ÀZYYd‰ëé×èıÿ‹Eü3Ò‰‹Eøè°åıÿè‹ëıÿèÚëıÿƒ{( u‹EøºäA èÛåıÿ„Àt‹uø‹Æè;ÿÿ‰s(_^[YY]Ã@ U‹ìQ‰Eü‹EüÆ@3ÒUhFFB dÿ2d‰"¸ÀíA èv#şÿ‹Eü‹@(…Àt!‹Uü€z: t²è8Åÿÿ‹Eüè,şÿÿ‹Eü€x| tï3ÀZYYd‰hMFB ‹EüÆ@ ÃéêıÿëñY]Ãj èuşÿÃSV‹ò‹Øèyşÿ…Àtj j jèjşÿPè¼şÿèCşÿºXc@ èåıÿ„ÀtEèCşÿº¨c@ èåıÿ„ÀuCfƒ»    tèëBşÿ‹È‹Ö‹ƒ   ÿ“Œ   ë"èÔBşÿ‹Ğ‹Ãè‹   ^[ÃèãBşÿPè½BşÿZèCşÿ^[ÃU‹ìƒÄôSVW‹ù‹ò‹ØèÜşÿ‰Eø3Àè:§ÿÿ‰Eô3ÀUhEGB dÿ0d‰ ·EPWV‹C Pèqşÿ‰Eü3ÀZYYd‰hLGB ‹Eôèµ§ÿÿ‹EøPèşÿÃééıÿëç‹Eü_^[‹å]Â U‹ìj j SVW‹ò‹Ø3ÀUhÎGB dÿ0d‰ jUü‹Ãè”úÿÿ‹Eüè°ïıÿP‹VEøè íıÿEøºäGB èÛíıÿ‹Eøèïıÿ‹Ğ‹ÃYè1ÿÿÿ3ÀZYYd‰hÕGB Eøº   èSìıÿÃé‘èıÿëë_^[YY]Ãÿÿÿÿ   .   SVWƒÄø‰$‹ú‹ğ3ÛÆD$fƒ¾    tD$P‹L$‹×‹†    ÿ–œ   ‹Ø€|$ tSƒ~0 t13Û‹F(…Àtè>9ÿÿ‹Ø‹$P·ÇP‹F0èìîıÿPSèéşÿ÷ØÀ÷Ø‹Øë€~~ u‹$P·ÇPh°  ‹F PèKşÿ‹ÃYZ_^[Ã@ ‹Êfº èYÿÿÿÃèSÿÿÿÃ‹ÀSV‹Ø:Sdt3‹ÂˆCd„Àt‹Ë²¡tI ÿP$‹ğ‰s`‹S<‹Æèÿÿÿë‹C`è‰âıÿ3À‰C`^[ÃU‹ìƒÄìSVW3Ò‰Uğ‰Uì‹Ø3ÀUhñIB dÿ0d‰ EøPèşÿEø²è¸÷şÿ‹ğ…ötöFt3öè-ğşÿ‹ø;s,tKƒ{, t…ÿt	…ÿt;{,uj 3Éº°  ‹C,èÿÿ‰s,ƒ{, t…ÿt	…ÿt;{,uj 3Éº°  ‹C,ègÿÿ€{d t‹C,…Àt;C`u‹ÃèÆ  Uì‹Æèlêÿÿ‹EìUğèiïşÿ‹Uğ¡(I èà   ÆE÷fƒ»®    tM÷‹Ó‹ƒ°   ÿ“¬   €}÷ tè^şÿ3ÀZYYd‰høIB Eìº   è0êıÿÃénæıÿëë_^[‹å]ÃSVW‹ú¡,I èEçÿÿ‹ØK…Û|C3öj ‹Ö¡,I è!çÿÿ·×3Éè§ÿÿFKuä_^[Ã@ S‹Ø€=ÄI  t‹ÃèÁöÿÿPjh€   ‹C PèÔşÿë‹C Pè¹şÿ…Àtjj ‹C Pè˜şÿfº°‹Ãèyÿÿÿ[Ã@ SV‹ò‹Ø‹C4‹Öèèëıÿt"C4‹Öè¨éıÿfƒ»¦    t‹Ó‹ƒ¨   ÿ“¤   ^[ÃSVWU‹Ù‹ú‹è‹Åè3   hL4B Wjj è¸şÿ‹ğf‰uhf…ö—ÀˆEeˆ]f€}e u‹ÅèM  ]_^[ÃS‹Ø€{e t·ChPj è7şÿÆCe [ÃSVWUƒÄì‹é‹ú‹ØT$‹EèşÿT$L$‹ÇèÜûşÿD$²èmõşÿèÔèÿÿ‹ğ…öt€~] u	‹Ãèá   ëd;s@u0T$‹Eè:şÿT$L$‹Çè—ûşÿÿt$ÿt$CDPè2şÿ…Àu/ŠC8ˆ$€<$ t‹{Xë‹{T‹Ãè   Š$ˆC8‰s@3É‹×‹ÃèïşÿÿƒÄ]_^[Ã@ SƒÄø‹Ø‹ÃèÿÿÿŠCf,ru‹Ãè   ëT/* 
 * Extreme Tux Racer (Not Copyrighted)
 * 
 * based on Planet Penguin Racer, Open Racer, and Tux Racer: 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "credits.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/font.h"


#define CREDITS_MAX_Y -140
#define CREDITS_MIN_Y 64

typedef struct {
    pp::Font *font;
	char *binding;
    char *text;
} credit_line_t;

static credit_line_t credit_lines[] = 
{
	{ NULL, "credits_h1", "Extreme Tux Racer" },
    { NULL, "credits_text", "Version 0.4" },
    { NULL, "credits_text", "on the web at: www.extremetuxracer.com" },
    { NULL, "credits_text", "" },	
	{ NULL, "credits_text", "Extreme Tux Racer is in the process of being" },	
	{ NULL, "credits_text", "completely rewritten. All versions after 0.35," },	
	{ NULL, "credits_text", "including this one, won't have major code changes" },	
	{ NULL, "credits_text", "because the programmers are busy making a new, improved game." },		
	{ NULL, "credits_text", "" },		
	{ NULL, "credits_text", "Extreme Tux Racer is based on:" },
	{ NULL, "credits_text", "PlanetPenguin Racer," },
	{ NULL, "credits_text", "Open Racer," },
	{ NULL, "credits_text", "and Tux Racer" },
	    { NULL, "credits_text", "" },
		{ NULL, "credits_text", "Some courses may have been modified from their" },
		{ NULL, "credits_text", "original versions to fit the requirements of the game." },		
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Development" },
    { NULL, "credits_text", "Volker StrÃ¶bel" },
    { NULL, "credits_text", "Steven Bell" },
    { NULL, "credits_text", "Hamish Morrison" },
    { NULL, "credits_text", "Andreas Tarandi" },
	{ NULL, "credits_text_small", "from Sunspire Studios:" },
    { NULL, "credits_text", "Patrick \"Pog\" Gilhuly" },
    { NULL, "credits_text", "Eric \"Monster\" Hall" },
    { NULL, "credits_text", "Rick Knowles" },
    { NULL, "credits_text", "Vincent Ma" },
    { NULL, "credits_text", "Jasmin Patry" },
    { NULL, "credits_text", "Mark Riddell" },
		{ NULL, "credits_text", "" },
		{ NULL, "credits_h2", "Non-Rewrite Version Organization" },
		{ NULL, "credits_text", "Christian Picon" },	
		{ NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Translators" },
	{ NULL, "credits_text", "Nicosmos (French)" },
	{ NULL, "credits_text", "teksturi (Finnish)" },
	{ NULL, "credits_text", "arith, spacedwarv, and JoyFM (German)" },
	{ NULL, "credits_text", "spectrum (Italian)" },
	{ NULL, "credits_text", "Andreas Tarandi and pingvin (Swedish)" },
    	{ NULL, "credits_text", "Asciimonster (Dutch)" },
    	{ NULL, "credits_text", "woody (Polish)" },
    	{ NULL, "credits_text", "ttsmj (Slovak)" },		
    	{ NULL, "credits_text_small", "other (incomplete) translations are from the PPRacer Project" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Graphics" },
    { NULL, "credits_text", "Nicosmos (Logo, Hud Graphics)" },
    { NULL, "credits_text", "Christian Picon (Objects, Skyboxes)" },
    { NULL, "credits_text", "Reinhard Niehoff (Trees)" },	
    { NULL, "credits_text", "Daniel Poeira and Christian Picon (Font)" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Music" },
    { NULL, "credits_text", "Joseph Toscano" },
	    { NULL, "credits_text_small", "'Race 1'" },
		{ NULL, "credits_text_small", "'Menu'" },
	    { NULL, "credits_text_small", "'Options'" },	
	    { NULL, "credits_text_small", "'Won Race'" },		
    { NULL, "credits_text", "Christian Picon" },
	    { NULL, "credits_text_small", "'Credits Ballad'" },	
		    { NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Misc. PPRacer Contributors" },
	{ NULL, "credits_text", "Peter Reichel" },
	{ NULL, "credits_text", "Teemu Vesala" },
	{ NULL, "credits_text", "Theo Snelleman" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Thanks To" },
    { NULL, "credits_text_small", "(for Exteme Tux Racer)" },
    { NULL, "credits_text", "Ranger (for hosting the website)" },
    { NULL, "credits_text", "Slythfox (for making the website)" },
    { NULL, "credits_text", "The Course Creators" },	
    { NULL, "credits_text", "The OpenRacer project" },
    { NULL, "credits_text", "Reinhard Niehoff (for helping with code modification)" },
    { NULL, "credits_text", "All the people on the forum." },
    { NULL, "credits_text", "Larry Ewing (creator of Tux)" },
    { NULL, "credits_text_small", "(for Tux Racer)" },
    { NULL, "credits_text", "Thatcher Ulrich" },
    { NULL, "credits_text", "Steve Baker" },
    { NULL, "credits_text", "Ingo Ruhnke" },
    { NULL, "credits_text", "James Barnard" },
    { NULL, "credits_text", "Alan Levy" },
    { NULL, "credits_text", "" },
  { NULL, "credits_text_small", "Tux Racer is a trademark of Jasmin F. Patry" },
	{ NULL, "credits_text_small", "Extreme Tux Racer is licenced under the GPL." },
	{ NULL, "credits_text_small", "We grant use of the name 'Extreme Tux Racer' to any forks or continuations. " },
	{ NULL, "credits_text_small", "" },
	{ NULL, "credits_text_small", "PlanetPenguin Racer is Copyright Â© 2005 Volker Stroebel" },
  { NULL, "credits_text_small", "Tux Racer and the Tux Racer Name are Copyright Â© 2001 Jasmin F. Patry" },
};

Credits::Credits()
 : m_yOffset(0.0)
{
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
		credit_lines[i].font = pp::Font::get(credit_lines[i].binding);		
	}
	
  play_music( "credits_screen" );
}


Credits::~Credits()
{	
}

void
Credits::loop(float timeStep)
{
	int width, height;

  width = getparam_x_resolution();
  height = getparam_y_resolution();

  update_audio();

  clear_rendering_context();

  set_gl_options( GUI );

  UIMgr.setupDisplay();

  drawText( timeStep );

	drawSnow(timeStep);

	theme.drawMenuDecorations();

  UIMgr.draw();

  reshape( width, height );

  winsys_swap_buffers();
}


void
Credits::drawText( float timeStep )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    float y;

    m_yOffset += timeStep * 30;
    y = CREDITS_MIN_Y + m_yOffset;

	//loop through all credit lines
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
	    credit_line_t line = credit_lines[i];

		//get the font and sizes for the binding
		//pp::Font *font = pp::Font::get(line.binding);
		float width = line.font->advance(line.text);
		float desc = line.font->descender();
		float asc = line.font->ascender();
		
		//draw the line on the screen
		line.font->draw(line.text, w/2 - width/2, y);

		//calculate the y value for the next line
		y-=asc-desc;
	}

	//if the last string reaches the top, reset the y offset
    if ( y > h+CREDITS_MAX_Y ) {
		m_yOffset = 0;
    }

    // Draw strips at the top and bottom to clip out text 
    glDisable( GL_TEXTURE_2D );

    glColor4dv( (double*)&theme.background );

    glRectf( 0, 0, w, CREDITS_MIN_Y );

    glBegin( GL_QUADS );
    {
	glVertex2f( 0, CREDITS_MIN_Y );
	glVertex2f( w, CREDITS_MIN_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( w, CREDITS_MIN_Y + 30 );
	glVertex2f( 0, CREDITS_MIN_Y + 30 );
    }
    glEnd();

    glColor4dv( (double*)&theme.background );

    glRectf( 0, h+CREDITS_MAX_Y, w, h );

    glBegin( GL_QUADS );
    {
	glVertex2f( w, h+CREDITS_MAX_Y );
	glVertex2f( 0, h+CREDITS_MAX_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( 0, h+CREDITS_MAX_Y - 30 );
	glVertex2f( w, h+CREDITS_MAX_Y - 30 );
    }
    glEnd();

    glColor4f( 1, 1, 1, 1 );

    glEnable( GL_TEXTURE_2D );
}

bool
Credits::mouseButtonReleaseEvent(int button, int x, int y)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}

bool
Credits::keyReleaseEvent(SDLKey key)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ÿ¤KA       €    ParentShowHintŒA P  ÿLJA       €   € 	PopupMenu @ ]  ÿˆKA `KA    €   € ShowHint„+A ‚A ˜‚A       €ÿÿÿÿ TabOrder @ Ä  ÿ°‚A       €     TabStop @ 7  ÿÜIA       €    Visibleì @ ˜  ÿ˜  ÿ      €   € OnClickì @    ÿ   ÿ      €   € 
OnDblClick$-A x  ÿx  ÿ      €   € 
OnDragDrop¸,A €  ÿ€  ÿ      €   € 
OnDragOver¨-A   ÿ  ÿ      €   € 	OnEndDragì @ ì  ÿì  ÿ      €   €  OnEnterì @ ô  ÿô  ÿ      €   €! OnExitP,A Ô  ÿÔ  ÿ      €   €" 	OnKeyDownŒ,A Ü  ÿÜ  ÿ      €   €# 
OnKeyPressP,A ä  ÿä  ÿ      €   €$ OnKeyUp¤+A `  ÿ`  ÿ      €   €% OnMouseDown,A h  ÿh  ÿ      €   €& OnMouseMove¤+A p  ÿp  ÿ      €   €' 	OnMouseUpl-A ˆ  ÿˆ  ÿ      €   €( OnStartDrag@ TListBoxStyle       Ì~B 
lbStandardlbOwnerDrawFixedlbOwnerDrawVariable        €B         ĞB €B 8  ¨0A ÜjA à*@ +@ Ä¸B H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA ğ·B ŒA ¬A  €A @CA ôCA ”IA |¾B T€A œ€A h|A €€A ¨^A ÜeA ¤¼B èdA „½B „eA $¾B ìlA ¸~A A ¤ÁB `ÂB    ½+¼,¼°óÿ ÁB 0ÁB t¿B €ÂB <ÃB à¾B hÃB DÁB TCustomListBox@ TCustomListBoxLB Ø2A 
 StdCtrls  @ Ä  ÿ°‚A       €   	 TabStop         B             B 8  LB ÜjA à*@ +@ Ä¸B H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA ğ·B ŒA ¬A  €A @CA ôCA ”IA |¾B T€A œ€A h|A €€A ¨^A ÜeA ¤¼B èdA „½B „eA $¾B ìlA ¸~A A ¤ÁB `ÂB TListBox@ TListBox€B €B 3 StdCtrls* +A ;  ÿ¤DA       €    
 Align¸áA   ÿh»B       €    BorderStyleøA H  ÿÄKA àKA    €  € Color4@  ÿH¹B       €     Columns @ Å  ÿÄA èA    €   € Ctl3D¸)A N  ÿN  ÿ      €ôÿÿÿ 
DragCursorT+A <  ÿ8  ş      €     DragMode @ 8  ÿJA       €    Enabled @  ÿğ¹B       €    ExtendedSelect€“A D  ÿLKA XKA    €   € Font @  ÿºB       €     IntegralHeight4@ ºB \ºB       €   € 
ItemHeight4¦@ ü  ÿ¨»B       €   € Items @  ÿºB       €     MultiSelect @ :  ÿèKA       €     ParentColor @ Æ  ÿôA       €    ParentCtl3D @ 9  ÿhKA       €    
ParentFont @ ^  ÿ¤KA       €    ParentShowHintŒA P  ÿLJA       €   € 	PopupMenu @ ]  ÿˆKA `KA    €   € ShowHint @  ÿ$»B       €     SortedÌ~B  ÿ8»B       €     Style„+A ‚A ˜‚A       €ÿÿÿÿ  TabOrder @ Ä  ÿ°‚A       €   	 TabStop4@  ÿtºB       €    ! TabWidth @ 7  ÿÜIA       €   " Visibleì @ ˜  ÿ˜  ÿ      €   €# OnClickì @    ÿ   ÿ      €   €$ 
OnDblClick$-A x  ÿx  ÿ      €   €% 
OnDragDrop¸,A €  ÿ€  ÿ      €   €& 
OnDragOver”dB ( ÿ( ÿ      €   €' 
OnDrawItem¨-A   ÿ  ÿ      €   €( 	OnEndDragì @ ì  ÿì  ÿ      €   €) OnEnterì @ ô  ÿô  ÿ      €   €* OnExitP,A Ô  ÿÔ  ÿ      €   €+ 	OnKeyDownŒ,A Ü  ÿÜ  ÿ      €   €, 
OnKeyPressP,A ä  ÿä  ÿ      €   €- OnKeyUpğdB 0 ÿ0 ÿ      €   €. OnMeasureItem¤+A `  ÿ`  ÿ      €   €/ OnMouseDown,A h  ÿh  ÿ      €   €0 OnMouseMove¤+A p  ÿp  ÿ      €   €1 	OnMouseUpl-A ˆ  ÿˆ  ÿ      €   €2 OnStartDrag²…ÀtƒxP u	‹@ …Àuó3Ò‹ÂÃ        ä‡B             Ô‡B    À¥@ ,,@ à*@ +@ L+@ H¸@ ¬»@ Ğº@ ¬–B T–B Ì¾@ @™B ğ–B  Â@ T™B ™B È¹@ ì¹@ º@  ™B p˜B ´¼@ Ğ¾@ À@ d—B ôÀ@ HÁ@ ôÁ@ Ã@ pÃ@ üÄ@ TMemoStrings@ TMemoStringsl‡B 4¦@   StdCtrls          ¸ˆB             ¤ˆB    À¥@ ,,@ à*@ +@ L+@ H¸@ ¬»@ Ğº@ HœB ,œB ŒœB (¿@ pÂ@ ¬œB PÅ@ äB ĞœB ì¹@ º@ tB TB ´¼@ Ğ¾@ À@ B ôÀ@ HÁ@ ôÁ@ Ã@ pÃ@ üÄ@ TComboBoxStrings@ TComboBoxStrings<ˆB 4¦@   StdCtrls          Œ‰B             |‰B    À¥@ ,,@ à*@ +@ L+@ H¸@ ¬»@ Ğº@ D¶B (¶B œ¶B (¿@ pÂ@ Ø¶B PÅ@ ¼·B ü¶B ì¹@ º@  ·B €·B ´¼@ Ğ¾@ À@ <·B ôÀ@ HÁ@ ôÁ@ Ã@ pÃ@ üÄ@ TListBoxStringsTListBoxStrings‰B 4¦@   StdCtrls  ‹ÀSV„ÒtƒÄğèn£ıÿ‹Ú‹ğ3Ò‹ÆèUüşÿf¡ŠB fF4f‰F4ºA   ‹ÆèË»şÿº   ‹Æèß»şÿÆ†±   Æ†³   „Ût
d    ƒÄ‹Æ^[Ã  @  SV‹ò‹Ø‹Ö‹ÃèUÀşÿ^[Ã‹ÀU‹ìj SVW‹ñ‹ú‹Ø3ÀUhıŠB dÿ0d‰ Uü‹Ã‹ÿQXf÷Æ t-ƒ}ü t€»³    t‹Eü€8&u‹Eü€x uEüº‹B èòªıÿ€»³    ufÎ ‹SD‹ƒ¨   èª$ÿÿ€{8 u‹ƒ¨   ‹@º  €è­ÿÿ·ÆPW‹Eüè¨ªıÿP‹Eüèc¬ıÿP‹ƒ¨   èK%ÿÿPè•Óıÿ3ÀZYYd‰h‹B Eüè©ıÿÃéb¥ıÿëğ_^[Y]Ã  ÿÿÿÿ       SVƒÄà‹Ø‹³¨   ‹Ãè@  „Àu+‹SH‹Fè	ÿÿ3Ò‹Fè·ÿÿT$‹Ã‹ÿQ,T$‹Æè ÿÿ²‹Fè—ÿÿ‹Ô‹Ã‹ÿQ,3ÀŠƒ²   f‹EhI fƒÉ@3ÀŠƒ°   fE`I ‹Ô‹Ãè•şÿÿƒÄ ^[Ã@ SVƒÄğ‹ØöC…†   €»±    t}‹Ô‹Ã‹ÿQ,j è>Óıÿ‹ğ‹Ö‹ƒ¨   èÏ$ÿÿ3ÀŠƒ²   f‹ElI fÉ@‹Ô‹Ãè5şÿÿ3Ò‹ƒ¨   è¤$ÿÿVj èäÔıÿ‹s$€»°   u	‹C,+D$ğ‹D$P‹D$P‹K(‹Ö‹Ã‹ÿSLƒÄ^[Ã@ :°   tˆ°   ‹ÿRDÃ:±   tˆ±   è9ÿÿÿÃö@4@•À4Ã‹À‰¬   …Òt’èèuşÿÃ@ :³   tˆ³   ‹ÿRDÃSV‹Ú‹ğ‹Æè¿ÿÿÿ:Øt+„Ûtf¡àŒB ÷Ğf#F4f‰F4ëf¡àŒB fF4f‰F4‹Æ‹ÿRD^[Ã @   :²   tˆ²   è­şÿÿÃSVW‹Ù‹ò‹ø‹Ë‹Ö‹Çèl·şÿ€ûu;·¬   u3À‰‡¬   _^[Ã@ S‹Ø‹Ã‹ÿRD‹Ãèkşÿÿ[ÃS‹Ø‹ÃèúÊşÿ‹ÃèWşÿÿ[ÃU‹ìj SVW‹ò‹Ø3ÀUhÑB dÿ0d‰ ƒ»¬    tH€{8 tB€»³    t9Uü‹Ãèô¼şÿ‹Uüf‹FèÔgÿÿ„Àt‹›¬   ‹Ãè?óşÿ„Àt‹Ã‹ÿRxÇF   3ÀZYYd‰hØB Eüè0¦ıÿÃé¢ıÿëğ_^[Y]Ã‹ÀSV„ÒtƒÄğèBŸıÿ‹Ú‹ğ3Ò‹ÆèéÊşÿ€=ÄI  tf¡|B f‰F4ë
f¡€B f‰F4ºy   ‹Æè·şÿº   ‹Æè¢·şÿ²‹Æèuôşÿ3Ò‹Æè¤½şÿÆ†   Æ†  Æ†  Æ†  ‹Æè±  „Ût
d    ƒÄ‹Æ^[Ã  ¨  ¸  :  tˆ  è=  ÃS‹Ø:“   tˆ“   ‹Ãè$  ‹ÃèÕ×şÿ[Ã@ :  tˆ  è½×şÿÃ:  tˆ  è©×şÿÃSV‹ò‹Ø;³ü   t&‰³ü   ‹Ãèôşÿ„Àtj VhÅ   ‹ÃèfòşÿPèÒıÿ^[Ã:  tˆ  è]×şÿÃSV‹ğŠ	  ‹ÆèÏóşÿ„Àtj j h¸   ‹Æè#òşÿPèÙÑıÿ…À•Ã‹Ã^[Ã@ SV‹Ú‹ğ‹Æè›óşÿ„Àtj 3ÀŠÃPh¹   ‹ÆèìñşÿPè¢Ñıÿ^[Ãˆ	  ^[Ã‹ÀU‹ìj SVW‹Ø3ÀUhB dÿ0d‰ :“  tIˆ“  ‹ÃèCóşÿ„Àt8j 3ÀŠƒ  PhÌ   ‹ÃèñşÿPèFÑıÿUü‹Ãèˆºşÿ‹Eüè8§ıÿ‹Ğ‹ÃèSºşÿ3ÀZYYd‰h&B Eüèâ£ıÿÃé@ ıÿëğ_^[Y]ÃSV‹Ú‹ğ:  t*ˆ  ‹ÆèÉòşÿ„Àtj 3ÀŠÃPhÏ   ‹ÆèñşÿPèĞĞıÿ^[ÃSQ‹Øj D$Ph°   ‹ÃèùğşÿPè¯Ğıÿ‹$Z[ÃSV‹ò‹ØVVh±   ‹ÃèØğşÿPèĞıÿ^[Ã@ SƒÄø‹ØD$PD$Ph°   ‹Ãè°ğşÿPèfĞıÿ‹D$+$YZ[Ã@ SVƒÄø‹ò‹ØD$PD$Ph°   ‹Ãè}ğşÿPè3Ğıÿ4$‰t$‹D$P‹D$Ph±   ‹ÃèZğşÿPèĞıÿYZ^[Ã@ S‹ØhD‘B ‹Ãè=ğşÿPèƒĞıÿ[Ã     S‹Øj j h  ‹ÃèğşÿPèÓÏıÿ[ÃS‹Øjÿj h±   ‹ÃèğşÿPè·Ïıÿ[ÃU‹ìƒÄôSVW‹ú‹ğEôPEøPh°   ‹Æè×ïşÿPèÏıÿ‹]ô+]ø‹Ç‹Ë3Òè(£ıÿ…Ûta‹Æèm¸şÿ@è¿âıÿ‰Eü3ÀUh’B dÿ0d‰ ‹Eüèºâıÿ‹È‹Uü‹ÆèR¸şÿ‹‹Eø‹Mü‹Ëè8•ıÿ3ÀZYYd‰h’B ‹Eüè¿âıÿÃéIıÿëğ_^[‹å]ÃSV‹ò‹Ø‹Æè¥ıÿPj hÂ   ‹Ãè8ïşÿPèîÎıÿ^[Ã@ SV‹ò‹Ø‹Ö‹ÃèEĞşÿ¹ “B ‹Ö‹ÃèûÏşÿ‹FÀ   3ÒŠ“   •XI €»   •Âƒâ•pI 3ÒŠ“  •xI 3ÒŠ“  •€I 3ÒŠ“  •ŒI 3ÒŠ“  •”I ‰F€=ÄI  t €»Å    t€»   ufÿÿÿN   ^[Ã EDIT    U‹ìQ‰Eü‹EüÆ€  3ÀUhG“B dÿ0d‰ ‹EüèyĞşÿ3ÀZYYd‰hN“B ‹EüÆ€   Ãéıÿëîj ‹Eü‹€ü   PhÅ   ‹EüèîşÿPèÇÍıÿ‹EüŠ	  ‹Eüèêûÿÿ‹Eü€¸   t j ‹Eü¶€  PhÌ   ‹EüèÔíşÿPèŠÍıÿ‹Eüè"   Y]Ã@ S‹Ø‹Ãènûÿÿˆƒ	  ‹Ãè]Ñşÿ[Ã@ S‹Ø€»   t €»   uf¡”B fC4f‰C4‹Ãè   [Ãf¡”B ÷Ğf#C4f‰C4[Ã     SVWƒÄ‹ğj èåÊıÿ‹ØTSèTÈıÿ‹FDèHÿÿPSèÅÈıÿ‹øD$8PSè8ÈıÿWSè±ÈıÿSj è™Ìıÿ€=ÄI  t"€¾Å    t»   ë»   jè4Ëıÿ÷ë‹Øë%‹$‹D$8;Ø~‹ØjèËıÿÁà…ÛyƒÃÁûÃ‹Ø‹T$8Ó‹Æè±şÿƒÄp_^[Ãfƒ¸   t‹È‹Ğ‹  ÿ‘  ÃSV‹Ú‹ğ‹ƒèt-ş  u(‹Æè)òÿÿ„Àu&ëƒ=Ü I u‹CPè%Ëıÿ…Àu3À‰C‹Ó‹Æè»Õşÿ^[ÃV‹ğ‹Æ‹ÿQğ€=ÄI  t)jğ‹Æè8ìşÿPè~Êıÿ¨uj jhÓ   ‹ÆèìşÿPèÔËıÿ^Ã‹ÀSV‹ò‹Ø€=ÄI  t€»   u‹ÃèQşÿÿ‹ÃèÑşÿ‹Ö‹ÃèQãşÿ^[Ã‹ÀS‹Ø‹ÃèÆâşÿöC5töCtöCu‹Ãè]şÿÿ[Ã@ Sfz u€¸   u	f»àÿè%–ıÿ[Ã@ SV‹ò‹Ø€»   t öC6ujğ‹ÃèzëşÿPèÀÉıÿ¨u‹ÃèUûÿÿ‹Ö‹Ãè,áşÿ^[ÃSV‹ğ‹Æ‹ÿQğ‹Æèâìşÿ„Àtjğ‹Æè=ëşÿPèƒÉıÿ¨t‹Æf»àÿè°•ıÿ^[ÃSV‹Øj j hº   ‹CèëşÿPèÅÊıÿ‹ğj j ‹ÆHPh»   ‹CèòêşÿPè¨ÊıÿPhÁ   ‹CèŞêşÿPè”Êıÿ…ÀuN‹Æ^[Ã‹ÀSVWÄğÿÿP‹ù‹ò‹Ø‹ÄfÇ  ‹ÄPVhÄ   ‹Cè¤êşÿPèZÊıÿ‹È‹Ô‹ÇèûıÿÄ   _^[ÃSVW‹ù‹ò‹Øj Vh»   ‹CèoêşÿPè%Êıÿ‹ğ…ö|Jj VhÁ   ‹CèSêşÿPè	ÊıÿÆPVh±   ‹Cè<êşÿPèòÉıÿ‹ÇèïŸıÿPj hÂ   ‹CèêşÿPèÕÉıÿ_^[ÃU‹ìƒÄøSVW3Û‰]ø‰Mü‹ú‹ğ3ÀUhO˜B dÿ0d‰ …ÿŒª   j Wh»   ‹FèÙéşÿPèÉıÿ‹Ø…Û|Eø¹h˜B ‹UüèıÿëIj OWh»   ‹FèªéşÿPè`Éıÿ‹Ø…Û|_j ShÁ   ‹FèéşÿPèDÉıÿ…ÀtEØEø‹Müºh˜B èºıÿSSh±   ‹FècéşÿPèÉıÿ‹EøèŸıÿPj hÂ   ‹FèEéşÿPèûÈıÿ3ÀZYYd‰hV˜B Eøè²›ıÿÃé˜ıÿëğ_^[YY]Ã   ÿÿÿÿ   
      SVW‹ú‹Øj Wh»   ‹CèñèşÿPè§Èıÿ‹ğ…ö|gj GWh»   ‹CèÔèşÿPèŠÈıÿ‹ø…ÿ}j VhÁ   ‹Cè¸èşÿPènÈıÿ‹øşWVh±   ‹CèŸèşÿPèUÈıÿ¡œI Pj hÂ   ‹Cè„èşÿPè:Èıÿ_^[Ã‹À‹@è$øÿÿÃ@ SV‹Ú‹ğj ‹Ã4ƒàPj‹FèRèşÿPèÈıÿ€ó„Ût‹Fè¹µşÿ^[Ã‹ÀSV‹ò‹Ø‹Ö‹Cè0±şÿ^[ÃU‹ìj j SVW‹ò‹Ø3ÀUhšB dÿ0d‰ Uü‹Æè~Òıÿ‹Eüèò›ıÿ‹ğ‹Cè¤°şÿ;ğuUø‹Cèå°şÿ‹Uø‹EüèŞœıÿtA‹EüPj j‹CèÀçşÿPèvÇıÿ…Àu¹³ğ  ²¸¤)A èyòıÿèD—ıÿj 3Éº°  ‹Cèï·şÿ3ÀZYYd‰hšB EøèšıÿEüèú™ıÿÃéX–ıÿëè_^[YY]Ã@ U‹ìQSV„ÒtƒÄğè“ıÿˆUÿ‹Ø3Ò‹Ãè¨óÿÿº¹   ‹Ãèp«şÿºY   ‹Ãè„«şÿ3Ò‹Ãè+ôÿÿÆƒ  Æƒ  ²¸l‡B è½ıÿ‹ğ‰³  ‰^€}ÿ t
d    ƒÄ‹Ã^[Y]Ã@ SV‹Ú‹ğ‹†  è¯ıÿ3Ò‹Æèª¾şÿ„Ût‹Æè¯’ıÿ‹Æ^[Ã‹ÀSV‹ò‹Ø‹Ö‹Ãèy÷ÿÿ‹F3ÒŠ“  ‹•¼I ÷Ò#ÂƒÈ3ÒŠ“  • I 3ÒŠ“  •¬I ‰F^[ÃSVW‹Ú‹ğ‹C P¡I Pj ‹CP‹CP‹CP‹CP‹CP‹CPhh›B CLP‹CPè·Âıÿ‹ø‰¾À   ‹Pj jWèÒÅıÿ_^[Ã      S‹Ø‹Ãè^jşÿ3Ò‹Ãèéóÿÿ[Ã@ :  tˆ  èõÊşÿÃ‹€  ‹ÿQÃ:  tˆ  èÕÊşÿÃ:  tˆ  èÁÊşÿÃSV‹ò‹Ø‹Ö‹Ã‹ÿQğ€»   tƒNëƒfı€»   uƒfû^[Ã‹ÀèGÚşÿÃ‹ÀSV‹ò‹Ø‹Ö‹Ãè}Øşÿ€>u€»   uÆ ^[ÃS‹Øj j hF  ‹Cè8åşÿPèîÄıÿ[ÃSVWÄğÿÿP‹ù‹ò‹Ø‹ÄPVhH  ‹CèåşÿPèÅÄıÿƒøÿu3À‹Ô‹Ï‘è`˜ıÿÄ   _^[Ã‹ÀSV‹ò‹Øj VhP  ‹CèÖäşÿPèŒÄıÿ^[ÃSVW‹ù‹ò‹ØWVhQ  ‹Cè´äşÿPèjÄıÿ_^[Ã‹ÀSV‹ò‹Ø‹Æè[šıÿPj hC  ‹Cè‹äşÿPèAÄıÿ…À}¹¦ğ  ²¸`)A èDïıÿè”ıÿ^[ÃSVW‹ù‹ò‹Ø‹ÇèšıÿPVhJ  ‹CèIäşÿPèÿÃıÿ…À}¹¦ğ  ²¸`)A èïıÿèÍ“ıÿ_^[ÃSV‹ò‹Øj VhD  ‹CèäşÿPèÄÃıÿ^[ÃU‹ìj SVW‹Ø3ÀUh×B dÿ0d‰ Uü‹Cèé¬şÿj j hK  ‹CèĞãşÿPè†Ãıÿ‹Uü‹Cè÷¬şÿ‹C‹ÿRP3ÀZYYd‰hŞB Eüè*–ıÿÃéˆ’ıÿëğ_^[Y]ÃSV‹Ú‹ğj ‹Ã4ƒàPj‹FèzãşÿPè0Ãıÿ€ó„Ût‹Fèá°şÿ^[Ã‹ÀU‹ìQSV„ÒtƒÄğèıÿˆUÿ‹Ø3Ò‹Ãè¬ºşÿ€=ÄI  tf¡ ŸB f‰C4ë
f¡ŸB f‰C4º‘   ‹ÃèQ§şÿº   ‹Ãèe§şÿ²‹Ãè8äşÿ3Ò‹Ãèg­şÿ²¸<ˆB è£Œıÿ‹ğ‰³ü   ‰^²¸À*A èŒÿÿ‰ƒ   Çƒ     Æƒ   ShÈ£B è–Tÿÿ‰ƒ  Sh„¤B è…Tÿÿ‰ƒ   Çƒ     €}ÿ t
d    ƒÄ‹Ã^[Y]Ã  ¢  ²  SV‹Ú‹ğ‹Æèûãşÿ„Àt‹Æ‹ÿRh‹†   èÉTÿÿ‹†  è¾Tÿÿ‹†   èŒıÿ‹†ü   èŒıÿ‹†0  èŒıÿ3Ò‹Æèü¹şÿ„Ût‹Æèıÿ‹Æ^[ÃS‹ØºŒŸB ‹Ãèİªşÿ‹ƒü   ‹ÿR8[Ã    S‹Øj j hW  ‹ÃèÕáşÿPè‹Áıÿ÷ØÀ÷Ø[Ã@ SV‹Ú‹ğj 3ÀŠÃPhO  ‹Æè«áşÿPèaÁıÿ^[Ã‹ÀS‹Øj j hG  ‹ÃèáşÿPèCÁıÿ[ÃSV‹ò‹Øj VhN  ‹ÃèoáşÿPè%Áıÿ^[Ã‹ÀSV‹ò‹Ø;³  t&‰³  ‹Ãèáâşÿ„Àtj VhA  ‹Ãè6áşÿPèìÀıÿ^[Ã:  tˆ  è-ÆşÿÃS‹Ø:“  t2ˆ“  €úuf¡  B ÷Ğf#C4f‰C4ëf¡  B fC4f‰C4‹ÃèïÅşÿ[Ã    Š  €Âı€ês‹€  Ãj 3ÉºT  è±şÿÃ…Ò~‰  Ã‹€ü   ‹ÿQÃSV‹ò‹Ø‹Ö‹Ãè±Áşÿ¹D¡B ‹Ö‹ÃègÁşÿ‹F@  3ÒŠ“  •ÄI 3ÒŠ“  •ØI ‰F€=ÄI  t€»Å    tN   ^[Ã  COMBOBOX    SVW‹Ø‹ÃèLÂşÿ‹³  …ö~şÿ   ~¾ÿ   j VhA  ‹ÃèößşÿPè¬¿ıÿ‹ƒ0  …Àt ‹Ğ‹ƒü   ‹ÿQ‹ƒ0  è®‰ıÿ3À‰ƒ0  3À‰ƒ  3À‰ƒ  Šƒ  ,s~j‹Ãè£ßşÿPèá½ıÿ‹ğ…öti€»  u5‹ş‰»  jüWèÊ½ıÿ‰ƒ(  ‹ƒ   Pjü‹ƒ  Pè—¿ıÿjVèŸ½ıÿ‹ğ‹ş‰»  jüWè•½ıÿ‰ƒ$  ‹ƒ  Pjü‹ƒ  Pèb¿ıÿ€=ÄI  tƒ»   tj jhÓ   ‹ƒ  PèÃ¾ıÿ_^[Ã@ SV‹Ø‹ƒü   ‹ÿR…À~!²¸Ğ¦@ è™ˆıÿ‹ğ‰³0  ‹“ü   ‹Æ‹ÿQ‹ÃèyÂşÿ^[Ã‹ÀV‹ğ‹Æ‹ÿQğ‹F@P‹Æè«ŞşÿPèñ¾ıÿ^Ã@ SVƒÄğ‹ò‹Ø€»  s/‹C ‹€¼   è€ÿÿPT$‹Ã‹ÿQ,D$P‹FPèª»ıÿÇF   ƒÄ^[ÃV‹ğ‹Æ‹ÿQğ^ÃV‹ğ‹Æ‹ÿQğ^ÃV‹ğ‹Æ‹ÿQğ^ÃSV‹ò‹Ø‹Ö‹Ã‹ÿQğ‹Ãè6üÿÿ„ÀtƒN^[Ã@ ;Btj 3ÉºO  è]®şÿÃSV‹ò‹Ø€=ÄI  t‹ÃèúÂşÿ‹Ö‹ÃèIÕşÿ^[Ã‹ÀV‹ğ‹Æè¦´şÿ€=ÄI  u€¾  s‹Æ‹ÿRD^Ã@ SVWƒÄğ‹ò‹Ø>  u‹Ö‹Ã‹ÿQ@é•   ‹ƒ$  P‹Ö‹‹  ‹Ã‹8ÿW|‹ƒè0tZ-Ñ  tƒèul€{<ufTèçºıÿL$‹Ô‹Ãè*£şÿ‹D$‰$‹D$‰D$‹Äè¾ıÿPj h  ‹ƒ  Pèà¼ıÿ3Ò‹Ãè—«şÿë€=ÄI  tj jhÓ   ‹ƒ  Pè·¼ıÿƒÄ_^[ÃSV‹ğ‹†(  P‹  ‹Æ‹ÿS|^[ÃU‹ìƒÄôSVW‹Ù‹ò‰Eü3ÀUhF§B dÿ0d‰ ‹=  O„Û   =‡   $„ß  ƒè„†   H„¤   ƒè|„æ  é  -   „¥   H„"  H„Å   éñ  = ½  *„Ğ  -  „ş   -û   „  ƒè„  éÀ  -½  „¡  ƒè„˜  ƒè„  é  ‹Eüè›Pÿÿ‹Uüèuÿÿ„À…†  3ÀZYYd‰éÏ  ‹Eüö@6 „l  3ÀZYYd‰éµ  ‹Eü;˜  „P  ‹Ö‹Eüè3Íşÿ„À„>  3ÀZYYd‰é‡  ‹Ö‹EüèÔÎşÿ„Àt3ÀZYYd‰él  f‹~fƒÿt
fƒÿ…  ‹Eüèƒùÿÿ„À„ò   3Ò‹Eüè•ùÿÿ3ÀZYYd‰é1  ‹Ö‹EüèºÍşÿ„À„É   3ÀZYYd‰é  ‹Î‹Uü¡(I èÊ¤ÿÿé¨   ‹Eüè¹àÿÿ„À„˜   ¿F‰Eô¿F
‰EøjEôP‹EüèíÚşÿPSèò¹ıÿf‹Eôf‰Ff‹Eøf‰F
‹Ö‹Eü‹ÿQ@3ÀZYYd‰é¤   ‹EüèÏøÿÿ„ÀtBÇF   3ÀZYYd‰é„   ‹Eüö@t%ÇFÿÿÿÿ3ÀZYYd‰ëj‹Ö‹Eü‹ÿQ@3ÀZYYd‰ëV‹FP‹FP‹PS‹EPè¶ıÿ‰F>  u‹Eüö@4€t‹Eüf»ôÿèÄ„ıÿ3ÀZYYd‰ëé%‡ıÿ‹Uü¡(I è Ÿÿÿè/Šıÿ_^[‹å]Â ‹ÀSVW‹ò‹ØöCuM‹>ÿ  tÿ  u;‹Ãèş¨şÿ„Àu0€{<u*‹Ö‹Ãè‡Áşÿ„À…Ù    ˆ¨B 
C6ˆC6‹Ö‹Ãès„ıÿéÀ   ‹-  t{ƒÀĞƒèrDÿÿƒèrDé—   ‹CD‹@èøşÿP‹FPèEµıÿ‹ƒ¼   è:şşÿèé÷şÿP‹FPèµıÿ‹ƒ¼   è\şşÿ‰Fëa€=ÄI  uO€»  sF‹C ‹€¼   è7şşÿ‰Fë<‹Ö‹ÃèeÌşÿ„Àu/f‹~fƒÿtfƒÿu‹Ãè&÷ÿÿ„Àt3Ò‹Ãè=÷ÿÿë	‹Ö‹Ãè2Áşÿ_^[Ã     SV‹Ø‹ƒü   ‹ÿR‹ğ‹ƒ  ;ğ~‹ğƒş}¾   h   ‹Ãèå÷ÿÿ÷îC0ƒÀP‹C,Pj j j ‹ÃèŸØşÿPèİ¸ıÿj_j j j j j ‹Ãè†ØşÿPèÄ¸ıÿ^[ÃU‹ìj SVW‹ğ3ÀUh/ªB dÿ0d‰ ·Bƒø‡ø   ÿ$…(©B ªB Â©B H©B û©B ªB X©B ªB h©B ‹Æf»ôÿè­‚ıÿéÁ   ‹Æf»àÿè‚ıÿé±   Æ†-   ‹Æf»ßÿè†‚ıÿ‹Æèÿÿÿ€¾-   „‹   j j j‹Æèİ×şÿPè#·ıÿ€¾,   uoj j hO  ‹Æè¾×şÿPè·ıÿëW‹Æèöÿÿ‹ĞMü‹†ü   ‹ÿS‹Uü‹ÆèÍ şÿ‹Æf»õÿè‚ıÿ‹Æf»àÿè‚ıÿëÆ†,  Æ†-  ëÆ†,   Æ†-  3ÀZYYd‰h6ªB EüèÒ‰ıÿÃé0†ıÿëğ_^[Y]Ãfƒ¸6   t‹È‹Ğ‹8  ÿ‘4  ÃU‹ìƒÄìSVW3Û‰]ì‹ñ}ğ¹   ó¥‹ò‹Ø3ÀUhôªB dÿ0d‰ fƒ»F   tEğPŠEP‹Î‹Ó‹ƒH  ÿ“D  ë6Uğ‹ƒ   èBÿÿMì‹Ö‹ƒü   ‹0ÿV‹EìP‹UğƒÂ‹Mô‹ƒ   èvÿÿ3ÀZYYd‰hûªB Eìè‰ıÿÃék…ıÿëğ_^[‹å]Â fƒ¸>   t‹È‹Ğ‹@  ÿ‘<  ÃSfƒ¸N   tQ‹Ø‹Ê‹Ğ‹ƒP  ÿ“L  [ÃSVWUQ‹ğ‹zŠGˆ$‹W‹†   èNÿÿ‹VD‹†   èäÿÿ‹–¼   ‹†   èëÿÿƒ |,ö$t&‹†   ‹@º  €è¨úşÿ‹†   ‹@º  €è½õşÿ‹o…í|Š$PO‹Õ‹Æ‹ÿ“€   ëW‹†   è ÿÿö$tGP‹GPèh²ıÿ3Ò‹†   è³ÿÿZ]_^[ÃSVW‹ğ‹z‹†  ‰G€¾  uO‹W‹Æ‹ÿ“„   _^[Ã‹ÀSV‹ò‹Ø€{<u0€»  u'jèh³ıÿ‹S,+Ğ¿F;Ğ~‹Ã‹ÿRx3Ò‹Ãè‹£şÿ^[Ã‹Ö‹Ãè¨şÿ‹Ãè¨Ÿşÿ„Àt‹Ãè­Iÿÿ;˜  t	3Ò‹ÃèœŸşÿ^[ÃSV‹ò‹Ø‹-  tƒèt	-»  t;ëBöCu<‹ÃèjÔşÿ„Àu1Æƒü   ‹Ãè ÔşÿPè†´ıÿÆƒü    ‹ÃèDÔşÿ„Àtë	€»ü    u	‹Ö‹Ãè°¼şÿ^[ÃSV„ÒtƒÄğè€ıÿ‹Ú‹ğ3Ò‹ÆèÁ«şÿf¡d­B f‰F4ºK   ‹Æè{˜şÿº   ‹Æè˜şÿ²‹ÆèbÕşÿ„Ût
d    ƒÄ‹Æ^[Ã   à   S‹Ø‹ÃèšHÿÿ…Àt‹“  ‰(  ‹ÃèC¦şÿ[ÃSVWU‹Ú‹è‹ÅèuÕşÿ„Àt:„Ûtf¾ ë3öjğ‹ÅèÄÓşÿPè
²ıÿƒà·ş;ÇtjWhô   ‹Åè¥ÓşÿPè[³ıÿ]_^[Ã‹ÀS‹Øˆ“   ‹Ãè Õşÿ„Àt‹ÃèHÿÿ‹  R3Éº°  èÊ£şÿ[ÃSV‹ò‹Ø‹Ö‹Ãè´şÿ¹D®B ‹Ö‹Ãè7´şÿ3ÀŠƒ   ‹…àI 	F^[Ã  BUTTON  S‹Ø‹ÃèRµşÿŠƒ   ˆƒ  [ÃSfƒz u	f»õÿè‹}ıÿ[ÃSV‹ò‹Øfƒ~u	€»   ufƒ~u;€»   t2‹FèŠFÿÿŠÜ®B :Ğu ‹Ãè1Òşÿ„Àt‹Ãf»õÿè:}ıÿÇF   ^[Ã‹Ö‹Ãè·Èşÿ^[Ã    U‹ìj SVW‹ò‹Ø3ÀUhR¯B dÿ0d‰ Uü‹Ãè|›şÿ‹Uüf‹Fè\Fÿÿ„Àt‹ÃèÍÑşÿ„Àt‹Ãf»õÿèÖ|ıÿÇF   ë	‹Ö‹Ãè\Èşÿ3ÀZYYd‰hY¯B Eüè¯„ıÿÃéıÿëğ_^[Y]ÃSVW‹ò‹Ø‹~‹ÇºØmB è*|ıÿ„Àt;ß”Àˆƒ  ëŠƒ   ˆƒ  Š“  ‹Ã‹ÿQ|‹Ö‹Ãè÷Çşÿ_^[Ã@ SV„ÒtƒÄğèr}ıÿ‹Ú‹ğ3Ò‹Æè©şÿºa   ‹Æèİ•şÿº   ‹Æèñ•şÿ²‹ÆèÄÒşÿf¡°B f‰F4Æ†   Æ†   „Ût
d    ƒÄ‹Æ^[Ã     Š  €êrt şÊt$ë)€¸   t	²èQ   ë²èH   Ã3Òè@   Ã²è8   Ã@ €¸  ”ÀÃ:   tˆ   è¶şÿÃ„Òt²è	   Ã3Òè   ÃS‹Ø:“  t9ˆ“  ‹ÃèhÒşÿ„Àtj 3ÀŠƒ  Phñ   ‹ÃèµĞşÿPèk°ıÿ‹Ãf»õÿè,{ıÿ[Ã‹ÀSV‹ò‹Ø‹Ö‹Ãè¹±şÿ¹±B ‹Ö‹Ãèo±şÿ‹FƒÈ3ÒŠ“   •èI ‰F^[ÃBUTTON  S‹Ø‹Ãè†²şÿj 3ÀŠƒ  Phñ   ‹Ãè?ĞşÿPèõ¯ıÿ[Ã@ SVW‹ò‹Ø€»Å    „’   €=ÄI  ……   ‹F P¡I Pj ‹FP‹FP‹FP‹FP‹FP‹FP‹Phø±B ‹FPèj¬ıÿ‹ø‰»À   ‹ÇèWAÿÿjü‹ƒÀ   Pè®ıÿ‰ƒ¬   ‰ØI hÔ5A jü‹ƒÀ   PèÖ¯ıÿj j j ‹ƒÀ   PèL¯ıÿë	‹Ö‹Ãèõ²şÿ_^[Ã BUTTON  è‡´şÿÃ‹ÀU‹ìj SVW‹ò‹Ø3ÀUhˆ²B dÿ0d‰ Uü‹ÃèT˜şÿ‹Uüf‹Fè4Cÿÿ„Àt-‹Ãè¥Îşÿ„Àt"‹Ã‹ÿRx‹ÃèÛÎşÿ„Àt‹Ã‹ÿR|ÇF   ë	‹Ö‹Ãè&Åşÿ3ÀZYYd‰h²B EüèyıÿÃé×}ıÿëğ_^[Y]Ã@ fƒz u‹ÿR|Ã@ SV‹ò‹Ø€»Å    t€=ÄI  u‹Ãè±ÎşÿPèW¯ıÿ‹Ö‹Ã‹ÿQğ^[Ã@ SV„ÒtƒÄğèFzıÿ‹Ú‹ğ3Ò‹Æèí¥şÿºq   ‹Æè±’şÿº   ‹ÆèÅ’şÿf¡4³B f‰F4Æ†   „Ût
d    ƒÄ‹Æ^[Ã     :   tˆ   èA³şÿÃU‹ìQSVW‹E‹@ü‹@ …ÀtI‹U‰Eü‹Eüèv®şÿ‹ğN…ö|4F3ÿ‹×‹Eüè2®şÿ‹Ø‹E;Xüt‹ÃºyB èxıÿ„Àt	3Ò‹Ãè   GNuÏ_^[Y]ÃU‹ìQS‹Ú‰Eü‹Eü:˜  tV‹Eüˆ˜  ‹Ó‹EüèØÎşÿ‹Eüè0Ïşÿ„Àt j ‹Eü¶€  Phñ   ‹EüèzÍşÿPè0­ıÿ„ÛtUè>ÿÿÿY‹Eüf»õÿèåwıÿ[Y]ÃSV‹ò‹Ø‹Ö‹Ãèq®şÿ¹X´B ‹Ö‹Ãè'®şÿ‹FƒÈ3ÒŠ“   •ğI ‰F^[ÃBUTTON  S‹Ø‹Ãè>¯şÿj 3ÀŠƒ  Phñ   ‹Ãè÷ÌşÿPè­¬ıÿ[Ã@ SVW‹ò‹Ø€»Å    „’   €=ÄI  ……   ‹F P¡I Pj ‹FP‹FP‹FP‹FP‹FP‹FP‹Ph@µB ‹FPè"©ıÿ‹ø‰»À   ‹Çè>ÿÿjü‹ƒÀ   PèÅªıÿ‰ƒ¬   ‰ØI hÔ5A jü‹ƒÀ   Pè¬ıÿj j j ‹ƒÀ   Pè¬ıÿë	‹Ö‹Ãè­¯şÿ_^[Ã BUTTON  è?±şÿÃ‹ÀU‹ìj SVW‹ò‹Ø3ÀUh¾µB dÿ0d‰ Uü‹Ãè•şÿ‹Uüf‹Fèì?ÿÿ„Àt‹Ãè]Ëşÿ„Àt‹Ã‹ÿRxÇF   ë	‹Ö‹ÃèğÁşÿ3ÀZYYd‰hÅµB EüèC~ıÿÃé¡zıÿëğ_^[Y]ÃSf‹Rfƒêrfƒêtë²èÊıÿÿ[Ãf»ôÿèvıÿ[ÃSV‹ò‹Ø€»Å    t€=ÄI  u‹ÃèeËşÿPè¬ıÿ‹Ö‹Ã‹ÿQğ^[Ã@ S‹Øj j h‹  ‹Cè<ËşÿPèòªıÿ[ÃSVWÄğÿÿP‹ù‹ò‹Ø‹ÄPVh‰  ‹CèËşÿPèÉªıÿ…À}¹ğ  ²¸Ğ£@ èÌÕıÿè—zıÿ‹Ô‹Ï‘èQ~ıÿÄ   _^[Ã@ SV‹ò‹Øj Vh™  ‹CèÆÊşÿPè|ªıÿƒøÿu¹ğ  ²¸Ğ£@ è~ÕıÿèIzıÿ^[Ã‹ÀSVW‹ù‹ò‹ØWVhš  ‹CèˆÊşÿPè>ªıÿ_^[Ã‹ÀSV‹ò‹Ø‹Æè/€ıÿPj h€  ‹Cè_ÊşÿPèªıÿ…À}¹¦ğ  ²¸`)A èÕıÿèãyıÿ^[ÃSVW‹ù‹ò‹Ø‹ÇèìıÿPVh  ‹CèÊşÿPèÓ©ıÿ…À}¹¦ğ  ²¸`)A èÖÔıÿè¡yıÿ_^[ÃSV‹ò‹Øj Vh‚  ‹CèâÉşÿPè˜©ıÿ^[ÃS‹Øj j h„  ‹CèÄÉşÿPèz©ıÿ[ÃSV‹Ú‹ğj ‹Ã4ƒàPj‹Fè¢ÉşÿPèX©ıÿ€ó„Ût‹Fè	—şÿ^[Ã‹ÀU‹ìQSV„ÒtƒÄğè.uıÿˆUÿ‹Ø3Ò‹ÃèÔ şÿ€=ÄI  tf¡¼¸B f‰C4ë
f¡À¸B f‰C4ºy   ‹Ãèyşÿºa   ‹Ãèşÿ²‹Ãè`Êşÿ3Ò‹Ãè“şÿ²¸‰B èËrıÿ‹ğ‰³ü   ‰^²¸À*A è´îşÿ‹ğ‰³  ‹Ó‹ÆèÏ‰şÿÇƒ     Æƒ   Æƒ  €}ÿ t
d    ƒÄ‹Ã^[Y]Ã    °   SV‹Ú‹ğ‹†  èƒrıÿ‹†ü   èxrıÿ‹†  èmrıÿ3Ò‹Æèh şÿ„Ût‹Æèmtıÿ‹Æ^[Ã‹€ü   ‹ÿR8ÃSV‹Øƒ»   ~&j ‹C,‹³  Æƒè™÷şPh•  ‹Ãè;ÈşÿPèñ§ıÿ^[Ã‹ÀS‹Ø;“  t4ƒ»   t…Òu‰“  ‹Ãè­şÿë‰“  ‹Ãè”Éşÿ„Àt‹Ãè‰ÿÿÿ[Ã@ S‹Øj j hˆ  ‹ÃèÙÇşÿPè§ıÿ[ÃS‹Øj j h  ‹Ãè½ÇşÿPès§ıÿ[ÃSV‹ò‹Ø‹Ãè»ÿÿÿ;ğtj Vh†  ‹Ãè”ÇşÿPèJ§ıÿ^[Ã@ :  tˆ  è‰¬şÿÃ:  tˆ  èu¬şÿÃSVƒÄğ‹Ø‹³  ‹ÃèäÈşÿ„Àt"€»   u‹ÄP3Éº˜  ‹Ãè—şÿ‹t$+t$‹ÆƒÄ^[Ã‹À;  t…Ò~‰  è¬şÿÃ…Ò}3Ò;  t‰  èÿ«şÿÃ‹À:  tˆ  èé«şÿÃSV‹ò‹Øj Vh‡  ‹Ãè¿ÆşÿPèu¦ıÿƒøÿu¹ğ  ²¸P£@ èwÑıÿèBvıÿ÷ØÀ÷Ø^[ÃSVW‹Ù‹ò‹øV3ÀŠÃPh…  ‹ÇèyÆşÿPè/¦ıÿ@u¹ğ  ²¸P£@ è3Ñıÿèşuıÿ_^[Ã‹À:  tˆ  èU«şÿÃ:  tˆ  èA«şÿÃS‹Øj j h  ‹ÃèÆşÿPèÏ¥ıÿ[Ã:   tˆ   è«şÿÃSV‹ò‹Ø‹ÃèÃÿÿÿ;ğtj Vh—  ‹ÃèÜÅşÿPè’¥ıÿ^[Ã@ ‹€ü   ‹ÿQÃSVWUƒÄØ‹ò<$¥¥‹Ù‹ğÿt$ÿt$T$ ‹Æ‹ÿQ,D$ Pèñ¤ıÿ…ÀtL‹Æè^ÿÿÿ‹è‹†ü   ‹ÿR‹ø;ı~.D$P‹Íº˜  ‹ÆèÄ•şÿÿt$ÿt$D$Pè®¤ıÿ…ÀuE;ıÒ„ÛtƒÍÿ‹ÅƒÄ(]_^[ÃSVW‹ù‹Ú‹ğ‹†ü   ‹ÿR…Ût;Ã~W‹Ëº˜  ‹Æèm•şÿë3;Ãu!W‹ËIº˜  ‹ÆèW•şÿ‹G+GPj Wè$¤ıÿë‹Ç3Éº   è”lıÿ_^[ÃSV‹ò‹Ø‹Ö‹Ãèí¥şÿ¹|½B ‹Ö‹Ãè£¥şÿ¸I €»   t¸I ‹VÊA 0 3ÉŠ‹  øI 3ÉŠ‹  I 3ÉŠ‹  ˆ3ÀŠƒ  …I ƒ»   •Àƒà…$I 3ÀŠƒ   …XI ƒ»   •Àƒà…,I ‰V€=ÄI  t €»Å    t€»   ufÿÿÿN   ^[Ã LISTBOX SVW‹Ø‹s,‹{0‹Ãè¦şÿjWV‹C(P‹C$Pj ‹ÃèÍÃşÿPè¤ıÿƒ»   tƒ  Pjh’  ‹Ãè©ÃşÿPè_£ıÿ‹Ãè4ûÿÿ‹ƒ  …Àt:‹Ğ‹ƒü   ‹ÿQ‹“   ‹Ãè|ıÿÿ‹“$  ‹Ãè·ûÿÿ‹ƒ  è@mıÿ3À‰ƒ  _^[ÃSV‹Ø‹ƒü   ‹ÿR…À~;²¸Ğ¦@ èílıÿ‹ğ‰³  ‹“ü   ‹Æ‹ÿQ‹Ãèíüÿÿ‰ƒ   ‹Ãè ûÿÿ‰ƒ$  ‹Ãè³¦şÿ^[ÃSVW‹ò‹ØöCuF‹>ÿ  tÿ  u4‹Ãèê‘şÿ„Àu)€{<u#‹Ö‹Ãèsªşÿ„Àu Ü¾B 
C6ˆC6‹Ö‹Ãècmıÿë	‹Ö‹ÃèÜªşÿ_^[Ã   SVWUƒÄø‹ú‹ğf‹Gèè5ÿÿ‹Ø€~<uF€¾   t=öÃtöÃt3‹Ô‹Gè££ıÿ‹Ô±‹Æèüÿÿ‹è…í|‹Õ‹Æèqûÿÿ„Àt3Ò‹Æè´şÿë+‹×‹Æè-•şÿ€~<u€¾   t
öÃuöÃu	3Ò‹Æè‡şÿYZ]_^[ÃSf‹RfÿÊtfÿÊtëf»õÿèrlıÿ[Ãf»ôÿèglıÿ[ÃU‹ìÄpÿÿÿSVW‹}ƒÇüÇEè+¼  E¨‰Eğ‹è¶Áşÿ‰EìÇE¨   ÇE´   3À‰E¸‹E‹@ø‹@‰EÀ‹èÁşÿ‰E¬‹èƒÁşÿ‰E¼ÇEØ,¼  ‹èrÁşÿ‰EÜE‰EàÇE   ‹è[Áşÿ‰E”3ö‹è#ûÿÿ‹ØE€P‹E‹@ø‹@Pèî›ıÿ‹‹@0‰Eü‹‹@,‰Eø;uüŸ   ‰]˜‹‹€ü   ‹ÿR;Ø}‹‹€ü   ‹Ó‹ÿQ‰E¤‹Eø‰Eœ‹‹€  ‰E ‹E¤‰EÔ‰]°UØ‹èkıÿ‹E ÆP…pÿÿÿP‹Mœ‹Ö3ÀèëıÿVWµpÿÿÿ}Ä¹   ó¥_^Uè‹èXkıÿu C‹‹€ü   ‹ÿR;Ø}	;uüŒaÿÿÿ_^[‹å]Ã@ U‹ìƒÄø‰Uø‰Eü‹Eøƒx t	UèşÿÿYë‹Uø‹Eüè÷­şÿYY]Ã@ S‹Ø‹ÃèB°şÿ‹ÃèÏ÷ÿÿ[ÃSƒÄà‹ØÇD$  Tè¬ıÿL$‹Ô‹Ãèï…şÿD$èb¡ıÿ‰D$3À‰D$3À‰D$T$‹Ã‹ÿQğÇD$  T$‹Ã‹ÿQğƒÄ [Ã@ U‹ìƒÄìSVW3Û‰]ì‹ñ}ğ¹   ó¥‹ò‹Ø3ÀUhOÂB dÿ0d‰ fƒ»*   tEğPŠEP‹Î‹Ó‹ƒ,  ÿ“(  ëEUğ‹ƒ  èöéşÿ‹ƒü   ‹ÿR;ğ}(Mì‹Ö‹ƒü   ‹0ÿV‹EìP‹UğƒÂ‹Mô‹ƒ  èìşÿ3ÀZYYd‰hVÂB Eìè²qıÿÃénıÿëğ_^[‹å]Â Sfƒ¸2   tQ‹Ø‹Ê‹Ğ‹ƒ4  ÿ“0  [ÃSVWUQ‹ğ‹zŠGˆ$‹W‹†  èîşÿ‹VD‹†  è¤ìşÿ‹–¼   ‹†  è«ìşÿƒ |,ö$t&‹†  ‹@º  €èhãşÿ‹†  ‹@º  €è}Şşÿ‹o…í|Š$PO‹Õ‹Æ‹ÿS|ëW‹†  èâèşÿö$tGP‹GPè+›ıÿ3Ò‹†  èvíşÿZ]_^[ÃSV‹ğ‹B‹–  ‰P€¾  uH‹P‹Æ‹ÿ“€   ^[ÃSV‹ò‹Ø€=ÄI  t€»   u‹Ãè£şÿ‹Ö‹ÃèTµşÿ^[ÃSV‹ò‹Ø…Ût"‹Ãè¿  Š ,/t,-t‹Æ¹ÔÃB ‹Óèrıÿ^[Ã‹Æ‹ÓèËpıÿ^[Ãÿÿÿÿ   \   SV‹ò‹ØŠD3ÿPè$–ıÿ…Àt‹Ãèqıÿ;ğ}¸   ^[Ã¸   ^[ÃSVW‹ò‹ø»   ;ó|;óu3Àë‹Ó‹Çè¯ÿÿÿØ;ó}é°_^[Ã@ U‹ìƒÄøSVW3Û‰]ø‰Mü‹ú‹ğ3ÀUh¬ÄB dÿ0d‰ ‹Æèæ  ‹Ø…Ûu‹Eü‹Ï‹ÖèLqıÿë EøP‹ËIº   ‹Æèïrıÿ‹Uø‹Eü‹Ïè*qıÿ3ÀZYYd‰h³ÄB EøèUoıÿÃé³kıÿëğ_^[YY]Ã‹ÀSVWU‹é‹ú‹Ø…ÛtTº   ‹Ãèÿÿÿ‹ğF‹Ãè”pıÿ;ğu€|3ÿ:t‹Ãèr  Š ,/t,-u‹Å‹Ï‹Óè¹pıÿëSh4ÅB W‹Åº   èqıÿë	‹Å‹×èmoıÿ]_^[Ãÿÿÿÿ   \   U‹ìj j SVW‹ò‹Ø3ÀUh’ÅB dÿ0d‰ Uü‹ÆèŠ  ‹EüPUø‹Ãè|  ‹EøZè[¥ıÿ‹Ø3ÀZYYd‰h™ÅB Eøº   ènıÿÃéÍjıÿëë‹Ã_^[YY]Ã‹À3Òè   ÃSVWUQˆ$‹ğ‹Æè·oıÿ‹øƒÿ|YŠ,/t,-uOŠF,/t,-uD»   3í;û|4ŠDÿ,/t,-uEƒı}"C;û|ŠDÿ,/tó,-tïë‹Ó‹ÆèÆıÿÿØ;û}Ì‹ÃHë^ƒÿ|Š,/t,-u€<$ t¸   ëB3Àë>…ÿ~8º   ‹Æè‹ıÿÿ‹ØC;û|%€|ÿ:u€<$ t;û~Š,/t,-uCë‹Ãë3ÀZ]_^[Ã@ SVWUƒÄøˆ$‹ğ²‹Æèÿÿÿ‹ø‰|$‹ÆèÎnıÿ‹è_;ë|3ŠDÿ,/t,-u€<$ t‹ûë‹|$Cë‹Ó‹ÆèıÿÿØ‹ÃH‰D$;ë}Í‹ÇYZ]_^[ÃSVWÄğÿÿPƒÄü‹ú‹ğTD$Ph   ‹Æè)pıÿPè’ıÿ‰Ã…Û~û   }T$‹Ç‹Ëè°mıÿë	‹Ç‹ÖèYmıÿÄ  _^[Ã@ SVWU‹ğ3í‹Æènıÿ‹ø²‹Æè"ÿÿÿ‹ØC;û|€|ÿ.u‹ëCë‹Ó‹ÆèZüÿÿØ;û}å‹Å]_^[ÃSVW‹ú‹Ø3Ò‹Ãèèşÿÿ‹ğW‹Îº   ‹ÃèËoıÿ_^[Ã@ SVW‹ú‹ğ‹Æèâıÿÿ‹Ø…Ûu	‹Çè1lıÿëW‹Ëº   ‹Æè˜oıÿ_^[ÃSVW‹ú‹ğ‹ÆèVÿÿÿ‹Ø…Ûu	‹ÇèlıÿëW¹ÿÿÿ‹Ó‹Æèhoıÿ_^[ÃSVW‹ú‹Ø²‹Ãè`şÿÿ‹ğWV¹ÿÿÿ‹ÃèBoıÿ_^[Ã‹ÀSVW‹ú‹Ø²‹Ãè8şÿÿ‹ğW‹Îº   ‹Ãèoıÿ_^[Ã@ S‹Ø…Ûu3À[Ã‹ÃèşlıÿPSèD•ıÿ[Ã‹ÀSVWUƒÄø‹ê‰$3À‰D$…ítBƒ<$ t<‹ı‹ÅèËlıÿ4;÷v,Š„Ût!‹Ó‹$èp«ıÿ…Àt	‹Ç+Å@‰D$Wèé”ıÿ‹øëG;÷wÔ‹D$YZ]_^[ÃSVW‹ò‹Øƒ=Ü I tRj*èµ–ıÿ…ÀtG‹Æ‹Óèˆkıÿ‹èalıÿ‹ø»   ;û|5‹ŠDÿ¿,s‹Æènıÿ€Dÿ Cë‹‹Óè–úÿÿØ;û}Öë	‹Ö‹ÃèM¡ıÿ_^[ÃSVWU‹ò‹Ø‹Æèlıÿ‹ø½   ;ı|:\.ÿu‹Åë‹Õ‹ÆèSúÿÿè;ı}ç3À]_^[Ã‹ÀSVW‹ò‹Ø‹Æ‹Óèğjıÿ‹èÉkıÿ‹ø»   ;û|$‹€|ÿ/u‹Æè~mıÿÆDÿ\‹‹ÓèúÿÿØ;û}Ü»   ë2‹€|ÿ\u‹€<\uƒû~S‹Æ¹   è²mıÿë‹‹ÓèÇùÿÿØ‹èZkıÿ;Ø|Ã_^[Ã‹ÀS‹Úë„Òu3À[ÃPè…“ıÿŠ:Úuì[ÃSVW‹ú‹ğ‹Æè&kıÿ‹ØëK…Û~PVèc“ıÿŠ ,/të,-tç‹Æèkıÿ;Øu‹Ç‹ÖèjıÿëW‹Ëº   ‹Æèèlıÿ_^[ÃSVWU‹ê‹ğ²‹Æèûÿÿ‹ø‹ÆèÆjıÿ‹ØëK;û}PVè“ıÿŠ ,/të,-tç‹Æè¢jıÿ;Øu‹Å‹ÖèµiıÿëU‹Ëº   ‹Æèˆlıÿ]_^[Ã@ ÿ%è7I ‹Àÿ%ä7I ‹Àÿ%à7I ‹Àÿ%Ü7I ‹Àÿ%Ø7I ‹Àÿ%Ô7I ‹Àÿ%Ğ7I ‹Àÿ%ü7I ‹Àÿ%ø7I ‹Àÿ%ô7I ‹Àÿ%ğ7I ‹À…ÀÀÃ‹À…ÀœÀÃ‹Àÿ%8I ‹Àÿ%8I ‹Àÿ%8I ‹Àÿ%8I ‹Àÿ%$8I ‹Àÿ% 8I ‹Àÿ%8I ‹Àÿ%8I ‹ÀU‹ìj SVW‹Ø3ÀUhèËB dÿ0d‰ Uü‹ÃèÒşÿÿ‹EüènkıÿPè<ıÿ‹Ø3ÀZYYd‰hïËB EüèhıÿÃéwdıÿëğ‹Ã_^[Y]ÃS‹Ø‹Ãè6kK 25
svn:wc:ra_dav:version-url
V 63
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/src
END
translation.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/translation.cpp
END
credits.h
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/credits.h
END
joystick.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/joystick.h
END
course_mgr.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_mgr.cpp
END
event_race_select.cpp
K 25
svn:wc:ra_dav:version-url
V 84
/svnroot/extremetuxracer/!svn/ver/8/trunk/extreme-tuxracer/src/event_race_select.cpp
END
fog.cpp
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/fog.cpp
END
race_select.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/race_select.h
END
screenshot.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/screenshot.h
END
loading.h
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/loading.h
END
bench.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/bench.h
END
hier_util.cpp
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier_util.cpp
END
paused.h
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/paused.h
END
textures.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/textures.h
END
debug.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/debug.h
END
audioconfig.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/audioconfig.h
END
game_config.cpp
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/src/game_config.cpp
END
splash_screen.h
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/splash_screen.h
END
joystick.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/joystick.cpp
END
hier_cb.h
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier_cb.h
END
translation.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/translation.h
END
viewfrustum.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/viewfrustum.h
END
pp_types.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/pp_types.h
END
quadtree.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/quadtree.cpp
END
file_util.cpp
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/file_util.cpp
END
bench.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/bench.cpp
END
event_race_select.h
K 25
svn:wc:ra_dav:version-url
V 82
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/event_race_select.h
END
event_select.cpp
K 25
svn:wc:ra_dav:version-url
V 80
/svnroot/extremetuxracer/!svn/ver/19/trunk/extreme-tuxracer/src/event_select.cpp
END
nmrcl.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/nmrcl.h
END
tex_font_metrics.h
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/tex_font_metrics.h
END
textures.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/textures.cpp
END
snow.cpp
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/snow.cpp
END
audioconfig.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/audioconfig.cpp
END
mirror_course.cpp
K 25
svn:wc:ra_dav:version-url
V 80
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/mirror_course.cpp
END
joystickconfig.h
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/joystickconfig.h
END
hier_cb.cpp
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier_cb.cpp
END
track_marks.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/track_marks.cpp
END
winsys.h
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/winsys.h
END
game_config.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/game_config.h
END
keyframe.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/keyframe.h
END
loop.cpp
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/loop.cpp
END
game_over.h
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/game_over.h
END
nmrcl.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/nmrcl.cpp
END
quadtree.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/quadtree.h
END
render_util.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/render_util.h
END
reset.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/reset.cpp
END
snow.h
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/snow.h
END
fps.h
K 25
svn:wc:ra_dav:version-url
V 68
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/fps.h
END
mirror_course.h
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/mirror_course.h
END
phys_sim.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/phys_sim.h
END
player.h
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/player.h
END
keyframe.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/keyframe.cpp
END
model_hndl.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/model_hndl.cpp
END
game_type_select.cpp
K 25
svn:wc:ra_dav:version-url
V 83
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/game_type_select.cpp
END
configmode.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/configmode.cpp
END
course_mgr.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/course_mgr.h
END
configuration.h
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/configuration.h
END
game_mgr.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/game_mgr.cpp
END
tcl_util.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/tcl_util.h
END
hud.h
K 25
svn:wc:ra_dav:version-url
V 68
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hud.h
END
reset.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/reset.h
END
Makefile.am
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/27/trunk/extreme-tuxracer/src/Makefile.am
END
phys_sim.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/phys_sim.cpp
END
stuff.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/stuff.h
END
viewfrustum.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/viewfrustum.cpp
END
intro.h
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/intro.h
END
Makefile.in
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/27/trunk/extreme-tuxracer/src/Makefile.in
END
gl_util.h
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/gl_util.h
END
course_load.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_load.h
END
configuration.cpp
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/54/trunk/extreme-tuxracer/src/configuration.cpp
END
string_util.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/string_util.h
END
part_sys.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/part_sys.h
END
course_quad.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_quad.cpp
END
tex_font_metrics.cpp
K 25
svn:wc:ra_dav:version-url
V 83
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/tex_font_metrics.cpp
END
os_util.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/10/trunk/extreme-tuxracer/src/os_util.cpp
END
configmode.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/configmode.h
END
tcl_util.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/tcl_util.cpp
END
game_mgr.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/game_mgr.h
END
tux_shadow.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/tux_shadow.cpp
END
graphicsconfig.cpp
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/graphicsconfig.cpp
END
error_util.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/error_util.cpp
END
stuff.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/stuff.cpp
END
lights.cpp
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/lights.cpp
END
intro.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/intro.cpp
END
winsys.cpp
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/winsys.cpp
END
credits.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/src/credits.cpp
END
gl_util.cpp
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/gl_util.cpp
END
course_load.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_load.cpp
END
videoconfig.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/videoconfig.h
END
pp_classes.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/pp_classes.h
END
string_util.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/string_util.cpp
END
game_over.cpp
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/game_over.cpp
END
course_render.h
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_render.h
END
track_marks.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/track_marks.h
END
racing.h
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/racing.h
END
race_select.cpp
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/16/trunk/extreme-tuxracer/src/race_select.cpp
END
render_util.cpp
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/render_util.cpp
END
loading.cpp
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/loading.cpp
END
screenshot.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/screenshot.cpp
END
loop.h
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/loop.h
END
course_quad.h
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_quad.h
END
paused.cpp
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/18/trunk/extreme-tuxracer/src/paused.cpp
END
os_util.h
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/os_util.h
END
fps.cpp
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/fps.cpp
END
hier_util.h
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier_util.h
END
splash_screen.cpp
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/13/trunk/extreme-tuxracer/src/splash_screen.cpp
END
etracer.h
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/60/trunk/extreme-tuxracer/src/etracer.h
END
graphicsconfig.h
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/graphicsconfig.h
END
error_util.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/error_util.h
END
player.cpp
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/player.cpp
END
course_render.cpp
K 25
svn:wc:ra_dav:version-url
V 80
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/course_render.cpp
END
lights.h
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/lights.h
END
racing.cpp
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/13/trunk/extreme-tuxracer/src/racing.cpp
END
view.cpp
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/view.cpp
END
model_hndl.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/src/model_hndl.h
END
game_type_select.h
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/game_type_select.h
END
hud.cpp
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/32/trunk/extreme-tuxracer/src/hud.cpp
END
highscore.h
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/6/trunk/extreme-tuxracer/src/highscore.h
END
file_util.h
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/file_util.h
END
callbacks.h
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/callbacks.h
END
main.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/42/trunk/extreme-tuxracer/src/main.cpp
END
joystickconfig.cpp
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/joystickconfig.cpp
END
event_select.h
K 25
svn:wc:ra_dav:version-url
V 78
/svnroot/extremetuxracer/!svn/ver/14/trunk/extreme-tuxracer/src/event_select.h
END
keyboardconfig.cpp
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/keyboardconfig.cpp
END
hier.h
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier.h
END
part_sys.cpp
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/part_sys.cpp
END
highscore.cpp
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/11/trunk/extreme-tuxracer/src/highscore.cpp
END
view.h
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/view.h
END
callbacks.cpp
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/callbacks.cpp
END
fog.h
K 25
svn:wc:ra_dav:version-url
V 68
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/src/fog.h
END
debug.cpp
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/debug.cpp
END
hier.cpp
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/hier.cpp
END
tux_shadow.h
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/tux_shadow.h
END
keyboardconfig.h
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/src/keyboardconfig.h
END
videoconfig.cpp
K 25
svn:wc:ra_dav:version-url
V 79
/svnroot/extremetuxracer/!svn/ver/10/trunk/extreme-tuxracer/src/videoconfig.cpp
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ntimeClass‹À                        @C    Ä@ ,,@ à*@ +@ LXD TPSRuntimeClassImporterTPSExportedVar      |@     TSpecialProc      |@    ‹ÀSIƒù |Š
Iƒùÿuô[Ã@ SIƒù |Š
öÓ Iƒùÿuò[ÃSIƒù |Š
 Iƒùÿuô[Ã@ U‹ìSV‹òIƒù |ŠŠ"Ú:Út‹EÆ  ëIƒùÿuæ‹EÆ ^[]Â U‹ìSV‹uIƒù |Š:
tÆ ë	IƒùÿuíÆ^[]Â ‹ÀSV‹Ù‹Èƒá¾   ÓæÁè¶#ğ•^[ÃSV‹Ú‹ğ‹Ã‹Öè%ıÿë‹Ã¹   º   è6ıÿ‹èëıÿ…À~‹€8 tİë‹è×ıÿ‹Ğ‹Ã¹   èıÿ‹èÂıÿ…À~‹è·ıÿ‹€|ÿ tĞ^[Ã@ U‹ìS‹Øf‹EPÿuÿu‹Ãè[ıÿ[]Â @ U‹ìƒÄøSVW3Û‰]ø‹ñ‹Ú‰Eü‹Eüèıÿ3ÀUhUC dÿ0d‰ ‹EüèUıÿ‹Ó+ĞMø° èÿıÿ‹Uø‹Æ‹Müè†ıÿ3ÀZYYd‰h\C Eøº   èÌıÿÃé
ıÿëë_^[YY]ÃU‹ìƒÄøSVW3Û‰]ø‹ñ‹Ú‰Eü‹Eüè§ıÿ3ÀUhÍC dÿ0d‰ ‹Eüèİıÿ‹Ó+ĞMø°0è‡ıÿ‹Uø‹Æ‹Müèıÿ3ÀZYYd‰hÔC Eøº   èTıÿÃé’ıÿëë_^[YY]ÃU‹ìƒÄøSVW3Û‰]ø‹ñ‹Ú‰Eü‹Eüè/ıÿ3ÀUhEC dÿ0d‰ ‹Eüèeıÿ‹Ó+ĞMø° èıÿ‹Mø‹Æ‹Uüè–ıÿ3ÀZYYd‰hLC Eøº   èÜıÿÃéıÿëë_^[YY]ÃU‹ìj j SVW‹ò‹Ø3ÀUh~ C dÿ0d‰ ‹Æ‹Óèıÿ¿   3Ûé®   ‹€|8ÿ'u(„Ûu³‹Ö‹Ï¸” C è^ıÿG‹Ö‹Ï¸” C èOıÿƒÇë}‹€|8ÿ sZUø‹¶D8ÿèbòÿÿ‹MøEüº  C èâıÿ‹Æ¹   ‹×èÌıÿ„Ût3Û‹Ö‹Ï¸” C è ıÿG‹Ö‹Ï‹Eüèóıÿ‹Eüè_ıÿøë„Ûu³‹Ö‹Ï¸” C èÓıÿƒÇëG‹è:ıÿ;øCÿÿÿ„Ût‹Æº” C è*ıÿƒ> u‹Æº¬ C è1ıÿ3ÀZYYd‰h… C Eøº   è£ıÿÃéáıÿëë_^[YY]Ãÿÿÿÿ   '   ÿÿÿÿ   #   ÿÿÿÿ   ''  SVW‹ú‹ğ‹Ç‹ÖèÔıÿ‹Æè­ıÿ…À~$»   ŠTÿ€ê sW‹ËIº   ‹ÆèıÿëCHuá_^[ÃU‹ìÄôşÿÿSVW3Û‰ôşÿÿ‰]ø‹ù‰Uü‹ğ‹Eüèıÿ3ÀUhh#C dÿ0d‰ ƒ}ü …æ  •øşÿÿ‹èa	ıÿ•øşÿÿ‹ÇèÔıÿéñ  ‹Uü¸€#C èşıÿ‹Ø…ÛuUø‹EüèñûÿÿEüè‰ıÿë4…ôşÿÿP‹ËIº   ‹Eüèèıÿ‹…ôşÿÿUøèÂûÿÿEü‹Ëº   èıÿ‹èt
ıÿ‹Uøèìyıÿ‹Ø…Ûu‹ÇºŒ#C èÆıÿéo  ‹¶ ƒø‡  ÿ$…è!C ø"C "C 1"C \"C u"C ‘"C Á"C Ï"C Ü"C ø"C ‘"C ø"C ê"C ‹Ó‹Æèozıÿ‹×èğÿÿé  ‹Ó‹ÆèZzıÿ•ôşÿÿèëïÿÿ‹ôşÿÿ‹Çº¨#C èiıÿéæ   ‹Ó‹Æè/zıÿ‹Ğ‹Ï‹è´xıÿéÍ   ‹Ó‹ÆèŠ|ıÿƒÄôÛ<$›‹Çè@ûÿÿé±   h´#C ôşÿÿ‹Ó‹Æèã{ıÿÿµôşÿÿh´#C ‹Çº   èxıÿé   ‹ÇºÀ#C èÇıÿës‹Ó‹Æè¼yıÿ‹ğë*‹ÇºĞ#C è¬ıÿëX‹Çºä#C èıÿëJ‹Çºø#C èıÿë<…öu‹Çº$C è~ıÿë*‹EüèTıÿ…À)şÿÿ•øşÿÿ‹èkıÿ•øşÿÿ‹ÇèŞıÿ3ÀZYYd‰ho#C …ôşÿÿè¦ıÿEøº   è¹ıÿÃé÷ıÿëà_^[‹å]Ã  ÿÿÿÿ   .   ÿÿÿÿ   Unknown Identifier  ÿÿÿÿ   #   ÿÿÿÿ   '   ÿÿÿÿ   [Set]   ÿÿÿÿ   [Method]    ÿÿÿÿ	   [Variant]   ÿÿÿÿ	   [Unknown]   ÿÿÿÿ   nil SVWQ‹ñ‰$‹Ø‹CŠ@<u$‹‹8…ÿu‹Æºt$C è[ıÿë,‹Î‹Ç‹$è­üÿÿë<u‹Æº€$C è;ıÿë‹Æº”$C è-ıÿZ_^[Ãÿÿÿÿ   nil ÿÿÿÿ	   Interface   ÿÿÿÿ   Invalid Type    U‹ìÄìşÿÿSVW3Û‰]ø‰Mü‹ò‹Ø3ÀUh)C dÿ0d‰ ‹;…ÿu‹Eüº()C è·ıÿé  ‹C€x…¹   3ÒUh{%C dÿ2d‰"f‹fƒø	u‹Eüº4)C èıÿëYfƒøu‹EüºP)C èlıÿëDfƒøuEø‹×è®ıÿ‹Eø‹Uüèúÿÿë'f= uEø‹×è‘ıÿ‹Eø‹Uüèòùÿÿë
‹Eü‹×èzıÿ3ÀZYYd‰é€  é´	ıÿ   Xc@ Œ%C ‰Ã‹K‹Eüº`)C è"ıÿèéıÿéS  ‹{3ÀŠGƒø‡5  ÿ$…À%C î(C M&C p&C “&C ¶&C Ù&C û&C 'C F'C o'C ™'C U(C Æ'C Ñ(C ™'C î(C î(C î(C ª'C î(C î(C ,&C Æ'C î(C î(C à(C à(C Uø‹‹ èøëÿÿ‹Mø‹Eüºt)C èxıÿé®  •øşÿÿ‹¶ èoıÿ•øşÿÿ‹Eüèµıÿé‹  •øşÿÿ‹¾ èLıÿ•øşÿÿ‹Eüè’ıÿéh  •øşÿÿ‹· è)ıÿ•øşÿÿ‹EüèoıÿéE  •øşÿÿ‹¿ èıÿ•øşÿÿ‹EüèLıÿé"  •øşÿÿ‹‹ èäıÿ•øşÿÿ‹Eüè*ıÿé   •øşÿÿ‹‹ èÂıÿ•øşÿÿ‹EüèıÿéŞ  ‹Ù ƒÄôÛ<$›…øşÿÿè))ıÿ•øşÿÿ‹Eüèßıÿéµ  ‹İ ƒÄôÛ<$›…øşÿÿè )ıÿ•øşÿÿ‹Eüè¶ıÿéŒ  ‹f‹PRÿpÿ0…øşÿÿèÖ(ıÿ•øşÿÿ‹EüèŒıÿéb  ‹Uü‹‹ è¯÷ÿÿéQ  Eø‹ŠèZıÿ‹Eø‹Uüè“÷ÿÿé5  ‹Eüè2ıÿ‹C€xu‹Gë‹×‹‹ èA9  ‹ğN…ö|FF3ÿ‹Eüƒ8 t‹Eüº„)C èvıÿìşÿÿ‹Ã‹×èw …ìşÿÿMø3Òèüÿÿ‹Uø‹EüèLıÿGNu½h)C ‹Eüÿ0hœ)C ‹Eüº   èäıÿé¦   ‹Eüè£ıÿ‹C‹@‹@‹ğN…ö|FF3ÿ‹Eüƒ8 t‹Eüº„)C è÷ıÿìşÿÿ‹Ã‹×è| …ìşÿÿMø3Òè üÿÿ‹Uø‹EüèÍıÿGNu½h¨)C ‹Eüÿ0h´)C ‹Eüº   èeıÿë*‹EüºÀ)C è¶ıÿë‹Mü‹Ã‹Öè$ûÿÿë‹EüºÌ)C è™ıÿ3ÀZYYd‰h)C Eøèğ
ıÿÃéNıÿëğ_^[‹å]Ã ÿÿÿÿ   nil ÿÿÿÿ   Variant(IDispatch)  ÿÿÿÿ   Null    ÿÿÿÿ   Exception:  ÿÿÿÿ   Proc:   ÿÿÿÿ   ,   ÿÿÿÿ   [   ÿÿÿÿ   ]   ÿÿÿÿ   (   ÿÿÿÿ   )   ÿÿÿÿ   Nil ÿÿÿÿ	   [Invalid]   U‹ìj SVW‹Ù‹ò3ÒUh%,C dÿ2d‰"ƒàƒø‡  ÿ$…*C i*C z*C ˜*C ©*C º*C Ë*C Ü*C í*C ş*C +C  +C 1+C B+C S+C d+C u+C †+C ”+C ¢+C Ì+C Ú+C °+C ¾+C ê+C ø+C ‹Ãº<,C è
ıÿé•  Uü‹Æè,öÿÿ‹Mü‹ÃºP,C è-ıÿéw  ‹Ãºh,C èğ	ıÿéf  ‹Ãº€,C èß	ıÿéU  ‹Ãº˜,C èÎ	ıÿéD  ‹Ãº°,C è½	ıÿé3  ‹ÃºÈ,C è¬	ıÿé"  ‹Ãºì,C è›	ıÿé  ‹Ãº-C èŠ	ıÿé   ‹Ãº(-C èy	ıÿéï   ‹ÃºD-C èh	ıÿéŞ   ‹Ãº\-C èW	ıÿéÍ   ‹Ãºx-C èF	ıÿé¼   ‹Ãº-C è5	ıÿé«   ‹Ãº°-C è$	ıÿéš   ‹ÃºÈ-C è	ıÿé‰   ‹Ãºà-C è	ıÿë{‹Ãºô-C èôıÿëm‹Ãº.C èæıÿë_‹Ãº4.C èØıÿëQ‹ÃºT.C èÊıÿëC‹Ãºp.C è¼ıÿë5‹Ã‹Îºˆ.C èØ	ıÿë%‹Ãºœ.C èıÿë‹Ã‹Öè“ıÿë‹Ãº¼.C è…ıÿ3ÀZYYd‰h,,C EüèÜıÿÃé:ıÿëğ_^[Y]Ã  ÿÿÿÿ   No Error    ÿÿÿÿ   Cannot Import   ÿÿÿÿ   Invalid Type    ÿÿÿÿ   Internal error  ÿÿÿÿ   Invalid Header  ÿÿÿÿ   Invalid Opcode  ÿÿÿÿ   Invalid Opcode Parameter    ÿÿÿÿ   no Main Proc    ÿÿÿÿ   Out of Global Vars range    ÿÿÿÿ   Out of Proc Range   ÿÿÿÿ   Out Of Range    ÿÿÿÿ   Out Of Stack Range  ÿÿÿÿ   Type Mismatch   ÿÿÿÿ   Unexpected End Of File  ÿÿÿÿ   Version error   ÿÿÿÿ   divide by Zero  ÿÿÿÿ
   Math error  ÿÿÿÿ   Could not call proc ÿÿÿÿ   Out of Record Fields Range  ÿÿÿÿ   Null Pointer Exception  ÿÿÿÿ   Null variant error  ÿÿÿÿ   Out Of Memory   ÿÿÿÿ   Exception:  ÿÿÿÿ   Interface not supported ÿÿÿÿ   Unknown error   3ÒŠPƒú‡¤   Š’ç.C ÿ$•/C 
	        ~/C ./C 6/C >/C F/C N/C V/C ^/C f/C n/C v/C Ç@   ÃÇ@   ÃÇ@   ÃÇ@   ÃÇ@   ÃÇ@   ÃÇ@   ÃÇ@   ÃÇ@
   ÃÇ@   Ã3Ò‰PÃSVW„ÒtƒÄğèıüÿ‹ñ‹Ú‹ø3Ò‹Çèûüÿ‹Î²¸¤C èÄ3 ‰G„Ût
d    ƒÄ‹Ç_^[ÃSV‹Ú‹ğ‹Fè‚ûüÿ3Ò‹Æèmûüÿ„Ût‹Æè‚ıüÿ‹Æ^[ÃSVW‹Ø‹ÃèĞşÿÿ‹s‹VJ‹Æèªäÿÿ‹p‹{‹WJ‹Çè™äÿÿğ‰s_^[ÃSV„ÒtƒÄğèşüüÿ‹Ú‹ğ3Ò‹ÆèEÿÿÿ²¸°C è}âÿÿ‰F²¸°C ènâÿÿ‰F„Ût
d    ƒÄ‹Æ^[ÃSV‹Ú‹ğ‹FèÚúüÿ‹FèÒúüÿ3Ò‹Æè9ÿÿÿ„Ût‹ÆèÒüüÿ‹Æ^[ÃSVWUQ‹ò‹Ø3ÀŠFƒø‡S  ÿ$…¾0C 
2C *1C *1C 21C 21C <1C <1C <1C ‡1C 1C <1C «1C <1C E1C <1C 
2C 1C 
2C *1C 
2C 
2C `1C ß1C ş1C {1C <1C <1C Æ éØ   fÇ  éÎ   3À‰éÅ   3À‰‹Ã‹ĞƒÂ3É‰
ƒÀ3Ò‰éª   3À‰‹Ã‹ĞƒÂ3É‰
ƒÀ3Ò‰é   3À‰‰Céƒ   3À‰‰Cëz3À‰‰Cf‰Cëm‹Ãº„@ è[ıÿë_‹F‹hM…í|TEÇ$    ‹F‹$èëâÿÿ‹ø‹×‹ÃèÌşÿÿ_ÿ$Muáë+‹Æ‹x‹hM…í|E‹×‹Ãèªşÿÿ_Muñë‹V‹Ã3Éè*÷üÿZ]_^[ÃU‹ìƒÄìSVW‰Uü‹Ø‹Eü¶@ƒÀöƒø‡Õ  Š€>2C ÿ$…O2C           4C o2C {2C 2C Æ2C ë2C ˆ3C Ó3C ‹ÃèŠıÿé‹  ‹…ÀtP‹ ÿP3À‰év  3ÀUh·2C dÿ0d‰ ‹Ãº„@ èıÿ3ÀZYYd‰éO  é´ûüÿèËşüÿé@  ‹Ã‹ĞƒÂƒ: „0  ƒÀ‹‹èJ  3À‰é  ‹…À„  ƒè‰Eì‹Eìƒ8 Œÿ   ‹Eìÿ‹Eìƒ8 …î   ‹Eü‹p‹F‰Eø‹EìƒÀ‰Eì‹Eì‹ ‰Eğ‹EìƒÀ‰EìŠFö,r,t,u‹}ğO…ÿ|G‹Ö‹Eìè®şÿÿ‹EìEø‰EìOuê‹Uğ¯UøƒÂ‹ƒèèŞòüÿ3À‰ë~‹Eü‹@‹xO…ÿ|pGÇEô    ‹Eü‹@‹Uôèáÿÿ‹ğŠFö,r,	t,u	‹Ö‹ÃèHşÿÿ^ÿEôOuÍë3‹Eü‹pŠV€Âö€êr
€ê	t€êu‹xO…ÿ|G‹Ö‹Ãèşÿÿ^Ouñ_^[‹å]Ã@ SV‹Ø‹Cè,òüÿ‹ğ‹Ó‹Æèyüÿÿ‹Æ^[ÃSV‹ò‹Ø…Ût(ŠFö,r,t,t,u	‹Ö‹Ãè¼ıÿÿ‹V‹Ãèòüÿ^[Ã@ SV‹Ø‹CƒÀèÕñüÿ‹ğ‰‹ÆƒÀ‹Óèüÿÿ‹Æ^[ÃS‹Ø…Ût6‹ŠP€Âö€êr€êt
€êt€êu‹ÓƒÂ’èZıÿÿ‹‹PƒÂ‹Ãè›ñüÿ[ÃSV‹ğ‹^Kƒû |‹Ó‹ÆèÖßÿÿè¥ÿÿÿKƒûÿuì‹Æèhöüÿ^[ÃSVW‹ø€Lu[‹GD‹ÿ‹G@‹ÿ‹G<‹pNƒş |A‹Ö‹G<è	1 ‹Ø‹Š@ö,r,t,t,u‹ÃƒÀ‹èÈüÿÿ‹ÃƒÀ‹èLûÿÿNƒşÿu¿_^[Ã‹ÀSVW‹ğ‹F(‹XKƒû |‹F(‹@‹˜º   èÜğüÿKƒûÿuç‹F,‹XKƒû |&‹F,‹@‹<˜‹‹È‹W‹ÆÿÑº   ‹ÇèªğüÿKƒûÿuÚ‹F0‹XKƒû |‹F0‹@‹˜ºXC è¹ıÿKƒûÿuç‹F8‹XKƒû |‹F8‹@‹˜²‹ÿQüKƒûÿuê‹F8‹ÿ‹F<‹ÿ‹FD‹ÿ‹F4‹XKƒû |‹F4‹@‹˜è&õüÿKƒûÿuì‹F4‹ÿÆFL ‹F,‹ÿ‹F0‹ÿ‹F(‹ÿÇF`ÿÿÿÿ_^[ÃSVW„ÒtƒÄğè½öüÿ‹Ú‹ø3Ò‹Çè°ôüÿ²¸°C è<Üÿÿ‰G$²¸°C è-Üÿÿ‰G(ÆG ²¸°C èÜÿÿ‰G,²¸°C èÜÿÿ‰G4²¸°C èüÛÿÿ‰G8²¸`C èQ. ‰G<²¸`C èB. ‰G@3À‰GHÆGL ²¸°C èÆÛÿÿ‰Gl²¸°C è·Ûÿÿ‰G0²¸°C è¨Ûÿÿ‰Gh‹ÇèêÉ  ‹Ï²¸˜C èLøÿÿ‹ğ‰wÆF ‹Æ‹ÿ‹Ï²¸˜C è/øÿÿ‹ğ‰wÆF‹Æ‹ÿ‹Ï²¸üC èøÿÿ‹ğ‰w ÆF‹Æ‹ÿ‹G ‹W‰P²¸`C è–- ‰GD„Ût
d    ƒÄ‹Ç_^[Ã@ SVWU‹Ú‹ğ‹Æ‹ÿR‹Fè‘óüÿ‹Fè‰óüÿ‹F èóüÿ‹Fp…Àtèuóüÿ‹Fh‹hMƒı |‹Fh‹@‹¨ºxC è’ıÿMƒıÿuç‹F,èHóüÿ‹F0è@óüÿ‹F@è8óüÿ‹FDè0óüÿ‹F<è(óüÿ‹F8è óüÿ‹F4èóüÿ‹Fhèóüÿ‹Fl‹hMƒı |.‹Fl‹@‹<¨ƒ¿   t
‹×‹Æÿ—  º  ‹ÇèäíüÿMƒıÿuÒ‹FlèÎòüÿ‹F(èÆòüÿ‹F$‹hMƒı |‹Õ‹F$èÜÿÿè«òüÿMƒıÿuë‹F$èòüÿ3Ò‹Æèˆòüÿ„Ût‹Æèôüÿ‹Æ]_^[Ã‹ÀU‹ìQSVW‹Ø‰St‰KxŠEˆC|ƒ€   ‹UèUûüÿ‹Cp…ÀtèQòüÿ‹E‰Cp€} „è   ‹C(‹@Hƒø ŒÎ   ‰Eü‹Uü‹C(èÛÿÿ‹ğ‹CD‹@‹V;ÂvH‹ú+øO‹CDè@- Guõƒ> uº   ‹Æèøìüÿ‹C(èpÚÿÿÆCLé†   ‹‰CP‹@‰CT‹CP‹@‰CX‹F‰C`‹Fƒøÿt‰C\ÇFÿÿÿÿëX‹Fƒøÿtƒøşt‰C\ÇFşÿÿÿë?‹Fƒøÿt‰C\ÇFÿÿÿÿë+º   ‹Æè€ìüÿ‹C(èøÙÿÿÿMüƒ}üÿ…5ÿÿÿ€{L tÆCL_^[Y]Â U‹ìƒÄôSVW3É‰Mô‰Uü‹ø3ÀUh•:C dÿ0d‰ Eô‹Uüèìúüÿ‹EôèÄÖÿÿ‰Eø‹_Kƒû |5‹G‹4˜‹†   ;Eøu‹Æ‹Uü3ÉŠAè\îüÿu‹Ó‹ÇèAÚÿÿ‹ØëKƒûÿuË3Û3ÀZYYd‰hœ:C EôèlùüÿÃéÊõüÿëğ‹Ã_^[‹å]Ã@ U‹ìƒÄğSVW3Û‰]ô‰Mø‹Ú‰Eü3ÀUh¶;C dÿ0d‰ €; …   Eô‹Uø‹Rè±ùüÿEôP‹Uô¸Ğ;C èhıüÿ‹ÈIº   ‹Eôètüüÿ‹EôèôÕÿÿ‰Eğ‹Eü‹@h‹pNƒş |C‹Ö‹Eü‹@hè‰Ùÿÿ‹Øƒ{ t‹C;Eğu ‹C‹Uôè>ûüÿu‹K‹Uø‹Eüÿ„Àt³ëENƒşÿu½3Ûë;‹Ó‹Eü‹@lè–şÿÿ…Àu3Ûë&‹  ‹Mø‰Q‹  ‹Mø‰Q‹€  ‹Uø‰B³3ÀZYYd‰h½;C EôèKøüÿÃé©ôüÿëğ‹Ã_^[‹å]Ã  ÿÿÿÿ   :   U‹ìj SVW‹ù‹Ú‹ğ3ÀUhc<C dÿ0d‰ Uü‹ÃèêÙÿÿ¸  èDêüÿ‹Ø‹Ã‹Uü¹ÿ   èóüüÿ‹EüèãÔÿÿ‰ƒ   ‰»  3À‰ƒ  ‹E‰ƒ  ‹E‰ƒ  ‹Ó‹FlèK×ÿÿ3ÀZYYd‰hj<C Eüè÷üÿÃéüóüÿëğ‹Ã_^[Y]Â U‹ìSV‹Ú‹ğ‹E‹@øèìøüÿ‹U‹RüÓ;Â|"‹E‹@ü‹U‹Rø‹Ö‹ËèŒêüÿ‹EXü°ë3À^[]ÃU‹ìƒÄìSVW3À‰Eø3ÀUh[BC dÿ0d‰ ‹E‹@PEüº   èˆÿÿÿY„Àt‹E‹@‹@øèuøüÿ‹U‹R+Bü;Eü}‹E‹@‹@ô²
èã  3Ûé!  Eø‹Uüèuûüÿ‹E‹@PEøèúüÿ‹Uüè.ÿÿÿY„Àu‹E‹@‹@ô²
è¥  3Ûéã  ‹E‹@PEìº   èşşÿÿY„Àu‹E‹@‹@ô²
èu  3Ûé³  ‹E‹@üè³% ‹ğF‹Uøè¦öüÿ‹FèNÓÿÿ‰F‹}ìO…ÿŒø  G‹E‹@PEôº   èşÿÿY„Àt‹E‹@‹@ô‹@4‹@;Eôw‹E‹@‹@ô²
è  3Ûé?  ‹E‹@‹@ô‹@4‹UôèšÖÿÿ‹Ğ‹Æèµ$ ‹Ø‹¶@ƒø‡`  Š€A>C ÿ$…Z>C  
   
      	”AC †>C Á>C ü>C 7?C ¤?C @C Z@C •@C Ğ@C AC ‹E‹@P‹‹PCèÚıÿÿY„À…  ‹E‹@‹@ô²
èM  ‹ÃèÎõÿÿ3Ûé„  ‹E‹@PCº   èŸıÿÿY„À…Ô  ‹E‹@‹@ô²
è  ‹Ãè“õÿÿ3ÛéI  ‹E‹@PCº   èdıÿÿY„À…™  ‹E‹@‹@ô²
è×  ‹ÃèXõÿÿ3Ûé  ‹E‹@‹@ô‹@\ƒÀ‹U‹R‹Rô;BX|‹E‹@‹@ô²
èœ  ‹Ãèõÿÿ3ÛéÓ  ‹E‹@‹@ô‹@\‹U‹R‹Rô‹RT‹ ‰C‹E‹@‹@ôƒ@\é  ‹E‹@‹@ô‹@\ƒÀ‹U‹R‹Rô;BX|‹E‹@‹@ô²
è/  ‹Ãè°ôÿÿ3Ûéf  ‹E‹@‹@ô‹@\‹U‹R‹Rô‹RT‹ ‰C…Àu
3À‰C3À‰C‹E‹@‹@ôƒ@\é“  ‹E‹@PCº   èAüÿÿY„À…v  ‹E‹@‹@ô²
è´  ‹Ãè5ôÿÿ3Ûéë  ‹E‹@PCº   èüÿÿY„À…;  ‹E‹@‹@ô²
èy  ‹Ãèúóÿÿ3Ûé°  ‹E‹@PCº
   èËûÿÿY„À…   ‹E‹@‹@ô²
è>  ‹Ãè¿óÿÿ3Ûéu  ‹E‹@PCº   èûÿÿY„À…Å   ‹E‹@‹@ô²
è  ‹Ãè„óÿÿ3Ûé:  ‹E‹@PEüº   èUûÿÿY„Àu‹E‹@‹@ô²
èÌ  ‹ÃèMóÿÿ3Ûé  ‹E‹@‹@ôƒ@\C‹UüèJ÷üÿ‹E‹@PCèÛõüÿ‹UüèûÿÿY„Àu<‹E‹@‹@ô²
èz  ‹Ãèûòÿÿ3Ûé±   ‹E‹@‹@ô²è\  ‹Ãèİòÿÿ3Ûé“   O…	üÿÿ‹Fè?Ïÿÿ‰Eğ‹E‹@‹@ô‹@$‹XKƒû |j‹E‹@‹@ô‹@$‹@‹˜‹@;EğuJ‹E‹@‹@ô‹@$‹@‹˜‹@‹Vètôüÿu+‹E‹@‹@ô‹@$‹@‹<˜‹E‹@‹@ô‹Î‹VÿW„Àu3ÛëKƒûÿu–³3ÀZYYd‰hbBC Eøè¦ñüÿÃéîüÿëğ‹Ã_^[‹å]ÃU‹ìƒÄøS‰Eü‹EPEøº   èíùÿÿY„Àu‹E‹@ô²
èg  3Àë‹]øK…Û|CUèúÿÿY„Àu3ÀëKuî°[YY]ÃU‹ìQSVW‹ğ3ÿ‹F‹XK…Û|&CÇEü    ‹×‹Fè°Ğÿÿ‹Uü‹FèÁÑÿÿxÿEüKuâ°_^[Y]ÃU‹ìƒÄôSVW³‹E‹xàO…ÿŒ–  G‹EPEÿº   èDùÿÿY„Àu‹E‹@ô²è¾  3Ûék  öEÿ€t
ÆEş€mÿ€ëÆEş 3ÀŠEÿƒø‡|  Š€xCC ÿ$…“CC      çHC ·CC æCC DC 6EC –EC 4FC /GC ÏGC ‹E‹Hô²¸˜C è»ëÿÿ‹ğŠEÿˆF‹E‹@ô‹@4‹Öè·Ïÿÿé  ‹E‹Hô²¸PC èŒëÿÿ‹ğ‹EPEôº   èiøÿÿY„Àt	}ôÿ   ~‹Æè8çüÿ‹E‹@ô²èÓ  3Ûé€  F‹Uôèeôüÿ‹EPFèùòüÿ‹Uôè!øÿÿY„Àu‹Æèùæüÿ‹E‹@ô²è”  3ÛéA  ŠEÿˆF‹E‹@ô‹@4‹ÖèÏÿÿém  ‹E‹Hô²¸ğC èäêÿÿ‹ğ‹EPEôº   èÁ÷ÿÿY„Àt	}ôÿ   ~‹Æèæüÿ‹E‹@ô²è+  3ÛéØ  F‹Uôè½óüÿ‹EPFèQòüÿ‹Uôèy÷ÿÿY„Àu‹ÆèQæüÿ‹E‹@ô²èì  3Ûé™  ŠEÿˆF‹E‹@ô‹@4‹ÖègÎÿÿéÅ  ‹E‹Hô²¸¬C è<êÿÿ‹ğ‹EPFº   è÷ÿÿY„Àu‹Æèñåüÿ‹E‹@ô²èŒ  3Ûé9  ŠEÿˆF‹E‹@ô‹@4‹ÖèÎÿÿée  ‹E‹Hô²¸˜C èÜéÿÿ‹ğ‹EPEôº   è¹öÿÿY„Àu‹Æè‘åüÿ‹E‹@ô²è,  3ÛéÙ  }ô   ~‹Æèmåüÿ‹E‹@ô²è  3Ûéµ  ‹Æ‹Uô‰P‹VÁê‰Pö@tÿ@ŠEÿˆF‹E‹@ô‹@4‹ÖèiÍÿÿéÇ  ‹E‹Hô²¸HC è>éÿÿ‹ğ‹EPEôº   èöÿÿY„Àu‹Æèóäüÿ‹E‹@ô²è
  3Ûé;  ‹E‹@ô‹@4‹@;Eôw‹ÆèÇäüÿ‹E‹@ô²èb
  3Ûé  ‹E‹@ô‹@4‹UôèşÍÿÿ‰F‹EPEôº   èªõÿÿY„Àu‹Æè‚äüÿ‹E‹@ô²è
  3ÛéÊ  }ôÿÿÿ~‹Æè^äüÿ‹E‹@ô²èù	  3Ûé¦  ‹Eô‰FŠEÿˆF‹E‹@ô‹@4‹ÖènÌÿÿéÌ  ‹E‹Hô²¸üC èCèÿÿ‹ğ‹EPEôº   è õÿÿY„Àu‹Æèøãüÿ‹E‹@ô²è“	  3Ûé@  ‹E‹@ô‹@4‹@;Eôw‹ÆèÌãüÿ‹E‹@ô²èg	  3Ûé  ŠEÿˆF‹E‹@ô‹@4‹UôèıÌÿÿ‰F‹E‹@ô‹@4‹ÖèÎËÿÿé,  ‹E‹Hô²¸àC èCèÿÿ‹ğ‹EPEôº   è€ôÿÿY„Àtƒ}ô u‹ÆèRãüÿ‹E‹@ô²èí  3Ûéš  ƒ}ô …   ‹EPEøº   è?ôÿÿY„Àu‹Æèãüÿ‹E‹@ô²è²  3Ûé_  ‹E‹@ô‹@4‹@;Eøw‹Æèëâüÿ‹E‹@ô²
è†  3Ûé3  ‹E‹@ô‹@4‹Uøè"Ìÿÿ‹Ğ‹FèüÊÿÿÿMôƒ}ô {ÿÿÿ‹Æèúÿÿ„Àu‹Æèâüÿ‹E‹@ô²è8  3Ûéå   ŠEÿˆF‹E‹@ô‹@4‹Öè³Êÿÿë3Û‹E‹@ô²è
  é¹   €}ş „„   ‹EPEôº   è^óÿÿY„Àu‹E‹@ô²èØ  3Ûé…   }ô   @~‹E‹@ô²è»  3ÛëkF‹UôèPïüÿ‹EPFèäíüÿ‹UôèóÿÿY„Àu‹E‹@ô²è†  3Ûë6‹FèzÇÿÿ‰F‹Æ‹ÿ‹EƒxÜ|‹EP‹FèÈøÿÿY„Àu3ÛëO…kùÿÿ‹Ã_^[‹å]Ã@ U‹ìÄèşÿÿSVW3À‰Eø3ÀUhNC dÿ0d‰ ³‹E‹@äH…ÀŒ
  @‰Eè‹EPEÿº   èoòÿÿY„Àu‹E‹@ô²èé  3ÛéÜ  öEÿ„r  ‹E‹Hô²¸ĞC èú ‹ğ‹EPE÷º   è'òÿÿY„Àu‹Æèÿàüÿ‹E‹@ô²èš  3Ûé  3ÒŠU÷Eøè*îüÿ‹EPEøè¾ìüÿ3ÒŠU÷èäñÿÿY„Àu‹Æè¼àüÿ‹E‹@ô²èW  3ÛéJ  F‹Uøè•éüÿŠEÿ$<ur‹EPEğº   èŸñÿÿY„Àt‹E‹@øèêüÿ‹U+Bü;Eğ}‹Æèaàüÿ‹E‹@ô²èü  3Ûéï  Eø‹Uğèíüÿ‹EPEøè"ìüÿ‹UğèJñÿÿYF‹Uøèéüÿ…èşÿÿ‹V¹ÿ   è¿íüÿ•èşÿÿ‹E‹@ô‹Î‹8ÿW„À…[  ƒ~ t‹N‹E‹@ô²è6 ë‹N‹E‹@ô²è$ ‹ÆèÅßüÿ3Ûé`  ‹E‹Hô²¸4C èˆ ‹ğ‹EPEğº   èµğÿÿY„Àu‹Æèßüÿ‹E‹@ô²è(  3Ûé  ‹EPEìº   è„ğÿÿY„Àu‹Æè\ßüÿ‹E‹@ô²è÷  3Ûéê  ƒ}ğ |+‹E‹@øèSéüÿ;Eğ~‹E‹@øèCéüÿ‹UğUì;Â|ƒ}ì u‹Æèßüÿ‹E‹@ô²è«  3Ûé  ‹EìèäÙüÿ‹ş‰G‹E‹@ø‹UğÂ‹V‹Mìè¹Úüÿ‹Eì‰GöEÿ„0  ‹EPEìº   èÔïÿÿY„Àu‹Æè¬Şüÿ‹E‹@ô²èG  3Ûé:  }ì   @~‹ÆèˆŞüÿ‹E‹@ô²è#  3Ûé  G‹Uìèµëüÿ‹EPGèIêüÿ‹UìèqïÿÿY„Àu‹ÆèIŞüÿ‹E‹@ô²èä  3Ûé×   ‹EPEìº   è@ïÿÿY„Àu‹ÆèŞüÿ‹E‹@ô²è³  3Ûé¦   }ì   @~‹Æèôİüÿ‹E‹@ô²è  3Ûé‚   G‹Uìè!ëüÿ‹EPGèµéüÿ‹UìèİîÿÿY„Àu‹Æèµİüÿ‹E‹@ô²èP  3ÛëF‹GèDÃÿÿ‰GöEÿt‹EP‹Fè›ôÿÿY„Àu‹Æè{İüÿ3Ûë‹E‹@ô‹@8‹Öè§ÅÿÿÿMè…úûÿÿ3ÀZYYd‰hNC EøèñåüÿÃéOâüÿëğ‹Ã_^[‹å]ÃU‹ìƒÄğSVW³‹E‹pèN…öŒ’  F‹EPEğº   è(îÿÿY„Àu‹E‹@ô²è¢  3Ûég  ‹E‹@à;Eğw‹E‹@ô²èƒ  3ÛéH  ‹E‹@ô‹@4‹@‹Uğ‹‹E‹@ô‹@<èP …Àu‹E‹@ô²èK  3Ûé  öEô„ÿ   ‹EPEüº   èíÿÿY„Àu‹E‹@ô²è  3ÛéÜ   ºXC ¸   èìüÿ‰Eø3ÀUh˜OC dÿ0d‰ ‹}ø‹Ç‹Uüè‡éüÿ‹EP‹Çèèüÿ‹UüèDíÿÿY„Àu&ºXC ‹EøèRìüÿ‹E‹@ô²è±  3Û3ÀZYYd‰ëq‹Eø‹ è›Áÿÿ‹Uø‰B‹E‹@ô‹@<‹@‹Uø‰B‹E‹@ô‹@0‹Uøè
Äÿÿ3ÀZYYd‰ë-éÓŞüÿºXC ‹Eøèêëüÿ‹E‹@ô²èI  3ÛèÎáüÿëèÇáüÿN…oşÿÿ‹Ã_^[‹å]Ã@ U‹ìƒÄØS‰Uø‰Eô‹Eô‹ÿR3À‰Eü3ÛUEØº   èrìÿÿY„Àu²‹Eôèï   é«   }ØIFPSt²‹Eôè×   é“   ‹EÜƒøƒø}²‹Eôè»   ëzUè·òÿÿY„Àu
‹Eô‹ÿRëeUè^ùÿÿY„Àu
‹Eô‹ÿRëPUè©ıÿÿY„Àu
‹Eô‹ÿRë;‹Uô‹R8‹R‹Eì;Ğwƒøÿt²‹Eôè[   ‹Eô‹ÿRë‹Eô‹Uì‰PH‹EôÆ@L³‹Ã[‹å]Ã€xLuÆ@LÃSV‹ñ‹Ø‹C\‹ÈÎ;KX‹KT‹ÎèEÖüÿs\°^[Ã3À^[Ãj 3Éè÷€  Ã‹ÀSV‹ò‹Ø‹Î‹S‹èê  ^[Ã@ QRL$‹‹@’è»
  ZÃU‹ìj SVW‹Ø3ÀUhRC dÿ0d‰ €zu'‹ÃƒÀ‹‹…Ût…Òu¹¨RC ²¸Xc@ è$:ıÿèŸßüÿ3ÀŠBƒø‡×   Š€˜QC ÿ$…«QC            bRC ÏQC ÚQC äQC îQC øQC üQC  RC RC 3ÀŠ‹Øé   ¾‹Øé”   ·‹ØéŠ   ¿‹Øé€   ‹ë|‹ëx3ÀŠ‹Øëp‹Ãèıõüÿ-   uAUü‹Ãèöüÿ‹EüèLãüÿHuUü‹Ãèöüÿ‹Eü¶ ‹Øë9¹¨RC ²¸Xc@ èL9ıÿèÇŞüÿë!‹Ãè2íüÿ‹Øë¹¨RC ²¸Xc@ è)9ıÿè¤Şüÿ3ÀZYYd‰h•RC EüèsáüÿÃéÑİüÿëğ‹Ã_^[Y]Ã   ÿÿÿÿ   Type Mismatch   U‹ì€zu'‹ĞƒÂ‹‹ …Àt…Òu¹SC ²¸Xc@ è¹8ıÿè4ŞüÿŠR€êu‹U‰ë¹SC ²¸Xc@ è”8ıÿèŞüÿ]Â    ÿÿÿÿ   Type Mismatch   SƒÄğ‹Ø€zu'‹ÃƒÀ‹‹…Ût…Òu¹€TC ²¸Xc@ èB8ıÿè½İüÿ3ÀŠBƒø‡ì   Š€zSC ÿ$…“SC  	             
YTC ÃSC ØSC ìSC ıSC TC TC TC TC %TC <TC LTC 3ÀŠ‰D$ÛD$Û<$›é—   ¾‰D$ÛD$Û<$›éƒ   ·‰D$ÛD$Û<$›ërßÛ<$›ëjÛÛ<$›ëbÛÛ<$›ëZÙÛ<$›ëRİÛ<$›ëJ‹‰$‹C‰D$f‹Cf‰D$ë3Û-TC ß+ŞÉÛ<$›ë#‹ÃèİëüÿÛ<$›ë¹€TC ²¸Xc@ è27ıÿè­ÜüÿÛ,$ƒÄ[Ã ÿÿÿÿ   Type Mismatch   ,eâX·Ññ?  SƒÄô‹Ø€zu'‹ÃƒÀ‹‹…Ût…Òu¹VC ²¸Xc@ èÒ6ıÿèMÜüÿ3ÀŠBƒø‡  Š€êTC ÿ$…UC  	             
ñUC 3UC NUC hUC ‚UC UC UC ¬UC ºUC ÈUC ÖUC äUC 3ÀŠ‰D$ÛD$Ø(VC ß<$›é¹   ¾‰D$ÛD$Ø(VC ß<$›éŸ   ·‰D$ÛD$Ø(VC ß<$›é…   ßØ(VC ß<$›ëwÛØ(VC ß<$›ëiÛØ(VC ß<$›ë[ÙØ(VC ß<$›ëMİØ(VC ß<$›ë?Û+Ø(VC ß<$›ë1‹‰$‹C‰D$ë#‹Ãè¡êüÿß<$›ë¹VC ²¸Xc@ èš5ıÿèÛüÿß,$ƒÄ[Ã ÿÿÿÿ   Type Mismatch    @FS‹Ø€zu'‹ÃƒÀ‹‹…Ût…Òu¹øVC ²¸Xc@ èE5ıÿèÀÚüÿ3ÀŠBƒøwpŠ€sVC ÿ$…†VC            ÖVC ªVC °VC µVC ºVC ¿VC ÃVC ÇVC ÍVC 3ÀŠ[Ã¾[Ã·[Ã¿[Ã‹[Ã‹[Ã3ÀŠ[Ã‹Ãè¼èüÿ[Ã¹øVC ²¸Xc@ èµ4ıÿè0Úüÿ[Ã  ÿÿÿÿ   Type Mismatch   SV‹ñ‹Ø€zu'‹ÃƒÀ‹‹…Ût…Òu¹ÜWC ²¸Xc@ èf4ıÿèáÙüÿ3ÀŠBƒøwtŠ€RWC ÿ$…eWC               ¹WC }WC ‰WC •WC ¡WC ­WC ‹ÆŠèŠİüÿ^[Ã‹ÆŠè~İüÿ^[Ã‹Æ‹èİüÿ^[Ã‹Æ‹èêÜüÿ^[Ã‹Æ‹Óè2éüÿ^[Ã¹ÜWC ²¸Xc@ èÒ3ıÿèMÙüÿ^[Ã  ÿÿÿÿ   Type Mismatch   U‹ìQSVW‰Mü‹Ø…Ût…Òu‹EüÆ  éU  €zu‹ÃƒÀ‹‹…Ût…Òu‹EüÆ  é3  3ÀŠBƒø‡  ÿ$…BXC ZYC ¦XC °XC ºXC ÆXC ëXC òXC  YC YC YC ZYC ZYC ZYC ZYC ZYC ZYC &YC ZYC ùXC ZYC ZYC ÒXC ZYC ZYC YC ŠEˆé°   ŠEˆé¦   f‹Ef‰éš   f‹Ef‰é   ‹E‰‹Ã‹ĞƒÂ3É‰
ƒÀ3Ò‰ëu‹E‰ën‹E‰ëg‹Eˆë`ÛEÙ›ëXÛEİ›ëPÛEØhYC ß;›ëBÛEÛ;›ë:3ÀUhHYC dÿ0d‰ ‹Ã‹Uèæçüÿ3ÀZYYd‰ëé#Õüÿ‹EüÆ  è4Øüÿë‹EüÆ  _^[Y]Â  @FU‹ìQSVW‰Mü‹Ø…Ût…Òu‹EüÆ  é×   €zu‹ÃƒÀ‹‹…Ût…Òu‹EüÆ  éµ   3ÀŠBƒÀùƒø‡   Š€ËYC ÿ$…İYC              \ZC õYC ıYC ZC ZC (ZC ÛmÙ›ëeÛmİ›ë]ÛmØlZC ß;›ëO‹E‰‹E‰Cf‹Ef‰Cë:3ÀUhJZC dÿ0d‰ Ûm‹Ãèçüÿ3ÀZYYd‰ëé!Ôüÿ‹EüÆ  è2×üÿë‹EüÆ  _^[Y]Â    @FU‹ìQSVW‰Mü‹Ø…Ût…Òu‹EüÆ  éU  €zu‹ÃƒÀ‹‹…Ût…Òu‹EüÆ  é3  3ÀŠBƒø‡  ÿ$…ÆZC Ş[C *[C 4[C >[C J[C o[C v[C „[C Œ[C ¢[C Ş[C Ş[C Ş[C Ş[C Ş[C Ş[C ª[C Ş[C }[C Ş[C Ş[C V[C Ş[C Ş[C ”[C ŠEˆé°   ŠEˆé¦   f‹Ef‰éš   f‹Ef‰é   ‹E‰‹Ã‹ĞƒÂ3É‰
ƒÀ3Ò‰ëu‹E‰ën‹E‰ëg‹Eˆë`ÛEÙ›ëXÛEİ›ëPÛEØì[C ß;›ëBÛEÛ;›ë:3ÀUhÌ[C dÿ0d‰ ‹Ã‹Uèbåüÿ3ÀZYYd‰ëéŸÒüÿ‹EüÆ  è°Õüÿë‹EüÆ  _^[Y]Â  @FU‹ìQSVW‰Mü‹Ø…Ût…Òu‹EüÆ  ër€zu‹ÃƒÀ‹‹…Ût…Òu‹EüÆ  ëSŠB,
t,të@‹Ã‹UèØüÿë:3ÀUhf\C dÿ0d‰ ‹Ã‹Uèœåüÿ3ÀZYYd‰ëéÒüÿ‹EüÆ  èÕüÿë‹EüÆ  _^[Y]Â ‹ÀSVWUƒÄø‹é‰T$‰$‹E‹xO…ÿ|4G3ö‹E‹Öè¸ÿÿ‹Ø‹E‹Öèø·ÿÿP‹T$Ó‹D$Ã¹   è   FOuÏ°YZ]_^[ÃU‹ìQSVW‹ò‹ØUhaC dÿ5    d‰%    ‹E¶@ƒø‡â  ÿ$…]C ñ`C ‚]C ‚]C œ]C œ]C î]C î]C î]C ^C 0^C £^C =_C _C è_C î]C ñ`C €^C ñ`C ‚]C ñ`C ñ`C ¼]C Æ^C _C \^C î]C °_C ‹ùO…ÿŒr  GŠˆCFOu÷éc  ‹ùO…ÿŒX  Gf‹f‰ƒÃƒÆOuñéC  ‹ùO…ÿŒ8  G‹‰ƒÃƒÆ‹‰ƒÃƒÆ‹‰ƒÃƒÆOußé  ‹ùO…ÿŒ  G‹‰ƒÃƒÆOuóéó  ‹ùO…ÿŒè  G‹‰‹F‰CƒÃƒÆOuíéÏ  ‹ùO…ÿŒÄ  G‹‰‹F‰Cf‹Ff‰CƒÃ
ƒÆ
Ouåé£  ‹ùO…ÿŒ˜  G‹‰‹F‰CƒÃƒÆOuíé  ‹ùO…ÿŒt  G‹Ã‹ÖèCéüÿƒÃƒÆOuîé\  ‹ùO…ÿŒQ  G‹Ã‹è˜ÕüÿƒÃƒÆOuîé9  ‹E‹@‰Eü‹ùO…ÿŒ%  G‹E‹@P‹E‹H‹Ö‹Ãèïıÿÿ„Àu3Àd    ƒÄé  ]üuüOuÌéë  ‹ùO…ÿŒà  G‹‰…Àt‹ƒèÿ ƒÃƒÆOuèéÂ  ‹E‹@‰Eü‹ùO…ÿŒ®  G‹M‹Ö‹Ãè*ıÿÿ„Àu3Àd    ƒÄé­  ]üuüOuÖé~  ‹E‹@‰Eü‹ùO…ÿŒj  G‹Ó‹Æ‹Müè–Çüÿ]üuüOuëéO  ‹ùO…ÿŒD  G‹…Àt
P‹ ÿP3À‰‹‰…ÀtP‹ ÿPƒÃƒÆOuÙé  ‹ÃƒÀƒ8 u<‹ÆƒÀƒ8 u2‹ùO…ÿŒø   G‹‰ƒÃƒÆ‹‰ƒÃƒÆ3À‰ƒÃƒÆOußéÑ   ‹ùO…ÿŒÆ   G‹ÃƒÀƒ8 t‹ÃƒÀ‹‹èÚÓÿÿ‹…Òtt‹ÆƒÀƒ8 u‰‹ÖƒÂ‹‹ËƒÁ‰‹ ‹ÓƒÂ‰ëe‹ÆƒÀ‹ è‡Óÿÿ‰‹ÆƒÀ‹ ‹ÓƒÂ‰‹ÃƒÀÇ    ‹ÃƒÀ‹ P¹   ‹‹è&üÿÿ„Àu$3Àd    ƒÄëT3À‰‹ÃƒÀ3Ò‰‹ÃƒÀ3Ò‰ƒÃƒÆO…Kÿÿÿë3Àd    ƒÄë!d    ƒÄëé`Íüÿ3ÀèuĞüÿëènĞüÿ°_^[Y]Â €zt¹\aC ²¸Xc@ è]*ıÿèØÏüÿ…Àu3ÀÃƒè‹ Ã   ÿÿÿÿ   Invalid array   SVWUƒÄô‰$‹ê‹ğ€}t¹\cC ²¸Xc@ è*ıÿè†Ïüÿ‹‹Õè‰ÿÿÿ‰D$‹E‹@‰D$ƒ|$ u
ƒ<$ „  ƒ|$ „É   ‹ƒèƒ8…»   ‹<$‹\$K+ß|3C‹EŠP€Âö€êr€êt
€êt€êu‹‹L$¯ÏÑ’èşÏÿÿGKuÎ‹ƒè‰ƒ<$ ‹$¯T$ƒÂ‹è+Äüÿ3À‰é  ‹$¯T$ƒÂ‹Æè(Äüÿ‹ƒÀ‰‹‹$‰‹ƒÀ‰‹|$‹$K+ßŒÖ   C‹‹T$¯×Â‹UèÎÿÿGKuéé¹   ƒ<$ u.‹ƒè‹ƒúu‹T$¯T$ƒÂè«Ãüÿë…Ò~ÿ3À‰é…   ‹$÷l$ƒÀèsÃüÿ‹ØÇ   ƒÃ‹$‰ƒÃƒ|$ t6‹D$;$~‹EP‹‹L$‹Ãè×ùÿÿë‹EP‹‹L$‹ÃèÄùÿÿ‹Æ‹ÕèëÎÿÿ‰‹|$‹$K+ß|C‹‹T$¯×Â‹UèYÍÿÿGKuéƒÄ]_^[Ã ÿÿÿÿ   Invalid array   SVƒÄø‹Ú‹ğ‹Ó‹Æè.!ıÿƒ; uS‰t$ÆD$ T$3É¸¬cC èçıÿYZ^[Ã  ÿÿÿÿ   OLE error %.8x  U‹ìj SVW‹Ø3ÀUhdC dÿ0d‰ Uü‹Ãèÿÿÿ‹Mü²¸Xc@ è¯'ıÿè*Íüÿ3ÀZYYd‰hdC EüèùÏüÿÃéWÌüÿëğ_^[Y]Ã@ …À}è›ÿÿÿÃ‹ÀS‹‰…ÒtR‹ÿP…ÛtS‹ÿP[ÃSV‹ò‹Ø‹ÃèëØüÿ‰sfÇ	 …ötV‹ÿP^[Ã‹Àf‹
fƒérfƒétfé @të3Òë"‹Rë‹R‹ë¹°dC ²¸Xc@ èÿ&ıÿèzÌüÿè}ÿÿÿÃÿÿÿÿ(   Variant does not reference an OLE object    U‹ìƒÄàSVW3Û‰]à‹ù‰Uø‰Eü‹u3ÀUh÷oC dÿ0d‰ ³3ÉUhÉnC dÿ1d‰!‹E¶@ƒø‡‰	  ÿ$…*eC ¬nC ·eC ÊeC İeC ñeC fC •gC ›hC øiC [kC ølC ÁmC LmC ¬nC ÚlC ¬nC åmC ¬nC mC ¬nC ¬nC fC 'mC –eC ÆlC ?nC RnC ;uu‹E‹H‹Uø‹ÇèÁüÿéş  3Ûé÷  ‹Ö‹Çèxëÿÿ‹Uøˆéä  ‹Ö‹ÇèYğÿÿ‹UøˆéÑ  ‹Ö‹ÇèRëÿÿ‹Uøf‰é½  ‹Ö‹Çè2ğÿÿ‹Uøf‰é©  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ èo%ıÿèêÊüÿŠF,t,të5‹‹Uø‰éc  ‹‹Uø‰‹Ç‹ÈƒÁ‹	‹Uø‹òƒÆ‰ƒÀ‹ ƒÂ‰é:  ¹pC ²¸Xc@ è%ıÿè’Êüÿé  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ èå$ıÿè`Êüÿ3ÀŠFƒø‡°   Š€×fC ÿ$…êfC            zgC gC gC )gC 6gC CgC OgC [gC igC 3ÀŠ‹Uø‰é’  ¾‹Uø‰é…  ·‹Uø‰éx  ¿‹Uø‰ék  ‹‹Uø‰é_  ‹‹Uø‰éS  3ÀŠ‹Uø‰éE  ‹Çè Øüÿ‹Uø‰é4  ¹pC ²¸Xc@ è$ıÿèŒÉüÿé  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ èß#ıÿèZÉüÿ3ÀŠFƒø‡°   Š€İgC ÿ$…ğgC            €hC hC "hC /hC <hC IhC UhC ahC ohC 3ÀŠ‹Uø‰éŒ  ¾‹Uø‰é  ·‹Uø‰ér  ¿‹Uø‰ée  ‹‹Uø‰éY  ‹‹Uø‰éM  3ÀŠ‹Uø‰é?  ‹Çè×üÿ‹Uø‰é.  ¹pC ²¸Xc@ è#ıÿè†Èüÿé  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ èÙ"ıÿèTÈüÿ3ÀŠFƒø‡  Š€ãhC ÿ$…ühC  	             
İiC ,iC AiC UiC iiC viC ƒiC iC œiC ©iC ¶iC ËiC 3ÀŠ‰EäÛEä‹EøÙ›ém  ¾‰EäÛEä‹EøÙ›éY  ·‰EäÛEä‹EøÙ›éE  ß‹EøÙ›é8  Û‹EøÙ›é+  Û‹EøÙ›é  ‹‹Uø‰é  İ‹EøÙ›é  Û/‹EøÙ›éø  Û-$pC ß/ŞÉ‹EøÙ›éã  ‹Çè^Öüÿ‹EøÙ›éÑ  ¹pC ²¸Xc@ è®!ıÿè)Çüÿé¶  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ è|!ıÿè÷Æüÿ3ÀŠFƒø‡  Š€@jC ÿ$…YjC  	             
@kC ‰jC jC ²jC ÆjC ÓjC àjC íjC újC kC kC .kC 3ÀŠ‰EäÛEä‹Eøİ›é  ¾‰EäÛEä‹Eøİ›éü  ·‰EäÛEä‹Eøİ›éè  ß‹Eøİ›éÛ  Û‹Eøİ›éÎ  Û‹Eøİ›éÁ  Ù‹Eøİ›é´  ‹Eø‹‰‹W‰Pé¢  Û/‹Eøİ›é•  Û-$pC ß/ŞÉ‹Eøİ›é€  ‹ÇèûÔüÿ‹Eøİ›én  ¹pC ²¸Xc@ èK ıÿèÆÅüÿéS  €~u'‹ÇƒÀ‹0‹?…ÿt…öu¹pC ²¸Xc@ è ıÿè”Åüÿ3ÀŠFƒø‡  Š€£kC ÿ$…¼kC  	             
«lC ìkC lC lC )lC 6lC ClC PlC ]lC jlC „lC ™lC 3ÀŠ‰EäÛEä‹EøÛ8›é­  ¾‰EäÛEä‹EøÛ8›é™  ·‰EäÛEä‹EøÛ8›é…  ß‹EøÛ8›éx  Û‹EøÛ8›ék  Û‹EøÛ8›é^  Ù‹EøÛ8›éQ  İ‹EøÛ8›éD  ‹Eø‹‰‹W‰Pf‹Wf‰Pé*  Û-$pC ß/ŞÉ‹EøÛ8›é  ‹ÇèÓüÿ‹EøÛ8›é  ¹pC ²¸Xc@ èàıÿè[Äüÿéè  ‹Ö‹ÇèÍçÿÿ‹Eøß8›éÔ  Mà‹Ö‹Çè"êÿÿ‹EàèJÊüÿ‹Uø‰é¶  Mà‹Ö‹Çèêÿÿ‹Uà‹EøèAÇüÿéš  ‹Ö‹Çèäÿÿ‹Uøˆé‡  ;ut3Ûé{  ‹E‹PR‹H‹×‹Eøè™ïÿÿéb  €~u1‹E‹@;Fu&‹N‹Eø‹Uèôÿÿ‹FP‹N‹Eø‹ ‹×èbïÿÿé+  ;ut!‹E€xu€~u‹E‹@;Ft3Ûé  ‹EP¹   ‹×‹Eøè$ïÿÿéí   ;ut3Ûéá   ‹EP¹   ‹×‹Eøè ïÿÿéÉ   ‹Fº8pC è’Èüÿu‹‹EøèBöÿÿé«   €~u‹Eø‹×èÅÙüÿé–   ²‹EüèÒa  ‰Eè‰uğ‰}ì3À‰Eô‹UøEèèåŸ  ‹Øëo€~u	‹‹Uø‰ë`3Ûë\ŠF<u"‹E‹@º8pC èÈüÿu‹×‹Eøèïõÿÿë73Ûë3<u)‹Eø‹ …ÀtP‹ ÿP‹Eø3Ò‰‹‹Uø‰…ÀtP‹ ÿPë3Ûë3Û„Ûu
²‹EüèDâÿÿ3ÀZYYd‰é  é¢¿üÿèéÄüÿƒ¸     tèÛÄüÿ‹˜    ‹sèÍÄüÿ3À‰Cë3ö…ö„©   ‹Æº¤C è”¼üÿ„Àt$3Ûj‹ş‹GPj ‹O‹W‹Eü‹0ÿVè]Âüÿé­   ‹Æº¨d@ è`¼üÿ„Àt3ÛV3É²‹Eüè­b  è4Âüÿé„   ‹Æºìe@ è7¼üÿ„Àt3ÛV3É²‹Eüè„b  èÂüÿë^‹Æºle@ è¼üÿ„Àt3ÛV3É²‹Eüè^b  èåÁüÿë8…öt ‹ÆºXc@ èç»üÿ„ÀtV‹N²‹Eüè5b  ëV3É²‹Eüè&b  3Ûè«Áüÿ3ÀZYYd‰hşoC Eàè
ÄüÿÃéhÀüÿëğ‹Ã_^[‹å]Â    ÿÿÿÿ   Type Mismatch   ,eâX·Ññ?  ÿÿÿÿ	   IDISPATCH   U‹ìSVW‹Ù‹ò…öu3ÛëTºLD è`à  …Àu3ÛëB‹Sèäî  ‹Ø…Ûu3Ûë03ÀUhpC dÿ0d‰ ‹Æ‹Sè»üÿ‹Ø3ÀZYYd‰ëéÍ½üÿ3ÛèâÀüÿ‹Ã_^[]Ã@ U‹ìS‹ÚÆ‹U‹R¶Rƒúwsÿ$•ÓpC ?qC ïpC úpC qC qC #qC 1qC ‹U‹Rˆ[]Ã‹U‹Rˆ[]Ãƒà‹U‹Rf‰[]Ãƒà‹U‹Rf‰[]Ãƒà‹U‹R‰[]Ãƒà‹U‹R‰[]Ã‹E‹@ü²è´ßÿÿÆ []Ã‹ÀU‹ìQ¹   j j Iuù‡MüSVW‹ñ‰Uø‰Eü‹}‹]3ÀUh&C dÿ0d‰ ÆE÷3ÉUhÙ‹C dÿ1d‰!ƒÿ‡  ÿ$½¨qC ÈqC ÊuC ÌyC ª}C ˆC †C ¨ŠC -‹C ‹E¶@ƒø‡¬  ÿ$…ßqC „uC CrC —rC ±rC ËrC årC ürC |tC –tC ÒtC îtC „uC „uC „uC îtC „uC <uC „uC uC „uC „uC „uC „uC cuC °tC ŠC<
t<u/Eà‹UøŠèµÂüÿ‹EàPMÜ‹Ó‹Æèäÿÿ‹UÜXèÄüÿ“Eöé  ‹Ó‹Æè²Şÿÿ‹Uø¶;ÂEöé  ‹Ó‹ÆèŒãÿÿ‹Uø¾;ÂEöéê  ‹Ó‹Æè~Şÿÿ‹Uø·;ÂEöéĞ  ‹Ó‹ÆèXãÿÿ‹Uø¿;ÂEöé¶  ‹Ó‹ÆèJŞÿÿ‹Uø;–EöéŸ  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ èxıÿèó½üÿ3ÀŠCƒø‡*  ÿ$…>sC atC ŠsC sC ±sC ÄsC ×sC çsC tC ÷sC atC atC atC atC atC atC atC AtC atC -tC ‹Eø‹ 3ÒŠ;ÂEöéı  ‹Eø‹ ¾;ÂEöéê  ‹Eø‹ ·;ÂEöé×  ‹Eø‹ ¿;ÂEöéÄ  ‹Eø‹ ;Eöé´  ‹Eø‹ ;Eöé¤  ‹U‹Eøè*ßÿÿÛŞÙßà–Eöé‰  ‹U‹EøèßÿÿÛŞÙßà–Eöén  ‹Eø‹ 3ÒŠ;ÂEöéZ  EÌ‹Uø‹èÖÌüÿEÌ‹Öè¼ÑüÿEöé:  ¹DC ²¸Xc@ è*ıÿè¥¼üÿé  ‹Ó‹Æè§Şÿÿ‹EøØßà–Eöé  ‹Ó‹ÆèŞÿÿ‹EøÜßà–Eöéë   ‹Ó‹ÆèsŞÿÿØTC ‹Eøß(ŞÙßà“EöéÉ   ‹Ó‹ÆèQŞÿÿ‹EøÛ(ŞÙßà“Eöé­   Mà‹Ó‹Æèâÿÿ‹Uà‹Eø‹ è}Áüÿ“Eöé‹   Eà‹UøŠèó¿üÿ‹EàPMÜ‹Ó‹ÆèÛáÿÿ‹UÜXèNÁüÿ“Eöë_Mä‹Ó‹Æèd–  „ÀuÆE÷ ëI‹EøUäè·ĞüÿEöë8;]uEöP‹E‹H‹Uø‹Æèd§ÿÿëÆE÷ ë²‹EüèrÛÿÿ3ÀZYYd‰é`  €}÷ u²‹EüèUÛÿÿ3ÀZYYd‰éC  UU÷ŠEöèğúÿÿYé  ‹E¶@ƒø‡¬  ÿ$…áuC †yC EvC ™vC ³vC ÍvC çvC şvC ~xC ºxC ÔxC ğxC †yC †yC †yC ğxC †yC >yC †yC yC †yC †yC †yC †yC eyC ˜xC ŠC<
t<u/Eà‹UøŠè³¾üÿ‹EàPMÜ‹Ó‹Æè›àÿÿ‹UÜXèÀüÿ–Eöé  ‹Ó‹Æè°Úÿÿ‹Uø¶;ÂEöé  ‹Ó‹ÆèŠßÿÿ‹Uø¾;ÂEöéê  ‹Ó‹Æè|Úÿÿ‹Uø·;ÂEöéĞ  ‹Ó‹ÆèVßÿÿ‹Uø¿;ÂEöé¶  ‹Ó‹ÆèHÚÿÿ‹Uø;“EöéŸ  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ èvıÿèñ¹üÿ3ÀŠCƒø‡*  ÿ$…@wC cxC ŒwC  wC ³wC ÆwC ÙwC éwC xC ùwC cxC cxC cxC cxC cxC cxC cxC CxC cxC /xC ‹Eø‹ 3ÒŠ;ÂEöéı  ‹Eø‹ ¾;ÂEöéê  ‹Eø‹ ·;ÂEöé×  ‹Eø‹ ¿;ÂEöéÄ  ‹Eø‹ ;Eöé´  ‹Eø‹ ;Eöé¤  ‹U‹Eøè(ÛÿÿÛŞÙßà“Eöé‰  ‹U‹EøèÛÿÿÛŞÙßà“Eöén  ‹Eø‹ 3ÒŠ;ÂEöéZ  EÌ‹Uø‹èÔÈüÿEÌ‹ÖèºÍüÿEöé:  ¹DC ²¸Xc@ è(ıÿè£¸üÿé  ‹Ó‹Æè¥Úÿÿ‹EøØßà“Eöé  ‹Ó‹Æè‹ÚÿÿØTC ‹Eøß(ŞÙßà–Eöéã   ‹Ó‹ÆèiÚÿÿ‹EøÜßà“EöéÉ   ‹Ó‹ÆèOÚÿÿ‹EøÛ(ŞÙßà–Eöé­   Mà‹Ó‹ÆèŞÿÿ‹Uà‹Eø‹ è{½üÿ–Eöé‹   Eà‹UøŠèñ»üÿ‹EàPMÜ‹Ó‹ÆèÙİÿÿ‹UÜXèL½üÿ–Eöë_Mä‹Ó‹Æèb’  „ÀuÆE÷ ëI‹EøUäèµÌüÿEöë8;]uEöP‹E‹H‹Ö‹Eøèb£ÿÿëÆE÷ ë²‹Eüèp×ÿÿ3ÀZYYd‰é^  €}÷ u²‹EüèS×ÿÿ3ÀZYYd‰éA  UU÷ŠEöèîöÿÿYé   ‹E¶@ƒø‡ˆ  ÿ$…ãyC d}C GzC ›zC µzC ÏzC ézC  {C €|C š|C ´|C ò|C d}C d}C d}C ò|C d}C =}C d}C }C d}C d}C d}C d}C d}C Ğ|C ŠC<
t<u/Eà‹UøŠè±ºüÿ‹EàPMÜ‹Ó‹Æè™Üÿÿ‹UÜXè¼üÿ—Eöéú  ‹Ó‹Æè®Öÿÿ‹Uø¶;ÂœEöéà  ‹Ó‹ÆèˆÛÿÿ‹Uø¾;ÂœEöéÆ  ‹Ó‹ÆèzÖÿÿ‹Uø·;ÂœEöé¬  ‹Ó‹ÆèTÛÿÿ‹Uø¿;ÂœEöé’  ‹Ó‹ÆèFÖÿÿ‹Uø;’Eöé{  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ ètıÿèïµüÿ3ÀŠCƒø‡*  ÿ$…B{C e|C {C ¢{C µ{C È{C Û{C ë{C |C û{C e|C e|C e|C e|C e|C e|C e|C E|C e|C 1|C ‹Eø‹ 3ÒŠ;ÂŸEöéÙ  ‹Eø‹ ¾;ÂŸEöéÆ  ‹Eø‹ ·;ÂŸEöé³  ‹Eø‹ ¿;ÂŸEöé   ‹Eø‹ ;ŸEöé  ‹Eø‹ ;ŸEöé€  ‹U‹Eøè&×ÿÿÛŞÙßà’Eöée  ‹U‹Eøè×ÿÿÛŞÙßà’EöéJ  ‹Eø‹ 3ÒŠ;ÂŸEöé6  EÌ‹Uø‹èÒÄüÿEÌ‹Öè¸ÉüÿŸEöé  ¹DC ²¸Xc@ è&ıÿè¡´üÿéû   ‹Ó‹Æè£Öÿÿ‹EøØßà’Eöéá   ‹Ó‹Æè‰Öÿÿ‹EøÜßà’EöéÇ   ‹Ó‹ÆèoÖÿÿ‹EøÛ(ŞÙßà—Eöé«   ‹Ó‹ÆèSÖÿÿØTC ‹Eøß(ŞÙßà—Eöé‰   Mà‹Ó‹Æè
Úÿÿ‹Uà‹Eø‹ èy¹üÿ—EöëjEà‹UøŠèò·üÿ‹EàPMÜ‹Ó‹ÆèÚÙÿÿ‹UÜXèM¹üÿ—Eöë>Mä‹Ó‹Æèc  „ÀuÆE÷ ë(‹EøUäè¶ÈüÿŸEöë²‹Eüè’Óÿÿ3ÀZYYd‰é€  €}÷ u²‹EüèuÓÿÿ3ÀZYYd‰éc  UU÷ŠEöèóÿÿYé"  ‹E¶@ƒø‡ˆ  ÿ$…Á}C BC %~C y~C “~C ­~C Ç~C Ş~C ^€C x€C ´€C Ğ€C BC BC BC Ğ€C BC C BC ï€C BC BC BC BC BC ’€C ŠC<
t<u/Eà‹UøŠèÓ¶üÿ‹EàPMÜ‹Ó‹Æè»Øÿÿ‹UÜXè.¸üÿ’Eöéú  ‹Ó‹ÆèĞÒÿÿ‹Uø¶;ÂŸEöéà  ‹Ó‹Æèª×ÿÿ‹Uø¾;ÂŸEöéÆ  ‹Ó‹ÆèœÒÿÿ‹Uø·;ÂŸEöé¬  ‹Ó‹Æèv×ÿÿ‹Uø¿;ÂŸEöé’  ‹Ó‹ÆèhÒÿÿ‹Uø;—Eöé{  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ è–ıÿè²üÿ3ÀŠCƒø‡*  ÿ$… C C€C lC €C “C ¦C ¹C ÉC ôC ÙC C€C C€C C€C C€C C€C C€C C€C #€C C€C €C ‹Eø‹ 3ÒŠ;ÂœEöéÙ  ‹Eø‹ ¾;ÂœEöéÆ  ‹Eø‹ ·;ÂœEöé³  ‹Eø‹ ¿;ÂœEöé   ‹Eø‹ ;œEöé  ‹Eø‹ ;œEöé€  ‹U‹EøèHÓÿÿÛŞÙßà—Eöée  ‹U‹Eøè-ÓÿÿÛŞÙßà—EöéJ  ‹Eø‹ 3ÒŠ;ÂœEöé6  EÌ‹Uø‹èôÀüÿEÌ‹ÖèÚÅüÿœEöé  ¹DC ²¸Xc@ èHıÿèÃ°üÿéû   ‹Ó‹ÆèÅÒÿÿ‹EøØßà—Eöéá   ‹Ó‹Æè«Òÿÿ‹EøÜßà—EöéÇ   ‹Ó‹Æè‘ÒÿÿØTC ‹Eøß(ŞÙßà’Eöé¥   ‹Ó‹ÆèoÒÿÿ‹EøÛ(ŞÙßà’Eöé‰   Mà‹Ó‹Æè,Öÿÿ‹Uà‹Eø‹ è›µüÿ’EöëjEà‹UøŠè´üÿ‹EàPMÜ‹Ó‹ÆèüÕÿÿ‹UÜXèoµüÿ’Eöë>Mä‹Ó‹Æè…Š  „ÀuÆE÷ ë(‹EøUäèØÄüÿœEöë²‹Eüè´Ïÿÿ3ÀZYYd‰é¢  €}÷ u²‹Eüè—Ïÿÿ3ÀZYYd‰é…  UU÷ŠEöè2ïÿÿYéD
  ‹E¶@ƒø‡>  ÿ$…ŸC Ö…C I‚C ‚C ·‚C Ñ‚C 3ƒC JƒC Æ„C à„C ú„C 8…C Ö…C Ö…C Ö…C 8…C Ö…C †…C Ö…C Z…C Ö…C Ö…C ë‚C Ö…C ­…C …C *‚C ‚C €{u‹Eø‹ ;•EöéÌ  ÆE÷ éÃ  €{u‹Eø‹ ;•Eöé­  ÆE÷ é¤  ŠC<
t<u/Eà‹UøŠè¯²üÿ‹EàPMÜ‹Ó‹Æè—Ôÿÿ‹UÜXè
´üÿ•Eöéj  ‹Ó‹Æè¬Îÿÿ‹Uø¶;Â•EöéP  ‹Ó‹Æè†Óÿÿ‹Uø¾;Â•Eöé6  ‹Ó‹ÆèxÎÿÿ‹Uø·;Â•Eöé  ‹Ó‹ÆèRÓÿÿ‹Uø¿;Â•Eöé  ‹Eø‹ ;u6‹Eøƒ8 u%‹ÖƒÂ‹‹EøƒÀ‹ ;Ğu;Ğu3Àë°ˆEöéÌ  ÆEö éÃ  ÆEöéº  ‹Ó‹ÆèüÍÿÿ‹Uø;•Eöé£  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ è*ıÿè¥­üÿ3ÀŠCƒø‡&  Š€’ƒC ÿ$…¨ƒC         
 	  «„C ÔƒC èƒC ûƒC „C !„C 1„C A„C \„C w„C ‹„C ‹Eø‹ 3ÒŠ;Â•Eöé  ‹Eø‹ ¾;Â•Eöéò  ‹Eø‹ ·;Â•Eöéß  ‹Eø‹ ¿;Â•EöéÌ  ‹Eø‹ ;•Eöé¼  ‹Eø‹ ;•Eöé¬  ‹U‹EøèàÎÿÿÛŞÙßà•Eöé‘  ‹U‹EøèÅÎÿÿÛŞÙßà•Eöév  ‹Eø‹ 3ÒŠ;Â•Eöéb  EÌ‹Uø‹èŒ¼üÿEÌ‹ÖèrÁüÿ•EöéB  ¹DC ²¸Xc@ èàıÿè[¬üÿé'  ‹Ó‹Æè]Îÿÿ‹EøØßà•Eöé  ‹Ó‹ÆèCÎÿÿ‹EøÜßà•Eöéó   ‹Ó‹Æè)Îÿÿ‹EøÛ(ŞÙßà•Eöé×   ‹Ó‹ÆèÎÿÿØTC ‹Eøß(ŞÙßà•Eöéµ   Mà‹Ó‹ÆèÄÑÿÿ‹Uà‹Eø‹ è3±üÿ•Eöé“   Eà‹UøŠè©¯üÿ‹EàPMÜ‹Ó‹Æè‘Ñÿÿ‹UÜXè±üÿ•EöëgMä‹Ó‹Æè†  „ÀuÆE÷ ëQ‹EøUäèmÀüÿ•Eöë@;]uEöP‹E‹H‹Ö‹EøèN—ÿÿŠEö4ˆEöëÆE÷ ë²‹Eüè Ëÿÿ3ÀZYYd‰é  €}÷ u²‹EüèËÿÿ3ÀZYYd‰éñ  UU÷ŠEöèêÿÿYé°  ‹E¶@ƒø‡6  ÿ$…3†C bŠC İ†C 1‡C K‡C e‡C ‡C Ş‡C Z‰C t‰C ‰C Ì‰C bŠC bŠC bŠC Ì‰C bŠC ŠC bŠC î‰C bŠC bŠC –‡C bŠC AŠC ª‰C ¾†C Ÿ†C €{u‹Eø‹ ;”EöéÄ  ÆE÷ é»  €{u‹Eø‹ ;”Eöé¥  ÆE÷ éœ  ŠC<
t<u/Eà‹UøŠè®üÿ‹EàPMÜ‹Ó‹ÆèĞÿÿ‹UÜXèv¯üÿ”Eöéb  ‹Ó‹ÆèÊÿÿ‹Uø¶;Â”EöéH  ‹Ó‹ÆèòÎÿÿ‹Uø¾;Â”Eöé.  ‹Ó‹ÆèäÉÿÿ‹Uø·;Â”Eöé  ‹Ó‹Æè¾Îÿÿ‹Uø¿;Â”Eöéú  ‹Ó‹Æè°Éÿÿ‹Uø;”Eöéã  ‹Eø‹ ;u6‹Eøƒ8 u%‹ÖƒÂ‹‹EøƒÀ‹ ;Ğu;Ğt3Àë°ˆEöé­  ÆEöé¤  ÆEö é›  €{u'‹ÆƒÀ‹‹6…öt…Ûu¹DC ²¸Xc@ è–ıÿè©üÿ3ÀŠCƒø‡&  Š€&ˆC ÿ$…<ˆC         
 	  ?‰C hˆC |ˆC ˆC ¢ˆC µˆC ÅˆC ÕˆC ğˆC ‰C ‰C ‹Eø‹ 3ÒŠ;Â”Eöéı  ‹Eø‹ ¾;Â”Eöéê  ‹Eø‹ ·;Â”Eöé×  ‹Eø‹ ¿;Â”EöéÄ  ‹Eø‹ ;”Eöé´  ‹Eø‹ ;”Eöé¤  ‹U‹EøèLÊÿÿÛŞÙßà”Eöé‰  ‹U‹Eøè1ÊÿÿÛŞÙßà”Eöén  ‹Eø‹ 3ÒŠ;Â”EöéZ  EÌ‹Uø‹èø·üÿEÌ‹ÖèŞ¼üÿ”Eöé:  ¹DC ²¸Xc@ èLıÿèÇ§üÿé  ‹Ó‹ÆèÉÉÿÿ‹EøØßà”Eöé  ‹Ó‹Æè¯Éÿÿ‹EøÜßà”Eöéë   ‹Ó‹Æè•Éÿÿ‹EøÛ(ŞÙßà”EöéÏ   ‹Ó‹ÆèyÉÿÿØTC ‹Eøß(ŞÙßà”Eöé­   Mà‹Ó‹Æè0Íÿÿ‹Uà‹Eø‹ èŸ¬üÿ”Eöé‹   Eà‹UøŠè«üÿ‹EàPMÜ‹Ó‹ÆèıÌÿÿ‹UÜXèp¬üÿ”Eöë_Mä‹Ó‹Æè†  „ÀuÆE÷ ëI‹EøUäèÙ»üÿ”Eöë8;]uEöP‹E‹H‹Ö‹Eøèº’ÿÿëÆE÷ ë²‹Eüè”Æÿÿ3ÀZYYd‰é‚  €}÷ u²‹EüèwÆÿÿ3ÀZYYd‰ée  UU÷ŠEöèæÿÿYé$  €{uh‹U‹EøèÆÿÿ‹ø€}÷ u²‹Eüè5Æÿÿ3ÀZYYd‰é#  ;{r²‹EüèÆÿÿÆE÷ 3ÀZYYd‰é  Mö‹Ö‹Çè<’ÿÿUU÷ŠEöè¤åÿÿYé¶   ²‹EüèàÅÿÿ3ÀZYYd‰éÎ  ‹EŠ@,uF€{tÆE÷ ëQ‹Eü‹@4‹èd‰ÿÿ‹Ø…Ût€{tÆE÷ ë2U‹Eø‹‹Ë‹EüèÒäÿÿU÷è:åÿÿYë²‹EüèyÅÿÿ3ÀZYYd‰ég  €}÷ u2²‹Eüè\Åÿÿ3ÀZYYd‰éJ  ÆE÷ ²‹EüèAÅÿÿ3ÀZYYd‰é/  3ÀZYYd‰é"  é’¢üÿèÙ§üÿƒ¸     tèË§üÿ‹°    ‹^è½§üÿ3À‰Fë3Û…Û„±   ‹Ãº¤C è„Ÿüÿ„Àt&ÆE÷ j‹ó‹FPj ‹N‹V‹Eü‹ÿSèK¥üÿéµ   ‹Ãº¨d@ èNŸüÿ„ÀtÆE÷ S3É²‹Eüè™E  è ¥üÿéŠ   ‹Ãºìe@ è#Ÿüÿ„ÀtÆE÷ S3É²‹EüènE  èõ¤üÿëb‹Ãºle@ èûüÿ„ÀtÆE÷ S3É²‹EüèFE  èÍ¤üÿë:…Ût ‹ÃºXc@ èÏüÿ„ÀtS‹K²‹EüèE  ëS3É²‹EüèE  ÆE÷ è‘¤üÿ3ÀZYYd‰h-C EÌèĞºüÿEÜº   è§üÿEäè»ºüÿÃé9£üÿëÛŠE÷_^[‹å]Â    ÿÿÿÿ   Type Mismatch    @FS‹Ø‹ÃèªºüÿƒÀüƒè’À[Ã@ U‹ìQ¹   j j Iuù‡MüSVW‹ù‹ò‰Eü3ÀUh‹¿C dÿ0d‰ 3ÀUhK¾C dÿ0d‰ ³‹Eƒø
‡n0  ÿ$…¾C êC ›—C 0¡C ³ªC p´C ¸C (¹C bºC x»C ¼C ¤½C ‹E¶@ƒø‡k	  ÿ$…C e—C eC vC ‡C ™C «C ¸C ÅC =’C e•C ë–C e—C e—C e—C ë–C e—C —C e—C —C e—C e—C e—C e—C H—C µ“C ‹U‹ÇèÉÂÿÿ é	  ‹U‹Çè¬Çÿÿ éõ  ‹U‹Çè§Âÿÿféã  ‹U‹Çè‰ÇÿÿféÑ  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èÁüüÿè<¢üÿ‹E¶@ƒø‡­   Š€ıC ÿ$…C            C 4C ?C IC SC ]C fC oC zC 3ÀŠé=  ¾é3  ·é)  ¿é  ‹é  ‹é  3ÀŠé  EÜ‹è ±üÿEÜ‹×èfµüÿEÜèú¯üÿ‰éß  ¹¨¿C ²¸Xc@ èîûüÿèi¡üÿéÄ  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è´ûüÿè/¡üÿ‹E¶@ƒø‡­   Š€
C ÿ$…C            ªC AC LC VC `C jC sC |C ‡C 3ÀŠé0  ¾é&  ·é  ¿é  ‹é	  ‹é   3ÀŠéõ  EÜ‹è“°üÿEÜ‹×èY´üÿEÜèí®üÿ‰éÒ  ¹¨¿C ²¸Xc@ èáúüÿè\ üÿé·  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è§úüÿè" üÿ‹E¶@ƒø‡  Š€‘C ÿ$…0‘C  	             
"’C `‘C t‘C ‡‘C š‘C ¦‘C ²‘C ¾‘C Ê‘C Ö‘C â‘C ş‘C 3ÀŠ‰EØÛEØØÙ›é  ¾‰EØÛEØØÙ›éõ  ·‰EØÛEØØÙ›éâ  ßØÙ›éÖ  ÛØÙ›éÊ  ÛØÙ›é¾  ÙØÙ›é²  ÙÜÙ›é¦  Û/ØÙ›éš  ÙØ¸¿C ß/ŞÁÛ-¼¿C ŞÉÙ›é~  ÙEÜèT¯üÿEÜ‹×èâ²üÿEÜè®üÿÙ›éZ  ¹¨¿C ²¸Xc@ èiùüÿèäüÿé?  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è/ùüÿèªüÿ‹E¶@ƒø‡  Š€’C ÿ$…¨’C  	             
š“C Ø’C ì’C ÿ’C “C “C *“C 6“C B“C N“C Z“C v“C 3ÀŠ‰EØÛEØÜİ›é  ¾‰EØÛEØÜİ›é}  ·‰EØÛEØÜİ›éj  ßÜİ›é^  ÛÜİ›éR  ÛÜİ›éF  İØİ›é:  İÜİ›é.  Û/Üİ›é"  İØ¸¿C ß/ŞÁÛ-¼¿C ŞÉİ›é  İEÜèÜ­üÿEÜ‹×èj±üÿEÜè¬üÿİ›éâ  ¹¨¿C ²¸Xc@ èñ÷üÿèlüÿéÇ  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è·÷üÿè2üÿ‹E¶@ƒø‡P  Š€”C ÿ$… ”C  	             
J•C P”C l”C ‡”C ¢”C ¶”C È”C Ü”C ğ”C •C •C &•C 3ÀŠ‰EØÛEØØ¸¿C ß.ŞÁß>›é  ¾‰EØÛEØØ¸¿C ß.ŞÁß>›éõ  ·‰EØÛEØØ¸¿C ß.ŞÁß>›éÚ  ßØ¸¿C ß.ŞÁß>›éÆ  ÛÜØ¸¿C ß>›é´  ÛØ¸¿C ß.ŞÁß>›é   ÙØ¸¿C ß.ŞÁß>›éŒ  İØ¸¿C ß.ŞÁß>›éx  Û/Ø¸¿C ß.ŞÁß>›éd  ß.ß/ŞÁß>›éV  ß.EÜè\¬üÿEÜ‹×èº¯üÿEÜèJ«üÿß>›é2  ¹¨¿C ²¸Xc@ èAöüÿè¼›üÿé  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èöüÿè‚›üÿ‹E¶@ƒø‡&  Š€·•C ÿ$…Ğ•C  	             
Ğ–C  –C –C +–C @–C N–C \–C j–C v–C ‚–C –C ¬–C 3ÀŠ‰EØÛEØÛ.ŞÁÛ>›éf  ¾‰EØÛEØÛ.ŞÁÛ>›éQ  ·‰EØÛEØÛ.ŞÁÛ>›é<  ßÛ.ŞÁÛ>›é.  ÛÛ.ŞÁÛ>›é   ÛÛ.ŞÁÛ>›é  Û.ØÛ>›é  Û.ÜÛ>›éú   Û.Û/ŞÁÛ>›éì   Û.Ø¸¿C ß/ŞÁÛ-¼¿C ŞÉÛ>›éĞ   Û.EÜè¦ªüÿEÜ‹×è4®üÿEÜèh©üÿÛ>›é¬   ¹¨¿C ²¸Xc@ è»ôüÿè6šüÿé‘   MÔ‹U‹ÇèÀÿÿ‹UÔ‹Æèzüÿëx‹U‹Çè*ºÿÿ ëjMì‹U‹Çèt  „Àu3ÛëU‹ÖEÜè§°üÿEÜUìè¸­üÿUÜ‹Æè’°üÿë4‹E;Eu‹E‹H‹×‹Æè9…ÿÿë3Ûë²‹Eüè‘¹ÿÿ3ÀZYYd‰éç'  „Û…º&  ²‹Eüèr¹ÿÿ3ÀZYYd‰éÈ'  ‹E¶@ƒø‡O	  ÿ$…²—C ú C ˜C '˜C 8˜C J˜C \˜C i™C všC C ŸC ú C ú C ú C ú C ú C ú C § C ú C ™ C ú C ú C ú C ú C İ C î›C ‹U‹Çè¹ÿÿ(éê  ‹U‹Çèû½ÿÿ(éÙ  ‹U‹Çèö¸ÿÿf)éÇ  ‹U‹ÇèØ½ÿÿf)éµ  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èóüÿè‹˜üÿ‹E¶@ƒø‡­   Š€®˜C ÿ$…Á˜C            N™C å˜C ğ˜C ú˜C ™C ™C ™C  ™C +™C 3ÀŠ)é!  ¾)é  ·)é  ¿)é  ‹)éú  ‹)éñ  3ÀŠ)éæ  EÜ‹èï§üÿEÜ‹×èÁ«üÿEÜèI¦üÿ‰éÃ  ¹¨¿C ²¸Xc@ è=òüÿè¸—üÿé¨  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èòüÿè~—üÿ‹E¶@ƒø‡­   Š€»™C ÿ$…Î™C            [šC ò™C ı™C šC šC šC $šC -šC 8šC 3ÀŠ)é  ¾)é
  ·)é   ¿)éö  ‹)éí  ‹)éä  3ÀŠ)éÙ  EÜ‹èâ¦üÿEÜ‹×è´ªüÿEÜè<¥üÿ‰é¶  ¹¨¿C ²¸Xc@ è0ñüÿè«–üÿé›  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èöğüÿèq–üÿ‹E¶@ƒø‡  Š€ÈšC ÿ$…ášC  	             
Ó›C ›C %›C 8›C K›C W›C c›C o›C {›C ‡›C “›C ¯›C 3ÀŠ‰EØÛEØØ.Ù›éì  ¾‰EØÛEØØ.Ù›éÙ  ·‰EØÛEØØ.Ù›éÆ  ßØ.Ù›éº  ÛØ.Ù›é®  ÛØ.Ù›é¢  ÙØ'Ù›é–  ÙÜ'Ù›éŠ  Û/Ø.Ù›é~  ÙØ¸¿C ß/ŞéÛ-¼¿C ŞÉÙ›éb  ÙEÜè£¥üÿEÜ‹×è=©üÿEÜèe¤üÿÙ›é>  ¹¨¿C ²¸Xc@ è¸ïüÿè3•üÿé#  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è~ïüÿèù”üÿ‹E¶@ƒø‡P  Š€@œC ÿ$…YœC  	             
ƒC ‰œC ¥œC ÀœC ÛœC ïœC C C )C =C QC _C 3ÀŠ‰EØÛEØØ¸¿C ß.Şáß>›él  ¾‰EØÛEØØ¸¿C ß.Şáß>›éQ  ·‰EØÛEØØ¸¿C ß.Şáß>›é6  ßØ¸¿C ß.Şáß>›é"  ÛÜ.Ø¸¿C ß>›é  ÛØ¸¿C ß.Şáß>›éü  ÙØ¸¿C ß.Şáß>›éè  İØ¸¿C ß.Şáß>›éÔ  Û/Ø¸¿C ß.Şáß>›éÀ  ß.ß/Şéß>›é²  ß.EÜè#¤üÿEÜ‹×è§üÿEÜè£üÿß>›é  ¹¨¿C ²¸Xc@ èîüÿèƒ“üÿés  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èÎíüÿèI“üÿ‹E¶@ƒø‡  Š€ğC ÿ$…	C  	             
ûC 9C MC `C sC C ‹C —C £C ¯C »C ×C 3ÀŠ‰EØÛEØÜ.İ›éÄ  ¾‰EØÛEØÜ.İ›é±  ·‰EØÛEØÜ.İ›é  ßÜ.İ›é’  ÛÜ.İ›é†  ÛÜ.İ›éz  İØ'İ›én  İÜ'İ›éb  Û/Ü.İ›éV  İØ¸¿C ß/ŞéÛ-¼¿C ŞÉİ›é:  İEÜè{¢üÿEÜ‹×è¦üÿEÜè=¡üÿİ›é  ¹¨¿C ²¸Xc@ èìüÿè’üÿéû  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èVìüÿèÑ‘üÿ‹E¶@ƒø‡&  Š€hŸC ÿ$…ŸC  	             
 C ±ŸC ÇŸC ÜŸC ñŸC ÿŸC  C  C ' C 3 C A C ] C 3ÀŠ‰EØÛEØÛ.ŞáÛ>›éJ  ¾‰EØÛEØÛ.ŞáÛ>›é5  ·‰EØÛEØÛ.ŞáÛ>›é   ßÛ.ŞáÛ>›é  ÛÛ.ŞáÛ>›é  ÛÛ.ŞáÛ>›éö   Û.Ø'Û>›éê   Û.Ü'Û>›éŞ   Û.Û/ŞéÛ>›éĞ   Û.Ø¸¿C ß/ŞéÛ-¼¿C ŞÉÛ>›é´   Û.EÜèõ üÿEÜ‹×è¤üÿEÜè·ŸüÿÛ>›é   ¹¨¿C ²¸Xc@ è
ëüÿè…üÿëx‹U‹Çè•°ÿÿ(ëjMì‹U‹Çèøj  „Àu3ÛëU‹ÖEÜè§üÿEÜUìè/¤üÿUÜ‹Æèı¦üÿë4‹E;Eu‹E‹H‹×‹Æè¼{ÿÿë3Ûë²‹Eüèü¯ÿÿ3ÀZYYd‰éR  „Û…%  ²‹Eüèİ¯ÿÿ3ÀZYYd‰é3  ‹E¶@ƒø‡=	  ÿ$…G¡C }ªC «¡C ¾¡C Ñ¡C æ¡C û¡C  £C É¥C ;§C ­¨C }ªC }ªC }ªC }ªC }ªC }ªC *ªC }ªC }ªC }ªC }ªC }ªC }ªC `ªC E¤C ‹U‹Çèƒ¯ÿÿö.ˆéÖ  ‹U‹Çèd´ÿÿö.ˆéÃ  ‹U‹Çè]¯ÿÿf÷.f‰é®  ‹U‹Çè<´ÿÿf÷.f‰é™  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èqéüÿèìüÿ‹E¶@ƒø‡Å   Š€M¢C ÿ$…`¢C            £C „¢C “¢C ¡¢C ¯¢C ½¢C È¢C Ó¢C â¢C ‹3ÒŠ÷ê‰é  ‹¾÷ê‰éó  ‹·÷ê‰éå  ‹¿÷ê‰é×  ‹÷/‰éÌ  ‹÷/‰éÁ  ‹3ÒŠ÷ê‰é²  EÜ‹è8üÿEÜ‹×è¢üÿEÜè’œüÿ‰é  ¹¨¿C ²¸Xc@ è†èüÿèüÿét  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èLèüÿèÇüÿ‹E¶@ƒø‡Å   Š€r£C ÿ$……£C            *¤C ©£C ¸£C Æ£C Ô£C â£C í£C ø£C ¤C ‹3ÒŠ÷ê‰éÜ  ‹¾÷ê‰éÎ  ‹·÷ê‰éÀ  ‹¿÷ê‰é²  ‹÷/‰é§  ‹÷/‰éœ  ‹3ÒŠ÷ê‰é  EÜ‹èüÿEÜ‹×èñ üÿEÜèm›üÿ‰éj  ¹¨¿C ²¸Xc@ èaçüÿèÜŒüÿéO  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è'çüÿè¢Œüÿ‹E¶@ƒø‡$  Š€—¤C ÿ$…°¤C  	             
®¥C à¤C ö¤C ¥C  ¥C .¥C @¥C N¥C Z¥C f¥C t¥C Š¥C 3ÀŠ‰EØÛEØß.ŞÉß>›é  ¾‰EØÛEØß.ŞÉß>›é‰  ·‰EØÛEØß.ŞÉß>›ét  ßß.ŞÉß>›éf  ÛÜØ¸¿C ß>›éT  Ûß.ŞÉß>›éF  ß.Øß>›é:  ß.Üß>›é.  Û/ß.ŞÉß>›é   ß.ß/ŞÉÛ-¼¿C ŞÉß>›é
  ß.EÜèø›üÿEÜ‹×ènŸüÿEÜèæšüÿß>›éæ  ¹¨¿C ²¸Xc@ èİåüÿèX‹üÿéË  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è£åüÿè‹üÿ‹E¶@ƒø‡  Š€¦C ÿ$…4¦C  	             
 §C d¦C x¦C ‹¦C ¦C ª¦C ¶¦C Â¦C Î¦C Ú¦C æ¦C ü¦C 3ÀŠ‰EØÛEØØÙ›é  ¾‰EØÛEØØÙ›é	  ·‰EØÛEØØÙ›éö  ßØÙ›éê  ÛØÙ›éŞ  ÛØÙ›éÒ  ÙØÙ›éÆ  ÙÜÙ›éº  Û/ØÙ›é®  Ùß/ŞÉÛ-¼¿C ŞÉÙ›é˜  ÙEÜèVšüÿEÜ‹×èüüÿEÜè™üÿÙ›ét  ¹¨¿C ²¸Xc@ èkäüÿèæ‰üÿéY  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è1äüÿè¬‰üÿ‹E¶@ƒø‡  Š€§C ÿ$…¦§C  	             
’¨C Ö§C ê§C ı§C ¨C ¨C (¨C 4¨C @¨C L¨C X¨C n¨C 3ÀŠ‰EØÛEØÜİ›éª  ¾‰EØÛEØÜİ›é—  ·‰EØÛEØÜİ›é„  ßÜİ›éx  ÛÜİ›él  ÛÜİ›é`  İØİ›éT  İÜİ›éH  Û/Üİ›é<  İß/ŞÉÛ-¼¿C ŞÉİ›é&  İEÜèä˜üÿEÜ‹×èŠœüÿEÜè¦—üÿİ›é  ¹¨¿C ²¸Xc@ èùâüÿètˆüÿéç  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ è¿âüÿè:ˆüÿ‹E¶@ƒø‡   Š€ÿ¨C ÿ$…©C  	             
ªC H©C ^©C s©C ˆ©C –©C ¤©C ²©C ¾©C Ê©C Ø©C î©C 3ÀŠ‰EØÛEØÛ.ŞÉÛ>›é6  ¾‰EØÛEØÛ.ŞÉÛ>›é!  ·‰EØÛEØÛ.ŞÉÛ>›é  ßÛ.ŞÉÛ>›éş   ÛÛ.ŞÉÛ>›éğ   ÛÛ.ŞÉÛ>›éâ   Û.ØÛ>›éÖ   Û.ÜÛ>›éÊ   Û.Û/ŞÉÛ>›é¼   Û.ß/ŞÉÛ-¼¿C ŞÉÛ>›é¦   Û.EÜèd—üÿEÜ‹×è
›üÿEÜè&–üÿÛ>›é‚   ¹¨¿C ²¸Xc@ èyáüÿèô†üÿëjMì‹U‹Çèua  „Àu3ÛëU‹ÖEÜèüÿEÜUìè¸šüÿUÜ‹Æèzüÿë4‹E;Eu‹E‹H‹×‹ÆèQrÿÿë3Ûë²‹Eüèy¦ÿÿ3ÀZYYd‰éÏ  „Û…¢  ²‹EüèZ¦ÿÿ3ÀZYYd‰é°  ‹E¶@ƒø‡w	  Š€ĞªC ÿ$…éªC  	
             :´C «C 5«C P«C l«C ˆ«C Å¬C û­C k¯C í°C ]²C Ø³C ‹U‹Çè¦ÿÿP3ÀŠZ‹Ê™÷ùˆé	  ‹U‹ÇèíªÿÿP¾Z‹Ê™÷ùˆé	  ‹U‹ÇèŞ¥ÿÿP·Z‹Ê™÷ùf‰éå  ‹U‹Çè¶ªÿÿP¿Z‹Ê™÷ùf‰éÉ  ‹E€xu,‹ÇƒÀ‹ ‰E‹?…ÿtƒ} u¹¨¿C ²¸Xc@ èäßüÿè_…üÿ‹E¶@ƒø‡İ   Š€Ú«C ÿ$…í«C            ª¬C ¬C $¬C 6¬C H¬C/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* This file is complex.  However, the ultimate purpose is to make
   adding new configuration parameters easy.  Here's what you need to
   do to add a new parameter:

   1. Choose a name and type for the parameter.  By convention,
   parameters have lower case names and words_are_separated_like_this.
   Possible types are bool, int, char, and string.  (Nothing is ruling
   out floating point types; I just haven't needed them.)  As an
   example in the subsequent steps, suppose we wish to add a parameter
   foo_bar of type string.

   2. Add a field for the parameter to the 'params' struct defined
   below.  In our example, we would add the line
       struct param foo_bar;
   to the definition of struct params.

   Note that the order of the entries in this struct determines the
   order that the parameters will appear in the configuration file.

   3. Initialize and assign a default value to the parameter in the
   init_game_configuration() function.  The INIT_PARAM_<TYPE> macros
   will do this for you.  In our example, we would add the line
       INIT_PARAM_STRING( foo_bar, "baz" )
   to assign a default value of "baz" to the parameter foo_bar.

   4. Create the getparam/setparam functions for the parameter.  This
   is done using the FN_PARAM_<TYPE> macros.  In our example, we would
   add the line 
       FN_PARAM_STRING( foo_bar )
   somewhere in the top-level scope of this file (to keep things neat
   group it with the other definitions).  The will create
   getparam_foo_bar() and setparam_foo_bar() functions that can be
   used to query the value of the parameter.

   5. Create the prototypes for the getparam/setparam functions.  This
   is done in game_config.h using the PROTO_PARAM_<TYPE> macros.  In
   our example, we would add the line
       PROTO_PARAM_STRING( foo_bar );
   to game_config.h.

   6. You're done!  */

#include "file_util.h"
#include "game_config.h"
#include "string_util.h"
#include "course_mgr.h"
#include "winsys.h"
#include "ppgltk/audio/audio.h"


#if defined( WIN32 )
#  define OLD_CONFIG_FILE "etracer.cfg"
#else
#  define OLD_CONFIG_FILE ".etracer"
#endif /* defined( WIN32 ) */

#if defined( WIN32 )
#  define CONFIG_DIR "config"
#  define CONFIG_FILE "options.txt"
#else
#  define CONFIG_DIR ".etracer"
#  define CONFIG_FILE "options"
#endif /* defined( WIN32 ) */

#ifndef DATA_DIR
#  if defined( WIN32 )
#    define DATA_DIR "."
#  else
#    define DATA_DIR PP_DATADIR
#  endif /* defined( WIN32 ) */
#endif




static const char* sp_config_file=NULL;


/* Identifies the parameter type */
typedef enum {
    PARAM_STRING,
    PARAM_CHAR,
    PARAM_INT,
    PARAM_BOOL
} param_type;

/* Stores the value for all types */
typedef union {
    char* string_val;
    char  char_val;
    int   int_val;
    bool bool_val;
} param_val;

/* Stores state for each parameter */
struct param {
    int loaded;
    char *name;
    param_type type;
    param_val val;
    param_val deflt;
    char *comment;
};

/*
 * These macros are used to initialize parameter values
 */

#define INIT_PARAM( nam, val, typename, commnt ) \
   Params.nam.loaded = false; \
   Params.nam.name = #nam; \
   Params.nam.deflt.typename ## _val  = val; \
   Params.nam.comment = commnt;

#define INIT_PARAM_STRING( nam, val, commnt ) \
   INIT_PARAM( nam, val, string, commnt ); \
   Params.nam.type = PARAM_STRING;

#define INIT_PARAM_CHAR( nam, val, commnt ) \
   INIT_PARAM( nam, val, char, commnt ); \
   Params.nam.type = PARAM_CHAR;

#define INIT_PARAM_INT( nam, val, commnt ) \
   INIT_PARAM( nam, val, int, commnt ); \
   Params.nam.type = PARAM_INT;

#define INIT_PARAM_BOOL( nam, val, commnt ) \
   INIT_PARAM( nam, val, bool, commnt ); \
   Params.nam.type = PARAM_BOOL;


/*
 * These functions are used to get and set parameter values
 */

void fetch_param_string( struct param *p )
{
    const char *val;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    if ( val == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( val );
    }
    p->loaded = true;

}

void set_param_string( struct param *p, CONST84 char *new_val )
{
    const char *ret;

    check_assertion( p->type == PARAM_STRING, 
		     "configuration parameter type mismatch" );

    if ( p->loaded ) {
	free( p->val.string_val );
    }
    ret = Tcl_SetVar( tclInterp, p->name, new_val, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.string_val = string_copy( p->deflt.string_val );
    } else {
	p->val.string_val = string_copy( new_val );
    }
    p->loaded = true;

}

void fetch_param_char( struct param *p )
{
    const char *str_val;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL || str_val[0] == '\0' ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = str_val[0];
    }
    p->loaded = true;
}

void set_param_char( struct param *p, char new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_CHAR, 
		     "configuration parameter type mismatch" );

    buff[0] = new_val;
    buff[1] = '\0';

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.char_val = p->deflt.char_val;
    } else {
	p->val.char_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_int( struct param *p )
{
    CONST84 char *str_val;
    int val;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );
    
    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL 
	 || Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR  ) 
    {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = val;
    }
    p->loaded = true;
}

void set_param_int( struct param *p, int new_val )
{
    char buff[30];
    const char *ret;

    check_assertion( p->type == PARAM_INT, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.int_val = p->deflt.int_val;
    } else {
	p->val.int_val = new_val;
    }
    p->loaded = true;

}

void fetch_param_bool( struct param *p )
{
    CONST84 char *str_val;
    int val;
    bool no_val = false;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    str_val = Tcl_GetVar( tclInterp, p->name, TCL_GLOBAL_ONLY );
    
    if ( str_val == NULL ) {
	no_val = true;
    } else if ( strcmp( str_val, "false" ) == 0 ) {
	p->val.bool_val = false;
    } else if ( strcmp( str_val, "true" ) == 0 ) {
	p->val.bool_val = true;
    } else if ( Tcl_GetInt( tclInterp, str_val, &val) == TCL_ERROR ) {
	no_val = true;
    } else {
	p->val.bool_val = (val == 0) ? false : true ;
    }

    if ( no_val ) {
	p->val.bool_val = p->deflt.bool_val;
    }

    p->loaded = true;
}

void set_param_bool( struct param *p, bool new_val )
{
    char buff[2];
    const char *ret;

    check_assertion( p->type == PARAM_BOOL, 
		     "configuration parameter type mismatch" );

    sprintf( buff, "%d", new_val ? 1 : 0 );

    ret = Tcl_SetVar( tclInterp, p->name, buff, TCL_GLOBAL_ONLY );
    if ( ret == NULL ) {
	p->val.bool_val = p->deflt.bool_val;
    } else {
	p->val.bool_val = new_val;
    }
    p->loaded = true;
}


/*
 * Creates set/get functions for each parameter
 */
#define FN_PARAM( name, typename, type ) \
    type getparam_ ## name() { \
        if ( !Params.name.loaded ) { \
            fetch_param_ ## typename( &( Params.name ) ); \
        } \
        return Params.name.val.typename ## _val; \
    } \
    void setparam_ ## name( type val) { \
        set_param_ ## typename( &( Params.name ), val ); } 

#define FN_PARAM_STRING( name ) \
    FN_PARAM( name, string, char* )

#define FN_PARAM_CHAR( name ) \
    FN_PARAM( name, char, char )

#define FN_PARAM_INT( name ) \
    FN_PARAM( name, int, int )

#define FN_PARAM_BOOL( name ) \
    FN_PARAM( name, bool, bool )


/*
 * Main parameter struct
 */
struct params {
    struct param data_dir;
    struct param fullscreen;
    struct param x_resolution;
    struct param y_resolution;
    struct param x_resolution_half_width;		
    struct param bpp_mode;
    struct param capture_mouse; 
    struct param force_window_position;
    struct param quit_key;
    struct param turn_left_key;
    struct param turn_right_key;
    struct param trick_modifier_key;
    struct param brake_key;
    struct param paddle_key;
    struct param jump_key;
    struct param reset_key;
    struct param follow_view_key;
    struct param behind_view_key;
    struct param above_view_key;
    struct param view_mode; /* coresponds to view_mode_t */
    struct param screenshot_key;
    struct param pause_key;

    struct param joystick_paddle_button;
    struct param joystick_brake_button;
    struct param joystick_jump_button;
    struct param joystick_trick_button;
    struct param joystick_continue_button;
    struct param joystick_x_axis;
    struct param joystick_y_axis;
	struct param disable_joystick;

    struct param no_audio;
    struct param sound_enabled;
    struct param music_enabled;
    struct param sound_volume; /* 0-128 */
    struct param music_volume; /* 0-128 */
    struct param audio_freq_mode; /* 0 = 11025, 
				     1 = 22050, 
				     2 = 44100 */
    struct param audio_format_mode; /* 0 = 8 bits, 
				       1 = 16 bits */
    struct param audio_stereo; 
    struct param audio_buffer_size; 

    struct param display_fps;
	struct param display_course_percentage;	
	struct param course_detail_level;
    struct param forward_clip_distance;
    struct param backward_clip_distance;
    struct param tree_detail_distance;
    struct param terrain_blending;
    struct param perfect_terrain_blending;
    struct param terrain_envmap;
    struct param disable_fog;
		
    struct param stencil_buffer;
	struct param enable_fsaa;	
	struct param multisamples;
		
	struct param always_save_event_race_data;
		
	struct param draw_tux_shadow;	
    struct param tux_sphere_divisions;
    struct param tux_shadow_sphere_divisions;
    struct param draw_particles;
    struct param track_marks;
    struct param ui_snow;
    struct param nice_fog;
    struct param use_cva;
    struct param cva_hack;
    struct param use_sphere_display_list;
    struct param do_intro_animation;
    struct param mipmap_type; /* 0 = GL_NEAREST,
				 1 = GL_LINEAR,
				 2 = GL_NEAREST_MIPMAP_NEAREST,
				 3 = GL_LINEAR_MIPMAP_NEAREST,
				 4 = GL_NEAREST_MIPMAP_LINEAR,
				 5 = GL_LINEAR_MIPMAP_LINEAR
			      */
    struct param ode_solver; /* 0 = Euler,
				1 = ODE23,
				2 = ODE45
			     */
    struct param fov; 
    struct param debug; 
    struct param warning_level; 
    struct param write_diagnostic_log;
	struct param disable_collision_detection;
	struct param ui_language;
	struct param disable_videomode_autodetection;			
};

static struct params Params;


/*
 * Initialize parameter data
 */

void init_game_configuration()
{
    INIT_PARAM_STRING( 
	data_dir, DATA_DIR, 
	"# The location of the ET Racer data files" );

	INIT_PARAM_BOOL( 
	stencil_buffer, false, 
	"# Set this to true to activate the stencil buffer" );
	
	INIT_PARAM_BOOL( 
	enable_fsaa, false, 
	"# Set this to true to activate FSAA" );

	INIT_PARAM_INT( 
	multisamples, 2,
	"# Set multisamples for FSAA" );
	
    INIT_PARAM_BOOL( 
	draw_tux_shadow, false, 
	"# Set this to true to display Tux's shadow.  Note that this is a \n"
	"# hack and is quite expensive in terms of framerate.\n"
	"# [EXPERT] This looks better if your card has a stencil buffer; \n"
	"# if compiling use the --enable-stencil-buffer configure option \n"
	"# to enable the use of the stencil buffer" );
	
	

    INIT_PARAM_BOOL( 
	draw_particles, true,
	"# Controls the drawing of snow particles that are kicked up as Tux\n"
	"# turns and brakes.  Setting this to false should help improve \n"
	"# performance." );

    INIT_PARAM_INT( 
	tux_sphere_divisions, 15,
	"# [EXPERT] Higher values result in a more finely subdivided mesh \n"
	"# for Tux, and vice versa.  If you're experiencing low framerates,\n"
	"# try lowering this value." );

    INIT_PARAM_INT( 
	tux_shadow_sphere_divisions, 3,
	"# [EXPERT] The level of subdivision of Tux's shadow." );

    INIT_PARAM_BOOL( 
	nice_fog, false,
	"# [EXPERT] If true, then the GL_NICEST hint will be used when\n"
	"# rendering fog.  On some cards, setting this to false may improve\n"
	"# performance.");

    INIT_PARAM_BOOL( 
	use_sphere_display_list, true,
	"# [EXPERT]  Mesa 3.1 sometimes renders Tux strangely when display \n"
	"# lists are used.  Setting this to false should solve the problem \n"
	"# at the cost of a few Hz." );

    INIT_PARAM_BOOL( 
	display_fps, false,
	"# Set this to true to display the current framerate in Hz." );

    INIT_PARAM_BOOL( 
	display_course_percentage, true,
	"# Set this to true to display a progressbar of \n"
	"# the course percentage." );

    INIT_PARAM_INT( 
	x_resolution, 800,
	"# The horizontal size of the Tux Racer window" );

    INIT_PARAM_INT( 
	y_resolution, 600,
	"# The vertical size of the Tux Racer window" );

	INIT_PARAM_BOOL( 
	x_resolution_half_width, false, 
	"# Set this to true to use only half of the resolution width" );

    INIT_PARAM_BOOL( 
	capture_mouse, false,
	"# If true, then the mouse will not be able to leave the \n"
	"# Tux Racer window.\n"
	"# If you lose keyboard focus while running Tux Racer, try setting\n"
	"# this to true." );

    INIT_PARAM_BOOL( 
	do_intro_animation, true,
	"# If false, then the introductory animation sequence will be skipped." 
	);

    INIT_PARAM_INT( 
	mipmap_type, 5,
	"# [EXPERT] Allows you to control which type of texture\n"
	"# interpolation/mipmapping is used when rendering textures.  The\n"
	"# values correspond to the following OpenGL settings:\n"
	"#\n"
        "#  0: GL_NEAREST\n"
        "#  1: GL_LINEAR\n"
        "#  2: GL_NEAREST_MIPMAP_NEAREST\n"
	"#  3: GL_LINEAR_MIPMAP_NEAREST\n"
        "#  4: GL_NEAREST_MIPMAP_LINEAR\n"
        "#  5: GL_LINEAR_MIPMAP_LINEAR\n"
	"#\n"
	"# On some cards, you may be able to improve performance by\n"
        "# decreasing this number, at the cost of lower image quality." );

    INIT_PARAM_BOOL( 
	fullscreen, true,
	"# If true then the game will run in full-screen mode." );

    INIT_PARAM_INT( 
	bpp_mode, 0,
	"# Controls how many bits per pixel are used in the game.\n"
	"# Valid values are:\n"
	"#\n"
	"#  0: Use current bpp setting of operating system\n"
	"#  1: 16 bpp\n"
	"#  2: 32 bpp\n"
	"# Note that some cards (e.g., Voodoo1, Voodoo2, Voodoo3) only support\n"
	"# 16 bits per pixel." );

    INIT_PARAM_BOOL( 
	force_window_position, false ,
	"# If true, then the Tux Racer window will automatically be\n"
	"# placed at (0,0)" );

    INIT_PARAM_INT( 
	ode_solver, 2 ,
	"# Selects the ODE (ordinary differential equation) solver.  \n"
	"# Possible values are:\n"
	"#\n"
	"#   0: Modified Euler     (fastest but least accurate)\n"
        "#   1: Runge-Kutta (2,3)\n"
	"#   2: Runge-Kutta (4,5)  (slowest but most accurate)" );

    INIT_PARAM_STRING( 
	quit_key, "q escape" ,
	"# Key binding for quitting a race" );
    INIT_PARAM_INT( 
	turn_left_key, SDLK_LEFT ,
	"# Key binding for turning left" );
    INIT_PARAM_INT( 
	turn_right_key, SDLK_RIGHT ,
	"# Key binding for turning right" );
    INIT_PARAM_INT( 
	trick_modifier_key, 't' ,
	"# Key binding for doing tricks" );
    INIT_PARAM_INT( 
	brake_key, SDLK_DOWN ,
	"# Key binding for braking" );
    INIT_PARAM_INT( 
	paddle_key, SDLK_UP ,
	"# Key binding for paddling (on the ground) and flapping (in the air)" 
	);
    INIT_PARAM_STRING( 
	follow_view_key, "1" ,
	"# Key binding for the \"Follow\" camera mode" );
    INIT_PARAM_STRING( 
	behind_view_key, "2" ,
	"# Key binding for the \"Behind\" camera mode" );
    INIT_PARAM_STRING( 
	above_view_key, "3" ,
	"# Key binding for the \"Above\" camera mode" );
    INIT_PARAM_INT( 
	view_mode, 1 ,
	"# Default view mode. Possible values are\n" 
	"#\n"
	"#   0: Behind\n"
	"#   1: Follow\n"
	"#   2: Above" );
    INIT_PARAM_STRING( 
	screenshot_key, "=" ,
	"# Key binding for taking a screenshot" );
    INIT_PARAM_STRING( 
	pause_key, "p" ,
	"# Key binding for pausing the game" );
    INIT_PARAM_INT( 
	reset_key, 'r' ,
	"# Key binding for resetting the player position" );
    INIT_PARAM_INT( 
	jump_key, 'e' ,
	"# Key binding for jumping" );

    INIT_PARAM_INT( 
	joystick_paddle_button, 0 ,
	"# Joystick button for paddling (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_brake_button, 2 ,
	"# Joystick button for braking (numbering starts at 0).\n" 
	"# Set to -1 to disable." );

    INIT_PARAM_INT( 
	joystick_jump_button, 3 ,
	"# Joystick button for jumping (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_trick_button, 1 ,
	"# Joystick button for doing tricks (numbering starts at 0)" );

    INIT_PARAM_INT( 
	joystick_continue_button, 0 ,
	"# Joystick button for moving past intro, paused, and \n"
	"# game over screens (numbering starts at 0)" );
    
    INIT_PARAM_INT(
	joystick_x_axis, 0 ,
	"# Joystick axis to use for turning (numbering starts at 0)" );

    INIT_PARAM_INT(
	joystick_y_axis, 1 ,
	"# Joystick axis to use for paddling/braking (numbering starts at 0)" );
   
	INIT_PARAM_BOOL(
	disable_joystick, false ,
	"# Disables the joystick support" );

    INIT_PARAM_INT( 
	fov, 60 ,
	"# [EXPERT] Sets the camera field-of-view" );
    INIT_PARAM_STRING( 
	debug, "" ,
	"# [EXPERT] Controls the Tux Racer debugging modes" );
    INIT_PARAM_INT( 
	warning_level, 100 ,
	"# [EXPERT] Controls the Tux Racer warning messages" );
    INIT_PARAM_INT( 
	forward_clip_distance, 100 ,
	"# Controls how far ahead of the camera the course\n"
	"# is rendered.  Larger values mean that more of the course is\n"
	"# rendered, resulting in slower performance. Decreasing this \n"
	"# value is an effective way to improve framerates." );
    INIT_PARAM_INT( 
	backward_clip_distance, 10 ,
	"# [EXPERT] Some objects aren't yet clipped to the view frustum, \n"
	"# so this value is used to control how far up the course these \n"
	"# objects are drawn." );
    INIT_PARAM_INT( 
	tree_detail_distance, 20 ,
	"# [EXPERT] Controls the distance at which trees are drawn with \n"
	"# two rectangles instead of one." );
    INIT_PARAM_BOOL( 
	terrain_blending, true ,
	"# Controls the blending of the terrain textures.  Setting this\n"
	"# to false will help improve performance." );
    INIT_PARAM_BOOL( 
	perfect_terrain_blending, false ,
	"# [EXPERT] If true, then terrain triangles with three different\n"
	"# terrain types at the vertices will be blended correctly\n"
	"# (instead of using a faster but imperfect approximation)." );
    INIT_PARAM_BOOL( 
	terrain_envmap, true ,
	"# If true, then the ice will be drawn with an \"environment map\",\n"
	"# which gives the ice a shiny appearance.  Setting this to false\n"
	"# will help improve performance." );
    INIT_PARAM_BOOL( 
	disable_fog, false ,
	"# If true, then fog will be turned off.  Some Linux drivers for the\n"
	"# ATI Rage128 seem to have a bug in their fog implementation which\n"
	"# makes the screen nearly pure white when racing; if you experience\n"
	"# this problem then set this variable to true." );
    INIT_PARAM_BOOL( 
	use_cva, true ,
	"# [EXPERT] If true, then compiled vertex arrays will be used when\n"
	"# drawing the terrain.  Whether or not this helps performance\n"
	"# is driver- and card-dependent." );
    INIT_PARAM_BOOL( 
	cva_hack, true ,
	"# Some card/driver combinations render the terrrain incorrectly\n"
	"# when using compiled vertex arrays.  This activates a hack \n"
	"# to work around that problem." );
    INIT_PARAM_INT( 
	course_detail_level, 75 ,
	"# [EXPERT] This controls how accurately the course terrain is \n"
	"# rendered. A high value results in greater accuracy at the cost of \n"
	"# performance, and vice versa.  This value can be decreased and \n"
	"# increased in 10% increments at runtime using the F9 and F10 keys.\n"
	"# To better see the effect, activate wireframe mode using the F11 \n"
	"# key (this is a toggle)." );
    INIT_PARAM_BOOL( 
	no_audio, false ,
	"# If true, then audio in the game is completely disabled." );
    INIT_PARAM_BOOL( 
	sound_enabled, true ,
	"# Use this to turn sound effects on and off." );
    INIT_PARAM_BOOL( 
	music_enabled, true ,
	"# Use this to turn music on and off." );
    INIT_PARAM_INT( 
	sound_volume, 64 ,
	"# This controls the sound volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	music_volume, 127 ,
	"# This controls the music volume (valid range is 0-127)." );
    INIT_PARAM_INT( 
	audio_freq_mode, 1 ,
	"# The controls the frequency of the audio.  Valid values are:\n"
	"# \n"
	"#   0: 11025 Hz\n"
	"#   1: 22050 Hz\n"
	"#   2: 44100 Hz" );
    INIT_PARAM_INT( 
	audio_format_mode, 1 ,
	"# This controls the number of bits per sample for the audio.\n"
	"# Valid values are:\n"
	"#\n"
	"#   0: 8 bits\n"
	"#   1: 16 bits" );
    INIT_PARAM_BOOL( 
	audio_stereo, true ,
	"# Audio will be played in stereo of true, and mono if false" );
    INIT_PARAM_INT( 
	audio_buffer_size, 2048 ,
	"# [EXPERT] Controls the size of the audio buffer.  \n"
	"# Increase the buffer size if you experience choppy audio\n" 
	"# (at the cost of greater audio latency)" );
    INIT_PARAM_BOOL( 
	track_marks, true ,
	"# If true, then the players will leave track marks in the snow." );
    INIT_PARAM_BOOL( 
	ui_snow, true ,
	"# If true, then the ui screens will have falling snow." );

    INIT_PARAM_BOOL( 
	write_diagnostic_log, false ,
	"# If true, then a file called diagnostic_log.txt will be generated\n" 
	"# which you should attach to any bug reports you make.\n"
	"# To generate the file, set this variable to \"true\", and\n"
	"# then run the game so that you reproduce the bug, if possible."
	);
	
    INIT_PARAM_BOOL( 
	always_save_event_race_data, false ,
	"# only for cheating purpose"
	);	
	
	INIT_PARAM_BOOL( 
	disable_collision_detection, false ,
	"# If true, collision detection with tree models is disabled"
	);
	
	INIT_PARAM_BOOL( 
	disable_videomode_autodetection, false, 
	"# Set this to true disable the autodetection\n"
	"# for available video modes." );
		
	INIT_PARAM_STRING( 
	ui_language, "en_GB" ,
	"# set the language for the ui"
	);
	
}


/* 
 * Create the set/get functions for parameters
 */

FN_PARAM_STRING( data_dir )
FN_PARAM_BOOL( draw_tux_shadow )
FN_PARAM_BOOL( draw_particles )
FN_PARAM_INT( tux_sphere_divisions )
FN_PARAM_INT( tux_shadow_sphere_divisions )
FN_PARAM_BOOL( nice_fog )
FN_PARAM_BOOL( use_sphere_display_list )
FN_PARAM_BOOL( display_fps )
FN_PARAM_BOOL( display_course_percentage )
FN_PARAM_INT( x_resolution )
FN_PARAM_INT( y_resolution )
FN_PARAM_BOOL( x_resolution_half_width )
FN_PARAM_BOOL( capture_mouse )
FN_PARAM_BOOL( do_intro_animation )
FN_PARAM_INT( mipmap_type )
FN_PARAM_BOOL( fullscreen )
FN_PARAM_INT( bpp_mode )
FN_PARAM_BOOL( force_window_position )
FN_PARAM_INT( ode_solver )
FN_PARAM_STRING( quit_key )

FN_PARAM_INT( turn_left_key )
FN_PARAM_INT( turn_right_key )
FN_PARAM_INT( trick_modifier_key )
FN_PARAM_INT( brake_key )
FN_PARAM_INT( paddle_key )
FN_PARAM_STRING( above_view_key )
FN_PARAM_STRING( behind_view_key )
FN_PARAM_STRING( follow_view_key )
FN_PARAM_INT( view_mode )
FN_PARAM_STRING( screenshot_key )
FN_PARAM_STRING( pause_key )
FN_PARAM_INT( reset_key )
FN_PARAM_INT( jump_key )
FN_PARAM_INT( joystick_jump_button )
FN_PARAM_INT( joystick_brake_button )
FN_PARAM_INT( joystick_paddle_button )
FN_PARAM_INT( joystick_trick_button )
FN_PARAM_INT( joystick_continue_button )
FN_PARAM_INT( joystick_x_axis )
FN_PARAM_INT( joystick_y_axis )
FN_PARAM_BOOL ( disable_joystick )
FN_PARAM_INT( fov )
FN_PARAM_STRING( debug )
FN_PARAM_INT( warning_level )
FN_PARAM_INT( forward_clip_distance )
FN_PARAM_INT( backward_clip_distance )
FN_PARAM_INT( tree_detail_distance )
FN_PARAM_INT( course_detail_level )
FN_PARAM_BOOL( terrain_blending )
FN_PARAM_BOOL( perfect_terrain_blending )
FN_PARAM_BOOL( terrain_envmap )
FN_PARAM_BOOL( disable_fog )
FN_PARAM_BOOL( use_cva )
FN_PARAM_BOOL( cva_hack )
FN_PARAM_BOOL( track_marks )
FN_PARAM_BOOL( ui_snow )

FN_PARAM_BOOL( no_audio )
FN_PARAM_BOOL( sound_enabled )
FN_PARAM_BOOL( music_enabled )
FN_PARAM_INT( sound_volume )
FN_PARAM_INT( music_volume )
FN_PARAM_INT( audio_freq_mode )
FN_PARAM_INT( audio_format_mode )
FN_PARAM_BOOL( audio_stereo )
FN_PARAM_INT( audio_buffer_size )
FN_PARAM_BOOL( write_diagnostic_log )

FN_PARAM_BOOL( stencil_buffer )
FN_PARAM_BOOL( enable_fsaa )
FN_PARAM_INT( multisamples )

FN_PARAM_BOOL( always_save_event_race_data )
FN_PARAM_BOOL( disable_collision_detection )
FN_PARAM_BOOL( disable_videomode_autodetection )

FN_PARAM_STRING( ui_language )


/*
 * Functions to read and write the configuration file
 */

int get_old_config_file_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( OLD_CONFIG_FILE ) +1 > len ) {
	return 1;
    }
    strcpy( buff, OLD_CONFIG_FILE );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( OLD_CONFIG_FILE ) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, OLD_CONFIG_FILE );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_dir_name( char *buff, unsigned int len )
{
#if defined( WIN32 ) 
    if ( strlen( CONFIG_DIR ) +1 > len ) {
	return 1;
    }
    strcpy( buff, CONFIG_DIR );
    return 0;
#else
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 1;
    }

    if ( strlen( pwent->pw_dir ) + strlen( CONFIG_DIR) + 2 > len ) {
	return 1;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, CONFIG_DIR );
    return 0;
#endif /* defined( WIN32 ) */
}

int get_config_file_name( char *buff, unsigned int len )
{
    if (get_config_dir_name( buff, len ) != 0) {
	return 1;
    }
    if ( strlen( buff ) + strlen( CONFIG_FILE ) +2 > len ) {
	return 1;
    }

#if defined( WIN32 ) 
    strcat( buff, "\\" );
#else
    strcat( buff, "/" );
#endif /* defined( WIN32 ) */

    strcat( buff, CONFIG_FILE);
    return 0;
}

void clear_config_cache()
{
    struct param *parm;
    unsigned int i;

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	parm->loaded = false;
    }
}

void read_config_file(std::string& file)
{
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];

    clear_config_cache();

	if( !file.empty()){
		if ( Tcl_EvalFile( tclInterp, FUCKTCL file.c_str() ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", file.c_str(),
			      Tcl_GetStringResult( tclInterp ) );
	    }	
		sp_config_file = file.c_str();	
		return;
	}else{
		sp_config_file = NULL;
	}
	
    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
		return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
		return;
    }

	

    if ( dir_exists( config_dir ) ) {
	if ( file_exists( config_file ) ) {
	    /* File exists -- let's try to evaluate it. */
	    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
		handle_error( 1, "error evalating %s: %s", config_file,
			      Tcl_GetStringResult( tclInterp ) );
	    }
	}
	return;
    }

    /* File does not exist -- look for old version */
    if ( get_old_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( !file_exists( config_file ) ) {
	return;
    }
    /* Old file exists -- let's try to evaluate it. */
    if ( Tcl_EvalFile( tclInterp, config_file ) != TCL_OK ) {
	handle_error( 1, "error evalating deprecated %s: %s", config_file,
		      Tcl_GetStringResult( tclInterp ) );
    } else {
	/* Remove old file and save info in new file location */
	remove(config_file);
	write_config_file();
    }
}

void write_config_file()
{
    FILE *config_stream;
    char config_file[BUFF_LEN];
    char config_dir[BUFF_LEN];
    struct param *parm;
    unsigned int i;
	
	if(sp_config_file==NULL){

    if ( get_config_file_name( config_file, sizeof( config_file ) ) != 0 ) {
	return;
    }
    if ( get_config_dir_name( config_dir, sizeof( config_dir ) ) != 0 ) {
	return;
    }

    if ( !dir_exists( config_dir ) ) {

#if defined(WIN32) && !defined(__CYGWIN__)
	if (mkdir( config_dir ) != 0) {
	    return;
	}
#else
	if (mkdir( config_dir, 0775) != 0) {
	    return;
	}
#endif

    }

    config_stream = fopen( config_file, "w" );
	if ( config_stream == NULL ) {
	print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       config_file, strerror(errno) );
	return;
    }
	
	}else{
		std::cout << "Writing to custom config file: "
				  << sp_config_file << std::endl;
		config_stream = fopen( sp_config_file, "w" );
		if ( config_stream == NULL ) {
			print_warning( CRITICAL_WARNING, 
		       "couldn't open %s for writing: %s", 
		       sp_config_file, strerror(errno) );
			return;
    	}
	}
	
    fprintf( config_stream, 
	     "# PP Racer " VERSION " configuration file\n"
	     "#\n"
	);

    for (i=0; i<sizeof(Params)/sizeof(struct param); i++) {
	parm = (struct param*)&Params + i;
	if ( parm->comment != NULL ) {
	    fprintf( config_stream, "\n# %s\n#\n%s\n#\n", 
		     parm->name, parm->comment );
	}
	switch ( parm->type ) {
	case PARAM_STRING:
	    fetch_param_string( parm );
	    fprintf( config_stream, "set %s \"%s\"\n",
		     parm->name, parm->val.string_val );
	    break;
	case PARAM_CHAR:
	    fetch_param_char( parm );
	    fprintf( config_stream, "set %s %c\n",
		     parm->name, parm->val.char_val );
	    break;
	case PARAM_INT:
	    fetch_param_int( parm );
	    fprintf( config_stream, "set %s %d\n",
		     parm->name, parm->val.int_val );
	    break;
	case PARAM_BOOL:
	    fetch_param_bool( parm );
	    fprintf( config_stream, "set %s %s\n",
		     parm->name, parm->val.bool_val ? "true" : "false" );
	    break;
	default:
	    code_not_reached();
	}
    }

    if ( fclose( config_stream ) != 0 ) {
	perror( "fclose" );
    }
}

/*
 * Tcl callback to allow reading of game configuration variables from Tcl.
 */
static int get_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int num_params;
    struct param *parm;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Get value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	fetch_param_string( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( parm->val.string_val, -1 ) );
	break;

    case PARAM_CHAR:
	fetch_param_char( parm );
	Tcl_SetObjResult( ip, Tcl_NewStringObj( &parm->val.char_val, 1 ) );
	break;

    case PARAM_INT:
	fetch_param_int( parm );
	Tcl_SetObjResult( ip, Tcl_NewIntObj( parm->val.int_val ) );
	break;

    case PARAM_BOOL:
	fetch_param_bool( parm );
	Tcl_SetObjResult( ip, Tcl_NewBooleanObj( parm->val.bool_val ) );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

/* 
 * Tcl callback to allow setting of game configuration variables from Tcl.
 */
static int set_param_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int i;
    int tmp_int;
    int num_params;
    struct param *parm;

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <parameter name> <value>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    /* Search for parameter */
    parm = NULL;
    num_params = sizeof(Params)/sizeof(struct param);
    for (i=0; i<num_params; i++) {
	parm = (struct param*)&Params + i;

	if ( strcmp( parm->name, argv[1] ) == 0 ) {
	    break;
	}
    }

    /* If can't find parameter, report error */
    if ( parm == NULL || i == num_params ) {
	Tcl_AppendResult(ip, argv[0], ": invalid parameter `",
			 argv[1], "'", (char *)0 );
	return TCL_ERROR;
    }

    /* Set value of parameter */
    switch ( parm->type ) {
    case PARAM_STRING:
	set_param_string( parm, argv[2] ); 
	break;

    case PARAM_CHAR:
	if ( strlen( argv[2] ) > 1 ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a single character", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_char( parm, argv[2][0] );
	break;

    case PARAM_INT:
	if ( Tcl_GetInt( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be an integer", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	set_param_int( parm, tmp_int );
	break;

    case PARAM_BOOL:
	if ( Tcl_GetBoolean( ip, argv[2], &tmp_int ) != TCL_OK ) {
	    Tcl_AppendResult(ip, "\n", argv[0], ": value for `",
			     argv[1], "' must be a boolean", 
			     (char *)0 );
	    return TCL_ERROR;
	}
	check_assertion( tmp_int == 0 || tmp_int == 1, 
			 "invalid boolean value" );
	set_param_bool( parm, (bool) tmp_int );
	break;

    default:
	code_not_reached();
    }

    return TCL_OK;
} 

void register_game_config_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_get_param", get_param_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_set_param", set_param_cb,   0,0);
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-13T23:52:36.812478Z
65
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

translation.cpp
file
63



2007-12-31T17:50:28.000000Z
46ad8b2374c2e87a62faf32c199b4733
2007-09-03T03:13:45.611773Z
5
botsnlinux

credits.h
file
63



2007-12-31T17:50:28.000000Z
6ee29387d6bbc037b230bcfdefcc413e
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystick.h
file
63



2007-12-31T17:50:28.000000Z
714af5dcde494b89db59afa225d9049e
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_mgr.cpp
file
63



2007-12-31T17:50:28.000000Z
1ce9cc5ed16287f9568367c5b7d12d0a
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_race_select.cpp
file
63



2007-12-31T17:50:28.000000Z
e1a8e6edf3a14a84b623973b0cc6b4e6
2007-09-19T16:04:42.494002Z
8
Torandi

fog.cpp
file
63



2007-12-31T17:50:28.000000Z
a1b686f524c28d12ce3c687d8fea12b7
2007-09-01T16:38:12.025871Z
2
botsnlinux

race_select.h
file
63



2007-12-31T17:50:28.000000Z
e7c45e8d223baf9460eb0b22d504d6fb
2007-09-13T20:23:04.446280Z
6
Torandi

screenshot.h
file
63



2007-12-31T17:50:28.000000Z
ec6a68da1757b29f6dd3ca4cc0db5673
2007-09-01T16:38:12.025871Z
2
botsnlinux

loading.h
file
63



2007-12-31T17:50:28.000000Z
d336e5240a4ea8b2cdc3178ffc532df7
2007-09-01T16:38:12.025871Z
2
botsnlinux

bench.h
file
63



2007-12-31T17:50:28.000000Z
1807e48e3c90ea6561a27486dfe1717c
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_util.cpp
file
63



2007-12-31T17:50:28.000000Z
7ad8fc290a384809d848b80583cffb9d
2007-09-01T16:38:12.025871Z
2
botsnlinux

paused.h
file
63



2007-12-31T17:50:28.000000Z
c555fe819fa04859f83fdf83d39b9e3f
2007-09-01T16:38:12.025871Z
2
botsnlinux

textures.h
file
63



2007-12-31T17:50:28.000000Z
bb01b1edae4b00fc32ecd7d9122eab6b
2007-09-03T03:13:45.611773Z
5
botsnlinux

debug.h
file
63



2007-12-31T17:50:28.000000Z
fd2d15b3332cf8b082bd07424e0490fd
2007-09-03T03:13:45.611773Z
5
botsnlinux

audioconfig.h
file
63



2007-12-31T17:50:28.000000Z
d9abf934d93b15295a7a7502d287e66a
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_config.cpp
file




2007-12-31T17:50:28.000000Z
08f991e6e922bef060df0b8024a69d86
2008-01-13T23:52:36.812478Z
65
cpicon92

splash_screen.h
file
63



2007-12-31T17:50:28.000000Z
5a63e2cf04de3e4e5064dc54954f97f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystick.cpp
file
63



2007-12-31T17:50:28.000000Z
a9f5f62503888d303901c0c3ef44d7ab
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_cb.h
file
63



2007-12-31T17:50:28.000000Z
23ae4e88dca2f0cd4c303f6c7c6a8d1e
2007-09-01T16:38:12.025871Z
2
botsnlinux

translation.h
file
63



2007-12-31T17:50:28.000000Z
f89d1b21334ab2826168a92b3f5d9c44
2007-09-03T03:13:45.611773Z
5
botsnlinux

viewfrustum.h
file
63



2007-12-31T17:50:28.000000Z
5643cdc0f24e9c4ddd20655c1d59752f
2007-09-01T16:38:12.025871Z
2
botsnlinux

pp_types.h
file
63



2007-12-31T17:50:28.000000Z
be2ffc5d14defc64a5abf8e3faa87ed4
2007-09-03T03:13:45.611773Z
5
botsnlinux

quadtree.cpp
file
63



2007-12-31T17:50:28.000000Z
e813b1877f0dff86b02794f4f02f346b
2007-09-01T16:38:12.025871Z
2
botsnlinux

file_util.cpp
file
63



2007-12-31T17:50:28.000000Z
e5d4f65d1ddd22d98544fca33db312c3
2007-09-03T03:13:45.611773Z
5
botsnlinux

bench.cpp
file
63



2007-12-31T17:50:28.000000Z
613c1b52140663ee96acfc652c911918
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_race_select.h
file
63



2007-12-31T17:50:28.000000Z
60aa7641d158a3b01051ca943560190b
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_select.cpp
file
63



2007-12-31T17:50:28.000000Z
d28a888b9e5eeed7748b3305f43689fe
2007-09-21T22:57:14.407036Z
19
Torandi

nmrcl.h
file
63



2007-12-31T17:50:28.000000Z
b25c45c14f98d71751585950fb1f40a4
2007-09-01T16:38:12.025871Z
2
botsnlinux

tex_font_metrics.h
file
63



2007-12-31T17:50:28.000000Z
47d319d3104615df2a57586b15e400ec
2007-09-01T16:38:12.025871Z
2
botsnlinux

textures.cpp
file
63



2007-12-31T17:50:28.000000Z
c0f3398adb917d3f4e6fc6c47e65b6f9
2007-09-01T16:38:12.025871Z
2
botsnlinux

snow.cpp
file
63



2007-12-31T17:50:28.000000Z
b163d4202982e464e1dc327030587332
2007-09-01T16:38:12.025871Z
2
botsnlinux

audioconfig.cpp
file
63



2007-12-31T17:50:28.000000Z
0d63f308d1ea5aaa55f137ac99662b10
2007-09-01T16:38:12.025871Z
2
botsnlinux

mirror_course.cpp
file
63



2007-12-31T17:50:28.000000Z
c0990646e6cb1a7d6cbb230e2df0102a
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystickconfig.h
file
63



2007-12-31T17:50:28.000000Z
fd8ad50b7ad9dc5fb72ad2b089c3cbc7
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_cb.cpp
file
63



2007-12-31T17:50:28.000000Z
cfda7da6221104305a4b251ced45024f
2007-09-01T16:38:12.025871Z
2
botsnlinux

track_marks.cpp
file
63



2007-12-31T17:50:28.000000Z
986f0cf780750a7b31fba7b343e47bb2
2007-09-19T22:16:13.729253Z
9
Torandi

winsys.h
file
63



2007-12-31T17:50:28.000000Z
a92c6535d1bf43a1eb4b999bc436bb0a
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_config.h
file
63



2007-12-31T17:50:28.000000Z
f5eedd174003089ffe09484485b8767f
2007-09-03T03:13:45.611773Z
5
botsnlinux

keyframe.h
file
63



2007-12-31T17:50:28.000000Z
ef519de490fedd6f0647017a4ffc6741
2007-09-01T16:38:12.025871Z
2
botsnlinux

loop.cpp
file
63



2007-12-31T17:50:28.000000Z
8a548dfc192252949a8d7ffb445f37ee
2007-09-13T20:23:04.446280Z
6
Torandi

game_over.h
file
63



2007-12-31T17:50:28.000000Z
9008b29b549f2ded63e70ae64f85cc5a
2007-09-13T20:23:04.446280Z
6
Torandi

nmrcl.cpp
file
63



2007-12-31T17:50:28.000000Z
9a9d44c2862cf8f6bf8077d82466bd71
2007-09-03T03:13:45.611773Z
5
botsnlinux

quadtree.h
file
63



2007-12-31T17:50:28.000000Z
2a96744d6f751e3780508d5b1a5589af
2007-09-01T16:38:12.025871Z
2
botsnlinux

render_util.h
file
63



2007-12-31T17:50:28.000000Z
ff1ffab99ccb32809dbf47cab06f78d1
2007-09-01T16:38:12.025871Z
2
botsnlinux

reset.cpp
file
63



2007-12-31T17:50:28.000000Z
814e66d1764649e8c703192b7cb50236
2007-09-19T22:16:13.729253Z
9
Torandi

snow.h
file
63



2007-12-31T17:50:28.000000Z
de12b5e3bec667a310fb6d6a961af7b4
2007-09-01T16:38:12.025871Z
2
botsnlinux

fps.h
file
63



2007-12-31T17:50:28.000000Z
4a2c8cdbe071872b9521d49f541fded2
2007-09-01T16:38:12.025871Z
2
botsnlinux

mirror_course.h
file
63



2007-12-31T17:50:28.000000Z
8e0344474d10ef92151b48c9cc1a3890
2007-09-01T16:38:12.025871Z
2
botsnlinux

phys_sim.h
file
63



2007-12-31T17:50:28.000000Z
b4ca76d36ee16c5a727c1b654a7bd3aa
2007-09-01T16:38:12.025871Z
2
botsnlinux

player.h
file
63



2007-12-31T17:50:28.000000Z
d4039073405442b897c7d94ab923c4e8
2007-09-19T22:16:13.729253Z
9
Torandi

keyframe.cpp
file
63



2007-12-31T17:50:28.000000Z
66ba07bba081b22977a0aeb5d24ef121
2007-09-19T22:16:13.729253Z
9
Torandi

model_hndl.cpp
file
63



2007-12-31T17:50:28.000000Z
6734b47b287febc57a083962578e11d8
2007-09-19T22:16:13.729253Z
9
Torandi

game_type_select.cpp
file
63



2007-12-31T17:50:28.000000Z
6f5253332f948afc4e0e7ae56cad0843
2007-09-13T20:23:04.446280Z
6
Torandi

configmode.cpp
file
63



2007-12-31T17:50:28.000000Z
f89a6ea297db9ab874542aa00a135883
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_mgr.h
file
63



2007-12-31T17:50:28.000000Z
caf56d731e3ee59b96186afd3b5c8f72
2007-09-03T03:13:45.611773Z
5
botsnlinux

configuration.h
file
63



2007-12-31T17:50:28.000000Z
9ad764dbb642d094fd7a0c079d233c60
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_mgr.cpp
file
63



2007-12-31T17:50:28.000000Z
f6e212ffe254fb019ccb767ab2538ff3
2007-09-01T16:38:12.025871Z
2
botsnlinux

tcl_util.h
file
63



2007-12-31T17:50:28.000000Z
4e7b975a3da8eb9a34e9eb25bf55845b
2007-09-03T03:13:45.611773Z
5
botsnlinux

hud.h
file
63



2007-12-31T17:50:28.000000Z
2dbeb01ebd3e1497c88b8dfdaf1d200b
2007-09-01T16:38:12.025871Z
2
botsnlinux

reset.h
file
63



2007-12-31T17:50:28.000000Z
7dbedb1b06022ab1b4a10da139b73ac8
2007-09-01T16:38:12.025871Z
2
botsnlinux

Makefile.am
file
63



2007-12-31T17:50:28.000000Z
3bb051af1c061cf1107d808582093c84
2007-09-22T05:44:23.419072Z
27
botsnlinux

phys_sim.cpp
file
63



2007-12-31T17:50:28.000000Z
307564fd902902e1bea198ec7cfa1a2f
2007-09-19T22:16:13.729253Z
9
Torandi

stuff.h
file
63



2007-12-31T17:50:28.000000Z
4a5b84e7842972e8dfa4b8359c43ab62
2007-09-01T16:38:12.025871Z
2
botsnlinux

viewfrustum.cpp
file
63



2007-12-31T17:50:28.000000Z
5c8f41e128df678702174a5d713d7ff8
2007-09-01T16:38:12.025871Z
2
botsnlinux

intro.h
file
63



2007-12-31T17:50:28.000000Z
33a801fd9bf3a7352524800d3a2043cf
2007-09-01T16:38:12.025871Z
2
botsnlinux

Makefile.in
file
63



2007-12-31T17:50:28.000000Z
0f90cfa8ca546e931c73437ac7f9801a
2007-09-22T05:44:23.419072Z
27
botsnlinux

gl_util.h
file
63



2007-12-31T17:50:28.000000Z
fee95abe084c68e7bb8b2d626663d4d5
2007-09-03T03:13:45.611773Z
5
botsnlinux

course_load.h
file
63



2007-12-31T17:50:28.000000Z
b1198a305bcb3980859aa8d2a0f27312
2007-09-01T16:38:12.025871Z
2
botsnlinux

configuration.cpp
file
63



2007-12-31T17:50:28.000000Z
1c4f2f94542a9b0f3bfbb37f93cd2de5
2007-12-30T18:57:52.311507Z
54
cpicon92

string_util.h
file
63



2007-12-31T17:50:28.000000Z
f239fb41549dc1e24a345a07e67b2483
2007-09-01T16:38:12.025871Z
2
botsnlinux

part_sys.h
file
63



2007-12-31T17:50:28.000000Z
fc7207198f4773ab130d00f296cee953
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_quad.cpp
file
63



2007-12-31T17:50:28.000000Z
03447e95d02d2815cd65fdeca4682aca
2007-09-01T16:38:12.025871Z
2
botsnlinux

tex_font_metrics.cpp
file
63



2007-12-31T17:50:28.000000Z
2d65c6776bd33e42ec62a1d2fd934566
2007-09-03T03:13:45.611773Z
5
botsnlinux

os_util.cpp
file
63



2007-12-31T17:50:28.000000Z
475c4e604e7aa4493e9b59be59cfe0f8
2007-09-20T15:11:12.728578Z
10
Torandi

configmode.h
file
63



2007-12-31T17:50:28.000000Z
d4b620e1d0787d4a20dad03e61d4824b
2007-09-01T16:38:12.025871Z
2
botsnlinux

tcl_util.cpp
file
63



2007-12-31T17:50:28.000000Z
ee8d766ed0be2732d512c8944794bdce
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_mgr.h
file
63



2007-12-31T17:50:28.000000Z
def5aff8f797044ccdcf685c91f5ad0b
2007-09-01T16:38:12.025871Z
2
botsnlinux

tux_shadow.cpp
file
63



2007-12-31T17:50:28.000000Z
d3a0dad7304d623bff85d8463756d844
2007-09-19T22:16:13.729253Z
9
Torandi

graphicsconfig.cpp
file
63



2007-12-31T17:50:28.000000Z
be25bf620b9d92170fa34ad038139ff1
2007-09-19T22:16:13.729253Z
9
Torandi

error_util.cpp
file
63



2007-12-31T17:50:28.000000Z
fb4b43b44942c28875f69ee7ae2faeee
2007-09-01T16:38:12.025871Z
2
botsnlinux

stuff.cpp
file
63



2007-12-31T17:50:28.000000Z
94c16378eef80127d73df927a80a68f5
2007-09-01T16:38:12.025871Z
2
botsnlinux

lights.cpp
file
63



2007-12-31T17:50:28.000000Z
993cb4120939d8dc855e9744f25c82ea
2007-09-01T16:38:12.025871Z
2
botsnlinux

intro.cpp
file
63



2007-12-31T17:50:28.000000Z
8cb4fa57bf79142bfb660f54955d81d3
2007-09-19T22:16:13.729253Z
9
Torandi

winsys.cpp
file
63



2007-12-31T17:50:28.000000Z
c7ef8692896a17184505561286d5134a
2007-09-13T20:23:04.446280Z
6
Torandi

credits.cpp
file




2008-01-14T00:15:17.000000Z
1b56c63e614b97f4aa392569c2fafa12
2008-01-13T23:52:36.812478Z
65
cpicon92

gl_util.cpp
file
63



2007-12-31T17:50:28.000000Z
cc2811dd77963a388d695467db7137f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_load.cpp
file
63



2007-12-31T17:50:28.000000Z
7e9a5dcf7ee1410595d93bc284dd2cb0
2007-09-01T16:38:12.025871Z
2
botsnlinux

videoconfig.h
file
63



2007-12-31T17:50:28.000000Z
03326eb0c5340fa0e2457af673523c0d
2007-09-01T16:38:12.025871Z
2
botsnlinux

pp_classes.h
file
63



2007-12-31T17:50:28.000000Z
36e93475686ad654cedb4f52441a0482
2007-09-03T03:13:45.611773Z
5
botsnlinux

string_util.cpp
file
63



2007-12-31T17:50:28.000000Z
2e1c1574a18268ee1e7875fd0680b912
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_over.cpp
file
63



2007-12-31T17:50:28.000000Z
8ecdd01a6a9a95d33aa218882a299dd5
2007-09-19T22:16:13.729253Z
9
Torandi

course_render.h
file
63



2007-12-31T17:50:28.000000Z
6090e9308bc82c0ae0c5a556a65f4ca5
2007-09-01T16:38:12.025871Z
2
botsnlinux

track_marks.h
file
63



2007-12-31T17:50:28.000000Z
f547b35eb5bc010036b3ea75113b9526
2007-09-01T16:38:12.025871Z
2
botsnlinux

racing.h
file
63



2007-12-31T17:50:28.000000Z
0e7969c44243958b5a0fe518446c1d7b
2007-09-01T16:38:12.025871Z
2
botsnlinux

race_select.cpp
file
63



2007-12-31T17:50:28.000000Z
79931cbdb76fcb574b138c9796169f99
2007-09-21T20:25:59.273843Z
16
Torandi

render_util.cpp
file
63



2007-12-31T17:50:28.000000Z
e1ca351de207f7d58608c6ca2896562c
2007-09-01T16:38:12.025871Z
2
botsnlinux

loading.cpp
file
63



2007-12-31T17:50:28.000000Z
80ecb84cf633b625c7f08705b8a208f8
2007-09-01T16:38:12.025871Z
2
botsnlinux

screenshot.cpp
file
63



2007-12-31T17:50:28.000000Z
61231112ea6dce0ccbea86821730a7dc
2007-09-03T03:13:45.611773Z
5
botsnlinux

loop.h
file
63



2007-12-31T17:50:28.000000Z
6152b7f0add8a052756d31948aba972e
2007-09-13T20:23:04.446280Z
6
Torandi

course_quad.h
file
63



2007-12-31T17:50:28.000000Z
a6f8fc5e10507729b1b24bd03384b4b8
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppgltk
dir

paused.cpp
file
63



2007-12-31T17:50:28.000000Z
cd770e9d77197a941d49408cbb094513
2007-09-21T22:02:18.956889Z
18
Torandi

os_util.h
file
63



2007-12-31T17:50:28.000000Z
846b731ab5be08c9d9654e6420b3e429
2007-09-01T16:38:12.025871Z
2
botsnlinux

fps.cpp
file
63



2007-12-31T17:50:28.000000Z
23eee62734e3eeed22cc7c77c684e703
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_util.h
file
63



2007-12-31T17:50:28.000000Z
fb4cbfed3cfc4ef1f9c4e12ef7b8b020
2007-09-01T16:38:12.025871Z
2
botsnlinux

splash_screen.cpp
file
63



2007-12-31T17:50:28.000000Z
8343f44222b6b4563c42e555b097f016
2007-09-21T17:46:49.180251Z
13
Torandi

etracer.h
file
63



2008-01-12T07:49:53.000000Z
5206e5c64007c26bbb0065d6b8739660
2007-12-31T18:15:05.063809Z
60
cpicon92

graphicsconfig.h
file
63



2007-12-31T17:50:28.000000Z
8a18cae432b4e8ea43ee74543cac42a2
2007-09-19T22:16:13.729253Z
9
Torandi

error_util.h
file
63



2007-12-31T17:50:28.000000Z
98f6b098c47e646aaf023dfdaf990904
2007-09-01T16:38:12.025871Z
2
botsnlinux

player.cpp
file
63



2007-12-31T17:50:28.000000Z
9de07ba71b8f5553c9f38b49499f1c6f
2007-09-19T22:16:13.729253Z
9
Torandi

course_render.cpp
file
63



2007-12-31T17:50:28.000000Z
dd811dc9c038c1af5e2b2a8be94c45dc
2007-09-01T16:38:12.025871Z
2
botsnlinux

lights.h
file
63



2007-12-31T17:50:28.000000Z
0d43950b5370c384968784523e3ef997
2007-09-01T16:38:12.025871Z
2
botsnlinux

racing.cpp
file
63



2007-12-31T17:50:28.000000Z
45671d25cf49c97e5965c58cd414ce98
2007-09-21T17:46:49.180251Z
13
Torandi

view.cpp
file
63



2007-12-31T17:50:28.000000Z
e4f9a2ec206702666303ad6d800065d3
2007-09-19T22:16:13.729253Z
9
Torandi

model_hndl.h
file
63



2007-12-31T17:50:28.000000Z
71baeff826fa9a4f8cf020a3d6fcdbea
2007-09-19T22:16:13.729253Z
9
Torandi

game_type_select.h
file
63



2007-12-31T17:50:28.000000Z
d26495c1531579f8d625eeeca106bd0a
2007-09-13T20:23:04.446280Z
6
Torandi

hud.cpp
file
63



2007-12-31T17:50:28.000000Z
b5063297a8d7001fbc83bac8d163e0a8
2007-09-23T10:12:20.263423Z
32
hamishmorrison

highscore.h
file
63



2007-12-31T17:50:28.000000Z
737c7588af24647cfbb5c98fb94bb974
2007-09-13T20:23:04.446280Z
6
Torandi

file_util.h
file
63



2007-12-31T17:50:28.000000Z
5300c7ed17ecafc56f0933509b8554ca
2007-09-01T16:38:12.025871Z
2
botsnlinux

callbacks.h
file
63



2007-12-31T17:50:28.000000Z
d97c640d06bc74c92389994609760f29
2007-09-03T03:13:45.611773Z
5
botsnlinux

main.cpp
file
63



2007-12-31T17:50:28.000000Z
c741ab7f63090d5168a4be3effc992c3
2007-12-09T00:04:38.165111Z
42
cpicon92

joystickconfig.cpp
file
63



2007-12-31T17:50:28.000000Z
36f1d3be9de356f3db7c1a48faccd44e
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_select.h
file
63



2007-12-31T17:50:28.000000Z
70fe51d92b46095e501d5a7e9902dca5
2007-09-21T18:50:23.813603Z
14
Torandi

keyboardconfig.cpp
file
63



2007-12-31T17:50:28.000000Z
185908721a14b746d68f31a12a5b485c
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier.h
file
63



2007-12-31T17:50:28.000000Z
32e1c7044b7ee0009d90ad3fc4a5edb1
2007-09-01T16:38:12.025871Z
2
botsnlinux

part_sys.cpp
file
63



2007-12-31T17:50:28.000000Z
859de82d09412340071e81991157219d
2007-09-01T16:38:12.025871Z
2
botsnlinux

highscore.cpp
file
63



2007-12-31T17:50:28.000000Z
b606085c735d0ad5b14afee57e89ba7f
2007-09-20T19:47:02.958075Z
11
Torandi

callbacks.cpp
file
63



2007-12-31T17:50:28.000000Z
295b3c72e420475b00fd5a3d8c754973
2007-09-01T16:38:12.025871Z
2
botsnlinux

view.h
file
63



2007-12-31T17:50:28.000000Z
2746a69c4b93e12192b856b0bc00d059
2007-09-01T16:38:12.025871Z
2
botsnlinux

fog.h
file
63



2007-12-31T17:50:28.000000Z
884794cdc6ebee86cbde9f6393abf0c5
2007-09-03T03:13:45.611773Z
5
botsnlinux

debug.cpp
file
63



2007-12-31T17:50:28.000000Z
743a1db3dbb77c426b244305da6292c0
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier.cpp
file
63



2007-12-31T17:50:28.000000Z
2ef3f59253f29fd7ac48c03eea4cc939
2007-09-01T16:38:12.025871Z
2
botsnlinux

tux_shadow.h
file
63



2007-12-31T17:50:28.000000Z
c3fff18b9e2538c33ba368cbfdd996b2
2007-09-01T16:38:12.025871Z
2
botsnlinux

keyboardconfig.h
file
63



2007-12-31T17:50:28.000000Z
6ed80a3310d7ec6f11955fe0dd5e470c
2007-09-01T16:38:12.025871Z
2
botsnlinux

videoconfig.cpp
file
63



2007-12-31T17:50:28.000000Z
f97a275ed78da696565a125354978384
2007-09-20T15:11:12.728578Z
10
Torandi

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        Extreme Tux Racer - Version 0.4

go to extremetuxracer.com for more info                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      <modify-entry
   name="README"
   present-props=""
   cachable-props="svn:special svn:externals svn:needs-lock"
   has-prop-mods="false"
   has-props="false"/>
<rm
   name=".svn/props/README.svn-work"/>
<rm
   name=".svn/prop-base/README.svn-base"/>
<modify-entry
   committed-rev="65"
   committed-date="2008-01-13T23:52:36.812478Z"
   name="README"
   last-author="cpicon92"
   uuid="14c2f07d-8935-0410-8134-90508e68d282"/>
<modify-entry
   name="README"
   prop-time="working"/>
<modify-wcprop
   name="README"
   propname="svn:wc:ra_dav:version-url"
   propval="/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/README"/>
<modify-entry
   name="README"
   url="https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/README"
   kind="file"
   deleted="false"
   absent="false"
   revision="65"/>
<cp-and-translate
   dest="README"
   name=".svn/tmp/text-base/README.svn-base"/>
<mv
   dest=".svn/text-base/README.svn-base"
   name=".svn/tmp/text-base/README.svn-base"/>
<readonly
   name=".svn/text-base/README.svn-base"/>
<modify-entry
   name="README"
   checksum="db80ee1ce2cbf1682f01ce939d03f794"/>
<modify-entry
   name="README"
   text-time="working"/>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-12T07:44:01.776333Z
63
cpicon92


svn:special svn:externals svn:needs-lock










incomplete
14c2f07d-8935-0410-8134-90508e68d282

tux_snowboard.tcl
file
63



2007-12-31T17:50:58.000000Z
943b5b085e6728caefc7cd9870b5828b
2007-09-19T22:16:13.729253Z
9
Torandi

tux_walk.tcl
file
63



2007-12-31T17:50:58.000000Z
c0f2b685b74b08d9a2b346bb70a5d5f0
2007-09-01T16:38:12.025871Z
2
botsnlinux

music
dir

translations
dir

Makefile.in
file
63



2008-01-12T07:50:50.000000Z
b357974e068f3d4b96393c2c7900a9ad
2008-01-12T07:44:01.776333Z
63
cpicon92

courses
dir

textures
dir

etracer_init.tcl
file
63



2008-01-12T07:50:50.000000Z
0f47893cc3774d406360d3a9e8bbb1e2
2008-01-12T07:44:01.776333Z
63
cpicon92

fonts
dir

tux.tcl
file
63



2007-12-31T17:50:58.000000Z
bcf7b53f0ebe9a16e0944795b0f1fa36
2007-09-01T16:38:12.025871Z
2
botsnlinux

models.tcl
file
63



2007-12-31T17:50:58.000000Z
82dee86eb0fa46b8d2cddbbd17622072
2007-09-19T22:16:13.729253Z
9
Torandi

Makefile.am
file
63



2008-01-12T07:50:50.000000Z
10611bf214f448a27cb87b1f01dfbfe2
2008-01-12T07:44:01.776333Z
63
cpicon92

terrains.png
file
63



2007-12-31T17:50:58.000000Z
ff7f08f3015d00dd207b38dc50d0afd4
2007-12-09T00:04:38.165111Z
42
cpicon92
has-props

sounds
dir

objects.png
file
63



2007-12-31T17:50:58.000000Z
a4dc74c88186b7333556925bb525d0f4
2007-12-09T00:04:38.165111Z
42
cpicon92
has-props

Makefile
file
63



2007-12-31T17:50:58.000000Z
36551f7e1e3447b9cbd243fa37dec8aa
2008-01-12T07:44:01.776333Z
63
cpicon92

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        bject   CreateStdAccessibleObject   U‹ìƒÄô„ÒtƒÄğèúpûÿˆUû‰Eü3Ò‹Eüèªûıÿ²¸¤˜A èjıÿ‰Eô3ÀUhÃ¼D dÿ0d‰ h÷  j èí£ûÿ‹Ğ‹EôèÏıÿ‹Eô‹ÿR …ÀyƒÀÁø‹Uü‰‚H  ‹Eô‹ÿR¹   ™÷ù‹Uü‰‚D  3ÀZYYd‰hÊ¼D ‹Eôè–nûÿÃéœsûÿëğ²¸\¤@ èZnûÿ‹Uü‰‚h  ‹EüÇ€T     ‹EüÇ€X     ‹EüÆ€e  ²‹Eüè+şıÿ‹EüÇ€x  ÿÿÿÿ‹EüÇ€<  ÿÿÿÿ€}û t
d    ƒÄ‹Eü‹å]Ã‹ÀU‹ìQ‰Eü‹Eüÿ€|  3ÀUh}½D dÿ0d‰ ‹Eüè şÿ3ÀZYYd‰h„½D ‹Eüÿˆ|  ÃéârûÿëïY]ÃSV‹Ù‹ğ„Òt‹†p  …ÀtPÿˆI 3À‰†p  „Ût,èXóÿÿ„Àthà½D ‹Æè´ÃüÿPÿ„I ‰†p  ë3À‰†p  ^[Ã  B u t t o n     S‹Ø‹Ãèî¦üÿ±²‹Ãèƒÿÿÿ[ÃSVW‹Ú‹ø‹‡8  …ÀtèF)  3À‰‡8  ‹‡h  …Àt,‹pNƒş |‹Ö‹‡h  èõûÿèmûÿNƒşÿuè‹‡h  èülûÿ3É²‹Çè!ÿÿÿ3Ò‹ÇèTúıÿ„Ût‹Çèñnûÿ‹Ç_^[Ã@ U‹ìQSVW‰Mü‹ú‹ğ‹]€} u2‹†ü   ‹ÿR‹Ğ3ÉŠË‹Æèı   „Àt¹¿D ²¸Xc@ èÔÌûÿèOrûÿ‹EüPSŠEPŠEPŠEPŠEP‹EP‹Ï²‹Æè  _^[Y]Â    ÿÿÿÿ+   Radio item cannot have disabled child items U‹ìQŠMQj jj j‹MQ3É‡Êè·  ]Â @ U‹ìSVW‹ù‹ò‹M‹U„Éu3ÒWŠ]SRQj j‹UR²‹Îè‚  _^[]Â @ S‹Ø‹Ãèj  €{8 t€x t€x u3À[Ã°[ÃSVWU‹ù‹ê‹ğ³MO…í|‹Õ‹Æè5  3ÒŠP	;úu€xt:ëM…í}á…í|-‹Õ‹ÆèN  ‹è…í|‹Õ‹Æè  €xt‹Õ‹Æè0  ‹è…í}â3Û‹Ã]_^[ÃSVW‹ú‹Ø€»l   t]‹ÃèµÀüÿ„ÀtRf‹W‹Ãè  ‹ğ…ö|A‹Ã‹ÿRx;³<  u	€»@   tŠ“@  €ò‹ÃèL  ‹Ö‹ÃèKùıÿ‹Ö‹Ãè†  ÇG   _^[Ã@ SVW‹ú‹ğ€¾l   „¼   €¾L   „¯   jèŒûÿf…ÀŒŸ   j	è|ûÿf…À}3Ûjènûÿf…ÀÀë@j&è_ûÿf…À|j%èSûÿf…À}³3Àë"j(èAûÿf…À|j'è5ûÿf…À}³°ë3Û°„Àt€óS±ƒÊÿ‹ÆèÚ  ‹Ğ‹Æè‰øıÿë#€óS‹†ü   ‹ÿR‹Ğ3É‹Æèµ  ‹Ğ‹Æèdøıÿ‹×‹ÆèÛµüÿ_^[Ã@ SV‹ò‹Ø€»@   tjè¾ûÿf…À}3Òë²‹Ãè$  ‹Ö‹Ãè¯µüÿ^[ÃÆ€L  èğµüÿÃ@ S‹Ø‹Ãèª¶üÿ‹SD‹ƒ  è„íüÿ[Ã‹ÀU‹ìSVW‹}‹u‹]‹Æ3Ã¨u‹Î‹Ó‹ÇèÊêüÿS‹Î‹Çè†êüÿ_^[]Â @ U‹ìQSV‰Mü‹ò‹Ø‹E‹(I ‹‹ˆ  ‰ŠÄ   ‹‹Œ  ‰ŠÈ   ;u‹Æ‹“„  è	rûÿƒ„  è®qûÿëfƒ»Š   tP‹Mü‹Ö‹ƒŒ  ÿ“ˆ  ^[Y]Â U‹ìSVW‹ø‹E‹@ü€¸l   tT‹E‹Pø‹E‹@üè`  €x t=‹èÇrûÿ‹ğ»   ë%‹€|ÿ&u;ó~‹€<&t‹Ç¹   ‹ÓèßtûÿNC…ö~;ó}Ó_^[]ÃU‹ìƒÄÄSVW3É‰MÌ‹ò‰Eü3ÀUh+ÅD dÿ0d‰ ‹^‹C$‰Eğ‹C(‰Eô±Uğ‹Eüèøıÿ‰Eøƒ}ø Œã  EàPMÌ‹Uø‹EüèP  ‹EÌè(rûÿPMÌ‹Uø‹Eüè9  ‹EÌèÕsûÿP‹Eü‹€  èºìüÿPèü˜ûÿEèPMÌ‹Eü‹€ü   ‹Uø‹8ÿW‹EÌèÜqûÿPMÌ‹Eü‹€ü   ‹Uø‹8ÿW‹EÌèƒsûÿP‹Eü‹€  èhìüÿPèª˜ûÿMĞ‹Uø‹Eüèhøıÿ‹}Ğ‹Uø‹Eüè  €x t,‹Uø‹Eüè  %ÿ   @‹Uü‹’H  ‹Mü‹‰X  ÉÑ÷êEĞ‹Eü‹€X  ÀEà‹UØ+Ğ‹EĞEè;ĞÜ   MÄUĞ‹Eüèá‚üÿ‹EÄ‰C‹EÈ‰C‹Eü‹€  ºDÅD è¢êüÿ‹UÜ+UÔ+ĞÑúyƒÒ JSÿK‹Eü€¸l   tÿKëÿCVWuĞ{¹   ó¥_^‰{MÌ‹Eü‹€ü   ‹Uø‹ÿS‹UÌ‹Eü„  èˆoûÿU‹Eü„  è¢ıÿÿY‹Eü‹(I ‹ŠÄ   ‰ˆˆ  ‹ŠÈ   ‰ˆŒ  ¡(I ‹Uü‰È   Ç€Ä   ÂD 3À‰F3ÀZYYd‰h2ÅD EÌèÖnûÿÃé4kûÿëğ_^[‹å]Ã   ÿÿÿÿ   Wg  €¸l   tƒz	t3Àë°ƒà‰BÃSVWU‹ú‹Ø‹w‹n…í|3‹Õ‹Ãè  ¶@	‹è‹V‹Ãèq  €x tE‹ƒX  ÀƒH  ÷íF€»l   u#€=HI  tj j h)  ‹Ãè­»üÿPèc›ûÿ¨tƒfï‹×‹Ãèüıÿ]_^[ÃSVWQ‹ò‹Ø‹Ãè!ôıÿ‰$‹Ì‹Ö‹Ã‹8ÿ—€   ‹$PVh   ‹Ãè_»üÿPè›ûÿ‹$Z_^[ÃS‹Øj j j h  ‹Ãè;»üÿPèñšûÿPh—  ‹Ãè(»üÿPèŞšûÿ[ÃU‹ìƒÄàSVW3Û‰]à‰Mø‰Uü‹Ø3ÀUhªÇD dÿ0d‰ ‹»  ‹Uü‹Ãèt  ‹ğj EäP‹Ãè¹üÿ‹È3Ò3Àè*åûÿ3ÀŠF	€~ t@‹“X  Ò“H  ¯ĞUäÿEäƒ~ t‹V‹Çèëçüÿ‹“X  ÒÂ)Eìë	‹ƒX  )Eì€»l   uÿEäÇEô €»l   t€~ uMô   ‹EôPEäPMà‹Uü‹ƒü   ‹8ÿW‹Eàè9nûÿPMà‹Uü‹ƒü   ‹8ÿW‹EàèãoûÿP‹ƒ  èËèüÿPè—ûÿ‰F8‹ƒT  ;F8~‹Uø‰ë‹F8ƒÀ‹Uø‰‹Eøö t‹Eøÿ 3ÀZYYd‰h±ÇD EàèWlûÿÃéµhûÿëğ_^[‹å]ÃU‹ìSVW‹ù‹ò‹Ø€} „¼   ‹E‹@ü‹€  ‹@²èßüÿjjVèÆ˜ûÿjè¯—ûÿP‹E‹@ü‹€  è%èüÿPè/•ûÿWV‹Ãè^mûÿP‹ÃèoûÿP‹E‹@ü‹€  èüçüÿPèF–ûÿjÿjÿVèt˜ûÿjè]—ûÿP‹E‹@ü‹€  èÓçüÿPèİ”ûÿWV‹ÃèmûÿP‹ÃèÈnûÿP‹E‹@ü‹€  èªçüÿPèô•ûÿë)WV‹ÃèálûÿP‹ÃènûÿP‹E‹@ü‹€  èçüÿPèÉ•ûÿ_^[]Â ‹ÀU‹ìƒÄ„SVW3Û‰]„‹ñ}¸¹   ó¥‰Uø‰Eü3ÀUh«ÎD dÿ0d‰ ‹Eü€¸e   t‹Eü€¸t   u‹Eüè  ‹EüÆ€t  ‹Uø‹Eüèá
  ‹ø€=HI  tj j h)  ‹Eüè=¸üÿPèó—ûÿ‰EÔë3À‰EÔ‹Eü€x8 t
€ t3Àë°ˆE÷‹Eü‹°  ‹Eü€¸l   u(öEt"‹Eüèª·üÿ„Àtº  €‹Fè­ÜüÿÇEÜ  €ë)‹Eü‹PH‹Fè–Üüÿ€}÷ t	ÇEÜ  €ë‹Eü‹@D‹@‰EÜ‹Eü€¸e   „$  º  €‹Fè²Úüÿ‹Uø‹Eüè
  %ÿ   ‰Eè‹EèH…ÀŒø   @‰EÈ3Û‹Uø‹Eüèê	  ‹Óúÿ   w£PƒÉ   ‹Eü‹€H  ‹Ğ‹Mü‹‰X  ÉÑ¯ÓÑøyƒĞ Ğ‹EüX  ‰Uğ‹UÄ‹Ê‹E¼+ÈÑùyƒÑ È‰Mä‰Uì‹EèH;ØuT‹Uø‹Eüèz	  €x t‹Eä‰Eì‹Eü‹€  PhĞÁD ‹EäP‹Eü‹€H  ÑøyƒĞ Eğ‹Uü‚X  P‹EäP‹EğPèÍ‘ûÿ‹Eü‹€  PhĞÁD ‹EìP‹EğP‹E¼P‹EğPè©‘ûÿCÿMÈ…ÿÿÿ€ „Ü  ‹Uü‹’H  ‹Eü‹€X  ‹ÈÉÑ)U¸E¸‰E¨‹UÄ‹E¼+Ğ‹Mü+‘D  ÑúyƒÒ Ğ‰U¬‹Eü‹€D  Â‰E´‹Eü‹€H  E¨‰E°‹Eüƒ¸p   …•   ŠG,ru"3ÀŠG‹…|	I Ë   ë3ÀŠG‹…|	I ë»  ‹Eü€¸M   tË @  €}÷ tË   ‹Eü‹€<  ;Eøu ‹Eü€¸@   u‹Eü‹€P  ;EøuË   SjE¨P‹Æè0äüÿPèZ’#Translated by Andreas Tarandi <torandi@gmail.com>
#
#Changelog:
#2007-09-13 - Initial Translation
#2008-01-12 - Corrected two typos

#common strings
pp_translate_string "Back" "Tillbaka"
pp_translate_string "PRESS ANY KEY TO START" "TRYCK PÃ… VALFRI KNAPP FÃ–R ATT BÃ–RJA"

#game_type_select
pp_translate_string "Enter an event" "Turnering"
pp_translate_string "Practice" "TrÃ¤ning"
pp_translate_string "Configuration" "InstÃ¤llningar"
pp_translate_string "Credits" "Tack till"
pp_translate_string "Quit" "Avsluta"

#configuration
pp_translate_string "Graphics" "Grafik"
pp_translate_string "Video" "Video"
pp_translate_string "Audio" "Ljud"
pp_translate_string "Keyboard" "Tangentbord"
pp_translate_string "Joystick" "Joystick"

#config mode
pp_translate_string "Cancel" "Avbryt"
pp_translate_string "Ok" "OK"

#graphics configuration
pp_translate_string "Graphics Configuration" "GrafikinstÃ¤llningar"
pp_translate_string "Language:" "SprÃ¥k:"
pp_translate_string "Show UI Snow:" "Visa UI snÃ¶:"
pp_translate_string "Display FPS:" "Visa FPS:"
pp_translate_string "Display Progress Bar:" "Visa Progress Bar:"
pp_translate_string "Draw Fog:" "Rita dimma:"
pp_translate_string "Reflections:" "Reflektioner:"
pp_translate_string "Shadows:" "Skuggor:"
pp_translate_string "Model:" "Modell:"

#video configuration
pp_translate_string "Video Configuration" "VideoinstÃ¤llningar"
pp_translate_string "Resolution:" "UpplÃ¶sning:"
pp_translate_string "Bits Per Pixel:" "Bitar per pixel:"
pp_translate_string "Fullscreen:" "FullskÃ¤rm:"
pp_translate_string "Experimental (needs restart)" "Experimentel (krÃ¤ver omstart)"
pp_translate_string "Enable FSAA:" "Aktivera FSAA:"
pp_translate_string "To change the resolution, or switch into fullscreen mode" "FÃ¶r att byta upplÃ¶sning, eller fÃ¶r att byta"
pp_translate_string "use options.txt, located in the config folder." "till fullskÃ¤rm, anvÃ¤nd options.txt i konfigureringsmappen."

#audio configuration
pp_translate_string "Audio Configuration" "LjudinstÃ¤llningar"
pp_translate_string "Sound Effects:" "Ljudeffekter:"
pp_translate_string "Music:" "Musik:"
pp_translate_string "(needs restart)" "(krÃ¤ver omstart)"
pp_translate_string "Disable Audio:" "StÃ¤ng av ljud:"
pp_translate_string "Stereo:" "Stereo:"
pp_translate_string "Bits Per Sample:" "Bits Per Sample:"
pp_translate_string "Samples Per Second:" "Samples Per Second:"

#keyboard configuration
pp_translate_string "Keyboard Configuration" "TangentbordsinstÃ¤llningar"
pp_translate_string "Turn left:" "SvÃ¤ng vÃ¤nster:"
pp_translate_string "Turn right:" "SvÃ¤ng hÃ¶ger:"
pp_translate_string "Paddle:" "Skjut pÃ¥:"
pp_translate_string "Brake:" "Bromsa:"
pp_translate_string "Jump:" "Hoppa:"
pp_translate_string "Trick:" "Trick:"
pp_translate_string "Reset:" "Ã…terstÃ¤ll:"

#joystick configuration
pp_translate_string "Joystick Configuration" "JoystickinstÃ¤llningar"
pp_translate_string "Enable Joystick" "Aktivera Joystick:"

#race select
pp_translate_string "Race!" "KÃ¶r!"
pp_translate_string "Select a race" "VÃ¤lj ett lopp"
pp_translate_string "Contributed by:" "Skapad av:"
pp_translate_string "Unknown" "OkÃ¤nd"
pp_translate_string "Time:" "Tid:"
pp_translate_string "Herring:" "StrÃ¶mmingar:"
pp_translate_string "Score:" "PoÃ¤ng:"

#event select
pp_translate_string "Continue" "FortsÃ¤tt"
pp_translate_string "Select event and cup" "VÃ¤lj turnering och tÃ¤vling"
pp_translate_string "Event:" "Turnering:"
pp_translate_string "Cup:" "Cup:"
pp_translate_string "You've won this cup!" "Du har vunnit den hÃ¤r tÃ¤vlingen!"
pp_translate_string "You must complete this cup next" "Du mÃ¥ste vinna den hÃ¤r tÃ¤vlingen hÃ¤rnÃ¤st"
pp_translate_string "You cannot enter this cup yet" "Du kan inte delta i den hÃ¤r tÃ¤vlingen Ã¤nnu"

#event race select
pp_translate_string "You don't have any lives left." "Du har inga liv kvar."
pp_translate_string "Race won! Your result:" "Du vann loppet! Ditt resultat:"
pp_translate_string "Needed to advance:" "Krav fÃ¶r att avancera:"
pp_translate_string "You can't enter this race yet." "Du kan inte kÃ¶ra det hÃ¤r loppet Ã¤nnu."

#loading
pp_translate_string "Loading, Please Wait..." "Laddar, var vÃ¤nlig vÃ¤nta..."

#paused
pp_translate_string "Resume" "FortsÃ¤tt"
pp_translate_string "Paused" "Pausad"

#race over
pp_translate_string "Race Over" "Loppet slut"
pp_translate_string "Time: %02d:%02d.%02d" "Tid: %02d:%02d.%02d"
pp_translate_string "Herring: %3d" "StrÃ¶mmingar: %3d"
pp_translate_string "Score: %6d" "PoÃ¤ng: %6d"
pp_translate_string "Max speed: %3d km/h" "Maxfart: %3d km/h"
pp_translate_string "Was flying: %.01f %% of time" "FlÃ¶g: %.01f %% av tiden"
pp_translate_string "Race aborted" "Loppet avbrutet"
pp_translate_string "You beat your best score!" "Du slog rekordet!"
pp_translate_string "Congratulations! You won the event!" "Grattulerar! Du vann turneringen!"
pp_translate_string "Congratulations! You won the cup!" "Grattulerar! Du vann tÃ¤vlingen!"
pp_translate_string "You advanced to the next race!" "Du avancerade till nÃ¤sta lopp!"
pp_translate_string "You didn't advance." "Du avancerade inte."

#highscore
pp_translate_string "Highscore" "Highscore"
pp_translate_string "You made it to the %s place in the highscore!" "Du slog dig in pÃ¥ %s platsen pÃ¥ highscoren"
pp_translate_string "Player name:" "Namn:"
pp_translate_string "1:st" "1:a"
pp_translate_string "2:nd" "2:a"
pp_translate_string "3:rd" "3:e"
pp_translate_string "4:th" "4:e"
pp_translate_string "5:th" "5:e"
pp_translate_string "6:th" "6:e"
pp_translate_string "7:th" "7:e"
pp_translate_string "8:th" "8:e"
pp_translate_string "9:th" "9:e"
pp_translate_string "10:th" "10:e"
pp_translate_string "No records" "Inga rekord"

#HUD
#Strings are prefixed to prevent collisions with the UI
pp_translate_string "H|FPS: %.1f" "FPS: %.1f"
pp_translate_string "H|Time:" "Tid:"
pp_translate_string "H|km/h" "km/h"
#herring counter
pp_translate_string "H|%03d" "%03d"
#time counter
pp_translate_string "H|%02d:%02d:%02d" "%02d:%02d:%02d"
#speed counter
pp_translate_string "H|%.0f" "%.0f"

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          €è…Eûÿ3ÀZYYd‰h$ìD EüèäGûÿÃéBDûÿëğ‹Ã_^[Y]Â ‹ÀU‹ì¸   ]Â U‹ì¸   ]Â ÄlÿÿÿÆHI  ƒ=Ü I uÇ$”   TèImûÿ…Àtƒ|$HI Ä”   Ã‹Àƒ=`I  tÿ`I j èfŞıÿè±Şıÿ¢dI Ã@ €=dI  tèRŞıÿÃ¸Ä	I èj]ûÿ€=4I  u¡ I £`I Ç I „ìD èdÿÿÿèó¼ÿÿhíD híD èTlûÿPèflûÿ£LI ÃNotifyWinEvent  user32.dll          èíD         ĞíD ØíD   ¨0A ÜjA à*@ +@ \YA H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA tîD ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA ĞîD èdA ïD „eA ,eA ìlA ¸~A A   ÜïD TNewProgressBarTNewProgressBarTíD Ø2A  NewProgressBar h@ ü  ÿLïD       €   €	 Minh@   ÿ`ïD       €   €
 Maxh@  ÿtïD       €     PositionSV„ÒtƒÄğè®>ûÿ‹Ú‹ğ3Ò‹ÆèUjüÿº–   ‹ÆèWüÿjèqûÿ‹Ğ‹Æè)Wüÿ3À‰†ü   Ç†   d   „Ût
d    ƒÄ‹Æ^[ÃSV‹ò‹ØèEªûÿ‹Ö‹Ãè¼süÿ¹üîD ‹Ö‹ÃèrsüÿƒN^[Ã   msctls_progress32   S‹Ø‹Ãètüÿfºÿÿ3ÀèÃsûÿPj h  ‹ÃèD’üÿPèúqûÿ‹“  ‹Ãè-   [Ã@ ‰ü   ‹  è   Ã‹À‰   ‹  è   Ã‹ÀSVW‹ò‹Ø‹ƒü   ;ğ}‹ğë‹ƒ   ;ğ~‹ğ‰³  ‹Ãèn“üÿ„Àt1j ‹ƒ   ‹»ü   +ÇPhÿÿ  +÷Vè˜jûÿPh  ‹Ãè§‘üÿPè]qûÿ_^[ÃV‹ğ‹Æ‹ÿQğ^Ã        ¼ğD         ˜ğD ¬ğD (  °]B Ü”B à*@ +@ àñD H¸@ |SA d·@ l›B xDA ÈZA ¬HA ğA èA ¨ñD ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA òD ›B ¤òD „eA ¸“B ìlA ¸~A A  °$°N¼ˆôD œôD °ôD TRichEditViewerTRichEditViewerğD 4^B 4 RichEditViewer  @ $ ÿLóD       €   3 UseRichEdit‹Àƒ=Ğ	I  u;ÇlI    h\ñD èçhûÿ£hI ƒ=hI  uÇlI    hlñD èÅhûÿ£hI ÿĞ	I Ã RICHED20.DLL    RICHED32.DLL    ƒ=Ğ	I  ~!ÿĞ	I ƒ=Ğ	I  u¡hI Pèõfûÿ3À£hI ÃSV„ÒtƒÄğèz;ûÿ‹Ú‹ğ3Ò‹ÆèU¨ıÿÆ†$  „Ût
d    ƒÄ‹Æ^[Ã@ SV‹Ú‹ğ3Ò‹Æè©¨ıÿ€¾%   tÆ†%   èxÿÿÿ„Ût‹Æè];ûÿ‹Æ^[ÃSV‹ò‹Ø€»$   t€»%   uÆƒ%  èØşÿÿ‹Ö‹Ãèƒ¨ıÿ€»$   t*ƒ=lI u¹„òD ‹Ö‹Ãèpüÿë¹òD ‹Ö‹Ãèóoüÿ^[ÃºœòD FLèÿ€ûÿ^[ÃRichEdit20A RICHEDIT    /Text   SV‹Ø‹ÃèY ıÿ‹Ãè^   €»$   tRƒ=lI uIj j h;  ‹Ãè üÿPèVnûÿ‹ğÎ   Vj hE  ‹ÃèƒüÿPè9nûÿj jh[  ‹ÃèmüÿPè#nûÿ^[ÃS‹Ø€»$   t(‹Ãèéüÿ„Àt‹CHè½¬üÿPj hC  ‹Ãè6üÿPèìmûÿ[Ã‹ÀSV‹Ú‹ğ:$  t&ˆ$  ‹Æè%süÿ„Ûu€¾%   tÆ†%   èüıÿÿ^[ÃU‹ìQSV‹]3À‰Eü‹u‹F;Ø~‹Ø‹U‹‹Ëè3ûÿ)^‹E‰‹Eü^[Y]Â ‹ÀU‹ìƒÄìS‹Ø‹E‹@üèdCûÿ‰Eø‹E‹@üè’Aûÿ‰EüEø‰Eì3À‰Eğ¸„óD ‰EôEìPShI  ‹E‹@øèküÿPè!mûÿ‹Eğ[‹å]ÃU‹ìƒÄø‰Uü‰Eø‹Eø€¸$   u‹Uü‹EøèqVüÿ3Àë6hşÿÿj h5  ‹Eøè!üÿPè×lûÿU¸   èXÿÿÿY…ÀtU¸   èHÿÿÿYYY]Ã@ è—ÿÿÿÃ‹ÀS‹Ø‹Ãè®ƒüÿ‹Ãè{şÿÿ[ÃS‹Ø‹Ãè¦„üÿ‹Ãègşÿÿ[ÃU‹ìƒÄèSVW3É‰Mô‹Ø3ÀUh¢õD dÿ0d‰ ‹B‹Hé  …¬   x  …Ÿ   ‹P‰Uø‹P‰Uü‹Eø…Àuƒ}üÿuj j j‹ÃèeŒüÿPèlûÿë‹Uü+ĞB‹ÂUô’èxCûÿ‹Eø‰Eè‹Eü‰Eì‹EôèøAûÿ‰EğEèPj hK  ‹Ãè#ŒüÿPèÙkûÿ‹ĞEôè?Cûÿƒ}ô t!jj j ‹Eôè¿AûÿPh°õD ‹Ãèò‹üÿPèäÕıÿ3ÀZYYd‰h©õD Eôè_>ûÿÃé½:ûÿëğ_^[‹å]Ãopen                             öD    Ä@ ,,@ à*@ +@ L+@ öD öD öD $öD ,öD TCustomFileé+2ûÿ@ é#2ûÿ@ é2ûÿ@ é2ûÿ@ é2ûÿ@                         €öD    ìõD ,,@ à*@ +@ ,ùD ¨ùD àùD úD PúD àúD \ùD TFile‹À                        ÔöD   höD ,,@ à*@ +@ ,ùD ¨ùD àùD úD PúD àúD \ùD TTextFileReader                        0÷D    höD ,,@ à*@ +@ ,ùD ¨ùD àùD úD PúD àúD \üD TTextFileWriter                        t÷D    Xc@ ,,@ à*@ +@ L+@ 
EFileErrorSƒÄø‹Ø‹Ô‹Ã‹ÿQƒ|$ uöD$€u‹$ë¸ÿÿÿYZ[ÃU‹ìƒÄôSVW3É‰Mü‹Ú3ÀUh!øD dÿ0d‰ Uü‹Ãè|îıÿƒ}ü uEüP‰]ôÆEø Uô3É¸8øD è€ûÿ‹Mü²¸t÷D è™“ûÿ‰Xè9ûÿ3ÀZYYd‰h(øD Eüèà;ûÿÃé>8ûÿëğ_^[‹å]Ã ÿÿÿÿ   File I/O error %d   S‹ØèĞ`ûÿ‹Ğ‹ÃèOÿÿÿ[ÃSVW‹ñ‹ú‹Ø‹×‹Î‹Ã‹8ÿW;ğtº&   ‹è(ÿÿÿ_^[ÃƒÄø3É‰L$‰$‹Ô‹ÿQYZÃ‹ÀSV‹ò‹Ø‹ÆèÇ<ûÿP‹Æèƒ>ûÿ‹Ğ‹ÃY‹ÿS^[Ã‹ÀU‹ìQSV„ÒtƒÄğèZ4ûÿ‹ñˆUÿ‹Ø3Ò‹ÃèJ2ûÿŠEPŠEPŠM‹Ö‹Ã‹0ÿV‰C‹C…Àt@u‹è>ÿÿÿÆC€}ÿ t
d    ƒÄ‹Ã^[Y]Â SV‹Ú‹ğ€~ t	‹FPè£^ûÿ3Ò‹Æè2ûÿ„Ût‹Æè4ûÿ‹Æ^[Ã‹ÀU‹ìSV‹Ù‹òj h€   3ÀŠÃ‹…ğ	I Pj 3ÀŠE‹…à	I P3ÀŠE‹…Ô	I P‹Æè=ûÿPès^ûÿ^[]Â SV‹Ú‹ğ3À‰CjCPj ‹FPèø`ûÿ‰ƒ;ÿuèT_ûÿ…Àt‹èqşÿÿ^[Ã‹ÀSV‹Ú‹ğCP‹FPè!_ûÿ‰ƒ;ÿuè%_ûÿ…Àt‹èBşÿÿ^[Ã@ SVWQ‹ù‹ò‹Øj D$PWV‹CPèX`ûÿ…Àu€{ u
èé^ûÿƒømt‹èşÿÿ‹$Z_^[ÃSVWƒÄø‹ò<$¥¥‹Øj D$P‹D$P‹CPèH`ûÿ@uè¨^ûÿ…Àt‹èÅıÿÿYZ_^[Ã@ SQ‹Ø3À‰$jD$Pj ‹CPè`ûÿ@uèq^ûÿ…Àt‹èıÿÿZ[Ã@ S‹Ø‹CPèÔ_ûÿ…Àu‹èqıÿÿ[Ã@ SVWQ‹ñ‹ú‹Øj D$PVW‹CPè(`ûÿ…Àu‹èEıÿÿ;4$tº   ‹è”üÿÿZ_^[Ã@ SV‹Ø‹C;Cr'€{ u!S¹   ‹Ã‹0ÿV‰C3À‰Cƒ{ uÆC^[Ã‹ÀS‹Ø‹Ãè¾ÿÿÿŠC[ÃU‹ìƒÄøSVW3É‰Mø‰Uü‹Ø3ÀUhKüD dÿ0d‰ ‹Ãèÿÿÿ€{ tƒ}ø …Š   º&   ‹è üÿÿë|‹sëŠD3,
t
,tF;srî‹Eøè§9ûÿ‹ø‹Ö+S×EøèÆ<ûÿEøè^;ûÿ8‹Î+K‹CDè>+ûÿ‹Æ‰C;Cs…ÿC‹C€|u‹Ãèÿÿÿ‹C;Cs
€|
uÿC‹Eü‹Uøè_8ûÿ3ÀZYYd‰hRüD Eøè¶7ûÿÃé4ûÿëğ_^[YY]Ã@ U‹ìQˆMÿ‹M€ùu±QŠMQŠMÿèâüÿÿY]Â SVWƒÄô‹ò‹Ø€{ uu‹Ô‹Ã‹ÿQƒ<$ uƒ|$ t[‹Äº   è¦şÿ‹Ô‹Ã‹ÿQT$¹   ‹Ãè•ûÿÿŠD$,
t.,tëÆD$
T$¹   ‹Ã‹8ÿWëº
I ¹   ‹Ã‹8ÿWÆC‹Æèh8ûÿ‹È‹Ö‹Ã‹ÿSƒÄ_^[Ã‹ÀU‹ìj SVW‹ò‹Ø3ÀUhqıD dÿ0d‰ ‹ÖEüèP7ûÿEüºˆıD è+8ûÿ‹Uü‹Ãè%ÿÿÿ3ÀZYYd‰hxıD Eüè6ûÿÃéî2ûÿëğ_^[Y]Ã  ÿÿÿÿ   
                          ÀıD    Xc@ ,,@ à*@ +@ L+@ ECompressError                        şD    ÀıD ,,@ à*@ +@ L+@ ECompressDataError                        LşD    ÀıD ,,@ à*@ +@ L+@ ECompressInternalError                        ¤şD    Ä@ ,,@ à*@ +@ L+@ „ E ¸şD ÀşD TCustomDecompressoré)ûÿ@ éw)ûÿ@                         ÿD    ˜şD ,,@ à*@ +@ L+@ „ E Ä E ,E TStoredDecompressor                        PÿD   Ä@ ,,@ à*@ +@ dE TCompressedBlockReaderV3ö¹tI ‹Æº   ¨t	Ñè5 ƒ¸íëÑèJuî‰FƒÁş   uÙ^Ã@ SVW‹ñ‹ú‹Øƒ=pI  uèµÿÿÿjhpI èİXûÿ‹Ç…öt%‹Ófâÿ 3ÉŠf3Ñ·Ò‹•tI Áë3Ó‹ÚN@…öuÛ‹Ã_^[ÃSV‹ò‹Ø‹Ó‹ÎƒÈÿèšÿÿÿƒğÿ^[ÃU‹ìƒÄøSVˆMû‰Uüƒ}ü|]ƒmü‹ğ3À;Eü}PŠ€úèt€úéu=@ŠT„Òt€úÿu*‹UĞƒÂ€}û u÷Ú3É¶ÓˆÁêAƒùuèƒÀë@;Eü|°^[YY]Â @ U‹ìSV„ÒtƒÄğè›,ûÿ‹Ú‹ğ3Ò‹Æè*ûÿ‹E‰F‹E‰F„Ût
d    ƒÄ‹Æ^[]Â SVWQ‹Ù‰$‹ò…Û~/‹Ö‹<$‹Ë‹GÿW…Àu¹E ²¸şD è§Šûÿè"0ûÿğ+Ø…ÛÑZ_^[Ã ÿÿÿÿ   Unexpected end of stream    Ã@ U‹ìƒÄàSVW„ÒtƒÄğèë+ûÿ‹ñˆUÿ‹Ø3Ò‹ÃèÛ)ûÿ‰sUø¹   ‹Æ‹8ÿWƒøuUğ¹   ‹Æ‹8ÿWƒøt¹DE ²¸şD èŠûÿè†/ûÿEğº   èMşÿÿ;Eøt¹DE ²¸şD èã‰ûÿè^/ûÿUè‹Æ‹ÿEè‹UğèšşÿUà‹Æ‹ÿQUàEèèQşÿ…À~¹DE ²¸şD è ‰ûÿè/ûÿ€}ô tShdE ²‹Eÿ‰C‹Eğ‰CÆC€}ÿ t
d    ƒÄ‹Ã_^[‹å]Â ÿÿÿÿ   Compressed block is corrupted   SVƒÄø‹Ú‹ğ‹Fèã(ûÿ€~ t‹Ô‹F‹ÿ‹Ä‹VèŞ şÿ‹Ô‹F‹ÿQ3Ò‹Æè«(ûÿ„Ût‹ÆèÀ*ûÿ‹ÆYZ^[ÃSVQ‹Øƒ{}¹DE ²¸şD èÌˆûÿèG.ûÿ‹Ô¹   ‹Cè|õÿÿƒk‹sş   ~¾   S‹Î‹Cè[õÿÿ)s3À‰C‰sC‹ÖèÖüÿÿ;$t¹DE ²¸şD èlˆûÿèç-ûÿZ^[Ã   ÿÿÿÿ   Compressed block is corrupted   SVWUQ‹ù‹ğ3À‰$‹ê…ÿ~?ƒ~ uƒ~ t3‹Æè)ÿÿÿ‹ß‹F;Øv‹Ø‹Õ‹FD‹Ëè’#ûÿ^)^ë+û$…ÿÁ‹$Z]_^[ÃSVW‹ñ‹ú‹Ø‹C…Àt‹×‹Î‹ÿSë%‹×‹Î‹Ãè~ÿÿÿ;ğt¹E ²¸şD è¡‡ûÿè-ûÿ_^[Ãÿÿÿÿ   Compressed block is corrupted   U‹ìƒÄğSVW3Û‰]ğ‰]ô‰Mø‰Uü‹ğ‹}3ÀUhE dÿ0d‰ ‹Çè£/ûÿ…ö„   ²%‹ÆèÂoûÿ‹Ø…ÛuEğ‹Öè®0ûÿ‹Uğ‹Çèô0ûÿëi;ót‹Ë+ÎEô‹ÖèD0ûÿ‹Ç‹UôèÖ0ûÿ‹óCŠÏ,	s&3ÀŠƒè1;Eø‹Ç3ÒŠ‹Mü‹”‘<ÿÿÿèª0ûÿƒÆë‹Çº(E è™0ûÿF€;%…xÿÿÿFérÿÿÿ3ÀZYYd‰hE Eğº   è/ûÿÃéQ+ûÿëë_^[‹å]Â   ÿÿÿÿ   %   U‹ìSVW‹ù‹ò‹Ø‹EP3ÀŠÃ‹…tI èì1ûÿ‹Ö‹Ïè×şÿÿ_^[]Â SVWQ‹ù‹ò‹ØW‰t$T$3É‹Ãè´ÿÿÿZ_^[Ã@ SV³Ï¾tI ‹Æèp.ûÿƒÆşËuò^[Ã‹À¹¼E ²¸Xc@ èï…ûÿèj+ûÿÃ ÿÿÿÿG   The setup files are corrupted. Please obtain a new copy of the program. SVWUQ‹ú‹ğƒÿP~‹ÆºĞI ¹@   è¡"ûÿtèrÿÿÿ‹ÆƒÀ@‰Ã‹C‹S÷Ò;Âu;øu;Ï   tèNÿÿÿ‹ÃƒÀ‰Ås‰4$‹$+Õ‹Åè‰ùÿÿ;Cu	‹$€xÿ tè"ÿÿÿ³Ï¾tI ;,$rèÿÿÿ‹Åè¾kûÿ‹ø‹Æ‹Ï‹ÕèA.ûÿGïƒÆşËuØZ]_^[ÃU‹ìƒÄœSVˆMÿ‹ò‹Øè¼şÿÿ‹Ãè-Åıÿ„Àu#‰]œÆE EœPj ¹\E ²¸Xc@ èã„ûÿè**ûÿjj j‹Ë²¸höD è¾ñÿÿ‰Eø3ÒUhDE dÿ2d‰"‹Ö‹EøègñÿÿU´¹@   ‹Eø‹ÿSƒø@tèaşÿÿE´ºĞI ¹@   èw!ûÿtèHşÿÿU¤¹   ‹Eø‹ÿSƒøtè.şÿÿ‹U¬÷Ò‹E¨;ĞuƒøP~	}¤Ï   tèşÿÿ‹Ö‹Eøèñğÿÿ‹E¨è©ûÿ‰Eô3ÀUhêE dÿ0d‰ ‹]ô‹Ó‹M¨‹Eø‹0ÿV;E¨tèÒıÿÿ‹Ã‹U¨è0şÿÿ3ÀZYYd‰hñE ‹EôèwûÿÃéu(ûÿëğ€}ÿ t7º°I ¹,   ‹Eø‹ÿSƒø,tè‰ıÿÿ¸°I º	I ¹   è ûÿtènıÿÿ3ÀZYYd‰hKE ‹Eøè#ûÿÃé(ûÿëğ^[‹å]Ã   ÿÿÿÿ^   Messages file "%s" is missing. Please correct the problem or obtain a new copy of the program.  ƒÄøj ‰D$ÆD$ L$‹ÂºèE è¾oûÿYZÃ   ÿÿÿÿ   0x%.8x  è‹üÿÿÃ‹ÀU‹ìSVW3ÀUh$	E dÿ0d‰ ¸
I èAûÿ3ÀZYYd‰h+	E Ãé;'ûÿëø_^[]ÃU‹ìÄ şÿÿSVW‹ò‹ØÆEÿ 3ÀUhø
E dÿ0d‰ ‹ÃèŸÂıÿ„Àu3ÀZYYd‰é˜  jj j‹Ë²¸höD èFïÿÿ‰Eø3ÀUhç
E dÿ0d‰ U¸¹@   ‹Eø‹ÿSƒø@uf}¸MZuƒ}ô uè(ûÿ3ÀZYYd‰é>  ‹Uô‹Eøè¹îÿÿ•ôşÿÿ¹Ä   ‹Eø‹ÿS=Ä   u…ôşÿÿ8LE  tèÀ'ûÿ3ÀZYYd‰éù   ƒ}°wè¨'ûÿ3ÀZYYd‰éá   ‹U¬‹Eøè\îÿÿ•èşÿÿ¹   ‹Eøè!îÿÿ€½èşÿÿÿufƒ½éşÿÿu	€½ëşÿÿÿtè\'ûÿ3ÀZYYd‰é•   ƒ½ğşÿÿHsèA'ûÿ3ÀZYYd‰ë}• şÿÿ¹H   ‹EøèÈíÿÿ·… şÿÿƒøH|-·…¢şÿÿƒø4|!½´şÿÿ½ïşuV‹şµ´şÿÿ¹   ó¥^ÆEÿ3ÀZYYd‰hî
E ‹Eøèr ûÿÃéx%ûÿëğ3ÀZYYd‰ë
és#ûÿèŠ&ûÿŠEÿ_^[‹å]ÃU‹ìƒÄèSVW‰Uü‹ğÆEû EôP‹Æè,ûÿ‹øWèCPûÿ‹Ø…Û~w‹Ãè
ûÿ‰Eğ3ÀUh§E dÿ0d‰ ‹EğPS‹EôPWèPûÿ…Àt/EèPEìPhĞE ‹EğPèPûÿ…Àt‹Eì‹Uü‹ğ‹ú¹   ó¥ÆEû3ÀZYYd‰hÄE ‹EğèºûÿÃé¸$ûÿëğƒ=Ü I t‹Uü‹ÆèoıÿÿˆEûŠEû_^[‹å]Ã  \   SƒÄÌ‹Ú‹Ôè+ÿÿÿ„Àt‹T$‰‹T$‰SƒÄ4[Ã        #Translated by Andreas Tarandi <torandi@gmail.com>
#
#Changelog:
#2007-09-13 - Initial Translation
#2008-01-12 - Corrected two typos

#common strings
pp_translate_string "Back" "Tillbaka"
pp_translate_string "PRESS ANY KEY TO START" "TRYCK PÃ… VALFRI KNAPP FÃ–R ATT BÃ–RJA"

#game_type_select
pp_translate_string "Enter an event" "Turnering"
pp_translate_string "Practice" "TrÃ¤ning"
pp_translate_string "Configuration" "InstÃ¤llningar"
pp_translate_string "Credits" "Tack till"
pp_translate_string "Quit" "Avsluta"

#configuration
pp_translate_string "Graphics" "Grafik"
pp_translate_string "Video" "Video"
pp_translate_string "Audio" "Ljud"
pp_translate_string "Keyboard" "Tangentbord"
pp_translate_string "Joystick" "Joystick"

#config mode
pp_translate_string "Cancel" "Avbryt"
pp_translate_string "Ok" "OK"

#graphics configuration
pp_translate_string "Graphics Configuration" "GrafikinstÃ¤llningar"
pp_translate_string "Language:" "SprÃ¥k:"
pp_translate_string "Show UI Snow:" "Visa UI snÃ¶:"
pp_translate_string "Display FPS:" "Visa FPS:"
pp_translate_string "Display Progress Bar:" "Visa Progress Bar:"
pp_translate_string "Draw Fog:" "Rita dimma:"
pp_translate_string "Reflections:" "Reflektioner:"
pp_translate_string "Shadows:" "Skuggor:"
pp_translate_string "Model:" "Modell:"

#video configuration
pp_translate_string "Video Configuration" "VideoinstÃ¤llningar"
pp_translate_string "Resolution:" "UpplÃ¶sning:"
pp_translate_string "Bits Per Pixel:" "Bitar per pixel:"
pp_translate_string "Fullscreen:" "FullskÃ¤rm:"
pp_translate_string "Experimental (needs restart)" "Experimentel (krÃ¤ver omstart)"
pp_translate_string "Enable FSAA:" "Aktivera FSAA:"
pp_translate_string "To change the resolution, or switch into fullscreen mode" "FÃ¶r att byta upplÃ¶sning, eller fÃ¶r att byta"
pp_translate_string "use options.txt, located in the config folder." "till fullskÃ¤rm, anvÃ¤nd options.txt i konfigureringsmappen."

#audio configuration
pp_translate_string "Audio Configuration" "LjudinstÃ¤llningar"
pp_translate_string "Sound Effects:" "Ljudeffekter:"
pp_translate_string "Music:" "Musik:"
pp_translate_string "(needs restart)" "(krÃ¤ver omstart)"
pp_translate_string "Disable Audio:" "StÃ¤ng av ljud:"
pp_translate_string "Stereo:" "Stereo:"
pp_translate_string "Bits Per Sample:" "Bits Per Sample:"
pp_translate_string "Samples Per Second:" "Samples Per Second:"

#keyboard configuration
pp_translate_string "Keyboard Configuration" "TangentbordsinstÃ¤llningar"
pp_translate_string "Turn left:" "SvÃ¤ng vÃ¤nster:"
pp_translate_string "Turn right:" "SvÃ¤ng hÃ¶ger:"
pp_translate_string "Paddle:" "Skjut pÃ¥:"
pp_translate_string "Brake:" "Bromsa:"
pp_translate_string "Jump:" "Hoppa:"
pp_translate_string "Trick:" "Trick:"
pp_translate_string "Reset:" "Ã…terstÃ¤ll:"

#joystick configuration
pp_translate_string "Joystick Configuration" "JoystickinstÃ¤llningar"
pp_translate_string "Enable Joystick" "Aktivera Joystick:"

#race select
pp_translate_string "Race!" "KÃ¶r!"
pp_translate_string "Select a race" "VÃ¤lj ett lopp"
pp_translate_string "Contributed by:" "Skapad av:"
pp_translate_string "Unknown" "OkÃ¤nd"
pp_translate_string "Time:" "Tid:"
pp_translate_string "Herring:" "StrÃ¶mmingar:"
pp_translate_string "Score:" "PoÃ¤ng:"

#event select
pp_translate_string "Continue" "FortsÃ¤tt"
pp_translate_string "Select event and cup" "VÃ¤lj turnering och tÃ¤vling"
pp_translate_string "Event:" "Turnering:"
pp_translate_string "Cup:" "Cup:"
pp_translate_string "You've won this cup!" "Du har vunnit den hÃ¤r tÃ¤vlingen!"
pp_translate_string "You must complete this cup next" "Du mÃ¥ste vinna den hÃ¤r tÃ¤vlingen hÃ¤rnÃ¤st"
pp_translate_string "You cannot enter this cup yet" "Du kan inte delta i den hÃ¤r tÃ¤vlingen Ã¤nnu"

#event race select
pp_translate_string "You don't have any lives left." "Du har inga liv kvar."
pp_translate_string "Race won! Your result:" "Du vann loppet! Ditt resultat:"
pp_translate_string "Needed to advance:" "Krav fÃ¶r att avancera:"
pp_translate_string "You can't enter this race yet." "Du kan inte kÃ¶ra det hÃ¤r loppet Ã¤nnu."

#loading
pp_translate_string "Loading, Please Wait..." "Laddar, var vÃ¤nlig vÃ¤nta..."

#paused
pp_translate_string "Resume" "FortsÃ¤tt"
pp_translate_string "Paused" "Pausad"

#race over
pp_translate_string "Race Over" "Loppet slut"
pp_translate_string "Time: %02d:%02d.%02d" "Tid: %02d:%02d.%02d"
pp_translate_string "Herring: %3d" "StrÃ¶mmingar: %3d"
pp_translate_string "Score: %6d" "PoÃ¤ng: %6d"
pp_translate_string "Max speed: %3d km/h" "Maxfart: %3d km/h"
pp_translate_string "Was flying: %.01f %% of time" "FlÃ¶g: %.01f %% av tiden"
pp_translate_string "Race aborted" "Loppet avbrutet"
pp_translate_string "You beat your best score!" "Du slog rekordet!"
pp_translate_string "Congratulations! You won the event!" "Grattulerar! Du vann turneringen!"
pp_translate_string "Congratulations! You won the cup!" "Grattulerar! Du vann tÃ¤vlingen!"
pp_translate_string "You advanced to the next race!" "Du avancerade till nÃ¤sta lopp!"
pp_translate_string "You didn't advance." "Du avancerade inte."

#highscore
pp_translate_string "Highscore" "Highscore"
pp_translate_string "You made it to the %s place in the highscore!" "Du slog dig in pÃ¥ %s platsen pÃ¥ highscoren"
pp_translate_string "Player name:" "Namn:"
pp_translate_string "1:st" "1:a"
pp_translate_string "2:nd" "2:a"
pp_translate_string "3:rd" "3:e"
pp_translate_string "4:th" "4:e"
pp_translate_string "5:th" "5:e"
pp_translate_string "6:th" "6:e"
pp_translate_string "7:th" "7:e"
pp_translate_string "8:th" "8:e"
pp_translate_string "9:th" "9:e"
pp_translate_string "10:th" "10:e"
pp_translate_string "No records" "Inga rekord"

#HUD
#Strings are prefixed to prevent collisions with the UI
pp_translate_string "H|FPS: %.1f" "FPS: %.1f"
pp_translate_string "H|Time:" "Tid:"
pp_translate_string "H|km/h" "km/h"
#herring counter
pp_translate_string "H|%03d" "%03d"
#time counter
pp_translate_string "H|%02d:%02d:%02d" "%02d:%02d:%02d"
#speed counter
pp_translate_string "H|%.0f" "%.0f"

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          àèn	ûÿ@P‹Eàè(ûÿP‹EäPj ‹EÜP‹EøPè¸+ûÿëjEìP‹EäPj ‹EÜP‹EøPè+ûÿ‹EøPèL+ûÿ3ÀZYYd‰h‚,E E¸º   è¾ûÿEÌè–ûÿEØèûÿEàè†ûÿÃéäûÿëÓ_^[‹å]Ã   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ÿÿÿÿ4   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ÿÿÿÿ   

    ÿÿÿÿ   RegCreateKeyEx  U‹ìƒÄ¼SVW3É‰MÜ‰MĞ‰MÀ‰M¼‰Mà‰Uü3ÒUhü/E dÿ2d‰"ÆEû j jUôR¹0E º  €èÂ®ıÿ‹Øƒû„O  …Û„—   EÜPUĞ¸  €èîÿÿ‹EĞ‰EÔ¸P0E ‰EØUÔ¹   °=èy×ÿÿEÜº0E è¼ûÿEÜPEĞP¸ 0E ‰EÄUÀ‹Ãè?ûÿ‹EÀ‰EÈU¼‹Ãèf¸ıÿ‹E¼‰EÌUÄ¹   °4è-×ÿÿ‹UĞXètûÿ‹MÜ²¸Xc@ è…]ûÿè ûÿ3ÀUhÅ/E dÿ0d‰ EäPj EèPj ‹Eüèúûÿ‹ØS‹EôPèŠ)ûÿ…Àt
èiûÿét  ÆEó 3À‰Eì3ÀUh/E dÿ0d‰ ‹EèHtƒèt*Ht[éƒ   ‹ÓMà‹EôèÕ¬ıÿ„Àtr‹Eàè>ûÿ‰EìÆEóëaƒ}ä|[ƒ}äUEäPEìPj j ‹EüètûÿP‹EôPè)ûÿ…Àu3ÆEóë-ÇEä   EäPEìPj j ‹EüèEûÿP‹EôPè×(ûÿ…ÀuÆEó3ÀZYYd‰ë
é\ÿúÿèsûÿ€}ó u
è˜ûÿé£   ÿMìƒ}ì ÆEû‹EüèúûÿP‹EôPèd(ûÿëd‹EèHt
ƒÀşƒèr8ëTUà‹Eìè“=ûÿ‹Eàèûÿ@P‹EàèÁûÿPjj ‹Eüè´ûÿP‹EôPèN(ûÿëjEìP‹EèPj ‹Eüè”ûÿP‹EôPè.(ûÿ3ÀZYYd‰hÌ/E ‹EôPèĞ'ûÿÃéš ûÿëï3ÀZYYd‰h0E E¼º   è:ûÿEĞèûÿEÜº   è%ûÿÃéc ûÿëÖŠEû_^[‹å]Ã   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ÿÿÿÿ4   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ÿÿÿÿ   

    ÿÿÿÿ   RegOpenKeyEx    SÄÀşÿÿ‹Ù‹ÌèœßÿÿƒøÿtPè™'ûÿö$u‹D$‰‹D$‰C°ë3À3Ò‰3Ò‰SÄ@  [Ã@ U‹ìP¸   ÄğÿÿPHuö‹EüƒÄ¤SV‰Mü‹ò‹ØE èºÒıÿVjj j‹Ë²¸,E èAäÿÿ‰Eø3ÒUh†1E dÿ2d‰"• ÿşÿ¹   ‹Eø‹ÿS…Àt• ÿşÿM ‘è–ÒıÿëØ3ÀZYYd‰h1E ‹EøèÓùúÿÃéÙşúÿëğ‹UüE èÓıÿ^[‹å]Ã‹ÀSV‹ò‹Ø‹ÃèÇûÿ‹Ğ‹Î‹Ãè„Üıÿ^[ÃU‹ìP¸   ÄğÿÿPHuö‹EüƒÄôSVW3É‰øßÿÿ‰ôßÿÿ‰ğßÿÿ‰Mü‹ú‹Ø3ÀUh03E dÿ0d‰ €=èI  uq…ğßÿÿèƒ¥ıÿ‹…ğßÿÿ•ôßÿÿèn‘ıÿ…ôßÿÿºH3E èFûÿ‹…ôßÿÿè÷ûÿ‹Ğ…øßÿÿèŞûÿ‹…øßÿÿº €  èr¯ıÿ‹ğ…öthP3E Vèõ&ûÿ£ìI ÆèI ƒ=ìI  „€   Uü‹ÇèZ”ıÿ„Ûu•øßÿÿ‹EüèŒîÿÿ‹•øßÿÿEüèâûÿhÿ  …üßÿÿP‹Eüè®ûÿP‹EüèiûÿPj j è‹'ûÿfÇ„Eüßÿÿ  fƒ½üßÿÿ t…üßÿÿPj ÿìI …Àu3Ûë³ë3Û3ÀZYYd‰h73E …ğßÿÿº   èù ûÿEüèÑ ûÿÃé/ıúÿëà‹Ã_^[‹å]Ãÿÿÿÿ   sfc.dll SfcIsFileProtected  U‹ìQSVW‹ñ‹Ú‰Eü‹}3ÀUhç3E dÿ0d‰ €ûuÿÖj2‹EüPè—.ûÿ=  tì€ûuÿÖhÿ   jÿj EüPjèå,ûÿHtçÿÖW‹EüPè>%ûÿ…ÀuÇÿÿÿÿ3ÀZYYd‰hî3E ‹EüPèş#ûÿÃéxüúÿëï_^[Y]Â ‹ÀU‹ìƒÄœSVW3Û‰] ‰]œ‰]ø‹ù‹ÚˆEÿ‹u‹Eè
ûÿ3ÀUhì5E dÿ0d‰ ‹Ãº6E èLûÿuEø‹×èP ûÿéè   h6E Sh6E Eøº   èÓûÿ…ÿtÿuøh 6E WEøº   è¹ûÿU ‹Ãè_“ıÿ‹E º,6E èŠ5ûÿ…ÀtU ‹ÃèD“ıÿ‹E º<6E èo5ûÿ…Àupèn¥ıÿ„Àt7h6E EœèÑ¢ıÿ‹EœU èÂıÿÿu hL6E ÿuøh6E Eøº   èEûÿë0h6E Eœèn¢ıÿ‹EœU è‹ıÿÿu hd6E ÿuøEøº   èûÿƒ} u
U‹Ãè[’ıÿE´3ÉºD   èôóúÿÇE´D   ÇEà   f‹Ef‰Eäƒ} t‹EèÔûÿ‹Øë3Ûj j j h   j SE´PE¤P‹Eøè°ûÿ‹È3ÒŠEÿèPØÿÿ÷ØÀ÷Ø‹Ø„Ûu	è#ûÿ‰ë‹E¨Pè2"ûÿV‹MŠU‹E¤è£ıÿÿ3ÀZYYd‰hó5E Eœº   èEşúÿEøèşúÿEèşúÿÃésúúÿëÛ‹Ã_^[‹å]Â   ÿÿÿÿ   >   ÿÿÿÿ   "   ÿÿÿÿ       ÿÿÿÿ   .bat    ÿÿÿÿ   .cmd    ÿÿÿÿ   cmd.exe" /C "   ÿÿÿÿ   COMMAND.COM" /C     U‹ìƒÄÀSVW‰Mü‹ú‹Ø‹u‹Eè• ûÿ3ÀUhU7E dÿ0d‰ ƒ} u
U‹ÇèÛıÿEÀ3Éº<   ètòúÿÇEÀ<   ÇEÄ@  …Ût
‹Ãè_ ûÿ‰EÌ‹ÇèU ûÿ‰EĞ‹EüèJ ûÿ‰EÔƒ} t‹Eè9 ûÿ‰EØ‹E‰EÜEÀPèg”ıÿ‹Ø÷ÛÛ÷Û„Ûu	è"ûÿ‰ëÇ  ‹Eø…ÀtV‹MŠUè%üÿÿ3ÀZYYd‰h\7E Eè¬üúÿÃé
ùúÿëğ‹Ã_^[‹å]Â U‹ìj j j SVW‰Eü‹Eüè©ÿúÿ3ÀUh8E dÿ0d‰ 3Û‹Uü¸88E èÀ ûÿ‹ğ…öu¾ÿÿÿEôP‹ÎIº   ‹Eüè½ÿúÿ‹EôUøèz3ûÿƒ}ø t#‹EøèdÿúÿPj h   è‹"ûÿ…Àt
Pèùûÿ³ëEü‹Îº   èºÿúÿƒ}ü u‹3ÀZYYd‰h&8E Eôº   èüúÿÃé@øúÿëë‹Ã_^[‹å]Ã ÿÿÿÿ   ,   U‹ìƒÄìS‹ÚÆEÿ jjj ‹È²¸höD ègÀÿÿ‰Eø3ÀUhå8E dÿ0d‰ Uì‹Eø‹ÿQ}ìq  |Mºc   ‹Eøèù¿ÿÿU÷¹   ‹EøèÁ¿ÿÿöE÷•À:Øt!€u÷ºc   ‹EøèÍ¿ÿÿU÷¹   ‹Eø‹ÿSÆEÿ3ÀZYYd‰hì8E ‹EøètòúÿÃéz÷úÿëğŠEÿ[‹å]ÃSƒÄì‹ØÇ$   TD$Pè°ûÿ…Àt‹ÃT$¹   è<üúÿë‹Ãè×úúÿƒÄ[Ã‹ÀSÄüşÿÿ‹ØÇ$   TD$Pè)ûÿ…Àt‹ÃT$¹   èıûúÿë‹Ãè˜úúÿÄ  [Ãÿ%@8I ‹ÀƒÄìƒ=Ü I u[Tj(è@ûÿPèòûÿ…Àu3ÀëTD$Phô9E j èÑûÿÇD$   ÇD$   j j j D$Pj ‹D$PèÿÿÿèMûÿ…Àt3Àëj jèÌ$ûÿ÷ØÀ÷ØƒÄÃ  SeShutdownPrivilege U‹ìQSVW‰Uü‹Ø‹ñN…ö|GF3ÿƒÿu‹EPè¸ ûÿëƒÿ~	‹EPè¨ ûÿ‹Uü‹Ãè¾Ôÿÿ…ÀuèÕûÿƒøtèËûÿƒøtGNu¼_^[Y]Â ‹ÀU‹ìÄğÿÿPƒÄŒSVW3Ò‰•ïÿÿ‰•Œïÿÿ‰Uô‰Uğ‰Eü3ÀUhC<E dÿ0d‰ E”è3Éıÿ3ÒUh<E dÿ2d‰"èhŸıÿ„À„   j jEøP¹T<E º  €3ÀèW¡ıÿ…À…  Môºˆ<E ‹Eøè{ ıÿ„Àt‹EôèsúúÿPEôè:üúÿ‹ĞE”YèïÈıÿMôº¤<E ‹EøèK ıÿ„Àt‹EôèCúúÿPEôè
üúÿ‹ĞE”Yè¿Èıÿ‹EøPèFûÿé¦   …Œïÿÿèœıÿ‹…Œïÿÿ•ïÿÿè%ˆıÿ‹•ïÿÿEğ¹Ì<E è>úúÿ‹Eğènıÿ„Àtkjj j‹Mğ²¸höD è!½ÿÿ‰Eì3ÒUhò;E dÿ2d‰"•”ïÿÿ¹   ‹Eì‹ÿS…Àt•”ïÿÿM”‘è*ÈıÿëØ3ÀZYYd‰hù;E ‹EìègïúÿÃémôúÿëğ3ÀZYYd‰K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/data/translations
END
fr_FR.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/13/trunk/extreme-tuxracer/data/translations/fr_FR.tcl
END
es_ES.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/es_ES.tcl
END
eu_ES.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/eu_ES.tcl
END
de_DE.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/47/trunk/extreme-tuxracer/data/translations/de_DE.tcl
END
nl_NL.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/data/translations/nl_NL.tcl
END
pl_PL.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/53/trunk/extreme-tuxracer/data/translations/pl_PL.tcl
END
languages.tcl
K 25
svn:wc:ra_dav:version-url
V 91
/svnroot/extremetuxracer/!svn/ver/63/trunk/extreme-tuxracer/data/translations/languages.tcl
END
nn_NO.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/nn_NO.tcl
END
it_IT.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/it_IT.tcl
END
sk_SK.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/63/trunk/extreme-tuxracer/data/translations/sk_SK.tcl
END
en_GB.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/15/trunk/extreme-tuxracer/data/translations/en_GB.tcl
END
fi_FI.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/data/translations/fi_FI.tcl
END
sv_SE.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/data/translations/sv_SE.tcl
END
ro_RO.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/61/trunk/extreme-tuxracer/data/translations/ro_RO.tcl
END
pt_PT.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/pt_PT.tcl
END
nb_NO.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/nb_NO.tcl
END
ru_RU.tcl
K 25
svn:wc:ra_dav:version-url
V 86
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/translations/ru_RU.tcl
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/translations
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-13T23:52:36.812478Z
65
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

fr_FR.tcl
file
63



2007-12-31T17:50:29.000000Z
8396b6f0255239e208f32d30c2c32d6b
2007-09-21T17:46:49.180251Z
13
Torandi

es_ES.tcl
file
63



2007-12-31T17:50:29.000000Z
1b790e88fb409015bfa9c5014b615fa9
2007-09-01T16:38:12.025871Z
2
botsnlinux

eu_ES.tcl
file
63



2007-12-31T17:50:29.000000Z
d09709b1815f850a4b05cc70c3a9e9f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

de_DE.tcl
file
63



2007-12-31T17:50:29.000000Z
f0783346f027d7a7d758c6cbe27cb996
2007-12-15T18:12:19.951232Z
47
cpicon92

nl_NL.tcl
file
63



2007-12-31T17:50:29.000000Z
ca9b71c04b8143a4cf1bb804c5eb59ce
2007-09-19T22:16:13.729253Z
9
Torandi

pl_PL.tcl
file
63



2007-12-31T17:50:29.000000Z
bd0210e4ce456c5c5dbf9f2943486ccc
2007-12-30T15:40:33.102064Z
53
cpicon92

languages.tcl
file
63



2008-01-12T07:49:55.000000Z
aa454f044dee6a58d5f120af9ae74da7
2008-01-12T07:44:01.776333Z
63
cpicon92

nn_NO.tcl
file
63



2007-12-31T17:50:29.000000Z
cbf3fb591f5458648e6f6d3e8762c88a
2007-09-01T16:38:12.025871Z
2
botsnlinux

it_IT.tcl
file
63



2007-12-31T17:50:29.000000Z
9103de053f1c5496a4af262063c3ddfb
2007-09-01T16:38:12.025871Z
2
botsnlinux

sk_SK.tcl
file
63



2008-01-12T07:49:55.000000Z
69ef5e03f6803fb0d7b3999f27be4e22
2008-01-12T07:44:01.776333Z
63
cpicon92

en_GB.tcl
file
63



2007-12-31T17:50:29.000000Z
34179db61019880714b1309ab558f51c
2007-09-21T19:40:07.008509Z
15
Torandi

fi_FI.tcl
file
63



2007-12-31T17:50:29.000000Z
5948cb736b39db98efc7cb335b73514b
2007-09-19T22:16:13.729253Z
9
Torandi

sv_SE.tcl
file




2008-01-14T00:15:17.000000Z
58f2240e00d29deb4b6d3f49cb4a37c7
2008-01-13T23:52:36.812478Z
65
cpicon92

ro_RO.tcl
file
63



2008-01-12T07:49:55.000000Z
c7cb5d65ef1886a6a0a4b40d16b66766
2008-01-03T03:22:14.430488Z
61
cpicon92

pt_PT.tcl
file
63



2007-12-31T17:50:29.000000Z
72daa78fb818ee464795a0436d148adc
2007-09-01T16:38:12.025871Z
2
botsnlinux

nb_NO.tcl
file
63



2007-12-31T17:50:29.000000Z
b015b3699685d37c27b6fdb5f0a11cfa
2007-09-01T16:38:12.025871Z
2
botsnlinux

ru_RU.tcl
file
63



2007-12-31T17:50:29.000000Z
792c410d29398aa7086e1e3a6090e529
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              # Makefile.in generated by automake 1.10 from Makefile.am.
# data/Makefile.  Generated from Makefile.in by configure.

# Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006  Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.




pkgdatadir = $(datadir)/etracer
pkglibdir = $(libdir)/etracer
pkgincludedir = $(includedir)/etracer
am__cd = CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd
install_sh_DATA = $(install_sh) -c -m 644
install_sh_PROGRAM = $(install_sh) -c
install_sh_SCRIPT = $(install_sh) -c
INSTALL_HEADER = $(INSTALL_DATA)
transform = $(program_transform_name)
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_triplet = x86_64-unknown-linux-gnu
host_triplet = x86_64-unknown-linux-gnu
target_triplet = x86_64-unknown-linux-gnu
subdir = data
DIST_COMMON = $(nobase_dist_ppdata_DATA) $(srcdir)/Makefile.am \
	$(srcdir)/Makefile.in
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
am__aclocal_m4_deps = $(top_srcdir)/configure.ac
am__configure_deps = $(am__aclocal_m4_deps) $(CONFIGURE_DEPENDENCIES) \
	$(ACLOCAL_M4)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = $(top_builddir)/config.h
CONFIG_CLEAN_FILES =
SOURCES =
DIST_SOURCES =
am__vpath_adj_setup = srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`;
am__vpath_adj = case $$p in \
    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
    *) f=$$p;; \
  esac;
am__strip_dir = `echo $$p | sed -e 's|^.*/||'`;
am__installdirs = "$(DESTDIR)$(ppdatadir)"
nobase_dist_ppdataDATA_INSTALL = $(install_sh_DATA)
DATA = $(nobase_dist_ppdata_DATA)
DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
ACLOCAL = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run aclocal-1.10
AMTAR = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run tar
AUTOCONF = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run autoconf
AUTOHEADER = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run autoheader
AUTOMAKE = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run automake-1.10
AWK = mawk
CC = gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
CPP = gcc -E
CPPFLAGS =   -DTUXRACER_NO_ASSERT=1 -DHAVE_SDL_MIXER=1 
CXX = g++
CXXDEPMODE = depmode=gcc3
CXXFLAGS = -g -O2  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/tcl8.4 -DPNG_NO_MMX_CODE -I/usr/include/libpng12   -I/usr/include/freetype2
CYGPATH_W = echo
DEFS = -DHAVE_CONFIG_H
DEPDIR = .deps
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EGREP = /bin/grep -E
EXEEXT = 
FT2_CFLAGS = -I/usr/include/freetype2
FT2_CONFIG = /usr/bin/freetype-config
FT2_LIBS = -lfreetype -lz
GREP = /bin/grep
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = $(install_sh) -c -s
LDFLAGS = 
LIBOBJS = 
LIBS =   -lSM -lICE  -lX11 -lXi -lXext -lXmu -lXt   -ldl -lm -L/usr/lib -lSDL -lSDL_mixer  -lGL -lGLU -L/usr/lib -ltcl8.4${TCL_DBGX} -ldl  -lpthread -lieee -lm -lpng12   -lfreetype -lz
LTLIBOBJS = 
MAINT = #
MAKEINFO = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run makeinfo
MKDIR_P = /bin/mkdir -p
OBJEXT = o
PACKAGE = etracer
PACKAGE_BUGREPORT = 
PACKAGE_NAME = 
PACKAGE_STRING = 
PACKAGE_TARNAME = 
PACKAGE_VERSION = 
PATH_SEPARATOR = :
RANLIB = ranlib
SDL_CFLAGS = -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_CONFIG = /usr/bin/sdl-config
SDL_LIBS = -L/usr/lib -lSDL
SET_MAKE = 
SHELL = /bin/bash
STRIP = 
TR_CFLAGS =  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
TR_CPPFLAGS =  -DTUXRACER_NO_ASSERT=1 -DHAVE_SDL_MIXER=1 
TR_CXXFLAGS =  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
TR_LIBS =  -lSM -lICE  -lX11 -lXi -lXext -lXmu -lXt   -ldl -lm -L/usr/lib -lSDL -lSDL_mixer  -lGL -lGLU
VERSION = SVN Development
XMKMF = 
X_CFLAGS = 
X_EXTRA_LIBS = 
X_LIBS = 
X_PRE_LIBS =  -lSM -lICE
abs_builddir = /home/christian/Desktop/extremetuxracer/data
abs_srcdir = /home/christian/Desktop/extremetuxracer/data
abs_top_builddir = /home/christian/Desktop/extremetuxracer
abs_top_srcdir = /home/christian/Desktop/extremetuxracer
ac_ct_CC = gcc
ac_ct_CXX = g++
am__include = include
am__leading_dot = .
am__quote = 
am__tar = ${AMTAR} chof - "$$tardir"
am__untar = ${AMTAR} xf -
bindir = ${exec_prefix}/bin
build = x86_64-unknown-linux-gnu
build_alias = 
build_cpu = x86_64
build_os = linux-gnu
build_vendor = unknown
builddir = .
datadir = ${datarootdir}
datarootdir = ${prefix}/share
docdir = ${datarootdir}/doc/${PACKAGE}
dvidir = ${docdir}
exec_prefix = ${prefix}
host = x86_64-unknown-linux-gnu
host_alias = 
host_cpu = x86_64
host_os = linux-gnu
host_vendor = unknown
htmldir = ${docdir}
includedir = ${prefix}/include
infodir = ${datarootdir}/info
install_sh = $(SHELL) /home/christian/Desktop/extremetuxracer/install-sh
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localedir = ${datarootdir}/locale
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
mkdir_p = /bin/mkdir -p
oldincludedir = /usr/include
pdfdir = ${docdir}
ppdatadir = ${datarootdir}/etracer
prefix = /usr/local
program_transform_name = s,x,x,
psdir = ${docdir}
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
target = x86_64-unknown-linux-gnu
target_alias = 
target_cpu = x86_64
target_os = linux-gnu
target_vendor = unknown
top_builddir = ..
top_srcdir = ..
nobase_dist_ppdata_DATA = \
			courses/contrib/doing/terrain.png \
			courses/contrib/doing/elev.png \
			courses/contrib/doing/preview.png \
			courses/contrib/doing/trees.png \
			courses/contrib/doing/course.tcl \
courses/events/c-mountain_mania/herringrunicon.png \
courses/events/c-mountain_mania/event.tcl \
courses/events/c-mountain_mania/cupicon.png \
courses/events/c-mountain_mania/hippo_run/course.tcl \
courses/events/c-mountain_mania/hippo_run/preview.png \
courses/events/c-mountain_mania/hippo_run/trees.png \
courses/events/c-mountain_mania/hippo_run/terrain.png \
courses/events/c-mountain_mania/hippo_run/elev.png \
courses/events/c-mountain_mania/in_search_of_vodka/course.tcl \
courses/events/c-mountain_mania/in_search_of_vodka/elev.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/preview.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/trees.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/terrain.rgb \
courses/events/c-mountain_mania/crazy_path/course.tcl \
courses/events/c-mountain_mania/crazy_path/preview.png \
courses/events/c-mountain_mania/crazy_path/trees.png \
courses/events/c-mountain_mania/crazy_path/terrain.png \
courses/events/c-mountain_mania/crazy_path/elev.png \
courses/events/c-mountain_mania/hey_tux/course.tcl \
courses/events/c-mountain_mania/hey_tux/elev.rgb \
courses/events/c-mountain_mania/hey_tux/preview.rgb \
courses/events/c-mountain_mania/hey_tux/trees.rgb \
courses/events/c-mountain_mania/hey_tux/terrain.rgb \
courses/events/c-mountain_mania/ice_pipeline/course.tcl \
courses/events/c-mountain_mania/ice_pipeline/elev.rgb \
courses/events/c-mountain_mania/ice_pipeline/preview.png \
courses/events/c-mountain_mania/ice_pipeline/trees.rgb \
courses/events/c-mountain_mania/ice_pipeline/terrain.rgb \
courses/events/c-mountain_mania/slippy_slidey/course.tcl \
courses/events/c-mountain_mania/slippy_slidey/preview.png \
courses/events/c-mountain_mania/slippy_slidey/trees.png \
courses/events/c-mountain_mania/slippy_slidey/terrain.png \
courses/events/c-mountain_mania/slippy_slidey/elev.png \
courses/events/c-mountain_mania/volcanoes/course.tcl \
courses/events/c-mountain_mania/volcanoes/preview.png \
courses/events/c-mountain_mania/volcanoes/trees.png \
courses/events/c-mountain_mania/volcanoes/terrain.png \
courses/events/c-mountain_mania/volcanoes/elev.png \
courses/events/c-mountain_mania/merry_go_round/course.tcl \
courses/events/c-mountain_mania/merry_go_round/preview.png \
courses/events/c-mountain_mania/merry_go_round/trees.png \
courses/events/c-mountain_mania/merry_go_round/terrain.png \
courses/events/c-mountain_mania/merry_go_round/elev.png \
courses/events/c-mountain_mania/bobsled_ride/course.tcl \
courses/events/c-mountain_mania/bobsled_ride/preview.png \
courses/events/c-mountain_mania/bobsled_ride/trees.png \
courses/events/c-mountain_mania/bobsled_ride/terrain.png \
courses/events/c-mountain_mania/bobsled_ride/elev.png \
courses/events/c-mountain_mania/candy_lane/course.tcl \
courses/events/c-mountain_mania/candy_lane/preview.png \
courses/events/c-mountain_mania/candy_lane/trees.png \
courses/events/c-mountain_mania/candy_lane/terrain.png \
courses/events/c-mountain_mania/candy_lane/elev.png \
courses/events/c-mountain_mania/nature_stroll/course.tcl \
courses/events/c-mountain_mania/nature_stroll/preview.png \
courses/events/c-mountain_mania/nature_stroll/trees.png \
courses/events/c-mountain_mania/nature_stroll/terrain.png \
courses/events/c-mountain_mania/nature_stroll/elev.png \
courses/events/b-herring_run/herringrunicon.png \
courses/events/b-herring_run/hazzard_valley/course.tcl \
courses/events/b-herring_run/hazzard_valley/preview.png \
courses/events/b-herring_run/hazzard_valley/trees.png \
courses/events/b-herring_run/hazzard_valley/terrain.png \
courses/events/b-herring_run/hazzard_valley/elev.png \
courses/events/b-herring_run/mount_herring/course.tcl \
courses/events/b-herring_run/mount_herring/preview.png \
courses/events/b-herring_run/mount_herring/trees.png \
courses/events/b-herring_run/mount_herring/terrain.png \
courses/events/b-herring_run/mount_herring/elev.png \
courses/events/b-herring_run/ski_jump/course.tcl \
courses/events/b-herring_run/ski_jump/preview.png \
courses/events/b-herring_run/ski_jump/trees.png \
courses/events/b-herring_run/ski_jump/terrain.png \
courses/events/b-herring_run/ski_jump/elev.png \
courses/events/b-herring_run/cupicon.png \
courses/events/b-herring_run/keep_it_up/course.tcl \
courses/events/b-herring_run/keep_it_up/preview.png \
courses/events/b-herring_run/keep_it_up/trees.png \
courses/events/b-herring_run/keep_it_up/terrain.png \
courses/events/b-herring_run/keep_it_up/elev.png \
courses/events/b-herring_run/high_road/course.tcl \
courses/events/b-herring_run/high_road/preview.png \
courses/events/b-herring_run/high_road/trees.png \
courses/events/b-herring_run/high_road/terrain.png \
courses/events/b-herring_run/high_road/elev.png \
courses/events/b-herring_run/hamburger_hill/course.tcl \
courses/events/b-herring_run/hamburger_hill/preview.png \
courses/events/b-herring_run/hamburger_hill/trees.png \
courses/events/b-herring_run/hamburger_hill/terrain.png \
courses/events/b-herring_run/hamburger_hill/elev.png \
courses/events/b-herring_run/slalom/course.tcl \
courses/events/b-herring_run/slalom/preview.png \
courses/events/b-herring_run/slalom/trees.png \
courses/events/b-herring_run/slalom/terrain.png \
courses/events/b-herring_run/slalom/elev.png \
courses/events/b-herring_run/mount_satan/course.tcl \
courses/events/b-herring_run/mount_satan/preview.png \
courses/events/b-herring_run/mount_satan/trees.png \
courses/events/b-herring_run/mount_satan/terrain.png \
courses/events/b-herring_run/mount_satan/elev.png \
courses/events/b-herring_run/the_narrow_way/course.tcl \
courses/events/b-herring_run/the_narrow_way/preview.png \
courses/events/b-herring_run/the_narrow_way/trees.png \
courses/events/b-herring_run/the_narrow_way/terrain.png \
courses/events/b-herring_run/the_narrow_way/elev.png \
courses/events/b-herring_run/deadman/course.tcl \
courses/events/b-herring_run/deadman/preview.png \
courses/events/b-herring_run/deadman/trees.png \
courses/events/b-herring_run/deadman/terrain.png \
courses/events/b-herring_run/deadman/elev.png \
courses/events/b-herring_run/bumpy_ride/course.tcl \
courses/events/b-herring_run/bumpy_ride/preview.png \
courses/events/b-herring_run/bumpy_ride/trees.png \
courses/events/b-herring_run/bumpy_ride/terrain.png \
courses/events/b-herring_run/bumpy_ride/elev.png \
courses/events/b-herring_run/event.tcl \
courses/events/b-herring_run/snow_valley/course.tcl \
courses/events/b-herring_run/snow_valley/preview.png \
courses/events/b-herring_run/snow_valley/trees.png \
courses/events/b-herring_run/snow_valley/terrain.png \
courses/events/b-herring_run/snow_valley/elev.png \
courses/events/b-herring_run/tux-toboggan_run/course.tcl \
courses/events/b-herring_run/tux-toboggan_run/preview.png \
courses/events/b-herring_run/tux-toboggan_run/trees.png \
courses/events/b-herring_run/tux-toboggan_run/terrain.png \
courses/events/b-herring_run/tux-toboggan_run/elev.png \
courses/events/b-herring_run/skull_mountain/course.tcl \
courses/events/b-herring_run/skull_mountain/preview.png \
courses/events/b-herring_run/skull_mountain/trees.png \
courses/events/b-herring_run/skull_mountain/terrain.png \
courses/events/b-herring_run/skull_mountain/elev.png \
courses/events/b-herring_run/penguins_cant_fly/course.tcl \
courses/events/b-herring_run/penguins_cant_fly/preview.png \
courses/events/b-herring_run/penguins_cant_fly/trees.png \
courses/events/b-herring_run/penguins_cant_fly/terrain.png \
courses/events/b-herring_run/penguins_cant_fly/elev.png \
courses/events/b-herring_run/ice_labyrinth/course.tcl \
courses/events/b-herring_run/ice_labyrinth/preview.png \
courses/events/b-herring_run/ice_labyrinth/trees.png \
courses/events/b-herring_run/ice_labyrinth/terrain.png \
courses/events/b-herring_run/ice_labyrinth/elev.png \
courses/events/b-herring_run/ive_got_a_woody/course.tcl \
courses/events/b-herring_run/ive_got_a_woody/preview.png \
courses/events/b-herring_run/ive_got_a_woody/trees.png \
courses/events/b-herring_run/ive_got_a_woody/terrain.png \
courses/events/b-herring_run/ive_got_a_woody/elev.png \
courses/events/d-niehoff_experience/herringrunicon.png \
courses/events/d-niehoff_experience/event.tcl \
courses/events/d-niehoff_experience/frozen_lakes/course.tcl \
courses/events/d-niehoff_experience/frozen_lakes/preview.png \
courses/events/d-niehoff_experience/frozen_lakes/trees.png \
courses/events/d-niehoff_experience/frozen_lakes/terrain.png \
courses/events/d-niehoff_experience/frozen_lakes/elev.png \
courses/events/d-niehoff_experience/challenge_one/course.tcl \
courses/events/d-niehoff_experience/challenge_one/preview.png \
courses/events/d-niehoff_experience/challenge_one/trees.png \
courses/events/d-niehoff_experience/challenge_one/terrain.png \
courses/events/d-niehoff_experience/challenge_one/elev.png \
courses/events/d-niehoff_experience/cupicon.png \
courses/events/d-niehoff_experience/secret_valleys/course.tcl \
courses/events/d-niehoff_experience/secret_valleys/preview.png \
courses/events/d-niehoff_experience/secret_valleys/trees.png \
courses/events/d-niehoff_experience/secret_valleys/terrain.png \
courses/events/d-niehoff_experience/secret_valleys/elev.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/course.tcl \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/preview.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/trees.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/terrain.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/elev.png \
courses/events/d-niehoff_experience/explore_mountains/course.tcl \
courses/events/d-niehoff_experience/explore_mountains/preview.png \
courses/events/d-niehoff_experience/explore_mountains/trees.png \
courses/events/d-niehoff_experience/explore_mountains/terrain.png \
courses/events/d-niehoff_experience/explore_mountains/elev.png \
courses/events/d-niehoff_experience/tux_at_home/course.tcl \
courses/events/d-niehoff_experience/tux_at_home/preview.png \
courses/events/d-niehoff_experience/tux_at_home/trees.png \
courses/events/d-niehoff_experience/tux_at_home/terrain.png \
courses/events/d-niehoff_experience/tux_at_home/elev.png \
courses/events/d-niehoff_experience/wild_mountains/course.tcl \
courses/events/d-niehoff_experience/wild_mountains/preview.png \
courses/events/d-niehoff_experience/wild_mountains/trees.png \
courses/events/d-niehoff_experience/wild_mountains/terrain.png \
courses/events/d-niehoff_experience/wild_mountains/elev.png \
courses/events/d-niehoff_experience/chinese_wall/course.tcl \
courses/events/d-niehoff_experience/chinese_wall/preview.png \
courses/events/d-niehoff_experience/chinese_wall/trees.png \
courses/events/d-niehoff_experience/chinese_wall/terrain.png \
courses/events/d-niehoff_experience/chinese_wall/elev.png \
courses/events/a-tux_racer/bumpy_ride/course.tcl \
courses/events/a-tux_racer/bumpy_ride/preview.png \
courses/events/a-tux_racer/bumpy_ride/trees.png \
courses/events/a-tux_racer/bumpy_ride/terrain.png \
courses/events/a-tux_racer/bumpy_ride/elev.png \
courses/events/a-tux_racer/herringrunicon.png \
courses/events/a-tux_racer/event.tcl \
courses/events/a-tux_racer/bunny_hill/course.tcl \
courses/events/a-tux_racer/bunny_hill/preview.png \
courses/events/a-tux_racer/bunny_hill/trees.png \
courses/events/a-tux_racer/bunny_hill/terrain.png \
courses/events/a-tux_racer/bunny_hill/elev.png \
courses/events/a-tux_racer/cupicon.png \
courses/events/a-tux_racer/twisty_slope/course.tcl \
courses/events/a-tux_racer/twisty_slope/preview.png \
courses/events/a-tux_racer/twisty_slope/trees.png \
courses/events/a-tux_racer/twisty_slope/terrain.png \
courses/events/a-tux_racer/twisty_slope/elev.png \
courses/events/a-tux_racer/frozen_river/course.tcl \
courses/events/a-tux_racer/frozen_river/preview.png \
courses/events/a-tux_racer/frozen_river/trees.png \
courses/events/a-tux_racer/frozen_river/terrain.png \
courses/events/a-tux_racer/frozen_river/elev.png \
courses/events/a-tux_racer/path_of_daggers/course.tcl \
courses/events/a-tux_racer/path_of_daggers/preview.png \
courses/events/a-tux_racer/path_of_daggers/trees.png \
courses/events/a-tux_racer/path_of_daggers/terrain.png \
courses/events/a-tux_racer/path_of_daggers/elev.png \
			courses/themes/huds/common/huds.tcl \
				courses/themes/items/herrings/herring_dead.png \
				courses/themes/items/herrings/herring_red.png \
				courses/themes/items/herrings/items.tcl \
				courses/themes/items/herrings/star.png \
				courses/themes/items/herrings/herring_green.png \
				courses/themes/items/flags/flag2.png \
				courses/themes/items/flags/items.tcl \
				courses/themes/items/stuff/items.tcl \
				courses/themes/items/stuff/life.png \
		courses/themes/items/common/finish.png \
		courses/themes/items/common/items.tcl \
		courses/themes/items/common/flag.png \
		courses/themes/items/common/herring_standard.png \
		courses/themes/items/common/start.png \
		courses/themes/conditions/common/nighttop.png \
		courses/themes/conditions/common/sunny_light.tcl \
		courses/themes/conditions/common/eveningtop.png \
		courses/themes/conditions/common/nightback.png \
		courses/themes/conditions/common/cloudyleft.png \
		courses/themes/conditions/common/envmap.png \
		courses/themes/conditions/common/sunnyleft.png \
		courses/themes/conditions/common/sunnybottom.png \
		courses/themes/conditions/common/cloudyfront.png \
		courses/themes/conditions/common/eveningleft.png \
		courses/themes/conditions/common/eveningright.png \
		courses/themes/conditions/common/eveningfront.png \
		courses/themes/conditions/common/evening_light.tcl \
		courses/themes/conditions/common/foggy_light.tcl \
		courses/themes/conditions/common/sunnyfront.png \
		courses/themes/conditions/common/eveningbottom.png \
		courses/themes/conditions/common/cloudytop.png \
		courses/themes/conditions/common/cloudyright.png \
		courses/themes/conditions/common/cloudybottom.png \
		courses/themes/conditions/common/cloudyback.png \
		courses/themes/conditions/common/conditions.tcl \
		courses/themes/conditions/common/sunnyback.png \
		courses/themes/conditions/common/nightfront.png \
		courses/themes/conditions/common/night_light.tcl \
		courses/themes/conditions/common/sunnyright.png \
		courses/themes/conditions/common/eveningback.png \
		courses/themes/conditions/common/nightleft.png \
		courses/themes/conditions/common/nightright.png \
		courses/themes/conditions/common/nightenv.png \
		courses/themes/conditions/common/nightbottom.png \
		courses/themes/conditions/common/sunnytop.png \
		courses/themes/conditions/common/eveningenv.png \
		courses/themes/terrains/add/road.png \
		courses/themes/terrains/add/terrains.tcl \
		courses/themes/terrains/add/speed.png \
		courses/themes/terrains/ice/greenice.png \
		courses/themes/terrains/ice/terrains.tcl \
		courses/themes/terrains/ice/hardice.png \
		courses/themes/terrains/mud/mudstart.png \
		courses/themes/terrains/mud/mudstop.png \
		courses/themes/terrains/mud/mud.png \
		courses/themes/terrains/mud/mudprint.png \
		courses/themes/terrains/mud/terrains.tcl \
		courses/themes/terrains/mud/mudparticles.png \
		courses/themes/terrains/lava/lavastone.png \
		courses/themes/terrains/lava/terrains.tcl \
		courses/themes/terrains/lava/lava.png \
		courses/themes/terrains/sand/sand.png \
		courses/themes/terrains/sand/redsand.png \
		courses/themes/terrains/sand/terrains.tcl \
		courses/themes/terrains/rock/snowyrock.png \
		courses/themes/terrains/rock/terrains.tcl \
		courses/themes/terrains/snow/snowygrass.png \
		courses/themes/terrains/snow/dirtsnow.png \
		courses/themes/terrains/snow/buttprint.png \
		courses/themes/terrains/snow/snowparticles.png \
		courses/themes/terrains/snow/stsnow1.png \
		courses/themes/terrains/snow/stsnow2.png \
		courses/themes/terrains/snow/dsnow.png \
		courses/themes/terrains/snow/dsnow2.png \
		courses/themes/terrains/snow/terrains.tcl \
		courses/themes/terrains/snow/dirtsnowparticles.png \
		courses/themes/terrains/snow/buttstart.png \
		courses/themes/terrains/snow/buttstop.png \
		courses/themes/terrains/common/snow.png \
		courses/themes/terrains/common/buttprint.png \
		courses/themes/terrains/common/snowparticles.png \
		courses/themes/terrains/common/rock.png \
		courses/themes/terrains/common/terrains.tcl \
		courses/themes/terrains/common/ice.png \
		courses/themes/terrains/common/buttstart.png \
		courses/themes/terrains/common/buttstop.png \
		courses/themes/models/stuff/barrier.png \
		courses/themes/models/stuff/barrier.ac \
		courses/themes/models/stuff/models.tcl \
		courses/themes/models/trees/tree_xmas.ac \
		courses/themes/models/trees/tree_xmas.png \
		courses/themes/models/trees/models.tcl \
		courses/themes/models/common/shrub.png \
		courses/themes/models/common/tree.png \
		courses/themes/models/common/tree_barren.ac \
		courses/themes/models/common/shrub.ac \
		courses/themes/models/common/tree_barren.png \
		courses/themes/models/common/tree.ac \
		courses/themes/models/common/models.tcl \
		courses/themes/ppracer.tcl \
		courses/themes/common.tcl \
		courses/course_idx.tcl \
		fonts/PaperCuts20.ttf \
		fonts/PaperCuts_outline.ttf \
		music/race1-jt.it \
		music/wonrace1-jt.it \
		music/credits1-cp.it \
		music/options1-jt.it \
		music/start1-jt.it \
		music/readme \
		textures/menu_top_left.png \
		textures/checkmark.png \
		textures/snowparticles.png \
		textures/gaugespeedmask.png \
		textures/splash.png \
		textures/splash_small.png \
		textures/herringicon.png \
		textures/timeicon.png \
		textures/menu_bottom_left.png \
		textures/menu_top_right.png \
		textures/nopreview.png \
		textures/snow_button.png \
		textures/mirror_button.png \
		textures/mask_outline2.png \
		textures/gaugeenergymask.png \
		textures/wind_button.png \
		textures/mask_outline.png \
		textures/energymask.png \
		textures/gaugeoutline.png \
		textures/menu_bottom_right.png \
		textures/listbox_arrows.png \
		textures/noicon.png \
		textures/speedmask.png \
		textures/conditions_button.png \
		textures/mouse_cursor.png \
		textures/tuxlife.png \
		textures/menu_title.png \
		textures/menu_title_small.png \
		translations/en_GB.tcl \
		translations/it_IT.tcl \
		translations/de_DE.tcl \
		translations/nl_NL.tcl \
		translations/es_ES.tcl \
		translations/pl_PL.tcl \
		translations/eu_ES.tcl \
		translations/fr_FR.tcl \
		translations/fi_FI.tcl \
		translations/sv_SE.tcl \
		translations/languages.tcl \
		translations/nb_NO.tcl \
		translations/nn_NO.tcl \
		translations/pt_PT.tcl \
		translations/ro_RO.tcl \
		translations/ru_RU.tcl \
		translations/sk_SK.tcl \
		tux.tcl \
		tux_snowboard.tcl \
		sounds/tux_hit_tree1.wav \
		sounds/tux_on_snow1.wav \
		sounds/tux_on_rock1.wav \
		sounds/tux_on_ice1.wav \
		sounds/fish_pickup1.wav \
		sounds/fish_pickup2.wav \
		sounds/fish_pickup3.wav \
		terrains.png \
		objects.png \
		etracer_init.tcl \
		tux_walk.tcl \
		models.tcl
all: all-am

.SUFFIXES:
$(srcdir)/Makefile.in: # $(srcdir)/Makefile.am  $(am__configure_deps)
	@for dep in $?; do \
	  case '$(am__configure_deps)' in \
	    *$$dep*) \
	      cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh \
		&& exit 0; \
	      exit 1;; \
	  esac; \
	done; \
	echo ' cd $(top_srcdir) && $(AUTOMAKE) --gnu  data/Makefile'; \
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --gnu  data/Makefile
.PRECIOUS: Makefile
Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@case '$?' in \
	  *config.status*) \
	    cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh;; \
	  *) \
	    echo ' cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe)'; \
	    cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe);; \
	esac;

$(top_builddir)/config.status: $(top_srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh

$(top_srcdir)/configure: # $(am__configure_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
$(ACLOCAL_M4): # $(am__aclocal_m4_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
install-nobase_dist_ppdataDATA: $(nobase_dist_ppdata_DATA)
	@$(NORMAL_INSTALL)
	test -z "$(ppdatadir)" || $(MKDIR_P) "$(DESTDIR)$(ppdatadir)"
	@$(am__vpath_adj_setup) \
	list='$(nobase_dist_ppdata_DATA)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  echo " $(nobase_dist_ppdataDATA_INSTALL) '$$d$$p' '$(DESTDIR)$(ppdatadir)/$$f'"; \
	  $(nobase_dist_ppdataDATA_INSTALL) "$$d$$p" "$(DESTDIR)$(ppdatadir)/$$f"; \
	done

uninstall-nobase_dist_ppdataDATA:
	@$(NORMAL_UNINSTALL)
	@$(am__vpath_adj_setup) \
	list='$(nobase_dist_ppdata_DATA)'; for p in $$list; do \
	  $(am__vpath_adj) \
	  echo " rm -f '$(DESTDIR)$(ppdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(ppdatadir)/$$f"; \
	done
tags: TAGS
TAGS:

ctags: CTAGS
CTAGS:


distdir: $(DISTFILES)
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list='$(DISTFILES)'; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) $(MKDIR_P) `echo "$$dist_files" | \
			   sed '/\//!d;s|^|$(distdir)/|;s,/[^/]*$$,,' | \
			   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -pR $(srcdir)/$$file $(distdir)$$dir || exit 1; \
	    fi; \
	    cp -pR $$d/$$file $(distdir)$$dir || exit 1; \
	  else \
	    test -f $(distdir)/$$file \
	    || cp -p $$d/$$file $(distdir)/$$file \
	    || exit 1; \
	  fi; \
	done
check-am: all-am
check: check-am
all-am: Makefile $(DATA)
installdirs:
	for dir in "$(DESTDIR)$(ppdatadir)"; do \
	  test -z "$$dir" || $(MKDIR_P) "$$dir"; \
	done
install: install-am
install-exec: install-exec-am
install-data: install-data-am
uninstall: uninstall-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	  install_sh_PROGRAM="$(INSTALL_STRIP_PROGRAM)" INSTALL_STRIP_FLAG=-s \
	  `test -z '$(STRIP)' || \
	    echo "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'"` install
mostlyclean-generic:

clean-generic:

distclean-generic:
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-am

clean-am: clean-generic mostlyclean-am

distclean: distclean-am
	-rm -f Makefile
distclean-am: clean-am distclean-generic

dvi: dvi-am

dvi-am:

html: html-am

info: info-am

info-am:

install-data-am: install-nobase_dist_ppdataDATA

install-dvi: install-dvi-am

install-exec-am:

install-html: install-html-am

install-info: install-info-am

install-man:

install-pdf: install-pdf-am

install-ps: install-ps-am

installcheck-am:

maintainer-clean: maintainer-clean-am
	-rm -f Makefile
maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-am

mostlyclean-am: mostlyclean-generic

pdf: pdf-am

pdf-am:

ps: ps-am

ps-am:

uninstall-am: uninstall-nobase_dist_ppdataDATA

.MAKE: install-am install-strip

.PHONY: all all-am check check-am clean clean-generic distclean \
	distclean-generic distdir dvi dvi-am html html-am info info-am \
	install install-am install-data install-data-am install-dvi \
	install-dvi-am install-exec install-exec-am install-html \
	install-html-am install-info install-info-am install-man \
	install-nobase_dist_ppdataDATA install-pdf install-pdf-am \
	install-ps install-ps-am install-strip installcheck \
	installcheck-am installdirs maintainer-clean \
	maintainer-clean-generic mostlyclean mostlyclean-generic pdf \
	pdf-am ps ps-am uninstall uninstall-am \
	uninstall-nobase_dist_ppdataDATA

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ‰Gèşÿÿ‹Øˆ  „Ût‹G      ‰Gëè}úÿf%ÿ fƒør	O   ëƒOO   ƒg$ü_^[Ã   SysTreeView32   U‹ìÄœşÿÿSV‹ØÆƒü    ‹Ãè)‡ûÿƒ   èvWúÿöC…¿   è}úÿf%ÿ fƒør4ƒ=ğI  t+j h\İE ‹Ãè¾¤ûÿPÿğI jjh,  ‹Ãè§¤ûÿPè]„úÿh@  h`  …œşÿÿPj hpİE ètîüÿ‹ğ‹Ãè{¤ûÿ3É‹ÖèÚ¼úÿh  j èRƒúÿPèD„úÿ‰Eü3ÀUhLİE dÿ0d‰ 3Ò‹Ã‹ÿ‘€   3ÀZYYd‰hSİE ‹EüPè„úÿÃéSúÿëï^[‹å]Ã   E x p l o r e r         SV‹ò‹ØÆƒü   3Ò‹Ãèâ	  ‹Ö‹Ãè…˜ûÿ^[Ã‹ÀSVQˆ$‹ò‹Ø‹ÖŠ$‹Ãè•ûÿfƒ>qu7 ğİE "$ŠôİE :Ğu%fÇ  ‹Ãè¦£ûÿèe¼úÿ‹ğ…öt‹Ãè”£ûÿ‹ÖèÅ¼úÿZ^[Ã        SVW‹ú‹Øè úÿ‹ğ…öt ‹Ãèg£ûÿè²¼úÿ;ğuf‹Gfƒøtfƒøt	‹×‹Ãè-œûÿ_^[ÃV‹ğ‹Æ‹ÿQğ^ÃSV‹Ú‹ğ‹CP‹CP‹P‹Æè£ûÿPèºúÿ‰C^[ÃU‹ìƒÄÔSVW3Û‰]Ô‹ñ‹Ú‹ø3ÀUhßE dÿ0d‰ ‹ÆèoUúÿ…ÛtiÇEØ   ‰]Ü‹ÇèÒ¢ûÿUØèÒ»úÿ…Àu	‹ÆèGUúÿëCƒ> u‹Æ‹Uü‹èÈUúÿëUÔ‹Eü‹ è¹äüÿ‹EÔ‹Ö¹   èYúÿ‹Çè‡¢ûÿ‹Óè8»úÿ‹Ø…Ûu—3ÀZYYd‰hßE EÔèíTúÿÃéKQúÿëğ_^[‹å]Ã‹ÀU‹ìj SVW‹Ø3ÀUh£ßE dÿ0d‰ ‹Ãè5¢ûÿèôºúÿ‹ğ…ötMü‹Ö‹Ã‹0ÿV|‹Uüƒ   èèTúÿëƒ   è‹Túÿfƒ»
   t‹Ó‹ƒ  ÿ“  3ÀZYYd‰hªßE Eüè^TúÿÃé¼Púÿëğ_^[Y]ÃU‹ìƒÄÜSèÀúÿ‰EÜ‹EÜUàèò‚úÿUàMè‹E‹@üèygûÿ‹Eè‰Eğ‹Eì‰Eô‹E‹@üè†¡ûÿUğèâºúÿ‹Ø…ÛtnöEøt‹E‹@üèg¡ûÿ¹   ‹Óè§¹úÿëO‹E‹@üèN¡ûÿèºúÿ;Øt‹E‹@ü‹Óè,  ë,‹E‹@ü€¸   töEøFt‹E‹@üè¡ûÿ¹   ‹ÓèV¹úÿ[‹å]ÃU‹ìƒÄÈSVW3É‰Mô‹Ú‰Eü3ÀUh.äE dÿ0d‰ ‹Ó‹Eü‹ÿQğ‹C‹Púkşÿÿ&tZêaşÿÿ„ï  ƒê„  J„ö  Jt)éE  êmşÿÿ„5  J„¾  ê  „¨  é!  ‹@4ºğÚE èZúÿé  ‹Eü€¸   t¹DäE ²¸Xc@ èvªúÿèñOúÿ‹EüÆ€  3ÀUhâE dÿ0d‰ ‹sƒ~…¥   ‹F\€x …˜   ‹F\€x …‹   ‹F\Æ@h  j èç~úÿPèÙúÿ‰Eø3ÀUhîáE dÿ0d‰ ‹V<‹Eü‹ÿ‘€   „Àt#‹EüèÊŸûÿ‹V<èb¸úÿ…Àu3É‹V<‹Eüè¥  ë‹F\Æ@ ÇC   3ÀZYYd‰hõáE ‹EøPèoúÿÃéqNúÿëï3ÀZYYd‰häE ‹EüÆ€   ÃéRNúÿëî‹Cpötj ‹F$ŠH‹V‹Eü‹ÿ“„   ‰Fö tj‹F$ŠH‹V‹Eü‹ÿ“„   ‰Fö@t>‹EüèŸûÿ‹Vè­·úÿ…À•Àƒà‰F ƒ~  u‹F$€x u‹V‹Eü‹ÿ‘Œ   ƒà‰F    ét  ‹Eü€¸ü    …d  ‹EüèhüÿÿéW  ‹Cp‹F$€x …D  ÇC   é8  ‹Cp‹F$€x „%  ƒ~ „  Eô‹Vè$RúÿÆEó‹Eüfƒ¸   tEóPMô‹]ü‹Uü‹ƒ  ÿ“  €}ó „Ü   ‹F$‹Uôè	QúÿÇEÈ   ‹F‰EÌ‹EôèÜSúÿ‰EØ‹EüèûÿUÈè)·úÿ‹Eüèûÿ‹Vè±¶úÿP‹Eüèğûÿ3ÉZèd·úÿ‹EüèŒûÿÿë~UèüÿÿYÇC   ën‹ğƒ~ t0ƒ~< t*‹Eüè¸ûÿ‹V<èh¶úÿ‹ø‹Eüè¦ûÿ‹VèV¶úÿ;øtƒK‹F<…Àt/ÇEÈ   ‰EÌÇEÔ    ‹EüèvûÿUÈèv¶úÿ…Àt
öEĞ tƒK3ÀZYYd‰h5äE EôèÓOúÿÃé1Lúÿëğ_^[‹å]Ãÿÿÿÿ&   Internal error: Item already expanding  SƒÄØ‹ØÇ$@   ‰T$3ÀŠÁ‰D$ ‹Ãèìœûÿ‹Ôè¶úÿƒÄ([ÃU‹ìSVW‹ú‹ğ3Û;÷u³ë‹E‹@üèÁœûÿ‹×èrµúÿ‹ø…ÿuà‹Ã_^[]ÃU‹ìƒÄĞSVW‹ù‰Uø‰Eü‹Eüè“œûÿ‹Uøè+µúÿ‹Ø…Û„   ‹Eüèyœûÿ‹Óèµúÿ‹ğÇEĞ   ‰]Ô‹Eüè^œûÿUĞè^µúÿ…ÀtV‹Eô€x tAU‹×‹Ãè_ÿÿÿY„Àu2‹Eüè2œûÿ‹Óè_´úÿ‹Eüè#œûÿ‹Uøè»´úÿ…Àu3É‹Uø‹Eüèşşÿÿë‹Ï‹Ó‹EüèPÿÿÿ‹Ş…Û…qÿÿÿ_^[‹å]Ã@ U‹ìƒÄÌSVW‰Mü‹ò‹ø‹]„Ût‹Î3Ò‹Çèÿÿÿ‰uÌ„Ût	ÇEĞ ÿÿëÇEĞ ÿÿÇEÔg   3À‰EØ„ÛtƒMÔÇEà   ÇEÜ   ƒ} u‹Eüè?Qúÿ‰Eäë‹Eè2Qúÿ‰EäÇEìÿÿÿÿÇEğÿÿÿÿ„Ût3À‰Eôë…öu	ÇEô   ëÇEôÿÿÿÿ3À‰EøºğÚE ¸   è6Uúÿ‹ğ‹Æ‹UüèúMúÿˆ^ÆF ‰uø‹Çè›ûÿUÌè%³úÿ_^[‹å]Â SVWUƒÄØ‹ù‹Ú‹ğ‹ì‹Æèêšûÿ‹Óèƒ³úÿ‹Ø…Ût@ÇE    ‰]‹ÆèÌšûÿ‹ÕèÍ³úÿ…Àt‹E$‹ ‹×èuŞüÿ…Àt‹Æèªšûÿ‹ÓèO³úÿ‹Ø…ÛuÀ‹ÃƒÄ(]_^[Ã@ SVW‹ù‹ò‹Ø‹Ï‹Ö‹Ãè|ÿÿÿ…Àu…öt±‹Ö‹Ãè]ıÿÿj j‹Ï‹Ö‹Ãèrşÿÿ_^[Ã‹À3ÀÃU‹ìS‹Ø‹E‹@üè?šûÿ‹Óèğ²úÿ‹Ø…Ût#‹EP‹Ãè×ÿÿÿY‹E‹@üèšûÿ¹   ‹Óè[²úÿ[]ÃU‹ìQS‹Ú‰Eü…Ût	U‹Ãè¦ÿÿÿY‹Eüèí™ûÿ‹Óèâ²úÿ[Y]Ã‹ÀSVƒÄØ‹ò‹Ø‹ÃèĞ™ûÿ¹   ‹Öè²úÿÇ$D   ‰t$‹Ãè²™ûÿ‹Ôè³²úÿ…Àt‹D$$€x uƒ|$  t3Àë°ƒÄ(^[Ã‹ÀU‹ìƒÄğSVW3Û‰]ôˆMû‹Ú‰Eü3ÀUhäèE dÿ0d‰ 3Ò‹EüèNÿÿÿ‹Eü‹ÿ’ˆ   ‰Eğ‹ÃèOúÿ‹ğë}ƒ}ğ t‹Uğ‹EüèPÿÿÿ„Àtq‹ŞëSèfuúÿ‹ØŠ„Àt,/t,-uê‹Ë+ÎEô‹ÖèlLúÿ€}û tƒ}ğ t‹Mô‹Uğ‹EüèZşÿÿë‹Mô‹Uğ‹EüèÚıÿÿ…Àt‰Eğ‹óëFŠ,/tù,-tõ€> …zÿÿÿƒ}ğ t‹Uğ‹Eüèşÿÿ3ÀZYYd‰hëèE EôèKúÿÃé{Gúÿëğ_^[‹å]Ã‹À3ÉèõşÿÿÃU‹ìƒÄìSVW3É‰Mô‰Mø‰Uü‹Ø3ÀUh êE dÿ0d‰ ‹ÃèQ˜ûÿè±úÿ‹ø…ÿ„¯   ‹Ï3Ò‹Ãè‹ûÿÿ‹×‹ÃèNşÿÿ„À„“   3öFƒşuEø‹Uüè/Kúÿë(EôP‰uìÆEğ Uì3É¸êE èúÿ‹MôEø‹Uüè1Lúÿ‹Mø‹×‹Ãèİüÿÿ…Àuµ±‹×‹ÃèÂúÿÿj j‹Mø‹×‹ÃèÖûÿÿ‹ğ‹Ö‹Ãè§ıÿÿ‹Ãè—ûÿ„Àt‹Ã‹ÿRx‹Ãèš—ûÿ‹ÖèË°úÿ3ÀZYYd‰hêE Eôº   è!JúÿÃé_Fúÿëë_^[‹å]Ã  ÿÿÿÿ    (%d)   U‹ìƒÄğSVW3À‰Eü‰Eø3ÀUhêêE dÿ0d‰ èønúÿ‹ğ³A÷Æ   u‹Ãèíÿÿ„ÀthEø‹Óè«JúÿEøº ëE è
Kúÿ‹EøUüèûëÿÿ‹EüPj Eô‹ÓˆPÆ UôEğèß=úÿºëE Eğ±è =úÿUğEøèmJúÿ‹Mø‹E‹@ü3ÒèÍúÿÿÑîC€û[…yÿÿÿ3ÀZYYd‰hñêE Eøº   è7IúÿÃéuEúÿëë_^[‹å]Ãÿÿÿÿ   :\  :  U‹ìÄ¨şÿÿSVW3É‰¬şÿÿ‰¨şÿÿ‰Mğ‹Ú‹ğ3ÀUhÅìE dÿ0d‰ jènoúÿ‰Eø3ÀUh˜ìE dÿ0d‰ ‹ÃèJúÿƒøu9‹Ãèîÿÿ„ÀuÆEÿ èMFúÿé+  •¬şÿÿ‹Ãè÷êÿÿ‹¬şÿÿ‹E‹@ü‹Öèl  ÆEÿ…°şÿÿP•¬şÿÿ‹Ãèè×üÿ…¬şÿÿºàìE èÀIúÿ‹…¬şÿÿèqKúÿPèŸlúÿ‰Eôƒ}ôÿ„§   3ÀUhzìE dÿ0d‰ …°şÿÿèEéÿÿ„ÀtXEğ•Üşÿÿ¹  èRIúÿ•¨şÿÿ‹Ãè}×üÿ…¨şÿÿ‹UğèWIúÿ‹…¨şÿÿ•¬şÿÿèBêÿÿ‹…¬şÿÿPj ‹E‹@ü‹Mğ‹ÖèAùÿÿ…°şÿÿP‹EôPèlúÿ…Àu…3ÀZYYd‰hìE ‹EôPèëkúÿÃéåCúÿëï3ÀZYYd‰hŸìE ‹EøPènúÿÃéÇCúÿëï3ÀZYYd‰hÌìE …¨şÿÿº   èdGúÿEğè<GúÿÃéšCúÿëàŠEÿ_^[‹å]Ã  ÿÿÿÿ   *   U‹ìƒÄøSVW3É‰Mø‹ò‰Eü3ÀUhZíE dÿ0d‰ …öuUèıÿÿY³ë0UMø‹Ö‹Eü‹ÿS|‹Uø‹ÆèÜıÿÿY‹Ø„Ût‹Eüè=”ûÿ3É‹Öè°­úÿ3ÀZYYd‰haíE Eøè§FúÿÃéCúÿëğ‹Ã_^[YY]Ã‹ÀSVW‹Ù‹ú‹ğ‹Ë‹×‹Æèèğÿÿ‹èíGúÿƒøu‹€x:u‹Ãº¬íE èÜGúÿ_^[Ãÿÿÿÿ   \   U‹ìƒÄôSVW3Û‰]ôˆMû‰Uü‹ğ‹]3ÀUhîE dÿ0d‰ €}û t‹Ãè0éÿÿ‹ØëMô‹Uü‹Æ‹0ÿV|‹Eô‹ÓèÉèÿÿ‹Ø3ÀZYYd‰hîE EôèêEúÿÃéHBúÿëğ‹Ã_^[‹å]Â @ U‹ì3ÉQQQQQSVW‹ò‹Ø3ÀUhîîE dÿ0d‰ Mø‹Ö‹Ã‹ÿS|jèNlúÿ‰Eô3ÀUhÄîE dÿ0d‰ Uì‹Eøè:Ùüÿ‹EìUğèÕüÿ‹Eğè«HúÿPèajúÿƒøt‹Eøèäæÿÿ„Àu3Àë°ˆEÿ3ÀZYYd‰hËîE ‹EôPèékúÿÃé›Aúÿëï3ÀZYYd‰hõîE Eìº   è;EúÿEøèEúÿÃéqAúÿëãŠEÿ_^[‹å]ÃSVƒÄØ‹ñ‹ØÇ$"   ‰T$ÇD$ÿÿÿÿÇD$ÿÿÿÿ…ötƒ$‹ÆèHúÿ‰D$‹Ãè:’ûÿ‹ÔèS«úÿƒÄ(^[ÃS‹Ú‹Óènìÿÿƒcû[ÃU‹ì3ÒŠU‹„(  ]Â S‹Ø‹Ãèş‘ûÿèÍªúÿ[Ã@ U‹ìÄ¨şÿÿSVW3Û‰¬şÿÿ‰¨şÿÿ‰]ü‰]ğ‹ù‹Ú‹ğ3ÀUhkñE dÿ0d‰ …ÿt;•¬şÿÿ‹ÇèsØüÿ‹…¬şÿÿP•¨şÿÿ‹Ãè·Óüÿ‹…¨şÿÿZèOÕüÿ…Àu
Uü‹ÇèØüÿjè®júÿ‰Eø3ÀUh6ñE dÿ0d‰ …°şÿÿP•¬şÿÿ‹ÃèqÓüÿ…¬şÿÿº„ñE èIEúÿ‹…¬şÿÿèúFúÿPè(húÿ‰Eôƒ}ôÿ„Î   3ÀUhñE dÿ0d‰ …°şÿÿèÎäÿÿ„Àt{Eğ•Üşÿÿ¹  èÛDúÿ‹Uü‹Eğè¬Ôüÿ…ÀtY‹E‹@ü‹Mğ‹ÖèØõÿÿ…ÀuE•¨şÿÿ‹ÃèãÒüÿ…¨şÿÿ‹Uğè½Dúÿ‹…¨şÿÿ•¬şÿÿè¨åÿÿ‹…¬şÿÿPj ‹E‹@ü‹Mğ‹Öè§ôÿÿ…°şÿÿP‹EôPè{gúÿ…À…^ÿÿÿ3ÀZYYd‰hñE ‹EôPèMgúÿÃéG?úÿëï3ÀZYYd‰h=ñE ‹EøPèwiúÿÃé)?úÿëï3ÀZYYd‰hrñE …¨şÿÿº   èÆBúÿEğèBúÿEüè–BúÿÃéô>úÿëØ_^[‹å]Ã   ÿÿÿÿ   *   U‹ì3ÉQQQQQQSVW‹Ú‰Eü3ÀUhóE dÿ0d‰ ÆEû…Û…™   Eô‹Uü‹’  èÎBúÿƒ}ô uEô‹Uü‹’  è·Búÿƒ}ô „ÿ   3Ò‹Eôè×äÿÿ‹Ø‹Eü‰˜(  ‹Eü‰˜,  Uğ‹Eôèhäÿÿƒ}ğ uUğ‹EôèóÕüÿ‹EğPj 3É3Ò‹Eüè]óÿÿ‹Ø‹Eüè;ûÿ¹   ‹Óè{§úÿé˜   Mì‹Ó‹Eü‹0ÿV|‹Eüƒ¸   t1UUè‹Eü‹€  èÑüÿEè‹UìèøBúÿ‹Uè‹Eü‹ˆ$  ‹ÃèíüÿÿY‹Eüƒ¸   t1UUè‹Eü‹€  èŞĞüÿEè‹Uìè»Búÿ‹Uè‹Eü‹ˆ   ‹Ãè°üÿÿY‹Eüè›ûÿ3É‹Óè¨úÿ3ÀZYYd‰hóE Eèº   è AúÿÃé^=úÿëëŠEû_^[‹å]Ã‹ÀU‹ìj j SVW‹ò‹Ø3ÀUh½óE dÿ0d‰ Mü‹Ö‹Ã‹0ÿV|ƒ»   t)Uø‹ƒ  èAĞüÿEø‹UüèBúÿ‹Eøèâÿÿ„Àt³ë4ƒ»   t)Uø‹ƒ  èĞüÿEø‹UüèìAúÿ‹Eøèìáÿÿ„Àt³ë3Û3ÀZYYd‰hÄóE Eøº   èd@úÿÃé¢<úÿëë‹Ã_^[YY]Ã@ U‹ìSVW‹ù‹ò‹Øƒ  ‹Öèg@úÿƒ  ‹×èZ@úÿƒ   ‹UèL@úÿƒ$  ‹Uè>@úÿ‹Ãèsrûÿ_^[]Â è·µşÿh@ôE hXôE èàeúÿPè*eúÿ£LI ÃSHPathPrepareForWriteA  shell32.dll         ,õE         õE õE   ¨0A ÜjA à*@ +@ ĞşE H¸@ |SA d·@ ÔA xDA ÈZA ¬HA ğA èA xşE ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA ,ÿE èdA ¨cA „eA ,eA ìlA P F A  ÿÿ¬ÿE TNewNotebook@ TNewNotebook˜ôE Ø2A ! NewNotebook tùE ü  ÿ„ F       €   €	 
ActivePage+A ;  ÿ¤DA       €    
 AlignøA H  ÿÄKA àKA    €  € Color¸)A N  ÿN  ÿ      €ôÿÿÿ 
DragCursorT+A <  ÿ8  ş      €     DragMode @ 8  ÿJA       €    Enabled€“A D  ÿLKA XKA    €   € Font @ :  ÿèKA       €    ParentColor @ 9  ÿhKA       €    
ParentFont @ ^  ÿ¤KA       €    ParentShowHintŒA P  ÿLJA       €   € 	PopupMenu @ ]  ÿˆKA `KA    €   € ShowHint„+A ‚A ˜‚A       €ÿÿÿÿ TabOrder @ Ä  ÿ°‚A       €     TabStop @ 7  ÿÜIA       €    Visible$-A x  ÿx  ÿ      €   € 
OnDragDrop¸,A €  ÿ€  ÿ      €   € 
OnDragOver¨-A   ÿ  ÿ      €   € 	OnEndDragì @ ì  ÿì  ÿ      €   € OnEnterì @ ô  ÿô  ÿ      €   € OnExit¤+A `  ÿ`  ÿ      €   € OnMouseDown,A h  ÿh  ÿ      €   € OnMouseMove¤+A p  ÿp  ÿ      €   € 	OnMouseUpl-A ˆ  ÿˆ  ÿ      €   €  OnStartDrag@         tùE             `ùE    4A ÜjA à*@ +@ <ıE H¸@ |SA d·@ ÔA xDA ÔıE ¬HA ğA èA èüE ŒA ¬A  €A @CA ôCA ”IA ´iA T€A œ€A h|A €€A ¨^A ÜeA  bA èdA ¨cA „eA ,eA tŠA ¸~A A ˆıE TNewNotebookPage@ TNewNotebookPageàøE ¸4A  NewNotebook øA H  ÿÄKA àKA    €   €	 ColorT+A <  ÿ8  ş      €    
 DragMode @ 8  ÿJA       €    Enabled€“A D  ÿLKA XKA    €   € Font4@ 0  ÿÔEA        €   € Height4@ $  ÿtEA        €   € Left4@ lıE 8şE        €   € 	PageIndex @ :  ÿèKA       €    ParentColor @ 9  ÿhKA       €    
ParentFont @ ^  ÿ¤KA       €    ParentShowHintŒA P  ÿLJA       €   € 	PopupMenu @ ]  ÿˆKA `KA    €   € ShowHint4@ (  ÿ”EA        €   € Top @ 7  ÿÜIA        €    Visible4@ ,  ÿ´EA        €   € Width$-A x  ÿx  ÿ      €   € 
OnDragDrop¸,A €  ÿ€  ÿ      €   € 
OnDragOver¨-A   ÿ  ÿ      €   € 	OnEndDragì @ ì  ÿì  ÿ      €   € OnEnterì @ ô  ÿô  ÿ      €   € OnExit¤+A `  ÿ`  ÿ      €   € OnMouseDown,A h  ÿh  ÿ      €   € OnMouseMove¤+A p  ÿp  ÿ      €   € 	OnMouseUpl-A ˆ  ÿˆ  ÿ      €   € OnStartDragSV„ÒtƒÄğè:0úÿ‹Ú‹ğ3Ò‹ÆèéŒûÿ²‹Æè˜Gûÿf¡8ıE fF4f‰F43Ò‹Æè¹Lûÿ„Ût
d    ƒÄ‹Æ^[Ã    SV‹Ú‹ğ‹†   …Àt‹ÖèÍ  3Ò‹ÆèäŒûÿ„Ût‹Æè0úÿ‹Æ^[Ã‹   …Òt‹’   ’èš¶úÿÃƒÈÿÃS‹Ø‹Ãè6ûÿöCt7‹ƒü   ‹@²èx§ûÿ‹ƒü   ‹@²èD©ûÿ‹C,P‹C0P3É3Ò‹ƒü   è©¯ûÿ[Ã@ SVW‹ò‹Ø‹Ö‹Ãèä\ûÿ‹~(‹Çº˜ôE è­-úÿ„Àt	‹×‹Ãè   _^[ÃSV‹ò‹Ø‹ƒ   ;ğt!…Àt‹Óè  ‹Ö‹Ã‹ÿQ<…öt	‹Ó‹ÆèÏ  ^[ÃSVW‹ò‹Ø‹ƒ   …Àt(‹¸   ‹W;ò|‹òN…ö}3ö‹Ãèÿÿÿ‹Ğ‹Ç‹Îè+¶úÿ_^[Ã@ SV„ÒtƒÄğèª.úÿ‹Ú‹ğ3Ò‹ÆèQZûÿº–   ‹ÆèGûÿº–   ‹Æè)Gûÿ²¸\¤@ èy,úÿ‰†   „Ût
d    ƒÄ‹Æ^[ÃSVWU‹Ú‹è‹…   …Àt/‹pN…ö|F3ÿ‹×‹…   èÚ´úÿ3Ò‰   GNuç‹…   èG,úÿ3Ò‹ÅèBZûÿ„Ût‹ÅèG.úÿ‹Å]_^[ÃS‹Ú‹ÓèjcûÿK   [ÃSV‹Ù‹ğ‹†   ƒx ~UèÅ´úÿ‹Ğƒúÿu„Ût‹†   ‹PJë3Ò‹Â„Ût@‹–   ;Bu3Àë…Àu	‹†   ‹@H‹Ğ‹†   è3´úÿë;ĞuË3À^[ÃU‹ìQSV‰Eü‹Eü‹€   ‹XK…Û|C3ö‹Ö‹Eü‹€   èû³úÿ‹Ğ‹EÿUFKuä^[Y]Â ‹€   èİ³úÿÃ‹€   ‹@Ã‹ÀSV‹ò‹Ø‹Ö‹ƒ   è!³úÿ‰   ^[ÃSV‹ò‹Ø3À‰†   ‹Ö‹ƒ   èÉ´úÿ;³ü   u	3Ò‹Ãè8   ^[ÃSV‹ò‹Ø‹ÆºàøE è>+úÿ„Àt;   u	‹Ö‹Ãè   ‹Ö‹Ãè8~ûÿ^[ÃSVW‹ò‹Ø…öt;   …Ï   ;³ü   „Ã   ‹Ãè^õûÿ‹ø…ÿt+ƒ»ü    t"‹—  ‹ƒü   è_ûÿ„Àt‹“ü   ‹ÇèÕüÿ…ötJ‹ÆèrKûÿ²‹ÆèéHûÿ…ÿt6ƒ»ü    t-‹‡  ;ƒü   u‹ÆèÓûÿ„Àt‹Ö‹Çè’üÿë	‹Ó‹Çè‡üÿ‹ƒü   …Àt3ÒèHûÿ‰³ü   …ÿt"ƒ»ü    t‹‡  ;ƒü   u‹ƒü   èæƒûÿ_^[Ã‹À     F äF 2F ‘F     ÃF Ô  tÂH PB à*@ +@ ŒB H¸@ B d·@ ÔA ¨B 0B ¬HA äB èA dF ŒA €	B  €A @CA ôCA ¬B ˆÊH T€A œ€A h|A €€A ğB ÜeA ,B DB B B ,eA \B ¸~A ´-B         |@ Ì   ÕF ´    BrowseLabel¸   PathEdit¼   NewFolderButtonÀ   OKButtonÄ   CancelButton   F PathEditChange L	F NewFolderButtonClickTSelectFolderForm ¡D VB ØmB TSelectFolderForm¤F ÃH 8 SelFolderForm  @ U‹ìj j j j j j j SVW‹ñˆUÿ‹Ø‹}3ÀUhQF dÿ0d‰ S3É²¸¤F è  ‰Eø3ÀUh'F dÿ0d‰ ŠEÿ‹Uøˆ‚Ñ  ‹EøÌ  ‹×èÕ0úÿ€}ÿ u…ÿu3Òë²‹Eø‹€¼  èCFûÿ„Û„¬   ‹Eø‹€È  º€ÓE è(úÿ‹ø€=k"I  tUEôP3É²3Àè"X ‹EôPEğP3É²°èX ‹EğPEìP3É²°èüW ‹EìPEèP3É²3ÀèéW ‹Uè‹ÇYè¾ïÿÿë0EôP3É²3ÀèÍW ‹EôPj EğP3É²3Àè¸W ‹Uğ3É‹ÇèŒïÿÿ‹Æè1  ë‹Æèœ  €}ÿ t5Uô‹è(Ãüÿ‹Eô‹è2úÿt1Uô‹èÃüÿ‹Uô±‹Eø‹€È  ècãÿÿë‹±‹Eø‹€È  èOãÿÿ„Ûu‹Eø‹€È  ƒ¸    t‹Eø‹È  ‹Eøèìüÿ‹‹Eø‹€¸  èØEûÿ‹Eøè)üÿH”Eş€}ş t&Uä‹Eø‹€¸  è„Eûÿ‹EäUôè9fúÿ‹Uô‹Æè?/úÿ3ÀZYYd‰h.F ‹Eøè2&úÿÃé8+úÿëğ3ÀZYYd‰hXF Eäè½.úÿEèº   èĞ.úÿÃé+úÿëãŠEş_^[‹å]Â U‹ìƒÄğSVW„ÒtƒÄğè·'úÿˆUÿ‹ğ3Ò‹ÆèİÁ €¾Ğ   u=‹Î²¸ĞÑE ènÕÿÿ‹Ø‰È  ‰³  Çƒ  ”F ¡PI ‰ƒ  Çƒ  ŒlF ë;‹Î²¸€ÓE è1Õÿÿ‹Ø‰È  ‰³  Çƒ  ”F ¡PI ‰ƒ  Çƒ  dmF h=  hå   ¹@   º   ‹†È  ‹ÿSL3Ò‹†È  è¨Cûÿ‹Ö‹†È  ‹ÿQ<‹†¸  èFûÿ‹†´  èFûÿfº ‹†È  è2|ûÿ²‹†È  èiCûÿ‹ÆèÃ ‹˜I ‹Æè)Dûÿ‹”I ‹†´  èDûÿ‹–´  ¡PI èè  ‹Ø‹†¸  ‹P(ÓèÚ>ûÿ‹†È  ‹P(ÓèÊ>ûÿ‹°I ‹†¼  èÕCûÿ‹†¼  ‹P(Óè©>ûÿÆEôUô3É‹Æè©À ‹Ğ‹†¼  è¬>ûÿÆEğÆEñUğ¹   ‹Æè…À ‰Eø‹¤I ‹†Ä  è}Cûÿ‹EøP‹¾Ä  ‹G0P‹W$W,+Uø‹O(Ë‹Ç‹8ÿWL‹ÀI ‹†À  èICûÿ‹EøP‹¾À  ‹G0Pº   ‹ÆèÓÂ ‹–Ä  ‹R$+Ğ+Uø‹O(Ë‹†À  ‹8ÿWL‹Æèò>ûÿ‹ĞÓ‹Æè{üÿ3É‹PI ‹Æè”À €}ÿ t
d    ƒÄ‹Æ_^[‹å]Ã@ U‹ìQSV„ÒtƒÄğèB%úÿˆUÿ‹ØŠEˆƒĞ  3Ò‹Ã‹0ÿV$€}ÿ t
d    ƒÄ‹Ã^[Y]Â ‹ÀU‹ìj j SVW‹Ø3ÀUhƒF dÿ0d‰ Uø‹ƒ¸  è8Bûÿ‹EøUüèíbúÿƒ}ü •Â‹ƒÀ  è³Aûÿ3ÀZYYd‰hŠF Eøè†+úÿEüè~+úÿÃéÜ'úÿëè_^[YY]Ã@ U‹ìj SVW‹Ø3ÀUh=	F dÿ0d‰ €»Ñ   t2Uü‹ƒÈ  ‹€   èËºüÿEü‹“Ì  è¥,úÿ‹Uü‹ƒ¸  èËAûÿë‹ƒÈ  ‹   ‹ƒ¸  è²Aûÿ€»Ğ   u‹ƒÈ  ƒ¸    u3Òë²‹ƒ¼  èñ@ûÿ3ÀZYYd‰hD	F EüèÄ*úÿÃé"'úÿëğ_^[Y]Ã‹À‹Ì  ‹€È  èŸßÿÿÃ‹À    Ğ	F ø	F             ê	F d   Øª@ ,,@ à*@ +@ ÜF H¸@ A d·@ ÔA <A àA ¸A ğA èA ˜F F ,F HF `F |F         |@ ,   |@ 0   TWizardPage‹ÀTWizardPage”	F X«@  Wizard      È
F 8F F ¨F àF èF 8  tÂH PB à*@ +@ Ì7F H¸@ B d·@ ÔA ¨B 0B ¬HA äB èA ¨ F ŒA €	B  €A @CA ôCA ¬B ˆÊH T€A œ€A h|A €€A ğB ÜeA 08F DB B B ,eA \B ¸~A ´-B         |@ ô  |@ ø  |@ ü  |@    |@   |@   |@   |@   N ôF ´    CancelButton¸    
NextButton¼    
BackButtonÀ   OuterNotebookÄ   InnerNotebookÈ   WelcomePageÌ   	InnerPageĞ   FinishedPageÔ   LicensePageØ   PasswordPageÜ   InfoBeforePageà   UserInfoPageä   SelectDirPageè   SelectComponentsPageì   SelectProgramGroupPageğ   SelectTasksPageô   	ReadyPageø   PreparingPageü   InstallingPage    InfoAfterPage   DiskSpaceLabel   DirEdit   	GroupEdit   NoIconsCheck   PasswordLabel   PasswordEdit   PasswordEditLabel    	ReadyMemo$   
TypesCombo(  	 Bevel,  
 WizardBitmapImage0   WelcomeLabel14   InfoBeforeMemo8   InfoBeforeClickLabel<   	MainPanel@  	 Bevel1D   PageNameLabelH   PageDescriptionLabelL  
 WizardSmallBitmapImageP   
ReadyLabelT   FinishedLabelX   YesRadio\   NoRadio`  
 WizardBitmapImage2d   WelcomeLabel2h   LicenseLabel1l   LicenseMemop   InfoAfterMemot   InfoAfterClickLabelx   ComponentsList|   ComponentsDiskSpaceLabel€   BeveledLabel„   StatusLabelˆ   FilenameLabelŒ   ProgressGauge   SelectDirLabel”   SelectStartMenuFolderLabel˜   SelectComponentsLabelœ   SelectTasksLabel    LicenseAcceptedRadio¤   LicenseNotAcceptedRadio¨   UserInfoNameLabel¬   UserInfoNameEdit°   UserInfoOrgLabel´   UserInfoOrgEdit¸  
 PreparingErrorBitmapImage¼   PreparingLabelÀ   FinishedHeadingLabelÄ   UserInfoSerialLabelÈ   UserInfoSerialEditÌ   	TasksListĞ   RunListÔ    DirBrowseButtonØ    GroupBrowseButtonÜ  
 SelectDirBitmapImageà  
 SelectGroupBitmapImageä   SelectDirBrowseLabelè    SelectStartMenuFolderBrowseLabel  \`F NextButtonClick <cF BackButtonClick ĞcF CancelButtonClick hdF 	FormClose ĞeF NoIconsCheckClick |dF TypesComboChange eF ComponentsListClickCheck  `fF LicenseAcceptedRadioClick# xfF LicenseNotAcceptedRadioClick fF UserInfoEditChange ,kF DirBrowseButtonClick àkF GroupBrowseButtonClick $fF TWizardForm ØmB ˜ôE àøE ¡D VB tB 4ÉE °]B äfB ŒïB D¼E ğD xòB yB À±D TíD ‹ÀTWizardFormL
F ÃH 8 Wizard  SƒÄì‹Úß(Ø´F Û<$›Û,$èŠúÿØ¸F ßàvÛ,$èOúÿØ¼F Û<$›SD$‰D$ÆD$T$3É¸ÈF è×dúÿƒÄ[Ã    €:      €?ÿÿÿÿ   %.0n    SƒÄì‹Úß(Ø@F ØDF Û<$›Û,$èúÿØHF ßàvÛ,$èÑúÿØLF Û<$›Û,$Û-PF ŞÉÛ<$›SD$‰D$ÆD$T$3É¸dF èJdúÿƒÄ[Ã   €5   A      €?ÍÌÌÌÌÌÌÌû?  ÿÿÿÿ   %.1n    U‹ìj SVW‹ù‹ò‹Ø3ÀUhF dÿ0d‰ ‹Ç3ÒŠÓ‹•tI èúúÿ‹Ç‹˜"I ºF èLÅüÿ‹Ç‹œ"I º,F è:ÅüÿUü‹Æèşÿÿ‹Mü‹Çº@F è!ÅüÿUü‹Æèïşÿÿ‹Mü‹ÇºPF èÅüÿ3ÀZYYd‰hF EüèûúÿÃéYúÿëğ_^[Y]Ã ÿÿÿÿ   [name]  ÿÿÿÿ
   [name/ver]  ÿÿÿÿ   [kb]    ÿÿÿÿ   [mb]    SV‹ò‹Ø‹Îºˆ"I ‹Ãè ÿÿÿ^[ÃU‹ìƒÄôSVW3É‰Mô‰Uø‰Eü3ÀUhàF dÿ0d‰ ‹Eü‹ÿR‹ØK…Û|'C3öMô‹Ö‹Eü‹8ÿW‹Eô‹Uøè`Túÿ…Àu³ëFKuÜ3Û3ÀZYYd‰hçF Eôè!úÿÃéúÿëğ‹Ã_^[‹å]ÃU‹ìj j SVW‹Ø3ÀUhnF dÿ0d‰ Uü‹è,Uúÿ‹EüUøèu³üÿ‹EøUüèj´üÿ‹Uü‹ÃèúÿUü‹èUúÿ‹Eüè.úÿ‹ğ‹è%úÿ;ğu·3ÀZYYd‰huF Eøº   è³úÿÃéñúÿëë_^[YY]ÃU‹ìj j SVW‹Ø3ÀUh'F dÿ0d‰ Uü‹è Túÿ‹Uü‹Ãè¦úÿë‹Ã¹   º   èû úÿƒ; t‹Š ,/tâ,-tŞUø‹è¼²üÿ‹EøUüè]³üÿ‹Uü‹ÃècúÿUü‹èITúÿ‹Eüèuúÿ‹ğ‹èlúÿ;ğuŠ3ÀZYYd‰h.F Eøº   èúúÿÃé8úÿëë_^[YY]Ã@ S‹Ø‹Ãè2úÿ…À~º   €|ÿw°[ÃBHuñ3À[Ã‹ÀS‹Ø‹ÃèÎúÿë€8 uŠP€ú\t„Òu°[ÃPè5Fúÿ€8 uà3À[ÃS‹Ø‹ÃèúÿëFë@€8 tú3Òë²@€8.tøë@€8 tú„ÒtŠ€ù\t„Éu
°[ÃPèëEúÿŠ„Òt€ú\uï€8\u@€8 u¸3À[ÃU‹ìƒÄøSVW3É‰Mø‹Ú‰Eü3ÀUhuF dÿ0d‰ ‹Ãèìúÿ‹EüèXúÿ‹ø…ÿ~=¾   ‹Eü€|0ÿ t*ƒ; t‹ÃºŒF è:úÿEø‹UüŠT2ÿè¿úÿ‹Uø‹Ãè!úÿFOuÈ3ÀZYYd‰h|F EøèŒúÿÃéêúÿëğ_^[YY]Ã ÿÿÿÿ       SVW‹Ú‹s,‹{0²‹Ãèpşÿ3Ò‹Ãègşÿ‹Ö‹Ãèş,ûÿ‹C0+Ç_^[ÃSV‹ò‹F,P‹F0+ÁPN(‹V$‹Æ‹ÿSL^[Ã‹ÀU‹ìƒÄôSVW3Ò‰Uü‹Ø3ÀUhµF dÿ0d‰ €=q"I  „’   ƒ=´"I  „…   Uü‹ƒ¬  èZ1ûÿ‹Uü¸4"I èúÿUü‹ƒ´  è?1ûÿ‹Uü¸8"I èúÿUü‹ƒÈ  è$1ûÿ‹Uü¸<"I èçúÿj jj Uü‹ƒÈ  è1ûÿ‹Eü‰EôÆEøMôºĞF ¡´"I è›¤ ‹Øë³3ÀZYYd‰h¼F EüèLúÿÃéªúÿëğ‹Ã_^[‹å]Ã   ÿÿÿÿ   CheckSerial U‹ìƒÄìSVW3Ò‰Uì‰Eü3ÀUh"F dÿ0d‰ ‹|!I ‰TI ‹€!I ‰XI ²¸Ğ¦@ èúÿ‹øj 3É‹×‹Eüèí-  ¡ü!I ‹XK…Û|fC3ö‹Ö¡ü!I è‰™úÿ‰Eøj‹Mø3Ò‹Çèˆ	 „Àt>‹Eø‰Eô‹Eôƒx8ÿt‹Eô‹P8¡ "I èU™úÿP¸TI èèèüÿë‹EôP@¸TI èÖèüÿFKu¡ğ!I ‹XK…Û|6C3ö‹Ö¡ğ!I è™úÿ‰Eğ‹Eğ‹‹Çè£úÿÿ„Àt‹EğP¸TI è“èüÿFKuÍ‹ÇèlúÿMìºTI °èqùÿÿ‹Uì‹Eü‹€|  è¤/ûÿ3ÀZYYd‰h)F EìèßúÿÃé=úÿëğ_^[‹å]ÃU‹ìƒÄäSVW3Û‰]è‰]äˆMÿ‹ò‹Ø3ÀUhQF dÿ0d‰ ‹Ö‹ƒx  èÁ¸şÿ‹ø3À‰Eğ3À‰Eì€}ÿ tSh0F Mì‹Ö‹ƒx  èµ³şÿ‹G6‰Eô‹G:‰EøUìEôèÒçüÿ‹Ö‹ƒx  è!¸şÿ„ÀtUô‹Eè¶çüÿƒ}ô uƒ}ø t]€»0   u*UäEôèz÷ÿÿ‹UäMè°èqéşÿ‹Mè‹Ö‹ƒx  è1Âşÿë9UäEôèÈ÷ÿÿ‹UäMè°èGéşÿ‹Mè‹Ö‹ƒx  èÂşÿë3É‹Ö‹ƒx  èöÁşÿ3ÀZYYd‰hXF Eäº   èĞúÿÃéúÿëë_^[‹å]Â @ ƒÄøö›!I t#3Ò‰T$3Ò‰$Ph0F L$ƒÊÿ‹€x  è­²şÿYZÃ‹ÀSVW„ÒtƒÄğè‰úÿ‹ñ‹Ú‹ø‹Î3Ò‹Çè åúÿ‹ÆºL
F èôúÿ‰G`„Ût
d    ƒÄ‹Ç_^[ÃSV‹Ú‹ğ‹F`…Àtƒ¸ì   t‹€ì  ‹Öè˜úÿ3Ò‹Æèôäúÿ„Ût‹ÆèYúÿ‹Æ^[Ãfƒx6 t
‹È‹Ğ‹A8ÿQ4Ã‹ÀSV‹ò‹Øfƒ{> t
‹Ó‹C@ÿS<ˆ^[Ã‹ÀSfƒxF tQ‹Ê‹Ø‹Ğ‹CHÿSD[ÃSV‹ò‹Øfƒ{N t
‹Ó‹CPÿSLˆ^[Ã‹ÀSV‹ò‹Øfƒ{V t
‹Ó‹CXÿSTˆ^[Ã‹À‹H`‹‰Ì  ‹P$;Êu‹@(Ã‹ÂÃS‹Ø‹ÃèŞÿÿÿ‹@ ‹@0[Ã‹ÀS‹Ø‹ÃèÊÿÿÿ‹@ ‹@,[Ã‹ÀSV‹ò‹ØC,‹Öèhúÿ‹Ãè!   ^[Ã‹ÀSV‹ò‹ØC0‹ÖèLúÿ‹Ãè   ^[Ã‹ÀS‹Ø‹C`‹4  ;S u‹€D  ‹S,è,ûÿ‹C`‹€H  ‹S0èp,ûÿ[Ã‹ÀU‹ìƒÄìS‰Eüƒ}ü „§   3ÀUhøF dÿ0d‰ ‹š´   º    ‹Ã‹ÿQ,º    ‹Ã‹ÿQ(‹Ãèİ±ûÿ‹@º  €è¬‡ûÿj EìP¹    3Ò3Àè$úÿEìP‹Ãè±±ûÿZè;ûÿjj j j j ‹EüPj j ‹Ãè’±ûÿèQ‘ûÿPè‹?úÿ3ÀZYYd‰hÿF ‹EüPè=?úÿÃégúÿëï[‹å]ÃU‹ìÄœşÿÿSVW3À‰Eü3ÀUh‰ F dÿ0d‰ ‹E‹@ü‹€Ü  º    è{&ûÿ‹E‹@ü‹€Ü  º    è…&ûÿ‹E‹@ü‹€à  º    èO&ûÿ‹E‹@ü‹€à  º    èY&ûÿ3ÀUhi F dÿ0d‰ h  h`  …œşÿÿPjh˜ F èÂ«üÿ…Àt3€½¨şÿÿ t*‹… şÿÿP…¨şÿÿP¡I Pè”«üÿ‹U‹Rü‹’Ü  ègşÿÿEüP3É²3Àè< ƒ}ü uEüP3É²°èï; ƒ}ü tXh   h`  …œşÿÿPj ‹EüèúÿPè@«üÿ…Àt3€½¨şÿÿ t*‹… şÿÿP…¨şÿÿP¡I Pè«üÿ‹U‹Rü‹’à  èåıÿÿ3ÀZYYd‰ë
éúÿèúÿ3ÀZYYd‰h F EüèxúÿÃéÖúÿëğ_^[‹å]Ã c:\directory    U‹ìƒÄÀSVW3Û‰]Ğ‰]À‰]ì„ÒtƒÄğèhúÿ‰MôˆUû‰Eü]ü3ÀUh[7F dÿ0d‰ ‹Mô3Ò‹èv¦ ²¸\¤@ è6
úÿ‹‰‚ì  ²¸Ğ¦@ è"
úÿ‹‰‚(  ²¸Ğ¦@ è
úÿ‹‰‚  ²¸Ğ¦@ èú	úÿ‹‰‚  ²¸Ğ¦@ èæ	úÿ‹‰‚   ²¸Ğ¦@ èÒ	úÿ‹‰‚$  ¡$"I ‹ÿR‹‹’L  ‹r0+ğ…ö~;‹‹€L  ‹P0+Ö‹‹€L  è9$ûÿ‹‹€L  ‹P(ÑşyƒÖ Ö‹‹€L  èØ#ûÿ¡$"I ‹ÿR ‹‹’L  ‹r,+ğ…ö~;‹‹€L  ‹P,+Ö‹‹€L  èÃ#ûÿ‹‹€L  ‹P$ÑşyƒÖ Ö‹‹€L  èb#ûÿ‹è§ ö™!I t±‹ŒI ‹è#¦ ë‹è>§ j j‹‹€0  ‹@D‹Ø!I ‹°!I èº¡ ‹‹€0  ‹@DŠ|7F èø€ûÿ‹‹€D  ‹@DŠ|7F èâ€ûÿö™!I t‹ÄI ‹è(ûÿëMĞ‹˜"I °™è®âşÿ‹UĞ‹èø'ûÿö™!I uF‹èŒ#ûÿ‰EÜ‹èÆ#ûÿ‰EØ‹Š  
€7F ‹èFìûÿ²‹èiìûÿ‹UÜ‹èèûÿ‹UØ‹è%èûÿÆEÈ
ÆEÉÆEÊÆEËÆEÌUÈ¹   ‹èz¤ ‹ğº
   ‹è§ ‰Eğ‹‹€¼  ‹Öèj"ûÿ‹‹€¸  ‹Öè["ûÿ‹‹€´  ‹ÖèL"ûÿ‹èå"ûÿ‹ø+}ğ+ş‹‹€´  ‹×èï!ûÿ+}ğ+ş‹‹€¸  ‹×èÛ!ûÿ+ş‹‹€¼  ‹×èÊ!ûÿ‹‹€,  ‹`!I è#ÿÿ‹‹€,  ‹ "I è(ÿÿ‹‹€,  ²è%ÿÿöœ!I @•Â‹‹€,  èVÿÿ‹‹€`  ‹`!I è×ÿÿ‹‹€`  ‹ "I èÜÿÿ‹‹€`  ²èÙÿÿöœ!I @•Â‹‹€`  è
ÿÿ‹‹€L  ‹$"I è£ÿÿöœ!I @•Â‹‹€L  èàÿÿh„7F ¡I Pèà;úÿ‹Ğ‹‹€¸  ‹€´   è·±ûÿ‹‹€¸  ºÀÀÀ èyÿÿ‹‹PH‹‹€¸  èÿÿUèIúÿÿYj j j ‹‹ˆÈ  º   ‹èZ  UĞ°½èxğÿÿEĞº˜7F èúÿ‹UĞ‹‹€0  è³%ûÿ‹‹0  ‹è„óÿÿ‹‹€0  ‹H(H0‹‹d  +J(‹è”óÿÿUĞ°¾è"ğÿÿEĞº¤7F è9úÿEĞ‹ÜI è+úÿ‹UĞ‹‹€d  èO%ûÿ‹‹€Ô  P¡„I P¡I P‹‹ˆÌ  º   ‹è¦  UĞ°eèÄïÿÿ‹UĞ‹‹€h  è%ûÿ‹‹h  ‹èİòÿÿ‹ğ‹‹l  ‹Î‹èúòÿÿ‹‹€   ‹ I è×$ûÿ‹‹€¤  ‹I èÄ$ûÿ‹‹€Ø  P¡ˆI P¡@I P‹‹ˆÌ  º   ‹è  ‹‹€  ‹DI èˆ$ûÿ‹‹€  ‹<I èu$ûÿ‹‹  ‹èFòÿÿ‹ğ‹‹€  ‹P(Öè6ûÿ‹‹  ‹è#òÿÿğ‹‹€  ‹P(Öèûÿ‹‹€Ü  P¡|I P¡ØI P‹‹ˆÌ  º   ‹è†  ‹‹€8  ‹ÔI èó#ûÿ‹‹8  ‹èÄñÿÿ‹ğ‹‹4  ‹Î‹èáñÿÿ‹‹€à  P¡¨I P¡TI P‹‹ˆÌ  º   ‹è(  ‹‹€¨  ‹XI è•#ûÿ‹‹¨  ‹èfñÿÿ‹ğ‹‹€¬  ‹P(ÖèVûÿ‹‹€°  ‹P(ÖèDûÿ‹‹€°  ‹`I èM#ûÿ‹‹°  ‹èñÿÿğ‹‹€´  ‹P(Öèûÿ€=q"I  tZ‹‹€Ä  ‹P(Ö‹‹€Ä  èëûÿ‹‹€Ä  ‹dI èô"ûÿ‹‹Ä  ‹èÅğÿÿğ‹‹€È  ‹P(Ö‹‹€È  è­ûÿë‹‹€Ä  3Òèä!ûÿ‹‹€È  3ÒèÕ!ûÿ‹‹€ä  P¡”I PUĞ°‡è8íÿÿ‹EĞP‹‹ˆÌ  º   ‹èø  UĞ°‰èíÿÿ‹UĞ‹‹€  è^"ûÿº   ‹èö¡ ‹ø‹‹°Ü  ‹F$F,ø‹‹°  ‹Ç+F$‹V,+ĞR‹F0P‹N(‹Æ‹×‹0ÿVL‹‹  ‹èîïÿÿ‹‹€Ü  ‹@0‹‹’  ;B0~8‹‹€Ü  ‹P0‹‹€  ‹@0H+ĞÑúyƒÒ ‹‹€  P(‹‹€  è¢ûÿUĞ°†è\ìÿÿ‹UĞ‹‹€ä  è¤!ûÿº   ‹èL¡ ‹‹º  ‹W(W0JÂPº   ‹è.¡ ‹‹²Ü  ‹V(V0ÂZèü¼üÿ‹‹’ä  +B(‹ğ‹‹€ä  ‹P(Ö‹‹€ä  è!ûÿ‹‹ä  ‹èïÿÿğ‹‹€  ‹P(Ö‹‹€  èöûÿ‹‹€Ô  ‹ÄI èÿ ûÿÆEÄUÄ3É‹èã ‹øW‹‹€Ô  ‹@0P‹‹€Ô  ‹H(Î‹‹€Ä  ‹P,+×‹‹€Ô  ‹0ÿVLº
   ‹èQ  ‹‹’Ô  ‹R$+Ğ‹‹€  +P$‹‹€  èŒûÿUĞ°)è&ëÿÿ‹UĞ‹‹€  èn ûÿ‹‹°  ‹Ö‹è=îÿÿ‹V(+Ğ‹Æè5ûÿ‹‹€è  P¡˜I PUĞ°„èàêÿÿ‹EĞP‹‹ˆÌ  º   ‹è   UĞ°…è¾êÿÿ‹UĞ‹‹€˜  è ûÿ‹‹˜  ‹è×íÿÿ‹ğ‹‹€$  ‹P(Ö‹‹€$  è¿ûÿ‹‹x  ‹Î‹èÚíÿÿUĞ°èhêÿÿ‹UĞ‹‹€|  è°ûÿ‹‹|  ‹èíÿÿ€=w"I  t?¡ì!I ƒxu4‹‹€$  3Òèªûÿ‹‹€$  ‹H(‹‹€x  +H(‹‹x  ‹èiíÿÿ‹‹€ì  P¡œI PUĞ°èèéÿÿ‹EĞP‹‹ˆÌ  º   ‹è¨  UĞ°èÆéÿÿ‹UĞ‹‹€”  èûÿº   ‹è¦ ‹ø‹‹€à  ‹@$‹‹’à  B,ø‹‹°”  ‹Ç+F$‹V,+ĞR‹F0P‹N(‹Æ‹×‹0ÿVL‹‹”  ‹è–ìÿÿ‹‹€à  ‹@0‹‹’”  ;B0~8‹‹€à  ‹P0‹‹€”  ‹@0H+ĞÑúyƒÒ ‹‹€”  P(‹‹€”  èJûÿUĞ°èéÿÿ‹UĞ‹‹€è  èLûÿº   ‹èô ‹‹’”  ‹R(‹‹‰”  Q0JÂPº   ‹èÎ ‹‹’à  ‹R(‹‹‰à  Q0ÂZè”¹üÿ‹ğ‹‹€è  +p(‹‹€è  ‹P(Ö‹‹€è  è¹ûÿ‹‹è  ‹è¦ëÿÿğ‹‹€  ‹P(Ö‹‹€  èûÿ‹‹€Ø  ‹ÄI è—ûÿÆEÄUÄ3É‹è{š ‹øW‹‹€Ø  ‹@0P‹‹€Ø  ‹H(Î‹‹€Ä  ‹P,+×‹‹€Ø  ‹0ÿVLº
   ‹èéœ ‹‹’Ø  ‹R$+Ğ‹‹€  +P$‹‹€  è$ûÿ‹‹€  ‹I èûÿ‹‹€ğ  P¡ I PUĞ°èœçÿÿ‹EĞP‹‹ˆÌ  º	   ‹è\  UĞ°‘èzçÿÿ‹UĞ‹‹€œ  èÂûÿ‹‹œ  ‹è“êÿÿ‹ğ‹‹Ì  ‹Î‹è°êÿÿ‹‹€Ì  3ÒèIüÿº   ‹è9œ ‹‹’Ì  ‰‚T  öœ!I •Â‹‹€Ì  èÈ¯şÿ‹‹€ô  P¡I PUĞ°yèïæÿÿ‹EĞP‹‹ˆÌ  º
   ‹è¯
  ‹‹€ø  P¡ŒI PUĞ°wè¾æÿÿ‹EĞP‹‹ˆÌ  º   ‹è~
  ‹‹€ü  P¡€I PUĞ°[èæÿÿ‹EĞP‹‹ˆÌ  º   ‹èM
  ‹‹€   P¡xI P¡ĞI P‹‹ˆÌ  º   ‹è$
  ‹‹€t  ‹ÌI è‘ûÿ‹‹t  ‹èbéÿÿ‹ğ‹‹p  ‹Î‹èéÿÿj j j ‹‹ˆĞ  º   ‹èÕ	  j j‹‹€À  ‹@D‹Ø!I ‹°!I è™” ‹‹€À  ‹@DŠ|7F è×sûÿUĞ°Nè½åÿÿEĞPº˜7F XèÒúÿ‹UĞ‹‹€À  èöûÿ‹‹À  ‹èÇèÿÿ‹‹€À  ‹P(‹‹€À  P0‹‹€T  è¨ûÿ‹‹€X  ‹¬I è±ûÿ‹‹€\  ‹ I èûÿº   ‹èFš ‹‹’Ğ  ‰‚T  º´7F EĞè[úÿEĞ‹I è5úÿEĞº´7F è(úÿ‹UĞ‹‹€€  èLûÿƒ=("I  t"‹‹€l  ²èĞÂşÿ‹‹€l  ‹("I èñÃşÿƒ=,"I  t"‹‹€4  ²è¥Âşÿ‹‹€4  ‹,"I èÆÃşÿƒ=0"I  t"‹‹€p  ²èzÂşÿ‹‹€p  ‹0"I è›Ãşÿj ‹èŠPûÿPè¸.úÿ‹ğj j h   Vè,úÿ¡tI è)úÿPh'  j Vèg,úÿ‹è 	  ÇEèÿÿÿÿÆEç öœ!I „£   ‹ƒ¸   uYUĞ¡À I è8 ‹UĞ‹‹€¬  èLûÿUĞ¡Ä I è ‹UĞ‹‹€´  è/ûÿUĞ¡È I èş ‹UĞ‹‹€È  èûÿë?‹‹  ‹‹€¬  èûûÿ‹‹  ‹‹€´  èæûÿ‹‹  ‹‹€È  èÑûÿö˜!I „   UĞ¡˜ I è“ ‹UĞ‹ô  èHúÿƒ=¬I  tEì‹¬I èuúÿë&Eì‹‹’ü  ècúÿƒ}ì uEì‹‹’ô  èMúÿUÀ‹Eìèš”üÿ‹EÀUĞè7˜üÿ‹UĞEìè,úÿ‹‹€  ‹Uìè8ûÿë‹‹€  ‹( I è#ûÿ¡ì!I ƒx 
  ‹‹€$  èÇlüÿ¡ì!I ‹@H…À|u@‰EÔ3ö‹Ö¡ì!I èúÿ‰EàUĞ‹Eà‹@è³ ‹UĞ‹‹€$  ‹€ü   ‹Mà‹8ÿW0ƒ}èÿu-ƒ=ÄI  t$‹Eà‹ ‹ÄI è7úÿ…Àu‰uè‹Eàö@$uÆEçFÿMÔu‘ƒ}èÿuN‹ƒ¸   tC¡ì!I ‹@H…À|6@‰EÔ3ö‹Ö¡ì!I èy€úÿ‰Eà‹‹  ‹Eà‹ è¬6úÿ…Àu‰uèëFÿMÔuĞƒ}èÿt‹‹€$  ‹Uèè_lüÿë‹‹€$  3ÒèNlüÿ‹‹€x  èQ…üÿö›!I •Â‹‹€x  èÚ©şÿ¡ğ!I ‹@H…ÀŒ¨   @‰EÔ3ö‹Ö¡ğ!I èçúÿ‹øöG5t0ŠGPj öG5•À4PWUĞ‹Gè{ ‹UĞ‹‹€x  3Éè1‹şÿë<ŠGPj öG5•À4PŠG PöG5•À4PWUĞ‹Gè= ‹UĞ‹‹€x  3Éè#Šşÿƒ: u	6   r	‹Æ€0  FÿMÔ…^ÿÿÿ€}ç u}€=ĞI  tt€=w"I  tk¡ì!I ‹@H…ÀŒï   @‰EÔ3ö‹Ö¡ì!I èúÿ‰Eà‹Eàö@$t1‹‹€$  ‹Öèküÿ‹Eà‹±‹èÕ  3É‹ÈI ‹èf  é    FÿMÔu±é•   ƒ}èÿtg‹Uè¡ì!I è»~úÿ‰Eà‹Eàö@$t>3Ò¡ì!I è£~úÿ‹3É‹è€  ‹Eà‹±‹èr  ‹‹ˆ  ‹‹  ‹èû  ë8‹Eà‹3É‹èK  ë(¡ì!I ƒx ~3Ò¡ì!I èJ~úÿ‰Eà‹Eà‹3É‹è!  ‹èÂæÿÿ‹è3äÿÿ€=w"I  tP‹‹€$  èjüÿ‹Ğ¡ì!I è	~úÿ‰Eà‹Eàö@$u	ö›!I t‹‹€x  ²èíûÿë ‹‹€x  3ÒèÜûÿë‹‹€x  3ÒèËûÿ‹‹€x  ŠP7‹‹€|  è³ûÿ€=w"I  t(‹‹€$  è™iüÿ‹‰‚,  j ‹‹(  3É‹èÂ  UĞ¡œ I è% ‹UĞ‹ø  èÚıùÿƒ=°I  tö˜!I uEì‹°I èşıùÿëA‹ƒ¸    t‹‹€   ºÀ7F èÏÿùÿuEì‹‹’ø  èÍıùÿëEì‹‹’   è»ıùÿ‹‹€  ‹UìèÇûÿö˜!I  t4€=¼I  u‹€¸   t‹‹€  ²ègyüÿ‹‹€  ²è¸ûÿë‹‹€  3Òè§ûÿ3ÀZYYd‰hb7F EÀè¶üùÿEĞè®üùÿEìè¦üùÿÃéùùÿëà€}û t
d    ƒÄ‹_^[‹å]Ã       STOPIMAGE   ÿÿÿÿ   
  ÿÿÿÿ   

    ÿÿÿÿ       ÿÿÿÿ	   (Default)   SV‹Ú‹ğ†  èã©üÿ†   èØ©üÿ†$  èÍ©üÿ†  èÂ©üÿ†(  è·©üÿ†ì  è¬©üÿ3Ò‹ÆèoËûÿ„Ût‹ÆèDõùÿ‹Æ^[Ã@ S‹Ú‹ÓèòÜûÿ¡ŒI è4Iûÿ‰C[Ã@ U‹ìƒÄğSVW3É‰Mø‰Uü‹ø3ÀUhÕ8F dÿ0d‰ ‹‡ì  ‹pN…ö|F3Û‹Ó‹‡ì  èI{úÿ‹@ ;Eüt,CNuçEøP‹Eü‰EğÆEô Uğ3É¸ğ8F èĞ?úÿ‹Eøè„àşÿƒËÿ3ÀZYYd‰hÜ8F Eøè,ûùÿÃéŠ÷ùÿëğ‹Ã_^[‹å]Ã   ÿÿÿÿ   Could not find page with ID %d  SV‹ò‹Ø‹Ö‹Ãè-ÿÿÿ‹Ğ‹ƒì  è¨zúÿ^[ÃU‹ìQSVW‰Mü‹ú‹ğ‹†ì  èszúÿ‹Î²¸”	F èAãÿÿ‹Ø‰{ ‹Eü‰C$‹E‰C(‹U‹Ãèfäÿÿ‹U‹Ãèxäÿÿ‹Ó‹†ì  è¯yúÿ_^[Y]Â @ SVWU‹ù‹ò‹Øƒÿÿt‹×‹Ãè şÿÿ‹èEë	‹ƒì  ‹h‹ƒì  è÷yúÿÿƒğ  ƒ»ğ  u
Çƒğ  d   ‹Î²¸àøE èüÂÿÿ‹ø‹“Ä  ‹Çè	Äÿÿ‹ƒğ  ‰F ‹ƒÌ  ‰F$‰~(‹Î‹Õ‹ƒì  èzúÿ]_^[ÃU‹ìƒÄÜSVW3Ò‰Uì‰Uø‰Uô‰Uğ‹ğ3ÀUhY<F dÿ0d‰ Uô¡x I è4 ƒ}ô „Ü  Uğ‹EôèÇá  ¿   »¸
I j jEüPEìP¸p<F ‰EÜÆEà‹Eğ‰EäÆEèUÜ¹   ¸¬<F èÙ=úÿ‹Eìè…üùÿ‹È‹ DI ès¡üÿ…À…k  3ÀUh-<F dÿ0d‰ öš!I tü  º¸<F ‹Eüèq üÿöš!I t+   ºĞ<F ‹EüèU üÿºè<F ‹Eüè` üÿ„ÀtÆ†  öš!I @tW  º =F ‹Eüè! üÿMøº=F ‹Eüè üÿ„Àt‹Uø‹†  èGë  Møº8=F ‹EüèïŸüÿ„Àt‹Uø‹†  è%ë  ö›!I tDMøº\=F ‹EüèÄŸüÿ„Àt‹Uø‹†   èúê  Møºx=F ‹Eüè¢Ÿüÿ„Àt‹Uø‹†$  èØê  öœ!I t9  º˜=F ‹EüètŸüÿ  º´=F ‹EüèaŸüÿ  ºØ=F ‹EüèNŸüÿ3ÀZYYd‰h><F ‹EüPèhúÿÃé2ôùÿëïƒÃO…9şÿÿ3ÀZYYd‰h`<F Eìº   èÈ÷ùÿÃéôùÿëë_^[‹å]Ã ÿÿÿÿ3   Software\Microsoft\Windows\CurrentVersion\Uninstall ÿÿÿÿ	   %s\%s_is1   Inno Setup: App Path    Inno Setup: Icon Group  Inno Setup: No Icons    Inno Setup: Setup Type  Inno Setup: Selected Components Inno Setup: Deselected Components   Inno Setup: Selected Tasks  Inno Setup: Deselected Tasks    Inno Setup: User Info: Name Inno Setup: User Info: Organization Inno Setup: User Info: Serial   SVW‹Ø‹ƒP  ‹p,‹x0è¢ûÿ²‹ƒP  èùjşÿ‹Ö‹ƒP  èŒûÿ‹ƒP  ‹@0+Ç‹È‹“   ‹Ãè~Úÿÿ_^[Ã‹ÀU‹ìj SVW‹ò‹Ø3ÀUhö>F dÿ0d‰ ‹ÖEüè(öùÿEüº?F è÷ùÿ‹Uü‹ƒT  è)ûÿ‹“T  ‹ÃèüÙÿÿ‹ƒT  ‹p(p0‹ƒX  ‹Î+H(‹“Ğ  ‹ÃèÚÿÿ‹Ö‹ƒX  èÏûÿº   ‹Ãè“‹ ‹ĞÖ‹ƒ\  è´ûÿ3ÀZYYd‰hı>F EüèõùÿÃéiñùÿëğ_^[Y]Ã ÿÿÿÿ   
  U‹ìƒÄÜSVW3Û‰]à‰]Ü‰]ì‰Mô‰Uø‰Eü3ÀUh£@F dÿ0d‰ ‹Eü‹€Ğ  ‹€ü   ‹ÿR8¡"I ‹@H…ÀŒ   @‰EäÇEè    ‹Uè¡"I è\túÿ‰Eğ‹Eğö@M„ì   ‹Mğ‹Uô‹Eøè¾ä  „À„Ö   3ÀUh$@F dÿ0d‰ ‹Eğƒx tUì‹Eğ‹@èÉ ëU‹Eğö@Mu'UÜ‹Eğ‹ è± ‹EÜUàè.ˆüÿ‹UàMì°‚èiÅşÿë%UÜ‹Eğ‹ èŠ ‹EÜUàèˆüÿ‹UàMì°ƒèBÅşÿ3ÀZYYd‰ë$éGîùÿ‹Uü¡(I è"üÿEìº¼@F èQôùÿèDñùÿj ‹Eğö@M•À4Pjjj‹EèP‹Eü‹€Ğ  3É‹Uìè~şÿÿEèÿMä…ëşÿÿ3ÀZYYd‰hª@F EÜº   è†óùÿEìè^óùÿÃé¼ïùÿëã_^[‹å]Ã   ÿÿÿÿ   [Error] U‹ìƒÄØSVW3É‰MØ‰Mä‰Mà‰MÜ‰Uø‰Eü3ÀUhÿDF dÿ0d‰ 3À‰Eğ²¸Ğ¦@ è0êùÿ‰Eô3ÒUhİDF dÿ2d‰"²¸Ğ¦@ èêùÿ‰Eğ‹Mğ‹Uô‹Eüè
  ‹Eü‹€Ì  ‹€ü   ‹ÿR8EÜè¶òùÿ3À‰Eè3À‰Eì¡ô!I ‹pN…öŒU  F3ÿ‹×¡ô!I è`rúÿ‹Ø‹C;Eèÿ   S'Cèç „À…ì   j ‹CP‹CP‹K3Ò‹Eøèøà  „À„Í   Uä‹CèÍ
 Uà‹CèÂ
 ƒ{ u/‹Eà‹UÜè©ôùÿt"j j ‹Eü‹€Ì  3É‹Uàè<}şÿEÜ‹Uàè•òùÿöC1t2ŠCP€=ÑI  uöC1t3Àë°PjS‹Eü‹€Ì  3É‹Uäè}şÿë>ŠCP€=ÑI  uöC1t3Àë°PjŠCPöC1•À4PS‹Eü‹€Ì  3É‹Uäè|şÿ‹C@‰Eè‰]ìë1ƒ}ì t‹Eì‹@;Cu‹Eì‹‹è~'úÿ…Àt‹C;Eè}‰Eè3À‰EìGN…®şÿÿ€=ÑI  …   ‹Eü‹€Ì  ‹€ü   ‹ÿR‹ğN…ö|sF3ÿ‹Eü‹€Ì  ‹×è/‘şÿ‹Ø…ÛtV‹Eü‹€   ‹èeÒÿÿ„ÀtöC1•Á€ñ‹Eü‹€Ì  ‹×è•şÿë&‹Eü‹€$  ‹è5Òÿÿ„Àt‹Eü‹€Ì  3É‹×èç”şÿGNu¡ÌI ‹ÿR…ÀŞ   ‹Eü‹€Ì  ‹€ü   ‹ÿR‹ğN…öŒ¿   F3ÿ‹Eü‹€Ì  ‹×è‹şÿ‹Ø…Û„š   ºEF EØèäğùÿEØ‹èÂñùÿ‹UØ¡ÌI è©Ñÿÿ„Àt‹Eü‹€Ì  ±‹×è{™şÿë^‹¡ÌI è…Ñÿÿ„Àt‹Eü‹€Ì  ±‹×è7”şÿë:º$EF EØè„ğùÿEØ‹èbñùÿ‹UØ¡ÌI èIÑÿÿ„Àt‹Eü‹€Ì  3É‹×èû“şÿGN…Dÿÿÿ‹Eü‹€Ì  ‹€ü   ‹ÿR‹ğN…ö|_F3ÿ‹Eü‹€Ì  ‹×è±şÿ‹Ø…ÛtB‹‹EôèíĞÿÿ„Àt‹Eü‹€Ì  ±‹×èŸ“şÿë ‹‹EğèËĞÿÿ„Àt‹Eü‹€Ì  3É‹×è}“şÿGNu¤3ÀZYYd‰häDF ‹Eğè„æùÿ‹Eôè|æùÿÃé‚ëùÿëè3ÀZYYd‰hEF EØº   è"ïùÿÃé`ëùÿëë_^[‹å]Ã   ÿÿÿÿ   *   ÿÿÿÿ   !   SV‹Ø‹ƒ$  è¡Züÿ@t‹³$  ‹Æè‘Züÿ‹Ğ‹†ü   ‹ÿQ^[Ã3À^[ÃU‹ìƒÄğSVW3Û‰]ğ‰Mô‰Uø‰Eü3ÀUh”FF dÿ0d‰ ¡ğ!I ‹xO…ÿŒì   G3ö‹Ö¡ğ!I è3núÿ‹ØöC5…É   ƒ}ø „™   º¬FF EğèĞîùÿEğ‹è®ïùÿ‹Uğ‹Eøè—Ïÿÿ„Àt‹Eü‹€x  ±‹Öèi—şÿé‚   ‹‹EøèrÏÿÿ„Àt‹Eü‹€x  ±‹Öè$’şÿë`º¸FF EğèqîùÿEğ‹èOïùÿ‹Uğ‹Eøè8Ïÿÿ„Àt‹Eü‹€x  3É‹Öèê‘şÿë&ƒ}ô t ‹‹EôèÏÿÿ„Àt‹Eü‹€x  3É‹ÖèÂ‘şÿFO…ÿÿÿ3ÀZYYd‰h›FF EğèmíùÿÃéËéùÿëğ_^[‹å]Ã  ÿÿÿÿ   *   ÿÿÿÿ   !   SVWƒÄôˆL$‰T$‰$²¸Ğ¦@ èWäùÿ‹ø¡ğ!I ‹pN…ö|GF3Û‹Ó¡ğ!I èİlúÿ€|$ tö@5t'‹P‹Çèß  ‹T$‹ÇèWÎÿÿ‹È‹$‹€x  ‹Óè‘şÿCNu¼‹Çè"äùÿƒÄ_^[Ã@ U‹ìQS‹Ø²¸Ğ¦@ èİãùÿ‰Eü3ÀUh’GF dÿ0d‰ j 3É‹Uü‹Ãèª   ‹Uü‹ÃèHùÿÿ3ÀZYYd‰h™GF ‹EüèÇãùÿÃéÍèùÿëğ[Y]Ã@ U‹ìj SVW‹Ù‹ğ3ÀUhHF dÿ0d‰ „Òt1‹Ó‹FèÄ ‹E€x t'Mü‹VR° èKñùÿ‹Uü‹‹ÃèÓíùÿë	‹Ã‹èœìùÿ3ÀZYYd‰hHF EüèóëùÿÃéQèùÿëğ_^[Y]ÃU‹ìƒÄğSVW3Û‰]ğˆM÷‰Uø‰Eü3ÀUhÁHF dÿ0d‰ ‹Eø‹ÿR8‹Eü‹€x  ‹€ü   ‹ÿR‹ğN…ö|GF3Û‹Ó‹Eü‹€x  èY‹şÿ„Àt,‹Ó‹Eü‹€x  è™‹şÿ‹øUMğŠU÷‹ÇèÿÿÿY‹Uğ‹Eø‹ÿQ,CNu¼3ÀZYYd‰hÈHF Eğè@ëùÿÃéçùÿëğ_^[‹å]Â @ U‹ìƒÄøSVW3Û‰]øˆMÿ‹ğ‹]3ÀUhTIF dÿ0d‰ „Òt9Uø‹Fè… ‹Eø‹Óèïšüÿ€}ÿ t'Mø‹UR° èğùÿ‹Uø‹‹Ãèìùÿë	‹Ã‹èVëùÿ3ÀZYYd‰h[IF Eøè­êùÿÃéçùÿëğ_^[YY]Â U‹ìƒÄäSVW3Û‰]ä‰]ğ‰]ìˆMû‰Uü‹ğ3ÀUh¬JF dÿ0d‰ ‹Eü‹ÿR8€} tEìè\êùÿ‹†Ì  ‹€ü   ‹ÿRH…ÀŒË   @‰Eè3Û‹Ó‹†Ì  èÿ‰şÿ„À„¦   ‹Ó‹†Ì  è>Šşÿ…À„‘   ‹Ó‹†Ì  è)Šşÿ‹ø€} tVUğ‹Gè~ ƒ u4‹Eğ‹Uìèeìùÿt'ƒ}ğ tUä‹EğèÎ™üÿ‹Uä‹Eü‹ÿQ,Eì‹UğèLêùÿ‹G‰Eôƒ}ì tÿEôë‹G‰Eô‹EôPEäPŠMŠUû‹Çè`şÿÿ‹Uä‹Eü‹ÿQ,CÿMè…;ÿÿÿ3ÀZYYd‰h³JF EäèbéùÿEìº   èuéùÿÃé³åùÿëã_^[‹å]Â SVWUƒÄø‰L$‰$‹ğ‹$‹ÿR8‹D$‹ÿR8‹†x  ‹€ü   ‹ÿR‹øO…ÿ|@G3Û‹Ó‹†x  è‰şÿ‹è‹Ó‹†x  è¼ˆşÿ„Àt‹U ‹$‹ÿQ,ë‹U ‹D$‹ÿQ,COuÃYZ]_^[ÃSVWUƒÄø‰L$‰$‹è‹$‹ÿR8‹D$‹ÿR8‹…Ì  ‹€ü   ‹ÿR‹øO…ÿ|BG3ö‹…Ì  ‹ÖèŸˆşÿ‹Ø…Ût(‹…Ì  ‹Öè8ˆşÿ„Àt‹‹$‹ÿQ,ë‹‹D$‹ÿQ,FOuÁYZ]_^[Ã‹ÀU‹ìj SVW‹ğ3ÀUh‰LF dÿ0d‰ ³3Ò‹†¸  èõıúÿ3Ò‹†¼  èèıúÿè§ „ÀtvUüK 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/data/courses
END
course_idx.tcl
K 25
svn:wc:ra_dav:version-url
V 87
/svnroot/extremetuxracer/!svn/ver/60/trunk/extreme-tuxracer/data/courses/course_idx.tcl
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                W3À‰Eè‰Eä‰EØ‰Eü3ÀUh»]F dÿ0d‰ 3Û‹E‹@üèÒ
  „À„^  Uü‹E‹@ü‹€  è:îúÿ€=i"I  ….  EìPUä‹EüèRküÿ‹EäUèè'güÿ‹UèMô3Àè&åşÿ„Àtjºˆ"I Eôè­¦üÿ…À}Yh  jjEèPUä¸ˆ"I è¯¶ÿÿ‹Eä‰EÜUØEôè¶ÿÿ‹EØ‰EàUÜ¹   °*è]¨şÿ‹Eè±‹ I èa ƒø…¨   €=‰!I t€=‰!I  uJ‹Eü‹U‹Rü‹’ü  èvÙùÿt4‹Eüèoüÿ„Àt(jjjMè‹Uü°&è-¨şÿ‹Eè±‹I è ƒøuLö™!I t4‹EüèÇnüÿ„Àu(jjjMè‹Uü°$èğ§şÿ‹Eè±‹I èÄ ƒøu³¸@"I ‹UüèÀÖùÿ3ÀZYYd‰hÂ]F EØè[ÖùÿEäº   ènÖùÿEüèFÖùÿÃé¤ÒùÿëÛ‹Ã_^[‹å]ÃU‹ìƒÄĞSVW3À‰Eä‰Eà‰EĞ‰EÜ‰Eè3ÀUhÍ_F dÿ0d‰ 3Û€=i"I  …  EìPUÜ‹E‹@ü‹€  èeìúÿ‹EÜUàèiüÿ‹EàUäèceüÿ‹UäMô3Àèbãşÿ„ÀtjºTI Eôèé¤üÿ…À}Yh  jjEäPUà¸TI èë´ÿÿ‹Eà‰EÔUĞEôèÚ´ÿÿ‹EĞ‰EØUÔ¹   °*è™¦şÿ‹Eä±‹ I è ƒø…ö   ‹E‹@ü‹€  ‹ÿR…ÀÛ   Eèè3Õùÿ‹E‹@ü‹€x  ‹€ü   ‹ÿR‹ğN…öŒƒ   F3ÿ‹E‹@ü‹€x  ‹×èËtşÿ„Àue‹E‹@ü‹€x  ‹×èuşÿ‰Eü‹Eüö@5uF‹Eü‹‹E‹@ü‹€  è2¶ÿÿ„Àt,ƒ}è tEèºè_F è'ÖùÿUä‹Eü‹@è)í  ‹UäEèèÖùÿGNu€ƒ}è t(jjjMä‹Uè°mèÑ¥şÿ‹Eä±‹,I è¥ ƒøu³3ÀZYYd‰hÔ_F EĞèIÔùÿEÜèAÔùÿEàº   èTÔùÿÃé’ĞùÿëÛ‹Ã_^[‹å]Ã   ÿÿÿÿ      U‹ìj SVW3ÀUhJ`F dÿ0d‰ ‹E‹@üè×	  ‹Ø„Ût!Uü‹E‹@ü‹€  èYêúÿ‹Uü¸D"I èÔùÿ3ÀZYYd‰hQ`F Eüè·ÓùÿÃéĞùÿëğ‹Ã_^[Y]Ã@ U‹ìƒÄèSVW3É‰Mè‰Mì‰Eü}ü3ÀUhcF dÿ0d‰ ‹‹€4  ƒø„d  ƒø‡•   ÿ$…¢`F 0aF 0aF Æ`F İ`F 0aF î`F ÿ`F aF !aF ‹‹€   €¸   „  ëSUèeøÿÿY„À„
  ëBUè¼ùÿÿY„À„ù  ë1UèóúÿÿY„À„è  ë Uè¶üÿÿY„À„×  ëUèÅşÿÿY„À„Æ  ÆEû‹‹4  ‹èÍ×ÿÿUû‹ÿQ4€}û „¡  ƒ=´"I  t/j j j‹‹€4  ‰EğÆEô Mğº,cF ¡´"I è¯\ „À„i  ‹‹˜4  ‹Ó‹è®Öÿÿ‹ğƒëtKtpƒë„‘   ƒë„²   éÂ   Uì‹‹€¬  è±èúÿ‹Uì¸4"I ètÒùÿUì‹‹€´  è”èúÿ‹Uì¸8"I èWÒùÿUì‹‹€È  èwèúÿ‹Uì¸<"I è:ÒùÿëiUì‹‹€  èXèúÿ‹EìUèèahüÿ‹Uè¸@"I èÒùÿë?Uì‹‹€  è.èúÿ‹EìUèè7hüÿ‹Uè¸D"I èæÑùÿë‹Æ€1  ¡ŒI èíR ëuF‹‹€ì  ‹ÖèCQúÿ‹X ‹Ó‹èëïÿÿ‹ÃƒètHtë3‹èéÿÿ„Àt9ë&‹Ó‹èğÿÿ¡ŒI è_N „À…¼şÿÿ‹Æ€1  ë‹Ó‹èCôÿÿ„À…³şÿÿ‹Ó‹èfğÿÿ3ÀZYYd‰hcF EèèõĞùÿEìèíĞùÿÃéKÍùÿëè_^[‹å]Ã  ÿÿÿÿ   NextButtonClick SƒÄô‹Øƒ»4  thÆ$‹“4  ‹Ãè´Õÿÿ‹Ô‹ÿQ,€<$ tJƒ=´"I  t,j j j‹ƒ4  ‰D$ÆD$ L$ºÀcF ¡´"I èšZ „Àt‹Ãèkïÿÿƒøÿt	‹Ğ‹Ãè©ïÿÿƒÄ[Ãÿÿÿÿ   BackButtonClick èÈûÿÃ‹ÀSVWUƒÄè‹ù‹ò‹Ø‹“4  ‹ÃèÕÿÿ‹Ï‹Ö‹(ÿU0€> tAƒ=´"I  t8jj ‹ƒ4  ‰D$ÆD$ ‰t$ÆD$‰|$ÆD$L$ºTdF ¡´"I èY ƒÄ]_^[Ã   ÿÿÿÿ   CancelButtonClick   S‹Ù¡ŒI èoÇûÿÆ [Ã‹ÀSV‹Ø‹³$  ‹ÆèK;üÿ‹Ğ‹†ü   ‹ÿQ‹ğöF$•Á‹‹Ãèâÿÿ€=w"I  tQ‹ƒ$  è;üÿ‰ƒ,  j 3É‹“(  ‹ÃèEãÿÿö›!I u&öF$•Â‹ƒx  èêäúÿ‹ƒx  ŠP7‹ƒ|  èÖäúÿ‹ÃèW·ÿÿ‹ÃèÈ´ÿÿ^[ÃSVWUQ‹Ø²¸Ğ¦@ èÆùÿ‹ğj 3É‹Ö‹Ãèââÿÿ‹“(  ‹ÆèÅVúÿˆ$‹ÆèÆùÿ€<$ t‹“,  ‹ƒ$  èŒ:üÿëQ¡ì!I ‹xO…ÿ|DG3í‹Õ¡ì!I èNNúÿ‹ğöF$t)‹»$  ‹‡ü   ‹ÖèïZúÿ‹Ğ‹ÇèJ:üÿ±‹‹ÃèáÿÿëEOu¿‹Ãè¢¶ÿÿ‹Ãè´ÿÿZ]_^[ÃS‹Ø‹ƒ  è~Jüÿ‹Ğ€ò‹ƒ  è*äúÿ‹ƒ  3ÒŠP8‹•À
I è¿åúÿ‹ƒ  èLJüÿ‹Ğ€ò‹ƒØ  èøãúÿ[Ã‹ÀS‹J‹Ùãğÿ  û ğ  u¡(I èêÚûÿ[Ãù'  u¡ŒI èŞ= ëè«¾ûÿ[Ãƒ¸4  u²‹€¸  è¢ãúÿÃƒ¸4  u3Ò‹€¸  èŠãúÿÃU‹ìQSVW‰Eü‹Eüƒ¸4  uO3ÀUhÖfF dÿ0d‰ ‹Eüè$²ÿÿ‹Ğ‹Eü‹€¸  èLãúÿ3ÀZYYd‰ëé•Çùÿ3Ò‹Eü‹€¸  è-ãúÿèHÊùÿè—Êùÿ_^[Y]Ãö™!I @•Áö›!I €•Â‹€  èm   ÃU‹ìQS‹Ø=|"I   rRƒ=\I  tI3Àèı†ûÿ‰Eü3ÀUhzgF dÿ0d‰ j ‹ÃèßÏùÿPj j ÿ\I 3ÀZYYd‰hgF ‹Eüèw‡ûÿÃéåÈùÿëğ[Y]Ã@ U‹ìj j j j j SVWˆMşˆUÿ‹ğ3ÀUh¾iF dÿ0d‰ 3ÛUø‹ÆèÆâúÿEøè.®ÿÿ‹Uø‹Æèäâúÿ‹Eøè Íùÿ=ğ   ~j jj±3Ò¡I èQ é¯  ‹EøèxÍùÿƒø|‹Eø€8\u	‹Eø€x\t3Àë°„Àt€}ÿ uj jj±3Ò¡I è
 éh  „ÀuY€}ş u¾   ë¾   ‹EøèÍùÿ;ğ"‹EøŠ èM¿ùÿ¿,s‹Eø€x:u	‹Eø€x\tEj jj±3Ò¡ğI è­
 é  ‹Eøp²\‹Æè}aüÿ;ğrj jj±3Ò¡ğI è
 éß   ‹Eøèl®ÿÿ„Àu‹Eøèˆ®ÿÿ„Àu‹Eøè¬®ÿÿ„Àtj jj±3Ò¡äI èD
 é¢   EğP¹ÿÿÿº   ‹EøèaÎùÿ‹Uğ¸ØiF è`_üÿ…Àt.j jjUì¸ØiF è±®ÿÿ‹UìMğ°èœşÿ‹Eğ±3Òèì	 ëMUì‹EøèS^üÿ‹EìUğè(Züÿ‹EğUôèaüÿ‹Eôèıÿÿ‹Eôè•büÿ„Àuj jj±3Ò¡èI è¡	 ë³3ÀZYYd‰hÅiF Eìº   ècÊùÿÃé¡Æùÿëë‹Ã_^[‹å]Ã  ÿÿÿÿ   /:*?"<>|    U‹ìj j j SVW‹ğ3ÀUhkF dÿ0d‰ 3Û‹†  èOFüÿ„À…Ó   Uü‹†  è]àúÿEüèQ¬ÿÿ‹Uü‹†  èwàúÿ‹Eüè3Ëùÿƒøx~j jj±3Ò¡ÄI èæ é‹   ‹EüèÑ¬ÿÿ„Àu‹Eüèí¬ÿÿ„Àtj jj±3Ò¡ìI èµ ë]ƒ}ü uj jj±3Ò¡I è™ ëA‹Uü¸ kF èÎ]üÿ…Àt.j jjUô¸ kF è­ÿÿ‹UôMø°è‚šşÿ‹Eø±3ÒèZ ë³3ÀZYYd‰hkF Eôº   èÉùÿÃéZÅùÿëë‹Ã_^[‹å]Ã   ÿÿÿÿ   /:*?"<>|    U‹ì3ÉQQQQSVW‹Ø3ÀUhÒkF dÿ0d‰ Uğ‹ƒô  è>_üÿ‹EğUôè³\üÿ‹EôUüèÔÿùÿƒ}ü uUü¡I èÁÿùÿUø‹ƒ  èóŞúÿ‹EüPöœ!I €•ÂMø3Àèo—ÿÿ„Àt‹Uø‹ƒ  èùŞúÿ3ÀZYYd‰hÙkF Eğº   èOÈùÿÃéÄùÿëë_^[‹å]ÃU‹ìj j j SVW‹Ø3ÀUh{lF dÿ0d‰ Uô‹ƒø  è
\üÿ‹EôUüè+ÿùÿƒ}ü uUü¡I èÿùÿUø‹ƒ  èJŞúÿ‹EüPö!I •ÂMø°èÆ–ÿÿ„Àt‹Uø‹ƒ  èPŞúÿ3ÀZYYd‰h‚lF Eôº   è¦ÇùÿÃéäÃùÿëë_^[‹å]Ã@ U‹ìj j SVW‹Ù‹u3ÀUh@mF dÿ0d‰ Uü‹èşùÿ‹Uü‹Ãè“Çùÿƒ; t‹èÇªÿÿ„ÀtÆ j jj±3Ò¡äI è\ ë?‹¸XmF è’[üÿ…Àt/Æ j jjUø¸XmF èàªÿÿ‹UøMü°èC˜şÿ‹Eü±3Òè 3ÀZYYd‰hGmF Eøº   èáÆùÿÃéÃùÿëë_^[YY]Â ÿÿÿÿ	   \/:*?"<>|   U‹ìj j SVW‹Ù‹u3ÀUhnF dÿ0d‰ Uü‹èµıùÿ‹Uü‹Ãè»Æùÿƒ; t‹èï©ÿÿ„ÀtÆ j jj±3Ò¡ìI è„ ë?‹¸0nF èºZüÿ…Àt/Æ j jjUø¸0nF èªÿÿ‹UøMü°èk—şÿ‹Eü±3ÒèC 3ÀZYYd‰hnF Eøº   è	ÆùÿÃéGÂùÿëë_^[YY]Â ÿÿÿÿ	   \/:*?"<>|   U‹ìƒÄôSVW3Ò‰Uô‰Eü3ÀUh…oF dÿ0d‰ ‹Eüƒ¸4  u!j jjUô°xèá¦ÿÿ‹Eô±3Òè½ èèúÿ‹Eü‹€4  ‰Eø3ÀUhĞnF dÿ0d‰ ‹Eü‹€¸  è8ûÿ„Àt‹Eü‹€¸  f»õÿè:½ùÿ3ÀZYYd‰ë,é›¿ùÿ¡ŒI €¸¶  wèPÂùÿë‹Uü¡(I èa×ûÿèÂùÿ‹Eü€¸1   ug‹Eü‹€4  ;Eø…Aÿÿÿ¡ŒI €¸¶  w¸œoF è¾èşÿè9úÿéÿÿÿ¸ØoF èªèşÿ°èwû  ¡(I èÒûÿ¡(I ‹@ Pèçñùÿ¡PI è-¾ûÿ3ÀZYYd‰hŒoF Eôè|ÄùÿÃéÚÀùÿëğ_^[‹å]Ã ÿÿÿÿ0   Failed to proceed to next wizard page; aborting.    ÿÿÿÿ6   Failed to proceed to next wizard page; showing wizard.  h0pF º €  ¸PpF è¬qüÿPè:éùÿ£\I ÃSHPathPrepareForWriteA  ÿÿÿÿ   shell32.dll     qF ôqF qF ¤qF     ÓqF Ô  tÂH PB à*@ +@ ŒB H¸@ B d·@ ÔA ¨B 0B ¬HA äB èA ,sF ŒA €	B  €A @CA ôCA ¬B ˆÊH T€A œ€A h|A €€A ğB ÜeA ,B DB B B ,eA \B ¸~A ´-B         |@ Ğ   àqF ´    DiskBitmapImage¸   SelectDiskLabel¼   	PathLabelÀ   PathEditÄ   BrowseButtonÈ   OKButtonÌ   CancelButton   tF FormCloseQuery puF BrowseButtonClickTNewDiskForm D¼E ¡D VB ØmB ‹ÀTNewDiskFormpF ÃH 8 NewDisk  ‹ÀU‹ìƒÄìSVW3Û‰]ô‰]ì‹Ù‹ú‹ğ3ÀUhsF dÿ0d‰ ‹(I ²¸pF èİ   ‰Eø3ÀUhõrF dÿ0d‰ ‹EøĞ  ‹×èáÁùÿEôPUì‹Æè{úùÿ‹Eì‰EğUğ3É°Šè’şÿ‹Uô‹Eø‹€¸  èØúÿ‹‹Eø‹€À  è Øúÿj èÕíùÿ‹Eøè)»ûÿH”Eÿ€}ÿ tUô‹EøèK  ‹Uô‹ÃèqÁùÿ3ÀZYYd‰hürF ‹Eøèd¸ùÿÃéj½ùÿëğ3ÀZYYd‰h!sF EìèïÀùÿEôèçÀùÿÃéE½ùÿëèŠEÿ_^[‹å]ÃSV„ÒtƒÄğèö¹ùÿ‹Ú‹ğ3Ò‹ÆèT ‹ÆèJV ƒ=PI  t3É‹PI ‹ÆèîT ë‹Æè	V ‹ÔI ‹Æè4×úÿ‹HI ‹†¼  è#×úÿ‹ I ‹†Ä  è×úÿ‹ÀI ‹†È  è×úÿ‹¤I ‹†Ì  èğÖúÿhtF ¡I PèŒìùÿ‹Ğ‹†´  ‹€´   èebûÿº  ÿ ‹†´  è)Nÿÿ‹VH‹†´  è3Nÿÿ„Ût
d    ƒÄ‹Æ^[ÃDISKIMAGE   U‹ìj j j SVW‹ò‹Ø3ÀUhtF dÿ0d‰ Uô‹ƒÀ  è4Öúÿ‹EôUøèéöùÿ‹EøUüè2Vüÿ‹Eü‹Öè€Rüÿ3ÀZYYd‰h–tF Eôè¿ùÿEøº   è’¿ùÿÃéĞ»ùÿëã_^[‹å]Ã@ U‹ìƒÄğSVW3Û‰]ø‰]ü‹ñ‹Ø3ÀUhauF dÿ0d‰ ‹ƒ(  HtHtpëuUü‹ÃèEÿÿÿƒ}ü t%Uø‹Eüè¨NüÿEø‹“Ğ  è‚Àùÿ‹EøèöVüÿ„Àu@Æ j j j EøP‹ƒĞ  ‰Eğ‹Eü‰EôUğ¹   °Mèûşÿ‹Eø±3Òèş  ëèĞ) ˆ3ÀZYYd‰hhuF Eøº   èÀ¾ùÿÃéşºùÿëë_^[‹å]ÃU‹ìj SVW‹Ø3ÀUhÒuF dÿ0d‰ Uü‹Ãèşÿÿj ‹Ãèİûÿ‹ÈUü¡”I è²Oÿÿ„Àt‹Uü‹ƒÀ  èôÔúÿ3ÀZYYd‰hÙuF Eüè/¾ùÿÃéºùÿëğ_^[Y]Ã    vF                 &vF <  Ä@ ,,@ à*@ +@ ğvF         |@ 8  TFileExtractor@ ƒ=`I  u3À !I ‹…È
I ²¸vF è1   £`I ¡`I Ã‹À¸`I èNküÿÃ‹ìI ²¸Xc@ èúÿè‘ºùÿÃSVW„ÒtƒÄğè•¶ùÿ‹ñ‹Ú‹ø3Ò‹Çè†´ùÿÇGÿÿÿÿÇGÿÿÿÿWh(~F ²¸üşD èº‰şÿ‰GWh(~F ²‹Æÿ‰G„Ût
d    ƒÄ‹Ç_^[ÃSV‹Ú‹ğ‹FèZ´ùÿ‹FèR´ùÿ‹FèJ´ùÿ3Ò‹Æè5´ùÿ„Ût‹ÆèJ¶ùÿ‹Æ^[ÃU‹ìƒÄ´SVW3Û‰]ä‰]ô‰]ğ‰]ì‰]è‹Ù‰Uü3ÀUhPyF dÿ0d‰ Uä¡”I è²Püÿ‹EäMô3ÒèÉLüÿ‹Eü‹„!I ™÷ù‹ğF‹Eü™÷ù‰UøIu)EğP‹Eô‰EÔÆEØ‰uÜÆEà UÔ¹   ¸hyF èÓ úÿë4EğP‹Eô‰E¼ÆEÀ‰uÄÆEÈ ‹EøƒÀaˆEÌÆEĞU¼¹   ¸|yF è úÿEäP‰u´ÆE¸ U´3É¸yF èƒ úÿ‹UäEì‹Mğè±½ùÿƒ=dI  t)Uä¡dI èoKüÿ‹Uä‹Ã‹Mğè½ùÿ‹è¿Süÿ„À…ô   Uä¡  I èFKüÿ‹Uä‹Ã‹Mğèe½ùÿ‹è–Süÿ„À…Ë   ƒ=dI  t1Uä¡dI èKüÿEä‹Uìèñ¼ùÿ‹Eä‹ÓèWNüÿ‹è\Süÿ„À…‘   Uä¡  I èãJüÿEä‹UìèÀ¼ùÿ‹Eä‹Óè&Nüÿ‹è+Süÿ„ÀudEè‹  I èµ»ùÿ‹Eğ‰E´ÆE¸U´3É¸¤yF èøàşÿMè‹Uğ‹Æèùÿÿ„Àt'¸dI ‹Uèè:»ùÿUä‹EèèsJüÿ‹Uä‹Ã‹Mğè’¼ùÿëè;úÿ3ÀZYYd‰hWyF Eäº   èÑºùÿÃé·ùÿëë_^[‹å]Ã  ÿÿÿÿ	   %s-%d.bin   ÿÿÿÿ   %s-%d%s.bin ÿÿÿÿ
   ..\DISK%d\  ÿÿÿÿ)   Asking user for new disk containing "%s".   U‹ìƒÄèSVW3É‰Mü‹ò‹Ø3ÀUhÌzF dÿ0d‰ ;s„½   ÇCÿÿÿÿCè¸güÿƒ=œI  uMü‹Ö‹èıÿÿëEü‹”I ègºùÿjj j‹Mü²¸höD è‚~şÿ‰Cƒ=œI  ueUô¹   ‹C‹8ÿWƒøtèüÿÿEôº	I ¹   èJ®ùÿtèóûÿÿUğ¹   ‹C‹8ÿWƒøtèÙûÿÿUè‹C‹ÿQ‹Eè;EğtèÁûÿÿ‰s3ÀZYYd‰hÓzF Eüè5¹ùÿÃé“µùÿëğ_^[‹å]Ã‹ÀU‹ìQSVW‹Ù‰Eü3ÀUh{F dÿ0d‰ ‹Eü¶@1‹Mü‹D‹Ë‹0ÿV3ÀZYYd‰ëéV³ùÿ‹EüÆ@0è¶ùÿèa¶ùÿ‹EüƒÀ(‹Óè4ˆüÿ_^[Y]Ã‹ÀU‹ìƒÄSUø‹E‹@ü‹@¹   ‹ÿSƒøtèûÿÿE èmˆüÿUøE ¹   è…ˆüÿ‹E‹@ü‹€8  èä¹ùÿ‹È‹E‹@ü‹8  E è^ˆüÿUE è‰üÿU‹E‹@üƒÀ3¹   èİ)ÿÿ‹E‹@üƒÀ3ºè  èî)ÿÿ[‹å]ÃU‹ìP¸   ÄğÿÿPHuö‹EüƒÄüSVW‹ğ}ø¥¥uø‹}ƒÇø3ÀUhb|F dÿ0d‰ »   ƒ~ u;v‹…Ût)•øÿşÿ‹E‹@ü‹Ëèšşÿÿ‹Æ‹Óè‡üÿƒ? tÊ‹3ÀÿÒëÂ3ÀZYYd‰ëéÍ²ùÿ   şD s|F èüùÿÿèµùÿ_^[‹å]ÃU‹ìƒÄìSVW‰Mø‹Ú‰EüuüöCD@t‹ƒ¸8   u
¸ì}F èœşÿ‹‹@;u'‹‹@;Cu‹P(Cèe†üÿ…À|‹€x0 „Ä   ‹Ç@ÿÿÿÿöCD€•Àƒà‹‹D‚‹ÿR‹Æ@0 ‹‹èÀüÿÿ‹SœI ‹‹@èe{şÿUô‹‹@¹   ‹8ÿWƒøtè5ùÿÿ‹Eô;	I tè%ùÿÿöCD@tUèåıÿÿY‹‹‰B‹C‹‰B‹C‹‰B‹‹S‰P ‹S ‰P$‹3Ò‰P,‹3Ò‰P(öCD€•À‹ˆB1öCD@•À‹ˆB2‹P(Cè„…üÿ…À~#‹C‰Eì‹C‰Eğ‹P(Eìè…üÿUEìèşÿÿY_^[‹å]Ã  ÿÿÿÿ9   Cannot read an encrypted file before the key has been set   SVWƒÄø‹Ø‰T$‹ùƒ{$ u;{ v‹{ ‰<$…ÿtW‹T$‹Ï‹C‹0ÿV‹ğC ‹Öèó„üÿ€{2 tV‹L$‹T$C3è4'ÿÿ;÷t"+şt$‹C;C|èá÷ÿÿ‹SB‹Ãè2ûÿÿ…ÿu©‹$YZ_^[ÃU‹ìP¸   ÄğÿÿPHuö‹EüƒÄˆSVW‹ñ‰Uø‰Eü‹Eø‹P‰Uğ‹P‰UôUğ‹Æ‹ÿQ‹ÆèÔ{şÿ3Ò‹ÆèyşÿE”èÛ„üÿ3ÀUhšF dÿ0d‰ 3À‰Eì»   ƒ}ô u;]ğv‹]ğ…Ûte•”ÿşÿ‹Ë‹Eüè¡ûÿÿ‹Eøö@Dt‹EìP…”ÿşÿ3É‹Óè±€şÿ]ìEğ‹Óèôƒüÿ•”ÿşÿE”‹Ëè„üÿ•”ÿşÿ‹Ë‹Æ‹8ÿWƒ} t‹‹ÃÿUë„3ÀZYYd‰ëé•¯ùÿ   şD «F èÄöÿÿè×±ùÿ€} t(•„ÿşÿE”èë„üÿ…„ÿşÿ‹UøƒÂ$è’üÿ„Àuè‘öÿÿ_^[‹å]Â SVWƒÄ¼‹ğ3Û3Ò‹Æè‰xşÿT$¹@   ‹Æ‹8ÿWƒø@u>€|$Mu7€|$Zu0ƒ|$@ t)‹T$@‹ÆèTxşÿ‹Ô¹   ‹Æ‹0ÿVƒøu<$PE  u³‹ÃƒÄD_^[ÃSVWUƒÄà‰$‹ú‹ğ3Û‹Æèyÿÿÿ„ÀtYT$¹   ‹Æ‹(ÿUƒøuD·D$=à   u8T$‹Æ‹ÿ‹$‹T$‰‹T$‰P‹×¹à   ‹Æ‹0ÿV=à   u	f?u³‹ÃƒÄ ]_^[Ã‹ÀU‹ìÄÿÿÿSVW‰Mü‹ú‹ğMô•ÿÿÿ‹Æè]ÿÿÿ‹Ø„Ût.‰}”‹Eü‰E˜‹E‰…TÿÿÿUô‹Æ‹ÿQ•ÿÿÿ¹à   ‹Æ‹0ÿV‹Ã_^[‹å]Â ‹À                        €F $   ÜrE ,,@ à*@ +@ PyE ”F l–E p–E TSetupUninstallLog‹Ğ¡(I è¸ÄûÿÃ@ U‹ìj SVW‹Ú‹ğ3ÀUh‚F dÿ0d‰ EüP¡PI ‹¸ˆ  ‹O,‹WD‹ÆèÂfüÿ‹Uü‹ÇèÌÈúÿ„Ût¡PI ‹€ˆ  ‹ÿRP3ÀZYYd‰h‚F Eüèó±ùÿÃéQ®ùÿëğ_^[Y]Ã‹PI ‹’„  ’è‚Èúÿ¡PI ‹€„  ‹ÿRP²3Àè]ÿÿÿÃSVWUƒÄø3À‰D$¡"I i@è  ‰$¡"I ƒx t$è  ¡"I ƒx t$è  ¡ü!I ‹pN…ö|WF3ÿ‹×¡ü!I è01úÿ‹Øj ‹Ë‹X"I ¡P"I è*¡  „Àt*‹k8ƒıÿt‹Õ¡ "I è1úÿP‹Äè˜€üÿë
S@‹ÄèŒ€üÿGNu¬3À£pI ë‹Äº   è§€üÿÿpI ƒ|$ uç<$   sŞ¡PI ‹€Œ  ‹$è6lşÿYZ]_^[Ã@ SƒÄø‹hI ‰$‹lI ‰D$‹Ä‹pI èP€üÿ¡PI ‹˜Œ  ‹$‹Ãèlşÿ‹Ã‹ÿRPYZ[Ã‹‰hI ‹P‰lI è¦ÿÿÿÃºhI ’èÑüÿè”ÿÿÿÃ@ ‹Ğ¸hI èÄüÿèÿÿÿÃ‹À€=`"I  tèªúÿ¡(I ètÀûÿ€=`"I  tè’úÿÃè«ÿÿÿèÎÿÿÿÃU‹ìj SVW‹ú‹ğ3ÀUhx„F dÿ0d‰ 3Ûjjj‹ÖEüè|°ùÿEüº„F èW±ùÿEü‹×èM±ùÿ‹Eü±3Òèï  ƒètHtHt	ëè#úÿë³ë¸ „F è“Óşÿèúÿ3ÀZYYd‰h„F Eüè‰¯ùÿÃéç«ùÿëğ‹Ã_^[Y]Ã ÿÿÿÿ   

    ÿÿÿÿ:   LoggedMsgBox returned an unexpected value. Assuming Abort.  SƒÄ°‹ÚTPèkÓùÿD$PD$PèdÓùÿ…ÀtxS·D$‰D$ÆD$  ·D$‰D$$ÆD$( ·D$‰D$,ÆD$0 ·D$‰D$4ÆD$8 ·D$‰D$<ÆD$@ ·D$‰D$DÆD$H ·D$‰D$LÆD$P T$¹   ¸…F èóùÿë‹Ãº¼…F è¯ùÿƒÄP[Ã   ÿÿÿÿ"   %.4u-%.2u-%.2u %.2u:%.2u:%.2u.%.3u  ÿÿÿÿ	   (invalid)   U‹ìƒÄğSVW‹ù‹ò‹Ø3ÀUh†F dÿ0d‰ Mğ‹Ö‹Ãè«şÿVWuğ¹   ó¥_^³3ÀZYYd‰ëé`¨ùÿ3Ûèu«ùÿ‹Ã_^[‹å]ÃSVWUP¸   ÄğÿÿPHuö‹„$   ƒÄì‹ñ<$¥¥‹ò‹øºhI ‹Äè}üÿT$‹Ç‹ÿQT$‹Æ‹ÿQ‹ÆèRtşÿ3Ò‹Æèrşÿ»   ƒ|$ u
;\$v‹\$…ÛtuT$‹Ë‹Çè¾qşÿT$‹Ë‹Æ‹(ÿUD$‹Óè|üÿ‹hI ‰D$‹lI ‰D$D$‹Óè“|üÿ‹ÔD$èT|üÿ…À~‹$‰D$‹D$‰D$D$èxüÿÿè³üÿÿéqÿÿÿ‹ÄègüÿÿÄ  ]_^[ÃSVW‹ñ‹ú‹Ø…öt!‹×‹Ãèª‰şÿƒøÿt‹ÈáÿÿÿÎ‹×‹Ãè:şÿ_^[Ã‹ÀU‹ìj j SVW‹Ú‹ğ3ÀUhÒ‡F dÿ0d‰ ‹Ó‹Æèw?üÿEüP3É²3Àè¨Ô  ƒ}ü t-Uø‹èø?üÿ‹Eø‹Uüè™=üÿ…ÀuUø‹èg@üÿ‹Uø‹Ãèİ¬ùÿ3ÀZYYd‰hÙ‡F Eøº   èO¬ùÿÃé¨ùÿëë_^[YY]ÃTRegisterFilesListRec      |@     U‹ìƒÄğS‹ØEğPèÑùÿUğ‹Ã¹   è·¬ùÿ[‹å]Ã‹ÀU‹ìƒÄàSVW3À‰Eè‰Eä‰Eà‰Eü3ÀUhßˆF dÿ0d‰ ö˜!I tEü‹@"I è(¬ùÿëEüèŠ«ùÿjj Eèèr°şÿ‹Eè‰EìEäè °şÿ‹Eä‰Eğ‹Eü‰EôEàèdÿÿÿ‹Eà‰EøMì‹E‹@üfº èñşÿ3ÀZYYd‰hæˆF Eàº   èJ«ùÿEüè"«ùÿÃé€§ùÿëã_^[‹å]Ã@ U‹ìƒÄØSVW3À‰EØ‰Eü‰Eø‰Eô3ÀUhï‰F dÿ0d‰ ºä I Eü¹    è¹«ùÿö˜!I tUø¸ŠF èOÃ  ëEøè¹ªùÿ3ÀUhl‰F dÿ0d‰ Uô¸ŠF è*Ã  3ÀZYYd‰ëéÿ¤ùÿEôè‡ªùÿè¨ùÿjh ¡Ì I ‰EÜ‹Eü‰Eà‹Eø‰Eä‹Eô‰Eè¡D"I ‰EìUØ¸(ŠF èØÂ  ‹EØ‰EğMÜ‹E‹@üfº  èğşÿ3ÀZYYd‰hö‰F EØèªùÿEôº   è2ªùÿÃép¦ùÿëã_^[‹å]Ã   ÿÿÿÿ   {app}   ÿÿÿÿ   {group} ÿÿÿÿ
   {language}  ÿÿÿÿ   RegSetValueEx   ÿÿÿÿ   RegCreateKeyEx  ÿÿÿÿ   RegOpenKeyEx    U‹ìƒÄÔSVW3Û‰]ô‰]è‰]Ø‰]Ô‰Mø‰Uü‹Ø‹u3ÀUhu‹F dÿ0d‰ EôPUè‹Eüèñşÿ‹Eè‰Eì‹Eø‰EğUì‹ûƒçŠ‡Ø
I ¹   èMzşÿEôº‹F èªùÿEôPEèP‹½Ü
I ‰EÜUØ‹Æèğáùÿ‹EØ‰EàUÔ‹Æè8[üÿ‹EÔ‰EäUÜ¹   °4èÿyşÿ‹UèXèFªùÿ‹Mô²¸Xc@ èW úÿèÒ¥ùÿ3ÀZYYd‰h|‹F EÔº   è¼¨ùÿEèè”¨ùÿEôèŒ¨ùÿÃéê¤ùÿëÛ_^[‹å]Â    ÿÿÿÿ   

    U‹ìSVW‹Ù‹ú‹ğ‹ÃèÉ©ùÿ@P‹Ãè„«ùÿPjj WVèÌùÿ…ÀtP‹E‹Hø‹E‹Pü3Àè¤şÿÿ_^[]Ã@ U‹ìSVW‹Ù‹ú‹ğ…Ût‹EP‹Ë‹×‹Æè™ÿÿÿYK 25
svn:wc:ra_dav:version-url
V 64
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/data
END
tux_walk.tcl
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/tux_walk.tcl
END
tux_snowboard.tcl
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/data/tux_snowboard.tcl
END
tux.tcl
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/data/tux.tcl
END
Makefile.in
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/63/trunk/extreme-tuxracer/data/Makefile.in
END
models.tcl
K 25
svn:wc:ra_dav:version-url
V 74
/svnroot/extremetuxracer/!svn/ver/9/trunk/extreme-tuxracer/data/models.tcl
END
etracer_init.tcl
K 25
svn:wc:ra_dav:version-url
V 81
/svnroot/extremetuxracer/!svn/ver/63/trunk/extreme-tuxracer/data/etracer_init.tcl
END
Makefile.am
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/63/trunk/extreme-tuxracer/data/Makefile.am
END
terrains.png
K 25
svn:wc:ra_dav:version-url
V 77
/svnroot/extremetuxracer/!svn/ver/42/trunk/extreme-tuxracer/data/terrains.png
END
objects.png
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/42/trunk/extreme-tuxracer/data/objects.png
END
Makefile
K 25
svn:wc:ra_dav:version-url
V 73
/svnroot/extremetuxracer/!svn/ver/64/trunk/extreme-tuxracer/data/Makefile
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ^şÿ¡à I èg™ùÿP¸à I è,›ùÿ‹Ğ‹ÃY‹8ÿW¡”!I P¡à I èB™ùÿ‹È‹Ö‹ÃèŸäÿÿ„Àu¸¼œF è¥»şÿë
¸ôœF è™»şÿ_^[Ã ÿÿÿÿ   Applying digital signature. ÿÿÿÿ+   Failed to apply signature: Unexpected size. ÿÿÿÿ,   Failed to apply signature: Header not found.    ÿÿÿÿ   Signature applied successfully. U‹ìƒÄàSV‹Ø‹Ãèk]şÿUè‹E‹@øèm”şÿUè¹   ‹Ã‹0ÿVÇEøIMsgUà‹Ã‹ÿ‹Eà‰Eü‹Ãè¥ıÿÿUø¹   ‹Ã‹ÿS^[‹å]ÃSoftware\Microsoft\Windows\CurrentVersion\Fonts Software\Microsoft\Windows NT\CurrentVersion\Fonts  U‹ìj j j SVW‹Ù‹ú‹ğ3ÀUhÕF dÿ0d‰ j jEüP3À k"I ‹…è
I º  €3Àè>üÿ…Àu?‹ÆèO—ùÿ@P‹Æè
™ùÿPjj ‹Çèş˜ùÿP‹EüPè˜¹ùÿ…Àt
¸ìF è¹şÿ‹EüPè9¹ùÿë
¸ ŸF è‰¹şÿ„ÛtO‹ÆèÆ˜ùÿPè½ùÿ…Àtj j jhÿÿ  è¸Âùÿë,MôºLŸF °3è¿fşÿ‹EôUøè@Füÿ‹Eø‹$I è6åÿÿ„Àt±3ÀZYYd‰hÜF Eôº   èL•ùÿÃéŠ‘ùÿëë_^[‹å]Ã ÿÿÿÿ*   Failed to set value in Fonts registry key.  ÿÿÿÿ"   Failed to open Fonts registry key.  ÿÿÿÿ   AddFontResource U‹ìƒÄèSVW3É‰Mì‰Mè‰Mô‰Uø‰Eü‹Eøèª—ùÿ3ÀUh„ F dÿ0d‰ Uì‹EøèU'üÿ‹UìEøèò”ùÿUô‹Eøè;)üÿ‹Eôè¿•ùÿPEôè†—ùÿZè,`şÿ‰Eğ‹E‹@‹ÿR‹ğN…ö|aF3Û‹E‹@‹Ó‹ÿQ;EğuHMè‹E‹@‹Ó‹8ÿW‹EèUìèß(üÿ‹Eì‹Uôèp–ùÿu ‹E‹@3É‹Ó‹0ÿV‹E‹@3É‹Ó‹ÿS ëCNu¢‹E‹@‹Mø‹Uü‹ÿS‹E‹@‹Mğ‹Uü‹ÿS 3ÀZYYd‰h‹ F Eèº   èª“ùÿEôº   è“ùÿÃéÛùÿëŞ_^[‹å]Ã‹ÀU‹ìƒÄôSV‹Ù‹òˆEÿƒûÿtt‹ÖŠEÿè*pşÿƒøÿte¨ua‰uôÆEøUô3É¸,¡F è¹şÿ‹Ó¡è!I è÷úÿ‹Ø‹è”ùÿ¹   ™÷ùP‹‹È‹ÖŠEÿèÿÿ„Àuè¸ùÿ‰EôÆEø Uô3É¸T¡F èÔ¸şÿ^[‹å]Ã  ÿÿÿÿ   Setting permissions on file: %s ÿÿÿÿ'   Failed to set permissions on file (%d). U‹ìƒÄôSV‹Ù‹òˆEÿ„Ût‰uôÆEøUô3É¸ø¡F èK¸şÿë‰uôÆEøUô3É¸(¢F è3¸şÿ‹Ë‹ÖŠEÿè7sşÿ„ÀuèR·ùÿ‰EôÆEø Uô3É¸X¢F è¸şÿ^[‹å]Ã  ÿÿÿÿ$   Setting NTFS compression on file: %s    ÿÿÿÿ&   Unsetting NTFS compression on file: %s  ÿÿÿÿ*   Failed to set NTFS compression state (%d).  U‹ìÄ,ÿÿÿSVW3Û‰lÿÿÿ‰@ÿÿÿ‰]ä‰]Ü‰]Ø‰]Ô‰]Ğ‰]Ä‰MôˆUû‰Eü‹Eôèg”ùÿ‹Eè_”ùÿ3ÀUhî³F dÿ0d‰ ¸´F èµşÿÆEÍ ÆEÎ ‹Eü‹X8ƒûÿt‹Ó¡ "I èÔúÿ‰Eàë3À‰EàEÔèîùÿÆEÌ ÆEó ‹hI ‰Eè‹lI ‰EìEäèÌùÿEÄèÄùÿ3ÒUhµ²F dÿ2d‰"3ÒUhl²F dÿ2d‰"ÆEÊ 3À‰E¨€}û tM¨   ‹Eüö@JtƒM¨‹Eüö@J tƒM¨@‹Eüö@K€tM¨€   ‹Eüö@LtM¨   EĞèPùÿ‹EüŠ@NşÈuEØ‹T I èÌùÿë!ƒ} uUØ‹Eü‹@è®¨  ëEØ‹Uè©ùÿ•lÿÿÿ‹EØèó"üÿ‹•lÿÿÿEØèùÿ²‹EØè“İÿÿ‹EØ‰…dÿÿÿÆ…hÿÿÿ•dÿÿÿ3É¸$´F è½µşÿŠEû:u"I t€}û t¸@´F è¢³şÿë
¸d´F è–³şÿÆE® €=k"I  t0=|"I    r$‹UØŠEûè>şÿ„Àt¸ˆ´F èd³şÿ‹Eü€xN ”E®‹UØŠEûèúnşÿˆEÏ€}Í u
ŠEÏˆEÎÆEÍ€}Î tƒM¨ƒ}à t.‹Eàö@Dt‹Eà‹P4‰U ‹P8‰U¤ëE P‹EàƒÀ4Pè=µùÿ³ëM ‹UôŠEûè¯‹şÿ‹Ø„Ût5•lÿÿÿE èÇßÿÿ‹…lÿÿÿ‰…dÿÿÿÆ…hÿÿÿ•dÿÿÿ3É¸Ä´F è¶´şÿë
¸è´F èª²şÿ€}Ï „'  ¸µF è–²şÿ‹Eüö@Lt¸8µF èƒ²şÿéÒ
  Eä‹TI èùÿM˜‹UØŠEûè"‹şÿˆE¯€}¯ t5•lÿÿÿE˜è7ßÿÿ‹…lÿÿÿ‰…dÿÿÿÆ…hÿÿÿ•dÿÿÿ3É¸lµF è&´şÿë
¸”µF è²şÿ‹Eüö@L…¹  ÆE­ ƒ}à t!‹Eàö@D•ÀˆEÃ‹Eà‹@<‰E¸‹Eà‹@@‰E¼ë"•lÿÿÿ‹EôèĞ üÿ‹•lÿÿÿM¸ŠEûè‹kşÿˆEÃ€}Ã t[·Eº‰…DÿÿÿÆ…Hÿÿÿ ·E¸‰…LÿÿÿÆ…Pÿÿÿ ·E¾‰…TÿÿÿÆ…Xÿÿÿ ·E¼‰…\ÿÿÿÆ…`ÿÿÿ •Dÿÿÿ¹   ¸ÌµF èa³şÿë
¸øµF èU±şÿ•lÿÿÿ‹EØèC üÿ‹•lÿÿÿM°ŠEûèşjşÿ„À„Ä  ·E²‰…DÿÿÿÆ…Hÿÿÿ ·E°‰…LÿÿÿÆ…Pÿÿÿ ·E¶‰…TÿÿÿÆ…Xÿÿÿ ·E´‰…\ÿÿÿÆ…`ÿÿÿ •Dÿÿÿ¹   ¸¶F èÕ²şÿ€}Ã t‹E°;E¸w‹E°;E¸uu‹E´;E¼vm‹Eüö@L•À4
E®uMjjj‹UØ…lÿÿÿè6ùÿ…lÿÿÿºL¶F èùÿ…lÿÿÿ‹ŒI èıùÿ‹…lÿÿÿ±3Òè²Ë  ƒø„	  ¸\¶F èO°şÿé  ‹E°;E¸…î   ‹E´;E¼…â   ‹Eüö@K…Õ   ‹Eüö@L@„   pÿÿÿ‹UØŠEûèßİÿÿ„Àtoƒ}à t‹Eàp$}€¹   ó¥ë*Eä‹XI èŒùÿM€‹UôŠEûè×ˆşÿEä‹TI èeŒùÿU€…pÿÿÿè/füÿ„Àt¸¶F è¥¯şÿéô  ¸Ì¶F è–¯şÿëD¸·F èŠ¯şÿë8‹Eüö@J€u¸P·F èu¯şÿéÄ  ÆE­ë¸p·F è`¯şÿ€}Ã u
ÆE­ëÆE­€}­ „À   ‹Eüö@J€„³   „Ût€}¯ u¸œ·F è%¯şÿét  E PE˜Pè¯ùÿ…Àu¸È·F è¯şÿéT  E PE˜Pèï®ùÿ…À~i‹Eüö@L•À4
E®uIjjj‹UØ…lÿÿÿèn‹ùÿ…lÿÿÿºL¶F èFŒùÿ…lÿÿÿ‹ŒI è5Œùÿ‹…lÿÿÿ±3ÒèêÉ  ƒøt¸ì·F è‹®şÿéÚ  EäèŠùÿ€}® t¸$¸F èn®şÿé½  ‹Eüö@JtXjjj‹UØ…lÿÿÿèğŠùÿ…lÿÿÿºL¶F èÈ‹ùÿ…lÿÿÿ‹¤I è·‹ùÿ‹…lÿÿÿ±3ÒèlÉ  ƒøt¸p¸F è®şÿé\  ‹UØŠEûèéfşÿ‹Øƒûÿ„—   öÃ„   ‹Eüö@Ku!‹I ‹EØèÎÙÿÿ„Àt¸´¸F èÄ­şÿé  Eä‹(I èUŠùÿ‹Ëƒáş‹UØŠEûè5jşÿ…Àt¸¹F è“­şÿë
¸0¹F è‡­şÿ‹Eüö@Ku#éqÿÿÿ‹Eüö@K t€}Î u¸`¹F è`­şÿé¯  ¸˜¹F èQ­şÿEÜ‹Uôèê‰ùÿŠEû:u"I udƒ}Ü uTlÿÿÿ‹Eü‹P8‹E‹ÿSƒ½lÿÿÿ t7@ÿÿÿ‹Eü‹P8‹E‹ÿS‹•@ÿÿÿŠEûè¢hşÿ„ÀtMÜ‹Eü‹P8‹E‹ÿSƒ}Ü ”EËëÆEË Eä‹4I èi‰ùÿEÔP•lÿÿÿ‹EØèÿüÿ‹•lÿÿÿ¹¸¹F ŠEûè\rşÿ‹EP•lÿÿÿ‹EÔè*üÿ‹•lÿÿÿŠÀ¹F ŠEûèöéÿÿY‹EÔPj jj ŠMû²¸,E èèişÿ‰E”3ÀUhJ­F dÿ0d‰ ÆEÌ3ÀUh•¬F dÿ0d‰ ÆEóEä‹XI èÏˆùÿƒ}Ü uDèhÊÿÿ¹àƒF ‹Uàè§ĞÿÿEä‹,I è©ˆùÿhàƒF ‹Eüö@L€•À4Pè6Êÿÿ‹Uà‹M”èŸÒÿÿë|‹EÜPjj jŠMû²¸,E èPişÿ‰E3ÀUh„¬F dÿ0d‰ Eä‹,I èMˆùÿƒ}à t‹EàH‹U”‹EèÂÙÿÿë‹M‹U”‹Eè²Ùÿÿ3ÀZYYd‰h‹¬F ‹EèÕ~ùÿÃéÛƒùÿëğ3ÀZYYd‰ëéÖùÿÆEó EèèÒÖÿÿè„ùÿèÜ„ùÿE Pj j ‹E”‹@Pè ®ùÿ‹Eü€xNugÆEË ºInUn‹E”èÆîÿÿ‹E¶@÷‹…X
I ‰…dÿÿÿÆ…hÿÿÿ•dÿÿÿ3É¸Ì¹F èå¬şÿö!I  t
‹E”è´îÿÿë€=İI  u‹EP‹E”èáïÿÿY3ÀZYYd‰hQ­F ‹E”è~ùÿÃéƒùÿëğ‹Eüƒx t'Eäè†ùÿUĞ‹EØèãÙÿÿ€}Î t‹EĞè½‰ùÿPèc¯ùÿ€}Ï „ó   ‹Eü€xN„æ   Eä‹xI èî†ùÿ»   é»   èo«ùÿƒø„À   ƒøt	ƒø …   ‹UüöBJtQ€=n"I  tH‰…dÿÿÿÆ…hÿÿÿ •dÿÿÿ3É¸ü¹F èó«şÿEä‹€I è‰†ùÿÆr"I ‹MØ‹UÔŠEûèèsşÿÆEÊëX…Û~1‰…dÿÿÿÆ…hÿÿÿ •dÿÿÿ3É¸LºF è§«şÿKhè  è¬ùÿè_ÕÿÿëPèk¬ùÿ¸ŒºF è-lşÿ‹UØŠEûè`şÿ…À„2ÿÿÿ€}Ê u‹Eü€xN”À"EÎtY‹Eü€xNu‹EƒÀğ‹UÔèê…ùÿÆEÌ EäèJ…ùÿ¸ ºF è0©şÿ€}Ë tU‹UÔ‹Eü‹@8è‡ğÿÿY‹Eü‹H<‹UÔŠEûè5ØÿÿéŠ   Eä‹tI èš…ùÿ‹MØ‹UÔŠEûètcşÿ…Àu
¸ÔºF è‚kşÿÆEÌ EÔèŞ„ùÿEäèÖ„ùÿ¸èºF è¼¨şÿ€}Ë tU‹UØ‹Eü‹@8èğÿÿY‹Eüö@Jt3ÉŠMû‹UØ¡"I ‹ÿS0‹Eü‹H<‹UØŠEûè¦×ÿÿ‹Eüƒx tEEäèy„ùÿ‹Eü‹@‰…dÿÿÿÆ…hÿÿÿ•dÿÿÿ3É¸»F èDªşÿŠMÊ€ñ‹Eü‹P‹EĞèîÿÿƒM¨Eäè4„ùÿ‹Eü€xNtV‹Eüö@Juf‹Eüö@J@u]j‹E¨P‹EØ‰…0ÿÿÿ‹EÔ‰…4ÿÿÿ‹Eü‹@‰…8ÿÿÿ‹EĞ‰…<ÿÿÿ0ÿÿÿ‹E‹@üfº‚ è«Éşÿë‹Eƒxğ u	‹EÆ@ïë‹EÆ@ï‹Eüö@Ju	‹Eüö@J t~‹UØŠEûè;cşÿ„ÀtoEäè“ƒùÿ‹Eüö@J t¸@»F èp§şÿë
¸x»F èd§şÿºà‡F ¸   èåŠùÿ‹Ø‹Ã‹UØè©ƒùÿŠEûˆC‹Eüö@J •ÀˆC‹Eüö@K@•ÀˆC‹E‹@è‹Óèdúÿ‹Eüö@J@„ø   Eäèƒùÿ€}û t¸¬»F è÷¦şÿŠMÎ‹UØ°èÚxşÿë¸à»F èŞ¦şÿŠMÎ‹UØ°èÁxşÿ‹Eüö@JueƒM¨€}û tM¨   ‹Eüö@MtM¨   j‹E¨P‹EØ‰…0ÿÿÿ‹EÔ‰…4ÿÿÿ‹Eü‹@‰…8ÿÿÿ‹EĞ‰…<ÿÿÿ0ÿÿÿ‹E‹@üfº‚ èIÈşÿëL€}û t$j j‹EØ‰…,ÿÿÿ,ÿÿÿ‹E‹@üfºŠ èÈşÿë"j j ‹EØ‰…,ÿÿÿ,ÿÿÿ‹E‹@üfºŠ èûÇşÿEäè‚ùÿƒ}Ô t‹Eü¿HH‹UÔŠEûè›îÿÿë‹Eü¿HH‹UØŠEûè‡îÿÿ‹Eüö@M •Ã„Ûu	‹Eüö@M@t>EäèÔùÿƒ}Ô t‹Eüö@M •À‹Ë‹UÔŠEûè3ïÿÿë‹Eüö@M •À‹Ë‹UØŠEûèïÿÿ3ÀZYYd‰ë*éÿ{ùÿè"×ùÿº¨c@ è yùÿ„Àtè¯~ùÿEÄè“2üÿèö~ùÿ3ÀZYYd‰h¼²F €}Ì t‹UÔŠEûèP\şÿÃéª}ùÿëçƒ}Ä „¨   ‹Eü€xNu&‹Eƒxğ t‹E‹@ğèı»ùÿ‹EƒÀğèùÿ‹EÆ@ï ƒ}ä tEäº¼F èt‚ùÿ‹UØ…lÿÿÿè~ùÿ…lÿÿÿºL¶F èV‚ùÿ…lÿÿÿ‹UäèH‚ùÿ…lÿÿÿ‹UÄè:‚ùÿ‹…lÿÿÿ‹œI è™Ğÿÿ„Àu€}ó „©ïÿÿEèèĞÿÿéœïÿÿ€}ó uƒ}à t‹EàƒÀèĞÿÿë‹EèĞÿÿè$Ğÿÿ3Ò3ÀèÎÿÿ3ÀZYYd‰hõ³F …@ÿÿÿèK€ùÿ…lÿÿÿè@€ùÿEÄè8€ùÿEĞº   èK€ùÿEäè#€ùÿEôè€ùÿEè€ùÿÃéq|ùÿëµ_^[‹å]Â   ÿÿÿÿ   -- File entry --    ÿÿÿÿ   Dest filename: %s   ÿÿÿÿ   Non-default bitness: 64-bit ÿÿÿÿ   Non-default bitness: 32-bit ÿÿÿÿ2   Dest file is protected by Windows File Protection.  ÿÿÿÿ   Time stamp of our file: %s  ÿÿÿÿ(   Time stamp of our file: (failed to read)    ÿÿÿÿ   Dest file exists.   ÿÿÿÿ)   Skipping due to "onlyifdoesntexist" flag.   ÿÿÿÿ   Time stamp of existing file: %s ÿÿÿÿ-   Time stamp of existing file: (failed to read)   ÿÿÿÿ    Version of our file: %u.%u.%u.%u    ÿÿÿÿ   Version of our file: (none) ÿÿÿÿ%   Version of existing file: %u.%u.%u.%u   ÿÿÿÿ   

    ÿÿÿÿ+   Existing file is a newer version. Skipping. ÿÿÿÿ3   Existing file's MD5 sum matches our file. Skipping. ÿÿÿÿ?   Existing file's MD5 sum is different from our file. Proceeding. ÿÿÿÿ3   Failed to read existing file's MD5 sum. Proceeding. ÿÿÿÿ   Same version. Skipping. ÿÿÿÿ    Version of existing file: (none)    ÿÿÿÿ#   Couldn't read time stamp. Skipping. ÿÿÿÿ   Same time stamp. Skipping.  ÿÿÿÿ/   Existing file has a later time stamp. Skipping. ÿÿÿÿ@   Existing file is protected by Windows File Protection. Skipping.    ÿÿÿÿ8   User opted not to overwrite the existing file. Skipping.    ÿÿÿÿJ   User opted not to strip the existing file's read-only attribute. Skipping.  ÿÿÿÿ   Stripped read-only attribute.   ÿÿÿÿ$   Failed to strip read-only attribute.    ÿÿÿÿ,   Skipping due to "onlyifdestfileexists" flag.    ÿÿÿÿ   Installing the file.    ÿÿÿÿ   .tmp        ÿÿÿÿ&   Uninstaller requires administrator: %s  ÿÿÿÿE   The existing file appears to be in use (%d). Will replace on restart.   ÿÿÿÿ6   The existing file appears to be in use (%d). Retrying.  ÿÿÿÿ
   DeleteFile  ÿÿÿÿ(   Leaving temporary file in place for now.    ÿÿÿÿ   MoveFile    ÿÿÿÿ    Successfully installed the file.    ÿÿÿÿ!   Registering file as a font ("%s")   ÿÿÿÿ.   Will register the file (a type library) later.  ÿÿÿÿ)   Will register the file (a DLL/OCX) later.   ÿÿÿÿ(   Incrementing shared file count (64-bit).    ÿÿÿÿ(   Incrementing shared file count (Extreme Tux Racer - Version 0.4

go to extremetuxracer.com for more info                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      @‰EäÇEü    ‹Uü¡"I è¼çùÿ‰Eà‹Eà‹@P‹Eà‹@P‹Eà‹@P‹Eà‹H‹X"I ¡P"I è^V  „À„Ì  ‹Uü°
èÀ˜  ‹Eà‹@$èÕR  Uø‹Eà‹@è€  Uô‹Eà‹@è€  Uğ‹Eà‹@èÿ  Uì‹Eà‹ èò  ƒ}ô „Ş   ‹Eàö@<„Ñ   ‹Eàö@<t‹Mì‹Uô‹Eøèèüÿ„À…²   3ÛUè‹Eìè¯úûÿƒ}è td3ÀUhÍF dÿ0d‰ ‹EPŠdÎF ‹Uè3ÀègÈÿÿY3ÀZYYd‰ë73ÀZYYd‰éSaùÿEÜèÿüÿ‹EÜ‹$I è¹¶ÿÿ„Àt	³èNdùÿëèGdùÿëœ„Ûu7ëMÜ‹Uì°5è8şÿ‹EÜ‹$I è„¶ÿÿ„Àu‹EìP‹Mğ‹Uô‹Eøèîüÿ„ÀtË‹Eàö@<t"jj ‹Eì‰EÔ‹Eø‰EØMÔ‹E‹@üfº… è'¬şÿ‹Eàö@<t"jj‹Eì‰EÔ‹Eø‰EØMÔ‹E‹@üfº… èü«şÿ‹Eàö@<t.ƒ}ô t(jj ‹Eì‰EÈ‹Eø‰EÌ‹Eô‰EĞMÈ‹E‹@üfº„ èÅ«şÿ‹Eà‹@ èJQ  ÿEüÿMä…åıÿÿ¸è  è`µÿÿ3ÀZYYd‰hZÎF EÜè»eùÿEèº   èÎeùÿÃébùÿëã_^[‹å]Ã       SV‹ğ3Û‹ÆèÃhùÿë
€ú\t³ë@Š„Òuğ‹Ã^[ÃU‹ìƒÄàSVW3Û‰]è‹ù‹òˆEÿ‹]3ÀUh`ÏF dÿ0d‰ ƒûÿ„   Uè‹ÆèçLşÿ‹Eè‰EìÆEğ‰}ôÆEøUì¹   ¸xÏF è‹şÿ‹Ó¡è!I èàäùÿ‹Ø‹P‹ètfùÿ¹   ™÷ùP‹Ï‹ÖŠEÿè«Ôşÿ„Àu1èŠùÿƒøu¸¬ÏF èÃˆşÿëèğ‰ùÿ‰EàÆEä Uà3É¸ĞF è¦Šşÿ3ÀZYYd‰hgÏF Eèè¡dùÿÃéÿ`ùÿëğ_^[‹å]Â ÿÿÿÿ*   Setting permissions on registry key: %s\%s  ÿÿÿÿR   Could not set permissions on the registry key because it currently does not exist.  ÿÿÿÿ/   Failed to set permissions on registry key (%d). U‹ìƒÄ¬SVW3À‰E°‰Eô‰Eğ‰Eì‰Eà3ÀUh	×F dÿ0d‰ ¡"I ‹@H…ÀŒc  @‰EĞÇEØ    ‹UØ¡"I èKãùÿ‰EÌ‹EÌ‹@P‹EÌ‹@P‹EÌ‹@P‹EÌ‹H‹X"I ¡P"I èíQ  „À„	  ‹UØ°èO”  ‹EÌ‹@ èdN  Uà‹EÌ‹ è«{  Uô‹EÌ‹@è{   DI ˆEß‹EÌö@@tÆEß‹EÌö@@t€=l"I  u
¸ ×F è HşÿÆEßÆE× 3ÒUh¯ÕF dÿ2d‰"‹EÌö@?@t$‹Eàè!ıÿÿ„Àt‹Eàèåeùÿ‹È‹EÌ‹P8ŠEßèuüÿ‹EÌö@?@t‹EÌ€x> „,  ‹EÌö@?€tQ‹EÌ€x> uHj jEüP‹Eàèeùÿ‹È‹EÌ‹P8ŠEßè‰
üÿ…À…ò  ‹Eôè}eùÿP‹EüPèç…ùÿ‹EüPèÆ…ùÿéÒ  ‹EÌ¿@<P‹Mà‹EÌ‹P8ŠEßè üÿÿ‹EÌö@@uqj j j jj EüPEøP‹Eàè)eùÿ‹È‹EÌ‹P8ŠEßèİ	üÿ‹Ø…Ûu%ƒ}ø…Œ   ‹EÌ¿@<P‹Mà‹EÌ‹P8ŠEßèDüÿÿëq‹EÌö@@uhS‹Mà‹EÌ‹P8°è¸ÿÿëU‹EÌ€x> tGj jEüP‹Eàè¹dùÿ‹È‹EÌ‹P8ŠEßè¥	üÿ‹Ø…Ût&ƒût!‹EÌö@@uS‹Mà‹EÌ‹P8°èÈ·ÿÿë»   …Û…â  3ÒUhÕF dÿ2d‰"‹EÌö@?€t‹EôèVdùÿP‹EüPèÀ„ùÿ‹EÌ€x> „  ‹EÌö@?t‹Eôè.dùÿ‹Ğ‹Eüèhüÿ„À…k  ‹EÌŠ@>‹ĞJ€êr„²  şÊ„  şÊ…G  »   ‹Ğ€êt€êt	ë»   ë»   <„—   ‹EÌ‹P¸h×F èådùÿ…Àt‹Eôèµcùÿ‹ĞMì‹EüèÔüÿ„ÀuEìèd`ùÿ‹EÌö@? t/j j EèPj ‹EôècùÿP‹EüPè„ùÿ…Àuƒ}ètƒ}èu‹]èEğP¸|×F ‰EÄ‹Eì‰EÈUÄ¹   ‹EÌ‹@è³x  é   ‹EÌ‹P¸h×F èNdùÿ…Àt‹Eôècùÿ‹ĞMì‹EüèIüÿ„ÀuEìèÍ_ùÿEğP¸|×F ‰E´‹Eì‰E¸¸Œ×F ‰E¼¸œ×F ‰EÀU´¹   ‹EÌ‹@èDx  ƒ}ğ t‹Eğèş`ùÿ‹Uğ€|ÿ tEğºœ×F èï`ùÿ‹Eğèß`ùÿ@P‹Eğè™bùÿPSj ‹EôèbùÿP‹EüPè'ƒùÿ‹Ø…Û„È   ‹EÌö@@…»   S‹Mà‹EÌ‹P83ÀèŸµÿÿé¥   U°‹EÌ‹@èœw  ‹E°è0˜ùÿ‰EäjEäPjj ‹Eôè+bùÿP‹EüPèÅ‚ùÿ‹Ø…Ûtj‹EÌö@@uaS‹Mà‹EÌ‹P83ÀèEµÿÿëN‹EÌ‹@è0`ùÿP‹EÌ‹@èèaùÿPjj ‹EôèÛaùÿP‹EüPèu‚ùÿ‹Ø…Ût‹EÌö@@uS‹Mà‹EÌ‹P83Àèõ´ÿÿ3ÀZYYd‰h¥ÕF ‹EüPè÷ùÿÃéÁZùÿëï3ÀZYYd‰ë(é¼XùÿE°èhüÿ‹E°‹$I è"®ÿÿ„ÀuÆE×èµ[ùÿ€}× …Cûÿÿ‹EÌö@?t1‹Eàèvøÿÿ„Àt%‹EÌ‹@8P‹Eà‰E¬E¬Pj ‹E‹@üŠMßfº† èI¥şÿ‹EÌö@?t1‹Eàè<øÿÿ„Àt%‹EÌ‹@8P‹Eà‰E¬E¬Pj ‹E‹@üŠMßfºˆ è¥şÿ‹EÌö@?t+‹EÌ‹@8P‹Eà‰EÄ‹Eô‰EÈEÄPj‹E‹@üŠMßfº‰ èÛ¤şÿ‹EÌö@?t+‹EÌ‹@8P‹Eà‰EÄ‹Eô‰EÈEÄPj‹E‹@üŠMßfº‡ è§¤şÿ‹EÌ‹@èœH  ÿEØÿMĞ…¨ùÿÿ¸è  è²¬ÿÿ3ÀZYYd‰h×F E°è]ùÿEàè]ùÿEìº   è]ùÿÃéVYùÿëÛ_^[‹å]Ã ÿÿÿÿ=   Cannot access 64-bit registry keys on this version of Windows   ÿÿÿÿ	   {olddata}   ÿÿÿÿ   olddata ÿÿÿÿ   break   ÿÿÿÿ       U‹ìP¸   ÄğÿÿPHuö‹EüƒÄèSVW3É‰ìÿşÿ‰Mø‰Uü‹Ø3ÀUh+ÙF dÿ0d‰ EøP¹DÙF ‹Ó3ÀèÀEşÿ3À‰Eğjj j•ìÿşÿ3Àèûûÿ‹ìÿşÿ²¸höD è® şÿ‰Eô3ÒUhøØF dÿ2d‰"j jj ‹Mø²¸höD èˆ şÿ‰Eğ3ÒUh»ØF dÿ2d‰"•äÿşÿ‹Eô‹ÿQ•äÿşÿ‹Eğ‹ÿQ‹EğèS"şÿ3Ò‹Eğè şÿ•ğÿşÿ¹   ‹Eô‹ÿS…Àt•ğÿşÿ‹È‹Eğ‹ÿSë×ºInRS‹EğèóÂÿÿ3ÀZYYd‰ëé°UùÿEğèøüÿ‹Eøè–ùÿècXùÿè²Xùÿ3ÀZYYd‰hÿØF ‹EğèiRùÿ‹EôèaRùÿÃégWùÿëè‹Eü‹UøèŠ[ùÿ3ÀZYYd‰h2ÙF …ìÿşÿèŞZùÿEøèÖZùÿÃé4Wùÿëå_^[‹å]Ã   ÿÿÿÿ   .exe    U‹ìQSj jj ‹È²¸höD è_şÿ‰Eü3ÀUhºÙF dÿ0d‰ ‹@I ¡à!I èNÚùÿ‹Ø‹Cèä[ùÿPCè«]ùÿ‹Ğ‹EüY‹ÿS3ÀZYYd‰hÁÙF ‹EüèŸQùÿÃé¥Vùÿëğ[Y]Ã@ U‹ìƒÄÜSVW3À‰Eä‰Eü‰Eì‰Eè3ÀUhOİF dÿ0d‰ €=n"I  tD3ÀUhÚF dÿ0d‰ Uü¡( I èıÿÿ3ÀZYYd‰ë2éPTùÿEäè şûÿ‹EäUüèmıÿÿèTWùÿëEäèşûÿ‹EäUüèSıÿÿ‹Eü‰EÜÆEàUÜ3É¸hİF èŠşÿ3ÒUhåÜF dÿ2d‰"Mäº”İF ‹Eüè´éûÿ‹EäèÀşÿÿj jj Mäº¤İF ‹Eüè–éûÿ‹Mä²¸÷D èşÿ‰Eø3ÀUhÇÛF dÿ0d‰ º´İF ‹EøèQ"şÿºìİF Eäè¸YùÿEä‹œ"I è’Zùÿ‹Uä‹Eøè+"şÿºüİF Eäè’YùÿEä‹”I èlZùÿ‹Uä‹Eøè"şÿ3Ò‹Eøèû!şÿºŞF ‹Eøèî!şÿ3Ò‹Eøèä!şÿ‹E‹@‹@è‹pN…ö|dF3ÿ‹E‹@‹@è‹×ètØùÿ‹ØEè‹º`ŞF èOZùÿEèèË[ùÿ3ÒŠS3ÉŠKMğ
I ŠˆP€{ tEèè¦[ùÿÆ@q‹Uè‹Eøès!şÿGNuŸ3ÀZYYd‰hÎÛF ‹Eøè’OùÿÃé˜Tùÿëğ€=n"I  t¾  €ë¾  €j j j jj EôPEğP¹hŞF ‹Ö3ÀèùK 25
svn:wc:ra_dav:version-url
V 59
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer
END
mkinstalldirs
K 25
svn:wc:ra_dav:version-url
V 72
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/mkinstalldirs
END
configure
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/24/trunk/extreme-tuxracer/configure
END
etracericons.zip
K 25
svn:wc:ra_dav:version-url
V 76
/svnroot/extremetuxracer/!svn/ver/52/trunk/extreme-tuxracer/etracericons.zip
END
Makefile.in
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/Makefile.in
END
AUTHORS
K 25
svn:wc:ra_dav:version-url
V 67
/svnroot/extremetuxracer/!svn/ver/29/trunk/extreme-tuxracer/AUTHORS
END
depcomp
K 25
svn:wc:ra_dav:version-url
V 66
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/depcomp
END
ChangeLog
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/14/trunk/extreme-tuxracer/ChangeLog
END
config.guess
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/config.guess
END
etracericon.svg
K 25
svn:wc:ra_dav:version-url
V 75
/svnroot/extremetuxracer/!svn/ver/52/trunk/extreme-tuxracer/etracericon.svg
END
README
K 25
svn:wc:ra_dav:version-url
V 66
/svnroot/extremetuxracer/!svn/ver/65/trunk/extreme-tuxracer/README
END
config.sub
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/config.sub
END
config.h.in
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/config.h.in
END
configure.ac
K 25
svn:wc:ra_dav:version-url
V 71
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/configure.ac
END
INSTALL
K 25
svn:wc:ra_dav:version-url
V 67
/svnroot/extremetuxracer/!svn/ver/17/trunk/extreme-tuxracer/INSTALL
END
COPYING
K 25
svn:wc:ra_dav:version-url
V 66
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/COPYING
END
Makefile.am
K 25
svn:wc:ra_dav:version-url
V 70
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/Makefile.am
END
missing
K 25
svn:wc:ra_dav:version-url
V 66
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/missing
END
NEWS
K 25
svn:wc:ra_dav:version-url
V 63
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/NEWS
END
aclocal.m4
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/5/trunk/extreme-tuxracer/aclocal.m4
END
install-sh
K 25
svn:wc:ra_dav:version-url
V 69
/svnroot/extremetuxracer/!svn/ver/2/trunk/extreme-tuxracer/install-sh
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             è  …-ÿÿÿ3Û…üÿÿ€8 u7U‹ÃèíüÿÿY‹E‹@ä‰… üÿÿÆ…üÿÿ• üÿÿ3É¸TíF è«mşÿUèÉıÿÿYë1C@ûè  uºüÿÿ‹Uü°Eèôşÿ‹üÿÿ²¸Xc@ è"ŸùÿèDùÿ3ÀZYYd‰h¬ìF …üÿÿº   è„GùÿEüè\GùÿÃéºCùÿëà_^[‹å]Ã ÿÿÿÿ!   Directory for uninstall files: %s       ÿÿÿÿ)   Will append to existing uninstall log: %s   ÿÿÿÿ)   Will overwrite existing uninstall log: %s   ÿÿÿÿ   Creating new uninstall log: %s  U‹ì3ÉQQQQQQQSVW‹ú‹Ø3ÀUh`îF dÿ0d‰ èŠkùÿ‹ğÿ5T I hxîF 3ÀŠÃÿ4…tI hxîF EøP‰}ìUè‹Æè0ùÿ‹Eè‰EğUø‹Æèxøûÿ‹EøUäè÷ûÿ‹Eä‰EôUì¹   °4è4şÿÿuøEüº   è,Hùÿjjj±3Ò‹Eüè&…  ƒèt	ƒèu³ë3Ûë¸ˆîF èºişÿ3Û3ÀZYYd‰hgîF Eäº   èÎEùÿEøº   èÁEùÿÃéÿAùÿëŞ‹Ã_^[‹å]Ãÿÿÿÿ   

    ÿÿÿÿ;   LoggedMsgBox returned an unexpected value. Assuming Cancel. U‹ìSV»T I ‹uƒÆğƒ> tYëº@ïF °Aèşÿÿ„Àuè€œùÿ‹èéùÿ„ÀtàëºTïF °@èkşÿÿ„Àuè^œùÿ‹èHùÿP‹èHùÿPè%kùÿ…ÀtÒ‹ÆèÎDùÿ^[]Ã  ÿÿÿÿ
   DeleteFile  ÿÿÿÿ   MoveFile    U‹ìQ‹E€xï tpö!I  u	€=İI  t^j jj ‹E‹Hà²¸höD è-	şÿ‰Eü3ÀUhÖïF dÿ0d‰ ‹E€xïu‹EÆ@İ‹EüèD«ÿÿ3ÀZYYd‰hİïF ‹Eüèƒ;ùÿÃé‰@ùÿëğY]ÃU‹ìƒÄøSVWƒ=´"I  „š   º ğF ¡´"I è3Í „À„ƒ   €=r"I  up3ÀUhdğF dÿ0d‰ j j j 3À‰EøÆEüMøº ğF ¡´"I èïÍ „ÀtÆr"I ¸´ğF è–gşÿ3ÀZYYd‰ë,é>ùÿ¸ìğF è}gşÿ3Ò¡(I èÙUûÿèAùÿë
¸ñF è`gşÿ_^[YY]Ã ÿÿÿÿ   NeedRestart ÿÿÿÿ/   Will restart because NeedRestart returned True. ÿÿÿÿ    NeedRestart raised an exception.    ÿÿÿÿL   Not calling NeedRestart because a restart has already been deemed necessary.    SVW¡ğ!I ‹pN…ö|>F3ÿ‹×¡ğ!I èMÂùÿ‹Øj ‹CPj ‹3Ò¡P"I è1  „ÀtöC5t	Ær"I ëGNuÅ_^[Ã‹ÀSVW¡ô!I ‹pN…ö|@F3ÿ‹×¡ô!I èùÁùÿ‹Ø‹P‹CPj 3É‹X"I 3Àè¯0  „ÀtöC1t	Ær"I ëGNuÃ_^[ÃU‹ìÄ¼şÿÿSVW3Ò‰UÈ‰•¼şÿÿ‰Uğ‰Uä‰Uà‰Uø‰UĞ‰EØ»p I 3ÀUhã÷F dÿ0d‰ ‹EØÆ  ¸ü÷F è”eşÿ¡, I èÒùÿèİÿÿÆEï ÆEß ÆEİ ÆEŞ  o"I ˆE÷ÆE× 3À‰Eè²¸tF è€†şÿ‰Eü3ÀUh†÷F dÿ0d‰ 3ÀUhƒõF dÿ0d‰ Uø‹CèÊY  ƒ}ø u
¸(øF èn&şÿ‹EøèšBùÿƒø~
¸`øF èW&şÿUĞ‹Eøè@)  ‹EüŠm"I ˆP‹EüƒÀ‹˜"I è?Aùÿ‹EüƒÀ‹Uøè1Aùÿ€=l"I  t‹Eü€H €=n"I  t	‹Eü€Hë€=o"I  t‹Eü€H@‹Eü€Höƒ,  t‹Eü€Höƒ-  t‹Eü€HUè²”ÿÿYUèk•ÿÿY²¸\¤@ è7ùÿ‰EèèÎıÿÿèÿÿèşÿÿèÿÿèzğÿÿèÿÿƒ=¤"I  tj j ¡¤"I ‰EÌMÌfºŒ ‹Eüèı…şÿöƒ*  tj j 3À‰EÌMÌfº‹ ‹EüèÜ…şÿUè"ñÿÿYè´ÿÿ¡ğI èÿÿUè€¥ÿÿYèÿÿöƒ(  t¡I èïÿÿUèEöÿÿY¡ I èŞÿÿUè¤ËÿÿYènÿÿ€=j"I  t¡ôI è¿ÿÿUè}ÕÿÿYèOÿÿ¡"I ƒx t¡øI èÿÿUè@×ÿÿYè.ÿÿ¡"I ƒx t¡üI è}ÿÿUè“ÛÿÿYèÿÿ3Àèjÿÿè)ûÿÿèüÿÿ‹Eèƒx t¡I èMÿÿUè[îÿÿYèİÿÿöƒ(  t¡I è.ÿÿUèĞùÿÿYUèeúÿÿYöƒ*  t
U‹EĞè’—ÿÿYUèïğÿÿYj j EÈèæ’ÿÿ‹EÈ‰EÌMÌfº ‹Eüè„şÿöƒ*   •ÀPŠMŞ‹Uä‹Eüè ¢şÿ€=ğI  t
¡T I èZşÿ3Àè²ŒÿÿÆE×‹Eüèj†şÿ3ÀZYYd‰é¹  éè8ùÿ3ÀUh÷F dÿ0d‰ èı“ùÿº¨c@ èû5ùÿ„À…   EÈPèâ“ùÿ‹ •Àşÿÿèİ4ùÿ…Àşÿÿ‰EÀÆEÄUÀ3É¸øF è¥‚ùÿEÈP…¼şÿÿè6ïûÿ‹•¼şÿÿXè‚?ùÿ‹EÈèîaşÿÇ¬"I    3Ò¡(I è@Pûÿj jj±3Ò¡¼I è}  ë¸ÌøF è¸aşÿÇ¬"I    €}× …¿   ¸üøF èšaşÿ3ÀUh¦öF dÿ0d‰ ¡I è®‹ÿÿ¡PI ‹€Œ  3Òè\Súÿ¡PI ‹€´  3Òè†Súÿ¡PI ‹ÿRP3ÀZYYd‰ë
éÅ7ùÿèÜ:ùÿƒ}ğ t‹Eğè"xùÿ€}ïu
¡T I èxùÿ€}ß t‹Eäèxùÿ€}İ t‹Eàèöwùÿ3É3Ò‹Eüèvşÿ¡PI €x7 t
hÜ  èÙcùÿ3ÀZYYd‰ëéV7ùÿ3Ò¡(I è2Oûÿèa:ùÿè\:ùÿè‡:ùÿëfèP:ùÿ3ÀZYYd‰h÷F ƒ}è t.‹Eè‹XKƒû |‹Ó‹Eèèo¼ùÿºà‡F è%DùÿKƒûÿuæ‹EèèÛ3ùÿ‹EüèÓ3ùÿÃéÙ8ùÿë¼¸ùF èY`şÿ‹EØÆ 3ÀZYYd‰hê÷F …¼şÿÿèK<ùÿEÈèC<ùÿEĞè;<ùÿEàº   èN<ùÿEğè&<ùÿEøè<ùÿÃé|8ùÿëÀ_^[‹å]Ã   ÿÿÿÿ"   Starting the installation process.  ÿÿÿÿ.   Failed to get a non empty installation "AppId"  ÿÿÿÿ$   "AppId" cannot exceed 127 characters    ÿÿÿÿ3   Fatal exception during installation process (%s):
 ÿÿÿÿ'   User canceled the installation process. ÿÿÿÿ   Rolling back changes.   ÿÿÿÿ   Installation process succeeded. SV‹ò‹Ø‹Æ‹ÓèI;ùÿ»   ë/‹ŠDÿ<{u‹Ö‹Ë¸˜ùF è–>ùÿCë‹‹„I %ÿ   £sCC‹èì;ùÿ;Ø~Æ^[Ã ÿÿÿÿ   {   U‹ìƒÄäSVW3Û‰]ä‰]ô‹Ù‰Uü‹ğ3ÀUhíúF dÿ0d‰  u"I ˆEûUä¡$ I è½Éûÿ‹UäEô‹ÎèÜ;ùÿ‹EôPj jj ŠMû²¸,E è{şÿ‰Eğ3ÀUh¤úF dÿ0d‰ 3ÀUh“úF dÿ0d‰ è|ÿÿ‹Ó3ÉèZ‚ÿÿj ‹Eüö@L€•À4Pèú{ÿÿ‹Ó‹Mğèd„ÿÿöCDt‹C4‰Eè‹C8‰EìëEèPC4PèÃ_ùÿEèPj j ‹Eğ‹@PèG`ùÿ3ÀZYYd‰hšúF ‹EğèÆ0ùÿÃéÌ5ùÿëğ3ÀZYYd‰ëéÇ3ùÿ‹UôŠEûèPşÿè6ùÿèÎ6ùÿ‹Eü‹H<‹UôŠEûèMŒÿÿ3ÀZYYd‰hôúF Eäè9ùÿEôè9ùÿÃér5ùÿëè_^[‹å]ÃU‹ìƒÄìSVW3Ò‰Uô‰Uø‰Eü3ÀUhÆûF dÿ0d‰ Uø‹Eüèşÿÿ¡ü!I ‹pN…ö|PF3ÿ‹×¡ü!I è¸ùÿ‹Øƒ{8ÿt5Uô‹Cè¼Ìûÿ‹Eô‹Uøè¹nùÿ…Àu‹S8¡ "I è`¸ùÿ‹È‹Ó‹Eüèşÿÿë)GNu³EôP‹Eü‰EìÆEğUì3É¸ÜûF èá|ùÿ‹Eôè•şÿ3ÀZYYd‰hÍûF Eôº   è[8ùÿÃé™4ùÿëë_^[‹å]Ãÿÿÿÿ1   ExtractTemporaryFile: The file "%s" 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src/ppgltk/audio
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-03T03:13:45.611773Z
5
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

audio_data.h
file




2007-12-31T17:50:27.000000Z
ae629049a74fe1fb27a758f308593a6a
2007-09-03T03:13:45.611773Z
5
botsnlinux

audio.cpp
file




2007-12-31T17:50:27.000000Z
b8c6ad6d4dfcb619f4df6657fc27ed7e
2007-09-01T16:38:12.025871Z
2
botsnlinux

audio.h
file




2007-12-31T17:50:27.000000Z
92634286484933b65a0ce1f48730e04f
2007-09-03T03:13:45.611773Z
5
botsnlinux

audio_data.cpp
file




2007-12-31T17:50:27.000000Z
9e3fa739ad6343f3ede0e5a3e26c338d
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src/ppgltk/images
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

rgb_reader.h
file




2007-12-31T17:50:27.000000Z
efe7633cde54ab64acda4caed91e8c53
2007-09-01T16:38:12.025871Z
2
botsnlinux

image.cpp
file




2007-12-31T17:50:27.000000Z
861151946cb0bc6b17fa77910948ca2e
2007-09-01T16:38:12.025871Z
2
botsnlinux

png_reader.cpp
file




2007-12-31T17:50:27.000000Z
2b67a7645b42adb9c41d7c84a53720fa
2007-09-01T16:38:12.025871Z
2
botsnlinux

image.h
file




2007-12-31T17:50:27.000000Z
e6a5b5f69246edcbc1fff27302e2f2e2
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppm_writer.cpp
file




2007-12-31T17:50:27.000000Z
a652312c522836ace07464acc55d242e
2007-09-01T16:38:12.025871Z
2
botsnlinux

png_reader.h
file




2007-12-31T17:50:27.000000Z
7efc60da957b2b380af5e8d6deeed8b5
2007-09-01T16:38:12.025871Z
2
botsnlinux

rgb_reader.cpp
file




2007-12-31T17:50:27.000000Z
8b951212232406faa3805bd35adaab08
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppm_writer.h
file




2007-12-31T17:50:27.000000Z
3852bc380261186d00128b30ec5b6b15
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src/ppgltk/FT
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

FTTextureGlyph.cpp
file




2007-12-31T17:50:27.000000Z
1608f1ed70adbba480d147b1a4528b24
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTLibrary.cpp
file




2007-12-31T17:50:27.000000Z
a18eac87208ac9f6b08832a1f6557fa0
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTFont.cpp
file




2007-12-31T17:50:27.000000Z
f6ce4de8457b52f0a5ec015a5dd9192a
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTVector.h
file




2007-12-31T17:50:27.000000Z
78aeb3b8a0de3791293c107d17545b60
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGL.h
file




2007-12-31T17:50:27.000000Z
396eec620ba70244b87acfa8aec96080
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTTextureGlyph.h
file




2007-12-31T17:50:27.000000Z
1a33a9f0daa0cd2eac88b123fda6cd37
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTLibrary.h
file




2007-12-31T17:50:27.000000Z
16078f4cbab0152bea36f47b21c145bc
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTSize.cpp
file




2007-12-31T17:50:27.000000Z
d7e38f76d756d428e51dad0b24f91ca2
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGLTextureFont.cpp
file




2007-12-31T17:50:27.000000Z
7aef6c06de4bad2d4a5e404ab120f764
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTCharmap.cpp
file




2007-12-31T17:50:27.000000Z
8ea7f50da6aa9ad1eb1173f75bd42628
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTFont.h
file




2007-12-31T17:50:27.000000Z
c1d15f170e657327d494783b3c0422f0
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTSize.h
file




2007-12-31T17:50:27.000000Z
1af80bb01a148a5eb246486ffa1fafe2
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTCharToGlyphIndexMap.h
file




2007-12-31T17:50:27.000000Z
8c6f0d45251815ffd90f92221004e7d3
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGLTextureFont.h
file




2007-12-31T17:50:27.000000Z
13aa592438a6547558f50c95e6d25217
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTCharmap.h
file




2007-12-31T17:50:27.000000Z
0375b5a3df7af4ec7eb6c001decb9dc9
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTList.h
file




2007-12-31T17:50:27.000000Z
77dda7a2925184191c52d3acfec11e8a
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGlyph.cpp
file




2007-12-31T17:50:27.000000Z
15dcd2e5930cf3257a3d35c1cd5a1eeb
2007-09-01T16:38:12.025871Z
2
botsnlinux

license.txt
file




2007-12-31T17:50:27.000000Z
82ad89e8cc84873656763d3695e8d404
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGlyphContainer.cpp
file




2007-12-31T17:50:27.000000Z
18af386a8edbcb4ac6a1f778b764cfaf
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGlyph.h
file




2007-12-31T17:50:27.000000Z
8b4615a984d6f7e916c8ef3481e4a9cd
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTPoint.cpp
file




2007-12-31T17:50:27.000000Z
02c0558025e690ec97df0ad239534a9d
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTGlyphContainer.h
file




2007-12-31T17:50:27.000000Z
8c9c52e55c40be1d71591508135c3647
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTFace.cpp
file




2007-12-31T17:50:27.000000Z
657bd42ce7bc275947712c4e647248e6
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTPoint.h
file




2007-12-31T17:50:27.000000Z
c8fa1f244fcf13f8a57312f9531b2faf
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTBBox.h
file




2007-12-31T17:50:27.000000Z
9099e0292c98852f3fea9f78d9d2e3d1
2007-09-01T16:38:12.025871Z
2
botsnlinux

README.txt
file




2007-12-31T17:50:27.000000Z
82162c91e44cf0470b71132c856733b5
2007-09-01T16:38:12.025871Z
2
botsnlinux

FTFace.h
file




2007-12-31T17:50:27.000000Z
1f1aac9426adfed00866dca12126f592
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                    ‹Ëº¸,G èH¤ûÿSUü¡X"I èùÿÿ‹Mü¸h,G ºÌ,G è@£ûÿ3ÀZYYd‰hI,G Eüè¿ùÿÃéùÿëğ_^[Y]Ã ÿÿÿÿ   Lang    ÿÿÿÿ   Setup   ÿÿÿÿ   Dir ÿÿÿÿ   Group   ÿÿÿÿ   NoIcons ÿÿÿÿ	   SetupType   ÿÿÿÿ
   Components  ÿÿÿÿ   Tasks   SVWUƒÄô‰T$‰$ÆD$ ¡ä!I ‹pN…ö|DF3ÿ‹×¡ä!I èÑ†ùÿ‹Ø‹$‹è=ùÿ…Àu!‹kƒıÿt;-@I uÆD$‹D$‹SèùÿGNu¿ŠD$ƒÄ]_^[ÃU‹ìƒÄôSVW3Ò‰Uü‹Ø3ÀUhš-G dÿ0d‰ EüP‰]ôÆEøUô3É¸°-G èKùÿ‹Eüè¼ëıÿ3ÀZYYd‰h¡-G EüègùÿÃéÅùÿëğ_^[‹å]Ãÿÿÿÿ.   Cannot evaluate "%s" constant during Uninstall  U‹ì3ÉQQQQSVW‹ò‰Eü‹Eüè/	ùÿ3ÀUhÒ.G dÿ0d‰ Eü¹   º   èŸ	ùÿ‹Uü°|è‘¨ûÿ‹Ø…Ûu‹EüèCùÿ‹ØCEøP‹ËIº   ‹Eüè0	ùÿEôPS¹ÿÿÿ‹Eüè	ùÿ‹ÆèùÿEøèE§ûÿ„ÀtHEôè9§ûÿ„Àt<EğP‹E‹Hø‹E‹Pü‹Eøè  ‹Eğ‹Öè?¢ûÿƒ> uV‹E‹Hø‹E‹Pü‹Eôèõ  3ÀZYYd‰hÙ.G Eğº   èOùÿÃéùÿëë_^[‹å]Ãÿÿÿÿ   HKCR    ÿÿÿÿ   HKCU    ÿÿÿÿ   HKLM    ÿÿÿÿ   HKU ÿÿÿÿ   HKCC    U‹ìƒÄØSVW3É‰MØ‰Mô‰Mğ‰Mì‰Mè‰Uø‰Eü‹EüèÔùÿ3ÀUhÉ1G dÿ0d‰ Eü¹   º   èDùÿ‹Uü°\è6§ûÿ‹ğ…ö„  EôP‹ÎIº   ‹EüèÜùÿƒ}ô „î   DI ˆEç‹Eôè¾ùÿ‹Øƒû|_‹Eô€|ş3u‹Eô€|ÿ2uÆEç‹ÓƒêEôèÂùÿë8‹Eô€|ş6u.‹Eô€|ÿ4u$€=l"I  u
¸à1G è5éıÿÆEç‹ÓƒêEôèˆùÿ3À‰Eà¿   »LI ‹Uô‹èç9ùÿ…Àu‹C‰EàëƒÃOuäƒ}à „C  EôPV¹ÿÿÿ‹Eüèùÿ‹Uô°|èE¦ûÿ‹ğ…öu‹Eôè÷ùÿ‹ğFEèPV¹ÿÿÿ‹Eôèäùÿ‹ÖJEôèùÿ‹Uô°,è¦ûÿ‹ğ…ö„á   EğP‹ÎIº   ‹Eôè±ùÿEìPV¹ÿÿÿ‹EôèùÿEğèÍ¤ûÿ„À„©   Eìè½¤ûÿ„À„™   Eèè­¤ûÿ„À„‰   ‹EøP‹E‹Hø‹E‹Pü‹Eèè‰  j jEÜPEØP‹E‹Hø‹E‹Pü‹Eğèi  ‹EØèíùÿ‹È‹UàŠEçèÜªûÿ…ÀuBEØP‹E‹Hø‹E‹Pü‹Eìè8  ‹EØè¼ùÿ‹Ğ‹Mø‹EÜèÛ©ûÿ‹EÜPè&ùÿë
¸42G è¢çıÿ3ÀZYYd‰hĞ1G EØèMùÿEèº   è`ùÿEüè8ùÿÃé–şøÿëÛ_^[‹å]Ã ÿÿÿÿI   Cannot access a 64-bit key in a "reg" constant on this version of Windows   ÿÿÿÿ   Failed to parse "reg" constant  U‹ì3ÉQQQQQQQSVW‹ò‰Eü‹Eüè¸ùÿ3ÀUh]4G dÿ0d‰ Eü¹   º   è(ùÿ‹Uü°,è¤ûÿ‹Ø…Û„”  EøP‹ËIº   ‹EüèÀùÿƒ}ø „v  Eô‹UøèÇùÿEøPS¹ÿÿÿ‹Eüè—ùÿ‹Uø°|èÉ£ûÿ‹Ø…Ûu‹Eøè{ùÿ‹ØCEèPS¹ÿÿÿ‹Eøèhùÿ‹ÓJEøè‰ùÿ‹Uø°,è£ûÿ‹Ø…Û„	  EğP‹ËIº   ‹Eøè5ùÿEìPS¹ÿÿÿ‹Eøè!ùÿEôèQ¢ûÿ„À„Ñ   EğèA¢ûÿ„À„Á   Eìè1¢ûÿ„À„±   Eèè!¢ûÿ„À„¡   EäP‹E‹Hø‹E‹Pü‹Eôèı  ‹UäEôèÚ ùÿEäP‹E‹Hø‹E‹Pü‹EğèÚ  ‹UäEğè· ùÿEäP‹E‹Hø‹E‹Pü‹Eìè·  ‹UäEìè” ùÿEäP‹E‹Hø‹E‹Pü‹Eèè”  ‹UäEèèq ùÿ‹EôPV‹Mè‹Uì‹Eğè2˜ûÿë
¸t4G èşäıÿ3ÀZYYd‰hd4G Eäº   èÄÿøÿÃéüøÿëë_^[‹å]Ã ÿÿÿÿ   Failed to parse "ini" constant  U‹ìƒÄğSVW3Û‰]ğ‰]ô‰Mø‰Uü‹Ø3ÀUht5G dÿ0d‰ èûÿ‹ğ…ö„   ¿   Uô‹ÇèGûÿ‹Ãè” ùÿƒÀPºŒ5G Eğè£ÿøÿEğ‹Óè ùÿEğº˜5G èt ùÿ‹Eğè(ùÿP‹EôèùÿZYèÈ>ùÿ…Àu&‹ÃèI ùÿ‹ÈƒÁEôº   è{ùÿ‹Eø‹UôèLÿøÿëGNu‹Eø‹Uüè;ÿøÿ3ÀZYYd‰h{5G Eğº   è­şøÿÃéëúøÿëë_^[‹å]Ã  ÿÿÿÿ   /   ÿÿÿÿ   =   U‹ì3ÉQQQQQSVW‹ò‰Eü‹Eüèrùÿ3ÀUh¸6G dÿ0d‰ Eü¹   º   èâùÿEø‹Uüè³şøÿ‹Uø°|èÉ ûÿ‹Ø…Ûu‹Eøè{ÿøÿ‹ØCEğPS¹ÿÿÿ‹Eøèhùÿ‹ÓJEøè‰ùÿEô‹UøènşøÿEôè‚Ÿûÿ„ÀtaEğèvŸûÿ„ÀtUEìP‹E‹Hø‹E‹Pü‹EôèV  ‹UìEôè3şøÿEìP‹E‹Hø‹E‹Pü‹Eğè3  ‹UìEğèşøÿ‹Î‹Uğ‹Eôèşÿÿë
¸Ğ6G è£âıÿ3ÀZYYd‰h¿6G Eìº   èiıøÿÃé§ùøÿëë_^[‹å]Ã  ÿÿÿÿ    Failed to parse "param" constant    U‹ìƒÄøSVW‹ñ‹Ú‹øƒ=´"I  tj jSV‰]øÆEüMø‹×¡´"I èxˆ ë¸L7G èâıÿ‹ÆèÅüøÿ_^[YY]Ã  ÿÿÿÿ"   "code" constant with no CodeRunner  U‹ì3ÉQQQQQSVW‹ò‰Eü‹Eüèÿøÿ3ÀUhˆ8G dÿ0d‰ €=¨"I  t¹ 8G ²¸Xc@ èêSùÿèeùøÿEü¹   º   èïÿøÿEø‹UüèÀüøÿ‹Uø°|èÖûÿ‹Ø…Ûu‹Eøèˆıøÿ‹ØCEğPS¹ÿÿÿ‹Eøèuÿøÿ‹ÓJEøè– ùÿEô‹Uøè{üøÿEôèûÿ„Àt>Eğèƒûÿ„Àt2EìP‹E‹Hø‹E‹Pü‹Eğèc  ‹UìEğè@üøÿ‹Î‹Uğ‹Eôè“şÿÿë
¸ì8G èÓàıÿ3ÀZYYd‰h8G Eìº   è™ûøÿÃé×÷øÿëë_^[‹å]Ã  ÿÿÿÿ@   Cannot evaluate "code" constant because of possible side effects    ÿÿÿÿ   Failed to parse "code" constant U‹ìƒÄøSVW3É‰Mø‹Ú‰Eü‹Eüèşøÿ3ÀUhœ9G dÿ0d‰ Eü¹   º   èqşøÿEüèaœûÿ„Àt$EøP‹E‹Hø‹E‹Pü‹EüèA  ‹Eø‹Óè?ûÿë
¸´9G è¿ßıÿ3ÀZYYd‰h£9G Eøº   è…úøÿÃéÃöøÿëë_^[YY]Ã  ÿÿÿÿ    Failed to parse "drive" constant    U‹ìƒÄÈSVW3É‰MĞ‰Mø‹ú‰Eü‹Eüè2ıøÿ¹	   EÔº|@ èxÿøÿ3ÀUh„;G dÿ0d‰ €=t"I  t
¸œ;G èóÿÿEü¹   º   è}ıøÿ‹Uü°,èoœûÿ‹ğ…öuEø‹Uüè>úøÿëEøP‹ÎIº   ‹Eüèıøÿ3ÛëEü‹Îº   è9ıøÿ‹Uü°,è+œûÿ‹ğ…öuDÔ‹UüèùùøÿëDÔP‹ÎIº   ‹EüèÆüøÿDÔèõšûÿ„Àu
¸¨;G èwŞıÿEĞP‹E‹Hø‹E‹Pü‹DÔèÊ  ‹UĞDÔè¦ùøÿC…ö~	ƒûtÿÿÿ‹×‹EøèÎñÿÿ„Àu%EĞP‹Eø‰EÈÆEÌUÈ3É¸Ğ;G è]=ùÿ‹EĞèŞıÿEĞP‹èşûøÿ‹ËIUÔèçÈıÿ‹UĞ‹ÇèEùøÿ3ÀZYYd‰h‹;G EĞèœøøÿEÔ¹	   º|@ èÂşøÿEøº   èøøÿÃéÛôøÿëÑ_^[‹å]Ã  ÿÿÿÿ   cm  ÿÿÿÿ   Failed to parse "cm" constant   ÿÿÿÿ1   Unknown custom message name "%s" in "cm" constant   ÿÿÿÿ   userdesktop ÿÿÿÿ   userstartmenu   ÿÿÿÿ   userprograms    ÿÿÿÿ   userstartup ÿÿÿÿ   sendto  ÿÿÿÿ   fonts   ÿÿÿÿ   userappdata ÿÿÿÿ   userdocs    ÿÿÿÿ   usertemplates   ÿÿÿÿ   userfavorites   ÿÿÿÿ   localappdata    ÿÿÿÿ   commondesktop   ÿÿÿÿ   commonstartmenu ÿÿÿÿ   commonprograms  ÿÿÿÿ   commonstartup   ÿÿÿÿ   commonappdata   ÿÿÿÿ
   commondocs  ÿÿÿÿ   commontemplates ÿÿÿÿ   commonfavorites ÿÿÿÿ   src ÿÿÿÿ   srcexe  ÿÿÿÿ   userinfoname    ÿÿÿÿ   userinfoorg ÿÿÿÿ   userinfoserial  ÿÿÿÿ   hwnd    ÿÿÿÿ
   wizardhwnd  U‹ìƒÄäSVW3Û‰]ì‰]ô‰Mø‰Uü‹ø3ÀUh¦FG dÿ0d‰ €=t"I  t"¾   »ÌI ‹‹×èøøÿu‹èÌîÿÿƒÃNuè‹ÇºÀFG èú÷øÿu‹EºÀFG èûõøÿéê  ‹ÇºÌFG èÚ÷øÿuU€=t"I  t&ƒ=\ I  u
¸ØFG èxÚıÿ‹E‹\ I è¾õøÿé­  ƒ=@"I  u
¸8GG èRÚıÿ‹E‹@"I è˜õøÿé‡  ‹ÇºŒGG èw÷øÿu‹E‹( I èwõøÿéf  ‹Çº˜GG èV÷øÿu‹E‹, I èVõøÿéE  ‹Çº¤GG è5÷øÿuBƒ=0 I  t‹E‹0 I è,õøÿé  €=l"I  t
¸¸GG èÀÙıÿ‹E‹, I èõøÿéõ  ‹ÇºHG èåöøÿu‹E‹  I èåôøÿéÔ  ‹ÇºHG èÄöøÿu‹E‹”I èÄôøÿé³  ‹Çº(HG è£öøÿu‹E‹$ I è£ôøÿé’  ‹Çº4HG è‚öøÿu‹E‹4 I è‚ôøÿéq  ‹Çº@HG èaöøÿu/€=m"I  t‹E‹@ I èXôøÿéG  ‹E‹8 I èEôøÿé4  ‹ÇºLHG è$öøÿu/€=m"I  t‹E‹D I èôøÿé
  ‹E‹< I èôøÿé÷  ‹ÇºXHG èçõøÿu‹E‹8 I èçóøÿéÖ  ‹ÇºhHG èÆõøÿu‹E‹< I èÆóøÿéµ  ‹ÇºxHG è¥õøÿu+€=l"I  t‹E‹@ I èœóøÿé‹  ¸ˆHG è9Øıÿé|  ‹ÇºÌHG èlõøÿu+€=l"I  t‹E‹D I ècóøÿéR  ¸ÜHG è ØıÿéC  ‹Çº IG è3õøÿu‹U¸,IG è,  é#  ‹ÇºPIG èõøÿu‹E‹H I èóøÿé  ‹Çº\IG èòôøÿu‹EèX÷ıÿéç  ‹ÇºtIG è×ôøÿu‹Eèy÷ıÿéÌ  ‹ÇºˆIG è¼ôøÿuU€=t"I  t&ƒ=d I  u
¸œIG èZ×ıÿ‹E‹d I è òøÿé  ƒ=D"I  u
¸ôIG è4×ıÿ‹E‹D"I èzòøÿéi  ‹ÇºPJG èYôøÿu‹E‹L I èYòøÿéH  ‹ÇºhJG è8ôøÿu‹E‹P I è8òøÿé'  ‹Çº€JG èôøÿu‹E‹4"I èòøÿé  ‹Çº˜JG èöóøÿu‹E‹8"I èöñøÿéå  ‹Çº¬JG èÕóøÿu‹E‹<"I èÕñøÿéÄ  ‹ÇºÄJG è´óøÿu‹E‹T I è´ñøÿé£  ‹ÇºÜJG è“óøÿ…Œ   €=t"I  t&ƒ=` I  u
¸ìJG è-Öıÿ‹E‹` I èsñøÿéb  ƒ=D"I  u
¸@KG èÖıÿEôPö˜!I €•À43É²èÚ  ƒ}ô u
¸˜KG èŞÕıÿUì‹Eôè'€ûÿ‹Uì‹E‹D"I èBòøÿé  ‹ÇºÄKG èõòøÿu;€=t"I  t‹E‹h I èìğøÿéÛ  ‹@I ¡à!I èpùÿ‹‹EèÍğøÿé¼  ‹ÇºØKG è¬òøÿu2ƒ=ŒI  t¡ŒI è‹=úÿ‹Uè)ùÿé  ‹EºèKG èğøÿé|  ‹ÇºôKG èlòøÿu2ƒ=PI  t¡PI èK=úÿ‹UèÃ(ùÿéN  ‹EºèKG èMğøÿé<  ‹ÇºLG è,òøÿu‹Eè^şÿé!  …ÿt!€?%uUUì‹ÇèeéÿÿY‹Uì‹Eèğøÿéü  ‹Çè¥òøÿ‹Ø‹ÃºLG ¹   è/ùÿ…ÀuUUì‹ÇèwêÿÿY‹Uì‹EèÓïøÿéÂ  ‹ÃºLG ¹   èé.ùÿ…ÀuUUì‹ÇèníÿÿY‹Uì‹Eè¢ïøÿé‘  ‹ÃºLG ¹   è¸.ùÿ…ÀuUUì‹Çè…ğÿÿY‹Uì‹Eèqïøÿé`  ‹Ãº$LG ¹   è‡.ùÿ…ÀuUUì‹Çè(òÿÿY‹Uì‹Eè@ïøÿé/  ‹Ãº,LG ¹   èV.ùÿ…ÀuUUì‹Çè“óÿÿY‹Uì‹Eèïøÿéş   ‹Ãº4LG ¹   è%.ùÿ…ÀuUUì‹Çè.ôÿÿY‹Uì‹EèŞîøÿéÍ   ÆEó »tI ÆEò ‹ó‹Ç‹è±ğøÿuFEôP3ÉŠUòŠEóèF  ƒ}ô u"EìP‰}äÆEèUä3É¸@LG è~2ùÿ‹Eìè2Óıÿ‹E‹Uôè{îøÿëmşEòƒÆ€}òu£şEóƒÃ,€}óu‘…ÿt/3ö;uø}(‹Ç‹Uü‹²è9ğøÿu‹E‹Uü‹T²è8îøÿë*ƒÆ;uø|ØEìP‰}äÆEèUä3É¸tLG è2ùÿ‹Eìè¸Òıÿ3ÀZYYd‰h­FG EìècíøÿEôè[íøÿÃé¹éøÿëè_^[‹å]Â   ÿÿÿÿ   \   ÿÿÿÿ   app ÿÿÿÿV   An attempt was made to expand the "app" constant but Setup didn't create the "app" dir  ÿÿÿÿJ   An attempt was made to expand the "app" constant before it was initialized  ÿÿÿÿ   win ÿÿÿÿ   sys ÿÿÿÿ   syswow64    ÿÿÿÿH   Cannot expand "syswow64" constant because there is no SysWOW64 directory    ÿÿÿÿ   src ÿÿÿÿ   srcexe  ÿÿÿÿ   tmp ÿÿÿÿ   sd  ÿÿÿÿ   pf  ÿÿÿÿ   cf  ÿÿÿÿ   pf32    ÿÿÿÿ   cf32    ÿÿÿÿ   pf64    ÿÿÿÿ8   Cannot expand "pf64" constant on this version of Windows    ÿÿÿÿ   cf64    ÿÿÿÿ8   Cannot expand "cf64" constant on this version of Windows    ÿÿÿÿ   dao ÿÿÿÿ   {cf}\Microsoft Shared\DAO   ÿÿÿÿ   cmd ÿÿÿÿ   computername    ÿÿÿÿ   username    ÿÿÿÿ	   groupname   ÿÿÿÿO   Cannot expand "groupname" constant because it was not available at install time ÿÿÿÿP   An attempt was made to expand the "groupname" constant before it was initialized    ÿÿÿÿ   sysuserinfoname ÿÿÿÿ   sysuserinfoorg  ÿÿÿÿ   userinfoname    ÿÿÿÿ   userinfoorg ÿÿÿÿ   userinfoserial  ÿÿÿÿ   uninstallexe    ÿÿÿÿ   group   ÿÿÿÿK   Cannot expand "group" constant because it was not available at install time ÿÿÿÿL   An attempt was made to expand the "group" constant before it was initialized    ÿÿÿÿ!   Failed to expand "group" constant   ÿÿÿÿ   language    ÿÿÿÿ   hwnd    ÿÿÿÿ   0   ÿÿÿÿ
   wizardhwnd  ÿÿÿÿ   log reg:    ini:    param:  code:   drive:  cm: ÿÿÿÿ+   Failed to expand shell folder constant "%s" ÿÿÿÿ   Unknown constant "%s"   SVQ‹ò‹ØV3À‰D$T$3É‹Ãè   Z^[ÃU‹ìƒÄğSVW3Û‰]ô‰]ğ‰Mø‰Uü‹Ø‹u3ÀUhÿMG dÿ0d‰ ‹Æ‹Óèµçøÿ»   éì   ‹€|ÿ{…Ç   ‹èwèøÿ;Ø}‹€<{uC‹Æ¹   ‹Óè êøÿé¸   ‹û‹‹Óèˆûÿ‹Ø…Ûu
¸NG èÌıÿKEôPG‹Ë+ÈW‹è*êøÿEğP‹Uü‹Mø‹EôèÌğÿÿK+Ï‹Æ‹×èJêøÿ‹Ö‹Ï‹Eğè†êøÿ‹Eğèòçøÿ‹Øßƒ}ğ tI‹EğèĞzûÿ€8\u<‹èÔçøÿ;Ø1‹€|ÿ\u(‹Æ¹   ‹Óèıéøÿë‹ŠDÿ‹„I %ÿ   £sCC‹è˜çøÿ;Øÿÿÿ3ÀZYYd‰hNG Eğº   è"æøÿÃé`âøÿëë_^[‹å]Â  ÿÿÿÿ   Unclosed constant   U‹ìQSVW‹ñ‹ú‹Øj jEüP¹ˆNG º  €‹Ãèâûÿ…Àu"‹Î‹×‹Eüèûÿ„Àu‹Æè“åøÿ‹EüPè	ùÿë‹Æèåøÿ_^[Y]Ã   Software\Microsoft\Windows\CurrentVersion   Software\Microsoft\Windows\CurrentVersion   SOFTWARE\Microsoft\Windows NT\CurrentVersion    U‹ìQS€=l"I  t³ë3Ûj jEüP3À k"I ‹…èI º  €‹ÃèîŒûÿ…Àu-¹L I º|OG ‹EüèŒûÿ¹P I ºŒOG ‹Eüèö‹ûÿ‹EüPèùÿ[Y]Ã RegisteredOwner RegisteredOrganization  U‹ìj SVW3ÀUh¬QG dÿ0d‰ Eüèª‡ûÿ‹Uü¸( I èäøÿEüèÁ‡ûÿ‹Uü¸, I èläøÿEüèØ‡ûÿ‹Uü¸0 I èWäøÿƒ=Ü I uUü¸ÄQG èÍ€ûÿ‹Uü¸4 I è4äøÿë
¸4 I èØãøÿƒ=4 I  u2Uü¡( I èvwûÿ‹Uü¸4 I èäøÿƒ=4 I  u¸4 I ºØQG èíãøÿMüºÜQG °èºıÿÿ‹Uü¸8 I èÑãøÿƒ=8 I  u¸8 I ¹ôQG ‹4 I è#åøÿMüºRG °è€ıÿÿ‹Uü¸< I è—ãøÿƒ=< I  uUü¡8 I èÅrûÿ‹Uü¸< I ¹RG èßäøÿ€=l"I  t^MüºÜQG °è3ıÿÿ‹Uü¸@ I èJãøÿƒ=@ I  u
¸4RG è'ÈıÿMüºRG °èıÿÿ‹Uü¸D I èãøÿƒ=D I  u
¸tRG èøÇıÿ€=k"I  t!Uü¡, I è6rûÿ‹Uü¸H I ¹°RG èPäøÿëUü¡( I èrûÿ‹Uü¸H I ¹ÀRG è/äøÿèzıÿÿ3ÀZYYd‰h³QG EüèUâøÿÃé³Şøÿëğ_^[Y]Ã   ÿÿÿÿ   SystemDrive ÿÿÿÿ   C:  ProgramFilesDir ÿÿÿÿ   \Program Files  CommonFilesDir  ÿÿÿÿ   Common Files    ÿÿÿÿ4   Failed to get path of 64-bit Program Files directory    ÿÿÿÿ3   Failed to get path of 64-bit Common Files directory ÿÿÿÿ   cmd.exe ÿÿÿÿ   COMMAND.COM U‹ìƒÄèSVW3É‰Mü‰Mì‰Mè3ÉUhšSG dÿ1d‰!3ÉUhSG dÿ1d‰!è'|ùÿ3ÀZYYd‰ëlé$Üøÿ   @¡@ SG èùÿ‹ØEüP¡øI ‰EğUì‹Ãè¿ùÿ‹Eì‰EôUè‹Ãè“ûÿ‹Eè‰EøUğ¹   °`èÎ±ıÿ‹Mü²¸Xc@ è/8ùÿèªİøÿèŞøÿ3ÀZYYd‰h¡SG Eèº   èàøÿEüègàøÿÃéÅÜøÿëã_^[‹å]ÃU‹ìQS‹ÚPj
‹I ²¸P©@ è}ùÿ‰Eü3ÀUhõSG dÿ0d‰ ‹Ó‹Eüèíşÿÿ3ÀZYYd‰hüSG ‹Eüèd×øÿÃéjÜøÿëğ[Y]ÃU‹ì¹   j j IuùSVW3ÀUhsUG dÿ0d‰ EğèøËıÿ‹Uğ¸$ I èàøÿºŒUG EğèRàøÿEğ‹$ I è,áøÿ‹Eğè˜şÿ€=ğI  t
¡$ I è}ûıÿUğ¡$ I èoûÿ‹UğEü¹´UG è8áøÿj ‹Eüè¦âøÿPèlùÿ…Àuaèƒùÿ‹ØEğPMà‹Uü°/è¨°ıÿ‹Eà‰EäUÜ‹Ãè4ùÿ‹EÜ‰EèUØ‹Ãè|‘ûÿ‹EØ‰EìUä¹   °`èC°ıÿ‹Mğ²¸Xc@ è¤6ùÿèÜøÿEô¹ÄUG ‹Uüè³àøÿ‹Uô¸ÜUG èşÿÿEøèêşÿƒ}ø t#Eô¹ğUG ‹Uüèˆàøÿ‹Uô‹Eøèeşÿÿ‹Eôè-şÿ3ÀZYYd‰hzUG EØº   è»ŞøÿEğº   è®ŞøÿÃéìÚøÿëŞ_^[‹å]Ã   ÿÿÿÿ   Created temporary directory:    ÿÿÿÿ   _isetup ÿÿÿÿ   \_RegDLL.tmp    ÿÿÿÿ
   REGDLL_EXE  ÿÿÿÿ   \_setup64.tmp   SVWQ‹ù‹òˆ$‹ÖŠ$èï¸ıÿ‹Ø÷ÛÛ÷Û„Ûu+èşùÿƒøt!èôùÿƒøtèbùÿ+Ç=Ğ  s	j2èšùÿë¿‹ÃZ_^[ÃU‹ìj SVW3ÀUhîVG dÿ0d‰ 3Àèşÿ3Àè şÿƒ=$ I  tW€=ğI  t3ÀèWùıÿjjj h VG èûùÿP±‹$ I 3Àè'Ïıÿ„Àu#ºWG EüèÒİøÿEü‹$ I è¬Şøÿ‹Eüèşÿ3ÀZYYd‰hõVG EüèİøÿÃéqÙøÿëğ_^[Y]Ã ÿÿÿÿ&   Failed to remove temporary directory:   U‹ìƒÄàSVW3À‰Eè‰Eü3ÀUhgXG dÿ0d‰ Uè¡$ I è<lûÿ‹UèEü¹€XG èXŞøÿ‹Uü¸ XG è3üÿÿUì‹EüèT´ıÿ„Àu
¸´XG è²ÁıÿUô¸ìXG è9´ıÿ„Àt‹Eô;Eìw ‹Eô;Eìu‹Eø;Eğw‹Eô;Eìu‹Eø;EğuEüºìXG èÀÜøÿº €  ¸YG èí‰ûÿº €  ‹Eüèà‰ûÿ£(#I ƒ=(#I  u%EèP‹Eü‰EàÆEäUà3É¸YG èi ùÿ‹EèèÁıÿh0YG ¡(#I Pè1ùÿ‹Ø‰,#I …Ûu
¸LYG è÷Àıÿ3ÀZYYd‰hnXG Eèè¢ÛøÿEüèšÛøÿÃéø×øÿëè_^[‹å]Ã   ÿÿÿÿ   _isetup\_shfoldr.dll    ÿÿÿÿ   SHFOLDERDLL ÿÿÿÿ-   Failed to get version numbers of _shfoldr.dll   ÿÿÿÿ   shfolder.dll    ÿÿÿÿ   shell32.dll ÿÿÿÿ   Failed to load DLL "%s" SHGetFolderPathA    ÿÿÿÿ2   Failed to get address of SHGetFolderPathA function  3À£,#I ƒ=(#I  t¡(#I Pèùşøÿ3À£(#I ÃU‹ìÄäşÿÿSVW3Û‰øşÿÿ‰ôşÿÿ‹ù‹ğ3ÀUh½ZG dÿ0d‰ „ÒtÎ €  ¡|"I Áè=   u'÷Æ €  t…üşÿÿPj j ‹Æ%ÿÿÿPj ÿ,#I ‹Øë»  €…Ût…üşÿÿPj j Vj ÿ,#I ‹Ø…Ûu6…ôşÿÿ•üşÿÿ¹  èÛøÿ‹…ôşÿÿ•øşÿÿè’lûÿ‹…øşÿÿ‹×è-pûÿë6‹ÇèÙøÿ‰äşÿÿÆ…èşÿÿ ‰µìşÿÿÆ…ğşÿÿ •äşÿÿ¹   ¸ÔZG èQÿıÿ3ÀZYYd‰hÄZG …ôşÿÿº   èdÙøÿÃé¢Õøÿëè_^[‹å]Ã ÿÿÿÿA   Warning: SHGetFolderPath failed with code 0x%.8x on folder 0x%.4x   U‹ìj SVW‹Ø3ÀUhá[G dÿ0d‰ ‹E¶xÿ3ÀŠÃkğ†#I €<8 u^‹EµğI ‹¸‹UŠRş€òMüè<şÿÿ‹E¶@ÿµ¸"I ‚‹UüèÏØøÿ‹E€xş tƒ}ü t‹E¶@ÿ3ÒŠÓkÒÂ#I Æ‹E‹@‹U¶Rÿ3ÉŠËkÉ¸"I ‹‘èÉØøÿ3ÀZYYd‰hè[G Eüè ØøÿÃé~Ôøÿëğ_^[Y]Ã‹ÀU‹ìQSˆMşˆUÿ‹Ø‹Eèû×øÿU‹ÃèÿÿÿY‹Eƒ8 ”À„Øt	U3ÀèõşÿÿY[Y]Â ‹ÀU‹ìQSˆMşˆUÿ‹Ø„Ût€=n"I  t	€=k"I  uŠEÿú,r3Û‹EPŠMşŠUÿ‹Ãè…ÿÿÿ[Y]Â @ U‹ìƒÄôS‹|"I ‰Mü€=k"I  t‹H‰Mø‹J‰Môë
‹‰Mø‹
‰Mô3Éƒ}ø u±ëq‹]ü;]ør €=k"I  tf‹]şf;]úuf‹@f;€"I v±ëEƒ}ô t?‹Eüfƒ}ô u%  ÿÿ€=k"I  u	;Eôr"±ë;Eôwf‹Eşf;Eöuf‹Bf;€"I w±‹Á[‹å]Ã@ U‹ìƒÄôSVW‰Mü‹Ø‹Eü‹
‰‹J‰H²¸Ğ¦@ èİÍøÿ‰Eø3ÀUh ^G dÿ0d‰ ‹Ó‹Eø‹ÿQ,¡ü!I ‹xO…ÿ|oGÇEô    ‹Uô¡ü!I èDVùÿ‹ğ‹Şƒ{ tJƒ{ uDƒ{ u>j‹Î3Ò‹Eøè0Æÿÿ„Àt,‹s8ƒşÿt‹Ö¡ "I èVùÿP‹Eüè¥ûÿëS@‹Eüè¥ûÿÿEôOu™3ÀZYYd‰h^G ‹EøèYÍøÿÃé_Òøÿëğ_^[‹å]Ã‹ÀSVWUƒÄô‰L$ˆT$‰$‹D$3Ò‰P‹D$3Ò‰²¸Ğ¦@ èñÌøÿ‹ğ¡ğ!I ‹xO…ÿ|bG3í‹Õ¡ğ!I èwUùÿ‹Ø‹S‹ÆèCÈÿÿ€|$ t"öC5t6‹$‹Æèğ¶şÿ„Àt(S6‹D$èä¤ûÿë‹$‹ÆèÔ¶şÿ„ÀtS6‹D$èÈ¤ûÿEOu¡‹Æè¡ÌøÿƒÄ]_^[ÃSV‹Ø‹3÷Æ   t*÷Æ   u"º_G C,è¹ùÿ…Àtº_G C,è¨ùÿ…Àu3À^[Ã°^[Ã  .   ..  U‹ìj SVW‹ò‹Ø3ÀUhP_G dÿ0d‰ Uü‹Ãè¸iûÿ‹Uü‹Æ‹ÿQ,3ÀZYYd‰hW_G Eüè±ÔøÿÃéÑøÿëğ_^[Y]Ã@ U‹ìƒÄğSVW3Ò‰Uø‰Eü‹Eüè¯×øÿ3ÀUhM`G dÿ0d‰ è˜zûÿ„Àt!€=u"I  u.Uø‹EüèŒÁıÿ‹UøEüèåÔøÿëUø‹Eüè\wûÿ‹UøEüèÍÔøÿUø‹Eüèiûÿ‹UøEüè·Ôøÿ‹E‹@ü‹ÿR‹ØK…Û|AC3öMø‹E‹@ü‹Ö‹8ÿW‹Eø‹UüèwÖøÿu‹Eü‰EğÆEôUğ3É¸h`G èÈùıÿ³ëFKuÂ3Û3ÀZYYd‰hT`G Eøº   èÔÓøÿÃéĞøÿëë‹Ã_^[‹å]Ã   ÿÿÿÿ@   Found pending rename or delete that matches one of our files: %s    U‹ìÄœşÿÿSVW3Û‰ şÿÿ‰œşÿÿ‰]ì‰]è‰Mô‰Uø‰Eü3ÀUhrcG dÿ0d‰ ÿuüÿuøÿuôEìº   è;ÕøÿÆEó …¤şÿÿP‹Eìè,ÖøÿPèZ÷øÿ‰Eäƒ}äÿ„  3ÒUhbG dÿ2d‰"‹…¤şÿÿ¨…³   €} t¨…¥   Uè‹E‹@è5ëÿÿ‹Eö@Ku1ÿuèÿuø… şÿÿ•Ğşÿÿ¹  èàÓøÿÿµ şÿÿEèº   è¥Ôøÿë>ƒ}ø t8• şÿÿ‹Eèè—fûÿÿµ şÿÿÿuø•œşÿÿ‹EèèXfûÿÿµœşÿÿEèº   èeÔøÿ‹EP‹Eèè…ıÿÿY„ÀtÆEóèÓÏøÿéY  …¤şÿÿP‹EäPèvöøÿ…À…'ÿÿÿ3ÀZYYd‰h$bG ‹EäPèHöøÿÃéBÎøÿëï‹Eö@L „  …¤şÿÿP‹Uü… şÿÿèNÒøÿ… şÿÿ‹Uøè(Óøÿ… şÿÿºcG èÓøÿ‹… şÿÿèÉÔøÿPè÷õøÿ‰Eäƒ}äÿ„Å   3ÀUh@cG dÿ0d‰ …¤şÿÿè%üÿÿ„Àtr‹EPŠEP‹EP‹Uø… şÿÿèÛÑøÿ…œşÿÿ•Ğşÿÿ¹  èÒøÿ‹•œşÿÿ… şÿÿèœÒøÿ… şÿÿºœcG èŒÒøÿ‹• şÿÿ‹Mô‹Eüè«ıÿÿY„ÀtÆEóè­Îøÿë6…¤şÿÿP‹EäPèSõøÿ…À…gÿÿÿ3ÀZYYd‰hGcG ‹EäPè%õøÿÃéÍøÿëï3ÀZYYd‰hycG …œşÿÿº   è¼ĞøÿEèº   è¯ĞøÿÃéíÌøÿëÛŠEó_^[‹å]Â    ÿÿÿÿ   *   ÿÿÿÿ   \   U‹ì3ÉQQQQQSVW3ÀUheG dÿ0d‰ ÆEû ¡ü!I ƒx „  ²¸Ğ¦@ èVÇøÿ‰Eü3ÀUhädG dÿ0d‰ ¸_G ‹Uüè¨Üıÿ‹Eü‹ÿR…Àu
è³Íøÿéİ   ¡ü!I ‹pN…öŒ¯   F3ÿ‹×¡ü!I è¦Oùÿ‹Ø€{N …Œ   j ‹Ë‹X"I ¡P"I è–¿ÿÿ„Àttƒ{8ÿt$UUğ‹Cè(èÿÿ‹EğèôúÿÿY„ÀtUÆEûèBÍøÿëoUô‹èèÿÿU‹EôèuûÿPSUğ‹Eôèxcûÿ‹EğPUì‹Eôè‘cûÿ‹Eì3ÒYèöûÿÿY„ÀtÆEûèøÌøÿë%GN…Tÿÿÿ3ÀZYYd‰hëdG ‹EüèuÆøÿÃé{Ëøÿëğ3ÀZYYd‰heG Eìº   èÏøÿÃéYËøÿëëŠEû_^[‹å]ÃSVQ‹ò‹Ø€=ğI  „“   3ÀŠÃƒÀúƒø	‡ƒ   ÿ$…CeG keG qeG ¿eG weG }eG ƒeG ‰eG eG •eG ›eG Æ$ ë.Æ$ë(Æ$ë"Æ$ëÆ$ëÆ$ëÆ$ë
Æ$ëÆ$3ÀŠÃ‹…àI ‹Öè#Nùÿ‹ĞL$Š$è‘éıÿZ^[ÃU‹ì3ÉQQQQSVW‹Ú‹ø3ÀUhfgG dÿ0d‰ ‹¸|gG ènÒøÿHu+€=t"I  t‹ÇèÿÍøÿÆéB  ‹Ç¹   º   èÑøÿëÆ‹¸ŒgG è4ÒøÿHu+€=t"I  u‹ÇèÅÍøÿÆé  ‹Ç¹
   º   èdÑøÿëŒ‹¸ gG èúÑøÿH…Î   €=t"I  t‹Çè‡ÍøÿÆéÊ   EüP‹¹ÿÿÿº   èâĞøÿ³‹Uü°,èpûÿ‹ğ…öuEø‹UüèáÍøÿë#EøP‹ÎIº   ‹Eüè¯ĞøÿEü‹Îº   èàĞøÿUğ‹Eøè©åÿÿ‹UğEøè¦ÍøÿUğ¡$ I è™\ûÿ‹UğEô‹Møè·Îøÿ‹Eôèçdûÿ„Àu‹Eøèß“ÿÿ„Ût‹Ç‹Uôè%Íøÿ3Û…ö…cÿÿÿëUğ‹èKåÿÿ‹Uğ‹ÇèÍøÿ3ÀZYYd‰hmgG Eğº   è»ÌøÿÃéùÈøÿëë_^[‹å]Ãÿÿÿÿ   setup:  ÿÿÿÿ
   uninstall:  ÿÿÿÿ   files:  ‹Ê²	’èçıÿÃ‹Ê²	’è¢çıÿÃ€=ğI  t	‹Ê²	èçıÿÃSVWU‹è;-@I „   ‹Õ¡à!I èâKùÿ‹Ø‹CèxÍøÿPCè?ÏøÿZèùıÿ‰-@I ¸ !I ºC è Òøÿ‹ó¿ !I ¹   ó¥GÀºC è‘Òøÿƒ{ t¸("I ‹SèÌøÿë¸("I ‹¤ I èğËøÿƒ{  t¸,"I ‹S èİËøÿë¸,"I ‹¨ I èËËøÿƒ{$ t¸0"I ‹S$è¸Ëøÿë¸0"I ‹¬ I è¦Ëøÿ¡ÜI è„Îøÿ‹Ğ3ÀèSûÿ¡øI èqÎøÿ‹Ğ°è@ûÿ¡„I è^Îøÿ‹Ğ°è-ûÿ¡„I èKÎøÿ‹Ğ°èûÿ‹ÄI ¡(I èVÙúÿ¡ì!I ‹pN…ö|TF3ÿ‹×¡ì!I è²Jùÿ‹ØŠC%şÈt
şÈtşÈt"ë.C‹ÀI èËøÿëC‹àI èıÊøÿëC‹ I èíÊøÿGNu¯€= I  tUh'  h–  ¡¤I Pè¾÷øÿ]_^[ÃSVWU€=‹!I  ”h"I ƒ=¨I  tC¡à!I ‹xO…ÿ|6G3ö‹Ö¡à!I èJùÿ‹¡¨I èM ùÿ…Àu‹ÆèúıÿÿÆh"I  éÙ   FOuÍ Œ!I ,rtëèXxûÿ‹èëèŸïøÿ‹èë3íf…í„”   ¡à!I ‹xO…ÿ|6G3ö‹Ö¡à!I è¤Iùÿ‹Ø·Å;C(uƒ{, t
ègîøÿ;C,u	‹ÆèƒıÿÿëlFOuÍ¡à!I ‹xO…ÿ|DG3ö‹Ö¡à!I èaIùÿ‹Ø‹C(%ÿ  ‹Õfâÿ·Ò;Âuƒ{, t
èîøÿ;C,u	‹Æè2ıÿÿëFOu¿3Àè%ıÿÿ€=‹!I uÆh"I ]_^[ÃS‹Øjì¡(I ‹@ Pèíôøÿ¨€”À:Øt~h—   j j j j j ¡(I ‹@ Pè¿öøÿjì¡(I ‹@ Pè·ôøÿ„Ût%ÿÿÿë€   Pjì¡(I ‹@ Pè~öøÿ„Ûtj¡(I ‹@ PèªöøÿëjWj j j j j ¡(I ‹@ Pè^öøÿ[ÃU‹ìƒÄÈSVW3À‰Eø‰Eü3ÀUhçlG dÿ0d‰ fƒ=€"I  tYUøf¡€"I fÁè·ÀèZùÿ‹MøEüº mG èÊøÿf‹€"I fãÿ f…Ût#ÿuühmG Uø·Ãè&ùÿÿuøEüº   èRÊøÿ¡|"I Áè‰EĞÆEÔ ¡|"I Áè%ÿ   ‰EØÆEÜ ¡|"I %ÿÿ  ‰EàÆEä ‹Eü‰EèÆEì3À k"I ‹…X
I ‰EğÆEôUĞ¹   ¸mG è©íıÿ3À l"I ‹…X
I ‰EÈÆEÌUÈ3É¸TmG è…íıÿ3À HI ‹…$	I ‰EÈÆEÌUÈ3É¸pmG èaíıÿ€=k"I  t4€=n"I  t¸”mG èEëıÿë€=o"I  t¸¼mG è0ëıÿë
¸àmG è$ëıÿ3ÀZYYd‰hîlG Eøº   è:ÇøÿÃéxÃøÿëë_^[‹å]Ã   ÿÿÿÿ    SP ÿÿÿÿ   .   ÿÿÿÿ0   Windows version: %u.%.2u.%u%s  (NT platform: %s)    ÿÿÿÿ   64-bit Windows: %s  ÿÿÿÿ   Processor architecture: %s  ÿÿÿÿ   User privileges: Administrative ÿÿÿÿ   User privileges: Power User ÿÿÿÿ   User privileges: None   SV‹ò‹Ø‹Ãƒø‡¾   ÿ$…nG ÇnG @nG OnG ^nG mnG |nG ‹nG šnG ÇnG ÇnG ©nG ¸nG ‹ÆºÜnG èHÆøÿ^[Ã‹ÆºènG è9Æøÿ^[Ã‹ÆºønG è*Æøÿ^[Ã‹ÆºoG èÆøÿ^[Ã‹ÆºoG èÆøÿ^[Ã‹Æº(oG èıÅøÿ^[Ã‹Æº4oG èîÅøÿ^[Ã‹Æº@oG èßÅøÿ^[Ã‹ÆºToG èĞÅøÿ^[Ã‹Ö‹Ãè(şøÿ^[Ã ÿÿÿÿ   OK  ÿÿÿÿ   Cancel  ÿÿÿÿ   Abort   ÿÿÿÿ   Retry   ÿÿÿÿ   Ignore  ÿÿÿÿ   Yes ÿÿÿÿ   No  ÿÿÿÿ	   Try Again   ÿÿÿÿ   Continue    SVW‹ò‹Ø‹ûƒç‹Çƒø‡…   ÿ$…~oG šoG ¨oG ¶oG ÄoG ÒoG àoG îoG ‹ÆºpG èîÄøÿë]‹Æº pG èàÄøÿëO‹Æº4pG èÒÄøÿëA‹ÆºPpG èÄÄøÿë3‹ÆºhpG è¶Äøÿë%‹ÆºxpG è¨Äøÿë‹ÆºpG èšÄøÿë	‹Ö‹Çèóüøÿ_^[Ã   ÿÿÿÿ   OK  ÿÿÿÿ	   OK/Cancel   ÿÿÿÿ   Abort/Retry/Ignore  ÿÿÿÿ   Yes/No/Cancel   ÿÿÿÿ   Yes/No  ÿÿÿÿ   Retry/Cancel    ÿÿÿÿ   Cancel/Try Again/Continue   U‹ìƒÄäSVW3Û‰]ü‰]è‰]ä‹ù‹ò‹Ø3ÀUhTqG dÿ0d‰ EüPUè‹Çèıÿÿ‹Eè‰EìÆEğUä‹Æèjşÿÿ‹Eä‰EôÆEøUì¹   ¸lqG èrùÿEüPEè‹ÓèÄøÿ‹UèXèSÄøÿ‹Eüè¿æıÿ3ÀZYYd‰h[qG Eäº   èÕÂøÿEüè­ÂøÿÃé¿øÿëã_^[‹å]Ã  ÿÿÿÿ3   Defaulting to %s for suppressed message box (%s):
 U‹ìƒÄğSVW3É‰Mü‰Mğ‹ò‹Ø3ÀUh'rG dÿ0d‰ EüPUğ‹Æèıÿÿ‹Eğ‰EôÆEøUô3É¸@rG èšùÿEüPEğ‹Óè4Ãøÿ‹UğXè{Ãøÿ‹Eüèçåıÿ3ÀZYYd‰h.rG EğèâÁøÿEüèÚÁøÿÃé8¾øÿëè_^[‹å]Ã   ÿÿÿÿ   Message box (%s):
 U‹ìƒÄğSVW3Û‰]ğ‹ñ‰Uü‹Ø‹}3ÀUhórG dÿ0d‰  ÜI "Et‹Ï‹Ö‹Ãèşÿÿ‹ßëJ‹Ö‹Ãèÿÿÿ‹Î‹Uü‹Ãè¨xûÿ‹Ø…Ût%Uğ‹Ãè@ûÿÿ‹Eğ‰EôÆEøUô3É¸sG èçıÿë
¸(sG èåıÿ3ÀZYYd‰húrG EğèÁøÿÃél½øÿëğ‹Ã_^[‹å]Â    ÿÿÿÿ   User chose %s.  ÿÿÿÿ   AppMessageBox failed.   U‹ìƒÄìSVW3Û‰]ìˆMû‰Uü‹Ø‹}‹u3ÀUhïsG dÿ0d‰  ÜI "Et‹Ãè½Ãøÿ‹Ï‹Öè(ıÿÿ‹ßëQ‹Ãè©Ãøÿ‹Öè
şÿÿVŠMû‹Uü‹ÃèØxûÿ‹Ø…Ût%Uì‹ÃèDúÿÿ‹Eì‰EğÆEôUğ3É¸tG è#æıÿë
¸$tG èäıÿ3ÀZYYd‰hösG EìèÀøÿÃép¼øÿëğ‹Ã_^[‹å]Â    ÿÿÿÿ   User chose %s.  ÿÿÿÿ   MsgBox failed.  Æ±"I è8Åıÿ„Àu2j¡(I ‹@ Pè˜íøÿ¡(I ‹@ Pè
íøÿj jj±3Ò¡|I èÊşÿÿÃU‹ìƒÄØSVW3Ò‰Uè‹ğ3ÀUhEvG dÿ0d‰ h—   j j j j j ¡(I ‹@ Pè	íøÿÆEÿ 3ÀUhvuG dÿ0d‰ ²¸|G è7¡ÿÿ‰Eø3ÀUheuG dÿ0d‰ €= I  t‹¤I ë‹Eø‹XEèP‹Eø‹@‰EØÆEÜ ‰]àÆEä UØ¹   ¸\vG èaùÿEè‹ÖèOÀøÿ‹UèMì¡”I èOÿÿ‹EøŠ@ˆEğ‹Eø‹@‰Eô3ÀZYYd‰hluG ‹EøèôµøÿÃéúºøÿëğ3ÀZYYd‰ë8éõ¸øÿèùÿº¨c@ è¶øÿ„ÀtÆEÿëj¡(I ‹@ PèHìøÿè»øÿèŞ»øÿ€}ÿ t
¸   èšØøÿ€= I  u`3ÒUhvG dÿ2d‰"‹Eô…À|‹à!I ;B}èèñÿÿ€}ğ tè=şÿÿ3ÀZYYd‰ë&éj¸øÿj¡(I ‹@ PèÖëøÿ3Ò¡(I è6Ğúÿèe»øÿ‹Eìè)Øøÿ3ÀZYYd‰hLvG Eèè¼½øÿÃéºøÿëğ_^[‹å]Ã ÿÿÿÿ   /SPAWNWND=$%x /NOTIFYWND=$%x    è7jûÿ¢n"I €=n"I  uè0jûÿ„Àu3Àë°¢o"I èĞ°øÿÃ@ SƒÄø‹Ø3ÀŠÃ‹…X
I ‰$ÆD$‹Ô3É¸wG èãıÿˆm"I ˆu"I ˆv"I „Ût	ÆDI ëÆDI YZ[Ã   ÿÿÿÿ   64-bit install mode: %s U‹ìj SVW‹ğ3ÀUhwG dÿ0d‰ ‹°,èRûÿ‹Ø…Ûu¹¸wG ²¸Xc@ èDùÿè¿¹øÿEüP‹ËI‹º   èÀøÿ‹Eüè°õøÿ‹ø‹Æ‹Ëº   è0Àøÿ3ÀZYYd‰h¥wG Eüèc¼øÿÃéÁ¸øÿëğ‹Ç_^[Y]Ã   ÿÿÿÿ   ExtractLongWord: Missing comma  SV‹ò‹Øƒ; tÿ3hxG V‹Ãº   è=¾øÿ^[Ã‹Ã‹ÖèM¼øÿ^[Ã  ÿÿÿÿ   
  U‹ìQSVW‹úˆEÿ‹ÇèÙ»øÿ3Û¾$	I ‹Ã<wƒà£Eÿs	‹Ç‹è’ÿÿÿCƒÆ€ûuß_^[Y]Ã@ U‹ìƒÄØSVW3Û‰]à‹Ù‹ò‰Eü3ÀUhyyG dÿ0d‰ ¹   ¶}ş‹ÇQ¹
   ™÷ùY…ÒuI‹Ç¿
   ™÷ÿˆEşj3ÀŠEÿ‰EäÆEè ‰MìÆEğ 3ÀŠEş‰EôÆEø Eä‹È‹ÃºyG èÄÿøÿf‹}üf…ÿt'EàP·Ç‰EØÆEÜ UØ3É¸ yG èŠÿøÿ‹Uà‹Ãèx¼øÿf…ötZÿ3h¬yG Uà‹ÆfÁè·Àè×óøÿÿuà‹Ãº   è½øÿ‹şfçÿ f…ÿt'EàP·Ç‰EØÆEÜ UØ3É¸ yG è+ÿøÿ‹Uà‹Ãè¼øÿ3ÀZYYd‰h€yG EàèˆºøÿÃéæ¶øÿëğ_^[‹å]Ã ÿÿÿÿ   %d.%.*d ÿÿÿÿ   .%d ÿÿÿÿ    Service Pack   j jj%ÿ   ‹…tI ±3Òèiùÿÿè”ùÿÃ@ U‹ìƒÄøSVW3É‰Mü‹ò‹Ø3ÀUh@zG dÿ0d‰ j jjEüP‰uøUø3É‹Ãè‹ıÿ‹Eü±3ÒèùÿÿèFùÿ3ÀZYYd‰hGzG EüèÁ¹øÿÃé¶øÿëğ_^[YY]Ã‹ÀU‹ìƒÄğSVW3Û‰]ü‰]ğ‹ù‹ò‹Ø3ÀUhÔzG dÿ0d‰ j jjEüP‰uôMğf‹U‹ÇèÈıÿÿ‹Eğ‰EøUô¹   ‹Ãè‡Šıÿ‹Eü±3Òèøÿÿèºùÿ3ÀZYYd‰hÛzG Eğè5¹øÿEüè-¹øÿÃé‹µøÿëè_^[‹å]Â U‹ìƒÄôSVW‹ú‰Eü¸    èL«øÿ‰Eø3ÀUhp{G dÿ0d‰ Uô¹   ‹Çè¤ˆıÿƒ}ô ~3‹]ôû    ~»    ‹uø‹Ö‹Ë‹Çè€ˆıÿ‹Ö‹Ë‹EüèôPùÿ)]ôƒ}ô Í3ÀZYYd‰hw{G º    ‹EøèñªøÿÃéï´øÿëë_^[‹å]Ã‹ÀU‹ìQSV‹ò‹Ø²¸ü¨@ èš¯øÿ‰Eü3ÀUhë{G dÿ0d‰ ‹Ö‹Eüè3ÿÿÿ3É3Ò‹Eü‹0ÿV²¸¤˜A èóPúÿ‰‹‹Uü‹ÿQ<3ÀZYYd‰hò{G ‹Eüèn¯øÿÃét´øÿëğ^[Y]ÃU‹ìƒÄğSVW3À‰Eø‰Eü3ÀUhé|G dÿ0d‰ Uø¡$ I èpGûÿ‹UøEü¹ }G èŒ¹øÿ‹E‹@ü‹UüèŠÖÿÿ‹EƒÀüèseûÿº €  ‹Eüèveûÿ£0#I ƒ=0#I  u%EøP‹Eü‰EğÆEôUğ3É¸ }G èÿûøÿ‹Eøè³œıÿ !I şÈtşÈtë2¡0#I è‚)şÿ„Àu$¸@}G èŒœıÿë¡0#I è<-şÿ„Àu
¸l}G èrœıÿ3ÀZYYd‰hğ|G Eøº   è8·øÿÃév³øÿëë_^[‹å]Ã ÿÿÿÿ   _isetup\_isdecmp.dll    ÿÿÿÿ   Failed to load DLL "%s" ÿÿÿÿ"   ZlibInitDecompressFunctions failed  ÿÿÿÿ    BZInitDecompressFunctions failed    U‹ìƒÄğSVW3À‰Eø‰Eü3ÀUhX~G dÿ0d‰ Uø¡$ I èØEûÿ‹UøEü¹p~G èô·øÿ‹E‹@ø‹UüèòÔÿÿ‹EƒÀøèÛcûÿº €  ‹EüèŞcûÿ£4#I ƒ=4#I  u%EøP‹Eü‰EğÆEôUğ3É¸~G ègúøÿ‹Eøè›ıÿ¡4#I èÍ&şÿ„Àu
¸°~G è›ıÿ3ÀZYYd‰h_~G Eøº   èÉµøÿÃé²øÿëë_^[‹å]Ã  ÿÿÿÿ   _isetup\_iscrypt.dll    ÿÿÿÿ   Failed to load DLL "%s" ÿÿÿÿ   ISCryptInitFunctions failed U‹ìQSVW‰Mü‹ò‹Ø3ÀŠÃ‹…à!I ‹Öèd6ùÿN…ö|<F‹Eüè:êøÿ‹ø3ÀŠÃ‹… I P‹×‹E‹@ô‹Müè}ÿÿ3ÀŠÃ‹…à!I ‹×è4ùÿNuÅ_^[Y]ÃU‹ìƒÄøSVW‰Mü‹ò‹Ø€=ğI  t&²¸\¤@ è×«øÿ‹ø3ÀŠÃ‰<…àI 3ÀŠÃ‹Ç‹Öèİ5ùÿ3ÀŠÃ‹…à!I ‹ÖèË5ùÿ‹şO…ÿŒ—   GÇEø    ‹Eüè”éøÿ‹ğ3ÀŠÃ‹… I P‹Ö‹E‹@ô‹Müèv|ÿÿƒ}ÿt‹E‹Ğ‹EèÜÿÿ„Àu03ÀŠÃ‹…à!I ‹ÖèK3ùÿ€=ğI  t'3ÀŠÃ‹…àI ‹Uøè/3ùÿë3ÀŠÃ‹… I ‹Æèó{ÿÿÿEøO…qÿÿÿ_^[YY]Â U‹ìƒÄğSVW3Ò‰Uø‹Ø3ÀUhê€G dÿ0d‰ ˆ]ÿ„Ûtƒ=ÔI  tv3ÛEø‹ÔI è&´øÿö™!I  t
‹Eøè)œÿÿ‹Ø„Ûu,ƒ=´"I  t#j j S‹Eø‰EğÆEôMğºG ¡´"I è…= ‹Ø„ÛtÆEÿ ö!I tèqõşÿ8  ‹Uøè|³øÿ3ÀZYYd‰hñ€G Eøè³øÿÃéu¯øÿëğŠEÿ_^[‹å]Ã ÿÿÿÿ   CheckPassword   U‹ìƒÄøVW€=¾I  t	Æi"I ë€=½I  tÆi"I €=i"I  tG€=i"I u¡(I Æ@: 3Àèdéÿÿ¾˜!I }ø¥f¥Eøº”G ±è©øÿ‹Eø‰˜!I f‹Eüf‰œ!I _^YY]Ã       U‹ìÄ¤şÿÿSVW3Û‰¨şÿÿ‰¤şÿÿ‰]ø‰Mü‹ú‹ğ‹]3ÀUh˜ƒG dÿ0d‰ VWÿuüEøº   èQ´øÿ3À‰C3À‰…¬şÿÿP‹Eøè=µøÿPèkÖøÿ‰Eôƒ}ôÿtM‹…¬şÿÿ¨u&€} t¨u‹…Èşÿÿ‰Eğ‹…Ìşÿÿ‰EìUì‹Ãè:ûÿ…¬şÿÿP‹EôPè*Öøÿ…Àu¼‹EôPèÖøÿ€} „  …¬şÿÿP‹Ö…¨şÿÿè²øÿ…¨şÿÿ‹×èú²øÿ…¨şÿÿº°ƒG èê²øÿ‹…¨şÿÿè›´øÿPèÉÕøÿ‰Eôƒ}ôÿ„Â   3ÀUhkƒG dÿ0d‰ …¬şÿÿè÷Ûÿÿ„Àto‹EPŠEPŠEPEìP‹×…¨şÿÿèª±øÿ…¤şÿÿ•Øşÿÿ¹  è\²øÿ‹•¤şÿÿ…¨şÿÿèk²øÿ…¨şÿÿº¼ƒG è[²øÿ‹•¨şÿÿ‹Mü‹ÆèkşÿÿYUì‹Ãè8€ûÿ…¬şÿÿP‹EôPè(Õøÿ…À…jÿÿÿ3ÀZYYd‰hrƒG ‹EôPèúÔøÿÃéô¬øÿëï3ÀZYYd‰hŸƒG …¤şÿÿº   è‘°øÿEøèi°øÿÃéÇ¬øÿëà_^[‹å]Â ÿÿÿÿ   *   ÿÿÿÿ   \   U‹ìÄpÿÿÿSVW3À‰…|ÿÿÿ‰…xÿÿÿ‰Eğ‰Eì‰EÜ‰EØ‰EÈ3ÀUh8–G dÿ0d‰ è€òÿÿMìUğ¸   è,¿ıÿºP–G ‹Eğèæøÿ…ÀuI¾   ÆI Eìèïòÿÿ£¤I Æ I EìèÛòÿÿ£˜I EìèÎòÿÿ£œI ¸”I ‹Uìèì¯øÿë"¾   •|ÿÿÿ3Àè¨Nûÿ‹•|ÿÿÿ¸”I èÈ¯øÿ•|ÿÿÿ¡”I èôBûÿ‹•|ÿÿÿ¸  I è¨¯øÿÆEç 3ÛÆEæ 3À‰EàèNûÿ+ÆŒ•  @‰EÄ‰uèMìUğ‹Eèè_¾ıÿº`–G ‹Eğè6åøÿ…Àu³EÜè¯øÿéT  ºp–G ‹Eğèåøÿ…Àu³EÜ‹Uìè}¯øÿé1  º€–G ‹Eğèóäøÿ…ÀuÆ½I é  º–G ‹EğèÖäøÿ…ÀuÆ¾I é÷  º¤–G ‹Eğè¹äøÿ…ÀuÆ¿I éÚ  º¸–G ‹Eğèœäøÿ…ÀuÆ¼I é½  ºÌ–G ‹Eğèäøÿ…ÀuÆÀI é   ºà–G ‹Eğèbäøÿ…Àu¸¨I ‹Uìè…®øÿé}  ºğ–G ‹Eğè?äøÿ…Àu¸ÄI ‹Uìèb®øÿéZ  º —G ‹Eğèäøÿ…Àu*ÆĞI •|ÿÿÿ‹Eìè¿ÿÿ‹•|ÿÿÿ¡ÈI èƒ ÿÿé  º—G ‹Eğèáãøÿ…Àu*ÆÑI •|ÿÿÿ‹Eìè„ÿÿ‹•|ÿÿÿ¡ÌI èH ÿÿéä  º(—G ‹Eğè¦ãøÿ…Àu*ÆÑI  •|ÿÿÿ‹EìèIÿÿ‹•|ÿÿÿ¡ÌI è ÿÿé©  º@—G ‹Eğèkãøÿ…Àu#•|ÿÿÿ‹Eìè)@ûÿ‹•|ÿÿÿ¸´I è}­øÿéu  ºT—G ‹Eğè7ãøÿ…Àu#•|ÿÿÿ‹Eìèõ?ûÿ‹•|ÿÿÿ¸¸I èI­øÿéA  ºh—G ‹Eğèãøÿ…Àu¸¬I ‹Uìè&­øÿé  ºx—G ‹Eğèàâøÿ…Àu¸°I ‹Uìè­øÿéû   ºˆ—G ‹Eğè½âøÿ…Àu¸ÔI ‹Uìèà¬øÿéØ   ºœ—G ‹Eğèšâøÿ…Àu3Ò‹EìèÔåøÿ£ØI é³   º¸—G ‹Eğèuâøÿ…Àu	ÆEæé™   ºÔ—G ‹Eğè[âøÿ…Àu	ÆİI ëºì—G ‹EğèAâøÿ…ÀuÆEç‹Eìè=åøÿèH‰ÿÿë[º ˜G ‹Eğèâøÿ…Àu‹Eìèåøÿ£¤I Æ I ë4º˜G ‹Eğèöáøÿ…ÀuèíŒÿÿëº0˜G ‹EğèŞáøÿ…Àu‹EìèŞäøÿ‰EàÿEèÿMÄ…rüÿÿƒ=´I  t
¡´I ètŸÿÿ€}æ t ½I 
¾I tÆÜI ¸ĞI ºD˜G è»«øÿ¸ÈI ºÄ˜G è¬«øÿ¸ÌI º™G è«øÿ€=I  uN|ÿÿÿ3Ò¡”I èj;ûÿ‹•|ÿÿÿEØ¹¬™G èß¬øÿ‹EØèCûÿ„Àu+•|ÿÿÿ‹EØè?ûÿ‹•|ÿÿÿ°—èØğÿÿëEØ‹”I è|«øÿjj j‹MØ²¸höD è—oıÿ‰EÔ3ÀUhêG dÿ0d‰ ‹˜I ‹EÔè<oıÿU€¹@   ‹EÔ‹8ÿWƒø@t°–èTğÿÿE€ºI ¹@   èJŸøÿt°–è9ğÿÿ3ÀUh·G dÿ0d‰ h`®E ‹MÔ²¸PÿD è‹wıÿ‰Eô3ÀUhPG dÿ0d‰ jºp I ¹.  ‹EôèjrÿÿU¹@   ‹!I 3ÀèïôÿÿYU¹   ‹!I °èÛôÿÿYU¹   ‹!I °èÇôÿÿYU3ÀƒÀP3ÀƒÀP¹.   ‹!I °èõÿÿYèaßÿÿ€}ç u;€=Š!I  •Àè‰ÿÿ„Àt(EôètWûÿEÔèlWûÿ•|ÿÿÿ‹ÆèóGûÿ‹…|ÿÿÿèêÿÿƒ}à t
3Ò‹EàèğÁıÿ„Ûu	ö!I t_3ÀUh»ŠG dÿ0d‰ ƒ}Ü u¸¼™G è)Ëıÿë‹EÜè×Ìıÿ3ÀZYYd‰ë-ét¤øÿ   Xc@ ÌŠG ‰Ã‹KCºÌ™G èâªøÿèU¦øÿè¤¦øÿ¸ô™G èşÌıÿº$šG …|ÿÿÿè’©øÿ…|ÿÿÿ‹”I èiªøÿ‹…|ÿÿÿèÒÌıÿºDšG …|ÿÿÿèf©øÿ…xÿÿÿèÇFûÿ‹•xÿÿÿ…|ÿÿÿè2ªøÿ‹…|ÿÿÿè›Ìıÿèàÿÿö™!I  •À¢p"I Æq"I  ö˜!I @•À¢r"I Ujÿjÿ¹>   ‹!I °èóÿÿYUjÿjÿ¹2   ‹!I °è†óÿÿYU3ÀƒÀ P3ÀƒÀ*P¹7   ‹!I °èfóÿÿYU3ÀƒÀ$P3ÀƒÀ.P¹O   ‹ !I °èFóÿÿYU3ÀƒÀ0P3ÀƒÀ:P¹P   ‹(!I °	è&óÿÿYU3ÀƒÀ(P3ÀƒÀ2P¹=   ‹,!I °
èóÿÿYU3ÀƒÀ$P3ÀƒÀ.P¹A   ‹0!I °èæòÿÿYU3ÀƒÀP3ÀƒÀ&P¹1   ‹4!I °èÆòÿÿYU3ÀƒÀP3ÀƒÀ&P¹1   ‹8!I °è¦òÿÿYU3ÀƒÀ4P3ÀƒÀ>P¹O   ‹<!I °è†òÿÿYU3ÀƒÀ4P3ÀƒÀ>P¹O   ‹@!I °èfòÿÿY¸ "I ‹Uôè¤îÿÿ¸$"I ‹Uôè—îÿÿ3À‰Eü !I H,s²¸ü¨@ è,øÿ‰Eü‹Uô‹EüèÒíÿÿ3À‰Eøö!I t²¸ü¨@ èøÿ‰Eø‹Uô‹Eøèªíÿÿ3ÀZYYd‰hWG ‹Eôè	øÿÃé£øÿëğh`®E ‹MÔ²¸PÿD èÅsıÿ‰Eô3ÀUh¦G dÿ0d‰ U¹F   ‹$!I °è=ñÿÿY3ÀZYYd‰h­G ‹Eôè³øÿÃé¹¢øÿëğ3ÀZYYd‰ëéx¡øÿ   şD ÈG °•èíëÿÿè¸£øÿ3ÀZYYd‰hñG ‹EÔèoøÿÃéu¢øÿëğ€=h"I  t+¡à!I ƒx~ €=½I  u€=¾I  uè|pÿÿ„ÀuèKıøÿ \šG :!I t2 HI <w
ƒà£!I r•|ÿÿÿ !I è½éÿÿ‹•|ÿÿÿ°pè|ëÿÿºN!I ¸D!I èÎÿÿşÈtşÈt>şÈtté§   €=k"I  tºhšG °lèGëÿÿé   f¡L!I P‹H!I ºhšG °Àè™ëÿÿër€=k"I  tf¡L!I P‹H!I ºhšG °ÀèuëÿÿëNj ‹D!I º|šG °Àè_ëÿÿë8€=k"I  tf¡V!I P‹R!I ºhšG °¿è;ëÿÿëj ‹N!I º|šG °¿è%ëÿÿ HI <w
ƒà£!I s€=l"I  uºŒšG °gè‹êÿÿ°èPçÿÿë3ÀèGçÿÿ Š!I şÈtşÈtë"€=o"I  u°vè8êÿÿë€=n"I  u°è&êÿÿè	Àÿÿè`Äÿÿè‡Çÿÿ !I H,sUèCìÿÿYö!I tUèËíÿÿYèIñÿÿƒ=Ì I  „C  ²¸`¶H è, £´"I 3ÀUhG dÿ0d‰ ¡´"I Ç@ÄeG ¡´"I Ç@¨gG ¡´"I Ç@´gG ¡´"I Ç@ÀgG ‹øI ‹Ì I ¡´"I ès, €=p"I  uº˜šG ¡´"I èã, ¢p"I  p"I èÌïÿÿ¢p"I €=q"I  uº°šG ¡´"I è·, ¢q"I 3ÀZYYd‰ëéÛøÿ¸´"I è!Qûÿè” øÿèã øÿ3ÒUhíG dÿ2d‰"j j j3À‰…pÿÿÿÆ…tÿÿÿpÿÿÿºÄšG ¡´"I èS- d    ƒÄëé~øÿ¸ÜšG èôÆıÿè7 øÿè† øÿ„Àu ¸›G èÜÆıÿèWúøÿë p"I èïÿÿ¢p"I •|ÿÿÿ¡p I èR»ÿÿ‹•|ÿÿÿ¸˜"I è£øÿ•|ÿÿÿ¡t I è2»ÿÿ‹•|ÿÿÿ¸œ"I èæ¢øÿ•|ÿÿÿ¡| I è»ÿÿ‹•|ÿÿÿ¸ "I èÆ¢øÿ•|ÿÿÿ¡¼ I èòºÿÿ‹•|ÿÿÿ¸¤"I è¦¢øÿë4jjj|ÿÿÿ‹˜"I °“è—sıÿ‹…|ÿÿÿ±‹ÄI èháÿÿHtèùøÿ¡¤"I è~¥ıÿ„Àu¾¡ì!I ‹@H…À|m@‰EÄÇEè    j ‹Uè¡ì!I è¿!ùÿ‹@P‹Uè¡ì!I è®!ùÿ‹@P3É3Ò3Àèoÿÿ„Àu'‹Uè¡ì!I è!ùÿ‹I è¿iÿÿ3É‹Uè¡ì!I è€"ùÿÿEèÿMÄu¡ì!I è²"ùÿ3ÿ3ö¡ğ!I ‹@H…ÀŒ›   @‰EÄÇEè    ‹Uè¡ğ!I è5!ùÿ‹Ø;{|0S+C!èÃÉÿÿ„Àu!j ‹CP‹CP3É3Ò3ÀèÚÿÿ„Àt‹{G‹óë@…öt‹F;Cu‹‹è1×øÿ…Àt‹C;ø~‹ø3ö3É‹Uè¡ğ!I è×!ùÿ‹I ‹ÃèúhÿÿÿEèÿMÄ…pÿÿÿ¡ğ!I èø!ùÿÆw"I  ¡ì!I ‹@H…À|/@‰EÄÇEè    ‹Uè¡ì!I è| ùÿö@$t	Æw"I ëÿEèÿMÄuÜ¡ğ!I ƒx •x"I ¡"I ƒx •j"I ¡ô!I ƒx •y"I ‹|!I ‰ˆ"I ‹€!I ‰Œ"I ¡ü!I ‹@H…ÀŒX  @‰EÄÇEè    ‹Uè¡ü!I èôùÿ‰EÀ‹EÀƒx8ÿtI‹EÀƒx …  ‹EÀƒx …  ‹EÀƒx …  ‹EÀ‹P8¡ "I è±ùÿP¸ˆ"I èDoûÿéß   ‹EÀö@M…§   3ÀUhß”G dÿ0d‰ ‹EÀ€xN tUÈ3Àè½>ûÿëUÈ‹EÀ‹ è¸ÿÿU‹EÈè‰QûÿP‹EÀö@L •ÀP…pÿÿÿP•|ÿÿÿ‹EÈèx3ûÿ‹…|ÿÿÿP•xÿÿÿ‹EÈè‹3ûÿ‹…xÿÿÿ3ÒYèİìÿÿY‹EÀ‹•pÿÿÿ‰P@‹•tÿÿÿ‰PD3ÀZYYd‰ë
éŒ™øÿè£œøÿ‹EÀƒx u"‹EÀƒx u‹EÀƒx u‹EÀP@¸ˆ"I è`nûÿÿEèÿMÄ…³şÿÿ¡ğ!I ‹@H…À|D@‰EÄÇEè    ‹Uè¡ğ!I èùÿ‹ØpÿÿÿS‹èÕÇÿÿ‹…pÿÿÿ‰C6‹…tÿÿÿ‰C:ÿEèÿMÄuÇ¡ì!I ‹@…À~{H…À|i@‰EÄÇEè    ‹Uè¡ì!I è:ùÿ‹ØöC$•Âpÿÿÿ‹è`Èÿÿ‹…pÿÿÿ‰C&‹…tÿÿÿ‰C*ƒ}è tUÌC&èemûÿ…À}‹C&‰EÌ‹C*‰EĞÿEèÿMÄu¢UÌ¸ˆ"I è|mûÿ3ÀZYYd‰h?–G …xÿÿÿº   èøÿEÈèãøÿEØº   èöøÿEìº   èéøÿÃé'šøÿëÆ_^[‹å]Ã  ÿÿÿÿ   /SL5=   ÿÿÿÿ   /Log    ÿÿÿÿ   /Log=   ÿÿÿÿ   /Silent ÿÿÿÿ   /VerySilent ÿÿÿÿ
   /NoRestart  ÿÿÿÿ   /NoIcons    ÿÿÿÿ	   /NoCancel   ÿÿÿÿ   /Lang=  ÿÿÿÿ   /Type=  ÿÿÿÿ   /Components=    ÿÿÿÿ   /Tasks= ÿÿÿÿ   /MergeTasks=    ÿÿÿÿ	   /LoadInf=   ÿÿÿÿ	   /SaveInf=   ÿÿÿÿ   /DIR=   ÿÿÿÿ   /GROUP= ÿÿÿÿ
   /Password=  ÿÿÿÿ   /RestartExitCode=   ÿÿÿÿ   /SuppressMsgBoxes   ÿÿÿÿ   /DETACHEDMSG    ÿÿÿÿ
   /SPAWNWND=  ÿÿÿÿ   /NOTIFYWND= ÿÿÿÿ   /DebugSpawnServer   ÿÿÿÿ
   /DEBUGWND=  ÿÿÿÿw   The file %1 is missing from the installation directory. Please correct the problem or obtain a new copy of the program. ÿÿÿÿG   The setup files are corrupted. Please obtain a new copy of the program. ÿÿÿÿ   The setup files are corrupted, or are incompatible with this version of Setup. Please correct the problem or obtain a new copy of the program.  ÿÿÿÿ   -0.bin  ÿÿÿÿ   Setup   ÿÿÿÿ   Error creating log file:

    ÿÿÿÿ'   Setup version: Inno Setup version 5.2.2 ÿÿÿÿ   Original Setup EXE:     ÿÿÿÿ   Setup command line:         ÿÿÿÿ
   Windows NT  ÿÿÿÿ   Windows ÿÿÿÿ   1   ÿÿÿÿ   CheckPassword   ÿÿÿÿ   CheckSerial ÿÿÿÿ   InitializeSetup ÿÿÿÿ,   InitializeSetup raised an exception (fatal).    ÿÿÿÿ)   InitializeSetup returned False; aborting.   U‹ìƒÄôSVW3Ò‰Uô‹Ø3ÀUhƒG dÿ0d‰ ¸œG èˆ¼ıÿƒ=´"I  „Ç   „Ûtb3ÀUh»›G dÿ0d‰ j j ¡¬"I P3À‰EøÆEüMøº¼G ¡´"I è@# £¬"I 3ÀZYYd‰ë é°’øÿ¸ÜG è&¼ıÿ3Ò¡(I è‚ªúÿè±•øÿ3ÀUhœG dÿ0d‰ j j 3À‰EøÆEüMøºG ¡´"I èL! 3ÀZYYd‰ë éY’øÿ¸,G èÏ»ıÿ3Ò¡(I è+ªúÿèZ•øÿ¸´"I è„Eûÿ¡"I ‹ÿR‹ğN…ö|1F3ÛMô‹Ó¡"I ‹8ÿW‹EôP‹Ó¡"I ‹ÿQ…À•ÀZèŠrıÿCNuÒ¡"I ‹ÿR8¡”"I ‹ÿR‹ØKƒû |0Mô‹Ó¡”"I ‹0ÿV‹EôP‹Ó¡”"I ‹ÿQ…À•ÀZèHwıÿKƒûÿuĞ¡”"I ‹ÿR8èÙşÿƒ=4#I  t¡4#I Pè§»øÿƒ=0#I  t¡0#I Pè“»øÿèz¼ÿÿèI¹ÿÿ€=s"I  t€=ğI  t¸\G èÉºıÿÆs"I  èA°ıÿ€=s"I  t1¸¬G èªºıÿ€= I  tj h'  h–  ¡¤I PèŞÃøÿëèÇÖÿÿ3ÀZYYd‰hŠG Eôè~–øÿÃéÜ’øÿëğ_^[‹å]Ã   ÿÿÿÿ   Deinitializing Setup.   ÿÿÿÿ   GetCustomSetupExitCode  ÿÿÿÿ+   GetCustomSetupExitCode raised an exception. ÿÿÿÿ   DeinitializeSetup   ÿÿÿÿ&   DeinitializeSetup raised an exception.  ÿÿÿÿD   Not restarting Windows because Setup is being run from the debugger.    ÿÿÿÿ   Restarting Windows. U‹ìƒÄğSVW‰Mğ‰Uô‰Eø3À‰Eü¹   ]øuô}ü3ÀŠ3ÒŠ+Â÷mğQ¹ÿ   ™÷ùY3ÒŠÂˆGFCIuÛ‹Eü_^[‹å]Ãh  j j ±‹˜I ¡”I èÔÿÿƒø”ÀÃ‹Àj j jj èÁøÿÃ‹ÀU‹ìƒÄèSVW3Û‰]è„ÒtƒÄğèÎøÿˆUÿ‹Ø3ÀUh€ G dÿ0d‰ 3Ò‹Ãèæ' ‹Ãè* ö™!I t]ö™!I u3Ò‹Ãè¼oúÿëö™!I u	²‹Ãè¨oúÿ€»  ”ÀUìèb# Uì‹ÃèD¦ùÿ‹Ãè}áùÿö™!I t²‹Ãèuúÿë	¡(I Æ@: Mè‹˜"I °™èXeıÿ‹Uè‹Ãè¢ªùÿj ‹ÃèaáùÿPè¿øÿ‹ğj j h   VèV½øÿ¡tI è —øÿPh'  j Vè>½øÿShØ¸G ¡(I è¤úÿ¡(I €x: t	²‹Ãèîjúÿ3ÀZYYd‰h‡ G Eèè“øÿÃéßøÿëğ€}ÿ t
d    ƒÄ‹Ã_^[‹å]ÃSV‹Ú‹ğVhØ¸G ¡(I è¤úÿ3Ò‹ÆèÍbúÿ„Ût‹Æè¢Œøÿ‹Æ^[Ãz'  uèV  Ãè$„úÿÃ@ 3À‰BÃ‹ÀSVWUƒÄÄ‹ø‹Çèdoúÿ‹Ø¡X!I ;\!I u%‹Ğ‹Cè+úÿT$,‹Ç‹ÿQ,T$,‹ÃèÉ
úÿéá   è·şùÿ‹è¡\!I è«şùÿ‰$T$,‹Ç‹ÿQ,‹D$4‰D$‹D$8‰D$3ö‹Î‹$‹ÅèOıÿÿ‹Ğ‹CèÉúÿöš!I uChÿ   FP‹D$PèÅ¸øÿPD$0Phÿ   V‹D$Pè¯¸øÿ‹Ğ‹L$3Àè
ùÿT$,‹Ãè3
úÿëA‹D$PD$0Phÿ   FP‹D$Pèx¸øÿPhÿ   V‹D$Pèg¸øÿ3ÒYèÓ	ùÿT$,‹Ãèğ	úÿFş   …Pÿÿÿj‹ÃèúÿPèøºøÿh¤G j‹Ô!I ‹¬!I ‹CèÕ! ¡˜"I è‡>ûÿ„ÀtŠ ¤G ‹CèúÿëŠ$¤G ‹Cèı úÿT$‹Ç‹ÿQ,jøjøD$Pèp½øÿWt$|$ ¹   ó¥_jjD$$Pè¾øÿ3Ò‹Cè¿şùÿh  D$ Pjÿ¡˜"I èu”øÿP‹ÃèaúÿPè«»øÿºÿÿÿ ‹Cèşùÿh  D$Pjÿ¡˜"I èD”øÿP‹Ãè0úÿPèz»øÿh¤G j‹Ü!I ‹´!I ‹Cè÷  Š(¤G ‹Cè= úÿT$‹Ç‹ÿQ,jújúD$Pè°¼øÿt$|$¹   ó¥h  D$ Pjÿ¡ "I èÏ“øÿP‹Ãè»úÿPè»øÿ‹D$(+D$ ‹T$+Ğ‰T$t$|$¹   ó¥jjD$$Pè½øÿ3Ò‹Cè¼ıùÿh  D$ Pjÿ¡ "I èr“øÿP‹Ãè^úÿPè¨ºøÿºÿÿÿ ‹Cè‹ıùÿh  D$Pjÿ¡ "I èA“øÿP‹Ãè-úÿPèwºøÿƒÄ<]_^[Ã   ÿÿÿÿ   Arial             ‹ÿRHÃ‹ÀU‹ìj SVW3ÀUhü¤G dÿ0d‰ Eüº¥G è=øÿEüº8¥G è‘øÿÿuühà¥G hğ¥G h¦G h(¦G Eüº   è¬‘øÿƒ=|I  tÿuühà¥G ÿ5|I Eüº   èˆ‘øÿƒ=I  tÿuühà¥G ÿ5I Eüº   èd‘øÿj j j 3É‹€I ‹EüèZÎÿÿ3ÀZYYd‰h¥G EüèøÿÃéc‹øÿëğ_^[Y]Ã   ÿÿÿÿ   Inno Setup version 5.2.2
  ÿÿÿÿŸ   Copyright (C) 1997-2007 Jordan Russell
Portions Copyright (C) 2000-2007 Martijn Laan
All rights reserved.

Inno Setup home page:
http://www.innosetup.com/ ÿÿÿÿ   

    ÿÿÿÿ#   RemObjects Pascal Script home page: ÿÿÿÿ   
  ÿÿÿÿ   http://www.remobjects.com/?ps   U‹ìj SVW‹Ú3ÀUh¬¦G dÿ0d‰ ¸Ä¦G è†±ıÿjjUü¡(I è™›úÿ‹EüèµøÿP‹Ãè­øÿ¹   Zè¾Ëÿÿ3ÀZYYd‰h³¦G EüèUøÿÃé³‰øÿëğ_^[Y]Ã   ÿÿÿÿ   Exception message:  U‹ìj SVW‹ñ‹Ø3ÀUh§G dÿ0d‰ Uü‹Fèë=ûÿ‹Uü‹ÃèAÿÿÿ3ÀZYYd‰h$§G EüèäŒøÿÃéB‰øÿëğ_^[Y]Ã‹À¡(I èúÿÃU‹ìƒÄğSVWˆMû‰Eü‹Â‹Uüˆ‚¶  ƒ=´"I  tp3ÒUh“§G dÿ2d‰"j j ƒà‰EğÆEô MğºÜ§G ¡´"I èË 3ÀZYYd‰ë8éØ†øÿ€}û t¸ô§G èH°ıÿ‹Uü¡(I è£úÿë¸ ¨G è/°ıÿèr‰øÿèÁ‰øÿ_^[‹å]Ã  ÿÿÿÿ   CurStepChanged  ÿÿÿÿ#   CurStepChanged raised an exception. ÿÿÿÿ+   CurStepChanged raised an exception (fatal). U‹ìƒÄøSVW‹(I ²¸L
F èAxşÿ£PI ƒ=´"I  tP3ÀUh¬¨G dÿ0d‰ j j 3À‰EøÆEüMøº©G ¡´"I è² 3ÀZYYd‰ëé¿…øÿ¸,©G è5¯ıÿèxˆøÿèÇˆøÿº   ¡PI èˆªşÿ€=i"I  u¡(I ‹@ Pèa¸øÿ¡PI è§„úÿë
¡PI è;Åşÿ_^[YY]Ãÿÿÿÿ   InitializeWizard    ÿÿÿÿ-   InitializeWizard raised an exception (fatal).   Šu"I ö@M€t3Òö@Nt€=l"I  u
¸©G è½oıÿ²‹ÂÃÿÿÿÿ?   Cannot run files in 64-bit locations on this version of Windows U‹ìƒÄÌSVW3Ò‰Uì‰Uè‰UØ‰UÔ‰Uü‰Uø‹ğ3ÀUh­G dÿ0d‰ 3ÀUhÍ¬G dÿ0d‰ ¸0­G èÛ­ıÿöFN•Ã„Ût¸H­G èÆ­ıÿë
¸h­G èº­ıÿöFMu¸ˆ­G èª­ıÿë
¸œ­G è­ıÿUü‹è0¢ÿÿº´­G Eìè+ŠøÿEì‹Uüè‹øÿ‹Eìèt­ıÿUø‹Fè¢ÿÿƒ}ø t ºÈ­G Eìèú‰øÿEì‹Uøè×Šøÿ‹EìèC­ıÿÆE÷ŠFLşÈtşÈtë
ÆE÷ ëÆE÷öFM…  ‹ÆèƒşÿÿˆEööFMt‹UüŠEöè¯hıÿ„À„á   ‹EøPUì‹Fèˆ¡ÿÿ‹EìPŠE÷P‹FHPh,§G EğP‹MüŠUö‹ÃèÂcÿÿ„ÀuxMì‹Uü°1è%ZıÿEìºà­G è8ŠøÿEìPEèP¸ğ­G ‰EÜUØ‹Eğè™Áøÿ‹EØ‰EàUÔ‹Eğèà:ûÿ‹EÔ‰EäUÜ¹   °4è§Yıÿ‹UèXèî‰øÿ‹Mì²¸Xc@ èÿßøÿèz…øÿ€}÷…  EìP‹Eğ‰EÌÆEĞ UÌ3É¸®G è»Ìøÿ‹Eìè¬ıÿéí   ¸(®G è¬ıÿéŞ   öFMt‹Eüè= ûÿ„À„¾   ‹EøPUì‹Fè‚ ÿÿ‹EìPŠE÷P‹FHPh,§G EğPUè‹Fèb ÿÿ‹Uè‹Mü‹ÃèÙcÿÿ„À…„   Mì‹Uü°1èYıÿEìºà­G è#‰øÿEìPEèP¸P®G ‰EÜUØ‹Eğè„Àøÿ‹EØ‰EàUÔ‹EğèË9ûÿ‹EÔ‰EäUÜ¹   °4è’Xıÿ‹UèXèÙˆøÿ‹Mì²¸Xc@ èêŞøÿèe„øÿë
¸h®G è-«ıÿ3ÀZYYd‰ëéøÿ3Ò¡(I èz™úÿè©„øÿ3ÀZYYd‰h­G EÔº   è#‡øÿEèº   è‡øÿEøº   è	‡øÿÃéGƒøÿëÑ_^[‹å]Ã  ÿÿÿÿ   -- Run entry -- ÿÿÿÿ   Run as: Original user   ÿÿÿÿ   Run as: Current user    ÿÿÿÿ
   Type: Exec  ÿÿÿÿ   Type: ShellExec ÿÿÿÿ
   Filename:   ÿÿÿÿ   Parameters:     ÿÿÿÿ   

    ÿÿÿÿ   CreateProcess   ÿÿÿÿ   Process exit code: %u   ÿÿÿÿ   File doesn't exist. Skipping.   ÿÿÿÿ   ShellExecuteEx  ÿÿÿÿ'   File/directory doesn't exist. Skipping. U‹ìƒÄÈSVW3À‰EÈ3ÀUh±G dÿ0d‰ ¡"I ƒx „I  öœ!I t	€=r"I  t3Àë°ˆEÿ€}ÿ tEÜèˆ‹ıÿ3À‰Eø3ÀUhó°G dÿ0d‰ ¡"I ‹@H…ÀŒŠ  @‰EìÇEô    ‹Uô¡"I è´ùÿ‰Eğ‹Eğö@M…V  ‹Mğ‹X"I ¡P"I èuÿÿ„À„;  ƒ}ø u²¸ØçB è¯=ûÿ‰Eø‹Eğƒx tk3ÀUh¢¯G dÿ0d‰ UÈ‹Eğ‹@èÿÿ‹UÈ¡PI ‹€„  è›ùÿ3ÀZYYd‰ëHéÉ~øÿ‹E‹Pü¡(I è¡–úÿ¡PI ‹€„  ‹I èãšùÿèºøÿë¡PI ‹€„  ‹I èÆšùÿ¡PI ‹€„  ‹ÿRP‹Eğö@M@t4¡PI €x7 tP‹E‹@ü€¸µ   uA‹E‹@üÆ€µ  ‹E‹@üè  ë'‹E‹@ü€¸µ   t‹E‹@üÆ€µ   ‹E‹@üèŞ  ‹Uô°è°´ÿÿ‹Eğ‹@0èÅnÿÿ‹EğèUùÿÿ‹Eğ‹@,èŞnÿÿÿEôÿMì…şÿÿ3ÀZYYd‰hú°G ‹E‹@ü€¸µ   t‹E‹@üÆ€µ   ‹E‹@üèv  ‹EøèŠzøÿ€}ÿ tEÌè‰ıÿUÌEÜè…]ûÿ„ÀuÆr"I Ãéløÿë¥¡(I èÔúÿ3ÀZYYd‰h!±G EÈèç‚øÿÃéEøÿëğ_^[‹å]ÃU‹ìƒÄäSVW3Ò‰Uä‰Uğ‰Uô‰Eü»PI 3ÀUh=´G dÿ0d‰ ÆEû 3ÀUhñ³G dÿ0d‰ ‹è‘µşÿ„ÀuèüÙøÿUğ‹‹€  èü˜ùÿ‹Uğ¸@"I è¿‚øÿ‹èL¸şÿ„ÀuèÏÙøÿUğ‹‹€  èÏ˜ùÿ‹Uğ¸D"I è’‚øÿ‹‹€  è‘şúÿ¢H"I ‹èQ“şÿ£L"I ‹T"I ‹P"I ‹èÍ˜şÿ‹\"I ‹X"I ‹è:™şÿƒ=¸I  t
¡¸I èyÿÿ¡(I èUúÿ‹Eü‹ÿRP€=i"I u¡(I ‹@ Pè¯øÿ‹èW{úÿ‹‹ÿRP3É²‹EüèàôÿÿEúè°?ÿÿ€}ú uèÍìÿÿ3ÀZYYd‰éŸ  ‹‹€´  3Òè‘—ùÿUèüÿÿY±²‹Eüèôÿÿöš!I tj j j h   èåûÿö!I tèKıÿ€=i"I  t‹èÃzúÿ3À r"I ‹…X
I ‰EèÆEìUè3É¸X´G èû¦ıÿ€=r"I  t@€=¿I  u7Uä°QèGbşÿ‹Uä‹è-‹şÿ‹‹€X  ²è²–ùÿ‹‹€\  ²è£–ùÿéŠ   €=°"I  tUô°Oèbşÿë
Uô°Pèûaşÿ‹UôEäè,øÿEäº|´G è‚øÿEä‹ØI èùøÿ‹Uä‹è»Šşÿ€=r"I  u2‹X"I ‹P"I ‹èg‹şÿ‹‹°Ğ  ‹†ü   ‹ÿR…ÀŸÂ‹Æè–ùÿ€=i"I  u¡(I è™úÿ‹Eü‹ÿRPÆEû3ÀZYYd‰ë&ézzøÿ‹Uü¡(I èU’úÿÇ¬"I    è&ëÿÿèu}øÿ3ÀZYYd‰hD´G EäèÔøÿEğèÌøÿEôèÄøÿÃé"|øÿëàŠEû_^[‹å]Ã  ÿÿÿÿ   Need to restart Windows? %s ÿÿÿÿ   

    U‹ìQSVW3À‰Eü3ÀUh]µG dÿ0d‰ ¡PI ‹€Ğ  ‹€ü   ‹ÿR‹ğN…öŒˆ   F3Û¡PI ‹€Ğ  ‹Óèüıÿ„Àtgƒ}ü u²¸ØçB è"8ûÿ‰Eü¡PI ‹€Ğ  ‹Óè%ıÿ‹Ğ¡"I èÉşøÿ‹ø¡PI ‹€Ğ  ‹Óèıÿ‹Ğ°èğ¯ÿÿ‹G0èjÿÿ‹Çè™ôÿÿ‹G,è%jÿÿCN…{ÿÿÿ3ÀZYYd‰hdµG ‹EüèüuøÿÃé{øÿëğ_^[Y]Ã‹ÀU‹ìƒÄøSVW3Ò‰Uø‰Eü3ÀUh˜¶G dÿ0d‰ 3ÀUh\¶G dÿ0d‰ ¡PI èñwúÿ€=r"I  u
èÓşÿÿé   ƒ=ØI  t
¡ØI £¬"I €=¿I  t	Æs"I  ëX i"I ,rtşÈtBëG¡PI ‹€X  Š€  ¢s"I ë/jjjUø°RèD_şÿ‹Eø±3Òè ½ÿÿƒø”s"I ëÆs"I €=s"I  u
¸°¶G èª¡ıÿ±²‹Eüèæğÿÿ3ÀZYYd‰ë!éxøÿ‹Uü¡(I èêúÿÇ¬"I    è{øÿè¶èÿÿ3ÀZYYd‰hŸ¶G Eøèi}øÿÃéÇyøÿëğ_^[YY]Ã  ÿÿÿÿ'   Will not restart Windows automatically. U‹ìQÆEÿˆEşMşUÿ¡PI èå¬şÿ€}ÿ t€}ş tèèÿÿ„Àu3ÀY]Ã°Y]Ã‹ÀS‹ØÆ ƒ=PI  „­   ¡PI èßËùÿ„À„›   ¡PI è5ÊùÿPèû¨øÿ…À„ƒ   ¡PI èÊùÿPèÛ¨øÿ…Àto¡PI ‹€´  èsÉùÿ„Àt[Šƒ¶  ,rt+ëM¡PI ƒ¸4  •Àè=ÿÿÿ„Àt5Ç¬"I    èŠçÿÿë$öœ!I  t€=ÀI  u°èÿÿÿ„ÀtÆ`"I [Ã‹ÀƒJÃ@ U‹ìSV‹]jøSèÔ§øÿ;Et¾   ë3öSè¥øÿ‹Æ^[]Â U‹ìS‹ØöEufÇ  ‹ÃèTÉùÿPhà·G èµ øÿPèg¦øÿ[]Â ‹ÀS‹Øƒ=PI  „‡   ¡PI èºÊùÿ„Àty¡PI €¸Ç    t€»µ   u¡(I ‹@ Pè¥§øÿ…Àt3Àë°‹Øjğ¡PI èáÈùÿPè'§øÿ©   •À:Øt*„Ûtj¡PI è¿ÈùÿPè-©øÿëj ¡PI è«ÈùÿPè©øÿ[Ã@ SV‹ğ3Û‹ƒèGu)¡(I ‹@ Pè1§øÿ÷ØÀ÷Ø:†´  tˆ†´  ‹Æè.ÿÿÿ‹Ã^[ÃSV‹ò‹Ø‹Ö‹Ãèlúÿƒ~ tM‹ÃèHÈùÿ‹Øè¥øÿ;Øu;ƒ=PI  t2¡PI èÂÉùÿ„Àt$¡PI èÈùÿPèâ¦øÿ…Àt¡PI èÈùÿPèÖ§øÿ^[Ã@ SVƒÄØÆl"I  hTºG èºŸøÿ‹ØhdºG SèÅŸøÿ…ÀteT$RÿĞhxºG Sè¯Ÿøÿ‹ğ…ötWTèŸøÿPÿÖ…ÀtJƒ<$ tDèESıÿ„Àt;hˆºG Sè‚Ÿøÿ…Àt,h¤ºG h´ºG èWŸøÿPèiŸøÿ…ÀtÆl"I ë
D$PèzŸøÿf‹D$fƒèrfƒètfƒètëÆHI ëÆHI ëÆHI ëÆHI  ƒÄ(^[Ã kernel32.dll    GetNativeSystemInfo IsWow64Process  GetSystemWow64DirectoryA    RegDeleteKeyExA advapi32.dll    U‹ìƒÄğj jEüP¹4»G º  €3ÀèQ!ûÿ…ÀuFÇEø   EøPEğPEôPj h`»G ‹EüPèÊœøÿ…Àuƒ}ôuƒ}øu
f‹Eğf£€"I ‹EüPègœøÿ‹å]Ã   System\CurrentControlSet\Control\Windows    CSDVersion  U‹ìƒÄøSVW3À‰Eø3ÀUh$¼G dÿ0d‰ j jEüP¹4¼G º  €3Àè“ ûÿ…ÀuiMøºd¼G ‹Eüè¯ûÿ„ÀtLºx¼G ‹EøèV®øÿ…Àu	Æ‚"I ë2ºˆ¼G ‹Eøè<®øÿ…Àu	Æ‚"I ëºœ¼G ‹Eøè"®øÿ…ÀuÆ‚"I ‹EüPè†›øÿ3ÀZYYd‰h+¼G EøèİwøÿÃé;tøÿëğ_^[YY]Ã  System\CurrentControlSet\Control\ProductOptions ProductType ÿÿÿÿ   WinNT   ÿÿÿÿ   LanmanNT    ÿÿÿÿ   ServerNT    ÄĞşÿÿÇ$”   Tèùœøÿ…À„—   3ÀŠD$Áà3ÒŠT$ÁâÂ·T$Â£|"I €=k"I  tn‹D$ƒø|VÇ„$”   œ   „$”   Pè§œøÿ…ÀtI3ÀŠ„$(  Áà3ÒŠ”$*  fÂf£€"I Š„$.  ¢‚"I f‹„$,  f£„"I ëƒøu
èoıÿÿèşÿÿÄ0  Ã@ SV³¾à!I ²¸\¤@ è·møÿ‰ƒÆşËuë^[Ã@ SVWUƒÄøÆ$ğ¿"I »<I ÇD$ I ‹/…ít53À‰‹uNƒş |!‹Ö‹Åèöøÿ‹…ÒtèD>ÿÿëèhøÿNƒşÿuß‹Åèxmøÿ‹D$è×#ûÿƒl$ƒëƒïş$u¬YZ]_^[Ã¸ "I è¶#ûÿ¸$"I è¬#ûÿ¸”"I è¢#ûÿ¸"I è˜#ûÿè[ÿÿÿ¸\"I è‰#ûÿ¸X"I è#ûÿ¸T"I èu#ûÿ¸P"I èk#ûÿ¸ÌI èa#ûÿ¸ÈI èW#ûÿÃ‹ÀU‹ìSVW3ÀUhC¿G dÿ0d‰ ¸HI è¢‹øÿè•ûÿ¢k"I èãúÿÿè
şÿÿÇ„I ä I ²¸Ğ¦@ è|løÿ£ÈI ²¸Ğ¦@ èkløÿ£ÌI ²¸Ğ¦@ èZløÿ£P"I ²¸Ğ¦@ èIløÿ£T"I ²¸Ğ¦@ è8løÿ£X"I ²¸Ğ¦@ è'løÿ£\"I èQşÿÿ²¸Ğ¦@ èløÿ£"I ²¸Ğ¦@ è løÿ£”"I 3ÀZYYd‰hJ¿G Ãéqøÿëø_^[]Ã        Ø¿G             À¿G t   ”	F ,,@ à*@ +@ èÄG H¸@ A d·@ ÔA <A àA ¸A ğA èA œÄG F ,F HF `F |F TInputQueryWizardPage‹ÀTInputQueryWizardPage„¿G ø	F  	ScriptDlg  @         ÀG             xÀG p   ”	F ,,@ à*@ +@ ÜF H¸@ A d·@ ÔA <A àA ¸A ğA èA ˜F F ,F HF `F |F TInputOptionWizardPageTInputOptionWizardPage<ÀG ø	F  	ScriptDlg  ‹À    0ÁG XÁG             BÁG €   ”	F ,,@ à*@ +@ ĞÉG H¸@ A d·@ ÔA <A àA ¸A ğA èA tÉG F ,F HF ˆÊG |F         |@ p   TInputDirWizardPage‹ÀTInputDirWizardPageôÀG ø	F  	ScriptDlg          ÂG             ôÁG €   ”	F ,,@ à*@ +@ ÏG H¸@ A d·@ ÔA <A àA ¸A ğA èA ÎG F ,F HF `F |F TInputFileWizardPage@ TInputFileWizardPage¸ÁG ø	F  	ScriptDlg          ÀÂG             ¨ÂG h   ”	F ,,@ à*@ +@ ÜF H¸@ A d·@ ÔA <A àA ¸A ğA èA ˜F F ,F HF `F |F TOutputMsgWizardPage@ TOutputMsgWizardPagelÂG ø	F  	ScriptDlg          xÃG             \ÃG l   ”	F ,,@ à*@ +@ ÜF H¸@ A d·@ ÔA <A àA ¸A ğA èA ˜F F ,F HF `F |F TOutputMsgMemoWizardPage@ TOutputMsgMemoWizardPage ÃG ø	F  	ScriptDlg          4ÄG             ÄG t   ”	F ,,@ à*@ +@ ÜF H¸@ A d·@ ÔA <A àA ¸A ğA èA ôÕG F ,F HF `F |F TOutputProgressWizardPage‹ÀTOutputProgressWizardPageÜÃG ø	F  	ScriptDlg  @ VWQ‹ú‹ğŠF7ˆ$3Ò‹Æè^…ùÿ‹×‹Æ‹ÿQ<‹Æèâ‡ùÿŠ$‹ÆèD…ùÿZ_^ÃSV„ÒtƒÄğè†høÿ‹Ú‹ğ3Ò‹ÆèáWşÿ²¸\¤@ èmføÿ‰Fd²¸\¤@ è^føÿ‰Fh„Ût
d    ƒÄ‹Æ^[ÃSV‹Ú‹ğ‹Fhèbføÿ‹FdèZføÿ3Ò‹ÆèÕWşÿ„Ût‹ÆèZhøÿ‹Æ^[ÃSVWU‹ê‹ğ‹Î²¸¡D è¦àüÿ‹Ø‰^l‹û3Ò‹ÇèÖãüÿ‹ÆèXşÿ‹Ğ‹Çèf€ùÿº   ¡PI è ‹Ğ‹Çèn€ùÿ²‹Çèõãüÿ‹Õ‹Çè8…ùÿ‹ÆèXşÿ‹Ğ‹Ç‹ÿQ<‹Vl¡PI èûRşÿ‹Øº   ¡PI è¾ Ø‰^p]_^[ÃSVWU‹Ù‹ú‹è…ÿ„‘   ‹Í²¸¡D èàüÿ‹ğ3Ò‹Æè9ãüÿ‹Up‹Æè¯ùÿ‹ÅèØWşÿ‹Ğ‹Æè¿ùÿº   ¡PI è` ‹Ğ‹ÆèÇùÿ²‹ÆèNãüÿ‹×‹Æè‘„ùÿ‹ÅèrWşÿ‹Ğ‹Æè9şÿÿ‹Ö¡PI èURşÿ‹øº   ¡PI è ø}pë3ö‹Í²¸4ÉE è	şÿ‹ø‹Ó‹Çèf	şÿ‹Up‹Çèùÿ‹ÅèEWşÿ‹Ğ‹Çè,ùÿ‹Åè	Wşÿ‹Ğ‹ÇèĞıÿÿº$   ¡PI è½ Ep‹Ö‹Ehè„ìøÿ‹×‹Edèzìøÿ]_^[ÃSV‹ò‹Ø‹Ö‹Cdè íøÿ^[ÃSV‹ò‹Ø‹Ö‹Chèììøÿ^[ÃSVW‹ù‹ò‹Ø‹Ö‹ÃèÆÿÿÿ‹×è{ƒùÿ_^[Ã@ SVW‹ù‹ò‹Ø‹Ö‹Ãè¦ÿÿÿ‹×è‹ƒùÿ_^[Ã@ U‹ìƒÄøSVWˆMû‰Uü‹ğ‹Î²¸¡D è‰Şüÿ‹Ø‰^l‹û3Ò‹Çè¹áüÿ‹ÆèbVşÿ‹Ğ‹ÇèI~ùÿº   ¡PI èê ‹Ğ‹ÇèQ~ùÿ²‹ÇèØáüÿ‹Uü‹Çèƒùÿ‹ÆèûUşÿ‹Ğ‹Ç‹ÿQ<‹Vl¡PI èİPşÿ‹ø‹Î²¸À±D èaôüÿ‹Ø‰^dº   ¡PI è ‹Ğ×‹Ãè²}ùÿ‹ÆèÛUşÿ‹Ğ‹ÃèÂ}ùÿºå   ¡PI èc ‹Ğ+S(‹ÃèÇ}ùÿ€} t	ö›!I u3Òë²‹Ãè{ıÿ‹ÆèhUşÿ‹Ğ‹Fdè.üÿÿŠEûˆFh€} u63Ò‹Fdèóúÿº  €‹Fdègƒùÿº   ¡PI èø ‹^d‰ƒT  Æƒl  _^[YY]Â @ ŠHhQ3Éè   ÃU‹ì€} tQj jj ‹@d3Éè¦öüÿëQj jjjj ‹@d3Éè¿õüÿ]Â @ SVW‹ğ‹Fd‹€ü   ‹ÿR‹øO…ÿ|'G3Û‹Ó‹Fdèıÿ„Àu‹Ó‹FdèÏ
ıÿ„Àt‹ÃëCOuÜƒÈÿ_^[Ã‹@dè´
ıÿÃ@ SVWU‹ê‹ø‹Gd‹€ü   ‹ÿR‹ğN…ö|$F3Û‹Ó‹GdèÂ
ıÿ„Àu;ë”Á‹Ó‹GdèÛıÿCNuß]_^[Ã‹À‹@dèÈıÿÃ@ SV„ÒtƒÄğè®cøÿ‹Ú‹ğ3Ò‹Æè	Sşÿ²¸\¤@ è•aøÿ‰Fh²¸\¤@ è†aøÿ‰Fl²¸\¤@ èwaøÿ‰Ft„Ût
d    ƒÄ‹Æ^[ÃSV‹Ú‹ğ‹Ftèzaøÿ‹Flèraøÿ‹Fhèjaøÿ3Ò‹ÆèåRşÿ„Ût‹Æèjcøÿ‹Æ^[ÃU‹ìj SVW‹Ø3ÀUhzÊG dÿ0d‰ ‹Chèôéøÿ‹ğƒşÿt5‹Ö‹Clè›éøÿ‹ğUü‹Æè;€ùÿ‹CpPMüŠSd3Àè¾8şÿ„Àt
‹Uü‹ÆèL€ùÿ3ÀZYYd‰hÊG Eüè‡iøÿÃéåeøÿëğ_^[Y]ÃSVWUQ‰$‹è‹El‹XK…Û|<C3ÿ‹El‹×è*éøÿ‹ğ±²‹ÆèÑœşÿ„Àu¡PI €x7 t‹Æ‹ÿRx‹$Æ  ëGKuÇ‹$‹Åè}RşÿZ]_^[Ã@ U‹ìƒÄøSVWˆMû‰Uü‹ğ‹Î²¸¡D èÉÚüÿ‹Ø‰^x‹û3Ò‹Çèùİüÿ‹Æè¢Rşÿ‹Ğ‹Çè‰zùÿº   ¡PI è*ÿ  ‹Ğ‹Çè‘zùÿ²‹ÇèŞüÿ‹Uü‹ÇèZùÿ‹Æè;Rşÿ‹Ğ‹Ç‹ÿQ<‹Vx¡PI èMşÿ‹Øº   ¡PI èàş  Ø‰^|ŠEûˆFdFp‹Uè¶høÿ_^[YY]Â U‹ìƒÄğSVW3É‰Mğ‹ò‹Ø3ÀUh±ÍG dÿ0d‰ ÆEôUô3É¡PI èÍû  ‰Eü…ö„‘   ‹Ë²¸¡D èäÙüÿ‹ø3Ò‹Çèİüÿ‹S|‹Çèyùÿ‹Ãè¸Qşÿ‹Ğ‹ÇèŸyùÿº   ¡PI è@ş  ‹Ğ‹Çè§yùÿ²‹Çè.İüÿ‹Ö‹Çèq~ùÿ‹ÃèRQşÿ‹Ğ‹Çèøÿÿ‹×¡PI è5Lşÿ‹ğº   ¡PI èøı  ğs|ë3ÿ‹Ë²¸VB è]Áúÿ‰Eø‹S|‹Eøèyùÿº
   ¡PI è´ı  P‹ÃèQşÿ‹Ğ+UüX+Ğ‹Eøèüxùÿ‹ÃèÙPşÿ‹Ğ‹EøèŸ÷ÿÿ‹Ë²¸ØmB è1àúÿ‹ğ‹ÃèäPşÿ‹Ğ+Uü‹Æèˆxùÿ‹Eø‹P(J‹Æèšxùÿ‹Uü‹Æè°xùÿ¡PI ‹€¸  ‹P0‹Æè»xùÿ‹Clƒx u‹ÄI ‹Æè}ùÿëUğ¡ÄI èÂûÿ‹Uğ‹Æèh}ùÿ‰œ   Ç†˜   ÊG ‹Ãè9Pşÿ‹Ğ‹Æè ÷ÿÿº$   ¡PI èíü  C|‹Ö‹Chè´åøÿ‹×‹Ctèªåøÿ‹Uø‹ClèŸåøÿ‹Ø3ÀZYYd‰h¸ÍG EğèPføÿÃé®bøÿëğ‹Ã_^[‹å]Ã@ SV‹ò‹Ø‹Ö‹Chè æøÿ^[ÃSV‹ò‹Ø‹Ö‹Clèìåøÿ^[ÃSV‹ò‹Ø‹Ö‹CtèØåøÿ^[ÃSVW‹ù‹ò‹Ø‹Ö‹ÃèÆÿÿÿ‹×èg|ùÿ_^[Ã@ U‹ìj j SVW‹ù‹ò‹Ø3ÀUhÎG dÿ0d‰ Uø‹Çè¤øúÿ‹EøUüèAüúÿ‹EüP‹Ö‹ÃèxÿÿÿZèJ|ùÿ3ÀZYYd‰hˆÎG Eøº   è eøÿÃéŞaøÿëë_^[YY]ÃSV„ÒtƒÄğè’^øÿ‹Ú‹ğ3Ò‹ÆèíMşÿ²¸\¤@ èy\øÿ‰Fd²¸\¤@ èj\øÿ‰Fh²¸Ğ¦@ è[\øÿ‰Fl²¸Ğ¦@ èL\øÿ‰Fp²¸\¤@ è=\øÿ‰Ft„Ût
d    ƒÄ‹Æ^[Ã@ SV‹Ú‹ğ‹Ftè>\øÿ‹Fpè6\øÿ‹Flè.\øÿ‹Fhè&\øÿ‹Fdè\øÿ3Ò‹Æè™Mşÿ„Ût‹Æè^øÿ‹Æ^[ÃU‹ì3ÉQQQQQQSVW‹Ø3ÀUh“ĞG dÿ0d‰ ‹Cdè¢äøÿ‹ğƒşÿ„ó   ‹Ö‹ChèEäøÿ‰EüUø‹Eüèãzùÿ‹Ö‹ÃèJ  „Àu[Mô‹Ö‹Cp‹8ÿW‹EôPMğ‹Ö‹Cl‹8ÿW‹EğP‹ÃèÅMşÿè ±ùÿPUì‹EøèXøúÿ‹EìPUè¡ÄI èûÿ‹EèUøYè·øıÿ„Àuh‹Ö‹Ãèâ  „ÀtfMô‹Ö‹Cp‹8ÿW‹EôPMğ‹Ö‹Cl‹0ÿV‹EğP‹Ãè]Mşÿè8±ùÿPUì‹Eøèğ÷úÿ‹EìPUè¡ÄI è£ûÿ‹EèUøYèsøıÿ„Àt‹Uø‹Eüè8zùÿ3ÀZYYd‰hšĞG Eèº   ècøÿÃéÌ_øÿëë_^[‹å]Ã@ SVWU‹ê‹ğ‹Î²¸¡D èÕüÿ‹Ø‰^x‹û3Ò‹ÇèJØüÿ‹ÆèóLşÿ‹Ğ‹ÇèÚtùÿº   ¡PI è{ù  ‹Ğ‹Çèâtùÿ²‹ÇèiØüÿ‹Õ‹Çè¬yùÿ‹ÆèLşÿ‹Ğ‹Ç‹ÿQ<‹Vx¡PI èoGşÿ‹Øº   ¡PI è2ù  Ø‰^|]_^[ÃU‹ìƒÄìSVW3Û‰]ì‰Mü‹ò‹Ø3ÀUhbÓG dÿ0d‰ ÆEğUğ3É¡PI è2ö  ‰Eø…ö„‘   ‹Ë²¸¡D èIÔüÿ‹ø3Ò‹Çè~×üÿ‹S|‹Çèôsùÿ‹ÃèLşÿ‹Ğ‹Çètùÿº   ¡PI è¥ø  ‹Ğ‹Çètùÿ²‹Çè“×üÿ‹Ö‹ÇèÖxùÿ‹Ãè·Kşÿ‹Ğ‹Çè~òÿÿ‹×¡PI èšFşÿ‹ğº   ¡PI è]ø  ğs|ë3ÿ‹Ë²¸VB èÂ»úÿ‰Eô‹S|‹Eôèhsùÿº
   ¡PI èø  P‹ÃèKşÿ‹Ğ+UøX+Ğ‹Eôèasùÿ‹Ãè>Kşÿ‹Ğ‹Eôèòÿÿ‹Ë²¸ØmB è–Úúÿ‹ğ‹ÃèIKşÿ‹Ğ+Uø‹Æèírùÿ‹Eô‹P(J‹Æèÿrùÿ‹Uø‹Æèsùÿ¡PI ‹€¸  ‹P0‹Æè sùÿ‹Cdƒx u‹ÄI ‹ÆèæwùÿëUì¡ÄI è'ûÿ‹Uì‹ÆèÍwùÿ‰œ   Ç†˜   TÏG ‹ÃèJşÿ‹Ğ‹Æèeñÿÿº$   ¡PI èR÷  C|‹Uü‹Cp‹ÿQ,‹U‹Cl‹ÿQ,‹Ö‹Cdèàøÿ‹×‹Ctèùßøÿ‹Uô‹Chèîßøÿ‹Ø3ÀZYYd‰hiÓG EìèŸ`øÿÃéı\øÿëğ‹Ã_^[‹å]Â SV‹ò‹Ø‹Ö‹CdèPàøÿ^[ÃSV‹ò‹Ø‹Ö‹Chè<àøÿ^[ÃSV‹ò‹Ø‹Ö‹Ctè(àøÿ^[ÃSVW‹ù‹ò‹Ø‹Ö‹ÃèÆÿÿÿ‹×è·vùÿ_^[Ã@ SVW‹ù‹ò‹Ø‹Ö‹Ãè¦ÿÿÿ‹×èÇvùÿ_^[Ã@ SV‹ò‹Ø‹Ö‹Ãèuÿÿÿƒx”À^[Ã@ SV‹ò‹Ø„Ét‹Ö‹ÃèUÿÿÿÇ@   ^[Ã‹Ö‹ÃèBÿÿÿ3Ò‰P^[Ã‹ÀSVWU‹ê‹ğ‹Î²¸¡D è‚Ñüÿ‹Ø‰^d‹û3Ò‹Çè²Ôüÿ‹Æè[Işÿ‹Ğ‹ÇèBqùÿº   ¡PI èãõ  ‹Ğ‹ÇèJqùÿ²‹ÇèÑÔüÿ‹Õ‹Çèvùÿ‹ÆèõHşÿ‹Ğ‹Ç‹ÿQ<‹Vd¡PI è×Cşÿ]_^[Ã‹ÀSVWUQ‰$‹Ú‹ğ3í…Û„Š   ‹Î²¸¡D èğĞüÿ‹ø‰~h3Ò‹Çè"Ôüÿ‹ÆèËHşÿ‹Ğ‹Çè²pùÿº   ¡PI èSõ  ‹Ğ‹Çèºpùÿ²‹ÇèAÔüÿ‹Ó‹Çè„uùÿ‹ÆèeHşÿ‹Ğ‹Ç‹ÿQ<º   ¡PI èõ  ‹Ø‹Vh¡PI è6CşÿØëë3À‰Fh‹Î²¸ğD è5ıÿ‹Ø‰^d‹Õ‹Ãèpùÿ‹Æè<Hşÿ‹Ğ‹Ãè#pùÿºå   ¡PI èÄô  ‹Ğ+Õ‹Ãè)pùÿ²‹Ãèxºúÿ²‹ÃèçÅúÿÆƒ   ‹ÆèÍGşÿ‹Ğ‹Fdè“îÿÿ‹^d²‹Ãèkıÿ‹$‹Ãè•ıÿZ]_^[Ã@ SV„ÒtƒÄğè.Wøÿ‹Ú‹ğ3Ò‹Æè‰Fşÿ 0ÖG 
F\ˆF\„Ût
d    ƒÄ‹Æ^[Ã      SVWU‹ğ½PI ‹Î²¸¡D è‡Ïüÿ‹Ø‰^d‹û3Ò‹Çè·Òüÿ3Ò‹ÇèÚÒüÿ‹ÆèWGşÿ‹Ğ‹Çè>oùÿ‹E ‹€„  ‹P0‹ÇèKoùÿ‹E ‹€„  Š  ‹ÇèÅÒüÿ‹ÆèòFşÿ‹Ğ‹Ç‹ÿQ<‹Î²¸¡D èÏüÿ‹Ø‰^h3Ò‹ÃèIÒüÿ3Ò‹ÃèlÒüÿº   ‹E èƒó  ‹Ğ‹Ãèªnùÿ‹ÆèÓFşÿ‹Ğ‹Ãèºnùÿ‹E ‹€ˆ  ‹P0‹ÃèÇnùÿ‹Æè„Fşÿ‹Ğ‹FhèJíÿÿ‹Î²¸TíD èHıÿ‹Ø‰^lº*   ‹E è&ó  ‹Ğ‹ÃèMnùÿ‹ÆèvFşÿ‹Ğ‹Ãè]nùÿº   ‹E è ó  ‹Ğ‹Ãègnùÿ3Ò‹Ãèfrùÿ‹ÆèFşÿ‹Ğ‹Flèáìÿÿ]_^[ÃS‹Ø¡PI ‹€4  ;C uƒ{p t‹Sp¡PI èª{şÿ3À‰Cp[Ã@ ‹PI ‹’4  ;P u
¡(I èmlúÿÃSV‹ò‹Ø…É~ ‹Ñ‹Clètıÿ‹Ö‹Clè~ıÿ²‹ClèÜqùÿë
3Ò‹ClèĞqùÿ‹Ãè©ÿÿÿ^[Ã‹ÀU‹ìƒÄøSVW3Û‰]ø‰Mü‹Ø3ÀUh}ØG dÿ0d‰ ‹CdèorùÿEøP‹sh‹N,‹VD‹EüèFûÿ‹Uø‹ÆèPrùÿ‹ÃèUÿÿÿ3ÀZYYd‰h„ØG Eøè„[øÿÃéâWøÿëğ_^[YY]ÃS‹Ø¡PI ‹€4  ;C t‰Cp‹S ¡PI è­zşÿ‹Ãèÿÿÿ[Ãº¡D èr~üÿÃSVW‹ù‹ò‹Ø‹×‹Ãèúúüÿˆ_^[Ã‡ÊèUÿüÿÃSVW‹ù‹ò‹Ø‹×‹Ãè†ûüÿˆ_^[ÃU‹ìj SVW‹ù‹ò‹Ø3ÀUhDÙG dÿ0d‰ Mü‹×‹Ãèúüÿ‹Uü‹Æè"[øÿ3ÀZYYd‰hKÙG Eüè½ZøÿÃéWøÿëğ_^[Y]Ã@ ‡ÊèşüÿÃSVW‹ù‹ò‹Ø‹×‹Ãèzúüÿˆ_^[Ã‡Êè=ıÿÃSVW‹ù‹ò‹Ø‹×‹Ãè~úüÿˆ_^[ÃSVW‹ù‹ò‹Ø‹×‹Ãè~úüÿ‰_^[Ã‡Êè5ıÿÃU‹ìj SVW‹ù‹ò‹Ø3ÀUhüÙG dÿ0d‰ Mü‹×‹Ãè¸úüÿ‹Uü‹ÆèjZøÿ3ÀZYYd‰hÚG EüèZøÿÃécVøÿëğ_^[Y]Ã@ ‡ÊèıÿÃSºÀ±D è}üÿ‹Øº„¾D ¹ÛG ‹Ãè2züÿº0¿D ¹ÛG ‹Ãè!züÿºT¿D ¹,ÛG ‹ÃèzüÿºXİD ¹DÛG ‹ÃèÿyüÿhXÛG ¹ÜØG ºÄØG ‹ÃèqzüÿhhÛG ºäØG 3É‹Ãè^züÿhxÛG ¹TÙG ºüØG ‹ÃèHzüÿhŒÛG ¹tÙG º\ÙG ‹Ãè2züÿh ÛG º|ÙG 3É‹Ãèzüÿh´ÛG ¹¬ÙG º”ÙG ‹Ãè	züÿhÈÛG ¹ÚG º´ÙG ‹Ãèóyüÿ[Ã ÿÿÿÿ   ADDCHECKBOX ÿÿÿÿ   ADDGROUP    ÿÿÿÿ   ADDRADIOBUTTON  ÿÿÿÿ	   CheckItem   ÿÿÿÿ   CHECKED ÿÿÿÿ   STATE   ÿÿÿÿ   ITEMCAPTION ÿÿÿÿ   ITEMENABLED ÿÿÿÿ	   ITEMLEVEL   ÿÿÿÿ
   ITEMOBJECT  ÿÿÿÿ   ITEMSUBITEM ºTíD èV{üÿÃè›ıÿÃ‹ÀºğD èB{üÿhÜG ¹àÛG 3ÒèéxüÿÃÿÿÿÿ   RTFTEXT è»óıÿÃ‹ÀŠ€  ˆÃ@ º4ÉE è{üÿhPÜG ¹ÜG ºÜG è¦xüÿÃ ÿÿÿÿ   Password    è“şÿÃ‹ÀSV‹ò‹Ø‹Æ‹“   èÙWøÿ^[Ã‹ÀSºpĞE è­züÿ‹ØºğçE ¹ÌÜG ‹ÃèÊwüÿºüèE ¹äÜG ‹Ãè¹wüÿh İG ¹\ÜG ºdÜG ‹Ãè+xüÿ[Ã ÿÿÿÿ   ChangeDirectory ÿÿÿÿ   CreateNewDirectory  ÿÿÿÿ	   Directory   º€ÓE èzüÿºĞóE ¹0İG è?wüÿÃ  ÿÿÿÿ   SetPaths    ºĞÑE èîyüÿÃèÿäıÿÃ‹ÀŠ€Ä   ˆÃ@ èÓäıÿÃ‹À‹€À   ‰Ã@ è§äıÿÃ‹À‹€¼   ‰Ã@ èoäıÿÃ‹À‹€´   ‰Ã@ ègäıÿÃ‹ÀŠ€¸   ˆÃ@ è/äıÿÃ‹À‹€°   ‰Ã@ èäıÿÃ‹ÀŠ€¬   ˆÃ@ SºD¼E èUyüÿ‹ØhˆŞG ¹ÀİG ºÈİG ‹ÃèõvüÿhœŞG ¹¬İG º´İG ‹Ãèßvüÿh°ŞG ¹˜İG º İG ‹ÃèÉvüÿhÀŞG ¹„İG ºŒİG ‹Ãè³vüÿhĞŞG ¹pİG ºxİG ‹ÃèvüÿhèŞG ¹\İG ºdİG ‹Ãè‡vüÿhßG ¹HİG ºPİG ‹Ãèqvüÿ[Ã   ÿÿÿÿ   AutoSize    ÿÿÿÿ	   BackColor   ÿÿÿÿ   Center  ÿÿÿÿ   Bitmap  ÿÿÿÿ   ReplaceColor    ÿÿÿÿ   ReplaceWithColor    ÿÿÿÿ   Stretch ès!şÿÃ‹À‹€ü   ‰Ã@ SVW‹ù‹ò‹Ø‹×‹Ãèº şÿ‰_^[ÃSV‹ò‹Ø‹Ãè³ şÿ‰^[Ã‹ÀSº˜ôE èİwüÿ‹Øº@ÿE ¹°ßG ‹ÃèútüÿhÈßG º8ßG 3É‹ÃèouüÿhÜßG º ßG 3É‹Ãè\uüÿhìßG ¹ßG ºßG ‹ÃèFuüÿ[Ãÿÿÿÿ   FindNextPage    ÿÿÿÿ	   PageCount   ÿÿÿÿ   Pages   ÿÿÿÿ
   ActivePage  è;şÿÃ‹ÀSV‹ò‹Ø‹Ãè_şÿ‰^[Ã‹ÀèëşÿÃ‹À‹€   ‰Ã@ SºàøE èwüÿ‹ØhlàG ¹àG ºàG ‹Ãè¡tüÿh€àG ¹øßG º àG ‹Ãè‹tüÿ[Ã ÿÿÿÿ   Notebook    ÿÿÿÿ	   PageIndex   ºäC èvüÿÃSºtÂH è‘vüÿ‹ØºxÉH ¹ÔàG ‹Ãè®süÿºTÈH ¹äàG ‹Ãèsüÿ[Ã   ÿÿÿÿ   Center  ÿÿÿÿ   CenterInsideControl ºìG è2vüÿº4¤G ¹áG èSsüÿÃ  ÿÿÿÿ   ShowAboutBox    ‹€´  ‰Ã@ ‹€¸  ‰Ã@ ‹€¼  ‰Ã@ ‹€À  ‰Ã@ ‹€Ä  ‰Ã@ ‹€   ‰Ã@ ‹€ü  ‰Ã@ ‹€ø  ‰Ã@ ‹€ô  ‰Ã@ ‹€ğ  ‰Ã@ ‹€ì  ‰Ã@ ‹€è  ‰Ã@ ‹€ä  ‰Ã@ ‹€à  ‰Ã@ ‹€Ü  ‰Ã@ ‹€Ø  ‰Ã@ ‹€Ô  ‰Ã@ ‹€Ğ  ‰Ã@ ‹€Ì  ‰Ã@ ‹€È  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€  ‰Ã@ ‹€   ‰Ã@ ‹€$  ‰Ã@ ‹€(  ‰Ã@ ‹€,  ‰Ã@ ‹€0  ‰Ã@ ‹€4  ‰Ã@ ‹€8  ‰Ã@ ‹€<  ‰Ã@ ‹€@  ‰Ã@ ‹€D  ‰Ã@ ‹€H  ‰Ã@ ‹€L  ‰Ã@ ‹€P  ‰Ã@ ‹€T  ‰Ã@ ‹€X  ‰Ã@ ‹€\  ‰Ã@ ‹€`  ‰Ã@ ‹€d  ‰Ã@ ‹€h  ‰Ã@ ‹€l  ‰Ã@ ‹€p  ‰Ã@ ‹€t  ‰Ã@ ‹€x  ‰Ã@ ‹€|  ‰Ã@ ‹€€  ‰Ã@ ‹€„  ‰Ã@ ‹€ˆ  ‰Ã@ ‹€Œ  ‰Ã@ ‹€  ‰Ã@ ‹€”  ‰Ã@ ‹€˜  ‰Ã@ ‹€œ  ‰Ã@ ‹€   ‰Ã@ ‹€¤  ‰Ã@ ‹€¨  ‰Ã@ ‹€¬  ‰Ã@ ‹€°  ‰Ã@ ‹€´  ‰Ã@ ‹€¸  ‰Ã@ ‹€¼  ‰Ã@ ‹€À  ‰Ã@ ‹€Ä  ‰Ã@ ‹€È  ‰Ã@ ‹€Ì  ‰Ã@ ‹€Ğ  ‰Ã@ ‹€4  ‰Ã@ ‹€Ô  ‰Ã@ ‹€Ø  ‰Ã@ ‹€Ü  ‰Ã@ ‹€à  ‰Ã@ ‹€ä  ‰Ã@ ‹€è  ‰Ã@ SºL
F èIrüÿ‹ØhøêG º,áG 3É‹ÃèìoüÿhëG º8áG 3É‹ÃèÙoüÿh$ëG ºDáG 3É‹ÃèÆoüÿh8ëG ºPáG 3É‹Ãè³oüÿhPëG º\áG 3É‹Ãè oüÿhhëG ºâG 3É‹Ãèoüÿh|ëG ºâG 3É‹ÃèzoüÿhëG ºøáG 3É‹Ãègoüÿh¨ëG ºìáG 3É‹ÃèToüÿh¼ëG ºàáG 3É‹ÃèAoüÿhÔëG ºÔáG 3É‹Ãè.oüÿhìëG ºÈáG 3É‹ÃèoüÿhìG º¼áG 3É‹ÃèoüÿhìG º°áG 3É‹Ãèõnüÿh<ìG º¤áG 3É‹Ãèânüÿh\ìG º˜áG 3É‹ÃèÏnüÿhtìG ºŒáG 3É‹Ãè¼nüÿhˆìG º€áG 3É‹Ãè©nüÿh ìG ºtáG 3É‹Ãè–nüÿh¸ìG ºháG 3É‹ÃèƒnüÿhĞìG ºâG 3É‹ÃèpnüÿhèìG º(âG 3É‹Ãè]nüÿhøìG º4âG 3É‹ÃèJnüÿhíG º@âG 3É‹Ãè7nüÿh$íG ºLâG 3É‹Ãè$nüÿh<íG ºXâG 3É‹ÃènüÿhTíG ºdâG 3É‹ÃèşmüÿhpíG ºpâG 3É‹Ãèëmüÿh„íG º|âG 3É‹ÃèØmüÿh˜íG ºˆâG 3É‹ÃèÅmüÿh¨íG º”âG 3É‹Ãè²müÿhÄíG º âG 3É‹ÃèŸmüÿhÜíG º¬âG 3É‹ÃèŒmüÿhôíG º¸âG 3É‹ÃèymüÿhîG ºÄâG 3É‹Ãèfmüÿh(îG ºĞâG 3É‹ÃèSmüÿh8îG ºÜâG 3É‹Ãè@müÿhPîG ºèâG 3É‹Ãè-müÿhpîG ºôâG 3É‹ÃèmüÿhîG º ãG 3É‹Ãèmüÿh¤îG ºãG 3É‹Ãèôlüÿh¼îG ºãG 3É‹ÃèálüÿhĞîG º$ãG 3É‹ÃèÎlüÿhàîG º0ãG 3É‹Ãè»lüÿhüîG º<ãG 3É‹Ãè¨lüÿhïG ºHãG 3É‹Ãè•lüÿh,ïG ºTãG 3É‹Ãè‚lüÿh@ïG º`ãG 3É‹ÃèolüÿhXïG ºlãG 3É‹Ãè\lüÿhtïG ºxãG 3É‹ÃèIlüÿhŒïG º„ãG 3É‹Ãè6lüÿh°ïG ºãG 3É‹Ãè#lüÿhÈïG ºœãG 3É‹ÃèlüÿhÜïG º¨ãG 3É‹ÃèıküÿhôïG º´ãG 3É‹ÃèêküÿhğG ºÀãG 3É‹Ãè×küÿh$ğG ºÌãG 3É‹ÃèÄküÿhHğG ºØãG 3É‹Ãè±küÿhhğG ºäãG 3É‹Ãèküÿh„ğG ºğãG 3É‹Ãè‹küÿh¤ğG ºüãG 3É‹ÃèxküÿhÄğG ºäG 3É‹ÃèeküÿhàğG ºäG 3É‹ÃèRküÿhüğG º äG 3É‹Ãè?küÿhñG º,äG 3É‹Ãè,küÿh0ñG º8äG 3É‹ÃèküÿhTñG ºDäG 3É‹ÃèküÿhlñG ºPäG 3É‹ÃèójüÿhŒñG º\äG 3É‹Ãèàjüÿh¨ñG ºhäG 3É‹ÃèÍjüÿhÄñG ºtäG 3É‹ÃèºjüÿhØñG º€äG 3É‹Ãè§jüÿhèñG º˜äG 3É‹Ãè”jüÿh òG º¤äG 3É‹ÃèjüÿhòG º°äG 3É‹Ãènjüÿh<òG º¼äG 3É‹Ãè[jüÿh\òG ºÈäG 3É‹ÃèHjüÿh|òG ºÔäG 3É‹Ãè5jüÿh¨òG ºŒäG 3É‹Ãè"jüÿºF ¹¼òG ‹Ãè‰iüÿºÀF ¹ØòG ‹Ãèxiüÿ[Ã  ÿÿÿÿ   CANCELBUTTON    ÿÿÿÿ
   NEXTBUTTON  ÿÿÿÿ
   BACKBUTTON  ÿÿÿÿ   OuterNotebook   ÿÿÿÿ   InnerNotebook   ÿÿÿÿ   WelcomePage ÿÿÿÿ	   InnerPage   ÿÿÿÿ   FinishedPage    ÿÿÿÿ   LicensePage ÿÿÿÿ   PasswordPage    ÿÿÿÿ   InfoBeforePage  ÿÿÿÿ   UserInfoPage    ÿÿÿÿ# Makefile.in generated by automake 1.10 from Makefile.am.
# data/Makefile.  Generated from Makefile.in by configure.

# Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006  Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.




pkgdatadir = $(datadir)/etracer
pkglibdir = $(libdir)/etracer
pkgincludedir = $(includedir)/etracer
am__cd = CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd
install_sh_DATA = $(install_sh) -c -m 644
install_sh_PROGRAM = $(install_sh) -c
install_sh_SCRIPT = $(install_sh) -c
INSTALL_HEADER = $(INSTALL_DATA)
transform = $(program_transform_name)
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_triplet = x86_64-unknown-linux-gnu
host_triplet = x86_64-unknown-linux-gnu
target_triplet = x86_64-unknown-linux-gnu
subdir = data
DIST_COMMON = $(nobase_dist_ppdata_DATA) $(srcdir)/Makefile.am \
	$(srcdir)/Makefile.in
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
am__aclocal_m4_deps = $(top_srcdir)/configure.ac
am__configure_deps = $(am__aclocal_m4_deps) $(CONFIGURE_DEPENDENCIES) \
	$(ACLOCAL_M4)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = $(top_builddir)/config.h
CONFIG_CLEAN_FILES =
SOURCES =
DIST_SOURCES =
am__vpath_adj_setup = srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`;
am__vpath_adj = case $$p in \
    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
    *) f=$$p;; \
  esac;
am__strip_dir = `echo $$p | sed -e 's|^.*/||'`;
am__installdirs = "$(DESTDIR)$(ppdatadir)"
nobase_dist_ppdataDATA_INSTALL = $(install_sh_DATA)
DATA = $(nobase_dist_ppdata_DATA)
DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
ACLOCAL = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run aclocal-1.10
AMTAR = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run tar
AUTOCONF = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run autoconf
AUTOHEADER = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run autoheader
AUTOMAKE = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run automake-1.10
AWK = mawk
CC = gcc
CCDEPMODE = depmode=gcc3
CFLAGS = -g -O2  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
CPP = gcc -E
CPPFLAGS =   -DTUXRACER_NO_ASSERT=1 -DHAVE_SDL_MIXER=1 
CXX = g++
CXXDEPMODE = depmode=gcc3
CXXFLAGS = -g -O2  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/tcl8.4 -DPNG_NO_MMX_CODE -I/usr/include/libpng12   -I/usr/include/freetype2
CYGPATH_W = echo
DEFS = -DHAVE_CONFIG_H
DEPDIR = .deps
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EGREP = /bin/grep -E
EXEEXT = 
FT2_CFLAGS = -I/usr/include/freetype2
FT2_CONFIG = /usr/bin/freetype-config
FT2_LIBS = -lfreetype -lz
GREP = /bin/grep
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = $(install_sh) -c -s
LDFLAGS = 
LIBOBJS = 
LIBS =   -lSM -lICE  -lX11 -lXi -lXext -lXmu -lXt   -ldl -lm -L/usr/lib -lSDL -lSDL_mixer  -lGL -lGLU -L/usr/lib -ltcl8.4${TCL_DBGX} -ldl  -lpthread -lieee -lm -lpng12   -lfreetype -lz
LTLIBOBJS = 
MAINT = #
MAKEINFO = ${SHELL} /home/christian/Desktop/extremetuxracer/missing --run makeinfo
MKDIR_P = /bin/mkdir -p
OBJEXT = o
PACKAGE = etracer
PACKAGE_BUGREPORT = 
PACKAGE_NAME = 
PACKAGE_STRING = 
PACKAGE_TARNAME = 
PACKAGE_VERSION = 
PATH_SEPARATOR = :
RANLIB = ranlib
SDL_CFLAGS = -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_CONFIG = /usr/bin/sdl-config
SDL_LIBS = -L/usr/lib -lSDL
SET_MAKE = 
SHELL = /bin/bash
STRIP = 
TR_CFLAGS =  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
TR_CPPFLAGS =  -DTUXRACER_NO_ASSERT=1 -DHAVE_SDL_MIXER=1 
TR_CXXFLAGS =  -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
TR_LIBS =  -lSM -lICE  -lX11 -lXi -lXext -lXmu -lXt   -ldl -lm -L/usr/lib -lSDL -lSDL_mixer  -lGL -lGLU
VERSION = SVN Development
XMKMF = 
X_CFLAGS = 
X_EXTRA_LIBS = 
X_LIBS = 
X_PRE_LIBS =  -lSM -lICE
abs_builddir = /home/christian/Desktop/extremetuxracer/data
abs_srcdir = /home/christian/Desktop/extremetuxracer/data
abs_top_builddir = /home/christian/Desktop/extremetuxracer
abs_top_srcdir = /home/christian/Desktop/extremetuxracer
ac_ct_CC = gcc
ac_ct_CXX = g++
am__include = include
am__leading_dot = .
am__quote = 
am__tar = ${AMTAR} chof - "$$tardir"
am__untar = ${AMTAR} xf -
bindir = ${exec_prefix}/bin
build = x86_64-unknown-linux-gnu
build_alias = 
build_cpu = x86_64
build_os = linux-gnu
build_vendor = unknown
builddir = .
datadir = ${datarootdir}
datarootdir = ${prefix}/share
docdir = ${datarootdir}/doc/${PACKAGE}
dvidir = ${docdir}
exec_prefix = ${prefix}
host = x86_64-unknown-linux-gnu
host_alias = 
host_cpu = x86_64
host_os = linux-gnu
host_vendor = unknown
htmldir = ${docdir}
includedir = ${prefix}/include
infodir = ${datarootdir}/info
install_sh = $(SHELL) /home/christian/Desktop/extremetuxracer/install-sh
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localedir = ${datarootdir}/locale
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
mkdir_p = /bin/mkdir -p
oldincludedir = /usr/include
pdfdir = ${docdir}
ppdatadir = ${datarootdir}/etracer
prefix = /usr/local
program_transform_name = s,x,x,
psdir = ${docdir}
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
target = x86_64-unknown-linux-gnu
target_alias = 
target_cpu = x86_64
target_os = linux-gnu
target_vendor = unknown
top_builddir = ..
top_srcdir = ..
nobase_dist_ppdata_DATA = \
			courses/contrib/doing/terrain.png \
			courses/contrib/doing/elev.png \
			courses/contrib/doing/preview.png \
			courses/contrib/doing/trees.png \
			courses/contrib/doing/course.tcl \
courses/events/c-mountain_mania/herringrunicon.png \
courses/events/c-mountain_mania/event.tcl \
courses/events/c-mountain_mania/cupicon.png \
courses/events/c-mountain_mania/hippo_run/course.tcl \
courses/events/c-mountain_mania/hippo_run/preview.png \
courses/events/c-mountain_mania/hippo_run/trees.png \
courses/events/c-mountain_mania/hippo_run/terrain.png \
courses/events/c-mountain_mania/hippo_run/elev.png \
courses/events/c-mountain_mania/in_search_of_vodka/course.tcl \
courses/events/c-mountain_mania/in_search_of_vodka/elev.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/preview.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/trees.rgb \
courses/events/c-mountain_mania/in_search_of_vodka/terrain.rgb \
courses/events/c-mountain_mania/crazy_path/course.tcl \
courses/events/c-mountain_mania/crazy_path/preview.png \
courses/events/c-mountain_mania/crazy_path/trees.png \
courses/events/c-mountain_mania/crazy_path/terrain.png \
courses/events/c-mountain_mania/crazy_path/elev.png \
courses/events/c-mountain_mania/hey_tux/course.tcl \
courses/events/c-mountain_mania/hey_tux/elev.rgb \
courses/events/c-mountain_mania/hey_tux/preview.rgb \
courses/events/c-mountain_mania/hey_tux/trees.rgb \
courses/events/c-mountain_mania/hey_tux/terrain.rgb \
courses/events/c-mountain_mania/ice_pipeline/course.tcl \
courses/events/c-mountain_mania/ice_pipeline/elev.rgb \
courses/events/c-mountain_mania/ice_pipeline/preview.png \
courses/events/c-mountain_mania/ice_pipeline/trees.rgb \
courses/events/c-mountain_mania/ice_pipeline/terrain.rgb \
courses/events/c-mountain_mania/slippy_slidey/course.tcl \
courses/events/c-mountain_mania/slippy_slidey/preview.png \
courses/events/c-mountain_mania/slippy_slidey/trees.png \
courses/events/c-mountain_mania/slippy_slidey/terrain.png \
courses/events/c-mountain_mania/slippy_slidey/elev.png \
courses/events/c-mountain_mania/volcanoes/course.tcl \
courses/events/c-mountain_mania/volcanoes/preview.png \
courses/events/c-mountain_mania/volcanoes/trees.png \
courses/events/c-mountain_mania/volcanoes/terrain.png \
courses/events/c-mountain_mania/volcanoes/elev.png \
courses/events/c-mountain_mania/merry_go_round/course.tcl \
courses/events/c-mountain_mania/merry_go_round/preview.png \
courses/events/c-mountain_mania/merry_go_round/trees.png \
courses/events/c-mountain_mania/merry_go_round/terrain.png \
courses/events/c-mountain_mania/merry_go_round/elev.png \
courses/events/c-mountain_mania/bobsled_ride/course.tcl \
courses/events/c-mountain_mania/bobsled_ride/preview.png \
courses/events/c-mountain_mania/bobsled_ride/trees.png \
courses/events/c-mountain_mania/bobsled_ride/terrain.png \
courses/events/c-mountain_mania/bobsled_ride/elev.png \
courses/events/c-mountain_mania/candy_lane/course.tcl \
courses/events/c-mountain_mania/candy_lane/preview.png \
courses/events/c-mountain_mania/candy_lane/trees.png \
courses/events/c-mountain_mania/candy_lane/terrain.png \
courses/events/c-mountain_mania/candy_lane/elev.png \
courses/events/c-mountain_mania/nature_stroll/course.tcl \
courses/events/c-mountain_mania/nature_stroll/preview.png \
courses/events/c-mountain_mania/nature_stroll/trees.png \
courses/events/c-mountain_mania/nature_stroll/terrain.png \
courses/events/c-mountain_mania/nature_stroll/elev.png \
courses/events/b-herring_run/herringrunicon.png \
courses/events/b-herring_run/hazzard_valley/course.tcl \
courses/events/b-herring_run/hazzard_valley/preview.png \
courses/events/b-herring_run/hazzard_valley/trees.png \
courses/events/b-herring_run/hazzard_valley/terrain.png \
courses/events/b-herring_run/hazzard_valley/elev.png \
courses/events/b-herring_run/mount_herring/course.tcl \
courses/events/b-herring_run/mount_herring/preview.png \
courses/events/b-herring_run/mount_herring/trees.png \
courses/events/b-herring_run/mount_herring/terrain.png \
courses/events/b-herring_run/mount_herring/elev.png \
courses/events/b-herring_run/ski_jump/course.tcl \
courses/events/b-herring_run/ski_jump/preview.png \
courses/events/b-herring_run/ski_jump/trees.png \
courses/events/b-herring_run/ski_jump/terrain.png \
courses/events/b-herring_run/ski_jump/elev.png \
courses/events/b-herring_run/cupicon.png \
courses/events/b-herring_run/keep_it_up/course.tcl \
courses/events/b-herring_run/keep_it_up/preview.png \
courses/events/b-herring_run/keep_it_up/trees.png \
courses/events/b-herring_run/keep_it_up/terrain.png \
courses/events/b-herring_run/keep_it_up/elev.png \
courses/events/b-herring_run/high_road/course.tcl \
courses/events/b-herring_run/high_road/preview.png \
courses/events/b-herring_run/high_road/trees.png \
courses/events/b-herring_run/high_road/terrain.png \
courses/events/b-herring_run/high_road/elev.png \
courses/events/b-herring_run/hamburger_hill/course.tcl \
courses/events/b-herring_run/hamburger_hill/preview.png \
courses/events/b-herring_run/hamburger_hill/trees.png \
courses/events/b-herring_run/hamburger_hill/terrain.png \
courses/events/b-herring_run/hamburger_hill/elev.png \
courses/events/b-herring_run/slalom/course.tcl \
courses/events/b-herring_run/slalom/preview.png \
courses/events/b-herring_run/slalom/trees.png \
courses/events/b-herring_run/slalom/terrain.png \
courses/events/b-herring_run/slalom/elev.png \
courses/events/b-herring_run/mount_satan/course.tcl \
courses/events/b-herring_run/mount_satan/preview.png \
courses/events/b-herring_run/mount_satan/trees.png \
courses/events/b-herring_run/mount_satan/terrain.png \
courses/events/b-herring_run/mount_satan/elev.png \
courses/events/b-herring_run/the_narrow_way/course.tcl \
courses/events/b-herring_run/the_narrow_way/preview.png \
courses/events/b-herring_run/the_narrow_way/trees.png \
courses/events/b-herring_run/the_narrow_way/terrain.png \
courses/events/b-herring_run/the_narrow_way/elev.png \
courses/events/b-herring_run/deadman/course.tcl \
courses/events/b-herring_run/deadman/preview.png \
courses/events/b-herring_run/deadman/trees.png \
courses/events/b-herring_run/deadman/terrain.png \
courses/events/b-herring_run/deadman/elev.png \
courses/events/b-herring_run/bumpy_ride/course.tcl \
courses/events/b-herring_run/bumpy_ride/preview.png \
courses/events/b-herring_run/bumpy_ride/trees.png \
courses/events/b-herring_run/bumpy_ride/terrain.png \
courses/events/b-herring_run/bumpy_ride/elev.png \
courses/events/b-herring_run/event.tcl \
courses/events/b-herring_run/snow_valley/course.tcl \
courses/events/b-herring_run/snow_valley/preview.png \
courses/events/b-herring_run/snow_valley/trees.png \
courses/events/b-herring_run/snow_valley/terrain.png \
courses/events/b-herring_run/snow_valley/elev.png \
courses/events/b-herring_run/tux-toboggan_run/course.tcl \
courses/events/b-herring_run/tux-toboggan_run/preview.png \
courses/events/b-herring_run/tux-toboggan_run/trees.png \
courses/events/b-herring_run/tux-toboggan_run/terrain.png \
courses/events/b-herring_run/tux-toboggan_run/elev.png \
courses/events/b-herring_run/skull_mountain/course.tcl \
courses/events/b-herring_run/skull_mountain/preview.png \
courses/events/b-herring_run/skull_mountain/trees.png \
courses/events/b-herring_run/skull_mountain/terrain.png \
courses/events/b-herring_run/skull_mountain/elev.png \
courses/events/b-herring_run/penguins_cant_fly/course.tcl \
courses/events/b-herring_run/penguins_cant_fly/preview.png \
courses/events/b-herring_run/penguins_cant_fly/trees.png \
courses/events/b-herring_run/penguins_cant_fly/terrain.png \
courses/events/b-herring_run/penguins_cant_fly/elev.png \
courses/events/b-herring_run/ice_labyrinth/course.tcl \
courses/events/b-herring_run/ice_labyrinth/preview.png \
courses/events/b-herring_run/ice_labyrinth/trees.png \
courses/events/b-herring_run/ice_labyrinth/terrain.png \
courses/events/b-herring_run/ice_labyrinth/elev.png \
courses/events/b-herring_run/ive_got_a_woody/course.tcl \
courses/events/b-herring_run/ive_got_a_woody/preview.png \
courses/events/b-herring_run/ive_got_a_woody/trees.png \
courses/events/b-herring_run/ive_got_a_woody/terrain.png \
courses/events/b-herring_run/ive_got_a_woody/elev.png \
courses/events/d-niehoff_experience/herringrunicon.png \
courses/events/d-niehoff_experience/event.tcl \
courses/events/d-niehoff_experience/frozen_lakes/course.tcl \
courses/events/d-niehoff_experience/frozen_lakes/preview.png \
courses/events/d-niehoff_experience/frozen_lakes/trees.png \
courses/events/d-niehoff_experience/frozen_lakes/terrain.png \
courses/events/d-niehoff_experience/frozen_lakes/elev.png \
courses/events/d-niehoff_experience/challenge_one/course.tcl \
courses/events/d-niehoff_experience/challenge_one/preview.png \
courses/events/d-niehoff_experience/challenge_one/trees.png \
courses/events/d-niehoff_experience/challenge_one/terrain.png \
courses/events/d-niehoff_experience/challenge_one/elev.png \
courses/events/d-niehoff_experience/cupicon.png \
courses/events/d-niehoff_experience/secret_valleys/course.tcl \
courses/events/d-niehoff_experience/secret_valleys/preview.png \
courses/events/d-niehoff_experience/secret_valleys/trees.png \
courses/events/d-niehoff_experience/secret_valleys/terrain.png \
courses/events/d-niehoff_experience/secret_valleys/elev.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/course.tcl \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/preview.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/trees.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/terrain.png \
courses/events/d-niehoff_experience/in_search_of_the_holy_grail/elev.png \
courses/events/d-niehoff_experience/explore_mountains/course.tcl \
courses/events/d-niehoff_experience/explore_mountains/preview.png \
courses/events/d-niehoff_experience/explore_mountains/trees.png \
courses/events/d-niehoff_experience/explore_mountains/terrain.png \
courses/events/d-niehoff_experience/explore_mountains/elev.png \
courses/events/d-niehoff_experience/tux_at_home/course.tcl \
courses/events/d-niehoff_experience/tux_at_home/preview.png \
courses/events/d-niehoff_experience/tux_at_home/trees.png \
courses/events/d-niehoff_experience/tux_at_home/terrain.png \
courses/events/d-niehoff_experience/tux_at_home/elev.png \
courses/events/d-niehoff_experience/wild_mountains/course.tcl \
courses/events/d-niehoff_experience/wild_mountains/preview.png \
courses/events/d-niehoff_experience/wild_mountains/trees.png \
courses/events/d-niehoff_experience/wild_mountains/terrain.png \
courses/events/d-niehoff_experience/wild_mountains/elev.png \
courses/events/d-niehoff_experience/chinese_wall/course.tcl \
courses/events/d-niehoff_experience/chinese_wall/preview.png \
courses/events/d-niehoff_experience/chinese_wall/trees.png \
courses/events/d-niehoff_experience/chinese_wall/terrain.png \
courses/events/d-niehoff_experience/chinese_wall/elev.png \
courses/events/a-tux_racer/bumpy_ride/course.tcl \
courses/events/a-tux_racer/bumpy_ride/preview.png \
courses/events/a-tux_racer/bumpy_ride/trees.png \
courses/events/a-tux_racer/bumpy_ride/terrain.png \
courses/events/a-tux_racer/bumpy_ride/elev.png \
courses/events/a-tux_racer/herringrunicon.png \
courses/events/a-tux_racer/event.tcl \
courses/events/a-tux_racer/bunny_hill/course.tcl \
courses/events/a-tux_racer/bunny_hill/preview.png \
courses/events/a-tux_racer/bunny_hill/trees.png \
courses/events/a-tux_racer/bunny_hill/terrain.png \
courses/events/a-tux_racer/bunny_hill/elev.png \
courses/events/a-tux_racer/cupicon.png \
courses/events/a-tux_racer/twisty_slope/course.tcl \
courses/events/a-tux_racer/twisty_slope/preview.png \
courses/events/a-tux_racer/twisty_slope/trees.png \
courses/events/a-tux_racer/twisty_slope/terrain.png \
courses/events/a-tux_racer/twisty_slope/elev.png \
courses/events/a-tux_racer/frozen_river/course.tcl \
courses/events/a-tux_racer/frozen_river/preview.png \
courses/events/a-tux_racer/frozen_river/trees.png \
courses/events/a-tux_racer/frozen_river/terrain.png \
courses/events/a-tux_racer/frozen_river/elev.png \
courses/events/a-tux_racer/path_of_daggers/course.tcl \
courses/events/a-tux_racer/path_of_daggers/preview.png \
courses/events/a-tux_racer/path_of_daggers/trees.png \
courses/events/a-tux_racer/path_of_daggers/terrain.png \
courses/events/a-tux_racer/path_of_daggers/elev.png \
			courses/themes/huds/common/huds.tcl \
				courses/themes/items/herrings/herring_dead.png \
				courses/themes/items/herrings/herring_red.png \
				courses/themes/items/herrings/items.tcl \
				courses/themes/items/herrings/star.png \
				courses/themes/items/herrings/herring_green.png \
				courses/themes/items/flags/flag2.png \
				courses/themes/items/flags/items.tcl \
				courses/themes/items/stuff/items.tcl \
				courses/themes/items/stuff/life.png \
		courses/themes/items/common/finish.png \
		courses/themes/items/common/items.tcl \
		courses/themes/items/common/flag.png \
		courses/themes/items/common/herring_standard.png \
		courses/themes/items/common/start.png \
		courses/themes/conditions/common/nighttop.png \
		courses/themes/conditions/common/sunny_light.tcl \
		courses/themes/conditions/common/eveningtop.png \
		courses/themes/conditions/common/nightback.png \
		courses/themes/conditions/common/cloudyleft.png \
		courses/themes/conditions/common/envmap.png \
		courses/themes/conditions/common/sunnyleft.png \
		courses/themes/conditions/common/sunnybottom.png \
		courses/themes/conditions/common/cloudyfront.png \
		courses/themes/conditions/common/eveningleft.png \
		courses/themes/conditions/common/eveningright.png \
		courses/themes/conditions/common/eveningfront.png \
		courses/themes/conditions/common/evening_light.tcl \
		courses/themes/conditions/common/foggy_light.tcl \
		courses/themes/conditions/common/sunnyfront.png \
		courses/themes/conditions/common/eveningbottom.png \
		courses/themes/conditions/common/cloudytop.png \
		courses/themes/conditions/common/cloudyright.png \
		courses/themes/conditions/common/cloudybottom.png \
		courses/themes/conditions/common/cloudyback.png \
		courses/themes/conditions/common/conditions.tcl \
		courses/themes/conditions/common/sunnyback.png \
		courses/themes/conditions/common/nightfront.png \
		courses/themes/conditions/common/night_light.tcl \
		courses/themes/conditions/common/sunnyright.png \
		courses/themes/conditions/common/eveningback.png \
		courses/themes/conditions/common/nightleft.png \
		courses/themes/conditions/common/nightright.png \
		courses/themes/conditions/common/nightenv.png \
		courses/themes/conditions/common/nightbottom.png \
		courses/themes/conditions/common/sunnytop.png \
		courses/themes/conditions/common/eveningenv.png \
		courses/themes/terrains/add/road.png \
		courses/themes/terrains/add/terrains.tcl \
		courses/themes/terrains/add/speed.png \
		courses/themes/terrains/ice/greenice.png \
		courses/themes/terrains/ice/terrains.tcl \
		courses/themes/terrains/ice/hardice.png \
		courses/themes/terrains/mud/mudstart.png \
		courses/themes/terrains/mud/mudstop.png \
		courses/themes/terrains/mud/mud.png \
		courses/themes/terrains/mud/mudprint.png \
		courses/themes/terrains/mud/terrains.tcl \
		courses/themes/terrains/mud/mudparticles.png \
		courses/themes/terrains/lava/lavastone.png \
		courses/themes/terrains/lava/terrains.tcl \
		courses/themes/terrains/lava/lava.png \
		courses/themes/terrains/sand/sand.png \
		courses/themes/terrains/sand/redsand.png \
		courses/themes/terrains/sand/terrains.tcl \
		courses/themes/terrains/rock/snowyrock.png \
		courses/themes/terrains/rock/terrains.tcl \
		courses/themes/terrains/snow/snowygrass.png \
		courses/themes/terrains/snow/dirtsnow.png \
		courses/themes/terrains/snow/buttprint.png \
		courses/themes/terrains/snow/snowparticles.png \
		courses/themes/terrains/snow/stsnow1.png \
		courses/themes/terrains/snow/stsnow2.png \
		courses/themes/terrains/snow/dsnow.png \
		courses/themes/terrains/snow/dsnow2.png \
		courses/themes/terrains/snow/terrains.tcl \
		courses/themes/terrains/snow/dirtsnowparticles.png \
		courses/themes/terrains/snow/buttstart.png \
		courses/themes/terrains/snow/buttstop.png \
		courses/themes/terrains/common/snow.png \
		courses/themes/terrains/common/buttprint.png \
		courses/themes/terrains/common/snowparticles.png \
		courses/themes/terrains/common/rock.png \
		courses/themes/terrains/common/terrains.tcl \
		courses/themes/terrains/common/ice.png \
		courses/themes/terrains/common/buttstart.png \
		courses/themes/terrains/common/buttstop.png \
		courses/themes/models/stuff/barrier.png \
		courses/themes/models/stuff/barrier.ac \
		courses/themes/models/stuff/models.tcl \
		courses/themes/models/trees/tree_xmas.ac \
		courses/themes/models/trees/tree_xmas.png \
		courses/themes/models/trees/models.tcl \
		courses/themes/models/common/shrub.png \
		courses/themes/models/common/tree.png \
		courses/themes/models/common/tree_barren.ac \
		courses/themes/models/common/shrub.ac \
		courses/themes/models/common/tree_barren.png \
		courses/themes/models/common/tree.ac \
		courses/themes/models/common/models.tcl \
		courses/themes/ppracer.tcl \
		courses/themes/common.tcl \
		courses/course_idx.tcl \
		fonts/PaperCuts20.ttf \
		fonts/PaperCuts_outline.ttf \
		music/race1-jt.it \
		music/wonrace1-jt.it \
		music/credits1-cp.it \
		music/options1-jt.it \
		music/start1-jt.it \
		music/readme \
		textures/menu_top_left.png \
		textures/checkmark.png \
		textures/snowparticles.png \
		textures/gaugespeedmask.png \
		textures/splash.png \
		textures/splash_small.png \
		textures/herringicon.png \
		textures/timeicon.png \
		textures/menu_bottom_left.png \
		textures/menu_top_right.png \
		textures/nopreview.png \
		textures/snow_button.png \
		textures/mirror_button.png \
		textures/mask_outline2.png \
		textures/gaugeenergymask.png \
		textures/wind_button.png \
		textures/mask_outline.png \
		textures/energymask.png \
		textures/gaugeoutline.png \
		textures/menu_bottom_right.png \
		textures/listbox_arrows.png \
		textures/noicon.png \
		textures/speedmask.png \
		textures/conditions_button.png \
		textures/mouse_cursor.png \
		textures/tuxlife.png \
		textures/menu_title.png \
		textures/menu_title_small.png \
		translations/en_GB.tcl \
		translations/it_IT.tcl \
		translations/de_DE.tcl \
		translations/nl_NL.tcl \
		translations/es_ES.tcl \
		translations/pl_PL.tcl \
		translations/eu_ES.tcl \
		translations/fr_FR.tcl \
		translations/fi_FI.tcl \
		translations/sv_SE.tcl \
		translations/languages.tcl \
		translations/nb_NO.tcl \
		translations/nn_NO.tcl \
		translations/pt_PT.tcl \
		translations/ro_RO.tcl \
		translations/ru_RU.tcl \
		translations/sk_SK.tcl \
		tux.tcl \
		tux_snowboard.tcl \
		sounds/tux_hit_tree1.wav \
		sounds/tux_on_snow1.wav \
		sounds/tux_on_rock1.wav \
		sounds/tux_on_ice1.wav \
		sounds/fish_pickup1.wav \
		sounds/fish_pickup2.wav \
		sounds/fish_pickup3.wav \
		terrains.png \
		objects.png \
		etracer_init.tcl \
		tux_walk.tcl \
		models.tcl
all: all-am

.SUFFIXES:
$(srcdir)/Makefile.in: # $(srcdir)/Makefile.am  $(am__configure_deps)
	@for dep in $?; do \
	  case '$(am__configure_deps)' in \
	    *$$dep*) \
	      cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh \
		&& exit 0; \
	      exit 1;; \
	  esac; \
	done; \
	echo ' cd $(top_srcdir) && $(AUTOMAKE) --gnu  data/Makefile'; \
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --gnu  data/Makefile
.PRECIOUS: Makefile
Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@case '$?' in \
	  *config.status*) \
	    cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh;; \
	  *) \
	    echo ' cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe)'; \
	    cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@ $(am__depfiles_maybe);; \
	esac;

$(top_builddir)/config.status: $(top_srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh

$(top_srcdir)/configure: # $(am__configure_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
$(ACLOCAL_M4): # $(am__aclocal_m4_deps)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) am--refresh
install-nobase_dist_ppdataDATA: $(nobase_dist_ppdata_DATA)
	@$(NORMAL_INSTALL)
	test -z "$(ppdatadir)" || $(MKDIR_P) "$(DESTDIR)$(ppdatadir)"
	@$(am__vpath_adj_setup) \
	list='$(nobase_dist_ppdata_DATA)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__vpath_adj) \
	  echo " $(nobase_dist_ppdataDATA_INSTALL) '$$d$$p' '$(DESTDIR)$(ppdatadir)/$$f'"; \
	  $(nobase_dist_ppdataDATA_INSTALL) "$$d$$p" "$(DESTDIR)$(ppdatadir)/$$f"; \
	done

uninstall-nobase_dist_ppdataDATA:
	@$(NORMAL_UNINSTALL)
	@$(am__vpath_adj_setup) \
	list='$(nobase_dist_ppdata_DATA)'; for p in $$list; do \
	  $(am__vpath_adj) \
	  echo " rm -f '$(DESTDIR)$(ppdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(ppdatadir)/$$f"; \
	done
tags: TAGS
TAGS:

ctags: CTAGS
CTAGS:


distdir: $(DISTFILES)
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list='$(DISTFILES)'; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) $(MKDIR_P) `echo "$$dist_files" | \
			   sed '/\//!d;s|^|$(distdir)/|;s,/[^/]*$$,,' | \
			   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -pR $(srcdir)/$$file $(distdir)$$dir || exit 1; \
	    fi; \
	    cp -pR $$d/$$file $(distdir)$$dir || exit 1; \
	  else \
	    test -f $(distdir)/$$file \
	    || cp -p $$d/$$file $(distdir)/$$file \
	    || exit 1; \
	  fi; \
	done
check-am: all-am
check: check-am
all-am: Makefile $(DATA)
installdirs:
	for dir in "$(DESTDIR)$(ppdatadir)"; do \
	  test -z "$$dir" || $(MKDIR_P) "$$dir"; \
	done
install: install-am
install-exec: install-exec-am
install-data: install-data-am
uninstall: uninstall-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	  install_sh_PROGRAM="$(INSTALL_STRIP_PROGRAM)" INSTALL_STRIP_FLAG=-s \
	  `test -z '$(STRIP)' || \
	    echo "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'"` install
mostlyclean-generic:

clean-generic:

distclean-generic:
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-am

clean-am: clean-generic mostlyclean-am

distclean: distclean-am
	-rm -f Makefile
distclean-am: clean-am distclean-generic

dvi: dvi-am

dvi-am:

html: html-am

info: info-am

info-am:

install-data-am: install-nobase_dist_ppdataDATA

install-dvi: install-dvi-am

install-exec-am:

install-html: install-html-am

install-info: install-info-am

install-man:

install-pdf: install-pdf-am

install-ps: install-ps-am

installcheck-am:

maintainer-clean: maintainer-clean-am
	-rm -f Makefile
maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-am

mostlyclean-am: mostlyclean-generic

pdf: pdf-am

pdf-am:

ps: ps-am

ps-am:

uninstall-am: uninstall-nobase_dist_ppdataDATA

.MAKE: install-am install-strip

.PHONY: all all-am check check-am clean clean-generic distclean \
	distclean-generic distdir dvi dvi-am html html-am info info-am \
	install install-am install-data install-data-am install-dvi \
	install-dvi-am install-exec install-exec-am install-html \
	install-html-am install-info install-info-am install-man \
	install-nobase_dist_ppdataDATA install-pdf install-pdf-am \
	install-ps install-ps-am install-strip installcheck \
	installcheck-am installdirs maintainer-clean \
	maintainer-clean-generic mostlyclean mostlyclean-generic pdf \
	pdf-am ps ps-am uninstall uninstall-am \
	uninstall-nobase_dist_ppdataDATA

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src/ppgltk/alg
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

plane.cpp
file




2007-12-31T17:50:27.000000Z
e291ea53709deb116629b3f46b33410a
2007-09-01T16:38:12.025871Z
2
botsnlinux

glhelper.cpp
file




2007-12-31T17:50:27.000000Z
8ff7da381f395a2ffa65eff7191d44a6
2007-09-01T16:38:12.025871Z
2
botsnlinux

vec2d.cpp
file




2007-12-31T17:50:27.000000Z
0d2a54e6985a7df08932c79a57cafb31
2007-09-01T16:38:12.025871Z
2
botsnlinux

vec3d.cpp
file




2007-12-31T17:50:27.000000Z
d2a84f236ac129e3122566eda06ddef8
2007-09-01T16:38:12.025871Z
2
botsnlinux

gaus.h
file




2007-12-31T17:50:27.000000Z
1aac43d34e1a8c1888f4d91a3b21c9ba
2007-09-01T16:38:12.025871Z
2
botsnlinux

matrix.cpp
file




2007-12-31T17:50:27.000000Z
555884138b7b20bdabfb8c6904894b58
2007-09-01T16:38:12.025871Z
2
botsnlinux

plane.h
file




2007-12-31T17:50:27.000000Z
8b66207e3959cc14a9ed75f71c464a25
2007-09-01T16:38:12.025871Z
2
botsnlinux

glhelper.h
file




2007-12-31T17:50:27.000000Z
7afdf27a5a06cb3dc3bad5f641af0102
2007-09-01T16:38:12.025871Z
2
botsnlinux

vec2d.h
file




2007-12-31T17:50:27.000000Z
ac63d9e19b310f7adab6d9f0478a7950
2007-09-01T16:38:12.025871Z
2
botsnlinux

vec3d.h
file




2007-12-31T17:50:27.000000Z
a7e983fb766cdacd67bd4b291aa135e2
2007-09-01T16:38:12.025871Z
2
botsnlinux

matrix.h
file




2007-12-31T17:50:27.000000Z
5e26bfb7b48cef5a5698a28325abfaae
2007-09-01T16:38:12.025871Z
2
botsnlinux

quat.cpp
file




2007-12-31T17:50:27.000000Z
7e0f4d8332ae87dcd0fc47d2be4b3a11
2007-09-01T16:38:12.025871Z
2
botsnlinux

color.cpp
file




2007-12-31T17:50:27.000000Z
c61332ab326653d449d5b4409d7c568d
2007-09-01T16:38:12.025871Z
2
botsnlinux

quat.h
file




2007-12-31T17:50:27.000000Z
e2c10cda6b251e8e47cedb13926e71a8
2007-09-01T16:38:12.025871Z
2
botsnlinux

signal.h
file




2007-12-31T17:50:27.000000Z
cb0fdbf41747878a4a5d14781e03b971
2007-09-01T16:38:12.025871Z
2
botsnlinux

poly.cpp
file




2007-12-31T17:50:27.000000Z
3d73f1b44782f589b942cc29100a68dc
2007-09-01T16:38:12.025871Z
2
botsnlinux

color.h
file




2007-12-31T17:50:27.000000Z
eeae8fd5e9b376779a992f9a3e7a89be
2007-09-01T16:38:12.025871Z
2
botsnlinux

defs.h
file




2007-12-31T17:50:27.000000Z
292316d44af57034cf52c80c6d690ad8
2007-09-01T16:38:12.025871Z
2
botsnlinux

poly.h
file




2007-12-31T17:50:27.000000Z
50f0d68a7ca316f60fc8f99b96bbf35d
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src/ppgltk
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-21T17:46:49.180251Z
13
Torandi


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

frame.h
file




2007-12-31T17:50:28.000000Z
62f02daa13f988e6c5baa457423e8b58
2007-09-01T16:38:12.025871Z
2
botsnlinux

model.cpp
file




2007-12-31T17:50:28.000000Z
0f36518ed6dd6813a1ed9643a9434653
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_theme.h
file




2007-12-31T17:50:28.000000Z
666f7276eaab7a2ea3d64a638ee27e10
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppgltk.h
file




2007-12-31T17:50:28.000000Z
7590260bda8ff8ab577d752fdf607b9e
2007-09-01T16:38:12.025871Z
2
botsnlinux

entry.h
file




2007-12-31T17:50:28.000000Z
868e913b4a4dcfb6e90471110c731dc3
2007-09-01T16:38:12.025871Z
2
botsnlinux

model_ac.h
file




2007-12-31T17:50:28.000000Z
a95f7a9000a8faa7e8617739a95ad89c
2007-09-01T16:38:12.025871Z
2
botsnlinux

font.h
file




2007-12-31T17:50:28.000000Z
5ae3a7d9cd58697fa89ea294a7b6f494
2007-09-01T16:38:12.025871Z
2
botsnlinux

textarea.cpp
file




2007-12-31T17:50:28.000000Z
a5e78ebb075891b18454052b253669aa
2007-09-01T16:38:12.025871Z
2
botsnlinux

audio
dir

button.h
file




2007-12-31T17:50:28.000000Z
189a2ad292f7e3f0b278b5646d120ce4
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_mgr.cpp
file




2007-12-31T17:50:28.000000Z
7f62488be831613f9c1e44f70c812b0d
2007-09-01T16:38:12.025871Z
2
botsnlinux

alignment.h
file




2007-12-31T17:50:28.000000Z
323c2e743c01c0027df5757795296715
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_snow.cpp
file




2007-12-31T17:50:28.000000Z
64d9e94f3efcbb74dcedb7ea2206409b
2007-09-01T16:38:12.025871Z
2
botsnlinux

label.h
file




2007-12-31T17:50:28.000000Z
77c22e4ca3d653cb310360bac087cea3
2007-09-01T16:38:12.025871Z
2
botsnlinux

images
dir

ssbutton.h
file




2007-12-31T17:50:28.000000Z
c90b65396b8f0a1a68ed08d966ceaeb9
2007-09-01T16:38:12.025871Z
2
botsnlinux

widget.h
file




2007-12-31T17:50:28.000000Z
0e5c9780853f219bb80cd421c68b6d7f
2007-09-19T16:04:42.494002Z
8
Torandi

FT
dir

listbox.h
file




2007-12-31T17:50:28.000000Z
e866f0e6af72aca9980832d83ecef2fe
2007-09-01T16:38:12.025871Z
2
botsnlinux

frame.cpp
file




2007-12-31T17:50:28.000000Z
c72f2badda79b40d09f76f2efc01a2d2
2007-09-03T03:13:45.611773Z
5
botsnlinux

checkbox.h
file




2007-12-31T17:50:28.000000Z
19ccbb62ca3cde5ddc473ab7ec1e5623
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_theme.cpp
file




2007-12-31T17:50:28.000000Z
ad6ee0f956d324db8ae643b886595c75
2007-09-21T17:46:49.180251Z
13
Torandi

entry.cpp
file




2007-12-31T17:50:28.000000Z
6289da987eced94602945a473daf5765
2007-09-01T16:38:12.025871Z
2
botsnlinux

model_ac.cpp
file




2007-12-31T17:50:28.000000Z
b30414734eef1267358e01b2df2a8b9b
2007-09-01T16:38:12.025871Z
2
botsnlinux

model.h
file




2007-12-31T17:50:28.000000Z
3d21fe4102473ee4c3685507576db7cd
2007-09-01T16:38:12.025871Z
2
botsnlinux

font.cpp
file




2007-12-31T17:50:28.000000Z
1ff45ffaac7878a798fbedcc85c41140
2007-09-01T16:38:12.025871Z
2
botsnlinux

button.cpp
file




2007-12-31T17:50:28.000000Z
13c50b9337165070e44699320cc84d73
2007-09-01T16:38:12.025871Z
2
botsnlinux

alignment.cpp
file




2007-12-31T17:50:28.000000Z
47393727320eeb35d6c260b56cd3233b
2007-09-01T16:38:12.025871Z
2
botsnlinux

label.cpp
file




2007-12-31T17:50:28.000000Z
d02b93372ec5cb8e7524557a9dfbd64c
2007-09-01T16:38:12.025871Z
2
botsnlinux

ssbutton.cpp
file




2007-12-31T17:50:28.000000Z
656d87dd360236b2bcd8d85b5e4fbbd4
2007-09-01T16:38:12.025871Z
2
botsnlinux

alg
dir

textarea.h
file




2007-12-31T17:50:28.000000Z
4f9d7a4cc558911fda8aa97b0e4b3466
2007-09-01T16:38:12.025871Z
2
botsnlinux

widget.cpp
file




2007-12-31T17:50:28.000000Z
64a227c735d9819a28d54becd3124ca4
2007-09-01T16:38:12.025871Z
2
botsnlinux

listbox.cpp
file




2007-12-31T17:50:28.000000Z
57e3462a5ecd9c5be5a9c0b01f6a15db
2007-09-01T16:38:12.025871Z
2
botsnlinux

checkbox.cpp
file




2007-12-31T17:50:28.000000Z
4ab54a38f5765de9600113a1cb712efb
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_mgr.h
file




2007-12-31T17:50:28.000000Z
adb12e4900dcf7a84bf68ea52f0555d3
2007-09-01T16:38:12.025871Z
2
botsnlinux

ui_snow.h
file




2007-12-31T17:50:28.000000Z
7e9753d18f5426162147cfaf1276ea05
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/src
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-13T23:52:36.812478Z
65
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

translation.cpp
file




2007-12-31T17:50:28.000000Z
46ad8b2374c2e87a62faf32c199b4733
2007-09-03T03:13:45.611773Z
5
botsnlinux

credits.h
file




2007-12-31T17:50:28.000000Z
6ee29387d6bbc037b230bcfdefcc413e
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystick.h
file




2007-12-31T17:50:28.000000Z
714af5dcde494b89db59afa225d9049e
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_mgr.cpp
file




2007-12-31T17:50:28.000000Z
1ce9cc5ed16287f9568367c5b7d12d0a
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_race_select.cpp
file




2007-12-31T17:50:28.000000Z
e1a8e6edf3a14a84b623973b0cc6b4e6
2007-09-19T16:04:42.494002Z
8
Torandi

fog.cpp
file




2007-12-31T17:50:28.000000Z
a1b686f524c28d12ce3c687d8fea12b7
2007-09-01T16:38:12.025871Z
2
botsnlinux

race_select.h
file




2007-12-31T17:50:28.000000Z
e7c45e8d223baf9460eb0b22d504d6fb
2007-09-13T20:23:04.446280Z
6
Torandi

screenshot.h
file




2007-12-31T17:50:28.000000Z
ec6a68da1757b29f6dd3ca4cc0db5673
2007-09-01T16:38:12.025871Z
2
botsnlinux

loading.h
file




2007-12-31T17:50:28.000000Z
d336e5240a4ea8b2cdc3178ffc532df7
2007-09-01T16:38:12.025871Z
2
botsnlinux

bench.h
file




2007-12-31T17:50:28.000000Z
1807e48e3c90ea6561a27486dfe1717c
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_util.cpp
file




2007-12-31T17:50:28.000000Z
7ad8fc290a384809d848b80583cffb9d
2007-09-01T16:38:12.025871Z
2
botsnlinux

paused.h
file




2007-12-31T17:50:28.000000Z
c555fe819fa04859f83fdf83d39b9e3f
2007-09-01T16:38:12.025871Z
2
botsnlinux

textures.h
file




2007-12-31T17:50:28.000000Z
bb01b1edae4b00fc32ecd7d9122eab6b
2007-09-03T03:13:45.611773Z
5
botsnlinux

debug.h
file




2007-12-31T17:50:28.000000Z
fd2d15b3332cf8b082bd07424e0490fd
2007-09-03T03:13:45.611773Z
5
botsnlinux

audioconfig.h
file




2007-12-31T17:50:28.000000Z
d9abf934d93b15295a7a7502d287e66a
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_config.cpp
file




2007-12-31T17:50:28.000000Z
08f991e6e922bef060df0b8024a69d86
2008-01-13T23:52:36.812478Z
65
cpicon92

splash_screen.h
file




2007-12-31T17:50:28.000000Z
5a63e2cf04de3e4e5064dc54954f97f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystick.cpp
file




2007-12-31T17:50:28.000000Z
a9f5f62503888d303901c0c3ef44d7ab
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_cb.h
file




2007-12-31T17:50:28.000000Z
23ae4e88dca2f0cd4c303f6c7c6a8d1e
2007-09-01T16:38:12.025871Z
2
botsnlinux

translation.h
file




2007-12-31T17:50:28.000000Z
f89d1b21334ab2826168a92b3f5d9c44
2007-09-03T03:13:45.611773Z
5
botsnlinux

viewfrustum.h
file




2007-12-31T17:50:28.000000Z
5643cdc0f24e9c4ddd20655c1d59752f
2007-09-01T16:38:12.025871Z
2
botsnlinux

pp_types.h
file




2007-12-31T17:50:28.000000Z
be2ffc5d14defc64a5abf8e3faa87ed4
2007-09-03T03:13:45.611773Z
5
botsnlinux

quadtree.cpp
file




2007-12-31T17:50:28.000000Z
e813b1877f0dff86b02794f4f02f346b
2007-09-01T16:38:12.025871Z
2
botsnlinux

file_util.cpp
file




2007-12-31T17:50:28.000000Z
e5d4f65d1ddd22d98544fca33db312c3
2007-09-03T03:13:45.611773Z
5
botsnlinux

bench.cpp
file




2007-12-31T17:50:28.000000Z
613c1b52140663ee96acfc652c911918
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_race_select.h
file




2007-12-31T17:50:28.000000Z
60aa7641d158a3b01051ca943560190b
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_select.cpp
file




2007-12-31T17:50:28.000000Z
d28a888b9e5eeed7748b3305f43689fe
2007-09-21T22:57:14.407036Z
19
Torandi

nmrcl.h
file




2007-12-31T17:50:28.000000Z
b25c45c14f98d71751585950fb1f40a4
2007-09-01T16:38:12.025871Z
2
botsnlinux

tex_font_metrics.h
file




2007-12-31T17:50:28.000000Z
47d319d3104615df2a57586b15e400ec
2007-09-01T16:38:12.025871Z
2
botsnlinux

textures.cpp
file




2007-12-31T17:50:28.000000Z
c0f3398adb917d3f4e6fc6c47e65b6f9
2007-09-01T16:38:12.025871Z
2
botsnlinux

snow.cpp
file




2007-12-31T17:50:28.000000Z
b163d4202982e464e1dc327030587332
2007-09-01T16:38:12.025871Z
2
botsnlinux

audioconfig.cpp
file




2007-12-31T17:50:28.000000Z
0d63f308d1ea5aaa55f137ac99662b10
2007-09-01T16:38:12.025871Z
2
botsnlinux

mirror_course.cpp
file




2007-12-31T17:50:28.000000Z
c0990646e6cb1a7d6cbb230e2df0102a
2007-09-01T16:38:12.025871Z
2
botsnlinux

joystickconfig.h
file




2007-12-31T17:50:28.000000Z
fd8ad50b7ad9dc5fb72ad2b089c3cbc7
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_cb.cpp
file




2007-12-31T17:50:28.000000Z
cfda7da6221104305a4b251ced45024f
2007-09-01T16:38:12.025871Z
2
botsnlinux

track_marks.cpp
file




2007-12-31T17:50:28.000000Z
986f0cf780750a7b31fba7b343e47bb2
2007-09-19T22:16:13.729253Z
9
Torandi

winsys.h
file




2007-12-31T17:50:28.000000Z
a92c6535d1bf43a1eb4b999bc436bb0a
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_config.h
file




2007-12-31T17:50:28.000000Z
f5eedd174003089ffe09484485b8767f
2007-09-03T03:13:45.611773Z
5
botsnlinux

keyframe.h
file




2007-12-31T17:50:28.000000Z
ef519de490fedd6f0647017a4ffc6741
2007-09-01T16:38:12.025871Z
2
botsnlinux

loop.cpp
file




2007-12-31T17:50:28.000000Z
8a548dfc192252949a8d7ffb445f37ee
2007-09-13T20:23:04.446280Z
6
Torandi

game_over.h
file




2007-12-31T17:50:28.000000Z
9008b29b549f2ded63e70ae64f85cc5a
2007-09-13T20:23:04.446280Z
6
Torandi

nmrcl.cpp
file




2007-12-31T17:50:28.000000Z
9a9d44c2862cf8f6bf8077d82466bd71
2007-09-03T03:13:45.611773Z
5
botsnlinux

quadtree.h
file




2007-12-31T17:50:28.000000Z
2a96744d6f751e3780508d5b1a5589af
2007-09-01T16:38:12.025871Z
2
botsnlinux

render_util.h
file




2007-12-31T17:50:28.000000Z
ff1ffab99ccb32809dbf47cab06f78d1
2007-09-01T16:38:12.025871Z
2
botsnlinux

reset.cpp
file




2007-12-31T17:50:28.000000Z
814e66d1764649e8c703192b7cb50236
2007-09-19T22:16:13.729253Z
9
Torandi

snow.h
file




2007-12-31T17:50:28.000000Z
de12b5e3bec667a310fb6d6a961af7b4
2007-09-01T16:38:12.025871Z
2
botsnlinux

fps.h
file




2007-12-31T17:50:28.000000Z
4a2c8cdbe071872b9521d49f541fded2
2007-09-01T16:38:12.025871Z
2
botsnlinux

mirror_course.h
file




2007-12-31T17:50:28.000000Z
8e0344474d10ef92151b48c9cc1a3890
2007-09-01T16:38:12.025871Z
2
botsnlinux

phys_sim.h
file




2007-12-31T17:50:28.000000Z
b4ca76d36ee16c5a727c1b654a7bd3aa
2007-09-01T16:38:12.025871Z
2
botsnlinux

player.h
file




2007-12-31T17:50:28.000000Z
d4039073405442b897c7d94ab923c4e8
2007-09-19T22:16:13.729253Z
9
Torandi

keyframe.cpp
file




2007-12-31T17:50:28.000000Z
66ba07bba081b22977a0aeb5d24ef121
2007-09-19T22:16:13.729253Z
9
Torandi

model_hndl.cpp
file




2007-12-31T17:50:28.000000Z
6734b47b287febc57a083962578e11d8
2007-09-19T22:16:13.729253Z
9
Torandi

game_type_select.cpp
file




2007-12-31T17:50:28.000000Z
6f5253332f948afc4e0e7ae56cad0843
2007-09-13T20:23:04.446280Z
6
Torandi

configmode.cpp
file




2007-12-31T17:50:28.000000Z
f89a6ea297db9ab874542aa00a135883
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_mgr.h
file




2007-12-31T17:50:28.000000Z
caf56d731e3ee59b96186afd3b5c8f72
2007-09-03T03:13:45.611773Z
5
botsnlinux

configuration.h
file




2007-12-31T17:50:28.000000Z
9ad764dbb642d094fd7a0c079d233c60
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_mgr.cpp
file




2007-12-31T17:50:28.000000Z
f6e212ffe254fb019ccb767ab2538ff3
2007-09-01T16:38:12.025871Z
2
botsnlinux

tcl_util.h
file




2007-12-31T17:50:28.000000Z
4e7b975a3da8eb9a34e9eb25bf55845b
2007-09-03T03:13:45.611773Z
5
botsnlinux

hud.h
file




2007-12-31T17:50:28.000000Z
2dbeb01ebd3e1497c88b8dfdaf1d200b
2007-09-01T16:38:12.025871Z
2
botsnlinux

reset.h
file




2007-12-31T17:50:28.000000Z
7dbedb1b06022ab1b4a10da139b73ac8
2007-09-01T16:38:12.025871Z
2
botsnlinux

Makefile.am
file




2007-12-31T17:50:28.000000Z
3bb051af1c061cf1107d808582093c84
2007-09-22T05:44:23.419072Z
27
botsnlinux

phys_sim.cpp
file




2007-12-31T17:50:28.000000Z
307564fd902902e1bea198ec7cfa1a2f
2007-09-19T22:16:13.729253Z
9
Torandi

stuff.h
file




2007-12-31T17:50:28.000000Z
4a5b84e7842972e8dfa4b8359c43ab62
2007-09-01T16:38:12.025871Z
2
botsnlinux

viewfrustum.cpp
file




2007-12-31T17:50:28.000000Z
5c8f41e128df678702174a5d713d7ff8
2007-09-01T16:38:12.025871Z
2
botsnlinux

intro.h
file




2007-12-31T17:50:28.000000Z
33a801fd9bf3a7352524800d3a2043cf
2007-09-01T16:38:12.025871Z
2
botsnlinux

Makefile.in
file




2007-12-31T17:50:28.000000Z
0f90cfa8ca546e931c73437ac7f9801a
2007-09-22T05:44:23.419072Z
27
botsnlinux

gl_util.h
file




2007-12-31T17:50:28.000000Z
fee95abe084c68e7bb8b2d626663d4d5
2007-09-03T03:13:45.611773Z
5
botsnlinux

course_load.h
file




2007-12-31T17:50:28.000000Z
b1198a305bcb3980859aa8d2a0f27312
2007-09-01T16:38:12.025871Z
2
botsnlinux

configuration.cpp
file




2007-12-31T17:50:28.000000Z
1c4f2f94542a9b0f3bfbb37f93cd2de5
2007-12-30T18:57:52.311507Z
54
cpicon92

string_util.h
file




2007-12-31T17:50:28.000000Z
f239fb41549dc1e24a345a07e67b2483
2007-09-01T16:38:12.025871Z
2
botsnlinux

part_sys.h
file




2007-12-31T17:50:28.000000Z
fc7207198f4773ab130d00f296cee953
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_quad.cpp
file




2007-12-31T17:50:28.000000Z
03447e95d02d2815cd65fdeca4682aca
2007-09-01T16:38:12.025871Z
2
botsnlinux

tex_font_metrics.cpp
file




2007-12-31T17:50:28.000000Z
2d65c6776bd33e42ec62a1d2fd934566
2007-09-03T03:13:45.611773Z
5
botsnlinux

os_util.cpp
file




2007-12-31T17:50:28.000000Z
475c4e604e7aa4493e9b59be59cfe0f8
2007-09-20T15:11:12.728578Z
10
Torandi

configmode.h
file




2007-12-31T17:50:28.000000Z
d4b620e1d0787d4a20dad03e61d4824b
2007-09-01T16:38:12.025871Z
2
botsnlinux

tcl_util.cpp
file




2007-12-31T17:50:28.000000Z
ee8d766ed0be2732d512c8944794bdce
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_mgr.h
file




2007-12-31T17:50:28.000000Z
def5aff8f797044ccdcf685c91f5ad0b
2007-09-01T16:38:12.025871Z
2
botsnlinux

tux_shadow.cpp
file




2007-12-31T17:50:28.000000Z
d3a0dad7304d623bff85d8463756d844
2007-09-19T22:16:13.729253Z
9
Torandi

graphicsconfig.cpp
file




2007-12-31T17:50:28.000000Z
be25bf620b9d92170fa34ad038139ff1
2007-09-19T22:16:13.729253Z
9
Torandi

error_util.cpp
file




2007-12-31T17:50:28.000000Z
fb4b43b44942c28875f69ee7ae2faeee
2007-09-01T16:38:12.025871Z
2
botsnlinux

stuff.cpp
file




2007-12-31T17:50:28.000000Z
94c16378eef80127d73df927a80a68f5
2007-09-01T16:38:12.025871Z
2
botsnlinux

lights.cpp
file




2007-12-31T17:50:28.000000Z
993cb4120939d8dc855e9744f25c82ea
2007-09-01T16:38:12.025871Z
2
botsnlinux

intro.cpp
file




2007-12-31T17:50:28.000000Z
8cb4fa57bf79142bfb660f54955d81d3
2007-09-19T22:16:13.729253Z
9
Torandi

winsys.cpp
file




2007-12-31T17:50:28.000000Z
c7ef8692896a17184505561286d5134a
2007-09-13T20:23:04.446280Z
6
Torandi

credits.cpp
file




2008-01-14T00:15:17.000000Z
1b56c63e614b97f4aa392569c2fafa12
2008-01-13T23:52:36.812478Z
65
cpicon92

gl_util.cpp
file




2007-12-31T17:50:28.000000Z
cc2811dd77963a388d695467db7137f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

course_load.cpp
file




2007-12-31T17:50:28.000000Z
7e9a5dcf7ee1410595d93bc284dd2cb0
2007-09-01T16:38:12.025871Z
2
botsnlinux

videoconfig.h
file




2007-12-31T17:50:28.000000Z
03326eb0c5340fa0e2457af673523c0d
2007-09-01T16:38:12.025871Z
2
botsnlinux

pp_classes.h
file




2007-12-31T17:50:28.000000Z
36e93475686ad654cedb4f52441a0482
2007-09-03T03:13:45.611773Z
5
botsnlinux

string_util.cpp
file




2007-12-31T17:50:28.000000Z
2e1c1574a18268ee1e7875fd0680b912
2007-09-01T16:38:12.025871Z
2
botsnlinux

game_over.cpp
file




2007-12-31T17:50:28.000000Z
8ecdd01a6a9a95d33aa218882a299dd5
2007-09-19T22:16:13.729253Z
9
Torandi

course_render.h
file




2007-12-31T17:50:28.000000Z
6090e9308bc82c0ae0c5a556a65f4ca5
2007-09-01T16:38:12.025871Z
2
botsnlinux

track_marks.h
file




2007-12-31T17:50:28.000000Z
f547b35eb5bc010036b3ea75113b9526
2007-09-01T16:38:12.025871Z
2
botsnlinux

racing.h
file




2007-12-31T17:50:28.000000Z
0e7969c44243958b5a0fe518446c1d7b
2007-09-01T16:38:12.025871Z
2
botsnlinux

race_select.cpp
file




2007-12-31T17:50:28.000000Z
79931cbdb76fcb574b138c9796169f99
2007-09-21T20:25:59.273843Z
16
Torandi

render_util.cpp
file




2007-12-31T17:50:28.000000Z
e1ca351de207f7d58608c6ca2896562c
2007-09-01T16:38:12.025871Z
2
botsnlinux

loading.cpp
file




2007-12-31T17:50:28.000000Z
80ecb84cf633b625c7f08705b8a208f8
2007-09-01T16:38:12.025871Z
2
botsnlinux

screenshot.cpp
file




2007-12-31T17:50:28.000000Z
61231112ea6dce0ccbea86821730a7dc
2007-09-03T03:13:45.611773Z
5
botsnlinux

loop.h
file




2007-12-31T17:50:28.000000Z
6152b7f0add8a052756d31948aba972e
2007-09-13T20:23:04.446280Z
6
Torandi

course_quad.h
file




2007-12-31T17:50:28.000000Z
a6f8fc5e10507729b1b24bd03384b4b8
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppgltk
dir

paused.cpp
file




2007-12-31T17:50:28.000000Z
cd770e9d77197a941d49408cbb094513
2007-09-21T22:02:18.956889Z
18
Torandi

os_util.h
file




2007-12-31T17:50:28.000000Z
846b731ab5be08c9d9654e6420b3e429
2007-09-01T16:38:12.025871Z
2
botsnlinux

fps.cpp
file




2007-12-31T17:50:28.000000Z
23eee62734e3eeed22cc7c77c684e703
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier_util.h
file




2007-12-31T17:50:28.000000Z
fb4cbfed3cfc4ef1f9c4e12ef7b8b020
2007-09-01T16:38:12.025871Z
2
botsnlinux

splash_screen.cpp
file




2007-12-31T17:50:28.000000Z
8343f44222b6b4563c42e555b097f016
2007-09-21T17:46:49.180251Z
13
Torandi

etracer.h
file




2008-01-12T07:49:53.000000Z
5206e5c64007c26bbb0065d6b8739660
2007-12-31T18:15:05.063809Z
60
cpicon92

graphicsconfig.h
file




2007-12-31T17:50:28.000000Z
8a18cae432b4e8ea43ee74543cac42a2
2007-09-19T22:16:13.729253Z
9
Torandi

error_util.h
file




2007-12-31T17:50:28.000000Z
98f6b098c47e646aaf023dfdaf990904
2007-09-01T16:38:12.025871Z
2
botsnlinux

player.cpp
file




2007-12-31T17:50:28.000000Z
9de07ba71b8f5553c9f38b49499f1c6f
2007-09-19T22:16:13.729253Z
9
Torandi

course_render.cpp
file




2007-12-31T17:50:28.000000Z
dd811dc9c038c1af5e2b2a8be94c45dc
2007-09-01T16:38:12.025871Z
2
botsnlinux

lights.h
file




2007-12-31T17:50:28.000000Z
0d43950b5370c384968784523e3ef997
2007-09-01T16:38:12.025871Z
2
botsnlinux

racing.cpp
file




2007-12-31T17:50:28.000000Z
45671d25cf49c97e5965c58cd414ce98
2007-09-21T17:46:49.180251Z
13
Torandi

view.cpp
file




2007-12-31T17:50:28.000000Z
e4f9a2ec206702666303ad6d800065d3
2007-09-19T22:16:13.729253Z
9
Torandi

model_hndl.h
file




2007-12-31T17:50:28.000000Z
71baeff826fa9a4f8cf020a3d6fcdbea
2007-09-19T22:16:13.729253Z
9
Torandi

game_type_select.h
file




2007-12-31T17:50:28.000000Z
d26495c1531579f8d625eeeca106bd0a
2007-09-13T20:23:04.446280Z
6
Torandi

hud.cpp
file




2007-12-31T17:50:28.000000Z
b5063297a8d7001fbc83bac8d163e0a8
2007-09-23T10:12:20.263423Z
32
hamishmorrison

highscore.h
file




2007-12-31T17:50:28.000000Z
737c7588af24647cfbb5c98fb94bb974
2007-09-13T20:23:04.446280Z
6
Torandi

file_util.h
file




2007-12-31T17:50:28.000000Z
5300c7ed17ecafc56f0933509b8554ca
2007-09-01T16:38:12.025871Z
2
botsnlinux

callbacks.h
file




2007-12-31T17:50:28.000000Z
d97c640d06bc74c92389994609760f29
2007-09-03T03:13:45.611773Z
5
botsnlinux

main.cpp
file




2007-12-31T17:50:28.000000Z
c741ab7f63090d5168a4be3effc992c3
2007-12-09T00:04:38.165111Z
42
cpicon92

joystickconfig.cpp
file




2007-12-31T17:50:28.000000Z
36f1d3be9de356f3db7c1a48faccd44e
2007-09-01T16:38:12.025871Z
2
botsnlinux

event_select.h
file




2007-12-31T17:50:28.000000Z
70fe51d92b46095e501d5a7e9902dca5
2007-09-21T18:50:23.813603Z
14
Torandi

keyboardconfig.cpp
file




2007-12-31T17:50:28.000000Z
185908721a14b746d68f31a12a5b485c
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier.h
file




2007-12-31T17:50:28.000000Z
32e1c7044b7ee0009d90ad3fc4a5edb1
2007-09-01T16:38:12.025871Z
2
botsnlinux

part_sys.cpp
file




2007-12-31T17:50:28.000000Z
859de82d09412340071e81991157219d
2007-09-01T16:38:12.025871Z
2
botsnlinux

highscore.cpp
file




2007-12-31T17:50:28.000000Z
b606085c735d0ad5b14afee57e89ba7f
2007-09-20T19:47:02.958075Z
11
Torandi

callbacks.cpp
file




2007-12-31T17:50:28.000000Z
295b3c72e420475b00fd5a3d8c754973
2007-09-01T16:38:12.025871Z
2
botsnlinux

view.h
file




2007-12-31T17:50:28.000000Z
2746a69c4b93e12192b856b0bc00d059
2007-09-01T16:38:12.025871Z
2
botsnlinux

fog.h
file




2007-12-31T17:50:28.000000Z
884794cdc6ebee86cbde9f6393abf0c5
2007-09-03T03:13:45.611773Z
5
botsnlinux

debug.cpp
file




2007-12-31T17:50:28.000000Z
743a1db3dbb77c426b244305da6292c0
2007-09-01T16:38:12.025871Z
2
botsnlinux

hier.cpp
file




2007-12-31T17:50:28.000000Z
2ef3f59253f29fd7ac48c03eea4cc939
2007-09-01T16:38:12.025871Z
2
botsnlinux

tux_shadow.h
file




2007-12-31T17:50:28.000000Z
c3fff18b9e2538c33ba368cbfdd996b2
2007-09-01T16:38:12.025871Z
2
botsnlinux

keyboardconfig.h
file




2007-12-31T17:50:28.000000Z
6ed80a3310d7ec6f11955fe0dd5e470c
2007-09-01T16:38:12.025871Z
2
botsnlinux

videoconfig.cpp
file




2007-12-31T17:50:28.000000Z
f97a275ed78da696565a125354978384
2007-09-20T15:11:12.728578Z
10
Torandi

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/contrib/palettes
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

PPRacer---Full.gpl
file




2007-12-31T17:50:28.000000Z
4f1490f9e0fe7cfb75a2e24d253b824b
2007-09-01T16:38:12.025871Z
2
botsnlinux

PPRacer---Default.gpl
file




2007-12-31T17:50:28.000000Z
19ba0ec6f31551b5921476399eb49c61
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/contrib/script-fu
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

ppracer-save-as-pngs.scm
file




2007-12-31T17:50:28.000000Z
62fc5f3080fa829336d9688a15a1fe7e
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppracer-create-level.scm
file




2007-12-31T17:50:28.000000Z
260dfbb677ffe1616b33b2ed45a82283
2007-09-01T16:38:12.025871Z
2
botsnlinux

ppracer_load_level.scm
file




2007-12-31T17:50:28.000000Z
afb21fc0e51701896d31969fc6bbcd50
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/contrib
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

palettes
dir

script-fu
dir

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/autom4te.cache
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-22T05:44:23.419072Z
27
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

output.1
file




2007-12-31T17:50:29.000000Z
ff10fb521e7f0e74e0b25c3eb8f57d85
2007-09-03T03:13:45.611773Z
5
botsnlinux

traces.0
file




2007-12-31T17:50:29.000000Z
0906fef257507367aa7686a8528cffd0
2007-09-03T03:13:45.611773Z
5
botsnlinux

output.2
file




2007-12-31T17:50:29.000000Z
c5db3d13c4178b6e6d963f2840de4c94
2007-09-22T05:12:27.337331Z
24
botsnlinux

traces.1
file




2007-12-31T17:50:29.000000Z
32c161f856f89faae1a3189bcf514233
2007-09-03T03:13:45.611773Z
5
botsnlinux

output.3
file




2007-12-31T17:50:29.000000Z
c5db3d13c4178b6e6d963f2840de4c94
2007-09-22T05:12:27.337331Z
24
botsnlinux

traces.2
file




2007-12-31T17:50:29.000000Z
b457b9e9f7e61a5c52ed6be369e78ee2
2007-09-03T03:13:45.611773Z
5
botsnlinux

traces.3
file




2007-12-31T17:50:29.000000Z
19654b0ce04ecff0453789ed48a4013d
2007-09-03T03:13:45.611773Z
5
botsnlinux

requests
file




2007-12-31T17:50:29.000000Z
92b84a73d0519221533cde561d69a52b
2007-09-22T05:44:23.419072Z
27
botsnlinux

output.0
file




2007-12-31T17:50:29.000000Z
ff10fb521e7f0e74e0b25c3eb8f57d85
2007-09-03T03:13:45.611773Z
5
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/doc
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

Doxyfile
file




2007-12-31T17:50:29.000000Z
2154283dad31cf522b2bbfd6e8f82260
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/music
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-12T07:44:01.776333Z
63
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

race1-jt.it
file




2007-12-31T17:50:29.000000Z
9e824730d3c3cddc91e09be45e9d6049
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

options1-jt.it
file




2007-12-31T17:50:29.000000Z
aba0e960d44dde08cac22c50b71f3a82
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

start1-jt.it
file




2008-01-12T07:49:54.000000Z
0657496fa85a4f5d083a7a9972b5dc9c
2008-01-12T07:44:01.776333Z
63
cpicon92
has-props

wonrace1-jt.it
file




2007-12-31T17:50:29.000000Z
54ffa4fda7a8a6fedb05c09d1e39fc03
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

credits1-cp.it
file




2008-01-12T07:49:54.000000Z
6eb1c277e8640d2fbc53aef8a51f968c
2008-01-12T07:44:01.776333Z
63
cpicon92
has-props

readme
file




2008-01-12T07:49:54.000000Z
4969d267f31973fb4a130be953724ba9
2008-01-12T07:44:01.776333Z
63
cpicon92

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/translations
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2008-01-13T23:52:36.812478Z
65
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

fr_FR.tcl
file




2007-12-31T17:50:29.000000Z
8396b6f0255239e208f32d30c2c32d6b
2007-09-21T17:46:49.180251Z
13
Torandi

es_ES.tcl
file




2007-12-31T17:50:29.000000Z
1b790e88fb409015bfa9c5014b615fa9
2007-09-01T16:38:12.025871Z
2
botsnlinux

eu_ES.tcl
file




2007-12-31T17:50:29.000000Z
d09709b1815f850a4b05cc70c3a9e9f4
2007-09-01T16:38:12.025871Z
2
botsnlinux

de_DE.tcl
file




2007-12-31T17:50:29.000000Z
f0783346f027d7a7d758c6cbe27cb996
2007-12-15T18:12:19.951232Z
47
cpicon92

nl_NL.tcl
file




2007-12-31T17:50:29.000000Z
ca9b71c04b8143a4cf1bb804c5eb59ce
2007-09-19T22:16:13.729253Z
9
Torandi

pl_PL.tcl
file




2007-12-31T17:50:29.000000Z
bd0210e4ce456c5c5dbf9f2943486ccc
2007-12-30T15:40:33.102064Z
53
cpicon92

languages.tcl
file




2008-01-12T07:49:55.000000Z
aa454f044dee6a58d5f120af9ae74da7
2008-01-12T07:44:01.776333Z
63
cpicon92

nn_NO.tcl
file




2007-12-31T17:50:29.000000Z
cbf3fb591f5458648e6f6d3e8762c88a
2007-09-01T16:38:12.025871Z
2
botsnlinux

it_IT.tcl
file




2007-12-31T17:50:29.000000Z
9103de053f1c5496a4af262063c3ddfb
2007-09-01T16:38:12.025871Z
2
botsnlinux

sk_SK.tcl
file




2008-01-12T07:49:55.000000Z
69ef5e03f6803fb0d7b3999f27be4e22
2008-01-12T07:44:01.776333Z
63
cpicon92

en_GB.tcl
file




2007-12-31T17:50:29.000000Z
34179db61019880714b1309ab558f51c
2007-09-21T19:40:07.008509Z
15
Torandi

fi_FI.tcl
file




2007-12-31T17:50:29.000000Z
5948cb736b39db98efc7cb335b73514b
2007-09-19T22:16:13.729253Z
9
Torandi

sv_SE.tcl
file




2008-01-14T00:15:17.000000Z
58f2240e00d29deb4b6d3f49cb4a37c7
2008-01-13T23:52:36.812478Z
65
cpicon92

ro_RO.tcl
file




2008-01-12T07:49:55.000000Z
c7cb5d65ef1886a6a0a4b40d16b66766
2008-01-03T03:22:14.430488Z
61
cpicon92

pt_PT.tcl
file




2007-12-31T17:50:29.000000Z
72daa78fb818ee464795a0436d148adc
2007-09-01T16:38:12.025871Z
2
botsnlinux

nb_NO.tcl
file




2007-12-31T17:50:29.000000Z
b015b3699685d37c27b6fdb5f0a11cfa
2007-09-01T16:38:12.025871Z
2
botsnlinux

ru_RU.tcl
file




2007-12-31T17:50:29.000000Z
792c410d29398aa7086e1e3a6090e529
2007-09-01T16:38:12.025871Z
2
botsnlinux

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/contrib/doing
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:31.000000Z
557eb44437346fc9f8de09ca40968a4c
2007-09-01T16:38:12.025871Z
2
botsnlinux

preview.png
file




2007-12-31T17:50:31.000000Z
219c555dc6e728c9efed032a8bf18acd
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

trees.png
file




2007-12-31T17:50:31.000000Z
dba4e1c2e8a293518adec0de5d072bc0
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

terrain.png
file




2007-12-31T17:50:31.000000Z
835d4a1f5bbf8eece89b4377e231f79c
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

elev.png
file




2007-12-31T17:50:31.000000Z
0784f370d6ab3795512621392d285b88
2007-09-01T16:38:12.025871Z
2
botsnlinux
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/contrib
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-09-01T16:38:12.025871Z
2
botsnlinux


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

doing
dir

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/hippo_run
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:31.000000Z
562d442bd8fec49e014470a7508d361a
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:31.000000Z
746cb3f9bfe3319fb879377513c6b4f9
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:31.000000Z
e6eb06dde3b45af07b2491de3a2a644e
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:31.000000Z
4eac67582b302c70f0c19f2796c6ba64
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:31.000000Z
092b0724533fb97da498896bc98aa794
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/in_search_of_vodka
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:32.000000Z
51947e7b7b7a224c5f1beab0b359751a
2007-12-31T04:47:08.831977Z
57
cpicon92

elev.rgb
file




2007-12-31T17:50:32.000000Z
ce832932a1caaa3ff9050f26951f0528
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

preview.rgb
file




2007-12-31T17:50:32.000000Z
75fc375dfcc558c550addfc17485eed0
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.rgb
file




2007-12-31T17:50:32.000000Z
264484cda6806517b5d133cf626f22e6
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.rgb
file




2007-12-31T17:50:32.000000Z
37cd22ccef39b1ca1777703b44db9281
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/crazy_path
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:32.000000Z
c9ba9602f3117854bc5d2722ae535c7a
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:32.000000Z
924857e74baa7faa1c48a3485b3e5c64
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:32.000000Z
8db84a9975d8545150f513e1b30995cd
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:32.000000Z
9c1f262570902a6fbbd54c50d43f06fc
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:32.000000Z
fe926630992ae02631a6c96273e65532
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/hey_tux
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:32.000000Z
e4a4dce1c9effe372f583ff578ca0425
2007-12-31T04:47:08.831977Z
57
cpicon92

elev.rgb
file




2007-12-31T17:50:32.000000Z
e8982c929b6d9db88767e30a9ae10ada
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

preview.rgb
file




2007-12-31T17:50:32.000000Z
23627ca95f34458afe76c9dc3cc287d7
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.rgb
file




2007-12-31T17:50:32.000000Z
5fd8bcb842f844c1ae58c05015ae446f
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.rgb
file




2007-12-31T17:50:32.000000Z
06843522ef8e32834527ee3bf288fcd5
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/ice_pipeline
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:32.000000Z
1371d5489ffa4299a2e76f3387f21cc4
2007-12-31T04:47:08.831977Z
57
cpicon92

elev.rgb
file




2007-12-31T17:50:32.000000Z
2a056709281f2dbc083453f226a4c7d1
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

preview.png
file




2007-12-31T17:50:32.000000Z
6e6023b6a5d571f3ed8221a67a77ea15
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.rgb
file




2007-12-31T17:50:32.000000Z
d70f01ab120643461c731c758a4be1df
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.rgb
file




2007-12-31T17:50:32.000000Z
51a0374cd6809c48b9e95a7e8ffdd4f2
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/slippy_slidey
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:33.000000Z
212d44c4cce51b91fef76d07c5894f5c
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:33.000000Z
04f0199fcf7a4cecbc9e8243d41f8b1a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:33.000000Z
1ea1ca156ee6b8c510fdacdc2a08c4a2
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:33.000000Z
bdec87cd161b9b3e043555a885d183ae
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:33.000000Z
bd2e6c74ca86bcd38eb15bd9df3317b5
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/volcanoes
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:34.000000Z
36be67ba00ba041cdbf255582a85833c
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:34.000000Z
d10e3f8301ef8076ecbda986dbdf00c5
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:34.000000Z
fa661e52e721d11dae0edb0108adff3c
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:34.000000Z
2c8dad16b9c87695b4caf918761821e1
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:34.000000Z
56cef33e5aed8fd07520b0a2f6cccf8f
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/merry_go_round
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:34.000000Z
de2ac0bca0d948fe63cea21724081532
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:34.000000Z
fc6caaad4503e591c69bc0ce6e6f40ef
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:34.000000Z
bde0d6394d1b231e28895d65ad4a9a91
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:34.000000Z
fa20ab8b4f429e7fc1b3b5dac5b66a86
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:34.000000Z
818fff925e8df3510eac4b895cf0223a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/bobsled_ride
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:34.000000Z
febd2910654d7d68bf18827b037ba574
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:34.000000Z
d98682efd2cb109cb48d804e42f5db63
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:34.000000Z
d302813a4bd77f4200e106f870fcc51b
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:34.000000Z
3d2d66d42148c478024bc67c72e9592e
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:34.000000Z
c4269e11faeae33bd13823885ce0d187
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/candy_lane
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
fec0eef042f937e1f9bd0e60de8a357e
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
89eca2df748e6124988202d946f9c773
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
d938945a842552fc0737186505a60da5
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
29f916770f041771e3da07b03b6b054c
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
24a18c2f1b11768c7b25e1e8ae3c4659
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania/nature_stroll
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
8a06c814ec7a72b10d8edecbc738e285
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
042779a7aae73e0f04a775d8665d8ef3
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
b579a3bf0def785dd238053afbe06a3a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
0db2b8189d5347460ca56aaa305915dd
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
b9653badf8b31874a7fc7ebf64713183
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/c-mountain_mania
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

herringrunicon.png
file




2007-12-31T17:50:35.000000Z
07040f5d51331dbe8112ec6be7d40d9a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

event.tcl
file




2007-12-31T17:50:35.000000Z
2725e6ddb11e4bd3b628ef9c3b3d3c5b
2007-12-31T04:47:08.831977Z
57
cpicon92

hippo_run
dir

in_search_of_vodka
dir

cupicon.png
file




2007-12-31T17:50:35.000000Z
bb890130868b4573a09f34528d9c4584
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

crazy_path
dir

hey_tux
dir

ice_pipeline
dir

slippy_slidey
dir

volcanoes
dir

merry_go_round
dir

bobsled_ride
dir

candy_lane
dir

nature_stroll
dir

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/b-herring_run/hazzard_valley
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
b430948a52b381c25f76cb85735bc450
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
867fb28d7518e20c92f74ae4801fedb7
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
ae7f99e03039336fd7511bed85c12a6a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
316c10f04bbf9098ba140644f8780182
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
668a3ff7f9c392b6c45fe07f66146453
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/b-herring_run/mount_herring
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
d3ae5c7dcd318af8d5b28a70a62b7ba6
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
0feaa70b8ed00f1cc40db12d64a611ac
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
84ababefc8c3f2a4a27a17d14baca46a
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
12baf3061d06ed18c2b5330955f3ce22
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
4e64d740ac6b828e753e16ea0ac5c6eb
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/b-herring_run/ski_jump
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
2338026e8377fc369a616791d01e118b
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
a13086bd4303e1177cda188d0ca8ec62
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
3d3247ff80d6b736300eb3f4ab8895c3
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
0998db63c44b98ec55ce7efef4b61170
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

README
file




2007-12-31T17:50:35.000000Z
afdc00e7f3d7266967b68891c9b214dd
2007-12-31T04:47:08.831977Z
57
cpicon92

elev.png
file




2007-12-31T17:50:35.000000Z
192d4c090d88b003ee6ade6bc7c85d4c
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/b-herring_run/hamburger_hill
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
f206d3afca932a4ee73b0a17860fd472
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
928f007f29963182dcdbf2a66eb01f73
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
332b468ef1d1d49b22c717930cbdfd2c
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
35281ca9b979a997b5fbb25872b19ef6
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
5c60b8e147b1ba6ad2b163514fa9268b
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                8

dir
65
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer/data/courses/events/b-herring_run/high_road
https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer



2007-12-31T04:47:08.831977Z
57
cpicon92


svn:special svn:externals svn:needs-lock











14c2f07d-8935-0410-8134-90508e68d282

course.tcl
file




2007-12-31T17:50:35.000000Z
ab2ef3352c9e50056efc942eb55a8ec4
2007-12-31T04:47:08.831977Z
57
cpicon92

preview.png
file




2007-12-31T17:50:35.000000Z
9a4b727a66ff4d9a554b0e9351798ae4
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

trees.png
file




2007-12-31T17:50:35.000000Z
e21faaeafc8db3a25f7e22511e564803
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

terrain.png
file




2007-12-31T17:50:35.000000Z
49cd1c873ec33377df10005027f5cdc1
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

elev.png
file




2007-12-31T17:50:35.000000Z
36ee54ec64a3f5a4e7a66fa51f256b69
2007-12-31T04:47:08.831977Z
57
cpicon92
has-props

                     