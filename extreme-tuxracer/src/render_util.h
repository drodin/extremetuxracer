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

#ifndef _RENDER_UTIL_H_
#define _RENDER_UTIL_H_

#include "ppgltk/alg/color.h"
#include "ppgltk/alg/vec2d.h"
#include "ppgltk/alg/vec3d.h"

//#include "player.h"
//reduce dependencies
class Player;


#define NEAR_CLIP_DIST 0.1


void reshape( int w, int h );
void flat_mode();
void draw_overlay();
void clear_rendering_context();
void configure_fog();
void set_material( const pp::Color diffuse, const pp::Color specular,
		   const double specular_exp );

void draw_billboard( Player& plyr,
		     pp::Vec3d center_pt, double width, double height, 
		     bool use_world_y_axis, 
		     pp::Vec2d min_tex_coord, pp::Vec2d max_tex_coord );

#endif // _RENDER_UTIL_H_
