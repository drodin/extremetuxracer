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

#include "track_marks.h"
#include "ogl.h"
#include "tux.h"
#include "textures.h"
#include "course_render.h"
#include "env.h"
#include "course.h"

#define TRACK_WIDTH  0.7
#define MAX_TRACK_MARKS 10000
#define MAX_CONTINUE_TRACK_DIST TRACK_WIDTH * 4
#define MAX_CONTINUE_TRACK_TIME .1
#define SPEED_TO_START_TRENCH 0.0
#define SPEED_OF_DEEPEST_TRENCH 10

#define TRACK_HEIGHT 0.08
#define MAX_TRACK_DEPTH 0.7


typedef enum track_types_t {
    TRACK_HEAD,
    TRACK_MARK,
    TRACK_TAIL,
    NUM_TRACK_TYPES
} track_types_t;

typedef struct track_quad_t {
    TVector3 v1, v2, v3, v4;
    TVector2 t1, t2, t3, t4;
    TVector3 n1, n2, n3, n4;
    track_types_t track_type;
    double alpha;
} track_quad_t;

typedef struct track_marks_t {
    track_quad_t quads[MAX_TRACK_MARKS];
    int current_mark;
    int next_mark;
    double last_mark_time;
    TVector3 last_mark_pos;
} track_marks_t;

static track_marks_t track_marks;
static bool continuing_track;

static int trackid1 = 1;
static int trackid2 = 2;
static int trackid3 = 3;

void SetTrackIDs (int id1, int id2, int id3) {
	trackid1 = id1;
	trackid2 = id2;
	trackid3 = id3;
}

void init_track_marks (void) {
    track_marks.current_mark = 0;
    track_marks.next_mark = 0;
    track_marks.last_mark_time = -99999;
    track_marks.last_mark_pos = MakeVector(-9999, -9999, -9999);
    continuing_track = false;
}

// --------------------------------------------------------------------
//						draw_track_marks
// --------------------------------------------------------------------

void DrawTrackmarks (void) {
	if (param.perf_level < 3) return;	

    GLuint texid[NUM_TRACK_TYPES];
    int current_quad, num_quads;
    int first_quad;
    track_quad_t *q, *qnext;

    TColor track_colour = colWhite;
	set_gl_options (TRACK_MARKS); 

    glColor4f (0, 0, 0, 1);
	
	texid[TRACK_HEAD] = Tex.TexID (trackid1);
	texid[TRACK_MARK] = Tex.TexID (trackid2);
	texid[TRACK_TAIL] = Tex.TexID (trackid3);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    set_material (colWhite, colBlack, 1.0);

    num_quads = min (track_marks.current_mark, MAX_TRACK_MARKS -
		     track_marks.next_mark + track_marks.current_mark);
    first_quad = track_marks.current_mark - num_quads;

	for  (current_quad = 0; current_quad < num_quads; current_quad++) {
		q = &track_marks.quads[(first_quad + current_quad) % MAX_TRACK_MARKS];

		track_colour.a = q->alpha;
		set_material (track_colour, colBlack, 1.0);
		glBindTexture (GL_TEXTURE_2D, texid[q->track_type]);

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

				qnext = &track_marks.quads[(first_quad+current_quad+1)%MAX_TRACK_MARKS];
				while ( (qnext->track_type == TRACK_MARK) && (current_quad + 1 < num_quads)) {
					current_quad++;
					q = &track_marks.quads[(first_quad+current_quad) % MAX_TRACK_MARKS];
					track_colour.a = qnext->alpha;
					set_material (track_colour, colBlack, 1.0);

					glNormal3f (q->n4.x, q->n4.y, q->n4.z);
					glTexCoord2f (q->t4.x, q->t4.y);
					glVertex3f (q->v4.x, q->v4.y, q->v4.z);
		
				glNormal3f (q->n3.x, q->n3.y, q->n3.z);
				glTexCoord2f (q->t3.x, q->t3.y);
				glVertex3f (q->v3.x, q->v3.y, q->v3.z);
			
				qnext = &track_marks.quads[(first_quad+current_quad+1)%MAX_TRACK_MARKS];
			}
			glEnd();
		}
    }
}

