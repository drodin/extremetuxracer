
#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "textures.h"
#include "course.h"
#include "ogl.h"

#include "quadtree.h"

#define TERRAIN_ERROR_SCALE 0.1
#define VERTEX_FORCE_THRESHOLD 100
#define ERROR_MAGNIFICATION_THRESHOLD 20
#define ERROR_MAGNIFICATION_AMOUNT 3
#define ENV_MAP_ALPHA 50
#define colorval(j,ch) \
	VNCArray[j*STRIDE_GL_ARRAY+STRIDE_GL_ARRAY-4+(ch)]

#define setalphaval(i) colorval(VertexIndices[i], 3) = \
	( terrain <= VertexTerrains[i] ) ? 255 : 0

#define update_min_max( idx ) \
	if ( idx > VertexArrayMaxIdx ) { \
		VertexArrayMaxIdx = idx; \
	} \
	if ( idx < VertexArrayMinIdx ) { \
		VertexArrayMinIdx = idx; \
	}

#define make_tri_list(tri_func) \
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

TTexture* quadsquare::EnvmapTexture = NULL;
GLuint *quadsquare::VertexArrayIndices = NULL;
GLuint quadsquare::VertexArrayCounter;
GLuint quadsquare::VertexArrayMinIdx;
GLuint quadsquare::VertexArrayMaxIdx;

quadsquare::quadsquare (quadcornerdata* pcd) {
	pcd->Square = this;
	Static = false;
	ForceEastVert = false;
	ForceSouthVert = false;
	Dirty = true;

	for (int i = 0; i < 4; i++) {
		Child[i] = (quadsquare*) NULL;
	}

	EnabledFlags = 0;

	for (int i = 0; i < 2; i++) SubEnabledCount[i] = 0;

	Vertex[0].Y = 0.25 * (pcd->Verts[0].Y
		+ pcd->Verts[1].Y + pcd->Verts[2].Y + pcd->Verts[3].Y);
	Vertex[1].Y = 0.5 * (pcd->Verts[3].Y + pcd->Verts[0].Y);
	Vertex[2].Y = 0.5 * (pcd->Verts[0].Y + pcd->Verts[1].Y);
	Vertex[3].Y = 0.5 * (pcd->Verts[1].Y + pcd->Verts[2].Y);
	Vertex[4].Y = 0.5 * (pcd->Verts[2].Y + pcd->Verts[3].Y);

	for (int i = 0; i < 2; i++) Error[i] = 0;
	for (int i = 0; i < 4; i++) {
		Error[i+2] = fabs((Vertex[0].Y + pcd->Verts[i].Y)
			- (Vertex[i+1].Y + Vertex[((i+1)&3) + 1].Y)) * 0.25;
	}

	MinY = MaxY = pcd->Verts[0].Y;
	for (int i = 1; i < 4; i++) {
		float y = pcd->Verts[i].Y;
		if (y < MinY) MinY = y;
		if (y > MaxY) MaxY = y;
	}

	if (pcd->Parent == NULL) {
		EnvmapTexture = Tex.GetTexture (ENV_MAP);
	}
}

quadsquare::~quadsquare() {
	for (int i = 0; i < 4; i++) {
		if (Child[i]) delete Child[i];
	}
}

void quadsquare::SetStatic (const quadcornerdata &cd)
{
	if (Static == false) {
		Static = true;
		if (cd.Parent && cd.Parent->Square) {
			cd.Parent->Square->SetStatic(*cd.Parent);
		}
	}
}

int	quadsquare::CountNodes()
{
	int	count = 1;
	for (int i = 0; i < 4; i++) {
		if (Child[i]) count += Child[i]->CountNodes();
	}
	return count;
}

