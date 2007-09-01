// quadtree.cpp	-thatcher 9/15/1999 Copyright 1999-2000 Thatcher Ulrich

// Code for quadtree terrain manipulation, meshing, and display.

// This code may be freely modified and redistributed.  I make no
// warrantees about it; use at your own risk.  If you do incorporate
// this code into a project, I'd appreciate a mention in the credits.
//
// Thatcher Ulrich <tu@tulrich.com>

// Modified for use in Tux Racer by Jasmin Patry <jfpatry@cgl.uwaterloo.ca>

// Modifications for use in ppracer by Volker Stroebel <volker@planetpenguin.de>


#include "textures.h"
#include "course_load.h"
#include "fog.h"
#include "gl_util.h"
#include "course_render.h"
#include "game_config.h"


#include "ppgltk/alg/defs.h"

#include "quadtree.h"


/* Amount to scale terrain errors by in order to be comparable to
   height errors */
#define TERRAIN_ERROR_SCALE 0.1

/* Distance at which we force the activation of vertices that lie on
   texture edges */
#define VERTEX_FORCE_THRESHOLD 120

/* Maximum Distance at which we magnify the error term (to minimize
   popping near the camera */
#define ERROR_MAGNIFICATION_THRESHOLD 30

/* Amount to magnify errors by within ERROR_MAGNIFICATION_THRESHOLD */
#define ERROR_MAGNIFICATION_AMOUNT 3

/* Environment map alpha value, integer from 0-255 */ 
#define ENV_MAP_ALPHA 50

/* Useful macro for setting colors in the color array */
#define colorval(j,ch) \
VNCArray[j*STRIDE_GL_ARRAY+STRIDE_GL_ARRAY-4+(ch)]


extern terrain_tex_t terrain_texture[NUM_TERRAIN_TYPES];
extern unsigned int num_terrains;

int terrain_count[NUM_TERRAIN_TYPES];

//
// quadsquare functions.
//

GLuint *quadsquare::VertexArrayIndices[NUM_TERRAIN_TYPES]; 
GLuint quadsquare::VertexArrayCounter[NUM_TERRAIN_TYPES];
GLuint quadsquare::VertexArrayMinIdx[NUM_TERRAIN_TYPES];
GLuint quadsquare::VertexArrayMaxIdx[NUM_TERRAIN_TYPES];

quadsquare::quadsquare(quadcornerdata* pcd)
/// Constructor.
{
    pcd->Square = this;
	
    // Set static to true if/when this node contains real data, and
    // not just interpolated values.  When static == false, a node
    // can be deleted by the Update() function if none of its
    // vertices or children are enabled.
    Static = false;

    unsigned int	i;

	Child[0] = (quadsquare*) NULL;
	Child[1] = (quadsquare*) NULL;
	Child[2] = (quadsquare*) NULL;	
	Child[3] = (quadsquare*) NULL;


    EnabledFlags = 0;
	
	SubEnabledCount[0] = 0;
	SubEnabledCount[1] = 0;

	
    // Set default vertex positions by interpolating from given corners.
    // Just bilinear interpolation.
    Vertex[0] = 0.25 * (pcd->Verts[0] + pcd->Verts[1] + pcd->Verts[2] + pcd->Verts[3]);
    Vertex[1] = 0.5 * (pcd->Verts[3] + pcd->Verts[0]);
    Vertex[2] = 0.5 * (pcd->Verts[0] + pcd->Verts[1]);
    Vertex[3] = 0.5 * (pcd->Verts[1] + pcd->Verts[2]);
    Vertex[4] = 0.5 * (pcd->Verts[2] + pcd->Verts[3]);

    for (i = 0; i < 2; i++) {
	Error[i] = 0;
    }
    for (i = 0; i < 4; i++) {
	Error[i+2] = fabs((Vertex[0] + pcd->Verts[i]) - (Vertex[i+1] + Vertex[((i+1)&3) + 1])) * 0.25;
    }

    // Compute MinY/MaxY based on corner verts.
    MinY = MaxY = pcd->Verts[0];
    for (i = 1; i < 4; i++) {
	float	y = pcd->Verts[i];
	if (y < MinY) MinY = y;
	if (y > MaxY) MaxY = y;
    }

    if ( pcd->Parent == NULL ) {
		print_debug( DEBUG_QUADTREE, "initializing root node" );
		for (int i=0; i< NUM_TERRAIN_TYPES; i++){
			VertexArrayIndices[0] = NULL;		
		}
		Terrain = get_course_terrain_data();
    }
}


quadsquare::~quadsquare()
/// Destructor.
{
    // Recursively delete sub-trees.
    for (int i = 0; i < 4; i++) {
		if (Child[i]) delete Child[i];
		Child[i] = (quadsquare*) NULL;
    }
}


void	quadsquare::SetStatic(const quadcornerdata& cd)
/// Sets this node's static flag to true.  If static == true, then the
/// node or its children is considered to contain significant height data
/// and shouldn't be deleted.
{
    if (Static == false) {
	Static = true;
		
		// Propagate static status to ancestor nodes.
		if (cd.Parent && cd.Parent->Square) {
		    cd.Parent->Square->SetStatic(*cd.Parent);
		}
    }
}


int	quadsquare::CountNodes()
/// Debugging function.  Counts the number of nodes in this subtree.
{
    int	count = 1;	// Count ourself.

    // Count descendants.
    for (int i = 0; i < 4; i++) {
		if (Child[i]) count += Child[i]->CountNodes();
    }

    return count;
}


float	quadsquare::GetHeight(const quadcornerdata& cd, const float x, const float z)
/// Returns the height of the heightfield at the specified x,z coordinates.
{
    int	half = 1 << cd.Level;

    float	lx = (x - cd.xorg) / float(half);
    float	lz = (z - cd.zorg) / float(half);

    int	ix = (int) floor(lx);
    int	iz = (int) floor(lz);

    // Clamp.
    if (ix < 0) ix = 0;
    if (ix > 1) ix = 1;
    if (iz < 0) iz = 0;
    if (iz > 1) iz = 1;

    int	index = ix ^ (iz ^ 1) + (iz << 1);
    if (Child[index] && Child[index]->Static) {
	// Pass the query down to the child which contains it.
	quadcornerdata	q;
	SetupCornerData(&q, cd, index);
	return Child[index]->GetHeight(q, x, z);
    }

    // Bilinear interpolation.
    lx -= ix;
    if (lx < 0) lx = 0;
    if (lx > 1) lx = 1;
	
    lz -= iz;
    if (lx < 0) lz = 0;
    if (lz > 1) lz = 1;

    float	s00, s01, s10, s11;
    switch (index) {
    default:
    case 0:
	s00 = Vertex[2];
	s01 = cd.Verts[0];
	s10 = Vertex[0];
	s11 = Vertex[1];
	break;
    case 1:
	s00 = cd.Verts[1];
	s01 = Vertex[2];
	s10 = Vertex[3];
	s11 = Vertex[0];
	break;
    case 2:
	s00 = Vertex[3];
	s01 = Vertex[0];
	s10 = cd.Verts[2];
	s11 = Vertex[4];
	break;
    case 3:
	s00 = Vertex[0];
	s01 = Vertex[1];
	s10 = Vertex[4];
	s11 = cd.Verts[3];
	break;
    }

    return (s00 * (1-lx) + s01 * lx) * (1 - lz) + (s10 * (1-lx) + s11 * lx) * lz;
}


