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

#include "ui_snow.h"
#include "gl_util.h"
#include "course_render.h"
#include "render_util.h"
#include "game_config.h"
#include "textures.h"
#include "ui_mgr.h"
#include "loop.h"

#include "stuff.h"

#include "ppgltk/alg/defs.h"

#define MAX_NUM_PARTICLES 10000
#define BASE_NUM_PARTICLES 400
#define GRAVITY_FACTOR 0.015
#define BASE_VELOCITY 0.05
#define VELOCITY_RANGE 0.02
#define PUSH_DECAY_TIME_CONSTANT 0.2
#define PUSH_DIST_DECAY 100 
#define PUSH_FACTOR 0.5
#define MAX_PUSH_FORCE 5
#define WIND_FORCE 0.03
#define AIR_DRAG 0.4

#define PARTICLE_MIN_SIZE 1
#define PARTICLE_SIZE_RANGE 10

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))


typedef struct _particle_t {
    pp::Vec2d pt;
    double size;
    pp::Vec2d vel;
    pp::Vec2d tex_min;
    pp::Vec2d tex_max;
} particle_t;

static particle_t particles[MAX_NUM_PARTICLES];
static int num_particles = BASE_NUM_PARTICLES;
static GLfloat particleColor[4] = { 1, 1, 1, 0.4 };
static pp::Vec2d push_position(0,0);
static pp::Vec2d last_push_position;
static double last_update_time = -1;
static bool push_position_initialized = false;

static double frand()
{
    return (double)rand()/RAND_MAX;
} 

static void make_particle( int i, double x, double y )
{
    double p_dist;
    int type;

    particles[i].pt.x = x;
    particles[i].pt.y = y;
    p_dist = frand();
    particles[i].size = PARTICLE_MIN_SIZE + (1.0 - p_dist)*PARTICLE_SIZE_RANGE;
    particles[i].vel.x = 0;
    particles[i].vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
    type = (int) (frand() * (4.0 - EPS));
    if (type == 0) {
	particles[i].tex_min = pp::Vec2d( 0.0, 0.0 );
	particles[i].tex_max = pp::Vec2d( 0.5, 0.5 );
    } else if (type == 1) {
	particles[i].tex_min = pp::Vec2d( 0.5, 0.0 );
	particles[i].tex_max = pp::Vec2d( 1.0, 0.5 );
    } else if (type == 2) {
	particles[i].tex_min = pp::Vec2d( 0.5, 0.5 );
	particles[i].tex_max = pp::Vec2d( 1.0, 1.0 );
    } else {
	particles[i].tex_min = pp::Vec2d( 0.0, 0.5 );
	particles[i].tex_max = pp::Vec2d( 0.5, 1.0 );
    }
}

void init_ui_snow( void )
{
    int i;

    for( i=0; i<num_particles; i++) {
	make_particle( i, frand(), frand() );
    }
    push_position = pp::Vec2d( 0.0, 0.0 );
}

