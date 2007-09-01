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

#ifndef _COURSE_QUAD_H_
#define _COURSE_QUAD_H_

void reset_course_quadtree();

void init_course_quadtree( float *elevation, int nx, int nz, 
			   double scalex, double scalez,
			   pp::Vec3d view_pos, double detail );

void update_course_quadtree( const pp::Vec3d view_pos, const float detail );

void render_course_quadtree();

#endif /* _COURSE_QUAD_H_ */
