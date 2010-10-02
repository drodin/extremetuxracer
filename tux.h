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

#ifndef TUX_H
#define TUX_H

#include "bh.h"

typedef struct {
    TColor diffuse;
    TColor specular;
    float exp;
} TTuxMaterial2;

typedef struct NodeStruct2 {
    struct NodeStruct2 *parent;
    struct NodeStruct2 *next;
    struct NodeStruct2 *child;
    char *name;
	bool visible;
    TMatrix trans;
	TMatrix invtrans;   
	double radius;
	int divisions;
    TTuxMaterial2 *mat;
    bool render_shadow;
} TCharNode2;

bool GetCharNode2 (const char *node_name, TCharNode2 **node);
bool TranslateCharNode2 (const char *node_name, TVector3 vec);
void ScaleCharNode2 (const char *node_name, TVector3 vec);
bool RotateCharNode2 (const char *node_name, double axis, double angle);
bool ResetCharNode2 (const char *node_name);
bool TransformCharNode2 (const char *node_name, TMatrix mat, TMatrix invmat);
void ResetTuxJoints2 ();
void ResetTuxRoot2 ();

void AdjustOrientation2 (CControl *ctrl, double dtime, 
		double dist_from_surface, TVector3 surf_nml);
void AdjustTuxJoints2 (double turnFact, bool isBraking, double paddling_factor, 
		double speed, TVector3 net_force, double flap_factor);

void LoadTux2 ();	
void DrawTux2 ();
void DrawTuxShadow2 ();

bool CheckCollision2 (const char *node_name, TPolyhedron ph);
bool TuxCollision2 (TVector3 pos, TPolyhedron ph);

#endif
