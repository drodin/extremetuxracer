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

#ifndef _PP_MODEL_H
#define _PP_MODEL_H

//#include "model_ac.h"

#include "alg/poly.h"

namespace pp{
	
	
class ModelObject;
class ModelAC;
	

class Model{
	int m_displayList;
	pp::ModelAC* mp_model;
	void copyObject(pp::Polyhedron* ph, pp::ModelObject* object);
		
public:
	Model(const char* filename);
	~Model();

	void draw();

	///returns a Polyhedron tree of the ModelObject Tree
	pp::Polyhedron* getPolyhedron();

};

} // namespace pp


#endif // _PP_MODEL_H
