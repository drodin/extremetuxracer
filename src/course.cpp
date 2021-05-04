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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "bh.h"
#include "course.h"
#include "textures.h"
#include "ogl.h"
#include "audio.h"
#include "track_marks.h"
#include "spx.h"
#include "quadtree.h"
#include "env.h"
#include "game_ctrl.h"
#include "font.h"
#include "physics.h"
#include "winsys.h"
#include "translation.h"
#include <cmath>
#include <algorithm>
#include <iterator>


void TCourse::SetDescription(const std::string& description) {
	FT.AutoSizeN(2);
	std::vector<std::string> desclist = FT.MakeLineList(description.c_str(), 335.f * Winsys.scale - 16.f);
	std::size_t cnt = std::min<std::size_t>(desclist.size(), MAX_DESCRIPTION_LINES);
	num_lines = cnt;
	for (std::size_t ll = 0; ll < cnt; ll++) {
		desc[ll] = desclist[ll];
	}
}
void TCourse::SetTranslatedData(const std::string& line2) {
	std::string description = SPStrN(line2, "desc-" + Trans.languages[param.language].lang);
	std::string trans_name = SPStrN(line2, "name-" + Trans.languages[param.language].lang);
	if (description.empty()) // No translated description - fallback to default
		description = SPStrN(line2, "desc");
	if (!description.empty())
		SetDescription(description);
	if (!trans_name.empty())
		name = trans_name;
}


CCourse Course;


CCourse::CCourse()
	: curr_course(nullptr)
	, nx(0), ny(0)
	, base_height_value(0)
	, mirrored(false)
	, currentCourseList(nullptr)
	, vnc_array(nullptr) {
}

CCourse::~CCourse() {
	for (std::unordered_map<std::string, CCourseList>::iterator i = CourseLists.begin(); i != CourseLists.end(); ++i)
		i->second.Free();
	ResetCourse();
}

double CCourse::GetBaseHeight(double distance) const {
	double slope = std::tan(ANGLES_TO_RADIANS(curr_course->angle));

	return -slope * distance -
	       base_height_value / 255.0 * curr_course->scale;
}

double CCourse::GetMaxHeight(double distance) const {
	return GetBaseHeight(distance) + curr_course->scale;
}

const TPolyhedron& CCourse::GetPoly(std::size_t type) const {
	return PolyArr[ObjTypes[type].poly];
}

TCourse* CCourse::GetCourse(const std::string& group, const std::string& dir) {
	return &CourseLists.at(group)[dir];
}

std::size_t CCourse::GetCourseIdx(const TCourse* course) const {
	std::size_t idx = (course - &(*currentCourseList)[0]);
	if (idx >= currentCourseList->size())
		return -1;
	return idx;
}

