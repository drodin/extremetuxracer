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


This module has been completely rewritten. Remember that the way of
defining the character has radically changed though the character is
still shaped with spheres.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "tux.h"
#include "ogl.h"
#include "spx.h"
#include "textures.h"
#include "course.h"
#include "physics.h"

#define MAX_ARM_ANGLE2 30.0
#define MAX_PADDLING_ANGLE2 35.0
#define MAX_EXT_PADDLING_ANGLE2 30.0
#define MAX_KICK_PADDLING_ANGLE2 20.0
#define TO_AIR_TIME 0.5
#define TO_TIME 0.14

#define SHADOW_HEIGHT 0.03 // ->0.05

#ifdef USE_STENCIL_BUFFER
	static const TColor shad_col(0.0, 0.0, 0.0, 0.3);
#else
	static const TColor shad_col(0.0, 0.0, 0.0, 0.1);
#endif

static const TCharMaterial TuxDefMat = {TColor(0.5, 0.5, 0.5, 1.0), TColor(0.0, 0.0, 0.0, 1.0), 0.0};
static const TCharMaterial Highlight = {TColor(0.8, 0.15, 0.15, 1.0), TColor(0.0, 0.0, 0.0, 1.0), 0.0};
CCharShape TestChar;

CCharShape::CCharShape () {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		Nodes[i] = NULL;
		Index[i] = -1;
	}
	numNodes = 0;

	useActions = false;
	newActions = false;
	useMaterials = true;
	useHighlighting = false;
	highlighted = false;
	highlight_node = -1;
}

CCharShape::~CCharShape() {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		if (Nodes[i] != NULL) {
			delete Nodes[i]->action;
			delete Nodes[i];
		}
	}
}

// --------------------------------------------------------------------
//				nodes
// --------------------------------------------------------------------

size_t CCharShape::GetNodeIdx (size_t node_name) const {
	if (node_name >= MAX_CHAR_NODES) return -1;
	return Index[node_name];
}

TCharNode *CCharShape::GetNode (size_t node_name) {
	size_t idx = GetNodeIdx (node_name);
	if (idx >= numNodes) return NULL;
	return Nodes[idx];
}

void CCharShape::CreateRootNode () {
    TCharNode *node = new TCharNode;
	node->node_name = 0;
	node->parent = NULL;
	node->parent_name = 99;
    node->next = NULL;
	node->next_name = 99;
    node->child = NULL;
	node->child_name = 99;
    node->mat = NULL;
    node->joint = "root";
    node->render_shadow = false;
	node->visible = false;
	node->action = NULL;
    MakeIdentityMatrix (node->trans);
	MakeIdentityMatrix (node->invtrans);

	NodeIndex.clear();
	NodeIndex["root"] = 0;
	Index[0] = 0;
	Nodes[0] = node;
	numNodes = 1;
}

bool CCharShape::CreateCharNode
		(int parent_name, size_t node_name, const string& joint,
		const string& name, const string& order, bool shadow) {

	TCharNode *parent = GetNode (parent_name);
	if (parent == NULL) {
		Message ("wrong parent node");
		return false;
	}
    TCharNode *node = new TCharNode;
	node->node_name = node_name;
    node->parent = parent;
	node->parent_name = parent_name;
    node->next  = NULL;
	node->next_name = 99;
    node->child = NULL;
	node->child_name = 99;

	if (useActions) {
		node->action = new TCharAction;
		node->action->num = 0;
		node->action->name = name;
		node->action->order = order;
		node->action->mat = "";
	} else
		node->action = NULL;

    node->mat   = NULL;
	node->node_idx = numNodes;
	node->visible = false;
	node->render_shadow = shadow;
    node->joint = joint;

    MakeIdentityMatrix (node->trans);
    MakeIdentityMatrix (node->invtrans);

	if (!joint.empty()) NodeIndex[joint] = node_name;
	Nodes[numNodes] = node;
	Index[node_name] = numNodes;

/// -------------------------------------------------------------------
	if (parent->child == NULL) {
		parent->child = node;
		parent->child_name = node_name;
	} else {
		for (parent = parent->child; parent->next != NULL; parent = parent->next) {}
		parent->next = node;
		parent->next_name = node_name;
	}
/// -------------------------------------------------------------------

	numNodes++;
    return true;
}

