/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include <SFML/Audio.hpp>

#include "audio.h"
#include "spx.h"

// the global instances of the 2 audio classes
CSound Sound;
CMusic Music;
#define MIX_MAX_VOLUME 100

struct TSound {
	sf::SoundBuffer data;
	sf::Sound player;
	explicit TSound(int volume) {
		setVolume(volume);
	}
	void setVolume(int volume) {
		player.setVolume(volume);
	}

	void Play(bool loop) {
		if (player.getStatus() == sf::Sound::Playing) return;
		player.setLoop(loop);
		player.play();
	}
};

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

CSound::~CSound() {
	FreeSounds();
}

bool CSound::LoadChunk(const std::string& name, const std::string& filename) {
	sounds.emplace_back(new TSound(param.sound_volume));
	if (!sounds.back()->data.loadFromFile(filename)) // Try loading sound buffer
		return false;
	sounds.back()->player.setBuffer(sounds.back()->data);
	SoundIndex[name] = sounds.size()-1;
	return true;
}

// Load all soundfiles listed in "/sounds/sounds.lst"
void CSound::LoadSoundList() {
	CSPList list;
	if (list.Load(param.sounds_dir, "sounds.lst")) {
		for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
			std::string name = SPStrN(*line, "name");
			std::string soundfile = SPStrN(*line, "file");
			std::string path = MakePathStr(param.sounds_dir, soundfile);
			LoadChunk(name, path);
		}
	}
}

void CSound::FreeSounds() {
	HaltAll();
	for (std::size_t i = 0; i < sounds.size(); i++)
		delete sounds[i];
	sounds.clear();
	SoundIndex.clear();
}

std::size_t CSound::GetSoundIdx(const std::string& name) const {
	try {
		return SoundIndex.at(name);
	} catch (...) {
		return -1;
	}
}

void CSound::SetVolume(std::size_t soundid, int volume) {
	if (soundid >= sounds.size()) return;

	volume = clamp(0, volume, MIX_MAX_VOLUME);
	sounds[soundid]->setVolume(volume);
}

void CSound::SetVolume(const std::string& name, int volume) {
	SetVolume(GetSoundIdx(name), volume);
}

// ------------------- play -------------------------------------------

void CSound::Play(std::size_t soundid, bool loop) {
	if (soundid >= sounds.size()) return;

	sounds[soundid]->Play(loop);
}

void CSound::Play(const std::string& name, bool loop) {
	Play(GetSoundIdx(name), loop);
}

void CSound::Play(std::size_t soundid, bool loop, int volume) {
	if (soundid >= sounds.size()) return;

	volume = clamp(0, volume, MIX_MAX_VOLUME);
	sounds[soundid]->setVolume(volume);
	sounds[soundid]->Play(loop);
}

void CSound::Play(const std::string& name, bool loop, int volume) {
	Play(GetSoundIdx(name), loop, volume);
}

void CSound::Halt(std::size_t soundid) {
	if (soundid >= sounds.size()) return;

	// loop_count must be -1 (endless loop) for halt
	if (sounds[soundid]->player.getLoop())
		sounds[soundid]->player.stop();
}

void CSound::Halt(const std::string& name) {
	Halt(GetSoundIdx(name));
}

void CSound::HaltAll() {
	for (std::size_t i = 0; i < sounds.size(); i++) {
		sounds[i]->player.stop();
	}
}

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

CMusic::CMusic() {
	curr_music = 0;
	curr_volume = 10;
}
CMusic::~CMusic() {
	FreeMusics();
}

bool CMusic::LoadPiece(const std::string& name, const std::string& filename) {
	sf::Music* m = new sf::Music();
	if (!m->openFromFile(filename)) {
		Message("could not load music", filename);
		return false;
	}
	MusicIndex[name] = musics.size();
	musics.push_back(m);
	return true;
}

void CMusic::LoadMusicList() {
	// --- music ---
	CSPList list;
	if (list.Load(param.music_dir, "music.lst")) {
		for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
			std::string name = SPStrN(*line, "name");
			std::string musicfile = SPStrN(*line, "file");
			std::string path = MakePathStr(param.music_dir, musicfile);
			LoadPiece(name, path);
		}
	} else {
		Message("could not load music.lst");
		return;
	}

	// --- racing themes ---
	list.clear();
	ThemesIndex.clear();
	if (list.Load(param.music_dir, "racing_themes.lst")) {
		themes.resize(list.size());
		std::size_t i = 0;
		for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line, i++) {
			std::string name = SPStrN(*line, "name");
			ThemesIndex[name] = i;
			std::string item = SPStrN(*line, "race", "race_1");
			themes[i].situation[0] = musics[MusicIndex[item]];
			item = SPStrN(*line, "wonrace", "wonrace_1");
			themes[i].situation[1] = musics[MusicIndex[item]];
			item = SPStrN(*line, "lostrace", "lostrace_1");
			themes[i].situation[2] = musics[MusicIndex[item]];
		}
	} else Message("could not load racing_themes.lst");
}

void CMusic::FreeMusics() {
	Halt();
	for (std::size_t i = 0; i < musics.size(); i++)
		delete musics[i];
	musics.clear();
	MusicIndex.clear();

	themes.clear();
	ThemesIndex.clear();

	curr_music = nullptr;
}

std::size_t CMusic::GetMusicIdx(const std::string& name) const {
	try {
		return MusicIndex.at(name);
	} catch (...) {
		return -1;
	}
}

std::size_t CMusic::GetThemeIdx(const std::string& theme) const {
	try {
		return ThemesIndex.at(theme);
	} catch (...) {
		return -1;
	}
}

void CMusic::SetVolume(int volume) {
	int vol = clamp(0, volume, MIX_MAX_VOLUME);
	if (curr_music)
		curr_music->setVolume(volume);
	curr_volume = vol;
}

bool CMusic::Play(sf::Music* music, bool loop, int volume) {
	if (!music)
		return false;

	volume = clamp(0, volume, MIX_MAX_VOLUME);
	if (music != curr_music) {
		music->setVolume(volume);
		music->setLoop(loop);
		if (curr_music)
			curr_music->stop();
		curr_music = music;
		music->play();
	}
	return true;
}

bool CMusic::Play(std::size_t musid, bool loop) {
	if (musid >= musics.size()) return false;
	sf::Music* music = musics[musid];
	return Play(music, loop, curr_volume);
}

bool CMusic::Play(const std::string& name, bool loop) {
	return Play(GetMusicIdx(name), loop);
}

bool CMusic::Play(std::size_t musid, bool loop, int volume) {
	if (musid >= musics.size()) return false;
	sf::Music* music = musics[musid];
	return Play(music, loop, volume);
}

bool CMusic::Play(const std::string& name, bool loop, int volume) {
	return Play(GetMusicIdx(name), loop, volume);
}

bool CMusic::PlayTheme(std::size_t theme, ESituation situation) {
	if (theme >= themes.size()) return false;
	if (situation >= SITUATION_COUNT) return false;
	sf::Music* music = themes[theme].situation[situation];
	return Play(music, -1, curr_volume);
}

void CMusic::Halt() {
	if (curr_music) {
		curr_music->stop();
		curr_music = nullptr;
	}
}