void CCourse::CalcNormals() {
	for (unsigned int y = 0; y < ny; y++) {
		for (unsigned int x = 0; x < nx; x++) {
			TVector3d nml(0.0, 0.0, 0.0);
			TVector3d p0 = NMLPOINT(x,y);

			if ((x + y) % 2 == 0) {
				if (x > 0 && y > 0) {
					TVector3d p1 = NMLPOINT(x,  y-1);
					TVector3d p2 = NMLPOINT(x-1,y-1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;

					p1 = NMLPOINT(x-1, y-1);
					p2 = NMLPOINT(x-1, y);
					v1 = p1 - p0;
					v2 = p2 - p0;
					n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x > 0 && y < ny-1) {
					TVector3d p1 = NMLPOINT(x-1,y);
					TVector3d p2 = NMLPOINT(x-1,y+1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;

					p1 = NMLPOINT(x-1,y+1);
					p2 = NMLPOINT(x,y+1);
					v1 = p1 - p0;
					v2 = p2 - p0;
					n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x < nx-1 && y > 0) {
					TVector3d p1 = NMLPOINT(x+1,y);
					TVector3d p2 = NMLPOINT(x+1,y-1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;

					p1 = NMLPOINT(x+1,y-1);
					p2 = NMLPOINT(x,y-1);
					v1 = p1 - p0;
					v2 = p2 - p0;
					n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x < nx-1 && y < ny-1) {
					TVector3d p1 = NMLPOINT(x+1,y);
					TVector3d p2 = NMLPOINT(x+1,y+1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v1, v2);

					n.Norm();
					nml += n;

					p1 = NMLPOINT(x+1,y+1);
					p2 = NMLPOINT(x,y+1);
					v1 = p1 - p0;
					v2 = p2 - p0;
					n = CrossProduct(v1, v2);

					n.Norm();
					nml += n;
				}
			} else {
				if (x > 0 && y > 0) {
					TVector3d p1 = NMLPOINT(x,  y-1);
					TVector3d p2 = NMLPOINT(x-1,y);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x > 0 && y < ny-1) {
					TVector3d p1 = NMLPOINT(x-1,y);
					TVector3d p2 = NMLPOINT(x,y+1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x < nx-1 && y > 0) {
					TVector3d p1 = NMLPOINT(x+1,y);
					TVector3d p2 = NMLPOINT(x,y-1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v2, v1);

					n.Norm();
					nml += n;
				}
				if (x < nx-1 && y < ny-1) {
					TVector3d p1 = NMLPOINT(x+1,y);
					TVector3d p2 = NMLPOINT(x,y+1);
					TVector3d v1 = p1 - p0;
					TVector3d v2 = p2 - p0;
					TVector3d n = CrossProduct(v1, v2);

					n.Norm();
					nml += n;
				}
			}
			nml.Norm();
			Fields[x + nx * y].nml = nml;
			continue;

		}
	}
}

void CCourse::MakeCourseNormals() {
	CalcNormals();
}

// --------------------------------------------------------------------
//					FillGlArrays
// --------------------------------------------------------------------

void CCourse::FillGlArrays() {
	if (vnc_array == nullptr)
		vnc_array = new GLubyte[STRIDE_GL_ARRAY * nx * ny];

	for (unsigned int x = 0; x < nx; x++) {
		for (unsigned int y = 0; y < ny; y++) {
			int idx = STRIDE_GL_ARRAY * (y * nx + x);

			FLOATVAL(0) = (GLfloat)x / (nx-1.f) * curr_course->size.x;
			FLOATVAL(1) = Fields[x + nx*y].elevation;
			FLOATVAL(2) = -(GLfloat)y / (ny-1.f) * curr_course->size.y;

			const TVector3d& nml = Fields[ x + y * nx ].nml;
			FLOATVAL(4) = nml.x;
			FLOATVAL(5) = nml.y;
			FLOATVAL(6) = nml.z;
			FLOATVAL(7) = 1.0f;

			BYTEVAL(0) = 255;
			BYTEVAL(1) = 255;
			BYTEVAL(2) = 255;
			BYTEVAL(3) = 255;
		}
	}
}

void CCourse::MakeStandardPolyhedrons() {
	PolyArr.resize(2);

	// polyhedron "none"

	// poyhedron "tree"
	PolyArr[1].vertices = {
		TVector3d(0, 0, 0),
		TVector3d(0, 0.15, 0.5),
		TVector3d(0.5, 0.15, 0),
		TVector3d(0, 0.15, -0.5),
		TVector3d(-0.5, 0.15, 0),
		TVector3d(0, 1, 0)
	};

	PolyArr[1].polygons = {
		{ { 0, 1, 4 } },
		{ { 0, 2, 1 } },
		{ { 0, 3, 2 } },
		{ { 0, 4, 3 } },
		{ { 1, 5, 4 } },
		{ { 2, 5, 1 } },
		{ { 3, 5, 2 } },
		{ { 4, 5, 3 } },
	};
}

void CCourse::FreeTerrainTextures() {
	for (std::size_t i=0; i<TerrList.size(); i++) {
		delete TerrList[i].texture;
		TerrList[i].texture = nullptr;
	}
}

void CCourse::FreeObjectTextures() {
	for (std::size_t i=0; i<ObjTypes.size(); i++) {
		delete ObjTypes[i].texture;
		ObjTypes[i].texture = nullptr;
	}
}

// --------------------------------------------------------------------
//							LoadElevMap
// --------------------------------------------------------------------

bool CCourse::LoadElevMap() {
	sf::Image img;

	if (!img.loadFromFile(CourseDir + SEP "elev.png")) {
		Message("unable to open elev.png");
		return false;
	}
	img.flipVertically();

	// Get size of course from elevation map
	nx = img.getSize().x;
	ny = img.getSize().y;
	Fields.resize(nx*ny);

	double slope = std::tan(ANGLES_TO_RADIANS(curr_course->angle));
	int pad = 0;
	int depth = 4;
	const uint8_t* data = img.getPixelsPtr();
	for (unsigned int y = 0; y < ny; y++) {
		for (unsigned int x = 0; x < nx; x++) {
			Fields[(nx - 1 - x) + nx * (ny - 1 - y)].elevation =
			    ((data[(x + nx*y) * depth + pad]
			      - base_height_value) / 255.0) * curr_course->scale
			    - (double)(ny-1-y) / ny * curr_course->size.y * slope;
		}
		pad += (nx * depth) % 4;
	}
	return true;
}

// ====================================================================
//						LoadItemList
// ====================================================================

void CCourse::LoadItemList() {
	if (ObjTypes.empty()) {
		Message("No object types loaded.");
		return;
	}

	CSPList list;

	if (!list.Load(CourseDir, "items.lst")) {
		Message("could not load items list");
		return;
	}

	CollArr.clear();
	NocollArr.clear();
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		int x = SPIntN(*line, "x", 0);
		int z = SPIntN(*line, "z", 0);
		double height = SPFloatN(*line, "height", 1);
		double diam = SPFloatN(*line, "diam", 1);
		double xx = (nx - x) / (double)((double)nx - 1.0) * curr_course->size.x;
		double zz = -(int)(ny - z) / (double)((double)ny - 1.0) * curr_course->size.y;

		std::string name = SPStrN(*line, "name");
		std::size_t type = ObjectIndex[name];
		if (ObjTypes[type].texture == nullptr && ObjTypes[type].drawable) {
			ObjTypes[type].texture = new TTexture();
			ObjTypes[type].texture->Load(MakePathStr(param.obj_dir, ObjTypes[type].textureFile), false);
		}

		if (ObjTypes[type].collidable)
			CollArr.emplace_back(xx, FindYCoord(xx, zz), zz, height, diam, type);
		else
			NocollArr.emplace_back(xx, FindYCoord(xx, zz), zz, height, diam, ObjTypes[type]);
	}
	std::sort(CollArr.begin(), CollArr.end(), [](const TCollidable& l, const TCollidable& r) -> bool {
		return l.tree_type < r.tree_type;
	});
}

// --------------------	LoadObjectMap ---------------------------------


static int GetObject(const unsigned char* pixel) {
	int r = pixel[0];
	int g = pixel[1];
	int b = pixel[2];

	if (r<150 && b>200) return 0;
	if (std::abs(r-194)<10 && std::abs(g-40)<10 && std::abs(b-40)<10) return 1;
	if (std::abs(r-128)<10 && std::abs(g-128)<10 && b<10) return 2;
	if (r>220 && g>220 && b<20) return 3;
	if (r>220 && std::abs(g-128)<10 && b>220) return 4;
	if (r>220 && g>220 && b>220) return 5;
	if (r>220 && std::abs(g-96)<10 && b<40) return 6;
	if (r<40 && g >220 && b<80) return 7;
	return -1;
}

#define TREE_MIN 2.0
#define TREE_MAX 5.0
#define BARREN_MIN 4.0
#define BARREN_MAX 8.0
#define SHRUB_MIN 1.0
#define SHRUB_MAX 2.0

const double sizefact[6] = {0.5, 0.5, 0.7, 1.0, 1.4, 2.0};
const double varfact[6] = {1.0, 1.0, 1.22, 1.41, 1.73, 2.0};
const double diamfact = 1.4;

static void CalcRandomTrees(double baseheight, double basediam, double &height, double &diam) {
	double hhh = baseheight * sizefact[g_game.treesize];
	double minsiz = hhh / varfact[g_game.treevar];
	double maxsiz = hhh * varfact[g_game.treevar];
	height = XRandom(minsiz, maxsiz);
	diam = XRandom(height/diamfact, height);
}

bool CCourse::LoadAndConvertObjectMap() {
	sf::Image treeImg;

	if (!treeImg.loadFromFile(CourseDir + SEP "trees.png")) {
		Message("unable to open trees.png");
		return false;
	}
	treeImg.flipVertically();

	int pad = 0;
	int cnt = 0;
	int depth = 4;
	const unsigned char* data = (unsigned char*)treeImg.getPixelsPtr();
	double height, diam;
	CSPList savelist;

	CollArr.clear();
	NocollArr.clear();
	for (unsigned int y = 0; y < ny; y++) {
		for (unsigned int x = 0; x < nx; x++) {
			int imgidx = (x + nx * y) * depth + pad;
			int type = GetObject(&data[imgidx]);
			if (type >= 0) {
				cnt++;
				double xx = (nx - x) / (double)((double)nx - 1.0) * curr_course->size.x;
				double zz = -(int)(ny - y) / (double)((double)ny - 1.0) * curr_course->size.y;
				if (ObjTypes[type].texture == nullptr && ObjTypes[type].drawable) {
					ObjTypes[type].texture = new TTexture();
					ObjTypes[type].texture->Load(MakePathStr(param.obj_dir, ObjTypes[type].textureFile), false);
				}

				// set random height and diam - see constants above
				switch (type) {
					case 5:
						CalcRandomTrees(2.5, 2.5, height, diam);
						break;
					case 6:
						CalcRandomTrees(3, 3, height, diam);
						break;
					case 7:
						CalcRandomTrees(1.2, 1.2, height, diam);
						break;

					case 2:
					case 3:
						height = 6.0;
						diam = 9.0;
						break;

					default:
						height = 1;
						diam = 1;
						break;
				}

				if (ObjTypes[type].collidable)
					CollArr.emplace_back(xx, FindYCoord(xx, zz), zz, height, diam, type);
				else
					NocollArr.emplace_back(xx, FindYCoord(xx, zz), zz, height, diam, ObjTypes[type]);

				std::string line = "*[name]";
				line += ObjTypes[type].name;
				SPSetIntN(line, "x", x);
				SPSetIntN(line, "z", y);
				SPSetFloatN(line, "height", height, 1);
				SPSetFloatN(line, "diam", diam, 1);
				savelist.Add(line);
			}
		}
		pad += (nx * depth) % 4;
	}
	std::string itemfile = CourseDir + SEP "items.lst";
	savelist.Save(itemfile);  // Convert trees.png to items.lst
	return true;
}

// --------------------------------------------------------------------
//						LoadObjectTypes
// --------------------------------------------------------------------

bool CCourse::LoadObjectTypes() {
	CSPList list;

	if (!list.Load(param.obj_dir, "object_types.lst")) {
		Message("could not load object types");
		return false;
	}

	ObjTypes.resize(list.size());
	std::size_t i = 0;
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
		ObjTypes[i].name = SPStrN(*line, "name");
		ObjTypes[i].textureFile = ObjTypes[i].name;
		ObjTypes[i].texture = nullptr;

		ObjTypes[i].drawable = SPBoolN(*line, "draw", true);
		if (ObjTypes[i].drawable) {
			ObjTypes[i].textureFile = SPStrN(*line, "texture");
		}
		ObjTypes[i].collectable = SPBoolN(*line, "snap", true) != 0;
		if (ObjTypes[i].collectable == 0) {
			ObjTypes[i].collectable = -1;
		}

		ObjTypes[i].collidable = SPBoolN(*line, "coll", false);
		ObjTypes[i].reset_point = SPBoolN(*line, "reset", false);
		ObjTypes[i].use_normal = SPBoolN(*line, "usenorm", false);

		if (ObjTypes[i].use_normal) {
			ObjTypes[i].normal = SPVector3(*line, "norm", TVector3d(0, 1, 0));
			ObjTypes[i].normal.Norm();
		}
		ObjTypes[i].poly = 1;
	}
	list.MakeIndex(ObjectIndex, "name");
	return true;
}

// ====================================================================
//						Terrain
// ====================================================================

int CCourse::GetTerrain(const unsigned char* pixel) const {
	for (std::size_t i=0; i<TerrList.size(); i++) {
		if (std::abs(pixel[0]-TerrList[i].col.r) < 30
		        && std::abs(pixel[1]-TerrList[i].col.g) < 30
		        && std::abs(pixel[2]-TerrList[i].col.b) < 30) {
			return (int)i;
		}
	}
	return 0;
}

// --------------------------------------------------------------------
//         				LoadTerrainTypes
// --------------------------------------------------------------------

bool CCourse::LoadTerrainTypes() {
	CSPList list;

	if (!list.Load(param.terr_dir, "terrains.lst")) {
		Message("could not load terrain types");
		return false;
	}

	TerrList.resize(list.size());
	std::size_t i = 0;
	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
		TerrList[i].textureFile = SPStrN(*line, "texture");
		TerrList[i].sound = Sound.GetSoundIdx(SPStrN(*line, "sound"));
		TerrList[i].starttex = SPIntN(*line, "starttex", -1);
		TerrList[i].tracktex = SPIntN(*line, "tracktex", -1);
		TerrList[i].stoptex = SPIntN(*line, "stoptex", -1);
		TerrList[i].col = SPColor3N(*line, "col", sf::Color::White);
		TerrList[i].friction = SPFloatN(*line, "friction", 0.5f);
		TerrList[i].depth = SPFloatN(*line, "depth", 0.01f);
		TerrList[i].particles = SPBoolN(*line, "part", false);
		TerrList[i].trackmarks = SPBoolN(*line, "trackmarks", false);
		TerrList[i].texture = nullptr;
		TerrList[i].shiny = SPBoolN(*line, "shiny", false);
		TerrList[i].vol_type = SPIntN(*line, "vol_type", 1);
	}
	return true;
}

// --------------------------------------------------------------------
//					LoadTerrainMap
// --------------------------------------------------------------------

bool CCourse::LoadTerrainMap() {
	sf::Image terrImage;

	if (!terrImage.loadFromFile(CourseDir + SEP "terrain.png")) {
		Message("unable to open terrain.png");
		return false;
	}
	terrImage.flipVertically();
	if (nx != terrImage.getSize().x || ny != terrImage.getSize().y) {
		Message("wrong terrain size");
	}

	int depth = 4;
	const unsigned char* data = (const unsigned char*) terrImage.getPixelsPtr();
	int pad = 0;
	for (unsigned int y = 0; y < ny; y++) {
		for (unsigned int x = 0; x < nx; x++) {
			int imgidx = (x+nx*y) * depth + pad;
			int arridx = (nx-1-x) + nx * (ny-1-y);
			int terr = GetTerrain(&data[imgidx]);
			Fields[arridx].terrain = terr;
			if (TerrList[terr].texture == nullptr) {
				TerrList[terr].texture = new TTexture();
				TerrList[terr].texture->Load(param.terr_dir, TerrList[terr].textureFile, true);
			}
		}
		pad += (nx * depth) % 4;
	}
	return true;
}

// --------------------------------------------------------------------
//					CCourseList
// --------------------------------------------------------------------

bool CCourseList::Load(const std::string& dir) {
	CSPList list;

	if (!list.Load(dir, "courses.lst")) {
		Message("could not load courses.lst");
		return false;
	}

	CSPList paramlist;

	courses.resize(list.size());
	std::size_t i = 0;
	for (CSPList::const_iterator line1 = list.cbegin(); line1 != list.cend(); ++line1, i++) {
		courses[i].name = SPStrN(*line1, "name");
		courses[i].dir = SPStrN(*line1, "dir", "nodir");

		std::string coursepath = MakePathStr(dir, courses[i].dir);
		if (DirExists(coursepath.c_str())) {
			// preview
			std::string previewfile = coursepath + SEP "preview.png";
			courses[i].preview = new TTexture();
			if (!courses[i].preview->Load(previewfile, false)) {
				Message("couldn't load previewfile");
			}

			// params
			std::string paramfile = coursepath + SEP "course.dim";
			if (!paramlist.Load(paramfile)) {
				Message("could not load course.dim");
			}

			const std::string& line2 = paramlist.front();
			courses[i].author = SPStrN(line2, "author", Trans.Text(109));
			courses[i].size.x = SPFloatN(line2, "width", 100);
			courses[i].size.y = SPFloatN(line2, "length", 1000);
			courses[i].play_size.x = SPFloatN(line2, "play_width", 90);
			courses[i].play_size.y = SPFloatN(line2, "play_length", 900);
			courses[i].angle = SPFloatN(line2, "angle", 10);
			courses[i].scale = SPFloatN(line2, "scale", 10);
			courses[i].start.x = SPFloatN(line2, "startx", 50);
			courses[i].start.y = SPFloatN(line2, "starty", 5);
			courses[i].env = Env.GetEnvIdx(SPStrN(line2, "env", "etr"));
			courses[i].music_theme = Music.GetThemeIdx(SPStrN(line2, "theme", "normal"));
			courses[i].use_keyframe = SPBoolN(line2, "use_keyframe", false);
			courses[i].finish_brake = SPFloatN(line2, "finish_brake", 20);
			if (paramlist.size() >= 2)
				courses[i].SetTranslatedData(paramlist.back());
			paramlist.clear();	// the list is used several times
		}
	}
	list.MakeIndex(index, "dir");
	return true;
}

void CCourseList::Free() {
	for (std::size_t i = 0; i < courses.size(); i++) {
		delete courses[i].preview;
	}
	courses.clear();
}

void CCourse::FreeCourseList() {
	for (std::unordered_map<std::string, CCourseList>::iterator i = CourseLists.begin(); i != CourseLists.end(); ++i)
		i->second.Free();
}

bool CCourse::LoadCourseList() {
	CSPList list;

	if (!list.Load(param.common_course_dir, "groups.lst")) {
		Message("could not load groups.lst");
		return false;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		std::string dir = SPStrN(*line, "dir", "nodir");
		CourseLists[dir].Load(MakePathStr(param.common_course_dir, dir));
		CourseLists[dir].name = dir;
	}
	currentCourseList = &CourseLists["default"];
	return true;
}

CCourseList* CCourse::getGroup(std::size_t index) {
	std::unordered_map<std::string, CCourseList>::iterator i = CourseLists.begin();
	std::advance(i, index);
	return &i->second;
}

//  ===================================================================
//					LoadCourse
//  ===================================================================

void CCourse::ResetCourse() {
	Fields.clear();
	delete[] vnc_array;
	vnc_array = nullptr;

	FreeTerrainTextures();
	FreeObjectTextures();
	ResetQuadtree();
	curr_course = nullptr;
	mirrored = false;
}

bool CCourse::LoadCourse(TCourse* course) {
	if (course != curr_course || g_game.force_treemap) {
		ResetCourse();
		curr_course = course;
		CourseDir = param.common_course_dir + SEP + currentCourseList->name + SEP + curr_course->dir;

		start_pt.x = course->start.x;
		start_pt.y = -course->start.y;
		base_height_value = 127;

		g_game.use_keyframe = course->use_keyframe;
		g_game.finish_brake = course->finish_brake;

		if (!LoadElevMap()) {
			Message("could not load course elev map");
			return false;
		}

		MakeCourseNormals();
		FillGlArrays();

		if (!LoadTerrainMap()) {
			Message("could not load course terrain map");
			return false;
		}

		// ................................................................
		std::string itemfile = CourseDir + SEP "items.lst";
		bool itemsexists = FileExists(itemfile);
		const CControl *ctrl = g_game.player->ctrl;

		if (itemsexists && !g_game.force_treemap)
			LoadItemList();
		else
			LoadAndConvertObjectMap();
		g_game.force_treemap = false;
		// ................................................................

		init_track_marks();
		InitQuadtree(
		    &Fields[0], nx, ny,
		    curr_course->size.x / (nx - 1.0),
		    -curr_course->size.y / (ny - 1.0),
		    ctrl->viewpos,
		    param.course_detail_level);
	}

	if (g_game.mirrorred != mirrored) {
		MirrorCourse();
		mirrored = g_game.mirrorred;
	}
	return true;
}

std::size_t CCourse::GetEnv() const {
	return curr_course->env;
}

// --------------------------------------------------------------------
//				mirror course
// --------------------------------------------------------------------

void CCourse::MirrorCourseData() {
	for (unsigned int y = 0; y < ny; y++) {
		for (unsigned int x = 0; x < nx / 2; x++) {
			double tmp = ELEV(x,y);
			ELEV(x,y) = ELEV(nx-1-x, y);
			ELEV(nx-1-x,y) = tmp;

			int idx1 = (x+1) + nx*(y);
			int idx2 = (nx-1-x) + nx*(y);
			std::swap(Fields[idx1].terrain, Fields[idx2].terrain);

			idx1 = (x) + nx*(y);
			idx2 = (nx-1-x) + nx*(y);
			std::swap(Fields[idx1].nml, Fields[idx2].nml);
			Fields[idx1].nml.x *= -1;
			Fields[idx2].nml.x *= -1;
		}
	}

	for (std::size_t i=0; i<CollArr.size(); i++) {
		CollArr[i].pt.x = curr_course->size.x - CollArr[i].pt.x;
		CollArr[i].pt.y = FindYCoord(CollArr[i].pt.x, CollArr[i].pt.z);
	}

	for (std::size_t i=0; i<NocollArr.size(); i++) {
		NocollArr[i].pt.x = curr_course->size.x - NocollArr[i].pt.x;
		NocollArr[i].pt.y = FindYCoord(NocollArr[i].pt.x, NocollArr[i].pt.z);
	}

	FillGlArrays();

	ResetQuadtree();
	if (nx > 0 && ny > 0) {
		const CControl *ctrl = g_game.player->ctrl;
		InitQuadtree(&Fields[0], nx, ny, curr_course->size.x/(nx-1),
		             - curr_course->size.y/(ny-1), ctrl->viewpos, param.course_detail_level);
	}

	start_pt.x = curr_course->size.x - start_pt.x;
}

void CCourse::MirrorCourse() {
	MirrorCourseData();
	init_track_marks();
}

// ********************************************************************
//				from phys_sim:
// ********************************************************************

void CCourse::GetIndicesForPoint(double x, double z, unsigned int* x0, unsigned int* y0, unsigned int* x1, unsigned int* y1)  const {

	double xidx = x / curr_course->size.x * ((double) nx - 1.);
	double yidx = -z / curr_course->size.y * ((double) ny - 1.);

	if (xidx < 0) xidx = 0;
	else if (xidx > nx-1) xidx = nx-1;

	if (yidx < 0) yidx = 0;
	else if (yidx > ny-1) yidx = ny-1;

	*x0 = (unsigned int)std::floor(xidx);
	*x1 = (unsigned int)std::ceil(xidx);
	*y0 = (unsigned int)std::floor(yidx);
	*y1 = (unsigned int)std::ceil(yidx);

	if (*x0 == *x1) {
		if (*x1 < nx - 1)(*x1)++;
		else (*x0)--;
	}

	if (*y0 == *y1) {
		if (*y1 < ny - 1)(*y1)++;
		else (*y0)--;
	}
}

void CCourse::FindBarycentricCoords(double x, double z, TVector2i *idx0,
                                    TVector2i *idx1, TVector2i *idx2, double *u, double *v) const {
	double xidx, yidx;
	unsigned int x0, x1, y0, y1;
	double dx, ex, dz, ez, qx, qz, invdet;

	GetIndicesForPoint(x, z, &x0, &y0, &x1, &y1);
	xidx = x / curr_course->size.x * ((double) nx - 1.0);
	yidx = -z / curr_course->size.y * ((double) ny - 1.0);

	if ((x0 + y0) % 2 == 0) {
		if (yidx - y0 < xidx - x0) {
			*idx0 = TVector2i(x0, y0);
			*idx1 = TVector2i(x1, y0);
			*idx2 = TVector2i(x1, y1);
		} else {
			*idx0 = TVector2i(x1, y1);
			*idx1 = TVector2i(x0, y1);
			*idx2 = TVector2i(x0, y0);
		}
	} else {
		if (yidx - y0 + xidx - x0 < 1) {
			*idx0 = TVector2i(x0, y0);
			*idx1 = TVector2i(x1, y0);
			*idx2 = TVector2i(x0, y1);
		} else {
			*idx0 = TVector2i(x1, y1);
			*idx1 = TVector2i(x0, y1);
			*idx2 = TVector2i(x1, y0);
		}
	}

	dx = idx0->x - idx2->x;
	dz = idx0->y - idx2->y;
	ex = idx1->x - idx2->x;
	ez = idx1->y - idx2->y;
	qx = xidx - idx2->x;
	qz = yidx - idx2->y;

	invdet = 1 / (dx * ez - dz * ex);
	*u = (qx * ez - qz * ex) * invdet;
	*v = (qz * dx - qx * dz) * invdet;
}

#define COURSE_VERTX(_x, _y) TVector3d ( (double)(_x)/(nx-1.)*curr_course->size.x, \
                       ELEV((_x),(_y)), -(double)(_y)/(ny-1.)*curr_course->size.y )

