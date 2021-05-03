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

#include "tool_frame.h"
#include "tools.h"
#include "ogl.h"
#include "keyframe.h"
#include "font.h"
#include "textures.h"
#include "spx.h"
#include "tux.h"
#include "winsys.h"

static std::size_t curr_frame = 0;
static int curr_joint = 0;
static int last_joint = 0;
static TVector3d ref_position(0, 0, 0);
static bool must_render = true;
static int framebase = 24;
static int jointbase = 16;
static bool shift = false;
static bool control = false;
static bool alt = false;
static bool lastframe = 0;
static bool keyrun = false;

void InitFrameTools() {
	framebase = (int)((Winsys.resolution.height - 350) / 18);
	if (TestFrame.numFrames() < 1) TestFrame.AddFrame();
	curr_joint = 0;
	last_joint = TestFrame.GetNumJoints() -1;
}

void SingleFrameKeys(sf::Keyboard::Key key, bool release, int x, int y) {
//PrintInt (key);
	must_render = true;
	int keyfact;
	lastframe = TestFrame.numFrames() != 1;
	TKeyframe *frame = TestFrame.GetFrame(curr_frame);

	// setting the camera change state
	if (key == sf::Keyboard::F1) {GluCamera.turnright = !release; return;}
	else if (key == sf::Keyboard::F2) { GluCamera.turnleft = !release; return; }
	if (key == sf::Keyboard::F3) { GluCamera.nearer = !release; return; }
	else if (key == sf::Keyboard::F4) { GluCamera.farther = !release; return; }

	// additional keys if needed
	if (key == sf::Keyboard::LShift || key == sf::Keyboard::RShift) shift = !release;
	if (key == sf::Keyboard::LControl) control = !release;
	if (key == sf::Keyboard::LAlt) alt = !release;
	if (shift) keyfact = -1;
	else keyfact = 1;

	if (release) return;

	switch (key) {
		case sf::Keyboard::Y:
		case sf::Keyboard::J:
			if (ToolsFinalStage()) {
				SaveToolCharacter();
				SaveToolFrame();
				State::manager.RequestQuit();
			}
			break;
		case sf::Keyboard::N:
			if (ToolsFinalStage()) State::manager.RequestQuit();
			break;

		case sf::Keyboard::Escape:
		case sf::Keyboard::Q:
			QuitTool();
			break;
		case sf::Keyboard::S:
			SaveToolFrame();
			break;
		case sf::Keyboard::Tab:
			SetToolMode(0);
			break;

		case sf::Keyboard::A:
			TestFrame.AddFrame();
			SetFrameChanged(true);
			break;
		case sf::Keyboard::Insert:
			TestFrame.InsertFrame(curr_frame);
			SetFrameChanged(true);
			break;
		case sf::Keyboard::Delete:
			curr_frame = TestFrame.DeleteFrame(curr_frame);
			SetFrameChanged(true);
			break;
		case sf::Keyboard::PageDown:
			if (curr_frame < TestFrame.numFrames()-1) curr_frame++;
			break;
		case sf::Keyboard::PageUp:
			if (curr_frame > 0) curr_frame--;
			break;
		case sf::Keyboard::Up:
			if (curr_joint > 0) curr_joint--;
			break;
		case sf::Keyboard::Down:
			if (curr_joint < last_joint) curr_joint++;
			break;
		case sf::Keyboard::Right:
			if (curr_joint < 4) frame->val[curr_joint] += 0.05;
			else frame->val[curr_joint] += 1;
			SetFrameChanged(true);
			break;
		case sf::Keyboard::Left:
			if (curr_joint < 4) frame->val[curr_joint] -= 0.05;
			else frame->val[curr_joint] -= 1;
			SetFrameChanged(true);
			break;
		case sf::Keyboard::Num0:
			frame->val[curr_joint] = 0.0;
			SetFrameChanged(true);
			break;
		case sf::Keyboard::Space:
			if (curr_joint < 4) frame->val[curr_joint] += 0.05 * keyfact;
			else frame->val[curr_joint] += 1 * keyfact;
			SetFrameChanged(true);
			break;

		case sf::Keyboard::Return:
			TestFrame.InitTest(ref_position, &TestChar);
			SetToolMode(2);
			must_render = true;
			break;

		case sf::Keyboard::M :
			TestChar.useMaterials = !TestChar.useMaterials;
			break;
		case sf::Keyboard::H:
			TestChar.useHighlighting = !TestChar.useHighlighting;
			break;
		case sf::Keyboard::C:
			if (control) TestFrame.CopyToClipboard(curr_frame);
			else TestFrame.ClearFrame(curr_frame);
			SetFrameChanged(true);
			break;
		case sf::Keyboard::V:
			if (control) TestFrame.PasteFromClipboard(curr_frame);
			SetFrameChanged(true);
			break;
		case sf::Keyboard::P:
			if (curr_frame>0)
				TestFrame.CopyFrame(curr_frame-1, curr_frame);
			break;
		case sf::Keyboard::F10:
			Winsys.TakeScreenshot();
			break;

		case sf::Keyboard::Num1:
			GluCamera.angle = 0;
			break;
		case sf::Keyboard::Num2:
			GluCamera.angle = 45;
			break;
		case sf::Keyboard::Num3:
			GluCamera.angle = 90;
			break;
		case sf::Keyboard::Num4:
			GluCamera.angle = 135;
			break;
		case sf::Keyboard::Num5:
			GluCamera.angle = 180;
			break;
		case sf::Keyboard::Num6:
			GluCamera.angle = 225;
			break;
		case sf::Keyboard::Num7:
			GluCamera.angle = 270;
			break;
		case sf::Keyboard::Num8:
			GluCamera.angle = 315;
			break;
		default:
			break;
	}
}

