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

#include "tool_char.h"
#include "tools.h"
#include "ogl.h"
#include "font.h"
#include "textures.h"
#include "spx.h"
#include "tux.h"
#include "winsys.h"

static std::size_t firstnode = 0;
static std::size_t lastnode;
static std::size_t curr_node = 0;
static std::size_t firstact = 0;
static std::size_t lastact;
static std::size_t curr_act = 0;

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

void InitCharTools() {
	charbase = (int)((Winsys.resolution.height - 200) / 18);
	firstnode = 1;
	lastnode = TestChar.GetNumNodes() -1;
	curr_node = firstnode;
	curr_act = firstact;
	lastact = TestChar.GetNumActs(curr_node) -1;
	action = TestChar.GetAction(curr_node);
	StoreAction(action);
}

void StoreAction(TCharAction *act) {
	for (std::size_t i=0; i<=act->num; i++) {
		Undo.vec[i] = act->vec[i];
		Undo.dval[i] = act->dval[i];
	}
}

void RecallAction(TCharAction *act) {
	for (std::size_t i=0; i<=act->num; i++) {
		act->vec[i] = Undo.vec[i];
		act->dval[i] = Undo.dval[i];
	}
}

void ChangeValue(int type, double fact) {
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
	TestChar.RefreshNode(curr_node);
	SetCharChanged(true);
}

void ChangeNode(int steps) {
	bool ch;
	if (steps > 0) ch = (curr_node + steps <= lastnode);
	else ch = (curr_node + steps >= firstnode);
	if (ch) {
		curr_node += steps;
		curr_act = firstact;
		lastact = TestChar.GetNumActs(curr_node) -1;
		action = TestChar.GetAction(curr_node);
		if (action->num > 0 && action->type[0] == 4) comp = 0;
		else comp = 1;
		StoreAction(action);
	}
}

void SetRotation(double x, double y, double z) {
	xrotation = x;
	yrotation = y;
	zrotation = z;
}

void CharKeys(sf::Keyboard::Key key, bool release, int x, int y) {
	must_render = true;

	if (ToolsFinalStage()) {
		if (key == sf::Keyboard::Y || key == sf::Keyboard::J) {
			SaveToolCharacter();
			SaveToolFrame();
			State::manager.RequestQuit();
		} else if (key == sf::Keyboard::N) State::manager.RequestQuit();
		return;
	}

	if (key == sf::Keyboard::LShift || key == sf::Keyboard::RShift) shift = !release;
	if (key == sf::Keyboard::LControl || key == sf::Keyboard::RControl) control = !release;
	if (key == sf::Keyboard::LAlt || key == sf::Keyboard::RAlt) alt = !release;

	if (release) return;

	int type = action->type[curr_act];
	switch (key) {
		case sf::Keyboard::Tab:
			SetToolMode(1);
			break;
		case sf::Keyboard::Escape:
		case sf::Keyboard::Q:
			QuitTool();
			break;
		case sf::Keyboard::F10:
		case sf::Keyboard::C:
			Winsys.TakeScreenshot();
			break;
		case sf::Keyboard::S:
			SaveToolCharacter();
			break;
		case sf::Keyboard::M:
			TestChar.useMaterials = !TestChar.useMaterials;
			break;
		case sf::Keyboard::H:
			TestChar.useHighlighting = !TestChar.useHighlighting;
			break;
		case sf::Keyboard::R:
			TestChar.Reset();
			ReloadToolCharacter();
			Tools.Enter();
			break;
		case sf::Keyboard::U:
			if (action != nullptr) {
				RecallAction(action);
				TestChar.RefreshNode(curr_node);
			}
			break;
		case sf::Keyboard::Add:
		case sf::Keyboard::Equal: // zoom in
			zposition += 0.1f;
			xposition -= 0.03f;
			break;
		case sf::Keyboard::Dash: // zoom out
			zposition -= 0.1f;
			xposition += 0.03f;
			break;

		// set rotations for view
		case sf::Keyboard::Num1:
			SetRotation(0, 0, 0);
			break;
		case sf::Keyboard::Num2:
			SetRotation(-50, 180, 15);
			break;
		case sf::Keyboard::Num3:
			SetRotation(0, 180, 0);
			break;
		case sf::Keyboard::Num4:
			SetRotation(0, -80, 0);
			break;

		// select node
		case sf::Keyboard::PageUp:
			ChangeNode(-1);
			break;
		case sf::Keyboard::PageDown:
			ChangeNode(1);
			break;
		case sf::Keyboard::End:
			ChangeNode(charbase);
			break;
		case sf::Keyboard::Home:
			ChangeNode(-charbase);
			break;

		// select action
		case sf::Keyboard::Down:
			if (curr_act < lastact) curr_act++;
			if (action->type[curr_act] == 4) comp = 0;
			else comp = 1;
			break;
		case sf::Keyboard::Up:
			if (curr_act > 0) curr_act--;
			if (action->type[curr_act] == 4) comp = 0;
			else comp = 1;
			break;
		case sf::Keyboard::Left:
			ChangeValue(type, -1);
			break;
		case sf::Keyboard::Right:
			ChangeValue(type, 1);
			break;

		// select value
		case sf::Keyboard::Space:
			if (type == 0 || type == 4) {
				comp++;
				if (comp > 3) comp = 0;
				if (type == 0 && comp == 0) comp = 1;
			}
			break;
		default:
			break;
	}
}

