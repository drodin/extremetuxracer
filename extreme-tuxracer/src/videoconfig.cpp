/* 
 * ETRacer 
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
 
/*
 * Note:
 * Perhaps we should combine resolution and bpps in one list.
 * The only problem i see here is that the videomode autodetection
 * cannot detect what resolutions are supported in different 
 * bpp modes.
 *
 * Multisamples:
 * This is currently a little bit broken.
 * There is no test whether the hardware supports this and
 * the list of available multisamples is static. 
 */


 
#include "videoconfig.h"

#include "game_config.h"
#include "ppgltk/ui_mgr.h"
#include "winsys.h"

/// List of default resolitions (only if autodetection fails)
resolution_t resolutions[] = { 	{"1280 x 1024",1280,1024},
								{"1152 x 864",1152,864},
								{"1024 x 768",1024,768},
								{"800 x 600",800,600},
								{"640 x 480",640,480}};

/// List of bpp modes							
bpp_t bpps[] = { 	{"Display",0},
						{"16",1},
						{"32",2}};

/// List of multisamples						
multisample_t multisamples[] = { 	{"1",1},
						{"2",2},
						{"4",4}};					
 	
VideoConfig::VideoConfig()
{
	setTitle(_("Video Configuration"));	
	
	pp::Vec2d pos(0,0);
	
	// resolution listbox 
	
	bool found=false;
	#ifdef _WIN32
		//Skip the resolutionbox
	#else
	
	std::list<resolution_t>::iterator resit;

	
	initVideoModes();
	
	// check for the current resolution
	for (resit = m_resolutionList.begin();
		 resit != m_resolutionList.end();
		 resit++)
	{
		if ( (*resit).x==getparam_x_resolution() && (*resit).y==getparam_y_resolution())
			break;			 
	}

	if (resit == m_resolutionList.end()){
		// current resolution not in list
		// therefore we simply add this
		char buff[16];
		resolution_t resolution;
		sprintf(buff,"%d x %d",getparam_x_resolution(), getparam_y_resolution());
		resolution.name = buff;
		resolution.x=getparam_x_resolution();
		resolution.y=getparam_y_resolution();
		m_resolutionList.insert(m_resolutionList.begin(),resolution);
		resit = m_resolutionList.begin();
	}
	

	mp_resolutionListbox = new pp::Listbox<resolution_t>( pos,
				   pp::Vec2d(190, 32),
				   "listbox_item",
				   m_resolutionList);
     mp_resolutionListbox->setCurrentItem( resit );
	
  	//bpp listbox
	std::list<bpp_t>::iterator bppit;
	for (unsigned int i=0; i < sizeof(bpps)/sizeof(bpp_t) ; i++) {
		m_bppList.push_back(bpps[i]);
		if(bpps[i].data==getparam_bpp_mode()){
			bppit = --m_bppList.end();
		}
    }
    mp_bppListbox = new pp::Listbox<bpp_t>( pos,
					pp::Vec2d(190, 32),
					"listbox_item",
					m_bppList);
    mp_bppListbox->setCurrentItem( bppit );
	
	mp_fullscreenBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_fullscreenBox->setState( getparam_fullscreen() );
#endif
	
	mp_stencilBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_stencilBox->setState( getparam_stencil_buffer() );
	
	//multisample listbox
	found=false;
	std::list<multisample_t>::iterator multiit;
	for (unsigned int i=0; i < sizeof(multisamples)/sizeof(multisample_t) ; i++) {
		m_multisampleList.push_back(multisamples[i]);
		if(multisamples[i].data==getparam_multisamples()){
			multiit = --m_multisampleList.end();
			found=true;
		}
    }
	if(!found) multiit = m_multisampleList.begin();
    mp_multisampleListbox = new pp::Listbox<multisample_t>( pos,
					pp::Vec2d(150, 32),
					"listbox_item",
					m_multisampleList);
    mp_multisampleListbox->setCurrentItem( multiit );
	
	mp_fsaaBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_fsaaBox->setState( getparam_enable_fsaa());
}

VideoConfig::~VideoConfig()
{
	#ifdef _WIN32
	#else
		delete mp_resolutionListbox;
		delete mp_bppListbox;
		delete mp_fullscreenBox;
	#endif
	
	delete mp_multisampleListbox;
	delete mp_stencilBox;
	delete mp_fsaaBox;
}

