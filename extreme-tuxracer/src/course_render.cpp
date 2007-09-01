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
#include "textures.h"
#include "phys_sim.h"
#include "hier_util.h"
#include "gl_util.h"
#include "render_util.h"
#include "fog.h"
#include "course_quad.h"
#include "viewfrustum.h"
#include "track_marks.h"

#include "game_config.h"

#include "ppgltk/alg/defs.h"


/* 
 *  Constants 
 */
 

/* How long to make the flat part at the bottom of the course, as a
   fraction of the total length of the course */
#define FLAT_SEGMENT_FRACTION 0.2

/* Aspect ratio of background texture */
#define BACKGROUND_TEXTURE_ASPECT 3.0


/*
 * Statics 
 */

/* The course normal vectors */
static pp::Vec3d *nmls = NULL;

/* Should we activate clipping when drawing the course? */
static bool clip_course = false;

/* If clipping is active, it will be based on a camera located here */
static pp::Vec3d eye_pt;


/* Macros for converting indices in height map to world coordinates */
#define XCD(x) (  (double)(x) / (nx-1.) * courseWidth )
#define ZCD(y) ( -(double)(y) / (ny-1.) * courseLength )

#define NORMAL(x, y) ( nmls[ (x) + nx * (y) ] )


/*
 * Function definitions
 */

void set_course_clipping( bool state ) { clip_course = state; }
void set_course_eye_point( pp::Vec3d pt ) { eye_pt = pt; }

pp::Vec3d* get_course_normals() { return nmls; } 

