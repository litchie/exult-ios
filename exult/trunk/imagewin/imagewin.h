/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Imagewin.h - A window to blit images into.
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

#ifndef INCL_IMAGEWIN
#define INCL_IMAGEWIN	1

#include "SDL.h"
#include "imagebuf.h"

/*
 *	Here's the top-level class to use for image buffers.  Image_window
 *	should be derived from it.
 */

class Image_window
	{
protected:
	Image_buffer *ibuf;		// Where the data is actually stored.
	int scale;			// Only 1 or 2 for now.
	bool fullscreen;		// Rendering fullscreen.
	SDL_Surface *surface;		// Represents window in memory.
	SDL_Surface *scaled_surface;	// 2X surface if scaling, else 0.
					// Method to blit scaled:
	void (Image_window::*show_scaled)(int x, int y, int w, int h);
	/*
	 *	Scaled blits:
	 */
					// Scale 8-bits to 16-bits.
	void show_scaled8to16(int x, int y, int w, int h);
	void show_scaled8to555(int x, int y, int w, int h);
	void show_scaled8to565(int x, int y, int w, int h);
					// Scale 8-bits to 32-bits.
	void show_scaled8to32(int x, int y, int w, int h);	
	/*
	 *	Image info.
	 */
					// Create new SDL surface.
	void create_surface(unsigned int w, unsigned int h);
	void free_surface();		// Free it.
public:
					// Create with given buffer.
	Image_window(Image_buffer *ib, int scl = 1, bool fs = false)
		: ibuf(ib), scale(scl), fullscreen(fs), 
		  surface(0), 
		  scaled_surface(0), show_scaled(0)
		{ create_surface(ibuf->width, ibuf->height); }
	virtual ~Image_window();
	int get_scale()			// Returns 1 or 2.
		{ return scale; }
					// Is rect. visible within clip?
	int is_visible(int x, int y, int w, int h)
		{ return ibuf->is_visible(x, y, w, h); }
					// Set title.
	void set_title(const char *title)
		{
		SDL_WM_SetCaption(title, 0);
		}
	Image_buffer *get_ibuf()
		{ return ibuf; }
	int get_width()
		{ return ibuf->width; }
	int get_height()
		{ return ibuf->height; }
	int ready()			// Ready to draw?
		{ return (ibuf->bits != 0); }
	bool is_fullscreen() { return fullscreen; }
					// Create a compatible image buffer.
	Image_buffer *create_buffer(int w, int h);
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int nehh, int newsc);
	void show();			// Repaint entire window.
					// Repaint rectangle.
	void show(int x, int y, int w, int h);

	void toggle_fullscreen();
					// Set palette.
	virtual void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100)
		{  }
					// Rotate palette colors.
	virtual void rotate_colors(int first, int num, int upd)
		{  }
	/*
	 *	16-bit color methods.
	 */
					// Fill with given pixel.
	void fill16(unsigned short pix)
		{ ibuf->fill16(pix); }
					// Fill rect. wth pixel.
	void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty)
		{ ibuf->fill16(pix, srcw, srch, destx, desty); }
					// Copy rectangle into here.
	void copy16(unsigned short *src_pixels,
				int srcw, int srch, int destx, int desty)
		{ ibuf->copy16(src_pixels, srcw, srch, destx, desty); }
					// Copy rect. with transp. color.
	void copy_transparent16(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{ ibuf->copy_transparent16(src_pixels, srcw, srch,
							destx, desty); }
	/*
	 *	8-bit color methods:
	 */
					// Fill with given (8-bit) value.
	void fill8(unsigned char val)
		{ ibuf->fill8(val); }
					// Fill rect. wth pixel.
	void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty)
		{ ibuf->fill8(val, srcw, srch, destx, desty); }
					// Fill line with pixel.
	void fill_line8(unsigned char val, int srcw,
						int destx, int desty)
		{ ibuf->fill_line8(val, srcw, destx, desty); }
					// Copy rectangle into here.
	void copy8(unsigned char *src_pixels,
				int srcw, int srch, int destx, int desty)
		{ ibuf->copy8(src_pixels, srcw, srch, destx, desty); }
					// Copy line to here.
	void copy_line8(unsigned char *src_pixels, int srcw,
						int destx, int desty)
		{ ibuf->copy_line8(src_pixels, srcw, destx, desty); }
					// Copy with translucency table.
	void copy_line_translucent8(
		unsigned char *src_pixels, int srcw,
		int destx, int desty, int first_translucent,
		int last_translucent, Xform_palette *xforms)
		{ ibuf->copy_line_translucent8(src_pixels, srcw, destx, desty,
				first_translucent, last_translucent, xforms); }
					// Apply translucency to a line.
	void fill_line_translucent8(unsigned char val,
			int srcw, int destx, int desty, Xform_palette xform)
		{ ibuf->fill_line_translucent8(val, srcw, destx, desty,
								xform); }
					// Copy rect. with transp. color.
	void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{ ibuf->copy_transparent8(src_pixels, srcw, srch,
							destx, desty); }
	/*
	 *	Depth-independent methods:
	 */
	void clear_clip()		// Reset clip to whole window.
		{ ibuf->clear_clip(); }
					// Set clip.
	void set_clip(int x, int y, int w, int h)
		{ ibuf->set_clip(x, y, w, h); }
					// Copy within itself.
	void copy(int srcx, int srcy, int srcw, int srch, 
						int destx, int desty)
		{ ibuf->copy(srcx, srcy, srcw, srch, destx, desty); }
					// Get rect. into another buf.
	void get(Image_buffer *dest, int srcx, int srcy)
		{ ibuf->get(dest, srcx, srcy); }
					// Put rect. back.
	void put(Image_buffer *src, int destx, int desty)
		{ ibuf->put(src, destx, desty); }

	bool screenshot(SDL_RWops *dst);
	};


#endif	/* INCL_IMAGEWIN	*/
