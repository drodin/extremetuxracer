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

#include "pp_types.h"
#include "pp_classes.h"

#include "ppgltk/ppgltk.h"

#include "ppgltk/model.h"


#include <list>
#include <string>

#ifndef _COURSE_LOAD_H_
#define _COURSE_LOAD_H_

#define STRIDE_GL_ARRAY ( 8 * sizeof(GLfloat) + 4 * sizeof(GLubyte) )

/* Convenience macro for accessing terrain elevations */
#define ELEV(x,y) ( elevation[(x) + nx*(y)] )

/* Convenience acro to create a course vertex */
#define COURSE_VERTEX(x,y) pp::Vec3d( (float)(x)/(nx-1.)*course_width, \
                       ELEV((x),(y)), -(float)(y)/(ny-1.)*course_length ) 

void load_course( std::string& course );

float     *get_course_elev_data();
int    *get_course_terrain_data();
float      get_course_angle();
void          get_course_dimensions( float *width, float *length );
void          get_play_dimensions( float *width, float *length );
float      get_terrain_base_height( float distance );
float      get_terrain_max_height( float distance );
void          get_course_divisions( int *nx, int *ny );
Tree       *get_tree_locs();
int           get_num_trees();
void          set_start_pt( pp::Vec2d p );
pp::Vec2d     get_start_pt();
pp::Polyhedron*  get_tree_polyhedron( int type );
const char         *get_tree_name(int type);

std::string& get_course_author();
std::string& get_course_name(); 

light_t      *get_course_lights();
void          set_course_mirroring( bool state );
bool        get_course_mirroring( );
void          fill_gl_arrays();
void          get_gl_arrays( GLubyte **vertex_normal_arr );

void          register_course_load_tcl_callbacks( Tcl_Interp *interp );

Item       *get_item_locs();
int           get_num_items();
const char         *get_item_name(int type);

typedef struct {
	std::string name;
    double height;
	double diam;
    int  num_trees;
    unsigned char red, green, blue;
    std::list<pp::Vec2d> pos;
	pp::Model* model;
	pp::Polyhedron* ph;
} tree_type_t;

typedef struct {
    std::string name;
    std::string texture;
    double diam, height;
    double above_ground;
    unsigned char red, green, blue;
    //bool nocollision;
	Item::Type type;
	int score;
    bool reset_point;
    std::list<pp::Vec2d> pos;
    int  num_items;
    bool use_normal;
    pp::Vec3d normal;
} item_type_t;

typedef struct {
    int type;
	int value;
	double friction;
	double compression;
	GLuint texbind;
	GLuint partbind;
	GLuint envmapbind;
	std::string sound;
	bool soundactive;
	struct { GLuint head,mark,tail; }trackmark;
	int wheight;
	int count;
} terrain_tex_t;


item_type_t*	get_item_types();
int		get_num_item_types();


extern std::list<int> usedTerrains;

#endif
