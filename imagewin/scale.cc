/**
 **	Scale.cc - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_video.h"
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif

#include "exult_types.h"

using std::memcpy;

/** 
 ** 2xSaI scaling filter source code adapted for Exult
 ** August 29 2000, originally written in May 1999
 ** by Derek Liauw Kie Fa (DerekL666@yahoo.com/D.A.K.L.LiauwKieFa@student.tudelft.nl)
 ** This source is made available under the terms of the GNU GPL
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
inline Dest_pixel OInterpolate_2xSaI (Source_pixel colorA, Source_pixel colorB, Source_pixel colorC, const Manip_pixels &manip)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	unsigned int r2, g2, b2;
	manip.split_source(colorA, r0, g0, b0);
	manip.split_source(colorB, r1, g1, b1);
	manip.split_source(colorC, r2, g2, b2);
	unsigned int r = ((r0<<2) + (r0<<1) + r1 + r2)>>3;
	unsigned int g = ((g0<<2) + (g0<<1) + g1 + g2)>>3;
	unsigned int b = ((b0<<2) + (b0<<1) + b1 + b2)>>3;
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
inline int GetResult1(Source_pixel A, Source_pixel B, Source_pixel C, Source_pixel D)
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
inline int GetResult2(Source_pixel A, Source_pixel B, Source_pixel C, Source_pixel D) 
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


// 2xSaI scaler
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

				  r += GetResult1 <Source_pixel>(colorA, colorB, colorG, colorE);
				  r += GetResult2 <Source_pixel>(colorB, colorA, colorK, colorF);
				  r += GetResult2 <Source_pixel>(colorB, colorA, colorH, colorN);
				  r += GetResult1 <Source_pixel>(colorA, colorB, colorL, colorO);

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
			prev1_xoff = 1;
		}//end of for ( finish= width etc..)

		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
		prev1_yoff = 1;
	};
}



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
	)
{

	Source_pixel *srcPtr = source + (srcx + srcy*sline_pixels);
	Dest_pixel *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}

    int ybeforelast1 = sheight - 1 - srcy;
    int ybeforelast2 = sheight - 2 - srcy;
    int xbeforelast1 = sline_pixels - 1 - srcx;
    int xbeforelast2 = sline_pixels - 2 - srcx;
		
    for (int y = 0; y < srch; y++)
	{
		Source_pixel *bP = srcPtr;
		Dest_pixel *dP = dstPtr;

		for (int x = 0; x < srcw; x++)
		{
           Source_pixel color4, color5, color6;
           Source_pixel color1, color2, color3;
           Source_pixel colorA0, colorA1, colorA2, colorA3,
						colorB0, colorB1, colorB2, colorB3,
						colorS1, colorS2;
           Dest_pixel product1a, product1b,
					 product2a, product2b;

			//---------------------------------------  B0 B1 B2 B3
			//                                         4  5  6  S2
			//                                         1  2  3  S1
			//                                         A0 A1 A2 A3
			//--------------------------------------
			int add1, add2;
			int sub1;
			int nextl1, nextl2;
			int prevl1;

			if (x == 0)
				sub1 = 0;
			else
				sub1 = 1;

			if (x >= xbeforelast2)
				add2 = 0;
			else add2 = 1;

			if (x >= xbeforelast1)
				add1 = 0;
			else add1 = 1;

			if (y == 0)
				prevl1 = 0;
			else
				prevl1 = sline_pixels;

			if (y >= ybeforelast2)
				nextl2 = 0;
			else nextl2 = sline_pixels;

			if (y >= ybeforelast1)
				nextl1 = 0;
			else nextl1 = sline_pixels;


            colorB0 = *(bP- prevl1 - sub1);
            colorB1 = *(bP- prevl1);
            colorB2 = *(bP- prevl1 + add1);
            colorB3 = *(bP- prevl1 + add1 + add2);

            color4 = *(bP - sub1);
            color5 = *(bP);
            color6 = *(bP + add1);
            colorS2 = *(bP + add1 + add2);

            color1 = *(bP + nextl1 - sub1);
            color2 = *(bP + nextl1);
            color3 = *(bP + nextl1 + add1);
            colorS1 = *(bP + nextl1 + add1 + add2);

            colorA0 = *(bP + nextl1 + nextl2 - sub1);
            colorA1 = *(bP + nextl1 + nextl2);
            colorA2 = *(bP + nextl1 + nextl2 + add1);
            colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);

			if (color2 == color6 && color5 != color3)
			{
			   //product2b = product1b = color2;
				manip.copy(product2b, color2);
			   	product1b = product2b;
			}
			else
			if (color5 == color3 && color2 != color6)
			{
			   //product2b = product1b = color5;
			   	manip.copy(product2b, color5);
			   	product1b = product2b;
			}
			else
			if (color5 == color3 && color2 == color6)
			{
			   	register int r = 0;

               	//r += GetResult (color6, color5, color1, colorA1);
               	//r += GetResult (color6, color5, color4, colorB1);
               	//r += GetResult (color6, color5, colorA2, colorS1);
               	//r += GetResult (color6, color5, colorB2, colorS2);
			   	r += GetResult1 <Source_pixel>(color5, color6, color4, colorB1);
			   	r += GetResult2 <Source_pixel>(color6, color5, colorA2, colorS1);
			   	r += GetResult2 <Source_pixel>(color6, color5, color1, colorA1);
 			   	r += GetResult1 <Source_pixel>(color5, color6, colorB2, colorS2);

			   	if (r > 0)
				{
				 	//product2b = product1b = color6;
					manip.copy(product2b, color6);
					product1b = product2b;
				}
			   	else
			   	if (r < 0)
				{
					//product2b = product1b = color5;
					manip.copy(product2b, color5);
					product1b = product2b;
				}
			   	else
			   	{
				  	//product2b = product1b = INTERPOLATE (color5, color6);
				  	product1b = product2b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, manip);
			   	}

			}
			else
			{

			   if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
				  	//product2b = Q_INTERPOLATE (color3, color3, color3, color2);
				  	product2b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color3, color3, color3, color2, manip);
			   else
			   if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
				  	//product2b = Q_INTERPOLATE (color2, color2, color2, color3);
				   	product2b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color3, color2, color2, color2, manip);
			   else
				  	//product2b = INTERPOLATE (color2, color3);
				  	product2b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color3, manip);


			   if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
				  	//product1b = Q_INTERPOLATE (color6, color6, color6, color5);
				   	product1b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, color6, color6, manip);
			   else
			   if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
				  	//product1b = Q_INTERPOLATE (color6, color5, color5, color5);
				   	product1b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color6, color5, color5, color5, manip);
			   else
				  	//product1b = INTERPOLATE (color5, color6);
				  	product1b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, manip);

			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
			   	//product2a = INTERPOLATE (color2, color5);
			  	product2a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color2, manip);
			else
			if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
			   	//product2a = INTERPOLATE(color2, color5);
			  	product2a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color2, manip);
			else
			   	//product2a = color2;
				manip.copy(product2a, color2);


			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
			   	//product1a = INTERPOLATE (color2, color5);
			  	product1a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color2, manip);
			else
			if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
			   	//product1a = INTERPOLATE(color2, color5);
			  	product1a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color2, manip);
			else
			   	//product1a = color5;
				manip.copy(product1a, color5);


			*dP = product1a;
			*(dP+1) = product1b;
			*(dP+dline_pixels) = product2a;
			*(dP+dline_pixels+1) = product2b;

			bP += 1;
			dP += 2;

		}
		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	}; 
}



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
	)
{

	// Need to ensure that the update is alligned to 4 pixels - Colourless
	// The idea was to prevent artifacts from appearing, but it doesn't seem
	// to help
	/*
	{
		int sx = ((srcx-4)/4)*4;
		int ex = ((srcx+srcw+7)/4)*4;
		int sy = ((srcy-4)/4)*4;
		int ey = ((srcy+srch+7)/4)*4;

		if (sx < 0) sx = 0;
		if (sy < 0) sy = 0;
		if (ex > sline_pixels) ex = sline_pixels;
		if (ey > sheight) ey = sheight;

		srcx = sx;
		srcy = sy;
		srcw = ex - sx;
		srch = ey - sy;
	}
	*/

	Source_pixel *srcPtr = source + (srcx + srcy*sline_pixels);
	Dest_pixel *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}

    int ybeforelast1 = sheight - 1 - srcy;
    int ybeforelast2 = sheight - 2 - srcy;
    int xbeforelast1 = sline_pixels - 1 - srcx;
    int xbeforelast2 = sline_pixels - 2 - srcx;
		
    for (int y = 0; y < srch; y++)
	{
		Source_pixel *bP = srcPtr;
		Dest_pixel *dP = dstPtr;

		for (int x = 0; x < srcw; x++)
		{
           Source_pixel color4, color5, color6;
           Source_pixel color1, color2, color3;
           Source_pixel colorA0, colorA1, colorA2, colorA3,
						colorB0, colorB1, colorB2, colorB3,
						colorS1, colorS2;
           Dest_pixel product1a, product1b,
					 product2a, product2b;

			//---------------------------------------  B0 B1 B2 B3
			//                                         4  5  6  S2
			//                                         1  2  3  S1
			//                                         A0 A1 A2 A3
			//--------------------------------------
			int add1, add2;
			int sub1;
			int nextl1, nextl2;
			int prevl1;

			if (x == 0)
				sub1 = 0;
			else
				sub1 = 1;

			if (x >= xbeforelast2)
				add2 = 0;
			else add2 = 1;

			if (x >= xbeforelast1)
				add1 = 0;
			else add1 = 1;

			if (y == 0)
				prevl1 = 0;
			else
				prevl1 = sline_pixels;

			if (y >= ybeforelast2)
				nextl2 = 0;
			else nextl2 = sline_pixels;

			if (y >= ybeforelast1)
				nextl1 = 0;
			else nextl1 = sline_pixels;


            colorB0 = *(bP- prevl1 - sub1);
            colorB1 = *(bP- prevl1);
            colorB2 = *(bP- prevl1 + add1);
            colorB3 = *(bP- prevl1 + add1 + add2);

            color4 = *(bP - sub1);
            color5 = *(bP);
            color6 = *(bP + add1);
            colorS2 = *(bP + add1 + add2);

            color1 = *(bP + nextl1 - sub1);
            color2 = *(bP + nextl1);
            color3 = *(bP + nextl1 + add1);
            colorS1 = *(bP + nextl1 + add1 + add2);

            colorA0 = *(bP + nextl1 + nextl2 - sub1);
            colorA1 = *(bP + nextl1 + nextl2);
            colorA2 = *(bP + nextl1 + nextl2 + add1);
            colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);


			if (color2 == color6 && color5 != color3)
			{
			   //product1b = product2a = color2;
			   manip.copy(product2a, color2);
			   product1b = product2a;


			   if ((color1 == color2) || (color6 == colorB2))
			   {
				   //product1a = INTERPOLATE (color2, color5);
				   //product1a = INTERPOLATE (color2, product1a);
				   product1a = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color2, color2, color5, manip);

			   }
			   else
			   {
				   //product1a = INTERPOLATE (color5, color6);
				   product1a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color6, color5, manip);
			   }

			   if ((color6 == colorS2) || (color2 == colorA1))
               {
                   //product2b = INTERPOLATE (color2, color3);
                   //product2b = INTERPOLATE (color2, product2b);
				   product2b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color2, color2, color3, manip);

               }
               else
               {
                   //product2b = INTERPOLATE (color2, color3);
				   product2b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color3, manip);
               }
            }
            else
            if (color5 == color3 && color2 != color6)
            {
               //product2b = product1a = color5;
   			   manip.copy(product1a, color5);
			   product2b = product1a;

 
               if ((colorB1 == color5) ||  (color3 == colorS1))
               {
                   //product1b = INTERPOLATE (color5, color6);
				   //product1b = INTERPOLATE (color5, product1b);
				   product1b = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color5, color5, color6, manip);
               }
               else
               {
                  //product1b = INTERPOLATE (color5, color6);
				  product1b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, manip);
               }

			   if ((color3 == colorA2) || (color4 == color5))
               {
                   //product2a = INTERPOLATE (color5, color2);
                   //product2a = INTERPOLATE (color5, product2a);
				   product2a = QInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color5, color5, color5, manip);
               }
               else
               {
                  //product2a = INTERPOLATE (color2, color3);
				  product2a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color3, color2, manip);
               }

            }
            else
            if (color5 == color3 && color2 == color6)
            {
               register int r = 0;

               //r += GetResult (color6, color5, color1, colorA1);
               //r += GetResult (color6, color5, color4, colorB1);
               //r += GetResult (color6, color5, colorA2, colorS1);
               //r += GetResult (color6, color5, colorB2, colorS2);
			   r += GetResult1 <Source_pixel>(color5, color6, color4, colorB1);
			   r += GetResult2 <Source_pixel>(color6, color5, colorA2, colorS1);
			   r += GetResult2 <Source_pixel>(color6, color5, color1, colorA1);
 			   r += GetResult1 <Source_pixel>(color5, color6, colorB2, colorS2);

               if (r > 0)
               {
                  //product1b = product2a = color2;
  				   manip.copy(product2a, color2);
				   product1b = product2a;
                  //product1a = product2b = INTERPOLATE (color5, color6);
				  product1a = product2b = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, manip);
               }
               else
               if (r < 0)
               {
                  //product2b = product1a = color5;
				   manip.copy(product1a, color5);
				   product2b = product1a;
                  //product1b = product2a = INTERPOLATE (color5, color6);
				  product1b = product2a = Interpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, manip);
               }
               else
               {
                  //product2b = product1a = color5;
				   manip.copy(product1a, color5);
				   product2b = product1a;
                  //product1b = product2a = color2;
				   manip.copy(product2a, color2);
  				   product1b = product2a;

               }
            }
            else
            {
                  //product2b = product1a = INTERPOLATE (color2, color6);
                  //product2b = Q_INTERPOLATE (color3, color3, color3, product2b);
                  //product1a = Q_INTERPOLATE (color5, color5, color5, product1a);
				  product2b = OInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color3, color2, color6, manip);
				  product1a = OInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color5, color6, color2, manip);

                  //product2a = product1b = INTERPOLATE (color5, color3);
                  //product2a = Q_INTERPOLATE (color2, color2, color2, product2a);
                  //product1b = Q_INTERPOLATE (color6, color6, color6, product1b);
				  product2a = OInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color2, color5, color3, manip);
				  product1b = OInterpolate_2xSaI< Source_pixel,  Dest_pixel,  Manip_pixels>(color6, color5, color3, manip);
			}

			*dP = product1a;
			*(dP+1) = product1b;
			*(dP+dline_pixels) = product2a;
			*(dP+dline_pixels+1) = product2b;

			bP += 1;
			dP += 2;

		}
		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	}; 
}



