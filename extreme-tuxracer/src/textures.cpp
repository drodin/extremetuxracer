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

#include "textures.h"
#include "game_config.h"
#include <string>
#include <list>
#include <map>
#include <GL/glext.h>

#include "ppgltk/images/image.h"

static std::map<std::string,texture_node_t> textureTable;
static std::map<std::string,texture_node_t> bindingTable;


bool get_texture_binding( const char *binding, GLuint *texid )
{
    std::map<std::string,texture_node_t>::iterator texnode;
    if ( (texnode = bindingTable.find(binding))!=bindingTable.end() ){
		*texid = texnode->second.texture_id;
		return true;
    }
    return false;  
}

bool load_and_bind_texture( const char *binding, const char *filename )
{
    return (bool) ( load_texture( binding, filename, 1 ) &&
		      bind_texture( binding, binding ) );
}

void init_textures() 
{
} 

int get_min_filter()
{
    switch( getparam_mipmap_type() ) {
    case 0: 
	return GL_NEAREST;
    case 1:
	return GL_LINEAR;
    case 2: 
	return GL_NEAREST_MIPMAP_NEAREST;
    case 3: 
	return GL_LINEAR_MIPMAP_NEAREST;
    case 4: 
	return GL_NEAREST_MIPMAP_LINEAR;
    case 5: 
	return GL_LINEAR_MIPMAP_LINEAR;
    default:
	return GL_LINEAR_MIPMAP_NEAREST;
    }
}

bool load_texture( const char *texname, const char *filename, int repeatable )
{
	pp::Image *texImage = pp::Image::readFile(filename);
	
    texture_node_t *tex;
    int max_texture_size;


    print_debug(DEBUG_TEXTURE, "Loading texture %s from file: %s", 
		texname, filename);

    if ( texImage == NULL ) {
    	print_warning( IMPORTANT_WARNING, 
		       "couldn't load image %s", filename );
    	return false;
    }

	std::map<std::string,texture_node_t>::iterator it;
    if ( (it = textureTable.find(texname))!=textureTable.end() ){
		tex = &it->second;
		print_debug(DEBUG_TEXTURE, "Found texture %s with id: %d", 
		    texname, it->second.texture_id);
        glDeleteTextures( 1, &(tex->texture_id) );
	}else{
		tex = &textureTable[texname];
		tex->ref_count = 0;
	}
    
    tex->repeatable = repeatable;
    glGenTextures( 1, &(tex->texture_id) );
    glBindTexture( GL_TEXTURE_2D, tex->texture_id );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


    if ( repeatable ) {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    } else {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                     get_min_filter() );
		
    /* Check if we need to scale image */
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &max_texture_size );
    if ( texImage->width > max_texture_size ||
	 texImage->height > max_texture_size ) 
    {
	char *newdata = (char*)malloc( texImage->depth *
				       max_texture_size *
				       max_texture_size );

	check_assertion( newdata != NULL, "out of memory" );

	print_debug( DEBUG_TEXTURE, "Texture `%s' too large -- scaling to "
		     "maximum allowed size",
		     filename );

	/* In the case of large- or small-aspect ratio textures, this
           could end up using *more* space... oh well. */
	gluScaleImage( texImage->depth == 3 ? GL_RGB : GL_RGBA,
		       texImage->width, texImage->height, 
		       GL_UNSIGNED_BYTE,
		       texImage->data,
		       max_texture_size, max_texture_size, 
		       GL_UNSIGNED_BYTE,
		       newdata );

	free( texImage->data );
	texImage->data = (unsigned char*) newdata;
	texImage->width = max_texture_size;
	texImage->height = max_texture_size;
    }

    gluBuild2DMipmaps( GL_TEXTURE_2D, texImage->depth, texImage->width,
		       texImage->height, texImage->depth == 3 ? GL_RGB : GL_RGBA, 
		       GL_UNSIGNED_BYTE, texImage->data );

	delete texImage;

    return true;
} 


