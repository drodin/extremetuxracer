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

/* --------------------------------------------------------------------
This module has been almost completely rewritten. To distinguish the 
functions from the old functions (Tuxracer 0.61) they are cdesignated 
with the add-on "2". Remember that the way of defining the character
has radically changed though the charakter is still shaped with spheres.
---------------------------------------------------------------------*/

#include "tux.h"
#include "ogl.h"
#include "spx.h"
#include "textures.h"
#include "course.h"

#define USE_CHAR_DISPLAY_LIST true
#define MIN_SPHERE_DIV 3
#define MAX_SPHERE_DIV 16 

#define	MAX_CHAR_NODES 128
#define	MAX_MAT2 64

#define MAX_ARM_ANGLE2 30.0
#define MAX_PADDLING_ANGLE2 35.0
#define MAX_EXT_PADDLING_ANGLE2 30.0
#define MAX_KICK_PADDLING_ANGLE2 20.0

#define TO_AIR_TIME 0.5			
#define TO_TIME 0.14			

TTuxMaterial2 TuxDefMat = {{0.5, 0.5, 0.5, 1.0}, {0.0, 0.0, 0.0, 1.0}, 0.0};

TCharNode2 *CharNodes2 [MAX_CHAR_NODES];
static int  numCharNodes2 = 0;
static string NodeIndex2;

TTuxMaterial2 *CharMaterials2 [MAX_MAT2];
static int    numCharMat = 0;
static string MatIndex2;

// --------------------------------------------------------------------
//			draw
// --------------------------------------------------------------------

void DrawCharSphere2 (int num_divisions) {
    GLUquadricObj *qobj;
    qobj = gluNewQuadric();
    gluQuadricDrawStyle (qobj, GLU_FILL);
    gluQuadricOrientation (qobj, GLU_OUTSIDE);
    gluQuadricNormals (qobj, GLU_SMOOTH);
    gluSphere (qobj, 1.0, (GLint)2.0 * num_divisions, num_divisions);
    gluDeleteQuadric (qobj);
}

static GLuint GetTuxDisplayList2 (int divisions) {
    static bool initialized = false;
    static int num_display_lists;
    static GLuint *display_lists = NULL;
    int base_divisions;
    int i, idx;

    if  (!initialized) {
		initialized = true;
		base_divisions = param.tux_sphere_divisions;
		num_display_lists = MAX_SPHERE_DIV - MIN_SPHERE_DIV + 1;
		display_lists = (GLuint*) malloc (sizeof(GLuint) * num_display_lists);
		for (i=0; i<num_display_lists; i++) display_lists[i] = 0;
    }

    idx = divisions - MIN_SPHERE_DIV;
    if  (display_lists[idx] == 0) {
		display_lists[idx] = glGenLists (1);
		glNewList (display_lists[idx], GL_COMPILE);
		DrawCharSphere2 (divisions);
		glEndList ();
    }
    return display_lists[idx];
}

void DrawNodes2 (TCharNode2 *node, TTuxMaterial2 *mat) {
    TCharNode2 *child;
    glPushMatrix();
    glMultMatrixd ((double *) node->trans);

    if  (node->mat != NULL) mat = node->mat;
    if  (node->visible == true) {
        set_material (mat->diffuse, mat->specular, mat->exp);
		if (USE_CHAR_DISPLAY_LIST) glCallList (GetTuxDisplayList2 (node->divisions));
			else DrawCharSphere2 (node->divisions);
    } 

    child = node->child;
    while (child != NULL) {
        DrawNodes2 (child, mat);
        child = child->next;
    } 
    glPopMatrix();
} 


// -----------------------------------------------------------------
bool GetCharMaterial2 (char *mat_name, TTuxMaterial2 **mat) {
	int idx;
	idx = SPIntN (MatIndex2, mat_name, -1);
	if (idx >= 0 && idx < numCharMat) {
		*mat = CharMaterials2[idx];
		return true;
	} else {
		*mat = 0;
		return false;
	}
}

