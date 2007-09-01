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

#include "fps.h"

#include "game_mgr.h"
#include "bench.h"

FPS fpsCounter;

FPS::FPS() 
 : m_frames(0), 
   m_time(0.0),
   m_fps(0.0)
{
}

void
FPS::update()
{
    m_time += gameMgr->getTimeStep();
	m_frames++;
	if( m_time>0.3){
		m_fps = m_frames / m_time;
		m_time = 0.0;
		m_frames = 0;
		
		if( Benchmark::getMode()!= Benchmark::NONE ){
			Benchmark::updateFPS(m_fps);
		}
	}
}
