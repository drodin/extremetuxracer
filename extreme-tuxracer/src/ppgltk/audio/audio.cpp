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

#include "game_config.h"
#include "string_util.h"

#include "ppgltk/ui_mgr.h"


#if defined(HAVE_SDL_MIXER)

#include "SDL.h"
#include "SDL_mixer.h"
#include "audio.h"
#include "audio_data.h"

#include <string>
#include <map>


typedef struct {
    int num_sounds;
    char **sound_names; /* array of sound names */
    Mix_Chunk **chunks; /* array of cached Mix_Chunk* */
    int loop_count;
    int channel;
    int volume;
} sound_context_data_t;

typedef struct {
    char *music_name;
    Mix_Music *music;
    int loop;
} music_context_data_t;

//static hash_table_t sound_contexts_;
static std::map<std::string,sound_context_data_t> soundTable;

//static hash_table_t music_contexts_;
static std::map<std::string,music_context_data_t> musicTable;


static bool initialized_ = false;

/* Data related to currently-playing music */
static Mix_Music *current_music_data_ = NULL;
static char *current_music_name_ = NULL;

/*! 
  Initializes the audio module.
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-13
*/
void init_audio()
{
    int hz, channels, buffer;
    Uint16 format;

    check_assertion( !initialized_,
		     "init_audio called twice" );

    //sound_contexts_ = create_hash_table();
    //music_contexts_ = create_hash_table();
    initialized_ = true;

    /*
     * Init SDL Audio 
     */    
    if ( getparam_no_audio() == false ) {

	if ( SDL_Init( SDL_INIT_AUDIO ) < 0 ) {
	    handle_error( 1, "Couldn't initialize SDL: %s", SDL_GetError() );
	}

	/* Open the audio device */
	switch (getparam_audio_freq_mode()) {
	case 0:
	    hz = 11025;
	    break;
	case 1:
	    hz = 22050;
	    break;
	case 2:
	    hz = 44100;
	    break;
	default:
	    hz = 22050;
	    setparam_audio_freq_mode(1);
	}

	switch ( getparam_audio_format_mode() ) {
	case 0:
	    format = AUDIO_U8;
	    break;
	case 1:
	    format = AUDIO_S16SYS;
	    break;
	default:
	    format = AUDIO_S16SYS;
	    setparam_audio_format_mode( 1 );
	}

	if ( getparam_audio_stereo() ) {
	    channels = 2;
	} else {
	    channels = 1;
	}

	buffer = getparam_audio_buffer_size();

	if ( Mix_OpenAudio(hz, format, channels, buffer) < 0 ) {
	    print_warning( 1,
			   "Warning: Couldn't set %d Hz %d-bit audio\n"
			   "  Reason: %s\n", 
			   hz,  
			   getparam_audio_format_mode() == 0 ? 8 : 16,
			   SDL_GetError());
	} else {
	    print_debug( DEBUG_SOUND,
			 "Opened audio device at %d Hz %d-bit audio",
			 hz, 
			 getparam_audio_format_mode() == 0 ? 8 : 16 );
	}
    }
}


/*! 
  Returns the status of the audio device
  \return  true if audio has been opened, false otherwise
  \author  jfpatry
  \date    Created:  2000-09-02
  \date    Modified: 2000-09-02
*/
bool is_audio_open()
{
    int tmp_freq;
    Uint16 tmp_format;
    int tmp_channels;

    return Mix_QuerySpec( &tmp_freq, &tmp_format, &tmp_channels );
}

/*! 
  Associate the sounds in the _name_ array with the
  sound context sound_context.  If more than one sound is specified,
  then a sound will be chosen randomly from the list each time the
  sound is played.  If num_sounds == 0, then the entry for the sound
  context is deleted.

  \pre     \c sound_context != NULL
  \arg \c  sound_context sound context to bind sounds to
  \arg \c  names list of sound names to bind to context
  \arg \c  num_sounds number of elements in \c names

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-14
*/
void bind_sounds_to_context( CONST84 char *sound_context, CONST84 char **names, int num_sounds )
{
    int i;
    sound_context_data_t *data;

    check_assertion( initialized_, "audio module not initialized" );

	std::map<std::string,sound_context_data_t>::iterator it;
	
	if((it=soundTable.find(sound_context))!=soundTable.end()){
		/* Entry for this context already exists; decrement ref_cts &
		delete arrays */

		for (i=0; i<it->second.num_sounds; i++) {
	    	decr_sound_data_ref_ctr( it->second.sound_names[i] );
	    	free( it->second.sound_names[i] );
		}
		free( it->second.sound_names );
		free( it->second.chunks );
		soundTable.erase(it);
    }
    
	if ( num_sounds == 0 ) {
		return;
    }

	data = &soundTable[sound_context];

    check_assertion( num_sounds > 0, "num_sounds isn't > 0 " );

    data->num_sounds = num_sounds;
    data->sound_names = (char**)malloc( sizeof(char*)*num_sounds );
    data->chunks = (Mix_Chunk**)malloc( sizeof(Mix_Chunk*)*num_sounds );
    data->loop_count = 0;
    data->channel = 0;
    data->volume = 128;

    for (i=0; i<num_sounds; i++) {
		data->sound_names[i] = string_copy( names[i] );
		incr_sound_data_ref_ctr( names[i] );
		data->chunks[i] = NULL;
    }
}


