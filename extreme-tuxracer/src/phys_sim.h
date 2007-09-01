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

#ifndef _PHYS_SIM_H_
#define _PHYS_SIM_H_

#include "course_load.h"

#include "player.h"

#include "ppgltk/alg/plane.h"

/* Acceleration due to gravity m/s^2 */
#define EARTH_GRAV 9.81

#define M_PER_SEC_TO_KM_PER_H 3.6

#define KM_PER_H_TO_M_PER_SEC ( 1.0 / M_PER_SEC_TO_KM_PER_H )

/* Speed at which paddling ceases to be effective (m/s) */
#define MAX_PADDLING_SPEED ( 60.0 * KM_PER_H_TO_M_PER_SEC ) 

/* Time over which constant jump force is applied */
#define JUMP_FORCE_DURATION 0.20

void 	set_wind_velocity(pp::Vec3d velocity, float scale);
void      increment_turn_fact( Player& plyr, float amt );
//void      set_gravity( bool low );
void      set_friction_coeff( const float fric[3] ); 
//void      set_friction( const bool useFriction );
float  get_min_y_coord();
pp::Vec3d  find_course_normal( const float x, const float z );
float  find_y_coord( float x, float z );
void      get_surface_type( float x, float z, float weights[] );
pp::Plane   get_local_course_plane( pp::Vec3d pt );
float  get_compression_depth( const int terrain ); 
void      set_tux_pos( Player& plyr, pp::Vec3d newPos );
void      update_player_pos( Player& plyr, float dtime );
void      init_physical_simulation();

#endif // _PHYS_SIM_H_
