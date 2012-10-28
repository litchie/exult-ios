/*
 * xBR algorithm by Hyllian
 * Based on HqMAME version by Zenju
 * Initial Exult version by Marzo Sette Torres Junior 
 *
 * Copyright (C) 2011, 2012 - Hyllian/Jararaca
 * Copyright (C) 2012 - Zenju
 * Copyright (C) 2012 - Marzo Sette Torres Junior
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCL_SCALE_XBR_H
#define INCL_SCALE_XBR_H	1

#ifdef USE_XBR_SCALER

enum RotationDegree //clock-wise
{
	ROT_0,
	ROT_90,
	ROT_180,
	ROT_270
};

template <class Dest_pixel, size_t N, RotationDegree rotDeg>
class OutputMatrix;

//fill block of size n * n with the given color
template <class Dest_pixel>
static inline void fill_block(Dest_pixel* trg, int dline_pixels, Dest_pixel col, int n)
{
	for (int y = 0; y < n; ++y, trg += dline_pixels - n)
		for (int x = 0; x < n; ++x)
			*trg++ = col;
}

template <class Manip_pixels, int tol>
struct YUVColor
{
	int Y, u, v;
	YUVColor(unsigned char c, const Manip_pixels& manip)
	{
		unsigned int r, g, b;
		manip.split_source(c, r, g, b);
#if 1
		Y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
		u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
		v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
#else
		Y = (r + g + b) >> 2;
		u = 128 + ((r - b) >> 2);
		v = 128 + ((-r + 2*g -b)>>3);
#endif
	}
	int dist(YUVColor<Manip_pixels, tol> const& other) const
	{
		//weighted "manhattan" norm over YUV space
		return 48 * abs(Y - other.Y)
		      + 7 * abs(u - other.u)
		      + 6 * abs(v - other.v);
	}
	bool equals(YUVColor<Manip_pixels, tol> const& other) const
	{
		//weighted "manhattan" norm over YUV space
		return abs(Y - other.Y) <= (tol * 48)
			&& abs(u - other.u) <= (tol *  7)
			&& abs(v - other.v) <= (tol *  6);
	}
};

template <class Manip_pixels, int tol>
struct RGBColor
{
	unsigned int rs, gs, bs;
	RGBColor(unsigned char c, const Manip_pixels& manip)
	{
		unsigned int r, g, b;
		manip.split_source(c, r, g, b);
		rs = 3 * r * r;
		gs = 4 * g * g;
		bs = 2 * b * b;
	}
	// See http://www.compuphase.com/cmetric.htm
	int dist(RGBColor<Manip_pixels, tol> const& other) const
	{
		return abs(rs - other.rs) +
			   abs(gs - other.gs) +
			   abs(bs - other.bs);
	}
	bool equals(RGBColor<Manip_pixels, tol> const& other) const
	{
		return abs(rs - other.rs) <= (tol *  9)
		    && abs(gs - other.gs) <= (tol * 16)
		    && abs(bs - other.bs) <= (tol *  4);
	}
};

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#elif defined __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

/*
Input area naming convention:
     ----------------
     | A1 | B1 | C1 |
-----|----|----|----|-----
| A0 | A  | B  | C  | C4 |
-----|----|----|----|-----
| D0 | D  | E  | F  | F4 |
-----|----|----|----|-----
| G0 | G  | H  | I  | I4 |
-----|----|----|----|-----
     | G5 | G5 | I5 |
	 ----------------
*/

