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

#include "physics.h"
#include "course.h"
#include "tux.h"
#include "audio.h"
#include "particles.h"
#include "game_ctrl.h"
#include "game_over.h"

CControl::CControl () :
	cnet_force(0, 0, 0)
{
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
}

// --------------------------------------------------------------------
// 					init
// --------------------------------------------------------------------

void CControl::Init () {
	TVector3 nml = Course.FindCourseNormal (cpos.x, cpos.z);
    TMatrix rotMat;
	MakeRotationMatrix (rotMat, -90.0, 'x');
	TVector3 init_vel = TransformVector (rotMat, nml);
	init_vel = ScaleVector (INIT_TUX_SPEED, init_vel);

	turn_fact = 0;
	turn_animation = 0;
	is_braking = false;
	jump_amt = 0;
	is_paddling = false;
	jumping = false;
	jump_charging = false;
	cpos.y = Course.FindYCoord (cpos.x, cpos.z);
	cvel = init_vel;
	last_pos = cpos;
	cnet_force = TVector3 (0, 0, 0);
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

bool CControl::CheckTreeCollisions (const TVector3& pos, TVector3 *tree_loc, double *tree_diam) {
    // These variables are used to cache collision detection results
    static bool last_collision = false;
    static TVector3 last_collision_tree_loc(-999, -999, -999);
    static double last_collision_tree_diam = 0;
    static TVector3 last_collision_pos(-999, -999, -999);

    TVector3 dist_vec = SubtractVectors (pos, last_collision_pos);
	if (MAG_SQD (dist_vec) < COLL_TOLERANCE) {
		if (last_collision && !cairborne) {
			if (tree_loc != NULL) *tree_loc = last_collision_tree_loc;
		    if (tree_diam != NULL) *tree_diam = last_collision_tree_diam;
		    return true;
		} else return false;
    }

	CCharShape *shape = Char.GetShape (g_game.char_id);
    double diam = 0.0;
    double height;
    TVector3 loc(0, 0, 0);
    bool hit = false;
	TMatrix mat;

	TCollidable *trees = &Course.CollArr[0];
    size_t num_trees = Course.CollArr.size();
    size_t tree_type = trees[0].tree_type;
    TPolyhedron ph = Course.GetPoly (tree_type);

    for (size_t i=0; i<num_trees; i++) {
        diam = trees[i].diam;
        height = trees[i].height;
        loc = trees[i].pt;
        TVector3 distvec(loc.x - pos.x, 0.0, loc.z - pos.z);

		// check distance from tree; .6 is the radius of a bounding sphere
		double squared_dist = (diam / 2.0 + 0.6);
		squared_dist *= squared_dist;
        if (MAG_SQD(distvec) > squared_dist) continue;

		// have to look at polyhedron - switch to correct one if necessary
		if (tree_type != trees[i].tree_type) {
	    	tree_type = trees[i].tree_type;
	    	ph = Course.GetPoly (tree_type);
		}

        TPolyhedron ph2 = CopyPolyhedron (ph);
        MakeScalingMatrix (mat, diam, height, diam);
        TransPolyhedron (mat, ph2);
        MakeTranslationMatrix (mat, loc.x, loc.y, loc.z);
        TransPolyhedron (mat, ph2);
//		hit = TuxCollision2 (pos, ph2);
		hit = shape->Collision (pos, ph2);
        FreePolyhedron (ph2);

        if (hit == true) {
	    	if (tree_loc != NULL) *tree_loc = loc;
	    	if (tree_diam != NULL) *tree_diam = diam;
			Sound.Play ("tree_hit", 0);
            break;
        }
    }

    last_collision_tree_loc = loc;
    last_collision_tree_diam = diam;
    last_collision_pos = pos;

    if (hit) last_collision = true;
    else last_collision = false;
    return hit;
}

void CControl::AdjustTreeCollision (const TVector3& pos, TVector3 *vel) {
    TVector3 treeLoc;
    double tree_diam;
	double factor;

	if (CheckTreeCollisions (pos, &treeLoc, &tree_diam)) {
		TVector3 treeNml(
			pos.x - treeLoc.x,
			0,
			pos.z - treeLoc.z);
        NormVector (treeNml);

        double speed = NormVector (*vel);
        speed *= 0.8;  // original 0.7

        double costheta = DotProduct (*vel, treeNml);
		if (costheta < 0) {
       	    if (cairborne) factor = 0.5; else factor = 1.5;
			*vel = AddVectors (
				//ScaleVector (-2 * DotProduct (*vel, treeNml), treeNml), *vel);
				ScaleVector (-factor * costheta, treeNml), *vel);
		    NormVector (*vel);
	    }
		speed = max (speed, minSpeed);
        *vel = ScaleVector (speed, *vel);
    }
}

void CControl::CheckItemCollection (const TVector3& pos) {
    static TVector3 last_collision_pos(-999, -999, -999);
    TVector3 dist_vec = SubtractVectors (pos, last_collision_pos);
    if (MAG_SQD (dist_vec) < COLL_TOLERANCE) return;

    TItem *items = &Course.NocollArr[0];
    size_t num_items = Course.NocollArr.size();

    for (size_t i=0; i<num_items; i++) {
		if (items[i].collectable != 1) continue;

		double diam = items[i].diam;
		double height = items[i].height;
		const TVector3& loc = items[i].pt;

		TVector3 distvec(loc.x - pos.x, 0.0, loc.z - pos.z);
		double squared_dist =  (diam / 2. + 0.6);
		squared_dist *= squared_dist;
		if (MAG_SQD (distvec) > squared_dist) continue;

		if ((pos.y - 0.6 >= loc.y && pos.y - 0.6 <= loc.y + height) ||
			(pos.y + 0.6 >= loc.y && pos.y + 0.6 <= loc.y + height) ||
			(pos.y - 0.6 <= loc.y && pos.y + 0.6 >= loc.y + height)) {
			items[i].collectable = 0;
			g_game.herring += 1;
			Sound.HaltAll ();
			Sound.Play ("pickup1", 0);
			Sound.Play ("pickup2", 0);
			Sound.Play ("pickup3", 0);
		}
    }
}
// --------------------------------------------------------------------
//				position and velocity  ***
// --------------------------------------------------------------------

void CControl::AdjustVelocity (const TPlane& surf_plane) {
    double speed = NormVector (cvel);
    speed = max (minSpeed, speed);

	if (g_game.finish == false) {
		cvel = ScaleVector (speed, cvel);
	} else {
/// --------------- finish ------------------------------------
		cvel = ScaleVector (speed, cvel);
		if (speed < 3) State::manager.RequestEnterState (GameOver);
/// -----------------------------------------------------------
	}
}

void CControl::AdjustPosition (const TPlane& surf_plane, double dist_from_surface) {
    if (dist_from_surface < -MAX_SURF_PEN) {
		double displace = -MAX_SURF_PEN - dist_from_surface;
		cpos = AddVectors (cpos, ScaleVector (displace, surf_plane.nml));
    }
}

void CControl::SetTuxPosition (double speed) {
	CCharShape *shape = Char.GetShape (g_game.char_id);

	TVector2 playSize = Course.GetPlayDimensions();
	TVector2 courseSize = Course.GetDimensions();
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
			} else State::manager.RequestEnterState (GameOver);
		}
