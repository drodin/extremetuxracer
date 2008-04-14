/*  
 * ETRacer
 * Copyright (C) 2007-2008 The ETRacer Team <www.extremetuxracer.com>
 *
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

// KeyEntry declaration

class KeyEntry : public pp::Entry
{
	SDLKey keysym;
public:
	KeyEntry(pp::Vec2d pos, pp::Vec2d size, const char *binding, const char *content="");
	~KeyEntry();
	void setKeySym(SDLKey keysym);	
	SDLKey getKeySym();
};

// KeyboardConfig declaration

class KeyboardConfig : public ConfigMode
{
	KeyEntry* mp_left;
	KeyEntry* mp_right;
	KeyEntry* mp_paddle;
	KeyEntry* mp_brake;
	KeyEntry* mp_jump;
	KeyEntry* mp_trick;
	KeyEntry* mp_reset;
	
public:
	KeyboardConfig();
	~KeyboardConfig();

	void setWidgetPositions();
	void apply();

	void setKey(KeyEntry* widget, SDLKey key);
};

#endif // KEYBOARD_CONFIG_H
