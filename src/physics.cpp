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

#include "physics.h"
#include "course.h"
#include "tux.h"
#include "audio.h"
#include "particles.h"
#include "game_ctrl.h"
#include "game_over.h"
#include <algorithm>

CControl::CControl() :
	cnet_force(0, 0, 0) {
	minSpeed = 0;
	minFrictspeed = 0;
	turn_fact = 0;
	turn_animation = 0;
	is_braking = false;
	jump_amt = 0;
	is_paddling = false;
	jumping = false;
	jump_charging = false;
	last_pos = cpos;
	orientation_initialized = false;
	cairborne = false;
	way = 0.0;
	front_flip = false;
	back_flip = false;
	roll_left = false;
	roll_right = false;
	roll_factor = 0;
	flip_factor = 0;

	ode_time_step = -1;
	jump_start_time = 0;
	begin_jump = false;
	paddle_time = 0;
	view_init = false;
	finish_speed = 0;

	viewmode = ABOVE;
}

// --------------------------------------------------------------------
// 					init
// --------------------------------------------------------------------

void CControl::Init() {
	TVector3d nml = Course.FindCourseNormal(cpos.x, cpos.z);
	TMatrix<4, 4> rotMat;
	rotMat.SetRotationMatrix(-90.0, 'x');
	TVector3d init_vel = TransformVector(rotMat, nml);
	init_vel *= INIT_TUX_SPEED;

	turn_fact = 0;
	turn_animation = 0;
	is_braking = false;
	jump_amt = 0;
	is_paddling = false;
	jumping = false;
	jump_charging = false;
	cpos.y = Course.FindYCoord(cpos.x, cpos.z);
	cvel = init_vel;
	last_pos = cpos;
	cnet_force = TVector3d(0, 0, 0);
	orientation_initialized = false;
	plane_nml = nml;
	cdirection = init_vel;
	cairborne = false;
	way = 0.0;

	// tricks
	front_flip = false;
	back_flip = false;
	roll_left = false;
	roll_right = false;
	roll_factor = 0;
	flip_factor = 0;

	ode_time_step = -1;
}
// --------------------------------------------------------------------
//					collision
// --------------------------------------------------------------------

bool CControl::CheckTreeCollisions(const TVector3d& pos, TVector3d *tree_loc) const {
	// These variables are used to cache collision detection results
	static bool last_collision = false;
	static TVector3d last_collision_tree_loc(-999, -999, -999);
	static TVector3d last_collision_pos(-999, -999, -999);

	TVector3d dist_vec = pos - last_collision_pos;
	if (MAG_SQD(dist_vec) < COLL_TOLERANCE) {
		if (last_collision && !cairborne) {
			if (tree_loc != nullptr) *tree_loc = last_collision_tree_loc;
			return true;
		} else return false;
	}

	TVector3d loc(0, 0, 0);
	bool hit = false;
	TMatrix<4, 4> mat;

	for (std::size_t i = 0; i<Course.CollArr.size(); i++) {
		double diam = Course.CollArr[i].diam;
		double height = Course.CollArr[i].height;
		loc = Course.CollArr[i].pt;
		TVector3d distvec(loc.x - pos.x, 0.0, loc.z - pos.z);

		// check distance from tree; .6 is the radius of a bounding sphere
		double squared_dist = (diam / 2.0 + 0.6);
		squared_dist *= squared_dist;
		if (MAG_SQD(distvec) > squared_dist) continue;

		TPolyhedron ph2 = Course.GetPoly(Course.CollArr[i].tree_type);
		mat.SetScalingMatrix(diam, height, diam);
		TransPolyhedron(mat, ph2);
		mat.SetTranslationMatrix(loc.x, loc.y, loc.z);
		TransPolyhedron(mat, ph2);

		hit = g_game.character->shape->Collision(pos, ph2);
		if (hit == true) {
			if (tree_loc != nullptr) *tree_loc = loc;
			Sound.Play("tree_hit", 0);
			break;
		}
	}

	last_collision_tree_loc = loc;
	last_collision_pos = pos;
	last_collision = hit;

	return hit;
}