void CCharShape::AddAction (size_t node_name, int type, const TVector3& vec, double val) {
	size_t idx = GetNodeIdx (node_name);
	TCharAction *act = Nodes[idx]->action;
	act->type[act->num] = type;
	act->vec[act->num] = vec;
	act->dval[act->num] = val;
	act->num++;
}

bool CCharShape::TranslateNode (size_t node_name, const TVector3& vec) {
	TCharNode *node = GetNode (node_name);
	if (node == NULL) return false;

    TMatrix TransMatrix;

    MakeTranslationMatrix (TransMatrix, vec.x, vec.y, vec.z);
    MultiplyMatrices (node->trans, node->trans, TransMatrix);
	MakeTranslationMatrix (TransMatrix, -vec.x, -vec.y, -vec.z);
	MultiplyMatrices (node->invtrans, TransMatrix, node->invtrans);

	if (newActions && useActions) AddAction (node_name, 0, vec, 0);
    return true;
}

bool CCharShape::RotateNode (size_t node_name, int axis, double angle) {
	TCharNode *node = GetNode (node_name);
	if (node == NULL) return false;

	if (axis > 3) return false;

    TMatrix rotMatrix;
	char caxis = '0';
	switch (axis) {
		case 1: caxis = 'x'; break;
		case 2: caxis = 'y'; break;
		case 3: caxis = 'z'; break;
	}

    MakeRotationMatrix (rotMatrix, angle, caxis);
    MultiplyMatrices (node->trans, node->trans, rotMatrix);
	MakeRotationMatrix (rotMatrix, -angle, caxis);
	MultiplyMatrices (node->invtrans, rotMatrix, node->invtrans);

	if (newActions && useActions) AddAction (node_name, axis, NullVec, angle);
    return true;
}

bool CCharShape::RotateNode (const string& node_trivialname, int axis, double angle) {
	map<string, size_t>::const_iterator i = NodeIndex.find(node_trivialname);
	if (i == NodeIndex.end()) return false;
	return RotateNode (i->second, axis, angle);
}

void CCharShape::ScaleNode (size_t node_name, const TVector3& vec) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return;

    TMatrix matrix;

	MakeIdentityMatrix (matrix);
    MultiplyMatrices (node->trans, node->trans, matrix);
	MakeIdentityMatrix (matrix);
	MultiplyMatrices (node->invtrans, matrix, node->invtrans);

    MakeScalingMatrix (matrix, vec.x, vec.y, vec.z);
    MultiplyMatrices (node->trans, node->trans, matrix);
	MakeScalingMatrix (matrix, 1.0 / vec.x, 1.0 / vec.y, 1.0 / vec.z);
	MultiplyMatrices (node->invtrans, matrix, node->invtrans);

	MakeIdentityMatrix (matrix);
    MultiplyMatrices (node->trans, node->trans, matrix);
	MakeIdentityMatrix (matrix);
	MultiplyMatrices (node->invtrans, matrix, node->invtrans);

	if (newActions && useActions) AddAction (node_name, 4, vec, 0);
}

bool CCharShape::VisibleNode (size_t node_name, float level) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

	node->visible = (level > 0);

	if (node->visible) {
    	node->divisions =
			MIN (MAX_SPHERE_DIV, MAX (MIN_SPHERE_DIV,
			ROUND_TO_NEAREST (param.tux_sphere_divisions * level / 10)));
	    node->radius = 1.0;
	}
	if (newActions && useActions) AddAction (node_name, 5, NullVec, level);
	return true;
}

bool CCharShape::MaterialNode (size_t node_name, const string& mat_name) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;
	TCharMaterial *mat = GetMaterial(mat_name);
	if (mat == NULL) return false;
	node->mat = mat;
	if (newActions && useActions) node->action->mat = mat_name;
	return true;
}