float quadsquare::GetHeight(const quadcornerdata &cd, float x, float z) {
	int	half = 1 << cd.Level;

	float	lx = (x - cd.xorg) / float(half);
	float	lz = (z - cd.zorg) / float(half);

	int	ix = (int) floor(lx);
	int	iz = (int) floor(lz);

	if (ix < 0) ix = 0;
	if (ix > 1) ix = 1;
	if (iz < 0) iz = 0;
	if (iz > 1) iz = 1;

	int	index = (ix ^ (iz ^ 1)) + (iz << 1);
	if (Child[index] && Child[index]->Static) {
		quadcornerdata q;
		SetupCornerData(&q, cd, index);
		return Child[index]->GetHeight(q, x, z);
	}

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
		s00 = Vertex[2].Y;
		s01 = cd.Verts[0].Y;
		s10 = Vertex[0].Y;
		s11 = Vertex[1].Y;
		break;
	case 1:
		s00 = cd.Verts[1].Y;
		s01 = Vertex[2].Y;
		s10 = Vertex[3].Y;
		s11 = Vertex[0].Y;
		break;
	case 2:
		s00 = Vertex[3].Y;
		s01 = Vertex[0].Y;
		s10 = cd.Verts[2].Y;
		s11 = Vertex[4].Y;
		break;
	case 3:
		s00 = Vertex[0].Y;
		s01 = Vertex[1].Y;
		s10 = Vertex[4].Y;
		s11 = cd.Verts[3].Y;
		break;
	}

	return (s00 * (1-lx) + s01 * lx) * (1 - lz) + (s10 * (1-lx) + s11 * lx) * lz;
}

quadsquare*	quadsquare::GetNeighbor(int dir, const quadcornerdata& cd)
{
	if (cd.Parent == 0) return 0;
	quadsquare*	p = 0;

	int	index = cd.ChildIndex ^ 1 ^ ((dir & 1) << 1);
	bool SameParent = ((dir - cd.ChildIndex) & 2) ? true : false;

	if (SameParent) {
		p = cd.Parent->Square;
	} else {
		p = cd.Parent->Square->GetNeighbor(dir, *cd.Parent);
		if (p == 0) return 0;
	}
	quadsquare*	n = p->Child[index];
	return n;
}

