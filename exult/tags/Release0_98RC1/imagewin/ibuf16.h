/*
 *	ibuf16.h - 16-bit image buffer.
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

#ifndef INCL_IBUF16
#define INCL_IBUF16	1

#include "imagebuf.h"

/*
 *	A 16-bit image buffer:
 */
class Image_buffer16 : public Image_buffer
	{
	unsigned short *palette;	// Palette for 8-bit graphics.
	void create_default_palette();
	unsigned short *get_pixels()	// Cast bits.
		{ return (unsigned short *) bits; }
public:
	Image_buffer16(unsigned int w, unsigned int h, int dpth)
		: Image_buffer(w, h, dpth), palette(0)
		{
		create_default_palette();
		bits = new unsigned char[w*h*2];
		}
	~Image_buffer16()
		{
		delete palette;
		}
					// Create pixel from r,g,b values.
	unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b)
		{ return ((r&0x1f) << 11) + ((g&0x1f) << 6) + (b&0x1f); }
					// Set a palette color.
	void set_palette_color(unsigned char num,
		unsigned short r, unsigned short g, unsigned short b)
		{
		palette[num] = rgb(r, g, b);
		}
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100);
					// Rotate palette colors.
	void rotate_colors(int first, int num, int upd);
	/*
	 *	16-bit color methods.
	 */
					// Fill with given pixel.
	virtual void fill16(unsigned short pix);
					// Fill rect. wth pixel.
	virtual void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty);
					// Fill line with pixel.
	virtual void fill_line16(unsigned short pix, int srcw,
						int destx, int desty);
					// Copy rectangle into here.
	virtual void copy16(unsigned short *src_pixels,
				int srcw, int srch, int destx, int desty);
	/*
	 *	Depth-independent methods:
	 */
	virtual Image_buffer *create_another(int w, int h)
		{ return new Image_buffer16(w, h, depth); }
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
	virtual void fill8(unsigned char val)
		{ Image_buffer16::fill16(palette[val]); }
					// Fill rect. wth pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty)
		{ Image_buffer16::fill16(
				palette[val], srcw, srch, destx, desty); }
					// Fill line with pixel.
	virtual void fill_line8(unsigned char val, int srcw,
						int destx, int desty)
		{ Image_buffer16::fill_line16(palette[val], srcw,
							destx, desty); }
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
		int last_translucent, Xform_palette *xforms)
		{ copy_line8(src_pixels, srcw, destx, desty); }
					// Apply translucency to a line.
	virtual void fill_line_translucent8(unsigned char val,
			int srcw, int destx, int desty, Xform_palette xform)
		{ fill_line8(val, srcw, destx, desty); }
					// Apply translucency to a rectangle
	virtual void fill_translucent8(unsigned char val, int srcw, int srch, 
				int destx, int desty, Xform_palette xform)
		{ fill8(val, srcw, srcw, destx, desty); }
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
	};

#endif
