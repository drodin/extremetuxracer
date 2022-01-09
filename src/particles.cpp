/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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
#include <cstdlib>
#include <list>
#include <algorithm>

// ====================================================================
//					gui particles 2D
// ====================================================================

#define MAX_num_snowparticles 4000
#define BASE_snowparticles 1000.0/1024 // This is intentionally not divided by height*width to make particle count increasing slower than screen size
#define GRAVITY_FACTOR 0.015
#define BASE_VELOCITY 0.05
#define VELOCITY_RANGE 0.02
#define PUSH_DECAY_TIME_CONSTANT 0.2
#define PUSH_DIST_DECAY 100
#define PUSH_FACTOR 0.5
#define MAX_PUSH_FORCE 5.0
#define AIR_DRAG 0.4
#define TUX_WIDTH 0.45

#define PARTICLE_MIN_SIZE 1
#define PARTICLE_SIZE_RANGE 10

struct TGuiParticle {
	sf::Sprite sprite;
	float size;
	TVector2d vel;

	TGuiParticle(float x, float y);
	void Draw() const;
	void Update(float time_step, float push_timestep, const TVector2d& push_vector);
};

static std::list<TGuiParticle> particles_2d;
static TVector2d push_position(0, 0);
static TVector2d last_push_position;
static bool push_position_initialized = false;

TGuiParticle::TGuiParticle(float x, float y) {
	const sf::Texture& texture = Tex.GetSFTexture(SNOW_PART);
	sprite.setTexture(texture);
	sprite.setPosition(x*static_cast<float>(Winsys.resolution.width), y*static_cast<float>(Winsys.resolution.height));
	sprite.setColor(sf::Color(255, 255, 255, 76));
	double p_dist = FRandom();

	size = PARTICLE_MIN_SIZE + (1.0 - p_dist) * PARTICLE_SIZE_RANGE;

	sprite.setScale(size / (texture.getSize().x / 2), size / (texture.getSize().y / 2));
	vel.x = 0;
	vel.y = BASE_VELOCITY + p_dist * VELOCITY_RANGE;

	int type = std::rand() % 4;
	switch (type) {
		case 0:
			sprite.setTextureRect(sf::IntRect(0, 0, texture.getSize().x / 2, texture.getSize().y / 2));
			break;
		case 1:
			sprite.setTextureRect(sf::IntRect(texture.getSize().x / 2, 0, texture.getSize().x / 2, texture.getSize().y / 2));
			break;
		case 2:
			sprite.setTextureRect(sf::IntRect(texture.getSize().x / 2, texture.getSize().y / 2, texture.getSize().x / 2, texture.getSize().y / 2));
			break;
		case 3:
			sprite.setTextureRect(sf::IntRect(0, texture.getSize().y / 2, texture.getSize().x / 2, texture.getSize().y / 2));
			break;
	}
}

void TGuiParticle::Draw() const {
	Winsys.draw(sprite);
}

void TGuiParticle::Update(float time_step, float push_timestep, const TVector2d& push_vector) {
	TVector2d f;

	float x = sprite.getPosition().x / static_cast<float>(Winsys.resolution.width);
	float y = sprite.getPosition().y / static_cast<float>(Winsys.resolution.height);
	float dist_from_push = (std::pow((x - push_position.x), 2) +
	                        std::pow((y - push_position.y), 2));
	if (push_timestep > 0) {
		f = PUSH_FACTOR / push_timestep * push_vector;
		f.x = clamp(-MAX_PUSH_FORCE, f.x, MAX_PUSH_FORCE);
		f.y = clamp(-MAX_PUSH_FORCE, f.y, MAX_PUSH_FORCE);
		f *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) *
		     size/PARTICLE_SIZE_RANGE;
	}

	vel.x += (f.x - vel.x * AIR_DRAG) *  time_step;
	vel.y += (f.y + GRAVITY_FACTOR - vel.y * AIR_DRAG) * time_step;

	x += vel.x * time_step * (size / PARTICLE_SIZE_RANGE);
	y += vel.y * time_step * (size / PARTICLE_SIZE_RANGE);

	x = clamp(-0.05f, x, 1.05f);
	sprite.setPosition(x*Winsys.resolution.width, y*Winsys.resolution.height);
}

void init_ui_snow() {
	particles_2d.clear();
	for (int i = 0; i < BASE_snowparticles * Winsys.resolution.width; i++)
		particles_2d.emplace_back(static_cast<float>(FRandom()), static_cast<float>(FRandom()));
	push_position = TVector2d(0.0, 0.0);
}

