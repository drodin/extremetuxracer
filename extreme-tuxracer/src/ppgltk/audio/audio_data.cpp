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

#include "audio_data.h"
#include "audio.h"

#if defined(HAVE_SDL_MIXER)

#include "SDL.h"
#include "SDL_mixer.h"

#include <string>
#include <list>
#include <map>

#define SOUNDFILE_MISSING_WARNING_LEVEL 10
#define SOUNDFILE_BUSY_WARNING_LEVEL 10

typedef struct {
    Mix_Chunk *data;    /* sound data */
    int ref_ctr;        /* reference counter */
} sound_record_t;

typedef struct {
    Mix_Music *data;    /* music data */
    int ref_ctr;        /* reference counter */
    bool playing;     /* is this music playing? */
} music_record_t;

//static hash_table_t sound_hash_;
static std::map<std::string,sound_record_t> soundTable;

//static hash_table_t music_hash_;
static std::map<std::string,music_record_t> musicTable;

static bool sound_dirty_ = false;
static bool music_dirty_ = false;


/* Name:          init_sound_data
   Description:   initializes audio module
   Precondition:  none
   Postcondition: audio module initialized
   Return value:  none
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void 
init_audio_data()
{
}

/* Name:          load_sound
   Description:   Loads a sound from a file
   Precondition:  name, filename != null
   Return value:  true if successful, false otherwise
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
load_sound( const char *name, const char *filename ) 
{ 
    char *data_ptr;
    char *record_ptr;
    char *temp_record_ptr;
    int ref_ctr = 0;

    check_assertion( name != NULL, "null name" );
    check_assertion( filename != NULL, "null filename" );

    if ( ! is_audio_open() ) {
		return false;
    }

	std::map<std::string,sound_record_t>::iterator soundit;
	
	if((soundit=soundTable.find(name))!=soundTable.end()){
		print_debug( DEBUG_SOUND, "Overwriting sound name %s", name );
				
		ref_ctr = soundit->second.ref_ctr;
		soundTable.erase(soundit);
	}
	
	data_ptr = (char*) Mix_LoadWAV( filename );

    if ( data_ptr == NULL ) {
		print_warning( SOUNDFILE_MISSING_WARNING_LEVEL, 
		       "FAILED to load sound file %s: %s", 
		       filename, Mix_GetError() );
		return false;
    }

    print_debug( DEBUG_SOUND, "Successfully loaded sound file %s", 
		 filename );

	
	sound_record_t *srec = &soundTable[name];
		
	srec->data = (Mix_Chunk*) data_ptr;
	srec->ref_ctr = ref_ctr;
	record_ptr = (char*) srec;	
	

    /* Make sure it's there */
    /*
	check_assertion( get_hash_entry( hash, name, (void*) &temp_record_ptr ) &&
		     ( record_ptr == temp_record_ptr ), 
		     "failed addition to hash table" );
    */
	temp_record_ptr = NULL; /* to prevent warnings when assert turned off */

	sound_dirty_ = true;

    return true;
}

/* Name:          load_music
   Description:   Loads music from a file
   Precondition:  name, filename != null
   Return value:  true if successful, false otherwise
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
load_music( const char *name, const char *filename ) 
{ 
	char *data_ptr;
    char *record_ptr;
    char *temp_record_ptr;
    int ref_ctr = 0;

    check_assertion( name != NULL, "null name" );
    check_assertion( filename != NULL, "null filename" );

    if ( ! is_audio_open() ) {
	return false;
    }
   
	std::map<std::string,music_record_t>::iterator musicit;
		
	if((musicit=musicTable.find(name))!=musicTable.end()){
		if ( musicit->second.playing ) {
			print_warning( SOUNDFILE_BUSY_WARNING_LEVEL, 
			       "Can't overwrite music name %s since "
			       "it is playing", name );
			return false;
		}
		ref_ctr = musicit->second.ref_ctr;
		musicTable.erase(musicit);
	}
	/*
    if ( get_hash_entry( hash, name, (hash_entry_t*) &record_ptr ) ) {
		print_debug( DEBUG_SOUND, "Overwriting music name %s", name );

		// Need to save ref_ctr
		music_record_t *mrec = (music_record_t*)record_ptr;
		if ( mrec->playing ) {
			// Can't overwrite since music is playing 
			print_warning( SOUNDFILE_BUSY_WARNING_LEVEL, 
			       "Can't overwrite music name %s since "
			       "it is playing", name );
			return false;
		}
		ref_ctr = mrec->ref_ctr;
		del_hash_entry( hash, name, NULL );
    }
	*/
	
	data_ptr = (char*) Mix_LoadMUS( filename );

    if ( data_ptr == NULL ) {
	print_warning( SOUNDFILE_MISSING_WARNING_LEVEL, 
		       "FAILED to load music file %s: %s", 
		       filename, Mix_GetError() );
	return false;
    }

    print_debug( DEBUG_SOUND, "Successfully loaded music file %s", 
		 filename );

	music_record_t *mrec = (music_record_t*)malloc(sizeof(music_record_t));
	mrec->data = (Mix_Music*) data_ptr;
	mrec->ref_ctr = ref_ctr;
	mrec->playing = false;

	record_ptr = (char*) mrec;

    musicTable[name] = *mrec;

    /* Make sure it's there */
    /*check_assertion( get_hash_entry( hash, name, (void*) &temp_record_ptr ) &&
		     ( record_ptr == temp_record_ptr ), 
		     "failed addition to hash table" );
    */
	temp_record_ptr = NULL; /* to prevent warnings when assert turned off */


	music_dirty_ = true;
    
    return true;
	
}


