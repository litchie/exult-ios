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
#include <string.h>
#include <iostream.h>
#include <string>
#include <Configuration.h>
extern	Configuration *config;

/*
 *	Create buffer.
 */

Image_buffer_base::Image_buffer_base
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Depth (bits/pixel).
	) : width(w), height(h), depth(dpth), bits(0), line_width(w),
	    clipx(0), clipy(0),
	    clipw(w), cliph(h)
	{
	switch (depth)			// What depth?
		{
	case 8: 
		pixel_size = 1; break;
	case 15:
	case 16:
		pixel_size = 2; break;
	case 32:
		pixel_size = 4; break;
		}
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
	unsigned long c = (((unsigned long) val)*brightness*255L)/
							(100*(maxval + 1));
	return (c <= 255L ? (unsigned short) c : 255);
	}

/*
 *	Set palette.
 */

void Image_buffer8::set_palette
	(
	SDL_Surface *surface,		// Surface to set.
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 256; i++)
		{
		colors[i].r = Get_color8(rgbs[3*i], maxval, brightness);
		colors[i].g = Get_color8(rgbs[3*i + 1], maxval,
							brightness);
		colors[i].b = Get_color8(rgbs[3*i + 2], maxval,
							brightness);
		}
	SDL_SetColors(surface, colors, 0, 256);
	}

/*
 *	Rotate a range of colors.
 */

void Image_buffer8::rotate_colors
	(
	SDL_Surface *surface,		// Surface to set.
	int first,			// Palette index of 1st.
	int num,			// # in range.
	int upd				// 1 to update hardware palette.
	)
	{
	int cnt = num - 1;		// Shift downward.
	SDL_Color c0 = colors[first+num-1];
	memmove(colors+first+1,colors+first,sizeof(SDL_Color)*cnt);
	colors[first ] = c0;	// Shift 1st to end.
	if (upd)			// Take effect now?
		SDL_SetColors(surface, colors, 0, 256);
	}

/*
 *	Copy an area of the image within itself.
 */

void Image_buffer8::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	unsigned char *to = bits + yto*line_width + destx;
	unsigned char *from = bits + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Get a rectangle from here into another Image_buffer.
 */

void Image_buffer8::get
	(
	Image_buffer *dest,		// Copy to here.
	int srcx, int srcy		// Upper-left corner of source rect.
	)
	{
	int srcw = dest->get_width(), srch = dest->get_height();
	int destx = 0, desty = 0;
					// Constrain to window's space. (Note
					//   convoluted use of clip().)
	if (!clip(destx, desty, srcw, srch, srcx, srcy))
		return;
	Image_buffer_base *dbuf = dest->get_ibuf();
	unsigned char *to = (unsigned char *) dbuf->bits + 
				desty*dbuf->line_width + destx;
	unsigned char *from = (unsigned char *) bits + srcy*line_width + srcx;
					// Figure # pixels to next line.
	int to_next = dbuf->line_width - srcw;
	int from_next = line_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Retrieve data from another buffer.
 */

void Image_buffer8::put
	(
	Image_buffer *src,		// Copy from here.
	int destx, int desty		// Copy to here.
	)
	{
	Image_buffer8::copy8((unsigned char *) src->get_ibuf()->bits,
		src->get_width(), src->get_height(), destx, desty);
	}

/*
 *	Create a compatible buffer.
 */

Image_buffer *Image_buffer::create_buffer
	(
	int w, int h			// Dimensions.
	)
	{
	Image_buffer *newbuf = new Image_buffer(w, h, ibuf->depth);
	newbuf->ibuf->bits = new unsigned char[w*h*ibuf->pixel_size];
	return (newbuf);
	}

/*
 *	Fill with a given 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix
	)
	{
	unsigned char *pixels = bits;
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with an 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Fill a line with a given 8-bit value.
 */

void Image_buffer8::fill_line8
	(
	unsigned char pix,
	int srcw,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	memset(pixels, pix, srcw);
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer8::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy a line into this buffer.
 */

void Image_buffer8::copy_line8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	memcpy(to, from, srcw);
	}

/*
 *	Copy a line into this buffer where some of the colors are translucent.
 */

void Image_buffer8::copy_line_translucent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty,
	int first_translucent,		// Palette index of 1st trans. color.
	int last_translucent,		// Index of last trans. color.
	Xform_palette *xforms		// Transformers.  Need same # as
					//   (last_translucent - 
					//    first_translucent + 1).
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = (unsigned char *) bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	for (int i = srcw; i; i--)
		{
					// Get char., and transform.
		unsigned char c = *from++;
		if (c >= first_translucent && c <= last_translucent)
					// Use table to shift existing pixel.
			c = xforms[c - first_translucent][*to];
		*to++ = c;
		}
	}