void calc_normals()
{
    float *elevation;
    float courseWidth, courseLength;
    int nx, ny;
    int x,y;
    pp::Vec3d p0, p1, p2;
    pp::Vec3d n, nml, v1, v2;

    elevation = get_course_elev_data();
    get_course_dimensions( &courseWidth, &courseLength );
    get_course_divisions( &nx, &ny );

    if ( nmls != NULL ) {
        free( nmls );
    } 

    nmls = (pp::Vec3d *)malloc( sizeof(pp::Vec3d)*nx*ny ); 
    if ( nmls == NULL ) {
	handle_system_error( 1, "malloc failed" );
    }

    for ( y=0; y<ny; y++) {
        for ( x=0; x<nx; x++) {
            nml = pp::Vec3d( 0., 0., 0. );

            p0 = pp::Vec3d( XCD(x), ELEV(x,y), ZCD(y) );

	    /* The terrain is meshed as follows:
	             ...
	          +-+-+-+-+            x<---+
	          |\|/|\|/|                 |
	       ...+-+-+-+-+...              V
	          |/|\|/|\|                 y
	          +-+-+-+-+
		     ...

	       So there are two types of vertices: those surrounded by
	       four triangles (x+y is odd), and those surrounded by
	       eight (x+y is even).
	    */

#define POINT(x,y) pp::Vec3d( XCD(x), ELEV(x,y), ZCD(y) )

	    if ( (x + y) % 2 == 0 ) {
		if ( x > 0 && y > 0 ) {
		    p1 = POINT(x,  y-1);
		    p2 = POINT(x-1,y-1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;

		    p1 = POINT(x-1,y-1);
		    p2 = POINT(x-1,y  );
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x > 0 && y < ny-1 ) {
		    p1 = POINT(x-1,y  );
		    p2 = POINT(x-1,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;

		    p1 = POINT(x-1,y+1);
		    p2 = POINT(x  ,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x < nx-1 && y > 0 ) {
		    p1 = POINT(x+1,y  );
		    p2 = POINT(x+1,y-1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;

		    p1 = POINT(x+1,y-1);
		    p2 = POINT(x  ,y-1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x < nx-1 && y < ny-1 ) {
		    p1 = POINT(x+1,y  );
		    p2 = POINT(x+1,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v1^v2;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;

		    p1 = POINT(x+1,y+1);
		    p2 = POINT(x  ,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v1^v2;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;

		} 
	    } else {
		/* x + y is odd */
		if ( x > 0 && y > 0 ) {
		    p1 = POINT(x,  y-1);
		    p2 = POINT(x-1,y  );
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x > 0 && y < ny-1 ) {
		    p1 = POINT(x-1,y  );
		    p2 = POINT(x  ,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x < nx-1 && y > 0 ) {
		    p1 = POINT(x+1,y  );
		    p2 = POINT(x  ,y-1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v2^v1;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
		if ( x < nx-1 && y < ny-1 ) {
		    p1 = POINT(x+1,y  );
		    p2 = POINT(x  ,y+1);
		    v1 = p1-p0;
		    v2 = p2-p0;
		    n = v1^v2;

		    check_assertion( n.y > 0, "course normal points down" );

		    n.normalize();
		    nml = nml+n;
		} 
	    }

            nml.normalize();
            NORMAL(x,y) = nml;
            continue;
        } 
#undef POINT
    } 
} 

void setup_course_tex_gen()
{
    static GLfloat xplane[4] = { 1.0 / TEX_SCALE, 0.0, 0.0, 0.0 };
    static GLfloat zplane[4] = { 0.0, 0.0, 1.0 / TEX_SCALE, 0.0 };
    glTexGenfv( GL_S, GL_OBJECT_PLANE, xplane );
    glTexGenfv( GL_T, GL_OBJECT_PLANE, zplane );
}

#define DRAW_POINT \
    glNormal3f( nml.x, nml.y, nml.z ); \
    glVertex3f( pt.x, pt.y, pt.z ); 


void render_course()
{
    int nx, ny;

    get_course_divisions(&nx, &ny);
    set_gl_options( COURSE );

    setup_course_tex_gen();

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    set_material( pp::Color::white, pp::Color::black, 1.0 );
    
    update_course_quadtree( eye_pt, getparam_course_detail_level() );


    render_course_quadtree( );
	
    draw_track_marks();
}

void draw_sky(pp::Vec3d pos)
{
  GLuint texture_id[6];

  set_gl_options( SKY );

  if (!(get_texture_binding( "sky_front", &texture_id[0] ) && 
        get_texture_binding( "sky_top", &texture_id[1] ) && 
        get_texture_binding( "sky_bottom", &texture_id[2] ) && 
        get_texture_binding( "sky_left", &texture_id[3] ) && 
        get_texture_binding( "sky_right", &texture_id[4] ) && 
        get_texture_binding( "sky_back", &texture_id[5] ) ) ) {
    return;
  } 

  glColor4f( 1.0, 1.0, 1.0, 1.0 );

  glPushMatrix();

  glTranslatef(pos.x, pos.y, pos.z);

  glBindTexture( GL_TEXTURE_2D, texture_id[0] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f( -1, -1, -1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f(  1, -1, -1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f(  1,  1, -1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f( -1,  1, -1);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, texture_id[1] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f( -1,  1, -1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f(  1,  1, -1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f(  1,  1,  1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f( -1,  1,  1);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, texture_id[2] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f( -1, -1,  1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f(  1, -1,  1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f(  1, -1, -1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f( -1, -1, -1);
  glEnd();


  glBindTexture( GL_TEXTURE_2D, texture_id[3] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f( -1, -1,  1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f( -1, -1, -1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f( -1,  1, -1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f( -1,  1,  1);
  glEnd();


  glBindTexture( GL_TEXTURE_2D, texture_id[4] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f(  1, -1, -1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f(  1, -1,  1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f(  1,  1,  1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f(  1,  1, -1);
  glEnd();


  glBindTexture( GL_TEXTURE_2D, texture_id[5] );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 );
  glVertex3f(  1, -1,  1);
  glTexCoord2f( 1.0, 0.0 );
  glVertex3f( -1, -1,  1);
  glTexCoord2f( 1.0, 1.0 );
  glVertex3f( -1,  1,  1);
  glTexCoord2f( 0.0, 1.0 );
  glVertex3f(  1,  1,  1);
  glEnd();


  glPopMatrix();

}

void draw_trees() 
{
    Tree    *treeLocs;
    int       numTrees;
    //double  treeRadius;
    //double  treeHeight;
    int       i;
    GLuint    texture_id;
    pp::Vec3d  normal;
    double  fwd_clip_limit, bwd_clip_limit, fwd_tree_detail_limit;

    int tree_type = -1;
    const char *tree_name = 0;

    Item    *itemLocs;
    int       numItems;
    double  itemRadius;
    double  itemHeight;
    int       item_type = -1;
    const char *    item_name = 0;
    item_type_t *item_types;

    treeLocs = get_tree_locs();
    numTrees = get_num_trees();
    item_types = get_item_types();

    fwd_clip_limit = getparam_forward_clip_distance();
    bwd_clip_limit = getparam_backward_clip_distance();
    fwd_tree_detail_limit = getparam_tree_detail_distance();

    set_gl_options( TREES );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    set_material( pp::Color::white, pp::Color::black, 1.0 );
    
    for (i = 0; i< numTrees; i++ ) {

		if ( clip_course ) {
	   		if ( eye_pt.z - treeLocs[i].ray.pt.z > fwd_clip_limit ) 
			continue;
	    
	    	if ( treeLocs[i].ray.pt.z - eye_pt.z > bwd_clip_limit )
			continue;
		}

		// verify that the correct texture is bound
		if (treeLocs[i].type != tree_type) {
		    tree_type = treeLocs[i].type;
		    tree_name = get_tree_name(tree_type);
		}

        glPushMatrix();
        
		glTranslatef( treeLocs[i].ray.pt.x,
				treeLocs[i].ray.pt.y, 
        		treeLocs[i].ray.pt.z );

		normal = eye_pt - treeLocs[i].ray.pt;
		normal.normalize();

		glNormal3f( normal.x, normal.y, normal.z );
		//glCallList( treeLocs[i].getDisplayList() );	
		
		treeLocs[i].getModel()->draw();
		
        glPopMatrix();
    } 

    itemLocs = get_item_locs();
    numItems = get_num_items();

    for (i = 0; i< numItems; i++ ) {

	if ( !itemLocs[i].isDrawable() ) {
		continue;
	}

	if ( clip_course ) {
	    if ( eye_pt.z - itemLocs[i].ray.pt.z > fwd_clip_limit ) 
		continue;
	    
	    if ( itemLocs[i].ray.pt.z - eye_pt.z > bwd_clip_limit )
		continue;
	}

	/* verify that the correct texture is bound */
	if (itemLocs[i].type != item_type) {
	    item_type = itemLocs[i].type;
	    item_name = get_item_name(item_type);
	    if (!get_texture_binding( item_name, &texture_id ) ) {
		texture_id = 0;
	    }
	    glBindTexture( GL_TEXTURE_2D, texture_id );
	}

        glPushMatrix();
	{
	    glTranslatef( itemLocs[i].ray.pt.x, itemLocs[i].ray.pt.y, 
			  itemLocs[i].ray.pt.z );

	    itemRadius = itemLocs[i].diam/2.;
	    itemHeight = itemLocs[i].height;

	    if ( item_types[item_type].use_normal ) {
		normal = item_types[item_type].normal;
	    } else {
		normal = eye_pt - itemLocs[i].ray.pt;
		normal.normalize();
	    }

	    if (normal.y == 1.0) {
		continue;
	    }

	    glNormal3f( normal.x, normal.y, normal.z );

	    normal.y = 0.0;
	    normal.normalize();

	    glBegin( GL_QUADS );
	    {
		glTexCoord2f( 0., 0. );
		glVertex3f( -itemRadius*normal.z, 
			    0.0, 
			    itemRadius*normal.x );
		glTexCoord2f( 1., 0. );
		glVertex3f( itemRadius*normal.z, 
			    0.0, 
			    -itemRadius*normal.x );
		glTexCoord2f( 1., 1. );
		glVertex3f( itemRadius*normal.z, 
			    itemHeight, 
			    -itemRadius*normal.x );
		glTexCoord2f( 0., 1. );
		glVertex3f( -itemRadius*normal.z, 
			    itemHeight, 
			    itemRadius*normal.x );
	    }
	    glEnd();
	}
        glPopMatrix();
    } 

} 

/*! 
  Draws a fog plane at the far clipping plane to mask out clipping of terrain.

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-31
  \date    Modified: 2000-08-31
*/

void draw_fog_plane()
{
    pp::Plane left_edge_plane, right_edge_plane;
    pp::Plane left_clip_plane, right_clip_plane;
    pp::Plane far_clip_plane;
    pp::Plane bottom_clip_plane;
    pp::Plane bottom_plane, top_plane;

    float course_width, course_length;
    double course_angle, slope;

    pp::Vec3d left_pt, right_pt, pt;
    pp::Vec3d top_left_pt, top_right_pt;
    pp::Vec3d bottom_left_pt, bottom_right_pt;
    pp::Vec3d left_vec, right_vec;
    double height;

    GLfloat *fogColor;

    if ( fogPlane.isEnabled() == false ) {
	return;
    }

    //set_gl_options( FOG_PLANE );

    get_course_dimensions( &course_width, &course_length );
    course_angle = get_course_angle();
    slope = tan( ANGLES_TO_RADIANS( course_angle ) );

    left_edge_plane = pp::Plane( 1.0, 0.0, 0.0, 0.0 );

    right_edge_plane = pp::Plane( -1.0, 0.0, 0.0, course_width );

    far_clip_plane = get_far_clip_plane();
    left_clip_plane = get_left_clip_plane();
    right_clip_plane = get_right_clip_plane();
    bottom_clip_plane = get_bottom_clip_plane();


    // Find the bottom plane 
    bottom_plane.nml = pp::Vec3d( 0.0, 1, -slope );
    height = get_terrain_base_height( 0 );

    bottom_plane.d = -height * bottom_plane.nml.y;

    // Find the top plane 
    top_plane.nml = bottom_plane.nml;
    height = get_terrain_max_height( 0 );
    top_plane.d = -height * top_plane.nml.y;

    // Now find the bottom left and right points of the fog plane 
    if ( !pp::Plane::intersect( bottom_plane, far_clip_plane, left_clip_plane,
			    &left_pt ) )
    {
	return;
    }

    if ( !pp::Plane::intersect( bottom_plane, far_clip_plane, right_clip_plane,
			    &right_pt ) )
    {
	return;
    }

    if ( !pp::Plane::intersect( top_plane, far_clip_plane, left_clip_plane,
			    &top_left_pt ) )
    {
	return;
    }

    if ( !pp::Plane::intersect( top_plane, far_clip_plane, right_clip_plane,
			    &top_right_pt ) )
    {
	return;
    }

    if ( !pp::Plane::intersect( bottom_clip_plane, far_clip_plane, 
			    left_clip_plane, &bottom_left_pt ) )
    {
	return;
    }

    if ( !pp::Plane::intersect( bottom_clip_plane, far_clip_plane, 
			    right_clip_plane, &bottom_right_pt ) )
    {
	return;
    }

    left_vec = top_left_pt - left_pt;
    right_vec = top_right_pt - right_pt;


    // Now draw the fog plane 

    set_gl_options( FOG_PLANE );

    fogColor = fogPlane.getColor();

    glColor4fv( fogColor );

    glBegin( GL_QUAD_STRIP );

    glVertex3f( bottom_left_pt.x, bottom_left_pt.y, bottom_left_pt.z );
    glVertex3f( bottom_right_pt.x, bottom_right_pt.y, bottom_right_pt.z );
    glVertex3f( left_pt.x, left_pt.y, left_pt.z );
    glVertex3f( right_pt.x, right_pt.y, right_pt.z );

    glColor4f( fogColor[0], fogColor[1], fogColor[2], 0.9 );
    glVertex3f( top_left_pt.x, top_left_pt.y, top_left_pt.z );
    glVertex3f( top_right_pt.x, top_right_pt.y, top_right_pt.z );

    glColor4f( fogColor[0], fogColor[1], fogColor[2], 0.3 );
    pt = top_left_pt + left_vec ;
    glVertex3f( pt.x, pt.y, pt.z );
    pt = top_right_pt + right_vec;
    glVertex3f( pt.x, pt.y, pt.z );
		
    glColor4f( fogColor[0], fogColor[1], fogColor[2], 0.0 );
    pt = top_left_pt + left_vec*3.0;
    glVertex3f( pt.x, pt.y, pt.z );
    pt = top_right_pt + right_vec*3.0;
    glVertex3f( pt.x, pt.y, pt.z );

    glEnd();
}
