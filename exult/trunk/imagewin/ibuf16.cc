/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Ibuf16.cc - 16-bit image buffer.
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

#include "ibuf16.h"
#include <string.h>

/*
 *	Create a default palette for 8-bit graphics on a 16-bit display.
 */

void Image_buffer16::create_default_palette
	(
	)
	{
	delete palette;			// Delete old.
	palette = new unsigned short[256];
	for (int i = 0; i < 256; i++)
		palette[i] = rgb(4*((i >> 5) & 0x7), 
			4*((i >> 2) & 0x7), 9*(i & 0x3));
	}

/*
 *	Fill with a given 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix
	)
	{
	unsigned short *pixels = get_pixels();
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with a 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Fill a line with a given 16-bit value.
 */

void Image_buffer16::fill_line16
	(
	unsigned short pix,
	int srcw,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	for (int cnt = srcw; cnt; cnt--)
		*pixels++ = pix;
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy16
	(
	unsigned short *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned short *from = src_pixels + srcy*src_width + srcx;
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
 *	Copy an area of the image within itself.
 */

void Image_buffer16::copy
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
	unsigned short *to = get_pixels() + yto*line_width + destx;
	unsigned short *from = get_pixels() + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw * 2);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Get a rectangle from here into another Image_buffer.
 */

void Image_buffer16::get
	(
	Image_buffer *dest,		// Copy to here.
	int srcx, int srcy		// Upper-left corner of source rect.
	)
	{
	int srcw = dest->get_width(), srch = dest->get_height();
	int destx = 0, desty = 0;
					// Constrain to window's space. (Note
					//   convoluted use of clip().)
	if (!clip(destx, desty, srcw, srch, srcx, srcy))
		return;
	unsigned short *to = (unsigned short *) dest->bits + 
				desty*dest->line_width + destx;
	unsigned short *from = get_pixels() + srcy*line_width + srcx;
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

void Image_buffer16::put
	(
	Image_buffer *src,		// Copy from here.
	int destx, int desty		// Copy to here.
	)
	{
	Image_buffer16::copy16((unsigned short *) src->bits,
		src->get_width(), src->get_height(), destx, desty);
	}

/*
 *	Convert rgb value.
 */

inline unsigned char Get_color16
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned int c = (((unsigned int) val)*brightness*32)/
							(100*(maxval + 1));
	return (c < 32 ? c : 31);
	}

/*
 *	Set palette.
 */

void Image_buffer16::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 3*256; i += 3)
		{
		unsigned char r = Get_color16(rgbs[i], maxval, brightness);
		unsigned char g = Get_color16(rgbs[i + 1], maxval, brightness);
		unsigned char b = Get_color16(rgbs[i + 2], maxval, brightness);
		set_palette_color(i/3, r, g, b);
		}
	}

/*
 *	Rotate a range of colors.
 */

void Image_buffer16::rotate_colors
	(
	int first,			// Palette index of 1st.
	int num,			// # in range.
	int upd				// 1 to update hardware now.
	)
	{
	int cnt = num - 1;		// Shift downward.
	int c0 = palette[first];
	for (int i = first; cnt; i++, cnt--)
		palette[i] = palette[i + 1];
	palette[first + num - 1] = c0;	// Shift 1st to end.
					// +++++upd?
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy8
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
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = palette[*from++];
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy a line into this buffer.
 */

void Image_buffer16::copy_line8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	for (int cnt = srcw; cnt; cnt--)
		*to++ = palette[*from++];
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer16::copy_transparent8
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
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = palette[chr];
			}
		to += to_next;
		from += from_next;
		}
	}

