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

#ifndef _HIER_H_
#define _HIER_H_

#include "pp_types.h"

#include "ppgltk/alg/poly.h"


#define MIN_SPHERE_DIVISIONS 3
#define MAX_SPHERE_DIVISIONS 16

extern int get_scene_node( const char *node_name, scene_node_t **node );

extern char* reset_scene_node(char *node);
extern char* rotate_scene_node(const char *node, char axis, double angle);
extern char* translate_scene_node(const char *node, pp::Vec3d trans);
extern char* scale_scene_node(const char *node, pp::Vec3d origin, double factor[3]);
extern char* transform_scene_node(char *node, pp::Matrix mat, pp::Matrix invMat);

extern char* set_scene_node_material(const char *node, const char *mat);
extern char* create_material(const char *mat, pp::Color d, pp::Color s, double s_exp);

extern char* set_scene_resolution(char *resolution);

extern char* set_scene_node_shadow_state( const char *node, const char *state );
extern char* set_scene_node_eye( const char *node, const char *which_eye );

extern char* create_tranform_node(const char *parent, const char *name);
extern char* create_sphere_node( const char *parent_name, const char *child_name, double resolution );

extern void initialize_scene_graph();

extern void draw_scene_graph( char *node );
extern  bool collide( char *node, pp::Polyhedron ph );

#endif
