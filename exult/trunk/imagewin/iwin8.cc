/**
 **	Iwin8.h - 8-bit image window.
 **
 **	Written: 8/13/98 - JSF
 **/

/*
Copyright (C) 1998 Jeffrey S. Freedman

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "SDL_video.h"
#include "iwin8.h"
#include "exult_types.h"
#include "gamma.h"
#include <limits.h>

#ifndef UNDER_CE
using std::memmove;
#endif

GammaTable<uint8> Image_window8::GammaRed(256);
GammaTable<uint8> Image_window8::GammaBlue(256);
GammaTable<uint8> Image_window8::GammaGreen(256);

Image_window8::Image_window8(unsigned int w, unsigned int h, 
				int scl, bool fs, int sclr)
	: Image_window(new Image_buffer8(w, h, (Image_buffer *) 0), 
	  scl, fs, sclr)
{
	ib8 = (Image_buffer8 *) ibuf;
}

Image_window8::~Image_window8()
{
}


void Image_window8::get_gamma (float &r, float &g, float &b)
{
	r = GammaRed.get_gamma();
	g = GammaGreen.get_gamma();
	b = GammaBlue.get_gamma();
}

void Image_window8::set_gamma (float r, float g, float b)
{
	GammaRed.set_gamma(r);
	GammaGreen.set_gamma(g);
	GammaBlue.set_gamma(b);
}

/*
 *	Convert rgb value.
 */

inline unsigned short Get_color8
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	uint32 c = (((uint32) val)*brightness*255L)/
							(100*maxval);
	return (c <= 255L ? (unsigned short) c : 255);
	}

/*
 *	Set palette.
 */

void Image_window8::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
{
					// Get the colors.
	SDL_Color colors2[256];
	for (int i = 0; i < 256; i++)
	{
		colors2[i].r = colors[i*3] = GammaRed[Get_color8(rgbs[3*i], maxval, brightness)];
		colors2[i].g = colors[i*3+1]  = GammaGreen[Get_color8(rgbs[3*i + 1], maxval, brightness)];
		colors2[i].b = colors[i*3+2]  = GammaBlue[Get_color8(rgbs[3*i + 2], maxval, brightness)];
	}
	SDL_SetColors(surface, colors2, 0, 256);

	if (surface != unscaled_surface)
		SDL_SetColors(unscaled_surface, colors2, 0, 256);
}

/*
 *	Rotate a range of colors.
 */

void Image_window8::rotate_colors
	(
	int first,			// Palette index of 1st.
	int num,			// # in range.
	int upd				// 1 to update hardware palette.
	)
{
	first *= 3;
	num *= 3;
	int cnt = num - 3;		// Shift downward.
	unsigned char c0 = colors[first+num-3];
	unsigned char c1 = colors[first+num-2];
	unsigned char c2 = colors[first+num-1];
	memmove(colors+first+3,colors+first,cnt);
	colors[first ] = c0;	// Shift 1st to end.
	colors[first+1] = c1;	// Shift 1st to end.
	colors[first+2] = c2;	// Shift 1st to end.
	if (upd)			// Take effect now?
	{
		SDL_Color colors2[256];
		for (int i = 0; i < 256; i++)
		{
			colors2[i].r = colors[i*3];
			colors2[i].g = colors[i*3+1];
			colors2[i].b = colors[i*3+2];
		}
		SDL_SetColors(surface, colors2, 0, 256);

		if (surface != unscaled_surface)
			SDL_SetColors(unscaled_surface, colors2, 0, 256);
	}
}

//a nearest-average-colour 1/3 scaler
unsigned char* Image_window8::mini_screenshot()
{
	int i;
	if (!surface) return 0;

	unsigned char* pixels = ibuf->get_bits();
	int pitch = ibuf->get_line_width();
	unsigned char* buf = new Uint8[96*60];
	const int w = 3*96, h = 3*60;

	for (int y = 0; y < h; y+=3)
		for (int x = 0; x < w; x+=3)
#if 0
			buf[y*w/9 + x/3] = pixels[
				pitch * (y + (get_height()-h)/2) +
				x + (get_width()-w)/2 ];
#else
		{
			//calculate average colour
			int r=0, g=0, b=0;
			for (i=0; i<3; i++)
				for (int j=0; j<3; j++) {
					r+=colors[0 + 3*pixels[
				pitch * (j + y + (get_height()-h)/2) +
				i + x + (get_width()-w)/2 ]];
					g+=colors[1 + 3*pixels[
				pitch * (j + y + (get_height()-h)/2) +
				i + x + (get_width()-w)/2 ]];
					b+=colors[2 + 3*pixels[
				pitch * (j + y + (get_height()-h)/2) +
				i + x + (get_width()-w)/2 ]];
				}
			r = r/9; g = g/9; b = b/9;

			//find nearest-colour in non-rotating palette
			int bestdist = INT_MAX, bestindex = -1;
			for (i=0; i<224; i++) {
				int dist = (colors[0+3*i]-r)*(colors[0+3*i]-r)+
					(colors[1+3*i]-g)*(colors[1+3*i]-g)+
					(colors[2+3*i]-b)*(colors[2+3*i]-b);
				if (dist < bestdist) {
					bestdist = dist;
					bestindex = i;
				}
			}
			buf[y*w/9 + x/3] = bestindex;
		}
				
#endif

	return buf;
}
