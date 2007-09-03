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
#include "tex_font_metrics.h"

#include "ppgltk/ppgltk.h"

#define MAX_TEX_FONT_CHARS 256

typedef struct {
    pp::Vec2d ll, lr, ul, ur;                 /* geometry coordinates */
    pp::Vec2d tex_ll, tex_lr, tex_ul, tex_ur; /* texture coordinates */
    double kern_width;  /* distance that this char takes when rendered 
			     as part of a string */
} tfm_char_data_t;

struct tex_font_metrics_ {
    int max_ascent;
    int max_descent;
    tfm_char_data_t *char_data[MAX_TEX_FONT_CHARS];
};

struct char_dims {
    unsigned short ch;
    unsigned char w;
    unsigned char h;
    char x_offset;
    char y_offset;
    char kern_width;
    short x_pixel;
    short y_pixel;
};


#define READ_BYTES( file, addr, bytes, swap ) \
if ( fread( (addr), (bytes), 1, (file) ) != 1 ) { \
    err_msg = "Unexpected end of file"; \
    goto bail; \
} else { \
    if ((swap)) { \
	if ( (bytes) == 4 ) { \
	    SWAP_WORD(*(addr)); \
	} else if ( bytes == 2 ) { \
	    SWAP_SHORT(*(addr)); \
	} else { \
	    check_assertion( 0, \
		"Can't byte-swap storage with size != 2 of 4 bytes" ); \
	} \
    } \
}


tex_font_metrics_t* load_tex_font_metrics( const char *filename )
{
    tex_font_metrics_t *tfm = NULL;
    FILE *tfm_file = NULL;
    int i;
    char magic[4];
    char *err_msg;
    int endian_check;
    bool swap_bytes;
    struct char_dims ch_dims;
    int num_chars;
    int texture_width, texture_height;
    char dummy;

    check_assertion( sizeof(int) == 4,
		     "This architecture's integer size is != 4" );
    check_assertion( sizeof(short) == 2,
		     "This architecture's short integer size is != 2" );
    check_assertion( sizeof(char) == 1,
		     "This architecture's char size is != 1" );

    /* Open file */
    tfm_file = fopen( filename, "rb" );
    if ( tfm_file == NULL ) {
	print_warning( MISSING_FILE_WARNING,
		       "Couldn't open font metrics file %s", filename );
	return NULL;
    }

    tfm = (tex_font_metrics_t*)malloc( sizeof(tex_font_metrics_t) );
    check_assertion( tfm != NULL, "out of memory" );

    /* Initialize tfm */
    for (i=0; i<MAX_TEX_FONT_CHARS; i++) {
	tfm->char_data[i] = NULL;
    }

    /* Check magic number */
    READ_BYTES( tfm_file, magic, sizeof(magic), false );

    if ( strncmp( magic, "\377tfm", 4 ) != 0 ) {
	err_msg = "File is not a valid tfm file";
	goto bail;
    }

    /* Check endian-ness */
    READ_BYTES( tfm_file, &endian_check, sizeof(int), false );

    if ( endian_check == 0x12345678 ) {
	swap_bytes = false;
    } else if ( endian_check == 0x78563412 ) {
	swap_bytes = true;
    } else {
	err_msg = "File is not a valid tfm file";
	goto bail;
    }

    /* Read in texture_width, texture_height, max_ascent, max_descent */
    READ_BYTES( tfm_file, &texture_width, sizeof(int), swap_bytes );
    READ_BYTES( tfm_file, &texture_height, sizeof(int), swap_bytes );
    READ_BYTES( tfm_file, &tfm->max_ascent, sizeof(int), swap_bytes );
    READ_BYTES( tfm_file, &tfm->max_descent, sizeof(int), swap_bytes );

    READ_BYTES( tfm_file, &num_chars, sizeof(int), swap_bytes );

    for (i=0; i<num_chars; i++) {
	tfm_char_data_t *cd;
	double sstep = 0.5/texture_width;
	double tstep = 0.5/texture_height;

	READ_BYTES( tfm_file, &ch_dims.ch, sizeof(unsigned short), swap_bytes );
	READ_BYTES( tfm_file, &ch_dims.w, sizeof(unsigned char), false );
	READ_BYTES( tfm_file, &ch_dims.h, sizeof(unsigned char), false );
	READ_BYTES( tfm_file, &ch_dims.x_offset, sizeof(char), false );
	READ_BYTES( tfm_file, &ch_dims.y_offset, sizeof(char), false );
	READ_BYTES( tfm_file, &ch_dims.kern_width, sizeof(char), false );
	READ_BYTES( tfm_file, &dummy, sizeof(char), false );
	READ_BYTES( tfm_file, &ch_dims.x_pixel, sizeof(short), swap_bytes );
	READ_BYTES( tfm_file, &ch_dims.y_pixel, sizeof(short), swap_bytes );

	if ( ch_dims.ch >= MAX_TEX_FONT_CHARS ) {
	    err_msg = "Two-byte characters are not supported";
	    goto bail;
	}

	cd = ( tfm_char_data_t * ) malloc( sizeof( tfm_char_data_t ) );

	check_assertion( cd != NULL, "out of memory" );

	cd->ll = pp::Vec2d( ch_dims.x_offset, ch_dims.y_offset );
	cd->lr = pp::Vec2d( cd->ll.x + ch_dims.w, cd->ll.y );
	cd->ur = pp::Vec2d( cd->lr.x, cd->lr.y + ch_dims.h );
	cd->ul = pp::Vec2d( cd->ur.x - ch_dims.w, cd->ur.y );

	cd->tex_ll = pp::Vec2d( ch_dims.x_pixel / (double)texture_width + 
				   sstep,
				   ch_dims.y_pixel / (double)texture_height +
				   tstep );
	cd->tex_lr = pp::Vec2d( cd->tex_ll.x + sstep +
				   ch_dims.w / (double)texture_width,
				   cd->tex_ll.y + tstep );
	cd->tex_ur = pp::Vec2d( cd->tex_lr.x + sstep,
				   cd->tex_lr.y + tstep +
				   ch_dims.h / (double)texture_height );
	cd->tex_ul = pp::Vec2d( cd->tex_ur.x + sstep - 
				   ch_dims.w / (double)texture_width,
				   cd->tex_ur.y + tstep );

	cd->kern_width = ch_dims.kern_width;

	tfm->char_data[ch_dims.ch] = cd;
    }

    fclose( tfm_file );

    return tfm;

bail:
    if ( tfm != NULL ) {
	for (i=0; i<MAX_TEX_FONT_CHARS; i++) {
	    if ( tfm->char_data[i] != NULL ) {
		free( tfm->char_data[i] );
	    }
	}
	free( tfm );
    }

    if ( tfm_file != NULL ) {
	fclose( tfm_file );
    }

    print_warning( IMPORTANT_WARNING, 
		   "Error opening font metrics file `%s': %s\n",
		   filename, err_msg );
    return NULL;
}

