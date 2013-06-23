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

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "tool_char.h"
#include "tools.h"
#include "ogl.h"
#include "font.h"
#include "textures.h"
#include "spx.h"
#include "tux.h"
#include "winsys.h"

static size_t firstnode = 0;
static size_t lastnode;
static size_t curr_node = 0;
static size_t firstact = 0;
static size_t lastact;
static size_t curr_act = 0;

static float xposition = 0;
static float yposition = 0;
static float zposition = -3;
static float xrotation = 0;
static float yrotation = 0;
static float zrotation = 0;

static int drawcount = 0;
static int charbase = 24;
static bool must_render = true;
static TCharAction *action;
static bool shift = false;
static bool control = false;
static bool alt = false;
static TCharAction Undo;
static int startx, starty;
static double startrotx, startroty, startposx, startposy;
static bool rotactive = false;
static bool moveactive = false;
static int comp = 0;

void InitCharTools () {
	charbase = (int)((Winsys.resolution.height - 200) / 18);
	firstnode = 1;
	lastnode = TestChar.GetNumNodes () -1;
	curr_node = firstnode;
	curr_act = firstact;
	lastact = TestChar.GetNumActs (curr_node) -1;
	action = TestChar.GetAction (curr_node);
	StoreAction (action);
}

void StoreAction (TCharAction *act) {
	for (size_t i=0; i<=act->num; i++) {
		Undo.vec[i] = act->vec[i];
		Undo.dval[i] = act->dval[i];
	}
}

void RecallAction (TCharAction *act) {
	for (size_t i=0; i<=act->num; i++) {
		act->vec[i] = Undo.vec[i];
		act->dval[i] = Undo.dval[i];
	}
}

void ChangeValue (int type, double fact) {
	if (type == 0 || type == 4) {
		if (comp == 0) {
			action->vec[curr_act].x += 0.02 * fact;
			action->vec[curr_act].y += 0.02 * fact;
			action->vec[curr_act].z += 0.02 * fact;
		} else if (comp == 1) action->vec[curr_act].x += 0.02 * fact;
		else if (comp == 2) action->vec[curr_act].y += 0.02 * fact;
		else if (comp == 3) action->vec[curr_act].z += 0.02 * fact;
	} else if (type > 0 && type < 4) {
		action->dval[curr_act] += 1 * fact;
	} else if (type == 5) {
		action->dval[curr_act] += 1 * fact;
	}
	TestChar.RefreshNode (curr_node);
	SetCharChanged (true);
}

void ChangeNode (int steps) {
	bool ch;
	if (steps > 0) ch = (curr_node + steps <= lastnode);
	else ch = (curr_node + steps >= firstnode);
	if (ch) {
		curr_node += steps;
		curr_act = firstact;
		lastact = TestChar.GetNumActs (curr_node) -1;
		action = TestChar.GetAction (curr_node);
		if (action->num > 0 && action->type[0] == 4) comp = 0; else comp = 1;
		StoreAction (action);
	}
}

void SetRotation (double x, double y, double z) {
	xrotation = x;
	yrotation = y;
	zrotation = z;
}

