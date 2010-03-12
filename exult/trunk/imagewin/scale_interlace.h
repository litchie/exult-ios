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

#ifndef INCL_SCALE_INTERLACE_H
#define INCL_SCALE_INTERLACE_H	1

/** 
 ** Note: This file should only be included by source files that use the
 ** templates below; the templates will only be instantiated when they
 ** are used anyway.
 **/

//
// Interlaced Point Sampling Scaler; adapted from Pentagram.
//

template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_interlace
	(
	Source_pixel *source,	// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip,	// Manipulator methods.
	const int factor		// Scale factor
	)
	{
	// Source buffer pointers
	Source_pixel *from = source + srcy*sline_pixels + srcx;
	Source_pixel *limit_x = from + srcw;
	Source_pixel *limit_y = from + srch * sline_pixels;
	int sdiff = sline_pixels - srcw;

	// Dest buffer pointers
	Dest_pixel *to = (dest + factor*srcy*dline_pixels + factor*srcx);

	if (factor == 2)
		{
		int pdiff = 2*dline_pixels - 2*srcw;
		// Src loop Y
		do
			{
			// Src loop X
			do
				{
				Dest_pixel p = manip.copy(*from++);
				*(to +0) = p;
				*(to +1) = p;
				to  += 2;
				} while (from != limit_x);
			to  += pdiff;

			from += sdiff;
			limit_x += sline_pixels;
			} while (from != limit_y);
		}
	else
		{
		bool visible_line = ((srcy * factor) % 2 == 0);
		bool visible_line_inner = visible_line;
		Dest_pixel *px_end = to + factor;
		Dest_pixel *py_end = to + factor * dline_pixels;

		int block_h = dline_pixels * factor;
		int block_xdiff = dline_pixels - factor;
		int pdiff = block_h - factor*srcw;
		// Src loop Y
		do
			{
			// Src loop X
			do
				{
				visible_line_inner = visible_line;
				Dest_pixel p = manip.copy(*from++);
				// Inner loops
				// Dest loop Y
				do
					{
					if (visible_line_inner)
						{
						// Dest loop X
						do
							{
							*to++ = p;
							} while (to != px_end);
						}
					else
						to = px_end;
					to  += block_xdiff;
					px_end += dline_pixels;
					visible_line_inner = !visible_line_inner;
					} while (to != py_end);

				to  += factor - block_h;
				px_end += factor - block_h;
				py_end += factor;
				} while (from != limit_x);
			to  += pdiff;
			py_end += pdiff;
			px_end += pdiff;

			from += sdiff;
			limit_x += sline_pixels;
			visible_line = visible_line_inner;
			} while (from != limit_y);
		}
	}

// Interlaced Point Sampling Scaler
void Scale_interlace
(
	const unsigned char *source,	// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,		// Source height.
	unsigned char *dest,		// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	const int factor		// Scale factor
);

#endif