void
VideoConfig::setWidgetPositions()
{
	int width = 550;
	int height = 240;
	
	pp::Vec2d pos(getparam_x_resolution()/2 - width/2,
				  getparam_y_resolution()/2 + height/2);
	
	pp::Font *font = pp::Font::get("button_label");
	
	#ifdef _WIN32
		font->draw(_("To change the resolution, or switch into fullscreen mode"),pos);
		pos.y-=30;
		font->draw(_("use options.txt, located in the config folder."),pos);
	#else
		font->draw(_("Resolution:"),pos);
		mp_resolutionListbox->setPosition(pp::Vec2d(pos.x+width-190,pos.y));	
	
		pos.y-=40;
		font->draw(_("Bits Per Pixel:"),pos);
		mp_bppListbox->setPosition(pp::Vec2d(pos.x+width-190,pos.y));
		
		pos.y-=40;
		font->draw(_("Fullscreen:"),pos);
		mp_fullscreenBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	#endif
	pos.y-=80;
	font->draw(_("Experimental (needs restart)"),pos);
	
	pos.y-=40;
	font->draw("Stencil Buffer:",pos);
	mp_stencilBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Enable FSAA:"),pos);
	mp_fsaaBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw("FSAA Multisamples:",pos);
	mp_multisampleListbox->setPosition(pp::Vec2d(pos.x+width-150,pos.y));

}

void
VideoConfig::apply()
{
	bool updatevideo = false;
	
	std::list<multisample_t>::iterator multiit = mp_multisampleListbox->getCurrentItem();
	
	#ifdef _WIN32
		//Skip the resolutionbox
	#else
		std::list<resolution_t>::iterator resit = mp_resolutionListbox->getCurrentItem();
		std::list<bpp_t>::iterator bppit = mp_bppListbox->getCurrentItem();
	

		if ( (*resit).x != getparam_x_resolution() ){
			setparam_x_resolution((*resit).x);
			setparam_y_resolution((*resit).y);
			updatevideo=true;	
		}
	
	
		if ( (*bppit).data != getparam_bpp_mode() ){
			setparam_bpp_mode((*bppit).data);
			updatevideo=true;	
		}
		
		if (mp_fullscreenBox->getState() != getparam_fullscreen() ){
			setparam_fullscreen(bool( mp_fullscreenBox->getState() ));
			updatevideo=true;
		}
	#endif
	
	if (mp_stencilBox->getState() != getparam_stencil_buffer() ){
		setparam_stencil_buffer(bool( mp_stencilBox->getState() ));
		updatevideo=true;
	}
	
	if (mp_fsaaBox->getState() != getparam_enable_fsaa() ){
		setparam_enable_fsaa(bool( mp_fsaaBox->getState() ));
		updatevideo=true;
	}
	
	if ( (*multiit).data != getparam_multisamples() ){
		setparam_multisamples((*multiit).data);
		updatevideo=true;	
	}
	
	#ifdef _WIN32
		//Skip video update
	#else
 	if (updatevideo==true){
		printf("Set new videomode:%dx%d bpp:%d \n", (*resit).x, (*resit).y,(*bppit).data);
		setup_sdl_video_mode();
	}
	#endif
	
	write_config_file();
	
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();	
}

void
VideoConfig::initVideoModes()
/** 
 * fills m_resolutionList with modes 
 * by trying to autodetect the available modes.
 * If the autodetection fails, standard modes are used.
 */
{
	SDL_Rect **modes;
	int i;

	// get available fullscreen OpenGL modes
	if(!getparam_disable_videomode_autodetection()){	
		modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);
	}else{
		modes=(SDL_Rect **)-1;
	}
	// check if we cannot receive the modes
	if(	modes == (SDL_Rect **)-1 ||
		modes == (SDL_Rect **)0 )
	{
		// unable to find modes... fall back to standard modes
		for (unsigned int i=0; i<sizeof(resolutions)/sizeof(resolution_t); i++) {
			m_resolutionList.push_back(resolutions[i]);
		}	
	}else{
		char buff[16];
		resolution_t resolution;
		
		// fill list with modes
		for(i=0; modes[i]; ++i){
			sprintf(buff,"%d x %d", modes[i]->w, modes[i]->h);
			resolution.name = buff;
			resolution.x=modes[i]->w;
			resolution.y=modes[i]->h;
			m_resolutionList.push_back(resolution);	
		}	
	}
}