void CControl::AdjustTreeCollision(const TVector3d& pos, TVector3d *vel) const {
	TVector3d treeLoc;

	if (CheckTreeCollisions(pos, &treeLoc)) {
		TVector3d treeNml(
		    pos.x - treeLoc.x,
		    0,
		    pos.z - treeLoc.z);
		treeNml.Norm();

		double speed = vel->Norm();
		speed *= 0.8;  // original 0.7

		double costheta = DotProduct(*vel, treeNml);
		if (costheta < 0) {
			double factor;
			if (cairborne) factor = 0.5;
			else factor = 1.5;
			*vel += (-factor * costheta) * treeNml;
			vel->Norm();
		}
		speed = std::max(speed, minSpeed);
		*vel *= speed;
	}
}

void CControl::CheckItemCollection(const TVector3d& pos) {
	std::size_t num_items = Course.NocollArr.size();

	for (std::size_t i=0; i<num_items; i++) {
		if (Course.NocollArr[i].collectable != 1) continue;

		double diam = Course.NocollArr[i].diam;
		const TVector3d& loc = Course.NocollArr[i].pt;

		TVector3d distvec(loc.x - pos.x, loc.y - pos.y, loc.z - pos.z);
		double squared_dist = (diam / 2. + 0.7);
		squared_dist *= squared_dist;
		if (MAG_SQD(distvec) <= squared_dist) {  // Check collision using a bounding sphere
			Course.NocollArr[i].collectable = 0;
			g_game.herring += 1;
			Sound.Play("pickup1", 0);
			Sound.Play("pickup2", 0);
			Sound.Play("pickup3", 0);
		}
	}
}
// --------------------------------------------------------------------
//				position and velocity  ***
// --------------------------------------------------------------------

void CControl::AdjustVelocity() {
	double speed = cvel.Norm();
	speed = std::max(minSpeed, speed);
	cvel *= speed;

	if (g_game.finish == true) {
/// --------------- finish ------------------------------------
		if (speed < 3) State::manager.RequestEnterState(GameOver);
/// -----------------------------------------------------------
	}
}

void CControl::AdjustPosition(const TPlane& surf_plane, double dist_from_surface) {
	if (dist_from_surface < -MAX_SURF_PEN) {
		double displace = -MAX_SURF_PEN - dist_from_surface;
		cpos += displace * surf_plane.nml;
	}
}

void CControl::SetTuxPosition(double speed) {
	CCharShape *shape = g_game.character->shape;

	TVector2d playSize = Course.GetPlayDimensions();
	TVector2d courseSize = Course.GetDimensions();
	double boundaryWidth = (courseSize.x - playSize.x) / 2;
	if (cpos.x < boundaryWidth) cpos.x = boundaryWidth;
	if (cpos.x > courseSize.x - boundaryWidth) cpos.x = courseSize.x - boundaryWidth;
	if (cpos.z > 0) cpos.z = 0;

	if (g_game.finish == false) {
/// ------------------- finish --------------------------------
		if (-cpos.z >= playSize.y) {
			if (g_game.use_keyframe) {
				g_game.finish = true;
				finish_speed = speed;
//				SetStationaryCamera (true);
			} else State::manager.RequestEnterState(GameOver);
		}
/// -----------------------------------------------------------
	}
	double disp_y = cpos.y + TUX_Y_CORR;
	shape->ResetNode(0);
	shape->TranslateNode(0, TVector3d(cpos.x, disp_y, cpos.z));
}
// --------------------------------------------------------------------
//			forces ***
// --------------------------------------------------------------------

