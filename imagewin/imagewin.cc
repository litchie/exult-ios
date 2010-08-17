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

#include "istring.h"

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

#define SCALE_BIT(factor) (1<<((factor)-1))

const Image_window::ScalerType	Image_window::NoScaler(-1);
const Image_window::ScalerConst	Image_window::point("Point");
const Image_window::ScalerConst	Image_window::interlaced("Interlaced");
const Image_window::ScalerConst	Image_window::bilinear("Bilinear");
const Image_window::ScalerConst	Image_window::BilinearPlus("BilinearPlus");
const Image_window::ScalerConst	Image_window::SaI("2xSaI");
const Image_window::ScalerConst	Image_window::SuperEagle("SuperEagle");
const Image_window::ScalerConst	Image_window::Super2xSaI("Super2xSaI");
const Image_window::ScalerConst	Image_window::Scale2x("Scale2x");
const Image_window::ScalerConst	Image_window::Hq2x("Hq2X");
const Image_window::ScalerConst	Image_window::Hq3x("Hq3x");
const Image_window::ScalerConst	Image_window::OpenGL("OpenGL");
const Image_window::ScalerConst	Image_window::NumScalers(0);

Image_window::ScalerVector Image_window::p_scalers;
const Image_window::ScalerVector &Image_window::Scalers = Image_window::p_scalers;

// Constructor for the ScalerVector, setup the list
Image_window::ScalerVector::ScalerVector()
{
	reserve(12);

// This is all the names of the scalers. It needs to match the ScalerType enum
	const ScalerInfo point = { 
		"Point", 0xFFFFFFFF, 
		&Image_window::show_scaled8to565_point,
		&Image_window::show_scaled8to555_point,
		&Image_window::show_scaled8to16_point,
		&Image_window::show_scaled8to32_point,
		&Image_window::show_scaled8bit_point
	};
	push_back(point);

	const ScalerInfo Interlaced = { 
		"Interlaced", 0xFFFFFFFE,
		&Image_window::show_scaled8to565_interlace,
		&Image_window::show_scaled8to555_interlace,
		&Image_window::show_scaled8to16_interlace,
		&Image_window::show_scaled8to32_interlace,
		&Image_window::show_scaled8bit_interlace
	};
	push_back(Interlaced);

	const ScalerInfo Bilinear = { 
		"Bilinear", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_bilinear,
		&Image_window::show_scaled8to555_bilinear,
		&Image_window::show_scaled8to16_bilinear,
		&Image_window::show_scaled8to32_bilinear,
		0
	};
	push_back(Bilinear);

	const ScalerInfo BilinearPlus = { 
		"BilinearPlus", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_BilinearPlus,
		&Image_window::show_scaled8to555_BilinearPlus,
		&Image_window::show_scaled8to16_BilinearPlus,
		&Image_window::show_scaled8to32_BilinearPlus,
		0
	};
	push_back(BilinearPlus);

	const ScalerInfo _2xSaI = { 
		"2xSaI", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_2xSaI,
		&Image_window::show_scaled8to555_2xSaI,
		&Image_window::show_scaled8to16_2xSaI,
		&Image_window::show_scaled8to32_2xSaI,
		0
	};
	push_back(_2xSaI);

	const ScalerInfo SuperEagle = { 
		"SuperEagle", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_SuperEagle,
		&Image_window::show_scaled8to555_SuperEagle,
		&Image_window::show_scaled8to16_SuperEagle,
		&Image_window::show_scaled8to32_SuperEagle,
		0
	};
	push_back(SuperEagle);

	const ScalerInfo Super2xSaI = { 
		"Super2xSaI", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_Super2xSaI,
		&Image_window::show_scaled8to555_Super2xSaI,
		&Image_window::show_scaled8to16_Super2xSaI,
		&Image_window::show_scaled8to32_Super2xSaI,
		0
	};
	push_back(Super2xSaI);

	const ScalerInfo Scale2X = { 
		"Scale2X", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_2x_noblur,
		&Image_window::show_scaled8to555_2x_noblur,
		&Image_window::show_scaled8to16_2x_noblur,
		&Image_window::show_scaled8to32_2x_noblur,
		&Image_window::show_scaled8bit_2x_noblur
	};
	push_back(Scale2X);

	const ScalerInfo Hq2x = { 
		"Hq2x", SCALE_BIT(2), 
		&Image_window::show_scaled8to565_Hq2x,
		&Image_window::show_scaled8to555_Hq2x,
		&Image_window::show_scaled8to16_Hq2x,
		&Image_window::show_scaled8to32_Hq2x,
		0
	};
	push_back(Hq2x);

	const ScalerInfo Hq3x = { 
		"Hq3x", SCALE_BIT(2)|SCALE_BIT(3), 
		&Image_window::show_scaled8to565_Hq3x,
		&Image_window::show_scaled8to555_Hq3x,
		&Image_window::show_scaled8to16_Hq3x,
		&Image_window::show_scaled8to32_Hq3x,
		0
	};
	push_back(Hq3x);

#ifdef HAVE_OPENGL
	const ScalerInfo opengl = { 
		"OpenGL", 0xFFFFFFFF 
	};
	push_back(opengl);
#endif
}