void CharKeys (unsigned int key, bool special, bool release, int x, int y) {
	int keyfact;
	must_render = true;

	if (ToolsFinalStage ()) {
		if (key == SDLK_y || key == SDLK_j) {
			SaveToolCharacter ();
			SaveToolFrame ();
			State::manager.RequestQuit();
		} else if (key == SDLK_n) State::manager.RequestQuit();
		return;
	}

	if (key == 304) shift = !release;
	if (key == 306) control = !release;
	if (key == 308) alt = !release;
	if (shift) keyfact = -1; else keyfact = 1;

	if (release) return;

	int type = action->type[curr_act];
	switch (key) {
		case SDLK_TAB: SetToolMode (1); break;
		case SDLK_ESCAPE: case SDLK_q: QuitTool (); break;
		case SDLK_F10: ScreenshotN (); break;
		case SDLK_s: SaveToolCharacter ();  break;
		case SDLK_c: ScreenshotN (); break;
		case SDLK_m: TestChar.useMaterials = !TestChar.useMaterials; break;
		case SDLK_h: TestChar.useHighlighting = !TestChar.useHighlighting; break;
		case SDLK_r:
			TestChar.Reset ();
			ReloadToolCharacter ();
			Tools.Enter ();
			break;
		case SDLK_u: if (action != NULL) {
				RecallAction (action);
				TestChar.RefreshNode (curr_node);
			} break;
		case SDLK_PLUS: case SDLK_EQUALS: // zoom in
			zposition += 0.1;
			xposition -= 0.03;
			break;
		case SDLK_MINUS: // zoom out
			zposition -= 0.1;
			xposition += 0.03;
			break;

		// set rotations for view
		case SDLK_1: SetRotation (0, 0, 0); break;
		case SDLK_2: SetRotation (-50, 180, 15); break;
		case SDLK_3: SetRotation (0, 180, 0); break;
		case SDLK_4: SetRotation (0, -80, 0); break;

		// select node
		case SDLK_PAGEUP: ChangeNode (-1); break;
		case SDLK_PAGEDOWN: ChangeNode (1); break;
		case SDLK_END: ChangeNode (charbase); break;
		case SDLK_HOME: ChangeNode (-charbase); break;

		// select action
		case SDLK_DOWN:
			if (curr_act < lastact) curr_act++;
			if (action->type[curr_act] == 4) comp = 0; else comp = 1;
			break;
		case SDLK_UP:
			if (curr_act > 0) curr_act--;
			if (action->type[curr_act] == 4) comp = 0; else comp = 1;
			break;
		case SDLK_LEFT: ChangeValue (type, -1); break;
		case SDLK_RIGHT: ChangeValue (type, 1); break;

		// select value
		case SDLK_SPACE:
			if (type == 0 || type == 4) {
				comp++;
				if (comp > 3) comp = 0;
				if (type == 0 && comp == 0) comp = 1;
			} break;
		default: break;
	}
}

void CharMouse (int button, int state, int x, int y) {
	must_render = true;
	if (ToolsFinalStage ()) return;

	if (state<1) {
		rotactive = false;
		moveactive = false;
		return;
	}
	if (button == 1) {
		startx = x;
		starty = y;
		startrotx = xrotation;
		startroty = yrotation;
		rotactive = true;
	} else if (button == 3) {
		startx = x;
		starty = y;
		startposx = xposition;
		startposy = yposition;
		moveactive = true;
	} else if (button == 4) {
		zposition -= 0.1;
		xposition += 0.03;
	} else if (button == 5) {
		zposition += 0.1;
		xposition -= 0.03;
	}
}

void CharMotion (int x, int y) {
	int diffx, diffy;
	float diffposx, diffposy;

	must_render = true;
	if (rotactive) {
		diffx = cursor_pos.x - startx;
		diffy = cursor_pos.y - starty;
		yrotation = startroty + diffx;
		xrotation = startrotx + diffy;
	}
	if (moveactive) {
		diffposx = (double)(cursor_pos.x - startx) / 200;
		diffposy = (double)(cursor_pos.y - starty) / 200;
		yposition = startposy - diffposy;
		xposition = startposx + diffposx;
	}
}

