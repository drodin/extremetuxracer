/* 
 * ETRacer 
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

#include "course_load.h"
#include "course_render.h"
#include "textures.h"
#include "model_hndl.h"
#include "phys_sim.h"
#include "part_sys.h"
#include "keyframe.h"
#include "gl_util.h"
#include "game_config.h"
#include "loop.h"
#include "render_util.h"

#include "bench.h"

#include "fog.h"
#include "lights.h"

#include "ppgltk/ui_mgr.h"

#include "ppgltk/audio/audio_data.h"
#include "ppgltk/audio/audio.h"

#include "course_mgr.h"

#include "game_mgr.h"

#include "joystick.h"

#include "translation.h"

#include "callbacks.h"

#include "tcl_util.h"

#include "highscore.h"

/// instance of tcl interpreter
Tcl_Interp *tclInterp;

#define WINDOW_TITLE "Extreme Tux Racer"

#define GAME_INIT_SCRIPT "etracer_init.tcl"

std::string cfile;

static void
getopts( int argc, char *argv[] )
{
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i],"-c") ){
			i++;
			cfile = argv[i];
		}else if( !strcmp( argv[i],"-f") ){
			i++;
			if(argv[i] != NULL){			
				Benchmark::setCourse(argv[i]);
			}
		}else if( !strcmp( argv[i],"-m") ){
			i++;
			if(argv[i] != NULL){
				Benchmark::setMaxFrames(atoi(argv[i]));
			}
		}else if( !strcmp( argv[i],"-a") ){
			Benchmark::setMode(Benchmark::AUTO);
		}else if( !strcmp( argv[i],"-p") ){
			i++;
			pp::Vec2d pos;
			if(argv[i] != NULL){
				pos.x = atoi(argv[i]);
				i++;
				if(argv[i] != NULL){
					pos.y = atoi(argv[i]) * (-1);
					Benchmark::setPosition(pos);
					Benchmark::setMode(Benchmark::PAUSED);
				}			
			}		
		}else if( !strcmp( argv[i],"-t") ){
			i++;
			if(argv[i] != NULL){
				Benchmark::setTimeStep(atof(argv[i]));
			}
		}else if( !strcmp( argv[i],"-rc") ){
			i++;
			if(argv[i] != NULL){
				Benchmark::setRaceCondition(atoi(argv[i]));
			}
		}
	}	
}


/* This function is called on exit */
void cleanup(void)
{
    write_config_file();

    shutdown_audio();

    winsys_shutdown();
}

void read_game_init_script()
{
    char cwd[BUFF_LEN];
    if ( getcwd( cwd, BUFF_LEN ) == NULL ) {
	handle_system_error( 1, "getcwd failed" );
    }

    if ( chdir( getparam_data_dir() ) != 0 ) {
	/* Print a more informative warning since this is a common error */
	handle_system_error( 
	    1, "Can't find the ETRacer data "
	    "directory.  Please check the\nvalue of `data_dir' in "
	    "~/.etracer/options and set it to the location where you\n"
	    "installed the ETRacer-data files.\n\n"
	    "Couldn't chdir to %s", getparam_data_dir() );
    } 

    if ( Tcl_EvalFile( tclInterp, GAME_INIT_SCRIPT) == TCL_ERROR ) {
        handle_error( 1, "error evalating %s/%s: %s\n"
		      "Please check the value of `data_dir' in ~/.etracer/options "
		      "and make sure it\npoints to the location of the "
		      "latest version of the ETRacer-data files.", 
		      getparam_data_dir(), GAME_INIT_SCRIPT, 
		      Tcl_GetStringResult( tclInterp ) );
    } 

    check_assertion( !Tcl_InterpDeleted( tclInterp ),
		     "Tcl interpreter deleted" );

    if ( chdir( cwd ) != 0 ) {
	handle_system_error( 1, "couldn't chdir to %s", cwd );
    } 
}

    

