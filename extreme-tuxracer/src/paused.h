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

#ifndef _PAUSED_H_
#define _PAUSED_H_

#include "loop.h"

#include "ppgltk/frame.h"
#include "ppgltk/button.h"
#include "ppgltk/label.h"

class Paused : public GameMode
{
	pp::Frame *mp_backgroundFrm;
	pp::Button *mp_configBtn;
	pp::Button *mp_resumeBtn;
	pp::Button *mp_quitBtn;
	pp::Label *mp_pausedLbl;
	
public:
	Paused();
	~Paused();
	
	void loop(float timeStep);
	bool keyPressEvent(SDLKey key);

	void resume();
	void quit();
	void configuration();
};

#endif // _PAUSED_H_
