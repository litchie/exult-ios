/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Ibuf8.cc - 8-bit image buffer.
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

#include "ibuf8.h"
#include <string>

/*
 *	Copy an area of the image within itself.
 */

void Image_buffer8::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	unsigned char *to = bits + yto*line_width + destx;
	unsigned char *from = bits + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		std::memmove((char *) to, (char *) from, srcw);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Get a rectangle from here into another Image_buffer.
 */

void Image_buffer8::get
	(
	Image_buffer *dest,		// Copy to here.
	int srcx, int srcy		// Upper-left corner of source rect.
	)
	{
	int srcw = dest->width, srch = dest->height;
	int destx = 0, desty = 0;
					// Constrain to window's space. (Note
					//   convoluted use of clip().)
	if (!clip(destx, desty, srcw, srch, srcx, srcy))
		return;
	unsigned char *to = (unsigned char *) dest->bits + 
				desty*dest->line_width + destx;
	unsigned char *from = (unsigned char *) bits + srcy*line_width + srcx;
					// Figure # pixels to next line.
	int to_next = dest->line_width - srcw;
	int from_next = line_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Retrieve data from another buffer.
 */

void Image_buffer8::put
	(
	Image_buffer *src,		// Copy from here.
	int destx, int desty		// Copy to here.
	)
	{
	Image_buffer8::copy8((unsigned char *) src->bits,
		src->get_width(), src->get_height(), destx, desty);
	}

/*
 *	Fill with a given 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix
	)
	{
	unsigned char *pixels = bits;
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with an 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Fill a line with a given 8-bit value.
 */

void Image_buffer8::fill_line8
	(
	unsigned char pix,
	int srcw,
	int destx, int desty
	)
	{
	int srcx = 0;
					// Constrain to window's space.
	if (!clip_x(srcx, srcw, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	std::memset(pixels, pix, srcw);
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer8::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy a line into this buffer.
 */

void Image_buffer8::copy_line8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty
	)
	{
	int srcx = 0;
					// Constrain to window's space.
	if (!clip_x(srcx, srcw, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	std::memcpy(to, from, srcw);
	}

/*
 *	Copy a line into this buffer where some of the colors are translucent.
 */

void Image_buffer8::copy_line_translucent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty,
	int first_translucent,		// Palette index of 1st trans. color.
	int last_translucent,		// Index of last trans. color.
	Xform_palette *xforms		// Transformers.  Need same # as
					//   (last_translucent - 
					//    first_translucent + 1).
	)
	{
	int srcx = 0;
					// Constrain to window's space.
	if (!clip_x(srcx, srcw, destx, desty))
		return;
	unsigned char *to = (unsigned char *) bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	for (int i = srcw; i; i--)
		{
					// Get char., and transform.
		unsigned char c = *from++;
		if (c >= first_translucent && c <= last_translucent)
					// Use table to shift existing pixel.
			c = xforms[c - first_translucent][*to];
		*to++ = c;
		}
	}

/*
 *	Apply a translucency table to a line.
 */

void Image_buffer8::fill_line_translucent8
	(
	unsigned char val,		// Ignored for this method.
	int srcw,
	int destx, int desty,
	Xform_palette xform		// Transform table.
	)
	{
	int srcx = 0;
					// Constrain to window's space.
	if (!clip_x(srcx, srcw, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	while (srcw--)
		{
		*pixels = xform[*pixels];
		pixels++;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer8::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = chr;
			}
		to += to_next;
		from += from_next;
		}
	}