/*! 
  Associate the music with name _name_ to the music context
  _music_context_.  If _name_ is NULL or the null string, the data for
  the music context will be deleted.

  \pre     \c music_context != \c NULL
  \arg \c  music_context music context to bind sounds to
  \arg \c  name name of music to bind to context
  \arg \c  loop number of loops to play music for (-1 to loop forever)

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-13
  \date Modified: 2000-08-14 */
void bind_music_to_context( CONST84 char *music_context, CONST84 char *name, int loop )
{
    music_context_data_t *data;
	std::map<std::string,music_context_data_t>::iterator it;

	
    check_assertion( initialized_, "audio module not initialized" );

	if((it=musicTable.find(music_context))!=musicTable.end()){
		/* Entry for this context already exists; decrement ref count &
		   delete string */

		/* check if music is playing, stop if it is */
		if ( current_music_name_ != NULL && 
	     	strcmp( current_music_name_, it->second.music_name ) == 0 )
		{
	    	Mix_HaltMusic();
	    	current_music_name_ = NULL;
	    	current_music_data_ = NULL;
	    	check_assertion( get_music_playing_status( it->second.music_name ),
				"inconsistent music playing status info" );
	    	set_music_playing_status( it->second.music_name, false );
		}

		decr_music_data_ref_ctr( it->second.music_name );
		free( it->second.music_name );
		musicTable.erase(it);
    }
		
    if ( name == NULL || name[0]=='\0' ) {
		return;
    }
	
	data = &musicTable[music_context];

    data->music_name = string_copy( name );
    incr_music_data_ref_ctr( name );
    data->music = NULL;
    data->loop = loop;
}


/*! 
  Flushes cached copies of \c Mix_Chunks for all sound contexts

  \pre     

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-13
*/
static void flush_cached_sound_chunks()
{
	std::map<std::string,sound_context_data_t>::iterator it;
	
	for(it=soundTable.begin(); it!=soundTable.end(); it++){
		for (int i=0; i<it->second.num_sounds; i++) {
		    it->second.chunks[i] = NULL;
		}
		if (it->second.loop_count == -1) {
			Mix_HaltChannel(it->second.channel);
		}
    }
} 

/*! 
  Flushes cached copies of \c Mix_Music for all sound contexts

  \pre     

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-13
*/
static void flush_cached_music()
{
	std::map<std::string,music_context_data_t>::iterator it;
	
	for(it=musicTable.begin(); it!=musicTable.end(); it++){
		it->second.music = NULL;
	}
} 


/*! 
  Gets a \c Mix_Chunk for the specified sound context

  \arg \c  sound_context the sound context
  \arg \c  name if non-NULL, is set to the name of the chosen sound

  \return  A (randomly-chosen) \c Mix_Chunk for the specified sound context
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-13
*/
static Mix_Chunk* get_Mix_Chunk( sound_context_data_t *data, char **name )
{
    int i;

    check_assertion( initialized_, "audio module not initialized" );

    if ( is_sound_data_dirty() ) {
	flush_cached_sound_chunks();
	mark_sound_data_clean();
    }

    /* pick a random sound */
    
    i = (int) (((double)data->num_sounds)*rand()/(RAND_MAX+1.0));
    if ( data->chunks[i] == NULL ) {
	bool found;
	found = get_sound_data( data->sound_names[i], &(data->chunks[i]) );
	check_assertion( found, "sound name not found" );
	check_assertion( data->chunks[i]!=NULL, "sound chunk not set" );
    }
    if ( name != NULL ) {
	*name = data->sound_names[i];
    }

    return data->chunks[i];
}


