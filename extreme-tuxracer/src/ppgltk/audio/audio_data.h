/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include "../../etracer.h"

#if defined(HAVE_SDL_MIXER)
   
#include "SDL.h"
#include "SDL_mixer.h"

#else

typedef int Mix_Chunk;
typedef int Mix_Music;

#endif /* defined(HAVE_SDL) && defined(HAVE_SDL_MIXER) */

void init_audio_data();

/* SOUND */
bool load_sound( const char *name, const char *filename ) ;
bool get_sound_data( char *name, Mix_Chunk **data );
void incr_sound_data_ref_ctr( const char *name ) ;
void decr_sound_data_ref_ctr( const char *name ) ;
void delete_unused_audio_data();
bool is_sound_data_dirty();
void mark_sound_data_clean();
void register_sound_data_tcl_callbacks( Tcl_Interp *ip );

/* MUSIC */
bool load_music( const char *name, const char *filename ) ;
bool get_music_data( char *name, Mix_Music **data );
void set_music_playing_status( char *name, bool playing );
bool get_music_playing_status( char *name );
void incr_music_data_ref_ctr( const char *name ) ;
void decr_music_data_ref_ctr( const char *name ) ;
bool is_music_data_dirty();
void mark_music_data_clean();

#endif /* AUDIO_DATA_H */
