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
	{ NULL, "credits_h2", N_("Translators") },
	{ NULL, "credits_text", "Nicosmos (French)" },
	{ NULL, "credits_text", "teksturi (Finnish)" },
	{ NULL, "credits_text", "arith, spacedwarv, and JoyFM (German)" },
	{ NULL, "credits_text", "spectrum (Italian)" },
	{ NULL, "credits_text", "Karl Ove Hufthammer (Norwegian Bokmål)" },
	{ NULL, "credits_text", "Karl Ove Hufthammer (Norwegian Nynorsk)" },
	{ NULL, "credits_text", "Andreas Tarandi and pingvin (Swedish)" },
    	{ NULL, "credits_text", "Asciimonster (Dutch)" },
    	{ NULL, "credits_text", "woody (Polish)" },
    	{ NULL, "credits_text", "ttsmj (Slovak)" },		
	{ NULL, "credits_text", "Jonan (Spanish)" },	
	{ NULL, "credits_text", "EFU (Russian)" },		
    	{ NULL, "credits_text_small", N_("Other (incomplete) translations are from the PPRacer Project.") },
};

CreditsTrans::CreditsTrans()
 : m_yOffset(0.0)
{
	for (unsigned int i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
		credit_lines[i].font = pp::Font::get(credit_lines[i].binding);		
	}
	
  play_music( "credits_screen" );
}


CreditsTrans::~CreditsTrans()
{	
}

void
CreditsTrans::loop(float timeStep)
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
CreditsTrans::drawText( float timeStep )
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
        
		float width = line.font->advance(_(line.text));
		float desc = line.font->descender();
		float asc = line.font->ascender();
		
		//draw the line on the screen
		line.font->draw(_(line.text), w/2 - width/2, y);

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
CreditsTrans::mouseButtonReleaseEvent(int button, int x, int y)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}

bool
CreditsTrans::keyPressEvent(SDLKey key)
{
	set_game_mode( GAME_TYPE_SELECT );
    winsys_post_redisplay();
	return true;
}