TVector3d CControl::CalcRollNormal(double speed) {
	TVector3d vel = ProjectToPlane(ff.surfnml, ff.vel);
	vel.Norm();

	double roll_angle = MAX_ROLL_ANGLE;
	if (is_braking) roll_angle = BRAKING_ROLL_ANGLE;

	double angle = turn_fact * roll_angle *
	               std::min(1.0, std::max(0.0, ff.frict_coeff) / IDEAL_ROLL_FRIC) *
	               std::min(1.0, std::max(0.0, speed - minSpeed) / (IDEAL_ROLL_SPEED - minSpeed));

	TMatrix<4, 4> rot_mat = RotateAboutVectorMatrix(vel, angle);
	return TransformVector(rot_mat, ff.surfnml);
}

const double airlog[]  = {-1, 0, 1, 2, 3, 4, 5, 6};
const double airdrag[] = {2.25, 1.35, 0.6, 0, -0.35, -0.45, -0.33, -0.9};

TVector3d CControl::CalcAirForce() {
	TVector3d windvec = -ff.vel;
	if (g_game.wind_id > 0)
		windvec += WIND_FACTOR * Wind.WindDrift();

	double windspeed = windvec.Length();
	double re = 34600 * windspeed;
	int tablesize = sizeof(airdrag) / sizeof(airdrag[0]);
	double interpol = LinearInterp(airlog, airdrag, std::log10(re), tablesize);
	double dragcoeff = std::pow(10.0, interpol);
	double airfact = 0.104 * dragcoeff *  windspeed;
	return airfact * windvec;
}

TVector3d CControl::CalcSpringForce() {
	double springvel = DotProduct(ff.vel, ff.rollnml);
	double springfact = std::min(ff.compression, 0.05) * 1500;
	springfact += clamp(0.0, ff.compression - 0.05, 0.12) * 3000;
	springfact += std::max(0.0, ff.compression - 0.12 - 0.05) * 10000;
	springfact -= springvel * (ff.compression <= 0.05 ? 1500 : 500);
	springfact = clamp(0.0, springfact, 3000.0);
	return springfact * ff.rollnml;
}

TVector3d CControl::CalcNormalForce() {
	if (ff.surfdistance <= -ff.comp_depth) {
		ff.compression = -ff.surfdistance - ff.comp_depth;
		return CalcSpringForce();
	}
	return TVector3d(0, 0, 0);
}

TVector3d CControl::CalcJumpForce() {
	TVector3d jumpforce;
	if (begin_jump == true) {
		begin_jump = false;
		if (cairborne == false) {
			jumping = true;
			jump_start_time = g_game.time;
		} else jumping = false;
	}
	if ((jumping) && (g_game.time - jump_start_time < JUMP_FORCE_DURATION)) {
		double y = 294 + jump_amt * 294; // jump_amt goes from 0 to 1
		jumpforce.y = y;

	} else {
		jumping = false;
	}
	return jumpforce; // normally scaled with 1.0
}

TVector3d CControl::CalcFrictionForce(double speed, const TVector3d& nmlforce) {
	if ((cairborne == false && speed > minFrictspeed) || g_game.finish) {
		double fric_f_mag = nmlforce.Length() * ff.frict_coeff;
		fric_f_mag = std::min(MAX_FRICT_FORCE, fric_f_mag);
		TVector3d frictforce = fric_f_mag * ff.frictdir;

		double steer_angle = turn_fact * MAX_TURN_ANGLE;

		if (std::fabs(fric_f_mag * std::sin(steer_angle * M_PI / 180)) > MAX_TURN_PERP) {
			steer_angle = RADIANS_TO_ANGLES(std::asin(MAX_TURN_PERP / fric_f_mag)) *
			              turn_fact / std::fabs(turn_fact);
		}
		TMatrix<4, 4> fric_rot_mat = RotateAboutVectorMatrix(ff.surfnml, steer_angle);
		frictforce = TransformVector(fric_rot_mat, frictforce);
		return (1.0 + MAX_TURN_PEN) * frictforce;
	}
	return TVector3d(0, 0, 0);
}

