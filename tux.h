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

#ifndef TUX_H
#define TUX_H

#include "bh.h"

#define MAX_ACTIONS 8
#define	MAX_CHAR_NODES 256
#define	MAX_CHAR_MAT 32

#define USE_CHAR_DISPLAY_LIST true
#define MIN_SPHERE_DIV 3
#define MAX_SPHERE_DIV 16 


typedef struct {
    TColor diffuse;
    TColor specular;
    float exp;
} TCharMaterial;

typedef struct {
	int num;
	int type[MAX_ACTIONS];
	TVector3 vec[MAX_ACTIONS];
 	double dval[MAX_ACTIONS];
	string name;
	string order;
	string mat;
} TCharAction;		

typedef struct NodeStruct {
    struct NodeStruct *parent;
    struct NodeStruct *next;
    struct NodeStruct *child;

	int node_idx;		// number in node_array
	int parent_name;	// int identifier of parent
	int node_name;		// int identifier of node itself
    string joint;
	bool visible;
    TMatrix trans;
	TMatrix invtrans;   
	double radius;
	int divisions;
    TCharMaterial *mat;
    bool render_shadow;
} TCharNode;

class CCharShape {
private:
	TCharNode *Nodes[MAX_CHAR_NODES];
	TCharAction *Actions[MAX_CHAR_NODES];
	int Index[MAX_CHAR_NODES];
	string NodeIndex;
	int numNodes;
	bool useActions;
	bool newActions;
	TCharMaterial *Materials [MAX_CHAR_MAT];
	int numMaterials;
	string MaterialIndex;
	string file_name;
	string Matlines[MAX_CHAR_MAT];
	int numMatlines;

	// nodes 
	int GetNodeIdx (int node_name);
	TCharNode *GetNode (int node_name);
	bool GetNode (int node_name, TCharNode **node);
	void CreateRootNode ();
	bool CreateCharNode 
		(int parent_name, int node_name, const string joint, 
		string name, string order, bool shadow);
	bool VisibleNode (int node_name, float level);
	bool MaterialNode (int node_name, string mat_name);
	bool TransformNode (int node_name, TMatrix mat, TMatrix invmat);

	// material
	bool GetMaterial (const char *mat_name, TCharMaterial **mat);
	void CreateMaterial (const char *line);

	// drawing 
	void DrawCharSphere (int num_divisions);
	GLuint GetDisplayList (int divisions);
	void DrawNodes (TCharNode *node, TCharMaterial *mat);
	TVector3 AdjustRollvector (CControl *ctrl, TVector3 vel, TVector3 zvec);

	// collision
	bool CheckPolyhedronCollision (TCharNode *node, TMatrix modelMatrix, 
		TMatrix invModelMatrix, TPolyhedron ph);
	bool CheckCollision (TPolyhedron ph);

	// shadow
	void DrawShadowVertex (double x, double y, double z, TMatrix mat);
	void DrawShadowSphere (TMatrix mat);
	void TraverseDagForShadow (TCharNode *node, TMatrix mat);

	// testing and developing
	void AddAction (int node_name, int type, TVector3 vec, double val);
public:
	CCharShape ();
	~CCharShape ();
	bool useMaterials;

	// nodes 
	bool ResetNode (int node_name);
	bool ResetNode (string node_trivialname);
	bool TranslateNode (int node_name, TVector3 vec);
	bool RotateNode (int node_name, double axis, double angle);
	bool RotateNode (string node_trivialname, double axis, double angle);
	void ScaleNode (int node_name, TVector3 vec);
	void ResetRoot ();
	void ResetJoints ();

	// global functions
	void Draw ();
	void DrawShadow ();
	bool Load (string dir, string filename, bool with_actions);

	void AdjustOrientation (CControl *ctrl, double dtime,
		double dist_from_surface, TVector3 surf_nml);
	void AdjustJoints (double turnFact, bool isBraking, 
		double paddling_factor, double speed,
		TVector3 net_force, double flap_factor);
	bool Collision (TVector3 pos, TPolyhedron ph);

	// testing and tools
	string GetNodeName (int idx);
	int    GetNumNodes ();
	string GetNodeFullname (int idx);
	int    GetNumActs (int idx);
	TCharAction *GetAction (int idx);
	void   PrintAction (int idx);	
	void   PrintNode (int idx);
	void   RefreshNode (int idx);
	void   SaveCharNodes ();
};

#define MAX_CHARACTERS 16

typedef struct {
	int type;
	string name;
	string dir;
	GLuint preview;
	CCharShape *shape;
} TCharacter;

class CCharacter {
private:
	int curr_character; 
public:
	CCharacter ();
	~CCharacter ();

	TCharacter CharList [MAX_CHARACTERS];
	int numCharacters;

	void Draw (int idx);
	CCharShape *GetShape (int idx);
	void LoadCharacterList ();
};

extern CCharShape TestChar;
extern CCharacter Char;

#endif
