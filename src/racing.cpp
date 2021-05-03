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
#include "tux.h"
#include <algorithm>

#define MAX_JUMP_AMT 1.0
#define ROLL_DECAY 0.2
#define JUMP_MAX_START_HEIGHT 0.30

CRacing Racing;

static bool right_turn;
static bool left_turn;
static bool stick_turn;
static float stick_turnfact;
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

void CRacing::Keyb(sf::Keyboard::Key key, bool release, int x, int y) {
	switch (key) {
		// steering flipflops
		case sf::Keyboard::Up:
			key_paddling = !release;
			break;
		case sf::Keyboard::Down:
			key_braking = !release;
			break;
		case sf::Keyboard::Left:
			left_turn = !release;
			break;
		case sf::Keyboard::Right:
			right_turn = !release;
			break;
		case sf::Keyboard::Space:
			key_charging = !release;
			break;
		case sf::Keyboard::T:
			trick_modifier = !release;
			break;

		// mode changing and other actions
		case sf::Keyboard::Escape:
			if (!release) {
				g_game.raceaborted = true;
				g_game.race_result = -1;
				State::manager.RequestEnterState(GameOver);
			}
			break;
		case sf::Keyboard::P:
			if (!release) State::manager.RequestEnterState(Paused);
			break;
		case sf::Keyboard::R:
			if (!release) State::manager.RequestEnterState(Reset);
			break;
		case sf::Keyboard::S:
			if (!release) Winsys.TakeScreenshot();
			break;

		// view changing
		case sf::Keyboard::Num1:
			if (!release) {
				set_view_mode(g_game.player->ctrl, ABOVE);
				param.view_mode = ABOVE;
			}
			break;
		case sf::Keyboard::Num2:
			if (!release) {
				set_view_mode(g_game.player->ctrl, FOLLOW);
				param.view_mode = FOLLOW;
			}
			break;
		case sf::Keyboard::Num3:
			if (!release) {
				set_view_mode(g_game.player->ctrl, BEHIND);
				param.view_mode = BEHIND;
			}
			break;

		// toggle display settings
		case sf::Keyboard::H:
			if (!release) param.show_hud = !param.show_hud;
			break;
		case sf::Keyboard::F:
			if (!release) param.display_fps = !param.display_fps;
			break;
		case sf::Keyboard::F5:
			if (!release) sky = !sky;
			break;
		case sf::Keyboard::F6:
			if (!release) fog = !fog;
			break;
		case sf::Keyboard::F7:
			if (!release) terr = !terr;
			break;
		case sf::Keyboard::F8:
			if (!release) trees = !trees;
			break;
		default:
			break;
	}
}

void CRacing::Jaxis(int axis, float value) {
	if (axis == 0) { 	// left and right
		stick_turn = ((value < -0.2) || (value > 0.2));
		if (stick_turn) stick_turnfact = value;
		else stick_turnfact = 0.0;
	} else if (axis == 1) {	// paddling and braking
		stick_paddling = (value < -0.3);
		stick_braking = (value > 0.3);
	}
}

void CRacing::Jbutt(int button, bool pressed) {
	switch (button) {
		case 0:
			key_paddling = pressed;
			break;
		case 1:
			trick_modifier = pressed;
			break;
		case 2:
			key_braking = pressed;
			break;
		case 3:
			key_charging = pressed;
			break;
	}
}

static void CalcJumpEnergy(float time_step) {
	CControl *ctrl = g_game.player->ctrl;

	if (ctrl->jump_charging) {
		ctrl->jump_amt = std::min(MAX_JUMP_AMT, g_game.time - charge_start_time);
	} else if (ctrl->jumping) {
		ctrl->jump_amt *= (1.0 - (g_game.time - ctrl->jump_start_time) /
		                   JUMP_FORCE_DURATION);
	} else {
		ctrl->jump_amt = 0;
	}
}

static int CalcSoundVol(float fact) {
	return std::min(param.sound_volume * fact, 100.f);
}

static void SetSoundVolumes() {
	Sound.SetVolume("pickup1",    CalcSoundVol(1.0f));
	Sound.SetVolume("pickup2",    CalcSoundVol(0.8f));
	Sound.SetVolume("pickup3",    CalcSoundVol(0.8f));
	Sound.SetVolume("snow_sound", CalcSoundVol(1.5f));
	Sound.SetVolume("ice_sound",  CalcSoundVol(0.6f));
	Sound.SetVolume("rock_sound", CalcSoundVol(1.1f));
}