bool CCharShape::ResetNode (size_t node_name) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

    MakeIdentityMatrix (node->trans);
    MakeIdentityMatrix (node->invtrans);
	return true;
}

bool CCharShape::ResetNode (const string& node_trivialname) {
	map<string, size_t>::const_iterator i = NodeIndex.find(node_trivialname);
	if (i == NodeIndex.end()) return false;
	return ResetNode (i->second);
}

bool CCharShape::TransformNode (size_t node_name, const TMatrix mat, const TMatrix invmat) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

    MultiplyMatrices (node->trans, node->trans, mat);
	MultiplyMatrices (node->invtrans, invmat, node->invtrans);
    return true;
}

void CCharShape::ResetRoot () {
	ResetNode (0);
}

void CCharShape::ResetJoints () {
    ResetNode ("left_shldr");
    ResetNode ("right_shldr");
    ResetNode ("left_hip");
    ResetNode ("right_hip");
    ResetNode ("left_knee");
    ResetNode ("right_knee");
    ResetNode ("left_ankle");
    ResetNode ("right_ankle");
    ResetNode ("tail");
    ResetNode ("neck");
    ResetNode ("head");
}

void CCharShape::Reset () {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		if (Nodes[i] != NULL) {
			delete Nodes[i]->action;
			delete Nodes[i];
			Nodes[i] = NULL;
		}
		Index[i] = -1;
	}
	Materials.clear();
	NodeIndex.clear();
	MaterialIndex.clear();
	numNodes = 0;

	useActions = true;
	newActions = false;
	useMaterials = true;
	useHighlighting = false;
	highlighted = false;
	highlight_node = -1;
}

// --------------------------------------------------------------------
//				materials
// --------------------------------------------------------------------

TCharMaterial* CCharShape::GetMaterial (const string& mat_name) {
	map<string, size_t>::const_iterator i = MaterialIndex.find(mat_name);
	if (i != MaterialIndex.end() && i->second < Materials.size()) {
		return &Materials[i->second];
	}
	return NULL;
}

void CCharShape::CreateMaterial (const string& line) {
	TVector3 diff = SPVector3N (line, "diff", TVector3 (0,0,0));
	TVector3 spec = SPVector3N (line, "spec", TVector3 (0,0,0));
	float exp = SPFloatN (line, "exp", 50);
	std::string mat = SPItemN (line, "mat");
	STrimN(mat);

	Materials.push_back(TCharMaterial());
    Materials.back().diffuse.r = diff.x;
    Materials.back().diffuse.g = diff.y;
    Materials.back().diffuse.b = diff.z;
    Materials.back().diffuse.a = 1.0;
    Materials.back().specular.r = spec.x;
    Materials.back().specular.g = spec.y;
    Materials.back().specular.b = spec.z;
    Materials.back().specular.a = 1.0;
    Materials.back().exp = exp;
	if(useActions)
		Materials.back().matline = line;

	MaterialIndex[mat] = Materials.size()-1;
}

// --------------------------------------------------------------------
//				drawing
// --------------------------------------------------------------------

void CCharShape::DrawCharSphere (int num_divisions) {
    GLUquadricObj *qobj = gluNewQuadric();
    gluQuadricDrawStyle (qobj, GLU_FILL);
    gluQuadricOrientation (qobj, GLU_OUTSIDE);
    gluQuadricNormals (qobj, GLU_SMOOTH);
    gluSphere (qobj, 1.0, (GLint)2.0 * num_divisions, num_divisions);
    gluDeleteQuadric (qobj);
}

void CCharShape::DrawNodes (const TCharNode *node) {
    glPushMatrix();
    glMultMatrixd ((double *) node->trans);

	if (node->node_name == highlight_node) highlighted = true;
	const TCharMaterial *mat;
	if (highlighted && useHighlighting) {
		mat = &Highlight;
	} else {
		if (node->mat != NULL && useMaterials) mat = node->mat;
		else mat = &TuxDefMat;
	}

    if (node->visible == true) {
		set_material (mat->diffuse, mat->specular, mat->exp);

		DrawCharSphere (node->divisions);
    }
// -------------- recursive loop -------------------------------------
    TCharNode *child = node->child;
    while (child != NULL) {
        DrawNodes (child);
		if (child->node_name == highlight_node) highlighted = false;
        child = child->next;
    }
// -------------------------------------------------------------------
    glPopMatrix();
}

