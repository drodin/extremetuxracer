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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "track_marks.h"
#include "ogl.h"
#include "textures.h"
#include "course.h"
#include "physics.h"
#include <list>

#define TRACK_WIDTH 0.7
#define MAX_TRACK_MARKS 10000
#define SPEED_TO_START_TRENCH 0.0
#define TRACK_HEIGHT 0.08
#define MAX_TRACK_DEPTH 0.7


enum track_types_t {
	TRACK_HEAD,
	TRACK_MARK,
	TRACK_TAIL,
	NUM_TRACK_TYPES
};

struct track_quad_t {
	TVector3d v1, v2, v3, v4;
	TVector2d t1, t2, t3, t4;
	TVector3d n1, n2, n3, n4;
	track_types_t track_type;
	uint8_t alpha;
};

struct track_marks_t {
	std::list<track_quad_t> quads;
	std::list<track_quad_t>::iterator current_mark;
};

static track_marks_t track_marks;
static bool continuing_track;

static int trackid1 = 1;
static int trackid2 = 2;
static int trackid3 = 3;

void SetTrackIDs(int id1, int id2, int id3) {
	trackid1 = id1;
	trackid2 = id2;
	trackid3 = id3;
}

void init_track_marks() {
	track_marks.quads.clear();
	track_marks.current_mark = track_marks.quads.begin();
	continuing_track = false;
}

template<typename T>
static T incrementRingIterator(T q) {
	T ret = q;
	++ret;
	if (ret == track_marks.quads.end())
		ret = track_marks.quads.begin();
	return ret;
}

template<typename T>
static T decrementRingIterator(T q) {
	T ret = q;
	if (ret == track_marks.quads.begin())
		return track_marks.quads.end();
	--ret;
	return ret;
}

void DrawTrackmarks() {
	if (param.perf_level < 3 || track_marks.quads.empty())
		return;

	TTexture* textures[NUM_TRACK_TYPES];

	sf::Color track_colour = colWhite;
	set_material(track_colour, colBlack, 1.0);
	ScopedRenderMode rm(TRACK_MARKS);

	textures[TRACK_HEAD] = Tex.GetTexture(trackid1);
	textures[TRACK_MARK] = Tex.GetTexture(trackid2);
	textures[TRACK_TAIL] = Tex.GetTexture(trackid3);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	for (std::list<track_quad_t>::const_iterator q = track_marks.quads.begin(); q != track_marks.quads.end(); ++q) {
		if (q->alpha != track_colour.a) {
			track_colour.a = q->alpha;
			set_material_diffuse(track_colour);
		}
		textures[q->track_type]->Bind();

		if ((q->track_type == TRACK_HEAD) || (q->track_type == TRACK_TAIL)) {
			glBegin(GL_QUADS);

			glNormal3(q->n1);
			glTexCoord2(q->t1);
			glVertex3(q->v1);

			glNormal3(q->n2);
			glTexCoord2(q->t2);
			glVertex3(q->v2);

			glNormal3(q->n4);
			glTexCoord2(q->t4);
			glVertex3(q->v4);

			glNormal3(q->n3);
			glTexCoord2(q->t3);
			glVertex3(q->v3);

			glEnd();

		} else {
			glBegin(GL_QUAD_STRIP);
			glNormal3(q->n2);
			glTexCoord2(q->t2);
			glVertex3(q->v2);

			glNormal3(q->n1);
			glTexCoord2(q->t1);
			glVertex3(q->v1);

			glNormal3(q->n4);
			glTexCoord2(q->t4);
			glVertex3(q->v4);

			glNormal3(q->n3);
			glTexCoord2(q->t3);
			glVertex3(q->v3);

			std::list<track_quad_t>::const_iterator qnext = q;
			++qnext;
			while (qnext != track_marks.quads.end() && qnext->track_type != TRACK_TAIL) {
				q = qnext;
				if (q->alpha != track_colour.a) {
					track_colour.a = q->alpha;
					set_material_diffuse(track_colour);
				}

				glNormal3(q->n4);
				glTexCoord2(q->t4);
				glVertex3(q->v4);

				glNormal3(q->n3);
				glTexCoord2(q->t3);
				glVertex3(q->v3);

				++qnext;
			}
			glEnd();
		}
	}
}

void break_track_marks() {
	if (!continuing_track)
		return;

	std::list<track_quad_t>::iterator q = track_marks.current_mark;
	if (q != track_marks.quads.end()) {
		q->track_type = TRACK_TAIL;
		q->t1 = TVector2d(0.0, 0.0);
		q->t2 = TVector2d(1.0, 0.0);
		q->t3 = TVector2d(0.0, 1.0);
		q->t4 = TVector2d(1.0, 1.0);
		std::list<track_quad_t>::iterator qprev = decrementRingIterator(q);
		if (qprev != track_marks.quads.end()) {
			qprev->t3.y = std::max(qprev->t3.y+0.5, qprev->t1.y+1.0);
			qprev->t4.y = std::max(qprev->t3.y+0.5, qprev->t1.y+1.0);
		}
	}
	continuing_track = false;
}

