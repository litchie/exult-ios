/**
 **	Iwin8.h - 8-bit image window.
 **
 **	Written: 8/13/98 - JSF
 **/

/*
Copyright (C) 1998 Jeffrey S. Freedman

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/

#ifndef INCL_IWIN8
#define INCL_IWIN8	1

#include "imagewin.h"
#include "ibuf8.h"

template <class T> class GammaTable;


/*
 *	Here's an 8-bit color-depth window (faster than the generic).
 */
class Image_window8 : public Image_window
	{
	unsigned char colors[768];	// Palette.
	Image_buffer8 *ib8;		// Cast to 8-bit buffer.

	static GammaTable<unsigned char>	GammaRed;
	static GammaTable<unsigned char>	GammaGreen;
	static GammaTable<unsigned char>	GammaBlue;
public:
	Image_window8(unsigned int w, unsigned int h, int scl = 1, 
							bool fs = false, int sclr = point);
	~Image_window8();

	Image_buffer8 *get_ib8() const
		{ return ib8; }
					// Set palette.
	virtual void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100);
					// Rotate palette colors.
	virtual void rotate_colors(int first, int num, int upd);
	/*
	 *	8-bit color methods:
	 */
					// Fill with given (8-bit) value.
	void fill8(unsigned char val)
		{ ib8->Image_buffer8::fill8(val); }
					// Fill rect. wth pixel.
	void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty)
		{ IF_OPENGL(opengl_fill8(val, srcw, srch, destx, desty),
		  ib8->Image_buffer8::fill8(val, srcw, srch, destx, desty)); }
					// Fill line with pixel.
	void fill_line8(unsigned char val, int srcw,
						int destx, int desty)
		{ ib8->Image_buffer8::fill_line8(val, srcw, destx, desty); }
					// Copy rectangle into here.
	void copy8(unsigned char *src_pixels,
				int srcw, int srch, int destx, int desty)
		{ ib8->Image_buffer8::copy8(src_pixels, srcw, srch, 
							destx, desty); }
					// Copy line to here.
	void copy_line8(unsigned char *src_pixels, int srcw,
						int destx, int desty)
		{ ib8->Image_buffer8::copy_line8(src_pixels, srcw, 
							destx, desty); }
					// Copy with translucency table.
	void copy_line_translucent8(
		unsigned char *src_pixels, int srcw,
		int destx, int desty, int first_translucent,
		int last_translucent, Xform_palette *xforms)
		{ ib8->Image_buffer8::copy_line_translucent8(src_pixels, srcw,
			destx, desty,
				first_translucent, last_translucent, xforms); }
					// Apply translucency to a line.
	void fill_line_translucent8(unsigned char val,
			int srcw, int destx, int desty, Xform_palette xform)
		{ ib8->Image_buffer8::fill_line_translucent8(val, 
					srcw, destx, desty, xform); }
					// Copy rect. with transp. color.
	void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{ ib8->Image_buffer8::copy_transparent8(src_pixels, srcw, srch,
							destx, desty); }
					// Get/put a single pixel.
	unsigned char get_pixel8(int x, int y)
		{ return ib8->Image_buffer8::get_pixel8(x, y); }
	void put_pixel8(unsigned char pix, int x, int y)
		{ ib8->Image_buffer8::put_pixel8(pix, x, y); }

	static void get_gamma (float &r, float &g, float &b);
	static void set_gamma (float r, float g, float b);

	unsigned char* mini_screenshot();
	};

#endif
