/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.cc - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#include "SDL.h"
#include <string.h>
#include "scale.h"

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
	manip.split(pix0, r0, g0, b0);
	manip.split(pix1, r1, g1, b1);
	manip.copy(*to++, manip.rgb((r0 + r1)/2, (g0 + g1)/2, (b0 + b1)/2));
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
	manip.split(pix0, r0, g0, b0);
	manip.split(pix1, r1, g1, b1);
	manip.copy(*to++, manip.rgb((r0 + r1)/2, (g0 + g1)/2, (b0 + b1)/2));
	}

/*
 *	Scale X2 with bilinear interpolation.
 */
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale2x
	(
	Source_pixel *source,		// ->source pixels.
	int swidth, int sheight,	// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	const Manip_pixels& manip	// Manipulator methods.
	)
	{
					// Figure dest. dims.
	int dwidth = swidth*2, dheight = sheight*2;
	Source_pixel *from = source;
	Dest_pixel *to = dest;
					// Do each row, interpolating horiz.
	for (int y = 0; y < sheight; y++)
		{
		for (int x = 0; x < swidth - 1; x++)
			Interp_horiz(from, to, manip);
		manip.copy(*to++, *from);// End of row.
		manip.copy(*to++, *from++);
		to += dwidth;		// Skip odd rows.
		}
	Dest_pixel *from0 = dest;	// Interpolate vertically.
	to = dest + dwidth;
	Dest_pixel *from1 = to + dwidth;
	for (int y = 0; y < sheight - 1; y++)
		{
		for (int x = 0; x < dwidth; x++)
			Interp_vert(from0, from1, to, manip);
		to += dwidth;		// Skip row.
		from0 = from1;
		from1 += dwidth;
		}
					// Just copy last row.
	memcpy(to, from0, dwidth*sizeof(*from0));
	}

#if 1	/* Testing */
/*
 *	Manipulate from 16-bit to 16-bit pixels.
 */
class Manip16to16
	{
public:
	static void copy(unsigned short& dest, unsigned short src)
		{ dest = src; }
	static void split(unsigned short pix, unsigned short& r,
					unsigned short& g, unsigned short& b)
		{
		r = (pix>>10)&0x1f;
		g = (pix>>5)&0x1f;
		b = pix&0x1f;
		}
	static unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b)
		{ return ((r&0x1f)<<10) | ((g&0x1f)<<5) | (b&0x1f); }
	};

void test()
	{
	unsigned short *src, *dest;
	Manip16to16 manip;

	Scale2x<unsigned short, unsigned short, Manip16to16>
					(src, 20, 40, dest, manip);

	unsigned char *src8;
	Manip8to16 manip8(0, 0);		// ++++DOn't try to run this!
	Scale2x<unsigned char, unsigned short, Manip8to16>
					(src8, 20, 40, dest, manip8);
	}
#endif
