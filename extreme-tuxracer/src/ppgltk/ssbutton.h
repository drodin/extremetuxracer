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

#ifndef SSBUTTON_H
#define SSBUTTON_H

#include "widget.h"

#include "alg/color.h"

namespace pp{
	

class SSButton : public Widget{
	
	
	typedef struct {
		/// name of texture binding
		const char *binding;
		/// color to use when drawing texture
		pp::Color color;
		/// lower left
		pp::Vec2d ll;
		/// upper right
		pp::Vec2d ur;
	} texture_region_t;

	int m_numStates;
    texture_region_t *m_regions;
    int m_currentState;
	
public:
	SSButton(pp::Vec2d pos, pp::Vec2d size, int num_states );
	~SSButton();	

	void performClickAction();
	void draw();

	int getState(){return m_currentState;};
	void setState( int state );
	void setStateImage( int state, const char *binding,
			       pp::Vec2d ll, pp::Vec2d ur, 
			       pp::Color color );
};


} //namespace pp

#endif // SSBUTTON_H
