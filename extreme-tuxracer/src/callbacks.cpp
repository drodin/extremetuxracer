/* 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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

#include "callbacks.h"

#include "tcl_util.h"
#include "ppgltk/font.h"
#include "ppgltk/alg/color.h"

static int
pp_register_font_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[])
{
	bool error = false;
	double tmp_dbl;

    CONST84 char *binding = NULL;
    CONST84 char *fileName = NULL;
    pp::Color color = pp::Color::white;
    unsigned int size = 30;

	if ( argc < 7 ) {
		Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], " -binding <binding> -font <ttf file> -size <fontsize> [-color <color>]",
			 (char *)0 );
        return TCL_ERROR;
    } 
	    
    NEXT_ARG;

    while ( !error && argc > 0 ) {

	if ( strcmp( "-binding", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = true;
		break;
	    }
	    binding = *argv;
	} else if ( strcmp( "-font", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = true;
		break;
	    }
	    fileName = *argv;
	} else if ( strcmp( "-size", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = true;
		break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
		error = true;
		break;
	    }
	    size = int(tmp_dbl);
	} else if ( strcmp( "-color", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = true;
		break;
	    }
	    if ( get_tcl_tuple( ip, *argv, (double*)&color, 4 ) == 
		 TCL_ERROR ) 
	    {
		error = true;
		break;
	    }
		
	} else {
	    print_warning( TCL_WARNING, "pp_load_font: unrecognized "
			   "parameter `%s'", *argv );
	}

	NEXT_ARG;
    }

    if ( binding == NULL || fileName == NULL ){
		error = true;
    }

    if ( error ) {
		return TCL_ERROR;
    }

	pp::Font::registerFont(binding, fileName, size, color);

    return TCL_OK;
}

static int
pp_bind_font_cb ( ClientData cd, Tcl_Interp *ip, 
			  int argc, CONST84 char *argv[])
{
	bool error = false;

    CONST84 char *binding = NULL;
    CONST84 char *fontName = NULL;
    pp::Color *color = NULL;

	NEXT_ARG;

    while ( !error && argc > 0 ) {

	if ( strcmp( "-binding", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = true;
		break;
	    }
	    binding = *argv;
	} else if ( strcmp( "-font", *argv ) == 0 ) {
	    NEXT_ARG;
		if ( argc == 0 ) {
			error = true;
			break;
	    }
	    fontName = *argv;
	} else if ( strcmp( "-color", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }
		
		color = new pp::Color;
		
	    if ( get_tcl_tuple( ip, *argv, (double*)color, 4 ) == 
		 TCL_ERROR ) 
	    {
			error = true;
			break;
	    }
	} else {
	    print_warning( TCL_WARNING, "pp_bind_font: unrecognized "
			   "parameter `%s'", *argv );
	}

	NEXT_ARG;
    }

    if ( binding == NULL || fontName == NULL ){
		error = true;
    }

    if ( error ) {
		return TCL_ERROR;
    }

	if(color){
		pp::Font::bindFont(binding, fontName, *color);
		delete color;
	}else{
		pp::Font::bindFont(binding, fontName);
	}   
    return TCL_OK;
}


void
register_common_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "pp_load_font", pp_register_font_cb,   0,0);
    Tcl_CreateCommand (ip, "pp_bind_font", pp_bind_font_cb,   0,0);
}
