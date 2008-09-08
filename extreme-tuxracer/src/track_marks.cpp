/* 
 * ETRacer 
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

#include "track_marks.h"
#include "gl_util.h"
#include "model_hndl.h"
#include "hier.h"
#include "phys_sim.h"
#include "textures.h"
#include "course_render.h"
#include "render_util.h"
#include "game_config.h"
#include "course_mgr.h"

#include "ppgltk/alg/defs.h"

#include "game_mgr.h"


#undef TRACK_TRIANGLES

#define TRACK_WIDTH  0.7
#define MAX_TRACK_MARKS 16000
#define MAX_CONTINUE_TRACK_DIST TRACK_WIDTH*4
#define MAX_CONTINUE_TRACK_TIME .1
#define SPEED_TO_START_TRENCH 0.0
#define SPEED_OF_DEEPEST_TRENCH 10

#define MAX_CRACKS 100
#define CRACK_MIN_FORCE 10000
#define CRACK_SIZE_PER_FORCE 0.0001
#define CRACK_MAX_SIZE 1.5
#define CRACK_MAX_UNIFY_DISTANCE 0.5
#define CRACK_HEIGHT 0.08
#define MAX_CRACK_HEIGHT 0.2

#ifdef TRACK_TRIANGLES
  #define TRACK_HEIGHT 0.1
  #define MAX_TRACK_DEPTH 10
  #define MAX_TRIS MAX_TRACK_MARKS
#else
  #define TRACK_HEIGHT 0.08
  #define MAX_TRACK_DEPTH 0.7
#endif

typedef enum track_types_t {
    TRACK_HEAD,
    TRACK_MARK,
    TRACK_TAIL,
    NUM_TRACK_TYPES
} track_types_t;

typedef struct track_quad_t {
    pp::Vec3d v1, v2, v3, v4;
    pp::Vec2d t1, t2, t3, t4;
    pp::Vec3d n1, n2, n3, n4;
    track_types_t track_type;
	int terrain;
    double alpha;
} track_quad_t;

typedef struct track_marks_t {
    track_quad_t quads[MAX_TRACK_MARKS];
    int current_mark;
    int next_mark;
    double last_mark_time;
    pp::Vec3d last_mark_pos;
} track_marks_t;

static track_marks_t track_marks;
static bool continuing_track;

extern terrain_tex_t terrain_texture[NUM_TERRAIN_TYPES];
extern unsigned int num_terrains;

typedef struct crack_quad_t {
    pp::Vec3d v1, v2, v3, v4;
    pp::Vec2d t1, t2, t3, t4;
    pp::Vec3d n1, n2, n3, n4;
} crack_quad_t;

typedef struct cracks_t {
  crack_quad_t quads[MAX_CRACKS];
  int current_crack;
  double last_size;
  pp::Vec3d last_pos;
} cracks_t;

static cracks_t cracks;

#ifdef TRACK_TRIANGLES
typedef struct track_tris_t {
    triangle_t tri[MAX_TRIS];
    track_types_t *track_type[MAX_TRIS];
    double *alpha[MAX_TRIS];
    int first_mark;
    int next_mark;
    int current_start;
    int current_end;
    int num_tris;
} track_tris_t;

typedef struct track_tri_t {
    pp::Vec3d v1, v2, v3;
} track_tri_t;

static track_tris_t track_tris;

static void draw_tri( triangle_t *tri, double alpha )
{
    pp::Vec3d nml;
    GLfloat c[4] = {1.0, 0.0, 0.0, 1.0}; 

/*    set_material_alpha( white, black, 1.0, alpha ); */
    set_material_alpha( white, black, 1.0, 1.0 );  

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c);

    glBegin(GL_TRIANGLES);

    nml = find_course_normal( tri->p[0].x, tri->p[0].z );
    glNormal3f( nml.x, nml.y, nml.z );
    glTexCoord2f( tri->t[0].x, tri->t[0].y );
    glVertex3f( tri->p[0].x, tri->p[0].y, tri->p[0].z );
    
    nml = find_course_normal( tri->p[1].x, tri->p[1].z );
    glNormal3f( nml.x, nml.y, nml.z );
    glTexCoord2f( tri->t[1].x, tri->t[1].y );
    glVertex3f( tri->p[1].x, tri->p[1].y, tri->p[1].z );
    
    nml = find_course_normal( tri->p[2].x, tri->p[2].z );
    glNormal3f( nml.x, nml.y, nml.z );
    glTexCoord2f( tri->t[2].x, tri->t[2].y );
    glVertex3f( tri->p[2].x, tri->p[2].y, tri->p[2].z );

    glEnd();
}

