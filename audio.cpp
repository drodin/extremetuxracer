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

CSound::CSound () {
	for (int i=0; i<MAX_SOUNDS; i++) {
		sounds[i] = 0;
		active_arr[i] = false;
	}
	SoundIndex = ""; 
	numSounds = 0;
}

int CSound::LoadChunk (const char *name, const char *filename) {
    if (Audio.IsOpen == false) return -1;
	if (numSounds >= MAX_SOUNDS) return -1; 
	Mix_Chunk *load = Mix_LoadWAV (filename);
	if (load == NULL) return -1;

	sounds[numSounds] = new TSound;
	sounds[numSounds]->chunk = load;
	sounds[numSounds]->channel = -1;				// default: no channel
	sounds[numSounds]->loop_count = 0;				// default: playing once

	Mix_VolumeChunk (sounds[numSounds]->chunk, param.sound_volume);
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
	int soundid;
	if (list.Load (param.sounds_dir, "sounds.lst")) { 
		for (int i=0; i<list.Count(); i++) {
			line = list.Line(i);
			name = SPStrN (line, "name", "");		
			soundfile = SPStrN (line, "file", "");		
			path = MakePathStr (param.sounds_dir, soundfile);
			soundid = LoadChunk (name.c_str(), path.c_str());
		}
	}
}

int CSound::GetSoundIdx (string name) {
    if (Audio.IsOpen == false) return -1;
	return SPIntN (SoundIndex, name, -1);
}

void CSound::SetVolume (int soundid, int volume) {
    if (Audio.IsOpen == false) return;
	if (soundid < 0 || soundid >= numSounds) return;

	volume = MIN (MIX_MAX_VOLUME, MAX (0, volume));
	TSound *sound = sounds[soundid];
	if (sound->chunk == NULL) return;
	Mix_VolumeChunk (sound->chunk, volume);
}

void CSound::SetVolume (string name, int volume) {
	SetVolume (GetSoundIdx (name), volume);
}

// ------------------- play -------------------------------------------

void CSound::Play (int soundid, int loop) {
    if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	if (active_arr[soundid] == true) return;
	TSound *sound = sounds[soundid];
	if (sound->chunk == NULL) return;

    sound->channel = Mix_PlayChannel (-1, sound->chunk, loop);
    sound->loop_count = loop;
	if (loop < 0) active_arr[soundid] = true;
}

void CSound::Play (string name, int loop) {
	Play (GetSoundIdx (name), loop);
}

void CSound::Play (int soundid, int loop, int volume) {
    if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	if (active_arr[soundid] == true) return;
	TSound *sound = sounds[soundid];
	if (sound->chunk == NULL) return;

	volume = MIN (MIX_MAX_VOLUME, MAX (0, volume));
    Mix_VolumeChunk (sound->chunk, volume);  
    sound->channel = Mix_PlayChannel (-1, sound->chunk, loop);
    sound->loop_count = loop;
	if (loop < 0) active_arr[soundid] = true;
}

void CSound::Play (string name, int loop, int volume) {
	Play (GetSoundIdx (name), loop, volume);
}

void CSound::Halt (int soundid) {
    if (!Audio.IsOpen) return;
	if (soundid < 0 || soundid >= numSounds) return;
	TSound *sound = sounds[soundid];
	if (sound->chunk == NULL) return;

	// loop_count must be -1 (endless loop) for halt 
	if (sound->loop_count < 0) {
		Mix_HaltChannel (sound->channel);
	    sound->loop_count = 0;
   		sound->channel = -1;
		active_arr[soundid] = false;
	}
}

void CSound::Halt (string name) {
	Halt (GetSoundIdx (name));
}

void CSound::HaltAll () {
    if (!Audio.IsOpen) return;
	Mix_HaltChannel (-1);
	for (int i=0; i<numSounds; i++) {
		sounds[i]->loop_count = 0;
		sounds[i]->channel = -1;
		active_arr[i] = false;
	}
}

// --------------------------------------------------------------------
//				class CMusic
// --------------------------------------------------------------------

void Hook () {
	Mix_HaltMusic();
	PrintString ("halted");
}

CMusic::CMusic () {
	for (int i=0; i<MAX_MUSICS; i++) musics[i] = 0;
	MusicIndex = ""; 
	numMusics = 0;
	curr_musid = -1;
	curr_volume = 10;
	is_playing = false;
//	Mix_HookMusicFinished (Hook);	
}

int CMusic::LoadPiece (const char *name, const char *filename) {
    if (!Audio.IsOpen) return -1;
	if (numMusics >= MAX_MUSICS) return -1; 
	Mix_Music *load = Mix_LoadMUS (filename);
	if (load == 0) return -1;

	musics[numMusics] = new TMusic;
	musics[numMusics]->piece = load;

	MusicIndex = MusicIndex + "[" + name + "]" + Int_StrN (numMusics);
	numMusics++;
	return numMusics-1;
}

void CMusic::LoadMusicList () {
	if (!Audio.IsOpen) {
		Message ("cannot load music, first open Audio");
		return;
	}
	CSPList list(200);
	string name, musicfile, path, line;
	int musid;
	if (list.Load (param.music_dir, "music.lst")) { 
		for (int i=0; i<list.Count(); i++) {
			line = list.Line(i);
			name = SPStrN (line, "name", "");		
			musicfile = SPStrN (line, "file", "");		
			path = MakePathStr (param.music_dir, musicfile);
			musid = LoadPiece (name.c_str(), path.c_str());
		}
	}
}

int CMusic::GetMusicIdx (string name) {
    if (Audio.IsOpen == false) return -1;
	return SPIntN (MusicIndex, name, -1);
}

void CMusic::SetVolume (int volume) {
	int vol = MIN (MIX_MAX_VOLUME, MAX (0, volume));
	Mix_VolumeMusic (vol);
	curr_volume = vol;
}

// If the piece is played in a loop, the volume adjustment gets lost.
// probably a bug in SDL_mixer. Help: we have to refresh the volume 
// in each (!) frame.

void CMusic::Update () {
	Mix_VolumeMusic (curr_volume);
}

bool CMusic::Play (int musid, int loop) {
    if (!Audio.IsOpen) return false;
	if (musid < 0 || musid >= numMusics) return false;
	TMusic *music = musics[musid];
	if (music->piece == NULL) return false;
	if (musid != curr_musid) {
		Halt ();
		Mix_PlayMusic (music->piece, loop);
		curr_musid = musid;
		loop_count = loop;
	}
	Mix_VolumeMusic (curr_volume);
	return true;
}

bool CMusic::Play (string name, int loop) {
	return Play (GetMusicIdx (name), loop);
}

bool CMusic::Play (int musid, int loop, int volume) {
    if (!Audio.IsOpen) return false;
	if (musid < 0 || musid >= numMusics) return false;
	TMusic *music = musics[musid];
	if (music->piece == NULL) return false;

	int vol = MIN (MIX_MAX_VOLUME, MAX (0, volume));
	if (musid != curr_musid) {
		Halt ();
		Mix_PlayMusic (music->piece, loop);
		Mix_VolumeMusic (vol);
		curr_musid = musid;
		loop_count = loop;
	}
	return true;
}

bool CMusic::Play (string name, int loop, int volume) {
	return Play (GetMusicIdx (name), loop, volume);
}

void CMusic::Refresh (string name) {
	Play (name, -1);	
}

void CMusic::Halt () {
	if (Mix_PlayingMusic ()) Mix_HaltMusic();
	loop_count = -1;
	curr_musid = -1;
}





