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

#include "course_load.h"
#include "course_render.h"
#include "course_quad.h"

#include "game_config.h"

#include "ppgltk/images/image.h"

#include "textures.h"
#include "phys_sim.h"
#include "tcl_util.h"
#include "keyframe.h"
#include "gl_util.h"
#include "lights.h"
#include "fog.h"
#include "part_sys.h"

#include "hud.h"

#include "player.h"

#include "ppgltk/audio/audio_data.h"

#include "track_marks.h"

#include "ppgltk/alg/defs.h"

#include "translation.h"

#define MAX_TREES 8192
#define MAX_TREE_TYPES 32
#define MAX_ITEMS 8192
#define MAX_ITEM_TYPES 128

#define MIN_ANGLE 5
#define MAX_ANGLE 80

#define ICE_IMG_VAL  0
#define ROCK_IMG_VAL 127
#define SNOW_IMG_VAL 255
#define TREE_IMG_THRESHOLD 128

#define DEP_TREE_RED 255
#define DEP_TREE_GREEN 0
#define DEP_TREE_BLUE 0

terrain_tex_t terrain_texture[NUM_TERRAIN_TYPES];
unsigned int num_terrains=0;

static bool        course_loaded = false;

static float     *elevation;
static float      elev_scale;
static float      course_width, course_length;
static float      play_width, play_length;
static float      course_angle;
static int           nx, ny;
static Tree        tree_locs[MAX_TREES];
static int           num_trees;
static int    *terrain;
static pp::Vec2d     start_pt;


static std::string courseAuthor;
static std::string courseName;


static int           base_height_value;
static tree_type_t   tree_types[MAX_TREE_TYPES];
static int           num_tree_types = 0;
static int           tree_dep_call = -1;

static Item			item_locs[MAX_ITEMS];
static item_type_t   item_types[MAX_ITEM_TYPES];
static int           num_item_types = 0;
static int           num_items;


///All terrains that are used in the current race.
///The list is sorted with respect to the terrain wheights
std::list<int> usedTerrains;


/* Interleaved vertex, normal, and color data */
static GLubyte      *vnc_array = NULL;

float    *get_course_elev_data()    { return elevation; }
int    *get_course_terrain_data() { return terrain; }
float      get_course_angle()        { return course_angle; } 
Tree       *get_tree_locs()           { return tree_locs; }
int           get_num_trees()           { return num_trees; }
pp::Polyhedron*  get_tree_polyhedron(int type) { return tree_types[type].ph; }
const char* get_tree_name(int type)       { return tree_types[type].name.c_str(); }
pp::Vec2d     get_start_pt()            { return start_pt; }
void          set_start_pt( pp::Vec2d p ) { start_pt = p; }
std::string& get_course_author() { return courseAuthor; }
std::string& get_course_name() { return courseName; }

Item       *get_item_locs()           { return item_locs; }
int           get_num_items()           { return num_items; }
const char         *get_item_name(int type)   { return item_types[type].name.c_str(); }
int           get_num_item_types()      { return num_item_types; }
item_type_t  *get_item_types()          { return item_types; }



void get_gl_arrays( GLubyte **vnc_arr )
{
    *vnc_arr = vnc_array;
}

void get_course_dimensions( float *width, float *length )
{
    *width = course_width;
    *length = course_length;
} 

void get_play_dimensions( float *width, float *length )
{
    *width = play_width;
    *length = play_length;
} 

/*! 
  Returns the base (minimum) height of the terrain at \c distance
  \pre     A course has been loaded
  \arg \c  distance the (non-negative) distance down the course

  \return  Minimum height (y-coord) of terrain
  \author  jfpatry
  \date    Created:  2000-08-30
  \date    Modified: 2000-08-30
*/
float get_terrain_base_height( float distance )
{
    float slope = tan( ANGLES_TO_RADIANS( course_angle ) );
    float base_height;
    
    check_assertion( distance > -EPS,
		     "distance should be positive" );

    /* This will need to be fixed once we add variably-sloped terrain */
    base_height = -slope * distance - 
	base_height_value / 255.0 * elev_scale;
    return base_height;
}

/*! 
  Returns the maximum height of the terrain at \c distance
  \pre     A course has been loaded
  \arg \c  distance the (non-negative) distance down the course

  \return  Maximum height (y-coord) of terrain
  \author  jfpatry
  \date    Created:  2000-08-30
  \date    Modified: 2000-08-30
*/
float get_terrain_max_height( float distance )
{
    return get_terrain_base_height( distance ) + elev_scale;
}

void get_course_divisions( int *x, int *y )
{
    *x = nx;
    *y = ny;
} 


static void reset_course()
{
    int i;

    /*
     * Defaults
     */
	num_terrains = 0;
    num_trees     = 0;
    num_items     = 0;
    course_angle  = 20.;
    course_width  = 50.;
    course_length = 130.;
    play_width  = 50.;
    play_length = 130.;
    nx = ny = -1;
    start_pt.x = 0;
    start_pt.y = 0;
    base_height_value = 127; /* 50% grey */

    set_course_mirroring( false );

    reset_lights();
    fogPlane.reset();
    reset_particles();

	courseAuthor.erase();
	courseName.erase();

	if ( course_loaded == false ) return;

    reset_course_quadtree();

    free( elevation ); elevation = NULL;
    free( terrain ); terrain = NULL;

    free( vnc_array ); vnc_array = NULL;

    for ( i = 0; i < num_tree_types; i++) {
	unbind_texture( tree_types[i].name.c_str() );
	tree_types[i].name.erase();

	tree_types[i].num_trees = 0;

    }
    num_tree_types = 0;
    tree_dep_call = -1;

    for ( i = 0; i < num_item_types; i++) {
	if (item_types[i].reset_point == false) {
	    unbind_texture( item_types[i].name.c_str() );
	}

	item_types[i].name.erase();

	item_types[i].texture.erase();

	item_types[i].num_items = 0;
    }
    num_item_types = 0;

    course_loaded = false;

    reset_key_frame();
} 

