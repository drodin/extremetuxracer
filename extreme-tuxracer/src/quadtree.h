// quadtree.hpp	-thatcher 9/15/1999 Copyright 1999-2000 Thatcher Ulrich

// Data structures for quadtree terrain storage.

// This code may be freely modified and redistributed.  I make no
// warrantees about it; use at your own risk.  If you do incorporate
// this code into a project, I'd appreciate a mention in the credits.
//
// Thatcher Ulrich <tu@tulrich.com>

// Modified for use in Tux Racer by Jasmin Patry <jfpatry@cgl.uwaterloo.ca>

// Modifications for use in ppracer by Volker Stroebel <volker@planetpenguin.de>



#ifndef _QUADTREE_H_
#define _QUADTREE_H_


#include "viewfrustum.h"

typedef enum {
    East,
    South,
    Center
} vertex_loc_t;

struct HeightMapInfo {
    //double *Data;
	float *Data;
    int	XOrigin, ZOrigin;
    int	XSize, ZSize;
    int	RowWidth;
    int	Scale;

   	inline float Sample(int x, int z) const{
		if ( x >= XSize ) {
			x = XSize - 1;
		}
		if ( z >= ZSize ) {
			z = ZSize - 1;
		}
		return Data[ x + z * RowWidth ];
	}

};

class quadsquare;

// A structure used during recursive traversal of the tree to hold
// relevant but transitory data.
struct quadcornerdata {
    const quadcornerdata* Parent;
    quadsquare*	Square;
    int	ChildIndex;
    int	Level;
    int	xorg, zorg;
	float Verts[4];
};


class quadsquare {
    quadsquare*	Child[4];

    float	Vertex[5];	// center, e, n, w, s
    float	Error[6];	// e, s, children: ne, nw, sw, se
    float	MinY, MaxY;	// Bounds for frustum culling and error testing.
    unsigned char	EnabledFlags;	// bits 0-7: e, n, w, s, ne, nw, sw, se
    unsigned char	SubEnabledCount[2];	// e, s enabled reference counts.
    bool	Static;
    bool	Dirty;	// Set when vertex data has changed, but error/enabled data has not been recalculated.

    bool ForceEastVert;
    bool ForceSouthVert;
	

    static float ScaleX, ScaleZ;
    static int RowSize, NumRows;
    static int* Terrain;

    static GLuint *VertexArrayIndices[NUM_TERRAIN_TYPES];
    static GLuint VertexArrayCounter[NUM_TERRAIN_TYPES];
    static GLuint VertexArrayMinIdx[NUM_TERRAIN_TYPES];
    static GLuint VertexArrayMaxIdx[NUM_TERRAIN_TYPES];

    static void MakeTri( int a, int b, int c, int terrain );
    static void MakeSpecialTri( int a, int b, int c);
    static void MakeNoBlendTri( int a, int b, int c, int terrain );

    static void DrawTris(int terrain);
    static void DrawEnvmapTris(GLuint MapTexId, int terrain);
    static void InitArrayCounters();

public:
    quadsquare(quadcornerdata* pcd);
    ~quadsquare();

    void	AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm);
    void	StaticCullData(const quadcornerdata& cd, const float ThresholdDetail);	
    float	RecomputeError(const quadcornerdata& cd);
    int	CountNodes();
	
    void	Update(const quadcornerdata& cd, const float ViewerLocation[3], const float Detail);
    void	Render(const quadcornerdata& cd, GLubyte *vnc_array);

    float	GetHeight(const quadcornerdata& cd, const float x, const float z);

	inline void SetScale(const float x, const float z)
	{
    	ScaleX = x;
    	ScaleZ = z;
	};

	inline void	SetTerrain(int *terrain){Terrain = terrain;};
	
private:
    void	EnableEdgeVertex(int index, const bool IncrementCount, const quadcornerdata& cd);
    quadsquare*	EnableDescendant(int count, int stack[], const quadcornerdata& cd);
    void	EnableChild(int index, const quadcornerdata& cd);
    void	NotifyChildDisable(const quadcornerdata& cd, int index);

    void	ResetTree();
    void	StaticCullAux(const quadcornerdata& cd, const float ThresholdDetail, const int TargetLevel);

    quadsquare*	GetNeighbor(const int dir, const quadcornerdata& cd) const;
    void	CreateChild(int index, const quadcornerdata& cd);
    void	SetupCornerData(quadcornerdata* q, const quadcornerdata& pd, const int ChildIndex);

    void	UpdateAux(const quadcornerdata& cd, const float ViewerLocation[3], const float CenterError, clip_result_t vis);
    void	RenderAux(const quadcornerdata& cd, clip_result_t vi);
    void	RenderAuxSpezial(const quadcornerdata& cd, clip_result_t vis);
    void	SetStatic(const quadcornerdata& cd);
	int InitVert(const int i, const int x, const int z);

    bool	VertexTest(int x, float y, int z, float error, const float Viewer[3], int level, vertex_loc_t vertex_loc );
    bool	BoxTest(int x, int z, float size, float miny, float maxy, float error, const float Viewer[3]);
    clip_result_t ClipSquare( const quadcornerdata& cd );
};

#endif // _QUADTREE_H_