void CreateCharMaterial2 (const char *line) {
	char matName[32];
	TVector3 diff = {0,0,0}; 
	TVector3 spec = {0,0,0};
	float exp = 100;

	string lin = line;	
	SPCharN (lin, "mat", matName);
	diff = SPVector3N (lin, "diff", MakeVector (0,0,0));
	spec = SPVector3N (lin, "spec", MakeVector (0,0,0));
	exp = SPFloatN (lin, "exp", 50);

	TTuxMaterial2 *matPtr = (TTuxMaterial2 *) malloc (sizeof (TTuxMaterial2));

    matPtr->diffuse.r = diff.x;
    matPtr->diffuse.g = diff.y;
    matPtr->diffuse.b = diff.z;
    matPtr->diffuse.a = 1.0;
    matPtr->specular.r = spec.x;
    matPtr->specular.g = spec.y;
    matPtr->specular.b = spec.z;
    matPtr->specular.a = 1.0;
    matPtr->exp = exp;

	TTuxMaterial2 *test;
    if (GetCharMaterial2 (matName, &test)) {
		free (matPtr);
	} else {
		SPAddIntN (MatIndex2, matName, numCharMat);
		CharMaterials2[numCharMat] = matPtr;
		numCharMat++;
	}
}

void ResetTuxRoot2 () { ResetCharNode2 ("root"); }

void ResetTuxJoints2 () {
    ResetCharNode2 ("left_shoulder");
    ResetCharNode2 ("right_shoulder");
    ResetCharNode2 ("left_hip");
    ResetCharNode2 ("right_hip");
    ResetCharNode2 ("left_knee");
    ResetCharNode2 ("right_knee");
    ResetCharNode2 ("left_ankle");
    ResetCharNode2 ("right_ankle");
    ResetCharNode2 ("tail");
    ResetCharNode2 ("neck");
    ResetCharNode2 ("head");
}

void CreateRootNode () {
    TCharNode2 *child = (TCharNode2 *) malloc (sizeof (TCharNode2));
    child->parent = 0;
    child->next = 0;
    child->child = 0;
    child->mat = 0;
    child->render_shadow = false;
	child->visible = false;
    MakeIdentityMatrix (child->trans);
	MakeIdentityMatrix (child->invtrans);

    child->name = (char*) malloc (strlen ("root") + 1);
	strcpy (child->name, "root");
	NodeIndex2[0] = '\0';
	NodeIndex2 = "[root]0";
	CharNodes2[0] = child;
	numCharNodes2 = 1;
}

bool GetCharNode2 (const char *node_name, TCharNode2 **node) {
	int entry = SPIntN (NodeIndex2, node_name, -1);	
	if (entry < 0) { 
		*node = 0;	
		return false;
	} else {
		*node = (TCharNode2*) CharNodes2[entry];
		return true;
	}
}

bool CreateCharNode2 (char *parent_name, char *child_name, double vis, bool shadow) {
    TCharNode2 *parent, *child;

    if  (!GetCharNode2 (parent_name, &parent)) return false;
    child = (TCharNode2 *) malloc (sizeof (TCharNode2));

    child->parent = parent;
    child->next  = 0;
    child->child = 0;
    child->mat   = 0;
	child->visible = (vis > 0);
    child->render_shadow = shadow;

    MakeIdentityMatrix (child->trans);
    MakeIdentityMatrix (child->invtrans);

	if (vis > 0) {
	    child->radius = 1.0;
    	child->divisions = 
		MIN (MAX_SPHERE_DIV, MAX (MIN_SPHERE_DIV, 
 		    ROUND_TO_NEAREST (param.tux_sphere_divisions * vis / 10)));
	} 
    child->name = (char*) malloc (strlen(child_name) + 1);
	strcpy (child->name, child_name);

	if (SPIntN (NodeIndex2, child_name, -1) < 0) { 
		SPAddIntN (NodeIndex2, child_name, numCharNodes2);
		CharNodes2[numCharNodes2] = child;
		numCharNodes2++;
	} else {
		free (child);
		return false;
	}

    if  (parent != 0) {
        if  (parent->child == 0) {
            parent->child = child;
        } else {
			for (parent = parent->child; parent->next != 0; parent = parent->next) {} 
			parent->next = child;
        } 
    } 
    return true;
} 

bool TranslateCharNode2 (const char *node_name, TVector3 vec) {
    TCharNode2 *nodePtr;
    TMatrix TransMatrix;

    if  (!GetCharNode2 (node_name, &nodePtr)) return false;
    MakeTranslationMatrix (TransMatrix, vec.x, vec.y, vec.z);
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, TransMatrix);
	MakeTranslationMatrix (TransMatrix, -vec.x, -vec.y, -vec.z);
	MultiplyMatrices (nodePtr->invtrans, TransMatrix, nodePtr->invtrans);

    return true;
}

