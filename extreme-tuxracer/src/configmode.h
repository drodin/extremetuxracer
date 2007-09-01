/* 
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
 
#ifndef _CONFIGMODE_H
#define _CONFIGMODE_H

#include "loop.h"

#include "ppgltk/button.h"
#include "ppgltk/label.h"
#include "ppgltk/audio/audio.h"

#include <string>


class ConfigMode : public GameMode
{
protected:
	pp::Button *mp_cancelBtn;
	pp::Button *mp_applyBtn;
	pp::Label *mp_titleLbl;

	std::string m_title;

	void setTitle(const char* title);
		
public:
	ConfigMode();
	~ConfigMode();

	void loop(float timeStep);

	void drawTextandWidgets();
	bool keyPressEvent(SDLKey key);

	void cancel();

	virtual void customLoop(float timeStep){};
	virtual void setWidgetPositions() = 0;
	virtual void apply() = 0;
};

#endif // GRAPHICS_H
