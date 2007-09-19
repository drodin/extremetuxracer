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
 
#ifndef _GRAPHICS_CONFIG_H
#define _GRAPHICS_CONFIG_H

#include "configmode.h"
#include "model_hndl.h"

#include "ppgltk/checkbox.h"
#include "ppgltk/listbox.h"


class GraphicsConfig : public ConfigMode
{
	pp::CheckBox* mp_uiSnowBox;
	pp::CheckBox* mp_fpsBox;
	pp::CheckBox* mp_coursePercentageBox;
	pp::CheckBox* mp_fogBox;
	pp::CheckBox* mp_reflectionsBox;
	pp::CheckBox* mp_shadowsBox;
	
	pp::Listbox<language_t>* mp_langListBox;
	pp::Listbox<model_t>* mp_modelListBox;
	
	std::list<language_t> m_langList;
	std::list<model_t> m_modelList;
public:
	GraphicsConfig();
	~GraphicsConfig();

	void setWidgetPositions();
	void apply();
};

#endif // GRAPHICS_CONFIG_H
