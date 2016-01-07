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
#include "mathlib.h"
#include <vector>
#include <map>

#define FLOATVAL(i) (*(GLfloat*)(vnc_array+idx+(i)*sizeof(GLfloat)))
#define BYTEVAL(i) (*(GLubyte*)(vnc_array+idx+8*sizeof(GLfloat) +\
    i*sizeof(GLubyte)))
#define STRIDE_GL_ARRAY (8 * sizeof(GLfloat) + 4 * sizeof(GLubyte))
#define ELEV(x,y) (Fields[(x) + nx*(y)].elevation)
#define NORM_INTERPOL 0.05
#define XCD(_x) ((double)(_x) / (nx-1.0) * curr_course->size.x)
#define ZCD(_y) (-(double)(_y) / (ny-1.0) * curr_course->size.y)
#define NMLPOINT(x,y) TVector3d(XCD(x), ELEV(x,y), ZCD(y) )


#define MAX_COURSES 64
#define MAX_TERR_TYPES 64
#define MAX_OBJECT_TYPES 128
#define MAX_DESCRIPTION_LINES 8

class TTexture;


struct TTerrType {
	string textureFile;
	TTexture* texture;
	size_t sound;
	TColor3 col;

	double friction;
	double depth;
	int vol_type;
	int starttex;
	int tracktex;
	int stoptex;
	bool particles;
	bool trackmarks;
	bool shiny;
};

struct TObjectType {
	string		name;
	string		textureFile;
	TTexture*	texture;
	int			collectable;
	bool		collidable;
	bool		drawable;
	bool		reset_point;
	bool		use_normal;
	TVector3d	normal;
	int			num_items;
	int			poly;
};

struct TObject {
	TVector3d pt;
	double height;
	double diam;
	TObject(double x, double y, double z, double height_, double diam_)
		: pt(x, y, z), height(height_), diam(diam_)
	{}
};

struct TCollidable : public TObject {
	size_t tree_type;
	TCollidable(double x, double y, double z, double height_, double diam_, size_t type)
		: TObject(x, y, z, height_, diam_), tree_type(type)
	{}
};

struct TItem : public TObject {
	int collectable;
	const TObjectType& type;
	TItem(double x, double y, double z, double height_, double diam_, const TObjectType& type_)
		: TObject(x, y, z, height_, diam_), collectable(type_.collectable), type(type_)
	{}
};

struct TCourse {
	string name;
	string dir;
	string author;
	string desc[MAX_DESCRIPTION_LINES];
	size_t num_lines;
	TTexture* preview;
	TVector2d size;
	TVector2d play_size;
	double angle;
	double scale;
	TVector2d start;
	size_t env;
	size_t music_theme;
	bool use_keyframe;
	double finish_brake;

	void SetDescription(const std::string& description);
	void SetTranslatedData(const std::string& line2);
};

struct CourseFields {
	TVector3d nml;
	double elevation;
	uint8_t terrain;
};

class CCourseList {
	std::vector<TCourse> courses;
	map<string, size_t>  index;
public:
	string name;

	bool Load(const std::string& dir);
	void Free();
	TCourse& operator[](size_t idx) { return courses[idx]; }
	const TCourse& operator[](size_t idx) const { return courses[idx]; }
	TCourse& operator[](string name_) { return courses[index.at(name_)]; }
	const TCourse& operator[](string name_) const { return courses[index.at(name_)]; }
	size_t size() const { return courses.size(); }
};

class CCourse {
private:
	const TCourse* curr_course;
	map<string, size_t> ObjectIndex;
	string		CourseDir;

	unsigned int nx;
	unsigned int ny;
	TVector2d	start_pt;
	int			base_height_value;
	bool		mirrored;

	void		FreeTerrainTextures();
	void		FreeObjectTextures();
	void		CalcNormals();
	void		MakeCourseNormals();
	bool		LoadElevMap();
	void		LoadItemList();
	bool		LoadAndConvertObjectMap();
	bool		LoadTerrainMap();
	int			GetTerrain(const unsigned char* pixel) const;

	void		MirrorCourseData();
public:
	CCourse();
	~CCourse();

	map<string, CCourseList> CourseLists;
	CCourseList*             currentCourseList;
	vector<TTerrType>	TerrList;
	vector<TObjectType>	ObjTypes;
	vector<TCollidable>	CollArr;
	vector<TItem>		NocollArr;
	vector<TPolyhedron>	PolyArr;

	vector<CourseFields> Fields;
	GLubyte		*vnc_array;

	CCourseList* getGroup(size_t index);

	void ResetCourse();
	TCourse* GetCourse(const string& group, const string& dir);
	size_t GetCourseIdx(const TCourse* course) const;
	void FreeCourseList();
	bool LoadCourseList();
	bool LoadCourse(TCourse* course);
	bool LoadTerrainTypes();
	bool LoadObjectTypes();
	void MakeStandardPolyhedrons();
	GLubyte* GetGLArrays() const { return vnc_array; }
	void FillGlArrays();

	const TVector2d& GetDimensions() const { return curr_course->size; }
	const TVector2d& GetPlayDimensions() const { return curr_course->play_size; }
	double GetCourseAngle() const { return curr_course->angle; }
	double GetBaseHeight(double distance) const;
	double GetMaxHeight(double distance) const;
	size_t GetEnv() const;
	const TVector2d& GetStartPoint() const { return start_pt; }
	const TPolyhedron& GetPoly(size_t type) const;
	void MirrorCourse();

	void GetIndicesForPoint(double x, double z, unsigned int* x0, unsigned int* y0, unsigned int* x1, unsigned int* y1) const;
	void FindBarycentricCoords(double x, double z,
	                           TVector2i *idx0, TVector2i *idx1, TVector2i *idx2, double *u, double *v) const;
	TVector3d FindCourseNormal(double x, double z) const;
	double FindYCoord(double x, double z) const;
	void GetSurfaceType(double x, double z, double weights[]) const;
	int GetTerrainIdx(double x, double z, double level) const;
	TPlane GetLocalCoursePlane(TVector3d pt) const;
};

extern CCourse Course;

#endif
