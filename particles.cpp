/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#include "particles.h"
#include "textures.h"
#include "ogl.h"
#include "course.h"
#include "view.h"
#include "env.h"

// ====================================================================
//					gui particles 2D
// ====================================================================

#define MAX_num_snowparticles 10000
#define BASE_num_snowparticles 800 
#define GRAVITY_FACTOR 0.015    
#define BASE_VELOCITY 0.05
#define VELOCITY_RANGE 0.02
#define PUSH_DECAY_TIME_CONSTANT 0.2
#define PUSH_DIST_DECAY 100 
#define PUSH_FACTOR 0.5
#define MAX_PUSH_FORCE 5
#define AIR_DRAG 0.4

#define PARTICLE_MIN_SIZE 1 
#define PARTICLE_SIZE_RANGE 10 

typedef struct {
    TVector2 pt;
    double size;
    TVector2 vel;
    TVector2 tex_min;
    TVector2 tex_max;
} TGuiParticle;

static TGuiParticle particles[MAX_num_snowparticles];
static int num_snowparticles = BASE_num_snowparticles;
static GLfloat part_col[4] = {1, 1, 1, 0.5 };
static TVector2 push_position = {0, 0};
static TVector2 last_push_position;
static double last_update_time = -1;
static bool push_position_initialized = false;

static double frand () {return (double)rand() / RAND_MAX; }

static void make_particle (int i, double x, double y) {
    double p_dist;
    int type;
    particles[i].pt.x = x;
    particles[i].pt.y = y;
    p_dist = frand();

	particles[i].size = PARTICLE_MIN_SIZE + (1.0 - p_dist) * PARTICLE_SIZE_RANGE;
    particles[i].vel.x = 0;
    particles[i].vel.y = -BASE_VELOCITY - p_dist * VELOCITY_RANGE;
    
	type = (int) (frand() * (4.0 - EPS));
	if (type == 0) {
		particles[i].tex_min = MakeVector2  (0.0, 0.0);
		particles[i].tex_max = MakeVector2  (0.5, 0.5);
    } else if (type == 1) {
		particles[i].tex_min = MakeVector2  (0.5, 0.0);
		particles[i].tex_max = MakeVector2  (1.0, 0.5);
    } else if (type == 2) {
		particles[i].tex_min = MakeVector2  (0.5, 0.5);
		particles[i].tex_max = MakeVector2  (1.0, 1.0);
    } else {
		particles[i].tex_min = MakeVector2  (0.0, 0.5);
		particles[i].tex_max = MakeVector2  (0.5, 1.0);
    }
}

void init_ui_snow (void) {
    int i;
	for (i=0; i<num_snowparticles; i++) make_particle (i, frand(), frand());
    push_position = MakeVector2 (0.0, 0.0);
}

void update_ui_snow (double time_step) {
    TVector2 *v, f;
    TVector2 *pt;
    double size;
    double dist_from_push, p_dist;
    TVector2 push_vector;
    int i;
    double push_timestep, time;

    time = Winsys.ClockTime ();

    push_vector.x = 0;
    push_vector.y = 0;
    push_timestep = 0;
	
    if  (push_position_initialized) {
		push_vector.x = push_position.x - last_push_position.x;
		push_vector.y = push_position.y - last_push_position.y;
		push_timestep = time - last_update_time;
    }
    last_push_position = push_position;
    last_update_time = time;

    for  (i=0; i<num_snowparticles; i++) {
		pt = &particles[i].pt;
		v = &particles[i].vel;
		size = particles[i].size;

		f.x = 0;
		f.y = 0;

		dist_from_push = (pow((pt->x - push_position.x), 2) +
			  pow((pt->y - push_position.y), 2));
		if  (push_timestep > 0) {
	    	f.x = PUSH_FACTOR * push_vector.x / push_timestep; 
		    f.y = PUSH_FACTOR * push_vector.y / push_timestep; 
		    f.x = MIN (MAX_PUSH_FORCE, f.x);
		    f.x = MAX (-MAX_PUSH_FORCE, f.x);
	    	f.y = MIN (MAX_PUSH_FORCE, f.y);
		    f.y = MAX (-MAX_PUSH_FORCE, f.y);
		    f.x *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) * 
				size/PARTICLE_SIZE_RANGE;
		    f.y *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) *
				size/PARTICLE_SIZE_RANGE;
		}

		v->x +=  (f.x - v->x * AIR_DRAG) *  time_step;
		v->y +=  (f.y - GRAVITY_FACTOR - v->y * AIR_DRAG) * time_step;

		pt->x += v->x * time_step *  (size / PARTICLE_SIZE_RANGE); 
		pt->y += v->y * time_step *  (size / PARTICLE_SIZE_RANGE);

		if  (pt->x < 0) {
			pt->x = 1;
		} else if (pt->x > 1) {
			pt->x = 0.0;
		}
    }

    for (i=0; i<num_snowparticles; i++) {
		TGuiParticle *p = &particles[i];
		if (p->pt.y < -0.05) {
			if  (num_snowparticles > BASE_num_snowparticles && frand() > 0.5) {
				*p = particles[num_snowparticles-1];
				num_snowparticles -= 1;
			} else {
				p->pt.x = frand();
				p->pt.y = 1+frand()*BASE_VELOCITY;
				p_dist = frand();
				p->size = PARTICLE_MIN_SIZE + (1.0 - p_dist) * PARTICLE_SIZE_RANGE;
				p->vel.x = 0;
				p->vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
			}
		}
    }

    if  (time_step < PUSH_DECAY_TIME_CONSTANT) {
		push_vector.x *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
		push_vector.y *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
    } else {
		push_vector.x = 0.0;
		push_vector.y = 0.0;
    }
} 

