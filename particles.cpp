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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "particles.h"
#include "textures.h"
#include "ogl.h"
#include "course.h"
#include "view.h"
#include "env.h"
#include "game_over.h"
#include "winsys.h"
#include "physics.h"
#include <list>

// ====================================================================
//					gui particles 2D
// ====================================================================

#define MAX_num_snowparticles 4000
#define BASE_num_snowparticles 1000
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

struct TGuiParticle {
    TVector2 pt;
    double size;
    TVector2 vel;
    TVector2 tex_min;
    TVector2 tex_max;

	TGuiParticle(double x, double y);
	void Draw(double xres, double yres) const;
	void Update(double time_step, double push_timestep, const TVector2& push_vector);
};

static list<TGuiParticle> particles_2d;
static GLfloat part_col[4] = {1, 1, 1, 0.5 };
static TVector2 push_position(0, 0);
static TVector2 last_push_position;
static double last_update_time = -1;
static bool push_position_initialized = false;

static double frand () {return (double)rand() / RAND_MAX; }

TGuiParticle::TGuiParticle(double x, double y) {
    pt.x = x;
    pt.y = y;
    double p_dist = frand();

	size = PARTICLE_MIN_SIZE + (1.0 - p_dist) * PARTICLE_SIZE_RANGE;
    vel.x = 0;
    vel.y = -BASE_VELOCITY - p_dist * VELOCITY_RANGE;

	int type = (int) (frand() * (4.0 - EPS));
	if (type == 0) {
		tex_min = TVector2(0.0, 0.0);
		tex_max = TVector2(0.5, 0.5);
    } else if (type == 1) {
		tex_min = TVector2(0.5, 0.0);
		tex_max = TVector2(1.0, 0.5);
    } else if (type == 2) {
		tex_min = TVector2(0.5, 0.5);
		tex_max = TVector2(1.0, 1.0);
    } else {
		tex_min = TVector2(0.0, 0.5);
		tex_max = TVector2(0.5, 1.0);
    }
}

void TGuiParticle::Draw(double xres, double yres) const {
	glPushMatrix();
	glTranslatef (pt.x * xres, pt.y * yres, 0);
	glBegin (GL_QUADS);
		glTexCoord2f (tex_min.x, tex_min.y);
		glVertex2f (0, 0);
		glTexCoord2f (tex_max.x, tex_min.y);
		glVertex2f (size, 0);
		glTexCoord2f (tex_max.x, tex_max.y);
		glVertex2f (size, size);
		glTexCoord2f (tex_min.x, tex_max.y);
		glVertex2f (0, size);
	glEnd();
	glPopMatrix();
}