static void draw_tri_tracks( void )
{
    GLuint texid[NUM_TRACK_TYPES];
    int i;

    set_gl_options( TRACK_MARKS ); 

    glColor4f( 0, 0, 0, 1);

    get_texture_binding( "track_head", &texid[TRACK_HEAD] );
    get_texture_binding( "track_mark", &texid[TRACK_MARK] );
    get_texture_binding( "track_tail", &texid[TRACK_TAIL] );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    set_material( white, black, 1.0 );
    setup_course_lighting();

    for( i = 0; i < track_tris.num_tris; i++ ) {
	glBindTexture( GL_TEXTURE_2D, 
		       texid[*track_tris.track_type[(track_tris.first_mark+i)%MAX_TRIS]] );
	draw_tri( &track_tris.tri[(track_tris.first_mark+i)%MAX_TRIS],
		  *track_tris.alpha[(track_tris.first_mark+i)%MAX_TRIS] );
    }
}

static void add_tri_tracks_from_tri( pp::Vec3d p1, pp::Vec3d p2, pp::Vec3d p3,
				     pp::Vec2d t1, pp::Vec2d t2, pp::Vec2d t3,
				     track_types_t *track_type, double *alpha )
{
    double minx, maxx;
    double minz, maxz;
    int nx, nz;
    double width, length;
    int i, j, k;
    double xstep, zstep;
    line_t cut;
    int num_tris;
    int this_set_end;
    int num_new;

    /* Make 'em planar. Calculate y later anyway */
    p1.y = p2.y = p3.y = 0;

    get_course_divisions( &nx, &nz );
    get_course_dimensions( &width, &length );
    xstep = width/(nx-1);
    zstep = length/(nz-1);

    minx = min(min(p1.x, p2.x), p3.x);
    minz = min(min(p1.z, p2.z), p3.z);
    maxx = max(max(p1.x, p2.x), p3.x);
    maxz = max(max(p1.z, p2.z), p3.z);

    track_tris.current_start = track_tris.next_mark;
    track_tris.current_end = track_tris.next_mark;

    track_tris.tri[track_tris.next_mark].p[0] = p1;
    track_tris.tri[track_tris.next_mark].p[1] = p2;
    track_tris.tri[track_tris.next_mark].p[2] = p3;
    track_tris.tri[track_tris.next_mark].t[0] = t1;
    track_tris.tri[track_tris.next_mark].t[1] = t2;
    track_tris.tri[track_tris.next_mark].t[2] = t3; 

    /*
     * Make lengthwise cuts
     */
    for( i = (int)((minx/xstep)+0.9999); i < (int)(maxx/xstep); i++ ) {
	cut.pt = make_point( i*xstep, 0, 0 );
	cut.nml = make_vector( 1, 0, 0 );
	this_set_end = track_tris.current_end;
	for ( j = track_tris.current_start; j <= this_set_end; j++ ) {
	    num_tris = cut_triangle( &track_tris.tri[j%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+1)%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+2)%MAX_TRIS],
				     cut );
	    track_tris.current_end = (track_tris.current_end + num_tris - 1)%MAX_TRIS;
	}
    }
    
    /*
     * Make cross cuts
     */
    for( i = (int)((minz/zstep)+0.9999); i < (int)(maxz/zstep); i++ ) {
	cut.pt = make_point( 0, 0, i*zstep );
	cut.nml = make_vector( 0, 0, 1 );
	this_set_end = track_tris.current_end;
	for ( j = track_tris.current_start; j <= this_set_end; j++ ) {
	    num_tris = cut_triangle( &track_tris.tri[j%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+1)%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+2)%MAX_TRIS],
				     cut );
	    track_tris.current_end = (track_tris.current_end + num_tris - 1)%MAX_TRIS;
	}
    }

    /*
     * Make diagonal cuts
     */
    for( i = (int)((minx/xstep)+0.9999), j = (int)((minz/zstep)+0.9999);
	 (i < (int)(maxx/xstep)) && (j < (int)(maxz/zstep));
	 i++, j++) {
	if ( (i+j)%2 != 0 ) {
	    i--;
	}
	cut.pt = make_point( i*xstep, 0, j*zstep );
	cut.nml = make_vector( 1, 0, 1 );
	cut.nml.normalize();
	for ( k = track_tris.current_start; k <= this_set_end; k++ ) {
	    num_tris = cut_triangle( &track_tris.tri[k%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+1)%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+2)%MAX_TRIS],
				     cut );
	    track_tris.current_end = (track_tris.current_end + num_tris - 1)%MAX_TRIS;
	}
    }

    /*
     * Make other diagonal cuts
     */
    for( i = (int)((minx/xstep)+0.9999), j = (int)(maxz/zstep);
	 (i < (int)(maxx/xstep)) && (j > (int)((minz/zstep) + 0.9999));
	 i++, j--) {
	if ( (i+j)%2 != 0 ) {
	    i--;
	}
	cut.pt = make_point( i*xstep, 0, j*zstep );
	cut.nml = make_vector( 1, 0, 1 );
	cut.nml.normalize();
	for ( k = track_tris.current_start; k <= this_set_end; k++ ) {
	    num_tris = cut_triangle( &track_tris.tri[k%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+1)%MAX_TRIS],
				     &track_tris.tri[(track_tris.current_end+2)%MAX_TRIS],
				     cut );
	    track_tris.current_end = (track_tris.current_end + num_tris - 1)%MAX_TRIS;
	}
    }


    /* Reset first, next and num_tris */
    if (track_tris.current_start <= track_tris.current_end) {
	num_new = track_tris.current_end - track_tris.current_start + 1;
	track_tris.num_tris = track_tris.num_tris + num_new;
	track_tris.next_mark = (track_tris.current_end+1)%MAX_TRIS;
	if ( ((track_tris.num_tris - num_new) > 0) &&
	     (track_tris.first_mark >= track_tris.current_start) && 
	     (track_tris.first_mark <= track_tris.current_end) ) {
	    track_tris.num_tris = track_tris.num_tris - (track_tris.current_end - track_tris.first_mark + 1);
	    track_tris.first_mark = track_tris.next_mark;
	}

    } else {
	num_new = (track_tris.current_end + 1) + (MAX_TRIS - track_tris.current_start);
	track_tris.num_tris = track_tris.num_tris + num_new;
	track_tris.next_mark = (track_tris.current_end+1)%MAX_TRIS;
	if (track_tris.first_mark >= track_tris.current_start) {
	    track_tris.num_tris = track_tris.num_tris - (track_tris.current_end + 1) - 
		(MAX_TRIS - track_tris.first_mark);
	    track_tris.first_mark = track_tris.next_mark;
	} else if (track_tris.first_mark <= track_tris.current_end) {
	    track_tris.num_tris = track_tris.num_tris - (track_tris.current_end - track_tris.first_mark + 1);
	    track_tris.first_mark = track_tris.next_mark;
	}

    }

    for ( i = 0; i < num_new; i++ ) {
	track_tris.alpha[(track_tris.current_start+i)%MAX_TRIS] = alpha;
	track_tris.track_type[(track_tris.current_start+i)%MAX_TRIS] = track_type;
	track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[0].y = 
	    find_y_coord( track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[0].x, 
			  track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[0].z ) +
	    TRACK_HEIGHT; 
	track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[1].y =
	    find_y_coord( track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[1].x, 
			  track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[1].z ) +
	    TRACK_HEIGHT; 
	track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[2].y = 
	    find_y_coord( track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[2].x, 
			  track_tris.tri[(track_tris.current_start+i)%MAX_TRIS].p[2].z ) +
	    TRACK_HEIGHT; 
    }

}