void CCharShape::Draw () {
    static const float dummy_color[] = {0.0, 0.0, 0.0, 1.0};

    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dummy_color);
	ScopedRenderMode rm(TUX);
	glEnable (GL_NORMALIZE);

	TCharNode *node = GetNode(0);
	if (node == NULL) return;

	DrawNodes (node);
	glDisable (GL_NORMALIZE);
	if (param.perf_level > 2 && g_game.argument == 0) DrawShadow ();
	highlighted = false;
}

// --------------------------------------------------------------------

bool CCharShape::Load (const string& dir, const string& filename, bool with_actions) {
	CSPList list (500);

	useActions = with_actions;
	CreateRootNode ();
	newActions = true;

	if (!list.Load (dir, filename)) {
		Message ("could not load character", filename);
		return false;
	}

	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		int node_name = SPIntN (line, "node", -1);
		int parent_name = SPIntN (line, "par", -1);
		string mat_name = SPStrN (line, "mat", "");
		string name = SPStrN (line, "joint", "");
		string fullname = SPStrN (line, "name", "");

		if (SPIntN (line, "material", 0) > 0) {
			CreateMaterial (line);
		} else {
			double visible = SPFloatN (line, "vis", -1.0);
			bool shadow = SPBoolN (line, "shad", false);
			string order = SPStrN (line, "order", "");
			CreateCharNode (parent_name, node_name, name, fullname, order, shadow);
			TVector3 rot = SPVector3N (line, "rot", NullVec);
			MaterialNode (node_name, mat_name);
			for (size_t ii = 0; ii < order.size(); ii++) {
				int act = order[ii]-48;
				switch (act) {
					case 0: {
						TVector3 trans = SPVector3N (line, "trans", TVector3 (0,0,0));
						TranslateNode (node_name, trans);
						break;
					}
					case 1: RotateNode (node_name, 1, rot.x); break;
					case 2: RotateNode (node_name, 2, rot.y); break;
					case 3: RotateNode (node_name, 3, rot.z); break;
					case 4: {
						TVector3 scale = SPVector3N (line, "scale", TVector3 (1,1,1));
						ScaleNode (node_name, scale);
						break;
					}
					case 5: VisibleNode (node_name, visible); break;
					case 9: RotateNode (node_name, 2, rot.z); break;
					default: break;
				}
			}
		}
	}
	newActions = false;
	return true;
}

TVector3 CCharShape::AdjustRollvector (const CControl *ctrl, TVector3 vel, const TVector3& zvec) {
    TMatrix rot_mat;
    vel = ProjectToPlane (zvec, vel);
    NormVector (vel);
    if (ctrl->is_braking) {
		RotateAboutVectorMatrix (rot_mat, vel, ctrl->turn_fact * BRAKING_ROLL_ANGLE);
    } else {
		RotateAboutVectorMatrix (rot_mat, vel, ctrl->turn_fact * MAX_ROLL_ANGLE);
    }
    return TransformVector (rot_mat, zvec);
}

