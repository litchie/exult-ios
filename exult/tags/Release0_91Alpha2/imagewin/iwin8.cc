/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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


using std::memmove;

GammaTable<uint8> Image_window8::GammaRed(256);
GammaTable<uint8> Image_window8::GammaBlue(256);
GammaTable<uint8> Image_window8::GammaGreen(256);

Image_window8::Image_window8(unsigned int w, unsigned int h, 
				int scl, bool fs)
	: Image_window(new Image_buffer8(w, h, (Image_buffer *) 0), 
	  scl, fs)
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
	}
}