bool course_exists( int num )
{
    char buff[BUFF_LEN];
    struct stat s;

    sprintf( buff, "%s/courses/%d", getparam_data_dir(), num );
    if ( stat( buff, &s ) != 0 ) {
	return false;
    }
    if ( ! S_ISDIR( s.st_mode ) ) {
	return false;
    }
    return true;
}

void fill_gl_arrays()
{
    int x,y;
    pp::Vec3d *normals = get_course_normals();
    pp::Vec3d nml;
    int idx;

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    /* Align vertices and normals on 16-byte intervals (Q3A does this) */
    vnc_array = (GLubyte*) malloc( STRIDE_GL_ARRAY * nx * ny );

    for (x=0; x<nx; x++) {
	for (y=0; y<ny; y++) {
	    idx = STRIDE_GL_ARRAY*(y*nx+x);
	   
#define floatval(i) (*(GLfloat*)(vnc_array+idx+(i)*sizeof(GLfloat)))

	    floatval(0) = (GLfloat)x / (nx-1.) * course_width;
	    floatval(1) = ELEV(x,y);
	    floatval(2) = -(GLfloat)y / (ny-1.) * course_length;

	    nml = normals[ x + y * nx ];
	    floatval(4) = nml.x;
	    floatval(5) = nml.y;
	    floatval(6) = nml.z;
	    floatval(7) = 1.0f;
	   
#undef floatval
#define byteval(i) (*(GLubyte*)(vnc_array+idx+8*sizeof(GLfloat) +\
    i*sizeof(GLubyte)))

	    byteval(0) = 255;
	    byteval(1) = 255;
	    byteval(2) = 255;
	    byteval(3) = 255;

#undef byteval

	}
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer( 3, GL_FLOAT, STRIDE_GL_ARRAY, vnc_array );

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer( GL_FLOAT, STRIDE_GL_ARRAY, 
		     vnc_array + 4*sizeof(GLfloat) );

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer( 4, GL_UNSIGNED_BYTE, STRIDE_GL_ARRAY, 
		    vnc_array + 8*sizeof(GLfloat) );
}

void load_course( std::string& course )
{
    char buff[BUFF_LEN];
    char cwd[BUFF_LEN];

    reset_course();
	HUD1.reset();
	
	
    if ( getcwd( cwd, BUFF_LEN ) == NULL ) {
	handle_system_error( 1, "getcwd failed" );
    }
    if (course[0]=='/'){
		if ( chdir(course.c_str() ) != 0 ) {	
			handle_system_error( 1, "Couldn't chdir to %s", course.c_str() );
    	} 
	}else{
		sprintf( buff, "%s/courses/%s", getparam_data_dir(), course.c_str() );
		if ( chdir( buff ) != 0 ) {
			handle_system_error( 1, "Couldn't chdir to %s", buff );
    	} 
	}
	
    if ( Tcl_EvalFile( tclInterp, "./course.tcl") == TCL_ERROR ) {
	handle_error( 1, "Error evaluating %s/course.tcl: %s",  
		      buff, Tcl_GetStringResult( tclInterp ) );
    } 

    if ( chdir( cwd ) != 0 ) {
	handle_system_error( 1, "Couldn't chdir to %s", cwd );
    } 

    check_assertion( !Tcl_InterpDeleted( tclInterp ),
		     "Tcl interpreter deleted" );

    calc_normals();

    fill_gl_arrays();

    init_course_quadtree( elevation, nx, ny, course_width/(nx-1.), 
			  -course_length/(ny-1),
			  players[0].view.pos, 
			  getparam_course_detail_level() );

    init_track_marks();

    course_loaded = true;

    /* flush unused audio files */
    delete_unused_audio_data();
} 


static inline int intensity_to_terrain( const int intensity )
{
	for (unsigned int i=0; i<num_terrains; i++) {
		if (terrain_texture[i].value == intensity){
			terrain_texture[i].count++;
			return i;
		}
	}
	terrain_texture[0].count++;
	return 0;
}


static int course_dim_cb ( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{
    double width, length;

    if ( ( argc != 3 ) && ( argc != 5 ) ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <course width> <course length>",
			 " [<play width> <play length>]", (char *)0 );
        return TCL_ERROR;
    }

    if ( Tcl_GetDouble( ip, argv[1], &width ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid course width", 
			 (char *)0 );
        return TCL_ERROR;
    } 
    if ( Tcl_GetDouble( ip, argv[2], &length ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid course length", 
			 (char *)0 );
        return TCL_ERROR;
    } 

    course_width = width;
    course_length = length;

    if ( argc == 5 ) {
	if ( Tcl_GetDouble( ip, argv[3], &width ) != TCL_OK ) {
	    Tcl_AppendResult(ip, argv[0], ": invalid play width", 
			     (char *)0 );
	    return TCL_ERROR;
	} 
	if ( Tcl_GetDouble( ip, argv[4], &length ) != TCL_OK ) {
	    Tcl_AppendResult(ip, argv[0], ": invalid play length", 
			     (char *)0 );
	    return TCL_ERROR;
	} 
	play_width = width;
	play_length = length;
    } else {
	play_width = course_width;
	play_length = course_length;
    }

    return TCL_OK;
} 

static int angle_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    double angle;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <angle>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[1], &angle ) != TCL_OK ) {
        return TCL_ERROR;
    } 

    if ( angle < MIN_ANGLE ) {
	print_warning( TCL_WARNING, "course angle is too small. Setting to %f",
		       MIN_ANGLE );
	angle = MIN_ANGLE;
    }

    if ( MAX_ANGLE < angle ) {
	print_warning( TCL_WARNING, "course angle is too large. Setting to %f",
		       MAX_ANGLE );
	angle = MAX_ANGLE;
    }

    course_angle = angle;

    return TCL_OK;
} 


