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
//				class CSound
// --------------------------------------------------------------------


struct TSound;

class CSound {
private:
	vector<TSound*> sounds;
	map<string, size_t> SoundIndex;
public:
	~CSound();
	bool LoadChunk(const std::string& name, const std::string& filename);
	void LoadSoundList();
	size_t GetSoundIdx(const string& name) const;

	void SetVolume(size_t soundid, int volume);
	void SetVolume(const string& name, int volume);

	void Play(size_t soundid, bool loop);
	void Play(const string& name, bool loop);
	void Play(size_t soundid, bool loop, int volume);
	void Play(const string& name, bool loop, int volume);

	void Halt(size_t soundid);
	void Halt(const string& name);
	void HaltAll();

	void FreeSounds();
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

namespace sf {
class Music;
};

class CMusic {
private:
	vector<sf::Music*> musics;
	map<string, size_t> MusicIndex;

	struct Situation { sf::Music* situation[SITUATION_COUNT]; };
	vector<Situation> themes;
	map<string, size_t> ThemesIndex;

	sf::Music* curr_music;	// current music piece
	int curr_volume;

	bool Play(sf::Music* music, bool loop, int volume);
public:
	CMusic();
	~CMusic();

	bool LoadPiece(const string& name, const string& filename);
	void LoadMusicList();
	size_t GetMusicIdx(const string& name) const;
	size_t GetThemeIdx(const string& theme) const;

	void SetVolume(int volume);
	bool Play(size_t musid, bool loop);
	bool Play(const string& name, bool loop);
	bool Play(size_t musid, bool loop, int volume);
	bool Play(const string& name, bool loop, int volume);
	bool PlayTheme(size_t theme, ESituation situation);
	void Halt();
	void FreeMusics();
};

// --------------------------------------------------------------------

extern CMusic Music;
extern CSound Sound;

#endif