int main( int argc, char *argv[] ) 
{
    /* Print copyright notice */
    fprintf( stderr, "Extreme TuxRacer " VERSION " --  http://www.extremetuxracer.com \n"
	     "(c) 2007 The ETRacer team\n"
	     "(c) 2004-2005 The PPRacer team\n"
	     "(c) 1999-2001 Jasmin F. Patry"
	     "<jfpatry@sunspirestudios.com>\n"
	     "ETRacer comes with ABSOLUTELY NO WARRANTY. "
	     "This is free software,\nand you are welcome to redistribute "
	     "it under certain conditions.\n"
	     "See http://www.gnu.org/copyleft/gpl.html for details.\n\n" );

	gameMgr = new GameMgr();
	Highscore = new highscore();
	ModelHndl = new model_hndl();
	
    /* Seed the random number generator */
    srand( time(NULL) );


    /*
     * Set up the game configuration
     */

    /* Don't support multiplayer, yet... */
    gameMgr->numPlayers = 1;

    /* Create a Tcl interpreter */
    tclInterp = Tcl_CreateInterp();

    if ( tclInterp == NULL ) {
		handle_error( 1, "cannot create Tcl interpreter" ); 
    }

    /* Setup the configuration variables and read the ~/.etracer/options file */
    
	getopts(argc,argv);
	
	init_game_configuration();
    read_config_file(cfile);

    /* Set up the debugging modes */
    init_debug();

    /* Setup diagnostic log if requested */
    if ( getparam_write_diagnostic_log() ) {
		setup_diagnostic_log();
    }

    /*
     * Setup Tcl stdout and stderr channels to point to C stdout and stderr 
     * streams
     */
    setup_tcl_std_channels();
	
    /* 
     * Initialize rendering context, create window
     */
    winsys_init( &argc, argv, WINDOW_TITLE, WINDOW_TITLE );


    /* Ingore key-repeat messages */
    winsys_enable_key_repeat(false);

    /* 
     * Initial OpenGL settings 
     */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    init_opengl_extensions();

    /* Print OpenGL debugging information if requested */
    if ( debug_mode_is_active( DEBUG_GL_INFO ) ) {
		print_debug( DEBUG_GL_INFO, 
		     "OpenGL information:" );
		print_gl_info();
    }

    /* 
     * Load the game data and initialize game state
     */
    register_game_config_callbacks( tclInterp );
    register_course_load_tcl_callbacks( tclInterp );
    register_key_frame_callbacks( tclInterp );

	FogPlane::registerCallbacks( tclInterp );
    
	register_course_light_callbacks( tclInterp );
    register_particle_callbacks( tclInterp );
    register_texture_callbacks( tclInterp );
    register_sound_tcl_callbacks( tclInterp );
    register_sound_data_tcl_callbacks( tclInterp );
    register_course_manager_callbacks( tclInterp );
	register_translation_callbacks( tclInterp );
	register_common_callbacks( tclInterp );
	
	
	// Setup class for translation
	translation.getLanguages();
	translation.load( getparam_ui_language() );
	
    // Load model
    ModelHndl->init_models();
   // ModelHndl->load_model(0); Loaded in players[0]::loadData()
    
    init_textures();
    init_audio_data();
    init_audio();
    
	init_course_manager();
     init_joystick();
	init_ui_snow();

	/* Read the ppracer_init.tcl file */
    read_game_init_script();

     /* Temporary name until user enters another name*/
     players[0].name = "Tux";
	players[0].loadData();
	
	//Ugly hax to prevent from segfault, fix to later version
	players[0].saveData();
	players[0].loadData();
	
	
	/* Init highscore */
	
	Highscore->loadData();
	/*debug highscore:*/
	//Highscore->debug();
	//Highscore->printlist();

     GameMode::mode = NO_MODE;
	
	if(Benchmark::getMode()==Benchmark::NONE){
		set_game_mode( SPLASH );
	}else{
		set_game_mode( BENCHMARK );
	}
	
    gameMgr->difficulty = DIFFICULTY_LEVEL_NORMAL;
	
	winsys_show_cursor( false );

	/* 
     * ...and off we go!
     */
    winsys_process_events();

    return 0;
}