static void add_tri_tracks_from_quad( track_quad_t *q )
{
    add_tri_tracks_from_tri( q->v1, q->v2, q->v3, q->t1, q->t2, q->t3,
			     &q->track_type, &q->alpha );
    add_tri_tracks_from_tri( q->v2, q->v4, q->v3, q->t2, q->t4, q->t3,
			     &q->track_type, &q->alpha );
}

#endif



void init_track_marks(void)
{
    if(gameMgr->doesRaceHaveToBeRetried()) {
    //do nothing, since we want to keep last try's tracks
    } else {

        track_marks.current_mark = 0;
        track_marks.next_mark = 0;
        track_marks.last_mark_time = -99999;
        track_marks.last_mark_pos = pp::Vec3d(-9999, -9999, -9999);
        continuing_track = false;
#ifdef TRACK_TRIANGLES
        track_tris.first_mark = 0;
        track_tris.next_mark = 0;
        track_tris.num_tris = 0;
#endif
        cracks.current_crack = 0;
        cracks.last_pos = pp::Vec3d(-9999, -9999, -9999);
    }
}

void draw_cracks(void) {
    int curr_crack;
    int first_crack;
    int num_cracks;
    GLuint texid;

    /* opengl-settings should still be ok from draw_track_marks() */
//     set_gl_options( TRACK_MARKS );
//
//     glColor4f( 0, 0, 0, 1);
//
//     glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
//     set_material( pp::Color::white, pp::Color::black, 1.0 );
//     setup_course_lighting();

    get_texture_binding( "crack", &texid );


    num_cracks = MIN(cracks.current_crack, MAX_CRACKS);

    first_crack = cracks.current_crack - num_cracks;

    for (curr_crack = 0; curr_crack < num_cracks; curr_crack++) {
        crack_quad_t *q;

        q = &cracks.quads[(first_crack + curr_crack)%MAX_CRACKS];

        set_material( pp::Color::white, pp::Color::black, 1.0 );

        glBindTexture( GL_TEXTURE_2D, texid );

        glBegin(GL_QUADS);

        glNormal3f( q->n1.x, q->n1.y, q->n1.z );
        glTexCoord2f( q->t1.x, q->t1.y );
        glVertex3f( q->v1.x, q->v1.y, q->v1.z );

        glNormal3f( q->n2.x, q->n2.y, q->n2.z );
        glTexCoord2f( q->t2.x, q->t2.y );
        glVertex3f( q->v2.x, q->v2.y, q->v2.z );

        glNormal3f( q->n4.x, q->n4.y, q->n4.z );
        glTexCoord2f( q->t4.x, q->t4.y );
        glVertex3f( q->v4.x, q->v4.y, q->v4.z );

        glNormal3f( q->n3.x, q->n3.y, q->n3.z );
        glTexCoord2f( q->t3.x, q->t3.y );
        glVertex3f( q->v3.x, q->v3.y, q->v3.z );

        glEnd();
     }
 }


