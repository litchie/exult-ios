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

#ifndef INCL_SCALE_2X_H
#define INCL_SCALE_2X_H	1

void Scale2x_noblur
(
	const unsigned char *src1,	// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,		// Source height.
	unsigned char *dest,		// ->dest pixels.
	const int dline_pixels		// Pixels (words)/line for dest.
);

#endif