/// -----------------------------------------------------------
	}
    double disp_y = cpos.y + TUX_Y_CORR;
	shape->ResetNode (0);
	shape->TranslateNode (0, TVector3 (cpos.x, disp_y, cpos.z));
}
// --------------------------------------------------------------------
//			forces ***
// --------------------------------------------------------------------

TVector3 CControl::CalcRollNormal (double speed) {
	TVector3 vel = ProjectToPlane (ff.surfnml, ff.vel);
	NormVector (vel);

	double roll_angle = MAX_ROLL_ANGLE;
    if (is_braking) roll_angle = BRAKING_ROLL_ANGLE;

	double angle = turn_fact * roll_angle *
		MIN (1.0, MAX (0.0, ff.frict_coeff) / IDEAL_ROLL_FRIC) *
		MIN (1.0, MAX (0.0, speed - minSpeed) / (IDEAL_ROLL_SPEED - minSpeed));

	TMatrix rot_mat;
    RotateAboutVectorMatrix (rot_mat, vel, angle);
    return TransformVector (rot_mat, ff.surfnml);
}

const double airlog[]  = {-1, 0, 1, 2, 3, 4, 5, 6};
const double airdrag[] = {2.25, 1.35, 0.6, 0, -0.35, -0.45, -0.33, -0.9};

TVector3 CControl::CalcAirForce () {
    TVector3 windvec = ScaleVector (-1, ff.vel);
	if (g_game.wind_id > 0)
		windvec = AddVectors (windvec, ScaleVector (WIND_FACTOR, Wind.WindDrift ()));

	double windspeed = NormVector (windvec);
	double re = 34600 * windspeed;
	int tablesize = sizeof (airdrag) / sizeof (airdrag[0]);
	double interpol = LinearInterp (airlog, airdrag, log10 (re), tablesize);
    double dragcoeff = pow (10.0, interpol);
    double airfact = 0.104 * dragcoeff *  windspeed * windspeed;
    return ScaleVector (airfact, windvec);
}

