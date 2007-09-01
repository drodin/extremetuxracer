/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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
 */

#ifndef TEXTAREA_H
#define TEXTAREA_H

#include "widget.h"
#include "button.h"
#include "ui_theme.h"
#include "font.h"

#include <list>

namespace pp {
	
	
class Textarea : public Widget {

	double m_arrowWidth;
	double m_borderWidth;
	double m_textPad;
	double m_textRegionHeight;
    double m_textRegionWidth;
	
    pp::Font *mp_font;
	
    pp::Color m_borderColor;
    pp::Color m_backgroundColor;
    pp::Button *mp_upButton;
    pp::Button *mp_downButton;
    char *mp_textOrig;
    char *mp_textLines;
	std::list<char*> m_lines;
	std::list<char*>::iterator mi_topLine;
	
	void calcTextRegionDims();
	void updateButtonPositions();
	void createLines();
	void updateButtonEnabledStates();
	void drawTextLines();
	
public:
	Textarea( pp::Vec2d pos, pp::Vec2d size,
			     const char *binding, const char *text );
	~Textarea();

	void setText( const char *text );
	void setPosition(pp::Vec2d pos);

	void upButtonClick();
	void downButtonClick();

	void draw();
};
	
} //namespace pp

#endif // TEXTAREA_H
