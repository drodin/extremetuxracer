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

#ifndef WINSYS_H
#define WINSYS_H

#include "bh.h"

#define NUM_RESOLUTIONS 10
#define SCREENSHOT_FORMAT ".png"

extern TVector2i cursor_pos;

struct TScreenRes {
	unsigned int width, height;
	constexpr TScreenRes(unsigned int w = 0, unsigned int h = 0) : width(w), height(h) {}
};

class CWinsys {
private:
	unsigned int numJoysticks;

	// sfml window
	bool sfmlRenders;
	sf::RenderWindow window;
	TScreenRes resolutions[NUM_RESOLUTIONS];
	TScreenRes auto_resolution;
	float CalcScreenScale() const;
public:
	TScreenRes resolution;
	float scale;			// scale factor for screen, see 'use_quad_scale'

	CWinsys();

	const TScreenRes& GetResolution(std::size_t idx) const;
	std::string GetResName(std::size_t idx) const;
	void Init();
	void SetupVideoMode(const TScreenRes& res);
	void SetupVideoMode(std::size_t idx);
	void SetupVideoMode(int width, int height);
	void KeyRepeat(bool repeat);
	void PrintJoystickInfo() const;
	void ShowCursor(bool visible) { window.setMouseCursorVisible(visible); }
	void SwapBuffers() { window.display(); }
	void Quit();
	void Terminate();
	void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default) { window.draw(drawable, states); }
	void clear() { window.clear(colBackgr); }
	void beginSFML() { if (!sfmlRenders) window.pushGLStates(); sfmlRenders = true; }
	void endSFML() { if (sfmlRenders) window.popGLStates(); sfmlRenders = false; }
	bool PollEvent(sf::Event& event) { return window.pollEvent(event); }
	const sf::Window& getWindow() const { return window; }
	void TakeScreenshot() const;
};


extern CWinsys Winsys;

#endif
