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
 
#ifndef _JOYSTICK_CONFIG_H_
#define _JOYSTICK_CONFIG_H_

#include "configmode.h"

#include "ppgltk/checkbox.h"
#include "ppgltk/label.h"
#include "ppgltk/listbox.h"

typedef struct{
	std::string name;
	int data;
}button_t;


class JoystickConfig : public ConfigMode
{
	pp::CheckBox *mp_enableJoystickBox;
	
	pp::Label *mp_enableJoystickLbl;
	
	pp::Label *mp_paddleLbl;
	pp::Listbox<button_t> *mp_paddleListbox;
	std::list<button_t> m_paddleList;
	
	pp::Label *mp_brakeLbl;
	pp::Listbox<button_t> *mp_brakeListbox;
	std::list<button_t> m_brakeList;
	
	pp::Label *mp_jumpLbl;
	pp::Listbox<button_t> *mp_jumpListbox;
	std::list<button_t> m_jumpList;
	
	pp::Label *mp_trickLbl;
	pp::Listbox<button_t> *mp_trickListbox;
	std::list<button_t> m_trickList;
	
	void createButtonList(std::list<button_t> &list);
	void setButton(int button, pp::Listbox<button_t> *listbox);
	void updateWidgetsEnabledStates();
	
public:
	JoystickConfig();
	~JoystickConfig();

	void setWidgetPositions();
	void apply();

	void checkboxClicked();

	void paddleClicked();
	void brakeClicked();
	void jumpClicked();
	void trickClicked();
	
	void customLoop(double TimeStep);

};

#endif // _JOYSTICK_CONFIG_H_
