/**
 **	Imagescl.cc - Methods to blit with scaling.
 **
 **	Written: 6/16/2000 - JSF
 **/

/*
Copyright (C) 2000 Jeffrey S. Freedman

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "imagewin.h"
#include "scale.h"
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "exult_types.h"

/*
 *	Manipulate from 8-bit to 16-bit pixels.
 */
class Manip8to16
	{
protected:
	SDL_Color *colors;		// Palette for source window.
	SDL_PixelFormat *fmt;		// Format of dest. pixels.
public:
	Manip8to16(SDL_Color *c, SDL_PixelFormat *f)
		: colors(c), fmt(f)
		{  }
	uint16 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	void copy(uint16& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_source(unsigned char pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
		{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
		}
	void split_dest(uint16 pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
	{
		r = ((pix&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;
		g = ((pix&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;
		b = ((pix&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;
		}
	};

/*
 *	Manipulate from 8-bit to 16-bit 555 format.
 */
class Manip8to555 : public Manip8to16
	{
public:
	Manip8to555(SDL_Color *c) : Manip8to16(c, 0)
		{  }
	uint16 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{ return ((r>>3)<<10)|((g>>3)<<5)|(b>>3); }
	void copy(uint16& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_dest(uint16 pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
		{
		r = (((pix&0x7c00)>>10)<<3);
		g = (((pix&0x03e0)>>5)<<3);
		b = ((pix&0x001f)<<3);
		}
	};

/*
 *	Manipulate from 8-bit to 16-bit 565 format.
 */
class Manip8to565 : public Manip8to16
	{
public:
	Manip8to565(SDL_Color *c) : Manip8to16(c, 0)
	{  }
	uint16 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{ return ((r>>3)<<11)|((g>>2)<<5)|(b>>3); }
	void copy(uint16& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_dest(uint16 pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
		{
		r = (((pix&0xf800)>>11)<<3);
		g = (((pix&0x07e0)>>5)<<2);
		b = ((pix&0x001f)<<3);
		}
	};

/*
 *	Manipulate from 8-bit to 32-bit pixels.
 */
class Manip8to32
	{
	SDL_Color *colors;		// Source palette.
	SDL_PixelFormat *fmt;		// Format of dest. pixels.
public:
	Manip8to32(SDL_Color *c, SDL_PixelFormat *f)
		: colors(c), fmt(f)
		{  }
	uint32 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	void copy(uint32& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_source(unsigned char pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
		{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
		}
	void split_dest(uint32 pix, unsigned int& r,
				unsigned int& g, unsigned int& b) const
		{
		r = ((pix&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;
		g = ((pix&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;
		b = ((pix&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;
		}
	};

/*
 *	Manipulate from 16-bit to 16-bit pixels (555 bits in each case).
 */
class Manip16to16
	{
public:
	static void copy(uint16& dest, uint16 src)
		{ dest = src; }
	static void split_source(uint16 pix, unsigned int& r,
					unsigned int& g, unsigned int& b)
		{
		r = (pix>>10)&0x1f;
		g = (pix>>5)&0x1f;
		b = pix&0x1f;
		}
	static void split_dest(uint16 pix, unsigned int& r,
					unsigned int& g, unsigned int& b)
		{ split_source(pix, r, g, b); }
	static uint16 rgb(unsigned int r, unsigned int g,
							unsigned int b)
		{ return ((r&0x1f)<<10) | ((g&0x1f)<<5) | (b&0x1f); }
	};


inline void increase_area(int& x, int& y, int& w, int& h,
			int left, int right, int top, int bottom,
			int buf_width, int buf_height)
{
	x -= left; w += left+right;
	y -= top; h += top+bottom;

	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }

	if (x + w > buf_width) w = buf_width - x;
	if (y + h > buf_height) h = buf_height - y;
}

/*
 *	Scaling methods:
 */

//
// 2xSaI Filtering
//
void Image_window::show_scaled8to16_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xSaI<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(surface->format->palette->colors);
	Scale_2xSaI<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(surface->format->palette->colors);
	Scale_2xSaI<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xSaI<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}


//
// Super2xSaI Filtering
//
void Image_window::show_scaled8to16_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(surface->format->palette->colors);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(surface->format->palette->colors);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_Super2xSaI<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}


//
// SuperEagle Filtering
//
void Image_window::show_scaled8to16_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_SuperEagle<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(surface->format->palette->colors);
	Scale_SuperEagle<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(surface->format->palette->colors);
	Scale_SuperEagle<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_SuperEagle<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

//
// 2x Bilinear Filtering
//
void Image_window::show_scaled8to16_bilinear
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xBilinear<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_bilinear
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(surface->format->palette->colors);
	Scale_2xBilinear<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_bilinear
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(surface->format->palette->colors);
	Scale_2xBilinear<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_bilinear
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xBilinear<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

//
// 2x Bilinear Plus Filtering
//
void Image_window::show_scaled8to16_BilinearPlus
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xBilinearPlus<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_BilinearPlus
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(surface->format->palette->colors);
	Scale_2xBilinearPlus<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_BilinearPlus
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(surface->format->palette->colors);
	Scale_2xBilinearPlus<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_BilinearPlus
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale_2xBilinearPlus<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 2*x, 2*y, 2*w, 2*h);
	}

//
// Point Sampling
//
void Image_window::show_scaled_point
	(
	int x, int y, int w, int h	// Area to show.
	)
{
	Scale_point ((unsigned char *)ibuf->get_bits(), x, y, w, h,
				ibuf->line_width, ibuf->height,
				(unsigned char *) surface->pixels, surface->pitch,
				scale);

	SDL_UpdateRect(surface, scale*x, scale*y, scale*w, scale*h);
}

//
// Interlaced Point Sampling
//
void Image_window::show_scaled_interlace
	(
	int x, int y, int w, int h	// Area to show.
	)
{
	Scale_interlace ((unsigned char *)ibuf->get_bits(), x, y, w, h,
				ibuf->line_width, ibuf->height,
				(unsigned char *) surface->pixels, surface->pitch,
				scale);

	SDL_UpdateRect(surface, scale*x, scale*y, scale*w, scale*h);
}

//
// OpenGL 'scaler':
//
void Image_window::show_scaledOpenGL
	(
	int x, int y, int w, int h	// Area to show.
	)
{
	SDL_GL_SwapBuffers();		// Blit.
}