void draw_track_marks(void)
{
#ifdef TRACK_TRIANGLES
    draw_tri_tracks();
#else
    //GLuint texid[NUM_TRACK_TYPES];
    int current_quad, num_quads;
    int first_quad;
    track_quad_t *q, *qnext;
    pp::Color trackColor = pp::Color::white;

    if (getparam_track_marks() == false) {
		return;
    }

    set_gl_options( TRACK_MARKS ); 

    glColor4f( 0, 0, 0, 1);

    //get_texture_binding( "track_head", &texid[TRACK_HEAD] );
    //get_texture_binding( "track_mark", &texid[TRACK_MARK] );
    //get_texture_binding( "track_tail", &texid[TRACK_TAIL] );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    set_material( pp::Color::white, pp::Color::black, 1.0 );
    setup_course_lighting();

    num_quads = MIN( track_marks.current_mark, MAX_TRACK_MARKS -
		     track_marks.next_mark + track_marks.current_mark );
    first_quad = track_marks.current_mark - num_quads;

    for ( current_quad = 0;
	  current_quad < num_quads;
	  current_quad++ ) 
    {
	q = &track_marks.quads[(first_quad + current_quad)%MAX_TRACK_MARKS];

	trackColor.a = q->alpha;
	set_material( trackColor, pp::Color::black, 1.0 );

	//glBindTexture( GL_TEXTURE_2D, texid[q->track_type] );
	
	switch (q->track_type){
		case TRACK_HEAD:
			glBindTexture( GL_TEXTURE_2D, terrain_texture[q->terrain].trackmark.head);
			break;
		case TRACK_MARK:
			glBindTexture( GL_TEXTURE_2D, terrain_texture[q->terrain].trackmark.mark);
			break;
		case TRACK_TAIL:
			glBindTexture( GL_TEXTURE_2D, terrain_texture[q->terrain].trackmark.tail);
			break;		
		default:
			glBindTexture( GL_TEXTURE_2D, terrain_texture[q->terrain].trackmark.mark);
			break;	
	}

	if ((q->track_type == TRACK_HEAD) || (q->track_type == TRACK_TAIL)) { 
	    glBegin(GL_QUADS);
	    
	    glNormal3f( q->n1.x, q->n1.y, q->n1.z );
	    glTexCoord2f( q->t1.x, q->t1.y );
	    glVertex3f( q->v1.x, q->v1.y, q->v1.z );
	
	    glNormal3f( q->n2.x, q->n2.y, q->n2.z );
	    glTexCoord2f( q->t2.x, q->t2.y );
	    glVertex3f( q->v2.x, q->v2.y, q->v2.z );

	    glNormal3f( q->n4.x, q->n4.y, q->n4.z );
	    glTexCoord2f( q->t4.x, q->t4.y );
	    glVertex3f( q->v4.x, q->v4.y, q->v4.z );
	
	    glNormal3f( q->n3.x, q->n3.y, q->n3.z );
	    glTexCoord2f( q->t3.x, q->t3.y );
	    glVertex3f( q->v3.x, q->v3.y, q->v3.z );
	
	    glEnd();

	} else {
	      
	    glBegin(GL_QUAD_STRIP);

	    glNormal3f( q->n2.x, q->n2.y, q->n2.z );
	    glTexCoord2f( q->t2.x, q->t2.y );
	    glVertex3f( q->v2.x, q->v2.y, q->v2.z );

	    glNormal3f( q->n1.x, q->n1.y, q->n1.z );
	    glTexCoord2f( q->t1.x, q->t1.y );
	    glVertex3f( q->v1.x, q->v1.y, q->v1.z );

	    glNormal3f( q->n4.x, q->n4.y, q->n4.z );
	    glTexCoord2f( q->t4.x, q->t4.y );
	    glVertex3f( q->v4.x, q->v4.y, q->v4.z );

	    glNormal3f( q->n3.x, q->n3.y, q->n3.z );
	    glTexCoord2f( q->t3.x, q->t3.y );
	    glVertex3f( q->v3.x, q->v3.y, q->v3.z );
		glEnd();
		glBegin(GL_QUADS);
		
	    qnext = &track_marks.quads[(first_quad+current_quad+1)%MAX_TRACK_MARKS];
	    while (( qnext->track_type == TRACK_MARK ) && (current_quad+1 < num_quads)) {
		current_quad++;
		
		if (q->terrain != qnext->terrain){
			glEnd();
			glBindTexture( GL_TEXTURE_2D, terrain_texture[qnext->terrain].trackmark.mark);
			glBegin(GL_QUADS);		
		}
				
		q = &track_marks.quads[(first_quad+current_quad)%MAX_TRACK_MARKS];
		trackColor.a = qnext->alpha;
		set_material( trackColor, pp::Color::black, 1.0 );
		

	    glNormal3f( q->n1.x, q->n1.y, q->n1.z );
	    glTexCoord2f( q->t1.x, q->t1.y );
	    glVertex3f( q->v1.x, q->v1.y, q->v1.z );
		
		glNormal3f( q->n2.x, q->n2.y, q->n2.z );
	    glTexCoord2f( q->t2.x, q->t2.y );
	    glVertex3f( q->v2.x, q->v2.y, q->v2.z );	
		
		
			
		glNormal3f( q->n4.x, q->n4.y, q->n4.z );
		glTexCoord2f( q->t4.x, q->t4.y );
		glVertex3f( q->v4.x, q->v4.y, q->v4.z );

		glNormal3f( q->n3.x, q->n3.y, q->n3.z );
		glTexCoord2f( q->t3.x, q->t3.y );
		glVertex3f( q->v3.x, q->v3.y, q->v3.z );
		
		qnext = &track_marks.quads[(first_quad+current_quad+1)%MAX_TRACK_MARKS];
	    }
	    glEnd();
	}

    }
#endif

    draw_cracks();
}

