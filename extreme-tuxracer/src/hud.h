/* 
 * Tux Racer 
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

#ifndef _HUD_H_
#define _HUD_H_

#include "player.h"
#include "textures.h"
#include "ppgltk/font.h"

#define HUD_MAX_ITEMS 16

class HUD
{
public:
	HUD(void);

	struct Element{
		Element();
		int type;
		int x,y;
		pp::Font *font;
		GLuint texture;
		int width,height,size;
		std::string string;
		wchar_t u_string[16];
		int angle;
	};	

	void draw(Player& plyr);
	void reset(void);
	bool add(Element newElement);			
	bool update(const int i, Element newElement);
		

private:
		
	Element m_element[HUD_MAX_ITEMS];
	int m_numElements;
	
	void text(const int i);
	void fps(const int i);
	void herring(const int i, const int herring_count);
	void image(const int i);
	void time(const int i);
	void speed(const int i, const double speed);
	void gauge(const int i, const double speed, const double energy);
	void bar(const int i, double percentage);
	void coursePercentage(const int i);
		
	void draw_partial_tri_fan(const double fraction);
	pp::Vec2d calc_new_fan_pt(const double angle);
	void start_tri_fan(void);

	void fix_xy(int &x, int &y, const int asc=0, const int width=0);
	void initGauge();
	
	GLuint m_energymaskTex;
	GLuint m_speedmaskTex;
	GLuint m_outlineTex;	
};

extern HUD HUD1;


#endif // _HUD_H_
