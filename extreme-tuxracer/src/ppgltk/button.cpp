/* 
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

#include "button.h"

#include "textures.h"
#include "ui_mgr.h"


namespace pp {
	
Button::Button( pp::Vec2d pos, pp::Vec2d size,
			 const char *binding, const char *label )
 : 	mp_font(NULL),
	mp_hilitFont(NULL),
	mp_disabledFont(NULL)
{
    m_position = pos;
    m_size = size;

    m_tex.binding = NULL;
    m_hilitTex.binding = NULL;
    m_clickedTex.binding = NULL;
    m_disabledTex.binding = NULL;
	
	if(binding!=NULL){
		mp_font = pp::Font::get(binding);
	}
	
    mp_label = label;

    m_clicked = false;
    m_focused = false;
    m_sensitive = true;
    m_visible = false;
    m_active = false;
}

void
Button::setHilitFontBinding( const char *binding )
{
	mp_hilitFont = pp::Font::get(binding);
}

void
Button::setImage( const char *binding,
		       pp::Vec2d p0, pp::Vec2d p1, pp::Color color )
{
    m_tex.binding = binding;
    m_tex.ll = p0;
    m_tex.ur = p1;
    m_tex.color = color;
}

void
Button::setDisabledImage( const char *binding,
				pp::Vec2d p0, pp::Vec2d p1, pp::Color color )
{
	m_disabledTex.binding = binding;
    m_disabledTex.ll = p0;
    m_disabledTex.ur = p1;
    m_disabledTex.color = color;
}

void
Button::setHighlight(bool highlight)
{
    if (!(m_focused == highlight)) {
		UIMgr.setDirty();
    }
    m_focused = highlight;
}

void
Button::setHilitImage( const char *binding,
			     pp::Vec2d p0, pp::Vec2d p1, pp::Color color )
{
    m_hilitTex.binding = binding;
    m_hilitTex.ll = p0;
    m_hilitTex.ur = p1;
    m_hilitTex.color = color;
}

void
Button::setClickedImage( const char *binding,
			       pp::Vec2d p0, pp::Vec2d p1, pp::Color color )
{
    m_clickedTex.binding = binding;
    m_clickedTex.ll = p0;
    m_clickedTex.ur = p1;
    m_clickedTex.color = color;
}

void
Button::setDisabledFontBinding( const char *binding )
{
	mp_disabledFont = pp::Font::get(binding);
}

void
Button::draw()
{
    GLuint texobj;
    texture_region_t *tex;
	pp::Font *font=mp_font;

    glEnable( GL_TEXTURE_2D );

    tex = NULL;
    
    if ( !m_sensitive ) {
		if ( m_disabledTex.binding ) {
			tex = &m_disabledTex;
		} else if ( m_tex.binding ) {
			tex = &m_tex;
		}

		if ( mp_disabledFont ) {
			font = mp_disabledFont;
		}
    } else if ( m_clicked ) {
		if ( m_clickedTex.binding ) {
			tex = &m_clickedTex;
		} else if ( m_hilitTex.binding ) {
			tex = &m_hilitTex;
		} else if ( m_tex.binding ) {
			tex = &m_tex;
		} 

		if ( mp_hilitFont) {
			font = mp_hilitFont;
		} 
    } else if ( m_focused ) {
		if ( m_hilitTex.binding ) {
			tex = &m_hilitTex;
		} else if ( m_tex.binding ) {
			tex = &m_tex;
		} 

		if ( mp_hilitFont ) {
			font = mp_hilitFont;
		}
    } else {
		if ( m_tex.binding ) {
			tex = &m_tex;
		}
    }

    if ( tex != NULL ) {
	if ( !get_texture_binding( tex->binding, &texobj ) ) {
	    print_warning( IMPORTANT_WARNING,
			   "Couldnt get texture object for binding %s",
			   tex->binding );
	    texobj = 0;
	}

	glBindTexture( GL_TEXTURE_2D, texobj );

	glColor4dv( (double*) &tex->color );

	glBegin( GL_QUADS );
	{
	    glTexCoord2f( tex->ll.x, tex->ll.y );
	    glVertex3f( m_position.x, m_position.y, 0 );

	    glTexCoord2f( tex->ur.x, tex->ll.y );
	    glVertex3f( m_position.x + m_size.x, m_position.y, 0 );

	    glTexCoord2f( tex->ur.x, tex->ur.y );
	    glVertex3f( m_position.x + m_size.x, m_position.y + m_size.y, 0 );

	    glTexCoord2f( tex->ll.x, tex->ur.y );
	    glVertex3f( m_position.x, m_position.y + m_size.y, 0 );
	}
	glEnd();
    }

	if(mp_font!=NULL){
	
		float width = font->advance(mp_label);
		float asc = font->ascender();
		float desc = font->descender();
	
		font->draw(mp_label,
				m_position.x + m_size.x/2.0 - width/2.0,
				m_position.y + m_size.y/2.0 - asc/2.0 - desc/2.0);
	}    
}

} //namespace pp
