/* 
 * PPRacer 
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
 
#include "glhelper.h"

#include "../../textures.h"

namespace ppGL {

namespace draw {
	

void
rect(pp::Color color, pp::Vec2d startPos, pp::Vec2d endPos)
{
	glDisable( GL_TEXTURE_2D );
    glColor3dv( (double*)&color );
    glRectf( startPos.x, startPos.y,
	     	 endPos.x,endPos.y );
	glEnable( GL_TEXTURE_2D );
}	
	
void
rect(const char* binding, pp::Vec2d pos, pp::Vec2d size )
{
	GLuint texobj;
	
    if ( !get_texture_binding( binding, &texobj ) ) {
		texobj = 0;
    }
		
    glPushMatrix();
    {	
		glBindTexture( GL_TEXTURE_2D, texobj );
		glEnable( GL_TEXTURE_2D );
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		glTranslatef( pos.x, pos.y, 0.0 );
		glBegin( GL_QUADS );
		{
	    	glTexCoord2f( 0.0, 0.0 );
	    	glVertex2f( 0.0, 0.0 );
	    
	    	glTexCoord2f( 1.0, 0.0 );
			glVertex2f( size.x, 0.0 );
	    
	    	glTexCoord2f( 1.0, 1.0 );
	    	glVertex2f( size.x, size.y );
	    
	    	glTexCoord2f( 0.0, 1.0 );
	    	glVertex2f( 0.0, size.y );
		}
		glEnd();
    }
    glPopMatrix();
}

} //namespace draw
	
} //namespace ppGL