static int elev_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    pp::Image *elev_img;
    float slope;
    int   x,y;
    int   pad;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <elevation bitmap>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if (course_loaded) {
	print_warning( TCL_WARNING, "ignoring %s: course already loaded",
		       argv[0] );
	return TCL_OK;
    }

    elev_img = pp::Image::readFile( argv[1] );
    if ( elev_img == NULL ) {
	print_warning( TCL_WARNING, "%s: couldn't load %s", argv[0], argv[1] );
	return TCL_ERROR;
    }
	
    nx = elev_img->width;
    ny = elev_img->height;

    elevation = (float *)malloc( sizeof(float)*nx*ny );

    if ( elevation == NULL ) {
		handle_system_error( 1, "malloc failed" );
    }

    slope = tan( ANGLES_TO_RADIANS( course_angle ) );

	//elev_img->sizeZ=4;
	
    pad = 0;    /* RGBA images rows are aligned on 4-byte boundaries */
    for (y=0; y<ny; y++) {
        for (x=0; x<nx; x++) {
	    ELEV(nx-1-x, ny-1-y) = 
		( ( elev_img->data[ (x + nx * y) * elev_img->depth + pad ] 
		    - base_height_value ) / 255.0 ) * elev_scale
		- (double) (ny-1.-y)/ny * course_length * slope;
        } 
        //pad += (nx*elev_img->depth) % 4;
    } 

	delete elev_img;
	
    return TCL_OK;
} 


static bool
sort_terrain(const int x, const int y)
{
	if(terrain_texture[x].wheight < terrain_texture[y].wheight){
		return true;
	}else{
		return false;
	}	
}


static int terrain_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    pp::Image *terrain_img;
    int   x,y;
    int   pad;
    int   idx;
	int	terrain_value;
	int	image_pointer;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <terrain bitmap>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    terrain_img = pp::Image::readFile( argv[1] );

    if ( terrain_img == NULL ) {
	print_warning( TCL_WARNING, "%s: couldn't load %s", argv[0], argv[1] );
        Tcl_AppendResult(ip, argv[0], ": couldn't load ", argv[1],
			 (char *)0 );
	return TCL_ERROR;
    }

    if ( nx != terrain_img->width || ny != terrain_img->height ) {
        Tcl_AppendResult(ip, argv[0], ": terrain bitmap must have same " 
			 "dimensions as elevation bitmap",
			 (char *)0 );

	return TCL_ERROR;
    }

    terrain = (int *)malloc( sizeof(int)*nx*ny );

    if ( terrain == NULL ) {
	handle_system_error( 1, "malloc failed" );
    }
	
	pad = 0;
	
    for (y=0; y<ny; y++) {
        for (x=0; x<nx; x++) {
			
            idx = (nx-1-x) + nx*(ny-1-y);	
			image_pointer=(x+nx*y)*terrain_img->depth+pad;
			
			terrain_value=terrain_img->data[image_pointer] +
						(terrain_img->data[image_pointer+1] << 8)+ 
						(terrain_img->data[image_pointer+2] << 16);
			
	    terrain[idx] = intensity_to_terrain(terrain_value);
		
        } 
        //pad += (nx*terrain_img->depth) % 4;
    } 
	
    delete terrain_img;

	//build sorted list with used terrains for quadtree
	usedTerrains.clear();
	for (unsigned int i=0; i<num_terrains; i++){
		//check if the terraintype is used in the course
		if(terrain_texture[i].count>0){
			usedTerrains.push_back(i);
		}
	}
	usedTerrains.sort(sort_terrain);
	
    return TCL_OK;
} 

static int bgnd_img_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <background image>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if (!load_and_bind_texture( "background", argv[1] )) {
      Tcl_AppendResult(ip, argv[0], ": could not load texture", (char *) 0);
      return TCL_ERROR;
	}

    return TCL_OK;
}