void add_track_mark(const CControl *ctrl, int *id) {
	if (param.perf_level < 3)
		return;

	*id = Course.GetTerrainIdx(ctrl->cpos.x, ctrl->cpos.z, 0.5);
	if (*id < 1) {
		break_track_marks();
		return;
	}

	if (!Course.TerrList[*id].trackmarks) {
		break_track_marks();
		return;
	}

	double speed = ctrl->cvel.Length();
	if (speed < SPEED_TO_START_TRENCH) {
		break_track_marks();
		return;
	}

	TVector3d width_vector = CrossProduct(ctrl->cdirection, TVector3d(0, 1, 0));
	double magnitude = width_vector.Norm();
	if (magnitude == 0) {
		break_track_marks();
		return;
	}

	TVector3d left_vector = TRACK_WIDTH/2.0 * width_vector;
	TVector3d right_vector = -TRACK_WIDTH/2.0 * width_vector;
	TVector3d left_wing =  ctrl->cpos - left_vector;
	TVector3d right_wing = ctrl->cpos - right_vector;
	double left_y = Course.FindYCoord(left_wing.x, left_wing.z);
	double right_y = Course.FindYCoord(right_wing.x, right_wing.z);

	if (std::fabs(left_y-right_y) > MAX_TRACK_DEPTH) {
		break_track_marks();
		return;
	}

	TPlane surf_plane = Course.GetLocalCoursePlane(ctrl->cpos);
	double dist_from_surface = DistanceToPlane(surf_plane, ctrl->cpos);
	double comp_depth = 0.1;
	if (dist_from_surface >= (2 * comp_depth)) {
		break_track_marks();
		return;
	}

	if (track_marks.quads.size() < MAX_TRACK_MARKS)
		track_marks.quads.emplace_back();
	std::list<track_quad_t>::iterator qprev = track_marks.current_mark;
	if (track_marks.current_mark == track_marks.quads.end())
		track_marks.current_mark = track_marks.quads.begin();
	else
		track_marks.current_mark = incrementRingIterator(track_marks.current_mark);
	std::list<track_quad_t>::iterator q = track_marks.current_mark;

	if (!continuing_track) {
		q->track_type = TRACK_HEAD;
		q->v1 = TVector3d(left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v2 = TVector3d(right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->v3 = TVector3d(left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v4 = TVector3d(right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n1 = Course.FindCourseNormal(q->v1.x, q->v1.z);
		q->n2 = Course.FindCourseNormal(q->v2.x, q->v2.z);
		q->n3 = Course.FindCourseNormal(q->v3.x, q->v3.z);
		q->n4 = Course.FindCourseNormal(q->v4.x, q->v4.z);
		q->t1 = TVector2d(0.0, 0.0);
		q->t2 = TVector2d(1.0, 0.0);
		q->t3 = TVector2d(0.0, 1.0);
		q->t4 = TVector2d(1.0, 1.0);
	} else {
		q->track_type = TRACK_TAIL;
		q->v1 = qprev->v3;
		q->v2 = qprev->v4;
		q->v3 = TVector3d(left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v4 = TVector3d(right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n1 = qprev->n3;
		q->n2 = qprev->n4;
		q->n3 = Course.FindCourseNormal(q->v3.x, q->v3.z);
		q->n4 = Course.FindCourseNormal(q->v4.x, q->v4.z);
		q->t1 = qprev->t3;
		q->t2 = qprev->t4;
		double tex_end = speed*g_game.time_step/TRACK_WIDTH;
		q->t3 = TVector2d(0.0, q->t1.y + tex_end);
		q->t4 = TVector2d(1.0, q->t2.y + tex_end);
		if (qprev->track_type == TRACK_TAIL)
			qprev->track_type = TRACK_MARK;
	}
	q->alpha = std::min(static_cast<int>((2*comp_depth-dist_from_surface)/(4*comp_depth)*255), 255);
	continuing_track = true;
}

void UpdateTrackmarks(const CControl *ctrl) {
	if (param.perf_level < 3)
		return;

	int trackid;
	TTerrType *TerrList = &Course.TerrList[0];

	add_track_mark(ctrl, &trackid);
	if (trackid >= 0 && TerrList[trackid].trackmarks) {
		SetTrackIDs(TerrList[trackid].starttex,
		            TerrList[trackid].tracktex,
		            TerrList[trackid].stoptex);
	}
}