/*! 
  Gets the data associated for the specified music context

  \pre \c music != NULL

  \arg \c  music_context the sound context
  \arg \c  music set to the music data for this context
  \arg \c  name if non-NULL, *name is set to the name of the music
  \arg \c  loop if non-NULL, *loop is set to the loop value of the music

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-08-13
*/
static void get_music_context_data( char *music_context, Mix_Music **music,
				    char **name, int *loop )
{
    check_assertion( music != NULL, "music is null" );

    if ( is_music_data_dirty() ) {
		flush_cached_music();
		mark_music_data_clean();
    }

	std::map<std::string,music_context_data_t>::iterator it;
	
	if((it=musicTable.find(music_context))!=musicTable.end()){
		if ( it->second.music == NULL ) {
	    	bool found;
	    	found = get_music_data( it->second.music_name, &(it->second.music) );
	    	check_assertion( found, "music name not found" );
	    	check_assertion( it->second.music!=NULL, "music data not set" );
		}

		*music = it->second.music;

		if (name != NULL) {
	 	   *name = it->second.music_name;
		}

		if (loop != NULL) {
	 	   *loop = it->second.loop;
		}
    } else {
		// couldn't find any data for this sound_context
		*music = NULL;
    }
}


/*! 
  Play a sound associated with the specified sound context
  \pre     sound_context != NULL
  \arg \c  sound_context the sound context
       \i  SDL number of loops, -1 for infinite
  \return  true if sound was played; false otherwise
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-09-13 ehall
*/
bool play_sound( const char *sound_context, int loop )
{
    Mix_Chunk *chunk;
    check_assertion( initialized_, "audio module not initialized" );

    if ( getparam_sound_enabled() == false ) {
		return false;
    }

    if ( ! is_audio_open() ) {
		return false;
    }

    check_assertion( sound_context != NULL, "sound_context is null" );

	std::map<std::string,sound_context_data_t>::iterator it;
	
	if((it=soundTable.find(sound_context))==soundTable.end()){
		return false;
    }

    chunk = get_Mix_Chunk( &(it->second), NULL );

    if ( chunk == NULL ) {
		return false;
    }

    if (it->second.loop_count == -1) {
		return false;
    }

    Mix_VolumeChunk( chunk, it->second.volume );
    
    it->second.channel = Mix_PlayChannel( -1, chunk, loop );
    it->second.loop_count = loop;

    return true;
}

/*! 
  Halt a sound associated with the specified sound context
  \pre     sound_context != NULL
  \arg \c  sound_context the sound context
     
  \return  true if sound was halted; false otherwise
  \author  ehall
  \date    Created:  2000-09-13
  \date    Modified: 2000-09-13
*/
bool halt_sound( const char *sound_context )
{
    Mix_Chunk *chunk;

    if ( getparam_sound_enabled() == false ) {
	return false;
    }

    if ( ! is_audio_open() ) {
	return false;
    }

    check_assertion( sound_context != NULL, "sound_context is null" );

	std::map<std::string,sound_context_data_t>::iterator it;
	
	if((it=soundTable.find(sound_context))==soundTable.end()){
		return false;
    }

    chunk = get_Mix_Chunk( &(it->second), NULL );

    if ( chunk == NULL ) {
		return false;
    }
	
    if (it->second.loop_count == -1) {
		Mix_HaltChannel( it->second.channel );
    }

    it->second.loop_count = 0;
	it->second.channel = 0;
    return true;
}

/*! 
  Change volume for a sound associated with the specified sound context
  \pre     sound_context != NULL
  \arg \c  sound_context the sound context
       \i  volume is a value 0-128 
  \return  true if volume was set; false otherwise
  \author  ehall
  \date    Created:  2000-09-13
  \date    Modified: 2000-09-13
*/
bool set_sound_volume( const char *sound_context, int volume )
{
    //int i;
    //sound_context_data_t *data;

	
	
	std::map<std::string,sound_context_data_t>::iterator it;
	
	if((it=soundTable.find(sound_context))==soundTable.end()){
		return false;
    }
	
    it->second.volume = volume;

    //i = (int) (((double)data->num_sounds)*rand()/(RAND_MAX+1.0));
    for (int i=0; i<it->second.num_sounds; i++ ) {
		if ( it->second.chunks[i] == NULL ) {
			bool found;
	    	found = get_sound_data( it->second.sound_names[i], &(it->second.chunks[i]) );
	    	check_assertion( found, "sound name not found" );
	    	check_assertion( it->second.chunks[i]!=NULL, "sound chunk not set" );
		}
		Mix_VolumeChunk( it->second.chunks[i], it->second.volume );
    }
    return true;
}



