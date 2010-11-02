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

#include "tools.h"
#include "tux.h"
#include "ogl.h"
#include "env.h"
#include "font.h"
#include "textures.h"

static float xposition = 1;
static float yposition = 0;
static float zposition = -2.0;
static float xrotation = 0;
static float yrotation = 0;
static float zrotation = 0;

static bool shift;
static int base = 24;
static bool finalstage = false;
static bool charchanged = false;

static float tdef_amb[]  = {0.45, 0.53, 0.75, 1.0};    
static float tdef_diff[] = {1.0, 0.9, 1.0, 1.0};    
static float tdef_spec[] = {0.6, 0.6, 0.6, 1.0};    
static float tdef_pos[]  = {1, 2, 2, 0.0};    
static TLight toollight;

//static int steermode = 0;
static int firstnode = 0;
static int lastnode;
static int curr_node = 0;
static int firstact = 0;
static int lastact;
static int curr_act = 0;

static TAction *action;

void DrawQuad (float x, float y, float w, float h,
		float scrheight, TColor col, int frame) {
	glDisable (GL_TEXTURE_2D);
    glColor4f (col.r, col.g, col.b, col.a);
	glBegin (GL_QUADS);
		glVertex2f (x-frame,   scrheight-y-h-frame);
		glVertex2f (x+w+frame, scrheight-y-h-frame);
		glVertex2f (x+w+frame, scrheight-y+frame);
		glVertex2f (x-frame,   scrheight-y+frame);
	glEnd();
	glEnable (GL_TEXTURE_2D);
}

void InitToolLight () {
	toollight.is_on = true;
	for (int i=0; i<4; i++) { 
		toollight.ambient[i]  = tdef_amb[i];
		toollight.diffuse[i]  = tdef_diff[i];
		toollight.specular[i] = tdef_spec[i];
 		toollight.position[i] = tdef_pos[i];
	}
}

void SetToolLight () {
	glLightfv (GL_LIGHT0, GL_POSITION, toollight.position);
	glLightfv (GL_LIGHT0, GL_AMBIENT, toollight.ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, toollight.diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, toollight.specular);
	glEnable  (GL_LIGHT0);
	glEnable  (GL_LIGHTING);
}

static int loopcount = 0;
void TuxshapeMonitor () {
	glPushMatrix ();
 	SetToolLight ();
	
	glTranslatef (xposition, yposition, zposition);	
	glRotatef (xrotation, 1, 0, 0);
	glRotatef (yrotation, 0, 1, 0);
	glRotatef (zrotation, 0, 0, 1);
//	ResetTuxRoot2 ();
//	ResetTuxJoints2 ();

	if (loopcount > 0) Tux.Draw ();
	glPopMatrix ();
	loopcount++;
}

static TAction Undo;

void StoreAction (TAction *act) {
	int i;
	for (i=0; i<=act->num; i++) {
		Undo.vec[i] = act->vec[i];
		Undo.dval[i] = act->dval[i];
	}

}

void RecallAction (TAction *act) {
	int i;
	for (i=0; i<=act->num; i++) {
		act->vec[i] = Undo.vec[i];
		act->dval[i] = Undo.dval[i];
	}
}

void QuitTool () {
	if (!charchanged) Winsys.Quit ();
	else finalstage = true;
}

