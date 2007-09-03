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

#ifndef _FOG_H_
#define _FOG_H_

#include "etracer.h"

class FogPlane {
	
	bool m_isOn;
    GLint m_mode;
    GLfloat m_density;
    GLfloat m_start;
    GLfloat m_end;
    GLfloat m_color[4];
	
public:
	FogPlane(){};
	~FogPlane(){};
		
	void reset();
	
	inline bool isEnabled() const {return m_isOn;}
	inline void setEnabled(const bool state=true){m_isOn=true;}
	inline GLfloat* getColor() {return m_color;}
	inline void setMode(const GLint mode){m_mode=mode;}
	inline void setDensity(const GLfloat density){m_density=density;}
	inline void setStart(const GLfloat start){m_start=start;}
	inline void setEnd(const GLfloat end){m_end=end;}
	void setup();
	
	static void registerCallbacks( Tcl_Interp *ip );
};

extern FogPlane fogPlane;

#endif
