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


#include "ui_theme.h"
#include "ui_mgr.h"

#define DEFAULT_ARROW_BUTTON_HEIGHT 15
#define DEFAULT_ARROW_BUTTON_WIDTH  27
#define DEFAULT_ARROW_REGION_WIDTH  36
#define DEFAULT_ARROW_VERT_SEPARATION  4
#define DEFAULT_BORDER_WIDTH 4
#define LDEFAULT_TEXT_PAD 8


namespace pp{

template <class T>
Listbox<T>::Listbox(  pp::Vec2d pos, pp::Vec2d size, 
				const char *binding, 
				listType_t& itemList)
 : mp_insensitiveFont(NULL),
   m_items(itemList)
{
    char *tempbinding;
    pp::Vec2d ll;
    pp::Vec2d ur;

    m_position = pos;
    m_size = size;
	
    m_arrowWidth = DEFAULT_ARROW_REGION_WIDTH;
    m_borderWidth = DEFAULT_BORDER_WIDTH;
    m_textPad = LDEFAULT_TEXT_PAD;
    m_arrowVertSeparation = DEFAULT_ARROW_VERT_SEPARATION;
    m_borderColor = theme.foreground;
    m_backgroundColor = theme.background;
	
	mp_font = pp::Font::get(binding);	
	

    /* Create up arrow button */
    mp_upButton = new pp::Button( 
		pp::Vec2d( 0, 0 ), /* position will be set later */
		pp::Vec2d(DEFAULT_ARROW_BUTTON_WIDTH,DEFAULT_ARROW_BUTTON_HEIGHT),
		NULL,
		NULL );

    tempbinding = "listbox_arrows";

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

	mp_upButton->setActive( true );
    mp_upButton->signalClicked.Connect(pp::CreateSlot(this,&Listbox::upButtonClick));


    /* Create down arrow button */
    mp_downButton = new pp::Button( 
		pp::Vec2d( 0, 0 ), /* position will be set later */
		pp::Vec2d(DEFAULT_ARROW_BUTTON_WIDTH,DEFAULT_ARROW_BUTTON_HEIGHT),
		NULL,
		NULL );

    tempbinding = "listbox_arrows";

    ll = pp::Vec2d( 2.0/64.0, 1.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 16.0/64.0 );
    mp_downButton->setImage( tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 1.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 16.0/64.0 );
    mp_downButton->setDisabledImage( tempbinding, ll, ur, pp::Color::white );

    ll = pp::Vec2d( 34.0/64.0, 33.0/64.0 );
    ur = pp::Vec2d( 61.0/64.0, 48.0/64.0 );
    mp_downButton->setHilitImage(tempbinding, ll, ur,pp::Color::white );

    ll = pp::Vec2d( 2.0/64.0, 33.0/64.0 );
    ur = pp::Vec2d( 29.0/64.0, 48.0/64.0 );
    mp_downButton->setClickedImage( tempbinding, ll, ur, pp::Color::white );

	mp_downButton->setActive( true );
    mp_downButton->signalClicked.Connect(pp::CreateSlot(this,&Listbox::downButtonClick));

    m_items = itemList;
    mi_curItem = m_items.begin();

    m_visible = false;
    m_active = false;
	m_sensitive = true;

    updateButtonEnabledStates();
    updateButtonPositions();
}

template <class T>
Listbox<T>::~Listbox()
{
    setVisible( false );
    setActive( false );

    delete mp_upButton;
    delete mp_downButton;
}

template <class T>
void
Listbox<T>::updateButtonEnabledStates()
{
    if ( m_items.empty() || !m_sensitive ) {
		// disable the buttons because our list
		// is empty or insensitive		
		mp_upButton->setSensitive( false );
		mp_downButton->setSensitive( false );
    } else {
		if ( mi_curItem == m_items.begin() ) {
			mp_upButton->setSensitive( false );
		} else {
			mp_upButton->setSensitive( true );
		}

		if ( mi_curItem == --m_items.end() ) {
			mp_downButton->setSensitive( false );
		} else {
			mp_downButton->setSensitive( true );
		}
    }
}

template <class T>
void
Listbox<T>::updateButtonPositions()
{
	mp_upButton->setPosition(pp::Vec2d( 
	    m_position.x + m_size.x - mp_upButton->getWidth(),
	    m_position.y + m_size.y / 2.0 + m_arrowVertSeparation / 2.0 ) );

    mp_downButton->setPosition(pp::Vec2d( 
	    m_position.x + m_size.x - mp_downButton->getWidth(),
	    m_position.y + m_size.y / 2.0 - 
	    m_arrowVertSeparation / 2.0 -
	    mp_upButton->getHeight() ) );
}

template <class T>
void
Listbox<T>::setPosition(pp::Vec2d pos)
{
	Widget::setPosition(pos);
	updateButtonPositions();
}

template <class T>
void
Listbox<T>::setCurrentItem( listIterator_t item )
{
    mi_curItem = item;
    updateButtonEnabledStates();
}

template <class T>	
void
Listbox<T>::setItemList( listType_t &itemList )
{
    m_items = itemList;
    mi_curItem = m_items.begin();
    updateButtonEnabledStates();

    UIMgr.setDirty();
}

template <class T>
void
Listbox<T>::setSensitive(bool sensitive)
{
	Widget::setSensitive(sensitive);
	updateButtonEnabledStates();
}

template <class T>
void
Listbox<T>::setInsensitiveFont(const char* binding)
{
	mp_insensitiveFont = pp::Font::get(binding);	
}

template <class T>
bool 
Listbox<T>::gotoNextItem()
{
    if ( mi_curItem == --m_items.end() ) {
		return false;
    }
    mp_downButton->simulateMouseClick();
    return true;
}

template <class T>
bool
Listbox<T>::gotoPrevItem()
{
    if ( mi_curItem == m_items.begin() ) {
		return false;
    }
    mp_upButton->simulateMouseClick();
    return true;
}

template <class T>
void
Listbox<T>::upButtonClick()
{
	if(mi_curItem != m_items.begin()){
		mi_curItem--;
		signalChange.Emit();
    	updateButtonEnabledStates();
    	UIMgr.setDirty();
	}
}

template <class T>
void
Listbox<T>::downButtonClick()
{
	if(mi_curItem != --m_items.end()){
		mi_curItem++;
		signalChange.Emit();
    	updateButtonEnabledStates();
    	UIMgr.setDirty();
	}	
}

template <class T>
void
Listbox<T>::draw()
{
    glDisable( GL_TEXTURE_2D );

	if(m_sensitive){
		if(m_hasFocus){
			glColor4dv( (double*)&theme.focus );
		}else{		
			glColor4dv( (double*)&m_borderColor );
		}
	}else{
		glColor4dv( (double*)&theme.insensitive );
	}	
	
    glRectf( m_position.x, 
	     m_position.y,
	     m_position.x + m_size.x - m_arrowWidth,
	     m_position.y + m_size.y );

    glColor4dv( (double*)&m_backgroundColor );

    glRectf( m_position.x + m_borderWidth, 
	     m_position.y + m_borderWidth,
	     m_position.x + m_size.x - m_borderWidth - m_arrowWidth,
	     m_position.y + m_size.y - m_borderWidth );

    glEnable( GL_TEXTURE_2D );

	pp::Font *font = mp_font;
	
	if(!m_sensitive && mp_insensitiveFont){
		font = mp_insensitiveFont;
	}
	
    if (font){
		float asc = font->ascender();
		float desc = font->descender();
			
		font->draw((*mi_curItem).name.c_str(),
				m_position.x + m_borderWidth + m_textPad,
				m_position.y + m_size.y/2.0 - asc/2.0 - desc/2);

    }

    mp_upButton->draw();
    mp_downButton->draw();
}
	

} //namespace pp
