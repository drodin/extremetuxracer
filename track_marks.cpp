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

#include "track_marks.h"
#include "ogl.h"
#include "textures.h"
#include "course.h"
#include "physics.h"
#include <list>

#define TRACK_WIDTH  0.7
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
    TVector3 v1, v2, v3, v4;
    TVector2 t1, t2, t3, t4;
    TVector3 n1, n2, n3, n4;
    track_types_t track_type;
    double alpha;
};

struct track_marks_t {
    list<track_quad_t> quads;
	list<track_quad_t>::iterator current_mark;
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
	if(ret == track_marks.quads.end() || track_marks.quads.size() == MAX_TRACK_MARKS)
		ret = track_marks.quads.begin();
	return ret;
}

template<typename T>
static T decrementRingIterator(T q) {
	T ret = q;
	if(ret == track_marks.quads.begin() && track_marks.quads.size() == MAX_TRACK_MARKS)
		ret = track_marks.quads.end();
	else if(ret == track_marks.quads.begin())
		return track_marks.quads.end();
	--ret;
	return ret;
}

// --------------------------------------------------------------------
//						draw_track_marks
// --------------------------------------------------------------------

void DrawTrackmarks() {
	if (param.perf_level < 3)
		return;

    TTexture* textures[NUM_TRACK_TYPES];

    TColor track_colour = colWhite;
	ScopedRenderMode rm(TRACK_MARKS);

	textures[TRACK_HEAD] = Tex.GetTexture (trackid1);
	textures[TRACK_MARK] = Tex.GetTexture (trackid2);
	textures[TRACK_TAIL] = Tex.GetTexture (trackid3);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	for (list<track_quad_t>::const_iterator q = track_marks.quads.begin(); q != track_marks.quads.end(); ++q) {
		track_colour.a = q->alpha;
		set_material (track_colour, colBlack, 1.0);
		textures[q->track_type]->Bind();

		if ((q->track_type == TRACK_HEAD) || (q->track_type == TRACK_TAIL)) {
			glBegin(GL_QUADS);

			glNormal3f (q->n1.x, q->n1.y, q->n1.z);
			glTexCoord2f (q->t1.x, q->t1.y);
			glVertex3f (q->v1.x, q->v1.y, q->v1.z);

			glNormal3f (q->n2.x, q->n2.y, q->n2.z);
			glTexCoord2f (q->t2.x, q->t2.y);
			glVertex3f (q->v2.x, q->v2.y, q->v2.z);

			glNormal3f (q->n4.x, q->n4.y, q->n4.z);
			glTexCoord2f (q->t4.x, q->t4.y);
			glVertex3f (q->v4.x, q->v4.y, q->v4.z);

			glNormal3f (q->n3.x, q->n3.y, q->n3.z);
			glTexCoord2f (q->t3.x, q->t3.y);
			glVertex3f (q->v3.x, q->v3.y, q->v3.z);

			glEnd();

		} else {
			glBegin(GL_QUAD_STRIP);
				glNormal3f (q->n2.x, q->n2.y, q->n2.z);
				glTexCoord2f (q->t2.x, q->t2.y);
				glVertex3f (q->v2.x, q->v2.y, q->v2.z);

				glNormal3f (q->n1.x, q->n1.y, q->n1.z);
				glTexCoord2f (q->t1.x, q->t1.y);
				glVertex3f (q->v1.x, q->v1.y, q->v1.z);

				glNormal3f (q->n4.x, q->n4.y, q->n4.z);
				glTexCoord2f (q->t4.x, q->t4.y);
				glVertex3f (q->v4.x, q->v4.y, q->v4.z);

				glNormal3f (q->n3.x, q->n3.y, q->n3.z);
				glTexCoord2f (q->t3.x, q->t3.y);
				glVertex3f (q->v3.x, q->v3.y, q->v3.z);

				list<track_quad_t>::const_iterator qnext = q; ++qnext;
				while (qnext != track_marks.quads.end() && qnext->track_type != TRACK_TAIL) {
					q = qnext;
					track_colour.a = q->alpha;
					set_material (track_colour, colBlack, 1.0);

					glNormal3f (q->n4.x, q->n4.y, q->n4.z);
					glTexCoord2f (q->t4.x, q->t4.y);
					glVertex3f (q->v4.x, q->v4.y, q->v4.z);

					glNormal3f (q->n3.x, q->n3.y, q->n3.z);
					glTexCoord2f (q->t3.x, q->t3.y);
					glVertex3f (q->v3.x, q->v3.y, q->v3.z);

					++qnext;
				}
			glEnd();
		}
    }
}

void break_track_marks() {
	list<track_quad_t>::iterator q = track_marks.current_mark;
	if (q != track_marks.quads.end()) {
		q->track_type = TRACK_TAIL;
		q->t1 = TVector2(0.0, 0.0);
		q->t2 = TVector2(1.0, 0.0);
		q->t3 = TVector2(0.0, 1.0);
		q->t4 = TVector2(1.0, 1.0);
		list<track_quad_t>::iterator qprev = decrementRingIterator(q);
		if (qprev != track_marks.quads.end()) {
			qprev->t3.y = max((int)(qprev->t3.y+0.5), (int)(qprev->t1.y+1));
			qprev->t4.y = max((int)(qprev->t3.y+0.5), (int)(qprev->t1.y+1));
		}
    }
    continuing_track = false;
}

