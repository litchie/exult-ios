/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.h - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#ifndef INCL_SCALE
#define INCL_SCALE 1

// Scale 2x using 2xSaI
//template <class Source_pixel, class Dest_pixel, class Manip_pixels>
//void Scale_2xSaI
//(
//	Source_pixel *source,		// ->source pixels.
//	int sline_pixels,		// Pixels (words)/line for source.
//	int sheight,			// Source height.
//	Dest_pixel *dest,		// ->dest pixels.
//	int dline_pixels,		// Pixels (words)/line for dest.
//	const Manip_pixels& manip	// Manipulator methods.
//);

//Scale X2 with bilinear interpolation.
//template <class Source_pixel, class Dest_pixel, class Manip_pixels>
//void Scale_2xBi
//(
	//Source_pixel *source,		// ->source pixels.
	//int sline_pixels,		// Pixels (words)/line for source.
	//int sheight,			// Source height.
	//Dest_pixel *dest,		// ->dest pixels.
	//int dline_pixels,		// Pixels (words)/line for dest.
	//const Manip_pixels& manip	// Manipulator methods.
//);

// Point Sampling Scaler
void Scale_point
(
	unsigned char *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	unsigned char  *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	int factor			// Scale factor
);

// Interlaced Point Sampling Scaler
void Scale_interlace
(
	unsigned char *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	unsigned char  *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	int factor			// Scale factor
);

#include "scale.cc"		/* Seems to be needed. */

#endif

