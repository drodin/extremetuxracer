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
#include "checkbox.h"
 
#include "textarea.h"
#include "ui_mgr.h"

#include "render_util.h"

#define DEFAULT_ARROW_BUTTON_HEIGHT 15
#define DEFAULT_ARROW_BUTTON_WIDTH  27
#define DEFAULT_ARROW_REGION_WIDTH  36
#define DEFAULT_ARROW_VERT_SEPARATION  4
#define DEFAULT_BORDER_WIDTH 4
#define DEFAULT_TEXT_PAD 4

namespace pp {

Textarea::Textarea( pp::Vec2d pos, pp::Vec2d size,
			     const char *binding, const char *text )
{
    char *tempbinding;
    pp::Vec2d ll, ur;
	
	m_position = pos;
	m_size = size;
	
    m_arrowWidth = DEFAULT_ARROW_REGION_WIDTH;
    m_borderWidth = DEFAULT_BORDER_WIDTH;
    m_textPad = DEFAULT_TEXT_PAD;
	
    calcTextRegionDims();

	mp_font = pp::Font::get(binding);
	
    m_borderColor = theme.foreground;
    m_backgroundColor = theme.background;

    /* 
     * Create buttons 
     */
    mp_upButton = new pp::Button(
		pp::Vec2d( 0, 0 ), /* position will be set later */
		pp::Vec2d(DEFAULT_ARROW_BUTTON_WIDTH, DEFAULT_ARROW_BUTTON_HEIGHT),
		NULL,
		NULL );

    tempbinding = "textarea_arrows";

    ll = pp::Vec2d( 2.0/64.0, 17.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 32.0/64.0 );
    mp_upButton->setImage( tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 17.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 32.0/64.0 );
    mp_upButton->setDisabledImage( tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 49.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 64.0/64.0 );
    mp_upButton->setHilitImage( tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 2.0/64.0, 49.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 64.0/64.0 );
    mp_upButton->setClickedImage( tempbinding, ll, ur, pp::Color::white );
	
	mp_upButton->setActive(true);
    mp_upButton->signalClicked.Connect(pp::CreateSlot(this,&Textarea::upButtonClick));

    mp_downButton = new pp::Button(
		pp::Vec2d( 0, 0 ), /* position will be set later */
		pp::Vec2d(DEFAULT_ARROW_BUTTON_WIDTH,DEFAULT_ARROW_BUTTON_HEIGHT),
		NULL,
		NULL );

    tempbinding = "textarea_arrows";

    ll = pp::Vec2d( 2.0/64.0, 1.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 16.0/64.0 );
    mp_downButton->setImage(tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 1.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 16.0/64.0 );
    mp_downButton->setDisabledImage(tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 33.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 48.0/64.0 );
    mp_downButton->setHilitImage(tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 2.0/64.0, 33.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 48.0/64.0 );
    mp_downButton->setClickedImage(tempbinding, ll, ur, pp::Color::white );

 	mp_downButton->setActive(true);
    mp_downButton->signalClicked.Connect(pp::CreateSlot(this,&Textarea::downButtonClick));

	updateButtonPositions();
    

    if ( text == NULL ) {
		text = "";
    }
	
	
    mp_textOrig = new char[strlen( text ) + 1];
	strcpy( mp_textOrig, text );

    mp_textLines = NULL;

    createLines();

    updateButtonEnabledStates();

    this->m_visible = false;
    this->m_active = false;
}

Textarea::~Textarea()
{
    setVisible( false );
    setActive( false );

    if ( mp_upButton != NULL ) {
		delete mp_upButton;
    }
    if ( mp_downButton != NULL ) {
		delete mp_downButton;
    }

    if ( mp_textLines != NULL ) {
		delete( mp_textLines );
    }

    if ( mp_textOrig != NULL ) {
		delete( mp_textOrig );
    }	
}

void
Textarea::calcTextRegionDims()
{
    m_textRegionWidth = m_size.x - 2*m_borderWidth - 2*m_textPad - m_arrowWidth;
    m_textRegionHeight = m_size.y - 2*m_borderWidth - 2*m_textPad;
}

void
Textarea::updateButtonPositions()
{
	mp_upButton->setPosition(pp::Vec2d(
	    m_position.x + m_size.x - m_borderWidth -
	    mp_upButton->getWidth(),
	    m_position.y + m_size.y - m_borderWidth -
	    mp_upButton->getHeight() ) );
	
	mp_downButton->setPosition(pp::Vec2d(
	    m_position.x + m_size.x - m_borderWidth -
	    mp_downButton->getWidth(),
	    m_position.y + m_borderWidth ) );
}

void
Textarea::createLines()
{
	/*font_t font;

    if ( !get_font_binding( m_binding, font ) ) {
		print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding %s", m_binding );
		return;
    }
	*/
	
	
	if ( mp_textLines != NULL ) {
		delete mp_textLines;
		mp_textLines = NULL;
    }

    if ( !m_lines.empty() ) {
		m_lines.clear();
    }
	
	char *cur_ptr;
	char *newline_ptr;
	
	mp_textLines = new char[strlen(mp_textOrig)+1];
	strcpy(mp_textLines,mp_textOrig);
	
	
    cur_ptr = mp_textLines;

	while ( *cur_ptr != '\0' ){
		newline_ptr = strchr( cur_ptr, '\n' );
		if( newline_ptr!=NULL){
			*newline_ptr = '\0';
		}
		
		//if( string_width( cur_ptr, &font ) > m_textRegionWidth ) {
		if( mp_font->advance(cur_ptr) > m_textRegionWidth ) {
		
			//text is too long for linear_attenuation
			//try to find a suiteable space within this line
			char* old_space_ptr = NULL;
			char* space_ptr = strrchr( cur_ptr, ' ' );
			
			while (space_ptr !=NULL){
				*space_ptr = '\0';
				if(mp_font->advance(cur_ptr) < m_textRegionWidth ){
					if(newline_ptr!=NULL){
						*newline_ptr = '\n';
					}
					newline_ptr = space_ptr;
					break;
				}
				
				if(old_space_ptr !=NULL){
					*old_space_ptr = ' ';
				}
				old_space_ptr = space_ptr;
				space_ptr = strrchr( cur_ptr, ' ' );
			}
			if(old_space_ptr !=NULL){
				*old_space_ptr = ' ';
			}
		}
		m_lines.push_back(cur_ptr);
	
		if(newline_ptr==NULL){
			break;
		}
		cur_ptr=newline_ptr+1;
	}
	
    mi_topLine = m_lines.begin();
}

void
Textarea::updateButtonEnabledStates()
{
    if ( m_lines.empty() ) {
		/* No lines */
		mp_upButton->setSensitive( false );
		mp_downButton->setSensitive( false );
    } else {
		if ( mi_topLine == m_lines.begin() ) {
		    mp_upButton->setSensitive( false );
		} else {
		    mp_upButton->setSensitive( true );
		}
		if ( mi_topLine == --m_lines.end() ) {
		    mp_downButton->setSensitive( false );
		} else {
		    mp_downButton->setSensitive( true );
		}
    }
}

void
Textarea::setText( const char *text )
{
    if ( mp_textOrig != NULL ) {
		delete mp_textOrig;
		mp_textOrig = NULL;
    }

    if ( text == NULL ) {
		text = "";
    }

    mp_textOrig = new char[strlen( text ) + 1];
	strcpy( mp_textOrig, text );
	
    createLines();
    updateButtonEnabledStates();	
}

void
Textarea::setPosition(pp::Vec2d pos)
{
	Widget::setPosition(pos);
	updateButtonPositions();
}


void 
Textarea::upButtonClick()
{
	if( mi_topLine != m_lines.begin()){
		mi_topLine--;
    	updateButtonEnabledStates();
    	UIMgr.setDirty();
	}
}

void 
Textarea::downButtonClick()
{
	if( mi_topLine != --m_lines.end() ){
		mi_topLine++;
		updateButtonEnabledStates();
		UIMgr.setDirty();
	}
}

void
Textarea::drawTextLines()
{
	if (m_lines.empty())return;
	
	std::list<char*>::iterator it;
	float desc = mp_font->descender();
	float asc = mp_font->ascender();
	float y = m_position.y + m_size.y - m_borderWidth - m_textPad - asc;
	float x = m_position.x + m_borderWidth + m_textPad;
	
	for(it=mi_topLine; it!=m_lines.end(); it++){
		mp_font->draw(*it, x, y);
		
		//float desc = mp_font->descender();
		//float asc = mp_font->ascender();
		
		y-=asc-desc;
		if(y < m_position.y + m_borderWidth) {
			if ( mp_downButton ) {
		    	mp_downButton->setSensitive( true );
			}
			break;
		}
	}
}

void
Textarea::draw()
{
	glDisable( GL_TEXTURE_2D );

    glColor3dv( (double*)&m_borderColor );

    glRectf( m_position.x, 
	     m_position.y,
	     m_position.x + m_size.x,
	     m_position.y + m_size.y );

    glColor3dv( (double*)&m_backgroundColor );

    glRectf( m_position.x + m_borderWidth, 
	     m_position.y + m_borderWidth,
	     m_position.x + m_size.x - m_borderWidth,
	     m_position.y + m_size.y - m_borderWidth );

    glEnable( GL_TEXTURE_2D );

	drawTextLines();

    if ( mp_upButton != NULL ) {
		mp_upButton->draw();
    }

    if ( mp_downButton != NULL ) {
		mp_downButton->draw(); 
    }
}	


} //namespace pp
