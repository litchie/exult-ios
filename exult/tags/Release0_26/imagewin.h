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

class Font_face;

					// Table for translating palette vals.:
typedef unsigned char *Xform_palette;	// Should be 256-bytes.

/*
 *	Here's a generic off-screen buffer.  It's up to the derived classes
 *	to set the data.
 */
class Image_buffer_base
	{
protected:
	unsigned int width, height;	// Dimensions (in pixels).
	int depth;			// # bits/pixel.
	int pixel_size;			// # bytes/pixel.
	unsigned char *bits;		// Allocated image buffer.
	unsigned int line_width;	// # words/scan-line.

	int clipx, clipy, clipw, cliph; // Clip rectangle.
					// Clip.  Rets. 0 if nothing to draw.
	int clip(int& srcx, int& srcy, int& srcw, int& srch, 
							int& destx, int& desty)
		{
		if (destx < clipx)
			{
			if ((srcw += (destx - clipx)) <= 0)
				return (0);
			srcx -= (destx - clipx);
			destx = clipx;
			}
		if (desty < clipy)
			{
			if ((srch += (desty - clipy)) <= 0)
				return (0);
			srcy -= (desty - clipy);
			desty = clipy;
			}
		if (destx + srcw > (clipx + clipw))
			if ((srcw = ((clipx + clipw) - destx)) <= 0)
				return (0);
		if (desty + srch > (clipy + cliph))
			if ((srch = ((clipy + cliph) - desty)) <= 0)
				return (0);
		return (1);
		}
	Image_buffer_base(unsigned int w, unsigned int h, int dpth);
public:
	friend class Image_buffer8;
	friend class Image_buffer16;
	virtual ~Image_buffer_base()
		{
		delete bits;		// In case Image_window didn't.
		}
	friend class Image_window;
	friend class Image_buffer;
	unsigned char *get_bits()	// Get ->data.
		{ return bits; }
	void clear_clip()		// Reset clip to whole window.
		{ clipx = clipy = 0; clipw = width; cliph = height; }
					// Set clip.
	void set_clip(int x, int y, int w, int h)
		{
		clipx = x;
		clipy = y;
		clipw = w;
		cliph = h;
		}
	/*
	 *	16-bit color methods.  Default is to ignore them.
	 */
					// Fill with given pixel.
	virtual void fill16(unsigned short pix)
		{  }
					// Fill rect. wth pixel.
	virtual void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty)
		{  }
					// Copy rectangle into here.
	virtual void copy16(unsigned short *src_pixels,
				int srcw, int srch, int destx, int desty)
		{  }
					// Copy rect. with transp. color.
	virtual void copy_transparent16(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{  }
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(SDL_Surface *surface, 
		unsigned char *rgbs, int maxval, int brightness = 100) = 0;
					// Rotate palette colors.
	virtual void rotate_colors(SDL_Surface *surface,
					int first, int num, int upd) = 0;
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val) = 0;
					// Fill rect. with pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty) = 0;
					// Fill line with pixel.
	virtual void fill_line8(unsigned char val, int srcw,
						int destx, int desty) = 0;
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels,
				int srcw, int srch, int destx, int desty) = 0;
					// Copy line to here.
	virtual void copy_line8(unsigned char *src_pixels, int srcw,
						int destx, int desty) = 0;
					// Copy with translucency table.
	virtual void copy_line_translucent8(
		unsigned char *src_pixels, int srcw,
		int destx, int desty, int first_translucent,
		int last_translucent, Xform_palette *xforms) = 0;
					// Apply translucency to a line.
	virtual void fill_line_translucent8(unsigned char val,
		int srcw, int destx, int desty, Xform_palette xform) = 0;
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty) = 0;
	/*
	 *	Depth-independent methods:
	 */
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
						int destx, int desty) = 0;
					// Get rect. into another buf.
	virtual void get(Image_buffer *dest, int srcx, int srcy) = 0;
					// Put rect. back.
	virtual void put(Image_buffer *src, int destx, int desty) = 0;
					// Open a (truetype) font file.
	static Font_face *open_font(char *fname, int points);
	static void close_font(Font_face *font);// Close font.
#if 0
					// Draw text in given rectangle.
	void draw_text_box(Font_face *font, char *text,
						int x, int y, int w, int h);
					// Draw text at given point.
	int draw_text(Font_face *font, char *text, int x, int y);
	int draw_text(Font_face *font, char *text, int textlen, int x, int y);
					// Draw a char. at given point.
	int draw_char(Font_face *font, short chr, int x, int y);
					// Get width of text.
	static int get_text_width(Font_face *font, char *text);
	static int get_text_width(Font_face *font, char *text, int textlen);
					// Get height.
	static int get_text_height(Font_face *font);
#endif
	};

/*
 *	An 8-bit image buffer:
 */