float quadsquare::RecomputeError(const quadcornerdata& cd)
{
	int	half = 1 << cd.Level;
	int	whole = half << 1;
	float terrain_error;
	float maxerror = 0;
	float e;

	size_t numTerr = Course.TerrList.size();
	if (cd.ChildIndex & 1) {
		e = fabs(Vertex[0].Y - (cd.Verts[1].Y + cd.Verts[3].Y) * 0.5);
	} else {
		e = fabs(Vertex[0].Y - (cd.Verts[0].Y + cd.Verts[2].Y) * 0.5);
	}
	if (e > maxerror) maxerror = e;

	MaxY = Vertex[0].Y;
	MinY = Vertex[0].Y;

	for (int i = 0; i < 4; i++) {
		float y = cd.Verts[i].Y;
		if (y < MinY) MinY = y;
		if (y > MaxY) MaxY = y;
	}


	e = fabs(Vertex[1].Y - (cd.Verts[0].Y + cd.Verts[3].Y) * 0.5);
	if (e > maxerror) maxerror = e;
	Error[0] = e;

	e = fabs(Vertex[4].Y - (cd.Verts[2].Y + cd.Verts[3].Y) * 0.5);
	if (e > maxerror) maxerror = e;
	Error[1] = e;

	if (cd.Level == 0 && cd.xorg <= RowSize-1 && cd.zorg <= NumRows-1) {

		int x = cd.xorg + half;
		int z = cd.zorg + whole;
		int idx = x  + z * RowSize;
		bool different_terrains = false;

		terrain_error = 0.f;

		if (x < RowSize && z < NumRows) {

			if (x < RowSize - 1) {
				if (Terrain[idx] != Terrain[idx+1]) {
					different_terrains = true;
				}
			}
			if (z >= 1) {
				idx -= RowSize;
				if (Terrain[idx] != Terrain[idx+1]) {
					different_terrains = true;
				}
			}

			if (different_terrains) {
				ForceSouthVert = true;
				terrain_error = TERRAIN_ERROR_SCALE * whole * whole;
			} else {
				ForceSouthVert = false;
			}

			if (terrain_error > Error[0]) {
				Error[0] = terrain_error;
			}
			if (Error[0] > maxerror) {
				maxerror = Error[0];
			}
		}

		x = cd.xorg + whole;
		z = cd.zorg + half;
		idx = x  + z * RowSize;
		terrain_error = 0;
		different_terrains = false;

		if (x < RowSize && z < NumRows) {

			if (z >= 1) {
				if (Terrain[idx] != Terrain[idx-RowSize]) {
					different_terrains = true;
				}
			}
			if (z >= 1 && x < RowSize - 1) {
				idx += 1;
				if (Terrain[idx] != Terrain[idx-RowSize]) {
					different_terrains = true;
				}
			}

			if (different_terrains) {
				ForceEastVert = true;
				terrain_error = TERRAIN_ERROR_SCALE * whole * whole;
			} else {
				ForceEastVert = false;
			}

			if (terrain_error > Error[1]) {
				Error[1] = terrain_error;
			}
			if (Error[1] > maxerror) {
				maxerror = Error[1];
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		float	y = Vertex[1 + i].Y;
		if (y < MinY) MinY = y;
		if (y > MaxY) MaxY = y;
	}

	for (int i = 0; i < 4; i++) {
		quadcornerdata	q;
		if (Child[i]) {
			SetupCornerData(&q, cd, i);
			Error[i+2] = Child[i]->RecomputeError(q);

			if (Child[i]->MinY < MinY) MinY = Child[i]->MinY;
			if (Child[i]->MaxY > MaxY) MaxY = Child[i]->MaxY;
		} else {
			Error[i+2] = fabs((Vertex[0].Y + cd.Verts[i].Y)
				- (Vertex[i+1].Y + Vertex[((i+1)&3) + 1].Y)) * 0.25;
		}
		if (Error[i+2] > maxerror) maxerror = Error[i+2];
	}

	int terrain;

	int *terrain_count = new int[(int)numTerr];

	for (size_t t=0; t<numTerr; t++) {
		terrain_count[t] = 0;
	}

	for (int i=cd.xorg; i<=cd.xorg+whole; i++) {
		for (int j=cd.zorg; j<=cd.zorg+whole; j++) {

			if (i < 0 || i >= RowSize ||
				j < 0 || j >= NumRows)
			{
				continue;
			}

			terrain = (int) Terrain[ i + RowSize*j ];
			terrain_count[ terrain ] += 1;
		}
	}

	int max_count = 0;
	int total = 0;
	for (size_t t=0; t<numTerr; t++) {
		if (terrain_count[t] > max_count) {
			max_count = terrain_count[t];
		}
		total += terrain_count[t];
	}

	delete [] terrain_count;

	if (total > 0) {
		terrain_error = (1.0 - max_count / total);
		if (numTerr > 1) {
			terrain_error *= numTerr /  (numTerr - 1.0);
		}
	} else terrain_error = 0;
	terrain_error *= whole * whole;
	terrain_error *= TERRAIN_ERROR_SCALE;

	if (terrain_error > maxerror) maxerror = terrain_error;
	if (terrain_error > Error[0]) Error[0] = terrain_error;
	if (terrain_error > Error[1]) Error[1] = terrain_error;

	Dirty = false;

	return maxerror;
}

void quadsquare::ResetTree()
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


void quadsquare::StaticCullData(const quadcornerdata& cd, float ThresholdDetail)
{
	ResetTree();
	if (Dirty) RecomputeError(cd);
	for (int level = 0; level <= cd.Level; level++) {
		StaticCullAux(cd, ThresholdDetail, level);
	}
}


void quadsquare::StaticCullAux(const quadcornerdata& cd, float ThresholdDetail, int TargetLevel)
{
	quadcornerdata	q;

	if (cd.Level > TargetLevel) {
		for (int j = 0; j < 4; j++) {
			int i;
			if (j < 2) i = 1 - j;
			else i = j;

			if (Child[i]) {
				SetupCornerData(&q, cd, i);
				Child[i]->StaticCullAux(q, ThresholdDetail, TargetLevel);
			}
		}
		return;
	}
	float	size = 2 << cd.Level;	// Edge length.
	if (Child[0] == NULL && Child[3] == NULL && Error[0] * ThresholdDetail < size) {
		quadsquare*	s = GetNeighbor(0, cd);
		if (s == NULL || (s->Child[1] == NULL && s->Child[2] == NULL)) {

			float	y = (cd.Verts[0].Y + cd.Verts[3].Y) * 0.5;
			Vertex[1].Y = y;
			Error[0] = 0;
			if (s) s->Vertex[3].Y = y;

			Dirty = true;
		}
	}

	if (Child[2] == NULL && Child[3] == NULL && Error[1] * ThresholdDetail < size) {
		quadsquare*	s = GetNeighbor(3, cd);
		if (s == NULL || (s->Child[0] == NULL && s->Child[1] == NULL)) {
			float	y = (cd.Verts[2].Y + cd.Verts[3].Y) * 0.5;
			Vertex[4].Y = y;
			Error[1] = 0;

			if (s) s->Vertex[2].Y = y;
			Dirty = true;
		}
	}

	bool StaticChildren = false;
	for (int i = 0; i < 4; i++) {
		if (Child[i]) {
			StaticChildren = true;
			if (Child[i]->Dirty) Dirty = true;
		}
	}

	if (StaticChildren == false && cd.Parent != NULL) {
		bool NecessaryEdges = false;
		for (int i = 0; i < 4; i++) {
			float diff = fabs(Vertex[i+1].Y - (cd.Verts[i].Y
				+ cd.Verts[(i+3)&3].Y) * 0.5);
			if (diff > 0.00001) {
				NecessaryEdges = true;
			}
		}

		if (!NecessaryEdges) {
			size *= 1.414213562;
			if (cd.Parent->Square->Error[2 + cd.ChildIndex] * ThresholdDetail < size) {
				delete cd.Parent->Square->Child[cd.ChildIndex];
				cd.Parent->Square->Child[cd.ChildIndex] = 0;
			}
		}
	}
}

int	MaxCreateDepth = 0;

void quadsquare::EnableEdgeVertex(int index, bool IncrementCount, const quadcornerdata& cd) {
	int	ct = 0;
	int	stack[32];


	if ((EnabledFlags & (1 << index)) && IncrementCount == false) return;

	EnabledFlags |= 1 << index;
	if (IncrementCount == true && (index == 0 || index == 3)) {
		SubEnabledCount[index & 1]++;
	}
	quadsquare*	p = this;
	const quadcornerdata* pcd = &cd;
	for (;;) {
		int	ci = pcd->ChildIndex;
		if (pcd->Parent == NULL || pcd->Parent->Square == NULL) return;
		p = pcd->Parent->Square;
		pcd = pcd->Parent;

		bool SameParent = ((index - ci) & 2) ? true : false;

		ci = ci ^ 1 ^ ((index & 1) << 1);

		stack[ct] = ci;
		ct++;

		if (SameParent) break;
	}

	p = p->EnableDescendant(ct, stack, *pcd);

	index ^= 2;
	p->EnabledFlags |= (1 << index);
	if (IncrementCount == true && (index == 0 || index == 3)) {
		p->SubEnabledCount[index & 1]++;
	}
}


quadsquare*	quadsquare::EnableDescendant(int count, int path[], const quadcornerdata& cd)
{
	count--;
	int	ChildIndex = path[count];

	if ((EnabledFlags & (16 << ChildIndex)) == 0) {
		EnableChild(ChildIndex, cd);
	}

	if (count > 0) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, ChildIndex);
		return Child[ChildIndex]->EnableDescendant(count, path, q);
	} else {
		return Child[ChildIndex];
	}
}


