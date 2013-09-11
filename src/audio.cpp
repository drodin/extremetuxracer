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

#include "audio.h"
#include "spx.h"
#include <SDL/SDL.h>

// the global instances of the 3 audio classes
CAudio Audio;
CMusic Music;
CSound Sound;

// --------------------------------------------------------------------
//				class CAudio
// --------------------------------------------------------------------

CAudio::CAudio () {
	IsOpen = false;
}

void CAudio::Open () {
	// first initialize audio (SDL, not SDL_Mixer).
	if (SDL_Init (SDL_INIT_AUDIO) < 0) {
		Message ("Couldn't initialize SDL Audio", SDL_GetError());
		return;
	}
	Uint16 format = AUDIO_S16SYS;
	int channels = 2;
	if (Mix_OpenAudio (param.audio_freq, format, channels, param.audio_buffer_size) < 0)
		Message ("Couldn't open SDL_mixer", Mix_GetError());
	IsOpen = CheckOpen ();
	Mix_AllocateChannels (8);
}

void CAudio::Close () {
	if (IsOpen) {
		Music.FreeMusics ();
		Sound.FreeSounds ();
		Mix_CloseAudio();
		IsOpen = false;
	}
}

bool CAudio::CheckOpen() {
	int freq;
	Uint16 format;
	int channels;
	int ret = Mix_QuerySpec (&freq, &format, &channels);
	return (ret > 0);
}

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

bool CSound::LoadChunk (const std::string& name, const char *filename) {
	if (Audio.IsOpen == false) return false;
	sounds.push_back(TSound());
	sounds.back().chunk = Mix_LoadWAV (filename);
	if (sounds.back().chunk == NULL) return false;
	sounds.back().channel = -1;					// default: no channel
	sounds.back().loop_count = 0;				// default: playing once

	Mix_VolumeChunk (sounds.back().chunk, param.sound_volume);
	SoundIndex[name] = sounds.size()-1;
	return true;
}

// Load all soundfiles listed in "/sounds/sounds.lst"
void CSound::LoadSoundList () {
	if (!Audio.IsOpen) {
		Message ("cannot load music, first open Audio");
		return;
	}
	CSPList list(200);
	if (list.Load (param.sounds_dir, "sounds.lst")) {
		for (size_t i=0; i<list.Count(); i++) {
			const string& line = list.Line(i);
			string name = SPStrN (line, "name", "");
			string soundfile = SPStrN (line, "file", "");
			string path = MakePathStr (param.sounds_dir, soundfile);
			LoadChunk (name, path.c_str());
		}
	}
}

void CSound::FreeSounds () {
	HaltAll ();
	for (size_t i=0; i<sounds.size(); i++)
		if (sounds[i].chunk != NULL)
			Mix_FreeChunk (sounds[i].chunk);
	sounds.clear();
	SoundIndex.clear();
}

size_t CSound::GetSoundIdx (const string& name) const {
	if (Audio.IsOpen == false) return -1;
	try {
		return SoundIndex.at(name);
	} catch (...) {
		return -1;
	}
}

void CSound::SetVolume (size_t soundid, int volume) {
	if (Audio.IsOpen == false) return;
	if (soundid >= sounds.size()) return;

	volume = clamp(0, volume, MIX_MAX_VOLUME);
	if (sounds[soundid].chunk == NULL) return;
	Mix_VolumeChunk (sounds[soundid].chunk, volume);
}

void CSound::SetVolume (const string& name, int volume) {
	SetVolume (GetSoundIdx (name), volume);
}

// ------------------- play -------------------------------------------

void TSound::Play(int loop) {
	if (active == true) return;
	if (chunk == NULL) return;

	channel = Mix_PlayChannel (-1, chunk, loop);
	loop_count = loop;
	if (loop < 0) active = true;
}

void CSound::Play (size_t soundid, int loop) {
	if (!Audio.IsOpen) return;
	if (soundid >= sounds.size()) return;

	sounds[soundid].Play(loop);
}

void CSound::Play (const string& name, int loop) {
	Play (GetSoundIdx (name), loop);
}

void CSound::Play (size_t soundid, int loop, int volume) {
	if (!Audio.IsOpen) return;
	if (soundid >= sounds.size()) return;

	volume = clamp(0, volume, MIX_MAX_VOLUME);
	Mix_VolumeChunk (sounds[soundid].chunk, volume);
	sounds[soundid].Play(loop);
}

void CSound::Play (const string& name, int loop, int volume) {
	Play (GetSoundIdx (name), loop, volume);
}

void CSound::Halt (size_t soundid) {
	if (!Audio.IsOpen) return;
	if (soundid >= sounds.size()) return;
	if (sounds[soundid].chunk == NULL) return;

	// loop_count must be -1 (endless loop) for halt
	if (sounds[soundid].loop_count < 0) {
		Mix_HaltChannel (sounds[soundid].channel);
		sounds[soundid].loop_count = 0;
		sounds[soundid].channel = -1;
		sounds[soundid].active = false;
	}
}

void CSound::Halt (const string& name) {
	Halt (GetSoundIdx (name));
}

