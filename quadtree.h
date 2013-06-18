/* --------------------------------------------------------------------
Author of the quadtree algorithm is Ulrich Thatcher. To get the code running
in Tuxracer it was modified by Jasmin F. Patry. I've modified the code
again for Bunny Hill (old version). With this modification it was possible
to use more terrains than the 3 standard terrains (snow, rock and ice). For
the current version of ETR I've rearranged the modules. The
quadgeom.cpp ist not required because all needed mathematical functions are
available in the mathlib unit (now called "mathlib"). Furthermore the functions
of "course_quad.cpp" are integrated in quadtree.cpp. These functions were
purposed to acess the C++ quadtree from the other modules which were completely
written in C. The new Bunny Hill code or ETR code is C++-orientated though
not rigorously adapted to C++.

The quadtree algorithm works well but has some disadvantages. One disadvantage
ist the kind of blurring at adjacent terrain tiles, detailed terrain texturing
is not possible in this way. Another, more weightly disadvantage is the
performance that depends on the count of different terrains. Many terrains on
a course slow down the race appreciably. It's not urgent but anytime this
algorithm should be replaced with a more convenient quadtree algorithm.
--------------------------------------------------------------------- */

#ifndef QUADTREE_H
#define QUADTREE_H

#include "bh.h"
#include "view.h"

class TTexture;


enum vertex_loc_t {
    East,
    South,
    Center
};

struct HeightMapInfo {
    double *Data;
    int	XOrigin, ZOrigin;
    int	XSize, ZSize;
    int	RowWidth;
    int	Scale;
    float Sample(int x, int z) const;
};

struct	VertInfo { float Y; };
struct quadsquare;

class quadcornerdata {
public:
    const quadcornerdata* Parent;
    quadsquare*	Square;
    int	ChildIndex;
    int	Level;
    int	xorg, zorg;
    VertInfo Verts[4];
};

struct quadsquare {
    quadsquare*	Child[4];

    VertInfo Vertex[5];
    float	Error[6];
    float	MinY, MaxY;
    unsigned char	EnabledFlags;
    unsigned char	SubEnabledCount[2];
    bool	Static;
    bool	Dirty;

    bool ForceEastVert;
    bool ForceSouthVert;

    static double ScaleX, ScaleZ;
    static int RowSize, NumRows;
    static char *Terrain;
    static TTexture* EnvmapTexture;

    static GLuint *VertexArrayIndices;
    static GLuint VertexArrayCounter;
    static GLuint VertexArrayMinIdx;
    static GLuint VertexArrayMaxIdx;

    static void MakeTri( int a, int b, int c, int terrain );
    static void MakeSpecialTri( int a, int b, int c, int terrain );
    static void MakeNoBlendTri( int a, int b, int c, int terrain );

    static void DrawTris();
    static void DrawEnvmapTris();
    static void InitArrayCounters();

    quadsquare (quadcornerdata* pcd);
	~quadsquare();

    void	AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm);
    void	StaticCullData(const quadcornerdata& cd, float ThresholdDetail);
    float	RecomputeError(const quadcornerdata& cd);
    int		CountNodes();
    void	Update(const quadcornerdata& cd,
			const float ViewerLocation[3], float Detail);
    void	Render(const quadcornerdata& cd, GLubyte *vnc_array);
    float	GetHeight(const quadcornerdata& cd, float x, float z);
    void	SetScale(double x, double z);
    void	SetTerrain (char *terrain);

private:
    quadsquare*	EnableDescendant(int count, int stack[],
				const quadcornerdata& cd);
    quadsquare*	GetNeighbor(int dir, const quadcornerdata &cd);
    clip_result_t ClipSquare( const quadcornerdata &cd );

	void	EnableEdgeVertex(int index, bool IncrementCount,
			const quadcornerdata &cd);
	void	EnableChild(int index, const quadcornerdata &cd);
	void	NotifyChildDisable(const quadcornerdata& cd, int index);
	void	ResetTree();
	void	StaticCullAux (const quadcornerdata &cd, float ThresholdDetail,
			int TargetLevel);
    void	CreateChild(int index, const quadcornerdata &cd);
	void	SetupCornerData (quadcornerdata *q, const quadcornerdata &pd,
			int ChildIndex);
    void	UpdateAux(const quadcornerdata &cd, const float ViewerLocation[3],
			float CenterError, clip_result_t vis);
	void	RenderAux(const quadcornerdata &cd, clip_result_t vis,
			int terrain);
	void	SetStatic (const quadcornerdata &cd);
	void	InitVert(int i, int x, int z);
    bool	VertexTest(int x, float y, int z, float error, const float Viewer[3],
			int level, vertex_loc_t vertex_loc);
	bool	BoxTest(int x, int z, float size, float miny, float maxy,
			float error, const float Viewer[3]);
};

// --------------------------------------------------------------------
//				global calls
// --------------------------------------------------------------------

void ResetQuadtree();
void InitQuadtree (double *elevation, int nx, int nz,
			   double scalex, double scalez,
			   const TVector3& view_pos, double detail);

void UpdateQuadtree (const TVector3& view_pos, float detail);
void RenderQuadtree();


#endif
