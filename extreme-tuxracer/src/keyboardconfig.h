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
 
#ifndef _KEYBOARD_CONFIG_H
#define _KEYBOARD_CONFIG_H

#include "configmode.h"

#include "ppgltk/entry.h"

class KeyboardConfig : public ConfigMode
{
	pp::Entry* mp_leftEntry;
	pp::Entry* mp_rightEntry;
	pp::Entry* mp_paddleEntry;
	pp::Entry* mp_brakeEntry;
	pp::Entry* mp_jumpEntry;
	pp::Entry* mp_trickEntry;
	pp::Entry* mp_resetEntry;
	
public:
	KeyboardConfig();
	~KeyboardConfig();

	void setWidgetPositions();
	void apply();

	std::string getKey(SDLKey key);
	SDLKey getKey(std::string& string);

	void setKey(pp::Entry* widget, SDLKey key);
};

#endif // KEYBOARD_CONFIG_H
