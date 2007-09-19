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

#include "model_hndl.h"

#include "lights.h"
#include "hier_cb.h"
#include "hier.h"
#include "gl_util.h"
#include "game_config.h"
#include "string_util.h"

#include "course_mgr.h"

#include "ppgltk/alg/defs.h"


#define MAX_ARM_ANGLE 30.0
#define MAX_PADDLING_ANGLE 35.0
#define MAX_EXT_PADDLING_ANGLE 30.0
#define MAX_KICK_PADDLING_ANGLE 20.0

bool		model_hndl::tuxLoaded = false;
char*     model_hndl::tuxRootNode;
char*     model_hndl::tuxLeftShoulderJoint;
char*     model_hndl::tuxRightShoulderJoint;
char*     model_hndl::tuxLeftHipJoint;
char*     model_hndl::tuxRightHipJoint;
char*     model_hndl::tuxLeftKneeJoint;
char*     model_hndl::tuxRightKneeJoint;
char*     model_hndl::tuxLeftAnkleJoint;
char*     model_hndl::tuxRightAnkleJoint;
char*     model_hndl::tuxTailJoint;
char*     model_hndl::tuxNeck;
char*     model_hndl::tuxHead;


model_hndl* ModelHndl=NULL;

void
model_hndl::adjust_tux_joints( double turnFact, bool isBraking, 
			double paddling_factor, double speed,
			pp::Vec3d net_force, double flap_factor )
{
    double turning_angle[2] = {0., 0.};
    double paddling_angle = 0.;
    double ext_paddling_angle = 0.;  /* arm extension during paddling */
    double kick_paddling_angle = 0.;  /* leg kicking during paddling */
    double braking_angle = 0.;
    double force_angle = 0.; /* amount that legs move because of force */
    double turn_leg_angle = 0.; /* amount legs move when turning */
    double flap_angle = 0.;

    /* move arms */
    reset_scene_node( tuxLeftShoulderJoint );
    reset_scene_node( tuxRightShoulderJoint );

    if ( isBraking ) {
	braking_angle = MAX_ARM_ANGLE;
    }

    paddling_angle = MAX_PADDLING_ANGLE * sin(paddling_factor * M_PI);
    ext_paddling_angle = MAX_EXT_PADDLING_ANGLE * sin(paddling_factor * M_PI);
    kick_paddling_angle = MAX_KICK_PADDLING_ANGLE * 
	sin(paddling_factor * M_PI * 2.0);

    turning_angle[0] = MAX(-turnFact,0.0) * MAX_ARM_ANGLE;
    turning_angle[1] = MAX(turnFact,0.0) * MAX_ARM_ANGLE;

    flap_angle = MAX_ARM_ANGLE * (0.5 + 0.5*sin(M_PI*flap_factor*6-M_PI/2));

    /* Adjust arms for turning */
    rotate_scene_node( tuxLeftShoulderJoint, 'z', 
		       MIN( braking_angle + paddling_angle + turning_angle[0],
			    MAX_ARM_ANGLE ) + flap_angle );
    rotate_scene_node( tuxRightShoulderJoint, 'z',
		       MIN( braking_angle + paddling_angle + turning_angle[1], 
			    MAX_ARM_ANGLE ) + flap_angle );


    /* Adjust arms for paddling */
    rotate_scene_node( tuxLeftShoulderJoint, 'y', -ext_paddling_angle );
    rotate_scene_node( tuxRightShoulderJoint, 'y', ext_paddling_angle );

    force_angle = MAX( -20.0, MIN( 20.0, -net_force.z / 300.0 ) );
    turn_leg_angle = turnFact * 10;
    
	/* Adjust hip joints */
    reset_scene_node( tuxLeftHipJoint );
    rotate_scene_node( tuxLeftHipJoint, 'z', -20 + turn_leg_angle
		       + force_angle );
    reset_scene_node( tuxRightHipJoint );
    rotate_scene_node( tuxRightHipJoint, 'z', -20 - turn_leg_angle
		       + force_angle );
	
    /* Adjust knees */
    reset_scene_node( tuxLeftKneeJoint );
    rotate_scene_node( tuxLeftKneeJoint, 'z', -10 + turn_leg_angle
		       - MIN( 35, speed ) + kick_paddling_angle
		       + force_angle );
    reset_scene_node( tuxRightKneeJoint );
    rotate_scene_node( tuxRightKneeJoint, 'z', -10 - turn_leg_angle
		       - MIN( 35, speed ) - kick_paddling_angle 
		       + force_angle );

    /* Adjust ankles */
    reset_scene_node( tuxLeftAnkleJoint );
    rotate_scene_node( tuxLeftAnkleJoint, 'z', -20 + MIN(50, speed ) );
    reset_scene_node( tuxRightAnkleJoint );
    rotate_scene_node( tuxRightAnkleJoint, 'z', -20 + MIN(50, speed ) );

	/* Turn tail */
    reset_scene_node( tuxTailJoint );
    rotate_scene_node( tuxTailJoint, 'z', turnFact * 20 );

	/* Adjust head and neck */
    reset_scene_node( tuxNeck );
    rotate_scene_node( tuxNeck, 'z', -50 );
    reset_scene_node( tuxHead );
    rotate_scene_node( tuxHead, 'z', -30 );

	/* Turn head when turning */
    rotate_scene_node( tuxHead, 'y', -turnFact * 70 );

}

void
model_hndl::draw_tux()
{
    GLfloat dummyColor[]  = { 0.0, 0.0, 0.0, 1.0 };

    /* XXX: For some reason, inserting this call here makes Tux render
     * with correct lighting under Mesa 3.1. I'm guessing it's a Mesa bug.
     */
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dummyColor );

    set_gl_options( TUX );

    /* Turn on lights
     */
    setup_course_lighting();

    draw_scene_graph( tuxRootNode );
} 

