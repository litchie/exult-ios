/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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

#include "imagewin.h"
#include "scale.h"
#include <string.h>

/*
 *	Manipulate from 8-bit to 16-bit pixels.
 */
class Manip8to16
	{
	SDL_Color *colors;		// Palette for source window.
	SDL_PixelFormat *fmt;		// Format of dest. pixels.
public:
	Manip8to16(SDL_Color *c, SDL_PixelFormat *f)
		: colors(c), fmt(f)
		{  }
	unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	void copy(unsigned short& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_source(unsigned char pix, unsigned short& r,
				unsigned short& g, unsigned short& b) const
		{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
		}
	void split_dest(unsigned short pix, unsigned short& r,
				unsigned short& g, unsigned short& b) const
		{
		r = ((pix&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;
		g = ((pix&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;
		b = ((pix&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;
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
	unsigned long rgb(unsigned short r, unsigned short g,
							unsigned short b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	void copy(unsigned long& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split_source(unsigned char pix, unsigned short& r,
				unsigned short& g, unsigned short& b) const
		{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
		}
	void split_dest(unsigned long pix, unsigned short& r,
				unsigned short& g, unsigned short& b) const
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
	static void copy(unsigned short& dest, unsigned short src)
		{ dest = src; }
	static void split_source(unsigned short pix, unsigned short& r,
					unsigned short& g, unsigned short& b)
		{
		r = (pix>>10)&0x1f;
		g = (pix>>5)&0x1f;
		b = pix&0x1f;
		}
	static void split_dest(unsigned short pix, unsigned short& r,
					unsigned short& g, unsigned short& b)
		{ return split_source(pix, r, g, b); }
	static unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b)
		{ return ((r&0x1f)<<10) | ((g&0x1f)<<5) | (b&0x1f); }
	};

/*
 *	Scaling methods:
 */

void Image_window::show_scaled8to16
	(
	)
	{
	Manip8to16 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale2x<unsigned char, unsigned short, Manip8to16>
		(ibuf->get_bits(), ibuf->line_width, ibuf->height, 
		    (unsigned short *) scaled_surface->pixels, 
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(scaled_surface, 0, 0, 2*ibuf->width, 2*ibuf->height);
	}

void Image_window::show_scaled8to32
	(
	)
	{
	Manip8to32 manip(surface->format->palette->colors,
						scaled_surface->format);
	Scale2x<unsigned char, unsigned long, Manip8to32>
		(ibuf->get_bits(), 
			ibuf->line_width, ibuf->height, 
			(unsigned long *) scaled_surface->pixels,
			scaled_surface->pitch/
				scaled_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(scaled_surface, 0, 0, 2*ibuf->width, 2*ibuf->height);
	}