void draw_ui_snow (void) {
    TVector2 *pt, *tex_min, *tex_max;
    double size;
    double xres, yres;
    int i;
	
    xres = param.x_resolution;
    yres = param.y_resolution;
	
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	Tex.BindTex (SNOW_PART);
    glColor4f(part_col[0], part_col[1], part_col[2], part_col[3]);
	part_col[3] = 0.3;  
    glPushMatrix();
	for  (i=0; i<num_snowparticles; i++) {
	    pt = &particles[i].pt;
	    size = particles[i].size;
	    tex_min = &particles[i].tex_min;
	    tex_max = &particles[i].tex_max;
	    glPushMatrix();
		glTranslatef (pt->x * xres, pt->y * yres, 0);
		glBegin (GL_QUADS);
		    glTexCoord2f (tex_min->x, tex_min->y);
		    glVertex2f (0, 0);
		    glTexCoord2f (tex_max->x, tex_min->y);
		    glVertex2f (size, 0);
		    glTexCoord2f (tex_max->x, tex_max->y);
		    glVertex2f (size, size);
		    glTexCoord2f (tex_min->x, tex_max->y);
		    glVertex2f (0, size);
		glEnd();
	    glPopMatrix();
    }
    glPopMatrix();
} 

void reset_ui_snow_cursor_pos (TVector2 pos) {
    double xres, yres;

    xres = param.x_resolution;
    yres = param.y_resolution;
    push_position = MakeVector2 (pos.x/(double)xres, pos.y/(double)yres);
    last_push_position = push_position;
    push_position_initialized = true;
}

void push_ui_snow (TVector2 pos) {
    double xres, yres;

    xres = param.x_resolution;
    yres = param.y_resolution;
    push_position = MakeVector2 (pos.x/(double)xres, pos.y/(double)yres);
    if  (!push_position_initialized) last_push_position = push_position;
    push_position_initialized = true;
}

void make_ui_snow (TVector2 pos) {
    double xres, yres;

    xres = param.x_resolution;
    yres = param.y_resolution;

    if  (num_snowparticles < MAX_num_snowparticles) {
		make_particle (num_snowparticles, pos.x/xres, pos.y/yres);
		num_snowparticles++;
    }
}

// ====================================================================
//						tux particles
// ====================================================================

#define MAX_PARTICLES 500000
#define START_RADIUS 0.04
#define OLD_PART_SIZE 0.12	// orig 0.07
#define NEW_PART_SIZE 0.035	// orig 0.02
#define MIN_AGE     -0.2
#define MAX_AGE      1.0
#define VARIANCE_FACTOR 0.8
#define PARTICLE_SHADOW_HEIGHT 0.05
#define PARTICLE_SHADOW_ALPHA 0.1

#define MAX_TURN_PARTICLES 500
#define BRAKE_PARTICLES 2000
#define MAX_ROLL_PARTICLES 3000
#define PARTICLE_SPEED_FACTOR 40
#define MAX_PARTICLE_ANGLE 80
#define MAX_PARTICLE_ANGLE_SPEED 50
#define PARTICLE_SPEED_MULTIPLIER 0.3
#define MAX_PARTICLE_SPEED 2


typedef struct _Particle {
    TVector3 pt;
    short type;
    double base_size;
    double cur_size;
    double terrain_height;
    double age;
    double death;
    double alpha;
    TVector3 vel;
    struct _Particle *next;
} Particle;

static Particle* head = NULL;
static int num_particles = 0;

void draw_billboard (CControl *ctrl, 
		     TVector3 center_pt, double width, double height, 
		     bool use_world_y_axis, 
		     TVector2 min_tex_coord, TVector2 max_tex_coord)
{
    TVector3 pt;
    TVector3 x_vec;
    TVector3 y_vec;
    TVector3 z_vec;

    x_vec.x = ctrl->view_mat[0][0];
    x_vec.y = ctrl->view_mat[0][1];
    x_vec.z = ctrl->view_mat[0][2];

    if  (use_world_y_axis) {
		y_vec = MakeVector (0, 1, 0);
		x_vec = ProjectToPlane (y_vec, x_vec);
		NormVector (&x_vec);
		z_vec = CrossProduct (x_vec, y_vec);
    } else {
		y_vec.x = ctrl->view_mat[1][0];
		y_vec.y = ctrl->view_mat[1][1];
		y_vec.z = ctrl->view_mat[1][2];
		z_vec.x = ctrl->view_mat[2][0];
		z_vec.y = ctrl->view_mat[2][1];
		z_vec.z = ctrl->view_mat[2][2];
    }

    glBegin (GL_QUADS);
		pt = AddVectors (center_pt, ScaleVector (-width/2.0, x_vec));
		pt = AddVectors (pt, ScaleVector (-height/2.0, y_vec));
		glNormal3f (z_vec.x, z_vec.y, z_vec.z);
		glTexCoord2f (min_tex_coord.x, min_tex_coord.y);
		glVertex3f (pt.x, pt.y, pt.z);

		pt = AddVectors (pt, ScaleVector (width, x_vec));
		glTexCoord2f (max_tex_coord.x, min_tex_coord.y);
		glVertex3f (pt.x, pt.y, pt.z);

		pt = AddVectors (pt, ScaleVector (height, y_vec));
		glTexCoord2f (max_tex_coord.x, max_tex_coord.y);
		glVertex3f (pt.x, pt.y, pt.z);

		pt = AddVectors (pt, ScaleVector (-width, x_vec));
		glTexCoord2f (min_tex_coord.x, max_tex_coord.y);
		glVertex3f (pt.x, pt.y, pt.z);
    glEnd ();
}

