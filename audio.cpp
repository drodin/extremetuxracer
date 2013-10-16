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

#include "audio.h"
#include "spx.h"

#ifdef USE_AL
ALfloat MIX_MAX_VOLUME = 1.0f;
#endif

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
#ifndef USE_AL
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
#else
	ALmixer_Init(param.audio_freq, ALMIXER_DEFAULT_NUM_CHANNELS, 0);
	IsOpen = CheckOpen ();
#endif
}

void CAudio::Close () {
	if (IsOpen) {
		Music.FreeMusics ();
		Sound.FreeSounds ();
#ifndef USE_AL
		Mix_CloseAudio();
#else
		ALmixer_Quit();
#endif
		IsOpen = false;
	}
}

bool CAudio::CheckOpen() {
#ifndef USE_AL
    int freq;
    Uint16 format;
    int channels;
    int ret = Mix_QuerySpec (&freq, &format, &channels);
	return (ret > 0);
#else
	return ALmixer_IsInitialized();
#endif
}

// --------------------------------------------------------------------
//				class CSound
// --------------------------------------------------------------------

CSound::CSound () {
	for (int i=0; i<MAX_SOUNDS; i++) {
		sounds[i].chunk = NULL;
		active_arr[i] = false;
	}
	SoundIndex = ""; 
	numSounds = 0;
}

int CSound::LoadChunk (const char *name, const char *filename) {
    if (Audio.IsOpen == false) return -1;
	if (numSounds >= MAX_SOUNDS) return -1; 
#ifndef USE_AL
	sounds[numSounds].chunk = Mix_LoadWAV (filename);
#else
	sounds[numSounds].chunk = ALmixer_LoadAll(filename, AL_FALSE);
#endif
	if (sounds[numSounds].chunk == NULL) return -1;
	sounds[numSounds].channel = -1;				// default: no channel
	sounds[numSounds].loop_count = 0;				// default: playing once

#ifndef USE_AL
	Mix_VolumeChunk (sounds[numSounds].chunk, param.sound_volume);
#else
	sounds[numSounds].vol_fact = (float)param.sound_volume/128.0f;
#endif
	SoundIndex = SoundIndex + "[" + name + "]" + Int_StrN (numSounds);
	numSounds++;
	return numSounds-1;
}

// Load all soundfiles listed in "/sounds/sounds.lst"
void CSound::LoadSoundList () {
	if (!Audio.IsOpen) {
		Message ("cannot load music, first open Audio");
		return;
	}
	CSPList list(200);
	string name, soundfile, path, line;
//	int soundid = 0;
	if (list.Load (param.sounds_dir, "sounds.lst")) { 
		for (int i=0; i<list.Count(); i++) {
			line = list.Line(i);
			name = SPStrN (line, "name", "");		
			soundfile = SPStrN (line, "file", "");		
			path = MakePathStr (param.sounds_dir, soundfile);
			LoadChunk (name.c_str(), path.c_str());
		}
	}
}

void CSound::FreeSounds () {
	HaltAll ();
	for (int i=0; i<numSounds; i++)
#ifndef USE_AL
		if (sounds[i].chunk != NULL) Mix_FreeChunk (sounds[i].chunk);
#else
		if (sounds[i].chunk != NULL) ALmixer_FreeData (sounds[i].chunk);
#endif

	for (int i=0; i<MAX_SOUNDS; i++) {
		sounds[i].chunk = NULL;
		active_arr[i] = false;
	}
	SoundIndex = ""; 
	numSounds = 0;
}

int CSound::GetSoundIdx (string name) {
    if (Audio.IsOpen == false) return -1;
	return SPIntN (SoundIndex, name, -1);
}

void CSound::SetVolume (int soundid, int volume) {
    if (Audio.IsOpen == false) return;
	if (soundid < 0 || soundid >= numSounds) return;

#ifndef USE_AL
	volume = MIN (MIX_MAX_VOLUME, MAX (0, volume));
	if (sounds[soundid].chunk == NULL) return;
	Mix_VolumeChunk (sounds[soundid].chunk, volume);
#else
	ALfloat vol = MIN (MIX_MAX_VOLUME, MAX (0.0f, (float)volume/128.0f));
	if (sounds[soundid].chunk == NULL) return;
	sounds[soundid].vol_fact = vol;
#endif
}

void CSound::SetVolume (string name, int volume) {
	SetVolume (GetSoundIdx (name), volume);
}

// ------------------- play -------------------------------------------

void CSound::Play (int soundid, int loop) {
	if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	if (active_arr[soundid] == true) return;
	if (sounds[soundid].chunk == NULL) return;

#ifndef USE_AL
    sounds[soundid].channel = Mix_PlayChannel (-1, sounds[soundid].chunk, loop);
#else
    sounds[soundid].channel = ALmixer_PlayChannel (-1, sounds[soundid].chunk, loop);
    ALmixer_SetVolumeChannel(sounds[soundid].channel, sounds[soundid].vol_fact);
#endif
    sounds[soundid].loop_count = loop;
	if (loop < 0) active_arr[soundid] = true;
}