void break_track_marks (void) {
    track_quad_t *qprev, *qprevprev;
    qprev = &track_marks.quads[(track_marks.current_mark-1)%MAX_TRACK_MARKS];
    qprevprev = &track_marks.quads[(track_marks.current_mark-2)%MAX_TRACK_MARKS];

    if (track_marks.current_mark > 0) {
		qprev->track_type = TRACK_TAIL;
		qprev->t1 = MakeVector2(0.0, 0.0);
		qprev->t2 = MakeVector2(1.0, 0.0);
		qprev->t3 = MakeVector2(0.0, 1.0);
		qprev->t4 = MakeVector2(1.0, 1.0);
		qprevprev->t3.y = max((int)(qprevprev->t3.y+0.5), (int)(qprevprev->t1.y+1));
		qprevprev->t4.y = max((int)(qprevprev->t3.y+0.5), (int)(qprevprev->t1.y+1));
    }
    track_marks.last_mark_time = -99999;
    track_marks.last_mark_pos = MakeVector(-9999, -9999, -9999);
    continuing_track = false;
}

// --------------------------------------------------------------------
//                      add_track_mark
// --------------------------------------------------------------------
void add_track_mark (CControl *ctrl, int *id) {
    if (param.perf_level < 3)
		return;

	TTerrType *TerrList = &Course.TerrList[0];

    track_quad_t* q = &track_marks.quads[track_marks.current_mark%MAX_TRACK_MARKS];
    track_quad_t* qprev = &track_marks.quads[(track_marks.current_mark-1)%MAX_TRACK_MARKS];
    track_quad_t* qprevprev = &track_marks.quads[(track_marks.current_mark-2)%MAX_TRACK_MARKS];

    TVector3 vector_from_last_mark = SubtractVectors (ctrl->cpos, track_marks.last_mark_pos);
    double dist_from_last_mark = NormVector (&vector_from_last_mark);
	
	*id = Course.GetTerrainIdx (ctrl->cpos.x, ctrl->cpos.z, 0.5);
	if (*id < 1) {
		break_track_marks();
		return;
	} 

	if (TerrList[*id].trackmarks < 1) {
		break_track_marks();
		return;
	} 
    
	TVector3 vel = ctrl->cvel;
    double speed = NormVector (&vel);
    if (speed < SPEED_TO_START_TRENCH) {
		break_track_marks();
		return;
    }

    TVector3 width_vector = CrossProduct (ctrl->cdirection, MakeVector (0, 1, 0));
    double magnitude = NormVector (&width_vector);
    if  (magnitude == 0) {
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
    if  (dist_from_surface >= (2 * comp_depth)) {
		break_track_marks();
		return;
    }

    if (!continuing_track) {
		break_track_marks();
		q->track_type = TRACK_HEAD;
		q->v1 = MakeVector (left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v2 = MakeVector (right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n1 = Course.FindCourseNormal (q->v1.x, q->v1.z);
		q->n2 = Course.FindCourseNormal (q->v2.x, q->v2.z);
		q->t1 = MakeVector2(0.0, 0.0);
		q->t2 = MakeVector2(1.0, 0.0);
		track_marks.next_mark = track_marks.current_mark + 1;
    } else {
		if  (track_marks.next_mark == track_marks.current_mark) {
		    q->v1 = qprev->v3;
	    	q->v2 = qprev->v4;
		    q->n1 = qprev->n3;
		    q->n2 = qprev->n4;
		    q->t1 = qprev->t3; 
		    q->t2 = qprev->t4;
	    	if (qprev->track_type != TRACK_HEAD) qprev->track_type = TRACK_MARK;
	    	q->track_type = TRACK_MARK;
		}
		q->v3 = MakeVector (left_wing.x, left_y + TRACK_HEIGHT, left_wing.z);
		q->v4 = MakeVector (right_wing.x, right_y + TRACK_HEIGHT, right_wing.z);
		q->n3 = Course.FindCourseNormal (q->v3.x, q->v3.z);
		q->n4 = Course.FindCourseNormal (q->v4.x, q->v4.z);
		double tex_end = speed*g_game.time_step/TRACK_WIDTH;
		if (q->track_type == TRACK_HEAD) {
		    q->t3= MakeVector2 (0.0, 1.0);
		    q->t4= MakeVector2 (1.0, 1.0);
		} else {
		    q->t3 = MakeVector2 (0.0, q->t1.y + tex_end);
		    q->t4 = MakeVector2 (1.0, q->t2.y + tex_end);
		}
		track_marks.current_mark++;
		track_marks.next_mark = track_marks.current_mark;
    }
    q->alpha = min ((2*comp_depth-dist_from_surface)/(4*comp_depth), 1.0);
    track_marks.last_mark_time = g_game.time;
    continuing_track = true;
}

void UpdateTrackmarks (CControl *ctrl) {
	int trackid;
	TTerrType *TerrList = &Course.TerrList[0];

	if (param.perf_level < 3) return;	
	add_track_mark (ctrl, &trackid);
	if (trackid >= 0 && TerrList[trackid].trackmarks) {
		SetTrackIDs (TerrList[trackid].starttex,
					TerrList[trackid].tracktex,
					TerrList[trackid].stoptex);
 	}
}
