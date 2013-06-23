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

#include "tool_frame.h"
#include "tools.h"
#include "ogl.h"
#include "keyframe.h"
#include "font.h"
#include "textures.h"
#include "spx.h"
#include "tux.h"
#include "winsys.h"

static size_t curr_frame = 0;
static int curr_joint = 0;
static int last_joint = 0;
static TVector3 ref_position(0, 0, 0);
static bool must_render = true;
static int framebase = 24;
static int jointbase = 16;
static bool shift = false;
static bool control = false;
static bool alt = false;
static bool lastframe = 0;
static bool keyrun = false;

void InitFrameTools () {
	framebase = (int)((Winsys.resolution.height - 350) / 18);
	if (TestFrame.numFrames() < 1) TestFrame.AddFrame ();
	curr_joint = 0;
	last_joint = TestFrame.GetNumJoints () -1;
}

void SingleFrameKeys (unsigned int key, bool special, bool release, int x, int y) {
//PrintInt (key);
	must_render = true;
	int keyfact;
	lastframe = TestFrame.numFrames() != 1;
	TKeyframe *frame = TestFrame.GetFrame (curr_frame);

	// setting the camera change state
	if (key == SDLK_F1) {GluCamera.turnright = !release; return;}
	else if (key == SDLK_F2) {GluCamera.turnleft = !release; return;}
	if (key == SDLK_F3) {GluCamera.nearer = !release; return;}
	else if (key == SDLK_F4) {GluCamera.farther = !release; return;}

	// additional keys if needed
	if (key == SDLK_LSHIFT) shift = !release;
	if (key == SDLK_LCTRL) control = !release;
	if (key == SDLK_LALT) alt = !release;
	if (shift) keyfact = -1; else keyfact = 1;

	if (release) return;

	switch (key) {
		case SDLK_y: case SDLK_j:
			if (ToolsFinalStage ()) {
				SaveToolCharacter ();
				SaveToolFrame ();
				State::manager.RequestQuit();
			} break;
		case SDLK_n: if (ToolsFinalStage ()) State::manager.RequestQuit(); break;

		case SDLK_ESCAPE: case SDLK_q: QuitTool (); break;
		case SDLK_s: SaveToolFrame (); break;
		case SDLK_TAB: SetToolMode (0); break;

		case SDLK_a:
			TestFrame.AddFrame ();
			SetFrameChanged (true);
			break;
		case SDLK_INSERT:
			TestFrame.InsertFrame (curr_frame);
			SetFrameChanged (true);
			break;
		case SDLK_DELETE:
			curr_frame = TestFrame.DeleteFrame (curr_frame);
			SetFrameChanged (true);
			break;
		case SDLK_PAGEDOWN: if (curr_frame < TestFrame.numFrames()-1) curr_frame++; break;
		case SDLK_PAGEUP: if (curr_frame > 0) curr_frame--; break;
		case SDLK_UP: if (curr_joint > 0) curr_joint--; break;
		case SDLK_DOWN: if (curr_joint < last_joint) curr_joint++; break;
		case SDLK_RIGHT:
				if (curr_joint < 4) frame->val[curr_joint] += 0.05;
				else frame->val[curr_joint] += 1;
				SetFrameChanged (true);
				break;
		case SDLK_LEFT:
				if (curr_joint < 4) frame->val[curr_joint] -= 0.05;
				else frame->val[curr_joint] -= 1;
				SetFrameChanged (true);
				break;
		case SDLK_0:
			frame->val[curr_joint] = 0.0;
			SetFrameChanged (true);
			break;
		case SDLK_SPACE:
			if (curr_joint < 4) frame->val[curr_joint] += 0.05 * keyfact;
			else frame->val[curr_joint] += 1 * keyfact;
			SetFrameChanged (true);
			break;

		case SDLK_RETURN:
			TestFrame.InitTest (ref_position, &TestChar);
			SetToolMode (2);
			must_render = true;
			break;

		case SDLK_m: TestChar.useMaterials = !TestChar.useMaterials; break;
		case SDLK_h: TestChar.useHighlighting = !TestChar.useHighlighting; break;
		case SDLK_c:
			if (control) TestFrame.CopyToClipboard (curr_frame);
			else TestFrame.ClearFrame (curr_frame);
			SetFrameChanged (true);
			break;
		case SDLK_v:
			if (control) TestFrame.PasteFromClipboard (curr_frame);
			SetFrameChanged (true);
			break;
		case SDLK_p: if (curr_frame>0)
			TestFrame.CopyFrame (curr_frame-1, curr_frame); break;
		case SDLK_F10: ScreenshotN (); break;

		case SDLK_1: GluCamera.angle = 0; break;
		case SDLK_2: GluCamera.angle = 45; break;
		case SDLK_3: GluCamera.angle = 90; break;
		case SDLK_4: GluCamera.angle = 135; break;
		case SDLK_5: GluCamera.angle = 180; break;
		case SDLK_6: GluCamera.angle = 225; break;
		case SDLK_7: GluCamera.angle = 270; break;
		case SDLK_8: GluCamera.angle = 315; break;
	}
}

