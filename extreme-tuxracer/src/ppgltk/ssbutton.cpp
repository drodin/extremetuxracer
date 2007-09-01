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

#include "ssbutton.h"

#include "textures.h"

#include "render_util.h"

#include "ui_mgr.h"


namespace pp {
	
SSButton::SSButton(pp::Vec2d pos, pp::Vec2d size,int num_states )
{
    m_position=pos;
	m_size= size;
    m_sensitive = true;
    m_visible = false;
    m_active = false;
	m_numStates = num_states;
	m_regions = new texture_region_t[num_states];

    for (int i=0; i<num_states; i++) {
		m_regions[i].binding = NULL;
    }

    m_currentState = 0;
}

SSButton::~SSButton()
{
    setVisible( false );
    setActive( false );
    delete m_regions;	
}

void
SSButton::setState( int state )
{
	m_currentState = state;

    if ( m_visible ) {
		UIMgr.setDirty();
    }
}


void
SSButton::setStateImage( int state,
			       const char *binding,
			       pp::Vec2d ll, pp::Vec2d ur, 
			       pp::Color color )
{
	m_regions[state].binding = binding;
    m_regions[state].color = color;
    m_regions[state].ll = ll;
    m_regions[state].ur = ur;
}

void
SSButton::performClickAction()
{
	m_currentState = ( m_currentState + 1 ) %m_numStates;
	signalClicked.Emit();
}

void
SSButton::draw()
{
	GLuint texobj;
	
	glEnable( GL_TEXTURE_2D );
	
	if ( !get_texture_binding( m_regions[m_currentState].binding, &texobj ) ) {
		print_warning( IMPORTANT_WARNING,
			   "Couldnt get texture object for binding %s",
			   m_regions[m_currentState].binding );
	    texobj = 0;
	}

	glBindTexture( GL_TEXTURE_2D, texobj );

	glColor4dv( (double*) &m_regions[m_currentState].color );

	glBegin( GL_QUADS );
	{
	    glTexCoord2f( m_regions[m_currentState].ll.x, m_regions[m_currentState].ll.y );
	    glVertex3f( m_position.x, m_position.y, 0 );

	    glTexCoord2f( m_regions[m_currentState].ur.x, m_regions[m_currentState].ll.y );
	    glVertex3f( m_position.x + m_size.x, m_position.y, 0 );

	    glTexCoord2f( m_regions[m_currentState].ur.x, m_regions[m_currentState].ur.y );
	    glVertex3f( m_position.x + m_size.x, m_position.y + m_size.y, 0 );

	    glTexCoord2f( m_regions[m_currentState].ll.x, m_regions[m_currentState].ur.y );
	    glVertex3f( m_position.x, m_position.y + m_size.y, 0 );
	}
	glEnd();

}
	
} //namespace pp