void quadsquare::CreateChild(int index, const quadcornerdata& cd)
{
	if (Child[index] == 0) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, index);

		Child[index] = new quadsquare(&q);
	}
}

void quadsquare::EnableChild(int index, const quadcornerdata& cd)
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

int	BlockDeleteCount = 0;
int	BlockUpdateCount = 0;

void quadsquare::NotifyChildDisable(const quadcornerdata& cd, int index)
{
	EnabledFlags &= ~(16 << index);
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
}

static float DetailThreshold = 100;


bool quadsquare::VertexTest(int x, float y, int z, float error,
							const float Viewer[3], int level, vertex_loc_t vertex_loc)
{
	float	dx = fabs(x - Viewer[0]) * fabs (ScaleX);
	float	dy = fabs(y - Viewer[1]);
	float	dz = fabs(z - Viewer[2]) * fabs (ScaleZ);
	float	d = max (dx, max (dy, dz) );

	if (vertex_loc == South && ForceSouthVert && d < VERTEX_FORCE_THRESHOLD)
	{
		return true;
	}
	if (vertex_loc == East && ForceEastVert && d < VERTEX_FORCE_THRESHOLD)
	{
		return true;
	}
	if (d < ERROR_MAGNIFICATION_THRESHOLD) {
		error *= ERROR_MAGNIFICATION_AMOUNT;
	}

	return error * DetailThreshold  > d;
}

bool quadsquare::BoxTest(int x, int z, float size, float miny, float maxy, float error, const float Viewer[3])
{
	float	half = size * 0.5;
	float	dx =  (fabs(x + half - Viewer[0]) - half ) * fabs(ScaleX);
	float	dy = fabs((miny + maxy) * 0.5 - Viewer[1]) - (maxy - miny) * 0.5;
	float	dz =  (fabs(z + half - Viewer[2]) - half ) * fabs(ScaleZ);
	float	d = max (dx, max (dy , dz) );

	if (d < ERROR_MAGNIFICATION_THRESHOLD) {
		error *= ERROR_MAGNIFICATION_AMOUNT;
	}

	if (error * DetailThreshold > d) {
		return true;
	}
	if ( (x < RowSize-1 && x+size >= RowSize) ||
		(z < NumRows-1 && z+size >= NumRows) )
	{
		return true;
	}

	return false;
}

