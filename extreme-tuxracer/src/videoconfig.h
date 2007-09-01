/* 
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
 
#ifndef _VIDEO_CONFIG_H
#define _VIDEO_CONFIG_H

#include "configmode.h"

#include "ppgltk/listbox.h"
#include "ppgltk/checkbox.h"


typedef struct{
	std::string name;
	int x;
	int y;
}resolution_t;

typedef struct{
	std::string name;
	int data;
}bpp_t, multisample_t;


class VideoConfig : public ConfigMode
{
	pp::Listbox<resolution_t>* mp_resolutionListbox;
	pp::Listbox<bpp_t>* mp_bppListbox;
	pp::Listbox<multisample_t>* mp_multisampleListbox;
	
	std::list<resolution_t> m_resolutionList;
	std::list<bpp_t> m_bppList;
	std::list<multisample_t> m_multisampleList;

	pp::CheckBox* mp_fullscreenBox;
	pp::CheckBox* mp_stencilBox;
	pp::CheckBox* mp_fsaaBox;
	
	void initVideoModes();
		
public:
	VideoConfig();
	~VideoConfig();

	void setWidgetPositions();
	void apply();
};


#endif // _VIDEO_CONFIG_H
