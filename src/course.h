/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tux Racer Team

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
#include <unordered_map>

#define FLOATVAL(i) (*(GLfloat*)(vnc_array+idx+(i)*sizeof(GLfloat)))
#define BYTEVAL(i) (*(GLubyte*)(vnc_array+idx+8*sizeof(GLfloat) + i*sizeof(GLubyte)))
#define STRIDE_GL_ARRAY (8 * sizeof(GLfloat) + 4 * sizeof(GLubyte))
#define ELEV(x,y) (Fields[(x) + nx*(y)].elevation)
#define NORM_INTERPOL 0.05
#define XCD(_x) ((double)(_x) / (nx-1.0) * curr_course->size.x)
#define ZCD(_y) (-(double)(_y) / (ny-1.0) * curr_course->size.y)
#define NMLPOINT(x,y) TVector3d(XCD(x), ELEV(x,y), ZCD(y) )


#define MAX_DESCRIPTION_LINES 8

class TTexture;


struct TTerrType {
	std::string textureFile;
	TTexture* texture;
	std::size_t sound;
	sf::Color col;

	bool particles;
	bool trackmarks;
	bool shiny;
	double friction;
	double depth;
	int vol_type;
	int starttex;
	int tracktex;
	int stoptex;
};

struct TObjectType {
	std::string name;
	std::string textureFile;
	TTexture*	texture;
	int			collectable;
	bool		collidable;
	bool		drawable;
	bool		reset_point;
	bool		use_normal;
	TVector3d	normal;
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
	std::size_t tree_type;
	TCollidable(double x, double y, double z, double height_, double diam_, std::size_t type)
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
	sf::String name;
	std::string dir;
	std::string author;
	sf::String desc[MAX_DESCRIPTION_LINES];
	std::size_t num_lines;
	TTexture* preview;
	TVector2d size;
	TVector2d play_size;
	double angle;
	double scale;
	TVector2d start;
	std::size_t env;
	std::size_t music_theme;
	double finish_brake;
	bool use_keyframe;

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
	std::unordered_map<std::string, std::size_t>  index;
public:
	std::string name;

	bool Load(const std::string& dir);
	void Free();
	TCourse& operator[](std::size_t idx) { return courses[idx]; }
	const TCourse& operator[](std::size_t idx) const { return courses[idx]; }
	TCourse& operator[](std::string name_) { return courses[index.at(name_)]; }
	const TCourse& operator[](std::string name_) const { return courses[index.at(name_)]; }
	std::size_t size() const { return courses.size(); }
};

class CCourse {
private:
	const TCourse* curr_course;
	std::unordered_map<std::string, std::size_t> ObjectIndex;
	std::string CourseDir;

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

	std::unordered_map<std::string, CCourseList> CourseLists;
	CCourseList*				currentCourseList;
	std::vector<TTerrType>		TerrList;
	std::vector<TObjectType>	ObjTypes;
	std::vector<TCollidable>	CollArr;
	std::vector<TItem>			NocollArr;
	std::vector<TPolyhedron>	PolyArr;

	std::vector<CourseFields>	Fields;
	GLubyte *vnc_array;

	CCourseList* getGroup(std::size_t index);

	void ResetCourse();
	TCourse* GetCourse(const std::string& group, const std::string& dir);
	std::size_t GetCourseIdx(const TCourse* course) const;
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
	std::size_t GetEnv() const;
	const TVector2d& GetStartPoint() const { return start_pt; }
	const TPolyhedron& GetPoly(std::size_t type) const;
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
