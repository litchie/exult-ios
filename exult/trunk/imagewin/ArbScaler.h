/*
 *  Copyright (C) 2004  Ryan Nunn and The Pentagram Team
*   Copyright (C) 2010 The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SCALER_H_INCLUDED
#define SCALER_H_INCLUDED

#include "imagewin.h"
#include "manip.h"
#include <SDL.h>

namespace Pentagram {

/// Base Scaler class
class ArbScaler
{
protected:

	// Basic scaler function template
	typedef bool (*ScalerFunc) ( SDL_Surface *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
					uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src);

	//
	// Basic scaler functions (filled in by the scalers constructor)
	//
	ScalerFunc	Scale8To8;
	ScalerFunc	Scale8To555;
	ScalerFunc	Scale8To565;
	ScalerFunc	Scale8To16;
	ScalerFunc	Scale8To32;

	ScalerFunc	Scale555To555;
	ScalerFunc	Scale565To565;
	ScalerFunc	Scale16To16;
	ScalerFunc	Scale32To32;

public:
	ArbScaler() : Scale8To8(0), Scale8To555(0), Scale8To565(0), Scale8To16(0), Scale8To32(0),
		Scale555To555(0), Scale565To565(0), Scale16To16(0), Scale32To32(0)
	{ }

	virtual const char *	ScalerName() const = 0;			//< Name Of the Scaler (1 word)
	virtual const char *	ScalerDesc() const = 0;			//< Desciption of the Scaler
	virtual const char *	ScalerCopyright() const = 0;	//< Scaler Copyright info

	bool Support8bpp(int srcfmt) {
		return (srcfmt == 8 && Scale8To8);
	}

	bool Support16bpp(int srcfmt) {
		return (srcfmt == 8 && Scale8To16) || (srcfmt == 16 && Scale16To16);
	}

	bool Support32bpp(int srcfmt) {
		return (srcfmt == 8 && Scale8To32) || (srcfmt == 32 && Scale32To32);
	}

	// Call this to scale a section of the screen
	inline bool Scale(	SDL_Surface *texture, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						SDL_Surface *dest, sint32 dx, sint32 dy, sint32 dw, sint32 dh, bool clamp_src) const
	{
		uint8* pixel = (uint8*)dest->pixels + dx*dest->format->BytesPerPixel + dy*dest->pitch;
		sint32 pitch = dest->pitch;

		SDL_PixelFormat *src_format = texture->format;
		SDL_PixelFormat *dst_format = dest->format;

		if (dst_format->BytesPerPixel == 2)
		{
			int r = dst_format->Rmask;
			int g = dst_format->Gmask;
			int b = dst_format->Bmask;

			if ((r == 0xf800 && g == 0x7e0 && b == 0x1f) || (b == 0xf800 && g == 0x7e0 && r == 0x1f))
			{
				if (src_format->BitsPerPixel == 8 && Scale8To565)
				{
					Manip8to565 manip(src_format->palette->colors, dst_format);
					return Scale8To565(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
				}
				else if (src_format->BytesPerPixel == 2 && Scale565To565)
				{
					Manip565to565 manip(0, dst_format);
					return Scale565To565(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
				}
			}
			else if ((r == 0x7c00 && g == 0x3e0 && b == 0x1f) || (b == 0x7c00 && g == 0x3e0 && r == 0x1f))
			{
				if (src_format->BitsPerPixel == 8 && Scale8To555)
				{
					Manip8to555 manip(src_format->palette->colors, dst_format);
					return Scale8To555(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
				}
				else if (src_format->BytesPerPixel == 2 && Scale555To555)
				{
					Manip555to555 manip(0, dst_format);
					return Scale555To555(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
				}
			}

			if (src_format->BitsPerPixel == 8 && Scale8To16)
			{
				Manip8to16 manip(src_format->palette->colors, dst_format);
				return Scale8To16(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
			}
			else if (src_format->BytesPerPixel == 2 && Scale16To16)
			{
				Manip16to16 manip(0, dst_format);
				return Scale16To16(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
			}
		}
		else if (dst_format->BitsPerPixel == 32)
		{
			if (src_format->BitsPerPixel == 8 && Scale8To32)
			{
				Manip8to32 manip(src_format->palette->colors, dst_format);
				return Scale8To32(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
			}
			else if (src_format->BytesPerPixel == 4 && Scale32To32)
			{
				Manip32to32 manip(0, dst_format);
				return Scale32To32(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
			}
		}
		else if (dst_format->BitsPerPixel == 8)
		{
			if (src_format->BitsPerPixel == 8 && Scale8To8)
			{
				Manip8to8 manip(0, dst_format);
				return Scale8To8(texture, sx, sy, sw, sh,  pixel, dw, dh, pitch, clamp_src);
			}
		}		

		return false;
	}

	virtual ~ArbScaler() { }
};

}

#endif