TVector3d CControl::CalcBrakeForce(double speed) {
	if (g_game.finish == false) {
		if (cairborne == false && speed > minFrictspeed) {
			if (speed > minSpeed && is_braking) {
				return ff.frict_coeff * BRAKE_FORCE * ff.frictdir;
			}
		}
		return TVector3d(0, 0, 0);
	} else {
/// ------------------- finish --------------------------------
		if (cairborne == false) {
			is_braking = true;
			return finish_speed * g_game.finish_brake * ff.frictdir;
		} else {
			return finish_speed * FIN_AIR_BRAKE * ff.frictdir;
		}
/// -----------------------------------------------------------
	}
	return TVector3d(0, 0, 0);
}

TVector3d CControl::CalcPaddleForce(double speed) {
	TVector3d paddleforce(0, 0, 0);
	if (is_paddling)
		if (g_game.time - paddle_time >= PADDLING_DURATION) is_paddling = false;

	if (is_paddling) {
		if (cairborne) {
			paddleforce.z = -TUX_MASS * EARTH_GRAV / 4.0;
			paddleforce = RotateVector(corientation, paddleforce);
		} else {
			double factor = -std::min(MAX_PADD_FORCE, MAX_PADD_FORCE
			                          * (MAX_PADDLING_SPEED - speed) / MAX_PADDLING_SPEED
			                          * std::min(1.0, ff.frict_coeff / IDEAL_PADD_FRIC));
			paddleforce = factor * ff.frictdir;
		}
	} else return paddleforce;
	return PADDLE_FACT * paddleforce;
}

TVector3d CControl::CalcGravitationForce() {
	if (g_game.finish == false) {
		return TVector3d(0, -EARTH_GRAV * TUX_MASS, 0);
	} else {
/// ---------------- finish -----------------------------------
		if (cairborne) return TVector3d(0, -FIN_AIR_GRAV, 0);
		else return TVector3d(0, -FIN_GRAV, 0);
/// -----------------------------------------------------------
	}
}

TVector3d CControl::CalcNetForce(const TVector3d& pos, const TVector3d& vel) {
	// pos and vel are temporary, see ODE solver

	ff.pos = pos;
	ff.vel = vel;

	ff.frictdir = ff.vel;
	double speed = ff.frictdir.Norm();
	ff.frictdir *= -1.0;

	static std::vector<double> surfweights;
	if (surfweights.size() != Course.TerrList.size())
		surfweights.resize(Course.TerrList.size());
	Course.GetSurfaceType(ff.pos.x, ff.pos.z, &surfweights[0]);
	ff.frict_coeff = ff.comp_depth = 0;
	for (std::size_t i=0; i<Course.TerrList.size(); i++) {
		ff.frict_coeff += surfweights[i] * Course.TerrList[i].friction;
		ff.comp_depth += surfweights[i] * Course.TerrList[i].depth;
	}

	TPlane surfplane = Course.GetLocalCoursePlane(ff.pos);
	ff.surfnml = surfplane.nml;
	ff.rollnml = CalcRollNormal(speed);
	ff.surfdistance = DistanceToPlane(surfplane, ff.pos);
	cairborne = (bool)(ff.surfdistance > 0);

	// don't change this order:
	TVector3d gravforce = CalcGravitationForce();
	TVector3d nmlforce = CalcNormalForce();
	TVector3d jumpforce = CalcJumpForce();
	TVector3d frictforce = CalcFrictionForce(speed, nmlforce);
	TVector3d brakeforce = CalcBrakeForce(speed);
	TVector3d airforce = CalcAirForce();
	TVector3d paddleforce = CalcPaddleForce(speed);

	return jumpforce + gravforce + nmlforce + frictforce + airforce + brakeforce + paddleforce;
}

// --------------------------------------------------------------------
//				ODE solver
// --------------------------------------------------------------------

