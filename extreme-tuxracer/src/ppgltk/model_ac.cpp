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
 
/*
 * This code is based on the example loader from www.ac3d.org
 */


#include "model_ac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
	#include <WTypes.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "../textures.h"

namespace pp{
	
#define OBJECT_WORLD 999
#define OBJECT_NORMAL 0
#define OBJECT_GROUP 1
#define OBJECT_LIGHT 2

#define SURFACE_SHADED (1<<4)
#define SURFACE_TWOSIDED (1<<5)

#define SURFACE_TYPE_POLYGON (0)
#define SURFACE_TYPE_CLOSEDLINE (1)
#define SURFACE_TYPE_LINE (2)

Surface::Surface()
 : uvs(NULL),
   flags(0),
   mat(0)
{
}
	
Surface::~Surface()
{
	//todo: check if we need to free something	
}

ModelObject::ModelObject()
{
    loc.x = loc.y = loc.z = 0.0;
    vertices = NULL;
    num_vert = 0;
    surfaces = NULL;
    num_surf = 0;
    texture = -1;
    texture_repeat_x = texture_repeat_y = 1.0;
    texture_offset_x = texture_offset_y = 0.0;
    kids = NULL;
    num_kids = 0;
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 1;
    matrix[5] = 0;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 1;
}
	
ModelObject::~ModelObject()
{
	for (int i = 0; i < num_kids; i++){
		delete kids[i];
	}

	if (vertices)
		free(vertices);

	for (int i = 0; i < num_surf; i++){
		delete(surfaces[i].vertices);
		delete(surfaces[i].uvs);
	}

	if (surfaces)
		delete(surfaces);
}

ModelAC::ModelAC(const char *fileName)
 : mp_model(NULL),
   m_tokc(0),
   m_line(0),
   m_numPalette(0),
   m_startMatIndex(0)
{
    FILE *f = fopen(fileName, "r");
   
    if (f == NULL){
		printf("can't open %s\n", fileName);
		return;
    }

    readLine(f);

    if (strncmp(ma_buff, "AC3D", 4)){
		printf("ac_load_ac '%s' is not a valid AC3D file.", fileName);
		fclose(f);
		return;
    }
    m_startMatIndex = m_numPalette;
    mp_model = loadObject(f, NULL);
    fclose(f);
    calculateVertexNormals(mp_model);	
}
	
int
ModelAC::getDisplayList()
{
	int list;
    list = glGenLists(1);
    glNewList(list,GL_COMPILE);
	render(mp_model);
    glEndList();
    return(list);
}

int
ModelAC::loadTexture(const char *fileName)
{
    GLuint texid;
	std::string binding("ac/");
	binding+=fileName;
	
	if(!get_texture_binding( binding.c_str(), &texid )){
		load_and_bind_texture(binding.c_str(),fileName);
		get_texture_binding( binding.c_str(), &texid );
	}
	
	return texid;	
}

void
ModelAC::prepareRender()
{

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glDisable( GL_COLOR_MATERIAL ); 

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void
ModelAC::render(ModelObject *ob)
{
    int n, s, sr;
    int st;

    glPushMatrix();

    glTranslated(ob->loc.x, ob->loc.y, ob->loc.z);

    if (ob->texture != -1){
		static int lasttextureset = -1;
 
		glEnable(GL_TEXTURE_2D);

		if (ob->texture != lasttextureset){
			glBindTexture(GL_TEXTURE_2D, ob->texture);
			lasttextureset = ob->texture;
		}
	} else {
        glDisable(GL_TEXTURE_2D);
	}

    for (s = 0; s < ob->num_surf; s++){
		Surface *surf = &ob->surfaces[s];

		glNormal3dv((double*)&surf->normal);

		if (surf->flags & SURFACE_TWOSIDED){
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
		}
		
		st = surf->flags & 0xf;
		if (st == SURFACE_TYPE_CLOSEDLINE){
			glDisable(GL_LIGHTING);

			glBegin(GL_LINE_LOOP);
			setSimpleColor(surf->mat);
		} else if (st == SURFACE_TYPE_LINE){
			glDisable(GL_LIGHTING);

			glBegin(GL_LINE_STRIP);
			setSimpleColor(surf->mat);
		} else {
			glEnable(GL_LIGHTING);
			setColor(surf->mat); 
			if (surf->numVertices == 3){
				glBegin(GL_TRIANGLE_STRIP);
			} else {
				glBegin(GL_POLYGON);
			}
		}

		for (sr = 0; sr < surf->numVertices; sr++){
			pp::Vertex *v = &ob->vertices[surf->vertices[sr]];

			if (ob->texture > -1){
				double tu = surf->uvs[sr].u;
				double tv = surf->uvs[sr].v;

				double tx = ob->texture_offset_x + tu * ob->texture_repeat_x;
				double ty = ob->texture_offset_y + tv * ob->texture_repeat_y;

				glTexCoord2f(tx, ty);
			}

			if (surf->flags & SURFACE_SHADED){
				glNormal3dv((double *)&v->normal);
			}
			glVertex3dv((double *)v);
		}
		glEnd();
    }

    if (ob->num_kids){
		for (n = 0; n < ob->num_kids; n++){
	    	render(ob->kids[n]);
		}
	}
	
	glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void
ModelAC::setColor(long matno)
{
    Material *m = getMaterialFromPalette(matno);
    float rgba[4];
    static int lastcolset = -1;

    if (lastcolset == matno)
		return;
    else
	lastcolset = matno;

    rgba[0] = m->diffuse.r;
	rgba[1] = m->diffuse.g;
	rgba[2] = m->diffuse.b;
    rgba[3] = 1.0-m->transparency;
    
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rgba);
	
    if ( (1.0-m->transparency) < 1.0)
	    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
		}
    else
        glDisable(GL_BLEND);
}

void
ModelAC::setSimpleColor(long matno)
{
    Material *m = getMaterialFromPalette(matno);
    glColor3dv((double*)&m->diffuse);
}

int
ModelAC::stringToObjectType(std::string& string)
{
    if (string=="world")
        return(OBJECT_WORLD);
    if (string=="poly")
        return(OBJECT_NORMAL);
    if (string=="group")
        return(OBJECT_GROUP);
    if (string=="light")
        return(OBJECT_LIGHT);
    return(OBJECT_NORMAL);
}

ModelObject*
ModelAC::loadObject(FILE *f, ModelObject *parent)
{
    char t[20];
	std::string type;
    ModelObject *ob = NULL;

    while (!feof(f))
    {
	readLine(f);

	sscanf(ma_buff, "%s", t);
	type = t;

	if (type=="MATERIAL"){
        double shi, tran;
        Material m;
	    
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 22){
            printf("expected 21 params after \"MATERIAL\" - line %d\n", m_line);
	    } else {
			m.name = ma_tokv[1];
			m.diffuse.r = atof(ma_tokv[3]);
			m.diffuse.g = atof(ma_tokv[4]);
			m.diffuse.b = atof(ma_tokv[5]);

			m.ambient.r = atof(ma_tokv[7]);
			m.ambient.g = atof(ma_tokv[8]);
			m.ambient.b = atof(ma_tokv[9]);

			m.emissive.r = atof(ma_tokv[11]);
			m.emissive.g = atof(ma_tokv[12]);
			m.emissive.b = atof(ma_tokv[13]);

			m.specular.r = atof(ma_tokv[15]);
			m.specular.g = atof(ma_tokv[16]);
			m.specular.b = atof(ma_tokv[17]);

			m.shininess = atof(ma_tokv[19]);
			m.transparency = atof(ma_tokv[21]);
	
			shi = atof(ma_tokv[6]);
			tran = atof(ma_tokv[7]);

			ma_palette[m_numPalette++] = m;
	    }
	} else if (type=="OBJECT"){
		char type[20];
		char str[20];
		ob = new ModelObject;

		sscanf(ma_buff, "%s %s", str, type);
		
		std::string t(type);
		
		ob->type = stringToObjectType(t);
	} else if (type=="data"){
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 2){
			printf("expected 'data <number>' at line %d\n", m_line);
		} else {
			char *str;
			int len;

			len = atoi(ma_tokv[1]);
			if (len > 0){
			    str = (char *)malloc(len+1);
			    fread(str, len, 1, f);
			    str[len] = 0;
			    fscanf(f, "\n"); m_line++;
			    ob->data = str;
			    free(str);
			}
		}
	} else if (type=="name"){
		int numtok = getTokens(ma_buff, &m_tokc, ma_tokv);
		if (numtok != 2){
			printf("expected quoted name at line %d (got %d tokens)\n", m_line, numtok);
		} else {
			ob->name = ma_tokv[1];
		}
	} else if (type=="texture"){
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 2){
			printf("expected quoted texture name at line %d\n", m_line);
		} else {
			ob->texture = loadTexture(ma_tokv[1]);
		}
	} else if (type=="texrep"){
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 3){
			printf("expected 'texrep <float> <float>' at line %d\n", m_line);
		} else {
		    ob->texture_repeat_x = atof(ma_tokv[1]);
		    ob->texture_repeat_y = atof(ma_tokv[2]);
		}
	} else if (type=="texoff"){
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 3){
			printf("expected 'texoff <float> <float>' at line %d\n", m_line);
		} else {
			ob->texture_offset_x = atof(ma_tokv[1]);
			ob->texture_offset_y = atof(ma_tokv[2]);
		}
	} else if (type=="rot"){
		double r[9];
		char str2[5];
		int n;

		sscanf(ma_buff, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf", str2, 
				       &r[0], &r[1], &r[2], &r[3], &r[4], &r[5], &r[6], &r[7], &r[8] );

		for (n = 0; n < 9; n++){
			ob->matrix[n] = r[n];
		}
	} else if (type=="loc"){
		char str[5];
		sscanf(ma_buff, "%s %lf %lf %lf", str,
			   &ob->loc.x, &ob->loc.y, &ob->loc.z);			
	} else if (type=="url"){
		if (getTokens(ma_buff, &m_tokc, ma_tokv) != 2){
		    printf("expected one arg to url at line %d (got %s)\n", m_line, ma_tokv[0]);
		} else {
			ob->url = ma_tokv[1];
		}
	} else if (type=="numvert"){
		int num, n;
		char str[10];

		sscanf(ma_buff, "%s %d", str, &num);

		if (num > 0){
			ob->num_vert = num;
			ob->vertices = new pp::Vertex[num];
			for (n = 0; n < num; n++){
				pp::Vertex p;
				fscanf(f, "%lf %lf %lf\n", &p.vec.x, &p.vec.y, &p.vec.z); m_line++;
				ob->vertices[n] = p;
			}

		}
	} else if (type=="numsurf"){
		int num, n;
		char str[10];

		sscanf(ma_buff, "%s %d", str, &num);
		if (num > 0){
			ob->num_surf = num;
			ob->surfaces = new Surface[num];

			for (n = 0; n < num; n++){
				Surface *news = readSurface(f, &ob->surfaces[n], ob);
				if (news == NULL){
					printf("error whilst reading surface at line: %d\n", m_line);
					return(NULL);
				}
			}
		}
	} else if (type=="kids"){ 
		int num, n;

		sscanf(ma_buff, "%s %d", t, &num);
			
		if (num != 0){
			ob->kids = new ModelObject*[num];
			ob->num_kids = num;

			for (n = 0; n < num; n++){
				ModelObject *k = loadObject(f, ob);

				if (k == NULL){
					printf("error reading expected child object %d of %d at line: %d\n", n+1, num, m_line);
					return(ob);
				} else {
					ob->kids[n] = k;
				}
			}
	    }
	    return(ob);
	}

    }
    return(ob);
}