void CSound::Play (string name, int loop) {
	Play (GetSoundIdx (name), loop);
}

void CSound::Play (int soundid, int loop, int volume) {
    if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	if (active_arr[soundid] == true) return;
	if (sounds[soundid].chunk == NULL) return;

#ifndef USE_AL
	volume = MIN (MIX_MAX_VOLUME, MAX (0, volume));
    Mix_VolumeChunk (sounds[soundid].chunk, volume);  
    sounds[soundid].channel = Mix_PlayChannel (-1, sounds[soundid].chunk, loop);
#else
	ALfloat vol = MIN (MIX_MAX_VOLUME, MAX (0.0f, (float)volume/128.0f));
	sounds[soundid].channel = ALmixer_PlayChannel (-1, sounds[soundid].chunk, loop);
    ALmixer_SetVolumeChannel (sounds[soundid].channel, vol);
#endif
    sounds[soundid].loop_count = loop;
	if (loop < 0) active_arr[soundid] = true;
}

void CSound::Play (string name, int loop, int volume) {
	Play (GetSoundIdx (name), loop, volume);
}

void CSound::Halt (int soundid) {
    if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	if (sounds[soundid].chunk == NULL) return;

	// loop_count must be -1 (endless loop) for halt 
	if (sounds[soundid].loop_count < 0) {
#ifndef USE_AL
		Mix_HaltChannel (sounds[soundid].channel);
#else
		ALmixer_HaltChannel (sounds[soundid].channel);
#endif
	    sounds[soundid].loop_count = 0;
   		sounds[soundid].channel = -1;
		active_arr[soundid] = false;
	}
}

void CSound::Halt (string name) {
	Halt (GetSoundIdx (name));
}

void CSound::HaltAll () {
    if (!Audio.IsOpen) return;
#ifndef USE_AL
	Mix_HaltChannel (-1);
#endif
	for (int i=0; i<numSounds; i++) {
#ifdef USE_AL
		if (sounds[i].channel >= 0)
			ALmixer_HaltChannel (sounds[i].channel);
#endif
		sounds[i].loop_count = 0;
		sounds[i].channel = -1;
		active_arr[i] = false;
	}
}

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

void Hook () {
#ifndef USE_AL
	Mix_HaltMusic();
#endif
	PrintString ("halted");
}

CMusic::CMusic () {
	for (int i=0; i<MAX_MUSICS; i++) musics[i] = NULL;
	MusicIndex = ""; 
	numMusics = 0;

	for (int i=0; i<MAX_THEMES; i++) {
		for (int j=0; j<3; j++) themes[i][j] = -1;
	}
	ThemesIndex = "";
	numThemes = 0;

	curr_musid = -1;
	curr_volume = 10;
	loop_count = 0;
	is_playing = false;
//	Mix_HookMusicFinished (Hook);	
}

int CMusic::LoadPiece (const char *name, const char *filename) {
    if (!Audio.IsOpen) return -1;
	if (numMusics >= MAX_MUSICS) return -1; 
#ifndef USE_AL
	musics[numMusics] = Mix_LoadMUS (filename);
#else
	musics[numMusics] = ALmixer_LoadStream (filename, ALMIXER_DEFAULT_BUFFERSIZE * 4, 16, 8, 4, AL_FALSE);
#endif
	if (musics[numMusics] == NULL) {
		Message ("could not load music", filename);
		return -1;
	}
	MusicIndex = MusicIndex + "[" + name + "]" + Int_StrN (numMusics);
	numMusics++;
	return numMusics-1;
}

void CMusic::LoadMusicList () {
	if (!Audio.IsOpen) {
		Message ("cannot load music, first open audio");
		return;
	}
	// --- music ---
	CSPList list(200);
	string name, musicfile, path, line, item;
//	int musid = 0;
	if (list.Load (param.music_dir, "music.lst")) { 
		for (int i=0; i<list.Count(); i++) {
			line = list.Line(i);
			name = SPStrN (line, "name", "");		
			musicfile = SPStrN (line, "file", "");		
			path = MakePathStr (param.music_dir, musicfile);
			LoadPiece (name.c_str(), path.c_str());
		}
	} else {
		Message ("could not load music.lst");
		return;
	}

	// --- racing themes ---
	list.Clear();
	numThemes = 0;
	ThemesIndex = "";
	if (list.Load (param.music_dir, "racing_themes.lst")) { 
		for (int i=0; i<list.Count(); i++) {
			line = list.Line(i);
			name = SPStrN (line, "name", "");
			ThemesIndex = ThemesIndex + "[" + name + "]" + Int_StrN (numThemes);
			item  = SPStrN (line, "race", "race_1");
			themes [numThemes][0] = GetMusicIdx (item);
			item  = SPStrN (line, "wonrace", "wonrace_1");
			themes [numThemes][1] = GetMusicIdx (item);
			item  = SPStrN (line, "lostrace", "lostrace_1");
			themes [numThemes][2] = GetMusicIdx (item);
 			numThemes++;
		}
	} else Message ("could not load racing_themes.lst");
}