void ScaleCharNode2 (const char *node_name, TVector3 vec) {
    TCharNode2 *nodePtr;
    TMatrix matrix;

    if  (!GetCharNode2 (node_name, &nodePtr)) return;

	MakeIdentityMatrix (matrix);
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, matrix);
	MakeIdentityMatrix (matrix);
	MultiplyMatrices (nodePtr->invtrans, matrix, nodePtr->invtrans);

    MakeScalingMatrix (matrix, vec.x, vec.y, vec.z);
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, matrix);
	MakeScalingMatrix (matrix, 1.0 / vec.x, 1.0 / vec.y, 1.0 / vec.z);
	MultiplyMatrices (nodePtr->invtrans, matrix, nodePtr->invtrans);

	MakeIdentityMatrix (matrix);
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, matrix);
	MakeIdentityMatrix (matrix);
	MultiplyMatrices (nodePtr->invtrans, matrix, nodePtr->invtrans);
}

bool RotateCharNode2 (const char *node_name, double axis, double angle) {
	TCharNode2 *nodePtr;
    TMatrix rotMatrix;
	char caxis = '0';

    if (!GetCharNode2 (node_name, &nodePtr)) return false;

	int a = (int) axis;
	if (axis > 3) return false;
	switch (a) {
		case 1: caxis = 'x'; break;
		case 2: caxis = 'y'; break;
		case 3: caxis = 'z'; break;
	}

    MakeRotationMatrix (rotMatrix, angle, caxis);
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, rotMatrix);
	MakeRotationMatrix (rotMatrix, -angle, caxis);
	MultiplyMatrices (nodePtr->invtrans, rotMatrix, nodePtr->invtrans);

    return true;
}

bool MaterialCharNode2 (char *node_name, char *mat_name) {
    TTuxMaterial2 *matPtr;
    TCharNode2 *nodePtr;
    if (!GetCharNode2 (node_name, &nodePtr)) return false;
    if (!GetCharMaterial2 (mat_name, &matPtr)) return false;
	nodePtr->mat = matPtr;
	return true;
}

bool ResetCharNode2 (const char *node_name) {  
    TCharNode2 *nodePtr;
    if (!GetCharNode2 (node_name, &nodePtr)) {
		return false;
	}
    MakeIdentityMatrix (nodePtr->trans);
    MakeIdentityMatrix (nodePtr->invtrans);
	return true;
}

bool TransformCharNode2 (const char *node_name, TMatrix mat, TMatrix invmat) {
    TCharNode2 *nodePtr;
    if (!GetCharNode2 (node_name, &nodePtr)) return false;
    MultiplyMatrices (nodePtr->trans, nodePtr->trans, mat);
	MultiplyMatrices (nodePtr->invtrans, invmat, nodePtr->invtrans);
    return true;
}

// --------------------------------------------------------------------
//		public
// --------------------------------------------------------------------		

void DrawTux2 () {
    TCharNode2 *nodePtr;
    float dummy_color[]  = {0.0, 0.0, 0.0, 1.0};

    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dummy_color);
    set_gl_options (TUX);
	glEnable (GL_NORMALIZE);
	
	if (!GetCharNode2 ("root", &nodePtr)) return;
	DrawNodes2 (nodePtr, &TuxDefMat);
	glDisable (GL_NORMALIZE);
	if (param.perf_level > 2) DrawTuxShadow2 ();
} 

static TVector3 adjust_tux_zvec_for_roll (CControl *ctrl, TVector3 vel, TVector3 zvec) {
    TMatrix rot_mat; 
    vel = ProjectToPlane (zvec, vel);
    NormVector (&vel);
    if (ctrl->is_braking) {
		RotateAboutVectorMatrix (rot_mat, vel, ctrl->turn_fact * BRAKING_ROLL_ANGLE);
    } else {
		RotateAboutVectorMatrix (rot_mat, vel, ctrl->turn_fact * MAX_ROLL_ANGLE);
    }
    return TransformVector (rot_mat, zvec);
}

