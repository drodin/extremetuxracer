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

#include "image.h"

//Readers
#include "png_reader.h"
#include "rgb_reader.h"

//Writers
#include "ppm_writer.h"


namespace pp {
	
Image::Image(int width,int height, int depth)
{
	if(width>0 && height>0 && depth>0 && depth <4){ 
		this->width=width;
		this->height=height;
		this->depth=depth;
		this->data=new unsigned char[width*height*depth];
		return;
	}else{
		this->width=0;
		this->height=0;
		this->depth=0;
		this->data=0;
		return;
	}
}

Image::~Image(void)
{
	if(data!=0) delete data;	
}

Image*
Image::readFile(const char* fileName)
{
	int len = strlen(fileName);
	
	// PNG Image?
	if(!strcmp(fileName+len-3,"png")){
		Image* image=new ReaderPNG(fileName);
		
		// check if the image contains valid data
		if(image->data!=NULL){
			return image;		
		} else {
			delete image;
			return NULL;
		}
	}else{
		// try it as rgb (should we check for this first?)
		Image* image=new ReaderRGB(fileName);
		return image;		
	}
}

bool 
Image::writeToFile(const char* fileName) const
{
	WriterPPM writer(fileName,*this);
	
	return true;
}


} //namespace pp