void CCharShape::AdjustOrientation (CControl *ctrl, double dtime,
		 double dist_from_surface, const TVector3& surf_nml) {
    TVector3 new_y, new_z;
    TMatrix cob_mat, inv_cob_mat;
    TMatrix rot_mat;
    static const TVector3 minus_z_vec(0, 0, -1);
    static const TVector3 y_vec(0, 1, 0);

    if (dist_from_surface > 0) {
		new_y = ScaleVector (1, ctrl->cvel);
		NormVector (new_y);
		new_z = ProjectToPlane (new_y, TVector3(0, -1, 0));
		NormVector (new_z);
		new_z = AdjustRollvector (ctrl, ctrl->cvel, new_z);
    } else {
		new_z = ScaleVector (-1, surf_nml);
		new_z = AdjustRollvector (ctrl, ctrl->cvel, new_z);
		new_y = ProjectToPlane (surf_nml, ScaleVector (1, ctrl->cvel));
		NormVector(new_y);
    }

    TVector3 new_x = CrossProduct (new_y, new_z);
    MakeBasismatrix_Inv (cob_mat, inv_cob_mat, new_x, new_y, new_z);
    TQuaternion new_orient = MakeQuaternionFromMatrix (cob_mat);

    if (!ctrl->orientation_initialized) {
		ctrl->orientation_initialized = true;
		ctrl->corientation = new_orient;
    }

    double time_constant = dist_from_surface > 0 ? TO_AIR_TIME : TO_TIME;

    ctrl->corientation = InterpolateQuaternions (
			ctrl->corientation, new_orient,
			min (dtime / time_constant, 1.0));

    ctrl->plane_nml = RotateVector (ctrl->corientation, minus_z_vec);
    ctrl->cdirection = RotateVector (ctrl->corientation, y_vec);
    MakeMatrixFromQuaternion (cob_mat, ctrl->corientation);

    // Trick rotations
    new_y = TVector3 (cob_mat[1][0], cob_mat[1][1], cob_mat[1][2]);
    RotateAboutVectorMatrix (rot_mat, new_y, (ctrl->roll_factor * 360));
    MultiplyMatrices (cob_mat, rot_mat, cob_mat);
    new_x = TVector3 (cob_mat[0][0], cob_mat[0][1], cob_mat[0][2]);
    RotateAboutVectorMatrix (rot_mat, new_x, ctrl->flip_factor * 360);
    MultiplyMatrices (cob_mat, rot_mat, cob_mat);

	TransposeMatrix (cob_mat, inv_cob_mat);
	TransformNode (0, cob_mat, inv_cob_mat);
}

void CCharShape::AdjustJoints (double turnFact, bool isBraking,
			double paddling_factor, double speed,
			const TVector3& net_force, double flap_factor) {
    double turning_angle[2];
    double paddling_angle = 0;
    double ext_paddling_angle = 0;
    double kick_paddling_angle = 0;
    double braking_angle = 0;
    double force_angle = 0;
    double turn_leg_angle = 0;
    double flap_angle = 0;

    if (isBraking) braking_angle = MAX_ARM_ANGLE2;

    paddling_angle = MAX_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
    ext_paddling_angle = MAX_EXT_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
    kick_paddling_angle = MAX_KICK_PADDLING_ANGLE2 * sin(paddling_factor * M_PI * 2.0);

    turning_angle[0] = MAX(-turnFact,0.0) * MAX_ARM_ANGLE2;
    turning_angle[1] = MAX(turnFact,0.0) * MAX_ARM_ANGLE2;
    flap_angle = MAX_ARM_ANGLE2 * (0.5 + 0.5 * sin (M_PI * flap_factor * 6 - M_PI / 2));
    force_angle = max (-20.0, min (20.0, -net_force.z / 300.0));
    turn_leg_angle = turnFact * 10;

	ResetJoints ();

    RotateNode ("left_shldr", 3,
		    MIN (braking_angle + paddling_angle + turning_angle[0], MAX_ARM_ANGLE2) + flap_angle);
    RotateNode ("right_shldr", 3,
		    MIN (braking_angle + paddling_angle + turning_angle[1], MAX_ARM_ANGLE2) + flap_angle);

    RotateNode ("left_shldr", 2, -ext_paddling_angle);
    RotateNode ("right_shldr", 2, ext_paddling_angle);
    RotateNode ("left_hip", 3, -20 + turn_leg_angle + force_angle);
    RotateNode ("right_hip", 3, -20 - turn_leg_angle + force_angle);

    RotateNode ("left_knee", 3,
		-10 + turn_leg_angle - MIN (35, speed) + kick_paddling_angle + force_angle);
    RotateNode ("right_knee", 3,
		-10 - turn_leg_angle - MIN (35, speed) - kick_paddling_angle + force_angle);

    RotateNode ("left_ankle", 3, -20 + MIN (50, speed));
    RotateNode ("right_ankle", 3, -20 + MIN (50, speed));
    RotateNode ("tail", 3, turnFact * 20);
    RotateNode ("neck", 3, -50);
    RotateNode ("head", 3, -30);
    RotateNode ("head", 2, -turnFact * 70);
}

