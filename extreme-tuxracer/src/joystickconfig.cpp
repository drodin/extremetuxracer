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
 
#include "joystickconfig.h"

#include "joystick.h"
#include "ppgltk/ui_mgr.h"

#define MAX_JOY_BUTTON 8

JoystickConfig::JoystickConfig()
{	
	setTitle(_("Joystick Configuration"));
	pp::Vec2d pos(0,0);
	
	mp_enableJoystickLbl = new pp::Label(pos, "button_label",
										 _("Enable Joystick"));
	
	mp_enableJoystickBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_enableJoystickBox->setState( !getparam_disable_joystick());
    mp_enableJoystickBox->signalClicked.Connect(pp::CreateSlot(this,&JoystickConfig::checkboxClicked));
	
	
	//paddle button
	mp_paddleLbl = new pp::Label(pos,"button_label",_("Paddle:"));
	mp_paddleLbl->setInsensitiveFont("button_label_disabled");
	
	createButtonList(m_paddleList);
	mp_paddleListbox = new pp::Listbox<button_t>(pos,
				   pp::Vec2d(96, 32),
				   "listbox_item",
				   m_paddleList);
	mp_paddleListbox->setInsensitiveFont("listbox_item_insensitive");
	mp_paddleListbox->signalClicked.Connect(pp::CreateSlot(this,&JoystickConfig::paddleClicked));
	setButton(getparam_joystick_paddle_button(), mp_paddleListbox);
		
	//brake button
	mp_brakeLbl = new pp::Label(pos,"button_label",_("Brake:"));
	mp_brakeLbl->setInsensitiveFont("button_label_disabled");

	createButtonList(m_brakeList);
	mp_brakeListbox = new pp::Listbox<button_t>(pos,
				   pp::Vec2d(96, 32),
				   "listbox_item",
				   m_brakeList);
	mp_brakeListbox->setInsensitiveFont("listbox_item_insensitive");
	mp_brakeListbox->signalClicked.Connect(pp::CreateSlot(this,&JoystickConfig::brakeClicked));
	setButton(getparam_joystick_brake_button(), mp_brakeListbox);
	
	//jump button
	mp_jumpLbl = new pp::Label(pos,"button_label",_("Jump:"));
	mp_jumpLbl->setInsensitiveFont("button_label_disabled");

	createButtonList(m_jumpList);
	mp_jumpListbox = new pp::Listbox<button_t>(pos,
				   pp::Vec2d(96, 32),
				   "listbox_item",
				   m_jumpList);
	mp_jumpListbox->setInsensitiveFont("listbox_item_insensitive");
	mp_jumpListbox->signalClicked.Connect(pp::CreateSlot(this,&JoystickConfig::jumpClicked));
	setButton(getparam_joystick_jump_button(), mp_jumpListbox);
	
	//trick button
	mp_trickLbl = new pp::Label(pos,"button_label",_("Trick:"));
	mp_trickLbl->setInsensitiveFont("button_label_disabled");
	
	createButtonList(m_trickList);
	mp_trickListbox = new pp::Listbox<button_t>(pos,
				   pp::Vec2d(96, 32),
				   "listbox_item",
				   m_trickList);
	mp_trickListbox->setInsensitiveFont("listbox_item_insensitive");
	mp_trickListbox->signalClicked.Connect(pp::CreateSlot(this,&JoystickConfig::trickClicked));
	setButton(getparam_joystick_trick_button(), mp_trickListbox);
	
	updateWidgetsEnabledStates();
}

JoystickConfig::~JoystickConfig()
{
	delete mp_enableJoystickLbl;
	delete mp_enableJoystickBox;
	
	delete mp_paddleLbl;
	delete mp_paddleListbox;
		
	delete mp_brakeLbl;
	delete mp_brakeListbox;
			
	delete mp_jumpLbl;
	delete mp_jumpListbox;
		
	delete mp_trickLbl;
	delete mp_trickListbox;
}

