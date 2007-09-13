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

#ifndef _GAME_TYPE_SELECT_H_
#define _GAME_TYPE_SELECT_H_

#include "loop.h"
#include "ppgltk/button.h"


class GameTypeSelect : public GameMode
{
	pp::Button* mp_enterEventBtn;
	pp::Button* mp_practiceBtn;
	pp::Button* mp_configureBtn;
	pp::Button* mp_creditsBtn;
	pp::Button* mp_quitBtn;
	pp::Button* mp_highscoreBtn;
	
	void setWidgetPositions();
	
public:
	GameTypeSelect();
	~GameTypeSelect();

	void loop(float timeStep);
	bool keyPressEvent(SDLKey key);

	void eventSelect();
	void credits();
	void practice();
	void highscore();
	void configuration();
	void quit();
};

#endif // _GAME_TYPE_SELECT_H_
