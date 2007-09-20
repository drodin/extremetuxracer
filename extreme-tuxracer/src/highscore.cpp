/* 
 * ETRacer
 * Copyright (C) 2004-2006 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
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
 *
 */

#include "highscore.h"
#include "stuff.h"
#include "ppgltk/audio/audio.h"

#include "textures.h"
#include "joystick.h"

#include "player.h"
#include "game_mgr.h"



/* class highscore
handles reading and writing to the highscore.
*/

highscore* Highscore = NULL;

bool highscore::useHighscore=true;

bool highscore::hsDebug=false;

highscore::highscore() {
	level_hs_length=0;
	bool anyHighscore=false;
}

int
highscore::addScore(std::string level, std::string nick,int score) {
	int n=findLevel(level);
	if(n == -1) {
		redim();
		level_hs[level_hs_length-1].level = level;
		n = level_hs_length-1;
	}
	//std::cout<<score << " > " << level_hs[n].post[9].score<<std::endl;
	if(score > level_hs[n].post[9].score) {
		/*
			the list should be sorted, so if score is greater than the lowest score
			we replace that post with this new one. Then we sort the list
		*/
		level_hs[n].post[9].score = score;
		level_hs[n].post[9].nick.assign(nick,0,nick.length());
		if(level_hs[n].posts < 10)
			level_hs[n].posts++;
		sort(n);
		anyHighscore=true;
		//printlist();
		for(int i=0;i<level_hs[n].posts;i++) {
			if(level_hs[n].post[i].score == score) {
				return (i+1);
			}
		}
		return level_hs[n].posts;
	} else {
		return -1;
	}
		
}

std::string
highscore::posToStr(int pos) {
	switch(pos) {
		case 1:
			return _("1:st");
		case 2:
			return _("2:nd");
		case 3:
			return _("3:rd");
		case 4:
			return _("4:th");
		case 5:
			return _("5:th");
		case 6:
			return _("6:th");
		case 7:
			return _("7:th");
		case 8:
			return _("8:th");
		case 9:
			return _("9:th");
		case 10:
			return _("10:th");
		default:
			return "";
			
	}
}

void
highscore::sort(int l) {
		for(int i=1; i < 10; i++) {
	 		int pos = i;
	 		scorepost tmp = level_hs[l].post[i];
	 		while(pos > 0 && level_hs[l].post[pos-1].score < tmp.score) {
	 			level_hs[l].post[pos] = level_hs[l].post[pos-1];
	 			pos--;
	 		}
	 		level_hs[l].post[pos] = tmp;
	 	}
}

int
highscore::findLevel(std::string level) {
	for(int i=0;i<level_hs_length;i++) {
		if(level_hs[i].level.compare(level) == 0) {
			return i;
		}
	}
	return -1;
}

std::string
highscore::toOutputFormat(std::string str) {
	while(str.find(" ") != std::string::npos) {
		str.replace(str.find(" "),1,"_");
	}
	return str;
}

std::string
highscore::fromOutputFormat(std::string str) {
	while(str.find("_") != std::string::npos) {
		str.replace(str.find("_"),1," ");
	}
	return str;
}

bool
highscore::saveData()
{
	if(!anyHighscore) {
		//std::cout << "No highscore to write, skipping"<<std::endl;
		return true;
	}
	
    char buff[256];

    if (get_config_dir_name( buff, 255 ) != 0) {
		return false;
    }
	
	std::string filename(buff);
	
	filename+="/highscore.dat";
	
	//std::cout <<"Writing highscore: "<<filename<<std::endl;
	
	std::ofstream sfile(filename.c_str());
	
	sfile << "ETRacer HIGHSCORE "<< version << std::endl;
	
	for(int n=0;n<level_hs_length;n++) {
		sfile << "<level>" << std::endl;
		sfile << toOutputFormat(level_hs[n].level) << std::endl;
		for(int i=0;i<level_hs[n].posts;i++){
			sfile << toOutputFormat(level_hs[n].post[i].nick) << " " << level_hs[n].post[i].score << std::endl;
		}
		sfile << "</level> 0" << std::endl;
	}
	sfile << "EOF" << std::endl;
	
	
	return true;
}