void
JoystickConfig::createButtonList(std::list<button_t> &list)
{
	button_t button;
	char buff[8];
	
	for(int i=0; i<MAX_JOY_BUTTON; i++){
		sprintf(buff,"%d",i);
		button.name=buff;
		button.data=i;		
		list.push_back(button);			
	}	
}

void
JoystickConfig::setButton(int button, pp::Listbox<button_t> *listbox)
{
	std::list<button_t>::iterator it;
	std::list<button_t> &list = listbox->getItemList();
	
	
	for(it=list.begin(); it!=list.end(); it++){
		if((*it).data==button){
			listbox->setCurrentItem(it);
			break;
		}		
	}	
}

void
JoystickConfig::updateWidgetsEnabledStates()
{
	bool state = mp_enableJoystickBox->getState();
	
	mp_paddleLbl->setSensitive(state);
	mp_paddleListbox->setSensitive(state);
	
	mp_brakeLbl->setSensitive(state);
	mp_brakeListbox->setSensitive(state);
	
	mp_jumpLbl->setSensitive(state);
	mp_jumpListbox->setSensitive(state);
	
	mp_trickLbl->setSensitive(state);
	mp_trickListbox->setSensitive(state);
}

void
JoystickConfig::checkboxClicked()
{
	updateWidgetsEnabledStates();
}

void
JoystickConfig::customLoop(double TimeStep)
{
	// Check joystick
    if ( is_joystick_active() ){
		update_joystick();

		int button = get_joystick_down_button();
		if(button>-1){
			if(mp_paddleListbox->hasFocus()){
				setButton(button, mp_paddleListbox);
			}else if(mp_brakeListbox->hasFocus()){
				setButton(button, mp_brakeListbox);
			}else if(mp_jumpListbox->hasFocus()){
				setButton(button, mp_jumpListbox);
			}else if(mp_trickListbox->hasFocus()){
				setButton(button, mp_trickListbox);
			}
		}
    }
}

void
JoystickConfig::setWidgetPositions()
{
	int width = 500;
	int height = 240;

	pp::Vec2d pos(getparam_x_resolution()/2 - width/2,
				  getparam_y_resolution()/2 + height/2);
	
	mp_enableJoystickLbl->setPosition(pos);
	mp_enableJoystickBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=100;
	mp_paddleLbl->setPosition(pos);
	mp_paddleListbox->setPosition(pp::Vec2d(pos.x+width-96,pos.y));
		
	pos.y-=40;
	mp_brakeLbl->setPosition(pos);
	mp_brakeListbox->setPosition(pp::Vec2d(pos.x+width-96,pos.y));
	
	pos.y-=40;
	mp_jumpLbl->setPosition(pos);
	mp_jumpListbox->setPosition(pp::Vec2d(pos.x+width-96,pos.y));
	
	pos.y-=40;
	mp_trickLbl->setPosition(pos);
	mp_trickListbox->setPosition(pp::Vec2d(pos.x+width-96,pos.y));
}


void
JoystickConfig::apply()
{
	setparam_disable_joystick(!bool(mp_enableJoystickBox->getState() ));
		
	std::list<button_t>::iterator it;
	
	it = mp_paddleListbox->getCurrentItem();
	setparam_joystick_paddle_button((*it).data);
	
	it = mp_brakeListbox->getCurrentItem();
	setparam_joystick_brake_button((*it).data);
	
	it = mp_jumpListbox->getCurrentItem();
	setparam_joystick_jump_button((*it).data);
	
	it = mp_trickListbox->getCurrentItem();
	setparam_joystick_trick_button((*it).data);	
	
	write_config_file();
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();	
}

void
JoystickConfig::paddleClicked()
{
	mp_paddleListbox->setFocus();
}

void
JoystickConfig::brakeClicked()
{
	mp_brakeListbox->setFocus();
}

void
JoystickConfig::jumpClicked()
{
	mp_jumpListbox->setFocus();
}

void
JoystickConfig::trickClicked()
{
	mp_trickListbox->setFocus();
}