void AdjustOrientation2 (CControl *ctrl, double dtime,
		 double dist_from_surface, TVector3 surf_nml){
    TVector3 new_x, new_y, new_z; 
    TMatrix cob_mat, inv_cob_mat;
    TMatrix rot_mat;
    TQuaternion new_orient;
    double time_constant;
    static TVector3 minus_z_vec = { 0., 0., -1. };
    static TVector3 y_vec = { 0., 1., 0. };

    if  (dist_from_surface > 0) {
		new_y = ScaleVector (1., ctrl->cvel);
		NormVector (&new_y);
		new_z = ProjectToPlane (new_y, MakeVector(0., -1., 0.));
		NormVector (&new_z);
		new_z = adjust_tux_zvec_for_roll (ctrl, ctrl->cvel, new_z);
    } else { 
		new_z = ScaleVector (-1., surf_nml);
		new_z = adjust_tux_zvec_for_roll (ctrl, ctrl->cvel, new_z);
		new_y = ProjectToPlane (surf_nml, ScaleVector (1., ctrl->cvel));
		NormVector(&new_y);
    }

    new_x = CrossProduct (new_y, new_z);
    MakeBasismatrix_Inv (cob_mat, inv_cob_mat, new_x, new_y, new_z);
    new_orient = MakeQuaternionFromMatrix (cob_mat);

    if (!ctrl->orientation_initialized) {
		ctrl->orientation_initialized = true;
		ctrl->corientation = new_orient;
    }

    time_constant = dist_from_surface > 0 ? TO_AIR_TIME : TO_TIME;

    ctrl->corientation = InterpolateQuaternions (
			ctrl->corientation, new_orient, 
			min (dtime / time_constant, 1.0));

    ctrl->plane_nml = RotateVector (ctrl->corientation, minus_z_vec);
    ctrl->cdirection = RotateVector (ctrl->corientation, y_vec);

    MakeMatrixFromQuaternion (cob_mat, ctrl->corientation);

    // Trick rotations 
    new_y = MakeVector (cob_mat[1][0], cob_mat[1][1], cob_mat[1][2]); 
    RotateAboutVectorMatrix (rot_mat, new_y, 
				        (ctrl->roll_factor * 360));
    MultiplyMatrices (cob_mat, rot_mat, cob_mat);
    new_x = MakeVector (cob_mat[0][0], cob_mat[0][1], cob_mat[0][2]); 
    RotateAboutVectorMatrix (rot_mat, new_x, 
				       ctrl->flip_factor * 360);
    MultiplyMatrices (cob_mat, rot_mat, cob_mat);
    TransposeMatrix (cob_mat, inv_cob_mat);

	TransformCharNode2 ("root", cob_mat, inv_cob_mat); 
}

void AdjustTuxJoints2 (double turnFact, bool isBraking, 
			double paddling_factor, double speed,
			TVector3 net_force, double flap_factor) {
    double turning_angle[2] = {0, 0};
    double paddling_angle = 0;
    double ext_paddling_angle = 0; 
    double kick_paddling_angle = 0;
    double braking_angle = 0;
    double force_angle = 0;
    double turn_leg_angle = 0;
    double flap_angle = 0;

    if  (isBraking) braking_angle = MAX_ARM_ANGLE2;

    paddling_angle = MAX_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
    ext_paddling_angle = MAX_EXT_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
    kick_paddling_angle = MAX_KICK_PADDLING_ANGLE2 * sin(paddling_factor * M_PI * 2.0);

    turning_angle[0] = MAX(-turnFact,0.0) * MAX_ARM_ANGLE2;
    turning_angle[1] = MAX(turnFact,0.0) * MAX_ARM_ANGLE2;

    flap_angle = MAX_ARM_ANGLE2 * (0.5 + 0.5 * sin (M_PI * flap_factor * 6 - M_PI / 2));

    force_angle = max (-20.0, min (20.0, -net_force.z / 300.0));
    turn_leg_angle = turnFact * 10;
    
	ResetTuxJoints2 ();

    RotateCharNode2 ("left_shoulder", 3, 
		    MIN (braking_angle + paddling_angle + turning_angle[0], MAX_ARM_ANGLE2) + flap_angle);
    RotateCharNode2 ("right_shoulder", 3,
		    MIN (braking_angle + paddling_angle + turning_angle[1], MAX_ARM_ANGLE2) + flap_angle);

    RotateCharNode2 ("left_shoulder", 2, -ext_paddling_angle);
    RotateCharNode2 ("right_shoulder", 2, ext_paddling_angle);

    RotateCharNode2 ("left_hip", 3, -20 + turn_leg_angle + force_angle);
    RotateCharNode2 ("right_hip", 3, -20 - turn_leg_angle + force_angle);
	
    RotateCharNode2 ("left_knee", 3, 
		-10 + turn_leg_angle - MIN (35, speed) + kick_paddling_angle + force_angle);
    RotateCharNode2 ("right_knee", 3, 
		-10 - turn_leg_angle - MIN (35, speed) - kick_paddling_angle + force_angle);

    RotateCharNode2 ("left_ankle", 3, -20 + MIN(50, speed));
    RotateCharNode2 ("right_ankle", 3, -20 + MIN(50, speed));
    RotateCharNode2 ("tail", 3, turnFact * 20);
    RotateCharNode2 ("neck", 3, -50);
    RotateCharNode2 ("head", 3, -30);
    RotateCharNode2 ("head", 2, -turnFact * 70);
}

