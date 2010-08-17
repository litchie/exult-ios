/*
 *	Copyright (C) 2009 Exult Team
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

#include "imagewin.h"
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "exult_types.h"

#include "manip.h"
#include "scale_2xSaI.h"


//
// 2xSaI Filtering
//
void Image_window::show_scaled8to16_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_2xSaI<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_2xSaI<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_2xSaI<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_2xSaI<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) display_surface->pixels,
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}


//
// Super2xSaI Filtering
//
void Image_window::show_scaled8to16_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Super2xSaI<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_Super2xSaI
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_Super2xSaI<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) display_surface->pixels,
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}


//
// SuperEagle Filtering
//
void Image_window::show_scaled8to16_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to16 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_SuperEagle<unsigned char, uint16, Manip8to16>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to555_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to555 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_SuperEagle<unsigned char, uint16, Manip8to555>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to565_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to565 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_SuperEagle<unsigned char, uint16, Manip8to565>
		(ibuf->get_bits(), x, y, w, h,
		    ibuf->line_width, ibuf->height, 
		    (uint16 *) display_surface->pixels, 
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
			manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}

void Image_window::show_scaled8to32_SuperEagle
	(
	int x, int y, int w, int h	// Area to show.
	)
	{
	increase_area(x,y,w,h, 2,2,1,1, ibuf->width, ibuf->height);

	Manip8to32 manip(paletted_surface->format->palette->colors,
						display_surface->format);
	Scale_SuperEagle<unsigned char, uint32, Manip8to32>
		(ibuf->get_bits(), x, y, w, h,
			ibuf->line_width, ibuf->height, 
			(uint32 *) display_surface->pixels,
			display_surface->pitch/
				display_surface->format->BytesPerPixel,
								manip);
	SDL_UpdateRect(display_surface, 2*x, 2*y, 2*w, 2*h);
	}
