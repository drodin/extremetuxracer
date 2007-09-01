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
 
#include "audioconfig.h"

#include "game_config.h"
#include "ppgltk/ui_mgr.h"


bps_t bps[] = { 	{"8",0},
					{"16",1}};
						
freq_t freqs[] = { 	{"11025",0},
					{"22050",1},
					{"44100",2}};


AudioConfig::AudioConfig()
{
	setTitle(_("Audio Configuration"));
	pp::Vec2d pos(0,0);
	
	mp_audioBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_audioBox->setState( getparam_no_audio());
	
	mp_soundBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_soundBox->setState( getparam_sound_enabled());
		
	mp_musicBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_musicBox->setState( getparam_music_enabled());	
	
	mp_stereoBox = new pp::CheckBox(pos, pp::Vec2d(32, 32) );
	mp_stereoBox->setState( getparam_audio_stereo());

	//frequency
	std::list<freq_t>::iterator freqit;
	bool found=false;
	
	for (unsigned int i=0; i<sizeof(freqs)/sizeof(freq_t); i++) {
		m_freqList.push_back(freqs[i]);	
		if (freqs[i].data==getparam_audio_freq_mode()){
			freqit = --m_freqList.end();
			found=true;
		}
    }
	if(!found) freqit = m_freqList.begin();
	
	mp_freqListbox = new pp::Listbox<freq_t>( pos,
				   pp::Vec2d(150, 32),
				   "listbox_item",
				   m_freqList);
    mp_freqListbox->setCurrentItem( freqit );
	
	//bits per sample
	std::list<freq_t>::iterator bpsit;
	found=false;
	
	for (unsigned int i=0; i<sizeof(bps)/sizeof(bps_t); i++) {
		m_bpsList.push_back(bps[i]);	
		if (bps[i].data==getparam_audio_format_mode()){
			bpsit = --m_bpsList.end();
			found=true;
		}
    }
	if(!found) bpsit = m_bpsList.begin();
	
	mp_bpsListbox = new pp::Listbox<bps_t>( pos,
				   pp::Vec2d(150, 32),
				   "listbox_item",
				   m_bpsList);
    mp_bpsListbox->setCurrentItem( bpsit );

}

AudioConfig::~AudioConfig()
{
	delete mp_audioBox;
	delete mp_soundBox;
	delete mp_musicBox;
	delete mp_stereoBox;
	delete mp_bpsListbox;
	delete mp_freqListbox;
}

void
AudioConfig::setWidgetPositions()
{
	int width = 500;
	int height = 240;

	pp::Vec2d pos(getparam_x_resolution()/2 - width/2,
				  getparam_y_resolution()/2 + height/2);
	
	pp::Font *font= pp::Font::get("button_label");
	
	font->draw(_("Sound Effects:"),pos);
	mp_soundBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Music:"),pos);
	mp_musicBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=80;
	font->draw(_("(needs restart)"),pos);
	pos.y-=40;
	font->draw(_("Disable Audio:"),pos);
	mp_audioBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Stereo:"),pos);
	mp_stereoBox->setPosition(pp::Vec2d(pos.x+width-32,pos.y));
	
	pos.y-=40;
	font->draw(_("Bits Per Sample:"),pos);
	mp_bpsListbox->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
	
	pos.y-=40;
	font->draw(_("Samples Per Second:"),pos);
	mp_freqListbox->setPosition(pp::Vec2d(pos.x+width-150,pos.y));
}

void
AudioConfig::apply()
{
	setparam_no_audio(bool( mp_audioBox->getState() ));
	setparam_sound_enabled(bool( mp_soundBox->getState() ));
	setparam_music_enabled(bool( mp_musicBox->getState() ));
	setparam_audio_stereo(bool( mp_stereoBox->getState() ));
	
	std::list<bps_t>::iterator bpsit = mp_bpsListbox->getCurrentItem();
	setparam_audio_format_mode((*bpsit).data);
		
	std::list<freq_t>::iterator freqit = mp_freqListbox->getCurrentItem();
	setparam_audio_freq_mode((*freqit).data);
		
	write_config_file();
	set_game_mode( GameMode::prevmode );
    UIMgr.setDirty();	
}
