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

#include "racing.h"
#include "audio.h"
#include "course_render.h"
#include "ogl.h"
#include "view.h"
#include "env.h"
#include "track_marks.h"
#include "hud.h"
#include "course.h"
#include "particles.h"
#include "textures.h"
#include "game_ctrl.h"
#include "game_over.h"
#include "paused.h"
#include "reset.h"
#include "winsys.h"
#include "physics.h"

#define MAX_JUMP_AMT 1.0
#define ROLL_DECAY 0.2
#define JUMP_MAX_START_HEIGHT 0.30

CRacing Racing;

static bool right_turn;
static bool left_turn;
static bool stick_turn;
static double stick_turnfact;
static bool key_paddling;
static bool stick_paddling;
static bool key_charging;
static bool stick_charging;
static bool key_braking;
static bool stick_braking;
static double charge_start_time;
static bool trick_modifier;

static bool sky = true;
static bool fog = true;
static bool terr = true;
static bool trees = true;

static int newsound = -1;
static int lastsound = -1;

void CRacing::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	switch (key) {
		// steering flipflops
		case SDLK_UP: key_paddling = !release; break;
		case SDLK_DOWN: key_braking = !release; break;
		case SDLK_LEFT: left_turn = !release; break;
		case SDLK_RIGHT: right_turn = !release; break;
		case SDLK_SPACE: key_charging = !release; break;
		case SDLK_t: trick_modifier = !release; break;
		// mode changing and other actions
		case SDLK_ESCAPE: if (!release) {
			g_game.raceaborted = true;
			g_game.race_result = -1;
			State::manager.RequestEnterState (GameOver);
		} break;
		case SDLK_p: if (!release) State::manager.RequestEnterState (Paused); break;
		case SDLK_r: if (!release) State::manager.RequestEnterState (Reset); break;
		case SDLK_s: if (!release) ScreenshotN (); break;

		// view changing
		case SDLK_1: if (!release) {
			set_view_mode (Players.GetCtrl (g_game.player_id), ABOVE);
			param.view_mode = ABOVE;
		} break;
		case SDLK_2: if (!release) {
			set_view_mode (Players.GetCtrl (g_game.player_id), FOLLOW);
			param.view_mode = FOLLOW;
		} break;
		case SDLK_3: if (!release) {
			set_view_mode (Players.GetCtrl (g_game.player_id), BEHIND);
			param.view_mode = BEHIND;
		} break;

		// toggle
		case SDLK_h:  if (!release) param.show_hud = !param.show_hud; break;
		case SDLK_f:  if (!release) param.display_fps = !param.display_fps; break;
		case SDLK_F5: if (!release) sky = !sky; break;
		case SDLK_F6: if (!release) fog = !fog; break;
		case SDLK_F7: if (!release) terr = !terr; break;
		case SDLK_F8: if (!release) trees = !trees; break;
	}
}

void CRacing::Jaxis (int axis, double value) {
	if (axis == 0) { 	// left and right
		stick_turn = ((value < -0.2) || (value > 0.2));
		if (stick_turn) stick_turnfact = value; else stick_turnfact = 0.0;
	} else if (axis == 1) {	// paddling and braking
		stick_paddling = (value < -0.3);
		stick_braking = (value > 0.3);
	}
}

void CRacing::Jbutt (int button, int state) {
	if (button == 0) {
		key_charging = state != 0;
	} else if (button == 1) {
//		key_charging = (bool) state;
	}
}

void CalcJumpEnergy (double time_step) {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);

	if (ctrl->jump_charging) {
		ctrl->jump_amt = min (MAX_JUMP_AMT, g_game.time - charge_start_time);
	} else if (ctrl->jumping) {
		ctrl->jump_amt *=  (1.0 - (g_game.time - ctrl->jump_start_time) /
			JUMP_FORCE_DURATION);
	} else {
		ctrl->jump_amt = 0;
	}
}

int CalcSoundVol (double fact) {
	double vv = (double) param.sound_volume * fact;
	if (vv > 120) vv = 120;
	return (int) vv;
}

