/**
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "imagewin.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#  include <cstdlib>
#  include <iostream>
#  include <string>
#endif
#include "exult_types.h"
#include "utils.h"

#include <algorithm>

#include "SDL_video.h"
#include "SDL_error.h"

#ifdef HAVE_OPENGL
#ifdef MACOSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "shapes/glshape.h"
#endif

bool SavePCX_RW (SDL_Surface *saveme, SDL_RWops *dst, bool freedst);

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::cerr;
using std::endl;
using std::exit;
#endif

// This is all the names of the scalers. It needs to match the ScalerType enum
const char *Image_window::ScalerNames[] =  {
		"Point",
		"Interlaced",
		"Bilinear",
		"BilinearPlus",
		"2xSaI",
		"SuperEagle",
		"Super2xSaI",
		"Scale2X",
		"Hq2x",
		"Hq3x",
		"OpenGL",
		0
};

Image_window::ScalerType Image_window::get_scaler_for_name(const std::string &scaler)
{
	std::string sclr = to_uppercase(scaler);

	for (int s = 0; s < NumScalers; s++) {
		std::string sclr2 = to_uppercase(ScalerNames[s]);
		if (sclr == sclr2) return (ScalerType) s;
	}

	return NoScaler;
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
	uses_palette = true;
	show_scaled = 0;
	unscaled_surface = surface = scaled_surface = 0;
#if defined(__zaurus__)
	flags &= ~SDL_FULLSCREEN; // Zaurus would crash in fullscreen mode
#else
	if (try_scaler(w, h, flags)) return; // everyone else can test the try_scaler function
#endif

	if (!surface)			// No scaling, or failed?
		{
		unscaled_surface = surface = SDL_SetVideoMode(w, h, ibuf->depth, flags);
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
 *	Set up surfaces for scaling.
 *	Output:	False if error (reported).
 */

bool Image_window::create_scale_surfaces(int scl, int w, int h, uint32 flags,
	scalefun fun565, scalefun fun555, scalefun fun16, scalefun fun32)
	{
	int hwdepth;

	scale = scl;
	if ( SDL_VideoModeOK(w, h, 32, flags))
		hwdepth = 32;
	else if ( SDL_VideoModeOK(w, h, 16, flags))
		hwdepth = 16;
	else
		hwdepth = Get_best_depth();
	if ((hwdepth != 16 && hwdepth != 32) || ibuf->depth != 8) 
		{
		cout << "Scaling from " << ibuf->depth << "bits to "
				<< hwdepth << " not yet supported." << endl;
		return false;
		}
	else if ((scaled_surface = SDL_SetVideoMode(scale*w, scale*h, 
						hwdepth, flags)) != 0 &&
		 (unscaled_surface = surface = 
			SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
							8, 0, 0, 0, 0)) != 0)
		{			// Get color mask info.
		SDL_PixelFormat *fmt = scaled_surface->format;
		uint32 r = fmt->Rmask, g=fmt->Gmask, b=fmt->Bmask;
		if (hwdepth == 16)
			{
			show_scaled = (r == 0xf800 && g == 0x7e0 &&b == 0x1f) ?
								fun565
			   : (r == 0x7c00 && g == 0x3e0 && b == 0x1f) ? 
								fun555
			   : fun16;
			}
		else
			show_scaled = fun32;
		uses_palette = false;
		return true;
		}
	else
		{
		cout << "Couldn't create scaled surface" << endl;
		delete surface;
		delete scaled_surface;
		surface = scaled_surface = 0;
		return false;
		}
	}

/*
 *	Set up surfaces and scaler to call.
 */

