/*
 *	ibuf8.h - 8-bit image buffer.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef INCL_IBUF8
#define INCL_IBUF8	1

#include "imagebuf.h"

/*
 *	An 8-bit image buffer:
 */
class Image_buffer8 : public Image_buffer
	{
					// Private ctor. for Image_window8.
	Image_buffer8(unsigned int w, unsigned int h, Image_buffer *)
		: Image_buffer(w, h, 8)
		{  }
public:
	Image_buffer8(unsigned int w, unsigned int h)
		: Image_buffer(w, h, 8)
		{ bits = new unsigned char[w*h]; }
	friend class Image_window8;
	/*
	 *	Depth-independent methods:
	 */
	virtual Image_buffer *create_another(int w, int h)
		{ return new Image_buffer8(w, h); }
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
							int destx, int desty);
					// Get rect. into another buf.
	virtual void get(Image_buffer *dest, int srcx, int srcy);
					// Put rect. back.
	virtual void put(Image_buffer *src, int destx, int desty);
	
	virtual void fill_static(int black, int gray, int white);

	/*
	 *	8-bit color methods:
	 */
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val);
					// Fill rect. wth pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty);
					// Fill line with pixel.
	virtual void fill_line8(unsigned char val, int srcw,
						int destx, int desty);
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels,
				int srcw, int srch, int destx, int desty);
					// Copy line to here.
	virtual void copy_line8(unsigned char *src_pixels, int srcw,
						int destx, int desty);
					// Copy with translucency table.
	virtual void copy_line_translucent8(
		unsigned char *src_pixels, int srcw,
		int destx, int desty, int first_translucent,
		int last_translucent, Xform_palette *xforms);
					// Apply translucency to a line.
	virtual void fill_line_translucent8(unsigned char val,
			int srcw, int destx, int desty, Xform_palette& xform);
					// Apply translucency to a rectangle
	virtual void fill_translucent8(unsigned char val, int srcw, int srch, 
				int destx, int desty, Xform_palette& xform);
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
					// Get/put a single pixel.
	unsigned char get_pixel8(int x, int y)
		{ return bits[y*line_width + x]; }
	void put_pixel8(unsigned char pix, int x, int y)
		{ 
		if (x >= clipx && x < clipx + clipw &&
		    y >= clipy && y < clipy + cliph)
			bits[y*line_width + x] = pix;
		}

	void paint_rle (int xoff, int yoff, unsigned char *in);
					// Convert to 32-bit rgba.
	unsigned char *rgba(unsigned char *pal, unsigned char transp, 
		int first_translucent = 256,
		int last_translucent = 256, Xform_palette *xforms = 0);
	};

#endif