void update_ui_snow(float time_step) {
	static sf::Clock timer;
	float time = timer.getElapsedTime().asSeconds();
	timer.restart();

	TVector2d push_vector;
	float push_timestep = 0;

	if (push_position_initialized) {
		push_vector = push_position - last_push_position;
		push_timestep = time;
	}
	last_push_position = push_position;

	for (std::list<TGuiParticle>::iterator p = particles_2d.begin(); p != particles_2d.end(); ++p) {
		p->Update(time_step, push_timestep, push_vector);
	}

	if (FRandom() < time_step*20.f*(MAX_num_snowparticles - particles_2d.size()) / 1000.f) {
		particles_2d.emplace_back(static_cast<float>(FRandom()), -0.05f);
	}

	for (std::list<TGuiParticle>::iterator p = particles_2d.begin(); p != particles_2d.end();) {
		if (p->sprite.getPosition().y / static_cast<float>(Winsys.resolution.height) > 1.05) {
			if (particles_2d.size() > BASE_snowparticles * Winsys.resolution.width && FRandom() > 0.2) {
				p = particles_2d.erase(p);
			} else {
				p->sprite.setPosition(static_cast<float>(Winsys.resolution.width)*FRandom(), static_cast<float>(Winsys.resolution.height) * (-FRandom()*BASE_VELOCITY));
				double p_dist = FRandom();
				p->size = PARTICLE_MIN_SIZE + (1.f - p_dist) * PARTICLE_SIZE_RANGE;
				p->sprite.setScale(p->size / (p->sprite.getTexture()->getSize().x / 2), p->size / (p->sprite.getTexture()->getSize().x / 2));
				p->vel.x = 0;
				p->vel.y = BASE_VELOCITY + p_dist*VELOCITY_RANGE;
				++p;
			}
		} else
			++p;
	}

	if (time_step < PUSH_DECAY_TIME_CONSTANT) {
		push_vector *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
	} else {
		push_vector.x = 0.0;
		push_vector.y = 0.0;
	}
}
void draw_ui_snow() {
	for (std::list<TGuiParticle>::const_iterator i = particles_2d.begin(); i != particles_2d.end(); ++i) {
		i->Draw();
	}
}

void push_ui_snow(const TVector2i& pos) {
	push_position = TVector2d((pos.x) / static_cast<float>(Winsys.resolution.width), (pos.y) / static_cast<float>(Winsys.resolution.height));
	if (!push_position_initialized) last_push_position = push_position;
	push_position_initialized = true;
}

// ====================================================================
//						tux particles
// ====================================================================

#define MAX_PARTICLES 10000
#define START_RADIUS 0.04
#define OLD_PART_SIZE 0.12	// orig 0.07
#define NEW_PART_SIZE 0.035	// orig 0.02
#define MIN_AGE -0.2
#define MAX_AGE 1.0
#define VARIANCE_FACTOR 0.8
#define PARTICLE_SHADOW_HEIGHT 0.05
#define PARTICLE_SHADOW_ALPHA 0.1

#define MAX_TURN_PARTICLES 500
#define BRAKE_PARTICLES 2000
#define MAX_ROLL_PARTICLES 3000
#define PARTICLE_SPEED_FACTOR 40
#define MAX_PARTICLE_ANGLE 80.0
#define MAX_PARTICLE_ANGLE_SPEED 50
#define PARTICLE_SPEED_MULTIPLIER 0.3
#define MAX_PARTICLE_SPEED 2.0


struct Particle {
	TVector3d pt;
	short type;
	double base_size;
	double cur_size;
	double terrain_height;
	double age;
	double death;
	double alpha;
	TVector3d vel;

	void Draw(const CControl* ctrl) const;
private:
	void draw_billboard(const CControl *ctrl, double width, double height, bool use_world_y_axis, const GLfloat* tex) const;
};

static std::list<Particle> particles;

void Particle::Draw(const CControl* ctrl) const {
	static const GLfloat tex_coords[4][8] = {
		{
			0.0, 0.5,
			0.5, 0.5,
			0.5, 0.0,
			0.0, 0.0
		}, {
			0.5, 0.5,
			1.0, 0.5,
			1.0, 0.0,
			0.5, 0.0
		}, {
			0.0, 1.0,
			0.5, 1.0,
			0.5, 0.5,
			0.0, 0.5
		}, {
			0.5, 1.0,
			1.0, 1.0,
			1.0, 0.5,
			0.5, 0.5
		}
	};

	const sf::Color& particle_colour = Env.ParticleColor();
	glColor(particle_colour, particle_colour.a * alpha);

	draw_billboard(ctrl, cur_size, cur_size, false, tex_coords[type]);
}

