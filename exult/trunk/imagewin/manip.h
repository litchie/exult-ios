/*
*	Copyright (C) 2009-2013 Exult Team
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
#include <cstdlib>
#include <cstring>

class color_8
{
public:
	typedef uint8 T;
};

class color_16
{
public:
	typedef uint16 T;
};

class color_555 : public color_16
{
};

class color_565 : public color_16
{
};

class color_32
{
public:
	typedef uint32 T;
};

class ManipBase
{
protected:
	static SDL_PixelFormat *fmt;		// Format of dest. pixels (and src for rgb src).
	static SDL_Color colors[256];		// Palette for source window.
	ManipBase(SDL_Color *c, SDL_PixelFormat *f)
	{
		fmt = f;
		if (c) std::memcpy(colors,c,sizeof(colors)); 
	}
public:
	static SDL_Color *get_colors() { return colors; }
};

// Generic RGB dest
template<typename color_d> class ManipBaseDest : public ManipBase
{
protected:
	ManipBaseDest (SDL_Color *c, SDL_PixelFormat *f) : ManipBase(c,f) { }
public:
	typedef typename color_d::T uintD;

	static uintD rgb(unsigned int r, unsigned int g,
		unsigned int b)
	{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
			((g>>fmt->Gloss)<<fmt->Gshift) |
			((b>>fmt->Bloss)<<fmt->Bshift);
	}
	static void split_dest(uintD pix, unsigned int& r,
		unsigned int& g, unsigned int& b) 	
	{
		r = ((pix&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;
		g = ((pix&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;
		b = ((pix&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;
	}
};

// 555 RGB dest
template<> class ManipBaseDest<color_555> : public ManipBase
{
protected:
	ManipBaseDest (SDL_Color *c, SDL_PixelFormat *f) : ManipBase(0,f) 
	{ 
		if (c)
		{
			if (fmt->Rmask == 0x7c00 && fmt->Gmask == 0x03e0 && fmt->Bmask == 0x001f)
				std::memcpy(colors,c,sizeof(colors));
			else
			{
				SDL_Color *dst = colors;
				SDL_Color * const end = dst+256;
				while (dst != end) {
					dst->b = c->r;
					dst->g = c->g;
					dst->r = c->b;
					++dst;
					++c;
				}
			}
		}
	}
public:
	typedef color_555::T uintD;

	static uintD rgb(unsigned int r, unsigned int g,
		unsigned int b)
	{
		return ((r>>3)<<10) | ((g>>3)<<5) | ((b>>3));
	}
	static void split_dest(uintD pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		r = (pix>>10)<<3;
		g = (pix>>5)<<3;
		b = (pix)<<3;
	}
};

// 565 RGB dest
template<> class ManipBaseDest<color_565> : public ManipBase
{
protected:
	ManipBaseDest (SDL_Color *c, SDL_PixelFormat *f) : ManipBase(0,f) 
	{ 
		if (c)
		{
			if (fmt->Rmask == 0xf800 && fmt->Gmask == 0x7e0 && fmt->Bmask == 0x1f)
				std::memcpy(colors,c,sizeof(colors));
			else
			{
				SDL_Color *dst = colors;
				SDL_Color * const end = dst+256;
				while (dst != end) {
					dst->b = c->r;
					dst->g = c->g;
					dst->r = c->b;
					++dst;
					++c;
				}
			}
		}
	}
public:
	typedef color_565::T uintD;

	static uintD rgb(unsigned int r, unsigned int g,
		unsigned int b)
	{
		return ((r>>3)<<11) | ((g>>2)<<5) | ((b>>3));
	}
	static void split_dest(uintD pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		r = (pix>>11)<<3;
		g = (pix>>5)<<2;
		b = (pix)<<3;
	}
};

// Paletted dest
template<> class ManipBaseDest<uint8> : public ManipBase
{
protected:
	ManipBaseDest (SDL_Color *c, SDL_PixelFormat *f) :  ManipBase(c,f) { }
public:
	typedef color_8::T uintD;

	static int get_error(int r, int g, int b, int index) {
		SDL_Color& color = colors[index];
		return std::abs(r-color.r) + std::abs(g-color.g) + std::abs(b-color.b);
	}

	// This is god awful slow
	static uintD rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		uintD best = 0;
		int best_error = get_error(r,g,b,0);
		for (int i = 0; i < 256; i++) {
			int error = get_error(r,g,b,i);
			if (error < best_error) {
				best = i;
				best_error = error;
			}
		}
		return best;
	}
	static void split_dest(uintD pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
	}
};

// Generic RGB Src
template<typename color_s, typename color_d> class ManipBaseSrc : public ManipBaseDest<color_d>
{
protected:
	ManipBaseSrc(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseDest<color_d>(c,f) {  }
public:
	typedef typename color_s::T uintS;

	static void split_source(uintS pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		r = ((pix&ManipBase::fmt->Rmask)>>ManipBase::fmt->Rshift)<<ManipBase::fmt->Rloss;
		g = ((pix&ManipBase::fmt->Gmask)>>ManipBase::fmt->Gshift)<<ManipBase::fmt->Gloss;
		b = ((pix&ManipBase::fmt->Bmask)>>ManipBase::fmt->Bshift)<<ManipBase::fmt->Bloss;
	}
	static void split_source(uintS pix, uint8& r,
		uint8& g, uint8& b)
	{
		r = ((pix&ManipBase::fmt->Rmask)>>ManipBase::fmt->Rshift)<<ManipBase::fmt->Rloss;
		g = ((pix&ManipBase::fmt->Gmask)>>ManipBase::fmt->Gshift)<<ManipBase::fmt->Gloss;
		b = ((pix&ManipBase::fmt->Bmask)>>ManipBase::fmt->Bshift)<<ManipBase::fmt->Bloss;
	}
};

// 555 RGB Src
template<typename color_d> class ManipBaseSrc<color_555,color_d> : public ManipBaseDest<color_d>
{
protected:
	ManipBaseSrc(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseDest<color_d>(c,f) {  }
public:
	typedef typename color_555::T uintS;

	static void split_source(uintS pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		r = (pix>>10)<<3;
		g = (pix>>5)<<3;
		b = (pix)<<3;
	}

	static void split_source(uintS pix, uint8& r,
		uint8& g, uint8& b)
	{
		r = (pix>>10)<<3;
		g = (pix>>5)<<3;
		b = (pix)<<3;
	}
};

// 565 RGB Src
template<typename color_d> class ManipBaseSrc<color_565,color_d> : public ManipBaseDest<color_d>
{
protected:
	ManipBaseSrc(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseDest<color_d>(c,f) {  }
public:
	typedef typename color_565::T uintS;

	static void split_source(uintS pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		r = (pix>>11)<<3;
		g = (pix>>5)<<2;
		b = (pix)<<3;
	}
	static void split_source(uintS pix, uint8& r,
		uint8& g, uint8& b)
	{
		r = (pix>>11)<<3;
		g = (pix>>5)<<2;
		b = (pix)<<3;
	}
};

// Paletted Src
template<typename color_d> class ManipBaseSrc<color_8,color_d> : public ManipBaseDest<color_d>
{
protected:
	ManipBaseSrc(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseDest<color_d>(c,f) {  }
public:
	typedef typename color_8::T uintS;

	static void split_source(uintS pix, unsigned int& r,
		unsigned int& g, unsigned int& b)
	{
		SDL_Color& color = ManipBase::colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
	}

	static void split_source(uintS pix, uint8& r,
		uint8& g, uint8& b)
	{
		SDL_Color& color = ManipBase::colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
	}
};

// Src and dest are different
template<typename color_s, typename color_d> class ManipStoD : public ManipBaseSrc<color_s,color_d>
{
public:
	ManipStoD(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseSrc<color_s,color_d>(c,f) {  }

	typedef typename color_s::T uintS;
	typedef typename color_d::T uintD;

	static uintD copy(uintS src)
	{
		unsigned int r, g, b;
		ManipBaseSrc<color_s,color_d>::split_source(src,r,g,b);
		return ManipBaseDest<color_d>::rgb(r,g,b);
	}
	static void copy(uintD& dest, uintS src)
	{ dest = copy(src); }
	template <unsigned int N, unsigned int M>
	static inline void blend(uintD& dest, unsigned int rs, unsigned int gs, unsigned int bs)
	{
		unsigned int rd, gd, bd;
		ManipBaseDest<color_d>::split_dest(dest, rd, gd, bd);
		dest = ManipBaseDest<color_d>::rgb((rs * N + rd * (M - N)) / M,
		                                   (gs * N + gd * (M - N)) / M,
		                                   (bs * N + bd * (M - N)) / M);
	}
};

// Src and dest are the same
template<typename colorSD> class ManipStoD<colorSD,colorSD> : public ManipBaseSrc<colorSD,colorSD>
{
public:
	ManipStoD(SDL_Color *c, SDL_PixelFormat *f) : ManipBaseSrc<colorSD,colorSD>(c,f) {  }

	typedef typename colorSD::T uintS;
	typedef typename colorSD::T uintD;

	static uintD copy(uintS src)
	{ return src; }
	static void copy(uintD& dest, uintS src)
	{ dest = src; }
};

typedef ManipStoD<color_8,color_8> Manip8to8;
typedef ManipStoD<color_8,color_16> Manip8to16;
typedef ManipStoD<color_8,color_555> Manip8to555;
typedef ManipStoD<color_8,color_565> Manip8to565;
typedef ManipStoD<color_8,color_32> Manip8to32;

typedef ManipStoD<color_16,color_16> Manip16to16;
typedef ManipStoD<color_555,color_555> Manip555to555;
typedef ManipStoD<color_565,color_565> Manip565to565;
typedef ManipStoD<color_32,color_32> Manip32to32;

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