void quadsquare::Update (const quadcornerdata& cd, const float ViewerLocation[3], float Detail)
{
	float Viewer[3];

	DetailThreshold = Detail;
	Viewer[0] = ViewerLocation[0] / ScaleX;
	Viewer[1] = ViewerLocation[1];
	Viewer[2] = ViewerLocation[2] / ScaleZ;
	UpdateAux(cd, Viewer, 0, SomeClip);
}


void quadsquare::UpdateAux (const quadcornerdata& cd,
							const float ViewerLocation[3], float CenterError, clip_result_t vis) {
	BlockUpdateCount++;
	if (vis != NoClip) {
		vis = ClipSquare (cd);

		if (vis == NotVisible) {
			return;
		}
	}
	if (Dirty) {
		RecomputeError(cd);
	}

	int	half = 1 << cd.Level;
	int	whole = half << 1;
	if ((EnabledFlags & 1) == 0 &&
		VertexTest(cd.xorg + whole, Vertex[1].Y, cd.zorg + half,
		Error[0], ViewerLocation, cd.Level, East) == true )
	{
		EnableEdgeVertex(0, false, cd);
	}

	if ((EnabledFlags & 8) == 0 &&
		VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + whole,
		Error[1], ViewerLocation, cd.Level, South) == true )
	{
		EnableEdgeVertex(3, false, cd);
	}

	if (cd.Level > 0) {
		if ((EnabledFlags & 32) == 0) {
			if (BoxTest(cd.xorg, cd.zorg, half, MinY, MaxY, Error[3],
				ViewerLocation) == true) EnableChild(1, cd);
		}
		if ((EnabledFlags & 16) == 0) {
			if (BoxTest(cd.xorg + half, cd.zorg, half, MinY, MaxY,
				Error[2], ViewerLocation) == true) EnableChild(0, cd);
		}
		if ((EnabledFlags & 64) == 0) {
			if (BoxTest(cd.xorg, cd.zorg + half, half, MinY, MaxY,
				Error[4], ViewerLocation) == true) EnableChild(2, cd);
		}
		if ((EnabledFlags & 128) == 0) {
			if (BoxTest(cd.xorg + half, cd.zorg + half, half, MinY, MaxY,
				Error[5], ViewerLocation) == true) EnableChild(3, cd);
		}

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
	if ((EnabledFlags & 1) &&
		SubEnabledCount[0] == 0 &&
		VertexTest(cd.xorg + whole, Vertex[1].Y, cd.zorg + half,
		Error[0], ViewerLocation, cd.Level, East) == false)
	{
		EnabledFlags &= ~1;
		quadsquare*	s = GetNeighbor(0, cd);
		if (s) s->EnabledFlags &= ~4;
	}

	if ((EnabledFlags & 8) &&
		SubEnabledCount[1] == 0 &&
		VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + whole,
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
		cd.Parent->Square->NotifyChildDisable(*cd.Parent, cd.ChildIndex);
	}
}

GLuint VertexIndices[9];
int VertexTerrains[9];

void quadsquare::InitVert(int i, int x, int z) {
	if (x >= RowSize) x = RowSize-1;
	if (z >= NumRows) z = NumRows - 1;

	int idx = x + RowSize * z;

	VertexIndices[i] = idx;
	VertexTerrains[i] = Terrain[idx];
}

GLubyte *VNCArray;

void quadsquare::DrawTris() {
	int tmp_min_idx = VertexArrayMinIdx;

	/*
	if (glLockArraysEXT_p && param.use_cva) {
	if (tmp_min_idx == 0) tmp_min_idx = 1;
	glLockArraysEXT_p (tmp_min_idx, VertexArrayMaxIdx - tmp_min_idx + 1);
	}
	*/
	if (glLockArraysEXT_p) {
		if (tmp_min_idx == 0) tmp_min_idx = 1;
		glLockArraysEXT_p (tmp_min_idx, VertexArrayMaxIdx - tmp_min_idx + 1);
	}
	/*
	glDrawElements (GL_TRIANGLES, VertexArrayCounter,
	GL_UNSIGNED_INT, VertexArrayIndices);
	if (glUnlockArraysEXT_p && param.use_cva) glUnlockArraysEXT_p();
	*/
	glDrawElements (GL_TRIANGLES, VertexArrayCounter,
		GL_UNSIGNED_INT, VertexArrayIndices);
	if (glUnlockArraysEXT_p) glUnlockArraysEXT_p();
}

