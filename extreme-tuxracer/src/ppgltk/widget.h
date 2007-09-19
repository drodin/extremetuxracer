/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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
 
#ifndef _PP_WIDGET_H_
#define _PP_WIDGET_H_

#include "alg/signal.h"
#include "alg/vec2d.h"

#include "alignment.h"

#include <SDL.h>

namespace pp {

class Widget {
	
protected:

    pp::Vec2d m_position;
    pp::Vec2d m_size;
	
	///is the box currently pushed down?
	bool m_clicked;

    ///is the mouse pointer currently in the box?
	bool m_focused;

    ///is the widget sensitive (click event and font)
	bool m_sensitive;

    ///is the box being drawn by the UI manager?
	bool m_visible;

    /// is the box receiving mouse events?
	bool m_active;

	bool m_hasFocus;

	virtual void performClickAction();
		
public:
	Widget();
	virtual ~Widget();

	void setSensitive(bool sensitive);
	bool getSensitive() { return m_sensitive; };
	void setVisible(bool visible);
	void setActive(bool active);
	void setPosition(pp::Vec2d pos){m_position = pos;};

	bool isInBox(const int x, const int y) const;

	void mouseDown(int button, int x, int y );
	void mouseUp(int button, int x, int y );
	void mouseMotion(int x, int y);
	virtual bool keyboardEvent(SDLKey key, SDLMod mod, bool release){return false;};
		
	void removeFocus();
	void setFocus(bool focus = true);
	bool hasFocus(){return m_hasFocus;};

	void simulateMouseClick();

	virtual void draw()	= 0;

	inline double getWidth() const {return m_size.x;};
	inline double getHeight() const {return m_size.y;};

	//signals
	pp::Signal0 signalClicked;
	
	///the alignment of the widgets.
	pp::Alignment alignment;

};

} //namespace pp

#endif // _PP_WIDGET_H_
