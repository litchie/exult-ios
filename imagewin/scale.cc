/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.cc - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#include "../alpha_kludges.h"

#include "SDL_video.h"
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif


/** 
 ** 2xSaI scaling filter source code adapted for Exult
 ** August 29 2000, originally written in May 1999
 ** by Derek Liauw Kie Fa (derek-liauw@usa.net)
 ** Use of this source is free for non-commercial use
 ** I'd appreciate it I am given credit in the program or documentation 
 **/


template <class Source_pixel, class Dest_pixel, class Manip_pixels>
inline Dest_pixel Interpolate_2xSaI (Source_pixel colorA, Source_pixel colorB, const Manip_pixels &manip)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	manip.split_source(colorA, r0, g0, b0);
	manip.split_source(colorB, r1, g1, b1);
	int r = (r0 + r1)>>1;
	int g = (g0 + g1)>>1;
	int b = (b0 + b1)>>1;
	return manip.rgb(r, g, b);
}

template <class Source_pixel, class Dest_pixel, class Manip_pixels>
inline Dest_pixel QInterpolate_2xSaI (Source_pixel colorA, Source_pixel colorB, Source_pixel colorC, Source_pixel colorD, const Manip_pixels &manip)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	unsigned int r2, r3, g2, g3, b2, b3;
	manip.split_source(colorA, r0, g0, b0);
	manip.split_source(colorB, r1, g1, b1);
	manip.split_source(colorC, r2, g2, b2);
	manip.split_source(colorD, r3, g3, b3);
	unsigned int r = (r0 + r1 + r2 + r3)>>2;
	unsigned int g = (g0 + g1 + g2 + g3)>>2;
	unsigned int b = (b0 + b1 + b2 + b3)>>2;
	return manip.rgb(r, g, b);
}

template <class Source_pixel>
inline int GetResult1(Source_pixel A, Source_pixel B, Source_pixel C, Source_pixel D, Source_pixel E)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1; 
	if (y <= 1) r-=1;
	return r;
}

template <class Source_pixel>
inline int GetResult2(Source_pixel A, Source_pixel B, Source_pixel C, Source_pixel D, Source_pixel E) 
{
	int x = 0; 
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r-=1; 
	if (y <= 1) r+=1;
	return r;
}