double CControl::AdjustTimeStep(double h, const TVector3d& vel) {
	double speed = vel.Length();
	h = clamp(MIN_TIME_STEP, h, MAX_STEP_DIST / speed);
	h = std::min(h, MAX_TIME_STEP);
	return h;
}

void CControl::SolveOdeSystem(double timestep) {
	double pos_err[3], vel_err[3], tot_pos_err, tot_vel_err;
	double err=0, tol=0;

	static const TOdeSolver solver;
	double h = ode_time_step;
	if (h < 0 || solver.EstimateError == nullptr)
		h = AdjustTimeStep(timestep, cvel);
	double t = 0;
	double tfinal = timestep;

	TOdeData x;
	TOdeData y;
	TOdeData z;
	TOdeData vx;
	TOdeData vy;
	TOdeData vz;

	TVector3d new_pos = cpos;
	TVector3d new_vel = cvel;
	TVector3d new_f   = cnet_force;

	bool done = false;
	while (!done) {
		if (t >= tfinal) {
			Message("t >= tfinal in ode_system()");
			break;
		}
		if (1.1 * h > tfinal - t) {
			h = tfinal-t;
			done = true;
		}
		TVector3d saved_pos = new_pos;
		TVector3d saved_vel = new_vel;
		TVector3d saved_f = new_f;

		bool failed = false;
		for (;;) {
			solver.InitOdeData(&x, new_pos.x, h);
			solver.InitOdeData(&y, new_pos.y, h);
			solver.InitOdeData(&z, new_pos.z, h);
			solver.InitOdeData(&vx, new_vel.x, h);
			solver.InitOdeData(&vy, new_vel.y, h);
			solver.InitOdeData(&vz, new_vel.z, h);

			solver.UpdateEstimate(&x, 0, new_vel.x);
			solver.UpdateEstimate(&y, 0, new_vel.y);
			solver.UpdateEstimate(&z, 0, new_vel.z);
			solver.UpdateEstimate(&vx, 0, new_f.x / TUX_MASS);
			solver.UpdateEstimate(&vy, 0, new_f.y / TUX_MASS);
			solver.UpdateEstimate(&vz, 0, new_f.z / TUX_MASS);

			for (int i=1; i < solver.NumEstimates(); i++) {
				new_pos.x = solver.NextValue(&x, i);
				new_pos.y = solver.NextValue(&y, i);
				new_pos.z = solver.NextValue(&z, i);
				new_vel.x = solver.NextValue(&vx, i);
				new_vel.y = solver.NextValue(&vy, i);
				new_vel.z = solver.NextValue(&vz, i);

				solver.UpdateEstimate(&x, i, new_vel.x);
				solver.UpdateEstimate(&y, i, new_vel.y);
				solver.UpdateEstimate(&z, i, new_vel.z);

				new_f = CalcNetForce(new_pos, new_vel);

				solver.UpdateEstimate(&vx, i, new_f.x / TUX_MASS);
				solver.UpdateEstimate(&vy, i, new_f.y / TUX_MASS);
				solver.UpdateEstimate(&vz, i, new_f.z / TUX_MASS);
			}

			new_pos.x = solver.FinalEstimate(&x);
			new_pos.y = solver.FinalEstimate(&y);
			new_pos.z = solver.FinalEstimate(&z);

			new_vel.x = solver.FinalEstimate(&vx);
			new_vel.y = solver.FinalEstimate(&vy);
			new_vel.z = solver.FinalEstimate(&vz);

			if (solver.EstimateError != nullptr) {
				pos_err[0] = solver.EstimateError(&x);
				pos_err[1] = solver.EstimateError(&y);
				pos_err[2] = solver.EstimateError(&z);

				vel_err[0] = solver.EstimateError(&vx);
				vel_err[1] = solver.EstimateError(&vy);
				vel_err[2] = solver.EstimateError(&vz);

				tot_pos_err = 0.;
				tot_vel_err = 0.;

				for (int i=0; i<3; i++) {
					pos_err[i] *= pos_err[i];
					tot_pos_err += pos_err[i];
					vel_err[i] *= vel_err[i];
					tot_vel_err += vel_err[i];
				}
				tot_pos_err = std::sqrt(tot_pos_err);
				tot_vel_err = std::sqrt(tot_vel_err);
				if (tot_pos_err / MAX_POS_ERR > tot_vel_err / MAX_VEL_ERR) {
					err = tot_pos_err;
					tol = MAX_POS_ERR;
				} else {
					err = tot_vel_err;
					tol = MAX_VEL_ERR;
				}

				if (err > tol  && h > MIN_TIME_STEP + EPS) {
					done = false;
					if (!failed) {
						failed = true;
						h *=  std::max(0.5, 0.8 * std::pow(tol/err, solver.TimestepExponent()));
					} else h *= 0.5;

					h = AdjustTimeStep(h, saved_vel);
					new_pos = saved_pos;
					new_vel = saved_vel;
					new_f = saved_f;
				} else break;
			} else break;
		}

		t = t + h;
		double speed = new_vel.Length();
		if (param.perf_level > 2) generate_particles(this, h, new_pos, speed);

		new_f = CalcNetForce(new_pos, new_vel);

		if (!failed && solver.EstimateError != nullptr) {
			double temp = 1.25 * std::pow(err / tol, solver.TimestepExponent());
			if (temp > 0.2) h = h / temp;
			else h = 5.0 * h;
		}
		h = AdjustTimeStep(h, new_vel);
		AdjustTreeCollision(new_pos, &new_vel);
//		if (g_game.finish) new_vel = ScaleVector (0.99,new_vel);
		CheckItemCollection(new_pos);
	}
	ode_time_step = h;
	cnet_force = new_f;

	cvel = new_vel;
	last_pos = cpos;
	cpos = new_pos;

	double step = (cpos - last_pos).Length();
	way += step;
}

