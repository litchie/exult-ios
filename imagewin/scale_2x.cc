/*
 * Scale2X algorithm by Andrea Mazzoleni.
 *
 * Copyright (C) 2001-2002 Andrea Mazzoleni
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
 *	You should have received a copy of the GNU General Public
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
#include "scale_2x.h"

//
// Scale2x (no blurring) by Andrea Mazzoleni.
//
void Image_window::show_scale2x_noblur
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	Scale2x_noblur ((unsigned char *)ibuf->get_bits(), x, y, w, h,
				ibuf->line_width, ibuf->height,
				(unsigned char *) surface->pixels, 
				surface->pitch
				);

	SDL_UpdateRect(surface, scale*x, scale*y, scale*w, scale*h);
	}

void Scale2x_noblur
	(
	const unsigned char *src,	// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,		// Source height.
	unsigned char *dest,		// ->dest pixels.
	const int dline_pixels		// Pixels (words)/line for dest.
	)
	{
	dest += srcy*2*dline_pixels + srcx*2;
	unsigned char *dest0 = dest, *dest1 = dest + dline_pixels;
					// ->current row.
	const unsigned char *src1 = src + srcy*sline_pixels + srcx;
	const unsigned char *src0 = src1 - sline_pixels;	// ->prev. row.
	const unsigned char *src2 = src1 + sline_pixels;	// ->next row.
	const unsigned char * limit_y = src1 + srch*sline_pixels;
	const unsigned char * limit_x = src1 + srcw;
					// Very end of source surface:
	const unsigned char * end_src = src + sheight*sline_pixels;

	if (src0 < src)
		src0 = src1;		// Don't go before row 0.
	if (srcx + srcw == sline_pixels)	// Going to right edge?
		limit_x--;		// Stop 1 pixel before it.
	while (src1 < limit_y)
		{
		if (src2 > end_src)
			src2 = src1;	// On last row.
		if (srcx == 0)		// First pixel.
			{
			dest0[0] = dest1[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
					// Middle pixels.
		while (src1 < limit_x)
			{
			if (src1[-1] == src0[0] && src2[0] != src0[0] &&
			    src1[1] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0] && 
			    src1[-1] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0] && 
			    src1[1] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0] && 
			    src1[-1] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		if (srcx + srcw == sline_pixels)
			{		// End pixel in row.
			if (src1[-1] == src0[0] && src2[0] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			dest0[1] = src1[0];
			dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		src0 += sline_pixels - srcw;
		src1 += sline_pixels - srcw;
		src2 += sline_pixels - srcw;
		dest1 += dline_pixels - 2*srcw;
		if (src0 == src1)	// End of first row?
			src0 -= sline_pixels;
		limit_x += sline_pixels;
		dest0 = dest1;
		dest1 += dline_pixels;
		}
	}