Image_window::ScalerType Image_window::get_scaler_for_name(const char *scaler)
{
	for (int s = 0; s < NumScalers; s++) {
		if (!Pentagram::strcasecmp(scaler, Scalers[s].name)) 
			return (ScalerType) s;
	}

	return NoScaler;
}

/*
* Image_window::Get_scaled_video_mode
* 
* Get the bpp and flags for a scaled surface of the desired scaled video mode
*/

bool Image_window::Get_scaled_video_mode(int w, int h, int *bpp, uint32 *flags)
{
	static int desktop_depth = 0;
	if (desktop_depth == 0) desktop_depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

	if (w == 0 || h == 0 || bpp == 0 || flags == 0) return false;

	if (*flags == 0)
		*flags = SDL_SWSURFACE | (fullscreen?SDL_FULLSCREEN:0);

	// Explicit BPP required
	if (*bpp != 0)
	{
		if (SDL_VideoModeOK(w, h, *bpp, *flags) == 0)
		{
			cerr << "SDL Reports " << *bpp << " bpp surface is not OK." << endl;
			return false;
		}
	}
	else if ((desktop_depth != 16 && desktop_depth != 32) || !SDL_VideoModeOK(w, h, *bpp = desktop_depth, *flags))
	{
		int desired16 = SDL_VideoModeOK(w, h, 16, *flags);
		int desired32 = SDL_VideoModeOK(w, h, 32, *flags);

		if (desired16 == 16)
			*bpp = 16;
		else if (desired32 == 32)
			*bpp = 32;
		else if (desired16 != 0)
			*bpp = 16;
		else if (desired32 != 0)
			*bpp = 32;
		else 
		{
			cerr << "SDL Reports 16 bpp and 32 bpp surfaces are not OK." << endl;
			return false;
		}
	}

	return true;
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
	uses_palette = true;
	show_scaled = 0;
	draw_surface = paletted_surface = display_surface = 0;
#if defined(__zaurus__)
	fullscreen = false; // Zaurus would crash in fullscreen mode
#else
	if (try_scaler(w, h)) return; // everyone else can test the try_scaler function
#endif

	if (!paletted_surface)			// No scaling, or failed?
	{
		uint32 flags = SDL_SWSURFACE | (fullscreen?SDL_FULLSCREEN:0) | (ibuf->depth==8?SDL_HWPALETTE:0);
		display_surface = draw_surface = paletted_surface = SDL_SetVideoMode(w, h, ibuf->depth, flags);
		scale = 1;
	}
	if (!paletted_surface)
	{
		cerr << "Couldn't set video mode (" << w << ", " << h <<
			") at " << ibuf->depth << " bpp depth: " <<
			SDL_GetError() << endl;
		exit(-1);
	}
	ibuf->bits = (unsigned char *) draw_surface->pixels;
	// Update line size in words.
	ibuf->line_width = draw_surface->pitch/ibuf->pixel_size;
}

