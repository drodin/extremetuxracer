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


#include "configuration.h"

#include "ppgltk/button.h"
#include "ppgltk/audio/audio.h"

static game_mode_t preConfigMode = NO_MODE;


Configuration::Configuration()
{
  play_music( "options_screen" );
	if(GameMode::prevmode==GAME_TYPE_SELECT || GameMode::prevmode == PAUSED){
		preConfigMode = GameMode::prevmode;
	}
	
	pp::Vec2d pos(0,0);
	
	mp_titleLbl = new pp::Label(pos,"heading",_("Configuration"));
	mp_titleLbl->alignment.center();
	mp_titleLbl->alignment.middle();	
	
	mp_graphicsBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40),
				     "button_label",
				     _("Graphics") );
    mp_graphicsBtn->setHilitFontBinding( "button_label_hilit" );
    mp_graphicsBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::graphics));
	
	mp_videoBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Video") );
    mp_videoBtn->setHilitFontBinding( "button_label_hilit" );
	mp_videoBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::video));	

	mp_audioBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Audio") );
    mp_audioBtn->setHilitFontBinding( "button_label_hilit" );
	mp_audioBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::audio));	

	mp_keyboardBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Keyboard") );
   	mp_keyboardBtn->setHilitFontBinding( "button_label_hilit" );
	mp_keyboardBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::keyboard));	

	mp_joystickBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Joystick") );
   	mp_joystickBtn->setHilitFontBinding( "button_label_hilit" );
	mp_joystickBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::joystick));	


    mp_backBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Back") );
    mp_backBtn->setHilitFontBinding( "button_label_hilit" );
    mp_backBtn->signalClicked.Connect(pp::CreateSlot(this,&Configuration::back));		
}

Configuration::~Configuration()
{
	delete mp_graphicsBtn;
	delete mp_videoBtn;
	delete mp_audioBtn;
	delete mp_keyboardBtn;
	delete mp_joystickBtn;
	delete mp_backBtn;
	delete mp_titleLbl;
}

void
Configuration::loop(float timeStep)
{
	update_audio();
    set_gl_options( GUI );
    clear_rendering_context();
    UIMgr.setupDisplay();
	drawSnow(timeStep);
	theme.drawMenuDecorations();
	setWidgetPositions();
    UIMgr.draw();
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    winsys_swap_buffers();	
}

void
Configuration::setWidgetPositions()
{
	int w = getparam_x_resolution();
    int h = getparam_y_resolution();
	
	pp::Vec2d pos(w/2,h/2+100);

	mp_titleLbl->setPosition(pos);
	
	pos.x-=150;
	pos.y-=80;
	mp_graphicsBtn->setPosition(pos);
	pos.y-=40;
	mp_videoBtn->setPosition(pos);
	pos.y-=40;
	mp_audioBtn->setPosition(pos);
	pos.y-=40;
	mp_keyboardBtn->setPosition(pos);
	pos.y-=40;
	mp_joystickBtn->setPosition(pos);
	pos.y-=60;
	mp_backBtn->setPosition(pos);	
}

bool
Configuration::keyPressEvent(unsigned int key)
{
	switch (key){
		case 27:
		case 'q':
			back();
	    	return true;
	}	
	return false;
}

void
Configuration::back()
{
	set_game_mode( preConfigMode );
	preConfigMode = NO_MODE;
    UIMgr.setDirty();
}

void
Configuration::graphics()
{
	set_game_mode( CONFIG_GRAPHICS );
    UIMgr.setDirty();
}

void
Configuration::video()
{
	set_game_mode( CONFIG_VIDEO );
    UIMgr.setDirty();
}

void
Configuration::audio()
{
	set_game_mode( CONFIG_AUDIO );
    UIMgr.setDirty();
}

void
Configuration::keyboard()
{
	set_game_mode( CONFIG_KEYBOARD );
    UIMgr.setDirty();
}

void
Configuration::joystick()
{
	set_game_mode( CONFIG_JOYSTICK );
    UIMgr.setDirty();
}
