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

#include "bh.h"
#include "course.h"
#include "textures.h"
#include "ogl.h"
#include "audio.h"
#include "track_marks.h"
#include "spx.h"
#include "quadtree.h"
#include "course_render.h"
#include "keyframe.h"
#include "env.h"
#include "game_ctrl.h"
#include "font.h"

CCourse Course;

CCourse::CCourse () {
	numCourses = 0;
	numTerr = 0;
	numObjTypes = 0;
	numColl = 0;
 	numNocoll = 0;
	numPolys = 0;

	terrain = NULL;
	elevation = NULL;
	nmls = NULL;
	vnc_array = NULL;

	curr_course = -1;
	numObjects = 0;
}

TVector2 CCourse::GetStartPoint () { 
	return start_pt; 
}

void CCourse::SetStartPoint (TVector2 p)	{ 
	start_pt = p; 
}

double CCourse::GetBaseHeight (double distance){
    double slope = tan (ANGLES_TO_RADIANS (course_angle));
    double base_height;
    
    base_height = -slope * distance - 
	base_height_value / 255.0 * elev_scale;
    return base_height;
}

double CCourse::GetMaxHeight (double distance){
    return GetBaseHeight (distance) + elev_scale;
}

double CCourse::GetCourseAngle () {
	return course_angle; 
} 

double CCourse::GetCourseDescent () { 
	return course_descent; 
}

void CCourse::GetDimensions (double *w, double *l) {
    *w = width;
    *l = length;
} 

void CCourse::GetPlayDimensions (double *pw, double *pl) {
    *pw = play_width;
    *pl = play_length;
} 

void CCourse::GetDivisions (int *x, int *y) {
    *x = nx;
    *y = ny;
} 

void CCourse::GetGLArrays (GLubyte **vnc_arr) {
	*vnc_arr = vnc_array;
}

TPolyhedron	CCourse::GetPoly (int type) { 
	return PolyArr[ObjTypes[type].poly]; 
}

int CCourse::GetCourseIdx (string dir) {
	return SPIntN (CourseIndex, dir, 0);
}

