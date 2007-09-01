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

#ifndef _PP_MODEL_AC_H
#define _PP_MODEL_AC_H


#include "alg/poly.h"
#include "alg/color.h"

#include <string>
#include <stdio.h>

namespace pp{
	
class Vertex 
{
public:
    pp::Vec3d vec;
    pp::Vec3d normal;
};

class UV
{
public:
	double u,v;	
};
	
class Surface : public pp::Polygon
{
public:
	Surface();
	~Surface();

 	pp::UV *uvs;
    pp::Vec3d normal;
    int flags;
    int mat;
};

class Material
{
 public:
	pp::Color diffuse; 
    pp::Color ambient;
    pp::Color specular;
    pp::Color emissive;
    double shininess;
    double transparency;
    std::string name;
};



class ModelObject
{
public:
	ModelObject();
	~ModelObject();
	
	pp::Vec3d loc;
    std::string name;
    std::string data;
    std::string url;
    pp::Vertex *vertices;
    int num_vert;

    pp::Surface *surfaces;
    int num_surf;
    double texture_repeat_x, texture_repeat_y;
    double texture_offset_x, texture_offset_y;

    int num_kids;
    pp::ModelObject **kids;
    double matrix[9];
    int type;
    int texture;
};

class ModelAC
{
	pp::ModelObject* mp_model;

	int m_tokc;
	char* ma_tokv[30];
	
	int m_line;
	char ma_buff[255];

	pp::Material ma_palette[255];
	int m_numPalette;
	int m_startMatIndex;
		
	int loadTexture(const char *fileName);
	void prepareRender();	
	void render(ModelObject *ob);
	void setColor(long matno);
	void setSimpleColor(long matno);
	int stringToObjectType(std::string& string);
	ModelObject* loadObject(FILE *f, ModelObject *parent);
	void objectCalculateVertexNormals(ModelObject *ob);
	void calculateVertexNormals(ModelObject *ob);
	int getTokens(char *s, int *argc, char *argv[]);
	pp::Material* getMaterialFromPalette(int id);
	bool readLine(FILE *f);
	Surface* readSurface(FILE *f, Surface *s, ModelObject *ob);	
	void CalculateTriNormal(pp::Vec3d *v1, pp::Vec3d *v2, pp::Vec3d *v3, pp::Vec3d *n);
		
public:
	ModelAC(const char *fileName);

	pp::ModelObject* getModel(){return mp_model;}
	int getDisplayList();

};
//pp::ModelObject *ac_load_ac3d(const char *filename);
//int ac_display_list_render_object(pp::ModelObject *ob);

} //namepsace pp

#endif // _PP_MODEL_AC_H