void Particle::draw_billboard(const CControl *ctrl, double width, double height, bool use_world_y_axis, const GLfloat* tex) const {
	TVector3d x_vec;
	TVector3d y_vec;
	TVector3d z_vec;

	x_vec.x = ctrl->view_mat[0][0];
	x_vec.y = ctrl->view_mat[0][1];
	x_vec.z = ctrl->view_mat[0][2];

	if (use_world_y_axis) {
		y_vec = TVector3d(0, 1, 0);
		x_vec = ProjectToPlane(y_vec, x_vec);
		x_vec.Norm();
		z_vec = CrossProduct(x_vec, y_vec);
	} else {
		y_vec.x = ctrl->view_mat[1][0];
		y_vec.y = ctrl->view_mat[1][1];
		y_vec.z = ctrl->view_mat[1][2];
		z_vec.x = ctrl->view_mat[2][0];
		z_vec.y = ctrl->view_mat[2][1];
		z_vec.z = ctrl->view_mat[2][2];
	}

	TVector3d pt1 = pt - width/2.0 * x_vec - height/2.0 * y_vec;
	TVector3d pt2 = pt1 + width * x_vec;
	TVector3d pt3 = pt2 + height * y_vec;
	TVector3d pt4 = pt3 + -width * x_vec;
	const GLfloat vtx[] = {
		static_cast<GLfloat>(pt1.x),
		static_cast<GLfloat>(pt1.y),
		static_cast<GLfloat>(pt1.z),
		static_cast<GLfloat>(pt2.x),
		static_cast<GLfloat>(pt2.y),
		static_cast<GLfloat>(pt2.z),
		static_cast<GLfloat>(pt3.x),
		static_cast<GLfloat>(pt3.y),
		static_cast<GLfloat>(pt3.z),
		static_cast<GLfloat>(pt4.x),
		static_cast<GLfloat>(pt4.y),
		static_cast<GLfloat>(pt4.z),
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void create_new_particles(const TVector3d& loc, const TVector3d& vel, std::size_t num) {
	double speed = vel.Length();

	if (particles.size() + num > MAX_PARTICLES) {
		Message("maximum number of particles exceeded");
	}
	for (std::size_t i=0; i<num; i++) {
		particles.emplace_back();
		Particle* newp = &particles.back();
		newp->pt.x = loc.x + 2.*(FRandom() - 0.5) * START_RADIUS;
		newp->pt.y = loc.y;
		newp->pt.z = loc.z + 2.*(FRandom() - 0.5) * START_RADIUS;
		newp->type = std::rand() % 4;
		newp->base_size = (FRandom() + 0.5) * OLD_PART_SIZE;
		newp->cur_size = NEW_PART_SIZE;
		newp->age = FRandom() * MIN_AGE;
		newp->death = FRandom() * MAX_AGE;
		newp->vel = vel +
		            TVector3d(VARIANCE_FACTOR * (FRandom() - 0.5) * speed,
		                      VARIANCE_FACTOR * (FRandom() - 0.5) * speed,
		                      VARIANCE_FACTOR * (FRandom() - 0.5) * speed);
	}
}
void update_particles(float time_step) {
	for (std::list<Particle>::iterator p = particles.begin(); p != particles.end();) {
		p->age += time_step;
		if (p->age < 0) {
			++p;
			continue;
		}

		p->pt += static_cast<double>(time_step) * p->vel;
		double ycoord = Course.FindYCoord(p->pt.x, p->pt.z);
		if (p->pt.y < ycoord - 3)
			p->age = p->death + 1;
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
void draw_particles(const CControl *ctrl) {
	if (particles.empty())
		return;

	ScopedRenderMode rm(PARTICLES);
	Tex.BindTex(SNOW_PART);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1.f, 1.f, 1.f, 0.8f);

	for (std::list<Particle>::const_iterator p = particles.begin(); p != particles.end(); ++p) {
		if (p->age >= 0)
			p->Draw(ctrl);
	}
}
void clear_particles() {
	particles.clear();
}

static double adjust_particle_count(double count) {
	if (count < 1) {
		if (((double)std::rand()) / RAND_MAX < count) return 1.0;
		else return 0.0;
	} else return count;
}

void generate_particles(const CControl *ctrl, double dtime, const TVector3d& pos, double speed) {
	double surf_y = Course.FindYCoord(pos.x, pos.z);

	int id = Course.GetTerrainIdx(pos.x, pos.z, 0.5);
	if (id >= 0 && Course.TerrList[id].particles && pos.y < surf_y) {
		TVector3d xvec = CrossProduct(ctrl->cdirection, ctrl->plane_nml);

		TVector3d right_part_pt = pos + TUX_WIDTH/2.0 * xvec;

		TVector3d left_part_pt = pos + -TUX_WIDTH/2.0 * xvec;

		right_part_pt.y = left_part_pt.y  = surf_y;

		double brake_particles = dtime *
		                         BRAKE_PARTICLES * (ctrl->is_braking ? 1.0 : 0.0)
		                         * std::min(speed / PARTICLE_SPEED_FACTOR, 1.0);
		double turn_particles = dtime * MAX_TURN_PARTICLES
		                        * std::min(speed / PARTICLE_SPEED_FACTOR, 1.0);
		double roll_particles = dtime * MAX_ROLL_PARTICLES
		                        * std::min(speed / PARTICLE_SPEED_FACTOR, 1.0);

		double left_particles = turn_particles *
		                        std::fabs(std::min(ctrl->turn_fact, 0.)) +
		                        brake_particles +
		                        roll_particles * std::fabs(std::min(ctrl->turn_animation, 0.));

		double right_particles = turn_particles *
		                         std::fabs(std::max(ctrl->turn_fact, 0.)) +
		                         brake_particles +
		                         roll_particles * std::fabs(std::max(ctrl->turn_animation, 0.));

		left_particles = adjust_particle_count(left_particles);
		right_particles = adjust_particle_count(right_particles);

		TMatrix<4, 4> rot_mat = RotateAboutVectorMatrix(
		                            ctrl->cdirection,
		                            std::max(-MAX_PARTICLE_ANGLE,
		                                     -MAX_PARTICLE_ANGLE * speed / MAX_PARTICLE_ANGLE_SPEED));
		TVector3d left_part_vel = TransformVector(rot_mat, ctrl->plane_nml);
		left_part_vel *= std::min(MAX_PARTICLE_SPEED, speed * PARTICLE_SPEED_MULTIPLIER);

		rot_mat = RotateAboutVectorMatrix(
		              ctrl->cdirection,
		              std::min(MAX_PARTICLE_ANGLE,
		                       MAX_PARTICLE_ANGLE * speed / MAX_PARTICLE_ANGLE_SPEED));
		TVector3d right_part_vel = TransformVector(rot_mat, ctrl->plane_nml);
		right_part_vel *= std::min(MAX_PARTICLE_SPEED, speed * PARTICLE_SPEED_MULTIPLIER);


		create_new_particles(left_part_pt, left_part_vel, (std::size_t)left_particles);
		create_new_particles(right_part_pt, right_part_vel, (std::size_t)right_particles);
	}
}

// --------------------------------------------------------------------
//					snow flakes
// --------------------------------------------------------------------

#define SNOW_WIND_DRIFT  0.1

static CFlakes Flakes;


void TFlake::Draw(const TPlane& lp, const TPlane& rp, bool rotate_flake, float dir_angle) const {
	if ((DistanceToPlane(lp, pt) < 0) && (DistanceToPlane(rp, pt) < 0)) {
		glPushMatrix();
		glTranslate(pt);
		if (rotate_flake) glRotatef(dir_angle, 0, 1, 0);

		const GLfloat vtx[] = {
			0,    0,    0,
			size, 0,    0,
			size, size, 0,
			0,    size, 0
		};
		glVertexPointer(3, GL_FLOAT, 0, vtx);
		glTexCoordPointer(2, GL_FLOAT, 0, tex);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glPopMatrix();
	}
}


TFlakeArea::TFlakeArea(
    std::size_t num_flakes,
    float xrange_,
    float ytop_,
    float yrange_,
    float zback_,
    float zrange_,
    float minSize_,
    float maxSize_,
    float speed_,
    bool  rotate)
	: flakes(num_flakes) {
	xrange = xrange_;
	ytop = ytop_;
	yrange = yrange_;
	zback = zback_;
	zrange = zrange_;
	minSize = minSize_;
	maxSize = maxSize_;
	speed = speed_;
	rotate_flake = rotate;
	left = right = bottom = top = front = back = 0.f;
}

void TFlakeArea::Draw(const CControl *ctrl) const {
	if (g_game.snow_id < 1 || flakes.empty())
		return;

	const TPlane& lp = get_left_clip_plane();
	const TPlane& rp = get_right_clip_plane();
	float dir_angle(std::atan(ctrl->viewdir.x / ctrl->viewdir.z) * 180 / M_PI);

	ScopedRenderMode rm(PARTICLES);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	Tex.BindTex(SNOW_PART);
	const sf::Color& particle_colour = Env.ParticleColor();
	glColor(particle_colour);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (std::size_t i=0; i < flakes.size(); i++) {
		flakes[i].Draw(lp, rp, rotate_flake, dir_angle);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void TFlakeArea::Update(float timestep, float xcoeff, float ycoeff, float zcoeff) {
	for (std::size_t i=0; i<flakes.size(); i++) {
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

void CFlakes::Reset() {
	areas.clear();
}

void CFlakes::MakeSnowFlake(std::size_t ar, std::size_t i) {
	areas[ar].flakes[i].pt.x = XRandom(areas[ar].left, areas[ar].right);
	areas[ar].flakes[i].pt.y = -XRandom(areas[ar].top, areas[ar].bottom);
	areas[ar].flakes[i].pt.z = areas[ar].back - FRandom() * (areas[ar].back - areas[ar].front);

	areas[ar].flakes[i].size = XRandom(areas[ar].minSize, areas[ar].maxSize);
	areas[ar].flakes[i].vel.x = 0;
	areas[ar].flakes[i].vel.z = 0;
	areas[ar].flakes[i].vel.y = -areas[ar].flakes[i].size * areas[ar].speed;

	int type = std::rand() % 4;

	static const GLfloat tex_coords[4][8] = {
		{
			0.0, 0.5,
			0.5, 0.5,
			0.5, 0.0,
			0.0, 0.0
		}, {
			0.5, 0.5,
			1.0, 0.5,
			1.0, 0.0,
			0.5, 0.0
		}, {
			0.0, 1.0,
			0.5, 1.0,
			0.5, 0.5,
			0.0, 0.5
		}, {
			0.5, 1.0,
			1.0, 1.0,
			1.0, 0.5,
			0.5, 0.5
		}
	};

	areas[ar].flakes[i].tex = tex_coords[type];
}

void CFlakes::GenerateSnowFlakes(const CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	snow_lastpos = ctrl->cpos;
	for (std::size_t ar=0; ar<areas.size(); ar++) {
		for (std::size_t i=0; i<areas[ar].flakes.size(); i++) MakeSnowFlake(ar, i);
	}
}

void CFlakes::UpdateAreas(const CControl *ctrl) {
	for (std::size_t ar=0; ar<areas.size(); ar++) {
		areas[ar].left = ctrl->cpos.x - areas[ar].xrange / 2;
		areas[ar].right = areas[ar].left + areas[ar].xrange;
		areas[ar].back = ctrl->cpos.z - areas[ar].zback;
		areas[ar].front = areas[ar].back - areas[ar].zrange;
		areas[ar].top = ctrl->cpos.y + areas[ar].ytop;
		areas[ar].bottom = areas[ar].top - areas[ar].yrange;
	}
}

#define YDRIFT 0.8f
#define ZDRIFT 0.6f

void CFlakes::Init(int grade, const CControl *ctrl) {
	Reset();
	areas.reserve(3);
	switch (grade) {
		case 1:
//			areas.emplace_back(400, 5, 4, 4,     -2, 4, 0.01, 0.02,    5, true);
//			areas.emplace_back(400, 12, 5, 8,      2, 8, 0.03, 0.045,    5, false);
//			areas.emplace_back(400, 30, 6, 15,      10, 15, 0.06, 0.12,    5, false);
			areas.emplace_back(400, 5.f, 4.f, 4.f, -2.f, 4.f, 0.015f, 0.03f, 5.f, true);
			areas.emplace_back(400, 12.f, 5.f, 8.f, 2.f, 8.f, 0.045f, 0.07f, 5.f, false);
			areas.emplace_back(400, 30.f, 6.f, 15.f, 10.f, 15.f, 0.09f, 0.18f, 5.f, false);
//			areas.emplace_back(400, 5, 4, 4,     -2, 4, 0.02, 0.04,    5, true);
//			areas.emplace_back(400, 12, 5, 8,      2, 8, 0.06, 0.09,    5, false);
//			areas.emplace_back(400, 30, 6, 15,      10, 15, 0.15, 0.25,    5, false);
			break;
		case 2:
//			areas.emplace_back(500, 5, 4, 4,     -2, 4, 0.02, 0.03,    5, true);
//			areas.emplace_back(500, 12, 5, 8,      2, 8, 0.045, 0.07,    5, false);
//			areas.emplace_back(500, 30, 6, 15,      10, 15, 0.1, 0.15,    5, false);
			areas.emplace_back(500, 5.f, 4.f, 4.f, -2.f, 4.f, 0.03f, 0.045f, 5.f, true);
			areas.emplace_back(500, 12.f, 5.f, 8.f, 2.f, 8.f, 0.07f, 0.1f, 5.f, false);
			areas.emplace_back(500, 30.f, 6.f, 15.f, 10.f, 15.f, 0.15f, 0.22f, 5.f, false);
//			areas.emplace_back(500, 5, 4, 4,     -2, 4, 0.04, 0.06,    5, true);
//			areas.emplace_back(500, 12, 5, 8,      2, 8, 0.09, 0.15,    5, false);
//			areas.emplace_back(500, 30, 6, 15,      10, 15, 0.2, 0.32,    5, false);
			break;
		case 3:
//			areas.emplace_back(1000, 5, 4, 4,     -2, 4, 0.025, 0.04,    5, true);
//			areas.emplace_back(1000, 12, 5, 9,      2, 8, 0.06, 0.10,    5, false);
//			areas.emplace_back(1000, 30, 6, 15,      10, 15, 0.12, 0.2,    5, false);
			areas.emplace_back(1000, 5.f, 4.f, 4.f, -2.f, 4.f, 0.037f, 0.05f, 5.f, true);
			areas.emplace_back(1000, 12.f, 5.f, 9.f, 2.f, 8.f, 0.09f, 0.15f, 5.f, false);
			areas.emplace_back(1000, 30.f, 6.f, 15.f, 10.f, 15.f, 0.18f, 0.35f, 5.f, false);
//			areas.emplace_back(800, 5, 4, 4,     -2, 4, 0.05, 0.08,    5, true);
//			areas.emplace_back(800, 12, 5, 9,      2, 8, 0.12, 0.20,    5, false);
//			areas.emplace_back(800, 30, 6, 15,      10, 15, 0.25, 0.5,    5, false);
			break;
		default:
			break;
	}

	UpdateAreas(ctrl);
	GenerateSnowFlakes(ctrl);
}

void CFlakes::Update(float timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1)
		return;

	UpdateAreas(ctrl);

	float zdiff = ctrl->cpos.z - snow_lastpos.z;
	float ydiff = 0.f;
	if (State::manager.CurrentState() != &GameOver) {
		ydiff = ctrl->cpos.y - snow_lastpos.y;
	}

	TVector3d winddrift = SNOW_WIND_DRIFT * Wind.WindDrift();
	float xcoeff = winddrift.x * timestep;
	float ycoeff = (ydiff * YDRIFT) + (winddrift.z * timestep);
	float zcoeff = (zdiff * ZDRIFT) + (winddrift.z * timestep);

	for (std::size_t ar=0; ar<areas.size(); ar++) {
		areas[ar].Update(timestep, xcoeff, ycoeff, zcoeff);
	}
	snow_lastpos = ctrl->cpos;
}

void CFlakes::Draw(const CControl *ctrl) const {
	for (std::size_t ar=0; ar<areas.size(); ar++)
		areas[ar].Draw(ctrl);
}

// --------------------------------------------------------------------
//					snow curtains
// --------------------------------------------------------------------

#define NUM_CHANGES 6
#define CHANGE_DRIFT 15
#define CHANGE_SPEED 0.05f
#define CURTAIN_WINDDRIFT 0.35f

struct TChange {
	float min;
	float max;
	float curr;
	float step;
	bool forward;
};

TChange changes[NUM_CHANGES];

void InitChanges() {
	for (int i=0; i<NUM_CHANGES; i++) {
		changes[i].min = XRandom(-0.15, -0.05);
		changes[i].max = XRandom(0.05, 0.15);
		changes[i].curr = (changes[i].min + changes[i].max) / 2;
		changes[i].step = CHANGE_SPEED;
		changes[i].forward = true;
	}
}

void UpdateChanges(float timestep) {
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

TCurtain::TCurtain(int num_rows, float z_dist, float tex_size,
                   float base_speed, float start_angle, float min_height, int dense) {
	numRows = num_rows;
	zdist = z_dist;
	size = tex_size;
	speed = base_speed;
	startangle = start_angle;
	minheight = min_height;
	switch (dense) {
		case 1:
			texture = T_SNOW1;
			break;
		case 2:
			texture = T_SNOW2;
			break;
		case 3:
			texture = T_SNOW3;
			break;
	}

	angledist = std::atan(size / 2 / zdist) * 360 / M_PI;
	numCols = (unsigned int)(-2 * startangle / angledist) + 1;
	if (numCols > MAX_CURTAIN_COLS) numCols = MAX_CURTAIN_COLS;
	lastangle = startangle + (numCols-1) * angledist;

	for (unsigned int i=0; i<numRows; i++)
		chg[i] = IRandom(0, 5);
}

void TCurtain::SetStartParams(const CControl* ctrl) {
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			TCurtainElement* curt = &curtains[co][row];
			curt->height = minheight + row * size;
			float x, z;
			curt->angle = co * angledist + startangle;
			CurtainVec(curt->angle, zdist, x, z);
			curt->pt.x = ctrl->cpos.x + x;
			curt->pt.z = ctrl->cpos.z + z;
			curt->pt.y = ctrl->cpos.y + curt->height;
		}
	}
}

void TCurtain::Draw() const {
	Tex.BindTex(texture);
	float halfsize = size / 2.f;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			const TVector3d& pt = curtains[co][row].pt;
			glPushMatrix();
			glTranslate(pt);
			glRotatef(-curtains[co][row].angle, 0, 1, 0);

			static const GLshort tex[] = {
				0, 1,
				1, 1,
				1, 0,
				0, 0
			};
			const GLfloat vtx[] = {
				-halfsize, -halfsize, 0,
				    halfsize, -halfsize, 0,
				    halfsize, halfsize, 0,
				    -halfsize, halfsize, 0
			    };
			glVertexPointer(3, GL_FLOAT, 0, vtx);
			glTexCoordPointer(2, GL_SHORT, 0, tex);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glPopMatrix();
		}
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TCurtain::Update(float timestep, const TVector3d& drift, const CControl* ctrl) {
	for (unsigned int co=0; co<numCols; co++) {
		for (unsigned int row=0; row<numRows; row++) {
			TCurtainElement* curt = &curtains[co][row];

			curt->angle += changes[chg[row]].curr * timestep * CHANGE_DRIFT;
			curt->angle += drift.x * timestep * CURTAIN_WINDDRIFT;
			curt->height -= speed * timestep;

			if (curt->angle > lastangle + angledist) curt->angle = startangle;
			if (curt->angle < startangle - angledist) curt->angle = lastangle;
			float x, z;
			CurtainVec(curt->angle, zdist, x, z);
			curt->pt.x = ctrl->cpos.x + x;
			curt->pt.z = ctrl->cpos.z + z;
			curt->pt.y = ctrl->cpos.y + curt->height;
			if (curt->height < minheight - size) curt->height += numRows * size;
		}
	}
}


static CCurtain Curtain;
void TCurtain::CurtainVec(float angle, float zdist, float &x, float &z) {
	x = zdist  * std::sin(angle * M_PI / 180);
	if (angle > 90 || angle < -90) z = std::sqrt(zdist * zdist - x * x);
	else z = -std::sqrt(zdist * zdist - x * x);
}

void CCurtain::Draw() {
	if (g_game.snow_id < 1) return;

	ScopedRenderMode rm(PARTICLES);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	const sf::Color& particle_colour = Env.ParticleColor();
	glColor(particle_colour, 255);

	// glEnable (GL_NORMALIZE);
	for (std::size_t i=0; i<curtains.size(); i++) {
		curtains[i].Draw();
	}
}

void CCurtain::Update(float timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1) return;
	const TVector3d& drift = Wind.WindDrift();

	UpdateChanges(timestep);
	for (std::size_t i=0; i<curtains.size(); i++) {
		curtains[i].Update(timestep, drift, ctrl);
	}
	Draw();
}

void CCurtain::Reset() {
	curtains.clear();
}

void CCurtain::SetStartParams(const CControl *ctrl) {
	for (std::size_t i=0; i<curtains.size(); i++) {
		curtains[i].SetStartParams(ctrl);
	}
}

void CCurtain::Init(const CControl *ctrl) {
	Reset();
	InitChanges();
	curtains.reserve(3);
	switch (g_game.snow_id) {
		case 1:
//			curtains.emplace_back(3, 60, 10,       3, -100, -10, 1);
//			curtains.emplace_back(3, 50, 13,       3, -100, -10, 1);
//			curtains.emplace_back(3, 40, 16,       3, -100, -10, 1);
			curtains.emplace_back(3, 60.f, 15.f, 3.f, -100.f, -10.f, 1);
			curtains.emplace_back(3, 50.f, 19.f, 3.f, -100.f, -10.f, 1);
			curtains.emplace_back(3, 40.f, 23.f, 3.f, -100.f, -10.f, 1);
//			curtains.emplace_back(3, 60, 20,       3, -100, -10, 1);
//			curtains.emplace_back(3, 50, 25,       3, -100, -10, 1);
//			curtains.emplace_back(3, 40, 30,       3, -100, -10, 1);
			break;
		case 2:
//			curtains.emplace_back(3, 60, 15,       3, -100, -10, 2);
//			curtains.emplace_back(3, 50, 17,       3, -100, -10, 2);
//			curtains.emplace_back(3, 40, 20,       3, -100, -10, 2);
			curtains.emplace_back(3, 60.f, 22.f, 3.f, -100.f, -10.f, 2);
			curtains.emplace_back(3, 50.f, 25.f, 3.f, -100.f, -10.f, 2);
			curtains.emplace_back(3, 40.f, 30.f, 3.f, -100.f, -10.f, 2);
//			curtains.emplace_back(3, 60, 30,       3, -100, -10, 2);
//			curtains.emplace_back(3, 50, 35,       3, -100, -10, 2);
//			curtains.emplace_back(3, 40, 40,       3, -100, -10, 2);
			break;
		case 3:
//			curtains.emplace_back(3, 60, 20,       3, -100, -10, 3);
//			curtains.emplace_back(3, 50, 25,       3, -100, -10, 2);
//			curtains.emplace_back(3, 40, 30,       3, -100, -10, 2);
			curtains.emplace_back(3, 60.f, 22.f, 3.f, -100.f, -10.f, 3);
			curtains.emplace_back(3, 50.f, 27.f, 3.f, -100.f, -10.f, 2);
			curtains.emplace_back(3, 40.f, 32.f, 3.f, -100.f, -10.f, 2);
//			curtains.emplace_back(3, 60, 25,       3, -100, -10, 3);
//			curtains.emplace_back(3, 50, 30,       3, -100, -10, 2);
//			curtains.emplace_back(3, 40, 35,       3, -100, -10, 2);
			break;
		default:
			break;
	}
	SetStartParams(ctrl);
}

// --------------------------------------------------------------------
//					wind
// --------------------------------------------------------------------

#define UPDATE_TIME 0.04f

CWind Wind;

CWind::CWind()
	: WVector(0, 0, 0) {
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

void CWind::SetParams(int grade) {
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
		params.minChange = 0.1f;
		params.maxChange = 0.3f;

		min_base_angle = 70;
		max_base_angle = 110;
		min_angle_var = 0;
		max_angle_var = 90;
		params.minAngleChange = 0.1f;
		params.maxAngleChange = 1.0f;

		params.topSpeed = 100;
		params.topProbability = 0;
		params.nullProbability = 6;
		alt_angle = 180;
	} else if (grade == 1) {
		min_base_speed = 30;
		max_base_speed = 60;
		min_speed_var = 40;
		max_speed_var = 40;
		params.minChange = 0.1f;
		params.maxChange = 0.5f;

		min_base_angle = 70;
		max_base_angle = 110;
		min_angle_var = 0;
		max_angle_var = 90;
		params.minAngleChange = 0.1f;
		params.maxAngleChange = 1.0f;

		params.topSpeed = 100;
		params.topProbability = 0;
		params.nullProbability = 10;
		alt_angle = 180;
	} else {
		min_base_speed = 40;
		max_base_speed = 80;
		min_speed_var = 30;
		max_speed_var = 60;
		params.minChange = 0.1f;
		params.maxChange = 1.0f;

		min_base_angle = 0;
		max_base_angle = 180;
		min_angle_var = 180;
		max_angle_var = 360;
		params.minAngleChange = 0.1f;
		params.maxAngleChange = 1.0f;

		params.topSpeed = 100;
		params.topProbability = 10;
		params.nullProbability = 10;
		alt_angle = 0;
	}

	float speed, var, angle;

	speed = XRandom(min_base_speed, max_base_speed);
	var = XRandom(min_speed_var, max_speed_var) / 2;
	params.minSpeed = speed - var;
	params.maxSpeed = speed + var;
	if (params.minSpeed < 0) params.minSpeed = 0;
	if (params.maxSpeed > 100) params.maxSpeed = 100;

	angle = XRandom(min_base_angle, max_base_angle);
	if (XRandom(0, 100) > 50) angle = angle + alt_angle;
	var = XRandom(min_angle_var, max_angle_var) / 2;
	params.minAngle = angle - var;
	params.maxAngle = angle + var;
}

void CWind::CalcDestSpeed() {
	float rand = XRandom(0, 100);
	if (rand > (100 - params.topProbability)) {
		DestSpeed = XRandom(params.maxSpeed, params.topSpeed);
		WindChange = params.maxChange;
	} else if (rand < params.nullProbability) {
		DestSpeed = 0.0;
		WindChange = XRandom(params.minChange, params.maxChange);
	} else {
		DestSpeed = XRandom(params.minSpeed, params.maxSpeed);
		WindChange = XRandom(params.minChange, params.maxChange);
	}

	if (DestSpeed > WSpeed) SpeedMode = 1;
	else SpeedMode = 0;
}

void CWind::CalcDestAngle() {
	DestAngle = XRandom(params.minAngle, params.maxAngle);
	AngleChange = XRandom(params.minAngleChange, params.maxAngleChange);

	if (DestAngle > WAngle) AngleMode = 1;
	else AngleMode = 0;
}

void CWind::Update(float timestep) {
	if (!windy) return;

	// the wind needn't be updated in each frame
	CurrTime = CurrTime + timestep;
	if (CurrTime > UPDATE_TIME) {
		CurrTime = 0.f;

		if (SpeedMode == 1) { // current speed lesser than destination speed
			if (WSpeed < DestSpeed) {
				WSpeed = WSpeed + WindChange;
			} else CalcDestSpeed();
		} else {
			if (WSpeed > DestSpeed) {
				WSpeed = WSpeed - WindChange;
			} else CalcDestSpeed();
		}
		if (WSpeed > params.topSpeed) WSpeed = params.topSpeed;
		if (WSpeed < 0) WSpeed = 0;


		if (AngleMode == 1) {
			if (WAngle < DestAngle) {
				WAngle = WAngle + AngleChange;
			} else CalcDestAngle();
		} else {
			if (WAngle > DestAngle) {
				WAngle = WAngle - AngleChange;
			} else CalcDestAngle();
		}
		if (WAngle > params.maxAngle) WAngle = params.maxAngle;
		if (WAngle < params.minAngle) WAngle = params.minAngle;

		float xx = std::sin(WAngle * M_PI / 180.f);
		float zz = std::sqrt(1 - xx * xx);
		if ((WAngle > 90 && WAngle < 270) || (WAngle > 450 && WAngle < 630)) {
			zz = -zz;
		}

		WVector.x = WSpeed * xx;
		WVector.z = WSpeed * zz * 0.2;
	}
}

void CWind::Init(int wind_id) {
	if (wind_id < 1 || wind_id > 3) {
		windy = false;
		WVector = TVector3d(0, 0, 0);
		WAngle = 0;
		WSpeed = 0;
		return;
	}
	windy = true;;
	SetParams(wind_id -1);
	WSpeed = XRandom(params.minSpeed, (params.minSpeed + params.maxSpeed) / 2);
	WAngle = XRandom(params.minAngle, params.maxAngle);
	CalcDestSpeed();
	CalcDestAngle();
}

// ====================================================================
//			access functions
// ====================================================================

void InitSnow(const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Init(g_game.snow_id, ctrl);
	Curtain.Init(ctrl);
}

void UpdateSnow(float timestep, const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Update(timestep, ctrl);
	Curtain.Update(timestep, ctrl);
}

void DrawSnow(const CControl *ctrl) {
	if (g_game.snow_id < 1 || g_game.snow_id > 3) return;
	Flakes.Draw(ctrl);
	Curtain.Draw();
}

void InitWind() {
	Wind.Init(g_game.wind_id);
}

void UpdateWind(float timestep) {
	Wind.Update(timestep);
}