void CMusic::FreeMusics () {
	Halt ();
#ifndef USE_AL
	for (int i=0; i<numMusics; i++) if (musics[i] != NULL) Mix_FreeMusic (musics[i]);
#else
	for (int i=0; i<numMusics; i++) if (musics[i] != NULL) ALmixer_FreeData (musics[i]);
#endif
	for (int i=0; i<MAX_MUSICS; i++) musics[i] = NULL;
	MusicIndex = ""; 
	numMusics = 0;

	for (int i=0; i<MAX_THEMES; i++) {
		for (int j=0; j<3; j++) themes[i][j] = -1;
	}
	ThemesIndex = "";
	numThemes = 0;

	curr_musid = -1;
	curr_volume = 10;
	is_playing = false;
}

int CMusic::GetMusicIdx (string name) {
    if (Audio.IsOpen == false) return -1;
	return SPIntN (MusicIndex, name, -1);
}

int CMusic::GetThemeIdx (string theme) {
    if (Audio.IsOpen == false) return -1;
	return SPIntN (ThemesIndex, theme, -1);
}

void CMusic::SetVolume (int volume) {
#ifndef USE_AL
	int vol = MIN (MIX_MAX_VOLUME, MAX (0, volume));
	Mix_VolumeMusic (vol);
#else
	ALfloat vol = MIN (MIX_MAX_VOLUME, MAX (0.0f, (float)volume/128.0f));
#endif
	curr_volume = vol;
}

// If the piece is played in a loop, the volume adjustment gets lost.
// probably a bug in SDL_mixer. Help: we have to refresh the volume 
// in each (!) frame.

void CMusic::Update () {
#ifndef USE_AL
	Mix_VolumeMusic (curr_volume);
#else
	ALmixer_Update();
#endif
}

bool CMusic::Play (int musid, int loop) {
    if (!Audio.IsOpen) return false;
	if (musid < 0 || musid >= numMusics) return false;
#ifndef USE_AL
	Mix_Music *music = musics[musid];
#else
	ALmixer_Data *music = musics[musid];
#endif
	if (music == NULL) return false;
	if (musid != curr_musid) {
		Halt ();
#ifndef USE_AL
		Mix_PlayMusic (music, loop);
#else
		curr_channel = ALmixer_PlayChannel(-1, music, loop);
#endif
		curr_musid = musid;
		loop_count = loop;
	}
#ifndef USE_AL
	Mix_VolumeMusic (curr_volume);
#else
	ALmixer_SetVolumeChannel(curr_channel, curr_volume);
#endif
	return true;
}

bool CMusic::Play (string name, int loop) {
	return Play (GetMusicIdx (name), loop);
}

bool CMusic::Play (int musid, int loop, int volume) {
    if (!Audio.IsOpen) return false;
	if (musid < 0 || musid >= numMusics) return false;
#ifndef USE_AL
	Mix_Music *music = musics[musid];

	int vol = MIN (MIX_MAX_VOLUME, MAX (0, volume));
#else
	ALmixer_Data *music = musics[musid];

	float vol = MIN (MIX_MAX_VOLUME, MAX (0.0f, (float)volume/128.0f));
#endif
	if (musid != curr_musid) {
		Halt ();
#ifndef USE_AL
		Mix_PlayMusic (music, loop);
		Mix_VolumeMusic (vol);
#else
		ALmixer_PlayChannel(curr_channel, music, loop);
		ALmixer_SetVolumeChannel (curr_channel, vol);
#endif
		curr_musid = musid;
		loop_count = loop;
	}
	return true;
}

bool CMusic::Play (string name, int loop, int volume) {
	return Play (GetMusicIdx (name), loop, volume);
}

bool CMusic::PlayTheme (int theme, int situation)  {
	if (theme < 0 || theme >= numThemes) return false; 
	if (situation < 0 || situation >= 3) return false; 
	int musid = themes [theme][situation];
	return Play (musid, -1);
}

void CMusic::Refresh (string name) {
	Play (name, -1);	
}

void CMusic::Halt () {
#ifndef USE_AL
	if (Mix_PlayingMusic ()) Mix_HaltMusic();
#else
	ALmixer_HaltChannel(curr_channel);
	curr_channel = -1;
#endif
	loop_count = -1;
	curr_musid = -1;
}





