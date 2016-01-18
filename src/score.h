/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#ifndef SCORE_H
#define SCORE_H

#include "bh.h"
#include "states.h"
#include "spx.h"
#include <map>

#define MAX_SCORES 8

struct TScore {
	std::string player;
	int points;
	int herrings;
	float time;

	TScore(const std::string& player_ = emptyString, int points_ = 0, int herrings_ = 0, double time_ = 0)
		: player(player_), points(points_), herrings(herrings_), time(time_)
	{}
};

struct TScoreList {
	TScore scores[MAX_SCORES];
	int numScores;
	TScoreList() : numScores(0) {}
};

class CScore : public State {
private:
	std::map<std::string, std::map<std::string, TScoreList>> Scorelist;

	void Enter();
	void Loop(float time_step);
	void Keyb(sf::Keyboard::Key key, bool release, int x, int y);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
	int AddScore(const std::string& group, const std::string& course, TScore&& score);
	const TScoreList* GetScorelist(const std::string& group, const std::string& course) const;
	void PrintScorelist(const std::string& group, const std::string& course) const;
	bool SaveHighScore() const;
	bool LoadHighScore();
	int CalcRaceResult();
};

extern CScore Score;


#endif
