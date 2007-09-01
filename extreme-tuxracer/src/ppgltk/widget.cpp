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

#include "widget.h"

#include "debug.h"
#include "ui_mgr.h"

#include <iostream>

namespace pp {
	
Widget::Widget()
{
	UIMgr.add(this);	
	m_hasFocus=false;
}

Widget::~Widget()
{
	UIMgr.remove(this);
}

void
Widget::setSensitive(bool sensitive)
{
	if ( m_sensitive != sensitive ) {
		UIMgr.setDirty();
    }

    m_sensitive = sensitive;

    if ( !m_sensitive ) {
		m_clicked = false;
		m_focused = false;
    }
}

void
Widget::setVisible(bool visible)
{
    /* This check is necessary to prevent infinite mutual recursion */
    if ( m_active != visible ) {
		setActive( visible );
    }

    if ( !m_visible && visible ) {
		UIMgr.setDirty();
    } else if ( m_visible && !visible ) {
		UIMgr.setDirty();
    }
    m_visible = visible;
}

void 
Widget::setActive( bool active )
{
    if ( !m_active && active ) {
		UIMgr.setDirty();
    } else if ( m_active && !active ) {
		UIMgr.setDirty();
    }

    m_active = active;

    if ( !active ) {
		setVisible( false );
    }
}

void
Widget::performClickAction()
{
    signalClicked.Emit();
}
	
	
bool
Widget::isInBox(const int x, const int y) const
{
    return (bool) (
	( x >= m_position.x ) &&
	( x <= m_position.x + m_size.x ) &&
	( y >= m_position.y ) &&
	( y <= m_position.y + m_size.y ) );
}
	
void
Widget::mouseMotion(int x, int y)
{
    if ( !m_sensitive ) {
		return;
    }

    if ( isInBox(x, y) ) {
		if ( m_focused == false ) {
			print_debug( DEBUG_UI, "Mouse entered button" );
		    m_focused = true;
		    UIMgr.setDirty();
		}
    } else {
		if ( m_focused == true ) {
			print_debug( DEBUG_UI, "Mouse left button" );
		    m_focused = false;
		    UIMgr.setDirty();
		}
    }
}
void
Widget::mouseDown(int button,int x, int y )
{
    if ( button != SDL_BUTTON_LEFT ) {
		return;
    }

    if ( !isInBox(x, y) ) {
		return;
    }

    if ( !m_sensitive ) {
		return;
    }

    if ( m_clicked == false ) {
		print_debug( DEBUG_UI, "Button is down" );
		m_clicked = true;
		UIMgr.setDirty();
    }
}

void
Widget::mouseUp(int button, int x, int y )
{
	if ( button != SDL_BUTTON_LEFT ) {
		return;
	}

    if ( !m_sensitive ) {
		return;
    }

    if ( !isInBox(x, y) ) {
		if ( m_clicked ) {
		    print_debug( DEBUG_UI, "Button is up (not clicked)" );
		    m_clicked = false;
		    UIMgr.setDirty();
		}
		return;
    } 

    if ( m_clicked ) {
		m_clicked = false;
		print_debug( DEBUG_UI, "Button was clicked" );
		performClickAction();
		UIMgr.setDirty();
    }
}

void
Widget::simulateMouseClick()
{
	performClickAction();
}

void
Widget::removeFocus()
{
	m_hasFocus=false;
}

void
Widget::setFocus(bool focus)
{
	if(focus!=m_hasFocus){
		UIMgr.grabFocus(this,focus);
		m_hasFocus=focus;
	}
}


} //namespace pp