template <class Scaler, class OutputMatrix, class Manip_pixels>
static FORCE_INLINE //perf: quite worth it!
void scalePixel
	(
		              unsigned char a1, unsigned char b1, unsigned char c1,
	unsigned char a0, unsigned char a,  unsigned char b,  unsigned char c, unsigned char c4,
	unsigned char d0, unsigned char d,  unsigned char e,  unsigned char f, unsigned char f4,
	unsigned char g0, unsigned char g,  unsigned char h,  unsigned char i, unsigned char i4,
		              unsigned char g5, unsigned char h5, unsigned char i5,
	OutputMatrix& out, const Manip_pixels& manip
	)
{
	int const weight = 4;
	double const detectSteepWeight = 2.2;
	double const detectDirectionWeight = 3.6;
	
	if (e == h || e == f)
		return;

	RGBColor<Manip_pixels, 2> a1y(a1, manip), b1y(b1, manip), c1y(c1, manip),
	            a0y(a0, manip),  ay(a,  manip),  by(b,  manip),  cy(c,  manip), c4y(c4, manip),
	            d0y(d0, manip),  dy(d,  manip),  ey(e,  manip),  fy(f,  manip), f4y(f4, manip),
	            g0y(g0, manip),  gy(g,  manip),  hy(h,  manip),  iy(i,  manip), i4y(i4, manip),
	                            g5y(g5, manip), h5y(h5, manip), i5y(i5, manip);
	
	const int hf = ey.dist(cy) + ey.dist(gy)  + iy.dist(h5y) + iy.dist(f4y) + weight * hy.dist(fy);
	const int ei = hy.dist(dy) + hy.dist(i5y) + fy.dist(i4y) + fy.dist(by)  + weight * ey.dist(iy);

	unsigned char px = ey.dist(fy) <= ey.dist(hy) ? f : h;

#if 1
	const int dh = d0y.dist(gy) + gy.dist(h5y) + ay.dist(ey) + ey.dist(iy)  + weight * dy.dist(hy);
	const int eg = by.dist(dy)  + dy.dist(g0y) + fy.dist(hy) + hy.dist(g5y) + weight * ey.dist(gy);

	const int ec = dy.dist(by) + by.dist(c1y) + hy.dist(fy)  + fy.dist(c4y) + weight * ey.dist(cy);
	const int bf = ay.dist(ey) + ey.dist(iy)  + b1y.dist(cy) + cy.dist(f4y) + weight * by.dist(fy);

	
	const bool doBlendLine =
		                     // make sure there is no second blending in an adjacent rotation for this pixel
		                     ((eg < dh || ey.equals(dy) || ey.equals(hy)) &&
		                      (ec < bf || ey.equals(by) || ey.equals(fy)) &&
		                      // for L-shapes: blend corner only (handles insular pixels and "mario mushroom eyes")
		                      !(gy.equals(hy) && hy.equals(iy) && iy.equals(fy) && fy.equals(cy) && !ey.equals(iy))) ||
	                         // U-shape
	                         (dy.equals(hy) && hy.equals(fy) && ay.equals(ey) && ey.equals(cy) && !hy.equals(ey)) ||
                             (hy.equals(fy) && fy.equals(by) && gy.equals(ey) && ey.equals(ay) && !fy.equals(ey)) ||
                             detectDirectionWeight * hf < ei;
#else
	const bool doBlendLine = (!fy.equals(by) && !hy.equals(dy)) ||
                             (!ey.equals(iy) && !fy.equals(i4y) && !hy.equals(i5y)) ||
	                         ey.equals(gy) || ey.equals(cy);
#endif
	if (hf < ei)
	{
		if (doBlendLine)
		{
			const int fg = fy.dist(gy);
			const int hc = hy.dist(cy);
			bool ex2 = e != c && b != c;
			bool ex3 = e != g && d != g;

			if (detectSteepWeight * fg <= hc && ex3)
			{
				if (fg >= detectSteepWeight * hc && ex2)
					Scaler::blendLineSteepAndShallow(out, px, manip);
				else
					Scaler::blendLineShallow(out, px, manip);
			}
			else
			{
				if (fg >= detectSteepWeight * hc && ex2)
					Scaler::blendLineSteep(out, px, manip);
				else
					Scaler::blendLineDiagonal(out, px, manip);
			}
		}
		else
			Scaler::blendCorner(out, px, manip);
	}
}

