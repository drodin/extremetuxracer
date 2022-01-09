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

#ifndef PHYSICS_H
#define PHYSICS_H

#include "bh.h"
#include "mathlib.h"

#define MAX_PADDLING_SPEED (60.0 / 3.6)
#define PADDLE_FACT 1.0

#define EARTH_GRAV 9.81
#define JUMP_FORCE_DURATION 0.20
#define TUX_MASS 20
#define MIN_TUX_SPEED 1.4
#define INIT_TUX_SPEED 3.0
#define COLL_TOLERANCE 0.1

#define MAX_SURF_PEN 0.2
#define TUX_Y_CORR 0.36
#define IDEAL_ROLL_SPEED 6.0
#define IDEAL_ROLL_FRIC 0.35
#define WIND_FACTOR 1.5

#define MIN_FRICT_SPEED 2.8
#define MAX_FRICT_FORCE 800.0
#define MAX_TURN_ANGLE 45
#define MAX_TURN_PERP 400
#define MAX_TURN_PEN 0.15
#define PADDLING_DURATION 0.40
#define IDEAL_PADD_FRIC 0.35
#define MAX_PADD_FORCE 122.5
#define BRAKE_FORCE 200

#define MIN_TIME_STEP 0.01
#define MAX_TIME_STEP 0.10
#define MAX_STEP_DIST 0.20
#define MAX_POS_ERR 0.005
#define MAX_VEL_ERR	0.05

#define MAX_ROLL_ANGLE 30
#define BRAKING_ROLL_ANGLE 55

// constants for finish stage
#define FIN_AIR_GRAV 500
#define FIN_GRAV 500
#define FIN_AIR_BRAKE 20
#define FIN_BRAKE 12

struct TForce {
	TVector3d surfnml;
	TVector3d rollnml;
	TVector3d pos;
	TVector3d vel;
	TVector3d frictdir;

	double frict_coeff;
	double comp_depth;
	double surfdistance;
	double compression;
};

class CControl {
private:
	TForce ff;
	double ode_time_step;
	double finish_speed;

	bool CheckTreeCollisions(const TVector3d& pos, TVector3d *tree_loc) const;
	void AdjustTreeCollision(const TVector3d& pos, TVector3d *vel) const;
	static void CheckItemCollection(const TVector3d& pos);

	TVector3d CalcRollNormal(double speed);
	TVector3d CalcAirForce();
	TVector3d CalcSpringForce();
	TVector3d CalcNormalForce();
	TVector3d CalcJumpForce();
	TVector3d CalcFrictionForce(double speed, const TVector3d& nmlforce);
	TVector3d CalcPaddleForce(double speed);
	TVector3d CalcBrakeForce(double speed);
	TVector3d CalcGravitationForce();
	TVector3d CalcNetForce(const TVector3d& pos, const TVector3d& vel);

	void     AdjustVelocity();
	void     AdjustPosition(const TPlane& surf_plane, double dist_from_surface);
	void     SetTuxPosition(double speed);
	double   AdjustTimeStep(double h, const TVector3d& vel);
	void     SolveOdeSystem(double timestep);
public:
	CControl();

	// view:
	TVector3d viewpos;
	TVector3d plyr_pos;
	TVector3d viewdir;
	TVector3d viewup;
	TMatrix<4, 4> view_mat;
	TViewMode viewmode;
	bool view_init;

	// main:
	TVector3d cpos;
	TVector3d cvel;
	TVector3d last_pos;
	TVector3d cnet_force;
	TVector3d cdirection;
	TQuaternion corientation;
	double way;

	TVector3d plane_nml;

	// steering:
	double turn_fact;
	double turn_animation;
	double paddle_time;
	double jump_amt;
	double jump_start_time;
	bool   is_paddling;
	bool   is_braking;
	bool   begin_jump;
	bool   jumping;
	bool   jump_charging;
	bool   orientation_initialized;

	// trick:
	bool   front_flip;
	bool   back_flip;
	bool   cairborne;
	bool   roll_left;
	bool   roll_right;
	double roll_factor;
	double flip_factor;

	// pseudo constants:
	double minSpeed;
	double minFrictspeed;

	void Init();
	void UpdatePlayerPos(float timestep);
};

#endif