TVector3 CControl::CalcSpringForce () {
	double springvel = DotProduct (ff.vel, ff.rollnml);
	double springfact = MIN (ff.compression, 0.05) * 1500;
    springfact += MAX (0, min (ff.compression - 0.05, 0.12)) * 3000;
    springfact += MAX (0, ff.compression - 0.12 - 0.05) * 10000;
	springfact -= springvel * (ff.compression <= 0.05 ? 1500 : 500);
	springfact = MAX (springfact, 0.0);
    springfact = MIN (springfact, 3000);
	return ScaleVector (springfact, ff.rollnml);
}

TVector3 CControl::CalcNormalForce () {
	if (ff.surfdistance <= -ff.comp_depth) {
		ff.compression = -ff.surfdistance - ff.comp_depth;
		return CalcSpringForce ();
    }
	return TVector3(0, 0, 0);
}

TVector3 CControl::CalcJumpForce () {
	TVector3 jumpforce;
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
	return ScaleVector (1.0, jumpforce); // normally 1.0
}

TVector3 CControl::CalcFrictionForce (double speed, const TVector3& nmlforce) {
	double fric_f_mag;
	TMatrix fric_rot_mat;
	double steer_angle;

	if ((cairborne == false && speed > minFrictspeed) || g_game.finish) {
		TVector3 tmp_nml_f = nmlforce;
		fric_f_mag = NormVector (tmp_nml_f) * ff.frict_coeff;
		fric_f_mag = MIN (MAX_FRICT_FORCE, fric_f_mag);
		TVector3 frictforce = ScaleVector (fric_f_mag, ff.frictdir);

		steer_angle = turn_fact * MAX_TURN_ANGLE;

		if (fabs (fric_f_mag * sin (steer_angle * M_PI / 180)) > MAX_TURN_PERP) {
	    	steer_angle = RADIANS_TO_ANGLES (asin (MAX_TURN_PERP / fric_f_mag)) *
					turn_fact / fabs (turn_fact);
		}
		RotateAboutVectorMatrix (fric_rot_mat, ff.surfnml, steer_angle);
		frictforce = TransformVector (fric_rot_mat, frictforce);
		return ScaleVector (1.0 + MAX_TURN_PEN, frictforce);
    }
	return TVector3 (0, 0, 0);
}

TVector3 CControl::CalcBrakeForce (double speed) {
	if (g_game.finish == false) {
		if (cairborne == false && speed > minFrictspeed) {
			if (speed > minSpeed && is_braking) {
				return ScaleVector (ff.frict_coeff * BRAKE_FORCE, ff.frictdir);
			}
		}
		return TVector3(0, 0, 0);
	} else {
/// ------------------- finish --------------------------------
		if (cairborne == false) {
			is_braking = true;
			return ScaleVector (finish_speed * g_game.finish_brake, ff.frictdir);
		} else {
			return ScaleVector (finish_speed * FIN_AIR_BRAKE, ff.frictdir);
		}
/// -----------------------------------------------------------
	}
	return TVector3(0, 0, 0);
}

TVector3 CControl::CalcPaddleForce (double speed) {
	TVector3 paddleforce(0, 0, 0);
    if (is_paddling)
		if (g_game.time - paddle_time >= PADDLING_DURATION) is_paddling = false;

     if (is_paddling) {
		if (cairborne) {
			paddleforce.z = -TUX_MASS * EARTH_GRAV / 4.0;
			paddleforce = RotateVector (corientation, paddleforce);
		} else {
		    paddleforce = ScaleVector (-1 * min (MAX_PADD_FORCE, MAX_PADD_FORCE
				* (MAX_PADDLING_SPEED - speed) / MAX_PADDLING_SPEED
				* min(1.0, ff.frict_coeff / IDEAL_PADD_FRIC)), ff.frictdir);
		}
    } else return paddleforce;
	return ScaleVector (PADDLE_FACT, paddleforce);
}

TVector3 CControl::CalcGravitationForce () {
	if (g_game.finish == false) {
		return TVector3 (0, -EARTH_GRAV * TUX_MASS, 0);
	} else {
/// ---------------- finish -----------------------------------
		if (cairborne) return TVector3 (0, -FIN_AIR_GRAV, 0);
		else return TVector3 (0, -FIN_GRAV, 0);
/// -----------------------------------------------------------
	}
}