void SetSoundVolumes () {
	Sound.SetVolume ("pickup1",    CalcSoundVol (1.0));
	Sound.SetVolume ("pickup2",    CalcSoundVol (0.8));
	Sound.SetVolume ("pickup3",    CalcSoundVol (0.8));
	Sound.SetVolume ("snow_sound", CalcSoundVol (1.5));
	Sound.SetVolume ("ice_sound",  CalcSoundVol (0.6));
	Sound.SetVolume ("rock_sound", CalcSoundVol (1.1));
}

// ---------------------------- init ----------------------------------
void CRacing::Enter (void) {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);

    if (param.view_mode < 0 || param.view_mode >= NUM_VIEW_MODES) {
		param.view_mode = ABOVE;
    }
    set_view_mode (ctrl, (TViewMode)param.view_mode);
    left_turn = right_turn = trick_modifier = false;

    ctrl->turn_fact = 0.0;
    ctrl->turn_animation = 0.0;
    ctrl->is_braking = false;
    ctrl->is_paddling = false;
    ctrl->jumping = false;
    ctrl->jump_charging = false;

	lastsound = -1;
	newsound = -1;

	if (State::manager.PreviousState() != &Paused) ctrl->Init ();
    g_game.raceaborted = false;

	SetSoundVolumes ();
	Music.PlayTheme (g_game.theme_id, MUS_RACING);

	g_game.finish = false;
}

// -------------------- sound -----------------------------------------

// this function is not used yet.
int SlideVolume (CControl *ctrl, double speed, int typ) {
	if (typ == 1) {	// only at paddling or braking
	return (int)(MIN ((((pow(ctrl->turn_fact, 2) * 128)) +
		 (ctrl->is_braking ? 128:0) +
		 (ctrl->jumping ? 128:0) + 20) * (speed / 10), 128));
	} else { 	// always
		return (int)(128 * pow((speed/2),2));
	}
}

void PlayTerrainSound (CControl *ctrl, bool airborne) {
	int terridx = -1;
	TTerrType *TerrList = &Course.TerrList[0];

	if (airborne == false) {
		terridx = Course.GetTerrainIdx (ctrl->cpos.x, ctrl->cpos.z, 0.5);
		if (terridx >= 0) {
			newsound = (int)Sound.GetSoundIdx (TerrList[terridx].sound);
		} else newsound = -1;
	} else newsound = -1;
	if ((newsound != lastsound) && (lastsound >= 0)) Sound.Halt (lastsound);
	if (newsound >= 0) Sound.Play (newsound, -1);

	lastsound = newsound;
}

// ----------------------- controls -----------------------------------
void CalcSteeringControls (CControl *ctrl, double time_step) {
	if (stick_turn) {
		ctrl->turn_fact = stick_turnfact;
		ctrl->turn_animation += ctrl->turn_fact * 2 * time_step;
		ctrl->turn_animation = min (1.0, max (-1.0, ctrl->turn_animation));
	} else if (left_turn ^ right_turn) {
		if (left_turn) ctrl->turn_fact = -1.0;
		else ctrl->turn_fact = 1.0;
		ctrl->turn_animation += ctrl->turn_fact * 2 * time_step;
		ctrl->turn_animation = min (1.0, max (-1.0, ctrl->turn_animation));
	} else {
		ctrl->turn_fact = 0.0;
		if (time_step < ROLL_DECAY) {
	    	ctrl->turn_animation *= 1.0 - time_step / ROLL_DECAY;
		} else {
	    	ctrl->turn_animation = 0.0;
		}
	}

	bool paddling = key_paddling || stick_paddling;
    if (paddling && ctrl->is_paddling == false) {
		ctrl->is_paddling = true;
		ctrl->paddle_time = g_game.time;
    }

	bool braking = key_braking || stick_braking;
    ctrl->is_braking = braking;

	bool charge = key_charging || stick_charging;
	bool invcharge = !key_charging && !stick_charging;
	CalcJumpEnergy (time_step);
    if ((charge) && !ctrl->jump_charging && !ctrl->jumping) {
		ctrl->jump_charging = true;
		charge_start_time = g_game.time;
    }
    if ((invcharge) && ctrl->jump_charging) {
		ctrl->jump_charging = false;
		ctrl->begin_jump = true;
    }
}