quadsquare*	quadsquare::GetNeighbor(const int dir, const quadcornerdata& cd) const
/// Traverses the tree in search of the quadsquare neighboring this square to the
/// specified direction.  0-3 --> { E, N, W, S }.
/// Returns NULL if the neighbor is outside the bounds of the tree.
{
    // If we don't have a parent, then we don't have a neighbor.
    // (Actually, we could have inter-tree connectivity at this level
    // for connecting separate trees together.)
    if (cd.Parent == 0) return 0;
	
    // Find the parent and the child-index of the square we want to locate or create.
    quadsquare*	p = 0;
	
    int	index = cd.ChildIndex ^ 1 ^ ((dir & 1) << 1);
    bool	SameParent = ((dir - cd.ChildIndex) & 2) ? true : false;
	
    if (SameParent) {
	p = cd.Parent->Square;
    } else {
	p = cd.Parent->Square->GetNeighbor(dir, *cd.Parent);
		
	if (p == 0) return 0;
    }
	
    quadsquare*	n = p->Child[index];
	
    return n;
}

float	quadsquare::RecomputeError(const quadcornerdata& cd)
/// Recomputes the error values for this tree.  Returns the
/// max error.
/// Also updates MinY & MaxY.
{
    int	i;
    int j;
    unsigned int t;
    int	half = 1 << cd.Level;
    int	whole = half << 1;
    float terrain_error;
	
    // Measure error of center and edge vertices.
    float	maxerror = 0;

    // Compute error of center vert.
    float	e;
    if (cd.ChildIndex & 1) {
	e = fabs(Vertex[0] - (cd.Verts[1] + cd.Verts[3]) * 0.5);
    } else {
	e = fabs(Vertex[0] - (cd.Verts[0] + cd.Verts[2]) * 0.5);
    }
    if (e > maxerror) maxerror = e;

    // Initial min/max.
    MaxY = Vertex[0];
    MinY = Vertex[0];

    // Check min/max of corners.
    for (i = 0; i < 4; i++) {
	float	y = cd.Verts[i];
	if (y < MinY) MinY = y;
	if (y > MaxY) MaxY = y;
    }
	
    // Edge verts.
    e = fabs(Vertex[1] - (cd.Verts[0] + cd.Verts[3]) * 0.5);
    if (e > maxerror) maxerror = e;
    Error[0] = e;
	
    e = fabs(Vertex[4] - (cd.Verts[2] + cd.Verts[3]) * 0.5);
    if (e > maxerror) maxerror = e;
    Error[1] = e;

    // Terrain edge checks
    if ( cd.Level == 0 && cd.xorg <= RowSize-1 && cd.zorg <= NumRows-1 ) {

	// Check South vertex
	int x = cd.xorg + half;
	int z = cd.zorg + whole;
	int idx = x  + z * RowSize;
	bool different_terrains = false;

	terrain_error = 0.f;

	check_assertion( x >= 0, "x coordinate is negative" );
	check_assertion( z >= 0, "z coordinate is negative" );

        if ( x < RowSize && z < NumRows ) {

	    if ( x < RowSize - 1 ) {
		if ( Terrain[idx] != Terrain[idx+1] ) {
		    different_terrains = true;
		}
	    }
	    if ( z >= 1 ) {
		idx -= RowSize;
		if ( Terrain[idx] != Terrain[idx+1] ) {
		    different_terrains = true;
		}
	    }

	    if ( different_terrains ) {
		ForceSouthVert = true;
		terrain_error = TERRAIN_ERROR_SCALE * whole * whole;
	    } else {
		ForceSouthVert = false;
	    }

	    if ( terrain_error > Error[0] ) {
		Error[0] = terrain_error;
	    }
	    if ( Error[0] > maxerror ) {
		maxerror = Error[0];
	    }
	}

	// Check East vertex
	x = cd.xorg + whole;
	z = cd.zorg + half;
	idx = x  + z * RowSize;
	terrain_error = 0;
	different_terrains = false;

        if ( x < RowSize && z < NumRows ) {

	    if ( z >= 1 ) {
		if ( Terrain[idx] != Terrain[idx-RowSize] ) {
		    different_terrains = true;
		}
	    }
	    if ( z >= 1 && x < RowSize - 1 ) {
		idx += 1;
		if ( Terrain[idx] != Terrain[idx-RowSize] ) {
		    different_terrains = true;
		}
	    }

	    if ( different_terrains ) {
		ForceEastVert = true;
		terrain_error = TERRAIN_ERROR_SCALE * whole * whole;
	    } else {
		ForceEastVert = false;
	    }

	    if ( terrain_error > Error[1] ) {
		Error[1] = terrain_error;
	    }
	    if ( Error[1] > maxerror ) {
		maxerror = Error[1];
	    }
        }
    }

    // Min/max of edge verts.
    for (i = 0; i < 4; i++) {
	float	y = Vertex[1 + i];
	if (y < MinY) MinY = y;
	if (y > MaxY) MaxY = y;
    }
	
    // Check child squares.
    for (i = 0; i < 4; i++) {
	quadcornerdata	q;
	if (Child[i]) {
	    SetupCornerData(&q, cd, i);
	    Error[i+2] = Child[i]->RecomputeError(q);

	    if (Child[i]->MinY < MinY) MinY = Child[i]->MinY;
	    if (Child[i]->MaxY > MaxY) MaxY = Child[i]->MaxY;
	} else {
	    // Compute difference between bilinear average at child center, and diagonal edge approximation.
	    Error[i+2] = fabs((Vertex[0] + cd.Verts[i]) - (Vertex[i+1] + Vertex[((i+1)&3) + 1])) * 0.25;
	}
	if (Error[i+2] > maxerror) maxerror = Error[i+2];
    }

    //
    // Compute terrain_error
    //
     int terrain;

    //int *terrain_count = new int[(int)num_terrains];

    for (t=0; t<num_terrains; t++) {
		terrain_count[t] = 0;
    }
	
	
	int i_min = (cd.xorg<0) ? 0 : cd.xorg;
	
	int i_max = ( (cd.xorg+whole) >= RowSize) ? (RowSize-1) : cd.xorg+whole;
	
	int j_min = (cd.zorg<0) ? 0 : cd.zorg;
	
	int j_max = ( (cd.zorg+whole) >= NumRows) ? (NumRows-1) : cd.zorg+whole;

	for (i=i_min; i<=i_max; i++) {
	for (j=j_min; j<=j_max; j++) {

	    terrain = (int) Terrain[ i + RowSize*j ];
	    check_assertion( terrain >= 0 && 
			     terrain < int(num_terrains),
			     "Invalid terrain type" );
	    terrain_count[ terrain ] += 1;
	}
    }

    int max_count = 0;
    int max_type = 0;
    int total = 0;
    for (t=0; t<num_terrains; t++) {
	if ( terrain_count[t] > max_count ) {
	    max_count = terrain_count[t];
	    max_type = t;
	}
	total += terrain_count[t];
    }

    //delete [] terrain_count;

    /* Calculate a terrain error that varies between 0 and 1 */
    if ( total > 0 ) {
	terrain_error = (1.0 - max_count / total);  
	if ( num_terrains > 1 ) {
	    terrain_error *= num_terrains / ( num_terrains - 1.0 );
	}
    } else {
	terrain_error = 0;
    }

    /* and scale it by the square area */
    terrain_error *= whole * whole;

    /* and finally scale it so that it's comparable to height error */
    terrain_error *= TERRAIN_ERROR_SCALE;

    if ( terrain_error > maxerror ) {
	maxerror = terrain_error;
    }

    if ( terrain_error > Error[0] ) {
	Error[0] = terrain_error;
    }
    if ( terrain_error > Error[1] ) {
	Error[1] = terrain_error;
    }


    // The error  and MinY/MaxY values for this node and descendants are correct now.
    Dirty = false;
	
    return maxerror;
}