// ---------------------------- init ----------------------------------
void CRacing::Enter() {
	CControl *ctrl = g_game.player->ctrl;

	if (param.view_mode < 0 || param.view_mode >= NUM_VIEW_MODES) {
		param.view_mode = ABOVE;
	}
	set_view_mode(ctrl, param.view_mode);

	ctrl->turn_fact = 0.0;
	ctrl->turn_animation = 0.0;
	ctrl->is_braking = false;
	ctrl->is_paddling = false;
	ctrl->jumping = false;
	ctrl->jump_charging = false;

	key_paddling = false;
	key_braking = false;
	left_turn = false;
	right_turn = false;
	key_charging = false;
	trick_modifier = false;
	stick_paddling = false;
	stick_braking = false;
	stick_turn = false;

	lastsound = -1;
	newsound = -1;

	if (State::manager.PreviousState() != &Paused) ctrl->Init();
	g_game.raceaborted = false;

	SetSoundVolumes();
	Music.PlayTheme(g_game.theme_id, MUS_RACING);

	g_game.finish = false;

	Winsys.KeyRepeat(false);
}

// -------------------- sound -----------------------------------------

// this function is not used yet.
/*static int SlideVolume(CControl *ctrl, double speed, int typ) {
	if (typ == 1) {	// only at paddling or braking
		return (int)(std::min((((std::pow(ctrl->turn_fact, 2) * 128)) +
		                  (ctrl->is_braking ? 128:0) +
		                  (ctrl->jumping ? 128:0) + 20) * (speed / 10), 128.0));
	} else { 	// always
		return (int)(128 * std::pow((speed/2),2));
	}
}*/

static void PlayTerrainSound(CControl *ctrl, bool airborne) {
	if (airborne == false) {
		int terridx = Course.GetTerrainIdx(ctrl->cpos.x, ctrl->cpos.z, 0.5);
		if (terridx >= 0)
			newsound = (int)Course.TerrList[terridx].sound;
		else
			newsound = -1;
	} else
		newsound = -1;

	if ((newsound != lastsound) && (lastsound >= 0))
		Sound.Halt(lastsound);
	if (newsound >= 0)
		Sound.Play(newsound, true);

	lastsound = newsound;
}

// ----------------------- controls -----------------------------------
static void CalcSteeringControls(CControl *ctrl, float time_step) {
	if (stick_turn) {
		ctrl->turn_fact = stick_turnfact;
		ctrl->turn_animation += ctrl->turn_fact * 2 * time_step;
		ctrl->turn_animation = clamp(-1.0, ctrl->turn_animation, 1.0);
	} else if (left_turn ^ right_turn) {
		if (left_turn) ctrl->turn_fact = -1.0;
		else ctrl->turn_fact = 1.0;
		ctrl->turn_animation += ctrl->turn_fact * 2 * time_step;
		ctrl->turn_animation = clamp(-1.0, ctrl->turn_animation, 1.0);
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
	CalcJumpEnergy(time_step);
	if ((charge) && !ctrl->jump_charging && !ctrl->jumping) {
		ctrl->jump_charging = true;
		charge_start_time = g_game.time;
	}
	if ((invcharge) && ctrl->jump_charging) {
		ctrl->jump_charging = false;
		ctrl->begin_jump = true;
	}
}

static void CalcFinishControls(CControl *ctrl, float timestep, bool airborne) {
	double speed = ctrl->cvel.Length();
	double dir_angle = RADIANS_TO_ANGLES(std::atan(ctrl->cvel.x / ctrl->cvel.z));

	if (std::fabs(dir_angle) > 5 && speed > 5) {
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

static void CalcTrickControls(CControl *ctrl, float time_step, bool airborne) {
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

void CRacing::Loop(float time_step) {
	CControl *ctrl = g_game.player->ctrl;
	double ycoord = Course.FindYCoord(ctrl->cpos.x, ctrl->cpos.z);
	bool airborne = (bool)(ctrl->cpos.y > (ycoord + JUMP_MAX_START_HEIGHT));

	ClearRenderContext();
	Env.SetupFog();
	CalcTrickControls(ctrl, time_step, airborne);

	if (!g_game.finish) CalcSteeringControls(ctrl, time_step);
	else CalcFinishControls(ctrl, time_step, airborne);
	PlayTerrainSound(ctrl, airborne);

//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	ctrl->UpdatePlayerPos(time_step);
//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	if (g_game.finish) IncCameraDistance(time_step);
	update_view(ctrl, time_step);
	UpdateTrackmarks(ctrl);

	SetupViewFrustum(ctrl);
	if (sky) Env.DrawSkybox(ctrl->viewpos);
	if (fog) Env.DrawFog();
	Env.SetupLight();
	if (terr) RenderCourse();
	DrawTrackmarks();
	if (trees) DrawTrees();
	if (param.perf_level > 2) {
		update_particles(time_step);
		draw_particles(ctrl);
	}
	g_game.character->shape->Draw();
	UpdateWind(time_step);
	UpdateSnow(time_step, ctrl);
	DrawSnow(ctrl);
	DrawHud(ctrl);

	Reshape(Winsys.resolution.width, Winsys.resolution.height);
	Winsys.SwapBuffers();
	if (g_game.finish == false) g_game.time += time_step;
}

void CRacing::Exit() {
	Winsys.KeyRepeat(true);
	Sound.HaltAll();
	break_track_marks();
}
