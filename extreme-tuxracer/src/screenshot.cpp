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
#include "screenshot.h"
#include "ppgltk/images/image.h"

static int screenshot_num = 0;

void screenshot()
{
    char buff[20];
    sprintf( buff, "tux_sshot_%d.ppm", screenshot_num++ );
    if(!take_screenshot( buff )){
        fprintf( stderr, "Couldn't save screenshot\n");
    };
} 

bool take_screenshot ( char* fileName ) {

	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	glReadBuffer( GL_FRONT );
	
	pp::Image image(viewport[2],viewport[3],3);
	
	image.width=viewport[2];
	image.height=viewport[3];
	image.depth=3;
	
	for (int i=0; i<viewport[3]; i++){
		glReadPixels(viewport[0], viewport[1]+viewport[3]-1-i,
			viewport[2], 1, GL_RGB, 
			GL_UNSIGNED_BYTE, image.data+viewport[2]*i*3
			);
	}
	
	return image.writeToFile(fileName);
}