void CalcFinishControls (CControl *ctrl, double timestep, bool airborne) {
	TVector3 movdir = ctrl->cvel;
	double speed = NormVector (movdir);
	double dir_angle = atan (movdir.x / movdir.z) * 57.3;

	if (fabs (dir_angle) > 5 && speed > 5) {
		ctrl->turn_fact = dir_angle / 20;
		if (ctrl->turn_fact < -1) ctrl->turn_fact = -1;
		if (ctrl->turn_fact > 1) ctrl->turn_fact = 1;
		ctrl->turn_animation += ctrl->turn_fact * 2 * timestep;
	} else {
		ctrl->turn_fact = 0;
		if (timestep < ROLL_DECAY) {
	    	ctrl->turn_animation *= 1.0 - timestep / ROLL_DECAY;
		} else ctrl->turn_animation = 0.0;
	}
}

// ----------------------- trick --------------------------------------

void CalcTrickControls (CControl *ctrl, double time_step, bool airborne) {
	if (airborne && trick_modifier) {
		if (left_turn) ctrl->roll_left = true;
		if (right_turn) ctrl->roll_right = true;
		if (key_paddling) ctrl->front_flip = true;
		if (ctrl->is_braking) ctrl->back_flip = true;
	}

	if (ctrl->roll_left || ctrl->roll_right) {
		ctrl->roll_factor += (ctrl->roll_left ? -1 : 1) * 0.15 * time_step / 0.05;
		if (ctrl->roll_factor  > 1 || ctrl->roll_factor < -1) {
			ctrl->roll_factor = 0;
			ctrl->roll_left = ctrl->roll_right = false;
		}
    }
	if (ctrl->front_flip || ctrl->back_flip) {
		ctrl->flip_factor += (ctrl->back_flip ? -1 : 1) * 0.15 * time_step / 0.05;
		if (ctrl->flip_factor > 1 || ctrl->flip_factor < -1) {
			ctrl->flip_factor = 0;
			ctrl->front_flip = ctrl->back_flip = false;
		}
    }
}

// ====================================================================
//					loop
// ====================================================================

void CRacing::Loop (double time_step) {
    CControl *ctrl = Players.GetCtrl (g_game.player_id);
	double ycoord = Course.FindYCoord (ctrl->cpos.x, ctrl->cpos.z);
	bool airborne = (bool) (ctrl->cpos.y > (ycoord + JUMP_MAX_START_HEIGHT));

    check_gl_error();
    ClearRenderContext ();
	Env.SetupFog ();
	Music.Update ();
	CalcTrickControls (ctrl, time_step, airborne);

	if (!g_game.finish) CalcSteeringControls (ctrl, time_step);
		else CalcFinishControls (ctrl, time_step, airborne);
	PlayTerrainSound (ctrl, airborne);

//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	ctrl->UpdatePlayerPos (time_step);
//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	if (g_game.finish) IncCameraDistance (time_step);
	update_view (ctrl, time_step);
	UpdateTrackmarks (ctrl);

    SetupViewFrustum (ctrl);
	if (sky) Env.DrawSkybox (ctrl->viewpos);
	if (fog) Env.DrawFog ();
	void SetupLight ();
	if (terr) RenderCourse ();
	DrawTrackmarks ();
	if (trees) DrawTrees ();
	if (param.perf_level > 2) {
		update_particles (time_step);
		draw_particles (ctrl);
    }
	Char.Draw (g_game.char_id);
	UpdateWind (time_step);
	UpdateSnow (time_step, ctrl);
	DrawSnow (ctrl);
	DrawHud (ctrl);

	Reshape (Winsys.resolution.width, Winsys.resolution.height);
    Winsys.SwapBuffers ();
	if (g_game.finish == false) g_game.time += time_step;
}
// ---------------------------------- term ------------------
void CRacing::Exit() {
	Sound.HaltAll ();
    break_track_marks ();
}
