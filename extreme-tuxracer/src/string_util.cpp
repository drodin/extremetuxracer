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

#include "debug.h"
#include "error_util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *string_copy( const char *src ) 
{
    char *dest;

    check_assertion( src != NULL, "string NULL in string_copy" );

    dest = (char *) malloc( sizeof(char) * ( strlen( src ) + 1 ) );

    if ( dest == NULL ) {
	handle_system_error( 1, "malloc failed" );
    }

    strcpy( dest, src );
    return dest;
}
