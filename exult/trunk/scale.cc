/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.cc - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#include "SDL.h"
#include <string.h>

/*
 *	Going horizontally, split one pixel into two.
 */
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
inline void Interp_horiz
	(
	Source_pixel *& from,		// ->source pixels.
	Dest_pixel *& to,		// ->dest pixels.
	const Manip_pixels& manip	// Manipulator methods.
	)
	{
	Source_pixel pix0 = *from++;
	Source_pixel pix1 = *from;
	manip.copy(*to++, pix0);
	unsigned short r0, r1, g0, g1, b0, b1;
	manip.split_source(pix0, r0, g0, b0);
	manip.split_source(pix1, r1, g1, b1);
	*to++ = manip.rgb((r0 + r1)/2, (g0 + g1)/2, (b0 + b1)/2);
	}

/*
 *	Form new row by interpolating the pixels above and below.
 */
template <class Dest_pixel, class Manip_pixels>
inline void Interp_vert
	(
	Dest_pixel *& from0,		// ->row above.
	Dest_pixel *& from1,		// ->row below.
	Dest_pixel *& to,		// ->dest pixels.
	const Manip_pixels& manip	// Manipulator methods.
	)
	{
	Dest_pixel pix0 = *from0++, pix1 = *from1++;
	unsigned short r0, r1, g0, g1, b0, b1;
	manip.split_dest(pix0, r0, g0, b0);
	manip.split_dest(pix1, r1, g1, b1);
	*to++ = manip.rgb((r0 + r1)/2, (g0 + g1)/2, (b0 + b1)/2);
	}

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
	)
	{
	Source_pixel *from = source;
	Dest_pixel *to = dest;
	int swidth = dline_pixels/2;	// Take min. of pixels/line.
	if (swidth > sline_pixels)
		swidth = sline_pixels;
					// Do each row, interpolating horiz.
	for (int y = 0; y < sheight; y++)
		{
		int count = swidth - 1;
		register Source_pixel *source_line = from;
		register Dest_pixel *dest_line = to;
		register int n = ( count + 7 ) / 8;
		switch( count % 8 )
			{
	             	case 0: do { Interp_horiz(from, to, manip);
        	     	case 7:      Interp_horiz(from, to, manip);
	             	case 6:      Interp_horiz(from, to, manip);
        	     	case 5:      Interp_horiz(from, to, manip);
	             	case 4:      Interp_horiz(from, to, manip);
        	     	case 3:      Interp_horiz(from, to, manip);
	             	case 2:      Interp_horiz(from, to, manip);
        	     	case 1:      Interp_horiz(from, to, manip);
                	       } while( --n > 0 );
			}
		manip.copy(*to++, *from);// End of row.
		manip.copy(*to++, *from++);
		from = source_line + sline_pixels;
					// Skip odd rows.
		to = dest_line + 2*dline_pixels;
		}
	Dest_pixel *from0 = dest;	// Interpolate vertically.
	Dest_pixel *from1;
	for (int y = 0; y < sheight - 1; y++)
		{
		to = from0 + dline_pixels;
		from1 = to + dline_pixels;
		Dest_pixel *source_line1 = from1;
		int count = dline_pixels;
		int n = ( count + 7 ) / 8;
		switch( count % 8 )
			{
	             	case 0: do { Interp_vert(from0, from1, to, manip);
        	     	case 7:      Interp_vert(from0, from1, to, manip);
	             	case 6:      Interp_vert(from0, from1, to, manip);
        	     	case 5:      Interp_vert(from0, from1, to, manip);
	             	case 4:      Interp_vert(from0, from1, to, manip);
        	     	case 3:      Interp_vert(from0, from1, to, manip);
	             	case 2:      Interp_vert(from0, from1, to, manip);
        	     	case 1:      Interp_vert(from0, from1, to, manip);
                	       } while( --n > 0 );
			}
					// Doing every other line.
		from0 = source_line1;
		}
					// Just copy last row.
	memcpy(from0 + dline_pixels, from0, dline_pixels*sizeof(*from0));
	}

#if 1
/*
 *	Scale a rectangle X2 with bilinear interpolation.
 */
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale2x
	(
	Source_pixel *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	)
	{
	Source_pixel *from = source + srcy*sline_pixels + srcx;
	Dest_pixel *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Dest_pixel *from0 = to;		// We'll use in 2nd pass.
	int right_edge = 0;		// Row ends at rt. edge of window.
	int swidth = srcw;
	if (srcx + swidth >= sline_pixels)
		{
		right_edge = 1;
		swidth = sline_pixels - srcx - 1;
		}
					// Do each row, interpolating horiz.
	for (int y = 0; y < srch; y++)
		{
		int count = swidth;
		register Source_pixel *source_line = from;
		register Dest_pixel *dest_line = to;
		register int n = ( count + 7 ) / 8;
		switch( count % 8 )
			{
	             	case 0: do { Interp_horiz(from, to, manip);
        	     	case 7:      Interp_horiz(from, to, manip);
	             	case 6:      Interp_horiz(from, to, manip);
        	     	case 5:      Interp_horiz(from, to, manip);
	             	case 4:      Interp_horiz(from, to, manip);
        	     	case 3:      Interp_horiz(from, to, manip);
	             	case 2:      Interp_horiz(from, to, manip);
        	     	case 1:      Interp_horiz(from, to, manip);
                	       } while( --n > 0 );
			}
		if (right_edge)		// Handle right edge.
			{
			manip.copy(*to++, *from);
			manip.copy(*to++, *from++);
			}
		from = source_line + sline_pixels;
					// Skip odd rows.
		to = dest_line + 2*dline_pixels;
		}
					// Interpolate vertically.
	Dest_pixel *from1;
	int bottom_edge = 0;
	int dheight = srch;
	if (srcy + srch >= sheight)	// Watch for bottom row.
		{
		bottom_edge = 1;
		dheight = sheight - srcy - 1;
		}
	for (int y = 0; y < dheight; y++)
		{
		to = from0 + dline_pixels;
		from1 = to + dline_pixels;
		Dest_pixel *source_line1 = from1;
		int count = 2*srcw;
		int n = ( count + 7 ) / 8;
		switch( count % 8 )
			{
	             	case 0: do { Interp_vert(from0, from1, to, manip);
        	     	case 7:      Interp_vert(from0, from1, to, manip);
	             	case 6:      Interp_vert(from0, from1, to, manip);
        	     	case 5:      Interp_vert(from0, from1, to, manip);
	             	case 4:      Interp_vert(from0, from1, to, manip);
        	     	case 3:      Interp_vert(from0, from1, to, manip);
	             	case 2:      Interp_vert(from0, from1, to, manip);
        	     	case 1:      Interp_vert(from0, from1, to, manip);
                	       } while( --n > 0 );
			}
					// Doing every other line.
		from0 = source_line1;
		}
	if (bottom_edge)		// Just copy last row.
		memcpy(from0 + dline_pixels, from0, 2*srcw*sizeof(*from0));
	}
#endif

#if 0	/* Testing */
void test()
	{
	unsigned short *src, *dest;
	Manip16to16 manip;

	Scale2x<unsigned short, unsigned short, Manip16to16>
					(src, 20, 40, dest, manip);

	unsigned char *src8;
	Manip8to16 manip8(0);		// ++++DOn't try to run this!
	Scale2x<unsigned char, unsigned short, Manip8to16>
					(src8, 20, 40, dest, manip8);
	}
#endif
