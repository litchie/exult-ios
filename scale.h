/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.h - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#ifndef INCL_SCALE
#define INCL_SCALE 1

/*
 *	Scale X2 with bilinear interpolation.
 */
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale2x
	(
	Source_pixel *source,		// ->source pixels.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	);

#include "scale.cc"		/* Seems to be needed. */

#endif

