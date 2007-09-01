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

#ifndef _HIER_UTIL_H
#define _HIER_UTIL_H

#include "pp_types.h"

#include "ppgltk/alg/poly.h"

void draw_sphere( int num_divisions );

void traverse_dag( scene_node_t *node, material_t *mat );

pp::Vec3d make_normal( pp::Polygon p, pp::Vec3d *v );

bool intersect_polygon( pp::Polygon p, pp::Vec3d *v );

bool intersect_polyhedron( pp::Polyhedron p );

pp::Polyhedron copy_polyhedron( pp::Polyhedron ph );

void free_polyhedron( pp::Polyhedron ph ) ;

void trans_polyhedron( pp::Matrix mat, pp::Polyhedron ph );

bool  check_polyhedron_collision_with_dag( 
    scene_node_t *node, pp::Matrix modelMatrix, pp::Matrix invModelMatrix,
    pp::Polyhedron ph );

#endif