/* Name:          get_sound_data
   Description:   returns the Mix_Chunk for the sound _name_
   Precondition:  name != NULL
   Return value:  true if entry for name exists, false otherwise
   Modified args: data if non-NULL
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
get_sound_data( char *name, Mix_Chunk **data )
{
    std::map<std::string,sound_record_t>::iterator srec;
	if((srec=soundTable.find(name))==soundTable.end()){
		return false;
	}

    if ( data ) {
		*data = srec->second.data;
    }
    return true;
}

/* Name:          get_music_data
   Description:   returns the Mix_Music for the music _name_
   Precondition:  name != NULL
   Return value:  true if entry for name exists, false otherwise
   Modified args: data if non-NULL
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
get_music_data( char *name, Mix_Music **data )
{
	std::map<std::string,music_record_t>::iterator mrec;
	if((mrec=musicTable.find(name))==musicTable.end()){
		return false;
	}

    if ( data ) {
		*data = mrec->second.data;
    }
    return true;
}


/*! 

  Set the "playing" status of the music data (if music is marked as
  playing it can't be deleted).

  \brief   brief_desc
  \pre     name != NULL, named music exists 
  \arg \c  name name of music
  \arg \c  playing the new value for the playing status

  \return  none
  \author  jfpatry
  \date    Created:  2000-08-14
  \date Modified: 2000-08-14 
*/
void
set_music_playing_status( char *name, bool playing )
{
	std::map<std::string,music_record_t>::iterator mrec;

	if((mrec=musicTable.find(name))==musicTable.end()){
		check_assertion(0, "couldn't find music" );
	}
	
    mrec->second.playing = playing;
}


/*! 
  Get the "playing" status of the music data
  \pre     name != NULL, named music exists
  \arg \c  name name of music

  \return  playing status of music
  \author  jfpatry
  \date    Created:  2000-08-14
  \date    Modified: 2000-08-14
*/
bool
get_music_playing_status( char *name )
{
	std::map<std::string,music_record_t>::iterator mrec;

	if((mrec=musicTable.find(name))==musicTable.end()){
		check_assertion(0, "couldn't find music" );
	}
	
    return mrec->second.playing;
}