static int terrain_tex_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
 	
    int i;
	int  rtn, num_col;
	CONST84 char* text_bind =NULL;
	int convert_temp;
	CONST84 char** indices = 0;

	if (num_terrains>=NUM_TERRAIN_TYPES){
		Tcl_AppendResult(ip, argv[0], ": max number og terrains reached", (char *)0 );
        return TCL_ERROR;
	}
	

	/* fill in values not specified with defaults */
	terrain_texture[num_terrains].type=1;
	terrain_texture[num_terrains].value=0;
	terrain_texture[num_terrains].friction=0.5;
	terrain_texture[num_terrains].compression=0.1;
	terrain_texture[num_terrains].texbind=0;
	terrain_texture[num_terrains].partbind=0;
	terrain_texture[num_terrains].soundactive=false;
	terrain_texture[num_terrains].trackmark.head=0;
	terrain_texture[num_terrains].trackmark.mark=0;
	terrain_texture[num_terrains].trackmark.tail=0;
	terrain_texture[num_terrains].wheight=150;
	terrain_texture[num_terrains].count=0;
	

    for ( i = 1; (i < argc - 1); i += 2 ) {
	
	if ( strcmp( "-name", argv[i] ) == 0 ) {
		text_bind=argv[i+1];
	
	} else if ( strcmp( "-texture", argv[i]) == 0 ) {
		if ( !load_and_bind_texture( text_bind,argv[i+1]) ) {
     		Tcl_AppendResult(ip, argv[0], ": could not load texture", (char *) 0);
	  		return TCL_ERROR;
    	}
	   if (!get_texture_binding(text_bind, &terrain_texture[num_terrains].texbind)) {
	   terrain_texture[num_terrains].texbind = 0;
	}
		
	} else if ( strcmp( "-wheight", argv[i] ) == 0 ) {
	    if ( Tcl_GetInt( ip, argv[i+1],
		    &terrain_texture[num_terrains].wheight) != TCL_OK ){
				Tcl_AppendResult(ip, argv[0], ": invalid wheight",
				 (char *)0 );
			return TCL_ERROR;	
	    }
	} else if ( strcmp( "-color", argv[i] ) == 0 ) {
	    rtn = Tcl_SplitList(ip, argv[i+1], &num_col, &indices);
	    if( rtn != TCL_OK ) {
		Tcl_AppendResult(ip, "a list of colors must be provided\n",
			     (char *) 0);
		Tcl_Free((char *) indices);
		return TCL_ERROR;
	    }

	    if (num_col == 3 || num_col == 4) {
		Tcl_GetInt(ip, indices[0], &convert_temp);
		terrain_texture[num_terrains].value = (unsigned char) convert_temp;
		Tcl_GetInt(ip, indices[1], &convert_temp);
		terrain_texture[num_terrains].value += (unsigned char) convert_temp << 8;
		Tcl_GetInt(ip, indices[2], &convert_temp);
		terrain_texture[num_terrains].value += (unsigned char) convert_temp << 16;
			
	    } else {
		Tcl_AppendResult(ip, argv[0], ": must specify three colors"
			" to link with terrain type", (char *) 0);
		return TCL_ERROR;
	    }
	    Tcl_Free((char *) indices);
	}  else if ( strcmp( "-friction", argv[i] ) == 0 ) {
	    if ( Tcl_GetDouble( ip, argv[i+1],
		    &terrain_texture[num_terrains].friction) != TCL_OK ) {
				Tcl_AppendResult(ip, argv[0], ": invalid friction",
				 (char *)0 );
			return TCL_ERROR;	
	    }
	}else if ( strcmp( "-compression", argv[i] ) == 0 ) {
	    if ( Tcl_GetDouble( ip, argv[i+1],
		    &terrain_texture[num_terrains].compression) != TCL_OK ) {
				Tcl_AppendResult(ip, argv[0], ": invalid compression",
				 (char *)0 );
			return TCL_ERROR;
	    }
	}else if ( strcmp( "-particles", argv[i]) == 0 ) {
		if (!get_texture_binding(argv[i+1], &terrain_texture[num_terrains].partbind)) {
	   		terrain_texture[num_terrains].texbind = 0;
		}
	} else if ( strcmp( "-envmap_texture", argv[i]) == 0 ) {
		if (!get_texture_binding(argv[i+1], &terrain_texture[num_terrains].envmapbind)) {
	   		terrain_texture[num_terrains].envmapbind = 0;
		}
	} else if ( strcmp( "-track_head", argv[i]) == 0 ) {
		if (!get_texture_binding(argv[i+1], &terrain_texture[num_terrains].trackmark.head)) {
	   		terrain_texture[num_terrains].trackmark.head = 0;
		}
	} else if ( strcmp( "-track_mark", argv[i]) == 0 ) {
		if (!get_texture_binding(argv[i+1], &terrain_texture[num_terrains].trackmark.mark)) {
	   		terrain_texture[num_terrains].trackmark.mark = 0;
		}
	} else if ( strcmp( "-track_tail", argv[i]) == 0 ) {
		if (!get_texture_binding(argv[i+1], &terrain_texture[num_terrains].trackmark.tail)) {
	   		 terrain_texture[num_terrains].trackmark.tail = 0;
		}
	} else if ( strcmp( "-sound", argv[i]) == 0 ) {
		terrain_texture[num_terrains].sound = argv[i+1];		
	}

	
	
   } 

    num_terrains++;
    return TCL_OK;
	
	
} 


static int start_pt_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    double xcd, ycd;

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <x coord> <y coord>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[1], &xcd ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid x coordinate", (char *)0 );
        return TCL_ERROR;
    } 
    if ( Tcl_GetDouble( ip, argv[2], &ycd ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid y coordinate", (char *)0 );
        return TCL_ERROR;
    } 

    if ( !( xcd > 0 && xcd < course_width ) ) {
	print_warning( TCL_WARNING, "%s: x coordinate out of bounds, "
		       "using 0\n", argv[0] );
	xcd = 0;
    }

    if ( !( ycd > 0 && ycd < course_length ) ) {
	print_warning( TCL_WARNING, "%s: y coordinate out of bounds, "
		       "using 0\n", argv[0] );
	ycd = 0;
    }

    start_pt.x = xcd;
    start_pt.y = -ycd;

    return TCL_OK;
} 

static int elev_scale_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{

    double scale;
    
    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <scale>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[1], &scale ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid scale", (char *)0 );
        return TCL_ERROR;
    } 

    if ( scale <= 0 ) {
	print_warning( TCL_WARNING, "%s: scale must be positive", argv[0] );
	return TCL_ERROR;
    }

    elev_scale = scale;

    return TCL_OK;
} 

static int is_tree( unsigned char pixel[], tree_type_t ** which_type )
{
	int min_distance = pixel[0] + pixel[1] + pixel[2];
    
	int i;
    int distance;
 
    *which_type = NULL;
    for (i = 0; i < num_tree_types; i++) {
	/* assume red green blue pixel ordering */
	distance = abs ( tree_types[i].red - pixel[0] ) +
		    abs ( tree_types[i].green - pixel[1] ) +
		    abs ( tree_types[i].blue - pixel[2] );
	if (distance < min_distance) {
	    min_distance = distance;
	    *which_type = &tree_types[i];
	}
    }
    
    return min_distance;
}

static int is_item( unsigned char pixel[], item_type_t ** which_type )
{
    int      min_distance = pixel[0] + pixel[1] + pixel[2];
    int i;
    int distance;
 
    *which_type = NULL;
    for (i = 0; i < num_item_types; i++) {
	/* assume red green blue pixel ordering */
	distance = abs ( item_types[i].red - pixel[0] ) +
		    abs ( item_types[i].green - pixel[1] ) +
		    abs ( item_types[i].blue - pixel[2] );
	if (distance < min_distance) {
	    min_distance = distance;
	    *which_type = &item_types[i];
	}
    }
    
    return min_distance;
}