void create_new_particles (TVector3 loc, TVector3 vel, int num)  {
    Particle *newp;
    int i;
    double speed;

    speed = NormVector (&vel);

    if  (num_particles + num > MAX_PARTICLES) {
		Message ("maximum number of particles exceeded", "");
    } 

    for (i=0; i<num; i++) {
        newp = (Particle*)malloc (sizeof (Particle));
        if  (newp == NULL) Message ("out of memory", "");
        num_particles += 1;
        newp->next = head;
        head = newp;
        newp->pt.x = loc.x + 2.*(frand() - 0.5) * START_RADIUS;
        newp->pt.y = loc.y;
        newp->pt.z = loc.z + 2.*(frand() - 0.5) * START_RADIUS;
		newp->type = (int) (frand() * (4.0 - EPS));
		newp->base_size =  (frand() + 0.5) * OLD_PART_SIZE;
		newp->cur_size = NEW_PART_SIZE;
        newp->age = frand() * MIN_AGE;
        newp->death = frand() * MAX_AGE;
        newp->vel = AddVectors (
	    ScaleVector (speed, vel),
	    MakeVector (VARIANCE_FACTOR * (frand() - 0.5) * speed, 
			 VARIANCE_FACTOR * (frand() - 0.5) * speed,
			 VARIANCE_FACTOR * (frand() - 0.5) * speed ));
    }
} 

void update_particles (double time_step) {
    Particle **p, *q;
    double ycoord;

    for (p = &head; *p != NULL;) {
		(**p).age += time_step;
        if  ((**p).age < 0) continue;

		(**p).pt = AddVectors ((**p).pt, ScaleVector (time_step, (**p).vel));
		ycoord = Course.FindYCoord ((**p).pt.x, (**p).pt.z);
		if ((**p).pt.y < ycoord - 3) {(**p).age = (**p).death + 1;} 
        if ((**p).age >= (**p).death) {
            q = *p;
            *p = q->next;
            free(q);
            num_particles -= 1;
            continue;
        } 

        (**p).alpha = ((**p).death - (**p).age) / (**p).death;
		(**p).cur_size = NEW_PART_SIZE + 
			    (OLD_PART_SIZE - NEW_PART_SIZE) * ((**p).age / (**p).death);
        (**p).vel.y += -EARTH_GRAV * time_step;
        p = &((**p).next);
    } 
} 

void draw_particles (CControl *ctrl) {
    Particle *p;
    TVector2 min_tex_coord, max_tex_coord;

    set_gl_options (PARTICLES);
	Tex.BindTex (SNOW_PART);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(part_col[0], part_col[1], part_col[2], part_col[3]);
	part_col[3] = 0.8;    // !!!!!!!!!

    for (p=head; p!=NULL; p = p->next) {
        if  (p->age < 0) continue;

	if  (p->type == 0 || p->type == 1) {
	    min_tex_coord.y = 0;
	    max_tex_coord.y = 0.5;
	} else {
	    min_tex_coord.y = 0.5;
	    max_tex_coord.y = 1.0;
	}

	if  (p->type == 0 || p->type == 3) {
	    min_tex_coord.x = 0;
	    max_tex_coord.x = 0.5;
	} else {
	    min_tex_coord.x = 0.5;
	    max_tex_coord.x = 1.0;
	}

	TColor particle_colour = Env.ParticleColor ();
	glColor4f (particle_colour.r, 
		   particle_colour.g, 
		   particle_colour.b,
		   particle_colour.a * p->alpha);

	draw_billboard (ctrl, p->pt, p->cur_size, p->cur_size,
			false, min_tex_coord, max_tex_coord);
    } 

} 

void clear_particles() {
    Particle *p, *q;
    
	p = head;
    for (;;) {
        if (p == NULL) break;
        q=p;
        p=p->next;
        free(q);
    } 
    head = NULL;
    num_particles = 0;
}

double adjust_particle_count (double particles) {
    if  (particles < 1) {
		if (((double) rand()) / RAND_MAX < particles) return 1.0;
		else return 0.0;
    } else return particles;
}

void generate_particles (CControl *ctrl, double dtime, TVector3 pos, double speed) {
    TVector3 left_part_pt, right_part_pt;
    double brake_particles;
    double turn_particles;
    double roll_particles;
    double surf_y;
    double left_particles, right_particles;
    TVector3 left_part_vel, right_part_vel;
    TMatrix rot_mat;
    TVector3 xvec;
	TTerrType *TerrList = Course.TerrList;

    surf_y = Course.FindYCoord (pos.x, pos.z);

	int id;
	id = Course.GetTerrainIdx (pos.x, pos.z, 0.5);
	if (id >= 0 && TerrList[id].particles > 0 && pos.y < surf_y) {
		xvec = CrossProduct (ctrl->cdirection, ctrl->plane_nml);
        right_part_pt = left_part_pt = pos;

		right_part_pt = AddVectors (
		    right_part_pt, 
		    ScaleVector (TUX_WIDTH/2.0, xvec));

		left_part_pt = AddVectors (
		    left_part_pt, 
		    ScaleVector (-TUX_WIDTH/2.0, xvec));

        right_part_pt.y = left_part_pt.y  = surf_y;

		brake_particles = dtime *
		    BRAKE_PARTICLES *  (ctrl->is_braking ? 1.0 : 0.0)
		    * min (speed / PARTICLE_SPEED_FACTOR, 1.0);
		turn_particles = dtime * MAX_TURN_PARTICLES 
		    * min (speed / PARTICLE_SPEED_FACTOR, 1.0);
		roll_particles = dtime * MAX_ROLL_PARTICLES 
		    * min (speed / PARTICLE_SPEED_FACTOR, 1.0);

		left_particles = turn_particles * 
		    fabs (min(ctrl->turn_fact, 0.)) + 
		    brake_particles +
		    roll_particles * fabs (min(ctrl->turn_animation, 0.));

		right_particles = turn_particles * 
		    fabs (max(ctrl->turn_fact, 0.)) + 
		    brake_particles +
		    roll_particles * fabs (max(ctrl->turn_animation, 0.));

		left_particles = adjust_particle_count (left_particles);
		right_particles = adjust_particle_count (right_particles);

		RotateAboutVectorMatrix(
		    rot_mat, ctrl->cdirection,
		    MAX (-MAX_PARTICLE_ANGLE, 
			 -MAX_PARTICLE_ANGLE * speed / MAX_PARTICLE_ANGLE_SPEED));
		left_part_vel = TransformVector (rot_mat, ctrl->plane_nml);
		left_part_vel = ScaleVector (MIN (MAX_PARTICLE_SPEED, 
			speed * PARTICLE_SPEED_MULTIPLIER),left_part_vel);

		RotateAboutVectorMatrix(
		    rot_mat, ctrl->cdirection,
		    MIN (MAX_PARTICLE_ANGLE, 
			 MAX_PARTICLE_ANGLE * speed / MAX_PARTICLE_ANGLE_SPEED));
		right_part_vel = TransformVector (rot_mat, ctrl->plane_nml);
		right_part_vel = ScaleVector (MIN (MAX_PARTICLE_SPEED,
			speed * PARTICLE_SPEED_MULTIPLIER),right_part_vel);


        create_new_particles (left_part_pt, left_part_vel, 
			      (int)left_particles);
        create_new_particles (right_part_pt, right_part_vel, 
			      (int)right_particles);
    } 
}

