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

#ifndef PHYSICS_H
#define PHYSICS_H

#include "bh.h"

#define EARTH_GRAV 9.81 
#define JUMP_FORCE_DURATION 0.20
#define MAX_PADDLING_SPEED (60.0 / 3.6) 
#define TUX_MASS 20				
#define MIN_TUX_SPEED 1.4 
#define INIT_TUX_SPEED 3.0
#define COLL_TOLERANCE 0.1		
#define MAG_SQD(vec) ((vec).x * (vec).x + \
       (vec).y * (vec).y + (vec).z * (vec).z )

#define MAX_SURF_PEN 0.2		
#define TUX_Y_CORR 0.36			
#define IDEAL_ROLL_SPEED 6.0	
#define IDEAL_ROLL_FRIC 0.35	
#define WIND_FACTOR 1.5
#define AIR_DENSITY 1.308
#define AIR_VISCOSITY 17.00e-6

#define TG1_COMPRESS 0.05	// TG = Tux Glute
#define TG2_COMPRESS 0.12		
#define TG1_SPRING 1500			
#define TG2_SPRING 3000			
#define TG3_SPRING 10000		
#define TG1_Damp 100			
#define TG2_DAMP 500			
#define TG3_DAMP 1000			
#define TG_MAX_SPRING 3000		

#define MIN_FRICT_SPEED 2.8		
#define MAX_FRICT_FORCE 800		
#define BASE_JUMP_G_FORCE 1.5
#define MAX_JUMP_G_FORCE 3
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

class CControl {
private:
	double ode_time_step;

public:
	CControl ();
	~CControl ();
	
	// view:
	TViewMode viewmode;                
    TVector3 viewpos;                     
    TVector3 plyr_pos;                
    TVector3 viewdir;                    
    TVector3 viewup;                     
    TMatrix view_mat;         
    bool view_init;              
	// main:
	TVector3 cpos;          
    TVector3 cvel;                    
    TVector3 cnet_force;        
    TVector3 cdirection;        
	TQuaternion corientation;        
    bool orientation_initialized;  
    TVector3 plane_nml;              
	// steering:
    double turn_fact;              
    double turn_animation;         
	double paddle_time;
    bool   is_paddling;              
    bool   is_braking;               
    bool   begin_jump;
    bool   jumping;
    bool   jump_charging;
    double jump_amt;
    double jump_start_time;
	// trick:
    double roll_factor;
    double flip_factor;
    bool   front_flip;
    bool   back_flip;
    bool   airborne;           
    bool   roll_left;
    bool   roll_right;

	void Init ();
	bool CheckTreeCollisions (TVector3 pos, TVector3 *tree_loc, double *tree_diam);
	void AdjustTreeCollision (TVector3 pos, TVector3 *vel);
	void CheckItemCollection (TVector3 pos);
	double AdjustVelocity (TVector3 *vel, TVector3 pos, TPlane surf_plane, 
		double dist_from_surface);
	void AdjustPosition (TVector3 *pos, TPlane surf_plane, double dist_from_surface);
	void SetTuxPosition (TVector3 new_pos);
	TVector3 AdjustRollNormal (TVector3 vel, double fric_coeff, TVector3 nml);
	TVector3 CalcAirForce (TVector3 player_vel);
	TVector3 CalcSpringForce (double compression, TVector3 vel, TVector3 surf_nml);
	TVector3 CalcNetForce (TVector3 pos, TVector3 vel);
	double adjust_time_step_size (double h, TVector3 vel);
	void SolveOdeSystem (double timestep);
	void UpdatePlayerPos (double timestep);
};

// STRUCTURE OF PHYSICS.CPP

// ------------------- init -------------------------------------------
// void InitSimulation ()

// ----------------- collision ----------------------------------------
// bool  CheckTreeCollisions (TVector3 pos, TVector3 *tree_loc, double *tree_diam)
// void  AdjustTreeCollision (TVector3 pos, TVector3 *vel)
// void  CheckItemCollection (TVector3 pos)

// ----------------- position and vlelocity ---------------------------
// double AdjustVelocity (TVector3 *vel, TVector3 pos, TPlane surf_plane, 
// 	   double dist_from_surface)
// void   AdjustPosition (TVector3 *pos, TPlane surf_plane, double dist_from_surface)
// void   SetTuxPosition (TVector3 new_pos)

// ----------------- forces -------------------------------------------
// TVector3 AdjustRollNormal (TVector3 vel, double fric_coeff, TVector3 nml)
// TVector3 CalcAirForce (TVector3 player_vel)
// TVector3 CalcSpringForce (double compression, TVector3 vel, TVector3 surf_nml)
// TVector3 CalcNetForce (TVector3 pos, TVector3 vel)

// ------------------ ODE solver and port function
// static double adjust_time_step_size (double h, TVector3 vel)
// void   SolveOdeSystem (double timestep)
// void   UpdatePlayerPos (double timestep)

#endif