void delete_tex_font_metrics( tex_font_metrics_t *tfm ) 
{
    int i;
    if ( tfm != NULL ) {
	for (i=0; i<MAX_TEX_FONT_CHARS; i++) {
	    if ( tfm->char_data[i] != NULL ) {
		free( tfm->char_data[i] );
	    }
	}
	free( tfm );
    }
}

static tfm_char_data_t* find_char_data( tex_font_metrics_t *tfm, unsigned char c )
{
    int i;
    if ( tfm->char_data[(int)c] != NULL ) {
		return tfm->char_data[(int)c];
    } else if ( isupper(c) && tfm->char_data[ tolower(c) ] != NULL ) {
		return tfm->char_data[ tolower(c) ];
    } else if ( islower(c) && tfm->char_data[ toupper(c) ] != NULL ) {
		return tfm->char_data[ toupper(c) ];
    } else if ( tfm->char_data[int( ' ') ] != NULL ) {	
		print_warning( IMPORTANT_WARNING, 
		       "Font does not have a representation of "
		       "character `%c'; using space as placeholder", 
		       c );
		return tfm->char_data[ int(' ' )];
    } else {
	for (i=0; i<MAX_TEX_FONT_CHARS; i++) {
	    if ( tfm->char_data[i] != NULL ) {
		print_warning( IMPORTANT_WARNING, 
			       "Font does not have a representation of "
			       "character `%c'; using `%c' as placeholder", 
			       c, i );
		return tfm->char_data[i];
	    }
	}
	check_assertion( 0, "font contains no characters" );
    }
    
    /* Shouldn't get here */
    return NULL;
}

void get_tex_font_string_bbox( tex_font_metrics_t *tfm, 
			       const char *string, 
			       int *width, int *max_ascent, int *max_descent )
{
    int i;
    int len;
    tfm_char_data_t *cd;
    *width = 0;
    
    len = strlen( string );

    for (i=0; i<len; i++) {
		cd = find_char_data( tfm, string[i] );
		*width += int(cd->kern_width);
    }

    *max_ascent = tfm->max_ascent;
    *max_descent = tfm->max_descent;
}

void draw_tex_font_char( tex_font_metrics_t *tfm, const char c )
{
    tfm_char_data_t *cd;

    cd = find_char_data( tfm, c );

    glBegin( GL_QUADS );
    {
	glTexCoord2dv( (double*) &cd->tex_ll );
	glVertex2dv(   (double*) &cd->ll     );
	glTexCoord2dv( (double*) &cd->tex_lr );
	glVertex2dv(   (double*) &cd->lr     );
	glTexCoord2dv( (double*) &cd->tex_ur );
	glVertex2dv(   (double*) &cd->ur     );
	glTexCoord2dv( (double*) &cd->tex_ul );
	glVertex2dv(   (double*) &cd->ul     );
    }
    glEnd();

    glTranslatef( cd->kern_width, 0., 0. );
}

void draw_tex_font_string( tex_font_metrics_t *tfm, const char *string )
{
    int i;
    int len;

    len = strlen( string );

    for (i=0; i<len; i++) {
		draw_tex_font_char( tfm, string[i] );
    }
}

bool is_character_in_tex_font( tex_font_metrics_t *tfm, char c ) 
{
    return (bool) (tfm->char_data[(int)c] != NULL);
}

/* EOF */
