/* 
 * ETRacer 
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

#ifndef _MODEL_HNDL_H_
#define _MODEL_HNDL_H_

#include "etracer.h"
#include "ppgltk/ppgltk.h"
#include <list>
#include <string>

typedef struct {
	int id;
	std::string name;
	std::string filename;
} model_t;

class model_hndl {
//Class for handling models
	
public:
	model_hndl();
	std::list<model_t> l_models;
	int num_models;
	int cur_model;
	
	static bool   	  tuxLoaded;
	static char*     tuxRootNode;
	static char*     tuxLeftShoulderJoint;
	static char*     tuxRightShoulderJoint;
	static char*     tuxLeftHipJoint;
	static char*     tuxRightHipJoint;
	static char*     tuxLeftKneeJoint;
	static char*     tuxRightKneeJoint;
	static char*     tuxLeftAnkleJoint;
	static char*     tuxRightAnkleJoint;
	static char*     tuxTailJoint;
	static char*     tuxNeck;
	static char*     tuxHead;
	
	void adjust_tux_joints( double turnFact, bool isBraking, 
				double paddling_factor, double speed,
				pp::Vec3d net_force, double jump_factor );
				
			
	void load_model(int model);
	void load_model();
	void init_models();
	void draw_tux();
	char* get_tux_root_node() { return tuxRootNode; } 
	char* get_tux_left_shoulder_joint() { return tuxLeftShoulderJoint; } 
	char* get_tux_right_shoulder_joint() { return tuxRightShoulderJoint; } 
	char* get_tux_left_hip_joint() { return tuxLeftHipJoint; } 
	char* get_tux_right_hip_joint() { return tuxRightHipJoint; } 
	char* get_tux_left_knee_joint() { return tuxLeftKneeJoint; } 
	char* get_tux_right_knee_joint() { return tuxRightKneeJoint; } 
	char* get_tux_left_ankle_joint() { return tuxLeftAnkleJoint; } 
	char* get_tux_right_ankle_joint() { return tuxRightAnkleJoint; } 
	char* get_tux_tail_joint() { return tuxTailJoint; } 
	char* get_tux_neck() { return tuxNeck; } 
	char* get_tux_head() { return tuxHead; } 

};

static void register_tux_callbacks( Tcl_Interp *ip );

extern model_hndl* ModelHndl;

#endif /* _MODEL_HNDL_H_ */