void LoadTux2 () {
	string commands2 = "[create] 1 [prop] 2 [material] 3";
	char cmd_name[24], par_name[24], node_name[24], mat_name[24];
	CSPList list (500);
	int i, cmd;
	string line;
	TVector3 scale, trans;
	TVector2 rot;
	float axis, angle, res, visible;
	bool shadow;

	CreateRootNode ();

	if (!list.Load (param.char_dir, "tux.lst")) {
		Message ("could not load character");
		return;
	}
	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		SPCharN (line, "cmd", cmd_name);
		cmd = SPIntN (commands2, cmd_name, 0);
		SPCharN (line, "node", node_name);		

		if (cmd == 3) {
			CreateCharMaterial2 (line.c_str());
		} else {
			if (cmd == 1) {
				SPCharN (line, "par", par_name);		
				visible = SPFloatN (line, "vis", -1.0);
				shadow = SPBoolN (line, "shad", false);
				CreateCharNode2 (par_name, node_name, visible, shadow);					
			}

			res = SPFloatN (line, "res", 1.0);

			if (SPExistsN (line, "trans")) {
				trans = SPVector3N (line, "trans", MakeVector (0,0,0));
				TranslateCharNode2 (node_name, trans);
			}

			if (SPExistsN (line, "rot")) {
				rot = SPVector2N (line, "rot", MakeVector2 (0,0));
				axis = rot.x;
				angle = rot.y;
				RotateCharNode2 (node_name, axis, angle);
			}

			if (SPExistsN (line, "scale")) {
				scale = SPVector3N (line, "scale", MakeVector (1,1,1));
				ScaleCharNode2 (node_name, scale);
			}
			SPCharN (line, "mat", mat_name);
			MaterialCharNode2 (node_name, mat_name);
		}
	}
}

// --------------------------------------------------------------------
//			tux shadow
// --------------------------------------------------------------------

#define SHADOW_HEIGHT 0.05

#ifdef USE_STENCIL_BUFFER
	static TColor shad_col = { 0.0, 0.0, 0.0, 0.3 };
#else 
	static TColor shad_col = { 0.0, 0.0, 0.0, 0.1 };
#endif 

void DrawShadowVertex2 (double x, double y, double z, 
TMatrix model_matrix) {
    TVector3 pt;
    double old_y;
    TVector3 nml;

    pt = MakeVector (x, y, z);
    pt = TransformPoint (model_matrix, pt);
    old_y = pt.y;
    nml = Course.FindCourseNormal (pt.x, pt.z);
    pt.y = Course.FindYCoord (pt.x, pt.z) + SHADOW_HEIGHT;
    if  (pt.y > old_y) pt.y = old_y;
    glNormal3f (nml.x, nml.y, nml.z);
    glVertex3f (pt.x, pt.y, pt.z);
}