/** 
 ** End of 2xSaI code
 **/


typedef unsigned int COMPONENT;


// fill `row' with the the disassembled color components from the original
// pixel values in `from'; if we run out of source pixels, just keep copying
// the last one we got
template <class Source_pixel, class Manip_pixels>
inline void fill_rgb_row
	(
	 Source_pixel *from,
	 int src_width,	// number of pixels to read from 'from'
	 COMPONENT *row,
	 int width,		// number of pixels to write into 'row'
	 const Manip_pixels& manip
	 )
{
	COMPONENT *copy_start = row + src_width*3;
	COMPONENT *all_stop = row + width*3;
	while (row < copy_start)
		{
		COMPONENT& r = *row++;
		COMPONENT& g = *row++;
		COMPONENT& b = *row++;
		manip.split_source(*from++, r, g, b);
		}
	// any remaining elements to be written to 'row' are a replica of the
	// preceding pixel
	COMPONENT *p = row-3;
	while (row < all_stop) {
		// we're guaranteed three elements per pixel; could unroll the loop
		// further, especially with a Duff's Device, but the gains would be
		// probably limited (judging by profiler output)
		*row++ = *p++;
		*row++ = *p++;
		*row++ = *p++;
	}
}


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
	)
{
	Source_pixel *from = source + srcy*sline_pixels + srcx;
	Dest_pixel *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Dest_pixel *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static unsigned buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1, manip);

	for (unsigned y=0; y < srch; y++)
		{
		Source_pixel *from_orig = from;
		Dest_pixel *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next, 
						 srcw+1, manip);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1, manip);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (unsigned x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			*to++ = manip.rgb(*ar, *ag, *ab);

			// upper right
			*to++ = manip.rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = manip.rgb((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);

			// lower right
			*to_odd++ = manip.rgb((*ar+*br+*cr+*dr)>>2,
					  		      (*ag+*bg+*cg+*dg)>>2,
							      (*ab+*bb+*cb+*db)>>2);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}


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
	)
{
	Source_pixel *from = source + srcy*sline_pixels + srcx;
	Dest_pixel *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Dest_pixel *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static unsigned buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1, manip);

	for (unsigned y=0; y < srch; y++)
		{
		Source_pixel *from_orig = from;
		Dest_pixel *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next, 
						 srcw+1, manip);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1, manip);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (unsigned x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			//*to++ = manip.rgb(*ar, *ag, *ab);
#ifdef USE_ORIGINAL_BILINEAR_PLUS
			*to++ = manip.rgb(
			(((*ar)<<2) +((*ar)) + (*cr+*br+*br) )>> 3,
			(((*ag)<<2) +((*ag)) + (*cg+*bg+*bg) )>> 3,
			(((*ab)<<2) +((*ab)) + (*cb+*bb+*bb) )>> 3);
#else
			*to++ = manip.rgb(
			(((*ar)<<3) +((*ar)<<1) + (*cr+*br+*br+*cr) )>> 4,
			(((*ag)<<3) +((*ag)<<1) + (*cg+*bg+*bg+*cg) )>> 4,
			(((*ab)<<3) +((*ab)<<1) + (*cb+*bb+*bb+*cb) )>> 4);
#endif

			// upper right
			*to++ = manip.rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = manip.rgb((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);

			// lower right
			*to_odd++ = manip.rgb((*ar+*br+*cr+*dr)>>2,
					  		      (*ag+*bg+*cg+*dg)>>2,
							      (*ab+*bb+*cb+*db)>>2);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}


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


//
// Point Sampling Scaler
//
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
)
{
	source += srcy*sline_pixels + srcx;
	dest += srcy*factor*dline_pixels + srcx*factor;

	char data;
	unsigned char *dest2;
	const unsigned char *source2;
	const unsigned char * limit_y = source + srch*sline_pixels;
	const unsigned char * limit_x = source + srcw;

	if (factor == 2) {
		uint16 *dest16;
		uint16 *dest16_2;
		uint16 data16;
		
		while (source < limit_y)
		{
			source2 = source;

			dest16 = (uint16*) dest;
			dest += dline_pixels;
			dest16_2 = (uint16*) dest;

			while (source2 < limit_x)
			{
				data16 = *source2++;
				data16 |= data16 << 8;
				*dest16++ = data16;
				*dest16_2++ = data16;
			}
			dest += dline_pixels;
			limit_x += sline_pixels;
			source += sline_pixels;
		}
	} else {
		const unsigned int y2_pixels = dline_pixels*factor;
		const unsigned char * limit_y2 = dest;
		const unsigned char * limit_x2;

		while (source < limit_y)
		{
			limit_y2 += y2_pixels;
			while (dest < limit_y2)
			{
				limit_x2 = dest2 = dest;
				source2 = source;
				while (source2 < limit_x)
				{
					data = *source2++;
					limit_x2 += factor;
					while (dest2 < limit_x2)
						*dest2++ = data;
				}
				dest += dline_pixels;
			}
			limit_x += sline_pixels;
			source += sline_pixels;
		}
	}
}

