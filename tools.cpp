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
static bool must_render = true;

static float tdef_amb[]  = {0.45, 0.53, 0.75, 1.0};    
static float tdef_diff[] = {1.0, 0.9, 1.0, 1.0};    
static float tdef_spec[] = {0.6, 0.6, 0.6, 1.0};    
static float tdef_pos[]  = {1, 2, 2, 0.0};    
static TLight toollight;

static int firstnode = 0;
static int lastnode;
static int curr_node = 0;
static int firstact = 0;
static int lastact;
static int curr_act = 0;

static TCharAction *action;

void ToolsInit (void);

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

static int drawcount = 0;
void TuxshapeMonitor () {
	glPushMatrix ();
 	SetToolLight ();
	
	glTranslatef (xposition, yposition, zposition);	
	glRotatef (xrotation, 1, 0, 0);
	glRotatef (yrotation, 0, 1, 0);
	glRotatef (zrotation, 0, 0, 1);

//	ResetTuxRoot2 ();
//	ResetTuxJoints2 ();

	if (drawcount > 0) TestChar.Draw ();
	glPopMatrix ();
	drawcount++;
	if (drawcount > 3) must_render = false;
	g_game.loopdelay = 10;
}

static TCharAction Undo;

void StoreAction (TCharAction *act) {
	int i;
	for (i=0; i<=act->num; i++) {
		Undo.vec[i] = act->vec[i];
		Undo.dval[i] = act->dval[i];
	}
}

void RecallAction (TCharAction *act) {
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

	must_render = true;

	if (finalstage) {
		if (key == SDLK_y || key == SDLK_j) {
			TestChar.SaveCharNodes (g_game.arg_str);
			Winsys.Quit();
		} else if (key == SDLK_n) Winsys.Quit ();
	} else {
		if (key == 304) shift = !release;
		if (shift) keyfact = -1; else keyfact = 1;
		if (release) return;

		int type = action->type[curr_act];	
		switch (key) {
			case 27: case SDLK_q: QuitTool (); break;
			case SDLK_n: TestChar.PrintNode(curr_node); break;
			case SDLK_9: TestChar.PrintAction(curr_node); break;
			case SDLK_s: TestChar.SaveCharNodes (g_game.arg_str); charchanged = false; break;
			case SDLK_c: ScreenshotN (); break;
			case SDLK_m: TestChar.useMaterials = !TestChar.useMaterials; break;
			case SDLK_h: TestChar.useHighlighting = !TestChar.useHighlighting; break;
			case SDLK_8: TestChar.PrintNode (6); break;
			case SDLK_r: 
				TestChar.Reset (); 
				TestChar.Load (param.char_dir, g_game.arg_str, true);
				ToolsInit ();
				break;
			case SDLK_1: 
				xrotation = 0;
				yrotation = 0;
				zrotation = 0;
				break;
			case SDLK_2: 
				xrotation = -50;
				yrotation = 180;
				zrotation = 15;
				break;
			case SDLK_3: 
				xrotation = 0;
				yrotation = 180;
				zrotation = 0;
				break;
			case SDLK_4: 
				xrotation = 0;
				yrotation = -80;
				zrotation = 0;
				break;
			case SDLK_u: if (action != NULL) {
					RecallAction (action);
					TestChar.RefreshNode (curr_node);
				} break;
			case SDLK_UP: 
				if (curr_node > firstnode) {
					curr_node--; 
					curr_act = firstact;
					lastact = TestChar.GetNumActs (curr_node) -1;
					action = TestChar.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_DOWN: 
				if (curr_node < lastnode) {
					curr_node++; 
					curr_act = firstact;
					lastact = TestChar.GetNumActs (curr_node) -1;
					action = TestChar.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_RIGHT:
				if (curr_node + base <= lastnode) {
					curr_node += base;
					curr_act = firstact;
					lastact = TestChar.GetNumActs (curr_node) -1;
					action = TestChar.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_LEFT:
				if (curr_node - base >= 0) {
					curr_node -= base;
					curr_act = firstact;
					lastact = TestChar.GetNumActs (curr_node) -1;
					action = TestChar.GetAction (curr_node);
					StoreAction (action);
				}
				break;
			case SDLK_PAGEDOWN: if (curr_act < lastact) curr_act++; break; 
			case SDLK_PAGEUP: if (curr_act > 0) curr_act--; break; 
			case 257: case SDLK_x:
				if (type == 0 || type == 4) {
					action->vec[curr_act].x += (0.02 * keyfact); 
					TestChar.RefreshNode (curr_node);
					charchanged = true;
				} else if (type > 0 && type < 4) {
					action->dval[curr_act] += (1 * keyfact);
					TestChar.RefreshNode (curr_node);				
					charchanged = true;
				} else if (type == 5) {
					action->dval[curr_act] += (1 * keyfact);
					TestChar.RefreshNode (curr_node);				
					charchanged = true;
				}
				break;
			case 258: case SDLK_y:
				if (type == 0 || type == 4) {
					action->vec[curr_act].y += (0.02 * keyfact); 
					TestChar.RefreshNode (curr_node);
					charchanged = true;
				}
				break;
			case 259: case SDLK_z:
				if (type == 0 || type == 4) {
					action->vec[curr_act].z += (0.02 * keyfact); 
					TestChar.RefreshNode (curr_node);
					charchanged = true;
				}
				break;
			case SDLK_a: 
				if (type == 4) {
					action->vec[curr_act].x += (0.02 * keyfact); 
					action->vec[curr_act].y += (0.02 * keyfact); 
					action->vec[curr_act].z += (0.02 * keyfact); 
					TestChar.RefreshNode (curr_node);
					charchanged = true;
				}
				break;
		
			case SDLK_PLUS: // zoom in
				zposition += 0.1;
				xposition -= 0.03;
				break;
			case SDLK_EQUALS: // zoom in (for qwerty)
				zposition += 0.1;
				xposition -= 0.03;
				break;
			case SDLK_MINUS: // zoom out
				zposition -= 0.1;
				xposition += 0.03;
				break;
		}
	}
}

static int startx, starty;
static double startrotx, startroty, startposx, startposy;
static bool rotactive = false;
static bool moveactive = false;

static void ToolsMouse (int button, int state, int x, int y) {
	must_render = true;
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
	must_render = true;
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
	firstnode = 1;
	lastnode = TestChar.GetNumNodes () -1;
	curr_node = firstnode;
	curr_act = firstact;
	lastact = TestChar.GetNumActs (curr_node) -1;
	action = TestChar.GetAction (curr_node);
	StoreAction (action);
}

void RenderTools () {
	bool is_visible = false;
	check_gl_error();
	set_gl_options (TUX);
    ClearRenderContext (colDDBackgr);

	TestChar.highlight_node = TestChar.GetNodeName (curr_node);
	TuxshapeMonitor (); 

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
		FT.DrawString (xl, yt, TestChar.GetNodeJoint (i));		
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

void ToolsLoop (double timestep) {
	if (must_render) RenderTools ();
} 

void ToolsTerm () {}

void RegisterToolFuncs () {
	Winsys.SetModeFuncs (TOOLS, ToolsInit, ToolsLoop, ToolsTerm,
 		ToolsKeys, ToolsMouse, ToolsMotion, NULL, NULL);
}