TVector3d CCourse::FindCourseNormal(double x, double z) const {
	TVector2i idx0, idx1, idx2;
	double u, v;
	FindBarycentricCoords(x, z, &idx0, &idx1, &idx2, &u, &v);

	const TVector3d& n0 = Course.Fields[idx0.x + nx * idx0.y].nml;
	const TVector3d& n1 = Course.Fields[idx1.x + nx * idx1.y].nml;
	const TVector3d& n2 = Course.Fields[idx2.x + nx * idx2.y].nml;

	TVector3d p0 = COURSE_VERTX(idx0.x, idx0.y);
	TVector3d p1 = COURSE_VERTX(idx1.x, idx1.y);
	TVector3d p2 = COURSE_VERTX(idx2.x, idx2.y);

	TVector3d smooth_nml = u * n0 +
	                       v * n1 +
	                       (1.-u-v) * n2;

	TVector3d tri_nml = CrossProduct(p1 - p0, p2 - p0);
	tri_nml.Norm();

	double min_bary = std::min(u, std::min(v, 1. - u - v));
	double interp_factor = std::min(min_bary / NORM_INTERPOL, 1.0);

	TVector3d interp_nml = interp_factor * tri_nml + (1.-interp_factor) * smooth_nml;
	interp_nml.Norm();

	return interp_nml;
}

