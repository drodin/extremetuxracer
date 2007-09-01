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
#include "quadtree.h"

#include "ppgltk/alg/defs.h"

#define CULL_DETAIL_FACTOR 25

static quadsquare *root = (quadsquare*) NULL;
static quadcornerdata root_corner_data = { (quadcornerdata*)NULL };


void reset_course_quadtree()
{
    delete root;
    root = (quadsquare*)NULL;
}

static int get_root_level( int nx, int nz )
{
    int xlev, zlev;

    check_assertion( nx > 0, "heightmap has x dimension of 0 size" );
    check_assertion( nz > 0, "heightmap has z dimension of 0 size" );

    xlev = (int) ( log((double) nx ) / log ((double) 2.0 ) );
    zlev = (int) ( log((double) nz ) / log ((double) 2.0 ) );

    /* Check to see if nx, nz are powers of 2 
     */

    if ( ( nx >> xlev ) << xlev == nx ) {
	/* do nothing */
    } else {
	nx += 1;
    }

    if ( ( nz >> zlev ) << zlev == nz ) {
	/* do nothing */
    } else {
	nz += 1;
    }

    return MAX( xlev, zlev );
}

static void Vec3fo_float_array( float dest[3], pp::Vec3d src )
{
    dest[0] = src.x;
    dest[1] = src.y;
    dest[2] = src.z;
}


void init_course_quadtree( float *elevation, int nx, int nz, 
			   double scalex, double scalez,
			   pp::Vec3d view_pos, double detail )
{
    HeightMapInfo hm;
    int i;

    hm.Data = elevation;
    hm.XOrigin = 0;
    hm.ZOrigin = 0;
    hm.XSize = nx;
    hm.ZSize = nz;
    hm.RowWidth = hm.XSize;
    hm.Scale = 0;

    root_corner_data.Square = (quadsquare*)NULL;
    root_corner_data.ChildIndex = 0;
    root_corner_data.Level = get_root_level( nx, nz );
    root_corner_data.xorg = 0;
    root_corner_data.zorg = 0;

    for (i=0; i<4; i++) {
	root_corner_data.Verts[i] = 0;
	root_corner_data.Verts[i] = 0;
    }

    root = new quadsquare( &root_corner_data );

    root->AddHeightMap( root_corner_data, hm );
    root->SetScale( scalex, scalez );
    root->SetTerrain( get_course_terrain_data() );

    // Debug info.
    //print_debug( DEBUG_QUADTREE, "nodes = %d\n", root->CountNodes());
    print_debug( DEBUG_QUADTREE, "max error = %g\n", 
		 root->RecomputeError(root_corner_data));

    // Get rid of unnecessary nodes in flat-ish areas.
    print_debug( DEBUG_QUADTREE, 
		 "Culling unnecessary nodes (detail factor = %d)...\n",
		 CULL_DETAIL_FACTOR);
    root->StaticCullData(root_corner_data, CULL_DETAIL_FACTOR);

    // Post-cull debug info.
    print_debug( DEBUG_QUADTREE, "nodes = %d\n", root->CountNodes());
    print_debug( DEBUG_QUADTREE, "max error = %g\n", 
		 root->RecomputeError(root_corner_data));


    // Run the update function a few times before we start rendering
    // to disable unnecessary quadsquares, so the first frame won't
    // be overloaded with tons of triangles.

    float ViewerLoc[3];
    Vec3fo_float_array( ViewerLoc, view_pos );

    for (i = 0; i < 10; i++) {
	root->Update(root_corner_data, (const float*) ViewerLoc, 
		     detail);
    }
}


void update_course_quadtree( const pp::Vec3d view_pos, const float detail )
{
    float ViewerLoc[3];

    Vec3fo_float_array( ViewerLoc, view_pos );

    root->Update( root_corner_data, ViewerLoc, detail );
}

void render_course_quadtree()
{
    GLubyte *vnc_array;

    get_gl_arrays( &vnc_array );

    root->Render( root_corner_data, vnc_array );
}