TVector3 CControl::CalcNetForce (const TVector3& pos, const TVector3& vel) {
	// pos and vel are temporary, see ODE solver

	ff.pos = pos;
	ff.vel = vel;

	ff.frictdir = ff.vel;
    double speed = NormVector (ff.frictdir);
    ff.frictdir = ScaleVector (-1.0, ff.frictdir);

    vector<double> surfweights(Course.TerrList.size());
	Course.GetSurfaceType (ff.pos.x, ff.pos.z, &surfweights[0]);
	TTerrType *TerrList = &Course.TerrList[0];
	ff.frict_coeff = ff.comp_depth = 0;
    for (size_t i=0; i<Course.TerrList.size(); i++) {
		ff.frict_coeff += surfweights[i] * TerrList[i].friction;
		ff.comp_depth += surfweights[i] * TerrList[i].depth;
	}

	TPlane surfplane = Course.GetLocalCoursePlane (ff.pos);
	ff.surfnml = surfplane.nml;
	ff.rollnml = CalcRollNormal (speed);
	ff.surfdistance = DistanceToPlane (surfplane, ff.pos);
	cairborne = (bool)(ff.surfdistance > 0);

	// don't change this order:
	TVector3 gravforce = CalcGravitationForce ();
	TVector3 nmlforce = CalcNormalForce ();
	TVector3 jumpforce = CalcJumpForce ();
	TVector3 frictforce = CalcFrictionForce (speed, nmlforce);
	TVector3 brakeforce = CalcBrakeForce (speed);
	TVector3 airforce = CalcAirForce ();
	TVector3 paddleforce = CalcPaddleForce (speed);

	TVector3 netforce =
		AddVectors (jumpforce,
		AddVectors (gravforce,
		AddVectors (nmlforce,
		AddVectors (frictforce,
		AddVectors (airforce,
		AddVectors (brakeforce, paddleforce))))));

	return ScaleVector (1.0, netforce);
}

// --------------------------------------------------------------------
//				ODE solver
// --------------------------------------------------------------------

double CControl::AdjustTimeStep (double h, TVector3 vel) {
    double speed = NormVector (vel);
    h = max (h, MIN_TIME_STEP);
    h = min (h, MAX_STEP_DIST / speed);
    h = min (h, MAX_TIME_STEP);
    return h;
}

