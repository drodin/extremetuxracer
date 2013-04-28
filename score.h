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
#include "course.h"

#define MAX_SCORES 8
// MAX_SCORE_LISTS = MAX_COURSES = 64

typedef struct {
	string player;
	int points;
	int herrings;
	double time;
} TScore;

typedef struct {
	TScore scores[MAX_SCORES];
	int numScores;
} TScoreList;

class CScore {
private:
	TScoreList Scorelist[MAX_COURSES];
	void ResetScorelist (int list_idx);
	TScore TempScore;
public:
	void SetScorelist (int list_idx);
	int AddScore (int list_idx, const TScore& score);
	TScoreList *GetScorelist (int list_idx);
	void PrintScorelist (int list_idx);
	bool SaveHighScore ();
	bool LoadHighScore ();
	int CalcRaceResult ();
};

extern CScore Score;

// --------------------------------------------------------------------

void RegisterScoreFunctions ();

#endif
