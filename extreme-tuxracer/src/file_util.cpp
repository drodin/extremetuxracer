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

#include "etracer.h"
#include "file_util.h"
#include "string_util.h"

#if defined( __CYGWIN__ )
#  include <sys/cygwin.h>
#endif

#ifndef MAX_PATH
#  ifdef PATH_MAX
#    define MAX_PATH PATH_MAX
#  else
#    define MAX_PATH 8192 /* this ought to be more than enough */
#  endif
#endif

static void convert_path( char *new_path, char *orig_path ) 
{
#if defined( __CYGWIN__ )
    cygwin_conv_to_posix_path( orig_path, new_path );
#else
    strcpy( new_path, orig_path );
#endif /* defined( __CYGWIN__ ) */
}

bool file_exists( char *filename )
{
#if defined( WIN32 ) && !defined( __CYGWIN__ )

    /* Test existence by opening file -- I'm not a Win32 programmer,
       so if there's a better way let me know */
    FILE *file;
    file = fopen( filename, "r" );

    if ( file == NULL ) {
	return false;
    } else {
	if ( fclose( file ) != 0 ) {
	    handle_error( 1, "error closing file %s", filename );
	}
	return true;
    }

#else

    /* Unix/Linux/Cygwin */
    
    struct stat stat_info;
    bool file_exists = false;
    char filename_copy[MAX_PATH];

    convert_path( filename_copy, filename );

    if ( stat( filename_copy, &stat_info ) != 0 ) {
	if ( errno != ENOENT ) {
	    handle_system_error(1, "couldn't stat %s", filename_copy);
	}
	file_exists = false;
    } else {
	file_exists = true;
    }

    return file_exists;

#endif /* defined( WIN32 ) && !defined( __CYGWIN__ ) */
}


bool dir_exists( char *dirname )
{
#if defined( WIN32 ) && !defined( __CYGWIN__ )

    /* Win32 */

    char curdir[MAX_PATH];
    bool dir_exists = false;

    if ( getcwd( curdir, BUFF_LEN - 1 ) == NULL ) {
	handle_system_error( 1, "getcwd failed" );
    }

    if ( chdir( dirname ) == -1 ) {
	return false;
    }

    if ( chdir( curdir ) == -1 ) {
	handle_system_error( 1, "Couldn't access directory %s", curdir );
    }
    return true;

#else

    /* Unix/Linux/Cygwin */

    char dir_copy[MAX_PATH];
    DIR *d;

    convert_path( dir_copy, dirname );

    if ( ( d = opendir( dir_copy ) ) == NULL ) {
	return bool ((errno != ENOENT) && (errno != ENOTDIR));
    } 

    if ( closedir( d ) != 0 ) {
	handle_system_error( 1, "Couldn't close directory %s", dirname );
    }

    return true;

#endif /* defined( WIN32 ) && !defined( __CYGWIN__ ) */
}

std::list<char*>* get_dir_file_list( char *dirname ) 
{
	#if defined ( WIN32 ) && !defined( __CYGWIN__ ) 

    /* Win32 */
	std::list<char*> *dirList = new std::list<char*>();
    char curdir[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA finddata;

    if ( getcwd( curdir, BUFF_LEN - 1 ) == NULL ) {
		handle_system_error( 1, "getcwd failed" );
    }

    if ( chdir( dirname ) == -1 ) {
		return dirList;
    }

    if ( ( hFind = FindFirstFile( "*.*", &finddata ) ) == 
	 INVALID_HANDLE_VALUE ) 
    {
		return dirList;
    }

    do {
		dirList->push_back(string_copy( finddata.cFileName ));
    } while ( FindNextFile( hFind, &finddata ) );

    if ( !FindClose( hFind ) ) {
		handle_system_error( 1, "Couldn't close directory %s", dirname );
    }

    if ( chdir( curdir ) == -1 ) {
		handle_system_error( 1, "Couldn't access directory %s", curdir );
    }

    return dirList;

#else

    /* Unix/Linux/Cygwin */
    DIR *dir_stream;
	std::list<char*> *dirList = new std::list<char*>();
	
    struct dirent* cur_entry;
    char dir_copy[MAX_PATH];

    convert_path( dir_copy, dirname );
    dir_stream = opendir( dir_copy );

    if ( dir_stream == NULL ) {
		return dirList;
    }

    while ( ( cur_entry = readdir( dir_stream ) ) != NULL ) {
		dirList->push_back(string_copy( cur_entry->d_name ));
    }

    if ( closedir( dir_stream ) != 0 ) {
		handle_system_error( 1, "Couldn't close directory %s", dirname );
    }

    return dirList;

#endif /* defined ( WIN32 ) && !defined( __CYGWIN__ ) */
}

void free_dir_file_list(  std::list<char*>* dirList )
{
	std::list<char*>::iterator it;
	for(it=dirList->begin(); it!=dirList->end(); it++){
		free( *it );
	}
    delete dirList;
}
