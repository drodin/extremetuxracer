/* 
 * ETRacer 
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

#include "ui_theme.h"
#include "game_config.h"

#include "ppgltk/alg/glhelper.h"

pp::Theme theme;

namespace pp {
	
Theme::Theme() : 
	background(0.5, 0.6, 0.9),
	foreground(1.0, 1.0, 1.0), 
	highlight(1.0, 0.9, 0.0),
	disabled(1.0, 1.0, 1.0, 1.0),
	insensitive(1.0, 1.0, 1.0, 0.5),
	focus(1.0, 0.89, 0.01, 1.0),
	border(4,4),
	textPadding(8,8)
{	
}

void
Theme::drawMenuDecorations()
{
	int width = getparam_x_resolution();
	int height = getparam_y_resolution();
	pp::Vec2d size(255,255);
	
	ppGL::draw::rect("menu_bottom_left",pp::Vec2d(0,0), size);
	ppGL::draw::rect("menu_bottom_right",pp::Vec2d(width-256,0), size);
	ppGL::draw::rect("menu_top_left",pp::Vec2d(0,height-256), size);
	ppGL::draw::rect("menu_top_right",pp::Vec2d(width-256,height-256), size);
	if(height>760) {
		ppGL::draw::rect("menu_title",pp::Vec2d(width/2-256, height-256),pp::Vec2d(512,256));
	} else {
		ppGL::draw::rect("menu_title_small",pp::Vec2d(width/2-256, height-128),pp::Vec2d(512,128));
	}
}

} //namespace pp