void	quadsquare::ResetTree()
/// Clear all enabled flags, and delete all non-static child nodes.
{
	for (int i = 0; i < 4; i++) {
		if (Child[i]) {
			Child[i]->ResetTree();
			if (Child[i]->Static == false) {
				delete Child[i];
				Child[i] = 0;
	    	}
		}
    }
    EnabledFlags = 0;
    SubEnabledCount[0] = 0;
    SubEnabledCount[1] = 0;
    Dirty = true;
}


void	quadsquare::StaticCullData(const quadcornerdata& cd, const float ThresholdDetail)
/// Examine the tree and remove nodes which don't contain necessary
/// detail.  Necessary detail is defined as vertex data with a
/// edge-length to height ratio less than ThresholdDetail.
{
    // First, clean non-static nodes out of the tree.
    ResetTree();

    // Make sure error values are up-to-date.
    if (Dirty) RecomputeError(cd);
	
    // Recursively check all the nodes and do necessary removal.
    // We must start at the bottom of the tree, and do one level of
    // the tree at a time, to ensure the dependencies are accounted
    // for properly.
    for (int level = 0; level <= cd.Level; level++) {
		StaticCullAux(cd, ThresholdDetail, level);
    }
}


void	quadsquare::StaticCullAux(const quadcornerdata& cd, const float ThresholdDetail, const int TargetLevel)
/// Check this node and its descendents, and remove nodes which don't contain
/// necessary detail.
{
    int	i;
    quadcornerdata	q;

    if (cd.Level > TargetLevel) {
		// Just recurse to child nodes.
		for (int j = 0; j < 4; j++) {
			if (j < 2) i = 1 - j;
			else i = j;

			if (Child[i]) {
				SetupCornerData(&q, cd, i);
				Child[i]->StaticCullAux(q, ThresholdDetail, TargetLevel);
			}
		}
		return;
    }

    // We're at the target level.  Check this node to see if it's OK to delete it.

    // Check edge vertices to see if they're necessary.
    float size = 2 << cd.Level;	// Edge length.
    if (Child[0] == NULL && Child[3] == NULL && Error[0] * ThresholdDetail < size) {
		quadsquare*	s = GetNeighbor(0, cd);
		if (s == NULL || (s->Child[1] == NULL && s->Child[2] == NULL)){

			// Force vertex height to the edge value.
			float	y = (cd.Verts[0] + cd.Verts[3]) * 0.5;
			Vertex[1] = y;
			Error[0] = 0;
			
			// Force alias vertex to match.
			if (s) s->Vertex[3] = y;
			
			Dirty = true;
		}
    }

    if (Child[2] == NULL && Child[3] == NULL && Error[1] * ThresholdDetail < size) {
	quadsquare*	s = GetNeighbor(3, cd);
	if (s == NULL || (s->Child[0] == NULL && s->Child[1] == NULL)) {
	    float	y = (cd.Verts[2] + cd.Verts[3]) * 0.5;
	    Vertex[4] = y;
	    Error[1] = 0;
			
	    if (s) s->Vertex[2] = y;
			
	    Dirty = true;
	}
    }

    // See if we have child nodes.
    bool	StaticChildren = false;
    for (i = 0; i < 4; i++) {
	if (Child[i]) {
	    StaticChildren = true;
	    if (Child[i]->Dirty) Dirty = true;
	}
    }

    // If we have no children and no necessary edges, then see if we can delete ourself.
    if (StaticChildren == false && cd.Parent != NULL) {
	bool	NecessaryEdges = false;
	for (i = 0; i < 4; i++) {
	    // See if vertex deviates from edge between corners.
	    float	diff = fabs(Vertex[i+1] - (cd.Verts[i] + cd.Verts[(i+3)&3]) * 0.5);
	    if (diff > 0.00001) {
		NecessaryEdges = true;
	    }
	}

	if (!NecessaryEdges) {
	    size *= 1.414213562;	// sqrt(2), because diagonal is longer than side.
	    if (cd.Parent->Square->Error[2 + cd.ChildIndex] * ThresholdDetail < size) {
		delete cd.Parent->Square->Child[cd.ChildIndex];	// Delete this.
		cd.Parent->Square->Child[cd.ChildIndex] = 0;	// Clear the pointer.
	    }
	}
    }
}


int	MaxCreateDepth = 0;


