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

#ifndef AUDIO_H
#define AUDIO_H

#include "bh.h"
#include <vector>
#include <map>

// --------------------------------------------------------------------
//				class CAudio
// --------------------------------------------------------------------

class CAudio {
private:
public:
	CAudio ();

	void Open ();
	void Close ();
	static bool CheckOpen ();
	bool IsOpen;
};

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

struct TSound {
    Mix_Chunk *chunk;
    int channel;
    int loop_count;
	bool active;
};

class CSound {
private:
	vector<TSound> sounds;
	map<string, size_t> SoundIndex;
public:
	size_t LoadChunk (const std::string& name, const char *filename);
	void LoadSoundList ();
	size_t GetSoundIdx (const string& name) const;

	void SetVolume (size_t soundid, int volume);
	void SetVolume (const string& name, int volume);

	void Play (size_t soundid, int loop);
	void Play (const string& name, int loop); // -1 infinite, 0 once, 1 twice ...
	void Play (size_t soundid, int loop, int volume);
	void Play (const string& name, int loop, int volume);

	void Halt (size_t soundid);
	void Halt (const string& name);
	void HaltAll ();

	void FreeSounds ();
};

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

enum ESituation {
	MUS_RACING = 0,
	MUS_WONRACE = 1,
	MUS_LOSTRACE = 2,
	SITUATION_COUNT
};

class CMusic {
private:
	vector<Mix_Music*> musics;
	map<string, size_t> MusicIndex;

	struct Situation {size_t situation[SITUATION_COUNT];};
	vector<Situation> themes;
	map<string, size_t> ThemesIndex;

	int loop_count;			// we need only 1 variable for all pieces
	int curr_musid;			// ID of current music piece
	int curr_volume;
public:
	CMusic ();

	size_t LoadPiece (const string& name, const char *filename);
	void LoadMusicList ();
	size_t GetMusicIdx (const string& name) const;
	size_t GetThemeIdx (const string& theme) const;

	void SetVolume (int volume);
	void Update ();
	bool Play (size_t musid, int loop);
	bool Play (const string& name, int loop);
	bool Play (size_t musid, int loop, int volume);
	bool Play (const string& name, int loop, int volume);
	bool PlayTheme (size_t theme, ESituation situation);
	void Refresh (const string& name);
	void Halt ();
	void FreeMusics ();
};

// --------------------------------------------------------------------

extern CAudio Audio;
extern CMusic Music;
extern CSound Sound;

#endif
