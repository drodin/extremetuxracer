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
 
 
#include "configmode.h"

ConfigMode::ConfigMode()
{
	pp::Vec2d pos(0,0);
		
	mp_titleLbl = new pp::Label(pos,
					"heading", "");
	mp_titleLbl->alignment.center();
	mp_titleLbl->alignment.middle();
								
	
	mp_cancelBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40),
				     "button_label",
				     _("Cancel") );
    mp_cancelBtn->setHilitFontBinding( "button_label_hilit" );
    mp_cancelBtn->signalClicked.Connect(pp::CreateSlot(this,&ConfigMode::cancel));
   
	mp_applyBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Ok") );
    mp_applyBtn->setHilitFontBinding( "button_label_hilit" );
    mp_applyBtn->signalClicked.Connect(pp::CreateSlot(this,&ConfigMode::apply));
}

ConfigMode::~ConfigMode()
{
	delete mp_cancelBtn;
	delete mp_applyBtn;
	delete mp_titleLbl;
}

void
ConfigMode::loop(float timeStep)
{
	update_audio();
    set_gl_options( GUI );
    clear_rendering_context();
    UIMgr.setupDisplay();
	
	drawTextandWidgets();
	
	// forces the config mode to update it's widget positions
	// perhaps we should call this only when the screen size changes
	setWidgetPositions();
	
	// calls the custom loop of the derived class (if it exists)
	customLoop( timeStep );
	
	// draw snow if enabled
	drawSnow( timeStep );
	
	theme.drawMenuDecorations();
	
	// draw ui widgets
    UIMgr.draw();
	
	
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    winsys_swap_buffers();
}

void
ConfigMode::drawTextandWidgets()
{
	int w = getparam_x_resolution();
    int h = getparam_y_resolution();

	mp_cancelBtn->setPosition(pp::Vec2d(w/2-300,h/2-240));
	mp_applyBtn->setPosition(pp::Vec2d(w/2,h/2-240));
	mp_titleLbl->setPosition(pp::Vec2d(w/2,h/2+200));
}

bool
ConfigMode::keyPressEvent(SDLKey key)
{
	switch (key){
		case SDLK_ESCAPE:
			cancel();
	    	return true;
		case SDLK_RETURN:
			apply();
	    	return true;
		default:
			return false;
	}
}

void
ConfigMode::cancel()
{
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();
}

void
ConfigMode::setTitle(const char* title)
{
	mp_titleLbl->setText(title);
}