void add_crack( Player& plyr )
{
    crack_quad_t *q;
    //double terrain_weights[NumTerrains];
    pp::Vec3d normal_force;
    double force;
    double half_size;
    pp::Vec3d dist;


    /*
    get_surface_type(plyr.pos.x, plyr.pos.z, terrain_weights);
    if (terrain_weights[Snow] > 0.5) {
      return;
    }
    */

    normal_force = plyr.normal_force;
    force = normal_force.normalize();

    if (force < CRACK_MIN_FORCE)
      return;

    half_size = force * CRACK_SIZE_PER_FORCE / 2.0;
   
    dist = plyr.pos - cracks.last_pos;
    if (dist.normalize() <= CRACK_MAX_UNIFY_DISTANCE) {
       cracks.current_crack--;
        half_size += cracks.last_size;
    }

    /* Too big cracks may hang around in the air, if the terrain is not plane */
    if (half_size > CRACK_MAX_SIZE / 2.0)
        half_size = CRACK_MAX_SIZE / 2.0;

    q = &cracks.quads[cracks.current_crack%MAX_TRACK_MARKS];


    q->v1 = pp::Vec3d( plyr.pos.x - half_size,
           find_y_coord(plyr.pos.x - half_size, plyr.pos.z - half_size) + CRACK_HEIGHT,
                                            plyr.pos.z - half_size  );
    q->v2 = pp::Vec3d( plyr.pos.x + half_size,
           find_y_coord(plyr.pos.x + half_size, plyr.pos.z - half_size) + CRACK_HEIGHT,
                                            plyr.pos.z - half_size  );
    q->v3 = pp::Vec3d( plyr.pos.x - half_size,
           find_y_coord(plyr.pos.x - half_size, plyr.pos.z + half_size) + CRACK_HEIGHT,
                                            plyr.pos.z + half_size  );
    q->v4 = pp::Vec3d( plyr.pos.x + half_size,
           find_y_coord(plyr.pos.x + half_size, plyr.pos.z + half_size) + CRACK_HEIGHT,
                                            plyr.pos.z + half_size  );

    /* Big cracks tended to hang around in the air. Reduce the probability that
       this happens: */
    double center_height = (q->v1.y + q->v2.y + q->v3.y + q->v4.y) / 4
                           - find_y_coord(plyr.pos.x, plyr.pos.z);
    if (center_height > MAX_CRACK_HEIGHT) {
        center_height -= MAX_CRACK_HEIGHT;
        q->v1.y -= center_height;
        q->v2.y -= center_height;
        q->v3.y -= center_height;
        q->v4.y -= center_height;
    }
   
    q->n1 = find_course_normal( q->v1.x, q->v1.z);
    q->n2 = find_course_normal( q->v2.x, q->v2.z);
    q->n3 = find_course_normal( q->v3.x, q->v3.z);
    q->n4 = find_course_normal( q->v4.x, q->v4.z);
    q->t1 = pp::Vec2d(0.0, 0.0);
    q->t2 = pp::Vec2d(1.0, 0.0);
    q->t3 = pp::Vec2d(0.0, 1.0);
    q->t4 = pp::Vec2d(1.0, 1.0);
    cracks.current_crack++;

    cracks.last_pos = plyr.pos;
    cracks.last_size = half_size;
}