void CControl::SolveOdeSystem (double timestep) {
    bool failed = false;
    double speed;
    double pos_err[3], vel_err[3], tot_pos_err, tot_vel_err;
    double err=0, tol=0;

	TOdeSolver solver = NewOdeSolver23 ();
    double h = ode_time_step;
    if (h < 0 || solver.EstimateError == NULL)
		h = AdjustTimeStep (timestep, cvel);
	double t = 0;
    double tfinal = timestep;

    TOdeData *x = new TOdeData;
    TOdeData *y  = new TOdeData;
    TOdeData *z  = new TOdeData;
    TOdeData *vx = new TOdeData;
    TOdeData *vy = new TOdeData;
    TOdeData *vz = new TOdeData;

    TVector3 new_pos = cpos;
    TVector3 new_vel = cvel;
    TVector3 new_f   = cnet_force;

    bool done = false;
    while (!done) {
		if (t >= tfinal) {
			Message ("t >= tfinal in ode_system()", "");
		    break;
		}
		if (1.1 * h > tfinal - t) {
	    	h = tfinal-t;
		    done = true;
		}
		TVector3 saved_pos = new_pos;
		TVector3 saved_vel = new_vel;
		TVector3 saved_f = new_f;

		failed = false;
		for (;;) {
			solver.InitOdeData (x, new_pos.x, h);
		    solver.InitOdeData (y, new_pos.y, h);
	    	solver.InitOdeData (z, new_pos.z, h);
		    solver.InitOdeData (vx, new_vel.x, h);
		    solver.InitOdeData (vy, new_vel.y, h);
	    	solver.InitOdeData (vz, new_vel.z, h);

		    solver.UpdateEstimate (x, 0, new_vel.x);
		    solver.UpdateEstimate (y, 0, new_vel.y);
	    	solver.UpdateEstimate (z, 0, new_vel.z);
		    solver.UpdateEstimate (vx, 0, new_f.x / TUX_MASS);
		    solver.UpdateEstimate (vy, 0, new_f.y / TUX_MASS);
	    	solver.UpdateEstimate (vz, 0, new_f.z / TUX_MASS);

		    for (int i=1; i < solver.NumEstimates(); i++) {
				new_pos.x = solver.NextValue (x, i);
				new_pos.y = solver.NextValue (y, i);
				new_pos.z = solver.NextValue (z, i);
				new_vel.x = solver.NextValue (vx, i);
				new_vel.y = solver.NextValue (vy, i);
				new_vel.z = solver.NextValue (vz, i);

				solver.UpdateEstimate (x, i, new_vel.x);
				solver.UpdateEstimate (y, i, new_vel.y);
				solver.UpdateEstimate (z, i, new_vel.z);

				new_f = CalcNetForce (new_pos, new_vel);

				solver.UpdateEstimate (vx, i, new_f.x / TUX_MASS);
				solver.UpdateEstimate (vy, i, new_f.y / TUX_MASS);
				solver.UpdateEstimate (vz, i, new_f.z / TUX_MASS);
	    	}

		    new_pos.x = solver.FinalEstimate (x);
		    new_pos.y = solver.FinalEstimate (y);
	    	new_pos.z = solver.FinalEstimate (z);

		    new_vel.x = solver.FinalEstimate (vx);
		    new_vel.y = solver.FinalEstimate (vy);
	    	new_vel.z = solver.FinalEstimate (vz);

		    if (solver.EstimateError != NULL) {
				pos_err[0] = solver.EstimateError (x);
				pos_err[1] = solver.EstimateError (y);
				pos_err[2] = solver.EstimateError (z);

				vel_err[0] = solver.EstimateError (vx);
				vel_err[1] = solver.EstimateError (vy);
				vel_err[2] = solver.EstimateError (vz);

				tot_pos_err = 0.;
				tot_vel_err = 0.;

				for (int i=0; i<3; i++) {
				    pos_err[i] *= pos_err[i];
				    tot_pos_err += pos_err[i];
				    vel_err[i] *= vel_err[i];
				    tot_vel_err += vel_err[i];
				}
				tot_pos_err = sqrt (tot_pos_err);
				tot_vel_err = sqrt (tot_vel_err);
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
						h *=  max (0.5, 0.8 * pow (tol/err, solver.TimestepExponent()));
				    } else h *= 0.5;

				    h = AdjustTimeStep (h, saved_vel);
				    new_pos = saved_pos;
				    new_vel = saved_vel;
				    new_f = saved_f;
				} else break;
	    	} else break;
		}

		t = t + h;
		TVector3 tmp_vel = new_vel;
		speed = NormVector (tmp_vel);
		if (param.perf_level > 2) generate_particles (this, h, new_pos, speed);

		new_f = CalcNetForce (new_pos, new_vel);

		if (!failed && solver.EstimateError != NULL) {
	    	double temp = 1.25 * pow (err / tol, solver.TimestepExponent());
		    if (temp > 0.2) h = h / temp;
		    else h = 5.0 * h;
		}
		h = AdjustTimeStep (h, new_vel);
		AdjustTreeCollision (new_pos, &new_vel);
//		if (g_game.finish) new_vel = ScaleVector (0.99,new_vel);
		CheckItemCollection (new_pos);
	}
	ode_time_step = h;
	cnet_force = new_f;

	cvel = new_vel;
	last_pos = cpos;
	cpos = new_pos;

	float step = VectorLength (SubtractVectors (cpos, last_pos));
	way += step;

	delete x;
	delete y;
	delete z;
	delete vx;
	delete vy;
	delete vz;
}

// --------------------------------------------------------------------
//				update tux position
// --------------------------------------------------------------------

void CControl::UpdatePlayerPos (double timestep) {
	CCharShape *shape = Char.GetShape (g_game.char_id);
    double speed;
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

    if (timestep > 2 * EPS) SolveOdeSystem (timestep);

    TPlane surf_plane = Course.GetLocalCoursePlane (cpos);
    TVector3 surf_nml = surf_plane.nml; // normal vector of terrain
    dist_from_surface = DistanceToPlane (surf_plane, cpos);

    TVector3 temp_vel = cvel;
    AdjustVelocity (surf_plane);
    AdjustPosition (surf_plane, dist_from_surface);
    speed = NormVector (temp_vel);
    SetTuxPosition (speed);	// speed only to set finish_speed
	shape->AdjustOrientation (this, timestep, dist_from_surface, surf_nml);

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

    TVector3 local_force = RotateVector
		(ConjugateQuaternion (corientation), cnet_force);

    if (jumping)
		flap_factor = (g_game.time - jump_start_time) / JUMP_FORCE_DURATION;

    shape->AdjustJoints (turn_animation, is_braking, paddling_factor, speed,
			local_force, flap_factor);
}
