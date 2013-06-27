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

#include "textures.h"
#include "course_render.h"
#include "course.h"
#include "ogl.h"
#include "quadtree.h"
#include "particles.h"
#include "env.h"
#include "game_ctrl.h"
#include "physics.h"

#define TEX_SCALE 6
static const bool clip_course = true;

void setup_course_tex_gen () {
    static GLfloat xplane[4] = {1.0 / TEX_SCALE, 0.0, 0.0, 0.0 };
    static GLfloat zplane[4] = {0.0, 0.0, 1.0 / TEX_SCALE, 0.0 };
    glTexGenfv (GL_S, GL_OBJECT_PLANE, xplane);
    glTexGenfv (GL_T, GL_OBJECT_PLANE, zplane);
}

// --------------------------------------------------------------------
//							render course
// --------------------------------------------------------------------
void RenderCourse () {
	ScopedRenderMode rm(COURSE);
    setup_course_tex_gen ();
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);
	const CControl *ctrl = Players.GetCtrl (g_game.player_id);
    UpdateQuadtree (ctrl->viewpos, param.course_detail_level);
    RenderQuadtree ();
}

// --------------------------------------------------------------------
//				DrawTrees
// --------------------------------------------------------------------
void DrawTrees() {
    size_t			tree_type = -1;
    size_t			item_type = -1;
	TObjectType*	object_types = &Course.ObjTypes[0];
	const CControl*	ctrl = Players.GetCtrl (g_game.player_id);

	ScopedRenderMode rm(TREES);
    double fwd_clip_limit = param.forward_clip_distance;
    double bwd_clip_limit = param.backward_clip_distance;

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);


//	-------------- trees ------------------------
    TCollidable* treeLocs = &Course.CollArr[0];
    size_t numTrees = Course.CollArr.size();

    for (size_t i = 0; i< numTrees; i++) {
		if (clip_course) {
			if (ctrl->viewpos.z - treeLocs[i].pt.z > fwd_clip_limit) continue;
		    if (treeLocs[i].pt.z - ctrl->viewpos.z > bwd_clip_limit) continue;
		}

		if (treeLocs[i].tree_type != tree_type) {
		    tree_type = treeLocs[i].tree_type;
			object_types[tree_type].texture->Bind();
		}

        glPushMatrix();
        glTranslatef (treeLocs[i].pt.x, treeLocs[i].pt.y, treeLocs[i].pt.z);
		if (param.perf_level > 1) glRotatef (1, 0, 1, 0);

        double treeRadius = treeLocs[i].diam / 2.0;
        double treeHeight = treeLocs[i].height;
		TVector3 normal(0, 0, 1);
		glNormal3f (normal.x, normal.y, normal.z);
/*		// slower but better method of setting the normals
		normal = SubtractVectors (ctrl->viewpos, treeLocs[i].pt);
		NormVector (normal);
		glNormal3f (normal.x, normal.y, normal.z); */

		glBegin (GL_QUADS);
			glTexCoord2f (0.0, 0.0);
    	    glVertex3f (-treeRadius, 0.0, 0.0);
    	    glTexCoord2f (1.0, 0.0);
    	    glVertex3f (treeRadius, 0.0, 0.0);
    	    glTexCoord2f (1.0, 1.0);
    	    glVertex3f (treeRadius, treeHeight, 0.0);
    	    glTexCoord2f (0.0, 1.0);
    	    glVertex3f (-treeRadius, treeHeight, 0.0);

//			if (!clip_course || ctrl->viewpos.z - treeLocs[i].pt.z < fwd_tree_detail_limit) {
			    glTexCoord2f  (0., 0.);
			    glVertex3f  (0.0, 0.0, -treeRadius);
			    glTexCoord2f  (1., 0.);
			    glVertex3f  (0.0, 0.0, treeRadius);
			    glTexCoord2f  (1., 1.);
			    glVertex3f  (0.0, treeHeight, treeRadius);
			    glTexCoord2f  (0., 1.);
			    glVertex3f  (0.0, treeHeight, -treeRadius);
//			}
		glEnd();
        glPopMatrix();
	}

//  items -----------------------------
	TItem* itemLocs = &Course.NocollArr[0];
	size_t numItems = Course.NocollArr.size();

    for (size_t i = 0; i< numItems; i++) {
		if (itemLocs[i].collectable == 0 || itemLocs[i].drawable == false) continue;
		if (clip_course) {
		    if (ctrl->viewpos.z - itemLocs[i].pt.z > fwd_clip_limit) continue;
		    if (itemLocs[i].pt.z - ctrl->viewpos.z > bwd_clip_limit) continue;
		}

		if (itemLocs[i].item_type != item_type) {
		    item_type = itemLocs[i].item_type;
			object_types[item_type].texture->Bind();
		}

		glPushMatrix();
		    glTranslatef (itemLocs[i].pt.x, itemLocs[i].pt.y,  itemLocs[i].pt.z);
		    double itemRadius = itemLocs[i].diam / 2;
		    double itemHeight = itemLocs[i].height;

			TVector3 normal;
		    if (object_types[item_type].use_normal) {
				normal = object_types[item_type].normal;
		    } else {
//				normal = MakeVector (0, 0, 1);
				normal = SubtractVectors (ctrl->viewpos, itemLocs[i].pt);
				NormVector (normal);
		    }
		    glNormal3f (normal.x, normal.y, normal.z);
		    normal.y = 0.0;
		    NormVector (normal);
		    glBegin (GL_QUADS);
				glTexCoord2f (0., 0.);
				glVertex3f (-itemRadius*normal.z, 0.0,  itemRadius*normal.x);
				glTexCoord2f (1., 0.);
				glVertex3f (itemRadius*normal.z, 0.0, -itemRadius*normal.x);
				glTexCoord2f (1., 1.);
				glVertex3f (itemRadius*normal.z, itemHeight, -itemRadius*normal.x);
				glTexCoord2f (0., 1.);
				glVertex3f (-itemRadius*normal.z, itemHeight, itemRadius*normal.x);
	    	glEnd();
        glPopMatrix();
    }
}
