/* 
 * Tux Racer 
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

#include "tuxracer.h"
#include "fonts.h"
#include "gl_util.h"
#include "textures.h"
#include "fps.h"
#include "phys_sim.h"
#include "multiplayer.h"
#include "ui_mgr.h"
#include "game_logic_util.h"

#define SECONDS_IN_MINUTE 60

#define TIME_LABEL_X_OFFSET 12.0
#define TIME_LABEL_Y_OFFSET 12.0

#define TIME_X_OFFSET 30.0
#define TIME_Y_OFFSET 5.0

#define HERRING_ICON_HEIGHT 30.0
#define HERRING_ICON_WIDTH 48.0
#define HERRING_ICON_IMG_SIZE 64.0
#define HERRING_ICON_X_OFFSET 160.0
#define HERRING_ICON_Y_OFFSET 41.0
#define HERRING_COUNT_Y_OFFSET 37.0

#define GAUGE_IMG_SIZE 128

#define ENERGY_GAUGE_BOTTOM 3.0
#define ENERGY_GAUGE_HEIGHT 103.0
#define ENERGY_GAUGE_CENTER_X 71.0
#define ENERGY_GAUGE_CENTER_Y 55.0

#define GAUGE_WIDTH 127.0
#define SPEED_UNITS_Y_OFFSET 4.0

#define SPEEDBAR_OUTER_RADIUS ( ENERGY_GAUGE_CENTER_X )
#define SPEEDBAR_BASE_ANGLE 225
#define SPEEDBAR_MAX_ANGLE 45
#define SPEEDBAR_GREEN_MAX_SPEED ( MAX_PADDLING_SPEED * M_PER_SEC_TO_KM_PER_H )
#define SPEEDBAR_YELLOW_MAX_SPEED 100
#define SPEEDBAR_RED_MAX_SPEED 160
#define SPEEDBAR_GREEN_FRACTION 0.5
#define SPEEDBAR_YELLOW_FRACTION 0.25
#define SPEEDBAR_RED_FRACTION 0.25

#define FPS_X_OFFSET 12
#define FPS_Y_OFFSET 12


static GLfloat energy_background_color[] = { 0.2, 0.2, 0.2, 0.5 };
static GLfloat energy_foreground_color[] = { 0.54, 0.59, 1.00, 0.5 };
static GLfloat speedbar_background_color[] = { 0.2, 0.2, 0.2, 0.5 };
static GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };

static void draw_time()
{
    font_t *font;
    int minutes, seconds, hundredths;
    char *string;
    int w, asc, desc;
    char *binding;
    char buff[BUFF_LEN];
    scalar_t time_y_refval;

    get_time_components( g_game.time, &minutes, &seconds, &hundredths );

    binding = "time_label";

    if ( ! get_font_binding( binding, &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", binding );
	return;
    }

    bind_font_texture( font );
    set_gl_options( TEXFONT );
    glColor3f( 1, 1, 1 );

    string = "Time";

    get_font_metrics( font, string, &w, &asc, &desc );

    glPushMatrix();
    {
	glTranslatef( TIME_LABEL_X_OFFSET, 
		      getparam_y_resolution() - TIME_LABEL_Y_OFFSET - 
		      asc, 
		      0 );
	draw_string( font, string );
    }
    glPopMatrix();


    time_y_refval = getparam_y_resolution() - TIME_LABEL_Y_OFFSET - asc;

    binding = "time_value";

    if ( ! get_font_binding( binding, &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", binding );
	return;
    }

    bind_font_texture( font );

    sprintf( buff, "%02d:%02d", minutes, seconds );

    string = buff;

    get_font_metrics( font, string, &w, &asc, &desc );

    glPushMatrix();
    {
	glTranslatef( TIME_X_OFFSET, 
		      time_y_refval - TIME_Y_OFFSET - asc, 
		      0 );
	draw_string( font, string );
    }
    glPopMatrix();

    binding = "time_hundredths";

    if ( ! get_font_binding( binding, &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", binding );
	return;
    }

    bind_font_texture( font );

    sprintf( buff, "%02d", hundredths );
    string = buff;

    glPushMatrix();
    {
	glTranslatef( TIME_X_OFFSET + w + 5, 
		      time_y_refval - TIME_Y_OFFSET,  
		      0 );
	get_font_metrics( font, string, &w, &asc, &desc );
	glTranslatef( 0, -asc-2, 0 );
	draw_string( font, string );
    }
    glPopMatrix();
}

static void draw_herring_count( int herring_count )
{
    char *string;
    char buff[BUFF_LEN];
    GLuint texobj;
    font_t *font;
    char *binding;
    int w, asc, desc;

    set_gl_options( TEXFONT );
    glColor3f( 1.0, 1.0, 1.0 );

    binding = "herring_icon";

    if ( !get_texture_binding( binding, &texobj ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
	return;
    }

    binding = "herring_count";

    if ( !get_font_binding( binding, &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", binding );
	return;
    }

    sprintf( buff, " x %03d", herring_count ); 

    string = buff;

    get_font_metrics( font, string, &w, &asc, &desc );

    glBindTexture( GL_TEXTURE_2D, texobj );

    glPushMatrix();
    {
	glTranslatef( getparam_x_resolution() - HERRING_ICON_X_OFFSET,
		      getparam_y_resolution() - HERRING_ICON_Y_OFFSET - asc, 
		      0 );

	glBegin( GL_QUADS );
	{
	    glTexCoord2f( 0, 0 );
	    glVertex2f( 0, 0 );

	    glTexCoord2f( (GLfloat) HERRING_ICON_WIDTH / HERRING_ICON_IMG_SIZE,
			  0 );
	    glVertex2f( HERRING_ICON_WIDTH, 0 );

	    glTexCoord2f( 
		(GLfloat)HERRING_ICON_WIDTH / HERRING_ICON_IMG_SIZE,
		(GLfloat)HERRING_ICON_HEIGHT / HERRING_ICON_IMG_SIZE );
	    glVertex2f( HERRING_ICON_WIDTH, HERRING_ICON_HEIGHT );

	    glTexCoord2f( 
		0,
		(GLfloat)HERRING_ICON_HEIGHT / HERRING_ICON_IMG_SIZE );
	    glVertex2f( 0, HERRING_ICON_HEIGHT );
	}
	glEnd();

	
	bind_font_texture( font );

	glTranslatef( HERRING_ICON_WIDTH, 
		      HERRING_ICON_Y_OFFSET -  HERRING_COUNT_Y_OFFSET,
		      0 );

	draw_string( font, string );
    }
    glPopMatrix();

    
}

#define CIRCLE_DIVISIONS 10

point2d_t calc_new_fan_pt( scalar_t angle )
{
    point2d_t pt;
    pt.x = ENERGY_GAUGE_CENTER_X + cos( ANGLES_TO_RADIANS( angle ) ) *
	SPEEDBAR_OUTER_RADIUS;
    pt.y = ENERGY_GAUGE_CENTER_Y + sin( ANGLES_TO_RADIANS( angle ) ) *
	SPEEDBAR_OUTER_RADIUS;

    return pt;
}

void start_tri_fan()
{
    point2d_t pt;

    glBegin( GL_TRIANGLE_FAN );
    glVertex2f( ENERGY_GAUGE_CENTER_X, 
		ENERGY_GAUGE_CENTER_Y );

    pt = calc_new_fan_pt( SPEEDBAR_BASE_ANGLE ); 

    glVertex2f( pt.x, pt.y );
}

void draw_partial_tri_fan( scalar_t fraction )
{
    int divs;
    scalar_t angle, angle_incr, cur_angle;
    int i;
    bool_t trifan = False;
    point2d_t pt;

    angle = SPEEDBAR_BASE_ANGLE + 
	( SPEEDBAR_MAX_ANGLE - SPEEDBAR_BASE_ANGLE ) * fraction;

    divs = (int) ( SPEEDBAR_BASE_ANGLE - angle ) * CIRCLE_DIVISIONS / 360.0;

    cur_angle = SPEEDBAR_BASE_ANGLE;

    angle_incr = 360.0 / CIRCLE_DIVISIONS;

    for (i=0; i<divs; i++) {
	if ( !trifan ) {
	    start_tri_fan();
	    trifan = True;
	}

	cur_angle -= angle_incr;

	pt = calc_new_fan_pt( cur_angle );

	glVertex2f( pt.x, pt.y );
    }

    if ( cur_angle > angle + EPS ) {
	cur_angle = angle;
	if ( !trifan ) {
	    start_tri_fan();
	    trifan = True;
	}

	pt = calc_new_fan_pt( cur_angle );

	glVertex2f( pt.x, pt.y );
    }

    if ( trifan ) {
	glEnd();
	trifan = False;
    }
}

void draw_gauge( scalar_t speed, scalar_t energy )
{
    char *binding;
    GLfloat xplane[4] = { 1.0/GAUGE_IMG_SIZE, 0.0, 0.0, 0.0 };
    GLfloat yplane[4] = { 0.0, 1.0/GAUGE_IMG_SIZE, 0.0, 0.0 };
    GLuint energymask_texobj, speedmask_texobj, outline_texobj;
    font_t *speed_font;
    font_t *units_font;
    int w, asc, desc;
    char *string;
    char buff[BUFF_LEN];
    scalar_t y;
    scalar_t speedbar_frac;

    set_gl_options( GAUGE_BARS );

    binding = "gauge_energy_mask";
    if ( !get_texture_binding( binding, &energymask_texobj ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
	return;
    }

    binding = "gauge_speed_mask";
    if ( !get_texture_binding( binding, &speedmask_texobj ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
	return;
    }

    binding = "gauge_outline";
    if ( !get_texture_binding( binding, &outline_texobj ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
	return;
    }

    binding = "speed_digits";
    if ( !get_font_binding( binding, & speed_font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", speed_font );
    }

    binding = "speed_units";
    if ( !get_font_binding( binding, &units_font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", speed_font );
    }


    glTexGenfv( GL_S, GL_OBJECT_PLANE, xplane );
    glTexGenfv( GL_T, GL_OBJECT_PLANE, yplane );

    glPushMatrix();
    {
	glTranslatef( getparam_x_resolution() - GAUGE_WIDTH,
		      0,
		      0 );

	glColor4fv( energy_background_color );

	glBindTexture( GL_TEXTURE_2D, energymask_texobj );

	y = ENERGY_GAUGE_BOTTOM + energy * ENERGY_GAUGE_HEIGHT;

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, y );
	    glVertex2f( GAUGE_IMG_SIZE, y );
	    glVertex2f( GAUGE_IMG_SIZE, GAUGE_IMG_SIZE );
	    glVertex2f( 0.0, GAUGE_IMG_SIZE );
	}
	glEnd();

	glColor4fv( energy_foreground_color );

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, 0.0 );
	    glVertex2f( GAUGE_IMG_SIZE, 0.0 );
	    glVertex2f( GAUGE_IMG_SIZE, y );
	    glVertex2f( 0.0, y );
	}
	glEnd();


	/* Calculate the fraction of the speed bar to fill */
	speedbar_frac = 0.0;

	if ( speed > SPEEDBAR_GREEN_MAX_SPEED ) {
	    speedbar_frac = SPEEDBAR_GREEN_FRACTION;
	    
	    if ( speed > SPEEDBAR_YELLOW_MAX_SPEED ) {
		speedbar_frac += SPEEDBAR_YELLOW_FRACTION;
		
		if ( speed > SPEEDBAR_RED_MAX_SPEED ) {
		    speedbar_frac += SPEEDBAR_RED_FRACTION;
		} else {
		    speedbar_frac +=
			( speed - SPEEDBAR_YELLOW_MAX_SPEED ) /
			( SPEEDBAR_RED_MAX_SPEED - SPEEDBAR_YELLOW_MAX_SPEED ) *
			SPEEDBAR_RED_FRACTION;
		}

	    } else {
		speedbar_frac += 
		    ( speed - SPEEDBAR_GREEN_MAX_SPEED ) /
		    ( SPEEDBAR_YELLOW_MAX_SPEED - SPEEDBAR_GREEN_MAX_SPEED ) *
		    SPEEDBAR_YELLOW_FRACTION;
	    }
	    
	} else {
	    speedbar_frac +=  speed/SPEEDBAR_GREEN_MAX_SPEED * 
		SPEEDBAR_GREEN_FRACTION;
	}

	glColor4fv( speedbar_background_color );

	glBindTexture( GL_TEXTURE_2D, speedmask_texobj );

	draw_partial_tri_fan( 1.0 );

	glColor4fv( white );

	draw_partial_tri_fan( min( 1.0, speedbar_frac ) );



	glColor4fv( white );

	glBindTexture( GL_TEXTURE_2D, outline_texobj );

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, 0.0 );
	    glVertex2f( GAUGE_IMG_SIZE, 0.0 );
	    glVertex2f( GAUGE_IMG_SIZE, GAUGE_IMG_SIZE );
	    glVertex2f( 0.0, GAUGE_IMG_SIZE );
	}
	glEnd();



	sprintf( buff, "%d", (int)speed );
	string = buff;

	get_font_metrics( speed_font, string, &w, &asc, &desc );

	bind_font_texture( speed_font);
	set_gl_options( TEXFONT );
	glColor4f( 1, 1, 1, 1 );

	glPushMatrix();
	{
	    glTranslatef( ENERGY_GAUGE_CENTER_X - w/2.0,
			  ENERGY_GAUGE_BOTTOM + ENERGY_GAUGE_HEIGHT / 2.0,
			  0 );
	    draw_string( speed_font, string );
	    
	}
	glPopMatrix();

	string = "km/h";

	get_font_metrics( units_font, string, &w, &asc, &desc );

	bind_font_texture( units_font );

	glPushMatrix();
	{
	    glTranslatef( ENERGY_GAUGE_CENTER_X - w/2.0,
			  ENERGY_GAUGE_BOTTOM + ENERGY_GAUGE_HEIGHT / 2.0
			  - asc - SPEED_UNITS_Y_OFFSET,
			  0 );
	    draw_string( units_font, string );
	    
	}
	glPopMatrix();
	
    }
    glPopMatrix();
	
}

void print_fps()
{
    char buff[BUFF_LEN];
    char *string;
    char *binding;
    font_t *font;

    /* This is needed since this can be called from outside */
    ui_setup_display();

    if ( ! getparam_display_fps() ) {
	return;
    }

    binding = "fps";
    if ( !get_font_binding( binding, &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", binding );
	return;
    }

    bind_font_texture( font );
    set_gl_options( TEXFONT );
    glColor3f( 1, 1, 1 );

    sprintf( buff, "FPS: %.1f", get_fps() );
    string = buff;

    glPushMatrix();
    {
	glTranslatef( FPS_X_OFFSET,
		      FPS_Y_OFFSET,
		      0 );
	draw_string( font, string );
    }
    glPopMatrix();

}

void draw_hud( player_data_t *plyr )
{
    vector_t vel;
    scalar_t speed;

    vel = plyr->vel;
    speed = normalize_vector( &vel );

    ui_setup_display();

    draw_gauge( speed * M_PER_SEC_TO_KM_PER_H, plyr->control.jump_amt );
    draw_time();
    draw_herring_count( plyr->herring );

    print_fps();
}
