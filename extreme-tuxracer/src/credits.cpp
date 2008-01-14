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

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         <S:update-report send-all="true" xmlns:S="svn:"><S:src-path>https://extremetuxracer.svn.sourceforge.net/svnroot/extremetuxracer/trunk/extreme-tuxracer</S:src-path><S:target-revision>65</S:target-revision><S:entry rev="63" ></S:entry></S:update-report>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     �S[]� �@�	�t�@ÐU����SVW3҉U���3�Uhv�@ d�0d� �E�P�]��E��U�3ɸ�  �h����M���Ȣ@ �A���輄��3�ZYYd�h}�@ �E�苇���������_^[��]�SVWU��(I �pN��|"F3ۋӡ(I �.  ���Ջ��~����u(CNu�ա,I ��QH�؅�|�ӡ,I ��Q���3���]_^[�SV���������؅�u��������^[Í@ U�������SVW3ɉM��U�3�Uh��@ d�2d�"�q}���E��`�E����������tG�K��|?C�E�    �E��|����������>}���������E�谇���E��U�衼����t%�E�KuɋE��O}���E�E�=ܤ@ u��E��B�����3�ZYYd�h��@ �E��G����饂������_^[��]Ë�U�������SVW3҉U��ؾ(I 3�Uh��@ d�0d� �p���������|���������E������E��m�����t/�E�������ƅ����������Pj ��  ���@ �w����҂���Ӌ��  ��ܤ@ t���z|���؋Ӌ�  @t�3�ZYYd�h��@ �E��{�����ف����_^[��]�U��SV�ʋ�ISy��ċ��|F�؋������Nu�u��]���]�                        ��@    �@ ,,@ �*@ +@ L+@ 	TIntConst��U���t����~���H�M�H�M�H��t
d�    ��]� SVW�����VW�˲���@ �����С0I ��  _^[Í@ U��QSV�ڋ�j
���Ň��P�I P������Ѕ�����tWR����Vj
�I ��P�@ �/!  �E�3�Uhد@ d�0d� ��E��l  �3�ZYYd�h߯@ �E��{���釀����^[Y]Ë��dI �hI �@  ��\�@ �,{���dI Ë�SV�dI �XK��|C3��֡dI �  ��RFKu�^[Ë��dI �{���hI �D  �dI �hI �PJ�hI �  Í@ U�������SVW3҉U���3�Uh�@ d�0d� 3ہ�ت@ tF�E;p�t>�EP���6z������Y�؍���������y���������E��W����E��U�������
�3�ZYYd�h��@ �E�������p������_^[��]ÐU����U��E������3�UhI�@ d�0d� U�E��ty���;���Y�E������3�ZYYd�hP�@ �����������E���]ÐVW�Ɖ׉�1������u���u@_^Ë�SVW��؋��N  �����E  ;�u���:  �ȋV�C������u3���_^[Í@ U��QS��h   �Ȳ�h�@ �z   �E�3�Uh�@ d�0d� �ӋE���>  3�ZYYd�h
�@ �E��Vy����\~����[Y]Ë�U����SVW����E� ���tx�������kx��;���   ���p  �����g  ;���   ����@ ��x���E�3�Uh�@ d�0d� �ӋE��@�������@ �x���E�3�Uhò@ d�0d� �֋E������U�E�������E�3�ZYYd�hʲ@ �E��x����}����3�ZYYd�h�@ �E��yx����}�����E�_^[��]Í@ S�؋˲�P�@ �G����~��[ø�  �����ÐSV�ڋ����;   ��t���<z����^[Í@ SVW���؋s;su�Ë�R�C�<��C��_^[Í@ S��3ҋ��  3ҋ���  [ÐSV��؅�|;s|�Ë��K�C;�}+Ƌ����C���C�D��s��^[Ð�S���Ë�S�؋C;Cu�Ë�R��[Ë�SV��؅�|;s|�Ë��C��^[Ë��H��~�   ���~�   ��   ��5  �S3��A;H}�X;�u�;Hu�����[ÐSVW����؅�|;s~�Ë��C;Cu�Ë�R�C;�}+Ƌ����C�T��C���r���C�<��C_^[ËPJ�;���Ë�SVWU�����;�t.��|;s|�Ë��׋�������׋������͋֋��d���]_^[Í@ SVW����؅�|;s|�Ë��C�<�_^[Ë�SV�؋����������t	�֋��Y�����^[�SV���^K�� |�Ӌ�������u	�Ӌ��1���K���u�^[�SV���;s|�����~�Ë�;st�����C��p���s^[Í@ SV��؅�|�����~�Ë�;s~	�֋������C;�~�S����+ȋ���3ɒ�ls���s^[Ë�SV�ڋ�3ҋ��=   3ҋ��du����t���yw����^[ù��  ����@ �C����{��Ð��;�}�ʋ�ÐSVWUQ���;s��   ��}�������F H��y���������C�� H��y��������;�tO3��$��t����o���$�$3ɋ��r����t&�<$ t�׋��x����ȋ$�C�p���ՋC�o���$�C�sZ]_^[Ë�;Ps�@�t���Ã� ����PRQB�6���YZX���SVWQ�؋C�� H��y����H��|M@�$3��C�`�@ ;�t3�C�<�3��Ѐ�w����r������ЋS;�|���@< u�F�$u��CZ_^[�   ������t����3��   Ë�U�������SVW3ɉM����3�Uh.�@ d�0d� ��t���������r���������E��f}����E��D�@ ��|���E�������ƅ������������r��������������ƅ����������Pj� �  ��4g@ �����y��3�ZYYd�h5�@ �E���{����1x����_^[��]�����   nil ��*���ÐÍ@ SV�ڋ��F   �~ t����   �F��r����t����t����^[Ë��Ȳ�@�PÐU��QSVW��E��ƺL�@ ��r����th�E��p   3�Uh�@ d�0d� �E��n   ����   ��K��|C3��׋���   P�E�����Z��QGKu�3�ZYYd�h"�@ �E��   ��Nw�����֋E��B���_^[Y]��@Ãx u3ҋ�QË�U��QS�E��E��@�x ~M�E������3�Uh��@ d�0d� ����!�����q���E��X�{ �3�ZYYd�h��@ �E��   ���v����[Y]Í@ �H����Í@ �@�@Ð�@����Í@ Í@ SVW���؋Ë�R���ϋ֋Ë�SL��_^[Í@ SVW���؋Ë�Q,���ϋ֋Ë�S ��_^[Í@ ��Q,Ë�U����SVW3ɉM�U��E�3�Uh��@ d�0d� �E���   3�Uh��@ d�0d� �E���R��N��|-F3ۍM�ӋE��8�W�E�P�ӋE���Q�ȋE�Z�8�W0CNu�3�ZYYd�h��@ �E��W  ��u����3�ZYYd�hƺ@ �E��By����u����_^[��]Í@ U��QS�ډE��ú��@ �p����tE�E��N   3�Uh(�@ d�0d� �E���R8�ӋE���Q43�ZYYd�h9�@ �E���   ��7u�����ӋE��+���[Y]Í@ S�؃{ u	��Ë�Q(�C[ÐU��SV�E�@��p��t,��E�ƺ��@ �%p����t'�E�֋E�@��}   �؀���E�@���R���Ë�^[]�U����S�U��E��E�Ph��@ �E�Ph�@ U����Y�Ⱥ�@ �E���[YY]�   ����   Strings �H�x u3ҋ�Q(Í@ U����SVW3ɉM��M�U��E�3�Uh��@ d�0d� �E� �E���R�؋E���R;�u;��N��|0F3ۍM��ӋE��8�W�E�P�M�ӋE��8�W�U�X�z��uCNu��E�3�ZYYd�h��@ �E�   �w����s����E�_^[��]Ë�U����SVW3ۉ]��]�M����3�UhU�@ d�0d� �M�֋Ë8�W�֋Ë�Q�E��M��U��Ë8�W�M��֋Ë8�W�U��Ë�Q�ȋ֋Ë0�V �M�U��Ë0�V�M��U��Ë�S 3�ZYYd�h\�@ �E�   ��v����
s����_^[��]ÐU������P���SVW3ɉ������U��E�3�Uh��@ d�0d� �E���R����u+������3ҋE���S������ u�E��Ⱦ@ ��v����   ������N����   F�u��E�    �}� t�,C�������U��E��0�V�������.y���E�3�3��}��"��u��� v
��"t��,u����?"uFFG�? u��������   ;�s6��t�"C�}����"u�"C��CG���u��t�"C�E��M��_�����������+ȋЋE��Gv��3�ZYYd�h��@ �������Ou����q����_^[��]�����   ""  3�ÐU��j SVW��3�Uh�@ d�0d� �U��Ë�Q�E��>x��詵����3�ZYYd�h�@ �E���t����Hq������_^[Y]Ë�U����SVW3ɉM�M�U��E�3�Uh��@ d�0d� �E���R�E�3ۋu�N��|)F�E�    �M�U�E��8�W�E���u������E�NuߋE���3��Iu���]���u�N��|@F�E�    �M�U�E��8�W�E��u������t�ӋE���dg����C�
C�E�Nu�3�ZYYd�h�@ �E�   �"t����`p����_^[��]Í@ U����SVW3ɉM�U��E�3�Uh}�@ d�0d� �E���R��K��|#C3��M�֋E��8�W�E�U�謪����tFKu����3�ZYYd�h��@ �E��s�����o������_^[��]Í@ SVWU����ǋ�R��K��|C3��֋ǋ�Q;�tFKu�����]_^[�U��QSVW�M��ڋ��M��ӋƋ8�WL�M�ӋƋ�S _^[Y]� ��U��QS��j �ʸ\�@ ���  �E�3�Uh<�@ d�0d� �U��Ë�QT3�ZYYd�hC�@ �E��j����#o����[Y]ÐU����SVW3ɉM��ډE�3�Uh��@ d�0d� �E������3�Uh��@ d�0d� ���N
  �����)
  +��E���3��Cs���U��΋Ë��U��E���Q$3�ZYYd�h��@ �E��0�����n����3�ZYYd�h��@ �E��r����yn����_^[YY]�U����SVW3ۉ]��M����3�Uh`�@ d�0d� ;u�t.�M��֋Ë8�W�֋Ë�Q���֋Ë�Q<W�M��U����z���3�ZYYd�hg�@ �E��q�����m����_^[YY]Ë�SVWU���؋֋Ë�Q���֋Ë�Q<W�͋֋��*���]_^[ÐÍ@ U��j SVW�ڋ�3�Uh
�@ d�0d� ���  �Ƌ�R8��U����X(  �U��Ƌ�Q,���  ��t����  3�ZYYd�h�@ �E���p����Um����_^[Y]ÐU��QS��h��  �ʸ\�@ ��
  �E�3�Uhc�@ d�0d� �U��Ë�Q`3�ZYYd�hj�@ �E���g�����l����[Y]Ë�U��j SVW���3�Uh��@ d�0d� �Ë�RD�ЍE��q���E���q���ȋU����  3�ZYYd�h��@ �E��<p����l����_^[Y]Ë�U������P���SVW3ɉM��ډE�3�Uh��@ d�0d� �E��<���3�Uh��@ d�6d�&�E���R8���s���Ѝ�������  �ٮ���������C���t< v��; tp�Ӏ;"u��C���"uC�;"u��@C���u��C�< v<,u��Ë�+ʍE��Wp���U��E���Q,�C���t< v����t<,u�C�C���t�< v��3�ZYYd�h��@ �E��(�����k����3�ZYYd�h��@ �E��o����qk����_^[��]�U��j SVW���3�Uh@�@ d�0d� �E����p���U��Ë�Q$3�ZYYd�hG�@ �E���n����k����_^[Y]Í@ U����SVW3ɉM��ډE�3�Uh�@ d�0d� �E������3�Uh��@ d�2d�"�E���R8��t@�9���C���t
��
t��u��+ȍU���)o���U��E���Q,�;uC�;
uC�; u�3�ZYYd�h��@ �E�������vj����3�ZYYd�h�@ �E���m����Yj����_^[YY]�Í@ U����SVW3ɉM�U��E�3�Uh��@ d�0d� �E��R/  �E���R��K��|C3��M�֋E��8�W�U�E���:  FKu�E��$/  3�ZYYd�h��@ �E��sm�����i����_^[��]�SV�ڋ�3��F�F3��F �F$�F��t�V�ȸ|�@ ��os��3��F3ҋ��m  ��t���f����^[ÐSVWQ��؀{ u�C�$�&�̋֋Ë8�Wp��t�C,r��t�
��  ������΋$���x  �$Z_^[Ãx uf�x t
�ȋЋA�QÃx uf�x" t
�ȋЋA$�Q �S�؃{ t,�Ë�Rl�C�K�|�@ �r��3��C3ҋ��  �Ë�Rh[ÐSV��؅�|;s|�X����Ë�Rl�C��|�@ ��r���K�C;�}+Ƌ����C���C�D��J_���Ë�Rh^[�SVW����؅�|;s}	��|;{|������Ë�Rl�ϋ֋��   �Ë�Rh_^[Í@ S�X�Ӌ@�ȋ
����J�X�Z�H[ÐSVWU���L$�$���D$ 3��}O;�|4�7��E�؋$�z�����}�s���O��u�D$�}t��;�}̋D$�0�D$��]_^[Í@ SVW����؅�|;s|�1����ǋS���k��_^[Ë@�SV��؅�|;s|�����C�D�^[Ë��H��~�   ���~�   ��   ��  �SVQ��؀{ u�֋�������$��̋֋Ë�Sp��u�$�����$Z^[�SVW����؀{ t
��  �k�����|;s~�u����ϋ֋��   _^[Ë�SVW����؋Ë�Rl�C;Cu���I����C;�}+Ƌ����C�T��C���Q]���C����3��3��F�Ƌ��Oj���C�Ë�Rh_^[ÐSVW����؀{ t
��  �������|;s|������Ë�Rl�C�����j���Ë�Rh_^[Ë�SVW����؅�|;s|�����Ë�Rl�C�|��Ë�Rh_^[�U����SVW3ۉ]�M��U���3�Uh6�@ d�0d� �]��u��E�U�U���O����i���C�G�؋U��/�����|��N�G���U��������;�|�΋Ӌ��7���CN;�}�;u�~�΋U����p����]�;]�|�3�ZYYd�h=�@ �E���h����)e����_^[��]�SV��؋����C�![���s^[Í@ SV�ڋ�:^t��t�Ƌ�Rt�^^[Ä�t��RlË�Rh�S�؀{ u!�{~�Ë�Rl�KI3ҋ�������Ë�Rh[�Sf� 3ҋ�S[Ë�S3ɋ�S[Ë�SVW��f� 3ҋË0�V��f� 3ҋË8�W��3ɋ֋Ë�S��_^[�SVW�ً�����t$�׋ˋƋ0�;�t��  ��H�@ ������d��_^[Í@ SVW�ً�����t%�׋ˋƋ0�V;�t��  ����@ �߿���d��_^[Ë�U����SVW�ىU��E���u3ҋE��1����E��5����؉]�� �  ~	�E� �  ��]��E��Y���E�3�Uh�@ d�0d� ��t-;]�~�u����}�׋΋E������׋΋E��D���+ޅ�u�3�ZYYd�h"�@ �U��E��FY����Dc����E�_^[��]�U����S��h   �Ȳ� �@ �  �E�3�Uh|�@ d�0d� �ӋE��  �E�3�ZYYd�h��@ �E���]�����b�����E�[YY]Ð��t����_���H��t
d�    ��Ë��@��������u3�Ë@�ܠ�����u3���ɋ@������U����SVW��t����C_����U��؋}��=��  u5���H������{��}[�u��E��E�Pj ��  ����@ 荾����b���6�׋��П���C�{ }#�u��E��E�Pj ��  ��ȡ@ �U����b���}� t
d�    ����_^[��]� �SV�ڋ��F��|�>�����t����^����^[Ë��P�HÐSVW�؋{��|$��| �s+���~;�}��Cǋ��YX��s�3���_^[�f��r	tf��t��P�P�P�P�@ÐS�H��t�X�ڋ�����[Ë�U��QS��h��  �ʸ\�@ ������E�3�Uhs�@ d�0d� �U�������3�ZYYd�hz�@ �E���[�����`����[Y]Ë�SV�ڋ����   3ҋ��[����t����]����^[Ë�S��3ҋ��   3��C3��C[�SQ�$�؋ԋË�Q�ЋK��������$�CZ[Í@ SV��؋������֋�������s^[�SVW�: ~�
���  �� ����
�p�΋:�X;�tR�: u���Z���3��B��u�  I ���������  I ��������ȅ�u��  ��@�@ �ջ���`����_^[Ë�SVWU���؋C��|8��|4�����~,;{~;{~	�׋������{�SS�ŋ��rV���{���3�]_^[�U��SVW��t����J\����ڋ�3ҋ��;Z���EP�E�7f���ȋ֋��   ��t
d�    ����_^[]� �U����SVW3��E�3�Uh��@ d�0d� �E�P�E�@��E��E��U�3ɸ
�  �T����M����@ �-����_��3�ZYYd�h��@ �E��wb�����^����_^[��]�U��QSVW�M���؋EP�E�PV�Ȇ�����{��uU�a���Y�CPV�V������{��uU�G���Y�CPV����P�CP�J����Ћ�Y����_^[Y]� SV�ڋ��F�N����FP艆��3ҋ��(Y����t���=[����^[ù�  ��@�@ ������^��ÐU��SVW��t�����Z���ڋ��u�O����S���G�w��t
d�    ����_^[]� SV�ڋ��F��t�V�S����t����Z����^[Í@     ��@     �@         ,�@    �@ ,,@ �*@ +@ L+@         |@    |@     7�@      	FInstance    FInstanceRoot
TPropFixup ܤ@ ت@ �@ U��SV��t�����Y���ڋ��N�E�F�E�F�F�U��`���F�U��`����t
d�    ����^[]� U��S�؋E�@����u�����}�E�@����d�����|�E�@����o���[]�U��S�؋E�@����=������t�U�R�������E�@���������}�E�@����*���[]Í@ U����SVW�=`I  �4  �lI �x �%  ��\�@ ��V���E�3�UhW�@ d�0d� ��\�@ ��V���E�3�Uh�@ d�0d� 3��q�֡lI �Q����؋C�`I ����u�S�C�������t:��t�S���1  �ȋS�C�F���U�C�����Y�֡lI �������}V���U�C�����YF�lI ;p|�3�ZYYd�h�@ �E��QV����W[�����E��XK��|'C3��֋E��������Ǻت@ �kV����t�gFKu�3�ZYYd�h^�@ �E��V����[����_^[YY]Í@ SVWU����lI �XK�� |B�ӡlI �J�������t;~u%��t�V���y�����u�ӡlI ��������U��K���u�]_^[Í@ S�؋˲�H�@ �s����>[��[ø�  �����Ð��  �����ÐS�b����؃��u�������[Ë�SVW�ڋ��V+Vf� �F�8�W3ҋ��Z�����t���/W����_^[ÐSV�ڋ����?  :�t�N���5  �|���^[ÐU��SV��؋SP��術����u�ӋE�U�CP�q]��^[]� �@ U����S�ډE��E��PP���k�������   �E���  <
t�E��H�E���  �E��@L��������@ �ET���E�3�UhN�@ d�0d� �U��   �E���  �U�E�������E��P�M�E���  �E��@L�U��E�U3�ZYYd�hU�@ �E��T����Y�����E���P�\��[��]� �S�؋��  �����K[�SQ�$ f�xF tT�؋ʋЋCH�SD�$Z[�SVW����������؍D$�ֹ�   �Ma���T$��T���$�<$ �D$f�{6 tT�D$P�΋ӋC8�S4�|$ t������$��  _^[Ë�U��QSVW�E��E��@,��td3�Uhu�@ d�2d�"�pN��|1F3��׋E��@,�����؋S�E��@�.  �ȋS�C����GNu�3�ZYYd�h|�@ �E��<   ���W����_^[Y]Ë�SV�؋s���&����ЋC+C+Ћ��%���3��C3��C^[�SVW�؋C,��t+�pN��|F3��׋C,�����R��GNu�C,�vR��3��C,_^[ÐS�؋��  �K[Ð�  �����ÐVWS�׉ˉ��6�N+Nw
���2   �N9�r��V)ˋFFN�Ɖ�����у��^	�u�[_^Í@ SV�؋S�K�C�0����s��u��  ��H�@ �Ͳ���W��3��C^[�S�؋���  <	��[Í@ U����SVW3҉U���3�Uh�@ d�0d� ����}����U��   ���2����E�<t�K�U����
  ������U��   ���
���3�ZYYd�h�@ �E���Y����KV�����E�_^[YY]Í@ U��QSVW�U���3��E������3�Uh��@ d�0d� �]�������,s����  ����E������E��:���;�}�֋E��4��������?  �	�׋��  ���������t���*  F��������t����  3�ZYYd�h��@ �E��������U����_^[Y]�U��E�@�u	�E�x� t3�]ð]Í@ U��SV��3�芮���Xc@ �P����t;�EP����Y��t��'P��3���E�@�3��  �Q����P�E�@��Q�؋�^[]Í@ U��SVW3�Uh��@ d�0d� �E�P�E�@�@�|����U�R�J$��P$�U�B��E�@��H3�ZYYd��#�R���EP�E����<���Y��u�jU���U��_^[]�U��SVW3�Uh%�@ d�0d� �E�@�P(�E�@�f�����O���E�H�E�P��E�@��S3�ZYYd��#�FR���EP�E��������Y��u��T���DU��_^[]Í@ U����SVW3�Uh��@ d�0d� �E�P�E�@�@��)  �U�B��E�x� u)�E�@�E��E��E�Pj ��  ��H�@ �����iT��3�ZYYd��#�Q���EP�E����/���Y��u�]T���T��_^[YY]ÐU����SVW3ɉM�M�U��E�3�Uh�@ d�0d� �M�U��E���  �U�E���  �U�E���  �E��E��}� u�E�t	U����Y�U����Y�}� ��   3�Uh��@ d�0d� �E��H�E�uU�Z���Y�}� u
3�ZYYd��n�E��H�U�E���Q�E��`��E�t�E�@(�M�U�f����=N���E�@0�U��g���3�ZYYd��"�P��U�����Y��t�E��eM���@S���S��3�ZYYd�h�@ �E�   �	V����GR����E�_^[��]Í@ U��QS�ډE��E��x, uO��\�@ ��L���U��B,3�Uh��@ d�0d� �ӋE��3   �E�����3�ZYYd�h��@ �E��*������Q�����ӋE��   [Y]�U����SVW��E��
�֋E��n  �E�������t�E��
  �E��@(�E��E��@$�E��f����M���U��B(3�Uh^�@ d�0d� ��f�����L�����E��x$��u�E��@�U��B$�
3ҋE������E��6�����t�E��  3�ZYYd�he�@ �E��U��B(�E�U��B$��Q����_^[��]�S����؋��  <u�Թ
   ���}�����K����   �D$�D$�<$��,$��[ÐSVQ��؋���  ,t��t=��tG,tQ�]�Թ   ���.���3Ɋ$��3���T�����UW����3Ɋ$�������/�ƺ<�@ �T���!�ƺL�@ �{T����ƺ\�@ �mT�������Z^[�  ����   False   ����   True    ����   nil S����؋��#  ,t
��t ��t6�D�T$�   ���}����D$�$�/�T$�   ���c����D$�$��Թ   ���K���������$YZ[Ð��q����3��i����SVW����ؠ�@ ��������$�<�u���
  $��t	���P����_^[�      U�������SVW3҉U���3�Uh��@ d�0d� �E��R���E�@��ت@ �KJ����t�E��U�R��R�*S���}� u!�������E�@�� �I���������E��S���E�������ƅ�����E�@�������ƅ�����C������ƅ����������Pj��  ��H�@ �ު���9O��3�ZYYd�h �@ �E��R����fN����_^[��]ÐU��E�@��~
  ��  ����]Ë�U����SVW3ɉM��U��E�3�Uh��@ d�0d� 3�Uh��@ d�2d�"�U��E��  3�Uhs�@ d�2d�"�   �E���R�����E��E��E��@L���C;�|
�E��|�.u�E��PP��+ϋ׋E���T��;�|`�E�� �iI���U�RP�޸������u�E��$���3��E��8u�׋E�覹���E�E�ܤ@ �H����uU� ���Y�E�E�C�t����E�� �	I���U�RP�~�������t�ϋU��E��  �*�E��@L �U�E���Q�E��@L�E�xP t�E�����3�ZYYd���K��   Xc@ ��@ U����Y��M��3�ZYYd��5�K��   Xc@ ��@ �ËE�xL t�S�E��Q��u�nM���M��3�ZYYd�h��@ �E��P����zL����_^[��]ÐU����SVW�M�U��E��0I �pN��|<F3��ס0I �����؋E�� ;Cu�U��E��S��t�M��U��E�軸���	GNu�����_^[��]ÐU��j j j SVW�ىU���3�Uh,�@ d�0d� �E��{O���E���P���ӸD�@ ��S������t&�E�P��I�   ����R���E�P�V�������R���E�@��@P�E�P�E�P�E�P�ϲ���@ �Y����؃}� u�E�@��@,���5�����ӡlI �'���3�ZYYd�h3�@ �E��   ��N����3K����_^[��]�  ����   .   U����SVW3ۉ]�ىU��E�3�Uht�@ d�0d� ��~ u
��  �J����63����
��  �$���@ ^�@ ��@ �@ �@ C�@ a�@ ~�@ ��@ ��@ ^�@ a�@ �E��$���<u�U�E�������M�ӋE������u  �E��o����ȋӋE������\  �E�����3ɊȋӋE������A  �U�E������U��������ȋӋE��ƶ���  �E��!�������<$��ӋE�������   �U�E���  �M�ӋE��G�����   �֋E��  �ȋӋE��p�����   �E��K���,t��t�7�E���  3ɋӋE��E����   �E���  �ӋE�辵���ЋE��H����|U�U�E������M�ӋE��a���Y�`�E������<u�E��~  �4I �ӋE������;�U�E��~����M�E��P�E��0�V�E��E��@�E�}� t�M��ӋE�����3�ZYYd�h{�@ �E��L�����H����_^[��]Ë�U����SVW�ڋ�3����fL���=`I  t9�Ë���L���"FS�}��E��u��E� �U�   ���@ 贐����`I ��u�_^[��]� ����   %s_%d   U����SVW3ɉM�M�U��E�3�Uh�@ d�0d� �E��  3��E�3�Uh��@ d�0d� �M��U�E������}� u5�U�E��  �E�����3ɲ�P$�E�U�E��  �U�E��Q�H�E��E�U�E��f  �E��@t�U�E��R  �!�U�E��E  �E�U�������U�E��Q�E�U��B�=dI  t�dI �U��B0���\�@ �@B���U��B03�Uh��@ d�0d� �E��P�E��@0�#����E��@�U��B$�E��@�H�E��@�H�U��E��@��Q�E��@�`��=dI  u/�E��@0�XK��|!C�E�    �U��E��@0�Z�����R�E�Ku�3�ZYYd�h��@ �=dI  u�E��@0�A���E�3҉P0��F�����Y���3�ZYYd��'�D��3ҋE������}� u�E��vA���QG���G��3�ZYYd�h�@ �E�   �J����XF����E�_^[��]�U����SVW3ɉM��ډE�3�Uh��@ d�0d� 3�Uh��@ d�0d� �E��A  <t������蹰���p3ۍU��E��   �}� t�U����y�����w�����3�ZYYd����C���E��  �F����F��3�ZYYd�h��@ �E��AI����E������_^[YY]�Q�Թ   �+����$;$I t
�	�  �����Z�SVQ��؋Թ   �������3Ɋ$��3���I�����&L����3Ɋ$�������Z^[�SVQ���3��$���M   ,t,t� �Թ   ��������Թ   ��������n����Ƌ$3��bI����$���~���Z^[Ë�Q�Թ   �k����$ZË�U��j SVW��3�Uh��@ d�0d� �U����.����}� u�3�ZYYd�h��@ �E��H����}D����_^[Y]ÐU��S�]������   ��^�����t�����[]�U��� ���S�؅�~?��   ~�� ����E�@��   �������   ��� ����E�@�������3ۅ��[��]�U��Q�U��E�@��   �����EP�E�����YY]ÐU����SVW3҉U��E�3�Uhu�@ d�0d� �E������������   �$���@ _�@  �@ 	�@ �@ %�@ 3�@ A�@ A�@ _�@ _�@ N�@ W�@ �_U�����Y�VU�   ����Y�HU�   �����Y�:U�   �����Y�,U�
   �����Y��U��E������U����Y��E��E���3�ZYYd�h|�@ �E��F�����B����_^[YY]ÐU��j SVW��3�Uh��@ d�0d� �U����N����������3�ZYYd�h��@ �E��>F����B����_^[Y]�U����SVW3ɉM��3�Uha�@ d�0d� ��t*�M��U���������U��������U�����������h������E�����t�������	����������(�����t������3�ZYYd�hh�@ �E��E�����A����_^[��]ÐSVW����f�x> tW�؋΋ЋC@�S<��Ƌ�Q_^[�SV�ڋ����  3ҋ��������t���>����^[Ë��@,�p���Í@ U��S�؄�t����  �ӋE�U[]� ��U��S�؄�t���  �u�u���   []� �S�؋C����C[��  Ë�SV��؋C����;�	�S�;�~����   �֋C�t���^[�+��s^[ÐVWS�։ˉ��6�O+Ow
���   �O9�r��)�W�GGO�ǉ�����у��_	�u�[_^Í@ U����S�ز���@ �o;���E�3�Uh$�@ d�0d� �U��E�U�
����  �E�������E��U��   ���b����E��P�M����R���3�ZYYd�h+�@ �E��5;����;@����[YY]� ��S�؋S�K�C�����3��C[Ä�t�	�M  ò�E  �U��j SVW�ڋ�3�Uh��@ d�0d� �E����D���U����  3�ZYYd�h��@ �E��]C����?����_^[Y]Í@ SVWU��ز����  ���������N��|'F3����  �׋�������Ћ���  ���  GNu܋��  ]_^[Ë�U��QSVW�E��E�@��@,�XK��|(C3��E�@��@,���������U��F��x����tGKu�3���_^[Y]Í@ U��QS�ډE��K�E��x, tU�C����Y�U��B�U��Ë�Q �c�[Y]Í@ U�������SVW3ɉ������U��E�3�UhI�@ d�0d� �������E�� ��8��3ۊ������E��@�C���C�����E��@�U�+B;�~�E��4����E������E��X�@ �E�E��x t�M��E��X,��t1�C�U�;B0~&�E��x t�E��P0�E��@,�����U�;Bt�M��E��H4�U�E��8  �������E�� �$8���������������B���������E��e  �E��P�E��W  �E��_����E��E��@,��t �@�U�;B0~�E��x t�E��@0�E��@4�U��E���  �E��  �E��@,�E�E��@0�E�E��@4�E�3�Uh��@ d�0d� �E�3҉P,�E�3҉P0�E�3҉P4�E��x  ��   3�Uh��@ d�0d� �E��X��t:�úت@ �=8����t*��\�@ �7���U��B,�E�Ph��@ �E��@f����o8���E�PhX�@ �E�f����Z8��3�ZYYd�h��@ �E��@,�7����<����3�ZYYd�h��@ �E�U��B,�E�U��B0�E�U��B4��p<���݋E��  �E��U�;Bt'�\�@ :E�u�E�������U���;�u�U�E������3�ZYYd�hP�@ �������?����<����_^[��]�        U��S�ز���5  �U�
   �������[]� �SV��غ��@ ���u����u����  ^[ú�@ ���ju����u�	����  �-��@ ���Ou����u�����  �����  �֋��  ^[�   ����   False   ����   True    ����   nil SQ�$�؃<$�| �<$����h  �Թ   ������Z[Á<$ ���|"�<$�  ����<  �Թ   ������������#  �Թ   �������Z[Ë���	  �3��  �SVQ��$�ؠ��@ :$t(�$��D$�T$�   �������$t	�֋��@���Z^[�    U����SVW�U��E��E�� �'6�������@�E�}� ��   �E����&0���E�3�Uh��@ d�0d� �E�� ��5���U��ϥ���u�N��|(F3��E����ӋE�������t�ˋU��E��	  GNu�3�ZYYd�h��@ �U����E���/�����9����U��E���Q_^[��]Ë�U��S�E�@��x t0�E�@���3���؋E�@��@��3��;�t�E�@��U�R�;Bt3�[]ð[]Ë�U��j SVW3�UhC�@ d�0d� �E��U�R���>���U��E�@��w	  3�ZYYd�hJ�@ �E��<����9����_^[Y]�U��j SVW��3�Uh��@ d�0d� �E�@�襣���x�E�@���!
  3ۋÃ�w��s�M��Ӌ�臣���U��E�@��U	  C�� uԋE�@�3��B	  3�ZYYd�h��@ �E��-<����8����_^[Y]Í@ U����SVW3ɉM�U��E�3�Uhq�@ d�0d� �0I �pN��|:F3��ס0I 費���؋E�;Cu�U�E��S��t�E�@��U��=����GNuɋE�@��U������3�ZYYd�hx�@ �E��;�����7����_^[��]ÐU��j SVW��3�Uh�@ d�0d� �EP�W���Y�E��U�R��R(��;��3�Uh��@ d�0d� �E�@���(�1;���E�@��������3�ZYYd�h�@ �E�@���(�U��V;����d7����3�ZYYd�h�@ �E���:����G7����_^[Y]Í@ U��E�@P�u���Y��t%�E�@�P�E�@�@��@�>����U;B���]ËE�@��U�R�R�;B��]�U����SVW3��E�3�UhB�@ d�0d� �E�P�E�@������E�U�z���Y��uy�EP�<���Y�E�@�� ��t��t!��t<,t)�U�EP�E�@� �U������Y�>�U��E�@��d����.�EP�E��F���Y��M��E�@��U������U��E�@��X���3�ZYYd�hI�@ �E��9����6����_^[YY]�U��E�@P�M���Y��t*�E�@�P�E�@�@��@芤���E�h��������]ËE�h����@ �����]�      U����E�P�E�@��I����}��U����Y��u �EP�!���Yf�E�P�u��u��E�@��r�����]Ë�U��j SVW3�Uhk�@ d�0d� �E�@P����Y��t0�M��E�@�P�E�@�@��@�P����U��E�@��>;�����
�E�x� ��3�ZYYd�hr�@ �E��8�����4������_^[Y]Ë�U��j SVW3�Uh��@ d�0d� �M��E�P�E�@�����U�H���Y��u�EP�>���Y�E�@��U��  3�ZYYd�h��@ �E��"8����4����_^[Y]�U��S3ۋE�@P����Y��tl�E�@�P�E�@�@��@�w����؅�tL�C�U�R�R�;B$u;�E�x� t2�E�@��@�U�R�R�;Bu�E�@��P�C�m����u�E�X��E;X���[]ÐU��SVW���؋E�@�@��@�s;�u�ǋS��7���$��t�vh��@ �s�Ǻ   �t9������77��_^[]�  ����   .   U��3�QQQQSVW3�Uh��@ d�0d� �E�P�E�@�荟���E��}� u'U�����Y��u�EP�����Y�E�@���r  �m  �E��ܤ@ �\.�����X  �E��ت@ �G.����tNU����Y���8  U�U��E�����Y�U��E��7���}� �  �EP�a���Y�E�@��U��������   �E��L�@ ��-����tF�EP�����Y��t&�E�P�E�@��@趞���ЋE��(�������   �EP�E�����Y�   �E�@��X�E��U�R��R(�x6���E�@��p(�E��U�R����6���u�h��@ �E�@���(�   ��7���EP�R���Y��t�E�P�E�@��@�!����U�R��B�E�@��U��^����E�@��X�E�@���(�U��5��3�ZYYd�h��@ �E�   �b5����1����_^[��]�   ����   .   U�������SV3ۋE�@P����Y��t �M��E�@�P�E�@�@��@�����]��E;X�t5�E�p���t'�������E�֋E�@�@��@� �Z-�������� t3���^[��]ÐU�������SVW3��E�3�Uh A d�0d� �M��E�P�E�@��m���U�C���Y��uT�EP�Q���Y�]���u�E�@����  �4�������E�@��@� ����,���������E��<5���U�E�@�����3�ZYYd�h A �E���3����W0����_^[��]Ë�U�����M�U��E��E�x tn� �E��E�� ��
w^�$�G A � A s A s A s A | A � A s A � A � A � A � A U�����Y�"U�&���Y�U�����Y�U�H���Y�U�����Y��]Ë�U��j SVW���3�Uh� A d�0d� �S(�E���3���E����4���U����#   3�ZYYd�h� A �E��3����l/����_^[Y]�SVQ��؋��f4���$�<$�   ~�$�   �Թ   ���1������6���Ћ$������Z^[Ë�SVQ��؋��4���$�<$�   ����9   �Թ   ������������    �Թ   ��������֋$�������Z^[Ë�Q�$�Թ   ����ZË�SVW��t����i+����ڋ���A �G��t	�׋��n  ��t
d�    ����_^[�     SVWUQ�$�؋C��t/�pN��|F3��׋C貱����Ӌ(�UGNu�C�!)��3��C����  ���  �C��t���@  �<$ t���+����Z]_^[Í@ SV��؋C��t;Ft6�{ u��\�@ �(���C�֋C�{�����}�֋C艰���Ӌ�����^[ÐSV��؋�����f�C^[�SV��؋�����f�C^[��@��2���Ð�@��&���ÐSV��؃{ u��\�@ �(���C�֋C�����^^[ÐS��3��B�C�̱���C�x u
�(��3��C[Í@ SVW�ڋ��CP3ɋӋƋ8�W�Ӌ����������  �Ft	����q  3ɋӋƋ�S_^[Ë�SVW�ڋ��ӋƋ8�W3ҋ��  �Ӌ��l���3ҋ��3  j �K�ӋƋ�S_^[ÐSV���議�����֋��:�����Ƌ�Q��C��u�^[Í@ SVW���Cu)�K�C��t�pN��|F3��׋C襯�������GNu�_^[�SVWU����L$�$�؋C��t"�|$u�$袰���C�x u
��&��3��C�C��t%�pN��|F3��׋C�C����L$�$�(�UGNu�YZ]_^[�U��E�x� t�E�@�f�@�U�R�f;B��]ËE�@�f�x ��]Í@ U��E�x� t�E�@�f�@�U�R�f;B��]ËE�@�f�x ��]Í@ U����SV�ډE��C�E��E�Ph�A �E�Ph�A U�e���Y�Ⱥ�A �Ë0��E�Ph�A �E�Ph�A U�w���Y�Ⱥ�A �Ë�^[YY]�   ����   Left    ����   Top 3�ÐU��]� �3�ÐÍ@ Í@ 3�ÐÍ@ S�؀K@[Í@ S�؀c�[Í@ S�؀c�[Í@ ��F���Ð���QÐU����SVW�M����؋u��t>�֋E��d����t0�֋��J   ��t#�u��E��E�Pj ��  ���@ �|�����*���Ct�{ tV�M��׋C��S_^[��]� SVWUQ�$���<$ t3� t-�G�XK��|"C3�ՋG�>������$�F�yc����tEKu�3���Z]_^[Í@ SVW�����؋C���/��tl��t,���e����u!�4$�D$Tj ��  ���@ �Å���*���C��tV�K�Ӌ8�W�V�K3ҋË8�W3ҋ��  �֋��   �����   YZ_^[�SV��؍C��� -��^[Ð�P��t�z t
�R�責��Ã��ÐSV��؃{ u
��  �n����֋C�D���^[Ð�P��t�B�3�Ë�SVW��؋C��t?�@���`�����|1�S�z�W��}3�;���N;�t�ג薫���C�@�ˋ��K���_^[Í@ SVWU�ڋ���t�O��g��������M��|E3��֋��H����������FMu�]_^[�SVW�� ����ڋ��w��t(�ċW��   �0���ԋ��|$����t��t�8�3҉��   _^[Ë�SVW�0I ��XK��|C3��֋�6����"��FKu��"��_^[Ë��(I �"���,I �"������3�3������lI �x"���hI �n"��Ð�<I �6A����\�@ �."���(I ��Ц@ �"���,I ��\�@ �"���0I ��\�@ ��!���lI �O���\I ��\�@ ��!���hI Ë��pI ��!��Ð�HI �@��3��pI Ë�                        �	A    Xc@ ,,@ �*@ +@ L+@ 
EMenuError�
TMenuBreak       �	A mbNonembBreak
mbBarBreak�	TShortCut    ��      d
A �
A         ~
A �
A �   ت@ ,,@ �*@ +@ �A H�@ �A d�@ �A <A �A �A �A �A xA 0A �A         |@     |@ 8    �����������A �A �A A �A 	TMenuItem	TMenuItem4
A X�@  Menus �	A .  ��A       �     Break|@    ��A       �   � Caption @ (  �A       �     Checked @ *  �0A       �     Default @ )  ��A       �    EnabledH@ -  �A       �     
GroupIndexԠ@ 4  �4  �      �     HelpContext|@ 8  �8  �      �   �	 Hint @ +  � A       �    
 	RadioItem�	A @8

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

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ^[ÐSVW�ڋ�:^-t�~D��t�֋��  �Ћˋ��i����^-_^[Ë��P<��u3�ËBË�SV��؃{< u������֋C<�u���^[Ë�f�P@���Q(ÈP,���Q(Ð����HD��t�Ћ��=  �Ћ��SVW��؋CD��t4������}3�;���N�������;�t�{D�Ӌ��5  �ˋ֋��   _^[Ë�U��QSV�E��E��G�����K��|C3��֋E��C����ЋE�UFKu�^[Y]� SV��ڋú4
A ������g���^[�SVWU�ڋ�:]*tJ��t:�}D t4�ED�������O��|%G3��֋ED������x* t�֋ED������@* FOuވ]*��ŋ�Q(]_^[�SVWU����؃~D t�6�  ���	A �n���j���{< u��\�@ �l���C<��M��|.�C<;h}&�ՋC<������@-:F-v�ՋC<�����P-�������N-�׋������΋׋C<�(����^D���   ǆ�   PA �{$ t���$�����Ë�Q(]_^[Ë�SVW��؅�|�{< t�������;�|������֋C<�e������֋C<�����3��GD3����   ���   �{$ t��������Ë�Q(_^[Àx) tf���    t�ȋЋ��   ���   Í@ V����p<��t	���>����ȋ�^�SV��؋��C����Ћ΋�����^[ÐS�؋���������u�7�  ���	A �,m�������Ћ�����[�Sf���    t�؋ʋЋ��   ���   [ÐSV�؄�t�{$ t��������CD��t3ɋ������^[Ës�ƺ`A �����t���^  ^[Í@ �PD��t���    t���   Ë�Ë�SV��؋CD��t���8�����t5�ƺ�A ������t�F ���������ƺ4
A �����t	�Ӌ������^[�:P+t
�P+���Q(�U��QSVW��t����	�����U��؋˲�4
A �<������s ���   ��@(���   ���   ��3ҋ��U����}� t
d�    ����_^[Y]ÐSV�ڋ��F ��
��3ҋ��i�����t��������^[ÐU��S�u�u�@ f����F��[]� ��@ �|���Í@ SVW�����׋F f������_^[Í@ U��QSVW���E� �E�x� u�E�@��S0;�t+�E�x�u�E�@�;C$t�E�x�u�E�@��S@;�u�E�X��E��2���������N��|$F3��EP�׋����������Y��t�E��GNuߊE�_^[Y]Ë�U����S�M��U�3҉U��� A �P �Zd���������E�[��]ÐVQ�$3���<$ t3���������@D��t�x4 t��t�p4��Z^�S3���3�������t��R,���[�S3۱�v�����t��R,���[Í@ U��QSVW���E��CD��t�UR�����Y�E��}�u\�{) tR3�Uh"A d�0d� �Ë�R,�E�@�f�@@�Uf;B�t�E�3�ZYYd�� �����E�P��(I ��# ������E� �E�_^[Y]�U����S�ډE�3��U�z$ tj3��Cf�E�j�u<��f��}f�E�  j�c<��f��}f�E� @�C tf�E� �3��U���E��x����E��}� tU�E�����Y�؀�tփ}� ��[��]Ë�U����SVW�E��E�P�<���E�E�� ����E����E�3��~h   �E�+�PSV�E�P��;������N����h   V�E�P��;����f�� t��#A ���?O����f��@ t��#A ���*O����f�� t��#A ���O���غ $A ���O����F;u�}	;]��t���_^[��]�   $   @   #   ;   U��� ���SV��3�ƅ ��� �~$ tU�Ƌ�R,����Y�~( t�F(������ ����UO����t��F(�� ����   ������^[��]ÉP$����Í@ Í@ :P4t�P4�P$��tj j h�  R�<��ÐS�؃{$ t���\�����t	�C$P�9��[Ð��t�@ �R �����3ҋ@ �v���Ð��t�H �Id;J u
3ҋ@ �[���Ë�SV��3�3ҋË0�V(�C$��tj j h�  P�<��^[ËP0��t��������                        d%A    \�@ ,,@ �*@ +@ �@ ��@ ��@ 
TPopupList�U����SVW��E�3�Uhm'A d�1d�!���S��   -�   t��tM����   �  �E��XK����  C3��׋E�����f�V������t3�ZYYd��  GKu��Y  �E��XK���J  C3��׋E��ȍ���V�������t3�ZYYd��_  GKu��  �E� �Ft�E��E��XK��|;C3��׋E�耍���V�M�� �����t�P8�(I �$ 3�ZYYd��  GKu�3ҡ(I ��# �   �F�E�E��XK����   C3��׋E�������R,�U�;Bu�׋E������U�f�R��������u�׋E������U�f�R3������,I �R@��ud�    ���u��  t�Ⱥ   �(I �g! ��С(I �M! 3�ZYYd��CGK�b����FP�FP�P�E��@P�6���F3�ZYYd�������U��(I �� �
��_^[��]ÐSV��؃{ uShp%A �u�  �C�֋�艋��^[Ë�S�؋��F����{ u�C���  [Ë�SV��t����R���ڋ�3ҋ��)����F ���   ǀ�   X(A �(I �@ �F$�F-�֡�I �s�����t
d�    ����^[�SV�ڋ��֡�I �v���3ҋ��5�����t�������^[Ðf�x6 t�ȋA8�Q4Ë@ �@4Ð�@ �P4ÐSVW����؋Ӌ������j ��I �@Pj WV3��C,f�E�I f����P�C ����P�@9��_^[�U��QSVW3�Uh)A d�0d� �4
A �E��E�3�譅����������@ �8����I ��\%A �'����I 3�ZYYd�h#)A ��C����_^[Y]Í@                         `)A    �c@ ,,@ �*@ T�@ H�@ EOutOfResources                        �)A    Xc@ ,,@ �*@ +@ L+@ EInvalidOperation��TCursor ����  ��                        *A    �@ ,,@ �*@ +@ L+@ �9A :A 0:A �9A �9A 8:A <:A TDragObject                        x*A     *A ,,@ �*@ +@ L+@ �:A 0;A H;A �9A �9A �:A ;A TDragControlObject�        �*A             �*A @   �A ,,@ �*@ +@ �AA H�@ P�@ d�@ ��A ��A �AA TControlCanvas�TControlCanvas�*A <�A  Controls  �@ TAlign       +A alNonealTopalBottomalLeftalRightalClient�	TDragMode       T+A dmManualdmAutomatic�@ 	TTabOrder�����  
TCaption��TMouseEvent  SenderTObject ButtonTMouseButton ShiftTShiftState XInteger YInteger�@ TMouseMoveEvent  SenderTObject ShiftTShiftState XInteger YInteger	TKeyEvent  SenderTObjectKeyWord ShiftTShiftState��TKeyPressEvent  SenderTObjectKeyCharTDragOverEvent  SenderTObject SourceTObject XInteger YInteger State
TDragStateAcceptBoolean��TDragDropEvent  SenderTObject SourceTObject XInteger YIntegerTStartDragEvent  SenderTObject
DragObjectTDragObject�TEndDragEvent  SenderTObject TargetTObject XInteger YInteger�    x.A �/A         �.A �/A �   ت@ �RA �*@ +@ �BA H�@ |SA d�@ �A xDA DA �HA �A �A tBA �FA <FA (MA @CA �CA �IA RA �NA �NA EA �NA         |@ T   * � 	  �����	�#�
������������������������������������������xTA �TA �UA �UA �TA �UA $VA tVA WA �WA �WA �WA XA 8XA @XA HXA �XA PXA lXA �XA �XA �XA �XA �GA �SA �SA �PA �PA QA �PA DCA PCA 4UA HCA TA LVA �VA xCA TCA MA RA �PA TControl�@ TControl$.A X�@  Controls 4@ $  �tEA       �   � Left4@ (  ��EA       �   � Top4@ ,  ��EA       �   � Width4@ 0  ��EA       �   � Height�)A L  �LA       �     Cursor|@ T  �T  �      �   � Hint�@         �2A         $1A �2A �   $.A �jA �*@ +@ \YA H�@ |SA d�@ �A xDA �ZA �HA �A �A �XA ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA �bA �dA �cA �eA ,eA �lA �~A �A F  N  9 - + ,  G      / .  � �    .������������"����$�%�&�'�/� �����,����������������������� oA LoA loA �oA �oA  pA  pA DpA hpA �pA �pA �pA |qA �qA �qA �sA �sA TtA ttA uA 0uA �uA �uA �uA vA LvA hvA �vA wA �oA �oA �oA |wA �wA �wA �wA �wA  xA @xA hxA �xA �xA $yA �wA ,wA DwA PwA \wA LyA TyA \yA dyA lyA `zA 8{A �{A �{A 4|A �yA �}A �rA �rA ��A �rA �sA �tA �vA �A �A ̃A TWinControl��TWinControl�0A �/A 	 Controls Ԡ@ �  ��  �      �     HelpContext        �3A         �3A �3A �   $.A �RA �*@ +@ x�A H�@ |SA d�@ �A xDA DA �HA �A �A $�A �FA <FA (MA @CA �CA �IA RA �NA �NA EA �NA  �A   ��A TGraphicControlTGraphicControlT3A �/A  Controls  ��        �4A         �4A �4A    �0A �jA �*@ +@ @�A H�@ |SA d�@ �A xDA �ZA �HA �A �A �A ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA �bA �dA �cA �eA ,eA t�A �~A �A ȊA   l�A TCustomControl�TCustomControl 4A �2A 	 Controls  �@         �5A         �5A �5A     4A �jA �*@ +@ @�A H�@ |SA d�@ �A xDA �ZA �HA �A �A �A ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA ��A �dA �cA �eA ,eA t�A �~A �A ��A <�A D�A  ���A THintWindowTHintWindow5A �4A 	 Controls  ��U��QS�]���I �U���   ��I ���   Pj��EP�+��j��EP�)���   @tj�EP�)����u�EPj�EP�{+����I P��I P�EP�4+����I P��I P�EP�+���u�u�u�u��I ��I     ���   �E��[Y]� SVQ��3���t%TS�N)����t�-"��;$u��I PS��(������Z^[Ë�SV�(I �s ��tQRPV�X*��^[�3�^[�����	   crDefault   ����   crArrow ����   crCross ����   crIBeam ����
   crSizeNESW  ����   crSizeNS    ����
   crSizeNWSE  ����   crSizeWE    ����	   crUpArrow   ����   crHourGlass ����   crDrag  ����   crNoDrop    ����   crHSplit    ����   crVSplit    ����   crMultiDrag ����	   crSQLWait   ����   crNo    ����
   crAppStart  ����   crHelp  ����   crSize  SVW��   ��I �;�u��ƋW��������Ju�3ۋ�_^[Ë�SVWU����   ��I �׋�u1����u��S��U ���Nu�3�]_^[ÐSVW�����ָ9A �����؅�u�ǋ������W��I�   ������_^[�����   |   SVW�����ָH9A �A����؅�u�ǋ��j����W�S�������<���_^[�����   |   �%���6�����t�=|I  t�|I ;B u�|I �S���p'��3��|I ��t,�ú�0A ������u�{  t�|I �[ ����G  P�'��[Í@ �I Ë�SV�� �����؋ԋ������ԋ��B�����   ^[Ð3�ÐSPh@:A � �  ��S�T'����[�S����&����詺  [Í@ U���tf����f���]� �@ U��]� �Í@ Í@ U����SVW��3�Uh�:A d�0d� �-   t��t&�+�U��C�H(���E�P��I P�a#���E��  ���x  3�ZYYd��������=�I  t3��Y  ���������_^[YY]Ä�t����`����H��t
d�    ��Ë��@��R4Í@ S�؋C��R4��t�C��R4�4S  [Ë�S�؋C��R4��t�C��R4��R  [Ë�U���t	�@f�@N�f���]� U��QSVW�M������} u�Ff��������EP�M��׋Ff�������_^[Y]� �@ U����S�M�3Ʌ�t/�M��]�Y�]��M�M��M��M�M�Q��Rh/�  P�o%���ȋ�[��]� SQ�؅�t'TS�$����t����;$u��I PS�#����u3�Z[ðZ[Í@ S�p�0�-&���؅�t��������uS�V#���؅�u��[�SV��؋�������j S���I ��/���^[�S��3��=�I  t#��I Ph�I ��I �ӡ�I ���������[ÐSVWQ�؀=�I  u'��I +�3�+�}��I +C�3�+��(  �=�I  u&�=�I  t��I P�9"���Ћ�I ��I �P  ��I �ԋ��3�����;5�I t-��F����5�I �$��I ���I �C��I 3�� ������I �C��I �CP������Ћ��I �8�W���=�I  tj��t�F5 tA�ס�I �%O  ��I �xD u�CP�!���Ћ��I �WO  �C�K���I �*P  �2��I �P  �ס,I �1�  P�#����ס,I ��  P�n#��Z_^[ÐS�ڣ�I 3���I h�I �� ���� ����I ��I ��I ����I ��I ������I �=�I  t&�=�I  t��I P�� ���Ћ�I ��I �N  �=�I  t
��I ����[Ë�U����SVW�U����5�I 3�Uh�>A d�0d� 3��E���I  �U���f����D����}� u�β�\*A ������E���I �U��E�����3�ZYYd���|���3���I �8�������_^[YY]�U����SV��3��E�3���I 3�Uhk@A d�2d�"��I ��I �������I �E��=�I  t��I ��N  ���I P��!��3�UhE@A d�2d�"�5�I �ƺ$.A ������t�M���I ���  ���I �E��I �E��=�I  t�������t��u3�����3���I ���u�3҉�I 3҉�I 3҉U�3҉U���I Rh�I �M��С�I �u����E�PS�M��I �E���S3���I 3�ZYYd�hL@A 3���I �������3�ZYYd�hr@A �=�I  t�E��������������^[��]Ã=�I  t3�����3���I �SV�p�0�!����3���t�����������uS�����؅�u��^[Ë�SVWU����ڋ�3����������t���̋׋��i  �ԋˋ��r'  ��t����YZ]_^[Í@ SV��؃; u��\�@ ���������r��^[Í@ S�؋��s����x u�����3��[ÐSVW��������TS�)��j �D$+�P�D$+�PS����YZ_^[Ð3ҡ I �Hr���   Ë�������� I �x �ÐSV�ڋ����s   3ҋ��f  ��t��������^[Ë�S�؃{4 u	����n  [Ã{8 u2� I �@� I ;Bu�����S<�C4��Q0�C8�ӡ I �q���S8���n  [Ë�S�؃{8 t'3ҋ��rn  �ӡ I �r���C8P�C<P���3��C8[Ë�SV���;s4t
�������s4^[�U��QSV��t��������U���3ҋ��$���f��BA f�C4��l�A ��]  ���sD�^�FKA �CH  ��C7�C8�C9�C:�C^�C= f�CN���}� t
d�    ����^[Y]�   �   SV�ڋ��֡(I �6�  �FD�B����F@�1��3ҋƋ�Q<3ҋ��̾����t���1�����^[�3�Ð3�Ð�x  ��Ë@ �VW�����Ǻ�0A �:�����t	�׋Ƌ�Q<_^Ë�SVW����$���D$ ��f����m�������tO�T$�Ƌ�Q0�؊$4��PWS�V����S�����t�Ƌ�RDjWS�:��S����S�D$P����D$�D$YZ_^[Ë��P<ËP ��t��+  Ë�VWU�����N6�o(�ź�0A �~�����t	�ՋƋ�Q<�׋������f6��~  t0j 3ɺ	�  ���  j 3ɺ�  ���v  j 3ɺ#�  ���f  ]_^Ë�SVW�ً���ˋ֋�谿��;wPu��u	3ҋ��  _^[ÐSVQ���F;:�tM�$�V;�FuA�$H,���ʀ������:�u)�$��t",t��t��t�F0P�F,P�N(�V$�Ƌ�SL�������Z^[�U��SVW�����;s$u;{(u�E;C,u�E;C0t<3ɊS7����  �s$�{(�E�C,�E�C0�Ë�RDj 3ɺG   ���u  ������_^[]� �@ SV���F,P�F0P�N(�Ƌ�SL�N\^[Í@ SV���F,P�F0P�ʋƋV$��SL�N\^[ÐSV��R�F0P�N(�ƋV$��SL�N\^[Í@ SV���F,PR�N(�ƋV$��SL�N\^[Í@ �H$�
�H(�J�H$H,�J�H(H0�JË�SVWU���B�:+�P�B�j+�P�͋׋Ƌ�SL]_^[Ð3ɉ
3ɉJ�H,�J�@0�BË�S����؋ԋË�Q,�D$��[�SV�����؋��   �Ћ̋��1e���ԋ��  YZ^[Í@ S����؋ԋË�Q,�D$��[�SV�����؋������̋���d���ԋ��z  YZ^[ÐSV�����؃{  u$�C�$�D$Tj �/�  ���)A �E�������֋C ��Q(�C$�C(FYZ^[Í@ SVW�������؋ԋË�Q(�$��FD$�GYZ_^[�SVW�������؋ԋË�Q(�+$��F+D$�GYZ_^[�S��脮  ��t�����  [Í@ SVWU����$����;<$��   �Ft�^\���HA ��t�$PW�F$P�������n$��t�$PW�F(P�o���D$��F(�D$��t�F5u�$PW�F$F,P�F��+ŉD$��F,�D$��t!�F5u�$PW�F(F0P���+D$�D$��F0�D$�D$P�D$P�L$�ՋƋ(�UL�~9 u#��t�$PW�^D���Z  P�����Ћ��Z  ��HA �F\��]_^[�         U����SVW3ɉM��U���3�Uh;IA d�0d� �G4 t4�U����  �U��G����u�w��t�ƺ$.A ������t
�Ft3����U���衽����t
�U����  3�ZYYd�hBIA �E��������$�����_^[YY]Í@ SVW����<$���؍T$�Ë�Q,�C,+D$$P�C0+D$D$P�K(�S$�Ë�SL��_^[Í@ SV��؋C ;�t5;�u�C�  ���)A �B���a����C ��t���  ��t	�Ӌ���  ^[Í@ SVQ�$���F7:$t(��f����	����$�F7j 3ɺ�  ����  �������Z^[�:P8t�P8j 3ɺ�  �  Ðj 3ɺ   �  ÐR�   �  ÉPP��t�����Ë�S��R3ɋú   �j  j 3ɺ�  ���Z  [�SVW�����������؋ǋ�3��E�����t�K�������_^[ÐU��j SVW���3�UhKA d�0d� �U��������E�������t���Q����Ћ��l���3�ZYYd�hKA �E��������Y�����_^[Y]ÐS���C9 �CD�qW  ;CXt�K\�CD�`W  �CXj 3ɺ�  ���  [Í@ �@D��QÍ@ �@94Ë��@^4Ë�:P9t�P9�x  tj 3ɺ�  �T  Í@ :P]t�P]�@^ j 3ɺ"�  �6  Ð:P^t�P^�x  tj 3ɺ#�  �  Í@ ;PHt�PH�@: j 3ɺ�  ��  Ð�@:4Ë�:P:t�P:�x  tj 3ɺ	�  ��  Í@ f;PLtf�PLj 3ɺ�  �  Í@ S��� ���;���[ÐSV�ڋ��������:�t��t	���$����3�����^[�SV����f�������^[�SV��3ҋ�f�������^[�SVW��؋C ��tl���   ���}g����|[�S ���   �W��}3�;���N;�t>�ג�f���C ���   �ˋ��bg����S7���.  ���G�  �@6t�f�������_^[Ë��H ��t��t���   �RJ�f����3��]����SV����؃{  u$�C�$�D$Tj �/�  ���)A �h?��������C ��Q0��j �C(P�C$PV�����C0P�C,Pj j V�����YZ^[�U����SVW�E��E�@��@ ���   �E�P����gf���؅�~HK�Ӌ��f�����F4@t2�U܋������E�P�E���P�E�P�!���E���P�E�P�����u����E� �E�_^[��]Í@ U����S�M�ډE���u�E��@th�E��@5u_�E��x  tV�E��@ ��4  ��tG�U�E������}� u�E��@ �@4@uU����Y��t3�����P�E�P�E��@ ��2  P�|��[��]Í@ �@4@���P7�e����3��%����S�؋C ��t�Ӌ�Qt�Ct�C5t	��������[�V�p ��t�Ƌ�RP^Í@ ��RHË�U����S�E��E��x7 u�E��@��   �E��@5��   �E��x  ��   �E��@ ��3  ����   �E��@4@��   �E��@ �2  P����E�3�Uh�OA d�1d�!�U��R(�ʋ]�K0Q�E��@$�ȋ]�K,QRP�E�P�����M��U��E��@ �f  3�ZYYd�h�OA �E�P�E��@ �1  P� ���������E���RD�E���RP[YY]Í@ SV����ڋ��ƺ�A ������t�N�  ���)A �.<��������=�I  uU�5�I �F6t8T�����L$�ԋ��	����D$�$�D$�D$���o��P3ɺ  ���h  ;5�I u	�Ӌ��������^[ÐÍ@ ;�I ��Ë�U��SV�u�f���    tQ�MQ�MQV�؋ʋЋ��   ���   �� ^[]� U��Sf�xz tQ�MQ�؋ʋЋC|�Sx[]� �@ Sf���    t�ʋ؋Ћ��   ���   [ÐU��Sf���    tQ�MQ�؋ʋЋ��   ���   []� ��SVWU����$���$�[�s��ź\*A �9�����t�u�L$�Ӌ�������$�@�Ѐ�rt0�D�T$R�T$P�D$P�L$�֋�f����T���3��D$�$�B��D$P�L$�֋�f����0�����]_^[�U�����U��M�U�U�3҉U���t�U���Q@�E���]� Í@ SVW����Ct(����  ����t��,   t�΋�,  �Ӌ8���ur�=   |`=	  Y�C4�u-  t
��t��u�.�-   tHtHt,Ht�+�΋ӡ(I ��  ��{<u����Z�����K6��c6��֋�����_^[ÐSVW��؋��tJHtHt*�W�C@��t����(SA �NI�F���$ ���c���F�/�{@��u3��F�!���I���F��F�!�����C@�!���{@_^[�       SV��؋��S����C=^[Ð�@=�����Ë�U��E�@��@��t�U�@=�U�R�:B=��]ËE�@��@=]ÐU����S�U��E��E�Ph,SA �E�Ph@SA U����Y�Ⱥ�SA �E���[YY]�   ����	   IsControl   f���    t�ȋЋ��   ���   Ðf���    t�ȋЋ��   ���   ÐU��QS�M�f�xb t�M�Q�MQ�MQ�؋ʋЋCd�S`[Y]� �@ U��QSVW�M������F5u&�GP�G
Pf�G聠  ��
M�U���f�������_^[Y]� ��SV��؋Ӌ�������֋Ë�Q��C4t	��������C4t�K6��TA P��3ɋ��z���^[�       SV��؋Ӌ������֋Ë�Q�^[ÐSVW�����֋������׋Ƌ�Q��F4t	����,����F4t��f���������0UA P��3ɋ��
���_^[�  @   �@P�SVW���$���Gu\����tV��f��������؅�t>�{- t83ҋ������s0�T$�$�>���T$�L$�������L$�T$�Ë�S0��v ��u���_^[�SV��؋֋Ë�Q��UA P�ֱ���h���^[�     SV��؋֋Ë�Q��UA P�ֱ���@���^[� @   SV��؋֋Ë�Q� VA P�ֱ������^[�     SV��؋֋Ë�Q�HVA P�ֱ�������^[� @   U��QS�U�f�xj tQ�UR�؋ЊM��Cl�Sh[Y]� �SVW�����׋Ƌ�Q��F5u�G
Pf�G�D�  ���O��f����W���_^[Í@ U��QS�M�f�xr t�M�Q�MQ�MQ�؋ʋЋCt�Sp[Y]� �@ SVWQ�$�����F5u$�GP�G
Pf�G�؝  �ȊT$��f��������Z_^[Ë�SVW�������׋Ƌ�Q��F4t	3ҋ�������F6t:�f6��ԋG�j���t$�t$�T$�Ƌ�Q,�D$P�e	����t��f���������3ɋ��W�����_^[�SV��؋֋Ë�Q��ֱ���6����V������^[ÐSV��؋֋Ë�Q��ֱ������^[Í@ V���Ƌ�Q����;�����t3ҋ��>����F6tj�3ɺ  �������^Ë��@t�@5t�x7 t�@4@u3���������Ë���RDË���RDË���RDË�S�؀{: t�C �PH���^����C:[�S�؀{^ t�C �P]�������C^[�S�؀{9 t�C �PD�������C9[��B   ËP ��tP3ɋº�  ����Ð�P ��tP3ɋº�  �����Ð3��BË�U��QSV��t����:����U���3ҋ��p���Sh�gA �M�  ���   ��t�A �[L  �����   �SH���M  ƃ�   ǃ�   �����}� t
d�    ����^[Y]ÐSVQ�$�؋�虪���{  t	����  ���    t�Ë�Rh���S  ��t)��J���  ���֋��  ��Ƌ�Q����*  ��u׋��   �������   ��t��  3ҋ������<$ t���~�����Z^[�U����SV�E��E����    ��   ��\�@ �����E�3�Uh�ZA d�1d�!�E����   �@�E��U��E��>[���]�K��|0C3��E����   ���sY�����   ��|;U�}
�ȋE��fZ��FKuӋ]�K��|C3��֋E��AY����t���'  FKu�3�ZYYd�h�ZA �E������������^[��]Í@ U��QS�ډE��E���  3�Uh[A d�0d� �ӋE�����3�ZYYd�h[A �E��  ��W������E�������E��x  tj 3ɺ�  �E������E��  [Y]Í@ U��QS�M�3Ɋ]���t��t��t$��t+�:�@(;B(���/�H(H0�B(B0;�����@$;B$����H$H,�B$B,;�����[Y]Ð��}��ÐU��SVW������t��t��t"��t+�5�E�@��N0H�'�E�@��N0)H��E�@��N,��E�@��N,)H3����0  �$�\A 6]A %\A e\A �\A �\A ]A �E�@��@�U�Z��+ËV,�`���P�F0P�E�H��I+N0�E�@��ӋƋ�SL��   �E�@��@�U�Z��+ËV,� ���P�F0P�E�H��I�E�@��ӋƋ�SL�   �^,S�E�@��@�U�z��+ǋV0�����P�E�@��ϋE�P��+ӋƋ�SL�X�F,P�E�@��@�U�Z��[+ËV0����P�E�@��ˋE�P��R�Ƌ�SL��E�@�P�����u�E�P��������_^[]ÐU����SVW�E��E�@��V���E�x� tC�E�@�x7 u�E�@��@t+�E�@��@5u�E�@�@;:E�u�E�P�E�@��U���E�@��:  ��O��||G�E�    �E�@��U���  �؊C;:E�uV�{7 u�CtJ�C5uD�E;X�t<3��F�E�@�;p}�E�@�����U���ЊM����,�����t׋E�@��ˋ��V���E�Ou��E�@��xO��|)G�E�    �EP�E�@��U��U���U��K���Y�E�Ou�_^[YY]ÐU��SV��E�@��f  ��N�� |�E�@����!  �x; uN���u�3ۋ�^[]ÐU�����M��U�E�U����Y��tg��\�@ �b����E�3�Uh"_A d�0d� U��U���YU��L���YU��C���YU��:���YU��1���Y3�ZYYd�h)_A �E��7�����=�������]Í@ U����S�ډE��E���#  ��th�E�f���    t	�E��H6�R�E��R   3�Uh�_A d�0d� �U�E���Q,�M�ӋE���ST3�ZYYd�h�_A �E��`6�E��   �������[��]Í@ f���   �f���   f���    u�@6t�   Í@ 3��I������R ��t;�u�����ÐSV�ڋ�����  ��t	�ˋ���  ^[Í@ SV�ڋ���t<�ú�0A �n�����t���   ����������   ����������   �������s ^[Í@ SV�ڋ��ú�0A �&�����t���   ���������   ����������   ������3��C ^[ÐSV�ڋ�j�˺,�  �������Ӌ��M����C6utj 3ɺ	�  �������j 3ɺ�  �������j 3ɺ#�  ��������ú�0A ������tj 3ɺ�  ���������  �����!  ��t�Ë�RD�Ӌ������^[�SVW�ڋ��ú�0A �A�����t�������������  ����!  ��t3ɊS7�������Ӌ������j �˺,�  ���1������2���_^[Ë�V���   ��t�N�3�;�~	���R��^�+ы��   ��Q��^Ë�3ҋ��   ��tQ���   ��tQ��ÐSVWU�����������K��|C3��׋������֋�Q@�~ uGKu�]_^[Ã���҉$3҉T$3҉T$3҉T$��������Ë�SV��څ�t.�C$PV�I P�n�����u�C$PVj �^����C$%������C$^[ÐU�������SVW3ɉM��ڋ�3�Uh�cA d�0d� ��3ɺ�   �c����F@��C   D�F4t�K   �Fu�~8 u�K   ���    t�K   �F$�C�F(�C�F,�C�F0�C3��C�~ ��t
���@  �C�C$   �C(^@ h   j �����C<3��C@��������6����������E������U��CL���3�ZYYd�h�cA �E��h�����������_^[��]ÐS��D����؋ԋË�Q\�|$ u9�D$@t2�C��$�   Ƅ$�   ��$�   Pj �/�  ���)A ��(���"����D$(���   ��$�   P�D$PP�I P���������؄�t��5A ;�$�   tR��t�I P�D$PP������D$(�5A �I �D$4�D$L�D$H�D$$P�q���f��u�,�  ��`)A ��'��������I �ԋË�Q`���    u�-�  ��`)A �'���h����C@���3��C@���T  j�CD�<  �Ⱥ0   ��������ļ   [�SV�ڋ��C P�I Pj �CP�CP�CP�CP�CP�CP�P�CLP�CP��������   ^[ÐSVW�؋����������}��eA �_���C@��F�2�����{@���:���ȋ׋�������"����Ë�Rh_^[�       ���   P����Í@ SVW�ڋ����   ���tN����F���   �XK+�|C���   ���N�����   ��uFKu�3�_^[ÐSV�؃��    uZ�Ë�Rd��V��I P���   P�u���V��I P���   P�`����s ��t jj j j j �Ӌ��^���P���   P�y���^[Ë�SVW�؃��    t2���   ��t!�pN��|F3��׋��   �\M�������GNu�Ë�Rl_^[�SV�����    t0���  �؋��������  ��t���    t���   P����^[�U����SVW�E��E��x7 u�E��@t�E��@5u	�E��@6t3����E��}� tE�E����    u�E���RX�E����   ��t$�XK��|C3��֋E����   �L������FKu�E����    t`�E����   :E�tR�E��U����   3�Uh�gA d�0d� j 3ɺ�  �E��C���3�ZYYd��������E�4�U����   ���������_^[YY]ÐS�؋��>�  �Ѕ�t��;�t�@ ���    t;�u��������[ÐU��QSVW�E�3�UhKhA d�0d� 3�Uh:hA d�0d� �E���Q@3�ZYYd�hAhA �`����w_  ��%�����3�ZYYd��� ����U��(I ���  �*���_^[Y]�SVWU���$������   ����   �pN�� ��   �֋��   �6K���؍L$�U+S(�E +C$�C���t$�t$�T$�Ë�Q,�D$P������t<�Ct�{7 u<�C5t6�{7 t$�C8
$t�D$�����P3ɺ
�  ���������uN����u���3ۋÃ�]_^[�SVW����������?  ������;�u3ۃ=|I  t-�|I ;p u#�|I ��T$�G�O����T$3ɋ��������3���t+�G+C$�$�G
+C(�D$���2���P�O����-������_^[�SVW�����؋=�   ti��t.HtR����   ��    �����	��   	�����
r��   ���	�  ������   �Ӌ��n�  ����   �   �C6 ��   �   �֋�������~���   �T$�F�k����T$�̋��������3ɋ��������tf�F   �]�֋�������uP�E��� �����uC�8����  ���<���;�u&�=|I  t�|I ;X uj 3ɺ   �|I �����֋��5�����_^[Ë�SVWU��؋��   ����   �>��������rD����r�Y�^S�FP�� �  WS�����F�e�CD�@�4  P�FP�������   ��:  �4  P�FP�������   �;  �F�&�FP�FP�PU���   P�/����F�	�֋��)���]_^[�SVW��3����������t�FP�N��� �  ��� ����F���_^[ÐU��İSVW�U��E��]��[��u�E�P�E��  P������3�Uh�lA d�1d�!�/* 
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
    { NULL, "credits_text", "Volker Ströbel" },
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
	{ NULL, "credits_text_small", "PlanetPenguin Racer is Copyright © 2005 Volker Stroebel" },
  { NULL, "credits_text_small", "Tux Racer and the Tux Racer Name are Copyright © 2001 Jasmin F. Patry" },
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 �   ��t��  u��  t�$�
�$ ��$f��f   t�̋Ӌ�h  ��d  �<$ t=�(I ;X(u�(I ��  �'�<$u	���&  ��<$u����m�������\  Z[ÐSVWQ�؀�  u+�$ ���"�����N��|F3��׋��[����������t"GNu��$f��n   t�̋Ӌ�p  ��l  �$Z_^[�U����SVW�E�3�Uhh-B d�0d� �E� �E��z�����t&�E��E�f��f   t�M��]��U���h  ��d  �E�,r��t��E�3҉�(  ��E��  3�ZYYd��"����E�3҉�(  �U��(I ��  ���_^[YY]Í@ 3������S�ز�����������[Í@ ��   u'�x7 t�x8 u�.�  ���)A �r^���=�������Í@ U����S�E��(I �|���E��x7 u!�E��x8 t�E���  u�E���  u�2�  ���)A �^�������0����tj j j�0��P��2���2���E���  �j0���E�|I �E�,I �@L�E�,I �U��PL�,I f�@(f�E�3ҡ,I �  3�葿���E�3�Uh�/B d�2d�"�E������3�Uh{/B d�2d�"j j h �  �E��R��P�K2���E�3҉�(  ��Y  ��x| t�E�ǀ(     ��E���(   t�E�������E���(  ��t��E�j j h�  �E��1R��P��1���E��#R�����x/��;�t3��E�3�ZYYd�h�/B �E�������� ����3�ZYYd�h�/B f�U�,I �  �E��K����,I �U�PL�}� t	�E�P�1���E�|I �E���  ��� ��붋E�[��]Í@ S��j j h!�  ���Q��P��0��[ÐU��j SVW3�UhO0B d�0d� �E��U���    �3���U��E��Q,�   3�ZYYd�hV0B �E������ ������_^[Y]� U��QSV��t��������U���3ҋ��8������M  ��Ц@ �����C ��\�@ �����C,��\�@ �����C0j �Q.�����C Ph 0B j V�>+��jZV�n+���C$Vj �0���}� t
d�    ����^[Y]�SV�ڋ��F0�R����F,�J����F �B������  3ҋ��������t���;�����^[Ë�j�u.���j �m.��Ë@,脂��Í@ �@,�@ÐS�؋C@;CHt�CHf�{Z t�ӋC\�SX�C<;CDt�CDf�{R t�ӋCT�SP[Í@ �@,蘁��Í@ S�؋C,�U����C,�x u�(I �x` t�(I �@`��V��[Ë�SVWU��h   j �~.���E8�������I ���|����=I �3��PW�T.���ȋӋ��   C�����u�]_^[Í@ SVW���_4��t)�C���~��~	�CP��+���3�   �������ޅ�u�h   j ��-���W8;�tR�+��_^[Ë�SVW���_43������t;Su��t$�CP�+����u��G4����   ������_^[Í@ SVW����ظ   �m����S4��p�x�C4_^[Í@ �@0�܀��Í@ �@0�@Ð3Ƀ��t�H4��	��t;Qu���u�H8��I��Ë�SVWU������f;s(tsf�s(f��uZT�+���t$�t$��.������tAj W�,�����v%��;�u.�D$P�D$Ph�   W�-����f� ���.��PWj W�-����֋��Z���P�-��YZ]_^[ÐSVW����؅�u��uh   j �,���C8�"�{8����t�֋�������t�ϋ֋������_^[�SV��؅�t�{T u�[ ��ƋST�u ��^[Å�u�������^[���@ ��t�x] t��t�@t3��U��=(I  t
�(I �~  ]� ��U����S�(I h�  �N$��P�-���; t��x@ t�E�P�j*���E�������u΋�  ��[YY]� ��U��SVW�u�]V�EPS�TI P��(������|�=(I  t�֡(I �t  ��_^[]� �@ Q�(I �x uG�=TI  u��#��Pj ��4B Pj�,���TI �=XI  uTj j hh4B h�  j ��"���XI ZÍ@ �=TI  t�TI P�,��3��TI �=XI  tj �XI P�o%��3��XI Í@ ����$   j �D$Pj jH�J,����t�D$�����YZ�3�YZÃ���$   ���D$j �D$Pj jI�,��YZ�SVW���������؄�t3������WV��+����t�����_^[ÐTApplication    U�������SV��t���������U���3ҋ��o�����\�@ ������Cp��\�@ ��������   3��C@3��C`�C<  ��CT�  �CX2   �C\�	  �Cd �C}����A �������sxh|7B �I P�)���Ћ��v����Cx�X�@<JB h   ������P�I P�^"��������P������P�)���������\�J=����t�P�������;���������.�=����t�  ������@P�C&���Cl�������   �����=4I  u���   �C9�C:�}� t
d�    ����^[��]�MAINICON    SV�ڋ��F} ���  3ҋ��Z���Vh ;B ���  �F ��t'�~~ t!�=�I  tj jh�   P�`)���F P�g&���F$��t�����������   �_�����t���h�����^[Í@ S��؋؀{~ �#  Sh�;B �5����C$T�TI P�I P�&����u/�I �@I h0I �(��f��u�,�  ��`)A ��S������j �>'����y�� Pj�/'����y�� Pj j j j �I Pj �Cl�����й  ʔ�TI �:*���C �Cl�?����C~�	   �C �����C$Pj��C P��(���=�I  t���  Pjh�   �C P�2(��j �C P�&����j h0�  S�%��j h �  S��$���=�I  tj h�  S��$����([�S;P(u3ɉH(;P,u3ɉH,�,I ;Q<u�,I 3ۉY<�,I ;Q@u�,I 3ۉY@�,I ;QLu�,I 3ۉYL;P@u3҉P@�,I ����[Í@ U��SVW�]�   jS��%���(I ;B u;j�S��%���t(�(I �x(��t���G��;�t�(I �@p���1y����E�3���_^[]� �@ SVQ�ء(I �x  t|�{t us�C �$��Ph�9B �_$���Cp�x tWj�D$P�b%���$j�D$P�[%���t�$�����Cp�pN�� |%jj j j j �D$P�֋Cp�;y��P�'��N���u��CtZ^[�SV��(I �x  t?�Nt�~t u6�Fp�XK�� |"jj j j j j��ӋFp��x��P��&��K���uދFp�fx��^[Í@ S3ۋ��u�x9 t�K����[Í@ U��E�@��@P�E�@��@P�E�@�� P�E�@��@ P��"���U�R��B]Ë�U����S�E�P�E�@��@ P�"���؋E�@��  Pj j S��"���E�P�E�@��@ P��"��[��]Ë�U����SVW�U��E�3�Uh�@B d�2d�"�E�3҉P�E����   �XK��|2C3��E����   ����w���ȋU��A���t3�ZYYd��  FKuыU��E�������]����=  _��   ��-�X  ���w  ����  ���  H��   �  ����   H�j  ���a  ���$  �_  =�  4��  �������  -Ǯ  �8  H�Y  ���4  �$  -�  �h  ����  H��  ����  ��  �E��@%��  - �  t	-   t��E��!  ��  �E��\  ��  U�����Y��  �E��@(����  �����  �=�I  ��  �=HI  ��  �HI �  �E��@ P�"����tU�����Y�h  U����Y�\  �E�� '   U����Y�G  �E��e  �U��B�4  j j h�  �E��@ P��"��U�K���Y�  U�?���Y�E��@����؋U��B}�E��x t"�E�����j j h �  �E��@ P�"����  �E������j j h�  �E��@ P�"���  �E��x t1�E��^����E����   ��t萰���E�3҉��   U����Y�s  U����Y�E����    u�E��@ 觯���U����   �E��v����?  �u��vV�E��@P�E��� �  S�E�V�h"���U��B�  �E��x �  �k����  �E��X(����  ��$   ��  ���hB��P�&!������  �U��E��@(��$  �v�������  �E��@   �  �E��@(����  �����B������  ���	B��P�� �����m  ����A��P� �����X  �xI  ����؋���A��P�!���E��@P�M��I�  �����S�!���xI �E��@   �  �E�f���    ��   �M��U����   ���   ��   �E�f���    ��   �M��U����   ���   �   �E��@ P��������   �����U�;B ��   3��.�������   P�� ���|�M��I�U�f�R�E��e  �e�E��x u�E��@�p�0�E���  �G�E��@�p�0�E��  �2�E��xu�E����   �U��B��E��@�U����   �U�3���Y3�ZYYd���w����U��E��T  ����_^[YY]Ë@x谙����uh   j �D��Í@ S�؋C P������u5��������C P�  ���   �C ����f���    t�Ӌ��   ���   [ÐSV�؋C P�����tN�C P����	   �C �I������
����,I �p<��t����?��P���f���    t�Ӌ��   ���   ^[Ë�SV�؋C ��t+P�c������t;s tV�J����tV�8����tV�^��^[Í@ SV�� �����؀{~ t h   �D$P�C P����ȋԋ������
�ƋSl�D�����   ^[Í@ SV��؀{~ t�������P�C P�K��^[ÍCl�������^[Í@ SVW����3����   ��tWV�v�������_^[�SVW����3��s(��t:��  u1�,I �z@ t%�,I �R@��  uW��0  P��������_^[Ë�SVWU���3ۋ}��   |G��  ?�����u6�} �F(��t;�0  u�;>�����EP�EP�E �  PW������t���]_^[ÐSVWU����3ۋu`��t�׋Ƌ���   ��t����  ��]_^[ÐSV�����3�jj j j �D$P�������   ��|$tx�$ f���    t�̍T$���   ���   �T$���y�����uM�<$ uG�T$��������u8�T$���������u)�T$���n�����u�D$P�����D$P�����F|�Ã� ^[�S�؋��F�����u�[�S�؋��6�����u���o  [ÐU��S�؀{~ u�C ��t@�URj h �  P����-���   �o���   �����U��U�P�Ћ��   �yn��[]� U��SVW�؀{~ u�C ��t]�URjh �  P�J���J���   �xO��|<G3��֋��   ��n���;Uu!�U;Pu�   �=����֋��   �Hn���FOu�_^[]� �@ �= I  t� I �U����SVW�M���؋��P�E��E��U��3�Uh�EB d�0d� ��3ҋE��0�V$3�ZYYd��������E�3҉�E���������������{( u�E���A �������t�u����;���s(_^[YY]Í@ U��Q�E��E��@3�UhFFB d�2d�"���A �v#���E��@(��t!�U��z: t��8����E��,����E��x| t�3�ZYYd�hMFB �E��@ �������Y]�j �u���SV����y����tj j j�j��P����C���Xc@ ������tE�C����c@ ������uCf���    t��B���ȋ֋��   ���   �"��B���Ћ��   ^[���B��P�B��Z�C��^[�U����SVW����������E�3��:����E�3�UhEGB d�0d� �EPWV�C P�q���E�3�ZYYd�hLGB �E�赧���E�P����������E�_^[��]� U��j j SVW���3�Uh�GB d�0d� j�U��������E�����P�V�E�� ����E���GB ������E������Ћ�Y�1���3�ZYYd�h�GB �E��   �S����������_^[YY]�����   .   SVW����$����3��D$f���    t�D$P�L$�׋��   ���   �؀|$ tS�~0 t13ۋF(��t�>9���؋$P��P�F0�����PS��������؋���~~ u�$P��Ph�  �F P�K����YZ_^[Í@ ��f� �Y�����S���Ë�SV��:Sdt3�Cd��t�˲�tI �P$���s`�S<��������C`����3��C`^[ÐU����SVW3҉U��U��3�Uh�IB d�0d� �E�P����E����������t�Ft3��-�����;s,tK�{, t��t	��t;{,uj 3ɺ�  �C,����s,�{, t��t	��t;{,uj 3ɺ�  �C,�g���{d t�C,��t;C`u����  �U���l����E�U��i����U�(I ��   �E�f���    t�M��Ӌ��   ���   �}� t�^��3�ZYYd�h�IB �E�   �0�����n�����_^[��]ÐSVW���,I �E�����K��|C3�j �֡,I �!�����3����FKu�_^[Í@ S�؀=�I  t�������Pjh�   �C P������C P�����tjj �C P���f�����y���[Í@ SV��؋C4�������t"�C4������f���    t�Ӌ��   ���   ^[ÐSVWU�ً�����3   hL4B Wjj �����f�uhf�����Ee�]f�}e u���M  ]_^[�S�؀{e t�ChPj �7���Ce [ÐSVWU������؍T$�E����T$�L$��������D$��m������������t�~] u	����   �d;s@u0�T$�E�:���T$�L$�������t$�t$�CDP�2����u/�C8�$�<$ t�{X��{T���   �$�C8�s@3ɋ׋��������]_^[Í@ S����؋������Cf,ru���   �T/* 
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
    { NULL, "credits_text", "Volker Ströbel" },
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
	{ NULL, "credits_text_small", "PlanetPenguin Racer is Copyright © 2005 Volker Stroebel" },
  { NULL, "credits_text_small", "Tux Racer and the Tux Racer Name are Copyright © 2001 Jasmin F. Patry" },
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ��KA       �    ParentShowHint�A P  �LJA       �   � 	PopupMenu @ ]  ��KA `KA    �   � ShowHint�+A �A ��A       ����� TabOrder @ �  ���A       �     TabStop @ 7  ��IA       �    Visible�@ �  ��  �      �   � OnClick�@ �  ��  �      �   � 
OnDblClick$-A x  �x  �      �   � 
OnDragDrop�,A �  ��  �      �   � 
OnDragOver�-A �  ��  �      �   � 	OnEndDrag�@ �  ��  �      �   �  OnEnter�@ �  ��  �      �   �! OnExitP,A �  ��  �      �   �" 	OnKeyDown�,A �  ��  �      �   �# 
OnKeyPressP,A �  ��  �      �   �$ OnKeyUp�+A `  �`  �      �   �% OnMouseDown,A h  �h  �      �   �& OnMouseMove�+A p  �p  �      �   �' 	OnMouseUpl-A �  ��  �      �   �( OnStartDrag�@ TListBoxStyle       �~B 
lbStandardlbOwnerDrawFixedlbOwnerDrawVariable        �B         �B �B 8  �0A �jA �*@ +@ ĸB H�@ |SA d�@ �A xDA �ZA �HA �A �A �B ��A ��A  �A @CA �CA �IA |�B T�A ��A h|A ��A �^A �eA ��B �dA ��B �eA $�B �lA �~A �A ��B `�B    �+�,���� �B 0�B t�B ��B <�B �B h�B D�B TCustomListBox�@ TCustomListBoxLB �2A 
 StdCtrls  @ �  ���A       �   	 TabStop�         �B             �B 8  LB �jA �*@ +@ ĸB H�@ |SA d�@ �A xDA �ZA �HA �A �A �B ��A ��A  �A @CA �CA �IA |�B T�A ��A h|A ��A �^A �eA ��B �dA ��B �eA $�B �lA �~A �A ��B `�B TListBox�@ TListBox��B �B 3 StdCtrls* +A ;  ��DA       �    
 Align��A   �h�B       �    BorderStyle��A H  ��KA �KA    �  � Color4@  �H�B       �     Columns @ �  �āA �A    �   � Ctl3D�)A N  �N  �      ����� 
DragCursorT+A <  �8  �      �     DragMode @ 8  �JA       �    Enabled @  ��B       �    ExtendedSelect��A D  �LKA XKA    �   � Font @  ��B       �     IntegralHeight4@ �B \�B       �   � 
ItemHeight4�@ �  ���B       �   � Items @  ���B       �     MultiSelect @ :  ��KA       �     ParentColor @ �  ��A       �    ParentCtl3D @ 9  �hKA       �    
ParentFont @ ^  ��KA       �    ParentShowHint�A P  �LJA       �   � 	PopupMenu @ ]  ��KA `KA    �   � ShowHint @  �$�B       �     Sorted�~B  �8�B       �     Style�+A �A ��A       �����  TabOrder @ �  ���A       �   	 TabStop4@  �t�B       �    ! TabWidth @ 7  ��IA       �   " Visible�@ �  ��  �      �   �# OnClick�@ �  ��  �      �   �$ 
OnDblClick$-A x  �x  �      �   �% 
OnDragDrop�,A �  ��  �      �   �& 
OnDragOver�dB ( �( �      �   �' 
OnDrawItem�-A �  ��  �      �   �( 	OnEndDrag�@ �  ��  �      �   �) OnEnter�@ �  ��  �      �   �* OnExitP,A �  ��  �      �   �+ 	OnKeyDown�,A �  ��  �      �   �, 
OnKeyPressP,A �  ��  �      �   �- OnKeyUp�dB 0 �0 �      �   �. OnMeasureItem�+A `  �`  �      �   �/ OnMouseDown,A h  �h  �      �   �0 OnMouseMove�+A p  �p  �      �   �1 	OnMouseUpl-A �  ��  �      �   �2 OnStartDrag����t�xP u	�@ ��u�3ҋ��        �B             ԇB    ��@ ,,@ �*@ +@ L+@ H�@ ��@ к@ ��B T�B ̾@ @�B �B ��@ T�B �B ȹ@ �@ �@  �B p�B ��@ о@ �@ d�B ��@ H�@ ��@ �@ p�@ ��@ TMemoStrings�@ TMemoStringsl�B 4�@   StdCtrls  �        ��B             ��B    ��@ ,,@ �*@ +@ L+@ H�@ ��@ к@ H�B ,�B ��B (�@ p�@ ��B P�@ �B МB �@ �@ t�B T�B ��@ о@ �@ �B ��@ H�@ ��@ �@ p�@ ��@ TComboBoxStrings�@ TComboBoxStrings<�B 4�@   StdCtrls  �        ��B             |�B    ��@ ,,@ �*@ +@ L+@ H�@ ��@ к@ D�B (�B ��B (�@ p�@ ضB P�@ ��B ��B �@ �@ ��B ��B ��@ о@ �@ <�B ��@ H�@ ��@ �@ p�@ ��@ TListBoxStringsTListBoxStrings�B 4�@   StdCtrls  ��SV��t����n����ڋ�3ҋ��U���f��B fF4f�F4�A   ���˻���   ���߻��Ɔ�   Ɔ�   ��t
d�    ����^[�  @  SV��؋֋��U���^[Ë�U��j SVW�����3�Uh��B d�0d� �U��Ë�QXf�� t-�}� t���    t�E��8&u�E��x u�E���B �������    uf�� �SD���   �$���{8 u���   �@�  ������PW�E�訪��P�E��c���P���   �K%��P����3�ZYYd�h�B �E�������b�����_^[Y]�  ����       SV����؋��   ���@  ��u+�SH�F�	��3ҋF����T$�Ë�Q,�T$��� ����F����ԋË�Q,3����   f�EhI f��@3����   fE`I �ԋ������� ^[Í@ SV������C��   ���    t}�ԋË�Q,j �>������֋��   ��$��3����   f�ElI f��@�ԋ��5���3ҋ��   �$��Vj ������s$���   u	�C,+D$��D$P�D$P�K(�֋Ë�SL��^[Í@ :��   t���   ��RD�:��   t���   �9�����@4@��4Ë����   ��t���u��Í@ :��   t���   ��RD�SV�ڋ�������:�t+��tf���B ��f#F4f�F4�f���B fF4f�F4�Ƌ�RD^[� @   :��   t���   �����SVW�ً���ˋ֋��l�����u;��   u3����   _^[Í@ S�؋Ë�RD���k���[ÐS�؋���������W���[ÐU��j SVW���3�UhэB d�0d� ���    tH�{8 tB���    t9�U���������U�f�F��g����t���   ���?�����t�Ë�Rx�F   3�ZYYd�h؍B �E��0����鎢����_^[Y]Ë�SV��t����B����ڋ�3ҋ�������=�I  tf�|�B f�F4�
f���B f�F4�y   ��获���   ��袷������u���3ҋ�褽��Ɔ   Ɔ  Ɔ  Ɔ  ���  ��t
d�    ����^[�  �  �  :�  t��  �=  �S��:�   t��   ���$  �������[Í@ :�  t��  �����:�  t��  �����SV���;��   t&���   ��������tj Vh�   ���f���P����^[Ð:�  t��  �]����SV����	  ���������tj j h�   ���#���P��������Ë�^[Í@ SV�ڋ���������tj 3���Ph�   �������P����^[È�	  ^[Ë�U��j SVW��3�Uh�B d�0d� :�  tI��  ���C�����t8j 3���  Ph�   ������P�F����U���舺���E��8����Ћ��S���3�ZYYd�h&�B �E�������@�����_^[Y]�SV�ڋ�:�  t*��  ���������tj 3���Ph�   ������P�����^[ÐSQ��j �D$Ph�   �������P�����$Z[ÐSV���VVh�   �������P����^[Í@ S����؍D$P�D$Ph�   ������P�f����D$+$YZ[Í@ SV�����؍D$P�D$Ph�   ���}���P�3���4$�t$�D$P�D$Ph�   ���Z���P����YZ^[Í@ S��hD�B ���=���P����[�     S��j j h  ������P�����[ÐS��j�j h�   ������P����[ÐU����SVW�����E�P�E�Ph�   �������P�����]�+]��ǋ�3��(�����ta���m���@�����E�3�Uh�B d�0d� �E������ȋU����R�����E��M�����8���3�ZYYd�h�B �E�������I�����_^[��]�SV��؋�����Pj h�   ���8���P�����^[Í@ SV��؋֋��E���� �B �֋�������F�   3Ҋ�   �XI ��   ���pI 3Ҋ�  �xI 3Ҋ�  ��I 3Ҋ�  ��I 3Ҋ�  ��I �F�=�I  t ���    t��   u�f����N   ^[� EDIT    U��Q�E��E�ƀ  3�UhG�B d�0d� �E��y���3�ZYYd�hN�B �E�ƀ   �������j �E����   Ph�   �E�����P������E���	  �E�������E���   t j �E���  Ph�   �E������P�����E��"   Y]Í@ S�؋��n�����	  ���]���[Í@ S�؀�   t ��   uf��B fC4f�C4���   [�f��B ��f#C4f�C4[�     SVW�Đ��j �������TS�T����FD�H��PS��������D$8PS�8���WS����Sj �����=�I  t"���    t�   ��   j�4�������%�$�D$8;�~��j��������y����Ë؋T$8Ӌ�������p_^[Ðf��   t�ȋЋ�  ��  ÐSV�ڋ����t-�  u(���)�����u&��=� I u�CP�%�����u3��C�Ӌ�����^[�V���Ƌ�Q��=�I  t)j����8���P�~����uj jh�   ������P�����^Ë�SV��؀=�I  t��   u���Q����������֋��Q���^[Ë�S�؋�������C5t�Ct�Cu���]���[Í@ Sf�z u��   u	f����%���[Í@ SV��؀�   t �C6uj����z���P������u���U����֋��,���^[ÐSV���Ƌ�Q����������tj����=���P�����t��f���谕��^[ÐSV��j j h�   �C����P�������j j ��HPh�   �C�����P����Ph�   �C�����P������uN��^[Ë�SVW�����P����؋�f�  ��PVh�   �C����P�Z����ȋԋ��������   _^[ÐSVW�����j Vh�   �C�o���P�%�������|Jj Vh�   �C�S���P�	����PVh�   �C�<���P�����������Pj h�   �C����P�����_^[ÐU����SVW3ۉ]��M�����3�UhO�B d�0d� ����   j Wh�   �F�����P�����؅�|�E��h�B �U������Ij OWh�   �F����P�`����؅�|_j Sh�   �F����P�D�����tE؍E��M��h�B 躝��SSh�   �F�c���P�����E�����Pj h�   �F�E���P�����3�ZYYd�hV�B �E�貛���������_^[YY]�   ����   
      SVW����j Wh�   �C�����P��������|gj GWh�   �C�����P��������}j Vh�   �C����P�n������WVh�   �C����P�U�����I Pj h�   �C����P�:���_^[Ë��@�$���Í@ SV�ڋ�j ��4��Pj�F�R���P��������t�F蹵��^[Ë�SV��؋֋C�0���^[ÐU��j j SVW���3�Uh�B d�0d� �U����~����E��������C褰��;�u�U��C�����U��E��ޜ��tA�E�Pj j�C�����P�v�����u���  ���)A �y����D���j 3ɺ�  �C����3�ZYYd�h�B �E������E��������X�����_^[YY]Í@ U��QSV��t��������U���3ҋ�������   ���p����Y   ��脫��3ҋ��+���ƃ  ƃ  ��l�B 轐������  �^�}� t
d�    ����^[Y]Í@ SV�ڋ���  诐��3ҋ�誾����t��诒����^[Ë�SV��؋֋��y����F3Ҋ�  ���I ��#�3Ҋ�  ��I 3Ҋ�  ��I �F^[�SVW�ڋ��C P�I Pj �CP�CP�CP�CP�CP�CPhh�B �CLP�CP���������   �Pj jW�����_^[�      S�؋��^j��3ҋ������[Í@ :�  t��  �����Ë�  ��Q�:�  t��  ������:�  t��  ������SV��؋֋Ë�Q���   t�N��f���   u�f�^[Ë��G���Ë�SV��؋֋��}����>u��   u� ^[ÐS��j j hF  �C�8���P�����[�SVW�����P����؋�PVhH  �C����P��������u3��ԋϑ�`�����   _^[Ë�SV���j VhP  �C�����P����^[ÐSVW�����WVhQ  �C����P�j���_^[Ë�SV��؋��[���Pj hC  �C����P�A�����}���  ��`)A �D�������^[�SVW����؋�����PVhJ  �C�I���P�������}���  ��`)A �����͓��_^[ÐSV���j VhD  �C����P�����^[ÐU��j SVW��3�UhםB d�0d� �U��C����j j hK  �C�����P�����U��C������C��RP3�ZYYd�hޝB �E��*����鈒����_^[Y]�SV�ڋ�j ��4��Pj�F�z���P�0�������t�F����^[Ë�U��QSV��t��������U���3ҋ�謺���=�I  tf� �B f�C4�
f��B f�C4��   ���Q����   ���e�������8���3ҋ��g�����<�B 裌�������   �^���*A �����   ǃ     ƃ   ShȣB �T����  Sh��B �T����   ǃ     �}� t
d�    ����^[Y]�  �  �  SV�ڋ����������t�Ƌ�Rh��   ��T����  �T����   �������   ������0  ����3ҋ��������t��������^[�S�غ��B ���ݪ�����   ��R8[�    S��j j hW  �������P���������[Í@ SV�ڋ�j 3���PhO  ������P�a���^[Ë�S��j j hG  ������P�C���[ÐSV���j VhN  ���o���P�%���^[Ë�SV���;�  t&��  ���������tj VhA  ���6���P�����^[Ð:�  t��  �-����S��:�  t2��  ��uf���B ��f#C4f�C4�f���B fC4f�C4�������[�    ��  �����s��  �j 3ɺT  ����Å�~��  Ð���   ��Q�SV��؋֋������D�B �֋��g����F@  3Ҋ�  ��I 3Ҋ�  ��I �F�=�I  t���    t�N   ^[�  COMBOBOX    SVW�؋��L�����  ��~���   ~��   j VhA  �������P謿����0  ��t �Ћ��   ��Q��0  讉��3���0  3���  3���  ��  ,s~j������P��������ti��  u5����  j�W�ʽ����(  ��   Pj���  P藿��jV蟽��������  j�W蕽����$  ��  Pj���  P�b����=�I  t��   tj jh�   ��  P�þ��_^[Í@ SV�؋��   ��R��~!��Ц@ 虈������0  ���   �Ƌ�Q���y���^[Ë�V���Ƌ�Q��F@P������P����^Í@ SV�����؀�  s/�C ���   ���P�T$�Ë�Q,�D$P�FP誻���F   ��^[ÐV���Ƌ�Q�^�V���Ƌ�Q�^�V���Ƌ�Q�^�SV��؋֋Ë�Q����6�����t�N^[Í@ ;Btj 3ɺO  �]����SV��؀=�I  t��������֋��I���^[Ë�V����覴���=�I  u��  s�Ƌ�RD^Í@ SVW�����؁>  u�֋Ë�Q@�   ��$  P�֋�  �Ë8�W|���0tZ-�  t��ul�{<ufT�����L$�ԋ��*����D$�$�D$�D$��萾��Pj h  ��  P����3ҋ�藫����=�I  tj jh�   ��  P跼����_^[�SV����(  P��  �Ƌ�S|^[ÐU����SVW�ً�E�3�UhF�B d�0d� �=  O��   =�   $��  ����   H��   ��|��  �  -   ��   H�"  H��   ��  = �  *��  -  ��   -�   �  ���  ��  -�  ��  ����  ����  �  �E��P���U��u������  3�ZYYd���  �E��@6 �l  3�ZYYd��  �E�;�  �P  �֋E��3������>  3�ZYYd��  �֋E��������t3�ZYYd��l  f�~f��t
f���  �E���������   3ҋE�����3�ZYYd��1  �֋E���������   3�ZYYd��  �΋U��(I �ʤ���   �E���������   �F�E��F
�E�j�E�P�E������PS����f�E�f�Ff�E�f�F
�֋E���Q@3�ZYYd��   �E��������tB�F   3�ZYYd��   �E��@t%�F����3�ZYYd��j�֋E���Q@3�ZYYd��V�FP�FP�PS�EP萶���F�>  u�E��@4�t�E�f����Ą��3�ZYYd���%����U��(I � ����/���_^[��]� ��SVW����CuM�>��  t��  u;���������u0�{<u*�֋���������   ���B 
C6�C6�֋��s�����   �-  t{��Ѓ�rD����rD�   �CD�@����P�FP�E������   �:��������P�FP�������   �\����F�a�=�I  uO��  sF�C ���   �7����F�<�֋��e�����u/f�~f��tf��u���&�����t3ҋ��=����	�֋��2���_^[�     SV�؋��   ��R����  ;�~����}�   h�   ���������C0��P�C,Pj j j ������P�ݸ��j_j j j j j ������P�ĸ��^[ÐU��j SVW��3�Uh/�B d�0d� �B����   �$�(�B �B ©B H�B ��B �B X�B �B h�B ��f���譂����   ��f���蝂���   Ɔ-   ��f���膂����������-   ��   j j j�������P�#�����,   uoj j hO  ������P�����W�������ЍM����   ��S�U����͠����f���������f��������Ɔ,  Ɔ-  �Ɔ,   Ɔ-  3�ZYYd�h6�B �E��҉����0�����_^[Y]�f��6   t�ȋЋ�8  ��4  ÐU����SVW3ۉ]��}�   ���3�Uh��B d�0d� f��F   t�E�P�EP�΋Ӌ�H  ��D  �6�U���   �B���M�֋��   �0�V�E�P�U����M�   �v��3�ZYYd�h��B �E�������k�����_^[��]� f��>   t�ȋЋ�@  ��<  ÐSf��N   tQ�؋ʋЋ�P  ��L  [�SVWUQ���z�G�$�W��   �N���VD��   �������   ��   ����� |,�$t&��   �@�  �������   �@�  ������o��|�$P�O�ՋƋ���   ��W��   � ���$t�GP�GP�h���3ҋ�   ���Z]_^[ÐSVW���z��  �G��  u�O�W�Ƌ���   _^[Ë�SV��؀{<u0��  u'j�h����S,+��F;�~�Ë�Rx3ҋ�苣��^[Ë֋�������訟����t���I��;�  t	3ҋ�蜟��^[ÐSV��؋-  t��t	-�  t;�B�Cu<���j�����u1ƃ�   ������P膴��ƃ�    ���D�����t�	���    u	�֋�谼��^[ÐSV��t��������ڋ�3ҋ������f�d�B f�F4�K   ���{����   ��菘������b�����t
d�    ����^[�   �   S�؋��H����t��  ��(  ���C���[ÐSVWU�ڋ���u�����t:��tf� �3�j��������P�
�������;�tjWh�   ������P�[���]_^[Ë�S�؈�   ��� �����t���H����  R3ɺ�  �ʣ��[�SV��؋֋�聴���D�B �֋��7���3���   ���I 	F^[�  BUTTON  S�؋��R�����   ��  [�Sf�z u	f����}��[ÐSV���f�~u	��   uf�~u;��   t2�F�F���ܮB :�u ���1�����t��f����:}���F   ^[Ë֋�����^[�    U��j SVW���3�UhR�B d�0d� �U����|����U�f�F�\F����t���������t��f�����|���F   �	�֋��\���3�ZYYd�hY�B �E�评���������_^[Y]ÐSVW��؋~�Ǻ�mB �*|����t;�����  ���   ��  ��  �Ë�Q|�֋������_^[Í@ SV��t����r}���ڋ�3ҋ������a   ���ݕ���   ��������������f��B f�F4Ɔ   Ɔ   ��t
d�    ����^[� �   ��  ��rt ��t$�)��   t	��Q   ���H   �3��@   ò�8   Í@ ��  ��Ð:�   t��   ����Ä�t��	   �3��   �S��:�  t9��  ���h�����tj 3���  Ph�   ������P�k�����f����,{��[Ë�SV��؋֋�蹱����B �֋��o����F��3Ҋ�   ��I �F^[�BUTTON  S�؋�膲��j 3���  Ph�   ���?���P�����[Í@ SVW��؀��    ��   �=�I  ��   �F P�I Pj �FP�FP�FP�FP�FP�FP�Ph��B �FP�j��������   ���WA��j����   P�������   ��I h�5A j����   P�֯��j j j ���   P�L����	�֋������_^[� BUTTON  致��Ë�U��j SVW���3�Uh��B d�0d� �U����T����U�f�F�4C����t-��������t"�Ë�Rx���������t�Ë�R|�F   �	�֋��&���3�ZYYd�h��B �E��y������}����_^[Y]Í@ f�z u��R|Í@ SV��؀��    t�=�I  u������P�W����֋Ë�Q�^[Í@ SV��t����Fz���ڋ�3ҋ������q   ��豒���   ���Œ��f�4�B f�F4Ɔ   ��t
d�    ����^[� �   :�   t��   �A����U��QSVW�E�@��@ ��tI�U�E��E��v�����N��|4F3��׋E��2����؋E;X�t�ú�yB �x����t	3ҋ��   GNu�_^[Y]ÐU��QS�ډE��E�:�  tV�E���  �ӋE�������E��0�����t j �E���  Ph�   �E��z���P�0�����tU�>���Y�E�f�����w��[Y]ÐSV��؋֋��q����X�B �֋��'����F��3Ҋ�   ��I �F^[�BUTTON  S�؋��>���j 3���  Ph�   �������P譬��[Í@ SVW��؀��    ��   �=�I  ��   �F P�I Pj �FP�FP�FP�FP�FP�FP�Ph@�B �FP�"��������   ���>��j����   P�Ū�����   ��I h�5A j����   P莬��j j j ���   P�����	�֋�譯��_^[� BUTTON  �?���Ë�U��j SVW���3�Uh��B d�0d� �U��������U�f�F��?����t���]�����t�Ë�Rx�F   �	�֋������3�ZYYd�hŵB �E��C~����z����_^[Y]ÐSf�Rf��rf��t�������[�f����v��[ÐSV��؀��    t�=�I  u���e���P�����֋Ë�Q�^[Í@ S��j j h�  �C�<���P����[�SVW�����P����؋�PVh�  �C����P�ɪ����}��  ��У@ ������z���ԋϑ�Q~����   _^[Í@ SV���j Vh�  �C�����P�|������u��  ��У@ �~����Iz��^[Ë�SVW�����WVh�  �C����P�>���_^[Ë�SV��؋��/���Pj h�  �C�_���P������}���  ��`)A ������y��^[�SVW����؋�����PVh�  �C����P�ө����}���  ��`)A ������y��_^[ÐSV���j Vh�  �C�����P蘩��^[ÐS��j j h�  �C�����P�z���[�SV�ڋ�j ��4��Pj�F����P�X�������t�F�	���^[Ë�U��QSV��t����.u���U���3ҋ��Ԡ���=�I  tf���B f�C4�
f���B f�C4�y   ���y����a   ��荍������`���3ҋ�菓�����B ��r�������   �^���*A ��������  �Ӌ��ω��ǃ     ƃ   ƃ  �}� t
d�    ����^[Y]à   �   SV�ڋ���  �r�����   �xr����  �mr��3ҋ��h�����t���mt����^[Ë��   ��R8�SV�؃�   ~&j �C,��  ƃ����Ph�  ���;���P����^[Ë�S��;�  t4��   t��u��  ���������  ��������t������[Í@ S��j j h�  �������P菧��[ÐS��j j h�  ������P�s���[ÐSV��؋�����;�tj Vh�  ������P�J���^[Í@ :�  t��  艬���:�  t��  �u����SV����؋�  ���������t"��   u��P3ɺ�  ��莗���t$+t$�ƃ�^[Ë�;�  t��~��  ����Å�}3�;�  t��  �����Ë�:�  t��  �����SV���j Vh�  ������P�u������u��  ��P�@ �w����Bv�������^[ÐSVW�ً��V3���Ph�  ���y���P�/���@u��  ��P�@ �3�����u��_^[Ë�:�  t��  �U����:�  t��  �A����S��j j h�  ������P�ϥ��[Ð:�   t��   �����SV��؋������;�tj Vh�  �������P蒥��^[Í@ ���   ��Q�SVWU��؋�<$���ً��t$�t$�T$ �Ƌ�Q,�D$ P������tL���^����苆�   ��R��;�~.�D$P�ͺ�  ���ĕ���t$�t$�D$P认����uE;�҄�t����Ń�(]_^[�SVW���ڋ����   ��R��t;�~W�˺�  ���m����3;�u!W��I��  ���W����G+GPj W�$������3ɺ   �l��_^[�SV��؋֋������|�B �֋�裥���I ��   t�I �V��A 0 3Ɋ�  ��I 3Ɋ�  �I 3Ɋ�  �3���  �I ��   �����$I 3���   �XI ��   �����,I �V�=�I  t ���    t��   u�f����N   ^[� LISTBOX SVW�؋s,�{0������jWV�C(P�C$Pj �������P������   t��  Pjh�  ������P�_������4�����  ��t:�Ћ��   ��Q��   ���|�����$  ��������  �@m��3���  _^[�SV�؋��   ��R��~;��Ц@ ��l������  ���   �Ƌ�Q���������   ��� �����$  ��賦��^[�SVW����CuF�>��  t��  u4��������u)�{<u#�֋��s�����u�ܾB 
C6�C6�֋��cm���	�֋��ܪ��_^[�   SVWU�������f�G��5���؀~<uF��   t=��t��t3�ԋG裣���Ա���������|�Ջ��q�����t3ҋ�贐���+�׋��-����~<u��   t
��u��u	3ҋ�臐��YZ]_^[�Sf�Rf��tf��t�f����rl��[�f����gl��[ÐU���p���SVW�}����E�+�  �E��E�������E��E�   �E�   3��E��E�@��@�E�������E�������E��E�,�  ��r����E܍E��E��E�   ��[����E�3���#����؍E�P�E�@��@P������@0�E���@,�E�;u���   �]�����   ��R;�}����   �Ӌ�Q�E��E��E����  �E��E��Eԉ]��U؋�k���E��P��p���P�M���3�����VW��p����}Ĺ   �_^�U��Xk��u�C����   ��R;�}	;u��a���_^[��]Í@ U�����U��E��E��x t	U����Y��U��E������YY]Í@ S�؋��B����������[ÐS������D$  T謝���L$�ԋ������D$�b����D$3��D$3��D$�T$�Ë�Q��D$  �T$�Ë�Q��� [Í@ U����SVW3ۉ]��}�   ���3�UhO�B d�0d� f��*   t�E�P�EP�΋Ӌ�,  ��(  �E�U���  ��������   ��R;�}(�M�֋��   �0�V�E�P�U����M�  ����3�ZYYd�hV�B �E��q����n����_^[��]� �Sf��2   tQ�؋ʋЋ�4  ��0  [�SVWUQ���z�G�$�W��  �����VD��  �������   ��  ����� |,�$t&��  �@�  ��h�����  �@�  ��}����o��|�$P�O�ՋƋ�S|��W��  ������$t�GP�GP�+���3ҋ�  �v���Z]_^[�SV���B��  �P��  u�H�P�Ƌ���   ^[�SV��؀=�I  t��   u�������֋��T���^[ÐSV��؅�t"���  � ,/t,-t�ƹ��B ���r��^[ËƋ���p��^[�����   \   SV��؊D3�P�$�����t���q��;�}�   ^[ø   ^[ÐSVW����   ;�|;�u3���Ӌ������;�}�_^[Í@ U����SVW3ۉ]��M�����3�Uh��B d�0d� ����  �؅�u�E��ϋ��Lq��� �E�P��I�   ����r���U��E����*q��3�ZYYd�h��B �E��Uo����k����_^[YY]Ë�SVWU����؅�tT�   ��������F���p��;�u�|3�:t���r  � ,/t,-u�ŋϋ��p���Sh4�B W�ź   �q���	�ŋ��mo��]_^[�����   \   U��j j SVW���3�Uh��B d�0d� �U����  �E�P�U����|  �E�Z�[�����3�ZYYd�h��B �E��   �n�����j�����_^[YY]Ë�3��   �SVWUQ�$�����o������|Y�,/t,-uO�F,/t,-uD�   3�;�|4�D�,/t,-uE��}"C;�|�D�,/t�,-t���Ӌ�������;�}̋�H�^��|�,/t,-u�<$ t�   �B3��>��~8�   ��������C;�|%�|�:u�<$ t;�~�,/t,-u�C����3�Z]_^[Í@ SVWU����$�����������|$����n����_;�|3�D�,/t,-u�<$ t����|$C��Ӌ�����؋�H�D$;�}͋�YZ]_^[ÐSVW�����P�������T�D$Ph   ���)p��P�����Å�~��   }�T$�ǋ��m���	�ǋ��Ym����  _^[Í@ SVWU��3���n��������"�����C;�|�|�.u��C��Ӌ��Z����;�}��]_^[ÐSVW����3ҋ��������W�κ   ����o��_^[Í@ SVW������������؅�u	���1l���W�˺   ���o��_^[�SVW�������V����؅�u	���l���W�����Ӌ��ho��_^[�SVW���ز���`�����W�V�������Bo��_^[Ë�SVW���ز���8�����W�κ   ���o��_^[Í@ S�؅�u3�[Ë���l���PS�D���[Ë�SVWU�����$3��D$��tB�<$ t<������l���4;�v,���t!�Ӌ$�p�����t	��+�@�D$W�������G;�wԋD$YZ]_^[ÐSVW��؃=� I tRj*赖����tG�Ƌ��k����al�����   ;�|5��D��,s���n���D� C���������;�}��	�֋��M���_^[ÐSVWU��؋��l�����   ;�|:\.�u����Ջ��S����;�}�3�]_^[Ë�SVW��؋Ƌ���j�����k�����   ;�|$��|�/u���~m���D�\��������;�}ܻ   �2��|�\u��<\u��~�S�ƹ   �m�����������؋�Zk��;�|�_^[Ë�S�����u3�[�P腓���:�u�[ÐSVW�������&k�����K��~�PV�c���� ,/t�,-t���k��;�u�ǋ��j���W�˺   ����l��_^[�SVWU���������������j�����K;�}�PV����� ,/t�,-t���j��;�u�ŋ��i���U�˺   ���l��]_^[Í@ �%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ���%�7I ������Ë�����Ë��%8I ���%8I ���%8I ���%8I ���%$8I ���% 8I ���%8I ���%8I ��U��j SVW��3�Uh��B d�0d� �U���������E��nk��P�<�����3�ZYYd�h��B �E��h����wd������_^[Y]ÐS�؋��6kK 25
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ntimeClass��                        @C    �@ ,,@ �*@ +@ LXD TPSRuntimeClassImporterTPSExportedVar      |@     TSpecialProc      |@    ��SI�� |�
I���u�[Í@ SI�� |�
�� I���u�[ÐSI�� |�
 I���u�[Í@ U��SV��I�� |��"�:�t�E�  �I���u�E� ^[]� �U��SV�uI�� |�:
t� �	I���u��^[]� ��SV�ًȃ��   �����#��^[ÐSV�ڋ��Ë��%����ù   �   �6���������~��8 t��������Ћù   ����������~������|� t�^[Í@ U��S��f�EP�u�u���[��[]� �@ U����SVW3ۉ]���ډE��E����3�UhUC d�0d� �E��U����+ЍM�� �����U��ƋM����3�ZYYd�h\C �E��   ������
����_^[YY]ÐU����SVW3ۉ]���ډE��E����3�Uh�C d�0d� �E�������+ЍM��0����U��ƋM����3�ZYYd�h�C �E��   �T��������_^[YY]ÐU����SVW3ۉ]���ډE��E��/��3�UhEC d�0d� �E��e����+ЍM�� ����M��ƋU����3�ZYYd�hLC �E��   ����������_^[YY]ÐU��j j SVW���3�Uh~ C d�0d� �Ƌ�����   3��   ��|8�'u(��u��֋ϸ� C �^��G�֋ϸ� C �O�����}��|8� sZ�U���D8��b����M��E��� C �����ƹ   ��������t3ۋ֋ϸ� C � ��G�֋ϋE������E��_������u��֋ϸ� C �������G��:��;��C�����t�ƺ� C �*���> u�ƺ� C �1��3�ZYYd�h� C �E��   ����������_^[YY]�����   '   ����   #   ����   ''  SVW�����ǋ������������~$�   �T��� sW��I�   ������CHu�_^[ÐU�������SVW3ۉ������]����U����E����3�Uhh#C d�0d� �}� ��  ��������a	����������������  �U���#C �����؅�u�U��E�������E�����4������P��I�   �E������������U�������E��˺   �����t
���U���y���؅�u�Ǻ�#C �����o  �� ���  �$��!C �"C "C 1"C \"C u"C �"C �"C �"C �"C �"C �"C �"C �"C �Ӌ��oz���������  �Ӌ��Zz��������������������Ǻ�#C �i����   �Ӌ��/z���Ћϋ�x����   �Ӌ��|������<$����@����   h�#C �������Ӌ���{��������h�#C �Ǻ   �x���   �Ǻ�#C �����s�Ӌ��y�����*�Ǻ�#C ����X�Ǻ�#C ����J�Ǻ�#C ����<��u�Ǻ$C �~���*�E��T�����)�����������k��������������3�ZYYd�ho#C ����������E��   ����������_^[��]�  ����   .   ����   Unknown Identifier  ����   #   ����   '   ����   [Set]   ����   [Method]    ����	   [Variant]   ����	   [Unknown]   ����   nil SVWQ��$�؋C�@<u$��8��u�ƺt$C �[���,�΋ǋ$�����<u�ƺ�$C �;����ƺ�$C �-��Z_^[�����   nil ����	   Interface   ����   Invalid Type    U�������SVW3ۉ]��M����3�Uh)C d�0d� �;��u�E��()C ����  �C�x��   3�Uh{%C d�2d�"f�f��	u�E��4)C ����Yf��u�E��P)C �l���Df��u�E�������E��U������'f= u�E�������E��U�������
�E����z��3�ZYYd��  �	��   Xc@ �%C �ËK�E��`)C �"�������S  �{3��G���5  �$��%C �(C M&C p&C �&C �&C �&C �&C 'C F'C o'C �'C U(C �'C �(C �'C �(C �(C �(C �'C �(C �(C ,&C �'C �(C �(C �(C �(C �U��� ������M��E��t)C �x���  �������� �o���������E�����  �������� �L���������E�����h  �������� �)���������E��o���E  �������� ����������E��L���"  �������� �����������E��*���   �������� �����������E������  �� ����<$��������))���������E������  �� ����<$�������� )���������E�����  �f�PR�p�0��������(���������E�����b  �U��� �����Q  �E����Z���E��U������5  �E��2���C�xu�G��׋� �A9  ��N��|FF3��E��8 t�E���)C �v���������Ë��w �������M�3������U��E��L��GNu�h�)C �E��0h�)C �E��   �����   �E�����C�@�@��N��|FF3��E��8 t�E���)C �����������Ë��| �������M�3�� ����U��E�����GNu�h�)C �E��0h�)C �E��   �e���*�E���)C �����M��Ë��$�����E���)C ���3�ZYYd�h)C �E���
����N����_^[��]� ����   nil ����   Variant(IDispatch)  ����   Null    ����   Exception:  ����   Proc:   ����   ,   ����   [   ����   ]   ����   (   ����   )   ����   Nil ����	   [Invalid]   U��j SVW�ً�3�Uh%,C d�2d�"�����  �$�*C i*C z*C �*C �*C �*C �*C �*C �*C �*C +C  +C 1+C B+C S+C d+C u+C �+C �+C �+C �+C �+C �+C �+C �+C �+C �ú<,C �
���  �U����,����M��úP,C �-���w  �úh,C ��	���f  �ú�,C ��	���U  �ú�,C ��	���D  �ú�,C �	���3  �ú�,C �	���"  �ú�,C �	���  �ú-C �	���   �ú(-C �y	����   �úD-C �h	����   �ú\-C �W	����   �úx-C �F	���   �ú�-C �5	���   �ú�-C �$	���   �ú�-C �	���   �ú�-C �	���{�ú�-C �����m�ú.C �����_�ú4.C �����Q�úT.C �����C�úp.C ����5�Ëκ�.C ��	���%�ú�.C �����Ë������ú�.C ���3�ZYYd�h,,C �E�������:����_^[Y]�  ����   No Error    ����   Cannot Import   ����   Invalid Type    ����   Internal error  ����   Invalid Header  ����   Invalid Opcode  ����   Invalid Opcode Parameter    ����   no Main Proc    ����   Out of Global Vars range    ����   Out of Proc Range   ����   Out Of Range    ����   Out Of Stack Range  ����   Type Mismatch   ����   Unexpected End Of File  ����   Version error   ����   divide by Zero  ����
   Math error  ����   Could not call proc ����   Out of Record Fields Range  ����   Null Pointer Exception  ����   Null variant error  ����   Out Of Memory   ����   Exception:  ����   Interface not supported ����   Unknown error   3ҊP����   ���.C �$�/C 
	        ~/C ./C 6/C >/C F/C N/C V/C ^/C f/C n/C v/C �@   ��@   ��@   ��@   ��@   ��@   ��@   ��@   ��@
   ��@   �3҉P�SVW��t���������ڋ�3ҋ������β��C ��3 �G��t
d�    ����_^[ÐSV�ڋ��F����3ҋ��m�����t��������^[ÐSVW�؋�������s�VJ�������p�{�WJ��������s_^[�SV��t���������ڋ�3ҋ��E������C �}����F���C �n����F��t
d�    ����^[�SV�ڋ��F������F�����3ҋ��9�����t���������^[ÐSVWUQ���3��F���S  �$��0C 
2C *1C *1C 21C 21C <1C <1C <1C �1C �1C <1C �1C <1C E1C <1C 
2C �1C 
2C *1C 
2C 
2C `1C �1C �1C {1C <1C <1C � ��   f�  ��   3����   3���ËЃ�3ɉ
��3҉�   3���ËЃ�3ɉ
��3҉�   3���C�   3���C�z3���Cf�C�m�ú�@ �[���_�F�hM��|TE�$    �F�$��������׋������_�$Mu��+�Ƌx�hM��|E�׋�����_Mu���V��3��*���Z]_^[�U����SVW�U��؋E��@�������  ��>2C �$�O2C           4C o2C {2C �2C �2C �2C �3C �3C ������  ���tP� �P3���v  3�Uh�2C d�0d� �ú�@ ���3�ZYYd��O  ����������@  �ËЃ��: �0  �����J  3���  ����  ���E�E�8 ��   �E���E�8 ��   �E��p�F�E��E���E�E� �E��E���E�F�,r,t,u�}�O��|G�֋E������E�E��E�Ou�U��U�����������3���~�E��@�xO��|pG�E�    �E��@�U��������F�,r,	t,u	�֋��H���^�E�Ou��3�E��p�V�����r
��	t��u�xO��|G�֋�����^Ou�_^[��]Í@ SV�؋C�,������Ӌ��y�����^[�SV��؅�t(�F�,r,t,t,u	�֋������V������^[Í@ SV�؋C�����������ƃ���������^[�S�؅�t6��P�����r��t
��t��u�Ӄ���Z�����P��������[ÐSV���^K�� |�Ӌ����������K���u���h���^[ÐSVW���Lu[�GD���G@���G<�pN�� |A�֋G<�	1 �؋�@�,r,t,t,u�Ã��������Ã���L���N���u�_^[Ë�SVW���F(�XK�� |�F(�@���   �����K���u�F,�XK�� |&�F,�@�<���ȋW���Ѻ   ������K���uڋF0�XK�� |�F0�@���XC ���K���u�F8�XK�� |�F8�@�����Q�K���u�F8���F<���FD���F4�XK�� |�F4�@���&���K���u�F4���FL �F,���F0���F(���F`����_^[ÐSVW��t��������ڋ�3ҋ��������C �<����G$���C �-����G(�G ���C �����G,���C �����G4���C ������G8��`C �Q. �G<��`C �B. �G@3��GH�GL ���C ������Gl���C �����G0���C �����Gh�����  �ϲ��C �L������w�F �Ƌ��ϲ��C �/������w�F�Ƌ��ϲ��C �������w �F�Ƌ��G �W�P��`C �- �GD��t
d�    ����_^[Í@ SVWU�ڋ��Ƌ�R�F�����F�����F �����Fp��t�u����Fh�hM�� |�Fh�@���xC ���M���u�F,�H����F0�@����F@�8����FD�0����F<�(����F8� ����F4�����Fh�����Fl�hM�� |.�Fl�@�<���   t
�׋���  �  �������M���uҋFl������F(������F$�hM�� |�ՋF$��������M���u�F$����3ҋ�������t��������]_^[Ë�U��QSVW�؉St�Kx�E�C|���   �U�U����Cp��t�Q����E�Cp�} ��   �C(�@H�� ��   �E��U��C(�������CD�@�V;�vH��+�O�CD�@- Gu��> u�   ��������C(�p����CL�   ��CP�@�CT�CP�@�CX�F�C`�F���t�C\�F�����X�F���t���t�C\�F�����?�F���t�C\�F�����+�   �������C(������M��}���5����{L t�CL_^[Y]� �U����SVW3ɉM�U���3�Uh�:C d�0d� �E�U�������E�������E��_K�� |5�G�4���   ;E�u�ƋU�3ɊA�\���u�Ӌ��A������K���u�3�3�ZYYd�h�:C �E��l�������������_^[��]Í@ U����SVW3ۉ]�M��ډE�3�Uh�;C d�0d� �; ��   �E�U��R�����E�P�U���;C �h�����I�   �E��t����E�������E��E��@h�pN�� |C�֋E��@h�����؃{ t�C;E�u �C�U��>���u�K�U��E����t��EN���u�3��;�ӋE��@l������u3��&��  �M��Q��  �M��Q��  �U��B�3�ZYYd�h�;C �E��K������������_^[��]�  ����   :   U��j SVW���ڋ�3�Uhc<C d�0d� �U���������  �D����؋ËU���   ������E��������   ��  3���  �E��  �E��  �ӋFl�K���3�ZYYd�hj<C �E���������������_^[Y]� U��SV�ڋ��E�@�������U�R��;�|"�E�@��U�R���֋������EX���3�^[]�U����SVW3��E�3�Uh[BC d�0d� �E�@P�E��   ����Y��t�E�@�@��u����U�R+B�;E�}�E�@�@��
��  3��!  �E��U��u����E�@P�E������U��.���Y��u�E�@�@��
�  3���  �E�@P�E�   �����Y��u�E�@�@��
�u  3��  �E�@��% ���F�U������F�N����F�}�O����  G�E�@P�E��   ����Y��t�E�@�@�@4�@;E�w�E�@�@��
�  3��?  �E�@�@�@4�U������Ћ��$ �؋�@���`  ��A>C �$�Z>C  
   
      	�AC �>C �>C �>C 7?C �?C @C Z@C �@C �@C AC �E�@P��P�C�����Y���  �E�@�@��
�M  �������3��  �E�@P�C�   ����Y����  �E�@�@��
�  ������3��I  �E�@P�C�   �d���Y����  �E�@�@��
��  ���X���3��  �E�@�@�@\���U�R�R�;BX|�E�@�@��
�  ������3���  �E�@�@�@\�U�R�R�RT�� �C�E�@�@�@\�  �E�@�@�@\���U�R�R�;BX|�E�@�@��
�/  ������3��f  �E�@�@�@\�U�R�R�RT�� �C��u
3��C3��C�E�@�@�@\�  �E�@P�C�   �A���Y���v  �E�@�@��
�  ���5���3���  �E�@P�C�   ����Y���;  �E�@�@��
�y  �������3��  �E�@P�C�
   �����Y���   �E�@�@��
�>  ������3��u  �E�@P�C�   ����Y����   �E�@�@��
�  ������3��:  �E�@P�E��   �U���Y��u�E�@�@��
��  ���M���3��  �E�@�@�@\�C�U��J����E�@P�C������U�����Y��u<�E�@�@��
�z  �������3��   �E�@�@���\  �������3��   O�	����F�?����E��E�@�@�@$�XK�� |j�E�@�@�@$�@���@;E�uJ�E�@�@�@$�@���@�V�t���u+�E�@�@�@$�@�<��E�@�@�΋V�W��u3��K���u��3�ZYYd�hbBC �E��������������_^[��]ÐU����S�E��EP�E��   �����Y��u�E�@��
�g  3���]�K��|CU����Y��u3��Ku�[YY]ÐU��QSVW��3��F�XK��|&C�E�    �׋F�����U��F�����x�E�Ku�_^[Y]�U����SVW��E�x�O����  G�EP�E��   �D���Y��u�E�@���  3��k  �E��t
�E��m����E� 3��E����|  ��xCC �$��CC      �HC �CC �CC �DC 6EC �EC 4FC /GC �GC �E�H����C �������E��F�E�@�@4�������  �E�H���PC �������EP�E��   �i���Y��t	�}��   ~���8����E�@����  3��  �F�U��e����EP�F������U��!���Y��u��������E�@���  3��A  �E��F�E�@�@4�������m  �E�H����C ��������EP�E��   �����Y��t	�}��   ~�������E�@���+  3���  �F�U������EP�F�Q����U��y���Y��u���Q����E�@����  3��  �E��F�E�@�@4���g�����  �E�H����C �<������EP�F�   ����Y��u��������E�@���  3��9  �E��F�E�@�@4�������e  �E�H����C ��������EP�E��   ����Y��u�������E�@���,  3���  �}�   ~���m����E�@���  3��  �ƋU�P�V���P�@t�@�E��F�E�@�@4���i�����  �E�H���HC �>������EP�E��   ����Y��u��������E�@���
  3��;  �E�@�@4�@;E�w��������E�@���b
  3��  �E�@�@4�U�������F�EP�E��   ����Y��u�������E�@���
  3���  �}����~���^����E�@����	  3��  �E�F�E��F�E�@�@4���n�����  �E�H����C �C������EP�E��   � ���Y��u��������E�@���	  3��@  �E�@�@4�@;E�w��������E�@���g	  3��  �E��F�E�@�@4�U�������F�E�@�@4��������,  �E�H����C �C������EP�E��   ����Y��t�}� u���R����E�@����  3��  �}� ��   �EP�E��   �?���Y��u�������E�@���  3��_  �E�@�@4�@;E�w��������E�@��
�  3��3  �E�@�@4�U��"����ЋF������M�}� �{�����������u�������E�@���8  3���   �E��F�E�@�@4�������3ۋE�@���
  �   �}� ��   �EP�E��   �^���Y��u�E�@����  3��   �}�   @~�E�@���  3��k�F�U��P����EP�F������U�����Y��u�E�@���  3��6�F�z����F�Ƌ��E�x�|�EP�F�����Y��u3��O�k�����_^[��]Í@ U�������SVW3��E�3�UhNC d�0d� ��E�@�H���
  @�E�EP�E��   �o���Y��u�E�@����  3���  �E��r  �E�H����C �� ���EP�E��   �'���Y��u��������E�@���  3��  3ҊU��E��*����EP�E�����3ҊU������Y��u�������E�@���W  3��J  �F�U������E�$<ur�EP�E�   ����Y��t�E�@������U+B�;E�}���a����E�@����  3���  �E��U������EP�E��"����U��J���Y�F�U������������V��   �����������E�@�΋8�W���[  �~ t�N�E�@���6 ��N�E�@���$ �������3��`  �E�H���4C � ���EP�E�   ����Y��u�������E�@���(  3��  �EP�E�   ����Y��u���\����E�@����  3���  �}� |+�E�@��S���;E�~�E�@��C����U�U�;�|�}� u�������E�@���  3��  �E���������G�E�@��U�V�M������E�G�E��0  �EP�E�   �����Y��u�������E�@���G  3��:  �}�   @~�������E�@���#  3��  �G�U������EP�G�I����U��q���Y��u���I����E�@����  3���   �EP�E�   �@���Y��u�������E�@���  3��   �}�   @~��������E�@���  3��   �G�U��!����EP�G�����U������Y��u�������E�@���P  3��F�G�D����G�E�t�EP�F����Y��u���{���3���E�@�@8�������M������3�ZYYd�hNC �E��������O�������_^[��]�U����SVW��E�p�N����  F�EP�E�   �(���Y��u�E�@���  3��g  �E�@�;E�w�E�@���  3��H  �E�@�@4�@�U����E�@�@<�P ��u�E�@���K  3��  �E���   �EP�E��   ����Y��u�E�@���  3���   �XC �   �����E�3�Uh�OC d�0d� �}��ǋU������EP�������U��D���Y��u&�XC �E��R����E�@���  3�3�ZYYd��q�E�� �����U��B�E�@�@<�@�U��B�E�@�@0�U��
���3�ZYYd��-������XC �E�������E�@���I  3������������N�o�����_^[��]Í@ U����S�U��E�E��R3��E�3�U�Eغ   �r���Y��u��E���   �   �}�IFPSt��E���   �   �E܃���}��E��   �zU����Y��u
�E��R�eU�^���Y��u
�E��R�PU����Y��u
�E��R�;�U�R8�R�E�;�w���t��E��[   �E��R��E�U�PH�E��@L���[��]ÀxLu�@LÐSV��؋C\���;KX�KT����E���s\�^[�3�^[�j 3����  Ë�SV��؋΋S���  ^[Í@ QR�L$��@��
  ZÐU��j SVW��3�Uh�RC d�0d� �zu'�Ã�����t��u��RC ��Xc@ �$:������3��B����   ���QC �$��QC            bRC �QC �QC �QC �QC �QC �QC  RC RC 3�����   ����   ����   ����   ��|��x3�����p�������-   uA�U��������E��L���Hu�U��������E�� ���9��RC ��Xc@ �L9��������!���2��������RC ��Xc@ �)9������3�ZYYd�h�RC �E��s�������������_^[Y]�   ����   Type Mismatch   U��zu'�Ѓ��� ��t��u�SC ��Xc@ �8���4����R��u�U���SC ��Xc@ �8������]�    ����   Type Mismatch   S����؀zu'�Ã�����t��u��TC ��Xc@ �B8������3��B����   ��zSC �$��SC  	             
YTC �SC �SC �SC �SC TC TC TC TC %TC <TC LTC 3���D$�D$�<$��   ��D$�D$�<$��   ��D$�D$�<$��r��<$��j��<$��b��<$��Z��<$��R��<$��J��$�C�D$f�Cf�D$�3�-�TC �+���<$��#��������<$����TC ��Xc@ �27�������,$��[� ����   Type Mismatch   ,e�X���?  S���؀zu'�Ã�����t��u�VC ��Xc@ ��6���M���3��B���  ���TC �$�UC  	             
�UC 3UC NUC hUC �UC �UC �UC �UC �UC �UC �UC �UC 3���D$�D$�(VC �<$��   ��D$�D$�(VC �<$��   ��D$�D$�(VC �<$��   ��(VC �<$��w��(VC �<$��i��(VC �<$��[��(VC �<$��M��(VC �<$��?�+�(VC �<$��1��$�C�D$�#�������<$���VC ��Xc@ �5�������,$��[� ����   Type Mismatch    @FS�؀zu'�Ã�����t��u��VC ��Xc@ �E5�������3��B��wp��sVC �$��VC            �VC �VC �VC �VC �VC �VC �VC �VC �VC 3��[��[��[��[Ë[Ë[�3��[Ë�����[ù�VC ��Xc@ �4���0���[�  ����   Type Mismatch   SV��؀zu'�Ã�����t��u��WC ��Xc@ �f4�������3��B��wt��RWC �$�eWC               �WC }WC �WC �WC �WC �WC �Ɗ����^[ËƊ�~���^[ËƋ����^[ËƋ�����^[ËƋ��2���^[ù�WC ��Xc@ ��3���M���^[�  ����   Type Mismatch   U��QSVW�M��؅�t��u�E��  �U  �zu�Ã�����t��u�E��  �3  3��B���  �$�BXC ZYC �XC �XC �XC �XC �XC �XC  YC YC YC ZYC ZYC ZYC ZYC ZYC ZYC &YC ZYC �XC ZYC ZYC �XC ZYC ZYC YC �E��   �E��   f�Ef��   f�Ef��   �E��ËЃ�3ɉ
��3҉�u�E��n�E��g�E��`�E���X�E���P�E�hYC �;��B�E�;��:3�UhHYC d�0d� �ËU�����3�ZYYd���#����E��  �4�����E��  _^[Y]�  @FU��QSVW�M��؅�t��u�E��  ��   �zu�Ã�����t��u�E��  �   3��B�������   ���YC �$��YC              \ZC �YC �YC ZC ZC (ZC �m���e�m���]�m�lZC �;��O�E��E�Cf�Ef�C�:3�UhJZC d�0d� �m������3�ZYYd���!����E��  �2�����E��  _^[Y]�    @FU��QSVW�M��؅�t��u�E��  �U  �zu�Ã�����t��u�E��  �3  3��B���  �$��ZC �[C *[C 4[C >[C J[C o[C v[C �[C �[C �[C �[C �[C �[C �[C �[C �[C �[C �[C }[C �[C �[C V[C �[C �[C �[C �E��   �E��   f�Ef��   f�Ef��   �E��ËЃ�3ɉ
��3҉�u�E��n�E��g�E��`�E���X�E���P�E��[C �;��B�E�;��:3�Uh�[C d�0d� �ËU�b���3�ZYYd�������E��  ������E��  _^[Y]�  @FU��QSVW�M��؅�t��u�E��  �r�zu�Ã�����t��u�E��  �S�B,
t,t�@�ËU�����:3�Uhf\C d�0d� �ËU����3�ZYYd�������E��  ������E��  _^[Y]� ��SVWU�����T$�$�E�xO��|4G3��E�������؋E�������P�T$ӋD$ù   �   FOuϰYZ]_^[�U��QSVW���UhaC d�5    d�%    �E�@����  �$�]C �`C �]C �]C �]C �]C �]C �]C �]C ^C 0^C �^C =_C _C �_C �]C �`C �^C �`C �]C �`C �`C �]C �^C �_C \^C �]C �_C ��O���r  G��CFOu��c  ��O���X  Gf�f�����Ou��C  ��O���8  G������������������Ou��  ��O���  G������Ou���  ��O����  G���F�C����Ou���  ��O����  G���F�Cf�Ff�C��
��
Ou��  ��O����  G���F�C����Ou��  ��O���t  G�Ë��C�������Ou��\  ��O���Q  G�Ë��������Ou��9  �E�@�E���O���%  G�E�@P�E�H�֋��������u3�d�    ���  ]�u�Ou���  ��O����  G����t���� ����Ou���  �E�@�E���O����  G�M�֋��*�����u3�d�    ���  ]�u�Ou��~  �E�@�E���O���j  G�ӋƋM�����]�u�Ou��O  ��O���D  G���t
P� �P3������tP� �P����Ou��  �Ã��8 u<�ƃ��8 u2��O����   G������������3������Ou���   ��O����   G�Ã��8 t�Ã�����������tt�ƃ��8 u��փ���˃��� �Ӄ���e�ƃ�� ������ƃ�� �Ӄ���Ã��    �Ã�� P�   ���&�����u$3�d�    ���T3���Ã�3҉�Ã�3҉����O�K����3�d�    ���!d�    ����`���3��u�����n����_^[Y]� �zt�\aC ��Xc@ �]*���������u3�Ã�� �   ����   Invalid array   SVWU���$����}t�\cC ��Xc@ �*��������������D$�E�@�D$�|$ u
�<$ ��  �|$ ��   ����8��   �<$�\$K+�|3C�E�P�����r��t
��t��u��L$��ђ�����GKu΋����<$ �$�T$����+���3���  �$�T$�����(���������$������|$�$K+���   C��T$��U����GKu��   �<$ u.������u�T$�T$���������~�3���   �$�l$���s������   ���$����|$ t6�D$;$~�EP��L$���������EP��L$��������Ƌ��������|$�$K+�|C��T$��U�Y���GKu��]_^[� ����   Invalid array   SV����ڋ��Ӌ��.!���; uS�t$�D$ �T$3ɸ�cC ����YZ^[�  ����   OLE error %.8x  U��j SVW��3�UhdC d�0d� �U��������M���Xc@ �'���*���3�ZYYd�hdC �E��������W�����_^[Y]Í@ ��}����Ë�S����tR��P��tS��P[ÐSV��؋�������sf�	 ��tV��P^[Ë�f�
f��rf��tf�� @t�3��"�R��R����dC ��Xc@ ��&���z����}��������(   Variant does not reference an OLE object    U����SVW3ۉ]����U��E��u3�Uh�oC d�0d� �3�Uh�nC d�1d�!�E�@����	  �$�*eC �nC �eC �eC �eC �eC �fC �gC �hC �iC [kC �lC �mC LmC �nC �lC �nC �mC �nC mC �nC �nC fC 'mC �eC �lC ?nC RnC ;uu�E�H�U���������  3���  �֋��x����U����  �֋��Y����U����  �֋��R����U�f��  �֋��2����U�f��  �~u'�ǃ��0�?��t��u�pC ��Xc@ �o%��������F,t,t�5��U���c  ��U���ǋȃ��	�U�������� ����:  �pC ��Xc@ �%�������  �~u'�ǃ��0�?��t��u�pC ��Xc@ ��$���`���3��F����   ���fC �$��fC            zgC gC gC )gC 6gC CgC OgC [gC igC 3���U���  ��U���  ��U���x  ��U���k  ��U���_  ��U���S  3���U���E  ��� ����U���4  �pC ��Xc@ �$�������  �~u'�ǃ��0�?��t��u�pC ��Xc@ ��#���Z���3��F����   ���gC �$��gC            �hC hC "hC /hC <hC IhC UhC ahC ohC 3���U���  ��U���  ��U���r  ��U���e  ��U���Y  ��U���M  3���U���?  �������U���.  �pC ��Xc@ �#�������  �~u'�ǃ��0�?��t��u�pC ��Xc@ ��"���T���3��F���  ���hC �$��hC  	             
�iC ,iC AiC UiC iiC viC �iC �iC �iC �iC �iC �iC 3���E��E�E����m  ��E��E�E����Y  ��E��E�E����E  ��E����8  ��E����+  ��E����  ��U���  ��E����  �/�E�����  �-$pC �/�ɋE�����  ���^����E�����  �pC ��Xc@ �!���)����  �~u'�ǃ��0�?��t��u�pC ��Xc@ �|!�������3��F���  ��@jC �$�YjC  	             
@kC �jC �jC �jC �jC �jC �jC �jC �jC kC kC .kC 3���E��E�E����  ��E��E�E�����  ��E��E�E�����  ��E�����  ��E�����  ��E�����  ��E����  �E����W�P�  �/�E����  �-$pC �/�ɋE����  ��������E����n  �pC ��Xc@ �K ��������S  �~u'�ǃ��0�?��t��u�pC ��Xc@ � ������3��F���  ���kC �$��kC  	             
�lC �kC lC lC )lC 6lC ClC PlC ]lC jlC �lC �lC 3���E��E�E��8��  ��E��E�E��8��  ��E��E�E��8��  ��E��8��x  ��E��8��k  ��E��8��^  ��E��8��Q  ��E��8��D  �E����W�Pf�Wf�P�*  �-$pC �/�ɋE��8��  �������E��8��  �pC ��Xc@ �����[�����  �֋�������E��8���  �M��֋��"����E��J����U���  �M��֋������U��E��A����  �֋������U���  ;ut3��{  �E�PR�H�׋E������b  �~u1�E�@;Fu&�N�E��U�����FP�N�E�� ���b����+  ;ut!�E�xu�~u�E�@;Ft3��  �EP�   �׋E��$�����   ;ut3���   �EP�   �׋E�� �����   �F�8pC ����u��E��B����   �~u�E���������   ��E���a  �E�u��}�3��E�U��E���  ���o�~u	��U���`3��\�F<u"�E�@�8pC ����u�׋E�������73��3<u)�E�� ��tP� �P�E�3҉��U����tP� �P�3��3ۄ�u
��E��D���3�ZYYd��  颿���������     t�������    �s�����3��C�3�����   �ƺ�C 蔼����t$3�j���GPj �O�W�E��0�V�]����   �ƺ�d@ �`�����t3�V3ɲ�E��b  �4����   �ƺ�e@ �7�����t3�V3ɲ�E��b  �����^�ƺle@ ������t3�V3ɲ�E��^b  ������8��t �ƺXc@ ������tV�N��E��5b  �V3ɲ�E��&b  3�����3�ZYYd�h�oC �E��
�����h�������_^[��]�    ����   Type Mismatch   ,e�X���?  ����	   IDISPATCH   U��SVW�ً��u3��T�LD �`�  ��u3��B�S���  �؅�u3��03�Uh�pC d�0d� �ƋS������3�ZYYd���ͽ��3��������_^[]Í@ U��S����U�R�R��ws�$��pC ?qC �pC �pC qC qC #qC 1qC �U�R�[]ËU�R�[]Ã��U�Rf�[]Ã��U�Rf�[]Ã��U�R�[]Ã��U�R�[]ËE�@������� []Ë�U��Q�   j j Iu��M�SVW��U��E��}�]3�Uh&�C d�0d� �E�3�UhًC d�1d�!���  �$��qC �qC �uC �yC �}C ��C �C ��C -�C �E�@����  �$��qC �uC CrC �rC �rC �rC �rC �rC |tC �tC �tC �tC �uC �uC �uC �tC �uC <uC �uC uC �uC �uC �uC �uC cuC �tC �C<
t<u/�E��U�������E�P�M܋Ӌ������U�X�����E��  �Ӌ������U��;��E��  �Ӌ������U��;��E���  �Ӌ��~����U��;��E���  �Ӌ��X����U��;��E��  �Ӌ��J����U�;�E��  �{u'�ƃ���6��t��u�D�C ��Xc@ �x������3��C���*  �$�>sC atC �sC �sC �sC �sC �sC �sC tC �sC atC atC atC atC atC atC atC AtC atC -tC �E�� 3Ҋ;��E���  �E�� �;��E���  �E�� �;��E���  �E�� �;��E���  �E�� ;�E��  �E�� ;�E��  �U�E��*����������E��  �U�E������������E��n  �E�� 3Ҋ;��E��Z  �E̋U��������E̋������E��:  �D�C ��Xc@ �*��襼���  �Ӌ������E������E��  �Ӌ������E������E���   �Ӌ��s����T�C �E��(������E���   �Ӌ��Q����E��(������E��   �M��Ӌ������U��E�� �}����E��   �E��U�������E�P�M܋Ӌ�������U�X�N����E��_�M�Ӌ��d�  ��u�E� �I�E��U������E��8;]u�E�P�E�H�U����d�����E� ���E��r���3�ZYYd��`  �}� u��E��U���3�ZYYd��C  U�U��E������Y�  �E�@����  �$��uC �yC EvC �vC �vC �vC �vC �vC ~xC �xC �xC �xC �yC �yC �yC �xC �yC >yC �yC yC �yC �yC �yC �yC eyC �xC �C<
t<u/�E��U��賾���E�P�M܋Ӌ������U�X�����E��  �Ӌ������U��;��E��  �Ӌ������U��;��E���  �Ӌ��|����U��;��E���  �Ӌ��V����U��;��E��  �Ӌ��H����U�;�E��  �{u'�ƃ���6��t��u�D�C ��Xc@ �v������3��C���*  �$�@wC cxC �wC �wC �wC �wC �wC �wC xC �wC cxC cxC cxC cxC cxC cxC cxC CxC cxC /xC �E�� 3Ҋ;��E���  �E�� �;��E���  �E�� �;��E���  �E�� �;��E���  �E�� ;�E��  �E�� ;�E��  �U�E��(����������E��  �U�E������������E��n  �E�� 3Ҋ;��E��Z  �E̋U��������E̋������E��:  �D�C ��Xc@ �(��裸���  �Ӌ������E������E��  �Ӌ������T�C �E��(������E���   �Ӌ��i����E������E���   �Ӌ��O����E��(������E��   �M��Ӌ������U��E�� �{����E��   �E��U�������E�P�M܋Ӌ�������U�X�L����E��_�M�Ӌ��b�  ��u�E� �I�E��U������E��8;]u�E�P�E�H�֋E��b�����E� ���E��p���3�ZYYd��^  �}� u��E��S���3�ZYYd��A  U�U��E������Y�   �E�@����  �$��yC d}C GzC �zC �zC �zC �zC  {C �|C �|C �|C �|C d}C d}C d}C �|C d}C =}C d}C }C d}C d}C d}C d}C d}C �|C �C<
t<u/�E��U��豺���E�P�M܋Ӌ������U�X�����E���  �Ӌ������U��;��E���  �Ӌ������U��;��E���  �Ӌ��z����U��;��E��  �Ӌ��T����U��;��E��  �Ӌ��F����U�;�E��{  �{u'�ƃ���6��t��u�D�C ��Xc@ �t������3��C���*  �$�B{C e|C �{C �{C �{C �{C �{C �{C |C �{C e|C e|C e|C e|C e|C e|C e|C E|C e|C 1|C �E�� 3Ҋ;��E���  �E�� �;��E���  �E�� �;��E��  �E�� �;��E��  �E�� ;�E��  �E�� ;�E��  �U�E��&����������E��e  �U�E������������E��J  �E�� 3Ҋ;��E��6  �E̋U��������E̋������E��  �D�C ��Xc@ �&��衴����   �Ӌ������E������E���   �Ӌ������E������E���   �Ӌ��o����E��(������E��   �Ӌ��S����T�C �E��(������E��   �M��Ӌ��
����U��E�� �y����E��j�E��U�������E�P�M܋Ӌ�������U�X�M����E��>�M�Ӌ��c�  ��u�E� �(�E��U������E����E�����3�ZYYd��  �}� u��E��u���3�ZYYd��c  U�U��E�����Y�"  �E�@����  �$��}C B�C %~C y~C �~C �~C �~C �~C ^�C x�C ��C ЀC B�C B�C B�C ЀC B�C �C B�C �C B�C B�C B�C B�C B�C ��C �C<
t<u/�E��U���Ӷ���E�P�M܋Ӌ������U�X�.����E���  �Ӌ�������U��;��E���  �Ӌ������U��;��E���  �Ӌ������U��;��E��  �Ӌ��v����U��;��E��  �Ӌ��h����U�;�E��{  �{u'�ƃ���6��t��u�D�C ��Xc@ �������3��C���*  �$� C C�C lC �C �C �C �C �C �C �C C�C C�C C�C C�C C�C C�C C�C #�C C�C �C �E�� 3Ҋ;��E���  �E�� �;��E���  �E�� �;��E��  �E�� �;��E��  �E�� ;�E��  �E�� ;�E��  �U�E��H����������E��e  �U�E��-����������E��J  �E�� 3Ҋ;��E��6  �E̋U��������E̋�������E��  �D�C ��Xc@ �H���ð����   �Ӌ�������E������E���   �Ӌ������E������E���   �Ӌ������T�C �E��(������E��   �Ӌ��o����E��(������E��   �M��Ӌ��,����U��E�� 蛵���E��j�E��U�������E�P�M܋Ӌ�������U�X�o����E��>�M�Ӌ�腊  ��u�E� �(�E��U�������E����E�����3�ZYYd��  �}� u��E�����3�ZYYd��  U�U��E��2���Y�D
  �E�@���>  �$���C օC I�C ��C ��C тC 3�C J�C ƄC ��C ��C 8�C օC օC օC 8�C օC ��C օC Z�C օC օC �C օC ��C �C *�C �C �{u�E�� ;�E���  �E� ��  �{u�E�� ;�E��  �E� �  �C<
t<u/�E��U��诲���E�P�M܋Ӌ������U�X�
����E��j  �Ӌ������U��;��E��P  �Ӌ������U��;��E��6  �Ӌ��x����U��;��E��  �Ӌ��R����U��;��E��  �E�� ;u6�E��8 u%�փ���E���� ;�u;�u3����E���  �E� ��  �E��  �Ӌ�������U�;�E��  �{u'�ƃ���6��t��u�D�C ��Xc@ �*��襭��3��C���&  ����C �$���C         
 	  ��C ԃC �C ��C �C !�C 1�C A�C \�C w�C ��C �E�� 3Ҋ;��E��  �E�� �;��E���  �E�� �;��E���  �E�� �;��E���  �E�� ;�E��  �E�� ;�E��  �U�E�������������E��  �U�E�������������E��v  �E�� 3Ҋ;��E��b  �E̋U��茼���E̋��r����E��B  �D�C ��Xc@ �����[����'  �Ӌ��]����E������E��  �Ӌ��C����E������E���   �Ӌ��)����E��(������E���   �Ӌ������T�C �E��(������E��   �M��Ӌ�������U��E�� �3����E��   �E��U��詯���E�P�M܋Ӌ������U�X�����E��g�M�Ӌ���  ��u�E� �Q�E��U��m����E��@;]u�E�P�E�H�֋E��N����E�4�E���E� ���E�� ���3�ZYYd��  �}� u��E�����3�ZYYd���  U�U��E�����Y�  �E�@���6  �$�3�C b�C ݆C 1�C K�C e�C �C އC Z�C t�C ��C ̉C b�C b�C b�C ̉C b�C �C b�C �C b�C b�C ��C b�C A�C ��C ��C ��C �{u�E�� ;�E���  �E� �  �{u�E�� ;�E��  �E� �  �C<
t<u/�E��U�������E�P�M܋Ӌ������U�X�v����E��b  �Ӌ������U��;��E��H  �Ӌ�������U��;��E��.  �Ӌ�������U��;��E��  �Ӌ������U��;��E���  �Ӌ������U�;�E���  �E�� ;u6�E��8 u%�փ���E���� ;�u;�t3����E��  �E��  �E� �  �{u'�ƃ���6��t��u�D�C ��Xc@ �������3��C���&  ��&�C �$�<�C         
 	  ?�C h�C |�C ��C ��C ��C ňC ՈC ��C �C �C �E�� 3Ҋ;��E���  �E�� �;��E���  �E�� �;��E���  �E�� �;��E���  �E�� ;�E��  �E�� ;�E��  �U�E��L����������E��  �U�E��1����������E��n  �E�� 3Ҋ;��E��Z  �E̋U��������E̋��޼���E��:  �D�C ��Xc@ �L���ǧ���  �Ӌ�������E������E��  �Ӌ������E������E���   �Ӌ������E��(������E���   �Ӌ��y����T�C �E��(������E��   �M��Ӌ��0����U��E�� 蟬���E��   �E��U�������E�P�M܋Ӌ�������U�X�p����E��_�M�Ӌ�膁  ��u�E� �I�E��U��ٻ���E��8;]u�E�P�E�H�֋E�躒����E� ���E�����3�ZYYd��  �}� u��E��w���3�ZYYd��e  U�U��E�����Y�$  �{uh�U�E��������}� u��E��5���3�ZYYd��#  ;{r��E������E� 3�ZYYd��  �M��֋��<���U�U��E�����Y�   ��E������3�ZYYd���  �E�@,uF�{t�E� �Q�E��@4��d����؅�t�{t�E� �2U�E���ˋE�������U��:���Y���E��y���3�ZYYd��g  �}� u2��E��\���3�ZYYd��J  �E� ��E��A���3�ZYYd��/  3�ZYYd��"  钢���٧����     t�˧����    �^轧��3��F�3ۅ���   �ú�C 脟����t&�E� j��FPj �N�V�E���S�K����   �ú�d@ �N�����t�E� S3ɲ�E��E  � ����   �ú�e@ �#�����t�E� S3ɲ�E��nE  ������b�úle@ �������t�E� S3ɲ�E��FE  �ͤ���:��t �úXc@ �Ϟ����tS�K��E��E  �S3ɲ�E��E  �E� 葤��3�ZYYd�h-�C �E��к���Eܺ   �����E�軺����9����ۊE�_^[��]�    ����   Type Mismatch    @FS�؋�誺���������[Í@ U��Q�   j j Iu��M�SVW����E�3�Uh��C d�0d� 3�UhK�C d�0d� ��E��
�n0  �$���C �C ��C 0�C ��C p�C �C (�C b�C x�C ��C ��C �E�@���k	  �$��C e�C e�C v�C ��C ��C ��C ��C ŐC =�C e�C �C e�C e�C e�C �C e�C �C e�C �C e�C e�C e�C e�C H�C ��C �U������� �	  �U������ ��  �U������f��  �U������f��  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ������<����E�@����   ����C �$��C            ��C 4�C ?�C I�C S�C ]�C f�C o�C z�C 3���=  ��3  ��)  ��  ��  ��  3���  �E܋蠱���E܋��f����E���������  ���C ��Xc@ ������i�����  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �����/����E�@����   ��
�C �$��C            ��C A�C L�C V�C `�C j�C s�C |�C ��C 3���0  ��&  ��  ��  ��	  ��   3����  �E܋蓰���E܋��Y����E��������  ���C ��Xc@ ������\����  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �����"����E�@���  ���C �$�0�C  	             
"�C `�C t�C ��C ��C ��C ��C ��C ʑC ֑C �C ��C 3���E��E�����  ��E��E������  ��E��E������  ������  ������  �����  �����  �����  �/����  ����C �/���-��C �����~  ��E��T����E܋������E��������Z  ���C ��Xc@ �i��������?  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �/���語���E�@���  ����C �$���C  	             
��C ؒC �C ��C �C �C *�C 6�C B�C N�C Z�C v�C 3���E��E�����  ��E��E�����}  ��E��E�����j  �����^  �����R  �����F  �����:  �����.  �/����"  ����C �/���-��C �����  ��E��ܭ���E܋��j����E�螬������  ���C ��Xc@ ������l�����  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �����2����E�@���P  ���C �$� �C  	             
J�C P�C l�C ��C ��C ��C ȔC ܔC �C �C �C &�C 3���E��E����C �.���>��  ��E��E����C �.���>���  ��E��E����C �.���>���  ����C �.���>���  �����C �>��  ����C �.���>��  ����C �.���>��  ����C �.���>��x  �/���C �.���>��d  �.�/���>��V  �.�E��\����E܋�躯���E��J����>��2  ���C ��Xc@ �A���輛���  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ����肛���E�@���&  ����C �$�ЕC  	             
ЖC  �C �C +�C @�C N�C \�C j�C v�C ��C ��C ��C 3���E��E��.���>��f  ��E��E��.���>��Q  ��E��E��.���>��<  ��.���>��.  ��.���>��   ��.���>��  �.��>��  �.��>���   �.�/���>���   �.���C �/���-��C ���>���   �.�E�親���E܋��4����E��h����>��   ���C ��Xc@ �����6����   �MԋU�������Uԋ��z����x�U���*��� �j�M�U���t  ��u3��U�֍E�觰���E܍U�踭���U܋�蒰���4�E;Eu�E�H�׋��9����3����E�葹��3�ZYYd���'  ����&  ��E��r���3�ZYYd���'  �E�@���O	  �$���C ��C �C '�C 8�C J�C \�C i�C v�C ��C �C ��C ��C ��C ��C ��C ��C ��C ��C ��C ��C ��C ��C ��C ݠC �C �U������(��  �U�������(��  �U�������f)��  �U���ؽ��f)�  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ����苘���E�@����   ����C �$���C            N�C �C �C ��C �C �C �C  �C +�C 3��)�!  �)�  �)�  �)�  �)��  �)��  3��)��  �E܋�����E܋�������E��I������  ���C ��Xc@ �=���踗���  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �����~����E�@����   ����C �$�ΙC            [�C �C ��C �C �C �C $�C -�C 8�C 3��)�  �)�
  �)�   �)��  �)��  �)��  3��)��  �E܋�����E܋�贪���E��<�����  ���C ��Xc@ �0���論���  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ������q����E�@���  ��ȚC �$��C  	             
ӛC �C %�C 8�C K�C W�C c�C o�C {�C ��C ��C ��C 3���E��E��.����  ��E��E��.����  ��E��E��.����  ��.���  ��.���  ��.���  ��'���  ��'���  �/�.���~  ����C �/���-��C �����b  ��E�裥���E܋��=����E��e������>  ���C ��Xc@ �����3����#  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �~���������E�@���P  ��@�C �$�Y�C  	             
��C ��C ��C ��C ۜC �C �C �C )�C =�C Q�C _�C 3���E��E����C �.���>��l  ��E��E����C �.���>��Q  ��E��E����C �.���>��6  ����C �.���>��"  ��.���C �>��  ����C �.���>���  ����C �.���>���  ����C �.���>���  �/���C �.���>���  �.�/���>��  �.�E��#����E܋�荧���E������>��  ���C ��Xc@ ����胓���s  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ������I����E�@���  ���C �$�	�C  	             
��C 9�C M�C `�C s�C �C ��C ��C ��C ��C ��C מC 3���E��E��.����  ��E��E��.���  ��E��E��.���  ��.���  ��.���  ��.���z  ��'���n  ��'���b  �/�.���V  ����C �/���-��C �����:  ��E��{����E܋������E��=������  ���C ��Xc@ ����������  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �V����ё���E�@���&  ��h�C �$���C  	             
��C ��C ǟC ܟC �C ��C �C �C '�C 3�C A�C ]�C 3���E��E��.���>��J  ��E��E��.���>��5  ��E��E��.���>��   ��.���>��  ��.���>��  ��.���>���   �.�'�>���   �.�'�>���   �.�/���>���   �.���C �/���-��C ���>��   �.�E�������E܋�菤���E�跟���>��   ���C ��Xc@ �
���腐���x�U��蕰��(�j�M�U����j  ��u3��U�֍E������E܍U��/����U܋�������4�E;Eu�E�H�׋��{���3����E������3�ZYYd��R  ���%  ��E��ݯ��3�ZYYd��3  �E�@���=	  �$�G�C }�C ��C ��C ѡC �C ��C  �C ɥC ;�C ��C }�C }�C }�C }�C }�C }�C *�C }�C }�C }�C }�C }�C }�C `�C E�C �U��胯���.���  �U���d����.���  �U���]���f�.f��  �U���<���f�.f��  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �q��������E�@����   ��M�C �$�`�C            �C ��C ��C ��C ��C ��C ȢC ӢC �C �3Ҋ���  ������  ������  ������  ��/���  ��/���  �3Ҋ���  �E܋�8����E܋������E�蒜����  ���C ��Xc@ ���������t  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �L����Ǎ���E�@����   ��r�C �$���C            *�C ��C ��C ƣC ԣC �C �C ��C �C �3Ҋ����  ������  ������  �����  ��/��  ��/��  �3Ҋ���  �E܋�����E܋������E��m�����j  ���C ��Xc@ �a����܌���O  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �'���袌���E�@���$  ����C �$���C  	             
��C �C ��C �C  �C .�C @�C N�C Z�C f�C t�C ��C 3���E��E��.���>��  ��E��E��.���>��  ��E��E��.���>��t  ��.���>��f  �����C �>��T  ��.���>��F  �.��>��:  �.��>��.  �/�.���>��   �.�/���-��C ���>��
  �.�E�������E܋��n����E������>���  ���C ��Xc@ ������X�����  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ���������E�@���  ���C �$�4�C  	             
 �C d�C x�C ��C ��C ��C ��C ¦C ΦC ڦC �C ��C 3���E��E�����  ��E��E�����	  ��E��E������  ������  ������  ������  ������  �����  �/����  ��/���-��C �����  ��E��V����E܋�������E��������t  ���C ��Xc@ �k��������Y  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �1���謉���E�@���  ����C �$���C  	             
��C ֧C �C ��C �C �C (�C 4�C @�C L�C X�C n�C 3���E��E�����  ��E��E�����  ��E��E�����  �����x  �����l  �����`  �����T  �����H  �/����<  ��/���-��C �����&  ��E������E܋�芜���E�覗�����  ���C ��Xc@ ������t�����  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ �����:����E�@���   ����C �$��C  	             
�C H�C ^�C s�C ��C ��C ��C ��C ��C ʩC ةC �C 3���E��E��.���>��6  ��E��E��.���>��!  ��E��E��.���>��  ��.���>���   ��.���>���   ��.���>���   �.��>���   �.��>���   �.�/���>��   �.�/���-��C ���>��   �.�E��d����E܋��
����E��&����>��   ���C ��Xc@ �y��������j�M�U���ua  ��u3��U�֍E�菝���E܍U�踚���U܋��z����4�E;Eu�E�H�׋��Qr���3����E��y���3�ZYYd���  ����  ��E��Z���3�ZYYd��  �E�@���w	  ��ЪC �$��C  	
             :�C �C 5�C P�C l�C ��C ŬC ��C k�C �C ]�C سC �U������P3��Z�ʙ����	  �U������P�Z�ʙ����	  �U���ޥ��P�Z�ʙ��f���  �U��趪��P�Z�ʙ��f���  �E�xu,�ǃ�� �E�?��t�} u���C ��Xc@ ������_����E�@����   ��ګC �$��C            ��C �C $�C 6�C H�C/* 
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

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        bject   CreateStdAccessibleObject   U�����t�����p���U��E�3ҋE���������A �j���E�3�UhüD d�0d� h�  j �����ЋE������E��R ��y�����U���H  �E��R�   ����U���D  3�ZYYd�hʼD �E��n����s�����\�@ �Zn���U���h  �E�ǀT     �E�ǀX     �E�ƀe  ��E��+����E�ǀx  �����E�ǀ<  �����}� t
d�    ���E���]Ë�U��Q�E��E���|  3�Uh}�D d�0d� �E�� ��3�ZYYd�h��D �E���|  ���r����Y]ÐSV�ً���t��p  ��tP��I 3���p  ��t,�X�����th�D ������P��I ��p  �3���p  ^[�  B u t t o n     S�؋�������������[ÐSVW�ڋ���8  ��t�F)  3���8  ��h  ��t,�pN�� |�֋�h  �����m��N���u苇h  ��l��3ɲ���!���3ҋ��T�����t����n����_^[Í@ U��QSVW�M������]�} u2���   ��R��3Ɋˋ���   ��t��D ��Xc@ ������Or���E�PS�EP�EP�EP�EP�EP�ϲ���  _^[Y]�    ����+   Radio item cannot have disabled child items U��Q�MQj jj j�MQ3ɇ��  ]� �@ U��SVW����M�U��u3�W�]SRQj j�UR����  _^[]� �@ S�؋��j  �{8 t�x t�x u3�[ð[�SVWU�����MO��|�Ջ��5  3ҊP	;�u�xt:�M��}��|-�Ջ��N  ���|�Ջ��  �xt�Ջ��0  ���}�3ۋ�]_^[ÐSVW���؀�l   t]��������tRf�W���  ����|A�Ë�Rx;�<  u	��@   t��@  �����L  �֋��K����֋��  �G   _^[Í@ SVW������l   ��   ��L   ��   j茞��f����   j	�|���f��}3�j�n���f�����@j&�_���f��|j%�S���f��}�3��"j(�A���f��|j'�5���f��}���3۰��t��S��������  �Ћ������#��S���   ��R��3ɋ��  �Ћ��d����׋��۵��_^[Í@ SV��؀�@   tj辝��f��}3������$  �֋�诵��^[�ƀL  ����Í@ S�؋�誶���SD��  ����[Ë�U��SVW�}�u�]��3èu�΋Ӌ�������S�΋�����_^[]� �@ U��QSV�M���؋E�(I ���  ���   ���  ���   ;u�Ƌ��  �	r�����  �q���f���   tP�M��֋��  ���  ^[Y]� �U��SVW���E�@���l   tT�E�P��E�@��`  �x t=���r����   �%��|�&u;�~��<&t�ǹ   ����t��NC��~;�}�_^[]�U����SVW3ɉM̋�E�3�Uh+�D d�0d� �^�C$�E��C(�E���U��E������E��}� ��  �E�P�M̋U��E��P  �E��(r��P�M̋U��E��9  �E���s��P�E���  ����P������E�P�M̋E����   �U��8�W�E���q��P�M̋E����   �U��8�W�E��s��P�E���  �h���P誘���MЋU��E��h����}ЋU��E��  �x t,�U��E��  %�   @�U���H  �M���X  ����EЋE���X  �E��U�+ЋE�E�;���   �MčUЋE������EĉC�EȉC�E���  �D�D �����U�+U�+���y�� JS�K�E���l   t�K��CVW�uЍ{�   �_^�{�M̋E����   �U���S�ŰE��  �o��U�E��  ����Y�E��(I ���   ���  ���   ���  �(I �U����   ǀ�   �D 3��F3�ZYYd�h2�D �E���n����4k����_^[��]�   ����   Wg  ��l   t�z	t3������B�SVWU���؋w�n��|3�Ջ��  �@	��V���q  �x tE��X  ��H  ��F��l   u#�=HI  tj j h)  ��譻��P�c����t�f�׋�����]_^[ÐSVWQ��؋��!����$�̋֋Ë8���   �$PVh�  ���_���P�����$Z_^[ÐS��j j j h�  ���;���P����Ph�  ���(���P�ޚ��[�U����SVW3ۉ]��M��U���3�Uh��D d�0d� ��  �U����t  ��j �E�P�������3�3��*���3��F	�~ t@��X  ��H  ��U��E�~ t�V���������X  ��)E��	��X  )E쀻l   u�E��E� ��l   t�~ u�M�   �E�P�E�P�M��U����   �8�W�E��9n��P�M��U����   �8�W�E���o��P��  �����P�����F8��T  ;F8~�U����F8���U���E�� t�E�� 3�ZYYd�h��D �E��Wl����h����_^[��]�U��SVW����؀} ��   �E�@���  �@�����jjV�Ƙ��j诗��P�E�@���  �%���P�/���WV���^m��P���o��P�E�@���  �����P�F���j�j�V�t���j�]���P�E�@���  �����P�ݔ��WV���m��P����n��P�E�@���  ����P������)WV����l��P���n��P�E�@���  ����P�ɕ��_^[]� ��U��ĄSVW3ۉ]���}��   �U��E�3�Uh��D d�0d� �E���e   t�E���t   u�E��  �E�ƀt  �U��E���
  ���=HI  tj j h)  �E��=���P�����E��3��EԋE��x8 t
� t3����E��E���  �E���l   u(�Et"�E�誷����t�  ��F�����E�  ��)�E��PH�F�����}� t	�E�  ���E��@D�@�E܋E���e   �$  �  ��F�����U��E��
  %�   �E�E�H����   @�E�3ۋU��E���	  �Ӂ��   w�P��   �E���H  �ЋM���X  ������y�� ЋE��X  �U��UċʋE�+���y�� ȉM�U�E�H;�uT�U��E��z	  �x t�E�E�E���  Ph��D �E�P�E���H  ��y�� E��U��X  P�E�P�E�P�͑���E���  Ph��D �E�P�E�P�E�P�E�P詑��C�M������ ��  �U���H  �E���X  ����)U�E��E��UċE�+ЋM�+�D  ��y�� ЉU��E���D  E��E���H  E��E��E���p   ��   �G,ru"3��G��|	I ��   �3��G��|	I ��  �E���M   t�� @  �}� t��   �E���<  ;E�u �E���@   u�E���P  ;E�u��   Sj�E�P���0���P�Z�#Translated by Andreas Tarandi <torandi@gmail.com>
#
#Changelog:
#2007-09-13 - Initial Translation
#2008-01-12 - Corrected two typos

#common strings
pp_translate_string "Back" "Tillbaka"
pp_translate_string "PRESS ANY KEY TO START" "TRYCK PÅ VALFRI KNAPP FÖR ATT BÖRJA"

#game_type_select
pp_translate_string "Enter an event" "Turnering"
pp_translate_string "Practice" "Träning"
pp_translate_string "Configuration" "Inställningar"
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
pp_translate_string "Graphics Configuration" "Grafikinställningar"
pp_translate_string "Language:" "Språk:"
pp_translate_string "Show UI Snow:" "Visa UI snö:"
pp_translate_string "Display FPS:" "Visa FPS:"
pp_translate_string "Display Progress Bar:" "Visa Progress Bar:"
pp_translate_string "Draw Fog:" "Rita dimma:"
pp_translate_string "Reflections:" "Reflektioner:"
pp_translate_string "Shadows:" "Skuggor:"
pp_translate_string "Model:" "Modell:"

#video configuration
pp_translate_string "Video Configuration" "Videoinställningar"
pp_translate_string "Resolution:" "Upplösning:"
pp_translate_string "Bits Per Pixel:" "Bitar per pixel:"
pp_translate_string "Fullscreen:" "Fullskärm:"
pp_translate_string "Experimental (needs restart)" "Experimentel (kräver omstart)"
pp_translate_string "Enable FSAA:" "Aktivera FSAA:"
pp_translate_string "To change the resolution, or switch into fullscreen mode" "För att byta upplösning, eller för att byta"
pp_translate_string "use options.txt, located in the config folder." "till fullskärm, använd options.txt i konfigureringsmappen."

#audio configuration
pp_translate_string "Audio Configuration" "Ljudinställningar"
pp_translate_string "Sound Effects:" "Ljudeffekter:"
pp_translate_string "Music:" "Musik:"
pp_translate_string "(needs restart)" "(kräver omstart)"
pp_translate_string "Disable Audio:" "Stäng av ljud:"
pp_translate_string "Stereo:" "Stereo:"
pp_translate_string "Bits Per Sample:" "Bits Per Sample:"
pp_translate_string "Samples Per Second:" "Samples Per Second:"

#keyboard configuration
pp_translate_string "Keyboard Configuration" "Tangentbordsinställningar"
pp_translate_string "Turn left:" "Sväng vänster:"
pp_translate_string "Turn right:" "Sväng höger:"
pp_translate_string "Paddle:" "Skjut på:"
pp_translate_string "Brake:" "Bromsa:"
pp_translate_string "Jump:" "Hoppa:"
pp_translate_string "Trick:" "Trick:"
pp_translate_string "Reset:" "Återställ:"

#joystick configuration
pp_translate_string "Joystick Configuration" "Joystickinställningar"
pp_translate_string "Enable Joystick" "Aktivera Joystick:"

#race select
pp_translate_string "Race!" "Kör!"
pp_translate_string "Select a race" "Välj ett lopp"
pp_translate_string "Contributed by:" "Skapad av:"
pp_translate_string "Unknown" "Okänd"
pp_translate_string "Time:" "Tid:"
pp_translate_string "Herring:" "Strömmingar:"
pp_translate_string "Score:" "Poäng:"

#event select
pp_translate_string "Continue" "Fortsätt"
pp_translate_string "Select event and cup" "Välj turnering och tävling"
pp_translate_string "Event:" "Turnering:"
pp_translate_string "Cup:" "Cup:"
pp_translate_string "You've won this cup!" "Du har vunnit den här tävlingen!"
pp_translate_string "You must complete this cup next" "Du måste vinna den här tävlingen härnäst"
pp_translate_string "You cannot enter this cup yet" "Du kan inte delta i den här tävlingen ännu"

#event race select
pp_translate_string "You don't have any lives left." "Du har inga liv kvar."
pp_translate_string "Race won! Your result:" "Du vann loppet! Ditt resultat:"
pp_translate_string "Needed to advance:" "Krav för att avancera:"
pp_translate_string "You can't enter this race yet." "Du kan inte köra det här loppet ännu."

#loading
pp_translate_string "Loading, Please Wait..." "Laddar, var vänlig vänta..."

#paused
pp_translate_string "Resume" "Fortsätt"
pp_translate_string "Paused" "Pausad"

#race over
pp_translate_string "Race Over" "Loppet slut"
pp_translate_string "Time: %02d:%02d.%02d" "Tid: %02d:%02d.%02d"
pp_translate_string "Herring: %3d" "Strömmingar: %3d"
pp_translate_string "Score: %6d" "Poäng: %6d"
pp_translate_string "Max speed: %3d km/h" "Maxfart: %3d km/h"
pp_translate_string "Was flying: %.01f %% of time" "Flög: %.01f %% av tiden"
pp_translate_string "Race aborted" "Loppet avbrutet"
pp_translate_string "You beat your best score!" "Du slog rekordet!"
pp_translate_string "Congratulations! You won the event!" "Grattulerar! Du vann turneringen!"
pp_translate_string "Congratulations! You won the cup!" "Grattulerar! Du vann tävlingen!"
pp_translate_string "You advanced to the next race!" "Du avancerade till nästa lopp!"
pp_translate_string "You didn't advance." "Du avancerade inte."

#highscore
pp_translate_string "Highscore" "Highscore"
pp_translate_string "You made it to the %s place in the highscore!" "Du slog dig in på %s platsen på highscoren"
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

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ��E��3�ZYYd�h$�D �E���G����BD������_^[Y]� ��U��   ]� U��   ]� ��l����HI  �=� I u�$�   T�Im����t�|$�HI �Ĕ   Ë��=`I  t�`I j �f��������dI Í@ �=dI  t�R���Ð��	I �j]���=4I  u� I �`I � I ��D �d�������h�D h�D �Tl��P�fl���LI �NotifyWinEvent  user32.dll          ��D         ��D ��D   �0A �jA �*@ +@ \YA H�@ |SA d�@ �A xDA �ZA �HA �A �A t�D ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA ��D �dA �D �eA ,eA �lA �~A �A   ��D TNewProgressBarTNewProgressBarT�D �2A  NewProgressBar h@ �  �L�D       �   �	 Minh@   �`�D       �   �
 Maxh@  �t�D       �     Position�SV��t����>���ڋ�3ҋ��Uj����   ���W��j�q���Ћ��)W��3����   ǆ   d   ��t
d�    ����^[�SV����E����֋��s�����D �֋��rs���N^[�   msctls_progress32   S�؋��t��f���3���s��Pj h  ���D���P��q����  ���-   [Í@ ���   ��  �   Ë���   ��  �   Ë�SVW��؋��   ;�}�����   ;�~����  ���n�����t1j ��   ���   +�Ph��  +�V�j��Ph  ��觑��P�]q��_^[ÐV���Ƌ�Q�^�        ��D         ��D ��D (  �]B ܔB �*@ +@ ��D H�@ |SA d�@ l�B xDA �ZA �HA �A �A ��D ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA �D �B ��D �eA ��B �lA �~A �A  �$�N���D ��D ��D TRichEditViewerTRichEditViewer�D 4^B 4 RichEditViewer  @ $ �L�D       �   3 UseRichEdit���=�	I  u;�lI    h\�D ��h���hI �=hI  u�lI    hl�D ��h���hI ��	I � RICHED20.DLL    RICHED32.DLL    �=�	I  ~!��	I �=�	I  u�hI P��f��3��hI ÐSV��t����z;���ڋ�3ҋ��U���Ɔ$  ��t
d�    ����^[Í@ SV�ڋ�3ҋ�詨����%   tƆ%   �x�����t���];����^[�SV��؀�$   t��%   uƃ%  ������֋�胨����$   t*�=lI u���D �֋��p������D �֋���o��^[ú��D �FL�����^[�RichEdit20A RICHEDIT    /Text   SV�؋��Y������^   ��$   tR�=lI uIj j h;  ��蠎��P�Vn������   Vj hE  ��胎��P�9n��j jh[  ���m���P�#n��^[�S�؀�$   t(��������t�CH转��Pj hC  ���6���P��m��[Ë�SV�ڋ�:�$  t&��$  ���%s����u��%   tƆ%   �����^[ÐU��QSV�]3��E��u�F;�~�؋U����3��)^�E��E�^[Y]� ��U����S�؋E�@��dC���E��E�@��A���E��E��E�3��E��D �E�E�PShI  �E�@��k���P�!m���E�[��]ÐU�����U��E��E���$   u�U��E��qV��3��6h���j h5  �E��!���P��l��U�   �X���Y��tU�   �H���YYY]Í@ ����Ë�S�؋�讃�����{���[ÐS�؋�覄�����g���[ÐU����SVW3ɉM��3�Uh��D d�0d� �B�H��  ��   �x  ��   �P�U��P�U��E���u�}��uj j j���e���P�l����U�+�B�U���xC���E��E�E��E�E���A���E��E�Pj hK  ���#���P��k���ЍE��?C���}� t!jj j �E��A��Ph��D ������P�����3�ZYYd�h��D �E��_>����:����_^[��]�open                             �D    �@ ,,@ �*@ +@ L+@ �D �D �D $�D ,�D TCustomFile�+2���@ �#2���@ �2���@ �2���@ �2���@                         ��D    ��D ,,@ �*@ +@ ,�D ��D ��D �D P�D ��D \�D TFile��                        ��D   h�D ,,@ �*@ +@ ,�D ��D ��D �D P�D ��D \�D TTextFileReader                        0�D    h�D ,,@ �*@ +@ ,�D ��D ��D �D P�D ��D \�D TTextFileWriter                        t�D    Xc@ ,,@ �*@ +@ L+@ 
EFileError�S����؋ԋË�Q�|$ u�D$�u�$�����YZ[ÐU����SVW3ɉM���3�Uh!�D d�0d� �U����|����}� u�E�P�]��E� �U�3ɸ8�D 萀���M���t�D 虓���X�9��3�ZYYd�h(�D �E���;����>8����_^[��]� ����   File I/O error %d   S����`���Ћ��O���[ÐSVW����؋׋΋Ë8�W;�t�&   ��(���_^[Ã��3ɉL$�$�ԋ�QYZË�SV��؋���<��P���>���Ћ�Y��S^[Ë�U��QSV��t����Z4����U���3ҋ��J2���EP�EP�M�֋Ë0�V�C�C��t@u��>����C�}� t
d�    ����^[Y]� �SV�ڋ��~ t	�FP�^��3ҋ��2����t���4����^[Ë�U��SV�ً�j h�   3��Ë��	I Pj 3��E���	I P3��E���	I P���=��P�s^��^[]� �SV�ڋ�3��Cj�CPj �FP��`����;�u�T_����t��q���^[Ë�SV�ڋ��CP�FP�!_����;�u�%_����t��B���^[Í@ SVWQ�����j �D$PWV�CP�X`����u�{ u
��^����mt������$Z_^[ÐSVW�����<$����j �D$P�D$P�CP�H`��@u�^����t������YZ_^[Í@ SQ��3��$j�D$Pj �CP�`��@u�q^����t�����Z[Í@ S�؋CP��_����u��q���[Í@ SVWQ�����j �D$PVW�CP�(`����u��E���;4$t�   �����Z_^[Í@ SV�؋C;Cr'�{ u!�S�   �Ë0�V�C3��C�{ u�C^[Ë�S�؋������C[ÐU����SVW3ɉM��U���3�UhK�D d�0d� �������{ t�}� ��   �&   �� ����|�s��D3,
t
,tF;sr�E��9������+S׍E���<���E��^;���8��+K�C�D�>+���ƉC;Cs��C�C�|u�������C;Cs
�|
u�C�E��U��_8��3�ZYYd�hR�D �E��7����4����_^[YY]Í@ U��Q�M��M��u�Q�MQ�M������Y]� �SVW����؀{ uu�ԋË�Q�<$ u�|$ t[�ĺ   ����ԋË�Q�T$�   �������D$,
t.,t��D$
�T$�   �Ë8�W��
I �   �Ë8�W�C���h8���ȋ֋Ë�S��_^[Ë�U��j SVW���3�Uhq�D d�0d� �֍E��P7���E����D �+8���U����%���3�ZYYd�hx�D �E��6�����2����_^[Y]�  ����   
                          ��D    Xc@ ,,@ �*@ +@ L+@ ECompressError�                        �D    ��D ,,@ �*@ +@ L+@ ECompressDataError�                        L�D    ��D ,,@ �*@ +@ L+@ ECompressInternalError�                        ��D    �@ ,,@ �*@ +@ L+@ � E ��D ��D TCustomDecompressor�)���@ �w)���@                         �D    ��D ,,@ �*@ +@ L+@ � E � E ,E TStoredDecompressor                        P�D   �@ ,,@ �*@ +@ dE TCompressedBlockReader�V3��tI �ƺ   �t	��5 ������Ju�F����   u�^Í@ SVW����؃=pI  u����jhpI ��X���ǅ�t%��f��� 3Ɋf3��ҋ�tI ��3Ӌ�N@��uۋ�_^[�SV��؋Ӌ΃���������^[�U����SV�M��U��}�|]�m���3�;E�}P����t���u=@�T��t���u*�UЃ��}� u��3ɍ�Ӎ���A��u���@;E�|�^[YY]� �@ U��SV��t����,���ڋ�3ҋ��*���E�F�E�F��t
d�    ����^[]� SVWQ�ى$���~/�֋<$�ˋG�W��u�E ���D 觊���"0���+؅��Z_^[� ����   Unexpected end of stream    Í@ U����SVW��t�����+����U���3ҋ���)���s�U��   �Ƌ8�W��u�U�   �Ƌ8�W��t�DE ���D �����/���E�   �M���;E�t�DE ���D �����^/���U�Ƌ��E�U�����U��Ƌ�Q�U��E��Q����~�DE ���D 蠉���/���}� tShdE ��E��C�E��C�C�}� t
d�    ����_^[��]� ����   Compressed block is corrupted   SV����ڋ��F��(���~ t�ԋF���ċV�� ���ԋF��Q3ҋ��(����t����*����YZ^[ÐSVQ�؃{}�DE ���D �̈���G.���Թ   �C�|����k�s��   ~�   �S�΋C�[���)s3��C�s�C�������;$t�DE ���D �l�����-��Z^[�   ����   Compressed block is corrupted   SVWUQ����3��$���~?�~ u�~ t3���)����ߋF;�v�؋ՋF�D���#��^)^�+�$����$Z]_^[�SVW����؋C��t�׋΋�S�%�׋΋��~���;�t�E ���D 衇���-��_^[�����   Compressed block is corrupted   U����SVW3ۉ]��]�M��U����}3�UhE d�0d� ���/������   �%����o���؅�u�E����0���U�����0���i;�t��+΍E���D0���ǋU���0����C��,	s&3����1;E���3Ҋ�M����<����0����뎋Ǻ(E �0��F�;%�x���F�r���3�ZYYd�hE �E�   �/����Q+����_^[��]�   ����   %   U��SVW����؋EP3��Ë�tI ��1���֋������_^[]� SVWQ�����W�t$�T$3ɋ�����Z_^[Í@ SV�ϾtI ���p.������u�^[Ë���E ��Xc@ �����j+��� ����G   The setup files are corrupted. Please obtain a new copy of the program. SVWUQ������P~�ƺ�I �@   �"��t�r����ƃ�@�ËC�S��;�u;�u�;�   t�N����Ã���s�4$�$+Ջ�����;Cu	�$�x� t�"����ϾtI ;,$r�������k�����Ƌϋ��A.��G����u�Z]_^[ÐU��ĜSV�M�����������-�����u#�]��E��E�Pj �\E ��Xc@ �����**��jj j�˲�h�D �����E�3�UhDE d�2d�"�֋E��g����U��@   �E���S��@t�a����E���I �@   �w!��t�H����U��   �E���S��t�.����U��ҋE�;�u��P~	�}��   t�����֋E�������E�����E�3�Uh�E d�0d� �]�ӋM��E��0�V;E�t������ËU��0���3�ZYYd�h�E �E��w����u(�����}� t7��I �,   �E���S��,t������I �	I �   � ��t�n���3�ZYYd�hKE �E��#����(����^[��]�   ����^   Messages file "%s" is missing. Please correct the problem or obtain a new copy of the program.  ���j �D$�D$ �L$�º�E �o��YZ�   ����   0x%.8x  ����Ë�U��SVW3�Uh$	E d�0d� �
I �A��3�ZYYd�h+	E ��;'����_^[]�U��Ġ���SVW����E� 3�Uh�
E d�0d� ��������u3�ZYYd��  jj j�˲�h�D �F����E�3�Uh�
E d�0d� �U��@   �E���S��@uf�}�MZu�}� u�(��3�ZYYd��>  �U�E�������������   �E���S=�   u�������8LE  t��'��3�ZYYd���   �}�w�'��3�ZYYd���   �U��E��\����������   �E��!����������uf������u	�������t�\'��3�ZYYd��   ������Hs�A'��3�ZYYd��}�������H   �E��������������H|-��������4|!���������uV���������   �^�E�3�ZYYd�h�
E �E��r ����x%����3�ZYYd��
�s#���&���E�_^[��]�U����SVW�U����E� �E�P���,����W�CP���؅�~w���
���E�3�Uh�E d�0d� �E�PS�E�PW�P����t/�E�P�E�Ph�E �E�P�P����t�E�U������   ��E�3�ZYYd�h�E �E������$�����=� I t�U����o����E��E�_^[��]�  \   S��̋ڋ��+�����t�T$��T$�S��4[Ð        #Translated by Andreas Tarandi <torandi@gmail.com>
#
#Changelog:
#2007-09-13 - Initial Translation
#2008-01-12 - Corrected two typos

#common strings
pp_translate_string "Back" "Tillbaka"
pp_translate_string "PRESS ANY KEY TO START" "TRYCK PÅ VALFRI KNAPP FÖR ATT BÖRJA"

#game_type_select
pp_translate_string "Enter an event" "Turnering"
pp_translate_string "Practice" "Träning"
pp_translate_string "Configuration" "Inställningar"
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
pp_translate_string "Graphics Configuration" "Grafikinställningar"
pp_translate_string "Language:" "Språk:"
pp_translate_string "Show UI Snow:" "Visa UI snö:"
pp_translate_string "Display FPS:" "Visa FPS:"
pp_translate_string "Display Progress Bar:" "Visa Progress Bar:"
pp_translate_string "Draw Fog:" "Rita dimma:"
pp_translate_string "Reflections:" "Reflektioner:"
pp_translate_string "Shadows:" "Skuggor:"
pp_translate_string "Model:" "Modell:"

#video configuration
pp_translate_string "Video Configuration" "Videoinställningar"
pp_translate_string "Resolution:" "Upplösning:"
pp_translate_string "Bits Per Pixel:" "Bitar per pixel:"
pp_translate_string "Fullscreen:" "Fullskärm:"
pp_translate_string "Experimental (needs restart)" "Experimentel (kräver omstart)"
pp_translate_string "Enable FSAA:" "Aktivera FSAA:"
pp_translate_string "To change the resolution, or switch into fullscreen mode" "För att byta upplösning, eller för att byta"
pp_translate_string "use options.txt, located in the config folder." "till fullskärm, använd options.txt i konfigureringsmappen."

#audio configuration
pp_translate_string "Audio Configuration" "Ljudinställningar"
pp_translate_string "Sound Effects:" "Ljudeffekter:"
pp_translate_string "Music:" "Musik:"
pp_translate_string "(needs restart)" "(kräver omstart)"
pp_translate_string "Disable Audio:" "Stäng av ljud:"
pp_translate_string "Stereo:" "Stereo:"
pp_translate_string "Bits Per Sample:" "Bits Per Sample:"
pp_translate_string "Samples Per Second:" "Samples Per Second:"

#keyboard configuration
pp_translate_string "Keyboard Configuration" "Tangentbordsinställningar"
pp_translate_string "Turn left:" "Sväng vänster:"
pp_translate_string "Turn right:" "Sväng höger:"
pp_translate_string "Paddle:" "Skjut på:"
pp_translate_string "Brake:" "Bromsa:"
pp_translate_string "Jump:" "Hoppa:"
pp_translate_string "Trick:" "Trick:"
pp_translate_string "Reset:" "Återställ:"

#joystick configuration
pp_translate_string "Joystick Configuration" "Joystickinställningar"
pp_translate_string "Enable Joystick" "Aktivera Joystick:"

#race select
pp_translate_string "Race!" "Kör!"
pp_translate_string "Select a race" "Välj ett lopp"
pp_translate_string "Contributed by:" "Skapad av:"
pp_translate_string "Unknown" "Okänd"
pp_translate_string "Time:" "Tid:"
pp_translate_string "Herring:" "Strömmingar:"
pp_translate_string "Score:" "Poäng:"

#event select
pp_translate_string "Continue" "Fortsätt"
pp_translate_string "Select event and cup" "Välj turnering och tävling"
pp_translate_string "Event:" "Turnering:"
pp_translate_string "Cup:" "Cup:"
pp_translate_string "You've won this cup!" "Du har vunnit den här tävlingen!"
pp_translate_string "You must complete this cup next" "Du måste vinna den här tävlingen härnäst"
pp_translate_string "You cannot enter this cup yet" "Du kan inte delta i den här tävlingen ännu"

#event race select
pp_translate_string "You don't have any lives left." "Du har inga liv kvar."
pp_translate_string "Race won! Your result:" "Du vann loppet! Ditt resultat:"
pp_translate_string "Needed to advance:" "Krav för att avancera:"
pp_translate_string "You can't enter this race yet." "Du kan inte köra det här loppet ännu."

#loading
pp_translate_string "Loading, Please Wait..." "Laddar, var vänlig vänta..."

#paused
pp_translate_string "Resume" "Fortsätt"
pp_translate_string "Paused" "Pausad"

#race over
pp_translate_string "Race Over" "Loppet slut"
pp_translate_string "Time: %02d:%02d.%02d" "Tid: %02d:%02d.%02d"
pp_translate_string "Herring: %3d" "Strömmingar: %3d"
pp_translate_string "Score: %6d" "Poäng: %6d"
pp_translate_string "Max speed: %3d km/h" "Maxfart: %3d km/h"
pp_translate_string "Was flying: %.01f %% of time" "Flög: %.01f %% av tiden"
pp_translate_string "Race aborted" "Loppet avbrutet"
pp_translate_string "You beat your best score!" "Du slog rekordet!"
pp_translate_string "Congratulations! You won the event!" "Grattulerar! Du vann turneringen!"
pp_translate_string "Congratulations! You won the cup!" "Grattulerar! Du vann tävlingen!"
pp_translate_string "You advanced to the next race!" "Du avancerade till nästa lopp!"
pp_translate_string "You didn't advance." "Du avancerade inte."

#highscore
pp_translate_string "Highscore" "Highscore"
pp_translate_string "You made it to the %s place in the highscore!" "Du slog dig in på %s platsen på highscoren"
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

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ��n	��@P�E��(��P�E�Pj �E�P�E�P�+���j�E�P�E�Pj �E�P�E�P�+���E�P�L+��3�ZYYd�h�,E �E��   ����E�����E�����E�����������_^[��]�   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ����4   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ����   

    ����   RegCreateKeyEx  U��ļSVW3ɉM܉MЉM��M��M��U�3�Uh�/E d�2d�"�E� j j�U�R�0E �  ��®���؃��O  ����   �E�P�Uи  ������EЉEԸP0E �E؍UԹ   �=�y����Eܺ�0E ����E�P�E�P��0E �EčU����?���E��EȍU����f����E��E̍UĹ   �4�-����U�X�t���Mܲ�Xc@ �]��� ��3�Uh�/E d�0d� �E�Pj �E�Pj �E�������S�E�P�)����t
�i���t  �E� 3��E�3�Uh/E d�0d� �E�Ht��t*Ht[�   �ӍM��E��լ����tr�E��>���E��E��a�}�|[�}�U�E�P�E�Pj j �E��t��P�E�P�)����u3�E��-�E�   �E�P�E�Pj j �E��E��P�E�P��(����u�E�3�ZYYd��
�\����s���}� u
����   �M�}� �E��E�����P�E�P�d(���d�E�Ht
�����r8�T�U��E��=���E����@P�E�����Pjj �E����P�E�P�N(���j�E�P�E�Pj �E����P�E�P�.(��3�ZYYd�h�/E �E�P��'���� ����3�ZYYd�h0E �E��   �:���E�����Eܺ   �%����c ���֊E�_^[��]�   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ����4   Software\Microsoft\Windows\CurrentVersion\SharedDLLs    ����   

    ����   RegOpenKeyEx    S�������ً��������tP�'���$u�D$��D$�C��3�3҉3҉S��@  [Í@ U��P�   �����PHu��E��ĤSV�M���؍E�����Vjj j�˲�,E �A����E�3�Uh�1E d�2d�"�������   �E���S��t�������M��������3�ZYYd�h�1E �E���������������U��E�����^[��]Ë�SV��؋������Ћ΋�����^[ÐU��P�   �����PHu��E����SVW3ɉ������������������M�����3�Uh03E d�0d� �=�I  uq������胥���������������n����������H3E �F�������������Ѝ���������������� �  �r�������thP3E V��&����I ��I �=�I  ��   �U����Z�����u�������E������������E�����h�  ������P�E����P�E��i��Pj j �'��fǄE����  f������ t������Pj ��I ��u3����3�3�ZYYd�h73E �������   �� ���E��� ����/�������_^[��]�����   sfc.dll SfcIsFileProtected  U��QSVW��ډE��}3�Uh�3E d�0d� ��u��j2�E�P�.��=  t��u��h�   j�j �E�Pj��,��Ht���W�E�P�>%����u�����3�ZYYd�h�3E �E�P��#����x�����_^[Y]� ��U��ĜSVW3ۉ]��]��]����ڈE��u�E�
��3�Uh�5E d�0d� �ú6E �L��u�E����P ����   h6E Sh6E �E��   ������t�u�h 6E W�E��   ����U����_����E��,6E �5����t�U����D����E��<6E �o5����up�n�����t7h6E �E��Ѣ���E��U�����u�hL6E �u�h6E �E��   �E���0h6E �E��n����E��U�苎���u�hd6E �u��E��   ����} u
�U���[����E�3ɺD   ������E�D   �E�   f�Ef�E�} t�E�������3�j j j h   j S�E�P�E�P�E������3ҊE��P�������؋؄�u	�#�����E�P�2"��V�M�U�E�����3�ZYYd�h�5E �E��   �E����E������E������s����ۋ�_^[��]�   ����   >   ����   "   ����       ����   .bat    ����   .cmd    ����   cmd.exe" /C "   ����   COMMAND.COM" /C     U����SVW�M����؋u�E� ��3�UhU7E d�0d� �} u
�U���ې���E�3ɺ<   �t����E�<   �E�@  ��t
���_ ���E̋��U ���EЋE��J ���Eԃ} t�E�9 ���E؋E�E܍E�P�g���������ۄ�u	�"�����  �E���tV�M�U�%���3�ZYYd�h\7E �E������
�������_^[��]� �U��j j j SVW�E��E�����3�Uh8E d�0d� 3ۋU��88E �� ������u�����E�P��I�   �E������E�U��z3���}� t#�E��d���Pj h   �"����t
P�������E��κ   �����}� u�3�ZYYd�h&8E �E��   ������@������_^[��]� ����   ,   U����S���E� jjj �Ȳ�h�D �g����E�3�Uh�8E d�0d� �U�E���Q�}�q  |M�c   �E�������U��   �E�������E���:�t!�u��c   �E��Ϳ���U��   �E���S�E�3�ZYYd�h�8E �E��t�����z������E�[��]�S�����$   T�D$P�����t�ÍT$�   �<�������������[Ë�S���������$   T�D$P�)����t�ÍT$�   ��������������  [��%@8I �����=� I u[Tj(�@��P������u3��T�D$Ph�9E j �����D$   �D$   j j j �D$Pj �D$P�����M����t3��j j��$������؃��  SeShutdownPrivilege U��QSVW�U��؋�N��|GF3���u�EP� �����~	�EP� ���U���������u������t������tGNu�_^[Y]� ��U������P�ČSVW3҉������������U�U��E�3�UhC<E d�0d� �E��3���3�Uh<E d�2d�"�h�������   j j�E�P�T<E �  �3��W������  �M���<E �E��{�����t�E��s���P�E��:����ЍE�Y������M���<E �E��K�����t�E��C���P�E��
����ЍE�Y�����E�P�F���   �����������������������%����������E��<E �>����E��n�����tkjj j�M��h�D �!����E�3�Uh�;E d�2d�"�������   �E��S��t�������M���*�����3�ZYYd�h�;E �E��g�����m�����3�ZYYd�K 25
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 �G�����؈�  ��t�G      �G��}��f%� f��r	�O   ��O�O   �g$�_^[�   SysTreeView32   U��Ĝ���SV��ƃ�    ���)�����   �vW���C��   �}��f%� f��r4�=�I  t+j h\�E ��辤��P��I jjh,  ��觤��P�]���h@  h`  ������Pj hp�E �t��������{���3ɋ��ڼ��h  j �R���P�D����E�3�UhL�E d�0d� 3ҋË���   3�ZYYd�hS�E �E�P������S����^[��]�   E x p l o r e r         SV���ƃ�   3ҋ���	  �֋�腘��^[Ë�SVQ�$��؋֊$������f�>qu7���E "$���E :�u%f�  ��覣���e�������t��蔣�����ż��Z^[�        SVW����� �������t ���g���貼��;�uf�Gf��tf��t	�׋��-���_^[ÐV���Ƌ�Q�^�SV�ڋ��CP�CP�P������P����C^[�U����SVW3ۉ]ԋ�ڋ�3�Uh�E d�0d� ���oU����ti�E�   �]܋��Ң���U��һ����u	���GU���C�> u�ƋU����U����UԋE�� �����Eԋֹ   �Y����臢�����8����؅�u�3�ZYYd�h�E �E���T����KQ����_^[��]Ë�U��j SVW��3�Uh��E d�0d� ���5������������t�M��֋Ë0�V|�U���   ��T�����   �T��f��
   t�Ӌ�  ��  3�ZYYd�h��E �E��^T����P����_^[Y]�U����S�����E܋E܍U������U��M�E�@��yg���E�E��E�E�E�@�膡���U������؅�tn�E�t�E�@��g����   ��觹���O�E�@��N�������;�t�E�@����,  �,�E�@���   t�E�Ft�E�@������   ���V���[��]ÐU����SVW3ɉM�ډE�3�Uh.�E d�0d� �ӋE���Q��C�P��k���&tZ��a�����  ���  J��  Jt)�E  ��m����5  J��  ��  ��  �!  �@4���E �Z���  �E���   t�D�E ��Xc@ �v�����O���E�ƀ  3�Uh�E d�0d� �s�~��   �F\�x ��   �F\�x ��   �F\�@h  j ��~��P�����E�3�Uh��E d�0d� �V<�E�����   ��t#�E��ʟ���V<�b�����u3ɋV<�E��  ��F\�@ �C   3�ZYYd�h��E �E�P�o����qN����3�ZYYd�h�E �E�ƀ   ��RN����C�p�tj �F$�H�V�E�����   �F� tj�F$�H�V�E�����   �F�@t>�E������V護���������F �~  u�F$�x u�V�E�����   ���F �   �t  �E����    �d  �E��h����W  �C�p�F$�x �D  �C   �8  �C�p�F$�x �%  �~ �  �E�V�$R���E��E�f��   t�E�P�M�]��U���  ��  �}� ��   �F$�U��	Q���E�   �F�E̋E���S���E؋E������U��)����E������V豶��P�E�����3�Z�d����E������~U����Y�C   �n���~ t0�~< t*�E�踝���V<�h������E�覝���V�V���;�t�K�F<��t/�E�   �E��E�    �E��v����U��v�����t
�E� t�K3�ZYYd�h5�E �E���O����1L����_^[��]�����&   Internal error: Item already expanding  S��؋��$@   �T$3����D$ ��������������([�U��SVW����3�;�u���E�@���������r�������u���_^[]ÐU����SVW���U��E��E�蓜���U��+����؅���   �E��y������������E�   �]ԋE��^����U��^�����tV�E�x tAU�׋��_���Y��u2�E��2������_����E��#����U�軴����u3ɋU��E��������ϋӋE��P����ޅ��q���_^[��]Í@ U����SVW�M�����]��t��3ҋ������ū�t	�E� ����E� ���E�g   3��E؄�t�M��E�   �E�   �} u�E��?Q���E���E�2Q���E��E������E�������t3��E����u	�E�   ��E�����3��E����E �   �6U�����ƋU���M���^�F �u��������U��%���_^[��]� SVWU��؋��ڋ����������胳���؅�t@�E    �]���̚�����ͳ����t�E$� ���u�����t��誚�����O����؅�u��Ã�(]_^[Í@ SVW����؋ϋ֋��|�����u��t��֋��]���j j�ϋ֋��r���_^[Ë�3�ÐU��S�؋E�@��?����������؅�t#�EP�������Y�E�@������   ���[���[]�U��QS�ډE���t	U������Y�E�����������[Y]Ë�SV��؋�؋��Й���   �������$D   �t$��貙����賲����t�D$$�x u�|$  t3�����(^[Ë�U����SVW3ۉ]�M��ډE�3�Uh��E d�0d� 3ҋE��N����E�����   �E����O�����}�}� t�U��E��P�����tq���S�fu���؊��t,/t,-u��+΍E���lL���}� t�}� t�M�U��E��Z�����M�U��E��������t�E����F�,/t�,-t��> �z����}� t�U��E�����3�ZYYd�h��E �E��K����{G����_^[��]Ë�3�������U����SVW3ɉM�M��U���3�Uh �E d�0d� ���Q�������������   ��3ҋ������׋��N�������   3�F��u�E��U��/K���(�E�P�u��E� �U�3ɸ�E �����M�E��U��1L���M��׋��������u���׋������j j�M��׋���������֋�������������t�Ë�Rx��蚗�����˰��3�ZYYd�h�E �E��   �!J����_F����_^[��]�  ����    (%d)   U����SVW3��E��E�3�Uh��E d�0d� ��n����A��   u��������th�E����J���E�� �E �
K���E��U�������E�Pj �E�ӈP� �U�E���=����E �E��=���U��E��mJ���M��E�@�3��������C��[�y���3�ZYYd�h��E �E��   �7I����uE����_^[��]�����   :\  :  U��Ĩ���SVW3ɉ������������M��ڋ�3�Uh��E d�0d� j�no���E�3�Uh��E d�0d� ���J����u9��������u�E� �MF���+  ��������������������E�@����l  �E�������P����������������������E ��I���������qK��P�l���E�}����   3�Uhz�E d�0d� �������E�����tX�E��������  �RI�����������}����������U��WI���������������B���������Pj �E�@��M����A���������P�E�P�l����u�3�ZYYd�h��E �E�P��k�����C����3�ZYYd�h��E �E�P�n�����C����3�ZYYd�h��E �������   �dG���E��<G����C�����E�_^[��]�  ����   *   U����SVW3ɉM���E�3�UhZ�E d�0d� ��uU����Y��0U�M��֋E���S|�U��������Y�؄�t�E��=���3ɋ�谭��3�ZYYd�ha�E �E��F����C������_^[YY]Ë�SVW�ً����ˋ׋���������G����u��x:u�ú��E ��G��_^[�����   \   U����SVW3ۉ]�M��U����]3�Uh�E d�0d� �}� t���0�������M�U��Ƌ0�V|�E���������3�ZYYd�h�E �E���E����HB������_^[��]� �@ U��3�QQQQQSVW���3�Uh��E d�0d� �M��֋Ë�S|j�Nl���E�3�Uh��E d�0d� �U�E��:����E�U������E��H��P�aj����t�E��������u3����E�3�ZYYd�h��E �E�P��k����A����3�ZYYd�h��E �E�   �;E���E��E����qA����E�_^[��]ÐSV��؋���$"   �T$�D$�����D$������t�$���H���D$���:������S�����(^[ÐS�ڋ��n����c�[�U��3ҊU���(  ]� �S�؋�������ͪ��[Í@ U��Ĩ���SVW3ۉ������������]��]����ڋ�3�Uhk�E d�0d� ��t;���������s���������P������������������Z�O�����u
�U�������j�j���E�3�Uh6�E d�0d� ������P���������q������������E �IE����������F��P�(h���E�}����   3�Uh�E d�0d� �������������t{�E��������  ��D���U��E�������tY�E�@��M����������uE��������������������U��D������������������������Pj �E�@��M�������������P�E�P�{g�����^���3�ZYYd�h�E �E�P�Mg����G?����3�ZYYd�h=�E �E�P�wi����)?����3�ZYYd�hr�E �������   ��B���E��B���E��B�����>����_^[��]�   ����   *   U��3�QQQQQQSVW�ډE�3�Uh�E d�0d� �E�����   �E�U���  ��B���}� u�E�U���  �B���}� ��   3ҋE�������؋E���(  �E���,  �U��E��h����}� u�U��E�������E�Pj 3�3ҋE��]����؋E��;����   ���{����   �M�ӋE��0�V|�E���   t1U�U�E���  �����E�U���B���U�E���$  �������Y�E���   t1U�U�E���  ������E�U��B���U�E���   ������Y�E�蛎��3ɋ�����3�ZYYd�h�E �E�   � A����^=����E�_^[��]Ë�U��j j SVW���3�Uh��E d�0d� �M��֋Ë0�V|��   t)�U���  �A����E��U��B���E�������t��4��   t)�U���  �����E��U���A���E��������t��3�3�ZYYd�h��E �E��   �d@����<�����_^[YY]Í@ U��SVW����؍�  ���g@����  ���Z@����   �U�L@����$  �U�>@�����sr��_^[]� 践��h@�E hX�E ��e��P�*e���LI �SHPathPrepareForWriteA  shell32.dll         ,�E         �E �E   �0A �jA �*@ +@ ��E H�@ |SA d�@ �A xDA �ZA �HA �A �A x�E ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA ,�E �dA �cA �eA ,eA �lA P F �A  ����E TNewNotebook�@ TNewNotebook��E �2A ! NewNotebook t�E �  �� F       �   �	 
ActivePage+A ;  ��DA       �    
 Align��A H  ��KA �KA    �  � Color�)A N  �N  �      ����� 
DragCursorT+A <  �8  �      �     DragMode @ 8  �JA       �    Enabled��A D  �LKA XKA    �   � Font @ :  ��KA       �    ParentColor @ 9  �hKA       �    
ParentFont @ ^  ��KA       �    ParentShowHint�A P  �LJA       �   � 	PopupMenu @ ]  ��KA `KA    �   � ShowHint�+A �A ��A       ����� TabOrder @ �  ���A       �     TabStop @ 7  ��IA       �    Visible$-A x  �x  �      �   � 
OnDragDrop�,A �  ��  �      �   � 
OnDragOver�-A �  ��  �      �   � 	OnEndDrag�@ �  ��  �      �   � OnEnter�@ �  ��  �      �   � OnExit�+A `  �`  �      �   � OnMouseDown,A h  �h  �      �   � OnMouseMove�+A p  �p  �      �   � 	OnMouseUpl-A �  ��  �      �   �  OnStartDrag�@         t�E             `�E    4A �jA �*@ +@ <�E H�@ |SA d�@ �A xDA ��E �HA �A �A ��E ��A ��A  �A @CA �CA �IA �iA T�A ��A h|A ��A �^A �eA �bA �dA �cA �eA ,eA t�A �~A �A ��E TNewNotebookPage�@ TNewNotebookPage��E �4A  NewNotebook ��A H  ��KA �KA    �   �	 ColorT+A <  �8  �      �    
 DragMode @ 8  �JA       �    Enabled��A D  �LKA XKA    �   � Font4@ 0  ��EA        �   � Height4@ $  �tEA        �   � Left4@ l�E 8�E        �   � 	PageIndex @ :  ��KA       �    ParentColor @ 9  �hKA       �    
ParentFont @ ^  ��KA       �    ParentShowHint�A P  �LJA       �   � 	PopupMenu @ ]  ��KA `KA    �   � ShowHint4@ (  ��EA        �   � Top @ 7  ��IA        �    Visible4@ ,  ��EA        �   � Width$-A x  �x  �      �   � 
OnDragDrop�,A �  ��  �      �   � 
OnDragOver�-A �  ��  �      �   � 	OnEndDrag�@ �  ��  �      �   � OnEnter�@ �  ��  �      �   � OnExit�+A `  �`  �      �   � OnMouseDown,A h  �h  �      �   � OnMouseMove�+A p  �p  �      �   � 	OnMouseUpl-A �  ��  �      �   � OnStartDragSV��t����:0���ڋ�3ҋ���������G��f�8�E fF4f�F43ҋ��L����t
d�    ����^[�    SV�ڋ���   ��t����  3ҋ�������t���0����^[Ë�   ��t��   �蚶��Ã��ÐS�؋��6����Ct7���   �@��x������   �@��D����C,P�C0P3�3ҋ��   詯��[Í@ SVW��؋֋���\���~(�Ǻ��E �-����t	�׋��   _^[�SV��؋�   ;�t!��t���  �֋Ë�Q<��t	�Ӌ���  ^[�SVW��؋�   ��t(��   �W;�|��N��}3��������Ћǋ��+���_^[Í@ SV��t����.���ڋ�3ҋ��QZ����   ���G����   ���)G����\�@ �y,����   ��t
d�    ����^[�SVWU�ڋ苅   ��t/�pN��|F3��׋�   �ڴ��3҉�   GNu狅   �G,��3ҋ��BZ����t���G.����]_^[�S�ڋ��jc���K   [ÐSV�ً���   �x ~U�Ŵ���Ѓ��u��t��   �PJ�3ҋ�t@��   ;Bu3����u	��   �@H�Ћ�   �3����;�u�3�^[�U��QSV�E��E���   �XK��|C3��֋E���   ������ЋE�UFKu�^[Y]� ��   �ݳ��Ë�   �@Ë�SV��؋֋�   �!�����   ^[�SV���3���   �֋�   �ɴ��;��   u	3ҋ��8   ^[ÐSV��؋ƺ��E �>+����t;�   u	�֋��   �֋��8~��^[ÐSVW��؅�t;�   ��   ;��   ��   ���^�������t+���    t"��  ���   �_����t���   ��������tJ���rK�������H����t6���    t-��  ;��   u��������t�֋�����	�Ӌ�������   ��t3��H�����   ��t"���    t��  ;��   u���   ����_^[Ë�     F �F 2F �F     �F �  t�H PB �*@ +@ �B H�@ B d�@ �A �B 0B �HA �B �A dF ��A �	B  �A @CA �CA �B ��H T�A ��A h|A ��A �B �eA ,B DB B B ,eA \B �~A �-B         |@ �   �F �    BrowseLabel�   PathEdit�   NewFolderButton�   OKButton�   CancelButton   F PathEditChange L	F NewFolderButtonClickTSelectFolderForm �D VB �mB �TSelectFolderForm�F �H 8 SelFolderForm  �@ U��j j j j j j j SVW��U��؋}3�UhQF d�0d� S3ɲ��F �  �E�3�Uh'F d�0d� �E��U����  �E��  ����0���}� u��u3����E����  �CF������   �E����  ���E �(�����=k"I  tU�E�P3ɲ3��"X �E�P�E�P3ɲ��X �E�P�E�P3ɲ���W �E�P�E�P3ɲ3���W �U��Y�����0�E�P3ɲ3���W �E�Pj �E�P3ɲ3��W �U�3ɋ��������1  ����  �}� t5�U��(����E��2��t1�U������U���E����  �c�������E����  �O�����u�E����  ��    t�E����  �E�������E����  ��E���E��)��H�E��}� t&�U�E����  �E���E�U��9f���U���?/��3�ZYYd�h.F �E��2&����8+����3�ZYYd�hXF �E��.���E�   ��.����+����E�_^[��]� U����SVW��t����'���U���3ҋ���� ���   u=�β���E �n����؉��  ��  ǃ  �F �PI ��  ǃ  �lF �;�β���E �1����؉��  ��  ǃ  �F �PI ��  ǃ  dmF h=  h�   �@   �   ���  ��SL3ҋ��  �C���֋��  ��Q<���  �F�����  �F��f� ���  �2|������  �iC������ ��I ���)D����I ���  �D�����  �PI ��  �؋��  �P(���>�����  �P(���>����I ���  ��C�����  �P(��>���E��U�3ɋ��� �Ћ��  �>���E��E��U�   ���� �E���I ���  �}C���E�P���  �G0P�W$W,+U��O(ˋǋ8�WL��I ���  �IC���E�P���  �G0P�   ����� ���  �R$+�+U��O(ˋ��  �8�WL����>����Ӌ��{��3ɋPI ���� �}� t
d�    ����_^[��]Í@ U��QSV��t����B%���U��؊E���  3ҋË0�V$�}� t
d�    ����^[Y]� ��U��j j SVW��3�Uh�F d�0d� �U����  �8B���E��U���b���}� ���  �A��3�ZYYd�h�F �E��+���E��~+�����'����_^[YY]Í@ U��j SVW��3�Uh=	F d�0d� ���   t2�U����  ��   �˺���E����  �,���U����  ��A������  ��   ���  �A�����   u���  ��    u3������  ��@��3�ZYYd�hD	F �E���*����"'����_^[Y]Ë����  ���  ����Ë�    �	F �	F             �	F d   ت@ ,,@ �*@ +@ �F H�@ A d�@ �A <A �A �A �A �A �F F ,F HF `F |F         |@ ,   |@ 0   TWizardPage��TWizardPage�	F X�@  Wizard      �
F 8F F �F �F �F 8  t�H PB �*@ +@ �7F H�@ B d�@ �A �B 0B �HA �B �A � F ��A �	B  �A @CA �CA �B ��H T�A ��A h|A ��A �B �eA 08F DB B B ,eA \B �~A �-B         |@ �  |@ �  |@ �  |@    |@   |@   |@   |@   N �F �    CancelButton�    
NextButton�    
BackButton�   OuterNotebook�   InnerNotebook�   WelcomePage�   	InnerPage�   FinishedPage�   LicensePage�   PasswordPage�   InfoBeforePage�   UserInfoPage�   SelectDirPage�   SelectComponentsPage�   SelectProgramGroupPage�   SelectTasksPage�   	ReadyPage�   PreparingPage�   InstallingPage    InfoAfterPage   DiskSpaceLabel   DirEdit   	GroupEdit   NoIconsCheck   PasswordLabel   PasswordEdit   PasswordEditLabel    	ReadyMemo$   
TypesCombo(  	 Bevel,  
 WizardBitmapImage0   WelcomeLabel14   InfoBeforeMemo8   InfoBeforeClickLabel<   	MainPanel@  	 Bevel1D   PageNameLabelH   PageDescriptionLabelL  
 WizardSmallBitmapImageP   
ReadyLabelT   FinishedLabelX   YesRadio\   NoRadio`  
 WizardBitmapImage2d   WelcomeLabel2h   LicenseLabel1l   LicenseMemop   InfoAfterMemot   InfoAfterClickLabelx   ComponentsList|   ComponentsDiskSpaceLabel�   BeveledLabel�   StatusLabel�   FilenameLabel�   ProgressGauge�   SelectDirLabel�   SelectStartMenuFolderLabel�   SelectComponentsLabel�   SelectTasksLabel�   LicenseAcceptedRadio�   LicenseNotAcceptedRadio�   UserInfoNameLabel�   UserInfoNameEdit�   UserInfoOrgLabel�   UserInfoOrgEdit�  
 PreparingErrorBitmapImage�   PreparingLabel�   FinishedHeadingLabel�   UserInfoSerialLabel�   UserInfoSerialEdit�   	TasksList�   RunList�    DirBrowseButton�    GroupBrowseButton�  
 SelectDirBitmapImage�  
 SelectGroupBitmapImage�   SelectDirBrowseLabel�    SelectStartMenuFolderBrowseLabel  \`F NextButtonClick <cF BackButtonClick �cF CancelButtonClick hdF 	FormClose �eF NoIconsCheckClick |dF TypesComboChange eF ComponentsListClickCheck  `fF LicenseAcceptedRadioClick# xfF LicenseNotAcceptedRadioClick �fF UserInfoEditChange ,kF DirBrowseButtonClick �kF GroupBrowseButtonClick $fF TWizardForm �mB ��E ��E �D VB tB 4�E �]B �fB ��B D�E �D x�B �yB ��D T�D ��TWizardFormL
F �H 8 Wizard  S�����(��F �<$��,$�����F ���v�,$�O����F �<$�S�D$�D$�D$�T$3ɸ�F ��d����[�    �:      �?����   %.0n    S�����(�@F �DF �<$��,$����HF ���v�,$�����LF �<$��,$�-PF ���<$�S�D$�D$�D$�T$3ɸdF �Jd����[�   �5   A      �?���������?  ����   %.1n    U��j SVW�����3�UhF d�0d� ��3ҊӋ�tI �����ǋ�"I �F �L����ǋ�"I �,F �:����U��������M��Ǻ@F �!����U���������M��ǺPF ����3�ZYYd�hF �E�������Y����_^[Y]� ����   [name]  ����
   [name/ver]  ����   [kb]    ����   [mb]    SV��؋κ�"I ��� ���^[ÐU����SVW3ɉM�U��E�3�Uh�F d�0d� �E���R��K��|'C3��M�֋E��8�W�E�U��`T����u��FKu�3�3�ZYYd�h�F �E��!����������_^[��]�U��j j SVW��3�UhnF d�0d� �U���,U���E��U��u����E��U��j����U�������U���U���E��.������%��;�u�3�ZYYd�huF �E��   ����������_^[YY]�U��j j SVW��3�Uh'F d�0d� �U���T���U��������ù   �   �� ���; t�� ,/t�,-tލU��輲���E��U��]����U����c���U���IT���E��u������l��;�u�3�ZYYd�h.F �E��   ������8����_^[YY]Í@ S�؋��2����~�   �|�w�[�BHu�3�[Ë�S�؋�������8 u�P��\t��u�[�P�5F���8 u�3�[�S�؋�����F�@�8 t�3���@�8.t��@�8 t���t���\t��u
�[�P��E�����t��\u�8\u@�8 u�3�[ÐU����SVW3ɉM��ډE�3�UhuF d�0d� �������E��X������~=�   �E��|0� t*�; t�ú�F �:���E��U��T2�����U����!��FOu�3�ZYYd�h|F �E�����������_^[YY]� ����       SVW�ڋs,�{0����p���3ҋ��g����֋���,���C0+�_^[ÐSV��F,P�F0+�PN(�V$�Ƌ�SL^[Ë�U����SVW3҉U���3�Uh�F d�0d� �=q"I  ��   �=�"I  ��   �U����  �Z1���U��4"I ����U����  �?1���U��8"I ����U����  �$1���U��<"I ����j jj �U����  �1���E��E��E��M���F ��"I 蛤 ����3�ZYYd�h�F �E��L����������_^[��]�   ����   CheckSerial U����SVW3҉U�E�3�Uh"F d�0d� �|!I �TI ��!I �XI ��Ц@ �����j 3ɋ׋E���-  ��!I �XK��|fC3��֡�!I 艙���E�j�M�3ҋ��	 ��t>�E��E�E�x8�t�E�P8� "I �U����P�TI �������E�P@�TI �����FKu���!I �XK��|6C3��֡�!I �����E��E����������t�E��P�TI ����FKu͋��l���M�TI ��q����U�E���|  �/��3�ZYYd�h)F �E�������=����_^[��]�U����SVW3ۉ]�]�M����3�UhQF d�0d� �֋�x  �������3��E�3��E�}� tSh0F �M�֋�x  赳���G6�E�G:�E��U�E�������֋�x  �!�����t�U�E�����}� u�}� t]��0   u*�U�E��z����U�M��q����M�֋�x  �1����9�U�E�������U�M��G����M�֋�x  �����3ɋ֋�x  �����3�ZYYd�hXF �E�   ����������_^[��]� �@ �����!I t#3҉T$3҉$Ph0F �L$�����x  譲��YZË�SVW��t��������ڋ���3ҋ�� ����ƺL
F �����G`��t
d�    ����_^[ÐSV�ڋ��F`��t���   t���  ������3ҋ��������t���Y����^[�f�x6 t
�ȋЋA8�Q4Ë�SV���f�{> t
�ӋC@�S<�^[Ë�Sf�xF tQ�ʋ؋ЋCH�SD[ÐSV���f�{N t
�ӋCP�SL�^[Ë�SV���f�{V t
�ӋCX�ST�^[Ë��H`���  �P$;�u�@(Ë�ÐS�؋�������@ �@0[Ë�S�؋�������@ �@,[Ë�SV��؍C,���h�����!   ^[Ë�SV��؍C0���L�����   ^[Ë�S�؋C`��4  ;S u��D  �S,�,���C`��H  �S0�p,��[Ë�U����S�E��}� ��   3�Uh�F d�0d� ���   �    �Ë�Q,�    �Ë�Q(���ݱ���@�  �謇��j �E�P�    3�3��$����E�P��豱��Z�;���jj j j j �E�Pj j ��蒱���Q���P�?��3�ZYYd�h�F �E�P�=?����g����[��]�U��Ĝ���SVW3��E�3�Uh� F d�0d� �E�@����  �    �{&���E�@����  �    �&���E�@����  �    �O&���E�@����  �    �Y&��3�Uhi F d�0d� h  h`  ������Pjh� F �«����t3������ t*������P������P�I P蔫���U�R����  �g����E�P3ɲ3��< �}� u�E�P3ɲ���; �}� tXh   h`  ������Pj �E����P�@�����t3������ t*������P������P�I P�����U�R����  �����3�ZYYd��
������3�ZYYd�h� F �E��x���������_^[��]� c:\directory    U����SVW3ۉ]Љ]��]��t����h���M�U��E��]�3�Uh[7F d�0d� �M�3ҋ�v� ��\�@ �6
������  ��Ц@ �"
�����(  ��Ц@ �
�����  ��Ц@ ��	�����  ��Ц@ ��	�����   ��Ц@ ��	�����$  �$"I ��R���L  �r0+���~;���L  �P0+֋��L  �9$�����L  �P(��y�� ֋��L  ��#���$"I ��R ���L  �r,+���~;���L  �P,+֋��L  ��#�����L  �P$��y�� ֋��L  �b#����� ��!I t���I ��#� ���>� j j���0  �@D��!I ��!I 躡 ���0  �@D�|7F ��������D  �@D�|7F ������!I t��I ��(����MЋ�"I �������UЋ��'����!I uF��#���E܋��#���E؋��  
�7F ��F������i����U܋�����U؋�%����E�
�E��E��E��E��Uȹ   ��z� ��
   ��� �E�����  ���j"������  ���["������  ���L"�����"����+}�+�����  ����!��+}�+�����  ����!��+�����  ����!�����,  �`!I �#������,  � "I �(������,  ��%�����!I @���,  �V������`  �`!I �ם�����`  � "I �ܝ�����`  ��ٝ����!I @���`  �
������L  �$"I 裝����!I @���L  �����h�7F �I P��;���Ћ���  ���   跱������  ���� �y�����PH����  ����U�I���Yj j j ����  �   ��Z  �Uа��x����Eк�7F ����UЋ��0  �%�����0  ��������0  �H(H0���d  +J(������Uа��"����Eк�7F �9���EЋ�I �+���UЋ��d  �O%������  P��I P�I P����  �   ��  �Uаe������UЋ��h  �%�����h  �����������l  �΋���������  � I ��$������  �I ��$������  P��I P�@I P����  �   ��  ���  �DI �$�����  �<I �u$�����  ��F��������  �P(��6�����  ��#�������  �P(��������  P�|I P��I P����  �   ��  ���8  ��I ��#�����8  �����������4  �΋���������  P��I P�TI P����  �   ��(  ����  �XI �#������  ��f���������  �P(��V������  �P(��D������  �`I �M#������  ����������  �P(�����=q"I  tZ����  �P(֋���  ��������  �dI ��"������  �����������  �P(֋���  ��������  3���!������  3���!������  P��I P�Uа��8����E�P����  �   ���  �Uа������UЋ���  �^"���   ���� ������  �F$F,�����  ��+F$�V,+�R�F0P�N(�Ƌ׋0�VL����  ����������  �@0����  ;B0~8����  �P0����  �@0H+���y�� ����  P(����  ����Uа��\����UЋ���  �!���   ��L� ����  �W(W0J�P�   ��.� ����  �V(V0�Z���������  +B(������  �P(֋���  �!������  ���������  �P(֋��  ��������  ��I �� ���E��U�3ɋ�� ��W����  �@0P����  �H(΋���  �P,+׋���  �0�VL�
   ��Q� ����  �R$+Ћ��  +P$���  ����Uа)�&����UЋ��  �n �����  �֋�=����V(+Ћ��5������  P��I P�Uа�������E�P����  �   ��  �Uа������UЋ���  � ������  �����������$  �P(֋��$  ������x  �΋������Uа�h����UЋ��|  ������|  ������=w"I  t?��!I �xu4���$  3�������$  �H(���x  +H(���x  ��i�������  P��I P�Uа�������E�P����  �   ��  �Uа�������UЋ���  ����   �覞 ������  �@$����  B,�����  ��+F$�V,+�R�F0P�N(�Ƌ׋0�VL����  ���������  �@0����  ;B0~8����  �P0����  �@0H+���y�� ����  P(����  �J���Uа������UЋ���  �L���   ���� ����  �R(����  Q0J�P�   ��Ν ����  �R(����  Q0�Z蔹��������  +p(����  �P(֋���  �������  ���������  �P(֋��  �������  ��I ����E��U�3ɋ�{� ��W����  �@0P����  �H(΋���  �P,+׋���  �0�VL�
   ��� ����  �R$+Ћ��  +P$���  �$�����  �I �������  P��I P�Uа������E�P����  �	   ��\  �Uа��z����UЋ���  ��������  �����������  �΋��������  3��I����   ��9� ����  ��T  ��!I ����  �ȯ������  P��I P�Uаy������E�P����  �
   ��
  ����  P��I P�Uаw�����E�P����  �   ��~
  ����  P��I P�Uа[�����E�P����  �   ��M
  ���   P�xI P��I P����  �   ��$
  ���t  ��I ������t  ��b��������p  �΋����j j j ����  �   ���	  j j����  �@D��!I ��!I 虔 ����  �@D�|7F ��s���UаN�����E�P��7F X�����UЋ���  ��������  ����������  �P(����  P0���T  ������X  ��I ������\  � I ����   ��F� ����  ��T  ��7F �E��[���EЋ�I �5���Eк�7F �(���UЋ���  �L���=("I  t"���l  ���������l  �("I ������=,"I  t"���4  ��������4  �,"I ������=0"I  t"���p  ��z������p  �0"I ����j ��P��P�.����j j h   V�,���tI �)��Ph'  j V�g,���� 	  �E������E� ��!I ��   ���   uY�UС� I �8 �UЋ���  �L���UС� I � �UЋ���  �/���UС� I �� �UЋ���  ����?���  ����  �������  ����  �������  ����  ������!I ��   �UС� I � �UЋ�  �H���=�I  t�E��I �u���&�E����  �c���}� u�E����  �M���U��E�蚔���E��U��7����UЍE��,�����  �U��8������  �( I �#����!I �x �
  ���$  ��l����!I �@H��|u@�E�3��֡�!I �����E��UЋE��@� �UЋ��$  ���   �M��8�W0�}��u-�=�I  t$�E�� ��I �7����u�u�E��@$u�E�F�M�u��}��uN���   tC��!I �@H��|6@�E�3��֡�!I �y����E����  �E�� �6����u�u��F�M�uЃ}��t���$  �U��_l������$  3��Nl�����x  �Q�����!I ���x  �ک����!I �@H����   @�E�3��֡�!I �������G5t0�GPj �G5��4PW�UЋG�{ �UЋ��x  3��1����<�GPj �G5��4P�G P�G5��4PW�UЋG�= �UЋ��x  3��#����: u	�6   r	�ƀ0  F�M��^����}� u}�=�I  tt�=w"I  tk��!I �@H����   @�E�3��֡�!I ����E��E��@$t1���$  ���k���E������  3ɋ�I ��f  �   F�M�u��   �}��tg�U��!I �~���E��E��@$t>3ҡ�!I �~���3ɋ�  �E�����r  ���  ���  ���  �8�E��3ɋ�K  �(��!I �x ~3ҡ�!I �J~���E��E��3ɋ�!  ��������3����=w"I  tP���$  �j���С�!I �	~���E��E��@$u	��!I t���x  ������ ���x  3���������x  3��������x  �P7���|  ����=w"I  t(���$  �i�����,  j ���(  3ɋ��  �UС� I �% �UЋ�  ������=�I  t��!I u�E��I ������A���    t���   ��7F �����u�E����  �������E���   �������  �U�������!I  t4�=�I  u���   t���  ��gy�����  ��������  3����3�ZYYd�hb7F �E������E������E�������������}� t
d�    ���_^[��]�       STOPIMAGE   ����   
  ����   

    ����       ����	   (Default)   SV�ڋ���  ������   �ة����$  �ͩ����  �©����(  跩�����  謩��3ҋ��o�����t���D�����^[Í@ S�ڋ��������I �4I���C[Í@ U����SVW3ɉM��U���3�Uh�8F d�0d� ���  �pN��|F3ۋӋ��  �I{���@ ;E�t,CNu�E�P�E��E��E� �U�3ɸ�8F ��?���E��������3�ZYYd�h�8F �E��,������������_^[��]�   ����   Could not find page with ID %d  SV��؋֋��-����Ћ��  �z��^[ÐU��QSVW�M��������  �sz���β��	F �A����؉{ �E��C$�E�C(�U���f����U���x����Ӌ��  �y��_^[Y]� �@ SVWU����؃��t�׋�������E�	���  �h���  ��y�����  ���  u
ǃ�  d   �β���E ����������  ���	������  �F ���  �F$�~(�΋Ջ��  �z��]_^[�U����SVW3҉U�U��U�U���3�UhY<F d�0d� �U��x I �4 �}� ��  �U��E����  �   ��
I j j�E�P�E�P�p<F �E��E��E��E��E��Uܹ   ��<F ��=���E������ȋ�DI �s������k  3�Uh-<F d�0d� ��!I t���  ��<F �E��q�����!I t+��   ��<F �E��U�����<F �E��`�����tƆ  ��!I @tW��  � =F �E��!����M��=F �E�������t�U���  �G�  �M��8=F �E�������t�U���  �%�  ��!I tD�M��\=F �E��ğ����t�U���   ���  �M��x=F �E�袟����t�U���$  ���  ��!I t9��  ��=F �E��t�����  ��=F �E��a�����  ��=F �E��N���3�ZYYd�h><F �E�P�h����2������O�9���3�ZYYd�h`<F �E�   ������������_^[��]� ����3   Software\Microsoft\Windows\CurrentVersion\Uninstall ����	   %s\%s_is1   Inno Setup: App Path    Inno Setup: Icon Group  Inno Setup: No Icons    Inno Setup: Setup Type  Inno Setup: Selected Components Inno Setup: Deselected Components   Inno Setup: Selected Tasks  Inno Setup: Deselected Tasks    Inno Setup: User Info: Name Inno Setup: User Info: Organization Inno Setup: User Info: Serial   SVW�؋�P  �p,�x0������P  ��j���֋�P  �����P  �@0+ǋȋ�   ���~���_^[Ë�U��j SVW���3�Uh�>F d�0d� �֍E��(����E��?F �����U���T  �)����T  ���������T  �p(p0��X  ��+H(���  �������֋�X  �����   ��蓋 ��֋�\  ���3�ZYYd�h�>F �E�������i�����_^[Y]� ����   
  U����SVW3ۉ]��]܉]�M�U��E�3�Uh�@F d�0d� �E����  ���   ��R8�"I �@H���   @�E��E�    �U�"I �\t���E��E��@M��   �M��U�E���  ����   3�Uh$@F d�0d� �E��x t�U�E��@�� �U�E��@Mu'�U܋E�� � �E܍U��.����U��M찂�i����%�U܋E�� � �E܍U������U��M찃�B���3�ZYYd��$�G����U��(I �"���E캼@F �Q����D���j �E��@M��4Pjjj�E�P�E����  3ɋU��~���E��M������3�ZYYd�h�@F �Eܺ   �����E��^����������_^[��]�   ����   [Error] U����SVW3ɉM؉M�M��M܉U��E�3�Uh�DF d�0d� 3��E��Ц@ �0����E�3�Uh�DF d�2d�"��Ц@ �����E��M��U�E��
  �E����  ���   ��R8�E�����3��E�3��E��!I �pN���U  F3��ס�!I �`r���؋C;E���   �S'�C�� ����   j �CP�CP�K3ҋE����  ����   �U�C��
 �U��C��
 �{ u/�E��U�����t"j j �E����  3ɋU��<}���E܋U������C1t2�CP�=�I  u�C1t3���PjS�E����  3ɋU��}���>�CP�=�I  u�C1t3���Pj�CP�C1��4PS�E����  3ɋU��|���C@�E�]��1�}� t�E�@;Cu�E���~'����t�C;E�}�E�3��E�GN������=�I  ��   �E����  ���   ��R��N��|sF3��E����  ���/����؅�tV�E���   ��e�����t�C1�����E����  �������&�E���$  ��5�����t�E����  3ɋ�����GNu���I ��R����   �E����  ���   ��R��N����   F3��E����  ��苐���؅���   �EF �E�������E؋������Uء�I ������t�E����  ����{����^���I ������t�E����  ����7����:�$EF �E������E؋�b����Uء�I �I�����t�E����  3ɋ������GN�D����E����  ���   ��R��N��|_F3��E����  ��豏���؅�tB��E��������t�E����  ���蟓��� ��E��������t�E����  3ɋ��}���GNu�3�ZYYd�h�DF �E������E��|����������3�ZYYd�hEF �Eغ   �"�����`�����_^[��]�   ����   *   ����   !   SV�؋�$  �Z��@t��$  ���Z���Ћ��   ��Q^[�3�^[�U����SVW3ۉ]��M�U��E�3�Uh�FF d�0d� ��!I �xO����   G3��֡�!I �3n�����C5��   �}� ��   ��FF �E�������E�������U��E�������t�E���x  ����i����   ��E��r�����t�E���x  ����$����`��FF �E��q����E���O����U��E��8�����t�E���x  3ɋ������&�}� t ��E�������t�E���x  3ɋ����FO����3�ZYYd�h�FF �E��m�����������_^[��]�  ����   *   ����   !   SVW���L$�T$�$��Ц@ �W�������!I �pN��|GF3ۋӡ�!I ��l���|$ t�@5t'�P����  �T$���W����ȋ$��x  ������CNu����"�����_^[Í@ U��QS�ز�Ц@ ������E�3�Uh�GF d�0d� j 3ɋU����   �U����H���3�ZYYd�h�GF �E��������������[Y]Í@ U��j SVW�ً�3�UhHF d�0d� ��t1�ӋF�� �E�x t'�M��V�R� �K����U����������	�Ë����3�ZYYd�hHF �E��������Q�����_^[Y]ÐU����SVW3ۉ]��M��U��E�3�Uh�HF d�0d� �E���R8�E���x  ���   ��R��N��|GF3ۋӋE���x  �Y�����t,�ӋE���x  虋����U�M��U�������Y�U��E���Q,CNu�3�ZYYd�h�HF �E��@����������_^[��]� �@ U����SVW3ۉ]��M����]3�UhTIF d�0d� ��t9�U��F� �E��������}� t'�M��U�R� �����U���������	�Ë�V���3�ZYYd�h[IF �E������������_^[YY]� U����SVW3ۉ]�]��]�M��U���3�Uh�JF d�0d� �E���R8�} t�E��\������  ���   ��RH����   @�E�3ۋӋ��  ���������   �Ӌ��  �>�������   �Ӌ��  �)������} tV�U��G�~ � u4�E��U��e���t'�}� t�U�E��Ι���U�E���Q,�E�U��L����G�E�}� t�E���G�E�E�P�E�P�M�U����`����U�E���Q,C�M��;���3�ZYYd�h�JF �E��b����E�   �u����������_^[��]� SVWU����L$�$���$��R8�D$��R8��x  ���   ��R��O��|@G3ۋӋ�x  ������Ӌ�x  輈����t�U �$��Q,��U �D$��Q,COu�YZ]_^[�SVWU����L$�$��$��R8�D$��R8���  ���   ��R��O��|BG3����  ��蟈���؅�t(���  ���8�����t��$��Q,���D$��Q,FOu�YZ]_^[Ë�U��j SVW��3�Uh�LF d�0d� �3ҋ��  �����3ҋ��  ������ ��tv�U�K 25
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                W3��E�E�E؉E�3�Uh�]F d�0d� 3ۋE�@���
  ���^  �U��E�@���  �:����=i"I  �.  �E�P�U�E��Rk���E�U��'g���U�M�3��&�����tj��"I �E�警����}Yh  jj�E�P�U丈"I 诶���E�E܍U؍E�螶���E؉E��Uܹ   �*�]����E�� I �a ����   �=�!I t�=�!I  uJ�E��U�R����  �v���t4�E��o����t(jjj�M�U��&�-����E��I � ��uL��!I t4�E���n����u(jjj�M�U��$�����E��I �� ��u��@"I �U������3�ZYYd�h�]F �E��[����E�   �n����E��F���������ۋ�_^[��]ÐU����SVW3��E�E��EЉE܉E�3�Uh�_F d�0d� 3ۀ=i"I  ��  �E�P�U܋E�@���  �e����E܍U��i���E��U��ce���U�M�3��b�����tj�TI �E�������}Yh  jj�E�P�U�TI �����E��EԍUЍE��ڴ���EЉE؍UԹ   �*虦���E�� I � ����   �E�@���  ��R����   �E��3����E�@���x  ���   ��R��N����   F3��E�@���x  ����t����ue�E�@���x  ���u���E��E��@5uF�E���E�@���  �2�����t,�}� t�E��_F �'����U�E��@�)�  �U�E�����GNu��}� t(jjj�M�U�m�ѥ���E��,I � ��u�3�ZYYd�h�_F �E��I����E��A����E�   �T���������ۋ�_^[��]�   ����      U��j SVW3�UhJ`F d�0d� �E�@���	  �؄�t!�U��E�@���  �Y����U��D"I ����3�ZYYd�hQ`F �E��������������_^[Y]Í@ U����SVW3ɉM�M�E��}�3�UhcF d�0d� ���4  ���d  ����   �$��`F 0aF 0aF �`F �`F 0aF �`F �`F aF !aF ����  ��   �  �SU�e���Y���
  �BU����Y����  �1U�����Y����  � U����Y����  �U�����Y����  �E����4  �������U���Q4�}� ��  �=�"I  t/j j j���4  �E��E� �M�,cF ��"I �\ ���i  ���4  �Ӌ��������tKtp����   ����   ��   �U����  �����U�4"I �t����U����  �����U�8"I �W����U����  �w����U�<"I �:����i�U���  �X����E�U��ah���U�@"I �����?�U���  �.����E�U��7h���U�D"I �������ƀ1  ��I ��R �uF����  ���CQ���X �Ӌ������Ã�tHt�3�������t9�&�Ӌ������I �_N ��������ƀ1  ��Ӌ�C�����������Ӌ�f���3�ZYYd�hcF �E�������E��������K�����_^[��]�  ����   NextButtonClick S���؃�4  th�$��4  �������ԋ�Q,�<$ tJ�=�"I  t,j j j��4  �D$�D$ �L$��cF ��"I �Z ��t���k������t	�Ћ�������[�����   BackButtonClick ����Ë�SVWU������؋�4  �������ϋ֋(�U0�> tA�=�"I  t8jj ��4  �D$�D$ �t$�D$�|$�D$�L$�TdF ��"I �Y ��]_^[�   ����   CancelButtonClick   S�١�I �o���� [Ë�SV�؋�$  ���K;���Ћ��   ��Q���F$����������=w"I  tQ��$  �;����,  j 3ɋ�(  ���E�����!I u&�F$��x  �������x  �P7��|  ��������W������ȴ��^[ÐSVWUQ�ز�Ц@ ������j 3ɋ֋��������(  ����V���$�������<$ t��,  ��$  �:���Q��!I �xO��|DG3�ա�!I �NN�����F$t)��$  ���   ����Z���Ћ��J:�����������EOu���袶��������Z]_^[ÐS�؋�  �~J���Ѐ���  �*�����  3ҊP8���
I ������  �LJ���Ѐ����  �����[Ë�S�J�ف���  �� �  u�(I �����[Á�'  u��I ��= �諾��[Ð��4  u����  ����Ð��4  u3ҋ��  ����ÐU��QSVW�E��E���4  uO3�Uh�fF d�0d� �E��$����ЋE����  �L���3�ZYYd������3ҋE����  �-����H�������_^[Y]Ð��!I @����!I ���  �m   �U��QS�؁=|"I   rR�=\I  tI3�������E�3�UhzgF d�0d� j �������Pj j �\I 3�ZYYd�h�gF �E��w�����������[Y]Í@ U��j j j j j SVW�M��U���3�Uh�iF d�0d� 3ۍU���������E��.����U���������E�����=�   ~j jj�3ҡI �Q �  �E��x�����|�E��8\u	�E��x\t3�����t�}� uj jj�3ҡI �
 �h  ��uY�}� u�   ��   �E�����;�"�E�� �M����,s�E��x:u	�E��x\tEj jj�3ҡ�I �
 �  �E��p�\���}a��;�rj jj�3ҡ�I �
 ��   �E��l�����u�E�舮����u�E�謮����tj jj�3ҡ�I �D
 �   �E�P�����   �E��a����U��iF �`_����t.j jj�U��iF 豮���U�M������E�3���	 �M�U�E��S^���E�U��(Z���E��U��a���E������E��b����uj jj�3ҡ�I �	 ��3�ZYYd�h�iF �E�   �c�����������_^[��]�  ����   /:*?"<>|    U��j j j SVW��3�UhkF d�0d� 3ۋ�  �OF������   �U���  �]����E��Q����U���  �w����E��3�����x~j jj�3ҡ�I �� �   �E��Ѭ����u�E�������tj jj�3ҡ�I � �]�}� uj jj�3ҡI � �A�U�� kF ��]����t.j jj�U�� kF �����U�M��肚���E��3��Z ��3�ZYYd�hkF �E��   ������Z������_^[��]�   ����   /:*?"<>|    U��3�QQQQSVW��3�Uh�kF d�0d� �U����  �>_���E��U��\���E�U�������}� u�U��I ������U���  ������E�P��!I ��M�3��o�����t�U���  �����3�ZYYd�h�kF �E�   �O����������_^[��]�U��j j j SVW��3�Uh{lF d�0d� �U��  �
\���E�U��+����}� u�U��I �����U���  �J����E�P��!I �M���Ɩ����t�U���  �P���3�ZYYd�h�lF �E��   ������������_^[��]Í@ U��j j SVW�ًu3�Uh@mF d�0d� �U�������U��������; t��Ǫ����t� j jj�3ҡ�I �\ �?��XmF �[����t/� j jj�U��XmF �����U��M���C����E��3�� 3�ZYYd�hGmF �E��   ������������_^[YY]� ����	   \/:*?"<>|   U��j j SVW�ًu3�UhnF d�0d� �U�������U��������; t�������t� j jj�3ҡ�I � �?��0nF �Z����t/� j jj�U��0nF �����U��M���k����E��3��C 3�ZYYd�hnF �E��   �	�����G�����_^[YY]� ����	   \/:*?"<>|   U����SVW3҉U�E�3�Uh�oF d�0d� �E���4  u!j jj�U��x�����E��3�� �����E���4  �E�3�Uh�nF d�0d� �E����  �8����t�E����  f����:���3�ZYYd��,雿����I ���  w�P�����U��(I �a��������E���1   ug�E���4  ;E��A�����I ���  w��oF �����9��������oF ������w�  �(I �����(I �@ P������PI �-���3�ZYYd�h�oF �E��|�����������_^[��]� ����0   Failed to proceed to next wizard page; aborting.    ����6   Failed to proceed to next wizard page; showing wizard.  h0pF � �  �PpF �q��P�:����\I �SHPathPrepareForWriteA  ����   shell32.dll     qF �qF qF �qF     �qF �  t�H PB �*@ +@ �B H�@ B d�@ �A �B 0B �HA �B �A ,sF ��A �	B  �A @CA �CA �B ��H T�A ��A h|A ��A �B �eA ,B DB B B ,eA \B �~A �-B         |@ �   �qF �    DiskBitmapImage�   SelectDiskLabel�   	PathLabel�   PathEdit�   BrowseButton�   OKButton�   CancelButton  �tF FormCloseQuery puF BrowseButtonClickTNewDiskForm D�E �D VB �mB ��TNewDiskForm�pF �H 8 NewDisk  ��U����SVW3ۉ]�]�ً���3�UhsF d�0d� �(I ���pF ��   �E�3�Uh�rF d�0d� �E��  ��������E�P�U���{����E�E��U�3ɰ�蝒���U�E����  ������E����  � ���j ������E��)���H�E��}� t�U�E��K  �U���q���3�ZYYd�h�rF �E��d�����j�����3�ZYYd�h!sF �E�������E��������E�����E�_^[��]ÐSV��t���������ڋ�3ҋ��T ���JV �=PI  t3ɋPI ����T ����	V ��I ���4����HI ���  �#�����I ���  ������I ���  ������I ���  �����htF �I P�����Ћ��  ���   �eb���  � ���  �)N���VH���  �3N����t
d�    ����^[�DISKIMAGE   U��j j j SVW���3�Uh�tF d�0d� �U��  �4����E�U�������E��U��2V���E����R��3�ZYYd�h�tF �E������E��   蒿����л����_^[��]Í@ U����SVW3ۉ]��]����3�UhauF d�0d� ��(  HtHtp�u�U����E����}� t%�U��E��N���E����  �����E���V����u@� j j j �E�P���  �E��E��E�U�   �M������E��3���  ���) �3�ZYYd�hhuF �E��   �������������_^[��]ÐU��j SVW��3�Uh�uF d�0d� �U�������j �������ȍU���I �O����t�U����  �����3�ZYYd�h�uF �E��/����鍺����_^[Y]Ð    vF                 &vF <  �@ ,,@ �*@ +@ �vF         |@ 8  TFileExtractor�@ �=`I  u3���!I ���
I ��vF �1   �`I �`I Ë��`I �Nk��Ð��I ��Xc@ ���葺���SVW��t���蕶����ڋ�3ҋ�膴���G�����G����Wh(~F ����D 躉���GWh(~F �����G��t
d�    ����_^[�SV�ڋ��F�Z����F�R����F�J���3ҋ��5�����t���J�����^[ÐU��ĴSVW3ۉ]�]�]��]�]�ىU�3�UhPyF d�0d� �U䡔I �P���E�M�3���L���E���!I �����F�E�����U�Iu)�E�P�E�E��E��u��E� �UԹ   �hyF �� ���4�E�P�E�E��E��u��E� �E���a�E��E��U��   �|yF � ���E�P�u��E� �U�3ɸ�yF � ���U�E�M�豽���=dI  t)�U�dI �oK���U�ËM�莽����S������   �U�  I �FK���U�ËM��e�����S������   �=dI  t1�U�dI �K���E�U������E���WN����\S������   �U�  I ��J���E�U�������E���&N����+S����ud�E�  I 赻���E��E��E��U�3ɸ�yF ������M�U���������t'�dI �U��:����U�E��sJ���U�ËM�蒼����;��3�ZYYd�hWyF �E�   �Ѻ���������_^[��]�  ����	   %s-%d.bin   ����   %s-%d%s.bin ����
   ..\DISK%d\  ����)   Asking user for new disk containing "%s".   U����SVW3ɉM����3�Uh�zF d�0d� ;s��   �C�����C�g���=�I  u�M��֋������E���I �g���jj j�M���h�D �~���C�=�I  ue�U��   �C�8�W��t�����E��	I �   �J���t������U�   �C�8�W��t������U�C��Q�E�;E�t������s3�ZYYd�h�zF �E��5����铵����_^[��]Ë�U��QSVW�ىE�3�Uh{F d�0d� �E��@1�M��D��ˋ0�V3�ZYYd���V����E��@0�����a����E���(���4���_^[Y]Ë�U��ĐS�U��E�@��@�   ��S��t�����E��m����U��E��   腈���E�@���8  �����ȋE�@���8  �E��^����U��E������U��E�@���3�   ��)���E�@���3��  ��)��[��]ÐU��P�   �����PHu��E����SVW���}����u��}���3�Uhb|F d�0d� �   �~ u;v���t)�������E�@��������Ƌ������? tʋ3�����3�ZYYd���Ͳ��   �D s|F ���������_^[��]�U����SVW�M��ډE��u��CD@t���8   u
��}F 荜����@;u'��@;Cu��P(�C�e�����|��x0 ��   ��@�����CD�������D���R��@0 ��������S�I ��@�e{���U��@�   �8�W��t�5����E�;	I t�%����CD@tU�����Y���B�C��B�C��B��S�P �S �P$�3҉P,�3҉P(�CD�����B1�CD@����B2��P(�C脅����~#�C�E�C�E���P(�E�菅��U�E�����Y_^[��]�  ����9   Cannot read an encrypted file before the key has been set   SVW����؉T$���{$ u;{ v�{ �<$��tW�T$�ϋC�0�V���C �������{2 tV�L$�T$�C3�4'��;�t"+�t$�C;C|������SB���2�����u��$YZ_^[ÐU��P�   �����PHu��E��ĈSVW��U��E��E��P�U��P�U�U��Ƌ�Q����{��3ҋ��y���E��ۄ��3�Uh�F d�0d� 3��E�   �}� u;]�v�]���te�������ˋE������E��@Dt�E�P������3ɋ�豀��]�E��������������E���萄���������ˋƋ8�W�} t����U�3�ZYYd��镯��   �D �F ������ױ���} t(�������E������������U���$蒎����u����_^[��]� SVW�ļ��3�3ҋ��x���T$�@   �Ƌ8�W��@u>�|$Mu7�|$Zu0�|$@ t)�T$@���Tx���Թ   �Ƌ0�V��u�<$PE  u��Ã�D_^[ÐSVWU����$����3ۋ��y�����tY�T$�   �Ƌ(�U��uD�D$=�   u8�T$�Ƌ��$�T$��T$�P�׹�   �Ƌ0�V=�   u	f�?u��Ã� ]_^[Ë�U������SVW�M������M�������]����؄�t.�}��E��E��E��T����U�Ƌ�Q�������   �Ƌ0�V��_^[��]� ��                        ��F $   �rE ,,@ �*@ +@ PyE ��F l�E p�E TSetupUninstallLog��С(I ����Í@ U��j SVW�ڋ�3�Uh�F d�0d� �E�P�PI ���  �O,�WD����f���U����������t�PI ���  ��RP3�ZYYd�h�F �E�������Q�����_^[Y]Ð�PI ���  ������PI ���  ��RP�3��]����SVWU���3��D$�"I i@�  �$�"I �x t�$�  �"I �x t�$�  ��!I �pN��|WF3��ס�!I �01����j �ˋX"I �P"I �*�  ��t*�k8���t�ա "I �1���P��蘀���
�S@��茀��GNu�3��pI ��ĺ   觀���pI �|$ u�<$   sޡPI ���  �$�6l��YZ]_^[Í@ S����hI �$�lI �D$�ċpI �P����PI ���  �$���l���Ë�RPYZ[Ë�hI �P�lI ����Ð�hI ���������Í@ �иhI ��������Ë��=`"I  t����(I �t����=`"I  t���Ð���������ÐU��j SVW����3�Uhx�F d�0d� 3�jjj�֍E��|����E����F �W����E����M����E��3���  ��tHtHt	��#��������F �������3�ZYYd�h�F �E�良�����������_^[Y]� ����   

    ����:   LoggedMsgBox returned an unexpected value. Assuming Abort.  S�İ��TP�k����D$P�D$P�d�����txS�D$�D$�D$  �D$�D$$�D$( �D$�D$,�D$0 �D$�D$4�D$8 �D$�D$<�D$@ �D$�D$D�D$H �D$�D$L�D$P �T$�   ���F ������ú��F ������P[�   ����"   %.4u-%.2u-%.2u %.2u:%.2u:%.2u.%.3u  ����	   (invalid)   U����SVW�����3�Uh�F d�0d� �M��֋�����VW�u�   �_^�3�ZYYd���`���3��u�����_^[��]�SVWUP�   �����PHu���$   ����<$������hI ���}���T$�ǋ�Q�T$�Ƌ�Q���Rt��3ҋ��r���   �|$ u
;\$v�\$��tu�T$�ˋ��q���T$�ˋƋ(�U�D$���|���hI �D$�lI �D$�D$���|���ԍD$�T|����~�$�D$�D$�D$�D$�x��������q������g�����  ]_^[�SVW����؅�t!�׋�誉�����t�ȁ����΋׋��:���_^[Ë�U��j j SVW�ڋ�3�Uh҇F d�0d� �Ӌ��w?���E�P3ɲ3���  �}� t-�U����?���E��U��=����u�U���g@���U����ݬ��3�ZYYd�hهF �E��   �O����鍨����_^[YY]�TRegisterFilesListRec      |@     �U����S�؍E�P�����U��ù   跬��[��]Ë�U����SVW3��E�E�E��E�3�Uh߈F d�0d� ��!I t�E��@"I �(�����E�芫��jj �E��r����E�E�E�蠰���E�E��E��E�E��d����E��E��M�E�@�f� ����3�ZYYd�h�F �E�   �J����E��"����逧����_^[��]Í@ U����SVW3��E؉E��E��E�3�Uh�F d�0d� �� I �E��    蹫����!I t�U���F �O�  ��E�蹪��3�Uhl�F d�0d� �U���F �*�  3�ZYYd��������E�自������jh �� I �E܋E��E��E��E�E�E�D"I �E�Uظ(�F ���  �E؉E��M܋E�@�f�  ����3�ZYYd�h��F �E������E��   �2�����p�����_^[��]�   ����   {app}   ����   {group} ����
   {language}  ����   RegSetValueEx   ����   RegCreateKeyEx  ����   RegOpenKeyEx    U����SVW3ۉ]�]�]؉]ԉM��U��؋u3�Uhu�F d�0d� �E�P�U�E������E�E�E��E��U�������
I �   �Mz���E����F 萪���E�P�E�P���
I �E܍U؋�������E؉E��Uԋ��8[���EԉE�Uܹ   �4��y���U�X�F����M���Xc@ �W ���ҥ��3�ZYYd�h|�F �EԺ   輨���E�蔨���E�茨���������_^[��]�    ����   

    U��SVW�ً������ɩ��@P��脫��Pjj WV������tP�E�H��E�P�3�����_^[]Í@ U��SVW�ً�����t�EP�ˋ׋�����YK 25
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ^���� I �g���P�� I �,����Ћ�Y�8�W��!I P�� I �B����ȋ֋�������u���F 襻���
���F 虻��_^[� ����   Applying digital signature. ����+   Failed to apply signature: Unexpected size. ����,   Failed to apply signature: Header not found.    ����   Signature applied successfully. U����SV�؋��k]���U�E�@��m����U�   �Ë0�V�E�IMsg�U��Ë��E��E��������U��   �Ë�S^[��]�Software\Microsoft\Windows\CurrentVersion\Fonts Software\Microsoft\Windows NT\CurrentVersion\Fonts  U��j j j SVW�ً���3�Uh՞F d�0d� j j�E�P3��k"I ���
I �  �3��>����u?���O���@P���
���Pjj �������P�E�P蘹����t
��F 螹���E�P�9����
� �F 艹����tO���Ƙ��P������tj j jh��  �����,�M��L�F �3�f���E�U��@F���E��$I �6�����t�3�ZYYd�hܞF �E��   �L����銑����_^[��]� ����*   Failed to set value in Fonts registry key.  ����"   Failed to open Fonts registry key.  ����   AddFontResource U����SVW3ɉM�M�M�U��E��E�誗��3�Uh��F d�0d� �U�E��U'���U�E������U�E��;)���E�迕��P�E�膗��Z�,`���E��E�@��R��N��|aF3ۋE�@�Ӌ�Q;E�uH�M�E�@�Ӌ8�W�E�U���(���E�U��p���u �E�@3ɋӋ0�V�E�@3ɋӋ�S �CNu��E�@�M��U���S�E�@�M��U���S 3�ZYYd�h��F �E�   誓���E��   蝓����ۏ����_^[��]Ë�U����SV�ً�E����tt�֊E��*p�����te�ua�u��E��U�3ɸ,�F �����ӡ�!I �����؋莔���   ���P��ȋ֊E������u�����E��E� �U�3ɸT�F �Ը��^[��]�  ����   Setting permissions on file: %s ����'   Failed to set permissions on file (%d). U����SV�ً�E���t�u��E��U�3ɸ��F �K�����u��E��U�3ɸ(�F �3����ˋ֊E��7s����u�R����E��E� �U�3ɸX�F ����^[��]�  ����$   Setting NTFS compression on file: %s    ����&   Unsetting NTFS compression on file: %s  ����*   Failed to set NTFS compression state (%d).  U���,���SVW3ۉ�l�����@����]�]܉]؉]ԉ]Љ]ĉM�U��E��E��g����E�_���3�Uh�F d�0d� ��F �����E� �E� �E��X8���t�ӡ "I �����E��3��E��E������E� �E� �hI �E�lI �E�E��̐���E��Đ��3�Uh��F d�2d�"3�Uhl�F d�2d�"�E� 3��E��}� t�M�   �E��@Jt�M��E��@J t�M�@�E��@K�t�M��   �E��@Lt�M�   �E��P����E��@N��u�E؋T I �̐���!�} u�U؋E��@讨  ��E؋U詐����l����E���"����l����E�荐����E������E؉�d���ƅh�����d���3ɸ$�F 轵���E�:u"I t�}� t�@�F 袳���
�d�F 薳���E� �=k"I  t0�=|"I    r$�U؊E��>�����t���F �d����E��xN �E��U؊E���n���Eπ}� u
�EψE��E��}� t�M��}� t.�E��@Dt�E��P4�U��P8�U���E�P�E���4P�=������M��U�E�诋���؄�t5��l����E��������l�����d���ƅh�����d���3ɸĴF 趴���
��F 課���}� �'  ��F 薲���E��@Lt�8�F 胲����
  �E�TI �����M��U؊E��"����E��}� t5��l����E��7�����l�����d���ƅh�����d���3ɸl�F �&����
���F �����E��@L��  �E� �}� t!�E��@D���EËE��@<�E��E��@@�E��"��l����E��� ����l����M��E��k���EÀ}� t[�E���D���ƅH��� �E���L���ƅP��� �E���T���ƅX��� �E���\���ƅ`��� ��D����   �̵F �a����
���F �U�����l����E��C ����l����M��E���j������  �E���D���ƅH��� �E���L���ƅP��� �E���T���ƅX��� �E���\���ƅ`��� ��D����   ��F �ղ���}� t�E�;E�w�E�;E�uu�E�;E�vm�E��@L��4
E�uMjjj�U؍�l����6�����l����L�F ������l�����I �������l����3���  ���	  �\�F �O����  �E�;E���   �E�;E���   �E��@K��   �E��@L@��   ��p����U؊E��������to�}� t�E��p$�}��   ��*�E�XI 职���M��U�E��׈���E�TI �e����U���p����/f����t���F 襯����  �̶F 薯���D��F 芯���8�E��@J�u�P�F �u�����  �E���p�F �`����}� u
�E���E��}� ��   �E��@J���   ��t�}� u���F �%����t  �E�P�E�P������u�ȷF �����T  �E�P�E�P������~i�E��@L��4
E�uIjjj�U؍�l����n�����l����L�F �F�����l�����I �5�����l����3����  ��t��F 苮����  �E�莊���}� t�$�F �n����  �E��@JtXjjj�U؍�l����������l����L�F �ȋ����l�����I 跋����l����3��l�  ��t�p�F �����\  �U؊E���f���؃����   ����   �E��@Ku!��I �E��������t���F �ĭ���  �E�(I �U����˃���U؊E��5j����t��F 蓭���
�0�F 臭���E��@Ku#�q����E��@K t�}� u�`�F �`����  ���F �Q����E܋U������E�:u"I ud�}� uT��l����E��P8�E��S��l��� t7��@����E��P8�E��S��@����E��h����t�M܋E��P8�E��S�}� �E���E� �E�4I �i����E�P��l����E�������l������F �E��\r���EP��l����E��*����l������F �E������Y�E�Pj jj �M���,E ��i���E�3�UhJ�F d�0d� �E�3�Uh��F d�0d� �E��E�XI �ψ���}� uD�h������F �U������E�,I 詈��h��F �E��@L���4P�6����U��M������|�E�Pjj j�M���,E �Pi���E�3�Uh��F d�0d� �E�,I �M����}� t�E��H�U��E��������M�U��E�����3�ZYYd�h��F �E���~����ۃ����3�ZYYd���ց���E� �E������荄���܄���E�Pj j �E��@P� ����E��xNug�E� �InUn�E�������E�@���X
I ��d���ƅh�����d���3ɸ̹F ������!I  t
�E�������=�I  u�EP�E������Y3�ZYYd�hQ�F �E��~����������E��x t'�E�螆���UЋE�������}� t�E�轉��P�c����}� ��   �E��xN��   �E�xI �����   �   �o�������   ��t	�� ��   �U��BJtQ�=n"I  tH��d���ƅh��� ��d���3ɸ��F �����E��I 艆���r"I �M؋UԊE���s���E��X��~1��d���ƅh��� ��d���3ɸL�F 觫��Kh�  萬���_����P�k������F �-l���U؊E��`�����2����}� u�E��xN��"E�tY�E��xNu�E����U������E� �E��J������F �0����}� tU�UԋE��@8����Y�E��H<�UԊE��5����   �E�tI 蚅���M؋UԊE��tc����u
�ԺF �k���E� �E��ބ���E��ք����F 輨���}� tU�U؋E��@8����Y�E��@Jt3ɊM��Uء�"I ��S0�E��H<�U؊E������E��x tE�E��y����E��@��d���ƅh�����d���3ɸ�F �D����Mʀ��E��P�E������M��E��4����E��xNtV�E��@Juf�E��@J@u]j�E�P�E؉�0����Eԉ�4����E��@��8����EЉ�<�����0����E�@�f�� ������E�x� u	�E�@���E�@��E��@Ju	�E��@J t~�U؊E��;c����to�E�蓃���E��@J t�@�F �p����
�x�F �d������F �   �����؋ËU�詃���E��C�E��@J ���C�E��@K@���C�E�@���d���E��@J@��   �E������}� t���F ������M΋Uذ��x�����F �ަ���M΋Uذ��x���E��@Jue�M��}� t�M�   �E��@Mt�M�   j�E�P�E؉�0����Eԉ�4����E��@��8����EЉ�<�����0����E�@�f�� �I����L�}� t$j j�E؉�,�����,����E�@�f�� �����"j j �E؉�,�����,����E�@�f�� ������E������}� t�E��HH�UԊE�������E��HH�U؊E������E��@M �Ä�u	�E��@M@t>�E��ԁ���}� t�E��@M ���ˋUԊE��3�����E��@M ���ˋU؊E�����3�ZYYd��*��{���"�����c@ � y����t�~���E��2����~��3�ZYYd�h��F �}� t�UԊE��P\����}����}� ��   �E��xNu&�E�x� t�E�@�������E��������E�@� �}� t�E��F �t����U؍�l����~�����l����L�F �V�����l����U��H�����l����U��:�����l�����I ������u�}� ������E����������}� u�}� t�E���������E�����$���3�3�����3�ZYYd�h��F ��@����K�����l����@����E��8����Eк   �K����E��#����E������E������q|���_^[��]�   ����   -- File entry --    ����   Dest filename: %s   ����   Non-default bitness: 64-bit ����   Non-default bitness: 32-bit ����2   Dest file is protected by Windows File Protection.  ����   Time stamp of our file: %s  ����(   Time stamp of our file: (failed to read)    ����   Dest file exists.   ����)   Skipping due to "onlyifdoesntexist" flag.   ����   Time stamp of existing file: %s ����-   Time stamp of existing file: (failed to read)   ����    Version of our file: %u.%u.%u.%u    ����   Version of our file: (none) ����%   Version of existing file: %u.%u.%u.%u   ����   

    ����+   Existing file is a newer version. Skipping. ����3   Existing file's MD5 sum matches our file. Skipping. ����?   Existing file's MD5 sum is different from our file. Proceeding. ����3   Failed to read existing file's MD5 sum. Proceeding. ����   Same version. Skipping. ����    Version of existing file: (none)    ����#   Couldn't read time stamp. Skipping. ����   Same time stamp. Skipping.  ����/   Existing file has a later time stamp. Skipping. ����@   Existing file is protected by Windows File Protection. Skipping.    ����8   User opted not to overwrite the existing file. Skipping.    ����J   User opted not to strip the existing file's read-only attribute. Skipping.  ����   Stripped read-only attribute.   ����$   Failed to strip read-only attribute.    ����,   Skipping due to "onlyifdestfileexists" flag.    ����   Installing the file.    ����   .tmp        ����&   Uninstaller requires administrator: %s  ����E   The existing file appears to be in use (%d). Will replace on restart.   ����6   The existing file appears to be in use (%d). Retrying.  ����
   DeleteFile  ����(   Leaving temporary file in place for now.    ����   MoveFile    ����    Successfully installed the file.    ����!   Registering file as a font ("%s")   ����.   Will register the file (a type library) later.  ����)   Will register the file (a DLL/OCX) later.   ����(   Incrementing shared file count (64-bit).    ����(   Incrementing shared file count (Extreme Tux Racer - Version 0.4

go to extremetuxracer.com for more info                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      @�E��E�    �U��"I �����E��E��@P�E��@P�E��@P�E��H�X"I �P"I �^V  ����  �U��
���  �E��@$��R  �U��E��@��  �U�E��@��  �U��E��@��  �U�E�� ��  �}� ��   �E��@<��   �E��@<t�M�U�E���������   3ۍU�E������}� td3�Uh�F d�0d� �EP�d�F �U�3��g���Y3�ZYYd��73�ZYYd��Sa���E������E܋$I 蹶����t	��Nd����Gd��뜄�u7��M܋U�5�8���E܋$I 脶����u�E�P�M��U�E�������tˋE��@<t"jj �E�EԋE��E؍MԋE�@�f�� �'����E��@<t"jj�E�EԋE��E؍MԋE�@�f�� ������E��@<t.�}� t(jj �E�EȋE��E̋E�EЍMȋE�@�f�� �ū���E��@ �JQ  �E��M��������  �`���3�ZYYd�hZ�F �E��e���E�   ��e����b����_^[��]�       SV��3ۋ���h���
��\t��@���u���^[�U����SVW3ۉ]����E��]3�Uh`�F d�0d� �����   �U����L���E�E��E��}��E��U�   �x�F �����ӡ�!I ������؋P��tf���   ���P�ϋ֊E�������u1������u���F �È���������E��E� �U�3ɸ�F 覊��3�ZYYd�hg�F �E��d�����`����_^[��]� ����*   Setting permissions on registry key: %s\%s  ����R   Could not set permissions on the registry key because it currently does not exist.  ����/   Failed to set permissions on registry key (%d). U��ĬSVW3��E��E�E��E�E�3�Uh	�F d�0d� �"I �@H���c  @�E��E�    �Uء"I �K����E̋E̋@P�E̋@P�E̋@P�E̋H�X"I �P"I ��Q  ���	  �Uذ�O�  �E̋@ �dN  �U��E̋ �{  �U�E̋@�{  �DI �EߋE��@@t�E��E��@@t�=l"I  u
� �F � H���E��E� 3�Uh��F d�2d�"�E��@?@t$�E��!�����t�E���e���ȋE̋P8�E��u���E��@?@t�Èx> �,  �E��@?�tQ�Èx> uHj j�E�P�E��e���ȋE̋P8�E��
������  �E��}e��P�E�P�����E�P�ƅ����  �E��@<P�M��E̋P8�E������E��@@uqj j j jj �E�P�E�P�E��)e���ȋE̋P8�E���	���؅�u%�}���   �E��@<P�M��E̋P8�E��D����q�E��@@uhS�M��E̋P8������U�Èx> tGj j�E�P�E��d���ȋE̋P8�E��	���؅�t&��t!�E��@@uS�M��E̋P8��ȷ����   ����  3�Uh��F d�2d�"�E��@?�t�E��Vd��P�E�P������Èx> ��  �E��@?t�E��.d���ЋE��h�����k  �E̊@>��J��r��  ���  ���G  �   �Ѐ�t��t	��   ��   <��   �E̋P�h�F ��d����t�E��c���ЍM�E�������u�E��d`���E��@? t/j j �E�Pj �E��c��P�E�P������u�}�t�}�u�]�E�P�|�F �EċE�EȍUĹ   �E̋@�x  �   �E̋P�h�F �Nd����t�E��c���ЍM�E��I����u�E���_���E�P�|�F �E��E�E����F �E����F �E��U��   �E̋@�Dx  �}� t�E���`���U��|� t�E��F ��`���E���`��@P�E��b��PSj �E��b��P�E�P�'����؅���   �E��@@��   S�M��E̋P83�蟵���   �U��E̋@�w  �E��0����E�j�E�Pjj �E��+b��P�E�P�ł���؅�tj�E��@@uaS�M��E̋P83��E����N�E̋@�0`��P�E̋@��a��Pjj �E���a��P�E�P�u����؅�t�E��@@uS�M��E̋P83������3�ZYYd�h��F �E�P��������Z����3�ZYYd��(�X���E��h���E��$I �"�����u�E��[���}� �C����E��@?t1�E��v�����t%�E̋@8P�E��E��E�Pj �E�@��M�f�� �I����E��@?t1�E��<�����t%�E̋@8P�E��E��E�Pj �E�@��M�f�� �����E��@?t+�E̋@8P�E��EċE�EȍE�Pj�E�@��M�f�� �ۤ���E��@?t+�E̋@8P�E��EċE�EȍE�Pj�E�@��M�f�� 觤���E̋@�H  �E��M��������  責��3�ZYYd�h�F �E��]���E��]���E�   �]����VY����_^[��]� ����=   Cannot access 64-bit registry keys on this version of Windows   ����	   {olddata}   ����   olddata ����   break   ����       U��P�   �����PHu��E����SVW3ɉ������M��U���3�Uh+�F d�0d� �E�P�D�F ��3���E��3��E�jj j������3�������������h�D � ���E�3�Uh��F d�2d�"j jj �M���h�D � ���E�3�Uh��F d�2d�"�������E��Q�������E���Q�E��S"��3ҋE�� ���������   �E��S��t�������ȋE���S�׺InRS�E������3�ZYYd���U���E������E������cX���X��3�ZYYd�h��F �E��iR���E��aR����gW����E��U��[��3�ZYYd�h2�F ��������Z���E���Z����4W����_^[��]�   ����   .exe    U��QSj jj �Ȳ�h�D �_���E�3�Uh��F d�0d� �@I ��!I �N����؋C��[��P�C�]���ЋE�Y��S3�ZYYd�h��F �E��Q����V����[Y]Í@ U����SVW3��E�E��E�E�3�UhO�F d�0d� �=n"I  tD3�Uh�F d�0d� �U��( I ����3�ZYYd��2�PT���E�� ����E�U��m����TW����E������E�U��S����E��E��E��U�3ɸh�F ���3�Uh��F d�2d�"�M五�F �E������E������j jj �M交�F �E������M���D ����E�3�Uh��F d�0d� ���F �E��Q"�����F �E��Y���E��"I �Z���U�E��+"�����F �E��Y���E��I �lZ���U�E��"��3ҋE���!����F �E���!��3ҋE���!���E�@�@�pN��|dF3��E�@�@���t����؍E��`�F �OZ���E���[��3ҊS3ɊK�M�
I ��P�{ t�E��[���@q�U�E��s!��GNu�3�ZYYd�h��F �E��O����T�����=n"I  t�  ���  �j j j jj �E�P�E�P�h�F ��3���K 25
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             �  �-���3ۍ�����8 u7U�������Y�E�@䉅 ���ƅ����� ���3ɸT�F �m��U�����Y�1C@���  u�������U��E�����������Xc@ �"����D��3�ZYYd�h��F ������   �G���E��\G����C����_^[��]� ����!   Directory for uninstall files: %s       ����)   Will append to existing uninstall log: %s   ����)   Will overwrite existing uninstall log: %s   ����   Creating new uninstall log: %s  U��3�QQQQQQQSVW����3�Uh`�F d�0d� �k�����5T I hx�F 3����4�tI hx�F �E�P�}�U���0���E�E��U����x����E��U������E�E�U�   �4�4���u��E��   �,H��jjj�3ҋE��&�  ��t	��u��3�����F �i��3�3�ZYYd�hg�F �E�   ��E���E��   ��E�����A���ދ�_^[��]�����   

    ����;   LoggedMsgBox returned an unexpected value. Assuming Cancel. U��SV�T I �u����> tY��@�F �A������u耜���������t���T�F �@�k�����u�^�����H��P��H��P�%k����tҋ���D��^[]�  ����
   DeleteFile  ����   MoveFile    U��Q�E�x� tp��!I  u	�=�I  t^j jj �E�H��h�D �-	���E�3�Uh��F d�0d� �E�x�u�E�@��E��D���3�ZYYd�h��F �E��;����@����Y]�U����SVW�=�"I  ��   ���F ��"I �3� ����   �=r"I  up3�Uhd�F d�0d� j j j 3��E��E��M����F ��"I ��� ��t�r"I ���F �g��3�ZYYd��,�>�����F �}g��3ҡ(I ��U���A���
��F �`g��_^[YY]� ����   NeedRestart ����/   Will restart because NeedRestart returned True. ����    NeedRestart raised an exception.    ����L   Not calling NeedRestart because a restart has already been deemed necessary.    SVW��!I �pN��|>F3��ס�!I �M�����j �CPj �3ҡP"I �1  ��t�C5t	�r"I �GNu�_^[Ë�SVW��!I �pN��|@F3��ס�!I ������؋P�CPj 3ɋX"I 3��0  ��t�C1t	�r"I �GNu�_^[�U��ļ���SVW3҉Uȉ������U��U�U��U��UЉEػp I 3�Uh��F d�0d� �E��  ���F �e���, I �����ݏ���E� �E� �E� �E� �o"I �E��E� 3��E��t�F 耆���E�3�Uh��F d�0d� 3�Uh��F d�0d� �U��C��Y  �}� u
�(�F �n&���E��B����~
�`�F �W&���UЋE��@)  �E��m"I �P�E�����"I �?A���E����U��1A���=l"I  t�E��H �=n"I  t	�E��H��=o"I  t�E��H@�E��H��,  t�E��H��-  t�E��HU貔��YU�k���Y��\�@ �7���E�������������������z��������=�"I  tj j ��"I �E̍M�f�� �E��������*  tj j 3��E̍M�f�� �E��܅��U�"���Y贏����I ����U耥��Y螏����(  t�I ����U�E���Y� I �ލ��U����Y�n����=j"I  t��I 迍��U�}���Y�O����"I �x t��I 融��U�@���Y�.����"I �x t��I �}���U����Y����3��j����)���������E�x t�I �M���U�[���Y�ݎ����(  t�I �.���U�����YU�e���Y��*  t
U�E�蒗��YU�����Yj j �E������EȉE̍M�f� �E�蝄����*   ��P�MދU�E�� ����=�I  t
�T I �Z��3�貌���E��E��j���3�ZYYd��  ��8��3�Uh�F d�0d� �������c@ ��5������   �E�P����� ��������4���������E��E��U�3ɸ��F 襂���E�P�������6���������X�?���E���a����"I    3ҡ(I �@P��j jj�3ҡ�I �}  ����F �a����"I    �}� ��   ���F �a��3�Uh��F d�0d� �I 讋���PI ���  3��\S���PI ���  3��S���PI ��RP3�ZYYd��
��7����:���}� t�E��"x���}�u
�T I �x���}� t�E��x���}� t�E���w��3�3ҋE��v����PI �x7 t
h�  ��c��3�ZYYd���V7��3ҡ(I �2O���a:���\:���:���f�P:��3�ZYYd�h��F �}� t.�E�XK�� |�ӋE��o������F �%D��K���u�E���3���E���3�����8��뼸�F �Y`���E�� 3�ZYYd�h��F �������K<���E��C<���E��;<���E�   �N<���E��&<���E��<����|8����_^[��]�   ����"   Starting the installation process.  ����.   Failed to get a non empty installation "AppId"  ����$   "AppId" cannot exceed 127 characters    ����3   Fatal exception during installation process (%s):
 ����'   User canceled the installation process. ����   Rolling back changes.   ����   Installation process succeeded. SV��؋Ƌ��I;���   �/��D�<{u�֋˸��F �>��C����I %�   �sCC���;��;�~�^[� ����   {   U����SVW3ۉ]�]�ىU���3�Uh��F d�0d� �u"I �E��U�$ I �����U�E����;���E�Pj jj �M���,E �{���E�3�Uh��F d�0d� 3�Uh��F d�0d� �|����3��Z���j �E��@L���4P��{���ӋM��d����CDt�C4�E�C8�E���E�P�C4P��_���E�Pj j �E��@P�G`��3�ZYYd�h��F �E���0�����5����3�ZYYd����3���U�E��P���6����6���E��H<�U�E��M���3�ZYYd�h��F �E��9���E��9����r5����_^[��]ÐU����SVW3҉U�U��E�3�Uh��F d�0d� �U��E�������!I �pN��|PF3��ס�!I 菸���؃{8�t5�U�C�����E�U��n����u�S8� "I �`����ȋӋE������)GNu��E�P�E��E��E��U�3ɸ��F ��|���E����3�ZYYd�h��F �E��   �[8����4����_^[��]�����1   ExtractTemporaryFile: The file "%s" 8

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

                                                                                                                                                                                                                                                                                                                                                                                    �˺�,G �H���S�U��X"I �����M��h,G ��,G �@���3�ZYYd�hI,G �E����������_^[Y]� ����   Lang    ����   Setup   ����   Dir ����   Group   ����   NoIcons ����	   SetupType   ����
   Components  ����   Tasks   SVWU���T$�$�D$ ��!I �pN��|DF3��ס�!I �ц���؋$��=����u!�k���t;-@I u�D$�D$�S���GNu��D$��]_^[�U����SVW3҉U���3�Uh�-G d�0d� �E�P�]��E��U�3ɸ�-G �K���E�����3�ZYYd�h�-G �E��g���������_^[��]�����.   Cannot evaluate "%s" constant during Uninstall  U��3�QQQQSVW��E��E��/	��3�Uh�.G d�0d� �E��   �   �	���U��|葨���؅�u�E��C����C�E�P��I�   �E��0	���E�P�S�����E��	��������E��E�����tH�E��9�����t<�E�P�E�H��E�P��E��  �E����?����> uV�E�H��E�P��E���  3�ZYYd�h�.G �E�   �O��������_^[��]�����   HKCR    ����   HKCU    ����   HKLM    ����   HKU ����   HKCC    U����SVW3ɉM؉M�M��M�M�U��E��E�����3�Uh�1G d�0d� �E��   �   �D���U��\�6��������  �E�P��I�   �E������}� ��  �DI �E�E�����؃�|_�E�|�3u�E�|�2u�E��Ӄ��E������8�E�|�6u.�E�|�4u$�=l"I  u
��1G �5����E��Ӄ��E����3��E�   �LI �U���9����u�C�E����Ou�}� �C  �E�P�V�����E�����U��|�E�������u�E�������F�E�P�V�����E�������J�E�����U��,����������   �E�P��I�   �E�����E�P�V�����E�����E��ͤ������   �E�轤������   �E�譤������   �E�P�E�H��E�P��E��  j j�E�P�E�P�E�H��E�P��E��i  �E������ȋU��E��ܪ����uB�E�P�E�H��E�P��E��8  �E�����ЋM��E��۩���E�P�&���
�42G ����3�ZYYd�h�1G �E��M���E�   �`���E��8���������_^[��]� ����I   Cannot access a 64-bit key in a "reg" constant on this version of Windows   ����   Failed to parse "reg" constant  U��3�QQQQQQQSVW��E��E����3�Uh]4G d�0d� �E��   �   �(���U��,�����؅���  �E�P��I�   �E������}� �v  �E�U������E�P�S�����E�����U��|�ɣ���؅�u�E��{����C�E�P�S�����E��h����J�E�����U��,菣���؅��	  �E�P��I�   �E��5���E�P�S�����E��!���E��Q�������   �E��A�������   �E��1�������   �E��!�������   �E�P�E�H��E�P��E���  �U�E��� ���E�P�E�H��E�P��E���  �U�E�� ���E�P�E�H��E�P��E��  �U�E�� ���E�P�E�H��E�P��E��  �U�E��q ���E�PV�M�U�E��2����
�t4G �����3�ZYYd�hd4G �E�   ������������_^[��]� ����   Failed to parse "ini" constant  U����SVW3ۉ]��]�M��U���3�Uht5G d�0d� ����������   �   �U���G������ ����P��5G �E������E���� ���E�5G �t ���E��(��P�E����ZY��>����u&���I ���ȃ��E��   �{���E��U��L����GNu��E��U��;���3�ZYYd�h{5G �E�   ������������_^[��]�  ����   /   ����   =   U��3�QQQQQSVW��E��E��r��3�Uh�6G d�0d� �E��   �   �����E��U������U��|�ɠ���؅�u�E��{�����C�E�P�S�����E��h����J�E�����E�U��n����E�肟����ta�E��v�����tU�E�P�E�H��E�P��E��V  �U�E��3����E�P�E�H��E�P��E��3  �U�E������΋U��E������
��6G ����3�ZYYd�h�6G �E�   �i����������_^[��]�  ����    Failed to parse "param" constant    U����SVW��ڋ��=�"I  tj jSV�]��E��M��ס�"I �x� ��L7G �����������_^[YY]�  ����"   "code" constant with no CodeRunner  U��3�QQQQQSVW��E��E�����3�Uh�8G d�0d� �=�"I  t��8G ��Xc@ ��S���e����E��   �   ������E��U�������U��|�֞���؅�u�E�������C�E�P�S�����E��u�����J�E�� ���E�U��{����E�菝����t>�E�胝����t2�E�P�E�H��E�P��E��c  �U�E��@����΋U��E������
��8G �����3�ZYYd�h�8G �E�   ������������_^[��]�  ����@   Cannot evaluate "code" constant because of possible side effects    ����   Failed to parse "code" constant U����SVW3ɉM��ډE��E�����3�Uh�9G d�0d� �E��   �   �q����E��a�����t$�E�P�E�H��E�P��E��A  �E����?����
��9G ����3�ZYYd�h�9G �E��   ������������_^[YY]�  ����    Failed to parse "drive" constant    U����SVW3ɉMЉM����E��E��2����	   �EԺ|@ �x���3�Uh�;G d�0d� �=t"I  t
��;G �����E��   �   �}����U��,�o�������u�E��U��>�����E�P��I�   �E�����3���E��κ   �9����U��,�+�������u�D�ԋU��������D��P��I�   �E�������D���������u
��;G �w����E�P�E�H��E�P��D����  �UЍD������C��~	���t����׋E��������u%�E�P�E��E��E��U�3ɸ�;G �]=���E������E�P��������I�U�������UЋ��E���3�ZYYd�h�;G �E������EԹ	   �|@ ������E��   ������������_^[��]�  ����   cm  ����   Failed to parse "cm" constant   ����1   Unknown custom message name "%s" in "cm" constant   ����   userdesktop ����   userstartmenu   ����   userprograms    ����   userstartup ����   sendto  ����   fonts   ����   userappdata ����   userdocs    ����   usertemplates   ����   userfavorites   ����   localappdata    ����   commondesktop   ����   commonstartmenu ����   commonprograms  ����   commonstartup   ����   commonappdata   ����
   commondocs  ����   commontemplates ����   commonfavorites ����   src ����   srcexe  ����   userinfoname    ����   userinfoorg ����   userinfoserial  ����   hwnd    ����
   wizardhwnd  U����SVW3ۉ]�]�M��U���3�Uh�FG d�0d� �=t"I  t"�   ��I �������u��������Nu�Ǻ�FG �����u�E��FG �������  �Ǻ�FG �����uU�=t"I  t&�=\ I  u
��FG �x����E�\ I �����  �=@"I  u
�8GG �R����E�@"I �����  �Ǻ�GG �w���u�E�( I �w����f  �Ǻ�GG �V���u�E�, I �V����E  �Ǻ�GG �5���uB�=0 I  t�E�0 I �,����  �=l"I  t
��GG ������E�, I ������  �ǺHG �����u�E�  I �������  �ǺHG �����u�E��I ������  �Ǻ(HG ����u�E�$ I �����  �Ǻ4HG ����u�E�4 I �����q  �Ǻ@HG �a���u/�=m"I  t�E�@ I �X����G  �E�8 I �E����4  �ǺLHG �$���u/�=m"I  t�E�D I �����
  �E�< I ������  �ǺXHG �����u�E�8 I �������  �ǺhHG �����u�E�< I ������  �ǺxHG ����u+�=l"I  t�E�@ I �����  ��HG �9����|  �Ǻ�HG �l���u+�=l"I  t�E�D I �c����R  ��HG � ����C  �Ǻ IG �3���u�U�,IG �,  �#  �ǺPIG ����u�E�H I �����  �Ǻ\IG �����u�E�X�����  �ǺtIG �����u�E�y�����  �Ǻ�IG ����uU�=t"I  t&�=d I  u
��IG �Z����E�d I �����  �=D"I  u
��IG �4����E�D"I �z����i  �ǺPJG �Y���u�E�L I �Y����H  �ǺhJG �8���u�E�P I �8����'  �Ǻ�JG ����u�E�4"I �����  �Ǻ�JG �����u�E�8"I �������  �Ǻ�JG �����u�E�<"I �������  �Ǻ�JG ����u�E�T I �����  �Ǻ�JG ������   �=t"I  t&�=` I  u
��JG �-����E�` I �s����b  �=D"I  u
�@KG �����E�P��!I ���43ɲ��  �}� u
��KG ������U�E��'����U�E�D"I �B����  �Ǻ�KG �����u;�=t"I  t�E�h I �������  �@I ��!I �p����E������  �Ǻ�KG ����u2�=�I  t��I �=���U�)���  �E��KG �����|  �Ǻ�KG �l���u2�=PI  t�PI �K=���U��(���N  �E��KG �M����<  �ǺLG �,���u�E�^���!  ��t!�?%uU�U���e���Y�U�E������  �������؋úLG �   �/����uU�U���w���Y�U�E�������  �úLG �   ��.����uU�U���n���Y�U�E�����  �úLG �   �.����uU�U������Y�U�E�q����`  �ú$LG �   �.����uU�U���(���Y�U�E�@����/  �ú,LG �   �V.����uU�U������Y�U�E������   �ú4LG �   �%.����uU�U���.���Y�U�E�������   �E� �tI �E� ��ǋ����uF�E�P3ɊU�E��F  �}� u"�E�P�}��E��U�3ɸ@LG �~2���E��2����E�U��{����m�E���}�u��E��,�}�u���t/3�;u�}(�ǋU����9���u�E�U��T��8����*��;u�|؍E�P�}��E��U�3ɸtLG �2���E�����3�ZYYd�h�FG �E��c����E��[����������_^[��]�   ����   \   ����   app ����V   An attempt was made to expand the "app" constant but Setup didn't create the "app" dir  ����J   An attempt was made to expand the "app" constant before it was initialized  ����   win ����   sys ����   syswow64    ����H   Cannot expand "syswow64" constant because there is no SysWOW64 directory    ����   src ����   srcexe  ����   tmp ����   sd  ����   pf  ����   cf  ����   pf32    ����   cf32    ����   pf64    ����8   Cannot expand "pf64" constant on this version of Windows    ����   cf64    ����8   Cannot expand "cf64" constant on this version of Windows    ����   dao ����   {cf}\Microsoft Shared\DAO   ����   cmd ����   computername    ����   username    ����	   groupname   ����O   Cannot expand "groupname" constant because it was not available at install time ����P   An attempt was made to expand the "groupname" constant before it was initialized    ����   sysuserinfoname ����   sysuserinfoorg  ����   userinfoname    ����   userinfoorg ����   userinfoserial  ����   uninstallexe    ����   group   ����K   Cannot expand "group" constant because it was not available at install time ����L   An attempt was made to expand the "group" constant before it was initialized    ����!   Failed to expand "group" constant   ����   language    ����   hwnd    ����   0   ����
   wizardhwnd  ����   log reg:    ini:    param:  code:   drive:  cm: ����+   Failed to expand shell folder constant "%s" ����   Unknown constant "%s"   SVQ���V3��D$�T$3ɋ��   Z^[ÐU����SVW3ۉ]�]��M��U��؋u3�Uh�MG d�0d� �Ƌ������   ��   ��|�{��   ��w���;�}��<{uC�ƹ   �������   ����������؅�u
�NG ����K�E�P�G��+ȍW��*����E�P�U��M��E�������K+ϋƋ��J����֋ϋE������E��������߃}� tI�E���z���8\u<������;�1��|�\u(�ƹ   ����������D���I %�   �sCC�����;�����3�ZYYd�hNG �E�   �"�����`�����_^[��]�  ����   Unclosed constant   U��QSVW�����j j�E�P��NG �  ���������u"�΋׋E�������u�������E�P�	���������_^[Y]�   Software\Microsoft\Windows\CurrentVersion   Software\Microsoft\Windows\CurrentVersion   SOFTWARE\Microsoft\Windows NT\CurrentVersion    U��QS�=l"I  t��3�j j�E�P3��k"I ���I �  ���������u-�L I �|OG �E������P I ��OG �E�������E�P���[Y]� RegisteredOwner RegisteredOrganization  U��j SVW3�Uh�QG d�0d� �E�誇���U��( I �����E�������U��, I �l����E��؇���U��0 I �W����=� I u�U���QG �̀���U��4 I �4����
�4 I ������=4 I  u2�U��( I �vw���U��4 I �����=4 I  u�4 I ��QG ������M���QG ������U��8 I ������=8 I  u�8 I ��QG �4 I �#����M��RG ������U��< I �����=< I  u�U��8 I ��r���U��< I �RG ������=l"I  t^�M���QG ��3����U��@ I �J����=@ I  u
�4RG �'����M��RG ������U��D I �����=D I  u
�tRG ������=k"I  t!�U��, I �6r���U��H I ��RG �P�����U��( I �r���U��H I ��RG �/����z���3�ZYYd�h�QG �E��U����������_^[Y]�   ����   SystemDrive ����   C:  ProgramFilesDir ����   \Program Files  CommonFilesDir  ����   Common Files    ����4   Failed to get path of 64-bit Program Files directory    ����3   Failed to get path of 64-bit Common Files directory ����   cmd.exe ����   COMMAND.COM U����SVW3ɉM��M�M�3�Uh�SG d�1d�!3�UhSG d�1d�!�'|��3�ZYYd��l�$���   @�@ SG ����؍E�P��I �E��U������E�E�U�������E�E��U�   �`�α���M���Xc@ �/8����������3�ZYYd�h�SG �E�   �����E��g�����������_^[��]�U��QS��Pj
�I ��P�@ �}���E�3�Uh�SG d�0d� �ӋE������3�ZYYd�h�SG �E��d�����j�����[Y]�U��   j j Iu�SVW3�UhsUG d�0d� �E�������U�$ I ������UG �E��R����E��$ I �,����E�����=�I  t
�$ I �}����U�$ I �o���U��E���UG �8���j �E�����P�l����ua����؍E�P�M��U��/訰���E��E�U܋��4���E܉E�U؋��|����E؉E�U�   �`�C����M��Xc@ �6�������E���UG �U������U���UG �����E������}� t#�E���UG �U������U�E��e����E��-��3�ZYYd�hzUG �Eغ   �����E�   ������������_^[��]�   ����   Created temporary directory:    ����   _isetup ����   \_RegDLL.tmp    ����
   REGDLL_EXE  ����   \_setup64.tmp   SVWQ����$�֊$����������ۄ�u+������t!������t�b��+�=�  s	j2���뿋�Z_^[ÐU��j SVW3�Uh�VG d�0d� 3����3�� ���=$ I  tW�=�I  t3��W���jjj h VG ����P��$ I 3��'�����u#�WG �E�������E��$ I �����E����3�ZYYd�h�VG �E�������q�����_^[Y]� ����&   Failed to remove temporary directory:   U����SVW3��E�E�3�UhgXG d�0d� �U�$ I �<l���U�E���XG �X����U���XG �3����U�E��T�����u
��XG �����U���XG �9�����t�E�;E�w �E�;E�u�E�;E�w�E�;E�u�E�;E�u�E���XG ������ �  �YG ����� �  �E�������(#I �=(#I  u%�E�P�E��E��E��U�3ɸYG �i ���E�����h0YG �(#I P�1���؉,#I ��u
�LYG �����3�ZYYd�hnXG �E������E�������������_^[��]�   ����   _isetup\_shfoldr.dll    ����   SHFOLDERDLL ����-   Failed to get version numbers of _shfoldr.dll   ����   shfolder.dll    ����   shell32.dll ����   Failed to load DLL "%s" SHGetFolderPathA    ����2   Failed to get address of SHGetFolderPathA function  3��,#I �=(#I  t�(#I P�����3��(#I ÐU�������SVW3ۉ���������������3�Uh�ZG d�0d� ��t�� �  �|"I ��=   u'�� �  t������Pj j ��%���Pj �,#I ����  ���t������Pj j Vj �,#I �؅�u6�������������  �����������������l�����������-p���6������������ƅ���� ������ƅ���� �������   ��ZG �Q���3�ZYYd�h�ZG �������   �d����������_^[��]� ����A   Warning: SHGetFolderPath failed with code 0x%.8x on folder 0x%.4x   U��j SVW��3�Uh�[G d�0d� �E�x�3���k���#I �<8 u^�E���I ���U�R����M��<����E�@����"I ���U�������E�x� t�}� t�E�@�3Ҋ�k���#I ��E�@�U�R�3Ɋ�k����"I �������3�ZYYd�h�[G �E�� �����~�����_^[Y]Ë�U��QS�M��U��؋E�����U������Y�E�8 ����t	U3������Y[Y]� ��U��QS�M��U��؄�t�=n"I  t	�=k"I  u�E��,r3ۋEP�M��U�������[Y]� �@ U����S�|"I �M��=k"I  t�H�M��J�M��
��M��
�M�3Ƀ}� u��q�]�;]�r �=k"I  tf�]�f;]�uf�@f;�"I v��E�}� t?�E�f�}� u%  ���=k"I  u	;E�r"��;E�wf�E�f;E�uf�Bf;�"I w���[��]Í@ U����SVW�M��؋E��
��J�H��Ц@ ������E�3�Uh ^G d�0d� �ӋE���Q,��!I �xO��|oG�E�    �U���!I �DV�����ރ{ tJ�{ uD�{ u>j��3ҋE��0�����t,�s8���t�֡ "I �V���P�E�蝥����S@�E�营���E�Ou�3�ZYYd�h^G �E��Y�����_�����_^[��]Ë�SVWU���L$�T$�$�D$3҉P�D$3҉��Ц@ ��������!I �xO��|bG3�ա�!I �wU���؋S���C����|$ t"�C5t6�$��������t(�S6�D$������$���Զ����t�S6�D$�Ȥ��EOu���������]_^[ÐSV�؋3��   t*��   u"�_G �C,�����t�_G �C,�����u3�^[ð^[�  .   ..  U��j SVW���3�UhP_G d�0d� �U����i���U��Ƌ�Q,3�ZYYd�hW_G �E������������_^[Y]Í@ U����SVW3҉U��E��E�����3�UhM`G d�0d� �z����t!�=u"I  u.�U��E������U��E��������U��E��\w���U��E�������U��E��i���U��E������E�@���R��K��|AC3��M��E�@��֋8�W�E��U��w���u�E��E��E��U�3ɸh`G �������FKu�3�3�ZYYd�hT`G �E��   �������������_^[��]�   ����@   Found pending rename or delete that matches one of our files: %s    U��Ĝ���SVW3ۉ������������]�]�M�U��E�3�UhrcG d�0d� �u��u��u�E�   �;����E� ������P�E��,���P�Z����E�}���  3�UhbG d�2d�"���������   �} t���   �U�E�@�5����E�@Ku1�u��u��������������  ������������E�   �����>�}� t8�������E��f���������u��������E��Xf���������E�   �e����EP�E�����Y��t�E�������Y  ������P�E�P�v������'���3�ZYYd�h$bG �E�P�H�����B�����E�@L �  ������P�U��������N����������U��(�����������cG ���������������P������E�}����   3�Uh@cG d�0d� �������%�����tr�EP�EP�EP�U�������������������������  ����������������������������cG �����������M�E�����Y��t�E������6������P�E�P�S������g���3�ZYYd�hGcG �E�P�%����������3�ZYYd�hycG �������   �����E�   �����������ۊE�_^[��]�    ����   *   ����   \   U��3�QQQQQSVW3�UheG d�0d� �E� ��!I �x �  ��Ц@ �V����E�3�Uh�dG d�0d� �_G �U������E���R��u
������   ��!I �pN����   F3��ס�!I �O���؀{N ��   j �ˋX"I �P"I 薿����tt�{8�t$U�U��C�(����E������Y��tU�E��B����o�U�����U�E��u���PS�U��E��xc���E�P�U�E��c���E�3�Y�����Y��t�E�������%GN�T���3�ZYYd�h�dG �E��u�����{�����3�ZYYd�heG �E�   ������Y�����E�_^[��]ÐSVQ��؀=�I  ��   3��Ã����	��   �$�CeG keG qeG �eG weG }eG �eG �eG �eG �eG �eG �$ �.�$�(�$�"�$��$��$��$�
�$��$3��Ë��I ���#N���ЍL$�$����Z^[ÐU��3�QQQQSVW�ڋ�3�UhfgG d�0d� ��|gG �n���Hu+�=t"I  t���������B  �ǹ   �   �����Ƌ��gG �4���Hu+�=t"I  u���������  �ǹ
   �   �d���댋��gG �����H��   �=t"I  t���������   �E�P������   �������U��,�p������u�E��U�������#�E�P��I�   �E������E��κ   ������U��E������U��E������U�$ I �\���U��E�M������E���d����u�E��ߓ����t�ǋU��%���3ۅ��c�����U���K����U�������3�ZYYd�hmgG �E�   ������������_^[��]�����   setup:  ����
   uninstall:  ����   files:  �ʲ	�����Ð�ʲ	�����Ð�=�I  t	�ʲ	����ÐSVWU��;-@I ��  �ա�!I ��K���؋C�x���P�C�?���Z������-@I ��!I �C ������!I �   �G��C �����{ t�("I �S������("I �� I ������{  t�,"I �S �������,"I �� I ������{$ t�0"I �S$������0"I �� I ������I ������3��S�����I �q����а�@�����I �^����а�-�����I �K����а������I �(I �V�����!I �pN��|TF3��ס�!I �J���؊C%��t
��t��t"�.�C��I ������C��I �������C� I �����GNu��=�I  tUh'  h�  ��I P����]_^[ÐSVWU�=�!I  �h"I �=�I  tC��!I �xO��|6G3��֡�!I �J�����I �M ����u��������h"I  ��   FOu͠�!I ,rt��Xx������������3�f����   ��!I �xO��|6G3��֡�!I �I������;C(u�{, t
�g���;C,u	�������lFOu͡�!I �xO��|DG3��֡�!I �aI���؋C(%�  ��f�����;�u�{, t
����;C,u	���2����FOu�3��%����=�!I u�h"I ]_^[�S��j�(I �@ P���������:�t~h�   j j j j j �(I �@ P����j�(I �@ P������t%�����   Pj�(I �@ P�~�����tj�(I �@ P�����jWj j j j j �(I �@ P�^���[�U����SVW3��E��E�3�Uh�lG d�0d� f�=�"I  tY�U�f��"I f�����Z���M��E�� mG ����f��"I f��� f��t#�u�hmG �U����&���u��E��   �R����|"I ���E��E� �|"I ��%�   �E��E� �|"I %��  �E��E� �E��E��E�3��k"I ��X
I �E��E��Uй   �mG ����3��l"I ��X
I �E��E��U�3ɸTmG ����3��HI ��$	I �E��E��U�3ɸpmG �a����=k"I  t4�=n"I  t��mG �E�����=o"I  t��mG �0����
��mG �$���3�ZYYd�h�lG �E��   �:�����x�����_^[��]�   ����    SP ����   .   ����0   Windows version: %u.%.2u.%u%s  (NT platform: %s)    ����   64-bit Windows: %s  ����   Processor architecture: %s  ����   User privileges: Administrative ����   User privileges: Power User ����   User privileges: None   SV��؋Ã���   �$�nG �nG @nG OnG ^nG mnG |nG �nG �nG �nG �nG �nG �nG �ƺ�nG �H���^[Ëƺ�nG �9���^[Ëƺ�nG �*���^[ËƺoG ����^[ËƺoG ����^[Ëƺ(oG �����^[Ëƺ4oG �����^[Ëƺ@oG �����^[ËƺToG �����^[Ë֋��(���^[� ����   OK  ����   Cancel  ����   Abort   ����   Retry   ����   Ignore  ����   Yes ����   No  ����	   Try Again   ����   Continue    SVW��؋����ǃ���   �$�~oG �oG �oG �oG �oG �oG �oG �oG �ƺpG ������]�ƺ pG ������O�ƺ4pG ������A�ƺPpG ������3�ƺhpG �����%�ƺxpG ������ƺ�pG �����	�֋������_^[�   ����   OK  ����	   OK/Cancel   ����   Abort/Retry/Ignore  ����   Yes/No/Cancel   ����   Yes/No  ����   Retry/Cancel    ����   Cancel/Try Again/Continue   U����SVW3ۉ]��]�]�����3�UhTqG d�0d� �E�P�U�������E�E��E��U���j����E�E��E��U�   �lqG �r���E�P�E�������U�X�S����E�����3�ZYYd�h[qG �E�   ������E������������_^[��]�  ����3   Defaulting to %s for suppressed message box (%s):
 U����SVW3ɉM��M����3�Uh'rG d�0d� �E�P�U��������E��E��E��U�3ɸ@rG ����E�P�E����4����U�X�{����E������3�ZYYd�h.rG �E�������E��������8�����_^[��]�   ����   Message box (%s):
 U����SVW3ۉ]���U��؋}3�Uh�rG d�0d� ��I "Et�ϋ֋��������J�֋������΋U����x���؅�t%�U����@����E��E��E��U�3ɸsG �����
�(sG ����3�ZYYd�h�rG �E�������l�������_^[��]�    ����   User chose %s.  ����   AppMessageBox failed.   U����SVW3ۉ]�M��U��؋}�u3�Uh�sG d�0d� ��I "Et�������ϋ��(������Q���������
���V�M��U�����x���؅�t%�U���D����E�E��E��U�3ɸtG �#����
�$tG ����3�ZYYd�h�sG �E�������p�������_^[��]�    ����   User chose %s.  ����   MsgBox failed.  ��"I �8�����u2j�(I �@ P�����(I �@ P�
���j jj�3ҡ|I �����ÐU����SVW3҉U��3�UhEvG d�0d� h�   j j j j j �(I �@ P�	����E� 3�UhvuG d�0d� ��|G �7����E�3�UheuG d�0d� �=�I  t��I ��E��X�E�P�E��@�E��E� �]��E� �Uع   �\vG �a���E���O����U�M졔I �O����E��@�E��E��@�E�3�ZYYd�hluG �E��������������3�ZYYd��8����������c@ ������t�E��j�(I �@ P�H���菻���޻���}� t
�   �����=�I  u`3�UhvG d�2d�"�E��|��!I ;B}������}� t�=���3�ZYYd��&�j���j�(I �@ P�����3ҡ(I �6����e����E��)���3�ZYYd�hLvG �E�輽���������_^[��]� ����   /SPAWNWND=$%x /NOTIFYWND=$%x    �7j���n"I �=n"I  u�0j����u3����o"I �а��Í@ S�����3��Ë�X
I �$�D$��3ɸwG �����m"I �u"I �v"I ��t	�DI ��DI YZ[�   ����   64-bit install mode: %s U��j SVW��3�Uh�wG d�0d� ��,�R���؅�u��wG ��Xc@ �D��迹���E�P��I��   �����E��������Ƌ˺   �0���3�ZYYd�h�wG �E��c�������������_^[Y]�   ����   ExtractLongWord: Missing comma  SV��؃; t�3hxG V�ú   �=���^[ËË��M���^[�  ����   
  U��QSVW���E����ٻ��3۾$	I ��<w���E�s	�ǋ����C����u�_^[Y]Í@ U����SVW3ۉ]��ً�E�3�UhyyG d�0d� �   �}���Q�
   ���Y��uI�ǿ
   ����E�j3��E��E��E� �M��E� 3��E��E��E� �E�ȋú�yG �����f�}�f��t'�E�P�ǉE��E� �U�3ɸ�yG �����U����x���f��tZ�3h�yG �U���f����������u��ú   ������f��� f��t'�E�P�ǉE��E� �U�3ɸ�yG �+����U�������3�ZYYd�h�yG �E�舺���������_^[��]� ����   %d.%.*d ����   .%d ����    Service Pack   j jj%�   ��tI �3��i������Í@ U����SVW3ɉM����3�Uh@zG d�0d� j jj�E�P�u��U�3ɋ������E��3������F��3�ZYYd�hGzG �E�������������_^[YY]Ë�U����SVW3ۉ]��]������3�Uh�zG d�0d� j jj�E�P�u�M�f�U��������E��E��U��   ��臊���E��3��������3�ZYYd�h�zG �E��5����E��-����鋵����_^[��]� U����SVW���E��    �L����E�3�Uhp{G d�0d� �U��   ��褈���}� ~3�]��    ~�    �u��֋ˋ�耈���֋ˋE���P��)]�}� �3�ZYYd�hw{G �    �E������������_^[��]Ë�U��QSV��ز���@ 蚯���E�3�Uh�{G d�0d� �֋E��3���3�3ҋE��0�V����A ��P�����U���Q<3�ZYYd�h�{G �E��n�����t�����^[Y]ÐU����SVW3��E��E�3�Uh�|G d�0d� �U��$ I �pG���U��E�� }G 茹���E�@��U������E����se��� �  �E��ve���0#I �=0#I  u%�E�P�E��E��E��U�3ɸ }G ������E�賜����!I ��t��t�2�0#I �)����u$�@}G 茜����0#I �<-����u
�l}G �r���3�ZYYd�h�|G �E��   �8�����v�����_^[��]� ����   _isetup\_isdecmp.dll    ����   Failed to load DLL "%s" ����"   ZlibInitDecompressFunctions failed  ����    BZInitDecompressFunctions failed    U����SVW3��E��E�3�UhX~G d�0d� �U��$ I ��E���U��E��p~G ������E�@��U�������E�����c��� �  �E���c���4#I �=4#I  u%�E�P�E��E��E��U�3ɸ�~G �g����E������4#I ��&����u
��~G ����3�ZYYd�h_~G �E��   �ɵ���������_^[��]�  ����   _isetup\_iscrypt.dll    ����   Failed to load DLL "%s" ����   ISCryptInitFunctions failed U��QSVW�M����3��Ë��!I ���d6��N��|<F�E��:�����3��Ë� I P�׋E�@�M��}��3��Ë��!I ���4��Nu�_^[Y]ÐU����SVW�M���؀=�I  t&��\�@ �׫����3��É<��I 3��Ëǋ���5��3��Ë��!I ����5����O����   G�E�    �E�������3��Ë� I P�֋E�@�M��v|���}�t�E��ЋE�������u03��Ë��!I ���K3���=�I  t'3��Ë��I �U��/3���3��Ë� I ����{���E�O�q���_^[YY]� U����SVW3҉U���3�Uh�G d�0d� �]���t�=�I  tv3ۍE���I �&�����!I  t
�E��)����؄�u,�=�"I  t#j j S�E��E��E��M��G ��"I �= �؄�t�E� ��!I t�q���8  �U��|���3�ZYYd�h�G �E�������u������E�_^[��]� ����   CheckPassword   U����VW�=�I  t	�i"I ��=�I  t�i"I �=i"I  tG�=i"I u�(I �@: 3��d�����!I �}��f��E����G ������E���!I f�E�f��!I _^YY]�       U��Ĥ���SVW3ۉ������������]��M������]3�Uh��G d�0d� VW�u��E��   �Q���3��C3��������P�E��=���P�k����E�}��tM�������u&�} t�u�������E��������E�U���:���������P�E�P�*�����u��E�P�����} �  ������P�֍�������������������������������G ����������蛴��P������E�}����   3�Uhk�G d�0d� �������������to�EP�EP�EP�E�P�׍�����誱���������������  �\����������������k������������G �[����������M����k���Y�U���8���������P�E�P�(������j���3�ZYYd�hr�G �E�P�������������3�ZYYd�h��G �������   葰���E��i�����Ǭ����_^[��]� ����   *   ����   \   U���p���SVW3���|�����x����E��E�E܉E؉E�3�Uh8�G d�0d� �����M�U�   �,����P�G �E�������uI�   ��I �E��������I ��I �E��������I �E��������I ��I �U������"�   ��|���3��N����|�����I �ȯ����|�����I ��B����|����  I 訯���E� 3��E� 3��E��N��+���  @�Eĉu�M�U��E��_����`�G �E��6�����u��E������T  �p�G �E�������u��E܋U��}����1  ���G �E��������u��I �  ���G �E��������u��I ��  ���G �E�������u��I ��  ���G �E�������u��I �  �̖G �E�������u��I �  ���G �E��b�����u��I �U�腮���}  ��G �E��?�����u��I �U��b����Z  � �G �E�������u*��I ��|����E�连����|�����I 胠���  ��G �E��������u*��I ��|����E�脞����|�����I �H�����  �(�G �E�������u*��I  ��|����E��I�����|�����I �����  �@�G �E��k�����u#��|����E��)@����|�����I �}����u  �T�G �E��7�����u#��|����E���?����|�����I �I����A  �h�G �E�������u��I �U��&����  �x�G �E��������u��I �U�������   ���G �E�������u��I �U�������   ���G �E�������u3ҋE��������I �   ���G �E��u�����u	�E��   �ԗG �E��[�����u	��I ���G �E��A�����u�E��E��=����H����[� �G �E�������u�E�������I ��I �4��G �E��������u������0�G �E��������u�E�������E��E��M��r����=�I  t
��I �t����}� t��I 
�I t��I ��I �D�G 軫����I �ĘG 謫����I ��G 蝫���=�I  uN��|���3ҡ�I �j;����|����Eع��G �߬���E��C����u+��|����E��?����|������������E؋�I �|���jj j�Mز�h�D �o���E�3�Uh�G d�0d� ��I �E��<o���U��@   �Eԋ8�W��@t���T����E��I �@   �J���t���9���3�Uh��G d�0d� h`�E �MԲ�P�D �w���E�3�UhP�G d�0d� j�p I �.  �E��jr��U�@   �!I 3������YU�   �!I ������YU�   �!I ������YU3���P3���P�.   �!I �����Y�a����}� u;�=�!I  ��������t(�E��tW���E��lW����|�������G����|��������}� t
3ҋE��������u	��!I t_3�Uh��G d�0d� �}� u���G �)�����E������3�ZYYd��-�t���   Xc@ ̊G �ËK�C�̙G �����U���褦�����G ������$�G ��|���蒩����|�����I �i�����|���������D�G ��|����f�����x�����F����x�����|����2�����|�������������!I  ���p"I �q"I  ��!I @���r"I Uj�j��>   �!I �����YUj�j��2   �!I �����YU3��� P3���*P�7   �!I ��f���YU3���$P3���.P�O   � !I ��F���YU3���0P3���:P�P   �(!I �	�&���YU3���(P3���2P�=   �,!I �
����YU3���$P3���.P�A   �0!I ������YU3���P3���&P�1   �4!I ������YU3���P3���&P�1   �8!I �����YU3���4P3���>P�O   �<!I �����YU3���4P3���>P�O   �@!I ��f���Y� "I �U������$"I �U�����3��E���!I H,s����@ �,����E��U�E������3��E���!I t����@ �����E��U�E�����3�ZYYd�hW�G �E��	����������h`�E �MԲ�P�D ��s���E�3�Uh��G d�0d� U�F   �$!I ��=���Y3�ZYYd�h��G �E�賝���鹢����3�ZYYd���x���   �D ȍG �������踣��3�ZYYd�h�G �E��o�����u������=h"I  t+��!I �x~ �=�I  u�=�I  u�|p����u�K����\�G :�!I t2�HI <w
����!I r��|�����!I ������|����p�|����N!I �D!I ������t��t>��tt�   �=k"I  t�h�G �l�G����   f�L!I P�H!I �h�G �������r�=k"I  tf�L!I P�H!I �h�G ���u����Nj �D!I �|�G ���_����8�=k"I  tf�V!I P�R!I �h�G ���;����j �N!I �|�G ���%����HI <w
����!I s�=l"I  u���G �g������P����3��G�����!I ��t��t�"�=o"I  u�v�8�����=n"I  u��&����	����`���������!I H,sU�C���Y��!I tU�����Y�I����=� I  �C  ��`�H �, ��"I 3�Uh��G d�0d� ��"I �@�eG ��"I �@�gG ��"I �@�gG ��"I �@�gG ��I �� I ��"I �s, �=p"I  u���G ��"I ��, �p"I �p"I ������p"I �=q"I  u���G ��"I �, �q"I 3�ZYYd���۝����"I �!Q��蔠������3�Uh�G d�2d�"j j j3���p���ƅt�����p����ĚG ��"I �S- d�    ����~����ܚG ������7���膠����u ��G ������W�����p"I �����p"I ��|����p I �R�����|�����"I ������|����t I �2�����|�����"I ������|����| I ������|�����"I �Ƣ����|����� I ������|�����"I 覢���4jjj��|�����"I ���s����|������I �h���Ht������"I �~�����u���!I �@H��|m@�E��E�    j �U��!I �!���@P�U��!I �!���@P3�3�3��o�����u'�U��!I �!���I �i��3ɋU��!I �"���E��M�u���!I �"��3�3���!I �@H����   @�E��E�    �U��!I �5!����;{|0�S+�C!�������u!j �CP�CP3�3�3��ڏ����t�{G���@��t�F;Cu���1�����t�C;�~��3�3ɋU��!I ��!���I ����h���E��M��p�����!I ��!���w"I  ��!I �@H��|/@�E��E�    �U��!I �| ���@$t	�w"I ��E��M�uܡ�!I �x �x"I �"I �x �j"I ��!I �x �y"I �|!I ��"I ��!I ��"I ��!I �@H���X  @�E��E�    �U��!I �����E��E��x8�tI�E��x �  �E��x �  �E��x �  �E��P8� "I ����P��"I �Do����   �E��@M��   3�UhߔG d�0d� �E��xN t�U�3��>����UȋE�� ����U�E��Q��P�E��@L ��P��p���P��|����E��x3����|���P��x����E��3����x���3�Y�����Y�E���p����P@��t����PD3�ZYYd��
錙��補���E��x u"�E��x u�E��x u�E��P@��"I �`n���E��M��������!I �@H��|D@�E��E�    �U��!I ����؍�p����S��������p����C6��t����C:�E��M�uǡ�!I �@��~{H��|i@�E��E�    �U��!I �:�����C$��p�����`�����p����C&��t����C*�}� t�U̍C&�em����}�C&�E̋C*�E��E��M�u��U̸�"I �|m��3�ZYYd�h?�G ��x����   �����E������Eغ   ������E�   ������'�����_^[��]�  ����   /SL5=   ����   /Log    ����   /Log=   ����   /Silent ����   /VerySilent ����
   /NoRestart  ����   /NoIcons    ����	   /NoCancel   ����   /Lang=  ����   /Type=  ����   /Components=    ����   /Tasks= ����   /MergeTasks=    ����	   /LoadInf=   ����	   /SaveInf=   ����   /DIR=   ����   /GROUP= ����
   /Password=  ����   /RestartExitCode=   ����   /SuppressMsgBoxes   ����   /DETACHEDMSG    ����
   /SPAWNWND=  ����   /NOTIFYWND= ����   /DebugSpawnServer   ����
   /DEBUGWND=  ����w   The file %1 is missing from the installation directory. Please correct the problem or obtain a new copy of the program. ����G   The setup files are corrupted. Please obtain a new copy of the program. �����   The setup files are corrupted, or are incompatible with this version of Setup. Please correct the problem or obtain a new copy of the program.  ����   -0.bin  ����   Setup   ����   Error creating log file:

    ����'   Setup version: Inno Setup version 5.2.2 ����   Original Setup EXE:     ����   Setup command line:         ����
   Windows NT  ����   Windows ����   1   ����   CheckPassword   ����   CheckSerial ����   InitializeSetup ����,   InitializeSetup raised an exception (fatal).    ����)   InitializeSetup returned False; aborting.   U����SVW3҉U��3�Uh��G d�0d� ���G 舼���=�"I  ��   ��tb3�Uh��G d�0d� j j ��"I P3��E��E��M����G ��"I �@# ��"I 3�ZYYd�� 鰒���ܝG �&���3ҡ(I 肪��豕��3�Uh�G d�0d� j j 3��E��E��M���G ��"I �L! 3�ZYYd�� �Y����,�G �ϻ��3ҡ(I �+����Z�����"I �E����"I ��R��N��|1F3ۍM�ӡ�"I �8�W�E�P�ӡ�"I ��Q����Z�r��CNuҡ�"I ��R8��"I ��R��K�� |0�M�ӡ�"I �0�V�E�P�ӡ�"I ��Q����Z�Hw��K���uС�"I ��R8�����=4#I  t�4#I P觻���=0#I  t�0#I P蓻���z����I����=s"I  t�=�I  t�\�G �ɺ���s"I  �A����=s"I  t1���G 誺���=�I  tj h'  h�  ��I P�����������3�ZYYd�h��G �E��~�����ܒ����_^[��]�   ����   Deinitializing Setup.   ����   GetCustomSetupExitCode  ����+   GetCustomSetupExitCode raised an exception. ����   DeinitializeSetup   ����&   DeinitializeSetup raised an exception.  ����D   Not restarting Windows because Setup is being run from the debugger.    ����   Restarting Windows. U����SVW�M��U�E�3��E��   �]��u�}�3��3Ҋ+��m�Q��   ���Y3ҊGFCIuۋE�_^[��]�h  j j ���I ��I ��������Ë�j j jj ����Ë�U����SVW3ۉ]��t����΍���U���3�Uh��G d�0d� 3ҋ���' ���* ��!I t]��!I u3ҋ��o�����!I u	����o����  ���U��b# �U���D������}�����!I t����u���	�(I �@: �M��"I ���Xe���U��袪��j ���a���P菿����j j h   V�V����tI � ���Ph'  j V�>���ShظG �(I �����(I �x: t	�����j��3�ZYYd�h��G �E�聓����ߏ�����}� t
d�    ����_^[��]�SV�ڋ�VhظG �(I ����3ҋ���b����t��袌����^[Ð�z'  u�V  ��$���Í@ 3��BË�SVWU��ċ����do���ءX!I ;\!I u%�ЋC�+���T$,�ǋ�Q,�T$,����
����   ������\!I �����$�T$,�ǋ�Q,�D$4�D$�D$8�D$3��΋$���O����ЋC������!I uCh�   �FP�D$P�Ÿ��P�D$0Ph�   V�D$P诸���ЋL$3��
���T$,���3
���A�D$P�D$0Ph�   �FP�D$P�x���Ph�   V�D$P�g���3�Y��	���T$,����	��F��   �P���j�����P�����h�G j��!I ��!I �C��! ��"I �>����t� �G �C�����$�G �C�� ���T$�ǋ�Q,j�j��D$P�p���W�t$�|$ �   �_jj�D$$P����3ҋC����h  �D$ Pj���"I �u���P���a��P諻������ �C����h  �D$Pj���"I �D���P���0��P�z���h�G j��!I ��!I �C��  �(�G �C�= ���T$�ǋ�Q,j�j��D$P谼���t$�|$�   �h  �D$ Pj���"I �ϓ��P�����P�����D$(+D$ �T$+ЉT$�t$�|$�   �jj�D$$P����3ҋC����h  �D$ Pj���"I �r���P���^��P診������ �C����h  �D$Pj���"I �A���P���-��P�w�����<]_^[�   ����   Arial             ��RHË�U��j SVW3�Uh��G d�0d� �E���G �=����E��8�G �����u�h�G h�G h�G h(�G �E��   謑���=|I  t�u�h�G �5|I �E��   舑���=I  t�u�h�G �5I �E��   �d���j j j 3ɋ�I �E��Z���3�ZYYd�h�G �E�������c�����_^[Y]�   ����   Inno Setup version 5.2.2
  �����   Copyright (C) 1997-2007 Jordan Russell
Portions Copyright (C) 2000-2007 Martijn Laan
All rights reserved.

Inno Setup home page:
http://www.innosetup.com/ ����   

    ����#   RemObjects Pascal Script home page: ����   
  ����   http://www.remobjects.com/?ps   U��j SVW��3�Uh��G d�0d� �ĦG 膱��jj�U��(I 虛���E�赐��P��譐���   Z����3�ZYYd�h��G �E��U����鳉����_^[Y]�   ����   Exception message:  U��j SVW���3�Uh�G d�0d� �U��F��=���U����A���3�ZYYd�h$�G �E�������B�����_^[Y]Ë��(I ����ÐU����SVW�M��E��U����  �=�"I  tp3�Uh��G d�2d�"j j ���E��E� �M�ܧG ��"I �� 3�ZYYd��8�؆���}� t���G �H����U��(I 裞���� �G �/����r��������_^[��]�  ����   CurStepChanged  ����#   CurStepChanged raised an exception. ����+   CurStepChanged raised an exception (fatal). U����SVW�(I ��L
F �Ax���PI �=�"I  tP3�Uh��G d�0d� j j 3��E��E��M���G ��"I � 3�ZYYd��鿅���,�G �5����x����ǈ���   �PI 航���=i"I  u�(I �@ P�a����PI 规���
�PI �;���_^[YY]�����   InitializeWizard    ����-   InitializeWizard raised an exception (fatal).   �u"I �@M�t3��@Nt�=l"I  u
���G �o����������?   Cannot run files in 64-bit locations on this version of Windows U����SVW3҉U�U�U؉UԉU��U���3�Uh�G d�0d� 3�UhͬG d�0d� �0�G �ۭ���FN�Ä�t�H�G �ƭ���
�h�G 躭���FMu���G 読���
���G 螭���U���0������G �E��+����E�U������E��t����U��F�����}� t �ȭG �E�������E�U��׊���E��C����E��FL��t��t�
�E� ��E��FM�  �������E��FMt�U��E��h������   �E�P�U�F舡���E�P�E�P�FHPh,�G �E�P�M��U�����c����ux�M�U��1�%Z���E��G �8����E�P�E�P��G �E܍U؋E������E؉E��UԋE���:���EԉE�Uܹ   �4�Y���U�X�����M��Xc@ ������z����}��  �E�P�E��E��E� �U�3ɸ�G �����E�������   �(�G ������   �FMt�E��= ������   �E�P�U�F肠���E�P�E�P�FHPh,�G �E�P�U�F�b����U�M�����c������   �M�U��1�Y���E��G �#����E�P�E�P�P�G �E܍U؋E������E؉E��UԋE���9���EԉE�Uܹ   �4�X���U�X�و���M��Xc@ ������e����
�h�G �-���3�ZYYd��鞁��3ҡ(I �z���詄��3�ZYYd�h�G �EԺ   �#����E�   �����E��   �	�����G�����_^[��]�  ����   -- Run entry -- ����   Run as: Original user   ����   Run as: Current user    ����
   Type: Exec  ����   Type: ShellExec ����
   Filename:   ����   Parameters:     ����   

    ����   CreateProcess   ����   Process exit code: %u   ����   File doesn't exist. Skipping.   ����   ShellExecuteEx  ����'   File/directory doesn't exist. Skipping. U����SVW3��E�3�Uh�G d�0d� �"I �x �I  ��!I t	�=r"I  t3����E��}� t�E�舋��3��E�3�Uh�G d�0d� �"I �@H����  @�E��E�    �U��"I ����E��E��@M�V  �M��X"I �P"I �u�����;  �}� u����B �=���E��E��x tk3�Uh��G d�0d� �UȋE��@�����UȡPI ���  ����3�ZYYd��H��~���E�P��(I 衖���PI ���  �I ����躁����PI ���  �I �ƚ���PI ���  ��RP�E��@M@t4�PI �x7 tP�E�@����   uA�E�@�ƀ�  �E�@��  �'�E�@����   t�E�@�ƀ�   �E�@���  �U��谴���E��@0��n���E��U����E��@,��n���E��M������3�ZYYd�h��G �E�@����   t�E�@�ƀ�   �E�@��v  �E��z���}� t�E�萉���U̍E��]����u�r"I ��l��륡(I �Ԑ��3�ZYYd�h!�G �E�������E����_^[��]�U����SVW3҉U�U��U�E��PI 3�Uh=�G d�0d� �E� 3�Uh�G d�0d� �葵����u������U����  ������U�@"I 迂����L�����u������U����  �Ϙ���U�D"I 蒂�����  �����H"I ��Q����L"I �T"I �P"I ��͘���\"I �X"I ��:����=�I  t
��I �y���(I �U����E���RP�=i"I u�(I �@ P������W{�����RP3ɲ�E�������E��?���}� u�����3�ZYYd��  ����  3�著��U����Y���E�������!I tj j j h   ������!I t�K����=i"I  t���z��3��r"I ��X
I �E��E��U�3ɸX�G ������=r"I  t@�=�I  u7�U�Q�Gb���U��-������X  �貖�����\  �裖���   �=�"I  t�U��O�b���
�U��P��a���U�E��,����E�|�G �����E��I ������U�車���=r"I  u2�X"I �P"I ��g�������  ���   ��R���������=i"I  u�(I 虍���E���RP�E�3�ZYYd��&�zz���U��(I �U�����"I    �&����u}��3�ZYYd�hD�G �E������E������E�������"|�����E�_^[��]�  ����   Need to restart Windows? %s ����   

    U��QSVW3��E�3�Uh]�G d�0d� �PI ���  ���   ��R��N����   F3ۡPI ���  ��������tg�}� u����B �"8���E��PI ���  ���%���С"I ��������PI ���  ������а�����G0�j���������G,�%j��CN�{���3�ZYYd�hd�G �E���u����{����_^[Y]Ë�U����SVW3҉U��E�3�Uh��G d�0d� 3�Uh\�G d�0d� �PI ��w���=r"I  u
������   �=�I  t
��I ��"I �=�I  t	�s"I  �X�i"I ,rt��tB�G�PI ��X  ��  �s"I �/jjj�U��R�D_���E��3�� ������s"I ��s"I �=s"I  u
���G 誡�����E������3�ZYYd��!�x���U��(I ������"I    �{������3�ZYYd�h��G �E��i}�����y����_^[YY]�  ����'   Will not restart Windows automatically. U��Q�E��E��M��U��PI �����}� t�}� t������u3�Y]ðY]Ë�S��� �=PI  ��   �PI ���������   �PI �5���P���������   �PI ����P�ۨ����to�PI ���  �s�����t[���  ,rt+�M�PI ��4  ���=�����t5��"I    �����$��!I  t�=�I  u�������t�`"I [Ë��JÍ@ U��SV�]j�S�ԧ��;Et�   �3�S营����^[]� U��S���Euf�  ���T���Ph�G 赠��P�g���[]� ��S�؃=PI  ��   �PI ������ty�PI ���    t���   u�(I �@ P襧����t3�����j�PI �����P�'����   ��:�t*��tj�PI ����P�-����j �PI ����P����[Í@ SV��3ۋ��Gu)�(I �@ P�1��������:��  t���  ���.�����^[ÐSV��؋֋��l���~ tM���H�����蝥��;�u;�=PI  t2�PI �������t$�PI ����P������t�PI ����P�֧��^[Í@ SV����l"I  hT�G 躟����hd�G S�ş����te�T$R��hx�G S诟������tWT����P�օ�tJ�<$ tD�ES����t;h��G S肟����t,h��G h��G �W���P�i�����t�l"I �
�D$P�z���f�D$f��rf��tf��t��HI ��HI ��HI ��HI  ��(^[� kernel32.dll    GetNativeSystemInfo IsWow64Process  GetSystemWow64DirectoryA    RegDeleteKeyExA advapi32.dll    U����j j�E�P�4�G �  �3��Q!����uF�E�   �E�P�E�P�E�Pj h`�G �E�P�ʜ����u�}�u�}�u
f�E�f��"I �E�P�g�����]�   System\CurrentControlSet\Control\Windows    CSDVersion  U����SVW3��E�3�Uh$�G d�0d� j j�E�P�4�G �  �3�� ����ui�M��d�G �E������tL�x�G �E��V�����u	��"I �2���G �E��<�����u	��"I ����G �E��"�����u��"I �E�P膛��3�ZYYd�h+�G �E���w����;t����_^[YY]�  System\CurrentControlSet\Control\ProductOptions ProductType ����   WinNT   ����   LanmanNT    ����   ServerNT    �������$�   T���������   3��D$��3ҊT$����T$£|"I �=k"I  tn�D$��|VǄ$�   �   ��$�   P觜����tI3���$(  ��3Ҋ�$*  f�f��"I ��$.  ��"I f��$,  f��"I ���u
�o���������0  Í@ SV���!I ��\�@ �m�������u�^[Í@ SVWU����$�"I �<I �D$ I �/��t53���uN�� |!�֋��������t�D>����h��N���uߋ��xm���D$��#���l$�����$u�YZ]_^[ø "I �#���$"I �#����"I �#����"I �#���[����\"I �#���X"I �#���T"I �u#���P"I �k#����I �a#����I �W#��Ë�U��SVW3�UhC�G d�0d� �HI 袋������k"I ������
�����I � I ��Ц@ �|l����I ��Ц@ �kl����I ��Ц@ �Zl���P"I ��Ц@ �Il���T"I ��Ц@ �8l���X"I ��Ц@ �'l���\"I �Q�����Ц@ �l����"I ��Ц@ � l����"I 3�ZYYd�hJ�G ��q����_^[]Ð        ؿG             ��G t   �	F ,,@ �*@ +@ ��G H�@ A d�@ �A <A �A �A �A �A ��G F ,F HF `F |F TInputQueryWizardPage��TInputQueryWizardPage��G �	F  	ScriptDlg  �@         ��G             x�G p   �	F ,,@ �*@ +@ �F H�@ A d�@ �A <A �A �A �A �A �F F ,F HF `F |F TInputOptionWizardPage�TInputOptionWizardPage<�G �	F  	ScriptDlg  ��    0�G X�G             B�G �   �	F ,,@ �*@ +@ ��G H�@ A d�@ �A <A �A �A �A �A t�G F ,F HF ��G |F         |@ p   TInputDirWizardPage��TInputDirWizardPage��G �	F  	ScriptDlg  �        �G             ��G �   �	F ,,@ �*@ +@ �G H�@ A d�@ �A <A �A �A �A �A ��G F ,F HF `F |F TInputFileWizardPage�@ TInputFileWizardPage��G �	F  	ScriptDlg          ��G             ��G h   �	F ,,@ �*@ +@ �F H�@ A d�@ �A <A �A �A �A �A �F F ,F HF `F |F TOutputMsgWizardPage�@ TOutputMsgWizardPagel�G �	F  	ScriptDlg          x�G             \�G l   �	F ,,@ �*@ +@ �F H�@ A d�@ �A <A �A �A �A �A �F F ,F HF `F |F TOutputMsgMemoWizardPage�@ TOutputMsgMemoWizardPage �G �	F  	ScriptDlg          4�G             �G t   �	F ,,@ �*@ +@ �F H�@ A d�@ �A <A �A �A �A �A ��G F ,F HF `F |F TOutputProgressWizardPage��TOutputProgressWizardPage��G �	F  	ScriptDlg  �@ VWQ�����F7�$3ҋ��^����׋Ƌ�Q<�������$���D���Z_^�SV��t����h���ڋ�3ҋ���W����\�@ �mf���Fd��\�@ �^f���Fh��t
d�    ����^[�SV�ڋ��Fh�bf���Fd�Zf��3ҋ���W����t���Zh����^[ÐSVWU����β��D �����؉^l��3ҋ���������X���Ћ��f����   �PI � �Ћ��n������������Ջ��8������X���Ћǋ�Q<�Vl�PI ��R���غ   �PI � ؉^p]_^[�SVWU�ً������   �Ͳ��D ������3ҋ��9����Up���������W���Ћ�����   �PI �` �Ћ���������N����׋�葄�����rW���Ћ��9����֡PI �UR�����   �PI � �}p�3��Ͳ�4�E �	�����Ӌ��f	���Up��������EW���Ћ��,�����	W���Ћ�������$   �PI � Ep�֋Eh�����׋Ed�z���]_^[ÐSV��؋֋Cd� ���^[ÐSV��؋֋Ch�����^[ÐSVW����؋֋���������{���_^[Í@ SVW����؋֋�������苃��_^[Í@ U����SVW�M��U����β��D �����؉^l��3ҋ��������bV���Ћ��I~���   �PI �� �Ћ��Q~�����������U�����������U���Ћǋ�Q<�Vl�PI ��P�����β���D �a����؉^d�   �PI � ��׋��}������U���Ћ���}����   �PI �c ��+S(����}���} t	��!I u3������{�����hU���ЋFd�.����E��Fh�} u63ҋFd�����  ��Fd�g����   �PI �� �^d��T  ƃl  _^[YY]� �@ �HhQ3��   �U��} tQj jj �@d3������Qj jjjj �@d3�����]� �@ SVW���Fd���   ��R��O��|'G3ۋӋFd�����u�ӋFd��
����t���COu܃��_^[Ë@d�
��Í@ SVWU����Gd���   ��R��N��|$F3ۋӋGd��
����u;����ӋGd����CNu�]_^[Ë��@d����Í@ SV��t����c���ڋ�3ҋ��	S����\�@ �a���Fh��\�@ �a���Fl��\�@ �wa���Ft��t
d�    ����^[ÐSV�ڋ��Ft�za���Fl�ra���Fh�ja��3ҋ���R����t���jc����^[ÐU��j SVW��3�Uhz�G d�0d� �Ch����������t5�֋Cl�������U����;����CpP�M��Sd3��8����t
�U����L���3�ZYYd�h��G �E��i�����e����_^[Y]ÐSVWUQ�$��El�XK��|<C3��El���*���������ќ����u�PI �x7 t�Ƌ�Rx�$�  �GKuǋ$���}R��Z]_^[Í@ U����SVW�M��U����β��D ������؉^x��3ҋ���������R���Ћ��z���   �PI �*�  �Ћ��z����������U����Z�����;R���Ћǋ�Q<�Vx�PI �M���غ   �PI ���  ؉^|�E��Fd�Fp�U�h��_^[YY]� �U����SVW3ɉM����3�Uh��G d�0d� �E��U�3ɡPI ���  �E�����   �˲��D �������3ҋ������S|���y�����Q���Ћ��y���   �PI �@�  �Ћ��y������.����֋��q~�����RQ���Ћ������סPI �5L����   �PI ���  �s|�3��˲�VB �]����E��S|�E��y���
   �PI ��  P���Q����+U�X+ЋE���x������P���ЋE������˲��mB �1���������P����+U����x���E��P(J���x���U����x���PI ���  �P0���x���Cl�x u��I ���}����U��I �����U����h}�����   ǆ�   �G ���9P���Ћ�� ����$   �PI ���  C|�֋Ch�����׋Ct�����U��Cl������3�ZYYd�h��G �E��Pf����b������_^[��]Í@ SV��؋֋Ch� ���^[ÐSV��؋֋Cl�����^[ÐSV��؋֋Ct�����^[ÐSVW����؋֋���������g|��_^[Í@ U��j j SVW�����3�Uh��G d�0d� �U��������E��U��A����E�P�֋��x���Z�J|��3�ZYYd�h��G �E��   �e�����a����_^[YY]ÐSV��t����^���ڋ�3ҋ���M����\�@ �y\���Fd��\�@ �j\���Fh��Ц@ �[\���Fl��Ц@ �L\���Fp��\�@ �=\���Ft��t
d�    ����^[Í@ SV�ڋ��Ft�>\���Fp�6\���Fl�.\���Fh�&\���Fd�\��3ҋ��M����t���^����^[ÐU��3�QQQQQQSVW��3�Uh��G d�0d� �Cd�����������   �֋Ch�E����E��U��E���z���֋��J  ��u[�M�֋Cp�8�W�E�P�M��֋Cl�8�W�E�P����M��蠱��P�U�E��X����E�P�U��I ����E�U�Y������uh�֋���  ��tf�M�֋Cp�8�W�E�P�M��֋Cl�0�V�E�P���]M���8���P�U�E�������E�P�U��I ����E�U�Y�s�����t�U��E��8z��3�ZYYd�h��G �E�   �c�����_����_^[��]Í@ SVWU����β��D �����؉^x��3ҋ��J�������L���Ћ���t���   �PI �{�  �Ћ���t������i����Ջ��y�����L���Ћǋ�Q<�Vx�PI �oG���غ   �PI �2�  ؉^|]_^[�U����SVW3ۉ]�M����3�Uhb�G d�0d� �E��U�3ɡPI �2�  �E�����   �˲��D �I�����3ҋ��~����S|����s�����L���Ћ��t���   �PI ��  �Ћ��t����������֋���x�����K���Ћ��~����סPI �F����   �PI �]�  �s|�3��˲�VB �»���E�S|�E��hs���
   �PI ��  P���K����+U�X+ЋE��as�����>K���ЋE������˲��mB ���������IK����+U�����r���E�P(J����r���U����s���PI ���  �P0��� s���Cd�x u��I ����w����U��I �'���U����w�����   ǆ�   T�G ���J���Ћ��e����$   �PI �R�  C|�U��Cp��Q,�U�Cl��Q,�֋Cd�����׋Ct������U�Ch�������3�ZYYd�hi�G �E��`�����\������_^[��]� SV��؋֋Cd�P���^[ÐSV��؋֋Ch�<���^[ÐSV��؋֋Ct�(���^[ÐSVW����؋֋���������v��_^[Í@ SVW����؋֋���������v��_^[Í@ SV��؋֋��u����x��^[Í@ SV��؄�t�֋��U����@   ^[Ë֋��B���3҉P^[Ë�SVWU����β��D �����؉^d��3ҋ��������[I���Ћ��Bq���   �PI ���  �Ћ��Jq�����������Ջ��v������H���Ћǋ�Q<�Vd�PI ��C��]_^[Ë�SVWUQ�$�ڋ�3����   �β��D ��������~h3ҋ��"�������H���Ћ��p���   �PI �S�  �Ћ��p������A����Ӌ��u�����eH���Ћǋ�Q<�   �PI ��  �؋Vh�PI �6C�����3��Fh�β��D �5���؉^d�Ջ��p�����<H���Ћ��#p����   �PI ���  ��+Ջ��)p������x�����������ƃ   ����G���ЋFd�����^d����k���$�����Z]_^[Í@ SV��t����.W���ڋ�3ҋ��F���0�G 
F\�F\��t
d�    ����^[�      SVWU��PI �β��D �����؉^d��3ҋ�����3ҋ���������WG���Ћ��>o���E ���  �P0���Ko���E ���  ��  �����������F���Ћǋ�Q<�β��D �����؉^h3ҋ��I���3ҋ��l����   �E ��  �Ћ��n������F���Ћ��n���E ���  �P0����n�����F���ЋFh�J����β�T�D �H���؉^l�*   �E �&�  �Ћ��Mn�����vF���Ћ��]n���   �E � �  �Ћ��gn��3ҋ��fr�����F���ЋFl�����]_^[�S�ءPI ��4  ;C u�{p t�Sp�PI �{��3��Cp[Í@ �PI ��4  ;P u
�(I �ml���SV��؅�~ �ыCl�t���֋Cl�~����Cl��q���
3ҋCl��q��������^[Ë�U����SVW3ۉ]��M���3�Uh}�G d�0d� �Cd�or���E�P�sh�N,�VD�E��F���U����Pr�����U���3�ZYYd�h��G �E��[�����W����_^[YY]ÐS�ءPI ��4  ;C t�Cp�S �PI �z��������[ú�D �r~��ÐSVW����؋׋�������_^[Ç��U����SVW����؋׋������_^[�U��j SVW�����3�UhD�G d�0d� �M��׋������U����"[��3�ZYYd�hK�G �E��Z����W����_^[Y]Í@ �������SVW����؋׋��z����_^[Ç��=���SVW����؋׋��~����_^[�SVW����؋׋��~����_^[Ç��5���U��j SVW�����3�Uh��G d�0d� �M��׋������U����jZ��3�ZYYd�h�G �E��Z����cV����_^[Y]Í@ ������S���D �}���غ��D ��G ���2z���0�D ��G ���!z���T�D �,�G ���z���X�D �D�G ����y��hX�G ���G ���G ���qz��hh�G ���G 3ɋ��^z��hx�G �T�G ���G ���Hz��h��G �t�G �\�G ���2z��h��G �|�G 3ɋ��z��h��G ���G ���G ���	z��h��G ��G ���G ����y��[� ����   ADDCHECKBOX ����   ADDGROUP    ����   ADDRADIOBUTTON  ����	   CheckItem   ����   CHECKED ����   STATE   ����   ITEMCAPTION ����   ITEMENABLED ����	   ITEMLEVEL   ����
   ITEMOBJECT  ����   ITEMSUBITEM �T�D �V{��Ð���Ë���D �B{��h�G ���G 3���x�������   RTFTEXT ����Ë���  �Í@ �4�E �{��hP�G ��G ��G �x��� ����   Password    ���Ë�SV��؋Ƌ�   ��W��^[Ë�S�p�E �z���غ��E ���G ����w�����E ���G ���w��h �G �\�G �d�G ���+x��[� ����   ChangeDirectory ����   CreateNewDirectory  ����	   Directory   ���E �z�����E �0�G �?w���  ����   SetPaths    ���E ��y��Ð�����Ë����   �Í@ �����Ë����   �Í@ ����Ë����   �Í@ �o���Ë����   �Í@ �g���Ë����   �Í@ �/���Ë����   �Í@ ����Ë����   �Í@ S�D�E �Uy����h��G ���G ���G ����v��h��G ���G ���G ����v��h��G ���G ���G ����v��h��G ���G ���G ���v��h��G �p�G �x�G ���v��h��G �\�G �d�G ���v��h�G �H�G �P�G ���qv��[�   ����   AutoSize    ����	   BackColor   ����   Center  ����   Bitmap  ����   ReplaceColor    ����   ReplaceWithColor    ����   Stretch �s!��Ë����   �Í@ SVW����؋׋�� ���_^[�SV��؋�� ���^[Ë�S���E ��w���غ@�E ���G ����t��h��G �8�G 3ɋ��ou��h��G � �G 3ɋ��\u��h��G ��G ��G ���Fu��[�����   FindNextPage    ����	   PageCount   ����   Pages   ����
   ActivePage  �;��Ë�SV��؋��_���^[Ë�����Ë���   �Í@ S���E �w����hl�G ��G ��G ���t��h��G ���G � �G ���t��[� ����   Notebook    ����	   PageIndex   ��C �v��ÐS�t�H �v���غx�H ���G ���s���T�H ���G ���s��[�   ����   Center  ����   CenterInsideControl ��G �2v���4�G ��G �Ss���  ����   ShowAboutBox    ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ��   �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ��  �Í@ ��  �Í@ ��  �Í@ ��  �Í@ ��  �Í@ ��  �Í@ ��  �Í@ ��   �Í@ ��$  �Í@ ��(  �Í@ ��,  �Í@ ��0  �Í@ ��4  �Í@ ��8  �Í@ ��<  �Í@ ��@  �Í@ ��D  �Í@ ��H  �Í@ ��L  �Í@ ��P  �Í@ ��T  �Í@ ��X  �Í@ ��\  �Í@ ��`  �Í@ ��d  �Í@ ��h  �Í@ ��l  �Í@ ��p  �Í@ ��t  �Í@ ��x  �Í@ ��|  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ��4  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ ���  �Í@ S�L
F �Ir����h��G �,�G 3ɋ���o��h�G �8�G 3ɋ���o��h$�G �D�G 3ɋ���o��h8�G �P�G 3ɋ��o��hP�G �\�G 3ɋ��o��hh�G ��G 3ɋ��o��h|�G ��G 3ɋ��zo��h��G ���G 3ɋ��go��h��G ���G 3ɋ��To��h��G ���G 3ɋ��Ao��h��G ���G 3ɋ��.o��h��G ���G 3ɋ��o��h�G ���G 3ɋ��o��h�G ���G 3ɋ���n��h<�G ���G 3ɋ���n��h\�G ���G 3ɋ���n��ht�G ���G 3ɋ��n��h��G ���G 3ɋ��n��h��G �t�G 3ɋ��n��h��G �h�G 3ɋ��n��h��G ��G 3ɋ��pn��h��G �(�G 3ɋ��]n��h��G �4�G 3ɋ��Jn��h�G �@�G 3ɋ��7n��h$�G �L�G 3ɋ��$n��h<�G �X�G 3ɋ��n��hT�G �d�G 3ɋ���m��hp�G �p�G 3ɋ���m��h��G �|�G 3ɋ���m��h��G ���G 3ɋ���m��h��G ���G 3ɋ��m��h��G ���G 3ɋ��m��h��G ���G 3ɋ��m��h��G ���G 3ɋ��ym��h�G ���G 3ɋ��fm��h(�G ���G 3ɋ��Sm��h8�G ���G 3ɋ��@m��hP�G ���G 3ɋ��-m��hp�G ���G 3ɋ��m��h��G � �G 3ɋ��m��h��G ��G 3ɋ���l��h��G ��G 3ɋ���l��h��G �$�G 3ɋ���l��h��G �0�G 3ɋ��l��h��G �<�G 3ɋ��l��h�G �H�G 3ɋ��l��h,�G �T�G 3ɋ��l��h@�G �`�G 3ɋ��ol��hX�G �l�G 3ɋ��\l��ht�G �x�G 3ɋ��Il��h��G ���G 3ɋ��6l��h��G ���G 3ɋ��#l��h��G ���G 3ɋ��l��h��G ���G 3ɋ���k��h��G ���G 3ɋ���k��h�G ���G 3ɋ���k��h$�G ���G 3ɋ���k��hH�G ���G 3ɋ��k��hh�G ���G 3ɋ��k��h��G ���G 3ɋ��k��h��G ���G 3ɋ��xk��h��G ��G 3ɋ��ek��h��G ��G 3ɋ��Rk��h��G � �G 3ɋ��?k��h�G �,�G 3ɋ��,k��h0�G �8�G 3ɋ��k��hT�G �D�G 3ɋ��k��hl�G �P�G 3ɋ���j��h��G �\�G 3ɋ���j��h��G �h�G 3ɋ���j��h��G �t�G 3ɋ��j��h��G ���G 3ɋ��j��h��G ���G 3ɋ��j��h �G ���G 3ɋ��j��h�G ���G 3ɋ��nj��h<�G ���G 3ɋ��[j��h\�G ���G 3ɋ��Hj��h|�G ���G 3ɋ��5j��h��G ���G 3ɋ��"j����F ���G ���i����F ���G ���xi��[�  ����   CANCELBUTTON    ����
   NEXTBUTTON  ����
   BACKBUTTON  ����   OuterNotebook   ����   InnerNotebook   ����   WelcomePage ����	   InnerPage   ����   FinishedPage    ����   LicensePage ����   PasswordPage    ����   InfoBeforePage  ����   UserInfoPage    ����# Makefile.in generated by automake 1.10 from Makefile.am.
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