void
quadsquare::EnableEdgeVertex(int index, const bool IncrementCount, const quadcornerdata& cd)
/// Enable the specified edge vertex.  Indices go { e, n, w, s }.
/// Increments the appropriate reference-count if IncrementCount is true.
{
    if ((EnabledFlags & (1 << index)) && IncrementCount == false) return;
	
    // Turn on flag and deal with reference count.
    EnabledFlags |= 1 << index;
    if (IncrementCount == true && (index == 0 || index == 3)) {
		SubEnabledCount[index & 1]++;
    }

    // Now we need to enable the opposite edge vertex of the adjacent square (i.e. the alias vertex).

    // This is a little tricky, since the desired neighbor node may not exist, in which
    // case we have to create it, in order to prevent cracks.  Creating it may in turn cause
    // further edge vertices to be enabled, propagating updates through the tree.

    // The sticking point is the quadcornerdata list, which
    // conceptually is just a linked list of activation structures.
    // In this function, however, we will introduce branching into
    // the "list", making it in actuality a tree.  This is all kind
    // of obscure and hard to explain in words, but basically what
    // it means is that our implementation has to be properly
    // recursive.

    // Travel upwards through the tree, looking for the parent in common with our desired neighbor.
    // Remember the path through the tree, so we can travel down the complementary path to get to the neighbor.
    quadsquare*	p = this;
    const quadcornerdata* pcd = &cd;
    int	ct = 0;
    int	stack[32];
    for (;;) {
		int	ci = pcd->ChildIndex;

		if (pcd->Parent == NULL || pcd->Parent->Square == NULL) {
		    // Neighbor doesn't exist (it's outside the tree), so there's no alias vertex to enable.
		    return;
		}
		p = pcd->Parent->Square;
		pcd = pcd->Parent;

		bool SameParent = ((index - ci) & 2) ? true : false;
		
		ci = ci ^ 1 ^ ((index & 1) << 1);	// Child index of neighbor node.

		stack[ct] = ci;
		ct++;
		
		if (SameParent) break;
    }

    // Get a pointer to our neighbor (create if necessary), by walking down
    // the quadtree from our shared ancestor.
    p = p->EnableDescendant(ct, stack, *pcd);
	
/*
  // Travel down the tree towards our neighbor, enabling and creating nodes as necessary.  We'll
  // follow the complement of the path we used on the way up the tree.
  quadcornerdata	d[16];
  int	i;
  for (i = 0; i < ct; i++) {
  int	ci = stack[ct-i-1];

  if (p->Child[ci] == NULL && CreateDepth == 0) CreateDepth = ct-i;	//xxxxxxx
		
  if ((p->EnabledFlags & (16 << ci)) == 0) {
  p->EnableChild(ci, *pcd);
  }
  p->SetupCornerData(&d[i], *pcd, ci);
  p = p->Child[ci];
  pcd = &d[i];
  }
*/

    // Finally: enable the vertex on the opposite edge of our neighbor, the alias of the original vertex.
    index ^= 2;
    p->EnabledFlags |= (1 << index);
    if (IncrementCount == true && (index == 0 || index == 3)) {
		p->SubEnabledCount[index & 1]++;
    }
}


quadsquare*	quadsquare::EnableDescendant(int count, int path[], const quadcornerdata& cd)
/// This function enables the descendant node 'count' generations below
/// us, located by following the list of child indices in path[].
/// Creates the node if necessary, and returns a pointer to it.
{
    count--;
    int	ChildIndex = path[count];

    if ((EnabledFlags & (16 << ChildIndex)) == 0) {
		EnableChild(ChildIndex, cd);
    }
	
    if (count > 0) {
		quadcornerdata q;
		SetupCornerData(&q, cd, ChildIndex);
		return Child[ChildIndex]->EnableDescendant(count, path, q);
    } else {
		return Child[ChildIndex];
    }
}


void
quadsquare::CreateChild(int index, const quadcornerdata& cd)
/// Creates a child square at the specified index.
{
    if (Child[index] == 0) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, index);
		Child[index] = new quadsquare(&q);
    }
}


void
quadsquare::EnableChild(int index, const quadcornerdata& cd)
/// Enable the indexed child node.  { ne, nw, sw, se }
/// Causes dependent edge vertices to be enabled.
{
    if ((EnabledFlags & (16 << index)) == 0) {
		EnabledFlags |= (16 << index);
		EnableEdgeVertex(index, true, cd);
		EnableEdgeVertex((index + 1) & 3, true, cd);
		if (Child[index] == 0) {
		    CreateChild(index, cd);
		}
    }
}


int	BlockDeleteCount = 0;	//xxxxx
int	BlockUpdateCount = 0;	//xxxxx


void
quadsquare::NotifyChildDisable(const quadcornerdata& cd, int index)
/// Marks the indexed child quadrant as disabled.  Deletes the child node
/// if it isn't static.
{
    // Clear enabled flag for the child.
    EnabledFlags &= ~(16 << index);
	
	// Update child enabled counts for the affected edge verts.
    quadsquare*	s;
	
    if (index & 2) s = this;
    else s = GetNeighbor(1, cd);
    if (s) {
		s->SubEnabledCount[1]--;
    }
	
    if (index == 1 || index == 2) s = GetNeighbor(2, cd);
    else s = this;
    if (s) {
		s->SubEnabledCount[0]--;
    }
	
    /* We don't really want to delete nodes when they're disabled, do we?     
	if (Child[index]->Static == false) {
       delete Child[index];
       Child[index] = 0;

       BlockDeleteCount++;//xxxxx
       }
     */
}


static float DetailThreshold = 100;


bool
quadsquare::VertexTest(int x, float y, int z, float error, const float Viewer[3], int level, vertex_loc_t vertex_loc )
/// Returns true if the vertex at (x,z) with the given world-space error between
/// its interpolated location and its true location, should be enabled, given that
/// the viewpoint is located at Viewer[].
{
    float	dx = fabs(x - Viewer[0]) * fabs( ScaleX );
    float	dy = fabs(y - Viewer[1]);
    float	dz = fabs(z - Viewer[2]) * fabs( ScaleZ );
    float	d = MAX( dx, MAX( dy, dz ) );


    if ( vertex_loc == South && ForceSouthVert && d < VERTEX_FORCE_THRESHOLD ) 
    {
		return true;
    }
    if ( vertex_loc == East && ForceEastVert && d < VERTEX_FORCE_THRESHOLD ) 
    {
		return true;
    }
    if ( d < ERROR_MAGNIFICATION_THRESHOLD ) {
		error *= ERROR_MAGNIFICATION_AMOUNT;
    }

    return error * DetailThreshold  > d;
}


bool
quadsquare::BoxTest(int x, int z, float size, float miny, float maxy, float error, const float Viewer[3])
/// Returns true if any vertex within the specified box (origin at x,z,
/// edges of length size) with the given error value could be enabled
/// based on the given viewer location.
{
    // Find the minimum distance to the box.
    float	half = size * 0.5;
    float	dx = ( fabs(x + half - Viewer[0]) - half ) * fabs(ScaleX);
    float	dy = fabs((miny + maxy) * 0.5 - Viewer[1]) - (maxy - miny) * 0.5;
    float	dz = ( fabs(z + half - Viewer[2]) - half ) * fabs(ScaleZ);
    float	d = MAX( dx, MAX( dy , dz ) );

    if ( d < ERROR_MAGNIFICATION_THRESHOLD ) {
		error *= ERROR_MAGNIFICATION_AMOUNT;
    }

    if ( error * DetailThreshold > d ) {
		return true;
    }

    // Check to see if this box crosses the heightmap boundary
    if ( ( x < RowSize-1 && x+size >= RowSize ) ||
	 ( z < NumRows-1 && z+size >= NumRows ) ) 
    {
		return true;
    }

    return false;
}

void
quadsquare::Update(const quadcornerdata& cd, const float ViewerLocation[3], const float Detail)
/// Refresh the vertex enabled states in the tree, according to the
/// location of the viewer.  May force creation or deletion of qsquares
/// in areas which need to be interpolated.
{

    float Viewer[3];

    DetailThreshold = Detail;

    Viewer[0] = ViewerLocation[0] / ScaleX;
    Viewer[1] = ViewerLocation[1];
    Viewer[2] = ViewerLocation[2] / ScaleZ;
	
    UpdateAux(cd, Viewer, 0, SomeClip);
}


