/* --------------------------------------------------------------------
EXTREME TUXRACER

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

#ifndef AUDIO_H
#define AUDIO_H

#include "bh.h"
#include <vector>
#include <unordered_map>

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------


struct TSound;

class CSound {
private:
	std::vector<TSound*> sounds;
	std::unordered_map<std::string, std::size_t> SoundIndex;
public:
	~CSound();
	bool LoadChunk(const std::string& name, const std::string& filename);
	void LoadSoundList();
	std::size_t GetSoundIdx(const std::string& name) const;

	void SetVolume(std::size_t soundid, int volume);
	void SetVolume(const std::string& name, int volume);

	void Play(std::size_t soundid, bool loop);
	void Play(const std::string& name, bool loop);
	void Play(std::size_t soundid, bool loop, int volume);
	void Play(const std::string& name, bool loop, int volume);

	void Halt(std::size_t soundid);
	void Halt(const std::string& name);
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
	std::vector<sf::Music*> musics;
	std::unordered_map<std::string, std::size_t> MusicIndex;

	struct Situation { sf::Music* situation[SITUATION_COUNT]; };
	std::vector<Situation> themes;
	std::unordered_map<std::string, std::size_t> ThemesIndex;

	sf::Music* curr_music;	// current music piece
	int curr_volume;

	bool Play(sf::Music* music, bool loop, int volume);
public:
	CMusic();
	~CMusic();

	bool LoadPiece(const std::string& name, const std::string& filename);
	void LoadMusicList();
	std::size_t GetMusicIdx(const std::string& name) const;
	std::size_t GetThemeIdx(const std::string& theme) const;

	void SetVolume(int volume);
	bool Play(std::size_t musid, bool loop);
	bool Play(const std::string& name, bool loop);
	bool Play(std::size_t musid, bool loop, int volume);
	bool Play(const std::string& name, bool loop, int volume);
	bool PlayTheme(std::size_t theme, ESituation situation);
	void Halt();
	void FreeMusics();
};

// --------------------------------------------------------------------

extern CMusic Music;
extern CSound Sound;

#endif