class Image_buffer8 : public Image_buffer_base
	{
	SDL_Color colors[256];		// Palette.
					// Private ctor. for Image_buffer.
	Image_buffer8(unsigned int w, unsigned int h, Image_buffer *)
		: Image_buffer_base(w, h, 8)
		{  }
public:
	Image_buffer8(unsigned int w, unsigned int h)
		: Image_buffer_base(w, h, 8)
		{
		bits = new unsigned char[w*h];
		}
	friend class Image_buffer;
	/*
	 *	Depth-independent methods:
	 */
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
							int destx, int desty);
					// Get rect. into another buf.
	virtual void get(Image_buffer *dest, int srcx, int srcy);
					// Put rect. back.
	virtual void put(Image_buffer *src, int destx, int desty);
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(SDL_Surface *surface, 
		unsigned char *rgbs, int maxval, int brightness = 100);
					// Rotate palette colors.
	virtual void rotate_colors(SDL_Surface *surface, int first, int num,
								int upd);
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
			int srcw, int destx, int desty, Xform_palette xform);
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
	};

/*
 *	A 16-bit image buffer:
 */
class Image_buffer16 : public Image_buffer_base
	{
	unsigned short *palette;	// Palette for 8-bit graphics.
	void create_default_palette();
	unsigned short *get_pixels()	// Cast bits.
		{ return (unsigned short *) bits; }
public:
	Image_buffer16(unsigned int w, unsigned int h, int dpth)
		: Image_buffer_base(w, h, dpth), palette(0)
		{
		create_default_palette();
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
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
							int destx, int desty);
					// Get rect. into another buf.
	virtual void get(Image_buffer *dest, int srcx, int srcy);
					// Put rect. back.
	virtual void put(Image_buffer *src, int destx, int desty);
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(SDL_Surface *surface, 
		unsigned char *rgbs, int maxval, int brightness = 100);
					// Rotate palette colors.
	virtual void rotate_colors(SDL_Surface *surface, int first, int num,
								int upd);
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
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
	};

/*
 *	Here's the top-level class to use for image buffers.  Image_window
 *	should be derived from it.
 */
class Image_buffer
	{
protected:
	Image_buffer_base *ibuf;	// Where the data is actually stored.
public:
	Image_buffer(unsigned int w, unsigned int h, int dpth);
	~Image_buffer()
		{ delete ibuf; }
	Image_buffer_base *get_ibuf()
		{ return ibuf; }
	int get_width()
		{ return ibuf->width; }
	int get_height()
		{ return ibuf->height; }
					// Create a compatible image buffer.
	Image_buffer *create_buffer(int w, int h);
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
#if 0
					// Open a (truetype) font file.
	static Font_face *open_font(char *fname, int points)
		{ return Image_buffer_base::open_font(fname, points); }
					// Close font.
	static void close_font(Font_face *font)
		{ Image_buffer_base::close_font(font); }
					// Draw text in given rectangle.
	void draw_text_box(Font_face *font, char *text,
						int x, int y, int w, int h)
		{ ibuf->draw_text_box(font, text, x, y, w, h); }
					// Draw text at given point.
	int draw_text(Font_face *font, char *text, int x, int y)
		{ return ibuf->draw_text(font, text, x, y); }
	int draw_text(Font_face *font, char *text, int textlen, int x, int y)
		{ return ibuf->draw_text(font, text, textlen, x, y); }
					// Draw a char. at given point.
	int draw_char(Font_face *font, short chr, int x, int y)
		{ return ibuf->draw_char(font, chr, x, y); }
					// Get width of text.
	static int get_text_width(Font_face *font, char *text)
		{ return Image_buffer_base::get_text_width(font, text); }
	static int get_text_width(Font_face *font, char *text, int textlen)
		{ return Image_buffer_base::get_text_width(
							font, text, textlen); }
					// Get height.
	static int get_text_height(Font_face *font)
		{ return Image_buffer_base::get_text_height(font); }
#endif
	};

/*
 *	Here's a window for showing and manipulating images using SDL.
 */
class Image_window : public Image_buffer
	{
	SDL_Surface *surface;		// Represents window in memory.
	/*
	 *	Image info.
	 */
					// Create new SDL surface.
	void create_surface(unsigned int w, unsigned int h);
	void free_surface();		// Free it.
public:
	Image_window(unsigned int w, unsigned int h);
	virtual ~Image_window();
	unsigned int get_width()	// Get dims.
		{ return ibuf->width; }
	unsigned int get_height()
		{ return ibuf->height; }
	int ready()			// Ready to draw?
		{ return (ibuf->bits != 0); }
					// Set title.
	void set_title(char *title)
		{
		SDL_WM_SetCaption(title, 0);
		}
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100)
		{ ibuf->set_palette(surface, rgbs, maxval, brightness); }
					// Rotate palette colors.
	virtual void rotate_colors(int first, int num, int upd)
		{ ibuf->rotate_colors(surface, first, num, upd); }
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int nehh);
	void show();			// Repaint entire window.
	};

#endif	/* INCL_IMAGEWIN	*/