void TGuiParticle::Update(double time_step, double push_timestep, const TVector2& push_vector) {
	TVector2 f;

	double dist_from_push = (pow((pt.x - push_position.x), 2) +
			pow((pt.y - push_position.y), 2));
	if (push_timestep > 0) {
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

	vel.x +=  (f.x - vel.x * AIR_DRAG) *  time_step;
	vel.y +=  (f.y - GRAVITY_FACTOR - vel.y * AIR_DRAG) * time_step;

	pt.x += vel.x * time_step *  (size / PARTICLE_SIZE_RANGE);
	pt.y += vel.y * time_step *  (size / PARTICLE_SIZE_RANGE);

	if (pt.x < 0) {
		pt.x = 1;
	} else if (pt.x > 1) {
		pt.x = 0.0;
	}
}

void init_ui_snow () {
	for (int i=0; i<BASE_num_snowparticles; i++)
		particles_2d.push_back(TGuiParticle( frand(), frand()));
    push_position = TVector2(0.0, 0.0);
}

void update_ui_snow (double time_step) {
    double time = Winsys.ClockTime ();

    TVector2 push_vector;
    double push_timestep = 0;

    if (push_position_initialized) {
		push_vector.x = push_position.x - last_push_position.x;
		push_vector.y = push_position.y - last_push_position.y;
		push_timestep = time - last_update_time;
    }
    last_push_position = push_position;
    last_update_time = time;

    for (list<TGuiParticle>::iterator p = particles_2d.begin(); p != particles_2d.end(); ++p) {
		p->Update(time_step, push_timestep, push_vector);
    }

	if(frand() < time_step*20.0*(MAX_num_snowparticles - particles_2d.size())/1000.0) {
		particles_2d.push_back(TGuiParticle(frand(), 1));
	}

    for (list<TGuiParticle>::iterator p = particles_2d.begin(); p != particles_2d.end();) {
		if (p->pt.y < -0.05) {
			if (particles_2d.size() > BASE_num_snowparticles && frand() > 0.2) {
				p = particles_2d.erase(p);
			} else {
				p->pt.x = frand();
				p->pt.y = 1+frand()*BASE_VELOCITY;
				double p_dist = frand();
				p->size = PARTICLE_MIN_SIZE + (1.0 - p_dist) * PARTICLE_SIZE_RANGE;
				p->vel.x = 0;
				p->vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
				++p;
			}
		} else
			++p;
    }

    if (time_step < PUSH_DECAY_TIME_CONSTANT) {
		push_vector.x *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
		push_vector.y *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
    } else {
		push_vector.x = 0.0;
		push_vector.y = 0.0;
    }
}
void draw_ui_snow () {
    double xres = Winsys.resolution.width;
    double yres = Winsys.resolution.height;

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	Tex.BindTex (SNOW_PART);
    glColor4f(part_col[0], part_col[1], part_col[2], part_col[3]);
	part_col[3] = 0.3;
	for (list<TGuiParticle>::const_iterator i = particles_2d.begin(); i != particles_2d.end(); ++i) {
		i->Draw(xres, yres);
    }
}

void push_ui_snow (const TVector2& pos) {
    push_position = TVector2(pos.x/(double)Winsys.resolution.width, 1.0 - pos.y/(double)Winsys.resolution.height);
    if (!push_position_initialized) last_push_position = push_position;
    push_position_initialized = true;
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


struct Particle {
    TVector3 pt;
    short type;
    double base_size;
    double cur_size;
    double terrain_height;
    double age;
    double death;
    double alpha;
    TVector3 vel;

	void Draw(const CControl* ctrl) const;
	void draw_billboard (const CControl *ctrl, double width, double height, bool use_world_y_axis,
		const TVector2& min_tex_coord, const TVector2& max_tex_coord) const;
};

static list<Particle> particles;

void Particle::Draw(const CControl* ctrl) const {
	TVector2 min_tex_coord, max_tex_coord;
	if (type == 0 || type == 1) {
		min_tex_coord.y = 0;
		max_tex_coord.y = 0.5;
	} else {
		min_tex_coord.y = 0.5;
		max_tex_coord.y = 1.0;
	}

	if (type == 0 || type == 3) {
		min_tex_coord.x = 0;
		max_tex_coord.x = 0.5;
	} else {
		min_tex_coord.x = 0.5;
		max_tex_coord.x = 1.0;
	}

	const TColor& particle_colour = Env.ParticleColor ();
	glColor4f (particle_colour.r,
			particle_colour.g,
			particle_colour.b,
			particle_colour.a * alpha);

	draw_billboard (ctrl, cur_size, cur_size, false, min_tex_coord, max_tex_coord);
}

void Particle::draw_billboard (const CControl *ctrl, double width, double height, bool use_world_y_axis, const TVector2& min_tex_coord, const TVector2& max_tex_coord) const
{
    TVector3 x_vec;
    TVector3 y_vec;
    TVector3 z_vec;

    x_vec.x = ctrl->view_mat[0][0];
    x_vec.y = ctrl->view_mat[0][1];
    x_vec.z = ctrl->view_mat[0][2];

    if (use_world_y_axis) {
		y_vec = TVector3(0, 1, 0);
		x_vec = ProjectToPlane (y_vec, x_vec);
		NormVector (x_vec);
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
		TVector3 newpt = AddVectors (pt, ScaleVector (-width/2.0, x_vec));
		newpt = AddVectors (newpt, ScaleVector (-height/2.0, y_vec));
		glNormal3f (z_vec.x, z_vec.y, z_vec.z);
		glTexCoord2f (min_tex_coord.x, min_tex_coord.y);
		glVertex3f (newpt.x, newpt.y, newpt.z);

		newpt = AddVectors (newpt, ScaleVector (width, x_vec));
		glTexCoord2f (max_tex_coord.x, min_tex_coord.y);
		glVertex3f (newpt.x, newpt.y, newpt.z);

		newpt = AddVectors (newpt, ScaleVector (height, y_vec));
		glTexCoord2f (max_tex_coord.x, max_tex_coord.y);
		glVertex3f (newpt.x, newpt.y, newpt.z);

		newpt = AddVectors (newpt, ScaleVector (-width, x_vec));
		glTexCoord2f (min_tex_coord.x, max_tex_coord.y);
		glVertex3f (newpt.x, newpt.y, newpt.z);
    glEnd ();
}

void create_new_particles (const TVector3& loc, TVector3 vel, int num) {
    double speed = NormVector (vel);

    if (particles.size() + num > MAX_PARTICLES) {
		Message ("maximum number of particles exceeded", "");
    }
    for (int i=0; i<num; i++) {
		particles.push_back(Particle());
        Particle* newp = &particles.back();
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
			TVector3(VARIANCE_FACTOR * (frand() - 0.5) * speed,
				 VARIANCE_FACTOR * (frand() - 0.5) * speed,
				 VARIANCE_FACTOR * (frand() - 0.5) * speed ));
    }
}
void update_particles (double time_step) {
	for(list<Particle>::iterator p = particles.begin(); p != particles.end();) {
		p->age += time_step;
        if (p->age < 0) {
			++p;
			continue;
		}

		p->pt = AddVectors (p->pt, ScaleVector (time_step, p->vel));
		double ycoord = Course.FindYCoord (p->pt.x, p->pt.z);
		if (p->pt.y < ycoord - 3) {p->age = p->death + 1;}
        if (p->age >= p->death) {
			p = particles.erase(p);
            continue;
        }
        p->alpha = (p->death - p->age) / p->death;
		p->cur_size = NEW_PART_SIZE +
			    (OLD_PART_SIZE - NEW_PART_SIZE) * (p->age / p->death);
        p->vel.y += -EARTH_GRAV * time_step;
        ++p;
	}
}
void draw_particles (const CControl *ctrl) {
    ScopedRenderMode rm(PARTICLES);
	Tex.BindTex (SNOW_PART);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(part_col[0], part_col[1], part_col[2], part_col[3]);
	part_col[3] = 0.8;    // !!!!!!!!!

	for(list<Particle>::const_iterator p = particles.begin(); p != particles.end(); ++p) {
        if (p->age >= 0)
			p->Draw(ctrl);
    }
}
void clear_particles() {
	particles.clear();
}

double adjust_particle_count (double particles) {
    if (particles < 1) {
		if (((double) rand()) / RAND_MAX < particles) return 1.0;
		else return 0.0;
    } else return particles;
}

void generate_particles (const CControl *ctrl, double dtime, const TVector3& pos, double speed) {
    double brake_particles;
    double turn_particles;
    double roll_particles;
    double surf_y;
    double left_particles, right_particles;
    TMatrix rot_mat;
	TTerrType *TerrList = &Course.TerrList[0];

    surf_y = Course.FindYCoord (pos.x, pos.z);

	int id = Course.GetTerrainIdx (pos.x, pos.z, 0.5);
	if (id >= 0 && TerrList[id].particles && pos.y < surf_y) {
		TVector3 xvec = CrossProduct (ctrl->cdirection, ctrl->plane_nml);

		TVector3 right_part_pt = AddVectors (
		    pos,
		    ScaleVector (TUX_WIDTH/2.0, xvec));

		TVector3 left_part_pt = AddVectors (
		    pos,
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
		TVector3 left_part_vel = TransformVector (rot_mat, ctrl->plane_nml);
		left_part_vel = ScaleVector (MIN (MAX_PARTICLE_SPEED,
			speed * PARTICLE_SPEED_MULTIPLIER),left_part_vel);

		RotateAboutVectorMatrix(
		    rot_mat, ctrl->cdirection,
		    MIN (MAX_PARTICLE_ANGLE,
			 MAX_PARTICLE_ANGLE * speed / MAX_PARTICLE_ANGLE_SPEED));
		TVector3 right_part_vel = TransformVector (rot_mat, ctrl->plane_nml);
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

static CFlakes Flakes;


void TFlake::Draw(const TPlane& lp, const TPlane& rp, bool rotate_flake, float dir_angle) const {
	if ((DistanceToPlane (lp, pt) < 0) && (DistanceToPlane (rp, pt) < 0)) {
		glPushMatrix();
		glTranslatef (pt.x, pt.y, pt.z);
		if (rotate_flake) glRotatef (dir_angle, 0, 1, 0);
		glBegin (GL_QUADS);
			glTexCoord2f (tex_min.x, tex_min.y);
			glVertex3f (0, 0, 0);
			glTexCoord2f (tex_max.x, tex_min.y);
			glVertex3f (size, 0, 0);
			glTexCoord2f (tex_max.x, tex_max.y);
			glVertex3f (size, size, 0);
			glTexCoord2f (tex_min.x, tex_max.y);
			glVertex3f (0, size, 0);
		glEnd();
		glPopMatrix();
	}
}


TFlakeArea::TFlakeArea (
		int   num_flakes,
		float _xrange,
		float _ytop,
		float _yrange,
		float _zback,
		float _zrange,
		float _minSize,
		float _maxSize,
		float _speed,
		bool  rotate) {
	xrange = _xrange;
	ytop = _ytop;
	yrange = _yrange;
	zback = _zback;
	zrange = _zrange;
	minSize = _minSize;
	maxSize = _maxSize;
	speed = _speed;
	rotate_flake = rotate;

	flakes.resize(num_flakes);
}

void TFlakeArea::Draw (const CControl *ctrl) const {
	if (g_game.snow_id < 1) return;

	const TPlane& lp = get_left_clip_plane ();
	const TPlane& rp = get_right_clip_plane ();
	float dir_angle (atan (ctrl->viewdir.x / ctrl->viewdir.z) * 180 / 3.14159);

	ScopedRenderMode rm(PARTICLES);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	Tex.BindTex (T_WIDGETS);
	const TColor& particle_colour = Env.ParticleColor ();
    glColor4f (particle_colour.r, particle_colour.g, particle_colour.b, particle_colour.a);

	for (size_t i=0; i < flakes.size(); i++) {
		flakes[i].Draw(lp, rp, rotate_flake, dir_angle);
	}
}

void TFlakeArea::Update(float timestep, float xcoeff, float ycoeff, float zcoeff) {
	for (size_t i=0; i<flakes.size(); i++) {
		flakes[i].pt.x += xcoeff;
		flakes[i].pt.y += flakes[i].vel.y * timestep + ycoeff;
		flakes[i].pt.z += zcoeff;

		if (flakes[i].pt.y < bottom) {
			flakes[i].pt.y += yrange;
		} else if (flakes[i].pt.x < left) {
			flakes[i].pt.x += xrange;
		} else if (flakes[i].pt.x > right) {
			flakes[i].pt.x -= xrange;
		} else if (flakes[i].pt.y > top) {
			flakes[i].pt.y -= yrange;
		} else if (flakes[i].pt.z < front) {
			flakes[i].pt.z += zrange;
		} else if (flakes[i].pt.z > back) {
			flakes[i].pt.z -= zrange;
		}
	}
}

void CFlakes::Reset () {
	areas.clear();
}

void CFlakes::MakeSnowFlake (size_t ar, size_t i) {
	areas[ar].flakes[i].pt.x = XRandom (areas[ar].left, areas[ar].right);
	areas[ar].flakes[i].pt.y = -XRandom (areas[ar].top, areas[ar].bottom);
	areas[ar].flakes[i].pt.z = areas[ar].back - FRandom () * (areas[ar].back - areas[ar].front);

	float p_dist = FRandom ();
    areas[ar].flakes[i].size = XRandom (areas[ar].minSize, areas[ar].maxSize);
	areas[ar].flakes[i].vel.x = 0;
	areas[ar].flakes[i].vel.z = 0;
	areas[ar].flakes[i].vel.y = -areas[ar].flakes[i].size * areas[ar].speed;

	int type = (int) (FRandom () * 3.9999);

	if (type == 0) {
		areas[ar].flakes[i].tex_min = TVector2(0.0, 0.875);
		areas[ar].flakes[i].tex_max = TVector2(0.125, 1.0);
    } else if (type == 1) {
		areas[ar].flakes[i].tex_min = TVector2(0.125, 0.875);
		areas[ar].flakes[i].tex_max = TVector2(0.25, 1.0);
	} else if (type == 2) {
		areas[ar].flakes[i].tex_min = TVector2(0.0, 0.75);
		areas[ar].flakes[i].tex_max = TVector2(0.125, 0.875);
    } else {
		areas[ar].flakes[i].tex_min = TVector2(0.125, 0.75);
		areas[ar].flakes[i].tex_max = TVector2(0.25, 0.875);
    }
}

void CFlakes::GenerateSnowFlakes (const CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	snow_lastpos = ctrl->cpos;
	for (size_t ar=0; ar<areas.size(); ar++) {
		for (size_t i=0; i<areas[ar].flakes.size(); i++) MakeSnowFlake (ar, i);
	}
}

void CFlakes::UpdateAreas (const CControl *ctrl) {
	for (size_t ar=0; ar<areas.size(); ar++) {
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

void CFlakes::Init (int grade, const CControl *ctrl) {
	Reset ();
	switch (grade) {
		case 1:
//			areas.push_back(TFlakeArea(400, 5, 4, 4,     -2, 4, 0.01, 0.02,    5, true));
//			areas.push_back(TFlakeArea(400, 12, 5, 8,      2, 8, 0.03, 0.045,    5, false));
//			areas.push_back(TFlakeArea(400, 30, 6, 15,      10, 15, 0.06, 0.12,    5, false));
			areas.push_back(TFlakeArea(400, 5, 4, 4,     -2, 4, 0.015, 0.03,    5, true));
			areas.push_back(TFlakeArea(400, 12, 5, 8,      2, 8, 0.045, 0.07,    5, false));
			areas.push_back(TFlakeArea(400, 30, 6, 15,      10, 15, 0.09, 0.18,    5, false));
//			areas.push_back(TFlakeArea(400, 5, 4, 4,     -2, 4, 0.02, 0.04,    5, true));
//			areas.push_back(TFlakeArea(400, 12, 5, 8,      2, 8, 0.06, 0.09,    5, false));
//			areas.push_back(TFlakeArea(400, 30, 6, 15,      10, 15, 0.15, 0.25,    5, false));
			break;
		case 2:
//			areas.push_back(TFlakeArea(500, 5, 4, 4,     -2, 4, 0.02, 0.03,    5, true));
//			areas.push_back(TFlakeArea(500, 12, 5, 8,      2, 8, 0.045, 0.07,    5, false));
//			areas.push_back(TFlakeArea(500, 30, 6, 15,      10, 15, 0.1, 0.15,    5, false));
			areas.push_back(TFlakeArea(500, 5, 4, 4,     -2, 4, 0.03, 0.045,    5, true));
			areas.push_back(TFlakeArea(500, 12, 5, 8,      2, 8, 0.07, 0.1,    5, false));
			areas.push_back(TFlakeArea(500, 30, 6, 15,      10, 15, 0.15, 0.22,    5, false));
//			areas.push_back(TFlakeArea(500, 5, 4, 4,     -2, 4, 0.04, 0.06,    5, true));
//			areas.push_back(TFlakeArea(500, 12, 5, 8,      2, 8, 0.09, 0.15,    5, false));
//			areas.push_back(TFlakeArea(500, 30, 6, 15,      10, 15, 0.2, 0.32,    5, false));
			break;
		case 3:
//			areas.push_back(TFlakeArea(1000, 5, 4, 4,     -2, 4, 0.025, 0.04,    5, true));
//			areas.push_back(TFlakeArea(1000, 12, 5, 9,      2, 8, 0.06, 0.10,    5, false));
//			areas.push_back(TFlakeArea(1000, 30, 6, 15,      10, 15, 0.12, 0.2,    5, false));
			areas.push_back(TFlakeArea(1000, 5, 4, 4,     -2, 4, 0.037, 0.05,    5, true));
			areas.push_back(TFlakeArea(1000, 12, 5, 9,      2, 8, 0.09, 0.15,    5, false));
			areas.push_back(TFlakeArea(1000, 30, 6, 15,      10, 15, 0.18, 0.35,    5, false));
//			areas.push_back(TFlakeArea(800, 5, 4, 4,     -2, 4, 0.05, 0.08,    5, true));
//			areas.push_back(TFlakeArea(800, 12, 5, 9,      2, 8, 0.12, 0.20,    5, false));
//			areas.push_back(TFlakeArea(800, 30, 6, 15,      10, 15, 0.25, 0.5,    5, false));
			break;
		default: break;
	}

	UpdateAreas (ctrl);
	GenerateSnowFlakes (ctrl);
}

void CFlakes::Update (double timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1)
		return;

	UpdateAreas (ctrl);

	float zdiff = ctrl->cpos.z - snow_lastpos.z;
	float ydiff = 0.f;
	if (State::manager.CurrentState() != &GameOver) {
		ydiff = ctrl->cpos.y - snow_lastpos.y;
	}

	TVector3 winddrift = ScaleVector (SNOW_WIND_DRIFT, Wind.WindDrift ());
	float xcoeff = winddrift.x * timestep;
	float ycoeff = (ydiff * YDRIFT) + (winddrift.z * timestep);
	float zcoeff = (zdiff * ZDRIFT) + (winddrift.z * timestep);

	for (size_t ar=0; ar<areas.size(); ar++) {
		areas[ar].Update(timestep, xcoeff, ycoeff, zcoeff);
	}
	snow_lastpos = ctrl->cpos;
}

void CFlakes::Draw (const CControl *ctrl) const {
	for (size_t ar=0; ar<areas.size(); ar++)
		areas[ar].Draw(ctrl);
}

// --------------------------------------------------------------------
//					snow curtains
// --------------------------------------------------------------------

#define NUM_CHANGES 6
#define CHANGE_DRIFT 15
#define CHANGE_SPEED 0.05
#define CURTAIN_WINDDRIFT 0.35

struct TChange {
	float min;
	float max;
	float curr;
	float step;
	bool forward;
};

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
	for (int i=0; i<NUM_CHANGES; i++) {
		TChange* ch = &changes[i];
		if (ch->forward) {
			ch->curr += ch->step * timestep;
			if (ch->curr > ch->max) ch->forward = false;
		} else {
			ch->curr -= ch->step * timestep;
			if (ch->curr < ch->min) ch->forward = true;
		}
	}
}

TCurtain::TCurtain (int num_rows, float z_dist, float tex_size,
		float base_speed, float start_angle, float min_height, int dense) {
	numRows = num_rows;
	zdist = z_dist;
	size = tex_size;
	speed = base_speed;
	startangle = start_angle;
	minheight = min_height;
	switch (dense) {
		case 1: texture = T_SNOW1; break;
		case 2: texture = T_SNOW2; break;
		case 3: texture = T_SNOW3; break;
	}

	angledist = atan (size / 2 / zdist) * 360 / 3.14159;
	numCols = (unsigned int)(-2 * startangle / angledist) + 1;
	if (numCols > MAX_CURTAIN_COLS) numCols = MAX_CURTAIN_COLS;
	lastangle = startangle + (numCols-1) * angledist;

	for (unsigned int i=0; i<numRows; i++)
		chg[i] = IRandom (0, 5);
}

void TCurtain::SetStartParams(const CControl* ctrl) {
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			TCurtainElement* curt = &curtains[co][row];
			curt->height = minheight + row * size;
			float x, z;
			curt->angle = co * angledist + startangle;
			CurtainVec (curt->angle, zdist, x, z);
			curt->pt.x = ctrl->cpos.x + x;
			curt->pt.z = ctrl->cpos.z + z;
			curt->pt.y = ctrl->cpos.y + curt->height;
		}
	}
}

void TCurtain::Draw() const {
	Tex.BindTex (texture);
	float halfsize = size / 2;
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			const TVector3* pt = &curtains[co][row].pt;
			glPushMatrix();
			glTranslatef (pt->x, pt->y, pt->z);
			glRotatef (-curtains[co][row].angle, 0, 1, 0);
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

void TCurtain::Update(float timestep, const TVector3& drift, const CControl* ctrl) {
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			TCurtainElement* curt = &curtains[co][row];

			curt->angle += changes[chg[row]].curr * timestep * CHANGE_DRIFT;
			curt->angle += drift.x * timestep * CURTAIN_WINDDRIFT;
			curt->height -= speed * timestep;

			if (curt->angle > lastangle + angledist) curt->angle = startangle;
			if (curt->angle < startangle - angledist) curt->angle = lastangle;
			float x, z;
			CurtainVec (curt->angle, zdist, x, z);
			curt->pt.x = ctrl->cpos.x + x;
			curt->pt.z = ctrl->cpos.z + z;
			curt->pt.y = ctrl->cpos.y + curt->height;
			if (curt->height < minheight - size) curt->height += numRows * size;
		}
	}
}


static CCurtain Curtain;
void TCurtain::CurtainVec (float angle, float zdist, float &x, float &z) {
	x = zdist  * sin (angle * 3.14159 / 180);
	if (angle > 90 || angle < -90) z = sqrt (zdist * zdist - x * x);
	else z = -sqrt (zdist * zdist - x * x);
}

void CCurtain::Draw () {
	if (g_game.snow_id < 1) return;

	ScopedRenderMode rm(PARTICLES);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	const TColor& particle_colour = Env.ParticleColor ();
	glColor4f (particle_colour.r, particle_colour.g, particle_colour.b, 1.0);

	// glEnable (GL_NORMALIZE);
	for (size_t i=0; i<curtains.size(); i++) {
		curtains[i].Draw();
	}
}

void CCurtain::Update (float timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	const TVector3& drift = Wind.WindDrift ();

	UpdateChanges (timestep);
	for (size_t i=0; i<curtains.size(); i++) {
		curtains[i].Update(timestep, drift, ctrl);
	}
	Draw ();
}

void CCurtain::Reset () {
	curtains.clear();
}

void CCurtain::SetStartParams (const CControl *ctrl) {
	for (size_t i=0; i<curtains.size(); i++) {
		curtains[i].SetStartParams(ctrl);
	}
}

void CCurtain::Init (const CControl *ctrl) {
	Reset ();
	InitChanges ();
	switch (g_game.snow_id) {
	case 1:
//		curtains.push_back(TCurtain(3, 60, 10,       3, -100, -10, 1));
//		curtains.push_back(TCurtain(3, 50, 13,       3, -100, -10, 1));
//		curtains.push_back(TCurtain(3, 40, 16,       3, -100, -10, 1));
		curtains.push_back(TCurtain(3, 60, 15,       3, -100, -10, 1));
		curtains.push_back(TCurtain(3, 50, 19,       3, -100, -10, 1));
		curtains.push_back(TCurtain(3, 40, 23,       3, -100, -10, 1));
//		curtains.push_back(TCurtain(3, 60, 20,       3, -100, -10, 1));
//		curtains.push_back(TCurtain(3, 50, 25,       3, -100, -10, 1));
//		curtains.push_back(TCurtain(3, 40, 30,       3, -100, -10, 1));
		break;
	case 2:
//		curtains.push_back(TCurtain(3, 60, 15,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 50, 17,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 40, 20,       3, -100, -10, 2));
		curtains.push_back(TCurtain(3, 60, 22,       3, -100, -10, 2));
		curtains.push_back(TCurtain(3, 50, 25,       3, -100, -10, 2));
		curtains.push_back(TCurtain(3, 40, 30,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 60, 30,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 50, 35,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 40, 40,       3, -100, -10, 2));
		break;
	case 3:
//		curtains.push_back(TCurtain(3, 60, 20,       3, -100, -10, 3));
//		curtains.push_back(TCurtain(3, 50, 25,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 40, 30,       3, -100, -10, 2));
		curtains.push_back(TCurtain(3, 60, 22,       3, -100, -10, 3));
		curtains.push_back(TCurtain(3, 50, 27,       3, -100, -10, 2));
		curtains.push_back(TCurtain(3, 40, 32,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 60, 25,       3, -100, -10, 3));
//		curtains.push_back(TCurtain(3, 50, 30,       3, -100, -10, 2));
//		curtains.push_back(TCurtain(3, 40, 35,       3, -100, -10, 2));
		break;
	default:
		break;
	}
	SetStartParams (ctrl);
}

// --------------------------------------------------------------------
//					wind
// --------------------------------------------------------------------

#define UPDATE_TIME 0.04

CWind Wind;

CWind::CWind ()
	: WVector(0, 0, 0)
{
	windy = false;
	CurrTime = 0.0;

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
		WVector = TVector3 (0, 0, 0);
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

void InitSnow (const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Init (g_game.snow_id, ctrl);
	Curtain.Init (ctrl);
}

 void UpdateSnow (double timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Update (timestep, ctrl);
	Curtain.Update (timestep, ctrl);
}

void DrawSnow (const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Draw (ctrl);
	Curtain.Draw ();
}

void InitWind () {
	Wind.Init (g_game.wind_id);
}

void UpdateWind (double timestep) {
	Wind.Update (timestep);
}