//
// Interlaced Point Sampling Scaler
//
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
)
{
	source += srcy*sline_pixels + srcx;
	dest += srcy*factor*dline_pixels + srcx*factor;

	char data;
	unsigned char *dest2;
	const unsigned char *source2;
	const unsigned char * limit_y = source + srch*sline_pixels;
	const unsigned char * limit_x = source + srcw;

	if (factor == 2) {
		uint16 *dest16;
		uint16 data16;
		
		while (source < limit_y)
		{
			source2 = source;
			dest16 = (uint16*) dest;
			while (source2 < limit_x)
			{
				data16 = *source2++;
				data16 |= data16 << 8;
				*dest16++ = data16;
			}
			dest += dline_pixels;
			dest += dline_pixels;
			limit_x += sline_pixels;
			source += sline_pixels;
		}
	} else {
		bool visible_line = ((srcy * factor) % 2 == 0);
		const unsigned int y2_pixels = dline_pixels*factor;
		const unsigned char * limit_y2 = dest;
		const unsigned char * limit_x2;

		while (source < limit_y)
		{
			limit_y2 += y2_pixels;
			while (dest < limit_y2)
			{
				if (visible_line)
				{
					limit_x2 = dest2 = dest;
					source2 = source;
					while (source2 < limit_x)
					{
						data = *source2++;
						limit_x2 += factor;
						while (dest2 < limit_x2)
							*dest2++ = data;
					}
				}
				dest += dline_pixels;
				visible_line = !visible_line;
			}
			limit_x += sline_pixels;
			source += sline_pixels;
		}
	}
}