/*---------------------------------------------------------------------------*/
/*! 
  Play music for the specified music context.

  \arg \c music_context the music context for which to play music.  If
  NULL, then music is halted.

  \return  true if music was played, false otherwise
  \author  jfpatry
  \date    Created:  2000-08-13
  \date    Modified: 2000-09-02
*/
bool play_music( char *music_context )
{
    Mix_Music *music;
    char *music_name;
    int loop;

    check_assertion( initialized_, "audio module not initialized" );

    if ( getparam_music_enabled() == false ) {
	return false;
    }

    if ( ! is_audio_open() ) {
	return false;
    }

    if ( music_context == NULL ) {
	if ( current_music_name_ != NULL ) {
	    set_music_playing_status( current_music_name_, false );
	}
	Mix_HaltMusic();
	current_music_name_ = NULL;
	current_music_data_ = NULL;
	return false;
    }

    get_music_context_data( music_context, &music, &music_name, &loop );

    if ( music == NULL ) {
	return false;
    }

    if ( current_music_data_ != music ) {
	if ( current_music_name_ != NULL ) {
	    set_music_playing_status( current_music_name_, false );
	}
	Mix_HaltMusic();
    }

    if ( ! Mix_PlayingMusic() ) {
	Mix_PlayMusic( music, loop );
	set_music_playing_status( music_name, true );
    }

    current_music_data_ = music;
    current_music_name_ = music_name;

    return true;
}


/*! 
  Updates audio status.  Must be called at every frame.

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-09-02
*/
void 
update_audio()
{
    int volume;

    check_assertion( initialized_, "audio module not initialized" );

    if ( ! is_audio_open() ) {
	return;
    }

    if ( getparam_music_enabled() == false ) {
	Mix_HaltMusic();
    }

    /* Set sounds volume level */
    volume = getparam_sound_volume();
    if ( volume < 0 ) {
	volume = 0;
    }
    if ( volume > 128 ) {
	volume = 128;
    }
    setparam_sound_volume( volume );

    Mix_Volume( -1, volume ); /* channel of -1 sets volume for all channels */

    /* Set music volume level */
    volume = getparam_music_volume();
    if ( volume < 0 ) {
	volume = 0;
    }
    if ( volume > 128 ) {
	volume = 128;
    }
    setparam_music_volume( volume );

    Mix_VolumeMusic( volume );

    /* Update music status */
    if ( current_music_name_ != NULL && !Mix_PlayingMusic() ) {
	set_music_playing_status( current_music_name_, false );
	current_music_name_ = NULL;
	current_music_data_ = NULL;
    }
}


/*! 
  Return true if music is playing

  \return  true iff music is playing
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
bool
is_music_playing()
{
    check_assertion( initialized_, "audio module not initialized" );

    check_assertion( ( current_music_name_ == NULL &&
		       current_music_data_ == NULL ) ||
		     ( current_music_name_ != NULL &&
		       current_music_data_ != NULL ),
		     "inconsistent state in sound module" );
    return (current_music_name_ != NULL);
}


/*! 
  Tcl callback for tux_bind_sounds
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
static int bind_sounds_cb( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{
    check_assertion( initialized_, "audio module not initialized" );

    if ( argc < 2 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <sound context> <sound name>"
			 " [<sound name> ...]",
			 (char *)0 );
        return TCL_ERROR;
    } 

    bind_sounds_to_context( argv[1], argv+2, argc-2 );
    return TCL_OK;
} 


/*! 
  Tcl callback for tux_bind_music
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
static int bind_music_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    int loops;

    check_assertion( initialized_, "audio module not initialized" );

    if ( argc != 4 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <music context> <music name> "
			 "<loops>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    if ( Tcl_GetInt( ip, argv[3], &loops ) ) {
	Tcl_AppendResult(
	    ip, "invalid value for loops parameter", (char *) 0 );
	return TCL_ERROR;
    }

    bind_music_to_context( argv[1], argv[2], loops );

    return TCL_OK;
} 


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates audio resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void shutdown_audio()
{
    if ( is_audio_open() ) {
	Mix_CloseAudio();
    }
}


/*! 
  Registers sound module's Tcl callbacks
  \arg \c  ip Tcl interpreter

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
void register_sound_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_bind_sounds", bind_sounds_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_bind_music", bind_music_cb,  0,0);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#else
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void init_audio()
{
}

void bind_sounds_to_context( CONST84 char *sound_context, CONST84 char **names, int num_sounds )
{
}

void bind_music_to_context( CONST84 char *music_context, CONST84 char *name, int loop )
{
}

bool play_sound( const char *sound_context, int loop )
{
    return false;
}

bool play_music( char *music_context )
{
    return false;
}

void update_audio()
{
}

bool is_music_playing()
{
    return false;
}

static int bind_sounds_cb( ClientData cd, Tcl_Interp *ip, 
			   int argc, CONST84 char *argv[]) 
{
    return TCL_OK;
} 

static int bind_music_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    return TCL_OK;
} 

void register_sound_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_bind_sounds", bind_sounds_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_bind_music", bind_music_cb,  0,0);
}

bool halt_sound( const char *sound_context )
{
    return true;
}

void shutdown_audio()
{
}

bool set_sound_volume( const char *sound_context, int volume )
{
    return true;
}

#endif /* defined(HAVE_SDL_MIXER) */