void DrawActionVec (size_t nr, const string& s, int y, const TVector3& v) {
	FT.SetColor (colLGrey);
	FT.DrawString (20, y, s);
	if (nr == curr_act) {
		if (comp == 0) {
			FT.SetColor (colYellow);
			FT.DrawString (100, y, Float_StrN (v.x, 2));
			FT.DrawString (150, y, Float_StrN (v.y, 2));
			FT.DrawString (200, y, Float_StrN (v.z, 2));
		} else {
			if (comp == 1) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
			FT.DrawString (100, y, Float_StrN (v.x, 2));
			if (comp == 2) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
			FT.DrawString (150, y, Float_StrN (v.y, 2));
			if (comp == 3) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
			FT.DrawString (200, y, Float_StrN (v.z, 2));
		}
	} else {
		FT.DrawString (100, y, Float_StrN (v.x, 2));
		FT.DrawString (150, y, Float_StrN (v.y, 2));
		FT.DrawString (200, y, Float_StrN (v.z, 2));
	}
}

void DrawActionFloat (size_t nr, const string& s, int y, float f) {
	FT.SetColor (colLGrey);
	FT.DrawString (20, y, s);
	if (nr == curr_act) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
	FT.DrawString (100, y, Float_StrN (f, 2));
}

void RenderChar (double timestep) {
	if (!must_render) return;
	bool is_visible = false;
	check_gl_error();

	// ------------- 3d scenery ---------------------------------------
	ScopedRenderMode rm1(TUX);
    ClearRenderContext (colDDBackgr);
	TestChar.highlight_node = TestChar.GetNodeName (curr_node);

	glLoadIdentity ();
	glPushMatrix ();
	SetToolLight ();

	TestChar.ResetRoot ();
	TestChar.ResetJoints ();
	glTranslatef (xposition, yposition, zposition);
	glRotatef (xrotation, 1, 0, 0);
	glRotatef (yrotation, 0, 1, 0);
	glRotatef (zrotation, 0, 0, 1);

	if (drawcount > 0) TestChar.Draw ();
	glPopMatrix ();
	drawcount++;

	// --------------- 2d screen --------------------------------------
	SetupGuiDisplay ();
	ScopedRenderMode rm2(TEXFONT);

	FT.SetFont ("bold");
	FT.SetSize (20);
	FT.SetColor (colYellow);
	FT.DrawString (-1, 10, "Edit mode");

	if (CharHasChanged ()) DrawChanged ();

	FT.SetFont ("normal");
	FT.SetSize (16);
	int xl, yt;
	for (size_t i=0; i<=lastnode; i++) {
		if (i != curr_node) {
			FT.SetColor (colLGrey);
			FT.SetFont ("normal");
		} else {
			FT.SetColor (colYellow);
			FT.SetFont ("bold");
		}
		xl = ITrunc ((int)i, charbase) * 100 + 20;
		yt = IFrac ((int)i, charbase) * 18 + 60;
		FT.DrawString (xl, yt, TestChar.GetNodeJoint (i));
	}

	size_t num = action->num;
	int type;
	if (num > 0) {
		for (size_t i=0; i<num; i++) {
			is_visible = false;
			type = action->type[i];
			yt = Winsys.resolution.height - 120 + (int)i * 18;
			switch (type) {
				case 0: DrawActionVec (i, "trans", yt, action->vec[i]); break;
				case 1: DrawActionFloat (i, "x-rot", yt, action->dval[i]); break;
				case 2: DrawActionFloat (i, "y-rot", yt, action->dval[i]); break;
				case 3: DrawActionFloat (i, "z-rot", yt, action->dval[i]); break;
				case 4: DrawActionVec (i, "scale", yt, action->vec[i]); break;
				case 5: DrawActionFloat (i, "vis", yt, action->dval[i]);
					is_visible = true; break;
				default: break;
			}
		}
	}

	if (is_visible) FT.SetColor (colYellow); else FT.SetColor (colLGrey);
	FT.DrawString (20, 20, action->name);

	if (ToolsFinalStage ()) {
		FT.SetSize (20);
		FT.SetColor (colYellow);
		FT.DrawString (-1, Winsys.resolution.height - 50, "Quit program. Save character list (y/n)");
	}

	Reshape (Winsys.resolution.width, Winsys.resolution.height);
    Winsys.SwapBuffers ();
	if (drawcount > 3) must_render = false;
}