// --------------------------------------------------------------------
//					snow flakes
// --------------------------------------------------------------------

#define SNOW_WIND_DRIFT  0.1

CFlakes::CFlakes () {
	for (int i=0; i<MAX_FLAKEAREAS; i++) areas[i].flakes = 0;
	numAreas = 0;
}

CFlakes::~CFlakes () {
	Reset ();
}

void CFlakes::Reset () {
	for (int i=0; i<MAX_FLAKEAREAS; i++) {
		if (areas[i].flakes != 0) delete [] areas[i].flakes; 
		areas[i].flakes = 0;
	}
	numAreas = 0;
}

static CFlakes Flakes;

void CFlakes::CreateArea (
		int   num_flakes, 
		float xrange,
		float ytop,
		float yrange,
		float zback,
		float zrange,
		float minSize,
		float maxSize,
		float speed,
		bool  rotate) {
	if (numAreas >= MAX_FLAKEAREAS) return;
	areas[numAreas].num_flakes = num_flakes;
	areas[numAreas].xrange = xrange;
	areas[numAreas].ytop = ytop;
	areas[numAreas].yrange = yrange;
	areas[numAreas].zback = zback;
	areas[numAreas].zrange = zrange;
	areas[numAreas].minSize = minSize;
	areas[numAreas].maxSize = maxSize;
	areas[numAreas].speed = speed;
	areas[numAreas].rotate_flake = rotate;

	areas[numAreas].flakes = new TFlake[num_flakes];
	numAreas++;
}

void CFlakes::MakeSnowFlake (int ar, int i) {
    float p_dist;
    int type;
	areas[ar].flakes[i].pt.x = XRandom (areas[ar].left, areas[ar].right);  
	areas[ar].flakes[i].pt.y = -XRandom (areas[ar].top, areas[ar].bottom);
	areas[ar].flakes[i].pt.z = areas[ar].back - FRandom () * (areas[ar].back - areas[ar].front);

	p_dist = FRandom ();
    areas[ar].flakes[i].size = XRandom (areas[ar].minSize, areas[ar].maxSize);
	areas[ar].flakes[i].vel.x = 0;
	areas[ar].flakes[i].vel.z = 0;
	areas[ar].flakes[i].vel.y = -areas[ar].flakes[i].size * areas[ar].speed;	
    
	type = (int) (FRandom () * 3.9999);

	if (type == 0) {
		areas[ar].flakes[i].tex_min = MakeVector2  (0.0, 0.875);
		areas[ar].flakes[i].tex_max = MakeVector2  (0.125, 1.0);
    } else if (type == 1) {
		areas[ar].flakes[i].tex_min = MakeVector2  (0.125, 0.875);
		areas[ar].flakes[i].tex_max = MakeVector2  (0.25, 1.0);
	} else if (type == 2) {
		areas[ar].flakes[i].tex_min = MakeVector2  (0.0, 0.75);
		areas[ar].flakes[i].tex_max = MakeVector2  (0.125, 0.875);
    } else {
		areas[ar].flakes[i].tex_min = MakeVector2  (0.125, 0.75);
 		areas[ar].flakes[i].tex_max = MakeVector2  (0.25, 0.875);
    }
}

void CFlakes::GenerateSnowFlakes (CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	snow_lastpos = ctrl->cpos;
	for (int ar=0; ar<numAreas; ar++) {		
		for (int i=0; i<areas[ar].num_flakes; i++) MakeSnowFlake (ar, i);
	}
}

void CFlakes::UpdateAreas (CControl *ctrl) {
	for (int ar=0; ar<numAreas; ar++) {
		areas[ar].left = ctrl->cpos.x - areas[ar].xrange / 2;
		areas[ar].right = areas[ar].left + areas[ar].xrange;
		areas[ar].back = ctrl->cpos.z - areas[ar].zback;
		areas[ar].front = areas[ar].back - areas[ar].zrange;
		areas[ar].top = ctrl->cpos.y + areas[ar].ytop;
		areas[ar].bottom = areas[ar].top - areas[ar].yrange;	
	}
}