void DrawShadowSphere2 (TMatrix model_matrix) {
    double theta, phi, d_theta, d_phi, eps, twopi;
    double x, y, z;
    int div = param.tux_shadow_sphere_divisions;
    
    eps = 1e-15;
    twopi = M_PI * 2.0;
    d_theta = d_phi = M_PI / div;

    for  (phi = 0.0; phi + eps < M_PI; phi += d_phi) {
		double cos_theta, sin_theta;
		double sin_phi, cos_phi;
		double sin_phi_d_phi, cos_phi_d_phi;

		sin_phi = sin (phi);
		cos_phi = cos (phi);
		sin_phi_d_phi = sin (phi + d_phi);
		cos_phi_d_phi = cos (phi + d_phi);
        
        if  (phi <= eps) {
			glBegin (GL_TRIANGLE_FAN);
				DrawShadowVertex2 (0., 0., 1., model_matrix);

				for  (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);

					x = cos_theta * sin_phi_d_phi;
					y = sin_theta * sin_phi_d_phi;
					z = cos_phi_d_phi;
					DrawShadowVertex2 (x, y, z, model_matrix);
				} 
				x = sin_phi_d_phi;
				y = 0.0;
				z = cos_phi_d_phi;
				DrawShadowVertex2 (x, y, z, model_matrix);
            glEnd();
        } else if  (phi + d_phi + eps >= M_PI) {
			glBegin (GL_TRIANGLE_FAN);
				DrawShadowVertex2 (0., 0., -1., model_matrix);
                for  (theta = twopi; theta - eps > 0; theta -= d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
                    x = cos_theta * sin_phi;
                    y = sin_theta * sin_phi;
                    z = cos_phi;
					DrawShadowVertex2 (x, y, z, model_matrix);
                } 
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
				DrawShadowVertex2 (x, y, z, model_matrix);
            glEnd();
        } else {
            glBegin (GL_TRIANGLE_STRIP);
				for (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
					x = cos_theta * sin_phi;
					y = sin_theta * sin_phi;
					z = cos_phi;
					DrawShadowVertex2 (x, y, z, model_matrix);

					x = cos_theta * sin_phi_d_phi;
					y = sin_theta * sin_phi_d_phi;
					z = cos_phi_d_phi;
					DrawShadowVertex2 (x, y, z, model_matrix);
                } 
                x = sin_phi;
                y = 0.0;
                z = cos_phi;
				DrawShadowVertex2 (x, y, z, model_matrix);
                x = sin_phi_d_phi;
                y = 0.0;
                z = cos_phi_d_phi;
				DrawShadowVertex2 (x, y, z, model_matrix);
            glEnd();
        } 
    } 
} 

void TraverseDagForShadow2 (TCharNode2 *node, TMatrix model_matrix) {
    TMatrix new_model_matrix;
    TCharNode2 *child;

    MultiplyMatrices(new_model_matrix, model_matrix, node->trans);
	if (node->visible && node->render_shadow)
		DrawShadowSphere2 (new_model_matrix);

    child = node->child;
    while (child != NULL) {
        TraverseDagForShadow2 (child, new_model_matrix);
        child = child->next;
    } 
}

void DrawTuxShadow2 () {
    TMatrix model_matrix;
    TCharNode2 *nodePtr;

	if (g_game.light_id == 1 || g_game.light_id == 3) return;

    set_gl_options (TUX_SHADOW); 
    glColor4f (shad_col.r, shad_col.g, shad_col.b, shad_col.a);
    MakeIdentityMatrix (model_matrix);

    if  (GetCharNode2 ("root", &nodePtr) == false) {
		Message ("couldn't find tux's root node", "");
		return;
    } 
    TraverseDagForShadow2 (nodePtr, model_matrix);
}

bool CheckPolyhedronCollision (TCharNode2 *node, TMatrix modelMatrix, 
		TMatrix invModelMatrix, TPolyhedron ph) {

    TMatrix newModelMatrix, newInvModelMatrix;
    TCharNode2 *child;
    TPolyhedron newph;
    bool hit = false;

    MultiplyMatrices (newModelMatrix, modelMatrix, node->trans);
    MultiplyMatrices (newInvModelMatrix, node->invtrans, invModelMatrix);

    if  (node->visible) {
        newph = CopyPolyhedron (ph);
        TransPolyhedron (newInvModelMatrix, newph);
        hit = IntersectPolyhedron (newph);
        FreePolyhedron (newph);
    } 

    if (hit == true) return hit;
    child = node->child;
    while (child != NULL) {
        hit = CheckPolyhedronCollision (child, newModelMatrix, newInvModelMatrix, ph);
        if  (hit == true) return hit;
        child = child->next;
    } 
    return false;
}

bool CheckCollision2 (const char *node, TPolyhedron ph) {
    TCharNode2 *nodePtr;
    TMatrix mat, invmat;

    MakeIdentityMatrix (mat);
    MakeIdentityMatrix (invmat);

	if (!GetCharNode2 ("root", &nodePtr)) return false;
    return CheckPolyhedronCollision (nodePtr, mat, invmat, ph);
} 

bool TuxCollision2 (TVector3 pos, TPolyhedron ph) {
	ResetCharNode2 ("root");
	TranslateCharNode2 ("root", MakeVector (pos.x, pos.y, pos.z));	
	return CheckCollision2 ("root", ph);
}