void SingleFrameMouse (int button, int state, int x, int y) {
	must_render = true;
	if (ToolsFinalStage ()) return;

	if (button == 4) {
		GluCamera.distance += 0.1;
	} else if (button == 5) {
		GluCamera.distance -= 0.1;
	}
}

void SingleFrameMotion (int x, int y) {
}

void PrintFrameParams (int ytop, TKeyframe *frame) {
	int y, x;
	int offs = 18;

	for (int i=0; i<=last_joint; i++) {
		if (i == curr_joint) FT.SetColor (colYellow);
		else FT.SetColor (colLGrey);

		x = ITrunc (i, jointbase) * 140 + 20;
		y = IFrac (i, jointbase) * offs + ytop;

		FT.DrawString (x, y, TestFrame.GetJointName(i));
		if (i < 4) FT.DrawString (x+80, y, Float_StrN (frame->val[i], 2));
		else FT.DrawString (x+80, y, Float_StrN (frame->val[i], 0));
	}
}

void RenderSingleFrame (double timestep) {
	if (!must_render) return;
	check_gl_error ();

	// ------------------ 3d scenery ----------------------------------
	ScopedRenderMode rm1(TUX);
    ClearRenderContext (colDDBackgr);

	const string& hlname = TestFrame.GetHighlightName (curr_joint);
	TestChar.highlight_node = TestChar.GetNodeName (hlname);

	glPushMatrix ();
	SetToolLight ();
	GluCamera.Update (timestep);

	TestFrame.CalcKeyframe (curr_frame, &TestChar, ref_position);
	TestChar.Draw ();
	glPopMatrix ();

	// ----------------- 2d screen ------------------------------------
	SetupGuiDisplay ();
	ScopedRenderMode rm2(TEXFONT);

	if (FrameHasChanged ()) DrawChanged ();

	FT.SetFont ("bold");
	FT.SetSize (20);
	FT.SetColor (colYellow);
	FT.DrawString (-1, 10, "Keyframe mode");

	FT.SetFont ("normal");
	FT.SetSize (16);
	int xl, yt;
	for (size_t i=0; i<TestFrame.numFrames(); i++) {
		if (i != curr_frame) {
			FT.SetColor (colLGrey);
			FT.SetFont ("normal");
		} else {
			FT.SetColor (colYellow);
			FT.SetFont ("bold");
		}
		xl = ITrunc ((int)i, framebase) * 100 + 20;
		yt = IFrac ((int)i, framebase) * 18 + 20;
		FT.DrawString (xl, yt, Int_StrN ((int)i));
	}

	FT.SetFont ("normal");
	FT.SetColor (colLGrey);
	PrintFrameParams (Winsys.resolution.height - 330, TestFrame.GetFrame (curr_frame));

	if (ToolsFinalStage ()) {
		FT.SetSize (20);
		FT.SetColor (colYellow);
		FT.DrawString (-1, Winsys.resolution.height - 50, "Quit program. Save character list (y/n)");
	}

	Reshape (Winsys.resolution.width, Winsys.resolution.height);
    Winsys.SwapBuffers();
	must_render = false;
}


// --------------------------------------------------------------------
//				frame sequence
// --------------------------------------------------------------------

void SequenceKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case SDLK_RETURN: keyrun = true; break;
		case SDLK_ESCAPE: case SDLK_TAB: SetToolMode (1); break;
		case SDLK_q: QuitTool (); break;
	}
}

void SequenceMouse (int button, int state, int x, int y) {}
void SequenceMotion (int x, int y) {}

void RenderSequence (double timestep) {
	check_gl_error();
	ScopedRenderMode rm(TUX);
    ClearRenderContext (colDDBackgr);

	GluCamera.Update (timestep);
	if (TestFrame.active) TestFrame.UpdateTest (timestep, &TestChar);
	else if (keyrun) {
		TestFrame.InitTest (NullVec, &TestChar);
		keyrun = false;
	}

	glPushMatrix ();
	TestChar.Draw ();
	glPopMatrix ();

	Reshape (Winsys.resolution.width, Winsys.resolution.height);
    Winsys.SwapBuffers();
}