bool
highscore::loadData()
{
	char buff[256];

	if (get_config_dir_name( buff, 255 ) != 0) {
		return false;
	}
	
	std::string filename(buff);
	
	filename+="/highscore.dat";
	
	
	std::ifstream sfile(filename.c_str());
	
	if(!sfile) return false;
	
	int _version;
	int _posts;
	
			
	sfile >> buff >> buff >> _version;
	if(_version == version) {
		bool end=false;
		bool loop=false;
		while(!end) {
			memset(buff,256,'\0');
			sfile >> buff;
			if(std::string("<level>").compare(buff)==0) {
				int n;
				redim();
				n = level_hs_length-1;
				memset(buff,256,'\0');
				sfile >> buff;
				level_hs[n].level.assign(fromOutputFormat(buff));
				loop=true;
				for(_posts=0;loop&&_posts<10;_posts++) {
					char nick[256];
					memset(nick,256,'\0');
					int score;
					sfile >> nick >> score;
					if(std::string("</level>").compare(nick)!=0) {
						level_hs[n].post[_posts].nick.assign(fromOutputFormat(nick));
						level_hs[n].post[_posts].score = score;
					} else {
						_posts--;
						loop=false;
					}
				}
				level_hs[n].posts = _posts;
			} else if(std::string("EOF").compare(buff)==0) {
				end=true;
			}
		}
		anyHighscore=true;
		return true;
	} else {
		std::cout<<"Highscore file version doesnt match current!"<<std::endl;
		return false;
	}
}

int
highscore::posFromString(std::string level) {
	for(int i=0;i<level_hs_length;i++) {
		if(level_hs[i].level.compare(level) == 0) {
			return i;
		}
	}
	return -1;
}

void
highscore::printlist() {
	std::cout<<"== HIGHSCORE =="<<std::endl;
	for(int n=0;n<level_hs_length;n++) {
		std::cout<<"LEVEL: " << level_hs[n].level<<std::endl;
		for(int i=0;i<level_hs[n].posts;i++) {
			std::cout<<level_hs[n].post[i].nick << ": " << level_hs[n].post[i].score<<std::endl;
		}
		std::cout<<"-----------"<<std::endl;
	}
	std::cout<<"== EOF HIGHSCORE =="<<std::endl;
}


void
highscore::debug() {
	addScore("DEBUG","TUX_DEBUG",100);
	printlist();
}

void
highscore::redim() {
	//std::cout<<"Highscore: redimming..."<<std::endl;
	level_highscore *tmp = level_hs;
	level_hs = new level_highscore[++level_hs_length];

	if(tmp != NULL) {
		for(int i=0;i<level_hs_length-1;i++) {
			level_hs[i] = tmp[i];
		}
	}
}

std::list<std::string>
highscore::getCourseList() {
	std::list<std::string> tmp;
	for(int i=0;i<level_hs_length;i++) {
		tmp.push_back(level_hs[i].level);
	}
	return tmp;
}

std::list<HighscoreData>
highscore::getCourseList_hd() {
	std::list<HighscoreData> tmp;
	if(level_hs_length>0) {
		for(int i=0;i<level_hs_length;i++) {
			HighscoreData tmp2;
			tmp2.name = level_hs[i].level;
			tmp2.highscore = level_hs[i].level;
			tmp.push_back(tmp2);
		}
	} else {
		HighscoreData tmp2;
		tmp2.name = _("No records");
		tmp2.highscore = "";
		tmp.push_back(tmp2);
	}
	return tmp;
}

std::string
highscore::scorepost::toString() {
	char buff[40];
	snprintf(buff, 40, (nick+" (%d points)").c_str(), score );
	return std::string(buff);
}




/* class HighscoreShow
The graphic list displaying the highscore
*/

static int curCourse=0;

