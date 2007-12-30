/* 
 * Extreme Tux Racer (Not Copyrighted)
 * 
 * based on Planet Penguin Racer, Open Racer, and Tux Racer: 
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


#include "credits.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/font.h"


#define CREDITS_MAX_Y -140
#define CREDITS_MIN_Y 64

typedef struct {
    pp::Font *font;
	char *binding;
    char *text;
} credit_line_t;

static credit_line_t credit_lines[] = 
{
	{ NULL, "credits_h1", "Extreme Tux Racer" },
    { NULL, "credits_text", "Version 0.4" },
    { NULL, "credits_text", "on the web at: www.extremetuxracer.com" },
    { NULL, "credits_text", "" },	
	{ NULL, "credits_text", "This version of Extreme Tux Racer was developed" },	
	{ NULL, "credits_text", "by the 'Media' team, organized by Christian Picon" },	
	{ NULL, "credits_text", "and includes very few code changes. " },	
	{ NULL, "credits_text", "" },		
	{ NULL, "credits_text", "Extreme Tux Racer is based on:" },
	{ NULL, "credits_text", "PlanetPenguin Racer," },
	{ NULL, "credits_text", "Open Racer," },
	{ NULL, "credits_text", "and Tux Racer" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Development" },
    { NULL, "credits_text", "Volker Ströbel" },
    { NULL, "credits_text", "Steven Bell" },
    { NULL, "credits_text", "Hamish Morrison" },
    { NULL, "credits_text", "Torandi" },
	{ NULL, "credits_text_small", "from Sunspire Studios:" },
    { NULL, "credits_text", "Patrick \"Pog\" Gilhuly" },
    { NULL, "credits_text", "Eric \"Monster\" Hall" },
    { NULL, "credits_text", "Rick Knowles" },
    { NULL, "credits_text", "Vincent Ma" },
    { NULL, "credits_text", "Jasmin Patry" },
    { NULL, "credits_text", "Mark Riddell" },
    { NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Translators" },
	{ NULL, "credits_text", "Nicosmos (French)" },
	{ NULL, "credits_text", "teksturi (Finnish)" },
	{ NULL, "credits_text", "arith, spacedwarv, and JoyFM (German)" },
	{ NULL, "credits_text", "spectrum (Italian)" },
	{ NULL, "credits_text", "Torandi (Swedish)" },
    	{ NULL, "credits_text", "Asciimonster (Dutch)" },
    	{ NULL, "credits_text", "woody (Polish)" },
    	{ NULL, "credits_text_small", "other (incomplete) translations are from the PPRacer Project" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Graphics" },
    { NULL, "credits_text", "Nicosmos (Logo, Hud Graphics)" },
    { NULL, "credits_text", "Christian Picon (Objects)" },
    { NULL, "credits_text", "Daniel Poeira and Christian Picon (Font)" },
    { NULL, "credits_text", "" },
	{ NULL, "credits_h2", "Contributors" },
	{ NULL, "credits_text", "Peter Reichel" },
	{ NULL, "credits_text", "Teemu Vesala" },
	{ NULL, "credits_text", "Theo Snelleman" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Music" },
    { NULL, "credits_text", "Joseph Toscano" },
    { NULL, "credits_text", "" },
    { NULL, "credits_h2", "Thanks To:" },
    { NULL, "credits_text_small", "(for Exteme Tux Racer)" },
    { NULL, "credits_text", "Ranger (for hosting the website)" },
    { NULL, "credits_text", "Slythfox (for making the website)" },
    { NULL, "credits_text", "The OpenRacer project" },
    { NULL, "credits_text", "Erin (for helping with code modification" },
    { NULL, "credits_text", "All the people on the forum." },
    { NULL, "credits_text", "Larry Ewing (creator of Tux)" },
    { NULL, "credits_text_small", "(for PP Racer and Tux Racer)" },
    { NULL, "credits_text", "Thatcher Ulrich" },
    { NULL, "credits_text", "Steve Baker" },
    { NULL, "credits_text", "Ingo Ruhnke" },
    { NULL, "credits_text", "James Barnard" },
    { NULL, "credits_text", "Alan Levy" },
    { NULL, "credits_text", "" },
  { NULL, "credits_text_small", "Tux Racer is a trademark of Jasmin F. Patry" },
	{ NULL, "credits_text_small", "Extreme Tux Racer is licenced under the GPL." },
	{ NULL, "credits_text_small", "We grant use of the name 'Extreme Tux Racer' to any forks or continuations. " },
	{ NULL, "credits_text_small", "" },
	{ NULL, "credits_text_small", "PlanetPenguin Racer is Copyright © 2005 Volker Stroebel" },
  { NULL, "credits_text_small", "Tux Racer and the Tux Racer Name is Copyright © 2001 Jasmin F. Patry" },
};

Credits::Credits()
 : m_yOffset(0.0)
{
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
		credit_lines[i].font = pp::Font::get(credit_lines[i].binding);		
	}
	
  play_music( "credits_screen" );
}


Credits::~Credits()
{	
}

void
Credits::loop(float timeStep)
{
	int width, height;

  width = getparam_x_resolution();
  height = getparam_y_resolution();

  update_audio();

  clear_rendering_context();

  set_gl_options( GUI );

  UIMgr.setupDisplay();

  drawText( timeStep );

	drawSnow(timeStep);

	theme.drawMenuDecorations();

  UIMgr.draw();

  reshape( width, height );

  winsys_swap_buffers();
}


void
Credits::drawText( float timeStep )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    float y;

    m_yOffset += timeStep * 30;
    y = CREDITS_MIN_Y + m_yOffset;

	//loop through all credit lines
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
	    credit_line_t line = credit_lines[i];

		//get the font and sizes for the binding
		//pp::Font *font = pp::Font::get(line.binding);
		float width = line.font->advance(line.text);
		float desc = line.font->descender();
		float asc = line.font->ascender();
		
		//draw the line on the screen
		line.font->draw(line.text, w/2 - width/2, y);

		//calculate the y value for the next line
		y-=asc-desc;
	}

	//if the last string reaches the top, reset the y offset
    if ( y > h+CREDITS_MAX_Y ) {
		m_yOffset = 0;
    }

    // Draw strips at the top and bottom to clip out text 
    glDisable( GL_TEXTURE_2D );

    glColor4dv( (double*)&theme.background );

    glRectf( 0, 0, w, CREDITS_MIN_Y );

    glBegin( GL_QUADS );
    {
	glVertex2f( 0, CREDITS_MIN_Y );
	glVertex2f( w, CREDITS_MIN_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( w, CREDITS_MIN_Y + 30 );
	glVertex2f( 0, CREDITS_MIN_Y + 30 );
    }
    glEnd();

    glColor4dv( (double*)&theme.background );

    glRectf( 0, h+CREDITS_MAX_Y, w, h );

    glBegin( GL_QUADS );
    {
	glVertex2f( w, h+CREDITS_MAX_Y );
	glVertex2f( 0, h+CREDITS_MAX_Y );
	glColor4f( theme.background.r, 
		   theme.background.g,
		   theme.background.b,
		   0 );
	glVertex2f( 0, h+CREDITS_MAX_Y - 30 );
	glVertex2f( w, h+CREDITS_MAX_Y - 30 );
    }
    glEnd();

    glColor4f( 1, 1, 1, 1 );

    glEnable( GL_TEXTURE_2D );
}

bool
Credits::mouseButtonReleaseEvent(int button, int x, int y)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}

bool
Credits::keyReleaseEvent(SDLKey key)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}