bool Image_window::try_scaler(int w, int h, uint32 flags)
{
	// OpenGL
	if (scaler ==OpenGL)
		{
#ifdef HAVE_OPENGL
					// Get info. about video.
		const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
		if (!vinfo)
			{
			cout << "SDL_GetVideoInfo() failed: " << SDL_GetError()
							<< endl;
			return false;
			}
					// Set up SDL video flags.
		int video_flags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER |
				SDL_HWPALETTE | SDL_RESIZABLE |
				(flags&SDL_FULLSCREEN);
					// Can surface be in video RAM?
		if (vinfo->hw_available)
			video_flags |= SDL_HWSURFACE;
		else
			video_flags |= SDL_SWSURFACE;
		if (vinfo->blit_hw)	// Hardware blits?
			video_flags |= SDL_HWACCEL;
					// Want double-buffering.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
					// Allocate surface.
		int hwdepth = vinfo->vfmt->BitsPerPixel;
					// +++++For now create 8-bit surface
					//   to avoid crashing places we
					//   haven't converted yet.
		if ((scaled_surface = SDL_SetVideoMode(scale*w, scale*h, 
						hwdepth, video_flags)) != 0 &&
		    (unscaled_surface = surface = SDL_CreateRGBSurface(
					SDL_SWSURFACE, w, h,
						8, 0, 0, 0, 0)) != 0)
			{
			show_scaled = &Image_window::show_scaledOpenGL;
			}
		else
			{
			cerr << "Couldn't allocate surface: " << 
					SDL_GetError() << endl;
			delete surface;
			delete scaled_surface;
			surface = scaled_surface = 0;
			}
#else
		cerr << "OpenGL not supported" << endl;
#endif
		}
	// 2xSaI scaler
	else if (scale >= 2 && scaler ==  SaI)
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_2xSaI,
				&Image_window::show_scaled8to555_2xSaI,
				&Image_window::show_scaled8to16_2xSaI,
				&Image_window::show_scaled8to32_2xSaI);
	}
	else if (scale >= 2 && scaler == bilinear)	// Bilinear scaler
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_bilinear,
				&Image_window::show_scaled8to555_bilinear,
				&Image_window::show_scaled8to16_bilinear,
				&Image_window::show_scaled8to32_bilinear);
	}
	else if (scale >= 2 && scaler == BilinearPlus)	// Bilinear Plus scaler
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_BilinearPlus,
				&Image_window::show_scaled8to555_BilinearPlus,
				&Image_window::show_scaled8to16_BilinearPlus,
				&Image_window::show_scaled8to32_BilinearPlus);
	}
	else if (scale >= 2 && scaler == SuperEagle)
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_SuperEagle,
				&Image_window::show_scaled8to555_SuperEagle,
				&Image_window::show_scaled8to16_SuperEagle,
				&Image_window::show_scaled8to32_SuperEagle);
	}
	else if (scale >= 2 && scaler == Super2xSaI)
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_Super2xSaI,
				&Image_window::show_scaled8to555_Super2xSaI,
				&Image_window::show_scaled8to16_Super2xSaI,
				&Image_window::show_scaled8to32_Super2xSaI);
	}
	else if (scale == 3 && scaler == Hq3x)
	{
		create_scale_surfaces(3, w, h, flags, 
				&Image_window::show_scaled8to565_Hq3x,
				&Image_window::show_scaled8to555_Hq3x,
				&Image_window::show_scaled8to16_Hq3x,
				&Image_window::show_scaled8to32_Hq3x);
	}
	else if (scale >= 2 && (scaler == Hq2x || scaler == Hq3x))
	{
		create_scale_surfaces(2, w, h, flags, 
				&Image_window::show_scaled8to565_Hq2x,
				&Image_window::show_scaled8to555_Hq2x,
				&Image_window::show_scaled8to16_Hq2x,
				&Image_window::show_scaled8to32_Hq2x);
	}
	else if (scale >= 2 && scaler == interlaced)
	{
		surface = SDL_SetVideoMode(w*scale, h*scale, ibuf->depth, flags);
		unscaled_surface = scaled_surface = 
				SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
							8, 0, 0, 0, 0);
		if (surface && scaled_surface)
		{
			show_scaled = &Image_window::show_scaled_interlace;
			ibuf->bits = (unsigned char *) scaled_surface->pixels;
					// Update line size in words.
			ibuf->line_width = scaled_surface->pitch/ibuf->pixel_size;
			return true;
		}
		else
		{
			cout << "Couldn't create 8bit scaled surface" << endl;
			delete surface;
			delete scaled_surface;
			surface = scaled_surface = 0;
		}
	}
	else if (scale >= 2 && scaler == Scale2x)
	{
		surface = SDL_SetVideoMode(w*scale, h*scale, ibuf->depth, flags);
		unscaled_surface = scaled_surface = 
				SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
							8, 0, 0, 0, 0);
		if (surface && scaled_surface)
		{
			show_scaled = &Image_window::show_scale2x_noblur;
			ibuf->bits = (unsigned char *) scaled_surface->pixels;
					// Update line size in words.
			ibuf->line_width = scaled_surface->pitch/ibuf->pixel_size;
			return true;
		}
		else
		{
			cout << "Couldn't create 8bit scaled surface" << endl;
			delete surface;
			delete scaled_surface;
			surface = scaled_surface = 0;
		}
	}
	else if (scale >= 2)
	{
		surface = SDL_SetVideoMode(w*scale, h*scale, ibuf->depth, flags);
		unscaled_surface = scaled_surface = SDL_CreateRGBSurface(
				SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
		if (surface && scaled_surface)
		{
			show_scaled = &Image_window::show_scaled_point;
			ibuf->bits = (unsigned char *) scaled_surface->pixels;
					// Update line size in words.
			ibuf->line_width = scaled_surface->pitch/ibuf->pixel_size;
			return true;
		}
		else
		{
			cout << "Couldn't create 8bit scaled surface" << endl;
			delete surface;
			delete scaled_surface;
			surface = scaled_surface = 0;
		}
	}

	return false;
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
	int newsc,
	int newscaler
	)
	{
	if (surface)
		{
		if (neww == ibuf->width && newh == ibuf->height && newsc == scale && scaler == newscaler)
			return;		// Nothing changed.
		free_surface();		// Delete old image.
		}
	scale = newsc;
	scaler = newscaler;
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

	SDL_Surface *surf = scaled_surface&&surface==unscaled_surface?scaled_surface:surface;

        w = unscaled_surface->w;
        h = unscaled_surface->h;
        bpp = surf->format->BitsPerPixel;

        if ( fullscreen ) {
		cout << "Switching to windowed mode."<<endl;
                flags = surf->flags & ~SDL_FULLSCREEN;
        } else {
		cout << "Switching to fullscreen mode."<<endl;
                flags = surf->flags | SDL_FULLSCREEN;
        }
					// First see if it's allowed.
        if ( SDL_VideoModeOK(w, h, bpp, flags) )
		{
		free_surface();		// Delete old.
		fullscreen = !fullscreen;
		create_surface(w, h);	// Create new.
		}
}