void
ModelAC::objectCalculateVertexNormals(ModelObject *ob)
{
    int s, v, vr;

    /** for each vertex in this object **/
    for (v = 0; v < ob->num_vert; v++)
    {
	pp::Vec3d n;
	int found = 0;

	/** go through each surface **/
	for (s = 0; s < ob->num_surf; s++)
	{
	    Surface *surf = &ob->surfaces[s];

	    /** check if this vertex is used in this surface **/
	    /** if it is, use it to create an average normal **/
	    for (vr = 0; vr < surf->numVertices; vr++)
		if (surf->vertices[vr] == v)
		{
		    n.x+=surf->normal.x;
		    n.y+=surf->normal.y;
		    n.z+=surf->normal.z;
		    found++;
		}
	}
	if (found > 0)
	{
	    n.x /= found;
	    n.y /= found;
	    n.z /= found;
	}
	ob->vertices[v].normal = n;
    }
}

void
ModelAC::calculateVertexNormals(ModelObject *ob)
{
    int n;
    objectCalculateVertexNormals(ob);
    if (ob->num_kids)
	for (n = 0; n < ob->num_kids; n++)
	    calculateVertexNormals(ob->kids[n]);
}

int
ModelAC::getTokens(char *s, int *argc, char *argv[])
/** bung '\0' chars at the end of tokens and set up the array (tokv) and count (tokc)
	like argv argc **/
{
    char *p = s;
    char *st;
    char c;
    //int n;
    int tc;

    tc = 0;
    while ((c=*p) != 0)
    {
	if ((c != ' ') && (c != '\t') && (c != '\n') && ( c != 13))
	{
	    if (c == '"')
	    {
		c = *p++;
		st = p;
		while ((c = *p) && ((c != '"')&&(c != '\n')&& ( c != 13)) )
		{
		    if (c == '\\')
			strcpy(p, p+1);
		    p++;
		}
		*p=0;
		argv[tc++] = st;
	    }
	    else
	    {
		st = p;
		while ((c = *p) && ((c != ' ') && (c != '\t') && (c != '\n') && ( c != 13)) )
		    p++;
		*p=0;
		argv[tc++] = st;
	    }			
	}
	p++;
    }

    *argc = tc;
    return(tc);
}