texture_node_t*
get_texture( const char *texname)
{
	std::map<std::string,texture_node_t>::iterator it;
	
	if ( (it = textureTable.find(texname))!=textureTable.end() ){
		return &(it->second);
	}else{
		return NULL;
	}
}


bool
del_texture( const char *texname )
{
    
	print_debug( DEBUG_TEXTURE, "Deleting texture %s", texname );

	std::map<std::string,texture_node_t>::iterator it;
	if ( (it = textureTable.find(texname))!= textureTable.end() ){
		check_assertion( it->second.ref_count == 0,
			 "Trying to delete texture with non-zero reference "
				"count" );
		glDeleteTextures( 1, &(it->second.texture_id) );
		textureTable.erase(it);
		return true;
    }
	return false;
}


bool bind_texture( const char *binding, const char *texname )
{
    
	texture_node_t *tex;
	
	print_debug(DEBUG_TEXTURE, "Binding %s to texture name: %s", 
		binding, texname);
	
	tex = get_texture( texname );
	
	if (tex == NULL){
		print_warning( IMPORTANT_WARNING, 
		       "Attempt to bind to Texture unloaded texture: `%s'\n", texname );
		return false;
	}

	std::map<std::string,texture_node_t>::iterator oldtex;
    if ( (oldtex = bindingTable.find(binding))!=bindingTable.end() ){
		oldtex->second.ref_count--;
		bindingTable.erase(oldtex);
    }

	bindingTable[binding] = *tex;
	tex->ref_count++;

    return true;
}

bool unbind_texture( const char *binding )
{
	std::map<std::string,texture_node_t>::iterator tex;
	if ( (tex = bindingTable.find(binding))!=bindingTable.end() ){
		tex->second.ref_count--;
		bindingTable.erase(tex);
		return true;
	}
	return false;
}

void get_current_texture_dimensions( int *width, int *height )
{
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width );
    glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height );
}

bool flush_textures(void)
{
	bool result;
	std::map<std::string,texture_node_t>::iterator mapit;
	for(mapit=textureTable.begin();mapit!=textureTable.end(); mapit++){
		if (mapit->second.ref_count == 0) {
			result = del_texture( (*mapit).first.c_str() );
			check_assertion(result, "Attempt to flush non-existant texture");	
		    // RK: reset pointer to start of table since mapit got deleted
            mapit = textureTable.begin();
		}
	}
	return true;
}

static int load_texture_cb ( ClientData cd, Tcl_Interp *ip, int argc, 
			     CONST84 char *argv[]) 
{
    int repeatable = 1;

    if ( ( argc != 3 ) && (argc != 4) ) {
	Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], "<texture name> <image file>",
			 " [repeatable]", (char *)0 );
	return TCL_ERROR;
    } 

    if ( ( argc == 4 ) && ( Tcl_GetInt( ip, argv[3], &repeatable ) != TCL_OK ) ) {
        Tcl_AppendResult(ip, argv[0], ": invalid repeatable flag",
			 (char *)0 );
        return TCL_ERROR;
    } 
    
    if (!load_texture(argv[1], argv[2], repeatable)) {
	Tcl_AppendResult(ip, argv[0], ": Could not load texture ", 
			 argv[2], (char*)0);
	return TCL_ERROR;
    }

    return TCL_OK;
}

static int bind_texture_cb ( ClientData cd, Tcl_Interp *ip, int argc, 
			     CONST84 char *argv[])
{
    if ( argc != 3 ) {
	Tcl_AppendResult(ip, argv[0], ": invalid number of arguments\n", 
			 "Usage: ", argv[0], "<object name> <texture name>",
			 (char *)0 );
	return TCL_ERROR;
    } 

    if (!bind_texture(argv[1], argv[2])) {
	Tcl_AppendResult(ip, argv[0], ": Could not bind texture ", 
			 argv[2], (char*)0);
	return TCL_ERROR;
    }

    return TCL_OK;
}


void register_texture_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_load_texture",   load_texture_cb,   0,0);
    Tcl_CreateCommand (ip, "tux_bind_texture",   bind_texture_cb,   0,0);
}