bool Image_window::screenshot(SDL_RWops *dst)
{
	if (!surface) return false;
#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())
		{
		int width = ibuf->width, height = ibuf->height;
		GL_manager *glman = GL_manager::get_instance();
		unsigned char *bits = glman->get_unscaled_bits(width, height, false);
		SDL_Surface *screenshot_surface = SDL_CreateRGBSurfaceFrom(bits,
					width, height, 24, 3 * width, 0, 0, 0, 0);
		bool ret = SavePCX_RW(screenshot_surface, dst, true);
		SDL_FreeSurface(screenshot_surface);
		delete [] bits;
		return ret;
		}
#endif
	return SavePCX_RW(unscaled_surface, dst, true);
}

void Image_window::set_title(const char *title)
{
	SDL_WM_SetCaption(title, 0);
}

#ifdef HAVE_OPENGL
/*
 *	Fill a rectangle with an 8-bit value.
 */

void Image_window::opengl_fill8
	(
	unsigned char pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	SDL_Color *colors = surface->format->palette->colors;
	SDL_Color& color = colors[pix];
	glDisable(GL_TEXTURE_2D);	// Disable texture-mapping.
	glPushMatrix();
	int x = destx;			// Left edge.
	int y = -(desty + srch);
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);
		{
		glColor3ub(color.r, color.g, color.b);
		glVertex3i(0, 0, 0);
		glVertex3i(srcw, 0, 0);
		glVertex3i(srcw, srch, 0);
		glVertex3i(0, srch, 0);
		}
	glEnd();
	glPopMatrix();
	}

/*
 *	Apply a translucency table to a rectangle.
 */

void Image_window::opengl_fill_translucent8
	(
	unsigned char /* val */,	// Not used.
	int srcw, int srch,
	int destx, int desty,
	Xform_palette& xform		// Transform table.
	)
	{
	glDisable(GL_TEXTURE_2D);	// Disable texture-mapping.
	int x = destx;			// Left edge.
	int y = -(desty + srch);
	glBegin(GL_QUADS);
		{
		glColor4ub(xform.r, xform.g, xform.b, xform.a);
		glVertex2i(x, y);
		glVertex2i(x + srcw, y);
		glVertex2i(x + srcw, y + srch);
		glVertex2i(x, y + srch);
		}
	glEnd();
	}

#endif