/*
 *	Apply a translucency table to a line.
 */

void Image_buffer8::fill_line_translucent8
	(
	unsigned char val,		// Ignored for this method.
	int srcw,
	int destx, int desty,
	Xform_palette xform		// Transform table.
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	while (srcw--)
		{
		*pixels = xform[*pixels];
		pixels++;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer8::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = chr;
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create a default palette for 8-bit graphics on a 16-bit display.
 */

void Image_buffer16::create_default_palette
	(
	)
	{
	delete palette;			// Delete old.
	palette = new unsigned short[256];
	for (int i = 0; i < 256; i++)
		palette[i] = rgb(4*((i >> 5) & 0x7), 
			4*((i >> 2) & 0x7), 9*(i & 0x3));
	}

/*
 *	Fill with a given 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix
	)
	{
	unsigned short *pixels = get_pixels();
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with a 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Fill a line with a given 16-bit value.
 */

void Image_buffer16::fill_line16
	(
	unsigned short pix,
	int srcw,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	for (int cnt = srcw; cnt; cnt--)
		*pixels++ = pix;
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy16
	(
	unsigned short *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned short *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy an area of the image within itself.
 */

void Image_buffer16::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	unsigned short *to = get_pixels() + yto*line_width + destx;
	unsigned short *from = get_pixels() + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw * 2);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Get a rectangle from here into another Image_buffer.
 */

void Image_buffer16::get
	(
	Image_buffer *dest,		// Copy to here.
	int srcx, int srcy		// Upper-left corner of source rect.
	)
	{
	int srcw = dest->get_width(), srch = dest->get_height();
	int destx = 0, desty = 0;
					// Constrain to window's space. (Note
					//   convoluted use of clip().)
	if (!clip(destx, desty, srcw, srch, srcx, srcy))
		return;
	Image_buffer_base *dbuf = dest->get_ibuf();
	unsigned short *to = (unsigned short *) dbuf->bits + 
				desty*dbuf->line_width + destx;
	unsigned short *from = get_pixels() + srcy*line_width + srcx;
					// Figure # pixels to next line.
	int to_next = dbuf->line_width - srcw;
	int from_next = line_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Retrieve data from another buffer.
 */

void Image_buffer16::put
	(
	Image_buffer *src,		// Copy from here.
	int destx, int desty		// Copy to here.
	)
	{
	Image_buffer16::copy16((unsigned short *) src->get_ibuf()->bits,
		src->get_width(), src->get_height(), destx, desty);
	}

/*
 *	Convert rgb value.
 */

inline unsigned char Get_color16
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned int c = (((unsigned int) val)*brightness*32)/
							(100*(maxval + 1));
	return (c < 32 ? c : 31);
	}

/*
 *	Set palette.
 */

void Image_buffer16::set_palette
	(
	SDL_Surface *surface,		// Surface to set (ignored).
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 3*256; i += 3)
		{
		unsigned char r = Get_color16(rgbs[i], maxval, brightness);
		unsigned char g = Get_color16(rgbs[i + 1], maxval, brightness);
		unsigned char b = Get_color16(rgbs[i + 2], maxval, brightness);
		set_palette_color(i/3, r, g, b);
		}
	}

/*
 *	Rotate a range of colors.
 */

void Image_buffer16::rotate_colors
	(
	SDL_Surface *surface,		// Surface to set (ignored).
	int first,			// Palette index of 1st.
	int num,			// # in range.
	int upd				// 1 to update hardware now.
	)
	{
	int cnt = num - 1;		// Shift downward.
	int c0 = palette[first];
	for (int i = first; cnt; i++, cnt--)
		palette[i] = palette[i + 1];
	palette[first + num - 1] = c0;	// Shift 1st to end.
					// +++++upd?
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = palette[*from++];
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy a line into this buffer.
 */

void Image_buffer16::copy_line8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw,			// Width to copy.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0, srch = 1;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcx;
	for (int cnt = srcw; cnt; cnt--)
		*to++ = palette[*from++];
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer16::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = palette[chr];
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create top-level image buffer.
 */

Image_buffer::Image_buffer
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Color depth (bits/pixel).
	)
	{
#if 1	/* +++++Let's see what SDL can do. */
	ibuf = new Image_buffer8(w, h, this);
#else
	switch (dpth)			// What depth?
		{
	case 8: 
		ibuf = new Image_buffer8(w, h, this);
		break;
	case 15:
	case 16:
		ibuf = new Image_buffer16(w, h, dpth);
		break;
	case 32:
		cout << "Ouch. No 32 bit support. Try a 16 bit or an 8 bit display" << endl;
		exit(-1);
	default:
		ibuf = 0;		// Later.
		}
#endif
	}

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

/*
 *	Create window.
 */

Image_window::Image_window
	(
	unsigned int w,			// Desired width, height.
	unsigned int h
	) : Image_buffer(w, h, Get_best_depth()),
	    surface(0)
	{
	create_surface(w, h);
	}

/*
 *	Destroy window.
 */

Image_window::~Image_window
	(
	)
	{
	free_surface();
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
	string	fullscreenstr;
	bool	fullscreen=false;
	config->value("config/video/fullscreen",fullscreenstr,"no");
	if(fullscreenstr=="yes")
		fullscreen=true;
	config->set("config/video/fullscreen",fullscreenstr,true);
	ibuf->width = w;
	ibuf->height = h;
	surface = SDL_SetVideoMode(w, h, ibuf->depth, (fullscreen?SDL_FULLSCREEN:0) |
					SDL_SWSURFACE |  SDL_HWPALETTE);
	if (!surface)
		{
		cout << "Couldn't set video mode (" << w << ", " << h <<
			") at " << ibuf->depth << " bpp depth: " <<
			SDL_GetError() << '\n';
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
	}

/*
 *	Window was resized.
 */

void Image_window::resized
	(
	unsigned int neww, 
	unsigned int newh
	)
	{
	if (surface)
		{
		if (neww == ibuf->width && newh == ibuf->height)
			return;		// Nothing changed.
		free_surface();		// Delete old image.
		}
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
	SDL_UpdateRect(surface, 0, 0, ibuf->width, ibuf->height);
	}

void Image_window::toggle_fullscreen() {
        Uint32 flags;
        int w, h, bpp;

        w = surface->w;
        h = surface->h;
        bpp = surface->format->BitsPerPixel;
        if ( surface->flags & SDL_FULLSCREEN ) {
		cout << "Switching to windowed mode.\n";
                flags = surface->flags & ~SDL_FULLSCREEN;
        } else {
		cout << "Switching to fullcsreen mode.\n";
                flags = surface->flags | SDL_FULLSCREEN;
        }
        if ( SDL_VideoModeOK(w, h, bpp, flags) ) {
                /* This changes the publicly visible surface */
                surface = SDL_SetVideoMode(w, h, bpp, flags);
                if ( surface == NULL ) {
           	        /* Uh oh... */;
                }	
					// Point to new buffer.
		ibuf->bits = (unsigned char *) surface->pixels;
					// Update line size in words.
		ibuf->line_width = surface->pitch/ibuf->pixel_size;

		/* Redraw surface contents... */
	
		// show();  // Why doesn't this work??
        }
}