// --------------------------------------------------------------------
//				update tux position
// --------------------------------------------------------------------

void CControl::UpdatePlayerPos(float timestep) {
	CCharShape *shape = g_game.character->shape;
	double paddling_factor;
	double flap_factor;
	double dist_from_surface;

	if (g_game.finish) {
/// --------------------- finish ------------------------------
		minSpeed = 0;
		minFrictspeed = 0;
/// -----------------------------------------------------------
	} else {
		minSpeed = MIN_TUX_SPEED;
		minFrictspeed = MIN_FRICT_SPEED;
	}

	if (timestep > 2 * EPS) SolveOdeSystem(timestep);

	TPlane surf_plane = Course.GetLocalCoursePlane(cpos);
	TVector3d surf_nml = surf_plane.nml; // normal vector of terrain
	dist_from_surface = DistanceToPlane(surf_plane, cpos);

	double speed = cvel.Length();
	AdjustVelocity();
	AdjustPosition(surf_plane, dist_from_surface);
	SetTuxPosition(speed);	// speed only to set finish_speed
	shape->AdjustOrientation(this, timestep, dist_from_surface, surf_nml);

	flap_factor = 0;
	if (is_paddling) {
		double factor;
		factor = (g_game.time - paddle_time) / PADDLING_DURATION;
		if (cairborne) {
			paddling_factor = 0;
			flap_factor = factor;
		} else {
			paddling_factor = factor;
			flap_factor = 0;
		}
	} else {
		paddling_factor = 0;
	}

	TVector3d local_force = RotateVector
	                        (ConjugateQuaternion(corientation), cnet_force);

	if (jumping)
		flap_factor = (g_game.time - jump_start_time) / JUMP_FORCE_DURATION;

	shape->AdjustJoints(turn_animation, is_braking, paddling_factor, speed,
	                    local_force, flap_factor);
}