void update_ui_snow( double time_step, bool windy )
{
    pp::Vec2d *v, f;
    pp::Vec2d *pt;
    double size;
    double dist_from_push, p_dist;
    pp::Vec2d push_vector;
    int i;
    double push_timestep, time;

    time = getClockTime();

    push_vector.x = 0;
    push_vector.y = 0;
    push_timestep = 0;
	
    if ( push_position_initialized ) {
	push_vector.x = push_position.x - last_push_position.x;
	push_vector.y = push_position.y - last_push_position.y;
	push_timestep = time - last_update_time;
    }
    last_push_position = push_position;
    last_update_time = time;

    for ( i=0; i<num_particles; i++) {
	pt = &particles[i].pt;
	v = &particles[i].vel;
	size = particles[i].size;

	f.x = 0;
	f.y = 0;

	/* Mouse push and gravity */
	dist_from_push = (pow((pt->x - push_position.x), 2) +
			  pow((pt->y - push_position.y), 2));
	if ( push_timestep > 0 ) {
	    f.x = PUSH_FACTOR * push_vector.x / push_timestep; 
	    
	    f.y = PUSH_FACTOR * push_vector.y / push_timestep; 

	    f.x = min(MAX_PUSH_FORCE,f.x);
	    f.x = max(-MAX_PUSH_FORCE,f.x);
	    f.y = min(MAX_PUSH_FORCE,f.y);
	    f.y = max(-MAX_PUSH_FORCE,f.y);

	    f.x *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) * 
		size/PARTICLE_SIZE_RANGE;
	    f.y *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) *
		size/PARTICLE_SIZE_RANGE;
	}

	/* Update velocity */
	v->x += ( f.x + ( windy ? WIND_FORCE : 0.0 ) - v->x * AIR_DRAG ) * 
	    time_step;
	v->y += ( f.y - GRAVITY_FACTOR - v->y * AIR_DRAG ) * 
	    time_step;

	/* Update position */
        pt->x += v->x * time_step * ( size / PARTICLE_SIZE_RANGE ); 
        pt->y += v->y * time_step * ( size / PARTICLE_SIZE_RANGE );

	if ( pt->x < 0 ) {
	    pt->x = 1;
	} else if ( pt->x > 1 ) {
	    pt->x = 0.0;
	}
    }

    /* Kill off & regenerate particles */
    for (i=0; i<num_particles; i++) {
	particle_t *p = &particles[i];

	if (p->pt.y < -0.05) {
	    /* If we have an excess of particles, kill off with
	       50% probability */
	    if ( num_particles > BASE_NUM_PARTICLES && frand() > 0.5 ) {
		/* Delete the particle */
		*p = particles[num_particles-1];
		num_particles -= 1;
	    } else {
		p->pt.x = frand();
		p->pt.y = 1+frand()*BASE_VELOCITY;
		p_dist = frand();
		p->size = PARTICLE_MIN_SIZE + 
		    ( 1.0 - p_dist ) * PARTICLE_SIZE_RANGE;
		p->vel.x = 0;
		p->vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
	    }
	}
    }

    if ( time_step < PUSH_DECAY_TIME_CONSTANT ) {
	push_vector.x *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
	push_vector.y *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
    } else {
	push_vector.x = 0.0;
	push_vector.y = 0.0;
    }
} 

void draw_ui_snow( void )
{
    GLuint   texture_id;
    char *binding;
    pp::Vec2d *pt, *tex_min, *tex_max;
    double size;
    double xres, yres;
    int i;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    
    UIMgr.setupDisplay();

    binding = "ui_snow_particle";
    if (!get_texture_binding( "ui_snow_particle", &texture_id ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", 
		       binding );
	texture_id = 0;
    } 

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glBindTexture( GL_TEXTURE_2D, texture_id );

    glColor4f( particleColor[0], 
	       particleColor[1], 
	       particleColor[2],
	       particleColor[3] );

    glPushMatrix();
    {
	for ( i=0; i<num_particles; i++) {
	    pt = &particles[i].pt;
	    size = particles[i].size;
	    tex_min = &particles[i].tex_min;
	    tex_max = &particles[i].tex_max;
	    glPushMatrix();
	    {
		glTranslatef( pt->x*xres, pt->y*yres, 0 );
		glBegin( GL_QUADS );
		{
		    glTexCoord2f( tex_min->x, tex_min->y );
		    glVertex2f( 0, 0 );
		    glTexCoord2f( tex_max->x, tex_min->y );
		    glVertex2f( size, 0 );
		    glTexCoord2f( tex_max->x, tex_max->y );
		    glVertex2f( size, size );
		    glTexCoord2f( tex_min->x, tex_max->y );
		    glVertex2f( 0, size );
		}
		glEnd();
	    }
	    glPopMatrix();
	} 
    }
    glPopMatrix();

} 

void
reset_ui_snow_cursor_pos( pp::Vec2d pos ) 
{
    double xres, yres;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    push_position = pp::Vec2d( pos.x/(double)xres,
				  pos.y/(double)yres );
    last_push_position = push_position;
    push_position_initialized = true;
}

void 
push_ui_snow( pp::Vec2d pos )
{
    double xres, yres;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    push_position = pp::Vec2d( pos.x/(double)xres,
				  pos.y/(double)yres );
    if ( !push_position_initialized ) {
	last_push_position = push_position;
    }
    push_position_initialized = true;
}

void
make_ui_snow( pp::Vec2d pos ) {
    double xres, yres;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();

    if ( num_particles < MAX_NUM_PARTICLES ) {
	make_particle( num_particles, pos.x/xres, pos.y/yres );
	num_particles++;
    }
}
