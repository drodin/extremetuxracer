/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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

#ifndef _SNOW_H_
#define _SNOW_H_

#include "ppgltk/ppgltk.h"

#include "player.h"

static double xrand (double min, double max);

void UpdateArea(pp::Vec3d pos);
static void MakeSnowParticle (int i);
static void MakeNearParticle (int i);
 
void init_snow( pp::Vec3d playerPos);
void update_snow( double time_step, bool windy, pp::Vec3d playerPos );
void draw_snow( pp::Vec3d eyepoint );

void draw_sprite( pp::Vec3d eyepoint, pp::Vec3d spriteLoc, double spriteSize, pp::Vec2d tex_min, pp::Vec2d tex_max );
void draw_cuboid_areas();

typedef struct {
    double speed;
    double minSize; 
    double maxSize;
    int MAXPART;
    int MAXNEAR;
} SnowType;

static const int SnowTypeArgCount = 5;

void reset_snow();
void RegisterSnowType(int index, SnowType type);
void SetSnowType(int index);

#endif // _SNOW_H_