void break_track_marks( void )
{
    track_quad_t *qprev, *qprevprev;
    qprev = &track_marks.quads[(track_marks.current_mark-1)%MAX_TRACK_MARKS];
    qprevprev = &track_marks.quads[(track_marks.current_mark-2)%MAX_TRACK_MARKS];

    if (track_marks.current_mark > 0) {
	qprev->track_type = TRACK_TAIL;
	qprev->t1 = pp::Vec2d(0.0, 0.0);
	qprev->t2 = pp::Vec2d(1.0, 0.0);
	qprev->t3 = pp::Vec2d(0.0, 1.0);
	qprev->t4 = pp::Vec2d(1.0, 1.0);
	qprevprev->t3.y = MAX((int)(qprevprev->t3.y+0.5), (int)(qprevprev->t1.y+1));
	qprevprev->t4.y = MAX((int)(qprevprev->t3.y+0.5), (int)(qprevprev->t1.y+1));
    }
    track_marks.last_mark_time = -99999;
    track_marks.last_mark_pos = pp::Vec3d(-9999, -9999, -9999);
    continuing_track = false;
}

void add_track_mark( Player& plyr )
{
    pp::Vec3d width_vector;
    pp::Vec3d left_vector;
    pp::Vec3d right_vector;
    float magnitude;
    track_quad_t *q, *qprev, *qprevprev;
    pp::Vec3d vel;
    float speed;
    pp::Vec3d left_wing, right_wing;
    float left_y, right_y;
    float dist_from_surface;
    pp::Plane surf_plane;
    float comp_depth;
    float tex_end;
    float terrain_weights[NUM_TERRAIN_TYPES];
    float dist_from_last_mark;
    pp::Vec3d vector_from_last_mark;
	bool break_marks;
	float terrain_compression=0;
	float old_terrain_weight=0;
	unsigned int i;

    if (getparam_track_marks() == false) {
	return;
    }
    
    add_crack(plyr);

    q = &track_marks.quads[track_marks.current_mark%MAX_TRACK_MARKS];
    qprev = &track_marks.quads[(track_marks.current_mark-1)%MAX_TRACK_MARKS];
    qprevprev = &track_marks.quads[(track_marks.current_mark-2)%MAX_TRACK_MARKS];

    vector_from_last_mark = plyr.pos - track_marks.last_mark_pos;
    dist_from_last_mark = vector_from_last_mark.normalize();
	
	
    get_surface_type(plyr.pos.x, plyr.pos.z, terrain_weights);
    
	break_marks=true;
	for (i=0;i<num_terrains;i++){
		if (terrain_texture[i].trackmark.mark !=0){	
			if (terrain_weights[i] >= 0.5) {
				if (old_terrain_weight < terrain_weights[i]) {
					break_marks=false;
					terrain_compression = get_compression_depth(i);
					q->terrain=i;
					old_terrain_weight = terrain_weights[i];
				}
    		}
		}
	}
	
	if (break_marks==true){
		break_track_marks();
		return;
	}

    vel = plyr.vel;
    speed = vel.normalize();
    if (speed < SPEED_TO_START_TRENCH) {
	break_track_marks();
	return;
    }

    width_vector = plyr.direction^pp::Vec3d( 0, 1, 0 );
    magnitude = width_vector.normalize();
    if ( magnitude == 0 ) {
	break_track_marks();
	return;
    }

    left_vector = (TRACK_WIDTH/2.0)*width_vector;
    right_vector = (-TRACK_WIDTH/2.0)*width_vector;
    left_wing =  plyr.pos - left_vector;
    right_wing = plyr.pos - right_vector;
    left_y = find_y_coord( left_wing.x, left_wing.z );
    right_y = find_y_coord( right_wing.x, right_wing.z );
    if (fabs(left_y-right_y) > MAX_TRACK_DEPTH) {
	break_track_marks();
	return;
    }

    surf_plane = get_local_course_plane( plyr.pos );
    dist_from_surface = surf_plane.distance( plyr.pos );
    comp_depth = terrain_compression;
    if ( dist_from_surface >= (2*comp_depth) ) {
	break_track_marks();
	return;
    }

    if (!continuing_track) {
	break_track_marks();
	q->track_type = TRACK_HEAD;
	q->v1 = pp::Vec3d( left_wing.x, left_y + TRACK_HEIGHT, left_wing.z );
	q->v2 = pp::Vec3d( right_wing.x, right_y + TRACK_HEIGHT, right_wing.z );
	q->n1 = find_course_normal( q->v1.x, q->v1.z);
	q->n2 = find_course_normal( q->v2.x, q->v2.z);
	q->t1 = pp::Vec2d(0.0, 0.0);
	q->t2 = pp::Vec2d(1.0, 0.0);
	track_marks.next_mark = track_marks.current_mark + 1;
    } else {
	if ( track_marks.next_mark == track_marks.current_mark ) {
	    q->v1 = qprev->v3;
	    q->v2 = qprev->v4;
	    q->n1 = qprev->n3;
	    q->n2 = qprev->n4;
	    q->t1 = qprev->t3; 
	    q->t2 = qprev->t4;
	    if ( qprev->track_type != TRACK_HEAD ) {
		qprev->track_type = TRACK_MARK;
	    }
	    q->track_type = TRACK_MARK;
	}
	q->v3 = pp::Vec3d( left_wing.x, left_y + TRACK_HEIGHT, left_wing.z );
	q->v4 = pp::Vec3d( right_wing.x, right_y + TRACK_HEIGHT, right_wing.z );
	q->n3 = find_course_normal( q->v3.x, q->v3.z);
	q->n4 = find_course_normal( q->v4.x, q->v4.z);
	tex_end = speed*gameMgr->getTimeStep()/TRACK_WIDTH;
	if (q->track_type == TRACK_HEAD) {
	    q->t3= pp::Vec2d(0.0, 1.0);
	    q->t4= pp::Vec2d(1.0, 1.0);
	} else {
	    q->t3 = pp::Vec2d(0.0, q->t1.y + tex_end);
	    q->t4 = pp::Vec2d(1.0, q->t2.y + tex_end);
	}

#ifdef TRACK_TRIANGLES
	add_tri_tracks_from_quad(q);
#endif
	track_marks.current_mark++;
	track_marks.next_mark = track_marks.current_mark;
    }

    q->alpha = MIN( (2*comp_depth-dist_from_surface)/(4*comp_depth), 1.0 );
		
    track_marks.last_mark_time = gameMgr->time;
    continuing_track = true;

}