/*
*	Set up surfaces for scaling.
*	Output:	False if error (reported).
*/

bool Image_window::create_scale_surfaces(int scl, int w, int h,
										 scalefun fun565, scalefun fun555, scalefun fun16, scalefun fun32)
{
	if (ibuf->depth != 8) {
		cerr << "Scaling from " << ibuf->depth << " not supported." << endl;
		return false;
	}

	int hwdepth = 0;
	uint32 flags = 0;

	// Are 16bit or 32 bit required?
	if (fun16 == 0 && fun32 != 0) hwdepth = 32;
	else if (fun16 != 0 && fun32 == 0) hwdepth = 16;

	// Get 
	if (!Get_scaled_video_mode(scl*w, scl*h, &hwdepth, &flags))
		return false;

	// Get_scaled_video_mode will only ever return 16 or 32 bpp, but doing this just incase someone
	// changes it
	if (hwdepth != 16 && hwdepth != 32) 
	{
		cerr << "Scaling to " << hwdepth << " not supported." << endl;
		return false;
	}
	else if ((display_surface = SDL_SetVideoMode(scl*w, scl*h, 
		hwdepth, flags)) != 0 &&
		(draw_surface = paletted_surface = 
		SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
		8, 0, 0, 0, 0)) != 0)
	{			// Get color mask info.
		SDL_PixelFormat *fmt = display_surface->format;
		uint32 r = fmt->Rmask, g=fmt->Gmask, b=fmt->Bmask;
		if (hwdepth == 16)
		{
			show_scaled = 
				(r == 0xf800 && g == 0x7e0 && b == 0x1f) || (b == 0xf800 && g == 0x7e0 && r == 0x1f) ? 
					(fun565!=0?fun565:fun16) : 
				(r == 0x7c00 && g == 0x3e0 && b == 0x1f) || (b == 0x7c00 && g == 0x3e0 && r == 0x1f) ? 
					(fun555!=0?fun555:fun16) : 
				fun16 ;
		}
		else
		{
			show_scaled = fun32;
		}
		uses_palette = false;
		return true;
	}
	else
	{
		cerr << "Couldn't create scaled surface" << endl;
		if (draw_surface != 0 && draw_surface != display_surface) SDL_FreeSurface(draw_surface);
		paletted_surface = 0;
		display_surface = 0;
		draw_surface = 0;
		return false;
	}
}

/*
*	Set up surfaces and scaler to call.
*/

bool Image_window::try_scaler(int w, int h)
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
			(fullscreen?SDL_FULLSCREEN:0);
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
		if ((display_surface = SDL_SetVideoMode(scale*w, scale*h, 
			hwdepth, video_flags)) != 0 &&
			(draw_surface = paletted_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE, w, h,
			8, 0, 0, 0, 0)) != 0)
		{
			show_scaled = &Image_window::show_scaledOpenGL;
		}
		else
		{
			cerr << "Couldn't allocate surface: " << 
				SDL_GetError() << endl;
			if (draw_surface != 0 && draw_surface != display_surface) SDL_FreeSurface(draw_surface);
			paletted_surface = 0;
			display_surface = 0;
			draw_surface = 0;
		}
#else
		cerr << "OpenGL not supported" << endl;