pp::Material*
ModelAC::getMaterialFromPalette(int id)
{
    return(&ma_palette[id]);
}

bool
ModelAC::readLine(FILE *f)
{
    fgets(ma_buff, 255, f); m_line++;
    return(true);
}

Surface*
ModelAC::readSurface(FILE *f, Surface *s, ModelObject *ob)
{
    char t[20];
	std::string type;
	
    while (!feof(f))
    {
	readLine(f);
	sscanf(ma_buff, "%s", t);
	type = t;
		
	if (type=="SURF"){
	    int flgs;

	    if (getTokens(ma_buff, &m_tokc, ma_tokv) != 2){
			printf("SURF should be followed by one flags argument\n");
        } else {
			flgs = strtol(ma_tokv[1], NULL, 0);
			s->flags = flgs;
		}
	} else if (type=="mat"){
		int mindx;
		sscanf(ma_buff, "%s %d", t, &mindx);
		s->mat = mindx + m_startMatIndex;
	} else if (type=="refs"){
		int num, n;
		int ind;
		double tx, ty;
  
		sscanf(ma_buff, "%s %d", t, &num);        

		s->numVertices = num;
		s->vertices = new int[num]; //(int *)malloc( num * sizeof(int));
		s->uvs = new pp::UV[num];

		for (n = 0; n < num; n++){
			fscanf(f, "%d %lf %lf\n", &ind, &tx, &ty); m_line++;
			s->vertices[n] = ind;
			s->uvs[n].u = tx;
			s->uvs[n].v = ty;
		}

		// calc surface normal
		if (s->numVertices >= 3)
		CalculateTriNormal(&ob->vertices[s->vertices[0]].vec, 
				&ob->vertices[s->vertices[1]].vec, 
				&ob->vertices[s->vertices[2]].vec,
				&s->normal);

		return(s);
	} else {
		printf("ignoring %s\n", t);
	}
	
    }
    return(NULL);
}

void
ModelAC::CalculateTriNormal(pp::Vec3d *v1, pp::Vec3d *v2, pp::Vec3d *v3, pp::Vec3d *n)
{
    double len;

    n->x = (v2->y-v1->y)*(v3->z-v1->z)-(v3->y-v1->y)*(v2->z-v1->z);
    n->y = (v2->z-v1->z)*(v3->x-v1->x)-(v3->z-v1->z)*(v2->x-v1->x);
    n->z = (v2->x-v1->x)*(v3->y-v1->y)-(v3->x-v1->x)*(v2->y-v1->y);
    len = sqrt(n->x*n->x + n->y*n->y + n->z*n->z);

    if (len > 0)
    {
		n->x /= len;
		n->y /= len;
		n->z /= len;  
    }
}

} //namespace pp