void
quadsquare::UpdateAux(const quadcornerdata& cd, const float ViewerLocation[3], const float CenterError, clip_result_t vis )
/// Does the actual work of updating enabled states and tree growing/shrinking.
{
    BlockUpdateCount++;	//xxxxx

    check_assertion( vis != NotVisible, "Invalid visibility value" );
    if ( vis != NoClip ) {
		vis = ClipSquare( cd );
		if ( vis == NotVisible ) {
	    	return;
		}
    }
	
    // Make sure error values are current.
    if (Dirty) {
		RecomputeError(cd);
    }

    const int half = 1 << cd.Level;
    const int whole = half << 1;

    // See about enabling child verts.

    // East vert.
    if ( (EnabledFlags & 1) == 0 && 
	 VertexTest(cd.xorg + whole, Vertex[1], cd.zorg + half, 
		    Error[0], ViewerLocation, cd.Level, East) == true ) 
    {
		EnableEdgeVertex(0, false, cd);	
    }

    // South vert.
    if ( (EnabledFlags & 8) == 0 && 
	 VertexTest(cd.xorg + half, Vertex[4], cd.zorg + whole, 
		    Error[1], ViewerLocation, cd.Level, South) == true ) 
    {
		EnableEdgeVertex(3, false, cd);	
    }

    if (cd.Level > 0) {
	if ((EnabledFlags & 32) == 0) {
	    if (BoxTest(cd.xorg, cd.zorg, half, MinY, MaxY, Error[3], ViewerLocation) == true) EnableChild(1, cd);	// nw child.er
	}
	if ((EnabledFlags & 16) == 0) {
	    if (BoxTest(cd.xorg + half, cd.zorg, half, MinY, MaxY, Error[2], ViewerLocation) == true) EnableChild(0, cd);	// ne child.
	}
	if ((EnabledFlags & 64) == 0) {
	    if (BoxTest(cd.xorg, cd.zorg + half, half, MinY, MaxY, Error[4], ViewerLocation) == true) EnableChild(2, cd);	// sw child.
	}
	if ((EnabledFlags & 128) == 0) {
	    if (BoxTest(cd.xorg + half, cd.zorg + half, half, MinY, MaxY, Error[5], ViewerLocation) == true) EnableChild(3, cd);	// se child.
	}
		
	// Recurse into child quadrants as necessary.
	quadcornerdata	q;
		
	if (EnabledFlags & 32) {
	    SetupCornerData(&q, cd, 1);
	    Child[1]->UpdateAux(q, ViewerLocation, Error[3], vis);
	}
	if (EnabledFlags & 16) {
	    SetupCornerData(&q, cd, 0);
	    Child[0]->UpdateAux(q, ViewerLocation, Error[2], vis);
	}
	if (EnabledFlags & 64) {
	    SetupCornerData(&q, cd, 2);
	    Child[2]->UpdateAux(q, ViewerLocation, Error[4], vis);
	}
	if (EnabledFlags & 128) {
	    SetupCornerData(&q, cd, 3);
	    Child[3]->UpdateAux(q, ViewerLocation, Error[5], vis);
	}
    }
	
    // Test for disabling.  East, South, and center.
    if ( (EnabledFlags & 1) && 
	 SubEnabledCount[0] == 0 && 
	 VertexTest(cd.xorg + whole, Vertex[1], cd.zorg + half, 
		    Error[0], ViewerLocation, cd.Level, East) == false) 
    {
		EnabledFlags &= ~1;
		quadsquare*	s = GetNeighbor(0, cd);
		if (s) s->EnabledFlags &= ~4;
    }

    if ( (EnabledFlags & 8) && 
	 SubEnabledCount[1] == 0 && 
	 VertexTest(cd.xorg + half, Vertex[4], cd.zorg + whole, 
		    Error[1], ViewerLocation, cd.Level, South) == false) 
    {
		EnabledFlags &= ~8;
		quadsquare*	s = GetNeighbor(3, cd);
		if (s) s->EnabledFlags &= ~2;
    }

    if (EnabledFlags == 0 &&
	cd.Parent != NULL &&
	BoxTest(cd.xorg, cd.zorg, whole, MinY, MaxY, CenterError, 
		ViewerLocation) == false)
    {
	// Disable ourself.
		cd.Parent->Square->NotifyChildDisable(*cd.Parent, cd.ChildIndex);	// nb: possibly deletes 'this'.
    }
}

GLuint VertexIndices[9];
int VertexTerrains[9];

//void
inline int
quadsquare::InitVert(const int i, const int x,const int z)
/// Initializes the indexed vertex of VertexArray[] with the
/// given values.
{
    const int idx = (x >= RowSize ? (RowSize - 1) : x) + RowSize * (  z >= NumRows ? (NumRows - 1)  : z);

    VertexIndices[i] = idx;

	return (VertexTerrains[i] = Terrain[idx]);
}

GLubyte *VNCArray;

void
quadsquare::DrawTris(int terrain)
{
    int tmp_min_idx = VertexArrayMinIdx[terrain];
    if ( glLockArraysEXT_p && getparam_use_cva()) {

	if ( getparam_cva_hack() ) {
	    /* This is a hack that seems to fix the "psychedelic colors" on 
	       some drivers (TNT/TNT2, for example)
	     */
	    if ( tmp_min_idx == 0 ) {
		tmp_min_idx = 1;
	    }
	} 
	
	glLockArraysEXT_p( tmp_min_idx, 
			 VertexArrayMaxIdx[terrain] - tmp_min_idx + 1 ); 
    }

    glDrawElements( GL_TRIANGLES, VertexArrayCounter[terrain],
		    GL_UNSIGNED_INT, VertexArrayIndices[terrain] );

    if ( glUnlockArraysEXT_p && getparam_use_cva()) {
		glUnlockArraysEXT_p();
    }
}

void
quadsquare::DrawEnvmapTris(GLuint MapTexId, int terrain) 
{
    if ( VertexArrayCounter[terrain] > 0 ) {
	
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

		glBindTexture( GL_TEXTURE_2D, MapTexId);

		DrawTris(terrain);

		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	} 
}

void
quadsquare::InitArrayCounters()
{
    for (unsigned int i=0; i<num_terrains ; i++){
		VertexArrayCounter[i] = 0;
		VertexArrayMinIdx[i] = INT_MAX;
		VertexArrayMaxIdx[i] = 0;
	}
}