HighscoreShow::HighscoreShow()
 {
 /*
  : m_titleLbl(_("Highscore"),"heading"),
 m_raceListBox(450,36),
 m_backBtn(_("Back"))
 */
 	pp::Vec2d pos( getparam_x_resolution()/2,
				   getparam_y_resolution()/2+130);
	
	mp_titleLbl = new pp::Label(pos,"heading",_("Highscore"));
	mp_titleLbl->alignment.center();
	
	if(Highscore->level_hs_length>0) {
		std::list<std::string> courses=Highscore->getCourseList();
		m_raceList = Highscore->getCourseList_hd();
		
		pos.y-=60;
		
		mp_raceListBox = new pp::Listbox<HighscoreData>(pp::Vec2d(pos.x-200,pos.y),
					   pp::Vec2d(450,36),
					   "listbox_item",
					   m_raceList);
		
		mp_raceListBox->alignment.center();
		mp_raceListBox->signalChange.Connect(pp::CreateSlot(this,&HighscoreShow::updateStatus));
		
	 	curCourse=Highscore->posFromString((*courses.begin()));
	 	
	 	
	 	//curCourse=0;
		pos.y-=20;
		

	    
	     for(int i=0;i<10;i++) {
	          pos.y-=30;
	          char buff[50];
	          sprintf(buff, "%s - %s",highscore::posToStr(i+1).c_str(),Highscore->level_hs[curCourse].post[i].toString().c_str());
	      	mp_post[i] = new pp::Label(pos,"menu_label",buff);
			mp_post[i]->alignment.center();
	     }
     } else {
     	//No records stored
		pos.y-=60;
		m_raceList = Highscore->getCourseList_hd();
		mp_raceListBox = new pp::Listbox<HighscoreData>(pp::Vec2d(pos.x-200,pos.y),
					   pp::Vec2d(450,36),
					   "listbox_item",
					   m_raceList);
		
		mp_raceListBox->alignment.center();
		
	 	
	 	
	 	//curCourse=0;
		pos.y-=20;
		

	    
	     for(int i=0;i<10;i++) {
	          pos.y-=30;
	          char buff[50];
	          sprintf(buff, "");
	      	mp_post[i] = new pp::Label(pos,"menu_label",buff);
			mp_post[i]->alignment.center();
	     }
     }
     pos.y-=50;
     
     mp_backBtn = new pp::Button( pp::Vec2d(pos.x-50,pos.y),
			      pp::Vec2d(100, 40), 
			      "button_label", 
			      _("Back") );
	mp_backBtn->setHilitFontBinding( "button_label_hilit" );
	mp_backBtn->alignment.center();
     mp_backBtn->signalClicked.Connect(pp::CreateSlot(this,&HighscoreShow::back));
     
 }
 
bool
HighscoreShow::keyReleaseEvent(SDLKey key)
{
	switch (key){
		case SDLK_ESCAPE:
		case 'q':
			back();
	    	return true;
		default:
			return false;
	}	
}

void
HighscoreShow::back()
{
	set_game_mode( GAME_TYPE_SELECT );
	UIMgr.setDirty();
	
}

void
HighscoreShow::loop(float timeStep)
{
    update_audio();
    set_gl_options( GUI );
    clear_rendering_context();
    UIMgr.setupDisplay();
	drawSnow(timeStep);
    theme.drawMenuDecorations();
    UIMgr.draw();
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    winsys_swap_buffers();
}
  
void
HighscoreShow::updateStatus()
{	
	HighscoreData tmp2=(*mp_raceListBox->getCurrentItem());
	std::string tmp= tmp2.name;
	//std::cout<<"Selected "<<tmp<<std::endl;
	curCourse=Highscore->posFromString(tmp);
     for(int i=0;i<10;i++) {
     	char buff[50];
     	sprintf(buff,"%s - %s",highscore::posToStr(i+1).c_str(),Highscore->level_hs[curCourse].post[i].toString().c_str());
      	mp_post[i]->setText(buff);
     } 
}

HighscoreShow::~HighscoreShow() {
	delete mp_titleLbl;
	delete mp_raceListBox;
	delete mp_backBtn;
	for(int i=0;i<10;i++) {
		delete mp_post[i];
	}
}