void CharMouse(int button, int state, int x, int y) {
	must_render = true;
	if (ToolsFinalStage()) return;

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
		zposition -= 0.1f;
		xposition += 0.03f;
	} else if (button == 5) {
		zposition += 0.1f;
		xposition -= 0.03f;
	}
}

void CharMotion(int x, int y) {
	must_render = true;
	if (rotactive) {
		int diffx = cursor_pos.x - startx;
		int diffy = cursor_pos.y - starty;
		yrotation = startroty + diffx;
		xrotation = startrotx + diffy;
	}
	if (moveactive) {
		float diffposx = (float)(cursor_pos.x - startx) / 200;
		float diffposy = (float)(cursor_pos.y - starty) / 200;
		yposition = startposy - diffposy;
		xposition = startposx + diffposx;
	}
}

void DrawActionVec(std::size_t nr, const std::string& s, int y, const TVector3d& v) {
	FT.SetColor(colLGrey);
	FT.DrawString(20, y, s);
	if (nr == curr_act) {
		if (comp == 0) {
			FT.SetColor(colYellow);
			FT.DrawString(100, y, Float_StrN(v.x, 2));
			FT.DrawString(150, y, Float_StrN(v.y, 2));
			FT.DrawString(200, y, Float_StrN(v.z, 2));
		} else {
			if (comp == 1) FT.SetColor(colYellow);
			else FT.SetColor(colLGrey);
			FT.DrawString(100, y, Float_StrN(v.x, 2));
			if (comp == 2) FT.SetColor(colYellow);
			else FT.SetColor(colLGrey);
			FT.DrawString(150, y, Float_StrN(v.y, 2));
			if (comp == 3) FT.SetColor(colYellow);
			else FT.SetColor(colLGrey);
			FT.DrawString(200, y, Float_StrN(v.z, 2));
		}
	} else {
		FT.DrawString(100, y, Float_StrN(v.x, 2));
		FT.DrawString(150, y, Float_StrN(v.y, 2));
		FT.DrawString(200, y, Float_StrN(v.z, 2));
	}
}

void DrawActionFloat(std::size_t nr, const std::string& s, int y, float f) {
	FT.SetColor(colLGrey);
	FT.DrawString(20, y, s);
	if (nr == curr_act) FT.SetColor(colYellow);
	else FT.SetColor(colLGrey);
	FT.DrawString(100, y, Float_StrN(f, 2));
}

void RenderChar(float timestep) {
	if (!must_render) return;
	bool is_visible = false;

	// ------------- 3d scenery ---------------------------------------
	ScopedRenderMode rm1(TUX);
	ClearRenderContext(colDDBackgr);
	TestChar.highlight_node = TestChar.GetNodeName(curr_node);

	glLoadIdentity();
	glPushMatrix();
	SetToolLight();

	TestChar.ResetRoot();
	TestChar.ResetJoints();
	glTranslatef(xposition, yposition, zposition);
	glRotatef(xrotation, 1, 0, 0);
	glRotatef(yrotation, 0, 1, 0);
	glRotatef(zrotation, 0, 0, 1);

	if (drawcount > 0) TestChar.Draw();
	glPopMatrix();
	drawcount++;

	// --------------- 2d screen --------------------------------------
	Setup2dScene();
	ScopedRenderMode rm2(TEXFONT);

	FT.SetProps("bold", 20, colYellow);
	FT.DrawString(-1, 10, "Edit mode");

	if (CharHasChanged()) DrawChanged();

	FT.SetSize(16);
	for (std::size_t i=0; i<=lastnode; i++) {
		if (i != curr_node) {
			FT.SetColor(colLGrey);
			FT.SetFont("normal");
		} else {
			FT.SetColor(colYellow);
			FT.SetFont("bold");
		}
		int xl = ITrunc((int)i, charbase) * 100 + 20;
		int yt = IFrac((int)i, charbase) * 18 + 60;
		FT.DrawString(xl, yt, TestChar.GetNodeJoint(i));
	}

	std::size_t num = action->num;
	for (std::size_t i=0; i<num; i++) {
		is_visible = false;
		int type = action->type[i];
		int yt = Winsys.resolution.height - 120 + (int)i * 18;
		switch (type) {
			case 0:
				DrawActionVec(i, "trans", yt, action->vec[i]);
				break;
			case 1:
				DrawActionFloat(i, "x-rot", yt, action->dval[i]);
				break;
			case 2:
				DrawActionFloat(i, "y-rot", yt, action->dval[i]);
				break;
			case 3:
				DrawActionFloat(i, "z-rot", yt, action->dval[i]);
				break;
			case 4:
				DrawActionVec(i, "scale", yt, action->vec[i]);
				break;
			case 5:
				DrawActionFloat(i, "vis", yt, action->dval[i]);
				is_visible = true;
				break;
			default:
				break;
		}
	}

	FT.SetFont("normal");
	if (is_visible) FT.SetColor(colYellow);
	else FT.SetColor(colLGrey);
	FT.DrawString(20, 20, action->name);

	if (ToolsFinalStage()) {
		FT.SetSize(20);
		FT.SetColor(colYellow);
		FT.DrawString(-1, Winsys.resolution.height - 50, "Quit program. Save character list (y/n)");
	}

	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
	if (drawcount > 3) must_render = false;
}
