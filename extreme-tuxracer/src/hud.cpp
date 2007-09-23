/* 
 * PPRacer 
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

#include "gl_util.h"
#include "fps.h"
#include "phys_sim.h"
#include "course_mgr.h"
#include "game_config.h"
#include "hud.h"

#include "stuff.h"
#include "game_mgr.h"

#include "ppgltk/ui_mgr.h"
#include "ppgltk/alg/defs.h"

HUD HUD1;

HUD::Element::Element()
 : type(-1), 
   x(0), y(0),
   font(NULL),
   texture(0),
   width(0), height(0),
   size(0),
   angle(0)   
{
}

HUD::HUD()
{
	m_numElements=0;	
}

void
HUD::reset()
{
	m_numElements=0;
	initGauge();
}

bool
HUD::add(Element newElement){
	if( m_numElements < HUD_MAX_ITEMS){
		m_element[m_numElements] = newElement;
		m_numElements++;
		return true;
	}else{
		return false;
	}	
}
	
bool
HUD::update(const int i, Element newElement){
	if( m_numElements > i ){
		m_element[i] = newElement;
		return true;	
	}else{
		//element i not available
		return false;
	}	
}

void
HUD::draw(Player& plyr)
{
	UIMgr.setupDisplay();
	set_gl_options( TEXFONT );
	
	for(int i=0; i<m_numElements; i++){	
		switch(m_element[i].type){
			case 0:
				text(i);
				break;
			case 1:
				fps(i);
				break;
			case 2:
				herring(i,plyr.herring);
				break;
			case 3:
				image(i);
				break;
			case 4:
				time(i);
				break;	
			case 5:
				{
				pp::Vec3d vel = plyr.vel;
    			speed(i,vel.normalize()* M_PER_SEC_TO_KM_PER_H);
				}
				break;
			case 6:
				{
				pp::Vec3d vel = plyr.vel;
				gauge(i,vel.normalize()* M_PER_SEC_TO_KM_PER_H,plyr.control.jump_amt);
				}
				break;
			case 7:
				bar(i,plyr.control.jump_amt);
				break;
			case 8:
				{
				pp::Vec3d vel = plyr.vel;
				bar(i,vel.normalize()* M_PER_SEC_TO_KM_PER_H/135);
				}	
				break;
			case 9:
				coursePercentage(i);
				break;
			case 10:
				if ( getparam_display_course_percentage() ){		
					bar(i,players[0].getCoursePercentage()/100);
				}
				break;
		}
	}
}

void
HUD::text(const int i)
{
    if(m_element[i].font){
		fix_xy(m_element[i].x, m_element[i].y,
			   m_element[i].height, m_element[i].width);
		
		m_element[i].font->draw(m_element[i].string.c_str(),
				   m_element[i].x, m_element[i].y);		
	}
}

void
HUD::fps(const int i)
{
	if ( ! getparam_display_fps() ) {
		return;
    }
	
    if(m_element[i].font){
		char string[BUFF_LEN];
		sprintf( string, m_element[i].string.c_str(), fpsCounter.get() );
		
		pp::Font::utf8ToUnicode(m_element[i].u_string,string);
		int width = int(m_element[i].font->advance(m_element[i].u_string));
		
		fix_xy(m_element[i].x,m_element[i].y,m_element[i].height,width);
		m_element[i].font->draw(m_element[i].u_string, m_element[i].x, m_element[i].y);		
	}
}

void
HUD::herring(const int i, const int herring_count)
{
	if(m_element[i].font){
		char string[BUFF_LEN];
		sprintf( string, m_element[i].string.c_str(), herring_count );
		
		pp::Font::utf8ToUnicode(m_element[i].u_string,string);
		int width = int(m_element[i].font->advance(m_element[i].u_string));
		
		fix_xy(m_element[i].x,m_element[i].y,m_element[i].height,width);
		m_element[i].font->draw(m_element[i].u_string, m_element[i].x, m_element[i].y);		
	}
}

void
HUD::image(const int i)
{
	if(!m_element[i].texture) return;

	if ( !getparam_display_course_percentage() && m_element[i].texture == 47) {
		return;
	}

    glColor3f( 1.0, 1.0, 1.0 );

    glBindTexture( GL_TEXTURE_2D, m_element[i].texture );

	fix_xy( m_element[i].x, m_element[i].y, m_element[i].height, m_element[i].width);
	
    glPushMatrix();
    {
	glTranslatef( m_element[i].x, m_element[i].y,0);

	glBegin( GL_QUADS );
	{
	   	glTexCoord2f( 0, 0 );
	    glVertex2f( 0, 0 );

	    glTexCoord2f( double(m_element[i].width) / m_element[i].size,
			  0 );
	    glVertex2f( m_element[i].width, 0 );

	    glTexCoord2f( 
			double(m_element[i].width) / m_element[i].size,
			double(m_element[i].height) / m_element[i].size );
	    glVertex2f( m_element[i].width, m_element[i].height );

	    glTexCoord2f( 
			0,
			double(m_element[i].height) / m_element[i].size );
	    glVertex2f( 0, m_element[i].height );
	}
	glEnd();
    }
    glPopMatrix();
}

void
HUD::time(const int i)
{
    if(m_element[i].font){
		char string[BUFF_LEN];
		int minutes, seconds, hundredths;
		    
		getTimeComponents( gameMgr->time, minutes, seconds, hundredths );
		sprintf( string, m_element[i].string.c_str(), minutes, seconds, hundredths);
		
		pp::Font::utf8ToUnicode(m_element[i].u_string,string);
		int width = int(m_element[i].font->advance(m_element[i].u_string));
		
		fix_xy(m_element[i].x,m_element[i].y,m_element[i].height,width);
		m_element[i].font->draw(m_element[i].u_string, m_element[i].x, m_element[i].y);		
	
	}
}

void
HUD::speed(const int i, const double speed)
{
	if(m_element[i].font){
		char string[BUFF_LEN];
		sprintf( string, m_element[i].string.c_str(), speed );
		
		pp::Font::utf8ToUnicode(m_element[i].u_string,string);
		int width = int(m_element[i].font->advance(m_element[i].u_string));
		
		fix_xy(m_element[i].x,m_element[i].y,m_element[i].height,width);
		m_element[i].font->draw(m_element[i].u_string, m_element[i].x-(width/2), m_element[i].y);		
		
	}	
}

void
HUD::bar(const int i, double percentage)
{
	if(!m_element[i].texture) return;
	
	if(percentage>1)percentage=1;
	
	double temp_sin=sin(double(m_element[i].angle)/180.0*M_PI);
	double temp_cos=cos(double(m_element[i].angle)/180.0*M_PI);
	
    glBindTexture( GL_TEXTURE_2D, m_element[i].texture );

	fix_xy(m_element[i].x,m_element[i].y,int(m_element[i].height));
	
    glPushMatrix();
    {
	glTranslatef(m_element[i].x, m_element[i].y,0);

	glBegin( GL_QUADS );
	{
		glTexCoord2f(0,0);
	    glVertex2f(0,0);

	    glTexCoord2f(1,0);
	    glVertex2f(temp_cos*m_element[i].width,temp_sin*m_element[i].width);

    	glTexCoord2f(1,percentage);
    	glVertex2f(temp_cos*m_element[i].width+temp_sin*m_element[i].height*percentage,
			temp_sin*m_element[i].width-temp_cos*m_element[i].height*percentage);

    	glTexCoord2f(0,percentage);
    	glVertex2f(temp_sin*m_element[i].height*percentage, (-1)*temp_cos*m_element[i].height*percentage);

	}
	glEnd();
    }
    glPopMatrix();
	
}

//Sollte man bei Gelegenheit noch konfigurierbar machen
#define ENERGY_GAUGE_BOTTOM 3.0
#define ENERGY_GAUGE_CENTER_X 71.0
#define ENERGY_GAUGE_CENTER_Y 55.0
#define SPEEDBAR_OUTER_RADIUS ( ENERGY_GAUGE_CENTER_X )
#define SPEEDBAR_BASE_ANGLE 225
#define SPEEDBAR_MAX_ANGLE 45
#define SPEEDBAR_GREEN_MAX_SPEED ( MAX_PADDLING_SPEED * M_PER_SEC_TO_KM_PER_H )
#define SPEEDBAR_YELLOW_MAX_SPEED 100
#define SPEEDBAR_RED_MAX_SPEED 160
#define SPEEDBAR_GREEN_FRACTION 0.5
#define SPEEDBAR_YELLOW_FRACTION 0.25
#define SPEEDBAR_RED_FRACTION 0.25
#define SPEED_UNITS_Y_OFFSET 4.0

static GLfloat energy_background_color[] = { 0.2, 0.2, 0.2, 0.5 };
static GLfloat energy_foreground_color[] = { 0.54, 0.59, 1.00, 0.5 };
static GLfloat speedbar_background_color[] = { 0.2, 0.2, 0.2, 0.5 };
static GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };

void
HUD::initGauge()
{
    const char *binding;
	
	binding = "gauge_energy_mask";
    if ( !get_texture_binding( binding, &m_energymaskTex ) ) {
		print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
    }
	
	binding = "gauge_speed_mask";
    if ( !get_texture_binding( binding, &m_speedmaskTex ) ) {
		print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
    }
	    
	binding = "gauge_outline";
    if ( !get_texture_binding( binding, &m_outlineTex ) ) {
		print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", binding );
    }
}


void
HUD::gauge(const int i, const double speed, const double energy)
{
	GLfloat xplane[4] = { 1.0/m_element[i].size, 0.0, 0.0, 0.0 };
    GLfloat yplane[4] = { 0.0, 1.0/m_element[i].size, 0.0, 0.0 };
    double y;
    double speedbar_frac;

	//the gauge bar needs it's own mode
	//we reset the mode at the end of the function
    set_gl_options( GAUGE_BARS );

    glTexGenfv( GL_S, GL_OBJECT_PLANE, xplane );
    glTexGenfv( GL_T, GL_OBJECT_PLANE, yplane );

    glPushMatrix();
    {
	glTranslatef( getparam_x_resolution() - m_element[i].width,
		      0,
		      0 );

	glColor4fv( energy_background_color );

	glBindTexture( GL_TEXTURE_2D, m_energymaskTex );

	y = ENERGY_GAUGE_BOTTOM + energy * m_element[i].height;

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, y );
	    glVertex2f( m_element[i].size, y );
	    glVertex2f( m_element[i].size, m_element[i].size );
	    glVertex2f( 0.0, m_element[i].size );
	}
	glEnd();

	glColor4fv( energy_foreground_color );

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, 0.0 );
	    glVertex2f( m_element[i].size, 0.0 );
	    glVertex2f( m_element[i].size, y );
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

	glBindTexture( GL_TEXTURE_2D, m_speedmaskTex );

	draw_partial_tri_fan( 1.0 );

	glColor4fv( white );

	draw_partial_tri_fan( MIN( 1.0, speedbar_frac ) );

	glColor4fv( white );

	glBindTexture( GL_TEXTURE_2D, m_outlineTex );

	glBegin( GL_QUADS );
	{
	    glVertex2f( 0.0, 0.0 );
	    glVertex2f( m_element[i].size, 0.0 );
	    glVertex2f( m_element[i].size, m_element[i].size );
	    glVertex2f( 0.0, m_element[i].size );
	}
	glEnd();
	
    }
    glPopMatrix();
	
	//we reset this because all other elements need TEXFONT
	set_gl_options( TEXFONT );
}

#define CIRCLE_DIVISIONS 10

void 
HUD::draw_partial_tri_fan(const double fraction)
{
    int divs;
    double angle, angle_incr, cur_angle;
    int i;
    bool trifan = false;
    pp::Vec2d pt;

    angle = SPEEDBAR_BASE_ANGLE + 
	( SPEEDBAR_MAX_ANGLE - SPEEDBAR_BASE_ANGLE ) * fraction;

    divs = int(( SPEEDBAR_BASE_ANGLE - angle ) * CIRCLE_DIVISIONS / 360.0);

    cur_angle = SPEEDBAR_BASE_ANGLE;

    angle_incr = 360.0 / CIRCLE_DIVISIONS;

    for (i=0; i<divs; i++) {
	if ( !trifan ) {
	    start_tri_fan();
	    trifan = true;
	}

	cur_angle -= angle_incr;

	pt = calc_new_fan_pt( cur_angle );

	glVertex2f( pt.x, pt.y );
    }

    if ( cur_angle > angle + EPS ) {
	cur_angle = angle;
	if ( !trifan ) {
	    start_tri_fan();
	    trifan = true;
	}

	pt = calc_new_fan_pt( cur_angle );

	glVertex2f( pt.x, pt.y );
    }

    if ( trifan ) {
	glEnd();
	trifan = false;
    }
}

pp::Vec2d
HUD::calc_new_fan_pt(const double angle )
{
    pp::Vec2d pt;
    pt.x = ENERGY_GAUGE_CENTER_X + cos( ANGLES_TO_RADIANS( angle ) ) *
	SPEEDBAR_OUTER_RADIUS;
    pt.y = ENERGY_GAUGE_CENTER_Y + sin( ANGLES_TO_RADIANS( angle ) ) *
	SPEEDBAR_OUTER_RADIUS;

    return pt;
}

void
HUD::start_tri_fan(void)
{
    pp::Vec2d pt;

    glBegin( GL_TRIANGLE_FAN );
    glVertex2f( ENERGY_GAUGE_CENTER_X, 
		ENERGY_GAUGE_CENTER_Y );

    pt = calc_new_fan_pt( SPEEDBAR_BASE_ANGLE ); 

    glVertex2f( pt.x, pt.y );
}

void
HUD::fix_xy(int &x, int &y, const int asc, const int width)
{
	if(x<0){
		x=getparam_x_resolution()+x-width;
	}
	if(y<0){
		y=getparam_y_resolution()+y-asc;
	}
}

void
HUD::coursePercentage(const int i)
{
	if ( !getparam_display_course_percentage() ) {
		return;
    }
	
	if(m_element[i].font){
		char string[BUFF_LEN];
		sprintf( string, m_element[i].string.c_str(), players[0].getCoursePercentage() );
		
		pp::Font::utf8ToUnicode(m_element[i].u_string,string);
		int width = int(m_element[i].font->advance(m_element[i].u_string));
		
		fix_xy(m_element[i].x,m_element[i].y,m_element[i].height,width);
		m_element[i].font->draw(m_element[i].u_string, m_element[i].x, m_element[i].y);		
		
	}
}