void CCourse::CalcNormals () {
    int x, y;
    TVector3 p0, p1, p2;
    TVector3 n, nml, v1, v2;

	for (y=0; y<ny; y++) {
        for  (x=0; x<nx; x++) {
            nml = MakeVector (0.0, 0.0, 0.0);
            p0 = MakeVector (XCD(x), ELEV(x,y), ZCD(y));

	    if  ((x + y) % 2 == 0) {
		if  (x > 0 && y > 0) {
		    p1 = NMLPOINT(x,  y-1);
		    p2 = NMLPOINT(x-1,y-1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);

		    p1 = NMLPOINT (x-1, y-1);
		    p2 = NMLPOINT (x-1, y);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x > 0 && y < ny-1) {
		    p1 = NMLPOINT(x-1,y);
		    p2 = NMLPOINT(x-1,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);

		    p1 = NMLPOINT(x-1,y+1);
		    p2 = NMLPOINT(x  ,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x < nx-1 && y > 0) {
		    p1 = NMLPOINT(x+1,y);
		    p2 = NMLPOINT(x+1,y-1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);

		    p1 = NMLPOINT(x+1,y-1);
		    p2 = NMLPOINT(x  ,y-1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x < nx-1 && y < ny-1) {
		    p1 = NMLPOINT(x+1,y);
		    p2 = NMLPOINT(x+1,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v1, v2);

		    NormVector (&n);
		    nml = AddVectors (nml, n);

		    p1 = NMLPOINT(x+1,y+1);
		    p2 = NMLPOINT(x  ,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v1, v2);

		    NormVector (&n);
		    nml = AddVectors (nml, n);

		} 
	    } else {
		if  (x > 0 && y > 0) {
		    p1 = NMLPOINT(x,  y-1);
		    p2 = NMLPOINT(x-1,y);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x > 0 && y < ny-1) {
		    p1 = NMLPOINT(x-1,y);
		    p2 = NMLPOINT(x  ,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x < nx-1 && y > 0) {
		    p1 = NMLPOINT(x+1,y);
		    p2 = NMLPOINT(x  ,y-1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v2, v1);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
		if  (x < nx-1 && y < ny-1) {
		    p1 = NMLPOINT(x+1,y);
		    p2 = NMLPOINT(x  ,y+1);
		    v1 = SubtractVectors (p1, p0);
		    v2 = SubtractVectors (p2, p0);
		    n = CrossProduct (v1, v2);

		    NormVector (&n);
		    nml = AddVectors (nml, n);
		} 
	    }
            NormVector (&nml);
			nmls [x + nx * y] = nml;
            continue;
        
		} 
    } 
}

void CCourse::MakeCourseNormals () {
    if (nmls != NULL) free (nmls);
    nmls = (TVector3 *) malloc (sizeof(TVector3) * nx * ny); 
    if (nmls == NULL) Message ("malloc failed" , "");
	CalcNormals ();	
} 

// --------------------------------------------------------------------
//					FillGlArrays
// --------------------------------------------------------------------

void CCourse::FillGlArrays() {
    int x,y;
    TVector3 *normals = nmls;
    TVector3 nml;
    int idx;

    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);

    vnc_array = (GLubyte*) malloc (STRIDE_GL_ARRAY * nx * ny);

    for (x=0; x<nx; x++) {
		for (y=0; y<ny; y++) {
			idx = STRIDE_GL_ARRAY * (y * nx + x);
		
			FLOATVAL(0) = (GLfloat)x / (nx-1.0) * width;
			FLOATVAL(1) = elevation[(x) + nx*(y)]; 
			FLOATVAL(2) = -(GLfloat)y / (ny-1.0) * length;
	
			nml = normals[ x + y * nx ];
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

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, STRIDE_GL_ARRAY, vnc_array);

    glEnableClientState (GL_NORMAL_ARRAY);
    glNormalPointer (GL_FLOAT, STRIDE_GL_ARRAY, 
		     vnc_array + 4*sizeof(GLfloat));

    glEnableClientState (GL_COLOR_ARRAY);
    glColorPointer (4, GL_UNSIGNED_BYTE, STRIDE_GL_ARRAY, 
		    vnc_array + 8*sizeof(GLfloat));
}

void CCourse::MakeStandardPolyhedrons () {
	int i;
	
	// polyhedron "none"
	PolyArr[0].num_vertices = 0;
	PolyArr[0].num_polygons = 0;
	PolyArr[0].vertices = NULL;
	PolyArr[0].polygons = NULL;

	// poyhedron "tree"	
	PolyArr[1].num_vertices = 6;
	PolyArr[1].vertices = (TVector3*) malloc (sizeof(TVector3) * 6);
	PolyArr[1].vertices[0] = MakeVector (0, 0, 0);
	PolyArr[1].vertices[1] = MakeVector (0, 0.15, 0.5);
	PolyArr[1].vertices[2] = MakeVector (0.5, 0.15, 0);
	PolyArr[1].vertices[3] = MakeVector (0, 0.15, -0.5);
	PolyArr[1].vertices[4] = MakeVector (-0.5, 0.15, 0);
	PolyArr[1].vertices[5] = MakeVector (0, 1, 0);

	PolyArr[1].num_polygons = 8;
	PolyArr[1].polygons = (TPolygon*) malloc (sizeof(TPolygon) * 8);
	for (i=0; i<PolyArr[1].num_polygons; i++) {
		PolyArr[1].polygons[i].num_vertices = 3;	
		PolyArr[1].polygons[i].vertices = (int*) malloc (sizeof(int) * 3);
	}	
	PolyArr[1].polygons[0].vertices[0] = 0;	
	PolyArr[1].polygons[0].vertices[1] = 1;	
	PolyArr[1].polygons[0].vertices[2] = 4;	
	
	PolyArr[1].polygons[1].vertices[0] = 0;	
	PolyArr[1].polygons[1].vertices[1] = 2;	
	PolyArr[1].polygons[1].vertices[2] = 1;
		
	PolyArr[1].polygons[2].vertices[0] = 0;	
	PolyArr[1].polygons[2].vertices[1] = 3;	
	PolyArr[1].polygons[2].vertices[2] = 2;	
	
	PolyArr[1].polygons[3].vertices[0] = 0;	
	PolyArr[1].polygons[3].vertices[1] = 4;	
	PolyArr[1].polygons[3].vertices[2] = 3;
		
	PolyArr[1].polygons[4].vertices[0] = 1;	
	PolyArr[1].polygons[4].vertices[1] = 5;	
	PolyArr[1].polygons[4].vertices[2] = 4;
		
	PolyArr[1].polygons[5].vertices[0] = 2;	
	PolyArr[1].polygons[5].vertices[1] = 5;	
	PolyArr[1].polygons[5].vertices[2] = 1;
		
	PolyArr[1].polygons[6].vertices[0] = 3;	
	PolyArr[1].polygons[6].vertices[1] = 5;	
	PolyArr[1].polygons[6].vertices[2] = 2;
		
	PolyArr[1].polygons[7].vertices[0] = 4;	
	PolyArr[1].polygons[7].vertices[1] = 5;	
	PolyArr[1].polygons[7].vertices[2] = 3;	

	PolyIndex = "[none]0[tree]1";
	numPolys = 2;
}

void CCourse::FreeTerrainTextures () {
	for (int i=0; i<numTerr; i++) {
		if (TerrList[i].texid > 0) {
			glDeleteTextures (1, &TerrList[i].texid);
			TerrList[i].texid = 0;
		}	
	}
}

void CCourse::FreeObjectTextures () {
	for (int i=0; i<numObjTypes; i++) {
		if (ObjTypes[i].texid > 0) {
			glDeleteTextures (1, &ObjTypes[i].texid);
			ObjTypes[i].texid = 0;
		}
	}
}

// --------------------------------------------------------------------
//							LoadElevMap
// --------------------------------------------------------------------

bool CCourse::LoadElevMap () {
	CImage img;
    double slope;
    int x, y, pad;

	if (!img.LoadPng (CourseDir.c_str(), "elev.png", true)) {
		Message ("unable to open elev.png");
		return false;
	}

    nx = img.nx;
    ny = img.ny;   
	elevation = (double *) malloc (sizeof (double) * nx * ny);

    if (elevation == NULL) {
		Message ("malloc failed in LoadElevMap");
		return false;
    }

    slope = tan (ANGLES_TO_RADIANS (course_angle));
    pad = 0;   
    for (y=0; y<ny; y++) {
        for (x=0; x<nx; x++) {
			elevation [(nx-1-x) + nx * (ny-1-y)] =     
				((img.data [(x+nx*y) * img.depth + pad] 
			    - base_height_value) / 255.0) * elev_scale
				- (double)(ny-1-y) / ny * length * slope;
   	     } 
        pad += (nx * img.depth) % 4;
    } 
	return true;
} 

// ====================================================================
//						LoadItemList
// ====================================================================

void CCourse::LoadItemList () {
	CSPList list (16000);
	int i, x, z, type;
	double height, diam, xx, zz;
	string ObjectsFile;
	string line;
	string name;
	bool coll;
	string terrpath;
	
    numColl = 0;
    numNocoll = 0;
    numObjects = 0;

	if (!list.Load (CourseDir, "items.lst")) {
		Message ("could not load items list");
		return;
	}

	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		x = SPIntN (line, "x", 0);
		z = SPIntN (line, "z", 0);
		height = SPFloatN (line, "height", 1);		
		diam = SPFloatN (line, "diam", 1);		
		xx = (nx - x) / (double)(nx - 1.0) * width;
		zz = -(ny - z) / (double)(ny - 1.0) * length;

		name = SPStrN (line, "name", "");
		type = SPIntN (ObjectIndex, name, 0);
		if (ObjTypes[type].texid < 1 && ObjTypes[type].drawable) {
			terrpath = param.obj_dir + SEP + ObjTypes[type].texture;
			ObjTypes[type].texid = Tex.LoadMipmapTexture (terrpath.c_str(), 0);
		}
		coll = ObjTypes[type].collidable;
		if (coll == 1) {
		    CollArr[numColl].pt.x = xx;
		    CollArr[numColl].pt.z = zz;
		    CollArr[numColl].pt.y = FindYCoord (xx, zz);
		    CollArr[numColl].height = height;
		    CollArr[numColl].diam = diam;
		    CollArr[numColl].tree_type = type;
		    numColl++;
		    numObjects++;	
		} else if (coll == 0) {
		    NocollArr[numNocoll].pt.x = xx;
		    NocollArr[numNocoll].pt.z = zz;
		    NocollArr[numNocoll].pt.y = FindYCoord (xx, zz);
			NocollArr[numNocoll].height = height; 
		    NocollArr[numNocoll].diam = diam;
		    NocollArr[numNocoll].item_type = type;
			NocollArr[numNocoll].collectable = ObjTypes[type].collectable;
			NocollArr[numNocoll].drawable = ObjTypes[type].drawable;
			ObjTypes[type].num_items++; 
			numNocoll++;
			numObjects++;
		}		
	}
}

// --------------------	LoadObjectMap ---------------------------------


int GetObject (unsigned char pixel[]) {
	int r = pixel[0];
	int g = pixel[1];
	int b = pixel[2];

	if (r<150 && b>200) return 0;
	if (abs(r-194)<10 && abs(g-40)<10 && abs(b-40)<10) return 1;
	if (abs(r-128)<10 && abs(g-128)<10 && b<10) return 2;
	if (r>220 && g>220 && b<20) return 3;
	if (r>220 && abs(g-128)<10 && b>220) return 4;
	if (r>220 && g>220 && b>220) return 5;
	if (r>220 && abs(g-96)<10 && b<40) return 6;
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

void CalcRandomTrees (double baseheight, double basediam, double &height, double &diam) {
	double hhh = baseheight * sizefact[g_game.treesize];
	double minsiz = hhh / varfact[g_game.treevar];
	double maxsiz = hhh * varfact[g_game.treevar];
	height = XRandom (minsiz, maxsiz); 
	diam = XRandom (height/diamfact, height);
}

bool CCourse::LoadObjectMap () {
    CImage treeImg;
    int x,y;
    int pad;
	string terrpath;
	string itemfile;
	int type, imgidx;
	double xx, zz, height, diam;
	bool coll;
	CSPList savelist (10000);
	string line;
	
	if (!treeImg.LoadPng (CourseDir.c_str(), "trees.png", true)) {
		Message ("unable to open trees.png");
		return false;
	}
	
	pad = 0;
    int cnt = 0;
		
    numColl = 0;
    numNocoll = 0;
    numObjects = 0;
	for (y=0; y<ny; y++) {
        for (x=0; x<nx; x++) {
            imgidx = (x + nx * y) * treeImg.depth + pad;
			type = GetObject (&treeImg.data[imgidx]);
			if (type >= 0) {
				cnt++;
				xx = (nx - x) / (double)(nx - 1.0) * width;
				zz = -(ny - y) / (double)(ny - 1.0) * length;
				if (ObjTypes[type].texid < 1 && ObjTypes[type].drawable) {
					terrpath = param.obj_dir + SEP + ObjTypes[type].texture;
					ObjTypes[type].texid = Tex.LoadMipmapTexture (terrpath.c_str(), 0);
				}
				
				// set random height and diam - see constants above
				switch (type) {	
					case 5: CalcRandomTrees (2.5, 2.5, height, diam); break;
					case 6: CalcRandomTrees (3, 3, height, diam); break;
					case 7: CalcRandomTrees (1.2, 1.2, height, diam); break;

					case 2: case 3:
					height = 6.0;
					diam = 9.0;
					break;
					
					default:
					height = 1;
					diam = 1;
					break;
				}
				
				coll = ObjTypes[type].collidable;
				if (coll == 1) {
				    CollArr[numColl].pt.x = xx;
				    CollArr[numColl].pt.z = zz;
			    	CollArr[numColl].pt.y = FindYCoord (xx, zz);
				    CollArr[numColl].height = height;
				    CollArr[numColl].diam = diam;
				    CollArr[numColl].tree_type = type;
				    numColl++;
			    	numObjects++;
				} 
				else if (coll == 0) {
				    NocollArr[numNocoll].pt.x = xx;
				    NocollArr[numNocoll].pt.z = zz;
			    	NocollArr[numNocoll].pt.y = FindYCoord (xx, zz);
				    NocollArr[numNocoll].height = height; 
				    NocollArr[numNocoll].diam = diam;
				    NocollArr[numNocoll].item_type = type;
					NocollArr[numNocoll].collectable = ObjTypes[type].collectable;
					NocollArr[numNocoll].drawable = ObjTypes[type].drawable;
					ObjTypes[type].num_items++; 
					numNocoll++;
					numObjects++;
				}		
		    	
				if (SaveItemsFlag) {
					line = "*[name]";
					line += ObjTypes[type].name;
 					SPSetIntN (line, "x", x);
					SPSetIntN (line, "z", y);
					SPSetFloatN (line, "height", height, 1);
					SPSetFloatN (line, "diam", diam, 1);
					savelist.Add (line);
				}
			} 
		} 
        pad += (nx * treeImg.depth) % 4;
	} 
	if (SaveItemsFlag) {
		itemfile = CourseDir + SEP + "items.lst";
		savelist.Save (itemfile);
	}
	return true;
}

// --------------------------------------------------------------------
//						LoadObjectTypes 
// --------------------------------------------------------------------

bool CCourse::LoadObjectTypes () {
	int i;
	string line;
	CSPList list (MAX_OBJECT_TYPES+10);

	for (i=0; i<MAX_OBJECT_TYPES; i++) ObjTypes[i].texid = 0;
	numObjTypes = 0;

	if (!list.Load (param.obj_dir, "object_types.lst")) {
		Message ("could not load object types");
		return false;
	}

	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
	    ObjTypes[i].name = SPStrN (line, "name", "");
	    ObjTypes[i].texture = ObjTypes[i].name;

		ObjTypes[i].drawable = SPIntN (line, "draw", 1);
		if (ObjTypes[i].drawable) {
			ObjTypes[i].texture = SPStrN (line, "texture", "");
		}
		ObjTypes[i].collectable = SPIntN (line, "snap", -1);
		if (ObjTypes[i].collectable == 0) {
			ObjTypes[i].collectable = -1;
		}

		ObjTypes[i].collidable = SPIntN (line, "coll", 0);
		ObjTypes[i].reset_point = SPIntN (line, "reset", 0);
		ObjTypes[i].use_normal = SPIntN (line, "usenorm", 0);

		if (ObjTypes[i].use_normal) {
			ObjTypes[i].normal = SPVector3N (line, "norm", MakeVector (0, 1, 0));
			NormVector (&(ObjTypes[i].normal));
		}
		ObjTypes[i].poly = 1;
		numObjTypes++;
	}
	list.MakeIndex (ObjectIndex, "name");
	return true;
}

// ====================================================================
//						Terrain
// ====================================================================

int CCourse::GetTerrain (unsigned char pixel[]) {
	int i;
	for (i=0; i<numTerr; i++) {
		if (abs(pixel[0]-(int)(TerrList[i].col.r)) < 30 
			&& abs(pixel[1]-(int)(TerrList[i].col.g)) < 30 
			&& abs(pixel[2]-(int)(TerrList[i].col.b)) < 30) {
			return i;
		}
	}
	return 0;
}

// --------------------------------------------------------------------
//         				LoadTerrainTypes 
// --------------------------------------------------------------------

bool CCourse::LoadTerrainTypes () {
	int i;
	string line;
	CSPList list(MAX_TERR_TYPES +10);
	
	for (i=0; i<MAX_TERR_TYPES; i++) TerrList[i].texid = 0;
	numTerr = 0;

	if (!list.Load (param.terr_dir, "terrains.lst")) {
		Message ("could not load terrain types");
		return false;
	}
	
	for (i=0; i<list.Count(); i++) {
		line = list.Line(i);
		TerrList[i].texture = SPStrN (line, "texture", "");
		TerrList[i].sound = SPStrN (line, "sound", "");
		TerrList[i].starttex = SPIntN (line, "starttex", -1);
		TerrList[i].tracktex = SPIntN (line, "tracktex", -1);
		TerrList[i].stoptex = SPIntN (line, "stoptex", -1);
		TerrList[i].col = SPColor3N (line, "col", MakeColor3 (1, 1, 1));
		TerrList[i].friction = SPFloatN (line, "friction", 0.5);
		TerrList[i].depth = SPFloatN (line, "depth", 0.01);
		TerrList[i].particles = SPIntN (line, "part", 0);
		TerrList[i].trackmarks = SPIntN (line, "trackmarks", 0);
		if (SPIntN (line, "shiny", 0) > 0) {
			TerrList[i].shiny = true;
		} else {
			TerrList[i].shiny = false;
		}
		TerrList[i].vol_type = SPIntN (line, "vol_type", 1);
		numTerr++;
	}
	return true;
}

// --------------------------------------------------------------------
//					LoadTerrainMap
// --------------------------------------------------------------------

bool CCourse::LoadTerrainMap () {
    CImage terrImage;
    int   x,y;
    int   pad;
    int   arridx, imgidx;
	int   terr;
	GLuint texid;
	string terrpath;
		
	if (!terrImage.LoadPng (CourseDir.c_str(), "terrain.png", true)) {
		Message ("unable to open terrain.png");
		return false;
	}
    if (nx != terrImage.nx || ny != terrImage.ny) {
		Message ("wrong terrain size", "");
    }

	terrain = (char *) malloc (sizeof (char) * nx * ny);
    if (terrain == NULL) {
		Message ("malloc failed in LoadTerrainMap", "");
    }
	pad = 0;
	for (y=0; y<ny; y++) {
        for (x=0; x<nx; x++) {
            imgidx = (x+nx*y) * terrImage.depth + pad;
			arridx = (nx-1-x) + nx * (ny-1-y);
			terr = GetTerrain (&terrImage.data[imgidx]);	
			terrain[arridx] = terr;	
			if (TerrList[terr].texid < 1) {
				terrpath = param.terr_dir + SEP + TerrList[terr].texture;
				texid = Tex.LoadMipmapTexture (terrpath.c_str(), 1);
				TerrList[terr].texid = texid;
			}
		} 
        pad += (nx * terrImage.depth) % 4;
    } 
    return true;
} 

// --------------------------------------------------------------------
//					LoadCourseList
// --------------------------------------------------------------------

bool CCourse::LoadCourseList () {
	string listfile, previewfile, paramfile, coursepath;
	string line, desc;
	int i, ll, cnt;
	GLuint texid;
	CSPList desclist (12);

	for (i=0; i<MAX_COURSES; i++) {
		CourseList[i].name = "";
		CourseList[i].dir = "";		
		CourseList[i].author = "";
		CourseList[i].preview = 0;
	}

	CSPList list (128);
	CSPList paramlist (48);

	if (!list.Load (param.common_course_dir, "courses.lst")) {
		Message ("could not load courses.lst");
		return false;
	}

	for (i=0; i<list.Count(); i++) {
		line = list.Line (i);
		CourseList[i].name = SPStrN (line, "name", "noname");
		CourseList[i].dir = SPStrN (line, "dir", "nodir");
		CourseList[i].author = SPStrN (line, "author", "unknown");

		desc = SPStrN (line, "desc", "");		
		FT.AutoSizeN (2);
		FT.MakeLineList (desc.c_str(), &desclist, 335 * param.scale - 16.0);
		cnt = desclist.Count ();
		if (cnt > MAX_DESCRIPTION_LINES) cnt = MAX_DESCRIPTION_LINES;
		CourseList[i].num_lines = cnt;
		for (ll=0; ll<cnt; ll++) {
			CourseList[i].desc[ll] = desclist.Line (ll);
		}
		desclist.Clear ();
	
		coursepath = param.common_course_dir + SEP + CourseList[i].dir;
		if (DirExists (coursepath.c_str())) {
			// preview
			previewfile = coursepath + SEP + "preview.png";
			texid = Tex.LoadMipmapTexture (previewfile.c_str(), 0);
			if (texid < 1) {
				Message ("couldn't load previewfile");					
//				texid = Tex.TexID (NO_PREVIEW);
			}
			CourseList[i].preview = texid;

			// params
			paramfile = coursepath + SEP + "course.dim";
			if (!paramlist.Load (paramfile)) {
				Message ("could not load course.dim");
			}

			line = paramlist.Line (0);
			CourseList[i].width = SPFloatN (line, "width", 100);
			CourseList[i].length = SPFloatN (line, "length", 1000);
			CourseList[i].play_width = SPFloatN (line, "play_width", 90);
			CourseList[i].play_length = SPFloatN (line, "play_length", 900);
			CourseList[i].angle = SPFloatN (line, "angle", 10);
			CourseList[i].scale = SPFloatN (line, "scale", 10);
			CourseList[i].startx = SPFloatN (line, "startx", 50);
			CourseList[i].starty = SPFloatN (line, "starty", 5);
			CourseList[i].env = Env.GetEnvIdx (SPStrN (line, "env", "etr"));
			CourseList[i].music_theme = Music.GetThemeIdx (SPStrN (line, "theme", "normal"));
			CourseList[i].use_keyframe = SPIntN (line, "use_keyframe", 0);
			CourseList[i].finish_brake = SPFloatN (line, "finish_brake", 20);
			paramlist.Clear ();	// the list is used several times
		}
		numCourses = i + 1;
	}
	list.MakeIndex (CourseIndex, "dir");
	return true;
}

void CCourse::FreeCourseList () {
	for (int i=0; i<MAX_COURSES; i++) {
		if (CourseList[i].preview > 0) glDeleteTextures (1, &CourseList[i].preview);
	}
}

//  ===================================================================
//           		LoadCourse
//  ===================================================================

void CCourse::ResetCourse () {
	if (nmls != NULL) {free (nmls); nmls = NULL;}
	if (vnc_array != NULL) {free (vnc_array); vnc_array = NULL;}
	if (elevation != NULL) {free (elevation); elevation = NULL;}
	if (terrain != NULL) {free (terrain); terrain = NULL;}

	FreeTerrainTextures ();
	FreeObjectTextures ();
	ResetQuadtree ();
	curr_course = -1;
}

bool CCourse::LoadCourse (int idx) {
	CControl *ctrl = Players.GetCtrl (g_game.player_id);

	if (idx < 0 || idx >= numCourses) {
		Message ("wrong course index");
		curr_course = -1;
		return false;
	}

	if (idx == curr_course && !g_game.force_treemap) return true;

	ResetCourse ();
	CourseDir = param.common_course_dir + SEP + CourseList[idx].dir;

	width = CourseList[idx].width;	
	length = CourseList[idx].length;	
	play_width = CourseList[idx].play_width;	
	play_length = CourseList[idx].play_length;
	course_angle = CourseList[idx].angle;
	course_descent = tan (course_angle * 6.2832 / 360.0) * length;
	elev_scale = CourseList[idx].scale;
	start_pt.x = CourseList[idx].startx;
	start_pt.y = -CourseList[idx].starty;
    base_height_value = 127; 
	env = CourseList[idx].env;
	music_theme = CourseList[idx].music_theme;

	g_game.use_keyframe = CourseList[idx].use_keyframe;
	g_game.finish_brake = CourseList[idx].finish_brake;

	if (!LoadElevMap ()) {
		Message ("could not load course elev map");
		return false;
	}

	MakeCourseNormals ();
    FillGlArrays ();

	if (!LoadTerrainMap ()) {
		Message ("could not load course terrain map");
		return false;
	}

	// ................................................................
	string itemfile = CourseDir + SEP + "items.lst";
	bool itemsexists = (FileExists (itemfile.c_str()));

	if (itemsexists && !g_game.force_treemap) {
		SaveItemsFlag = false;	
		LoadItemList ();		
	} else {					
		SaveItemsFlag = true;	
		LoadObjectMap ();		
	}
	g_game.force_treemap = false;
	// ................................................................

	init_track_marks ();
	InitQuadtree (
   		elevation, nx, ny, 
		width / (nx - 1.0), 
		-length / (ny - 1.0), 
		ctrl->viewpos, 
		param.course_detail_level);

	if (g_game.mirror_id > 0) MirrorCourse ();
	curr_course = idx;
	return true;
}

int CCourse::GetEnv () {
	return env;
}

// --------------------------------------------------------------------
//				mirror course
// --------------------------------------------------------------------

void CCourse::MirrorCourseData () {
    int x, y, i, idx1, idx2;
    double tmp;
    char tmp_terrain;
    TVector3 tmp_vec;
	CControl *ctrl = Players.GetCtrl (g_game.player_id);

	for (y=0; y<ny; y++) {
		for (x=0; x<nx/2; x++) {
			tmp = ELEV(x,y);
			ELEV(x,y) = ELEV(nx-1-x, y);
			ELEV(nx-1-x,y) = tmp;

			idx1 = (x+1) + nx*(y);
			idx2 = (nx-1-x) + nx*(y);
			tmp_terrain = terrain[idx1];
			terrain[idx1] = terrain[idx2];
			terrain[idx2] = tmp_terrain;

			idx1 = (x) + nx*(y);
			idx2 = (nx-1-x) + nx*(y);
			tmp_vec = nmls[idx1];
			nmls[idx1] = nmls[idx2];
			nmls[idx2] = tmp_vec;
			nmls[idx1].x *= -1;
			nmls[idx2].x *= -1;
		}
    }

    for  (i=0; i<numColl; i++) {
		CollArr[i].pt.x = width - CollArr[i].pt.x; 
		CollArr[i].pt.y = FindYCoord (CollArr[i].pt.x, CollArr[i].pt.z);
    }

    for  (i=0; i<numNocoll; i++) {
		NocollArr[i].pt.x = width - NocollArr[i].pt.x; 
		NocollArr[i].pt.y = FindYCoord (NocollArr[i].pt.x, NocollArr[i].pt.z);
    }

    FillGlArrays();

    ResetQuadtree ();
    if  (nx > 0 && ny > 0) {
	InitQuadtree (elevation, nx, ny, width/(nx-1), 
		- length/(ny-1), ctrl->viewpos, param.course_detail_level);
    }

    start_pt.x = width - start_pt.x;
}

void CCourse::MirrorCourse () {
		MirrorCourseData ();
		init_track_marks ();
}

// ********************************************************************
//				from phys_sim:
// ********************************************************************

void CCourse::GetIndicesForPoint 
		(double x, double z, int *x0, int *y0, int *x1, int *y1){

	double xidx = x / width * ((double) nx - 1.);
	double yidx = -z / length *  ((double) ny - 1.);

    if (xidx < 0) xidx = 0;
    else if  (xidx > nx-1) xidx = nx-1;

    if (yidx < 0) yidx = 0;
    else if  (yidx > ny-1) yidx = ny-1;

    *x0 = (int)(xidx);              // floor(xidx) 
    *x1 = (int)(xidx + 0.9999);     // ceil(xidx) 
    *y0 = (int)(yidx);              // floor(yidx)
    *y1 = (int)(yidx + 0.9999);     // ceil(yidx) 

    if (*x0 == *x1) {
		if (*x1 < nx - 1) (*x1)++; else (*x0)--;
    } 

    if (*y0 == *y1) {
		if (*y1 < ny - 1) (*y1)++; else (*y0)--;
    } 
}

void CCourse::FindBarycentricCoords (double x, double z, TIndex2 *idx0, 
		TIndex2 *idx1, TIndex2 *idx2, double *u, double *v) {
    double xidx, yidx;
    int x0, x1, y0, y1;
    double dx, ex, dz, ez, qx, qz, invdet; 
    double *elevation;

    elevation = Course.elevation;
    GetIndicesForPoint (x, z, &x0, &y0, &x1, &y1);
    xidx = x / width * ((double) nx - 1.);
    yidx = -z / length * ((double) ny - 1.);

    if  ((x0 + y0) % 2 == 0) {
		if  (yidx - y0 < xidx - x0) {
			*idx0 = MakeIndex2 (x0, y0); 
			*idx1 = MakeIndex2 (x1, y0); 
			*idx2 = MakeIndex2 (x1, y1); 
		} else {
			*idx0 = MakeIndex2 (x1, y1); 
			*idx1 = MakeIndex2 (x0, y1); 
			*idx2 = MakeIndex2 (x0, y0); 
		} 
    } else {
		if  (yidx - y0 + xidx - x0 < 1) {
			*idx0 = MakeIndex2 (x0, y0); 
			*idx1 = MakeIndex2 (x1, y0); 
			*idx2 = MakeIndex2 (x0, y1); 
		} else {
			*idx0 = MakeIndex2 (x1, y1); 
			*idx1 = MakeIndex2 (x0, y1); 
			*idx2 = MakeIndex2 (x1, y0); 
		} 
    }

    dx = idx0->i - idx2->i;
    dz = idx0->j - idx2->j;
    ex = idx1->i - idx2->i;
    ez = idx1->j - idx2->j;
    qx = xidx - idx2->i;
    qz = yidx - idx2->j;

    invdet = 1 / (dx * ez - dz * ex);
    *u = (qx * ez - qz * ex) * invdet;
    *v = (qz * dx - qx * dz) * invdet;
}

double CCourse::GetMinYCoord() {
	return (-length * tan (ANGLES_TO_RADIANS (course_angle)));
} 

#define COURSE_VERTX(x,y) MakeVector ( (double)(x)/(nx-1.)*width, \
                       ELEV((x),(y)), -(double)(y)/(ny-1.)*length ) 

TVector3 CCourse::FindCourseNormal (double x, double z) {

    double *elevation = Course.elevation;
    int x0, x1, y0, y1;
    GetIndicesForPoint (x, z, &x0, &y0, &x1, &y1);

    TIndex2 idx0, idx1, idx2;
    double u, v;
    FindBarycentricCoords (x, z, &idx0, &idx1, &idx2, &u, &v);

	TVector3 n0 = Course.nmls[ idx0.i + nx * idx0.j ];
	TVector3 n1 = Course.nmls[ idx1.i + nx * idx1.j ];
	TVector3 n2 = Course.nmls[ idx2.i + nx * idx2.j ];

	TVector3 p0 = COURSE_VERTX (idx0.i, idx0.j);
	TVector3 p1 = COURSE_VERTX (idx1.i, idx1.j);
	TVector3 p2 = COURSE_VERTX (idx2.i, idx2.j);
    
	TVector3 smooth_nml = AddVectors (
	ScaleVector (u, n0),
	AddVectors (ScaleVector (v, n1), ScaleVector (1.-u-v, n2)));

	TVector3 tri_nml = CrossProduct (
	SubtractVectors (p1, p0), SubtractVectors (p2, p0));
    NormVector (&tri_nml);

	double min_bary = min (u, min (v, 1. - u - v));
	double interp_factor = min (min_bary / NORM_INTERPOL, 1.0);

 	TVector3 interp_nml = AddVectors (
	ScaleVector (interp_factor, tri_nml),
	ScaleVector (1.-interp_factor, smooth_nml));
    NormVector (&interp_nml);

    return interp_nml;
}

double CCourse::FindYCoord (double x, double z) {
    static double last_x, last_z, last_y;
    static bool cache_full = false;

    if  (cache_full && last_x == x && last_z == z) return last_y;
    double *elevation = Course.elevation;

    TIndex2 idx0, idx1, idx2;
    double u, v;
    FindBarycentricCoords (x, z, &idx0, &idx1, &idx2, &u, &v);

	TVector3 p0 = COURSE_VERTX (idx0.i, idx0.j);
	TVector3 p1 = COURSE_VERTX (idx1.i, idx1.j);
	TVector3 p2 = COURSE_VERTX (idx2.i, idx2.j);

	double ycoord = u * p0.y + v * p1.y +  (1. - u - v) * p2.y;

    last_x = x;
    last_z = z;
    last_y = ycoord;
    cache_full = true;

    return ycoord;
} 

void CCourse::GetSurfaceType (double x, double z, double weights[]) {
    TIndex2 idx0, idx1, idx2;
    double u, v;
    FindBarycentricCoords (x, z, &idx0, &idx1, &idx2, &u, &v);

	char *terrain = Course.terrain;
    for (int i=0; i<Course.numTerr; i++) {
		weights[i] = 0;
		if (terrain [idx0.i + nx*idx0.j ] == i) weights[i] += u;
		if (terrain [idx1.i + nx*idx1.j ] == i) weights[i] += v;
		if (terrain [idx2.i + nx*idx2.j ] == i) weights[i] += 1.0 - u - v;
    }
} 

int CCourse::GetTerrainIdx (double x, double z, double level) {
    TIndex2 idx0, idx1, idx2;
    double u, v;
    FindBarycentricCoords (x, z, &idx0, &idx1, &idx2, &u, &v);
	char *terrain = Course.terrain;
    
	double weights[MAX_TERR_TYPES];
	for (int i=0; i<Course.numTerr; i++) {
		weights[i] = 0;
		if (terrain [idx0.i + nx*idx0.j] == i) weights[i] += u;
		if (terrain [idx1.i + nx*idx1.j] == i) weights[i] += v;
		if (terrain [idx2.i + nx*idx2.j] == i) weights[i] += 1.0 - u - v;
     	if (weights[i] > level) return (i);
	}
	return -1;
} 

TPlane CCourse::GetLocalCoursePlane (TVector3 pt) {
    TPlane plane;
	pt.y = FindYCoord (pt.x, pt.z);
    plane.nml = FindCourseNormal (pt.x, pt.z);
    plane.d = - (plane.nml.x * pt.x +  plane.nml.y * pt.y +  plane.nml.z * pt.z);
    return plane;
}