// --------------------------------------------------------------------
//				collision
// --------------------------------------------------------------------

bool CCharShape::CheckPolyhedronCollision (const TCharNode *node, const TMatrix modelMatrix,
		const TMatrix invModelMatrix, const TPolyhedron& ph) {

    TMatrix newModelMatrix, newInvModelMatrix;
    bool hit = false;

    MultiplyMatrices (newModelMatrix, modelMatrix, node->trans);
    MultiplyMatrices (newInvModelMatrix, node->invtrans, invModelMatrix);

    if (node->visible) {
        TPolyhedron newph = CopyPolyhedron (ph);
        TransPolyhedron (newInvModelMatrix, newph);
        hit = IntersectPolyhedron (newph);
        FreePolyhedron (newph);
    }

    if (hit == true) return hit;
    const TCharNode *child = node->child;
    while (child != NULL) {
        hit = CheckPolyhedronCollision (child, newModelMatrix, newInvModelMatrix, ph);
        if (hit == true) return hit;
        child = child->next;
    }
    return false;
}

bool CCharShape::CheckCollision (const TPolyhedron& ph) {
    TMatrix mat, invmat;

    MakeIdentityMatrix (mat);
    MakeIdentityMatrix (invmat);

	TCharNode *node = GetNode(0);
	if (node == NULL) return false;
    return CheckPolyhedronCollision (node, mat, invmat, ph);
}

bool CCharShape::Collision (const TVector3& pos, const TPolyhedron& ph) {
	ResetNode (0);
	TranslateNode (0, TVector3 (pos.x, pos.y, pos.z));
	return CheckCollision (ph);
}

// --------------------------------------------------------------------
//				shadow
// --------------------------------------------------------------------

void CCharShape::DrawShadowVertex (double x, double y, double z, const TMatrix mat) {
    TVector3 pt(x, y, z);
    pt = TransformPoint (mat, pt);
    double old_y = pt.y;
    TVector3 nml = Course.FindCourseNormal (pt.x, pt.z);
    pt.y = Course.FindYCoord (pt.x, pt.z) + SHADOW_HEIGHT;
    if (pt.y > old_y) pt.y = old_y;
    glNormal3f (nml.x, nml.y, nml.z);
    glVertex3f (pt.x, pt.y, pt.z);
}

void CCharShape::DrawShadowSphere (const TMatrix mat) {
    double theta, phi, d_theta, d_phi, eps, twopi;
    double x, y, z;
    int div = param.tux_shadow_sphere_divisions;

    eps = 1e-15;
    twopi = M_PI * 2.0;
    d_theta = d_phi = M_PI / div;

    for (phi = 0.0; phi + eps < M_PI; phi += d_phi) {
		double cos_theta, sin_theta;
		double sin_phi, cos_phi;
		double sin_phi_d_phi, cos_phi_d_phi;

		sin_phi = sin (phi);
		cos_phi = cos (phi);
		sin_phi_d_phi = sin (phi + d_phi);
		cos_phi_d_phi = cos (phi + d_phi);

        if (phi <= eps) {
			glBegin (GL_TRIANGLE_FAN);
				DrawShadowVertex (0., 0., 1., mat);

				for (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);

					x = cos_theta * sin_phi_d_phi;
					y = sin_theta * sin_phi_d_phi;
					z = cos_phi_d_phi;
					DrawShadowVertex (x, y, z, mat);
				}
				x = sin_phi_d_phi;
				y = 0.0;
				z = cos_phi_d_phi;
				DrawShadowVertex (x, y, z, mat);
            glEnd();
        } else if (phi + d_phi + eps >= M_PI) {
			glBegin (GL_TRIANGLE_FAN);
				DrawShadowVertex (0., 0., -1., mat);
                for (theta = twopi; theta - eps > 0; theta -= d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
                    x = cos_theta * sin_phi;
                    y = sin_theta * sin_phi;
                    z = cos_phi;
					DrawShadowVertex (x, y, z, mat);
                }
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
				DrawShadowVertex (x, y, z, mat);
            glEnd();
        } else {
            glBegin (GL_TRIANGLE_STRIP);
				for (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
					x = cos_theta * sin_phi;
					y = sin_theta * sin_phi;
					z = cos_phi;
					DrawShadowVertex (x, y, z, mat);

					x = cos_theta * sin_phi_d_phi;
					y = sin_theta * sin_phi_d_phi;
					z = cos_phi_d_phi;
					DrawShadowVertex (x, y, z, mat);
                }
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
				DrawShadowVertex (x, y, z, mat);
                x = sin_phi_d_phi;
                y = 0.0;
                z = cos_phi_d_phi;
				DrawShadowVertex (x, y, z, mat);
            glEnd();
        }
    }
}

