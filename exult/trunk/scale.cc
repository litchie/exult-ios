/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.cc - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#include <string.h>

/*
 *	Get color components from a 16-bit pixel.
 */
inline unsigned short R16(unsigned short c)
	{ return (c >> 10)&0x1f; }
inline unsigned short G16(unsigned short c)
	{ return (c >> 5)&0x1f; }
inline unsigned short B16(unsigned short c)
	{ return c&0x1f; }

/*
 *	Create 16-bit RGB value.
 */
inline unsigned short RGB16(unsigned short r, unsigned short g, 
							unsigned short b)
	{ return ((r&0x1f)<<10) | ((g&0x1f)<<5) | (b&0x1f); }

/*
 *	Scale X2 with bilinear interpolation.
 */
void Scale_x2
	(
	unsigned short *source,		// 16-bit.
	int swidth, int sheight,	// Source height.
	unsigned short *dest		// 16-bit.
	)
	{
					// Figure dest. dims.
	int dwidth = swidth*2, dheight = sheight*2;
	unsigned short *from = source;
	unsigned short *to = dest;
					// Do each row, interpolating horiz.
	for (int y = 0; y < sheight; y++)
		{
		for (int x = 0; x < swidth - 1; x++)
			{
			unsigned short pix0 = *from++;
			unsigned short pix1 = *from;
			*to++ = pix0;
			*to++ = RGB16((R16(pix0) + R16(pix1))/2,
					(B16(pix0) + B16(pix1))/2,
					(G16(pix0) + G16(pix1))/2);
			}
		*to++ = *from;	// End of row.
		*to++ = *from++;
		to += dwidth;		// Skip odd rows.
		}
	from = dest;			// Interpolate vertically.
	to = dest + dwidth;
	unsigned short *from1 = to + dwidth;
	for (int y = 0; y < sheight - 1; y++)
		{
		for (int x = 0; x < dwidth; x++)
			{
			unsigned short pix0 = *from++, pix1 = *from1++;
			*to++ = RGB16((R16(pix0) + R16(pix1))/2,
					(B16(pix0) + B16(pix1))/2,
					(G16(pix0) + G16(pix1))/2);
			}
		to += dwidth;		// Skip row.
		from = from1;
		from1 += dwidth;
		}
					// Just copy last row.
	memcpy(to, from, dwidth*sizeof(*from));
	}
