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
	static const GLfloat xplane[4] = {1.0 / TEX_SCALE, 0.0, 0.0, 0.0 };
	static const GLfloat zplane[4] = {0.0, 0.0, 1.0 / TEX_SCALE, 0.0 };
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
		glTranslate(treeLocs[i].pt);
		if (param.perf_level > 1) glRotatef (1, 0, 1, 0);

		float treeRadius = treeLocs[i].diam / 2.0;
		float treeHeight = treeLocs[i].height;
		glNormal3i(0, 0, 1);

		static const GLshort tex[] = {
			0, 0,
			1, 0,
			1, 1,
			0, 1,
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const GLfloat vtx[] = {
			-treeRadius, 0.0,        0.0,
			treeRadius,  0.0,        0.0,
			treeRadius,  treeHeight, 0.0,
			-treeRadius, treeHeight, 0.0,
			0.0,         0.0,        -treeRadius,
			0.0,         0.0,        treeRadius,
			0.0,         treeHeight, treeRadius,
			0.0,         treeHeight, -treeRadius
		};

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, vtx);
		glTexCoordPointer(2, GL_SHORT, 0, tex);
		glDrawArrays(GL_QUADS, 0, 8);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

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
		glTranslate(itemLocs[i].pt);
		double itemRadius = itemLocs[i].diam / 2;
		double itemHeight = itemLocs[i].height;

		TVector3 normal;
		if (object_types[item_type].use_normal) {
			normal = object_types[item_type].normal;
		} else {
			normal = SubtractVectors (ctrl->viewpos, itemLocs[i].pt);
			NormVector (normal);
		}
		glNormal3(normal);
		normal.y = 0.0;
		NormVector (normal);

		static const GLshort tex[] = {
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const GLfloat vtx[] = {
			-itemRadius*normal.z, 0.0,        itemRadius*normal.x,
			itemRadius*normal.z, 0.0,        -itemRadius*normal.x,
			itemRadius*normal.z,  itemHeight, -itemRadius*normal.x,
			-itemRadius*normal.z, itemHeight, itemRadius*normal.x
		};
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, vtx);
		glTexCoordPointer(2, GL_SHORT, 0, tex);
		glDrawArrays(GL_QUADS, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
}