void CCharShape::TraverseDagForShadow (const TCharNode *node, const TMatrix mat) {
    TMatrix new_matrix;

    MultiplyMatrices (new_matrix, mat, node->trans);
	if (node->visible && node->render_shadow)
		DrawShadowSphere (new_matrix);

    TCharNode* child = node->child;
    while (child != NULL) {
        TraverseDagForShadow (child, new_matrix);
        child = child->next;
    }
}

void CCharShape::DrawShadow () {
    TMatrix model_matrix;

	if (g_game.light_id == 1 || g_game.light_id == 3) return;

    ScopedRenderMode rm(TUX_SHADOW);
    glColor4f (shad_col.r, shad_col.g, shad_col.b, shad_col.a);
    MakeIdentityMatrix (model_matrix);

	TCharNode *node = GetNode(0);
	if (node == NULL) {
		Message ("couldn't find tux's root node", "");
		return;
    }
    TraverseDagForShadow (node, model_matrix);
}

// --------------------------------------------------------------------
//				testing and tools
// --------------------------------------------------------------------

string CCharShape::GetNodeJoint (size_t idx) const {
	if (idx >= numNodes) return "";
	TCharNode *node = Nodes[idx];
	if (node == NULL) return "";
	if (!node->joint.empty()) return node->joint;
	else return Int_StrN ((int)node->node_name);
}

size_t CCharShape::GetNodeName (size_t idx) const {
	if (idx >= numNodes) return -1;
	return Nodes[idx]->node_name;
}

size_t CCharShape::GetNodeName (const string& node_trivialname) const {
	return NodeIndex.at(node_trivialname);
}


void CCharShape::RefreshNode (size_t idx) {
	if (idx >= numNodes) return;
    TMatrix TempMatrix;
	char caxis;
	double angle;

	TCharNode *node = Nodes[idx];
	TCharAction *act = node->action;
	if (act == NULL) return;
	if (act->num < 1) return;

	MakeIdentityMatrix (node->trans);
	MakeIdentityMatrix (node->invtrans);

	for (size_t i=0; i<act->num; i++) {
		int type = act->type[i];
		const TVector3& vec = act->vec[i];
		double dval = act->dval[i];

		switch (type) {
			case 0:
				MakeTranslationMatrix (TempMatrix, vec.x, vec.y, vec.z);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeTranslationMatrix (TempMatrix, -vec.x, -vec.y, -vec.z);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);
				break;
			case 1:
				caxis = 'x';
				angle = dval;
				MakeRotationMatrix (TempMatrix, angle, caxis);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeRotationMatrix (TempMatrix, -angle, caxis);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);
				break;
			case 2:
				caxis = 'y';
				angle = dval;
				MakeRotationMatrix (TempMatrix, angle, caxis);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeRotationMatrix (TempMatrix, -angle, caxis);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);
				break;
			case 3:
				caxis = 'z';
				angle = dval;
				MakeRotationMatrix (TempMatrix, angle, caxis);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeRotationMatrix (TempMatrix, -angle, caxis);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);
				break;
			case 4:
				MakeIdentityMatrix (TempMatrix);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeIdentityMatrix (TempMatrix);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);

				MakeScalingMatrix (TempMatrix, vec.x, vec.y, vec.z);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeScalingMatrix (TempMatrix, 1.0 / vec.x, 1.0 / vec.y, 1.0 / vec.z);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);

				MakeIdentityMatrix (TempMatrix);
				MultiplyMatrices (node->trans, node->trans, TempMatrix);
				MakeIdentityMatrix (TempMatrix);
				MultiplyMatrices (node->invtrans, TempMatrix, node->invtrans);
				break;
			case 5:
				VisibleNode (node->node_name, dval); break;
			default: break;
		}
	}
}