#define YDRIFT 0.8
#define ZDRIFT 0.6

void CFlakes::Init (int grade, CControl *ctrl) {
	Reset ();
	switch (grade) {
		case 1:
//			CreateArea (400, 5, 4, 4,     -2, 4, 0.01, 0.02,    5, true);
//			CreateArea (400, 12, 5, 8,      2, 8, 0.03, 0.045,    5, false);
//			CreateArea (400, 30, 6, 15,      10, 15, 0.06, 0.12,    5, false);
			CreateArea (400, 5, 4, 4,     -2, 4, 0.015, 0.03,    5, true);
			CreateArea (400, 12, 5, 8,      2, 8, 0.045, 0.07,    5, false);
			CreateArea (400, 30, 6, 15,      10, 15, 0.09, 0.18,    5, false);
//			CreateArea (400, 5, 4, 4,     -2, 4, 0.02, 0.04,    5, true);
//			CreateArea (400, 12, 5, 8,      2, 8, 0.06, 0.09,    5, false);
//			CreateArea (400, 30, 6, 15,      10, 15, 0.15, 0.25,    5, false);
			break;
		case 2:
//			CreateArea (500, 5, 4, 4,     -2, 4, 0.02, 0.03,    5, true);
//			CreateArea (500, 12, 5, 8,      2, 8, 0.045, 0.07,    5, false);
//			CreateArea (500, 30, 6, 15,      10, 15, 0.1, 0.15,    5, false);
			CreateArea (500, 5, 4, 4,     -2, 4, 0.03, 0.045,    5, true);
			CreateArea (500, 12, 5, 8,      2, 8, 0.07, 0.1,    5, false);
			CreateArea (500, 30, 6, 15,      10, 15, 0.15, 0.22,    5, false);
//			CreateArea (500, 5, 4, 4,     -2, 4, 0.04, 0.06,    5, true);
//			CreateArea (500, 12, 5, 8,      2, 8, 0.09, 0.15,    5, false);
//			CreateArea (500, 30, 6, 15,      10, 15, 0.2, 0.32,    5, false);
			break;
		case 3:
//			CreateArea (1000, 5, 4, 4,     -2, 4, 0.025, 0.04,    5, true);
//			CreateArea (1000, 12, 5, 9,      2, 8, 0.06, 0.10,    5, false);
//			CreateArea (1000, 30, 6, 15,      10, 15, 0.12, 0.2,    5, false);
			CreateArea (1000, 5, 4, 4,     -2, 4, 0.037, 0.05,    5, true);
			CreateArea (1000, 12, 5, 9,      2, 8, 0.09, 0.15,    5, false);
			CreateArea (1000, 30, 6, 15,      10, 15, 0.18, 0.35,    5, false);
//			CreateArea (800, 5, 4, 4,     -2, 4, 0.05, 0.08,    5, true);
//			CreateArea (800, 12, 5, 9,      2, 8, 0.12, 0.20,    5, false);
//			CreateArea (800, 30, 6, 15,      10, 15, 0.25, 0.5,    5, false);
			break;
		default: break;		
	}

	UpdateAreas (ctrl);
	GenerateSnowFlakes (ctrl);
}

void CFlakes::Update (double timestep, CControl *ctrl) {
    int i;
	float ydiff, zdiff;
		
	if (g_game.snow_id < 1) return;
	UpdateAreas (ctrl);

	zdiff = ctrl->cpos.z - snow_lastpos.z;
	if (g_game.mode != GAME_OVER) {
		ydiff = ctrl->cpos.y - snow_lastpos.y;
	} else ydiff = 0; 

	TVector3 winddrift = ScaleVector (SNOW_WIND_DRIFT, Wind.WindDrift ());
	float xcoeff = winddrift.x * timestep;
	float ycoeff = (ydiff * YDRIFT) + (winddrift.z * timestep);	
	float zcoeff = (zdiff * ZDRIFT) + (winddrift.z * timestep);

	for (int ar=0; ar<numAreas; ar++) {
		for (i=0; i<areas[ar].num_flakes; i++) {
			areas[ar].flakes[i].pt.x += xcoeff;
			areas[ar].flakes[i].pt.y += areas[ar].flakes[i].vel.y * timestep + ycoeff;
			areas[ar].flakes[i].pt.z += zcoeff;

 			if (areas[ar].flakes[i].pt.y < areas[ar].bottom) {
				areas[ar].flakes[i].pt.y += areas[ar].yrange;	
			} else if (areas[ar].flakes[i].pt.x < areas[ar].left) {
				areas[ar].flakes[i].pt.x += areas[ar].xrange;
			} else if (areas[ar].flakes[i].pt.x > areas[ar].right) {
				areas[ar].flakes[i].pt.x -= areas[ar].xrange;
			} else if (areas[ar].flakes[i].pt.y > areas[ar].top) {
				areas[ar].flakes[i].pt.y -= areas[ar].yrange;			
			} else if (areas[ar].flakes[i].pt.z < areas[ar].front) {
				areas[ar].flakes[i].pt.z += areas[ar].zrange;		
			} else if (areas[ar].flakes[i].pt.z > areas[ar].back) {
				areas[ar].flakes[i].pt.z -= areas[ar].zrange;		
			} 
		}		
	}
	snow_lastpos = ctrl->cpos;
}