template <class Source_pixel, class Dest_pixel, class Manip_pixels>
//void _2xSaI
void Scale2x
	(
	Source_pixel *source,	// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	)
{
	Source_pixel *srcPtr = source + (srcx + srcy*sline_pixels);
	Dest_pixel *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}
					// Init offset to prev. line, next 2.
    int prev1_yoff = srcy ? sline_pixels : 0;
    int next1_yoff = sline_pixels, next2_yoff = 2*sline_pixels;
					// Figure threshholds for counters.
    int ybeforelast = sheight - 2 - srcy;
    int xbeforelast = sline_pixels - 2 - srcx;
    for (int y = 0; y < srch; y++, prev1_yoff = sline_pixels)
	{
		if (y >= ybeforelast)	// Last/next-to-last row?
			if (y == ybeforelast)
				next2_yoff = sline_pixels;
			else		// Very last line?
				next2_yoff = next1_yoff = 0;
		Source_pixel *bP = srcPtr;
		Dest_pixel *dP = dstPtr;
		int prev1_xoff = srcx ? 1 : 0;
		int next1_xoff = 1, next2_xoff = 2;

			for (int x = 0; x < srcw; x++)
			{
				Source_pixel colorA, colorB;
				Source_pixel colorC, colorD,
					   colorE, colorF, colorG, colorH,
					   colorI, colorJ, colorK, colorL,
					   colorM, colorN, colorO, colorP;
				Dest_pixel product, product1, product2, orig;

					// Last/next-to-last row?
				if (x >= xbeforelast)
					if (x == xbeforelast)
						next2_xoff = 1;
					else
						next2_xoff = next1_xoff = 0;

				//---------------------------------------
				// Map of the pixels:                    I|E F|J
				//                                       G|A B|K
				//                                       H|C D|L
				//                                       M|N O|P
				colorI = *(bP- prev1_yoff - prev1_xoff);
				colorE = *(bP- prev1_yoff);
				colorF = *(bP- prev1_yoff + next1_xoff);
				colorJ = *(bP- prev1_yoff + next2_xoff);

				colorG = *(bP - prev1_xoff);
				colorA = *(bP);
				colorB = *(bP + next1_xoff);
				colorK = *(bP + next2_xoff);

				colorH = *(bP + next1_yoff - prev1_xoff);
				colorC = *(bP + next1_yoff);
				colorD = *(bP + next1_yoff + next1_xoff);
				colorL = *(bP + next1_yoff + next2_xoff);

				colorM = *(bP + next2_yoff - prev1_xoff);
				colorN = *(bP + next2_yoff);
				colorO = *(bP + next2_yoff + next1_xoff);
				colorP = *(bP + next2_yoff + next2_xoff);

					if ((colorA == colorD) && (colorB != colorC))
					{
					   if ( ((colorA == colorE) && (colorB == colorL)) ||
							((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) )
					   {
						  //product = colorA;
							manip.copy(product, colorA);
					   }
					   else
					   {
						  //product = INTERPOLATE(colorA, colorB);
						  product = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, manip);
					   }

					   if (((colorA == colorG) && (colorC == colorO)) ||
						   ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) )
					   {
						  //product1 = colorA;
							manip.copy(product1, colorA);
					   }
					   else
					   {
						  //product1 = INTERPOLATE(colorA, colorC);
						  product1 = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorC, manip);
					   }
					   //product2 = colorA;
					   manip.copy(product2, colorA);
					}
					else
					if ((colorB == colorC) && (colorA != colorD))
					{
					   if (((colorB == colorF) && (colorA == colorH)) ||
						   ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) )
					   {
						  //product = colorB;
						  manip.copy(product, colorB);
					   }
					   else
					   {
						  //product = INTERPOLATE(colorA, colorB);
  						  product = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, manip);
					   }

					   if (((colorC == colorH) && (colorA == colorF)) ||
						   ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) )
					   {
						  //product1 = colorC;
						  manip.copy(product1, colorC);
					   }
					   else
					   {
						  //product1 = INTERPOLATE(colorA, colorC);
  						  product1 = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorC, manip);
					   }
					   //product2 = colorB;
					   manip.copy(product2, colorB);
					}
					else
					if ((colorA == colorD) && (colorB == colorC))
					{
					   if (colorA == colorB)
					   {
						  //product = colorA;
						  manip.copy(product, colorA);
						  //product1 = colorA;
						  manip.copy(product1, colorA);
						  //product2 = colorA;
						  manip.copy(product2, colorA);
					   }
					   else
					   {
						  register int r = 0;
						  //product1 = INTERPOLATE(colorA, colorC);
  						  product1 = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorC, manip);
						  //product = INTERPOLATE(colorA, colorB);
						  product = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, manip);

						  r += GetResult1 <Source_pixel>(colorA, colorB, colorG, colorE, colorI);
						  r += GetResult2 <Source_pixel>(colorB, colorA, colorK, colorF, colorJ);
						  r += GetResult2 <Source_pixel>(colorB, colorA, colorH, colorN, colorM);
						  r += GetResult1 <Source_pixel>(colorA, colorB, colorL, colorO, colorP);

						  if (r > 0)
							  //product2 = colorA;
							  manip.copy(product2, colorA);
						  else
						  if (r < 0)
							  //product2 = colorB;
							  manip.copy(product2, colorB);
						  else
						  {
							  //product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
							  product2 = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, colorC, colorD, manip);
						  }
					   }
					}
					else
					{
					   //product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
					   product2 = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, colorC, colorD, manip);

					   if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
					   {
						  //product = colorA;
						  manip.copy(product, colorA);
					   }
					   else
					   if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
					   {
						  //product = colorB;
						  manip.copy(product, colorB);
					   }
					   else
					   {
						  //product = INTERPOLATE(colorA, colorB);
						  product = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorB, manip);
					   }

					   if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
					   {
						  //product1 = colorA;
						  manip.copy(product1, colorA);
					   }
					   else
					   if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
					   {
						  //product1 = colorC;
						  manip.copy(product1, colorC);
					   }
					   else
					   {
						  //product1 = INTERPOLATE(colorA, colorC);
						  product1 = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(colorA, colorC, manip);
					   }
					}


                	//product = colorA | (product << 16);
					//product1 = product1 | (product2 << 16);
					manip.copy(orig, colorA);
					*dP = orig;
					*(dP+1) = product;
					*(dP+dline_pixels) = product1;
					*(dP+dline_pixels+1) = product2;

					bP += 1;
					dP += 2;
				}//end of for ( finish= width etc..)

		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	};
}

/** 
 ** End of 2xSaI code
 **/




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
	unsigned int r0, r1, g0, g1, b0, b1;
	manip.split_source(pix0, r0, g0, b0);
	manip.split_source(pix1, r1, g1, b1);
	*to++ = manip.rgb((r0 + r1)>>1, (g0 + g1)>>1, (b0 + b1)>>1);
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
	unsigned int r0, r1, g0, g1, b0, b1;
	manip.split_dest(pix0, r0, g0, b0);
	manip.split_dest(pix1, r1, g1, b1);
	*to++ = manip.rgb((r0 + r1)>>1, (g0 + g1)>>1, (b0 + b1)>>1);
	}


#if 0
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
		int n = ( count + 7 ) >>3;
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
#endif


#if 0
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
		register int n = ( count + 7 ) >>3;
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