static int trees_cb ( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
    pp::Image *treeImg;
    int sx, sy, sz;
    int x,y;
    int pad;
    tree_type_t *which_tree;
    item_type_t *which_item;
    int i;
    int best_tree_dist, best_item_dist;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <tree location bitmap>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    treeImg = pp::Image::readFile( argv[1] );
    if ( treeImg->data == NULL ) {
	print_warning( TCL_WARNING, "%s: couldn't load %s", 
		       argv[0], argv[1] );
        Tcl_AppendResult(ip, argv[0], ": couldn't load ", argv[1], 
			 (char *)0 );
	return TCL_ERROR;
    }

    if ( num_tree_types == 0 && num_item_types == 0 ) {
	print_warning( IMPORTANT_WARNING,
		       "tux_trees callback called with no tree or item "
		       "types set" );
    }

    sx = treeImg->width;
    sy = treeImg->height;
    sz = treeImg->depth;

    for (i = 0; i < num_tree_types; i++) {
		tree_types[i].num_trees = 0;
    }

    for (i = 0; i < num_item_types; i++) {
		item_types[i].num_items = 0;
    }

    num_trees = 0;
    num_items = 0;
    pad = 0;
    for (y=0; y<sy; y++) {
        for (x=0; x<sx; x++) {
			
			unsigned char* pixel=&treeImg->data[ (x + y*sx)*sz + pad ];
			if(pixel[0]!=0 || pixel[1]!=0 || pixel[2]!=0){
				
				best_tree_dist = is_tree ( pixel,&which_tree );
            	best_item_dist = is_item ( pixel,&which_item );

	    		if ( best_tree_dist < best_item_dist && which_tree != NULL ) {
        		if (num_trees+1 == MAX_TREES ) {
                    fprintf( stderr, "%s: maximum number of trees reached.\n", 
					argv[0] );
            	        break;
            	    }
					num_trees += 1;
					which_tree->num_trees += 1;
										
					which_tree->pos.push_back(pp::Vec2d(
						(sx-x)/(double)(sx-1.)*course_width,
						-(sy-y)/(double)(sy-1.)*course_length));
					
				} else if ( which_item != NULL ) {
                	if (num_items+1 == MAX_ITEMS ) {
                    	fprintf( stderr, "%s: maximum number of items reached.\n", 
			   	 		argv[0] );
             			break;
            	    }
					num_items += 1;
					which_item->num_items += 1;
					
					which_item->pos.push_back(pp::Vec2d(
						(sx-x)/(double)(sx-1.)*course_width,
						-(sy-y)/(double)(sy-1.)*course_length));
					
					}
        		}
			}
			//pad += ( sx * sz ) % 4; /* to compensate for word-aligned rows */
    }

    /*
    // double pass so that tree and object types are clumped together - reduce
    // texture switching
    */
	std::list<pp::Vec2d>::iterator it;
    num_trees = 0;
    for (i = 0; i < num_tree_types; i++) {
			
		for(it=tree_types[i].pos.begin();it!=tree_types[i].pos.end();it++){
		    tree_locs[num_trees].ray.pt.x = (*it).x;
	    	tree_locs[num_trees].ray.pt.z = (*it).y;
	    	tree_locs[num_trees].ray.pt.y = find_y_coord( (*it).x, (*it).y ) + tree_types[i].height;
	    	tree_locs[num_trees].ray.vec = pp::Vec3d( 0, 1, 0);

	   		//tree_locs[num_trees].height = (double)rand()/RAND_MAX*tree_types[i].vary*2;
	    	//tree_locs[num_trees].height -= tree_types[i].vary;
	    	//tree_locs[num_trees].height = tree_types[i].height + 
			//tree_locs[num_trees].height * tree_types[i].height;
	    	//tree_locs[num_trees].diam = (tree_locs[num_trees].height /
			//tree_types[i].height) * tree_types[i].diam;
			
			tree_locs[num_trees].diam = tree_types[i].diam;
	    	tree_locs[num_trees].type = i;
			tree_locs[num_trees].setModel(tree_types[i].model);
			tree_locs[num_trees].setPolyhedron(tree_types[i].ph);
		
	    	num_trees++;
		}
		tree_types[i].pos.clear();
    }

    num_items = 0;
    for (i = 0; i < num_item_types; i++) {
		
		for(it=item_types[i].pos.begin();it!=item_types[i].pos.end();it++){

	    	item_locs[num_items].ray.pt.x = (*it).x;
	    	item_locs[num_items].ray.pt.z = (*it).y;
	    	item_locs[num_items].ray.pt.y = find_y_coord( (*it).x, (*it).y )
					    + item_types[i].above_ground;
			item_locs[num_items].ray.vec = pp::Vec3d(0, 1, 0);

	    	item_locs[num_items].height = item_types[i].height ; 
	    	item_locs[num_items].diam = item_types[i].diam;
	    	item_locs[num_items].type = i;
			item_locs[num_items].setType(item_types[i].type);
			item_locs[num_items].setScore(item_types[i].score);

	    	if ( item_types[i].reset_point )  {
				item_locs[num_items].setDrawable(false);
	    	} else {
				item_locs[num_items].setDrawable(true);
	    	}
	    	num_items++;
		}
		item_types[i].pos.clear();
    }

    delete treeImg;

    return TCL_OK;
}

static int friction_cb ( ClientData cd, Tcl_Interp *ip, 
			 int argc, CONST84 char *argv[]) 
{
    double fric[4];
    float fric_s[4];
    unsigned int i;

    if ( argc != 5 ) {
        fprintf( stderr, "Usage: %s <ice> <rock> <snow> <ramp>", argv[0] );
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <ice coeff.> <rock coeff.> "
			 "<snow coeff.> <ramp coeff.>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[1], &fric[0] ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid ice coefficient",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[2], &fric[1] ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid rock coefficient",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetDouble( ip, argv[3], &fric[2] ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid snow coefficient",
			 (char *)0 );
        return TCL_ERROR;
    }
	 
	if ( Tcl_GetDouble( ip, argv[4], &fric[3] ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid ramp coefficient",
			 (char *)0 );
        return TCL_ERROR;
    }
	

    for ( i=0; i<sizeof(fric)/sizeof(fric[0]); i++) {
	fric_s[i] = fric[i];
    }

    set_friction_coeff( fric_s );

    return TCL_OK;
} 

static int course_author_cb( ClientData cd, Tcl_Interp *ip, 
			     int argc, CONST84 char *argv[]) 
{
    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <author's name>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    courseAuthor = argv[1];

    return TCL_OK;
} 