void quadsquare::DrawEnvmapTris()
{
	if (VertexArrayCounter > 0 && EnvmapTexture != NULL) {
		glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		EnvmapTexture->Bind();

		DrawTris();

		glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	}
}

void quadsquare::InitArrayCounters()
{
	VertexArrayCounter = 0;
	VertexArrayMinIdx = INT_MAX;
	VertexArrayMaxIdx = 0;
}

void quadsquare::Render (const quadcornerdata& cd, GLubyte *vnc_array) {
	VNCArray = vnc_array;
	bool fog_on;
	int nx, ny;
	Course.GetDivisions (&nx, &ny);
	const TTerrType *TerrList = &Course.TerrList[0];

	size_t numTerrains = Course.TerrList.size();
	//	fog_on = is_fog_on ();
	fog_on = true;
	for (size_t j=0; j<numTerrains; j++) {
		if (TerrList[j].texture != NULL) {
			InitArrayCounters();
			RenderAux (cd, SomeClip, (int)j);
			if (VertexArrayCounter == 0) continue;

			Course.TerrList[j].texture->Bind();
			DrawTris ();

			if (TerrList[j].shiny && param.perf_level > 1) {
				glDisableClientState (GL_COLOR_ARRAY);
				glColor4f (1.0, 1.0, 1.0, ENV_MAP_ALPHA / 255.0);
				DrawEnvmapTris();
				glEnableClientState (GL_COLOR_ARRAY);
			}

		}
	}

	if (param.perf_level > 1) {
		InitArrayCounters();
		RenderAux (cd, SomeClip, -1);

		if (VertexArrayCounter != 0) {
			glDisable (GL_FOG);
			for (GLuint i=0; i<VertexArrayCounter; i++) {
				colorval (VertexArrayIndices[i], 0) = 0;
				colorval (VertexArrayIndices[i], 1) = 0;
				colorval (VertexArrayIndices[i], 2) = 0;
				colorval (VertexArrayIndices[i], 3) = 255;
			}
			Course.TerrList[0].texture->Bind();
			DrawTris();
			if (fog_on) glEnable (GL_FOG);
			glBlendFunc  (GL_SRC_ALPHA, GL_ONE);
			for (GLuint i=0; i<VertexArrayCounter; i++) {
				colorval (VertexArrayIndices[i], 0) = 255;
				colorval (VertexArrayIndices[i], 1) = 255;
				colorval (VertexArrayIndices[i], 2) = 255;
			}

			for (size_t j=0; j<numTerrains; j++) {
				if (TerrList[j].texture > 0) {
					Course.TerrList[j].texture->Bind();

					for (GLuint i=0; i<VertexArrayCounter; i++) {
						colorval (VertexArrayIndices[i], 3) =
							(Terrain[VertexArrayIndices[i]] == (char)j ) ? 255 : 0;
					}
					DrawTris();
				}
			}

			/*
			if (param.perf_level > 1) {
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (i=0; i<VertexArrayCounter; i++) {
			colorval (VertexArrayIndices[i], 3) =
			(TerrList[Terrain[VertexArrayIndices[i]]].shiny) ?
			ENV_MAP_ALPHA : 0;
			}
			DrawEnvmapTris();
			}
			*/
		}
	}
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

clip_result_t quadsquare::ClipSquare (const quadcornerdata& cd)
{
	if (cd.xorg >= RowSize-1) {
		return NotVisible;
	}

	if (cd.zorg >= NumRows-1) {
		return NotVisible;
	}

	int whole = 2 << cd.Level;
	TVector3 min, max;
	min.x = cd.xorg * ScaleX;
	min.y = MinY;
	min.z =  cd.zorg * ScaleZ;
	max.x = (cd.xorg + whole) * ScaleX;
	max.y = MaxY;
	max.z = (cd.zorg + whole) * ScaleZ;
	if (min.x > max.x) {
		double tmp = min.x;
		min.x = max.x;
		max.x = tmp;
	}

	if (min.z > max.z) {
		double tmp = min.z;
		min.z = max.z;
		max.z = tmp;
	}

	clip_result_t clip_result = clip_aabb_to_view_frustum(min, max);

	if (clip_result == NotVisible || clip_result == SomeClip) {
		return clip_result;
	}

	if (cd.xorg + whole >= RowSize) {
		return SomeClip;
	}
	if (cd.zorg + whole >= NumRows) {
		return SomeClip;
	}

	return clip_result;
}


inline void quadsquare::MakeTri (int a, int b, int c, int terrain)
{
	if ( (VertexTerrains[a] == terrain ||
		VertexTerrains[b] == terrain ||
		VertexTerrains[c] == terrain) )
	{
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[a];
		setalphaval(a);
		update_min_max (VertexIndices[a]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[b];
		setalphaval(b);
		update_min_max (VertexIndices[b]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[c];
		setalphaval(c);
		update_min_max (VertexIndices[c]);
	}
}


inline void quadsquare::MakeSpecialTri (int a, int b, int c, int terrain)
{
	if (VertexTerrains[a] != VertexTerrains[b] &&
		VertexTerrains[a] != VertexTerrains[c] &&
		VertexTerrains[b] != VertexTerrains[c])
	{
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[a];
		update_min_max (VertexIndices[a]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[b];
		update_min_max (VertexIndices[b]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[c];
		update_min_max (VertexIndices[c]);
	}
}

inline void quadsquare::MakeNoBlendTri (int a, int b, int c, int terrain)
{
	if ( (VertexTerrains[a] == terrain ||
		VertexTerrains[b] == terrain ||
		VertexTerrains[c] == terrain) &&
		(VertexTerrains[a] >= terrain &&
		VertexTerrains[b] >= terrain &&
		VertexTerrains[c] >= terrain) )
	{
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[a];
		setalphaval(a);
		update_min_max (VertexIndices[a]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[b];
		setalphaval(b);
		update_min_max (VertexIndices[b]);
		VertexArrayIndices[VertexArrayCounter++] = VertexIndices[c];
		setalphaval(c);
		update_min_max (VertexIndices[c]);
	}
}

void quadsquare::RenderAux(const quadcornerdata& cd, clip_result_t vis, int terrain)
{
	int	half = 1 << cd.Level;
	int	whole = 2 << cd.Level;
	if (vis != NoClip) {
		vis = ClipSquare (cd);
		if (vis == NotVisible) return;
	}

	int	flags = 0;
	int	mask = 1;
	quadcornerdata	q;

	for (int i = 0; i < 4; i++, mask <<= 1) {
		if (EnabledFlags & (16 << i)) {
			SetupCornerData(&q, cd, i);
			Child[i]->RenderAux(q, vis, terrain);
		} else {
			flags |= mask;
		}
	}

	if (flags == 0) return;

	InitVert(0, cd.xorg + half, cd.zorg + half);
	InitVert(1, cd.xorg + whole, cd.zorg + half);
	InitVert(2, cd.xorg + whole, cd.zorg);
	InitVert(3, cd.xorg + half, cd.zorg);
	InitVert(4, cd.xorg, cd.zorg);
	InitVert(5, cd.xorg, cd.zorg + half);
	InitVert(6, cd.xorg, cd.zorg + whole);
	InitVert(7, cd.xorg + half, cd.zorg + whole);
	InitVert(8, cd.xorg + whole, cd.zorg + whole);
	if (terrain == -1) {
		make_tri_list (MakeSpecialTri);
	} else if (param.perf_level > 1) {
		make_tri_list (MakeTri);
	} else {
		make_tri_list (MakeNoBlendTri);
	}

}


void quadsquare::SetupCornerData(quadcornerdata* q, const quadcornerdata& cd, int ChildIndex)
{
	int	half = 1 << cd.Level;

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

void quadsquare::AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm)
{
	RowSize = hm.RowWidth;
	NumRows = hm.ZSize;

	if (cd.Parent == NULL) {
		if (VertexArrayIndices != NULL) {
			delete VertexArrayIndices;
		}
		VertexArrayIndices = new GLuint[6 * RowSize * NumRows];
	}
	int	BlockSize = 2 << cd.Level;
	if (cd.xorg > hm.XOrigin + ((hm.XSize + 2) << hm.Scale) ||
		cd.xorg + BlockSize < hm.XOrigin - (1 << hm.Scale) ||
		cd.zorg > hm.ZOrigin + ((hm.ZSize + 2) << hm.Scale) ||
		cd.zorg + BlockSize < hm.ZOrigin - (1 << hm.Scale))
	{
		return;
	}

	if (cd.Parent && cd.Parent->Square) {
		cd.Parent->Square->EnableChild(cd.ChildIndex, *cd.Parent);
	}

	int	half = 1 << cd.Level;
	for (int i = 0; i < 4; i++) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, i);

		if (Child[i] == NULL && cd.Level > hm.Scale) {
			Child[i] = new quadsquare(&q);
		}
		if (Child[i]) {
			Child[i]->AddHeightMap(q, hm);
		}
	}
	float	s[5];
	s[0] = hm.Sample(cd.xorg + half, cd.zorg + half);
	s[1] = hm.Sample(cd.xorg + half*2, cd.zorg + half);
	s[2] = hm.Sample(cd.xorg + half, cd.zorg);
	s[3] = hm.Sample(cd.xorg, cd.zorg + half);
	s[4] = hm.Sample(cd.xorg + half, cd.zorg + half*2);

	for (int i = 0; i < 5; i++) {
		if (s[i] != 0) {
			Dirty = true;
			Vertex[i].Y += s[i];
		}
	}

	if (!Dirty) {
		for (int i = 0; i < 4; i++) {
			if (Child[i] && Child[i]->Dirty) {
				Dirty = true;
				break;
			}
		}
	}

	if (Dirty) SetStatic(cd);
}

double quadsquare::ScaleX;
double quadsquare::ScaleZ;
void quadsquare::SetScale(double x, double z)
{
	ScaleX = x;
	ScaleZ = z;
}

char* quadsquare::Terrain;
void quadsquare::SetTerrain (char *t)
{
	Terrain = t;
}

float HeightMapInfo::Sample(int x, int z) const
{
	if (x >= XSize) {
		x = XSize - 1;
	}
	if (z >= ZSize) {
		z = ZSize - 1;
	}
	return Data[ x + z * RowWidth ];
}

// --------------------------------------------------------------------
// 				global calls
// --------------------------------------------------------------------

#define CULL_DETAIL_FACTOR 25

static quadsquare *root = (quadsquare*) NULL;
static quadcornerdata root_corner_data = {(quadcornerdata*)NULL };

void ResetQuadtree() {
	if (root != NULL) {
		delete root;
		root = (quadsquare*) NULL;
	}
}

static int get_root_level (int nx, int nz)
{
	int xlev, zlev;

	xlev = (int) (log(static_cast<double>(nx)) / log (2.0));
	zlev = (int) (log(static_cast<double>(nz)) / log (2.0));
	if ((nx >> xlev) << xlev == nx) {
	} else nx += 1;

	if ((nz >> zlev) << zlev == nz) {
	} else nz += 1;

	return max (xlev, zlev);
}

static void TVector3o_float_array (float dest[3], const TVector3& src)
{
	dest[0] = src.x;
	dest[1] = src.y;
	dest[2] = src.z;
}


void InitQuadtree (double *elevation, int nx, int nz,
				   double scalex, double scalez, const TVector3& view_pos, double detail) {
	HeightMapInfo hm;

	hm.Data = elevation;
	hm.XOrigin = 0;
	hm.ZOrigin = 0;
	hm.XSize = nx;
	hm.ZSize = nz;
	hm.RowWidth = hm.XSize;
	hm.Scale = 0;

	root_corner_data.Square = (quadsquare*)NULL;
	root_corner_data.ChildIndex = 0;
	root_corner_data.Level = get_root_level (nx, nz);
	root_corner_data.xorg = 0;
	root_corner_data.zorg = 0;

	for (int i=0; i<4; i++) {
		root_corner_data.Verts[i].Y = 0;
		root_corner_data.Verts[i].Y = 0;
	}

	root = new quadsquare (&root_corner_data);
	root->AddHeightMap (root_corner_data, hm);
	root->SetScale (scalex, scalez);
	root->SetTerrain (Course.terrain);

	root->StaticCullData (root_corner_data, CULL_DETAIL_FACTOR);

	float ViewerLoc[3];
	TVector3o_float_array (ViewerLoc, view_pos);

	for (int i = 0; i < 10; i++) {
		root->Update(root_corner_data, ViewerLoc, detail);
	}
}

void UpdateQuadtree (const TVector3& view_pos, float detail) {
	float ViewerLoc[3];
	TVector3o_float_array (ViewerLoc, view_pos);
	root->Update (root_corner_data, ViewerLoc, detail);
}

void RenderQuadtree() {
	GLubyte *vnc_array = Course.GetGLArrays();
	root->Render (root_corner_data, vnc_array);
}
