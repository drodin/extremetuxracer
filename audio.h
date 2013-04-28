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

// --------------------------------------------------------------------
//				class CAudio
// --------------------------------------------------------------------

class CAudio {
private:
public:
	CAudio ();
	
	void Open ();
	void Close ();
	bool CheckOpen ();
	bool IsOpen;
};

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

typedef struct {
    Mix_Chunk *chunk;  
    int channel;
	double vol_fact;
    int loop_count;
	bool active;
} TSound;

class CSound {
private:
	vector<TSound> sounds;
	string SoundIndex;
public:
	size_t LoadChunk (const std::string& name, const char *filename);
	void LoadSoundList ();
	int  GetSoundIdx (const string& name) const;

	void SetVolume (int soundid, int volume);
	void SetVolume (const string& name, int volume);

	void Play (int soundid, int loop);
	void Play (const string& name, int loop); // -1 infinite, 0 once, 1 twice ...
	void Play (int soundid, int loop, int volume);
	void Play (const string& name, int loop, int volume); 

	void Halt (int soundid);
	void Halt (const string& name);
	void HaltAll ();
	
	void FreeSounds ();
};

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

#define MUS_RACING 0
#define MUS_WONRACE 1
#define MUS_LOSTRACE 2

class CMusic {
private:
	vector<Mix_Music*> musics;
	string MusicIndex;		

	struct Situation {int situation[3];};
	vector<Situation> themes;
	string ThemesIndex;

	int loop_count;			// we need only 1 variable for all pieces
	int curr_musid;			// ID of current music piece
	int curr_volume;
public:
	CMusic ();
	bool is_playing;

	size_t LoadPiece (const char *name, const char *filename);
	void LoadMusicList ();
	int  GetMusicIdx (const string& name) const;
	int  GetThemeIdx (const string& theme) const;

	void SetVolume (int volume);
	void Update ();
	bool Play (int musid, int loop);
	bool Play (const string& name, int loop);
	bool Play (int musid, int loop, int volume);
	bool Play (const string& name, int loop, int volume);
	bool PlayTheme (int theme, int situation);
	void Refresh (const string& name);
	void Halt ();
	void FreeMusics ();
};

// --------------------------------------------------------------------

extern CAudio Audio;
extern CMusic Music;
extern CSound Sound;

#endif
