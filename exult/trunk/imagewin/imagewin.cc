/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Imagewin.cc - A window to blit images into.
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

#include "imagewin.h"
#ifdef __DECCXX
#  include "alpha_kludges.h"
#else
#  include <cstring>
#  include <cstdlib>
#  include <iostream>
#  include <string>
#endif
#ifdef MACOS
#  include "exult_types.h"
#else
#  include "../exult_types.h"
#endif

using std::cout;
using std::endl;
using std::exit;

/*
 *	Get best depth.  Returns bits/pixel.
 */
static int Get_best_depth
	(
	)
	{
					// Get video info.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
					// The "best" format:
	return (vinfo->vfmt->BitsPerPixel);
	}

#if 0
/*
 *	Create window with best color depth to match screen.
 */

Image_window::Image_window
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	bool fs				// Fullscreen.
	) : Image_buffer(w, h, Get_best_depth()),
	    scale(1), fullscreen(fs),
	    surface(0), scaled_surface(0), show_scaled(0)
	{
	create_surface(w, h);
	}
#endif

/*
 *	Destroy window.
 */

Image_window::~Image_window
	(
	)
	{
	free_surface();
	delete ibuf;
	}

/*
 *	Create the surface.
 */

void Image_window::create_surface
	(
	unsigned int w, 
	unsigned int h
	)
	{
	ibuf->width = w;
	ibuf->height = h;
	Uint32 flags = (fullscreen?SDL_FULLSCREEN:0) |
					SDL_SWSURFACE |  SDL_HWPALETTE;
	show_scaled = 0;
	surface = scaled_surface = 0;
	if (scale == 2)			// We support 2X scaling.
		{
		int hwdepth = Get_best_depth();
		if ((hwdepth != 16 && hwdepth != 32) || ibuf->depth != 8)
			cout << "Doubling from " << ibuf->depth << "bits to "
				<< hwdepth << " not yet supported." << endl;
		else if ((scaled_surface = SDL_SetVideoMode(2*w, 2*h, 
						hwdepth, flags)) != 0 &&
			 (surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
							8, 0, 0, 0, 0)) != 0)
			{			// Get color mask info.
			SDL_PixelFormat *fmt = scaled_surface->format;
			uint32 r = fmt->Rmask, g=fmt->Gmask, 
								b=fmt->Bmask;
			if (hwdepth == 16)
				show_scaled = 
				     (r == 0xf800 && g == 0x7e0 &&b == 0x1f) ? 
					&Image_window::show_scaled8to565
				   : (r == 0x7c00 && g == 0x3e0 && b == 0x1f) ?
					&Image_window::show_scaled8to555
				   : &Image_window::show_scaled8to16;
			else
			    	show_scaled = &Image_window::show_scaled8to32;
			}
		else
			{
			cout << "Couldn't create scaled surface" << endl;
			delete surface;
			delete scaled_surface;
			surface = scaled_surface = 0;
			}
		}
	if (!surface)			// No scaling, or failed?
		{
		surface = SDL_SetVideoMode(w, h, ibuf->depth, flags);
		scale = 1;
		}
	if (!surface)
		{
		cout << "Couldn't set video mode (" << w << ", " << h <<
			") at " << ibuf->depth << " bpp depth: " <<
			SDL_GetError() << endl;
		exit(-1);
		}
	ibuf->bits = (unsigned char *) surface->pixels;
					// Update line size in words.
	ibuf->line_width = surface->pitch/ibuf->pixel_size;
	}

/*
 *	Free the surface.
 */

void Image_window::free_surface
	(
	)
	{
	if (!surface)
		return;
	SDL_FreeSurface(surface);
	ibuf->bits = 0;
	surface = 0;
	if (scaled_surface)
		{
		SDL_FreeSurface(scaled_surface);
		scaled_surface = 0;
		}
	}

/*
 *	Create a compatible buffer.
 */

Image_buffer *Image_window::create_buffer
	(
	int w, int h			// Dimensions.
	)
	{
	Image_buffer *newbuf = ibuf->create_another(w, h);
	return (newbuf);
	}

/*
 *	Window was resized.
 */

void Image_window::resized
	(
	unsigned int neww, 
	unsigned int newh,
	int newsc
	)
	{
	if (surface)
		{
		if (neww == ibuf->width && newh == ibuf->height && newsc == scale)
			return;		// Nothing changed.
		free_surface();		// Delete old image.
		}
	scale = newsc;
	create_surface(neww, newh);	// Create new one.
	}

/*
 *	Repaint window.
 */

void Image_window::show
	(
	)
	{
	if (!ready())
		return;
	if (show_scaled)		// 2X scaling?
		(this->*show_scaled)(0, 0, ibuf->width, ibuf->height);
	else
		SDL_UpdateRect(surface, 0, 0, ibuf->width, ibuf->height);
	}

/*
 *	Repaint portion of window.
 */

void Image_window::show
	(
	int x, int y, int w, int h
	)
	{
	if (!ready())
		return;
	int srcx = 0, srcy = 0;
	if (!ibuf->clip(srcx, srcy, w, h, x, y))
		return;
	if (show_scaled)		// 2X scaling?
		(this->*show_scaled)(x, y, w, h);
	else
		SDL_UpdateRect(surface, x, y, w, h);
	}


/*
 *	Toggle fullscreen.
 */
void Image_window::toggle_fullscreen() {
        Uint32 flags;
        int w, h, bpp;

        w = surface->w;
        h = surface->h;
        bpp = scaled_surface ? scaled_surface->format->BitsPerPixel
			     : surface->format->BitsPerPixel;
        if ( fullscreen ) {
		cout << "Switching to windowed mode."<<endl;
                flags = surface->flags & ~SDL_FULLSCREEN;
        } else {
		cout << "Switching to fullcsreen mode."<<endl;
                flags = surface->flags | SDL_FULLSCREEN;
        }
					// First see if it's allowed.
        if ( SDL_VideoModeOK(w, h, bpp, flags) )
		{
		free_surface();		// Delete old.
		fullscreen = !fullscreen;
		create_surface(w, h);	// Create new.
		}
}

