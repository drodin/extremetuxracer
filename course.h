/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef COURSE_H
#define COURSE_H

#include "bh.h"

#define FLOATVAL(i) (*(GLfloat*)(vnc_array+idx+(i)*sizeof(GLfloat)))
#define BYTEVAL(i) (*(GLubyte*)(vnc_array+idx+8*sizeof(GLfloat) +\
    i*sizeof(GLubyte)))
#define STRIDE_GL_ARRAY (8 * sizeof(GLfloat) + 4 * sizeof(GLubyte))
#define ELEV(x,y) (elevation[(x) + nx*(y)] )
#define NORM_INTERPOL 0.05		
#define XCD(x) ((double)(x) / (nx-1.0) * width)
#define ZCD(y) (-(double)(y) / (ny-1.0) * length)
#define NMLPOINT(x,y) MakeVector (XCD(x), ELEV(x,y), ZCD(y) )


#define MAX_COURSES 164
#define MAX_TERR_TYPES 64
#define MAX_COLL 12192
#define MAX_NOCOLL 8192
#define MAX_OBJECTS 16000
#define MAX_OBJECT_TYPES 128
#define MAX_POLY 128
#define MAX_DESCRIPTION_LINES 8

typedef struct {
	string name;
	string dir;
	string desc[MAX_DESCRIPTION_LINES];
	int num_lines;
	GLuint preview;
	double width;
	double length;
	double play_width;
	double play_length;
	double angle;
	double scale;
	double startx;
	double starty;
	int env;
	int music_theme;
} TCourse;					   

typedef struct {
	string texture;
	string sound;
	GLuint	texid;
	TColor3 col;
	
	double friction;
	double depth;
	int vol_type;
	int particles;
	int trackmarks;
	int starttex;
	int tracktex;
	int stoptex;
	bool shiny;
} TTerrType;

typedef struct {
	string		name;
	string		texture;
	GLuint 		texid;
	bool		collidable;
    double		above_ground;
    int			collectable;
    bool		drawable;
    bool		reset_point;
    bool		use_normal;
    TVector3	normal;			
    int  		num_objects;	
    int			num_trees;
    int			num_items;
    int			poly;
} TObjectType;
			
class CCourse {
private:
	int			curr_course;
	string		CourseIndex;
	string		ObjectIndex;
	int			numObjects;
	string		PolyIndex;
	string		CourseDir;
	bool		SaveItemsFlag;

	int    		nx;
	int			ny;
	double 		width;
	double		length;
	double  	play_width;
	double		play_length;
	double 		course_angle;
	double		course_descent;
	double		elev_scale;
	TVector2	start_pt;
	int			base_height_value;
	int			env;
	int			music_theme;

	void 		ResetCourse ();
	void 		FreeTerrainTextures ();
	void 		FreeObjectTextures ();
	void 		CalcNormals ();
	void 		MakeCourseNormals ();
	bool 		LoadElevMap ();
	void 		LoadItemList ();
	bool 		LoadObjectMap ();
	bool 		LoadTerrainMap ();
	int  		GetTerrain (unsigned char pixel[]);

	void		MirrorCourseData ();
public:
	CCourse ();

	TCourse		CourseList[MAX_COURSES];
	TTerrType	TerrList[MAX_TERR_TYPES];
	TObjectType	ObjTypes[MAX_OBJECT_TYPES];
	TCollidable	CollArr[MAX_COLL];
	TItem		NocollArr [MAX_NOCOLL];

	int			numCourses;
	int			numTerr;
	int			numObjTypes;
	int         numColl;
	int         numNocoll;
	TPolyhedron	PolyArr[MAX_POLY];
	int			numPolys;

	char		*terrain;
	double		*elevation;
	TVector3 	*nmls;
	GLubyte		*vnc_array;

 	int  GetCourseIdx (string dir);
	bool LoadCourseList ();
	bool LoadCourse (int idx);
	bool LoadTerrainTypes ();
	bool LoadObjectTypes ();
	void MakeStandardPolyhedrons ();
	void GetGLArrays (GLubyte **vnc_array);
	void FillGlArrays();

	void GetDimensions (double *w, double *l);
	void GetPlayDimensions (double *pw, double *pl);
	void GetDivisions (int *nx, int *ny);
	double GetCourseAngle ();
	double GetCourseDescent ();
	double GetBaseHeight (double distance);
	double GetMaxHeight (double distance);
	int GetEnv ();
	TVector2 GetStartPoint ();
	void SetStartPoint (TVector2 p);
	TPolyhedron	GetPoly (int type);
	void MirrorCourse ();

	void GetIndicesForPoint (double x, double z, int *x0, int *y0, int *x1, int *y1);
	void FindBarycentricCoords (double x, double z, 
		TIndex2 *idx0, TIndex2 *idx1, TIndex2 *idx2, double *u, double *v);
	double GetMinYCoord();
	TVector3 FindCourseNormal (double x, double z);
	double FindYCoord (double x, double z);
	void GetSurfaceType (double x, double z, double weights[]);
	int  GetTerrainIdx (double x, double z, double level);
	TPlane GetLocalCoursePlane (TVector3 pt);
};

extern CCourse Course;

#endif