template <class Dest_pixel, class Manip_pixels, class Scaler>
void Scale_xBR
	(
	unsigned char *source,		// ->source pixels.
	int srcx, int srcy,		// Start of rectangle within src.
	int srcw, int srch,		// Dims. of rectangle.
	int sline_pixels,		// Pixels (words)/line for source.
	int sheight,			// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	int dline_pixels,		// Pixels (words)/line for dest.
	const Manip_pixels& manip	// Manipulator methods.
	)
	{
	for (int y = srcy; y < srcy + srch; ++y)
	{
		Dest_pixel *out = dest + Scaler::scale * (y * dline_pixels + srcx);

		unsigned char *sa2 = source + y * sline_pixels + srcx; //center
		unsigned char *sa1 = sa2 - sline_pixels;
		unsigned char *sa0 = sa1 - sline_pixels;
		unsigned char *sa3 = sa2 + sline_pixels;
		unsigned char *sa4 = sa3 + sline_pixels;

		if (y <= 1)
		{
			if (y == 1)
				sa0 = sa1;
			else
				sa0 = sa1 = sa2;
		}
		if (y >= sheight - 2)
		{
			if (y == sheight - 2)
				sa4 = sa3;
			else
				sa4 = sa3 = sa2;
		}

		for (int x = srcx; x < srcx + srcw; ++x, ++sa0, ++sa1, ++sa2, ++sa3, ++sa4,
		     out += Scaler::scale)
		{
			unsigned char a0;
			unsigned char d0;
			unsigned char g0;

			unsigned char a1;
			unsigned char a;
			unsigned char d;
			unsigned char g;
			unsigned char g5;

			//all those bounds checks have only insignificant impact on performance!
			if (x <= 1)
			{
				if (x == 0)
				{
					a0 = sa1[0];
					d0 = sa2[0];
					g0 = sa3[0];

					a1 = sa0[0];
					a  = sa1[0];
					d  = sa2[0];
					g  = sa3[0];
					g5 = sa4[0];
				}
				else
				{
					a0 = sa1[-1];
					d0 = sa2[-1];
					g0 = sa3[-1];

					a1 = sa0[-1];
					a  = sa1[-1];
					d  = sa2[-1];
					g  = sa3[-1];
					g5 = sa4[-1];
				}
			}
			else
			{
				a0 = sa1[-2];
				d0 = sa2[-2];
				g0 = sa3[-2];

				a1 = sa0[-1];
				a  = sa1[-1];
				d  = sa2[-1];
				g  = sa3[-1];
				g5 = sa4[-1];
			}

			unsigned char b1 = sa0[0];
			unsigned char b  = sa1[0];
			unsigned char e  = sa2[0];
			unsigned char h  = sa3[0];
			unsigned char h5 = sa4[0];

			unsigned char c1;
			unsigned char c;
			unsigned char f;
			unsigned char i;
			unsigned char i5;

			unsigned char c4;
			unsigned char f4;
			unsigned char i4;

			if (x >= sline_pixels - 2)
			{
				if (x == sline_pixels - 1)
				{
					c1 = sa0[0];
					c  = sa1[0];
					f  = sa2[0];
					i  = sa3[0];
					i5 = sa4[0];

					c4 = sa1[0];
					f4 = sa2[0];
					i4 = sa3[0];
				}
				else
				{
					c1 = sa0[1];
					c  = sa1[1];
					f  = sa2[1];
					i  = sa3[1];
					i5 = sa4[1];

					c4 = sa1[1];
					f4 = sa2[1];
					i4 = sa3[1];
				}
			}
			else
			{
				c1 = sa0[1];
				c  = sa1[1];
				f  = sa2[1];
				i  = sa3[1];
				i5 = sa4[1];

				c4 = sa1[2];
				f4 = sa2[2];
				i4 = sa3[2];
			}

			//fill block of size n * n with the given color
			fill_block(out, dline_pixels, manip.copy(e), Scaler::scale);
			//perf: no disadvantage compared to hand written assign!

			OutputMatrix<Dest_pixel, Scaler::scale, ROT_0> om0(out, dline_pixels);
			scalePixel<Scaler>(
				    a1, b1, c1,
				a0, a,  b,  c,  c4,
				d0, d,  e,  f,  f4,
				g0, g,  h,  i,  i4,
				    g5, h5, i5,
				om0, manip);

			OutputMatrix<Dest_pixel, Scaler::scale, ROT_90> om90(out, dline_pixels);
			scalePixel<Scaler>(
				    g0, d0, a0,
				g5, g,  d,  a,  a1,
				h5, h,  e,  b,  b1,
				i5, i,  f,  c,  c1,
				    i4, f4, c4,
				om90, manip);

			OutputMatrix<Dest_pixel, Scaler::scale, ROT_180> om180(out, dline_pixels);
			scalePixel<Scaler>(
				    i5, h5, g5,
				i4, i,  h,  g,  g0,
				f4, f,  e,  d,  d0,
				c4, c,  b,  a,  a0,
				    c1, b1, a1,
				om180, manip);

			OutputMatrix<Dest_pixel, Scaler::scale, ROT_270> om270(out, dline_pixels);
			scalePixel<Scaler>(
				    c4, f4, i4,
				c1, c,  f,  i,  i5,
				b1, b,  e,  h,  h5,
				a1, a,  d,  g,  g5,
				    a0, d0, g0,
				om270, manip);
		}
	}
}

class Scaler2xBR;
class Scaler3xBR;
class Scaler4xBR;

#endif //USE_XBR_SCALER

#endif //INCL_SCALE_XBR_H
