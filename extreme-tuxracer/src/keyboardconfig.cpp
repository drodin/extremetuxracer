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
 
#include "keyboardconfig.h"

#include "game_config.h"
#include "game_mgr.h"
#include "ppgltk/ui_mgr.h"

// Definition of Key

KeyEntry::KeyEntry(pp::Vec2d pos, pp::Vec2d size, const char *binding, const char *content) : pp::Entry(pos, size, binding, content)
{
keysym = SDLK_BACKSPACE; //Well, just to reserve the memory
}

KeyEntry::~KeyEntry()
{
}

void KeyEntry::setKeySym(SDLKey key)
{
keysym = key;
}

SDLKey KeyEntry::getKeySym()
{
return keysym;
}


// Definition of KeyboardConfig

KeyboardConfig::KeyboardConfig()
{
	setTitle(_("Keyboard Configuration"));
	
	pp::Vec2d pos(0,0);
	pp::Vec2d size(150,32);
	
	mp_left = new KeyEntry(pos, size, "listbox_item");
	mp_left->setEditable(false);
	mp_left->signalKeyPressed.Connect(pp::CreateSlot(this, &KeyboardConfig::setKey));
   	setKey( mp_left, SDLKey(getparam_turn_left_key()) );
	
	mp_right = new KeyEntry(pos, size,
				     "listbox_item");
	mp_right->setEditable(false);
	mp_right->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_right, SDLKey(getparam_turn_right_key()) );
	
	mp_paddle = new KeyEntry(pos, size,
				     "listbox_item");
	mp_paddle->setEditable(false);
	mp_paddle->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_paddle, SDLKey(getparam_paddle_key()) );
		
	mp_brake = new KeyEntry(pos, size,
				     "listbox_item");
	mp_brake->setEditable(false);
	mp_brake->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_brake, SDLKey(getparam_brake_key()) );
	
	mp_jump = new KeyEntry(pos, size,
				     "listbox_item");
	mp_jump->setEditable(false);
	mp_jump->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_jump, SDLKey(getparam_jump_key()) );
	
	mp_trick = new KeyEntry(pos, size,
				     "listbox_item");
	mp_trick->setEditable(false);
	mp_trick->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_trick, SDLKey(getparam_trick_modifier_key()) );
	
	mp_reset = new KeyEntry(pos, size,
				     "listbox_item");
	mp_reset->setEditable(false);
	mp_reset->signalKeyPressed.Connect(pp::CreateSlot(this,&KeyboardConfig::setKey));
   	setKey( mp_reset, SDLKey(getparam_reset_key()) );
}

KeyboardConfig::~KeyboardConfig()
{
	delete mp_left;
	delete mp_right;
	delete mp_paddle;
	delete mp_brake;
	delete mp_jump;
	delete mp_trick;
	delete mp_reset;
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
	mp_left->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
		
	pos.y-=40;
	font->draw(_("Turn right:"),pos);
	mp_right->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Paddle:"),pos);
	mp_paddle->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Brake:"),pos);
	mp_brake->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Jump:"),pos);
	mp_jump->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Trick:"),pos);
	mp_trick->setPosition(pp::Vec2d(pos.x+width-150,pos.y));

	pos.y-=40;
	font->draw(_("Reset:"),pos);
	mp_reset->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
}

void
KeyboardConfig::apply()
{
	setparam_turn_left_key( mp_left->getKeySym() );
	setparam_turn_right_key( mp_right->getKeySym() );		
	setparam_paddle_key( mp_paddle->getKeySym() );		
	setparam_brake_key( mp_brake->getKeySym() );		
	setparam_jump_key( mp_jump->getKeySym() );		
	setparam_trick_modifier_key( mp_trick->getKeySym() );		
	setparam_reset_key( mp_reset->getKeySym() );		
	
	write_config_file();
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();
}

void
KeyboardConfig::setKey(KeyEntry* widget, SDLKey key)
{
	std::string content="";
	
	if(key==SDLK_BACKSPACE){
		widget->setContent(content);
		return;
	}else{
		content = SDL_GetKeyName(key);
	}
	
	if (content!=""){
		widget->setContent(content);
		widget->setKeySym(key);
	}
}

