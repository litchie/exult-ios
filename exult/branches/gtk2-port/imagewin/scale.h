/**
 **	Scale.h - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#ifndef INCL_SCALE
#define INCL_SCALE 1

// Scale 2x using 2xSaI
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_2xSaI
(
	Source_pixel *source,	// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
);

// Super Eagle Scaler
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_SuperEagle
(
	Source_pixel *source,	// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
);

// Super 2xSaI Scaler
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_Super2xSaI
(
	Source_pixel *source,	// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
);


//Scale X2 with bilinear interpolation.
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_2xBilinear
(
	Source_pixel *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
);

//Scale X2 with bilinear plus interpolation.
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale_2xBilinearPlus
(
	Source_pixel *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
);

// Point Sampling Scaler
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
);

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

#include "scale.cc"		/* Seems to be needed. */

#endif

