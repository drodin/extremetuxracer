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

#ifndef _EVENT_SELECT_H_
#define _EVENT_SELECT_H_

#include "loop.h"

#include "course_mgr.h"

#include "ppgltk/button.h"
#include "ppgltk/listbox.h"
#include "ppgltk/label.h"
#include "ppgltk/entry.h"

class EventSelect : public GameMode
{
	pp::Listbox<EventData>* mp_eventListbox;
	pp::Listbox<CupData>* mp_cupListbox;
	pp::Vec2d mp_cupListbox_pos;
	pp::Vec2d mp_cupListbox_size;
	
	std::list<EventData>::iterator m_curEvent;
	std::list<CupData>::iterator m_curCup;
	
	bool m_curCupComplete;
	bool m_curCupPlayable;
	
	pp::Button* mp_backBtn;
	pp::Button* mp_continueBtn;
		
	pp::Label *mp_titleLbl;
	pp::Label *mp_eventLbl;
	pp::Label *mp_cupLbl;
	pp::Label *mp_statusLbl;
	pp::Label *mp_nameLbl;
	
	pp::Entry *mp_nameEnt; 
	
	void updateCupStates();
	void updateButtonEnabledStates();
	
public:
	EventSelect();
	~EventSelect();

	void loop(float timeStep);

	void cupChanged();
	void eventChanged();

	void back();
	void apply();
	
	bool keyPressEvent(SDLKey key);
};

#endif // _EVENT_SELECT_H_
