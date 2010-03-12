/*
 *	Copyright (C) 2009 Exult Team
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Library General Public
 *	License as published by the Free Software Foundation; either
 *	version 2 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	Library General Public License for more details.
 *
 *	You should have received a copy of the GNU Library General Public
 *	License along with this library; if not, write to the
 *	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *	Boston, MA  02111-1307, USA.
 */

#ifndef INCL_MANIP_H
#define INCL_MANIP_H	1

#include "SDL_video.h"

/*
 *	Identity manipulation. This is for the 8-bit scalers ONLY.
 */
class Manip8to8
	{
public:
	Manip8to8(SDL_Color *, SDL_PixelFormat *)
		{  }
	void copy(uint8& dest, unsigned char src) const
		{ dest = src; }
	uint8 copy(unsigned char src) const
		{ return src; }
	};

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
	SDL_Color *get_colors() const { return colors; }
	uint16 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	uint16 copy(unsigned char src) const
		{
		SDL_Color& color = colors[src];
		return rgb(color.r, color.g, color.b);
		}
	void copy(uint16& dest, unsigned char src) const
		{ dest = copy(src); }
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
	uint16 copy(unsigned char src) const
		{
		SDL_Color& color = colors[src];
		return rgb(color.r, color.g, color.b);
		}
	void copy(uint16& dest, unsigned char src) const
		{ dest = copy(src); }
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
	uint16 copy(unsigned char src) const
		{
		SDL_Color& color = colors[src];
		return rgb(color.r, color.g, color.b);
		}
	void copy(uint16& dest, unsigned char src) const
		{ dest = copy(src); }
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
	SDL_Color *get_colors() const { return colors; }
	uint32 rgb(unsigned int r, unsigned int g,
							unsigned int b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	uint32 copy(unsigned char src) const
		{
		SDL_Color& color = colors[src];
		return rgb(color.r, color.g, color.b);
		}
	void copy(uint32& dest, unsigned char src) const
		{ dest = copy(src); }
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
	uint16 copy(uint16 src) const
		{ return src; }
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

#endif