void
quadsquare::Render(const quadcornerdata& cd, GLubyte *vnc_array)
/// Draws the heightfield represented by this tree.
{
    VNCArray = vnc_array;
    const bool fog_on=fogPlane.isEnabled();
    int i,idx;
    int nx, ny;
    get_course_divisions( &nx, &ny );

    /*
     * Draw the "normal" blended triangles
     */

	InitArrayCounters();
	RenderAux(cd, SomeClip);

	
	//for (j=0; j<(int)num_terrains; j++) {
	
	std::list<int>::iterator it;
	for (it=usedTerrains.begin(); it!=usedTerrains.end(); it++) {
		
		if ( VertexArrayCounter[(*it)] == 0 ) {
			continue;
		} 

		for (i=0; i<(int)VertexArrayCounter[(*it)];i++){
			idx = VertexArrayIndices[(*it)][i];
			//colorval(idx, 3) =  ( (*it)<= Terrain[idx] ) ? 255 : 0;
			colorval(idx, 3) =  ( terrain_texture[(*it)].wheight<= terrain_texture[Terrain[idx]].wheight ) ? 255 : 0;
		}

		glBindTexture( GL_TEXTURE_2D, terrain_texture[(*it)].texbind );
		DrawTris((*it));

		if ( terrain_texture[(*it)].envmapbind != 0  && getparam_terrain_envmap() ) {
		    /* Render Ice with environment map */
		    glDisableClientState( GL_COLOR_ARRAY );
		    glColor4f( 1.0, 1.0, 1.0, ENV_MAP_ALPHA / 255.0 );

		    DrawEnvmapTris(terrain_texture[(*it)].envmapbind, (*it));	

		    glEnableClientState( GL_COLOR_ARRAY );
		}

    }

    /*
     * Draw the "special" triangles that have different terrain types
     * at each of the corners 
     */
    if ( getparam_terrain_blending() &&
	 getparam_perfect_terrain_blending()) {

	/*
	 * Get the "special" three-terrain triangles
	 */
	InitArrayCounters();
	RenderAuxSpezial( cd, SomeClip);
	
	if ( VertexArrayCounter[0] != 0 ) {
	    /* Render black triangles */
	    glDisable( GL_FOG );
	    
	    /* Set triangle vertices to black */
	    for (i=0; i<(int)VertexArrayCounter[0]; i++) {
			colorval( VertexArrayIndices[0][i], 0 ) = 0;
			colorval( VertexArrayIndices[0][i], 1 ) = 0;
			colorval( VertexArrayIndices[0][i], 2 ) = 0;
			colorval( VertexArrayIndices[0][i], 3 ) = 255;
	    }
	    
	    /* Draw the black triangles */
	    glBindTexture( GL_TEXTURE_2D, terrain_texture[0].texbind);
	    DrawTris(0);
	    
	    /* Now we draw the triangle once for each texture */
	    if (fog_on) {
			glEnable( GL_FOG );
	    }

	    /* Use additive blend function */
	    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	    /* First set triangle colors to white */
	    for (i=0; i<(int)VertexArrayCounter[0]; i++) {
			colorval( VertexArrayIndices[0][i], 0 ) = 255;
			colorval( VertexArrayIndices[0][i], 1 ) = 255;
			colorval( VertexArrayIndices[0][i], 2 ) = 255;
	    }

		//for (int j=0; j<(int)num_terrains; j++) {
		for(it=usedTerrains.begin(); it != usedTerrains.end(); it++){
			
			glBindTexture( GL_TEXTURE_2D, terrain_texture[(*it)].texbind);

			/* Set alpha values */
			for (i=0; i<(int)VertexArrayCounter[0]; i++) {
				colorval( VertexArrayIndices[0][i], 3 ) = 
				(Terrain[VertexArrayIndices[0][i]] == int(*it) ) ? 
				255 : 0;
			}

			DrawTris(0);
	    }


	    /* Render Ice with environment map */
	    if ( getparam_terrain_envmap() ) {
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	    
		/* Need to set alpha values for ice */
			
		/*
		for (i=0; i<VertexArrayCounter; i++) {
		    colorval( VertexArrayIndices[i], 3 ) = 
			(Terrain[VertexArrayIndices[i]] == Ice) ? 
			ENV_MAP_ALPHA : 0;
		}
		*/
			
		//the ice stuff should be rewritten...
		for (i=0; i<(int)VertexArrayCounter[0]; i++) {
		    colorval( VertexArrayIndices[0][i], 3 ) = 
			(Terrain[VertexArrayIndices[0][i]] == 0) ? 
			ENV_MAP_ALPHA : 0;
		}			
			
		DrawEnvmapTris(getparam_terrain_envmap(),0);
	    }
	}
    }

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

clip_result_t
quadsquare::ClipSquare( const quadcornerdata& cd )
{
    if ( cd.xorg >= RowSize-1 ) {
		return NotVisible;
    } 

    if ( cd.zorg >= NumRows-1 ) {
		return NotVisible;
    }
	
    const int whole = 2 << cd.Level;
	
	pp::Vec3d Min(cd.xorg*ScaleX, MinY, cd.zorg*ScaleZ);
    pp::Vec3d Max((cd.xorg + whole) * ScaleX, MaxY,(cd.zorg + whole) * ScaleZ);

    /* If the scales are negative we'll need to swap */
    if ( Min.x > Max.x ) {
		float tmp = Min.x;
		Min.x = Max.x;
		Max.x = tmp;
    }

    if ( Min.z > Max.z ) {
		float tmp = Min.z;
		Min.z = Max.z;
		Max.z = tmp;
    }

    const clip_result_t clip_result = clip_aabb_to_view_frustum(Min, Max);

    if ( clip_result == NotVisible || clip_result == SomeClip ) {
		return clip_result;
    }

    if ( cd.xorg + whole >= RowSize ) {
		return SomeClip;
    }
    if ( cd.zorg + whole >= NumRows ) {
		return SomeClip;
    }

    return clip_result;
}


typedef void (*make_tri_func_t)( int a, int b, int c, int terrain );

/* Local macro for setting alpha value based on terrain */
#define setalphaval(i) colorval(VertexIndices[i], 3) = \
    ( terrain <= VertexTerrains[i] ) ? 255 : 0 

#define update_min_max( idx,terrain ) \
    if ( idx > VertexArrayMaxIdx[terrain] ) { \
        VertexArrayMaxIdx[terrain] = idx; \
    } \
    if ( idx < VertexArrayMinIdx[terrain] ) { \
        VertexArrayMinIdx[terrain] = idx; \
    }
	
inline void quadsquare::MakeTri( int a, int b, int c, int terrain )
{
    if ( ( VertexTerrains[a] == terrain || 
	   VertexTerrains[b] == terrain || 
	   VertexTerrains[c] == terrain ) )
    { 
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[a]; 
	//setalphaval(a); 
	update_min_max( VertexIndices[a], terrain );
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[b]; 
	//setalphaval(b); 
	update_min_max( VertexIndices[b], terrain );
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[c]; 
	//setalphaval(c); 
	update_min_max( VertexIndices[c], terrain );
    }
}


inline void quadsquare::MakeSpecialTri( int a, int b, int c) 
{
    if ( VertexTerrains[a] != VertexTerrains[b] && 
	 VertexTerrains[a] != VertexTerrains[c] && 
	 VertexTerrains[b] != VertexTerrains[c] ) 
    { 
	VertexArrayIndices[0][VertexArrayCounter[0]++] = VertexIndices[a]; 
	update_min_max( VertexIndices[a],0 );
	VertexArrayIndices[0][VertexArrayCounter[0]++] = VertexIndices[b]; 
	update_min_max( VertexIndices[b],0 );
	VertexArrayIndices[0][VertexArrayCounter[0]++] = VertexIndices[c]; 
	update_min_max( VertexIndices[c],0 );
    }
}

inline void quadsquare::MakeNoBlendTri( int a, int b, int c, int terrain )
{
    if ( ( VertexTerrains[a] == terrain || 
	   VertexTerrains[b] == terrain || 
	   VertexTerrains[c] == terrain ) &&
	 ( VertexTerrains[a] >= terrain &&
	   VertexTerrains[b] >= terrain &&
	   VertexTerrains[c] >= terrain ) )
    { 
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[a]; 
	setalphaval(a); 
	update_min_max( VertexIndices[a],terrain );
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[b]; 
	setalphaval(b);
	update_min_max( VertexIndices[b],terrain );
	VertexArrayIndices[terrain][VertexArrayCounter[terrain]++] = VertexIndices[c];
	setalphaval(c);
	update_min_max( VertexIndices[c],terrain );
    }
}


static bool terraintest[NUM_TERRAIN_TYPES];

void
quadsquare::RenderAux(const quadcornerdata& cd, clip_result_t vis)
/// Does the work of rendering this square.  Uses the enabled vertices only.
/// Recurses as necessary.
{
    int	flags = 0;
    int	mask = 1;
    quadcornerdata q;
	
    for (int i = 0; i < 4; i++, mask <<= 1) {
		if (EnabledFlags & (16 << i)) {
			SetupCornerData(&q, cd, i);
			if (vis != NoClip) {
				if ((vis = ClipSquare( q ))!= NotVisible ) {
		 		   Child[i]->RenderAux(q, vis);
				}
   		 	}else{		
		   		Child[i]->RenderAux(q, vis);
			}
		} else {
		    flags |= mask;
		}
    }
    if (flags == 0) return;

	
    // Init vertex data.  Here's a diagram of what's going on.
    // Yes, this diagram is fucked, but that's because things are mirrored 
    // in the z axis for us (z coords are actually -z coords).
    //
    // (top of course) 
    //        N                
    //    +---+---+ (xorg, zorg)
    //   2|  3   4|        
    //    |(1) (2)|
    // E  +   +   +  W
    //   1|  0   5|
    //    |(8) (4)|
    //    +---+---+
    //   8   7   6
    //        S
    // (bottom of course)
    //
    // Values in parens are bitmask values for the corresponding child squares
    //

	    // Make the list of triangles to draw.
#define make_tri_list(tri_func,terrain) \
    if ((EnabledFlags & 1) == 0 ) { \
	tri_func(0, 2, 8, terrain); \
    } else { \
	if (flags & 8) tri_func(0, 1, 8, terrain); \
	if (flags & 1) tri_func(0, 2, 1, terrain); \
    } \
    if ((EnabledFlags & 2) == 0 ) {  \
	tri_func(0, 4, 2, terrain);  \
    } else { \
	if (flags & 1) tri_func(0, 3, 2, terrain); \
	if (flags & 2) tri_func(0, 4, 3, terrain); \
    } \
    if ((EnabledFlags & 4) == 0 ) { \
	tri_func(0, 6, 4, terrain); \
    } else { \
	if (flags & 2) tri_func(0, 5, 4, terrain); \
	if (flags & 4) tri_func(0, 6, 5, terrain); \
    } \
    if ((EnabledFlags & 8) == 0 ) { \
	tri_func(0, 8, 6, terrain); \
    } else { \
	if (flags & 4) tri_func(0, 7, 6, terrain); \
	if (flags & 8) tri_func(0, 8, 7, terrain); \
    }

	
	{
		//bool terraintest[num_terrains];
		for (unsigned int i=0; i<num_terrains; i++) terraintest[i]=false;
		
		const int	half = 1 << cd.Level;
    	const int	whole = 2 << cd.Level;

    	terraintest[ InitVert(0, cd.xorg + half, cd.zorg + half) ] = true;
    	terraintest[ InitVert(1, cd.xorg + whole, cd.zorg + half) ] = true;
    	terraintest[ InitVert(2, cd.xorg + whole, cd.zorg) ] = true;
    	terraintest[ InitVert(3, cd.xorg + half, cd.zorg) ] = true;
    	terraintest[ InitVert(4, cd.xorg, cd.zorg) ] = true;
    	terraintest[ InitVert(5, cd.xorg, cd.zorg + half) ] = true;
		terraintest[ InitVert(6, cd.xorg, cd.zorg + whole) ] = true;
	    terraintest[ InitVert(7, cd.xorg + half, cd.zorg + whole) ] = true;
	    terraintest[ InitVert(8, cd.xorg + whole, cd.zorg + whole) ] = true;
	
		for (unsigned int j=0; j<num_terrains; j++) {
			 if (terraintest[j]==true){			 
			 	if ( getparam_terrain_blending() ){make_tri_list(MakeTri,j);}
				else{make_tri_list(MakeNoBlendTri,j);}
			 }
		} 
	}
}


void
quadsquare::RenderAuxSpezial(const quadcornerdata& cd, clip_result_t vis)
/// Does the work of rendering this square.  Uses the enabled vertices only.
/// Recurses as necessary.
{
    /*
	// If this square is outside the frustum, then don't render it.
  
	//if (vis != NoClip) {

		if ((vis = ClipSquare( cd ))== NotVisible ) {
	    // This square is completely outside the view frustum.
		    return;
		}
	// else vis is either NoClip or SomeClip.  If it's NoClip, then child
	// squares won't have to bother with the frustum check.
    }
	*/
	
    int	flags = 0;
    int	mask = 1;
    quadcornerdata q;
	
    for (int i = 0; i < 4; i++, mask <<= 1) {
		if (EnabledFlags & (16 << i)) {
			SetupCornerData(&q, cd, i);
			if (vis != NoClip) {
				if ((vis = ClipSquare( q ))!= NotVisible ) {
		 		   Child[i]->RenderAuxSpezial(q, vis);
				}
   		 	}else{		
		   		Child[i]->RenderAuxSpezial(q, vis);
			}
		} else {
		    flags |= mask;
		}
    }
    if (flags == 0) return;

	
    // Init vertex data.  Here's a diagram of what's going on.
    // Yes, this diagram is fucked, but that's because things are mirrored 
    // in the z axis for us (z coords are actually -z coords).
    //
    // (top of course) 
    //        N                
    //    +---+---+ (xorg, zorg)
    //   2|  3   4|        
    //    |(1) (2)|
    // E  +   +   +  W
    //   1|  0   5|
    //    |(8) (4)|
    //    +---+---+
    //   8   7   6
    //        S
    // (bottom of course)
    //
    // Values in parens are bitmask values for the corresponding child squares
    //

	

	int terraintest=0;
	{
		const int	half = 1 << cd.Level;
    	const int	whole = 2 << cd.Level;

    	terraintest += InitVert(0, cd.xorg + half, cd.zorg + half);
    	terraintest += InitVert(1, cd.xorg + whole, cd.zorg + half);
    	terraintest += InitVert(2, cd.xorg + whole, cd.zorg);
    	terraintest += InitVert(3, cd.xorg + half, cd.zorg);
    	terraintest += InitVert(4, cd.xorg, cd.zorg);
    	terraintest += InitVert(5, cd.xorg, cd.zorg + half);
		terraintest += InitVert(6, cd.xorg, cd.zorg + whole);
	    terraintest += InitVert(7, cd.xorg + half, cd.zorg + whole);
	    terraintest += InitVert(8, cd.xorg + whole, cd.zorg + whole);
	}
		
		
    // Make the list of triangles to draw.
#define make_spezialtri_list(tri_func) \
    if ((EnabledFlags & 1) == 0 ) { \
	tri_func(0, 2, 8); \
    } else { \
	if (flags & 8) tri_func(0, 1, 8); \
	if (flags & 1) tri_func(0, 2, 1); \
    } \
    if ((EnabledFlags & 2) == 0 ) {  \
	tri_func(0, 4, 2);  \
    } else { \
	if (flags & 1) tri_func(0, 3, 2); \
	if (flags & 2) tri_func(0, 4, 3); \
    } \
    if ((EnabledFlags & 4) == 0 ) { \
	tri_func(0, 6, 4); \
    } else { \
	if (flags & 2) tri_func(0, 5, 4); \
	if (flags & 4) tri_func(0, 6, 5); \
    } \
    if ((EnabledFlags & 8) == 0 ) { \
	tri_func(0, 8, 6); \
    } else { \
	if (flags & 4) tri_func(0, 7, 6); \
	if (flags & 8) tri_func(0, 8, 7); \
    }
	
	//if (terraintest>0){
		make_spezialtri_list(MakeSpecialTri);	
	//}
}



void
quadsquare::SetupCornerData(quadcornerdata* q, const quadcornerdata& cd, const int ChildIndex)
/// Fills the given structure with the appropriate corner values for the
/// specified child block, given our own vertex data and our corner
/// vertex data from cd.
//
// ChildIndex mapping:
// +-+-+
// |1|0|
// +-+-+
// |2|3|
// +-+-+
//
// Verts mapping:
// 1-0
// | |
// 2-3
//
// Vertex mapping:
// +-2-+
// | | |
// 3-0-1
// | | |
// +-4-+
{
    const int half = 1 << cd.Level;

    q->Parent = &cd;
    q->Square = Child[ChildIndex];
    q->Level = cd.Level - 1;
    q->ChildIndex = ChildIndex;
	
    switch (ChildIndex) {
    default:
    case 0:
	q->xorg = cd.xorg + half;
	q->zorg = cd.zorg;
	q->Verts[0] = cd.Verts[0];
	q->Verts[1] = Vertex[2];
	q->Verts[2] = Vertex[0];
	q->Verts[3] = Vertex[1];
	break;

    case 1:
	q->xorg = cd.xorg;
	q->zorg = cd.zorg;
	q->Verts[0] = Vertex[2];
	q->Verts[1] = cd.Verts[1];
	q->Verts[2] = Vertex[3];
	q->Verts[3] = Vertex[0];
	break;

    case 2:
	q->xorg = cd.xorg;
	q->zorg = cd.zorg + half;
	q->Verts[0] = Vertex[0];
	q->Verts[1] = Vertex[3];
	q->Verts[2] = cd.Verts[2];
	q->Verts[3] = Vertex[4];
	break;

    case 3:
	q->xorg = cd.xorg + half;
	q->zorg = cd.zorg + half;
	q->Verts[0] = Vertex[1];
	q->Verts[1] = Vertex[0];
	q->Verts[2] = Vertex[4];
	q->Verts[3] = cd.Verts[3];
	break;
    }	
}

int quadsquare::RowSize;
int quadsquare::NumRows;

void
quadsquare::AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm)
/// Sets the height of all samples within the specified rectangular
/// region using the given array of floats.  Extends the tree to the
/// level of detail defined by (1 << hm.Scale) as necessary.
{
    RowSize = hm.RowWidth;
    NumRows = hm.ZSize;

    if ( cd.Parent == NULL ) {
		if ( VertexArrayIndices[0] != NULL ) {
			for (int i=0; i< NUM_TERRAIN_TYPES; i++){
				if (VertexArrayIndices[i]!=NULL)
						delete VertexArrayIndices[i];
			}
		}

	/* Maximum number of triangles is 2 * RowSize * NumRows 
	   This uses up a lot of space but it is a *big* performance gain.
	*/
		for (unsigned int i=0; i<num_terrains; i++){	
			VertexArrayIndices[i] = new GLuint[6 * RowSize * NumRows];
		}
    }

    // If block is outside rectangle, then don't bother.
    int	BlockSize = 2 << cd.Level;
    if (cd.xorg > hm.XOrigin + ((hm.XSize + 2) << hm.Scale) ||
	cd.xorg + BlockSize < hm.XOrigin - (1 << hm.Scale) ||
	cd.zorg > hm.ZOrigin + ((hm.ZSize + 2) << hm.Scale) ||
	cd.zorg + BlockSize < hm.ZOrigin - (1 << hm.Scale))
    {
	// This square does not touch the given height array area; no need to modify this square or descendants.
	return;
    }

    if (cd.Parent && cd.Parent->Square) {
	cd.Parent->Square->EnableChild(cd.ChildIndex, *cd.Parent);	// causes parent edge verts to be enabled, possibly causing neighbor blocks to be created.
    }
	
    int	i;
	
    int	half = 1 << cd.Level;

    // Create and update child nodes.
    for (i = 0; i < 4; i++) {
	quadcornerdata	q;
	SetupCornerData(&q, cd, i);
				
	if (Child[i] == NULL && cd.Level > hm.Scale) {
	    // Create child node w/ current (unmodified) values for corner verts.
	    Child[i] = new quadsquare(&q);
	}
		
	// Recurse.
	if (Child[i]) {
	    Child[i]->AddHeightMap(q, hm);
	}
    }
	
    // Deviate vertex heights based on data sampled from heightmap.
    float	s[5];
    s[0] = hm.Sample(cd.xorg + half, cd.zorg + half);
    s[1] = hm.Sample(cd.xorg + half*2, cd.zorg + half);
    s[2] = hm.Sample(cd.xorg + half, cd.zorg);
    s[3] = hm.Sample(cd.xorg, cd.zorg + half);
    s[4] = hm.Sample(cd.xorg + half, cd.zorg + half*2);

    // Modify the vertex heights if necessary, and set the dirty
    // flag if any modifications occur, so that we know we need to
    // recompute error data later.
    for (i = 0; i < 5; i++) {
		if (s[i] != 0) {
		    Dirty = true;
		    Vertex[i] += s[i];
		}
    }

    if (!Dirty) {
	// Check to see if any child nodes are dirty, and set the dirty flag if so.
	for (i = 0; i < 4; i++) {
	    if (Child[i] && Child[i]->Dirty) {
		Dirty = true;
		break;
	    }
	}
    }

    if (Dirty) SetStatic(cd);
}

float quadsquare::ScaleX;
float quadsquare::ScaleZ;
int* quadsquare::Terrain;
