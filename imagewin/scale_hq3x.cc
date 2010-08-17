/*
*	Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
*
*	Adapted for Exult: 4/7/07 - JSF
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Library General Public
*	License as published by the Free Software Foundation; either
*	version 2 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Library General Public License for more details.
*
*	You should have received a copy of the GNU Library General Public
*	License along with this library; if not, write to the
*	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*	Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_video.h"

#include "imagewin.h"
#include <cstdlib>
#include <cstring>

#include "exult_types.h"

#include "manip.h"
#include "scale_hqnx.h"
#include "scale_hq3x.h"

//
// Hq3x Filtering
//
void Image_window::show_scaled8to16_Hq3x
(
 int x, int y, int w, int h	// Area to show.
 )
{
	if (scale == 2) {
		show_scaled8to16_Hq2x(x,y,w,h);
		return;
	}

	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(paletted_surface->format->palette->colors,
		display_surface->format);
	Scale_Hq3x<uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		ibuf->line_width, ibuf->height, 
		(uint16 *) display_surface->pixels, 
		display_surface->pitch/
		display_surface->format->BytesPerPixel,
		manip);
	SDL_UpdateRect(display_surface, 3*x, 3*y, 3*w, 3*h);
}

void Image_window::show_scaled8to555_Hq3x
(
 int x, int y, int w, int h	// Area to show.
 )
{
	if (scale == 2) {
		show_scaled8to555_Hq2x(x,y,w,h);
		return;
	}

	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Hq3x<uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		ibuf->line_width, ibuf->height, 
		(uint16 *) display_surface->pixels, 
		display_surface->pitch/
		display_surface->format->BytesPerPixel,
		manip);
	SDL_UpdateRect(display_surface, 3*x, 3*y, 3*w, 3*h);
}

void Image_window::show_scaled8to565_Hq3x
(
 int x, int y, int w, int h	// Area to show.
 )
{
	if (scale == 2) {
		show_scaled8to565_Hq2x(x,y,w,h);
		return;
	}

	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Hq3x<uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		ibuf->line_width, ibuf->height, 
		(uint16 *) display_surface->pixels, 
		display_surface->pitch/
		display_surface->format->BytesPerPixel,
		manip);
	SDL_UpdateRect(display_surface, 3*x, 3*y, 3*w, 3*h);
}

void Image_window::show_scaled8to32_Hq3x
(
 int x, int y, int w, int h	// Area to show.
 )
{
	if (scale == 2) {
		show_scaled8to32_Hq2x(x,y,w,h);
		return;
	}

	increase_area(x,y,w,h, 1,1,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(paletted_surface->format->palette->colors,
		display_surface->format);
	Scale_Hq3x<uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
		ibuf->line_width, ibuf->height, 
		(uint32 *) display_surface->pixels,
		display_surface->pitch/
		display_surface->format->BytesPerPixel,
		manip);
	SDL_UpdateRect(display_surface, 3*x, 3*y, 3*w, 3*h);
}