void CFlakes::DrawArea (int ar, CControl *ctrl) {
    TVector2 *tex_min, *tex_max;
    TVector3 *pt;
	float size;
    int i;
	TFlake flake;

	if (g_game.snow_id < 1) return;

	TPlane lp = get_left_clip_plane ();
	TPlane rp = get_right_clip_plane ();
	float dir_angle (atan (ctrl->viewdir.x / ctrl->viewdir.z) * 180 / 3.14159);

	set_gl_options (PARTICLES);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	Tex.BindTex (T_WIDGETS);  
	TColor particle_colour = Env.ParticleColor ();
    glColor4f (particle_colour.r, particle_colour.g, particle_colour.b, particle_colour.a);

	for  (i=0; i < areas[ar].num_flakes; i++) {
		flake = areas[ar].flakes[i];
		pt = &flake.pt;
	    size = flake.size;
	    tex_min = &flake.tex_min;
	    tex_max = &flake.tex_max;

		if ((DistanceToPlane (lp, *pt) < 0) && (DistanceToPlane (rp, *pt) < 0)) {
			glPushMatrix();
			glTranslatef (pt->x, pt->y, pt->z);
			if (areas[ar].rotate_flake) glRotatef (dir_angle, 0, 1, 0);
			glBegin (GL_QUADS);
				glTexCoord2f (tex_min->x, tex_min->y);
				glVertex3f (0, 0, 0);
				glTexCoord2f (tex_max->x, tex_min->y);
				glVertex3f (size, 0, 0);
				glTexCoord2f (tex_max->x, tex_max->y);
				glVertex3f (size, size, 0);
				glTexCoord2f (tex_min->x, tex_max->y);
				glVertex3f (0, size, 0);
			glEnd();
			glPopMatrix();
		}
	} 
} 

void CFlakes::Draw (CControl *ctrl) {
	for (int ar=0; ar<numAreas; ar++) DrawArea (ar, ctrl);
}

// --------------------------------------------------------------------
//					snow curtains
// --------------------------------------------------------------------

#define NUM_CHANGES 6
#define CHANGE_DRIFT 15
#define CHANGE_SPEED 0.05
#define CURTAIN_WINDDRIFT 0.35

typedef struct {
	float min;
	float max;
	float curr;
	float step;
	bool forward;
} TChange;

TChange changes[NUM_CHANGES];

void InitChanges () {
 	for (int i=0; i<NUM_CHANGES; i++) {
		changes[i].min = XRandom (-0.15, -0.05);
		changes[i].max = XRandom (0.05, 0.15);
		changes[i].curr = (changes[i].min + changes[i].max) / 2;
		changes[i].step = CHANGE_SPEED;
		changes[i].forward = true;
	}
}

void UpdateChanges (double timestep) {
	TChange *ch;
	for (int i=0; i<NUM_CHANGES; i++) {
		ch = &changes[i];
		if (ch->forward) {
			ch->curr += ch->step * timestep;
			if (ch->curr > ch->max) ch->forward = false;
		} else {
			ch->curr -= ch->step * timestep;
			if (ch->curr < ch->min) ch->forward = true;
		}
	}
}

static CCurtain Curtain; 

void CCurtain::CurtainVec (float angle, float zdist, float &x, float &z) {
	x = zdist  * sin (angle * 3.14159 / 180);
	if (angle > 90 || angle < -90) z = sqrt (zdist * zdist - x * x);
	else z = -sqrt (zdist * zdist - x * x);
}

void CCurtain::Draw (CControl *ctrl) {
    TVector3 *pt;

	if (g_game.snow_id < 1) return;
	set_gl_options (PARTICLES);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	TColor particle_colour = Env.ParticleColor ();
	glColor4f (particle_colour.r, particle_colour.g, particle_colour.b, 1.0);

	// glEnable (GL_NORMALIZE);
	for (int i=0; i<MAX_CURTAINS; i++) {
		if (enabled[i]) {	
			Tex.BindTex (texture[i]);  
			float halfsize = size[i] / 2;
			for (int co=0; co<numCols[i]; co++) {
				for (int row=0; row<numRows[i]; row++) {
					pt = &curtains[i][co][row].pt;
					glPushMatrix();
					glTranslatef (pt->x, pt->y, pt->z);
					glRotatef (-curtains[i][co][row].angle, 0, 1, 0);
					// glNormal3f (0, 0, 1);
					glBegin (GL_QUADS);
						glTexCoord2f (0, 0);
						glVertex3f (-halfsize, -halfsize, 0);
						glTexCoord2f (1, 0);
						glVertex3f (halfsize, -halfsize, 0);
						glTexCoord2f (1, 1);
						glVertex3f (halfsize, halfsize, 0);
						glTexCoord2f (0, 1);
						glVertex3f (-halfsize, halfsize, 0);
					glEnd();
					glPopMatrix();
				}
			}
		}
	}
}

void CCurtain::Update (float timestep, CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	float x, z;
	TCurtainElement *curt = 0; 
	TVector3 drift = Wind.WindDrift ();

	UpdateChanges (timestep);
	for (int i=0; i<MAX_CURTAINS; i++) {
		if (enabled[i]) {
			for (int co=0; co<numCols[i]; co++) {
				for (int row=0; row<numRows[i]; row++) {
					curt = &curtains[i][co][row];

					curt->angle += changes[chg[i][row]].curr * timestep * CHANGE_DRIFT;
					curt->angle += drift.x * timestep * CURTAIN_WINDDRIFT;
					curt->height -= speed[i] * timestep;

					if (curt->angle > lastangle[i] + angledist[i]) curt->angle = startangle[i];
					if (curt->angle < startangle[i] - angledist[i]) curt->angle = lastangle[i];		
					CurtainVec (curt->angle, zdist[i], x, z);
					curt->pt.x = ctrl->cpos.x + x;
					curt->pt.z = ctrl->cpos.z + z ;
					curt->pt.y = ctrl->cpos.y + curt->height;
					if (curt->height < minheight[i] - size[i]) curt->height += numRows[i] * size[i];		
				}
			}
		}
	}
	Draw (ctrl);
}

