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

#ifndef _RGB_READER_H
#define _RGB_READER_H

#include "image.h"

#include <stdio.h>
#include <stdlib.h>

namespace pp {
	
class ReaderRGB : public Image
{
	typedef struct  
	{
		unsigned short imagic;
		unsigned short type;
		unsigned short dim;
		unsigned short sizeX, sizeY, sizeZ;
		unsigned long min, max;
		unsigned long wasteBytes;
		char name[80];
		unsigned long colorMap;
		FILE *file;
		unsigned char *tmp[5];
		unsigned long rleEnd;
		unsigned int *rowStart;
		unsigned int *rowSize;
	}ImageInfo;

	ReaderRGB::ImageInfo* ImageOpen(const char *fileName);
	void ImageGetRawData( ReaderRGB::ImageInfo *image, unsigned char *data);
	void ImageGetRow( ReaderRGB::ImageInfo *image, unsigned char *buf, int y, int z);
	void ImageClose( ReaderRGB::ImageInfo *image);
	
public:
	
	ReaderRGB(const char* fileName);
	~ReaderRGB();
};


} //namespace pp

#endif // RGB_READER_H
