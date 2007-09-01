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
 
#include "keyboardconfig.h"

#include "game_config.h"
#include "game_mgr.h"
#include "ppgltk/ui_mgr.h"
	
struct KeyMap{
	SDLKey key;
	char string[10];
};

static KeyMap keymap[] = {
	{SDLK_LEFT,"left"},
	{SDLK_RIGHT,"right"},
	{SDLK_UP,"up"},
	{SDLK_DOWN,"down"},
	{SDLK_KP0,"keypad 0"},
	{SDLK_KP1,"keypad 1"},
	{SDLK_KP2,"keypad 2"},
	{SDLK_KP3,"keypad 3"},
	{SDLK_KP4,"keypad 4"},
	{SDLK_KP5,"keypad 5"},
	{SDLK_KP6,"keypad 6"},
	{SDLK_KP7,"keypad 7"},
	{SDLK_KP8,"keypad 8"},
	{SDLK_KP9,"keypad 9"},
	{SDLK_KP_PERIOD,"keypad ."},
	{SDLK_KP_DIVIDE,"keypad /"},
	{SDLK_KP_MULTIPLY,"keypad *"},
	{SDLK_KP_MINUS,"keypad m"},
	{SDLK_KP_PLUS,"keypad +"},
	{SDLK_KP_EQUALS,"keypad ="}	
};

	
	
KeyboardConfig::KeyboardConfig()
{
	setTitle(_("Keyboard Configuration"));
	
	pp::Vec2d pos(0,0);
	pp::Vec2d size(150,32);
	
	mp_leftEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_leftEntry->setEditable(false);
	mp_leftEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_leftEntry->setContent(getKey(SDLKey(getparam_turn_left_key())));
	
	mp_rightEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_rightEntry->setEditable(false);
	mp_rightEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_rightEntry->setContent(getKey(SDLKey(getparam_turn_right_key())));
	
	mp_paddleEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_paddleEntry->setEditable(false);
	mp_paddleEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_paddleEntry->setContent(getKey(SDLKey(getparam_paddle_key())));
	
	mp_brakeEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_brakeEntry->setEditable(false);
	mp_brakeEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_brakeEntry->setContent(getKey(SDLKey(getparam_brake_key())));
	
	mp_jumpEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_jumpEntry->setEditable(false);
	mp_jumpEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_jumpEntry->setContent(getKey(SDLKey(getparam_jump_key())));
	
	mp_trickEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_trickEntry->setEditable(false);
	mp_trickEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_trickEntry->setContent(getKey(SDLKey(getparam_trick_modifier_key())));
	
	mp_resetEntry = new pp::Entry(pos, size,
				     "listbox_item");
	mp_resetEntry->setEditable(false);
	mp_resetEntry->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	mp_resetEntry->setContent(getKey(SDLKey(getparam_reset_key())));	
}

KeyboardConfig::~KeyboardConfig()
{
	delete mp_leftEntry;
	delete mp_rightEntry;
	delete mp_paddleEntry;
	delete mp_brakeEntry;
	delete mp_jumpEntry;
	delete mp_trickEntry;
	delete mp_resetEntry;
}

void
KeyboardConfig::setWidgetPositions()
{
	int width = 500;
	int height = 240;
		
	pp::Vec2d pos(getparam_x_resolution()/2 - width/2,
				  getparam_y_resolution()/2 + height/2);
	
	pp::Font *font = pp::Font::get("button_label");
	
	font->draw(_("Turn left:"),pos);
	mp_leftEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
		
	pos.y-=40;
	font->draw(_("Turn right:"),pos);
	mp_rightEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Paddle:"),pos);
	mp_paddleEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Brake:"),pos);
	mp_brakeEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Jump:"),pos);
	mp_jumpEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Trick:"),pos);
	mp_trickEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));

	pos.y-=40;
	font->draw(_("Reset:"),pos);
	mp_resetEntry->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
}

void
KeyboardConfig::apply()
{
	setparam_turn_left_key(getKey(mp_leftEntry->getContent()));
	setparam_turn_right_key(getKey(mp_rightEntry->getContent()));		
	setparam_paddle_key(getKey(mp_paddleEntry->getContent()));		
	setparam_brake_key(getKey(mp_brakeEntry->getContent()));		
	setparam_jump_key(getKey(mp_jumpEntry->getContent()));		
	setparam_trick_modifier_key(getKey(mp_trickEntry->getContent()));		
	setparam_reset_key(getKey(mp_resetEntry->getContent()));		
	
	write_config_file();
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();
}

void
KeyboardConfig::setKey(pp::Entry* widget, SDLKey key)
{
	std::string content="";
	
	if(key==SDLK_BACKSPACE){
		widget->setContent(content);
		return;
	}else{
		content = getKey(key);
	}
	
	if (content!=""){
		widget->setContent(content);
	}
}

std::string KeyboardConfig::getKey(SDLKey key)
{
	std::string content="";
	if(isalnum(key)){
		content=key;
	}else{
		for(unsigned int i=0;i<sizeof(keymap)/sizeof(KeyMap);i++){
			if(key==keymap[i].key){
				content=keymap[i].string;
				break;
			}
		}
	}
	return content;
}

SDLKey
KeyboardConfig::getKey(std::string& string)
{
	if(string!=""){
		for(unsigned int i=0;i<sizeof(keymap)/sizeof(KeyMap);i++){
			if(string==keymap[i].string){
				return keymap[i].key;
			}
		}
		return SDLKey(string.c_str()[0]);		
	}else{
		return SDLKey(0);
	}
}