static int course_name_cb( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{
    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <course name>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    courseName = argv[1];

    return TCL_OK;
} 

static int base_height_value_cb( ClientData cd, Tcl_Interp *ip, 
				 int argc, CONST84 char *argv[]) 
{
    int value;

    if ( argc != 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <base height>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetInt( ip, argv[1], &value ) != TCL_OK ) {
        Tcl_AppendResult(ip, argv[0], ": invalid base height",
			 (char *)0 );
        return TCL_ERROR;
    } 

    base_height_value = value;

    return TCL_OK;
}

static int tree_model_cb( ClientData cd, Tcl_Interp *ip, 
				 int argc, CONST84 char *argv[])
{
	if ( num_tree_types + 1 >= MAX_TREE_TYPES ) {
		Tcl_AppendResult(ip, argv[0], ": max number of tree types reached",
			 (char *)0 );
		return TCL_ERROR;
    }
	
    tree_types[num_tree_types].height = 1.0;
	tree_types[num_tree_types].diam = 1.0;
    tree_types[num_tree_types].num_trees = 0;
    tree_types[num_tree_types].red = 255;
    tree_types[num_tree_types].green = 255;
    tree_types[num_tree_types].blue = 255;
	tree_types[num_tree_types].model = NULL;
	

	int error = 0;
	
	for ( int i = 1; (i < argc - 1) && !error; i += 2 ) {
		if ( strcmp( "-name", argv[i] ) == 0 ) {
		    tree_types[num_tree_types].name = argv[i+1];

		} else if ( strcmp( "-height", argv[i] ) == 0 ) {
	    	if ( Tcl_GetDouble( ip, argv[i+1],
		   		&tree_types[num_tree_types].height) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid height",
					(char *)0 );
			error = 1;
	    	}
		} else if ( strcmp( "-diam", argv[i] ) == 0 ) {
	    	if ( Tcl_GetDouble( ip, argv[i+1],
		   		&tree_types[num_tree_types].diam) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid diameter",
					(char *)0 );
			error = 1;
		
	    	}
			
		} else if ( strcmp( "-model", argv[i] ) == 0 ) {
			tree_types[num_tree_types].model =
				new pp::Model(argv[i+1]);
			
		} else if ( strcmp( "-color", argv[i] ) == 0 ) {
			int         rtn, num_col;
    		CONST84 char **     indices = 0;
			
	    	rtn = Tcl_SplitList(ip, argv[i+1], &num_col, &indices);
			if( rtn != TCL_OK ) {
				Tcl_AppendResult(ip, "a list of colors must be provided\n",
			    		(char *) 0);
				Tcl_Free((char *) indices);
				error = 1;
	    	}
    
			int convert_temp;

	    	if (num_col == 3 || num_col == 4) {
				Tcl_GetInt(ip, indices[0], &convert_temp);
				tree_types[num_tree_types].red = (unsigned char) convert_temp;
				Tcl_GetInt(ip, indices[1], &convert_temp);
				tree_types[num_tree_types].green = (unsigned char) convert_temp;
				Tcl_GetInt(ip, indices[2], &convert_temp);
				tree_types[num_tree_types].blue = (unsigned char) convert_temp;
	    	} else {
				Tcl_AppendResult(ip, argv[0], ": must specify three colors"
				" to link with tree type", (char *) 0);
				error = 1;
	    	}
	    	Tcl_Free((char *) indices);
		} 
	}
	
	tree_types[num_tree_types].ph = 
			tree_types[num_tree_types].model->getPolyhedron();	
	
	num_tree_types += 1;
	return TCL_OK;
}