void CCurtain::Reset () {
	for (int i=0; i<MAX_CURTAINS; i++) enabled[i] = false;
}

void CCurtain::GenerateCurtain (int nr,	int num_rows, float z_dist, float tex_size,
		float base_speed, float start_angle, float min_height, int dense) {
	if (nr < 0 || nr >= MAX_CURTAINS) return;
	enabled[nr] = true;
	numRows[nr] = num_rows;
	zdist[nr] = z_dist;
	size[nr] = tex_size;
	speed[nr] = base_speed;
	startangle[nr] = start_angle;
	minheight[nr] = min_height;
	switch (dense) {
		case 1: texture[nr] = T_SNOW1; break;
		case 2: texture[nr] = T_SNOW2; break;
		case 3: texture[nr] = T_SNOW3; break;
	}

	angledist[nr] = atan (size[nr] / 2 / zdist[nr]) * 360 / 3.14159;
	numCols[nr] = (int)(-2 * startangle[nr] / angledist[nr]) + 1;
	if (numCols[nr] > MAX_CURTAIN_COLS) numCols[nr] = MAX_CURTAIN_COLS;
	lastangle[nr] = startangle[nr] + (numCols[nr]-1) * angledist[nr];

	for (int i=0; i<numRows[nr]; i++) chg[nr][i] = IRandom (0, 5);		
}

void CCurtain::SetStartParams (CControl *ctrl) {
	float x, z;
	TCurtainElement *curt = 0; 

	for (int i=0; i<MAX_CURTAINS; i++) {
		if (enabled[i]) {
			for (int co=0; co<numCols[i]; co++) {
				for (int row=0; row<numRows[i]; row++) {
					curt = &curtains[i][co][row];
					curt->height = minheight[i] + row * size[i];
					curt->angle = co * angledist[i] + startangle[i];
					CurtainVec (curt->angle, zdist[i], x, z);
					curt->pt.x = ctrl->cpos.x + x;
					curt->pt.z = ctrl->cpos.z + z;
					curt->pt.y = ctrl->cpos.y + curt->height;
				}
			}
		}
	}
}

void CCurtain::Init (CControl *ctrl) {
	Reset ();
	InitChanges ();
	switch (g_game.snow_id) {
	case 1: 
//		GenerateCurtain (0, 3, 60, 10,       3, -100, -10, 1);
//		GenerateCurtain (1, 3, 50, 13,       3, -100, -10, 1);
//		GenerateCurtain (2, 3, 40, 16,       3, -100, -10, 1);
		GenerateCurtain (0, 3, 60, 15,       3, -100, -10, 1);
		GenerateCurtain (1, 3, 50, 19,       3, -100, -10, 1);
		GenerateCurtain (2, 3, 40, 23,       3, -100, -10, 1);
//		GenerateCurtain (0, 3, 60, 20,       3, -100, -10, 1);
//		GenerateCurtain (1, 3, 50, 25,       3, -100, -10, 1);
//		GenerateCurtain (2, 3, 40, 30,       3, -100, -10, 1);
		break;
	case 2: 
//		GenerateCurtain (0, 3, 60, 15,       3, -100, -10, 2);
//		GenerateCurtain (1, 3, 50, 17,       3, -100, -10, 2);
//		GenerateCurtain (2, 3, 40, 20,       3, -100, -10, 2);
		GenerateCurtain (0, 3, 60, 22,       3, -100, -10, 2);
		GenerateCurtain (1, 3, 50, 25,       3, -100, -10, 2);
		GenerateCurtain (2, 3, 40, 30,       3, -100, -10, 2);
//		GenerateCurtain (0, 3, 60, 30,       3, -100, -10, 2);
//		GenerateCurtain (1, 3, 50, 35,       3, -100, -10, 2);
//		GenerateCurtain (2, 3, 40, 40,       3, -100, -10, 2);
		break;
	case 3: 
//		GenerateCurtain (0, 3, 60, 20,       3, -100, -10, 3);
//		GenerateCurtain (1, 3, 50, 25,       3, -100, -10, 2);
//		GenerateCurtain (2, 3, 40, 30,       3, -100, -10, 2);
		GenerateCurtain (0, 3, 60, 22,       3, -100, -10, 3);
		GenerateCurtain (1, 3, 50, 27,       3, -100, -10, 2);
		GenerateCurtain (2, 3, 40, 32,       3, -100, -10, 2);
//		GenerateCurtain (0, 3, 60, 25,       3, -100, -10, 3);
//		GenerateCurtain (1, 3, 50, 30,       3, -100, -10, 2);
//		GenerateCurtain (2, 3, 40, 35,       3, -100, -10, 2);
		break;
	default: {}
	}
	SetStartParams (ctrl);
}

// --------------------------------------------------------------------
//					wind
// --------------------------------------------------------------------

#define UPDATE_TIME 0.04

CWind Wind;

CWind::CWind () {
	windy = false;
	CurrTime = 0.0;
	WVector = MakeVector (0, 0, 0);
	
	SpeedMode = 0;
	AngleMode = 0;
	WSpeed = 0;
	WAngle = 0;
	DestSpeed = 0;
	DestAngle = 0;
	WindChange = 0;
	AngleChange = 0;
}

