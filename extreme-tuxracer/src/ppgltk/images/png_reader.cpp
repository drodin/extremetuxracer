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

#include "png_reader.h"

namespace pp {

ReaderPNG::ReaderPNG(const char *fileName)
{
	png_byte header[4]; 
	
	data = NULL; 
	
	FILE *fp = fopen(fileName, "rb");
    if (!fp){
		return;
    }
	
    fread(header, 1, 4, fp);
    bool is_png = !png_sig_cmp(header, 0, 4);
   	if (!is_png)
    {
		fclose(fp);
        return;
	}
		
    png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		
    if (!png_ptr){
		fclose(fp);
        return;
	}

	
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
        return;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
		fclose(fp);
        return;
    }

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 4);
	
	png_read_info(png_ptr, info_ptr);
		
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,int_p_NULL, int_p_NULL);
	
	if(bit_depth == 16)
        png_set_strip_16(png_ptr);
	
	if(color_type == PNG_COLOR_TYPE_PALETTE &&
        bit_depth <= 8)
	{
		png_set_expand(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height,
			   &bit_depth, &color_type, &interlace_type,int_p_NULL, int_p_NULL);
	}
	
	if( color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA ){
		png_set_gray_to_rgb(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height,
			   &bit_depth, &color_type, &interlace_type,int_p_NULL, int_p_NULL);
	}
		
	this->width=width;
	this->height=height;
			
	if ( color_type==PNG_COLOR_TYPE_RGB){
		depth=3;
		loadData(png_ptr, 3);
	} else if ( color_type==PNG_COLOR_TYPE_RGB_ALPHA){
		depth=4;
		loadData(png_ptr, 4);
	} else {
		fflush(stdout);			
		data = NULL;
	}
	
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
}

void
ReaderPNG::loadData(png_structp png_ptr, int depth)
/// loads rgb or rgba into the data buffer 
{
	png_bytep *row_pointers = new png_bytep[height];
	data = new unsigned char[width*height*depth];
		
	for (unsigned int i=0; i<height; i++)
		row_pointers[height-1-i]=data + i*width*depth;
	
	png_read_image(png_ptr, row_pointers);
	delete [] row_pointers;
}

ReaderPNG::~ReaderPNG()
{
	if(data!=NULL) delete(data);
}

} //namespace pp