// --------------------------------------------------------------------
//                      add_track_mark
// --------------------------------------------------------------------
void add_track_mark(const CControl *ctrl, int *id) {
    if (param.perf_level < 3)
		return;

	TTerrType *TerrList = &Course.TerrList[0];

	*id = Course.GetTerrainIdx (ctrl->cpos.x, ctrl->cpos.z, 0.5);
	if (*id < 1) {
		break_track_marks();
		return;
	}

	if (!TerrList[*id].trackmarks) {
		break_track_marks();
		return;
	}

	TVector3 vel = ctrl->cvel;
    double speed = NormVector (vel);
    if (speed < SPEED_TO_START_TRENCH) {
		break_track_marks();
		return;
    }

    TVector3 width_vector = CrossProduct (ctrl->cdirection, TVector3 (0, 1, 0));
    double magnitude = NormVector (width_vector);
    if (magnitude == 0) {
		break_track_marks();
		return;
    }

    TVector3 left_vector = ScaleVector (TRACK_WIDTH/2.0, width_vector);
    TVector3 right_vector = ScaleVector (-TRACK_WIDTH/2.0, width_vector);
    TVector3 left_wing =  SubtractVectors (ctrl->cpos, left_vector);
    TVector3 right_wing = SubtractVectors (ctrl->cpos, right_vector);
    double left_y = Course.FindYCoord (left_wing.x, left_wing.z);
    double right_y = Course.FindYCoord (right_wing.x, right_wing.z);

	if (fabs(left_y-right_y) > MAX_TRACK_DEPTH) {
		break_track_marks();
		return;
    }

    TPlane surf_plane = Course.GetLocalCoursePlane (ctrl->cpos);
    double dist_from_surface = DistanceToPlane (surf_plane, ctrl->cpos);
	// comp_depth = get_compression_depth(Snow);
	double comp_depth = 0.1;
    if (dist_from_surface >= (2 * comp_depth)) {
		break_track_marks();
		return;
    }

	if(track_marks.quads.size() < MAX_TRACK_MARKS)
		track_marks.quads.push_back(track_quad_t());
	list<track_quad_t>::iterator qprev = track_marks.current_mark;
	if(track_marks.current_mark == track_marks.quads.end())
		track_marks.current_mark = track_marks.quads.begin();
	else
		track_marks.current_mark = incrementRingIterator(track_marks.current_mark);
	list<track_quad_t>::iterator q = track_marks.current_mark;

    if (!continuing_track) {
		q->track_type = TRACK_HEAD;
		q->v1 = TVector3 (left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v2 = TVector3 (right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->v3 = TVector3 (left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v4 = TVector3 (right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n1 = Course.FindCourseNormal (q->v1.x, q->v1.z);
		q->n2 = Course.FindCourseNormal (q->v2.x, q->v2.z);
		q->t1 = TVector2(0.0, 0.0);
		q->t2 = TVector2(1.0, 0.0);
    } else {
		q->track_type = TRACK_TAIL;
		if (qprev != track_marks.quads.end()) {
		    q->v1 = qprev->v3;
	    	q->v2 = qprev->v4;
		    q->n1 = qprev->n3;
		    q->n2 = qprev->n4;
		    q->t1 = qprev->t3;
		    q->t2 = qprev->t4;
	if (qprev->track_type == TRACK_TAIL) qprev->track_type = TRACK_MARK;
		}
		q->v3 = TVector3 (left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v4 = TVector3 (right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n3 = Course.FindCourseNormal (q->v3.x, q->v3.z);
		q->n4 = Course.FindCourseNormal (q->v4.x, q->v4.z);
		double tex_end = speed*g_game.time_step/TRACK_WIDTH;
		if (q->track_type == TRACK_HEAD) {
		    q->t3= TVector2 (0.0, 1.0);
		    q->t4= TVector2 (1.0, 1.0);
		} else {
		    q->t3 = TVector2 (0.0, q->t1.y + tex_end);
		    q->t4 = TVector2 (1.0, q->t2.y + tex_end);
		}
    }
    q->alpha = min ((2*comp_depth-dist_from_surface)/(4*comp_depth), 1.0);
    continuing_track = true;
}

void UpdateTrackmarks(const CControl *ctrl) {
	if (param.perf_level < 3)
		return;

	int trackid;
	TTerrType *TerrList = &Course.TerrList[0];

	add_track_mark (ctrl, &trackid);
	if (trackid >= 0 && TerrList[trackid].trackmarks) {
		SetTrackIDs (TerrList[trackid].starttex,
					TerrList[trackid].tracktex,
					TerrList[trackid].stoptex);
	}
}
