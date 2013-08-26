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
#include <vector>

#define MAX_SCORES 8

struct TScore {
	string player;
	int points;
	int herrings;
	double time;
};

struct TScoreList {
	TScore scores[MAX_SCORES];
	int numScores;
	TScoreList() : numScores(0) {}
};

class CScore : public State {
private:
	vector<TScoreList> Scorelist;
	TScore TempScore;

	void Enter();
	void Loop(double time_step);
	void Keyb(unsigned int key, bool special, bool release, int x, int y);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
	int AddScore (size_t list_idx, const TScore& score);
	const TScoreList *GetScorelist (size_t list_idx) const;
	void PrintScorelist (size_t list_idx) const;
	bool SaveHighScore () const;
	bool LoadHighScore ();
	int CalcRaceResult ();
};

extern CScore Score;

// --------------------------------------------------------------------


#endif
