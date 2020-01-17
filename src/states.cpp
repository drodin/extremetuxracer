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

#include "states.h"
#include "ogl.h"
#include "winsys.h"
#ifdef ANDROID
#include "game_ctrl.h"
#include "score.h"
#include "audio.h"
#endif

State::Manager State::manager(Winsys);

State::Manager::~Manager() {
	if (current)
		current->Exit();
}

void State::Manager::Run(State& entranceState) {
	current = &entranceState;
	current->Enter();
	while (!quit) {
		PollEvent();
		if (next)
			EnterNextState();
		CallLoopFunction();
	}
	current->Exit();
	previous = current;
	current = nullptr;
}

void State::Manager::EnterNextState() {
	current->Exit();
	previous = current;
	current = next;
	next = nullptr;
	current->Enter();
}

void State::Manager::PollEvent() {
	sf::Event event;
	sf::Keyboard::Key key;

	while (Winsys.PollEvent(event)) {
		if (!next) {
			switch (event.type) {
				case sf::Event::KeyPressed:
					key = event.key.code;
					current->Keyb(key, false, sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
					break;

				case sf::Event::KeyReleased:
					key = event.key.code;
					current->Keyb(key, true, sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
					break;

				case sf::Event::TextEntered:
					current->TextEntered(static_cast<char>(event.text.unicode));
					break;

				case sf::Event::MouseButtonPressed:
				case sf::Event::MouseButtonReleased:
					current->Mouse(event.mouseButton.button, event.type == sf::Event::MouseButtonPressed, event.mouseButton.x, event.mouseButton.y);
					break;

				case sf::Event::MouseMoved: {
					TVector2i old = cursor_pos;
					cursor_pos.x = event.mouseMove.x;
					cursor_pos.y = event.mouseMove.y;
					current->Motion(event.mouseMove.x - old.x, event.mouseMove.y - old.y);
					break;
				}

#ifdef ANDROID
				case sf::Event::TouchBegan:
				case sf::Event::TouchEnded: {
					TVector2i old = cursor_pos;
					cursor_pos.x = event.touch.x;
					cursor_pos.y = event.touch.y;
					current->Motion(event.touch.x - old.x, event.touch.y - old.y);
					current->Mouse(event.touch.finger, event.type == sf::Event::TouchBegan, event.touch.x, event.touch.y);
					current->Jbutt(3, event.type == sf::Event::TouchBegan);
					break;
				}

				case sf::Event::SensorChanged: {
					if (event.sensor.z > 0)
						current->Jaxis(1, -event.sensor.z + 9.81/2.0);
					current->Jaxis(0, event.sensor.y / (9.81/2.0));
					break;
				}

				case sf::Event::GainedFocus:
					if (!active) {
						active = true;
						Winsys.getWindow().setActive(active);
						Music.Resume();
					}
					if(sf::Sensor::isAvailable(sf::Sensor::Accelerometer)) {
						sf::Sensor::setEnabled(sf::Sensor::Accelerometer, true);
					}
					break;
				case sf::Event::MouseLeft:
					active = false;
					Winsys.getWindow().setActive(active);
					if(sf::Sensor::isAvailable(sf::Sensor::Accelerometer)) {
						sf::Sensor::setEnabled(sf::Sensor::Accelerometer, false);
					}
					Music.Pause();
					current->Keyb(sf::Keyboard::P, false, cursor_pos.x, cursor_pos.y);
					Score.SaveHighScore();
					SaveMessages();
					if (g_game.argument < 1) Players.SavePlayers();
					break;
#endif

				case sf::Event::JoystickMoved: {
					float val = event.joystickMove.position / 100.f;
					current->Jaxis(event.joystickMove.axis == sf::Joystick::X ? 0 : 1, val);
					break;
				}
				case sf::Event::JoystickButtonPressed:
				case sf::Event::JoystickButtonReleased:
					current->Jbutt(event.joystickButton.button, event.type == sf::Event::JoystickButtonPressed);
					break;

				case sf::Event::Resized:
					if (Winsys.resolution.width != event.size.width || Winsys.resolution.height != event.size.height) {
						Winsys.resolution.width = event.size.width;
						Winsys.resolution.height = event.size.height;
						Winsys.SetupVideoMode(event.size.width, event.size.height);
					}
					break;

				case sf::Event::Closed:
					quit = true;
					break;

				default:
					break;
			}
		}
	}
}

void State::Manager::CallLoopFunction() {
	check_gl_error();

	g_game.time_step = std::max(0.0001f, timer.getElapsedTime().asSeconds());
	timer.restart();
#ifdef ANDROID
	if (!active) {
		//sf::sleep(sf::seconds(1));
		return;
	}
#endif
	current->Loop(g_game.time_step);
}
