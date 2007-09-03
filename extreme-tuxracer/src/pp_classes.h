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

#ifndef _PP_CLASSES_H_
#define _PP_CLASSES_H_

#include "pp_types.h"
#include "etracer.h"

#include "ppgltk/model.h"


class BaseItem {
public:
	ray_t ray;
	double height;
    double diam;
    int type;
};

class Item : public BaseItem {
public:

	enum Type{
		UNCOLLECTABLE,
		HERRING,
		LIFE	
	};
	
private:	
	bool m_drawable;
	bool m_collected;
	Type m_type;
	int m_score;

public:
	Item():m_collected(false){};

	inline bool isDrawable(){ return m_drawable; };
	inline void setDrawable(const bool state=true){ m_drawable=state; };

	inline int getScore(){return m_score;}
	inline void setScore(const int score){m_score=score;}
	
	inline Type getType(){return m_type;};
	inline void setType(Type type){m_type=type;};
	
	inline void setCollected(bool collected=true){m_collected=collected;};
	inline bool isCollected(){return m_collected;};
};

class Tree : public BaseItem {
	pp::Model* mp_model;
	pp::Polyhedron* mp_ph;
	
public:
	inline void setPolyhedron(pp::Polyhedron* ph){mp_ph = ph;};
	inline pp::Polyhedron* getPolyhedron(){return mp_ph;};
	inline void setModel(pp::Model* model){mp_model = model;};
	inline pp::Model* getModel(){return mp_model;}	
};

#endif // PP_CLASSES_H_
