/*
 * PPGLTK
 *
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

#ifndef _PP_COLOR_H
#define _PP_COLOR_H

namespace pp {

/// Color
class Color
{
public:
	/// red
	double r;
	/// green
	double g;
	/// blue
	double b;
	/// alpha
	double a;

	Color(void) : r(0.0), g(0.0), b(0.0), a(1.0){};
	Color(const double red, const double green, const double blue, const double alpha=1.0);	
	Color(const double *color);
		
	static const pp::Color black;
	static const pp::Color white;
};

} //namespace pp


#endif // _PP_COLOR_H