static int item_spec_cb( ClientData cd, Tcl_Interp *ip, 
				 int argc, CONST84 char *argv[])
{
    int          rtn, num_col;
    CONST84 char **      indices = NULL;
    int          convert_temp;
    char *       err_msg = "";
    char         buff[BUFF_LEN];

    if ( num_item_types + 1 >= MAX_ITEM_TYPES ) {
	Tcl_AppendResult(ip, argv[0], ": max number of item types reached",
			 (char *)0 );
	return TCL_ERROR;
    }

    item_types[num_item_types].diam = .8;
    item_types[num_item_types].height = 0.5;
    item_types[num_item_types].above_ground = 0.0;
    item_types[num_item_types].red = 255;
    item_types[num_item_types].green = 255;
    item_types[num_item_types].blue = 255;
	item_types[num_item_types].type = Item::UNCOLLECTABLE;
    item_types[num_item_types].reset_point = false;
    item_types[num_item_types].num_items = 0;
    item_types[num_item_types].use_normal = false;
	item_types[num_item_types].score=1;

    NEXT_ARG;

    while ( *argv != NULL ) {
	if ( strcmp( "-name", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-name", err_msg, item_spec_bail );

	    item_types[num_item_types].name = *argv;

	} else if ( strcmp( "-height", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-height", err_msg, item_spec_bail );

	    if ( Tcl_GetDouble( ip, *argv,
		    &item_types[num_item_types].height) != TCL_OK ) {
		Tcl_AppendResult(ip, argv[0], ": invalid height\n",
					(char *) 0);
	    }

	} else if ( strcmp( "-diameter", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-diameter", err_msg, item_spec_bail );

	    if ( Tcl_GetDouble( ip, *argv,
		    &item_types[num_item_types].diam) != TCL_OK ) {
		Tcl_AppendResult(ip, argv[0], ": invalid diameter\n",
					(char *) 0);
	    }
	
	} else if ( strcmp( "-texture", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-texture", err_msg, item_spec_bail );

	    if ( item_types[num_item_types].texture.empty() ) {
			item_types[num_item_types].texture = *argv;
	    } else {
			Tcl_AppendResult(ip, argv[0], ": specify only one texture\n",
				(char *)0 );
	    }

	} else if ( strcmp( "-above_ground", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-above_ground", err_msg, item_spec_bail );

	    if ( Tcl_GetDouble( ip, *argv,
		    &item_types[num_item_types].above_ground) != TCL_OK ) {
		Tcl_AppendResult(ip, argv[0], ": invalid height above ground\n",
					(char *) 0);
	    }

	} else if ( strcmp( "-score", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-score", err_msg, item_spec_bail );

	    if ( Tcl_GetInt( ip, *argv,
		    &item_types[num_item_types].score) != TCL_OK ) {
		Tcl_AppendResult(ip, argv[0], ": invalid score\n",
					(char *) 0);
	    }

	} else if ( strcmp( "-color", *argv ) == 0 ) 
	{
	    NEXT_ARG;
	    CHECK_ARG( "-color", err_msg, item_spec_bail );

	    rtn = Tcl_SplitList(ip, *argv, &num_col, &indices);
	    if( rtn != TCL_OK ) {
		err_msg = "Must provide a list of colors for -color";
		goto item_spec_bail; 
	    }

	    if (num_col == 3 || num_col == 4) {
		Tcl_GetInt(ip, indices[0], &convert_temp);
		item_types[num_item_types].red = (unsigned char) convert_temp;
		Tcl_GetInt(ip, indices[1], &convert_temp);
		item_types[num_item_types].green = (unsigned char) convert_temp;
		Tcl_GetInt(ip, indices[2], &convert_temp);
		item_types[num_item_types].blue = (unsigned char) convert_temp;
	    } else {
		err_msg = "Color specification must have 3 or 4 elements";
		goto item_spec_bail;
	    }
	    Tcl_Free((char *) indices);
	    indices = NULL;

	} else if ( strcmp( "-type", *argv ) == 0) {
		NEXT_ARG;
	    CHECK_ARG( "-type", err_msg, item_spec_bail );
		
		if(!strcmp(*argv,"herring")){
			item_types[num_item_types].type=Item::HERRING;
		}else if (!strcmp(*argv,"life")){
			item_types[num_item_types].type=Item::LIFE;
		}	
	} else if ( strcmp( "-reset_point", *argv ) == 0 ) {
	    item_types[num_item_types].reset_point = true;
	    item_types[num_item_types].type = Item::UNCOLLECTABLE;
	} else if ( strcmp( "-normal", *argv ) == 0 ) {
	    NEXT_ARG;
	    CHECK_ARG( "-normal", err_msg, item_spec_bail );

	    if ( get_tcl_tuple( 
		ip, *argv, (double*)&(item_types[num_item_types].normal), 3 )
		 != TCL_OK )
	    {
		err_msg = "Must specify a list of size three for -normal";
		goto item_spec_bail;
	    }

	    item_types[num_item_types].normal.normalize();

	    item_types[num_item_types].use_normal = true;

	} else {
	    sprintf( buff, "Unrecognized option `%s'", *argv );
	    goto item_spec_bail;
	}

	NEXT_ARG;
    }

    if ( item_types[num_item_types].name.empty() ||
	 ( item_types[num_item_types].texture.empty() &&
	   item_types[num_item_types].reset_point == false ) ) 
    {
	err_msg = "Some mandatory elements not filled.  "
	    "Item name and texture name must be supplied.";
	goto item_spec_bail;
    }

    if ( item_types[num_item_types].reset_point == false &&
	 !bind_texture( item_types[num_item_types].name.c_str(),
			item_types[num_item_types].texture.c_str() )) 
    {
	err_msg = "could not bind specified texture";
	goto item_spec_bail;
    }

    num_item_types += 1;
    return TCL_OK;

item_spec_bail:
    if ( indices ) {
	Tcl_Free( (char*) indices );
	indices = NULL;
    }

	item_types[num_item_types].name.erase();
	item_types[num_item_types].texture.erase();

        

    Tcl_AppendResult(
	ip,
	"Error in call to tux_item_spec: ", 
	err_msg,
	"\n",
	"Usage: tux_item_spec -name <name> -height <height> "
	"-diameter <diameter> -color {r g b [a]} "
	"[-texture <texture>] [-above_ground <distance>] [-score <score>] "
	"[-nocollect] [-reset_point] [-normal {x y z}]",
	(NULL) );
    return TCL_ERROR;
}

static int wind_velocity_cb( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
	pp::Vec3d velocity;
	double scale;
	char *err_msg = "";
	int rtn, num_col;
	CONST84 char ** indices = NULL;
	int convert_temp;

    NEXT_ARG;

    while ( *argv != NULL ){
		if ( strcmp( "-scale", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-scale", err_msg, item_spec_bail );		

		    if ( Tcl_GetDouble( ip, *argv,
			    &scale) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid scale\n",
						(char *) 0);
		    }
		}else if ( strcmp( "-velocity", *argv ) == 0 ||
		    strcmp( "-velocity", *argv ) == 0 ){
		    NEXT_ARG;
		    CHECK_ARG( "-velocity", err_msg, item_spec_bail );
	
		    rtn = Tcl_SplitList(ip, *argv, &num_col, &indices);
	    	if( rtn != TCL_OK ) {
				err_msg = "Must provide a list of velocities for -velocity";
				goto item_spec_bail; 
			}

			if (num_col == 3) {
				Tcl_GetInt(ip, indices[0], &convert_temp);
				velocity.x = (unsigned char) convert_temp;
				Tcl_GetInt(ip, indices[1], &convert_temp);
				velocity.y = (unsigned char) convert_temp;
				Tcl_GetInt(ip, indices[2], &convert_temp);
				velocity.z = (unsigned char) convert_temp;
		    } else {
				err_msg = "Velocity specification must have 3 elements";
				goto item_spec_bail;
	    	}
			Tcl_Free((char *) indices);
	    	indices = NULL;
		}

		NEXT_ARG;
    }	
	
	set_wind_velocity(velocity,scale);
    return TCL_OK;

item_spec_bail:
    if ( indices ){
	Tcl_Free( (char*) indices );
	indices = NULL;
    }

    Tcl_AppendResult(
		ip,
		"Error in call to tux_wind_velocity: ", 
		err_msg,
		"\n",
		"Usage: tux-wind_velocity -velocity {x y z} -scale <scale>",
		(NULL) );
    return TCL_ERROR;
}


static int hud_cb( ClientData cd, Tcl_Interp *ip, int argc, CONST84 char *argv[]) 
{
	char *err_msg = "";
	int rtn, num_col;
	CONST84 char ** indices = NULL;
	int convert_temp;
	
	int hud=1, element_num=-1;
	
	HUD::Element element;
	
    NEXT_ARG;

    while ( *argv != NULL ){
		if ( strcmp( "-hud", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-hud", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &hud) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid HUD\n",
						(char *) 0);
		    }
		}else if ( strcmp( "-element", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-element", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &element_num) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid HUD element number\n",
						(char *) 0);
		    }
		}else if ( strcmp( "-type", *argv ) == 0 ) {
			NEXT_ARG;
			CHECK_ARG( "-typr", err_msg, item_spec_bail );
			if(!strcmp("text",*argv)){
				element.type=0;
			}else if(!strcmp("fsb",*argv)){
				element.type=1;
			}else if(!strcmp("herring",*argv)){
				element.type=2;
			}else if(!strcmp("image",*argv)){
				element.type=3;
			}else if(!strcmp("time",*argv)){
				element.type=4;
			}else if(!strcmp("speed",*argv)){
				element.type=5;	
			}else if(!strcmp("gauge",*argv)){
				element.type=6;	
			}else if(!strcmp("energybar",*argv)){
				element.type=7;	
			}else if(!strcmp("speedbar",*argv)){
				element.type=8;	
			}else if(!strcmp("percentage",*argv)){
				element.type=9;	
			}else if(!strcmp("percentagebar",*argv)){
				element.type=10;	
			}else{
				err_msg = "invalid type";
				goto item_spec_bail;
			}					
		}else if ( strcmp( "-position", *argv ) == 0 ||
		    strcmp( "-position", *argv ) == 0 ){
		    NEXT_ARG;
		    CHECK_ARG( "-position", err_msg, item_spec_bail );
	
		    rtn = Tcl_SplitList(ip, *argv, &num_col, &indices);
	    	if( rtn != TCL_OK ) {
				err_msg = "Must provide a list with x and y for -position";
				goto item_spec_bail; 
			}
			if (num_col == 2) {
				Tcl_GetInt(ip, indices[0], &convert_temp);
				element.x = convert_temp;
				Tcl_GetInt(ip, indices[1], &convert_temp);
				element.y = convert_temp;
		    } else {
				err_msg = "Position must have 2 elements";
				goto item_spec_bail;
	    	}
			Tcl_Free((char *) indices);
	    	indices = NULL;
		} else if ( strcmp( "-texture", *argv ) == 0 ) {
		    NEXT_ARG;
			CHECK_ARG( "-texture", err_msg, item_spec_bail );
			
			if ( !get_texture_binding(*argv, &element.texture ) ) {
				err_msg = "Couldn't get texture";
				goto item_spec_bail;
			}
		} else if ( strcmp( "-font", *argv ) == 0 ) {
		    NEXT_ARG;
			CHECK_ARG( "-font", err_msg, item_spec_bail );
	
			element.font = pp::Font::get(*argv);
			if( element.font==NULL){
				err_msg = "Couldn't get font";
				goto item_spec_bail;
			}else{
				element.height = int(element.font->ascender());			
			}			
		} else if ( strcmp( "-string", *argv ) == 0 ) {
			NEXT_ARG;
			CHECK_ARG( "-string", err_msg, item_spec_bail );
	   		if ( element.string.empty() ) {
				element.string = _(*argv);
	    	} else {
				err_msg = "Specify only one string";
				goto item_spec_bail;
	    	}
		} else if ( strcmp( "-angle", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-angle", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &element.angle) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid angle\n",
						(char *) 0);
		    }
		} else if ( strcmp( "-width", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-width", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &element.width) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid width\n",
						(char *) 0);
		    }
		} else if ( strcmp( "-height", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-height", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &element.height) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid height\n",
						(char *) 0);
		    }
		} else if ( strcmp( "-size", *argv ) == 0 ) {
		    NEXT_ARG;
		    CHECK_ARG( "-size", err_msg, item_spec_bail );		

		    if ( Tcl_GetInt( ip, *argv,
			    &element.size) != TCL_OK ) {
			Tcl_AppendResult(ip, argv[0], ": invalid size\n",
						(char *) 0);
		    }
		}
		
		NEXT_ARG;
    }
	
	
	//in case of a text element we're precalculating
	//the width and the unicode string
	if(element.type==0 && element.font && !element.string.empty()){
		pp::Font::utf8ToUnicode(element.u_string,element.string.c_str());
		element.width = int(element.font->advance(element.u_string));
	}
	
	if(hud==2){
		//no multiplayer support
	}else{
		if (element_num==-1){
			HUD1.add(element);
		}else{
			HUD1.update(element_num,element);
		}
	}
	
    return TCL_OK;

item_spec_bail:
    if ( indices ){
	Tcl_Free( (char*) indices );
	indices = NULL;
    }

    Tcl_AppendResult(
		ip,
		"Error in call to tux_hud: ", 
		err_msg,
		"\n",
		"Usage: tux_hud -hud <hud> -element <element number> -type <type> -position {x y} -binding <binding> -string <string> -width <width> -height <height> -size <size> -angle <angle>",
		"\n",
		"Valid types: text, fsb, herring, image, time, speed, gauge, energybar and speedbar",
		(NULL) );
    return TCL_ERROR;
}


void register_course_load_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_course_dim", course_dim_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_angle",      angle_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_elev_scale", elev_scale_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_elev",       elev_cb,        0,0);
    Tcl_CreateCommand (ip, "tux_load_terrain",    terrain_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_trees",      trees_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_bgnd_img",   bgnd_img_cb,   0,0);
	Tcl_CreateCommand (ip, "tux_terrain_tex",   terrain_tex_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_start_pt",   start_pt_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_friction",   friction_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_course_author", course_author_cb, 0,0);
    Tcl_CreateCommand (ip, "tux_course_name", course_name_cb, 0,0);
    Tcl_CreateCommand (ip, "tux_base_height_value", base_height_value_cb, 0,0);
    Tcl_CreateCommand (ip, "tux_tree_model",  tree_model_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_item_spec",  item_spec_cb,   0,0);
	Tcl_CreateCommand (ip, "tux_wind_velocity",  wind_velocity_cb,   0,0);
	Tcl_CreateCommand (ip, "tux_hud",  hud_cb,   0,0);
}