/* Name:          incr_sound_data_ref_ctr
   Description:   Increments the reference count for sound _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void incr_sound_data_ref_ctr( const char *name ) 
{
   	std::map<std::string,sound_record_t>::iterator srec;
	if((srec=soundTable.find(name))==soundTable.end()){
		//check_assertion( found, "hashtable entry not found" );
	}
	
	srec->second.ref_ctr++;
	print_debug( DEBUG_SOUND, "incremented reference counter of sound %s "
		 "to %d", name, srec->second.ref_ctr );
}


/* Name:          decr_sound_data_ref_ctr
   Description:   Decrements the reference count for sound _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void decr_sound_data_ref_ctr( const char *name ) 
{
   	std::map<std::string,sound_record_t>::iterator srec;
	if((srec=soundTable.find(name))==soundTable.end()){
		//check_assertion( found, "hashtable entry not found" );
	}
	
	srec->second.ref_ctr--;
	print_debug( DEBUG_SOUND, "decremented reference counter of sound %s "
		 "to %d", name, srec->second.ref_ctr );
}

/* Name:          incr_music_data_ref_ctr
   Description:   Increments the reference count for music _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void incr_music_data_ref_ctr( const char *name ) 
{
	std::map<std::string,music_record_t>::iterator mrec;

	if((mrec=musicTable.find(name))==musicTable.end()){
		check_assertion(0, "couldn't find music" );
	}
    mrec->second.ref_ctr++;

    print_debug( DEBUG_SOUND, "incremented reference counter of music %s "
		 "to %d", name, mrec->second.ref_ctr );
}

/* Name:          decr_music_data_ref_ctr
   Description:   Decrements the reference count for music _name_
   Precondition:  name != NULL, entry for name exists
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void decr_music_data_ref_ctr( const char *name ) 
{
	std::map<std::string,music_record_t>::iterator mrec;

	if((mrec=musicTable.find(name))==musicTable.end()){
		check_assertion(0, "couldn't find music" );
	}

    mrec->second.ref_ctr--;

    print_debug( DEBUG_SOUND, "decremented reference counter of music %s "
		 "to %d", name, mrec->second.ref_ctr );
}

/* Name:          delete_unused_audio_data
   Description:   Frees all sound and music data with reference counts == 0
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void delete_unused_audio_data()
{
	std::list<char*> keysToDelete;

    // clean sounds first
	std::map<std::string,sound_record_t>::iterator sit;

	for(sit=soundTable.begin(); sit!=soundTable.end(); sit++){
		if ( sit->second.ref_ctr == 0 ) {
	    	Mix_FreeChunk( sit->second.data );
			soundTable.erase(sit);
		}	
	}
	
    // clean music
	std::map<std::string,music_record_t>::iterator mit;

	for(mit=musicTable.begin(); mit!=musicTable.end(); mit++){
		if ( mit->second.ref_ctr == 0 ) {
			// we shouldn't be playing music with ref cnt of 0
			check_assertion( mit->second.playing == false, 
			     "playing music with reference count of 0" );
			Mix_FreeMusic( mit->second.data );
			musicTable.erase(mit);
		}
	}
}


/* Name:          is_sound_data_dirty

   Description: Returns true if new sound data has been loaded since
   last call to mark_sound_data_clean()

   Precondition:  
   Return value:  true iff sound data dirty
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
is_sound_data_dirty()
{
    //check_assertion( initialized_, "audio_data module not initialized" );

    return sound_dirty_;
}

/* Name:          is_music_data_dirty

   Description: Returns true if new music data has been loaded since
   last call to mark_music_data_clean()

   Precondition:  
   Return value:  true iff music data dirty
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
bool
is_music_data_dirty()
{
//    check_assertion( initialized_, "audio_data module not initialized" );

    return music_dirty_;
}


/* Name:          mark_sound_data_clean
   Description:   Mark sound data as clean
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void
mark_sound_data_clean()
{
//    check_assertion( initialized_, "audio_data module not initialized" );

    sound_dirty_ = false;
}


/* Name:          mark_music_data_clean
   Description:   Mark music data as clean
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void
mark_music_data_clean()
{
//    check_assertion( initialized_, "audio_data module not initialized" );

    music_dirty_ = false;
}


/* Name:          load_sound_cb
   Description:   Tcl callback for tux_sound_load
   Precondition:  
   Return value:  Tcl error status
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-09-02
*/
static int load_sound_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    Tcl_Obj *result;

//    check_assertion( initialized_, "audio_data module not initialized" );

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <name> <sound file>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    result = Tcl_NewBooleanObj( load_sound( argv[1], argv[2] ) );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 

/* Name:          load_music_cb
   Description:   Tcl callback for tux_music_load
   Precondition:  
   Return value:  Tcl error status
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-09-02
*/
static int load_music_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    Tcl_Obj *result;

//    check_assertion( initialized_, "audio_data module not initialized" );

    if ( argc != 3 ) {
        Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " <name> <sound file>",
			 (char *)0 );
        return TCL_ERROR;
    } 

    result = Tcl_NewBooleanObj( load_music( argv[1], argv[2] ) );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 


/* Name:          register_sound_data_tcl_callbacks
   Description:   register sound data tcl callbacks ;)
   Precondition:  
   Return value:  
   Modified args: 
   Author:        jfpatry
   Created:       2000-08-13
   Last Modified: 2000-08-13
*/
void register_sound_data_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_load_sound", load_sound_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_load_music", load_music_cb,  0,0);
}

#else

void 
init_audio_data()
{
}

bool 
load_sound( char *name, char *filename ) 
{ 
    return true;
}

bool 
load_music( char *name, char *filename ) 
{ 
    return true;
}

bool
get_sound_data( char *name, Mix_Chunk **data )
{
    return false;
}

bool
get_music_data( char *name, Mix_Music **data )
{
    return false;
}

void
set_music_playing_status( char *name, bool playing )
{
}


bool
get_music_playing_status( char *name )
{
    return false;
}

void incr_sound_data_ref_ctr( char *name ) 
{
}

void decr_sound_data_ref_ctr( char *name ) 
{
}

void incr_music_data_ref_ctr( char *name ) 
{
}

void decr_music_data_ref_ctr( char *name ) 
{
}
 
void delete_unused_audio_data()
{
}

bool
is_sound_data_dirty()
{
    return false;
}

bool
is_music_data_dirty()
{
    return false;
}

void
mark_sound_data_clean()
{
}

void
mark_music_data_clean()
{
}


static int load_sound_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    Tcl_Obj *result;

    result = Tcl_NewBooleanObj( 1 );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 

static int load_music_cb( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[]) 
{
    Tcl_Obj *result;

    result = Tcl_NewBooleanObj( 1 );
    Tcl_SetObjResult( ip, result );
    return TCL_OK;
} 


void register_sound_data_tcl_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_load_sound", load_sound_cb,  0,0);
    Tcl_CreateCommand (ip, "tux_load_music", load_music_cb,  0,0);
}

#endif /* defined(HAVE_SDL_MIXER) */