size_t CCharShape::GetNumNodes () const {
	return numNodes;
}

const string& CCharShape::GetNodeFullname (size_t idx) const {
	if (idx >= numNodes) return emptyString;
	return Nodes[idx]->action->name;
}

size_t CCharShape::GetNumActs (size_t idx) const {
	if (idx >= numNodes) return -1;
	return Nodes[idx]->action->num;
}

TCharAction *CCharShape::GetAction (size_t idx) const {
	if (idx >= numNodes) return NULL;
	return Nodes[idx]->action;
}

void CCharShape::PrintAction (size_t idx) const {
	if (idx >= numNodes) return;
	TCharAction *act = Nodes[idx]->action;
	PrintInt ((int)act->num);
	for (size_t i=0; i<act->num; i++) {
		PrintInt (act->type[i]);
		PrintDouble (act->dval[i]);
		PrintVector (act->vec[i]);
	}
}

void CCharShape::PrintNode (size_t idx) const {
	TCharNode *node = Nodes[idx];
	PrintInt ("node: ", (int)node->node_name);
	PrintInt ("parent: ", (int)node->parent_name);
	PrintInt ("child: ", (int)node->child_name);
	PrintInt ("next: ", (int)node->next_name);
}

void CCharShape::SaveCharNodes (const string& dir, const string& filename) {
	CSPList list (MAX_CHAR_NODES + 10);

	list.Add ("# Generated by Tuxracer tools");
	list.Add ("");
	if (!Materials.empty()) {
		list.Add ("# Materials:");
		for (size_t i=0; i<Materials.size(); i++)
			if(!Materials[i].matline.empty())
				list.Add (Materials[i].matline);
		list.Add ("");
	}

	list.Add ("# Nodes:");
	for (size_t i=1; i<numNodes; i++) {
		TCharNode* node = Nodes[i];
		TCharAction* act = node->action;
		if (node->parent_name >= node->node_name) Message ("wrong parent index");
		string line = "*[node] " + Int_StrN ((int)node->node_name);
		line += " [par] " + Int_StrN ((int)node->parent_name);
		bool rotflag = false;

		if (!act->order.empty()) {
			TVector3 rotation;
			line += " [order] " + act->order;
			for (size_t ii=0; ii<act->order.size(); ii++) {
				int aa = act->order[ii]-48;
				switch (aa) {
					case 0: line += " [trans] " + Vector_StrN (act->vec[ii], 2); break;
					case 4: line += " [scale] " + Vector_StrN (act->vec[ii], 2); break;
					case 1: rotation.x = act->dval[ii]; rotflag = true; break;
					case 2: rotation.y = act->dval[ii]; rotflag = true; break;
					case 3: rotation.z = act->dval[ii]; rotflag = true; break;
					case 5: line += " [vis] " + Float_StrN (act->dval[ii], 0); break;
					case 9: rotation.z = act->dval[ii]; rotflag = true; break;
				}
			}
			if (rotflag) line += " [rot] " + Vector_StrN (rotation, 2);
		}
		if (!act->mat.empty()) line += " [mat] " + act->mat;
		if (!node->joint.empty()) line += " [joint] " + node->joint;
		if (!act->name.empty()) line += " [name] " + act->name;
		if (node->render_shadow) line += " [shad] 1";

		list.Add (line);
		if (i<numNodes-3) {
			if (node->visible && !Nodes[i+1]->visible) list.Add ("");
			const string& joint = Nodes[i+2]->joint;
			if (joint.empty()) list.Add ("# " + joint);
		}
	}
	list.Save (dir, filename);
}
