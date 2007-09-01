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
 
#ifndef _PP_BUTTON_H
#define _PP_BUTTON_H

#include "widget.h"

#include "alg/color.h"
#include "font.h"

namespace pp {
	
class Button : public Widget {
	typedef struct {
		/// name of texture binding
		const char *binding;
		/// color to use when drawing texture
		pp::Color color; 
		/// lower left
		pp::Vec2d ll;
		/// upper right
		pp::Vec2d ur;  
	} texture_region_t;

    texture_region_t m_tex;
    texture_region_t m_hilitTex;
    texture_region_t m_clickedTex;
    texture_region_t m_disabledTex;
    	
	pp::Font *mp_font;
	pp::Font *mp_hilitFont;
	pp::Font *mp_disabledFont;
	
    const char *mp_label;

public:
	Button( pp::Vec2d pos, pp::Vec2d size,
			const char *binding, const char *label );

	void setHighlight(bool highlight);
	void setHilitFontBinding( const char *binding );
	
	void setDisabledFontBinding( const char *binding );
	void setImage(const char *binding,
		       pp::Vec2d p0, pp::Vec2d p1, pp::Color color);
	void setDisabledImage(const char *binding,
				pp::Vec2d p0, pp::Vec2d p1, pp::Color color );
	void setHilitImage(const char *binding,
			     pp::Vec2d p0, pp::Vec2d p1, pp::Color color );	
	void setClickedImage(const char *binding,
			       pp::Vec2d p0, pp::Vec2d p1, pp::Color color );

	void draw();
};
	
} //namespace pp

#endif // _PP_BUTTON_H
