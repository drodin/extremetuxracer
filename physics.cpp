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

#include "physics.h"
#include "course.h"
#include "tux.h"
#include "audio.h"
#include "particles.h"

CControl::CControl () {}

CControl::~CControl () {}

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
	cnet_force = MakeVector (0, 0, 0);
	orientation_initialized = false;
	plane_nml = nml;
	cdirection = init_vel;
	airborne = false;

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

bool CControl::CheckTreeCollisions (TVector3 pos, TVector3 *tree_loc, double *tree_diam){
	CCharShape *shape = Char.GetShape (g_game.char_id);
    double diam = 0.0; 
    double height;
    TVector3 loc = MakeVector (0, 0, 0);
    TVector3 distvec;
    bool hit = false;
	TMatrix mat;
    
    // These variables are used to cache collision detection results 
    static bool last_collision = false;
    static TVector3 last_collision_tree_loc = {-999, -999, -999};
    static double last_collision_tree_diam = 0;
    static TVector3 last_collision_pos = {-999, -999, -999};

    TVector3 dist_vec = SubtractVectors (pos, last_collision_pos);
	if (MAG_SQD (dist_vec) < COLL_TOLERANCE) {
		if (last_collision && !airborne) {
			if (tree_loc != NULL) *tree_loc = last_collision_tree_loc;
		    if (tree_diam != NULL) *tree_diam = last_collision_tree_diam;
		    return true;
		} else return false;
    }

	TCollidable *trees = Course.CollArr;
    int num_trees = Course.numColl;
    int tree_type = trees[0].tree_type;
    TPolyhedron ph = Course.GetPoly (tree_type);

    for (int i=0; i<num_trees; i++) {
        diam = trees[i].diam;
        height = trees[i].height;
        loc = trees[i].pt;
        distvec = MakeVector (loc.x - pos.x, 0.0, loc.z - pos.z);

		// check distance from tree; .6 is the radius of a bounding sphere
		double squared_dist = (diam / 2.0 + 0.6);
		squared_dist *= squared_dist;
        if (MAG_SQD(distvec) > squared_dist) continue;

		// have to look at polyhedron - switch to correct one if necessary 
		if (tree_type != trees[i].tree_type)  {
	    	tree_type = trees[i].tree_type;
	    	ph = Course.GetPoly (tree_type);
		}

        TPolyhedron ph2 = CopyPolyhedron (ph);
        MakeScalingMatrix (mat, diam, height, diam);
        TransPolyhedron (mat, ph2);
        MakeTranslationMatrix (mat, loc.x, loc.y, loc.z);
        TransPolyhedron (mat, ph2);
		hit = shape->Collision (pos, ph2); 
        FreePolyhedron (ph2);

        if  (hit == true) {
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

void CControl::AdjustTreeCollision (TVector3 pos, TVector3 *vel){
    TVector3 treeNml;
    TVector3 treeLoc;
    double tree_diam;
	double factor;
	
	if (CheckTreeCollisions (pos, &treeLoc, &tree_diam)) {
        treeNml.x = pos.x - treeLoc.x;
        treeNml.y = 0;
        treeNml.z = pos.z - treeLoc.z; 
        NormVector (&treeNml);

        double speed = NormVector (vel);
        speed *= 0.8;  // original 0.7

        double costheta = DotProduct (*vel, treeNml);
		if (costheta < 0) {
       	    if (airborne) factor = 0.5; else factor = 1.5;
			*vel = AddVectors (
				//ScaleVector (-2 * DotProduct (*vel, treeNml), treeNml), *vel);
				ScaleVector (-factor * costheta, treeNml), *vel);
		    NormVector (vel);
   	    } 
		speed = max (speed, MIN_TUX_SPEED);
        *vel = ScaleVector (speed, *vel);
    } 
}

void CControl::CheckItemCollection (TVector3 pos) {
    static TVector3 last_collision_pos = {-999, -999, -999};
    TVector3 dist_vec = SubtractVectors (pos, last_collision_pos);
    if (MAG_SQD (dist_vec) < COLL_TOLERANCE) return;

    TItem *items = Course.NocollArr;
    int num_items = Course.numNocoll;

    for (int i=0; i<num_items; i++) {
		if (items[i].collectable != 1) continue;

		double diam = items[i].diam;
		double height = items[i].height;
		TVector3 loc = items[i].pt;

		TVector3 distvec = MakeVector (loc.x - pos.x, 0.0, loc.z - pos.z);
		double squared_dist =  (diam / 2. + 0.6);
		squared_dist *= squared_dist;
		if (MAG_SQD (distvec) > squared_dist) continue;

		if  ((pos.y - 0.6 >= loc.y && pos.y - 0.6 <= loc.y + height) ||
			(pos.y + 0.6 >= loc.y && pos.y + 0.6 <= loc.y + height) ||
			(pos.y - 0.6 <= loc.y && pos.y + 0.6 >= loc.y + height)) {
			items[i].collectable = 0;
			g_game.herring += 1;
			Sound.Play ("pickup1", 0);
			Sound.Play ("pickup2", 0);
			Sound.Play ("pickup3", 0);
		}
    } 
} 

// --------------------------------------------------------------------
//				position and velocity
// --------------------------------------------------------------------

double CControl::AdjustVelocity (TVector3 *vel, TVector3 pos, TPlane surf_plane, 
		double dist_from_surface){

	TVector3 surf_nml = surf_plane.nml;
    double speed = NormVector (vel);
	if (speed < EPS) {
		if  (fabs (surf_nml.x) + fabs (surf_nml.z) > EPS) {
			*vel = ProjectToPlane (surf_nml, MakeVector (0, -1, 0));
		    NormVector (vel);
		} else *vel = MakeVector (0, 0, -1);
    }
    speed = max (MIN_TUX_SPEED, speed);
    *vel = ScaleVector (speed, *vel);
    return speed;
}

void CControl::AdjustPosition (TVector3 *pos, TPlane surf_plane, double dist_from_surface) {
    if  (dist_from_surface < -MAX_SURF_PEN) {
		*pos = AddVectors (*pos, 
			ScaleVector (-MAX_SURF_PEN - dist_from_surface, surf_plane.nml));
    }
    return;
}

void CControl::SetTuxPosition (TVector3 new_pos){
	CCharShape *shape = Char.GetShape (g_game.char_id);
	double playWidth, playLength;
	double courseWidth, courseLength;

	Course.GetPlayDimensions (&playWidth, &playLength);
	Course.GetDimensions (&courseWidth, &courseLength);
	double boundaryWidth = (courseWidth - playWidth) / 2; 

	if (new_pos.x < boundaryWidth) {
		new_pos.x = boundaryWidth;
	} else if (new_pos.x > courseWidth - boundaryWidth) {
		new_pos.x = courseWidth - boundaryWidth;
	}

	if  (new_pos.z > 0) {
		new_pos.z = 0;
	} else if  (-new_pos.z >= playLength) {
		new_pos.z = -playLength;
		Winsys.SetMode (GAME_OVER);
	} 

    cpos = new_pos;
    double disp_y = new_pos.y + TUX_Y_CORR; 
	shape->ResetNode (0);
	shape->TranslateNode (0, MakeVector (new_pos.x, disp_y, new_pos.z));	
} 

// --------------------------------------------------------------------
//			forces
// --------------------------------------------------------------------

TVector3 CControl::AdjustRollNormal (TVector3 vel, double fric_coeff, TVector3 nml) {
	TMatrix rot_mat; 
    double roll_angle;

	double speed = NormVector (&vel);
    vel = ProjectToPlane (nml, vel);    
	NormVector (&vel);
    if  (is_braking) roll_angle = BRAKING_ROLL_ANGLE;
		else roll_angle = MAX_ROLL_ANGLE;

	double angle = turn_fact * roll_angle *
		min (1.0, max(0.0, fric_coeff) / IDEAL_ROLL_FRIC)
		* min (1.0, max(0.0,speed - MIN_TUX_SPEED) / (IDEAL_ROLL_SPEED - MIN_TUX_SPEED));

    RotateAboutVectorMatrix (rot_mat, vel, angle);
    return TransformVector (rot_mat, nml);
}

const double air_log_re[] = { -1, 0, 1, 2, 3, 4, 5, 6 };
const double air_drag_coeff[] = {2.25, 1.35, 0.6, 0, -0.35, -0.45, -0.33, -0.9};

TVector3 CControl::CalcAirForce (TVector3 player_vel) {
    static double last_time_called = -1;

    TVector3 total_vel = ScaleVector (-1, player_vel);
	if (g_game.wind_id > 0)
		total_vel = AddVectors (total_vel, ScaleVector (WIND_FACTOR, Wind.WindDrift ()));
	
	double wind_speed = NormVector (&total_vel);
	double re = AIR_DENSITY * wind_speed * TUX_WIDTH / AIR_VISCOSITY;    
	int table_size = sizeof (air_drag_coeff) / sizeof (air_drag_coeff[0]);
    double drag_coeff = pow (10.0, LinearInterp (air_log_re, air_drag_coeff, 
		log10(re), table_size));

    double df_mag = 0.5 * drag_coeff * AIR_DENSITY *  (wind_speed * wind_speed)
		*  (M_PI *  (TUX_WIDTH * TUX_WIDTH) * 0.25);
    last_time_called = g_game.time;
    return ScaleVector (df_mag, total_vel);
}

TVector3 CControl::CalcSpringForce (double compression, TVector3 vel, TVector3 surf_nml) {
	double spring_vel = DotProduct (vel, surf_nml);
	double spring_f_mag = MIN (compression, TG1_COMPRESS) * TG1_SPRING;
    spring_f_mag += MAX (0, min (compression - TG1_COMPRESS, TG2_COMPRESS)) * TG2_SPRING;
    spring_f_mag += MAX (0, compression - TG2_COMPRESS - TG1_COMPRESS) * TG3_SPRING;
	spring_f_mag -= spring_vel * (compression <= TG1_COMPRESS 
		? TG1_SPRING : (compression <= TG2_COMPRESS ? TG2_DAMP : TG2_DAMP));

	spring_f_mag = MAX (spring_f_mag, 0.0);
    spring_f_mag = MIN (spring_f_mag, TG_MAX_SPRING);
    return ScaleVector (spring_f_mag, surf_nml);
}

TVector3 CControl::CalcNetForce (TVector3 pos, TVector3 vel) {
    TVector3 nml_f;      		
    TVector3 fric_f;     		
    TVector3 fric_dir;   		
    TVector3 grav_f;     		
    TVector3 air_f;      		
    TVector3 brake_f;    		
    TVector3 paddling_f; 		
    TVector3 net_force;  		
    TVector3 jump_f;

    double fric_f_mag; 			
//    double comp_depth; 			
    double speed;      			
    TVector3 orig_surf_nml; 	
    TVector3 surf_nml;   		
    double glute_compression;	
    double steer_angle; 		
	TMatrix fric_rot_mat; 		
    TPlane surf_plane;
    double dist_from_surface;
    double surf_weights[MAX_TERR_TYPES];
    double surf_fric_coeff;
    int i;
	TTerrType *TerrList = Course.TerrList;

	grav_f = MakeVector (0, -EARTH_GRAV * TUX_MASS, 0);
	
	Course.GetSurfaceType (pos.x, pos.z, surf_weights);
	surf_plane = Course.GetLocalCoursePlane (pos);
	orig_surf_nml = surf_plane.nml;
	surf_fric_coeff = 0;

    for (i=0; i<Course.numTerr; i++)
		surf_fric_coeff += surf_weights[i] * TerrList[i].friction;

	surf_nml = AdjustRollNormal (vel, surf_fric_coeff, orig_surf_nml);
	double comp_depth = 0;

    for (i=0; i<Course.numTerr; i++)
		comp_depth += surf_weights[i] * TerrList[i].depth;

	dist_from_surface = DistanceToPlane (surf_plane, pos);
    if  (dist_from_surface <= 0) airborne = false;
	else airborne = true;

    nml_f = MakeVector (0, 0, 0);    
	if  (dist_from_surface <= -comp_depth) {
		glute_compression = -dist_from_surface - comp_depth;
		nml_f = CalcSpringForce (glute_compression, vel, surf_nml);
    }
	
	if (begin_jump == true) {
		begin_jump = false;
		if  (dist_from_surface <= 0) {
			jumping = true;
		    jump_start_time = g_game.time;
		} else jumping = false;
    }

    if ((jumping) &&
			(g_game.time - jump_start_time < JUMP_FORCE_DURATION)) {
		jump_f = MakeVector (0, BASE_JUMP_G_FORCE * TUX_MASS * EARTH_GRAV + 
	    	jump_amt * (MAX_JUMP_G_FORCE - BASE_JUMP_G_FORCE) 
	    	* TUX_MASS * EARTH_GRAV, 0);
    } else {
		jump_f = MakeVector (0, 0, 0);
		jumping = false;
    }

    fric_dir = vel;
    speed = NormVector (&fric_dir);
    fric_dir = ScaleVector (-1.0, fric_dir);
    
	if (dist_from_surface < 0 && speed > MIN_FRICT_SPEED) {
		TVector3 tmp_nml_f = nml_f;
		fric_f_mag = NormVector (&tmp_nml_f) * surf_fric_coeff;
		fric_f_mag = MIN (MAX_FRICT_FORCE, fric_f_mag);
		fric_f = ScaleVector (fric_f_mag, fric_dir);

		steer_angle = turn_fact * MAX_TURN_ANGLE;

		if  (fabs (fric_f_mag * sin (steer_angle * M_PI / 180)) > MAX_TURN_PERP) {
	    	steer_angle = RADIANS_TO_ANGLES (asin (MAX_TURN_PERP / fric_f_mag)) * 
					turn_fact / fabs (turn_fact);
		}
		RotateAboutVectorMatrix (fric_rot_mat, orig_surf_nml, steer_angle);
		fric_f = TransformVector (fric_rot_mat, fric_f);
		fric_f = ScaleVector (1.0 + MAX_TURN_PEN, fric_f);

		if (speed > MIN_TUX_SPEED && is_braking) {
		    brake_f = ScaleVector (surf_fric_coeff * BRAKE_FORCE, fric_dir); 
		} else brake_f = MakeVector (0., 0., 0.);
	
    } else {
		fric_f = brake_f = MakeVector (0., 0., 0.);
    }

    air_f = CalcAirForce (vel);

    if (is_paddling) {
		if (g_game.time - paddle_time >= PADDLING_DURATION)
			is_paddling = false;
    }
    if (is_paddling) {
		if (airborne) {
		    paddling_f = MakeVector (0, 0, -TUX_MASS * EARTH_GRAV / 4.0);
	    	paddling_f = RotateVector (corientation, paddling_f);
		} else {
		    paddling_f = ScaleVector (-1 * min (MAX_PADD_FORCE, MAX_PADD_FORCE 
				* (MAX_PADDLING_SPEED - speed) / MAX_PADDLING_SPEED 
				* min(1.0, surf_fric_coeff/IDEAL_PADD_FRIC)), fric_dir);
		}
    } else paddling_f = MakeVector (0, 0, 0);

	net_force = 
		AddVectors (jump_f, 
		AddVectors (grav_f, 
		AddVectors (nml_f, 
		AddVectors (fric_f, 
		AddVectors (air_f, 
		AddVectors (brake_f, paddling_f))))));

	net_force.z = net_force.z;		
    return ScaleVector (1.0, net_force);
}

// --------------------------------------------------------------------
//				ODE solver
// --------------------------------------------------------------------

double CControl::adjust_time_step_size (double h, TVector3 vel) {
    double speed;

    speed = NormVector (&vel);
    h = max (h, MIN_TIME_STEP);
    h = min (h, MAX_STEP_DIST / speed);
    h = min (h, MAX_TIME_STEP);
    return h;
}

void CControl::SolveOdeSystem (double timestep) {
    bool failed = false;
    double speed;
    TVector3 saved_pos, saved_vel, saved_f;
    double pos_err[3], vel_err[3], tot_pos_err, tot_vel_err;
    double err=0, tol=0;
    int i;

 	TOdeSolver solver = NewOdeSolver23 ();  	
    double h = ode_time_step;
    if (h < 0 || solver.EstimateError == NULL)
		h = adjust_time_step_size (timestep, cvel);
	double t = 0;
    double tfinal = timestep;

    TOdeData *x = solver.NewOdeData();
    TOdeData *y  = solver.NewOdeData();
    TOdeData *z  = solver.NewOdeData();
    TOdeData *vx = solver.NewOdeData();
    TOdeData *vy = solver.NewOdeData();
    TOdeData *vz = solver.NewOdeData();

    TVector3 new_pos = cpos;
    TVector3 new_vel = cvel;
    TVector3 new_f   = cnet_force;

    bool done = false;
    while (!done) {
		if  (t >= tfinal) {
	   		Message ("t >= tfinal in ode_system()", "");
		    break; 
		}
		if  (1.1 * h > tfinal - t) {
	    	h = tfinal-t;
		    done = true; 
		}
		saved_pos = new_pos;
		saved_vel = new_vel;
		saved_f = new_f;
	
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

		    for (i=1; i < solver.NumEstimates(); i++) {
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
				
				for  (i=0; i<3; i++) {
				    pos_err[i] *= pos_err[i];
				    tot_pos_err += pos_err[i];
				    vel_err[i] *= vel_err[i];
				    tot_vel_err += vel_err[i];
				}
				tot_pos_err = sqrt (tot_pos_err);
				tot_vel_err = sqrt (tot_vel_err);
				if  (tot_pos_err / MAX_POS_ERR > tot_vel_err / MAX_VEL_ERR){
			    	err = tot_pos_err;
				    tol = MAX_POS_ERR;
				} else {
				    err = tot_vel_err;
				    tol = MAX_VEL_ERR;
				}

				if (err > tol  && h > MIN_TIME_STEP + EPS) {
				    done = false;
				    if  (!failed) {
						failed = true;
						h *=  max (0.5, 0.8 * pow (tol/err, solver.TimestepExponent()));
				    } else h *= 0.5;

				    h = adjust_time_step_size (h, saved_vel);
				    new_pos = saved_pos;
				    new_vel = saved_vel;
				    new_f = saved_f;
				} else break;
	    	} else break; 
		} 
	
		t = t + h;
		TVector3 tmp_vel = new_vel; 
		speed = NormVector (&tmp_vel);
		if (param.perf_level > 2) generate_particles (this, h, new_pos, speed);
		new_f = CalcNetForce (new_pos, new_vel);
		
		if  (!failed && solver.EstimateError != NULL) {
	    	double temp = 1.25 * pow (err / tol, solver.TimestepExponent());
		    if (temp > 0.2) h = h / temp;
		    else h = 5.0 * h;
		}
		h = adjust_time_step_size (h, new_vel);
		AdjustTreeCollision (new_pos, &new_vel);
		CheckItemCollection (new_pos);
	}
    
	ode_time_step = h;
	cvel = new_vel; 
    cpos = new_pos;
    cnet_force = new_f;

    free (x);
    free (y);
    free (z);
    free (vx);
    free (vy);
    free (vz);
}

// --------------------------------------------------------------------
//				update tux position
// --------------------------------------------------------------------

void CControl::UpdatePlayerPos (double timestep) {
	CCharShape *shape = Char.GetShape (g_game.char_id);
    TVector3 surf_nml;   // normal vector of terrain 
    TVector3 tmp_vel;
    double speed;
    double paddling_factor; 
    TVector3 local_force;
    double flap_factor;
    TPlane surf_plane;
    double dist_from_surface;

    if (timestep > 2 * EPS) SolveOdeSystem (timestep);
    tmp_vel = cvel;
    surf_plane = Course.GetLocalCoursePlane (cpos);
    surf_nml = surf_plane.nml;
    dist_from_surface = DistanceToPlane (surf_plane, cpos);

    AdjustVelocity (&cvel, cpos, surf_plane, dist_from_surface);
    AdjustPosition (&cpos, surf_plane, dist_from_surface);
    speed = NormVector (&tmp_vel);

    SetTuxPosition (cpos);
	shape->AdjustOrientation (this, timestep, dist_from_surface, surf_nml);

    flap_factor = 0;
    if (is_paddling) {
		double factor;
		factor = (g_game.time - paddle_time) / PADDLING_DURATION;
		if  (airborne) {
		    paddling_factor = 0;
		    flap_factor = factor;
		} else {
		    paddling_factor = factor;
		    flap_factor = 0;
		}
    } else {
		paddling_factor = 0;
    }

    local_force = RotateVector 
		(ConjugateQuaternion (corientation), cnet_force);

    if (jumping)
		flap_factor = (g_game.time - jump_start_time) / JUMP_FORCE_DURATION;

    shape->AdjustJoints (turn_animation, is_braking, paddling_factor, speed, 
			local_force, flap_factor);
}







