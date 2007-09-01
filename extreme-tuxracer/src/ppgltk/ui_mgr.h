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

#ifndef UI_MGR_H
#define UI_MGR_H

#include "winsys.h"
#include "widget.h"

#include "alg/vec2d.h"

#include <list>

namespace pp{
	
class Widget;

class UIManager 
{
	std::list<pp::Widget*> widgets;	
		
	pp::Widget* focusWidget;
		
	bool leftMouseButtonDown;
	bool middleMouseButtonDown;
	bool rightMouseButtonDown;
	
	bool dirty;
	pp::Vec2d cursorPos;
	
	void drawCursor();
	
public:
	UIManager();
	~UIManager();

	void add(pp::Widget *widget);
	void remove(pp::Widget *widget);
	
	void setDirty();
	void checkDirty();

	void setupDisplay();
	void draw();
	void mouseEvent( int button, int state, int x, int y );
	void motionEvent( int x, int y );
	pp::Vec2d getMousePosition();

	bool keyboardEvent(SDLKey key, SDLMod mod, bool release);

	void grabFocus(pp::Widget* widget,bool focus);

};

} //namespace pp

extern pp::UIManager UIMgr;

#endif // UI_MGR_H