#endif
	}
	else if (scale >= 2)
	{
		const ScalerInfo *info;

		if (scaler < 0 || scaler >= NumScalers)
			info = &Scalers[point];
		else
			info = &Scalers[scaler];

		int selected_factor = scale;

		// Is the size supported?
		while (selected_factor != 0 && !(info->size_mask & SCALE_BIT(selected_factor)))
		{
			// If not, try the next one down
			selected_factor--;
		}

		// Uh oh, couldn't find any factors the same size or smaller
		if (selected_factor == 1) 
		{
			// Try try to find one above
			selected_factor = scale+1;
			while (selected_factor < 32 && !(info->size_mask & SCALE_BIT(selected_factor)))
			{
				// If not, try the next one down
				selected_factor++;
			}

			// Someone messed up it seems....
			if (selected_factor == 32) 
			{
				cerr << "Scaler doesn't support any scaling factors." << endl;
				return false;
			}
		}

		// First try 16 bit/32 bit scaler
		if (info->fun16 !=0 || info->fun32 != 0)
		{
			if (create_scale_surfaces(selected_factor, w, h, info->fun565, info->fun555, info->fun16, info->fun32))
			{
				scale = selected_factor;
				ibuf->bits = (unsigned char *) draw_surface->pixels;
				// Update line size in words.
				ibuf->line_width = draw_surface->pitch/ibuf->pixel_size;
				return true;
			}
		}

		// Try 8but scaler
		if (info->fun8)
		{
			if (ibuf->depth != 8) return false;

			display_surface = paletted_surface = SDL_SetVideoMode(w*selected_factor, h*selected_factor, 8, SDL_SWSURFACE|SDL_HWPALETTE|(fullscreen?SDL_FULLSCREEN:0));

			draw_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
			if (paletted_surface && draw_surface)
			{
				show_scaled = info->fun8;

				scale = selected_factor;
				ibuf->bits = (unsigned char *) draw_surface->pixels;
				// Update line size in words.
				ibuf->line_width = draw_surface->pitch/ibuf->pixel_size;
				return true;
			}
			else
			{
				cerr << "Couldn't create 8bit scaled surface" << endl;
				if (draw_surface != 0 && draw_surface != display_surface) SDL_FreeSurface(draw_surface);
				paletted_surface = 0;
				display_surface = 0;
				draw_surface = 0;
			}
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
	if (draw_surface != 0 && draw_surface != display_surface) SDL_FreeSurface(draw_surface);
	paletted_surface = 0;
	display_surface = 0;
	draw_surface = 0;
	ibuf->bits = 0;
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
	if (paletted_surface)
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
		SDL_UpdateRect(display_surface, 0, 0, ibuf->width, ibuf->height);
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
		SDL_UpdateRect(display_surface, x, y, w, h);
}


/*
*	Toggle fullscreen.
*/
void Image_window::toggle_fullscreen() {
	int w, h;

	w = draw_surface->w;
	h = draw_surface->h;

	if ( fullscreen ) {
		cout << "Switching to windowed mode."<<endl;
	} else {
		cout << "Switching to fullscreen mode."<<endl;
	}
	/* First see if it's allowed.
	* for now this is preventing the switch to fullscreen
	*if ( SDL_VideoModeOK(w, h, bpp, flags) )
	*/
	{
		free_surface();		// Delete old.
		fullscreen = !fullscreen;
		create_surface(w, h);	// Create new.
	}
}

bool Image_window::screenshot(SDL_RWops *dst)
{
	if (!paletted_surface) return false;
#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())
	{
		int width = ibuf->width, height = ibuf->height;
		GL_manager *glman = GL_manager::get_instance();
		unsigned char *bits = glman->get_unscaled_rgb(width, height, false, true);
		SDL_Surface *screenshot_surface = SDL_CreateRGBSurfaceFrom(bits,
			width, height, 24, 3 * width, 0, 0, 0, 0);
		bool ret = SavePCX_RW(screenshot_surface, dst, true);
		SDL_FreeSurface(screenshot_surface);
		delete [] bits;
		return ret;
	}
#endif
	return SavePCX_RW(draw_surface, dst, true);
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
	SDL_Color *colors = paletted_surface->format->palette->colors;
	SDL_Color& color = colors[pix];
	glDisable(GL_TEXTURE_2D);	// Disable texture-mapping.
	glPushMatrix();
	int x = destx;			// Left edge.
	int y = -(desty + srch);
	glTranslatef((float)x, (float)y, 0);
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

