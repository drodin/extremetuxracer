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

#ifndef LISTBOX_H
#define LISTBOX_H

#include "widget.h"
#include "button.h"

#include <list>

namespace pp {


template <class T>
class Listbox : public Widget {
	double m_arrowWidth;
    double m_borderWidth;
    double m_textPad;
    double m_arrowVertSeparation;

	pp::Font *mp_font;
	pp::Font *mp_insensitiveFont;
	
    pp::Color m_borderColor;
    pp::Color m_backgroundColor;
    pp::Button *mp_upButton;
    pp::Button *mp_downButton;
	
	typedef typename std::list<T> listType_t;
	typedef typename std::list<T>::iterator listIterator_t; 
	
	listType_t& m_items;
	listIterator_t mi_curItem;
	
	void updateButtonEnabledStates();
	void updateButtonPositions();

public:
	Listbox( pp::Vec2d pos, pp::Vec2d size, 
			   const char *binding, listType_t& itemList);	
	~Listbox();

	void setPosition(pp::Vec2d pos);
	void setCurrentItem( listIterator_t item );
	listIterator_t getCurrentItem(){return mi_curItem;};
	void setItemList( listType_t& itemList );
	listType_t& getItemList(){return m_items;};
	
	void setSensitive(bool sensitive);
		
	void setInsensitiveFont(const char* binding);

	bool gotoNextItem();
	bool gotoPrevItem();
	
	void upButtonClick();
	void downButtonClick();
	
	void draw();
	
	//signals
	pp::Signal0 signalChange;
};

} //namespace pp

#include "listbox.cpp"

#endif // LISTBOX_H