double CCourse::FindYCoord(double x, double z) const {
	static double last_x, last_z, last_y;
	static bool cache_full = false;

	if (cache_full && last_x == x && last_z == z) return last_y;

	TVector2i idx0, idx1, idx2;
	double u, v;
	FindBarycentricCoords(x, z, &idx0, &idx1, &idx2, &u, &v);

	TVector3d p0 = COURSE_VERTX(idx0.x, idx0.y);
	TVector3d p1 = COURSE_VERTX(idx1.x, idx1.y);
	TVector3d p2 = COURSE_VERTX(idx2.x, idx2.y);

	double ycoord = u * p0.y + v * p1.y + (1. - u - v) * p2.y;

	last_x = x;
	last_z = z;
	last_y = ycoord;
	cache_full = true;

	return ycoord;
}

void CCourse::GetSurfaceType(double x, double z, double weights[]) const {
	TVector2i idx0, idx1, idx2;
	double u, v;
	FindBarycentricCoords(x, z, &idx0, &idx1, &idx2, &u, &v);

	for (std::size_t i=0; i<Course.TerrList.size(); i++) {
		weights[i] = 0;
		if (Course.Fields[idx0.x + nx*idx0.y].terrain == i) weights[i] += u;
		if (Course.Fields[idx1.x + nx*idx1.y].terrain == i) weights[i] += v;
		if (Course.Fields[idx2.x + nx*idx2.y].terrain == i) weights[i] += 1.0 - u - v;
	}
}

int CCourse::GetTerrainIdx(double x, double z, double level) const {
	TVector2i idx0, idx1, idx2;
	double u, v;
	FindBarycentricCoords(x, z, &idx0, &idx1, &idx2, &u, &v);

	for (std::size_t i=0; i<Course.TerrList.size(); i++) {
		double wheight = 0.0;
		if (Course.Fields[idx0.x + nx*idx0.y].terrain == i) wheight += u;
		if (Course.Fields[idx1.x + nx*idx1.y].terrain == i) wheight += v;
		if (Course.Fields[idx2.x + nx*idx2.y].terrain == i) wheight += 1.0 - u - v;
		if (wheight > level) return (int)i;
	}
	return -1;
}

TPlane CCourse::GetLocalCoursePlane(TVector3d pt) const {
	TPlane plane;
	pt.y = FindYCoord(pt.x, pt.z);
	plane.nml = FindCourseNormal(pt.x, pt.z);
	plane.d = -DotProduct(plane.nml, pt);
	return plane;
}