void SingleFrameMouse(int button, int state, int x, int y) {
	must_render = true;
	if (ToolsFinalStage()) return;

	if (button == 4) {
		GluCamera.distance += 0.1;
	} else if (button == 5) {
		GluCamera.distance -= 0.1;
	}
}

void SingleFrameMotion(int x, int y) {
}

void PrintFrameParams(int ytop, TKeyframe *frame) {
	int offs = 18;

	for (int i=0; i<=last_joint; i++) {
		if (i == curr_joint) FT.SetColor(colYellow);
		else FT.SetColor(colLGrey);

		int x = ITrunc(i, jointbase) * 140 + 20;
		int y = IFrac(i, jointbase) * offs + ytop;

		FT.DrawString(x, y, TestFrame.GetJointName(i));
		if (i < 4) FT.DrawString(x+80, y, Float_StrN(frame->val[i], 2));
		else FT.DrawString(x+80, y, Int_StrN((int)frame->val[i]));
	}
}

void RenderSingleFrame(float timestep) {
	if (!must_render) return;

	// ------------------ 3d scenery ----------------------------------
	ScopedRenderMode rm1(TUX);
	ClearRenderContext(colDDBackgr);

	const std::string& hlname = TestFrame.GetHighlightName(curr_joint);
	TestChar.highlight_node = TestChar.GetNodeName(hlname);

	glPushMatrix();
	SetToolLight();
	GluCamera.Update(timestep);

	TestFrame.CalcKeyframe(curr_frame, &TestChar, ref_position);
	TestChar.Draw();
	glPopMatrix();

	// ----------------- 2d screen ------------------------------------
	Setup2dScene();
	ScopedRenderMode rm2(TEXFONT);

	if (FrameHasChanged()) DrawChanged();

	FT.SetProps("bold", 20, colYellow);
	FT.DrawString(-1, 10, "Keyframe mode");

	FT.SetProps("normal", 16);
	for (std::size_t i=0; i<TestFrame.numFrames(); i++) {
		if (i != curr_frame) {
			FT.SetColor(colLGrey);
			FT.SetFont("normal");
		} else {
			FT.SetColor(colYellow);
			FT.SetFont("bold");
		}
		int xl = ITrunc((int)i, framebase) * 100 + 20;
		int yt = IFrac((int)i, framebase) * 18 + 20;
		FT.DrawString(xl, yt, Int_StrN((int)i));
	}

	FT.SetFont("normal");
	FT.SetColor(colLGrey);
	PrintFrameParams(Winsys.resolution.height - 330, TestFrame.GetFrame(curr_frame));

	if (ToolsFinalStage()) {
		FT.SetSize(20);
		FT.SetColor(colYellow);
		FT.DrawString(-1, Winsys.resolution.height - 50, "Quit program. Save character list (y/n)");
	}

	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
	must_render = false;
}


// --------------------------------------------------------------------
//				frame sequence
// --------------------------------------------------------------------

void SequenceKeys(sf::Keyboard::Key key, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case sf::Keyboard::Return:
			keyrun = true;
			break;
		case sf::Keyboard::Escape:
		case sf::Keyboard::Tab:
			SetToolMode(1);
			break;
		case sf::Keyboard::Q:
			QuitTool();
			break;
		default:
			break;
	}
}

void SequenceMouse(int button, int state, int x, int y) {}
void SequenceMotion(int x, int y) {}

void RenderSequence(float timestep) {
	ScopedRenderMode rm(TUX);
	ClearRenderContext(colDDBackgr);

	GluCamera.Update(timestep);
	if (TestFrame.active) TestFrame.UpdateTest(timestep, &TestChar);
	else if (keyrun) {
		TestFrame.InitTest(NullVec3, &TestChar);
		keyrun = false;
	}

	glPushMatrix();
	TestChar.Draw();
	glPopMatrix();

	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
}
