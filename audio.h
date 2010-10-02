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

// --------------------------------------------------------------------
//				class CAudio
// --------------------------------------------------------------------

class CAudio {
private:
public:
	CAudio ();
	~CAudio () {}
	
	void Open ();
	void Close ();
	bool CheckOpen ();
	bool IsOpen;
};

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

#define MAX_SOUNDS 64

typedef struct {
    Mix_Chunk *chunk;  
    int channel;
	double vol_fact;
    int loop_count;
} TSound;

class CSound {
private:
	TSound *sounds[MAX_SOUNDS];
	int numSounds;
	string SoundIndex;
	bool active_arr[MAX_SOUNDS];
public:
	CSound (); 
	~CSound () {}

	int  LoadChunk (const char *name, const char *filename);
	void LoadSoundList ();
	int  GetSoundIdx (string name);

	void SetVolume (int soundid, int volume);
	void SetVolume (string name, int volume);

	void Play (int soundid, int loop);
	void Play (string name, int loop); // -1 infinite, 0 once, 1 twice ...
	void Play (int soundid, int loop, int volume);
	void Play (string name, int loop, int volume); 

	void Halt (int soundid);
	void Halt (string name);
	void HaltAll ();
};

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

#define MAX_MUSICS 32

typedef struct {
    Mix_Music *piece;  
	// perhaps we need more params ...
} TMusic;

class CMusic {
private:
	TMusic *musics[MAX_MUSICS];
	int numMusics;
	string MusicIndex;		
	int loop_count;			// we need only 1 variable for all pieces
	int curr_musid;			// ID of current music piece
	int curr_volume;
public:
	CMusic (); 
	~CMusic () {}

	int  LoadPiece (const char *name, const char *filename); 
	void LoadMusicList ();
	int  GetMusicIdx (string name);

	void SetVolume (int volume);
	void Update ();
	void Play (int musid, int loop);
	void Play (string name, int loop);
	void Play (int musid, int loop, int volume);
	void Play (string name, int loop, int volume);
	void Halt ();
};

// --------------------------------------------------------------------

extern CAudio Audio;
extern CMusic Music;
extern CSound Sound;

#endif