void CSound::HaltAll () {
	if (!Audio.IsOpen) return;
	Mix_HaltChannel (-1);
	for (size_t i=0; i<sounds.size(); i++) {
		sounds[i].loop_count = 0;
		sounds[i].channel = -1;
		sounds[i].active = false;
	}
}

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

void Hook () {
	Mix_HaltMusic();
	PrintStr ("halted");
}

CMusic::CMusic () {
	curr_music = 0;
	curr_volume = 10;
	loop_count = 0;
//	Mix_HookMusicFinished (Hook);
}

bool CMusic::LoadPiece (const string& name, const char *filename) {
	if (!Audio.IsOpen) return -1;
	Mix_Music* m = Mix_LoadMUS (filename);
	if (m == NULL) {
		Message ("could not load music", filename);
		return false;
	}
	MusicIndex[name] = musics.size();
	musics.push_back(m);
	return true;
}

void CMusic::LoadMusicList () {
	if (!Audio.IsOpen) {
		Message ("cannot load music, first open audio");
		return;
	}
	// --- music ---
	CSPList list(200);
	if (list.Load (param.music_dir, "music.lst")) {
		for (size_t i=0; i<list.Count(); i++) {
			const string& line = list.Line(i);
			string name = SPStrN (line, "name", "");
			string musicfile = SPStrN (line, "file", "");
			string path = MakePathStr (param.music_dir, musicfile);
			LoadPiece (name, path.c_str());
		}
	} else {
		Message ("could not load music.lst");
		return;
	}

	// --- racing themes ---
	list.Clear();
	ThemesIndex.clear();
	if (list.Load (param.music_dir, "racing_themes.lst")) {
		themes.resize(list.Count());
		for (size_t i=0; i<list.Count(); i++) {
			const string& line = list.Line(i);
			string name = SPStrN (line, "name", "");
			ThemesIndex[name] = i;
			string item = SPStrN (line, "race", "race_1");
			themes[i].situation[0] = musics[MusicIndex[item]];
			item = SPStrN (line, "wonrace", "wonrace_1");
			themes[i].situation[1] = musics[MusicIndex[item]];
			item = SPStrN (line, "lostrace", "lostrace_1");
			themes[i].situation[2] = musics[MusicIndex[item]];
		}
	} else Message ("could not load racing_themes.lst");
}

void CMusic::FreeMusics () {
	Halt ();
	for (size_t i=0; i<musics.size(); i++)
		if (musics[i] != NULL)
			Mix_FreeMusic (musics[i]);
	musics.clear();
	MusicIndex.clear();

	themes.clear();
	ThemesIndex.clear();

	curr_music = 0;
	curr_volume = 10;
}

size_t CMusic::GetMusicIdx (const string& name) const {
	if (Audio.IsOpen == false) return -1;
	try {
		return MusicIndex.at(name);
	} catch (...) {
		return -1;
	}
}

size_t CMusic::GetThemeIdx (const string& theme) const {
	if (Audio.IsOpen == false) return -1;
	try {
		return ThemesIndex.at(theme);
	} catch (...) {
		return -1;
	}
}

void CMusic::SetVolume (int volume) {
	int vol = clamp(0, volume, MIX_MAX_VOLUME);
	Mix_VolumeMusic (vol);
	curr_volume = vol;
}

// If the piece is played in a loop, the volume adjustment gets lost.
// probably a bug in SDL_mixer. Help: we have to refresh the volume
// in each (!) frame.

void CMusic::Update () {
	Mix_VolumeMusic (curr_volume);
}

bool CMusic::Play (Mix_Music* music, int loop, int volume) {
	if(!music)
		return false;

	int vol = clamp(0, volume, MIX_MAX_VOLUME);
	if (music != curr_music) {
		Halt ();
		Mix_PlayMusic (music, loop);
		curr_music = music;
		loop_count = loop;
	}
	Mix_VolumeMusic (vol);
	return true;
}

bool CMusic::Play (size_t musid, int loop) {
	if (!Audio.IsOpen) return false;
	if (musid >= musics.size()) return false;
	Mix_Music *music = musics[musid];
	return Play(music, loop, curr_volume);
}

bool CMusic::Play (const string& name, int loop) {
	return Play (GetMusicIdx(name), loop);
}

bool CMusic::Play (size_t musid, int loop, int volume) {
	if (!Audio.IsOpen) return false;
	if (musid >= musics.size()) return false;
	Mix_Music *music = musics[musid];
	return Play(music, loop, volume);
}

bool CMusic::Play (const string& name, int loop, int volume) {
	return Play (GetMusicIdx (name), loop, volume);
}

bool CMusic::PlayTheme (size_t theme, ESituation situation) {
	if (theme >= themes.size()) return false;
	if (situation >= SITUATION_COUNT) return false;
	Mix_Music *music = themes [theme].situation[situation];
	return Play (music, -1, curr_volume);
}

void CMusic::Halt () {
	if (Mix_PlayingMusic ()) Mix_HaltMusic();
	loop_count = -1;
	curr_music = 0;
}
