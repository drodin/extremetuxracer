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

#include "viewfrustum.h"
#include "game_config.h"

#include "ppgltk/alg/defs.h"

#define DOT_PRODUCT( v1, v2 ) ((double) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z))


static pp::Plane frustum_planes[6];

/* This will be used as a bitfield to select the "n" and "p" vertices
of the bounding boxes wrt to each plane.  
*/

//static char p_vertex_code[6];
static bool p_vertex_code_x[6];
static bool p_vertex_code_y[6];
static bool p_vertex_code_z[6];


void setup_view_frustum( Player& plyr, double near_dist, 
			 double far_dist )
{
    double aspect = (double) getparam_x_resolution() /
	getparam_y_resolution();

    int i;
    pp::Vec3d pt;
    pp::Vec3d origin(0., 0., 0.);
    double half_fov = ANGLES_TO_RADIANS( getparam_fov() * 0.5 );
    double half_fov_horiz = atan( tan( half_fov ) * aspect ); 


    /* create frustum in viewing coordinates */

    /* near */
    frustum_planes[0] = pp::Plane(0, 0, 1, near_dist);
    
    /* far */
    frustum_planes[1] = pp::Plane(0, 0, -1, -far_dist);

    /* left */
    frustum_planes[2] = pp::Plane( -cos(half_fov_horiz), 0, 
				    sin(half_fov_horiz), 0 );

    /* right */
    frustum_planes[3] = pp::Plane( cos(half_fov_horiz), 0, 
				    sin(half_fov_horiz), 0 );

    /* top */
    frustum_planes[4] = pp::Plane( 0, cos(half_fov),  
				    sin(half_fov), 0 );

    /* bottom */
    frustum_planes[5] = pp::Plane( 0, -cos(half_fov),  
				    sin(half_fov), 0 );


    /* We now transform frustum to world coordinates */
    for (i=0; i<6; i++) {
	pt = plyr.view.inv_view_mat.transformPoint(
	    origin + (-frustum_planes[i].d*frustum_planes[i].nml) );

	frustum_planes[i].nml = plyr.view.inv_view_mat.transformVector( frustum_planes[i].nml );

	frustum_planes[i].d = -( 
	    frustum_planes[i].nml*
	    (pt-origin));
    }

    for (i=0; i<6; i++) {
		p_vertex_code_x[i] = false;
		p_vertex_code_y[i] = false;
		p_vertex_code_z[i] = false;
		
		
	if ( frustum_planes[i].nml.x > 0 ) {
		p_vertex_code_x[i] = true;
	}
	if ( frustum_planes[i].nml.y > 0 ) {
		p_vertex_code_y[i] = true;
	}
	if ( frustum_planes[i].nml.z > 0 ) {
		p_vertex_code_z[i] = true;
	}

	
    }
}

/** View frustum clipping for AABB (axis-aligned bounding box). See
   Assarsson, Ulf and Tomas M\"oller, "Optimized View Frustum Culling
   Algorithms", unpublished, http://www.ce.chalmers.se/staff/uffe/ .  */
clip_result_t clip_aabb_to_view_frustum( const pp::Vec3d& min, const pp::Vec3d& max )
{
    pp::Vec3d n, p;
    clip_result_t intersect = NoClip;

    for (int i=5; i>=0; i--) {
		p = min;
		n = max;
		
		if ( p_vertex_code_x[i]) {
		    p.x = max.x;
		    n.x = min.x;
		}

		if ( p_vertex_code_y[i]) {
		    p.y = max.y;
		    n.y = min.y;
		}

		if ( p_vertex_code_z[i]) {
		    p.z = max.z;
		    n.z = min.z;
		}

		if ( DOT_PRODUCT( n, frustum_planes[i].nml ) +
		     frustum_planes[i].d > 0 )
		{
		    return NotVisible;
		}

		if ( DOT_PRODUCT( p, frustum_planes[i].nml ) +
		     frustum_planes[i].d > 0 )
		{
			intersect = SomeClip;
		}

    }	
    return intersect;
}

pp::Plane get_far_clip_plane()
{
    return frustum_planes[1];
}

pp::Plane get_left_clip_plane()
{
    return frustum_planes[2];
}

pp::Plane get_right_clip_plane()
{
    return frustum_planes[3];
}

pp::Plane get_bottom_clip_plane()
{
    return frustum_planes[5];
}