model_hndl::model_hndl() {
	cur_model=0;
	num_models=0;
}

void model_hndl::init_models() {
	//Load modellist from data/models.tcl
	char buff[BUFF_LEN];
     
	sprintf(buff, "%s/models.tcl", getparam_data_dir());
			
	registerHierCallbacks( tclInterp );
     register_tux_callbacks( tclInterp );
     
	if ( Tcl_EvalFile( tclInterp , buff ) != TCL_OK) {
		std::cerr << " error evalating model list file " << buff
				<< " : " << Tcl_GetStringResult (tclInterp ) << std::endl;
	}
	
	/*
	*Debug stuff:
	for(std::list<model_t>::iterator it=l_models.begin();it!=l_models.end();++it) {
		std::cout<<(*it).name<<": "<<(*it).filename<<std::endl;
	}
	*/
}

void
model_hndl::load_model() 
{
	tuxLoaded=false;
	load_model(cur_model);
}

void
model_hndl::load_model(int model=0)
{	
    //Loads a model.
    char cwd[BUFF_LEN];
    char buff[BUFF_LEN];


    std::list<model_t>::iterator c_model=l_models.begin();
    if(model!=0) {
    	for(int i=0;i<model;i++) {
    		c_model++;
    	}
    }


    if ( tuxLoaded == true && cur_model==model) 
        return;
    cur_model=model;
    tuxLoaded = true;
    initialize_scene_graph();

    if ( getcwd( cwd, BUFF_LEN ) == NULL ) {
	handle_system_error( 1, "getcwd failed" );
    }
    if ( chdir( getparam_data_dir() ) != 0 ) {
	/* Print a more informative warning since this is a common error */
	handle_system_error( 
	    1, "Can't find the etracer data "
	    "directory.  Please check the\nvalue of `data_dir' in "
	    "~/.etracer/options and set it to the location where you\n"
	    "installed the etracer-data files.\n\n"
	    "Couldn't chdir to %s", getparam_data_dir() );
	/*
        handle_system_error( 1, "couldn't chdir to %s", getparam_data_dir() );
	*/
    } 
    
    
    //std::cout<<"Loading model "<< (*c_model).name<<std::endl;
    if ( Tcl_EvalFile( tclInterp, (*c_model).filename.c_str()) == TCL_ERROR ) {
        handle_error( 1, "error evalating %s/%s: %s\n"
		      "Please check the value of `data_dir' in ~/.etracer/options "
		      "and make sure it\npoints to the location of the "
		      "latest version of the etracer-data files.", 
		      getparam_data_dir(),(*c_model).filename.c_str(),
		      Tcl_GetStringResult( tclInterp ) );
    } 
    check_assertion( !Tcl_InterpDeleted( tclInterp ),
		     "Tcl interpreter deleted" );

    if ( chdir( cwd ) != 0 ) {
	handle_system_error( 1, "couldn't chdir to %s", cwd );
    } 
} 

static int
head_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
                         "Usage: ", argv[0], " <head joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxHead = string_copy( argv[1] );

    return TCL_OK;
} 

static int
neck_cb ( ClientData cd, Tcl_Interp *ip, 
		     int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <neck joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxNeck = string_copy( argv[1] );

    return TCL_OK;
} 

static int
root_node_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <root node>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxRootNode = string_copy( argv[1] );

    return TCL_OK;
} 

static int
left_shoulder_cb ( ClientData cd, Tcl_Interp *ip, 
			      int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <left shoulder joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxLeftShoulderJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
right_shoulder_cb ( ClientData cd, Tcl_Interp *ip, 
			       int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <right shoulder joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxRightShoulderJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
left_hip_cb ( ClientData cd, Tcl_Interp *ip, 
			 int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <left hip joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxLeftHipJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
right_hip_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <right hip joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxRightHipJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
left_knee_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <left knee joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxLeftKneeJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
right_knee_cb ( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <right knee joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxRightKneeJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
left_ankle_cb ( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <left ankle joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxLeftAnkleJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
right_ankle_cb ( ClientData cd, Tcl_Interp *ip, 
			    int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <right ankle joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxRightAnkleJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
tail_cb ( ClientData cd, Tcl_Interp *ip, 
		     int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <tail joint>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    ModelHndl->tuxTailJoint = string_copy( argv[1] );

    return TCL_OK;
} 

static int
tux_add_model_cb ( ClientData cd, Tcl_Interp *ip, 
		     int argc, CONST84 char *argv[]) 
{
    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <model file> <model name>",
			 (char *)0 );
        return TCL_ERROR;
    }
 
    model_t tmp_model;
    tmp_model.name = string_copy( argv[2] );
    tmp_model.filename = string_copy( argv[1] );
    tmp_model.id = ModelHndl->num_models++;
    ModelHndl->l_models.push_back(tmp_model);
 
    return TCL_OK;
}


static void register_tux_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_root_node", root_node_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_shoulder",  left_shoulder_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_shoulder",  right_shoulder_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_hip",  left_hip_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_hip",  right_hip_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_knee",  left_knee_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_knee",  right_knee_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_ankle",  left_ankle_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_ankle",  right_ankle_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_neck",  neck_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_head", head_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_tail", tail_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_add_model", tux_add_model_cb,  0,0);
}

/*
static void register_tuxplayer_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_root_node", root_node_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_shoulder",  left_shoulder_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_shoulder",  right_shoulder_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_hip",  left_hip_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_hip",  right_hip_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_knee",  left_knee_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_knee",  right_knee_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_left_ankle",  left_ankle_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_right_ankle",  right_ankle_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_neck",  neck_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_head", head_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_tail", tail_cb,  0,0);
}
*/
