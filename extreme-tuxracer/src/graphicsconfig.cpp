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
 
 
#include "graphicsconfig.h"

#include "game_config.h"
#include "game_mgr.h"
#include "ppgltk/ui_mgr.h"


GraphicsConfig::GraphicsConfig()
{
	setTitle(_("Graphics Configuration"));	
	
	pp::Vec2d pos(0,0);

	m_modelList = ModelHndl->l_models;
	std::list<model_t>::iterator modelit=m_modelList.begin();

	m_langList=translation.LanguageList();
	std::list<language_t>::iterator langit,iter;
	
      	bool found=false;

	for (iter=m_langList.begin();iter != m_langList.end(); ++iter) {
		if ((*iter).language==getparam_ui_language()) {
			langit=iter;
			found=true;
		}
	}
		
	if(!found) langit = m_langList.begin();
     
     for(int i=0;i<ModelHndl->cur_model;i++) {
     	modelit++;
     }
     
 	mp_modelListBox = new pp::Listbox<model_t>( pos, pp::Vec2d(240, 32), "listbox_item", m_modelList);
 	mp_modelListBox->setCurrentItem( modelit );
 
	mp_langListBox = new pp::Listbox<language_t>( pos, pp::Vec2d(240, 32), "listbox_item", m_langList);
	mp_langListBox->setCurrentItem( langit );
	    
	
	mp_uiSnowBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_uiSnowBox->setState( getparam_ui_snow() );

	mp_fpsBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_fpsBox->setState( getparam_display_fps() );

	mp_coursePercentageBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_coursePercentageBox->setState( getparam_display_course_percentage() );
	
	mp_fogBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_fogBox->setState( !getparam_disable_fog() );

	mp_reflectionsBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_reflectionsBox->setState( getparam_terrain_envmap() );

	mp_shadowsBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_shadowsBox->setState( getparam_draw_tux_shadow() );
}

GraphicsConfig::~GraphicsConfig()
{
	delete mp_modelListBox;
	delete mp_langListBox;
	delete mp_uiSnowBox;
	delete mp_fpsBox;
	delete mp_coursePercentageBox;
	delete mp_fogBox;
	delete mp_reflectionsBox;
	delete mp_shadowsBox;
}

void
GraphicsConfig::setWidgetPositions()
{
	int width = 550;
	int height = 240;
		
	pp::Vec2d pos(getparam_x_resolution()/2 - width/2,
				  getparam_y_resolution()/2 + height/2);
	
	pp::Font* font = pp::Font::get("button_label");

	font->draw(_("Model:"),pos);
	mp_modelListBox->setPosition(pp::Vec2d(pos.x+width-204,pos.y));

	pos.y-=40;	

	font->draw(_("Language:"),pos);
	mp_langListBox->setPosition(pp::Vec2d(pos.x+width-204,pos.y));

	pos.y-=40;	
	font->draw(_("Show UI Snow:"),pos);
	mp_uiSnowBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Display FPS:"),pos);
	mp_fpsBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Display Progress Bar:"),pos);
	mp_coursePercentageBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Draw Fog:"),pos);
	mp_fogBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Reflections:"),pos);
	mp_reflectionsBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Shadows:"),pos);
	mp_shadowsBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
}

void
GraphicsConfig::apply()
{
	std::list<language_t>::iterator langit = mp_langListBox->getCurrentItem();
	translation.load((*langit).language.c_str());	
	std::list<model_t>::iterator modelit = mp_modelListBox->getCurrentItem();
	ModelHndl->load_model((*modelit).id);	
	setparam_ui_language((char*)(*langit).language.c_str());
	setparam_ui_snow(bool( mp_uiSnowBox->getState() ));
	setparam_display_fps(bool( mp_fpsBox->getState() ));
	setparam_display_course_percentage(bool( mp_coursePercentageBox->getState() ));
	setparam_disable_fog(!bool(mp_fogBox->getState() ));
	setparam_terrain_envmap(bool( mp_reflectionsBox->getState() ));
	setparam_draw_tux_shadow(bool( mp_shadowsBox->getState() ));
	
	write_config_file();
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();	
}
