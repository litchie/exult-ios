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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "imagewin.h"
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "exult_types.h"
#include "manip.h"
#include "scale_point.h"

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
// Point Sampling Scaler
//
void Scale_point
	(
	const unsigned char *source,	// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,		// Source height.
	unsigned char *dest,		// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	const int factor		// Scale factor
	)
	{
	source += srcy*sline_pixels + srcx;
	dest += srcy*factor*dline_pixels + srcx*factor;

	char data;
	unsigned char *dest2;
	const unsigned char *source2;
	const unsigned char * limit_y = source + srch*sline_pixels;
	const unsigned char * limit_x = source + srcw;

	if (factor == 2)
		{
		uint16 *dest16;
		uint16 *dest16_2;
		uint16 data16;
		
		while (source < limit_y)
			{
			source2 = source;

			dest16 = (uint16*) dest;
			dest += dline_pixels;
			dest16_2 = (uint16*) dest;

			while (source2 < limit_x)
				{
				data16 = *source2++;
				data16 |= data16 << 8;
				*dest16++ = data16;
				*dest16_2++ = data16;
				}
			dest += dline_pixels;
			limit_x += sline_pixels;
			source += sline_pixels;
			}
		}
	else
		{
		const unsigned int y2_pixels = dline_pixels*factor;
		const unsigned char * limit_y2 = dest;
		const unsigned char * limit_x2;

		while (source < limit_y)
			{
			limit_y2 += y2_pixels;
			while (dest < limit_y2)
				{
				limit_x2 = dest2 = dest;
				source2 = source;
				while (source2 < limit_x)
					{
					data = *source2++;
					limit_x2 += factor;
					while (dest2 < limit_x2)
						*dest2++ = data;
					}
				dest += dline_pixels;
				}
			limit_x += sline_pixels;
			source += sline_pixels;
			}
		}
	}
