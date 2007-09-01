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

#include "fog.h"
#include "gl_util.h"
#include "tcl_util.h"

#include "game_config.h"


FogPlane fogPlane;

void 
FogPlane::reset()
{
    m_isOn = true;
    m_mode = GL_LINEAR;
    m_density = 0.005;
    m_start = 0.0;
    m_end = getparam_forward_clip_distance();
    init_glfloat_array( 4, m_color, 1.0, 1.0, 1.0, 1.0 );
}

void 
FogPlane::setup()
{
    if ( !m_isOn || getparam_disable_fog() ) {
		glDisable( GL_FOG );
		return;	
    }

    glEnable( GL_FOG );

    glFogi( GL_FOG_MODE, m_mode );
    glFogf( GL_FOG_DENSITY, m_density );
    glFogf( GL_FOG_START, m_start );
    glFogf( GL_FOG_END, m_end );
    glFogfv( GL_FOG_COLOR, m_color );

    if ( getparam_nice_fog() ) {
		glHint( GL_FOG_HINT, GL_NICEST );
    } else {
		glHint( GL_FOG_HINT, GL_FASTEST );
    }
	
}

static int fog_cb (ClientData cd, Tcl_Interp *ip, 
		   int argc, CONST84 char *argv[]) 
{
    double tmp_arr[4];
    double tmp_dbl;
    bool error = false;
    
    if (argc < 2) {
		error = true;
    }

    NEXT_ARG;

    while ( !error && argc > 0 ) {

	if ( strcmp( "-on", *argv ) == 0 ) {
	    fogPlane.setEnabled();
	} else if ( strcmp( "-off", *argv ) == 0 ) {
	    fogPlane.setEnabled(false);
	} else if ( strcmp( "-mode", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }

	    if ( strcmp( "exp", *argv ) == 0 ) {
			fogPlane.setMode(GL_EXP);
	    } else if ( strcmp( "exp2", *argv ) == 0 ) {
			fogPlane.setMode(GL_EXP2);
	    } else if ( strcmp( "linear", *argv ) == 0 ) {
			fogPlane.setMode(GL_LINEAR);
	    } else {
			print_warning( TCL_WARNING, "tux_fog: mode must be one of "
			       "`exp', `exp2', or `linear'" );
			error = true;
	    }
	} else if ( strcmp( "-density", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }
	    	if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
			error = true;
			break;
	    }
	    fogPlane.setDensity(tmp_dbl);
	} else if ( strcmp( "-start", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
			error = true;
			break;
	    }
	    fogPlane.setStart(tmp_dbl);
	} else if ( strcmp( "-end", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
			error = true;
			break;
	    }
	    fogPlane.setEnd(tmp_dbl);
	} else if ( strcmp( "-color", *argv ) == 0 ) 
	{
	    NEXT_ARG;
	    if ( argc == 0 ) {
			error = true;
			break;
	    }
	    if ( get_tcl_tuple ( ip, *argv, tmp_arr, 4 ) == TCL_ERROR ) {
			error = true;
			break;
	    }
	    copy_to_glfloat_array( fogPlane.getColor(), tmp_arr, 4 );
	} else {
	    print_warning( TCL_WARNING, "tux_fog: unrecognized "
			   "parameter `%s'", *argv );
	}

	NEXT_ARG;
    }

    if ( error ) {
	print_warning( TCL_WARNING, "error in call to tux_fog" );
	Tcl_AppendResult(
	    ip, 
	    "\nUsage: tux_fog [-on|-off] "
	    "[-mode [exp|exp2|linear]] "
	    "[-density <value>] "
	    "[-start <value>] "
	    "[-end <value>] "
	    "[-color { r g b a }] ",
	    (char *) 0 );
	return TCL_ERROR;
    }
    
    return TCL_OK;
}

void
FogPlane::registerCallbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_fog", fog_cb,  0,0);
}