//
// Scale2X algorithm by Andrea Mazzoleni.
//
/* This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
void Scale2x_noblur
(
	const unsigned char *src,	// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,		// Source height.
	unsigned char *dest,		// ->dest pixels.
	const int dline_pixels		// Pixels (words)/line for dest.
)
{
	dest += srcy*2*dline_pixels + srcx*2;
	unsigned char *dest0 = dest, *dest1 = dest + dline_pixels;
					// ->current row.
	const unsigned char *src1 = src + srcy*sline_pixels + srcx;
	const unsigned char *src0 = src1 - sline_pixels;	// ->prev. row.
	const unsigned char *src2 = src1 + sline_pixels;	// ->next row.
	const unsigned char * limit_y = src1 + srch*sline_pixels;
	const unsigned char * limit_x = src1 + srcw;
					// Very end of source surface:
	const unsigned char * end_src = src + sheight*sline_pixels;

	if (src0 < src)
		src0 = src1;		// Don't go before row 0.
	if (srcx + srcw == sline_pixels)	// Going to right edge?
		limit_x--;		// Stop 1 pixel before it.
	while (src1 < limit_y)
		{
		if (src2 > end_src)
			src2 = src1;	// On last row.
		if (srcx == 0)		// First pixel.
			{
			dest0[0] = dest1[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
					// Middle pixels.
		while (src1 < limit_x)
			{
			if (src1[-1] == src0[0] && src2[0] != src0[0] &&
			    src1[1] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0] && 
			    src1[-1] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0] && 
			    src1[1] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0] && 
			    src1[-1] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		if (srcx + srcw == sline_pixels)
			{		// End pixel in row.
			if (src1[-1] == src0[0] && src2[0] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			dest0[1] = src1[0];
			dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		src0 += sline_pixels - srcw;
		src1 += sline_pixels - srcw;
		src2 += sline_pixels - srcw;
		dest1 += dline_pixels - 2*srcw;
		if (src0 == src1)	// End of first row?
			src0 -= sline_pixels;
		limit_x += sline_pixels;
		dest0 = dest1;
		dest1 += dline_pixels;
		}
}




