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

#ifndef _GAME_OVER_H_
#define _GAME_OVER_H_

#include "loop.h"

#include "ppgltk/label.h"


class GameOver : public GameMode
{
	bool m_aborted;
	bool m_bestScore;
	
	pp::Label *mp_raceOverLbl;
	
	pp::Label *mp_timeLbl;
	pp::Label *mp_herringLbl;
	pp::Label *mp_scoreLbl;
	pp::Label *mp_maxspeedLbl;
	pp::Label *mp_flyingLbl;
	pp::Label *mp_highscoreLbl;
	
	pp::Label *mp_resultsLbl;
	
public:
	GameOver();
	~GameOver();

	void loop(float timeStep);

	bool keyPressEvent(SDLKey key);
	bool mouseButtonEvent(int button, int x, int y, bool pressed);
};

#endif
