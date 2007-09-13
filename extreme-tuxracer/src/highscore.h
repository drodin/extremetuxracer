/* 
 * PlanetPenguin Racer 
 * Copyright (C) 2004-2006 Volker Stroebel <volker@planetpenguin.de>
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


#ifndef _HIGHSCORE_H_
#define _HIGHSCORE_H_

#include <fstream>
#include <string>

#include "loop.h"
#include "ppgltk/ui_mgr.h"
#include "ppgltk/label.h"
#include "ppgltk/listbox.h"
#include "ppgltk/button.h"

typedef struct{
	std::string highscore; 
	std::string name;
}HighscoreData;

class highscore
{	
	private:
	static const int version = 2; //Highscore file version
	int findLevel(std::string level);
	void sort(int level);
	
	bool anyHighscore;
	
	std::string toOutputFormat(std::string str);
	std::string fromOutputFormat(std::string str);
	
 	public:
	highscore();
	
	static bool useHighscore;
	static bool hsDebug;
	
	struct
	scorepost {
		std::string nick;
		int score;
		scorepost() {
			nick="";
			score = 0;
		}
		std::string toString();
	};
	
	struct
	level_highscore {
		std::string level;
		scorepost post[10];
		int posts;
		level_highscore() {
			level="";
			posts=0;
		};
		
		
	};
	
	level_highscore *level_hs;
	int level_hs_length;
	
	int addScore(std::string level,std::string nick,int score);
	int posFromString(std::string level);
	std::list<std::string> getCourseList();
	std::list<HighscoreData> getCourseList_hd();
	static std::string posToStr(int pos);
	
	bool saveData();
	bool loadData();
	void debug();
	void printlist();
	void redim();
};

extern highscore* Highscore;

class HighscoreShow
 : public GameMode
{
	pp::Listbox<HighscoreData>* mp_raceListBox;
	std::list<HighscoreData> m_raceList;
	
	pp::Label* mp_titleLbl;
	pp::Label* mp_post[10];
	pp::Button* mp_backBtn;
	
public:
	HighscoreShow();
	~HighscoreShow();

	void loop(float timeStep);

	bool keyReleaseEvent(SDLKey key);

	void back();
	void updateStatus();
};

#endif // _HIGHSCORE_H_