void ToolsKeys (unsigned int key, bool special, bool release, int x, int y) {
	int keyfact;

	if (finalstage) {
		if (key == SDLK_y || key == SDLK_j) {
			Tux.SaveCharNodes ();
			Winsys.Quit();
		} else if (key == SDLK_n) Winsys.Quit ();
	} else {
		if (key == 304) shift = !release;
		if (shift) keyfact = -1; else keyfact = 1;
		if (release) return;

		int type = action->type[curr_act];	
		switch (key) {
			case SDLK_n: Tux.PrintNode(curr_node); break;
			case SDLK_a: Tux.PrintAction(curr_node); break;
			case SDLK_s: Tux.SaveCharNodes (); break;
			case SDLK_c: ScreenshotN (); break;
			case 27: QuitTool (); break;
			case SDLK_q: QuitTool (); break;
			case SDLK_m: Tux.useMaterials = !Tux.useMaterials; break;
			case SDLK_u: if (action != NULL) {
					RecallAction (action);
					Tux.RefreshNode (curr_node);
				} break;
			case SDLK_UP: 
				if (curr_node > firstnode) {
					curr_node--; 
					curr_act = firstact;
					lastact = Tux.GetNumActs (curr_node) -1;
					action = Tux.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_DOWN: 
				if (curr_node < lastnode) {
					curr_node++; 
					curr_act = firstact;
					lastact = Tux.GetNumActs (curr_node) -1;
					action = Tux.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_RIGHT:
				if (curr_node + base <= lastnode) {
					curr_node += base;
					curr_act = firstact;
					lastact = Tux.GetNumActs (curr_node) -1;
					action = Tux.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_LEFT:
				if (curr_node - base >= 0) {
					curr_node -= base;
					curr_act = firstact;
					lastact = Tux.GetNumActs (curr_node) -1;
					action = Tux.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_r: Tux.RefreshNode (curr_node); break;
			case SDLK_PAGEDOWN: if (curr_act < lastact) curr_act++; break; 
			case SDLK_PAGEUP: if (curr_act > 0) curr_act--; break; 
			case 257: 
				if (type == 0 || type == 4) {
					action->vec[curr_act].x += (0.02 * keyfact); 
					Tux.RefreshNode (curr_node);
					charchanged = true;
				} else if (type > 0 && type < 4) {
					action->dval[curr_act] += (1 * keyfact);
					Tux.RefreshNode (curr_node);				
					charchanged = true;
				} else if (type == 5) {
					action->dval[curr_act] += (1 * keyfact);
					Tux.RefreshNode (curr_node);				
					charchanged = true;
				}
				break;
			case 258: 
				if (type == 0 || type == 4) {
					action->vec[curr_act].y += (0.02 * keyfact); 
					Tux.RefreshNode (curr_node);
					charchanged = true;
				}
				break;
			case 259: 
				if (type == 0 || type == 4) {
					action->vec[curr_act].z += (0.02 * keyfact); 
					Tux.RefreshNode (curr_node);
					charchanged = true;
				}
				break;
		}
	}
}

static int startx, starty;
static double startrotx, startroty, startposx, startposy;
static bool rotactive = false;
static bool moveactive = false;

static void ToolsMouse (int button, int state, int x, int y) {
	if (!finalstage) {
		if (button == 1) {
			if (state < 1) { 
				rotactive = false;
				return;
			}
			startx = x;
			starty = y;
			startrotx = xrotation;
			startroty = yrotation;
			rotactive = true;
		} else if (button == 3) {
			if (state < 1) { 
				moveactive = false;
				return;
			}
			startx = x;
			starty = y;
			startposx = xposition;
			startposy = yposition;
			moveactive = true;
		} else if (button == 4) {
			if (state==1) {
				zposition -= 0.1;
				xposition += 0.03;
			}
		} else if (button == 5) {
			if (state==1) {
				zposition += 0.1;
				xposition -= 0.03;
			}
		}
	}
}

static void ToolsMotion (int x, int y) {
	int diffx, diffy;
	float diffposx, diffposy;
	if (rotactive) {
		diffx = x - startx;
		diffy = y - starty;
		yrotation = startroty + diffx;
		xrotation = startrotx + diffy;
	}
	if (moveactive) {
		diffposx = (double)(x - startx) / 200;
		diffposy = (double)(y - starty) / 200;
		yposition = startposy - diffposy;
		xposition = startposx + diffposx;
	}
}

void DrawActionVec (string s, int y, TVector3 v) {
	FT.DrawString (20, y, s);
	FT.DrawString (100, y, Float_StrN (v.x, 2));
	FT.DrawString (150, y, Float_StrN (v.y, 2));
	FT.DrawString (200, y, Float_StrN (v.z, 2));
}

void DrawActionFloat (string s, int y, float f) {
	FT.DrawString (20, y, s);
	FT.DrawString (100, y, Float_StrN (f, 2));
}

// --------------------------------------------------------------------

void ToolsInit (void) {
	base = (int)((param.y_resolution - 200) / 18); 
	Winsys.KeyRepeat (true);
	InitToolLight ();
	switch (g_game.toolmode) {
		case NONE: break;
		case TUXSHAPE: break;
		case KEYFRAME: break;
		case TREEGEN: break;
		case LEARN: break;
	}
	firstnode = 1;
	lastnode = Tux.GetNumNodes () -1;
	curr_node = firstnode;
	curr_act = firstact;
	lastact = Tux.GetNumActs (curr_node) -1;
	action = Tux.GetAction (curr_node);
	StoreAction (action);
}

void ToolsLoop (double timestep) {
	bool is_visible;
	check_gl_error();
	set_gl_options (TUX);
    ClearRenderContext (colDDBackgr);
		switch (g_game.toolmode) {
			case TUXSHAPE: TuxshapeMonitor (); break;
			case KEYFRAME: break;
			case TREEGEN: break;
			case NONE: break;
			case LEARN: break;
		}

		SetupGuiDisplay ();
		set_gl_options (TEXFONT);

		FT.SetFont ("normal");
		FT.SetSize (16);

		int xl, yt;
		for (int i=0; i<=lastnode; i++) {
			if (i != curr_node) {
				FT.SetColor (colLGrey); 
				FT.SetFont ("normal");
			} else {
				FT.SetColor (colYellow);
				FT.SetFont ("bold");
			}
			xl = ITrunc (i, base) * 100 + 20;
			yt = IFrac (i, base) * 18 + 60;
			FT.DrawString (xl, yt, Tux.GetNodeName (i));		
		}

		int num = action->num;
		int type;
		if (num > 0) {
			for (int i=0; i<num; i++) {
				is_visible = false;
				if (i == curr_act) FT.SetColor (colYellow); 
				else FT.SetColor (colLGrey); 
				type = action->type[i];
				yt = param.y_resolution - 120 + i * 18;
				switch (type) {
					case 0: DrawActionVec ("trans", yt, action->vec[i]); break;
					case 1: DrawActionFloat ("x-rot", yt, action->dval[i]); break;
					case 2: DrawActionFloat ("y-rot", yt, action->dval[i]); break;
					case 3: DrawActionFloat ("z-rot", yt, action->dval[i]); break;
					case 4: DrawActionVec ("scale", yt, action->vec[i]); break;
					case 5: DrawActionFloat ("vis", yt, action->dval[i]); 
						is_visible = true;
						break;
					default: break;
				}
			}
		}
	if (is_visible) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
	FT.DrawString (20, 20, action->name);

	if (finalstage) {
		FT.SetSize (20);
		FT.SetColor (colYellow);
		FT.DrawString (-1, param.y_resolution - 50, "Quit program. Save character list (y/n)");
	}

	Reshape (param.x_resolution, param.y_resolution);
    Winsys.SwapBuffers();
} 

void ToolsTerm () {}

void RegisterToolFuncs () {
	Winsys.SetModeFuncs (TOOLS, ToolsInit, ToolsLoop, ToolsTerm,
 		ToolsKeys, ToolsMouse, ToolsMotion, NULL, NULL);
}

/*
// --------------------------------------------------------------------
//			MakeSPModel
//			Functions to convert .OBJ t0 .LST
// --------------------------------------------------------------------

TIndex3 Vec3_Idx3 (const TVector3 &v) {
	TIndex3 res;
	res.i = (int) v.x - 1;
	res.j = (int) v.y - 1;
	res.k = (int) v.z - 1;
	return res;
} 

void MakeSPModel (int smooth, int mat) {
	TVector3 vert[1000];
	int numVert = 0;
	TVector3 nmls[3000];
	int numNmls = 0;
	TVector2 coords[3000];
	int numCoords = 0;
	struct TTTri {TIndex3 elem[3];};
	TTTri tri[400];		
	int numTri = 0;
	struct TTQuad {TIndex3 elem[4];};
	TTQuad quad[400];		
	int numQuad = 0;

	CSPList list (1000, 1);
	int pos, cnt, i;
	string ss;
	string line;

	string modeldir = cfg.develop_dir;
	modeldir += SEP;
	modeldir += "model";

	// read the .obj file:
	if (list.Load (modeldir, "testmodel.obj")) {

		// pass 1: read the vertices, normals and texl coords
		for (i=0; i<list.Count(); i++) {
			line = list.Line (i);

			pos = SPosN (line, "v ");
			if (pos >= 0) {
				SDeleteN (line, pos, 2);
				SInsertN (line, pos, "[v] ");
				vert[numVert] = SPVector3N (line, "v");
				numVert++;
			}

			pos = SPosN (line, "vn ");
			if (pos >= 0) {
				SDeleteN (line, pos, 3);
				SInsertN (line, pos, "[vn] ");
				nmls[numNmls] = SPVector3N (line, "vn");
				numNmls++;
			}

			pos = SPosN (line, "vt ");
			if (pos >= 0) {
				SDeleteN (line, pos, 3);
				SInsertN (line, pos, "[vt] ");
				coords[numCoords] = SPVector2N (line, "vt");
				numCoords++;
			}
		}


		// pass 2 - read the faces
		for (i=0; i<list.Count(); i++) {
			line = list.Line (i);
			pos = SPosN (line, "f ");
			if (pos >= 0) {
				cnt = 0;
				do {
					pos = SPosN (line, " ");
					if (pos >= 0) {
						SDeleteN (line, pos, 1);
						ss = "["+ Int_StrN (cnt) +"]";			
						SInsertN (line, pos, ss);
						cnt++;
					}	
				} while (pos >= 0);

				do {
					pos = SPosN (line, "/");
					if (pos >= 0) {
						SDeleteN (line, pos, 1);
						SInsertN (line, pos, " ");
					}					
				} while (pos >= 0);

				if (cnt == 3) {
					tri[numTri].elem[0] = Vec3_Idx3 (SPVector3N (line, "0"));
					tri[numTri].elem[1] = Vec3_Idx3 (SPVector3N (line, "1"));
					tri[numTri].elem[2] = Vec3_Idx3 (SPVector3N (line, "2"));
					numTri++;
				}
				if (cnt == 4) {
					quad[numQuad].elem[0] = Vec3_Idx3 (SPVector3N (line, "0"));
					quad[numQuad].elem[1] = Vec3_Idx3 (SPVector3N (line, "1"));
					quad[numQuad].elem[2] = Vec3_Idx3 (SPVector3N (line, "2"));
					quad[numQuad].elem[3] = Vec3_Idx3 (SPVector3N (line, "3"));
					numQuad++;
				}

				line += "[cnt]";
				line += Int_StrN (cnt);
			}
		}
	} else {
		Message ("could not load obj file");
		return;
	}


	// generate the SP file:
	CSPList list2 (1000);

	string s;
	list2.Add ("# model description generated from .obj file");
	list2.Add ("");
	s = "*[S] 0 [vertices] ";
	s += Int_StrN (numVert);
	s += " [triangles] ";
	s += Int_StrN (numTri);
	s += " [quads] ";
	s += Int_StrN (numQuad);
	s += " [tex] 1 [mat] ";
	s += Int_StrN (mat);
	s += " [smooth] ";
	s += Int_StrN (smooth);
	list2.Add (s);
	list2.Add ("");
	list2.Add ("# Vertices:");
	for (int i=0; i<numVert; i++) {
		s = "*[S] 1 ";
		SPAddVec3N (s, "v", vert[i], 5);
		list2.Add (s);
	}

	TIndex3 idx[4];
	TIndex3 idx3;
	TIndex4 idx4;

	list2.Add ("");
	list2.Add ("# Triangles:");
	for (i=0; i<numTri; i++) {
		for (int j=0; j<3; j++) {
			idx[j] = tri[i].elem[j];
		}		
		s = "*[S] 2 ";
		idx3.i = idx[0].i;
		idx3.j = idx[1].i;
		idx3.k = idx[2].i;
		SPAddIndx3N  (s, "v", idx3);
		SPAddVec2N (s, "t1", coords[idx[0].j], 2);
		SPAddVec2N (s, "t2", coords[idx[1].j], 2);
		SPAddVec2N (s, "t3", coords[idx[2].j], 2);		
		list2.Add (s);
		if (smooth == 1) {
			s = " ";
			SPAddVec3N (s, "n1", nmls[idx[0].k], 2);
			SPAddVec3N (s, "n2", nmls[idx[1].k], 2);
			SPAddVec3N (s, "n3", nmls[idx[2].k], 2);		
			list2.Add (s);
		}
	}


	list2.Add ("");
	list2.Add ("# Quads:");
	for (i=0; i<numQuad; i++) {
		for (int j=0; j<4; j++) idx[j] = quad[i].elem[j];
		s = "*[S] 3 ";
		idx4.i = idx[0].i;
		idx4.j = idx[1].i;
		idx4.k = idx[2].i;
		idx4.l = idx[3].i;
		SPAddIndx4N  (s, "v", idx4);
		SPAddVec2N (s, "t1", coords[idx[0].j], 2);
		SPAddVec2N (s, "t2", coords[idx[1].j], 2);
		SPAddVec2N (s, "t3", coords[idx[2].j], 2);		
		SPAddVec2N (s, "t4", coords[idx[3].j], 2);				
		list2.Add (s);
		if (smooth == 1) {
			s = " ";
			SPAddVec3N (s, "n1", nmls[idx[0].k], 2);
			SPAddVec3N (s, "n2", nmls[idx[1].k], 2);
			SPAddVec3N (s, "n3", nmls[idx[2].k], 2);		
			SPAddVec3N (s, "n4", nmls[idx[3].k], 2);				
			list2.Add (s);
		}
	}

	list2.Print();
	list2.Save (modeldir, "testmodel.lst");
}
*/
