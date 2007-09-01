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

#include "gl_util.h"
#include "render_util.h"
#include "textures.h"
#include "game_config.h"
#include "player.h"

#include "stuff.h"

#include "ppgltk/ui_theme.h"

/*
 * Constants 
 */

/* Distance by which to push back far clip plane, to ensure that the
   fogging plane is drawn (m) */
#define FAR_CLIP_FUDGE_AMOUNT 5

static const pp::Color textColor(0.0, 0.0, 0.0);

void reshape( int w, int h )
{
    double far_clip_dist;

    setparam_x_resolution( w );
    setparam_y_resolution( h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    far_clip_dist = getparam_forward_clip_distance() + FAR_CLIP_FUDGE_AMOUNT;

    gluPerspective( getparam_fov(), (double)w/h, NEAR_CLIP_DIST, 
		    far_clip_dist );

    glMatrixMode( GL_MODELVIEW );
} 

void flat_mode()
{
    set_gl_options( TEXT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -0.5, 639.5, -0.5, 479.5, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void draw_overlay() {
    glColor4f( 0.0, 0.0, 1.0, 0.1 );
    glRecti( 0, 0, 640, 480 );
} 

void clear_rendering_context()
{
    glDepthMask( GL_TRUE );
    glClearColor( theme.background.r,
		  theme.background.g,
		  theme.background.b,
		  theme.background.a );
    glClearStencil( 0 );
    glClear( GL_COLOR_BUFFER_BIT 
	     | GL_DEPTH_BUFFER_BIT 
	     | GL_STENCIL_BUFFER_BIT );
}

/* 
 * Sets the material properties
 */
void set_material( const pp::Color diffuse, const pp::Color specular,
			 const double specular_exp )
{
  GLfloat mat_amb_diff[4];
  GLfloat mat_specular[4];

  /* Set material color (used when lighting is on) */
  mat_amb_diff[0] = diffuse.r;
  mat_amb_diff[1] = diffuse.g;
  mat_amb_diff[2] = diffuse.b;
  mat_amb_diff[3] = diffuse.a; 
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff );

  mat_specular[0] = specular.r;
  mat_specular[1] = specular.g;
  mat_specular[2] = specular.b;
  mat_specular[3] = specular.a;
  glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );

  glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, specular_exp );

  /* Set standard color */
  glColor4f( diffuse.r, diffuse.g, diffuse.b, diffuse.a);
} 


void draw_billboard( Player& plyr, 
		     pp::Vec3d center_pt, double width, double height, 
		     bool use_world_y_axis, 
		     pp::Vec2d min_tex_coord, pp::Vec2d max_tex_coord )
{
    pp::Vec3d pt;
    pp::Vec3d x_vec;
    pp::Vec3d y_vec;
    pp::Vec3d z_vec;

    x_vec.x = plyr.view.inv_view_mat.data[0][0];
    x_vec.y = plyr.view.inv_view_mat.data[0][1];
    x_vec.z = plyr.view.inv_view_mat.data[0][2];

    if ( use_world_y_axis ) {
	y_vec = pp::Vec3d( 0, 1, 0 );
	x_vec = projectIntoPlane( y_vec, x_vec );
	x_vec.normalize();
	z_vec = x_vec^y_vec;
    } else {
	y_vec.x = plyr.view.inv_view_mat.data[1][0];
	y_vec.y = plyr.view.inv_view_mat.data[1][1];
	y_vec.z = plyr.view.inv_view_mat.data[1][2];
	z_vec.x = plyr.view.inv_view_mat.data[2][0];
	z_vec.y = plyr.view.inv_view_mat.data[2][1];
	z_vec.z = plyr.view.inv_view_mat.data[2][2];
    }

    glBegin( GL_QUADS );
    {
	pt = center_pt+((-width/2.0)*x_vec);
	pt = pt+((-height/2.0)*y_vec);
	glNormal3f( z_vec.x, z_vec.y, z_vec.z );
	glTexCoord2f( min_tex_coord.x, min_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = pt + (width*x_vec);
	glTexCoord2f( max_tex_coord.x, min_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = pt - (height*y_vec);
	glTexCoord2f( max_tex_coord.x, max_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );

	pt = pt - (-width *x_vec);
	glTexCoord2f( min_tex_coord.x, max_tex_coord.y );
	glVertex3f( pt.x, pt.y, pt.z );
    }
    glEnd();
}
