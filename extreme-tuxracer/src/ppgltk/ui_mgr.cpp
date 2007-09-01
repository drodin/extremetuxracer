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

#include "ui_mgr.h"
#include "textures.h"
#include "ui_snow.h"
#include "loop.h"
#include "game_config.h"

pp::UIManager UIMgr;

namespace pp {
	
UIManager::UIManager() 
	: cursorPos(0.0,0.0)
{
	leftMouseButtonDown = false;
	middleMouseButtonDown = false;
	rightMouseButtonDown = false;
	dirty = true;
	focusWidget = NULL;
}

UIManager::~UIManager()
{
}

void
UIManager::setDirty()
{
    dirty = true;
}
	
void
UIManager::setupDisplay()
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, getparam_x_resolution(), 
	     	0, getparam_y_resolution(), -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -1.0 );

    glColor4f( 1.0, 1.0, 1.0, 1.0 );
}
	
void
UIManager::draw()
{
	setupDisplay();
	std::list<pp::Widget*>::iterator it;
	for(it=widgets.begin();it!=widgets.end();it++){
		(*it)->draw();		
	}
	drawCursor();	
}

void
UIManager::drawCursor()
{
    GLuint texobj;
    char *binding;

    binding = "mouse_cursor";
    if ( !get_texture_binding( binding, &texobj ) ) {
	texobj = 0;
    }

    setupDisplay();

    glBindTexture( GL_TEXTURE_2D, texobj );

    glBegin( GL_QUADS );
    {
	glTexCoord2f( 0, 1 );
	glVertex2f( cursorPos.x, 
		    cursorPos.y );

	glTexCoord2f( 0, 0 );
	glVertex2f( cursorPos.x, 
		    cursorPos.y - 32 );

	glTexCoord2f( 1, 0 );
	glVertex2f( cursorPos.x + 32, 
		    cursorPos.y - 32 );

	glTexCoord2f( 1, 1 );
	glVertex2f( cursorPos.x + 32, 
		    cursorPos.y );

    }
    glEnd();
}

void 
UIManager::mouseEvent( int button, int state, int x, int y )
{
    if ( is_mode_change_pending() ) {
		// Don't process events until mode change occurs 
		return;
    }

    // Reverse y coordinate 
    y = getparam_y_resolution() - y;
	
	
	std::list<pp::Widget*>::iterator it;
	
	if ( state == SDL_PRESSED ) {
		for(it=widgets.begin();it!=widgets.end();it++){
			(*it)->mouseDown(button,x,y);
		}
	} else {
		for(it=widgets.begin();it!=widgets.end();it++){
			(*it)->mouseUp(button,x,y);
		}
	}

    if ( button == SDL_BUTTON_LEFT ) {
		leftMouseButtonDown = (bool) ( state == SDL_PRESSED );
    }
    if ( button == SDL_BUTTON_MIDDLE ) {
		middleMouseButtonDown = (bool) ( state == SDL_PRESSED );
    }
    if ( button == SDL_BUTTON_RIGHT ) {
		rightMouseButtonDown = (bool) ( state == SDL_PRESSED );
    }

    checkDirty();
}

void
UIManager::checkDirty()
{
    if( dirty){
		winsys_post_redisplay();
		dirty = false;
    }
}

pp::Vec2d
UIManager::getMousePosition()
{
    return cursorPos;
}

void
UIManager::motionEvent( int x, int y )
{
    if ( is_mode_change_pending() ) {
		// Don't process events until mode change occurs 
		return;
    }

    // Reverse y coordinate
    y = getparam_y_resolution() - y;
	
	std::list<pp::Widget*>::iterator it;
	
	for(it=widgets.begin();it!=widgets.end();it++){
		(*it)->mouseMotion(x,y);
	}
	
    pp::Vec2d oldPos = cursorPos;
    cursorPos = pp::Vec2d(x,y);

    if ( oldPos.x != x || oldPos.y != y ) {
		// Update UI snow 
		if ( getparam_ui_snow() ) {
			if ( rightMouseButtonDown ) {
				make_ui_snow( cursorPos );
				reset_ui_snow_cursor_pos( cursorPos );
			} else if ( middleMouseButtonDown ) {
				make_ui_snow( cursorPos );
				push_ui_snow( cursorPos );
			} else {
				push_ui_snow( cursorPos );
			}
		}

		// Need to redraw cursor
		setDirty();
		checkDirty();
    }
}

void
UIManager::add(pp::Widget *widget)
{
	widgets.push_back(widget);	
}

void
UIManager::remove(pp::Widget *widget)
{
	if(focusWidget==widget){
		focusWidget=NULL;
	}	
	widgets.remove(widget);
}

bool
UIManager::keyboardEvent(SDLKey key, SDLMod mod, bool release)
{
	if(focusWidget!=NULL){
		return focusWidget->keyboardEvent(key, mod, release);	
	}
	return false;
}

void
UIManager::grabFocus(pp::Widget* widget, bool focus)
{
	if(focus){
		if(focusWidget!=NULL){
			focusWidget->removeFocus();
		}
		focusWidget=widget;
	}else{
		if(focusWidget==widget){
			focusWidget=NULL;
		}
	}
}


} //namepsace pp
