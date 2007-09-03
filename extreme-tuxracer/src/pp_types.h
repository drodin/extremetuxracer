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

#ifndef _PP_TYPES_H_
#define _PP_TYPES_H_

#include "etracer.h"

#include "ppgltk/alg/color.h"
#include "ppgltk/alg/vec2d.h"
#include "ppgltk/alg/vec3d.h"
#include "ppgltk/alg/matrix.h"

typedef struct {
    pp::Vec3d p[3];
    pp::Vec2d t[3];
} triangle_t;

/// Ray (half-line)
typedef struct { 
    pp::Vec3d pt;
    pp::Vec3d vec;
} ray_t;

/// Material
typedef struct {
    pp::Color diffuse;
    pp::Color specular;
    float specular_exp;
} material_t;

/// Light

typedef struct {
    bool is_on;
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat spot_direction[3];
    GLfloat spot_exponent;
    GLfloat spot_cutoff;
    GLfloat constant_attenuation;
    GLfloat linear_attenuation;
    GLfloat quadratic_attenuation;
} light_t;

/// Key frame for animation sequences
typedef struct {
    float time;
    pp::Vec3d pos;
	
	/// angle of rotation about y axis
    float yaw;
	
	/// angle of rotation about x axis
    float pitch;
    float l_shldr;
    float r_shldr;
    float l_hip;
    float r_hip;
} key_frame_t; 

/// Scene graph node types.
typedef enum { 
    Empty, Sphere
} geometry_t;

/// Data for Sphere node type.
typedef struct {
    float radius;
	
	/// How many divisions do we use to draw a sphere?
    int divisions;		
} sphere_t;

/// Tux's eyes
typedef enum {
    TuxLeftEye = 0, 
    TuxRightEye = 1
} tux_eye_t;
  
/// Scene graph node.
typedef struct scene_node_struct {
    struct scene_node_struct* parent;
    struct scene_node_struct* next;
    struct scene_node_struct* child;

	/// type of node
    geometry_t geom;

    union {
        sphere_t sphere;   
    } param;
      
    material_t* mat;


    /// Do we draw the shadow of this node?
    bool render_shadow;

    /// Is this node one of tux's eyes?
    bool eye;

    /// If so, which one?
    tux_eye_t which_eye;

    /// The forward and inverse transforms
    pp::Matrix trans;
    pp::Matrix invtrans;   

    /// name of node (for debugging)
    char *name;
} scene_node_t;


#define NUM_TERRAIN_TYPES 64

/// Difficulty levels
typedef enum {
    DIFFICULTY_LEVEL_EASY,
    DIFFICULTY_LEVEL_NORMAL,
    DIFFICULTY_LEVEL_HARD,
    DIFFICULTY_LEVEL_INSANE,
    DIFFICULTY_NUM_LEVELS
} difficulty_level_t;

/// Race conditions
typedef enum {
    RACE_CONDITIONS_SUNNY,
    RACE_CONDITIONS_CLOUDY,
    RACE_CONDITIONS_NIGHT,
	RACE_CONDITIONS_EVENING,
    RACE_CONDITIONS_NUM_CONDITIONS
} race_conditions_t;

/// View mode
typedef enum {
    BEHIND,
    FOLLOW,
    ABOVE,
    NUM_VIEW_MODES
} view_mode_t;

//// View point
typedef struct {
    /// View mode
	view_mode_t mode;       		
    
	/// position of camera
	pp::Vec3d pos;           		
    
	/// position of player
	pp::Vec3d plyr_pos;           	
    
	/// viewing direction
	pp::Vec3d dir;            		
    
	/// up direction
	pp::Vec3d up;               	
    
	/// inverse view matrix
	pp::Matrix inv_view_mat;    	
    
	/// has view been initialized?
	bool initialized;          		
} view_t;

/// Control mode
typedef enum {
    KEYBOARD = 0,
    MOUSE = 1,
    JOYSTICK = 2
} control_mode_t;

/// Control data
typedef struct {
    /// control mode
	control_mode_t mode; 
	
    /// turning [-1,1]
	float turn_fact;
	
    /// animation step [-1,1]
	float turn_animation;
	
    /// is player braking?
	bool is_braking;
	
    /// is player paddling?
	bool is_paddling;
	
    float paddle_time;
    bool begin_jump;
    bool jumping;
    bool jump_charging;
    float jump_amt;
    float jump_start_time;
    bool barrel_roll_left;
    bool barrel_roll_right;
    float barrel_roll_factor;
    bool front_flip;
    bool back_flip;
    float flip_factor;
} control_t;

#endif // _PP_TYPES_H_