void CWind::SetParams (int grade) {
	float min_base_speed = 0; 
	float max_base_speed = 0;
	float min_speed_var = 0;
	float max_speed_var = 0;
	float min_base_angle = 0; 
	float max_base_angle = 0; 
	float min_angle_var = 0;
	float max_angle_var = 0;
	float alt_angle = 0;

	if (grade == 0) {
		min_base_speed = 20; 
		max_base_speed = 35;
		min_speed_var = 20;
		max_speed_var = 20;
		params.minChange = 0.1;
		params.maxChange = 0.3;

		min_base_angle = 70; 
		max_base_angle = 110; 
		min_angle_var = 0;
		max_angle_var = 90;
		params.minAngleChange = 0.1;
		params.maxAngleChange = 1.0;

		params.topSpeed = 100;
		params.topProbability = 0;
		params.nullProbability = 6;
		alt_angle = 180;
	} else if (grade == 1) {
		min_base_speed = 30; 
		max_base_speed = 60;
		min_speed_var = 40;
		max_speed_var = 40;
		params.minChange = 0.1;
		params.maxChange = 0.5;

		min_base_angle = 70; 
		max_base_angle = 110; 
		min_angle_var = 0;
		max_angle_var = 90;
		params.minAngleChange = 0.1;
		params.maxAngleChange = 1.0;

		params.topSpeed = 100;
		params.topProbability = 0;
		params.nullProbability = 10;
		alt_angle = 180;
	} else {
		min_base_speed = 40; 
		max_base_speed = 80;
		min_speed_var = 30;
		max_speed_var = 60;
		params.minChange = 0.1;
		params.maxChange = 1.0;

		min_base_angle = 0; 
		max_base_angle = 180; 
		min_angle_var = 180;
		max_angle_var = 360;
		params.minAngleChange = 0.1;
		params.maxAngleChange = 1.0;

		params.topSpeed = 100;
		params.topProbability = 10;
		params.nullProbability = 10;
		alt_angle = 0;
	}

	float speed, var, angle;

	speed = XRandom (min_base_speed, max_base_speed);		
	var = XRandom (min_speed_var, max_speed_var) / 2;
	params.minSpeed = speed - var;
	params.maxSpeed = speed + var;
	if (params.minSpeed < 0) params.minSpeed = 0;
	if (params.maxSpeed > 100) params.maxSpeed = 100;

	angle = XRandom (min_base_angle, max_base_angle);	
	if (XRandom (0, 100) > 50) angle = angle + alt_angle;	
	var = XRandom (min_angle_var, max_angle_var) / 2;
	params.minAngle = angle - var;
	params.maxAngle = angle + var;
}

void CWind::CalcDestSpeed () {	
	float rand = XRandom (0, 100);
	if (rand > (100 - params.topProbability)) {
		DestSpeed = XRandom (params.maxSpeed, params.topSpeed);
		WindChange = params.maxChange;
	} else if (rand < params.nullProbability) {
		DestSpeed = 0.0;
		WindChange = XRandom (params.minChange, params.maxChange);
	} else {
		DestSpeed = XRandom (params.minSpeed, params.maxSpeed);
		WindChange = XRandom (params.minChange, params.maxChange);
	}

	if (DestSpeed > WSpeed) SpeedMode = 1; else SpeedMode = 0;
}

void CWind::CalcDestAngle () {
	DestAngle = XRandom (params.minAngle, params.maxAngle);
	AngleChange = XRandom (params.minAngleChange, params.maxAngleChange);

	if (DestAngle > WAngle) AngleMode = 1; else AngleMode = 0;
}
		

void CWind::Update (float timestep) {
	float xx, zz;
	
	if (!windy) return;	
	// the wind needn't be updated in each frame
	CurrTime = CurrTime + timestep;
	if (CurrTime > UPDATE_TIME) {
		CurrTime = 0.0;
		
		if (SpeedMode == 1) { // current speed lesser than destination speed
			if (WSpeed < DestSpeed) {
				WSpeed = WSpeed + WindChange;		
			} else CalcDestSpeed ();		
		} else {
			if (WSpeed > DestSpeed) {
				WSpeed = WSpeed - WindChange;		
			} else CalcDestSpeed ();							
		}	
		if (WSpeed > params.topSpeed) WSpeed = params.topSpeed;
		if (WSpeed < 0) WSpeed = 0;
	
	
		if (AngleMode == 1) {
			if (WAngle < DestAngle) {
				WAngle = WAngle + AngleChange;
			} else CalcDestAngle ();
		} else {
			if (WAngle > DestAngle) {
				WAngle = WAngle - AngleChange;
			} else CalcDestAngle ();
		}
		if (WAngle > params.maxAngle) WAngle = params.maxAngle;
		if (WAngle < params.minAngle) WAngle = params.minAngle;
			
		xx = sin (WAngle * 3.14159 / 180);
		zz = sqrt (1 - xx * xx);
		if ((WAngle > 90 && WAngle < 270) || (WAngle > 450 && WAngle < 630)) {
			zz = -zz;
		}

		WVector.x = WSpeed * xx;
		WVector.z = WSpeed * zz * 0.2;
	}
}

void CWind::Init (int wind_id) {
	if (wind_id < 1 || wind_id > 3) {
		windy = false;
		WVector = MakeVector (0, 0, 0);
		WAngle = 0;
		WSpeed = 0;
		return;
	}
	windy = true;;
	SetParams (wind_id -1);
	WSpeed = XRandom (params.minSpeed, (params.minSpeed + params.maxSpeed) / 2);
	WAngle = XRandom (params.minAngle, params.maxAngle);
	CalcDestSpeed ();
	CalcDestAngle ();
}

// ====================================================================
//			access functions
// ====================================================================

void InitSnow (CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Init (g_game.snow_id, ctrl);
	Curtain.Init (ctrl);
}

 void UpdateSnow (double timestep, CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Update (timestep, ctrl);
	Curtain.Update (timestep, ctrl);
}

void DrawSnow (CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Draw (ctrl);
	Curtain.Draw (ctrl);
}

void InitWind () {
	Wind.Init (g_game.wind_id);
}

void UpdateWind (double timestep, CControl *ctrl) {
	Wind.Update (timestep);
}



