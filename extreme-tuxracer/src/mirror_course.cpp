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
#include "keyframe.h"
#include "phys_sim.h"
#include "course_quad.h"
#include "track_marks.h"
#include "game_config.h"
#include "course_mgr.h"

static bool mirrored = false;

void mirror_course() 
{
    int x, y, i;
    int idx1, idx2;
    float tmp;
    int tmp_terrain;
    pp::Vec3d tmp_vec;
    float *elevation;
    pp::Vec3d *nmls;
    int *terrain;
    Tree *tree_locs;
    int num_trees;
    Item *item_locs;
    int num_items;
    pp::Vec2d start_pt;
    int nx, ny;
    float course_width, course_length;

    get_course_dimensions( &course_width, &course_length );
    get_course_divisions( &nx, &ny );
    elevation = get_course_elev_data();
    terrain = get_course_terrain_data();
    nmls = get_course_normals();
    tree_locs = get_tree_locs();
    item_locs = get_item_locs();

    for ( y=0; y<ny; y++ ) {
	for ( x=0; x<nx/2; x++ ) {
	    tmp = ELEV(x,y);
	    ELEV(x,y) = ELEV(nx-1-x, y);
	    ELEV(nx-1-x,y) = tmp;

	    /* first column of texture values not used */
            idx1 = (x+1) + nx*(y);
            idx2 = (nx-1-x) + nx*(y);
	    tmp_terrain = terrain[idx1];
	    terrain[idx1] = terrain[idx2];
	    terrain[idx2] = tmp_terrain;

            idx1 = (x) + nx*(y);
            idx2 = (nx-1-x) + nx*(y);
	    tmp_vec = nmls[idx1];
	    nmls[idx1] = nmls[idx2];
	    nmls[idx2] = tmp_vec;
	    nmls[idx1].x *= -1;
	    nmls[idx2].x *= -1;
	}
    }

    num_trees = get_num_trees();
    for ( i=0; i<num_trees; i++) {
	tree_locs[i].ray.pt.x = course_width - tree_locs[i].ray.pt.x; 
	tree_locs[i].ray.pt.y = 
	    find_y_coord( tree_locs[i].ray.pt.x,
			  tree_locs[i].ray.pt.z );
    }

    num_items = get_num_items();
    for ( i=0; i<num_items; i++) {
	item_locs[i].ray.pt.x = course_width - item_locs[i].ray.pt.x; 
	item_locs[i].ray.pt.y = 
	    find_y_coord( item_locs[i].ray.pt.x,
			  item_locs[i].ray.pt.z );
    }

    fill_gl_arrays();

    reset_course_quadtree();
    if ( nx > 0 && ny > 0 ) {
	print_debug( DEBUG_QUADTREE, "mirroring quadtree" );
	init_course_quadtree( elevation, nx, ny, course_width/(nx-1), 
			      -course_length/(ny-1),
			      players[0].view.pos, 
			      getparam_course_detail_level() );
    }

    start_pt = get_start_pt();
    start_pt.x = course_width - start_pt.x;
    set_start_pt( start_pt );
}

void mirror_key_frame()
{
    int i;
    float course_width, course_length;
    int num_frames;
    key_frame_t *frames;

    get_key_frame_data( &frames, &num_frames );

    get_course_dimensions( &course_width, &course_length );

    for ( i=0; i<num_frames; i++ ) {
	frames[i].yaw = - frames[i].yaw;
	frames[i].pos.x = course_width - frames[i].pos.x;
    }
}

void set_course_mirroring( bool state )
{
    if ( mirrored != state ) {
	mirror_key_frame();
	mirror_course();
	init_track_marks();
    }
    mirrored = state;
    
}

bool get_course_mirroring( )
{
    return mirrored;
}
