/*
 *	glshape.cc - Paint 2D shapes in OpenGL
 *
 *	Written: 7/16/02 - JSF
 *
 *  Copyright (C) 2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_OPENGL

#include "glshape.h"
#include "vgafile.h"
#include "utils.h"
#include "exult_constants.h"
#include "ibuf8.h"

GL_manager *GL_manager::instance = 0;

const unsigned char transp = 255;	// Transparent pixel.

/*
 *	Create from a given source.  Assumes src is square, with texsize
 *	already set to length.
 */

void GL_texshape::create
	(
	Image_buffer8 *src,		// Source image.
	unsigned char *pal,		// 3*256 bytes (rgb).
	int firstrot, int lastrot,	// Palette rotations.
	Xform_palette *xforms,		// Transforms translucent colors if !0.
	int xfcnt,			// Number of xforms.
	int alpha			// Alpha value to use.
	)
	{
	assert(pal != 0);
	const int xfstart = 0xff - xfcnt;
					// Convert to rgba.
	unsigned char *pixels = xforms ? 
			src->rgba(pal, transp, rotates, firstrot, lastrot,
						xfstart, 0xfe, xforms, alpha)
			: src->rgba(pal, transp, rotates, firstrot, lastrot,
						256, 256, 0, alpha);
	GLuint tex;
	glGenTextures(1, &tex);		// Generate (empty) texture.
	texture = tex;
	glBindTexture(GL_TEXTURE_2D, texture);
					// +++++Might want GL_RGBA, depending
					//   on video card.
					// Need translucency?
	GLint iformat = xforms || alpha != 255 ? GL_RGBA4 : GL_RGB5_A1;
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texsize, texsize, 0, GL_RGBA,
	glTexImage2D(GL_TEXTURE_2D, 0, iformat, texsize, texsize, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);
	delete [] pixels;
					// Linear filtering.
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else	/* This looks less blurry, and helps get rid of the lines.	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
	// Enable texture clamping. This will get rid of all the lines everywhere
	// -Colourless
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);

	}

/*
 *	Create for a given frame.
 */

GL_texshape::GL_texshape
	(
	Shape_frame *f,
	unsigned char *pal,		// 3*256 bytes (rgb).
	int first_rot, int last_rot,	// Palette rotations.
	Xform_palette *xforms,		// Transforms translucent colors if !0.
	int xfcnt,			// Number of xforms.
	int alpha			// Alpha value to use.
	) : frame(f), lru_next(0), lru_prev(0), outline(-1)
	{
					// Figure texture size as 2^n, rounding up.
	int w = fgepow2(frame->get_width()), h = fgepow2(frame->get_height());
	texsize = w > h ? w : h;
					// Render frame.
	Image_buffer8 buf8(texsize, texsize);
	buf8.fill8(transp);		// Fill with transparent value.
					// ++++Guessing a bit on the offset:
	frame->paint(&buf8, texsize - frame->get_xright() - 1,
					texsize - frame->get_ybelow() - 1);
	create(&buf8, pal, first_rot, last_rot, xforms, xfcnt, alpha);
	}

GL_texshape::GL_texshape
	(
	Shape_frame *f,
	unsigned char *pal,			// 'Pixel' position from top-left.
	int first_rot, int last_rot,	// Palette rotations.
	unsigned char *trans,	// Translation table.
	int alpha			// Alpha value to use.
	) : frame(f), lru_next(0), lru_prev(0), outline(-1)
	{
					// Figure texture size as 2^n, rounding up.
	int w = fgepow2(frame->get_width()), h = fgepow2(frame->get_height());
	texsize = w > h ? w : h;
					// Render frame.
	Image_buffer8 buf8(texsize, texsize);
	buf8.fill8(transp);		// Fill with transparent value.
					// ++++Guessing a bit on the offset:
	frame->paint_rle_remapped(&buf8, texsize - frame->get_xright() - 1,
					texsize - frame->get_ybelow() - 1, trans);
	create(&buf8, pal, first_rot, last_rot, 0, 0, alpha);
	}

GL_texshape::GL_texshape
	(
	Shape_frame *f,
	unsigned char *pal,		// 3*256 bytes (rgb).
	int first_rot, int last_rot,	// Palette rotations.
	Xform_palette& xform,		// Transforms colors.
	int alpha			// Alpha value to use.
	) : frame(f), lru_next(0), lru_prev(0), outline(-1)
	{
					// Figure texture size as 2^n, rounding up.
	int w = fgepow2(frame->get_width()), h = fgepow2(frame->get_height());
	texsize = w > h ? w : h;
					// Render frame.
	Image_buffer8 buf8(texsize, texsize);
	buf8.fill8(transp);		// Fill with transparent value.
					// ++++Guessing a bit on the offset:
	frame->paint_rle_transformed(&buf8, texsize - frame->get_xright() - 1,
					texsize - frame->get_ybelow() - 1, xform);
	create(&buf8, pal, first_rot, last_rot, 0, 0, alpha);
	}

GL_texshape::GL_texshape
	(
	Shape_frame *f,
	unsigned char *pal,		// 3*256 bytes (rgb).
	int first_rot, int last_rot,	// Palette rotations.
	unsigned char color,		// Transforms colors.
	int alpha			// Alpha value to use.
	) : frame(f), lru_next(0), lru_prev(0), outline(color)
	{
					// Figure texture size as 2^n, rounding up.
	int w = fgepow2(frame->get_width()), h = fgepow2(frame->get_height());
	texsize = w > h ? w : h;
					// Render frame.
	Image_buffer8 buf8(texsize, texsize);
	buf8.fill8(transp);		// Fill with transparent value.
					// ++++Guessing a bit on the offset:
	frame->paint_rle_outline(&buf8, texsize - frame->get_xright() - 1,
					texsize - frame->get_ybelow() - 1, outline);
	create(&buf8, pal, first_rot, last_rot, 0, 0, alpha);
	}

/*
 *	Create from a given (square, size 2^n) image.
 */

GL_texshape::GL_texshape
	(
	Image_buffer8 *src,		// Must be square.
	unsigned char *pal,		// 3*256 bytes (rgb).
	int first_rot, int last_rot,	// Palette rotations.
	int alpha			// Alpha value to use.
	) : frame(0), lru_next(0), lru_prev(0), outline(-1)
	{
	int w = src->get_width(), h = src->get_height();
	assert (w == h);
	assert (fgepow2(w) == w);
	texsize = w;
	create(src, pal, first_rot, last_rot, 0, 0, alpha);
	}

/*
 *	Free resources.
 */

GL_texshape::~GL_texshape
	(
	)
	{
	glDeleteTextures(1, &texture);	// Free the texture.
	if (frame)
		{	// Tell owner.
		if (outline < 0)
			frame->glshape = 0;
		else
			frame->gloutline = 0;
		}
	}

/*
 *	Paint it.
 */

void GL_texshape::paint
	(
	int px, int py			// Location in 'pixels' from top-left
					//   of screen.
	)
	{
	glEnable(GL_TEXTURE_2D);	// Enable texture-mapping.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPushMatrix();
	float x = static_cast<float>(px);
	float y = static_cast<float>(py) + texsize;
	if (frame)
		{
		x += frame->get_xright() + 1 - (int) texsize;
		y += frame->get_ybelow() + 1 - (int) texsize;
		}
					// Game y-coord goes down from top.
	y = -y;
	float w = static_cast<float>(texsize), 
	      h = static_cast<float>(texsize);
	glTranslatef(x, y, 0);
					// Choose texture.
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
		{
		glTexCoord2f(0, 0);		glVertex3f(0, h, 0);
		glTexCoord2f(0, 1);		glVertex3f(0, 0, 0);
		glTexCoord2f(1, 1);		glVertex3f(w, 0, 0);
		glTexCoord2f(1, 0);		glVertex3f(w, h, 0);
		}
	glEnd();
	glPopMatrix();
	}

/*
 *	Create OpenGL manager.
 */

GL_manager::GL_manager
	(
	) : shapes(0), palette(0), scale(1), num_shapes(0),
	    first_rot(224), last_rot(254)
	{
	assert (instance == 0);		// Should only be one.
	instance = this;
	GLint max_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
	max_texsize = max_size;
	glShadeModel(GL_SMOOTH);	// Smooth shading.
	glClearColor(1, 1, 1, 0);	// Background is white.
	glClearDepth(1);
/*
	glEnable(GL_DEPTH_TEST);	// Enable depth-testing.
	glDepthFunc(GL_LEQUAL);
*/
	// I'm disabling depth buffer because it's not needed, at the moment.
	// It's also causing problems. Not being cleared perhaps? or bad depth
	// values for each object. Anyway, I'm disabling both testing and writing
	// -Colourless
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// ??
	glEnable(GL_BLEND);		// !These two calls do the trick.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Just a note for Jeff, You are using Alpha Blending, but you should
	// perhaps think about using Alpha Testing. 
	// The functions are glAlphaFunc() and glEnable(GL_ALPHA_TEST).
	// Normally a glAlphaFunc(GL_GEQUAL, 127) will do the trick for you
	// -Colourless
	}

/*
 *	Free resources.
 */

GL_manager::~GL_manager
	(
	)
	{
	assert (this == instance);
	while (shapes)
		{
		GL_texshape *next = shapes->lru_next;
		delete shapes;
		shapes = next;
		}
	delete [] palette;
	instance = 0;
	}

/*
 *	Window was resized.
 */
void GL_manager::resized
	(
	int new_width, int new_height,
	int new_scale
	)
	{
	scale = new_scale;
					// Set viewing area to whole window.
	glViewport(0, 0, scale*new_width, scale*new_height);
	glMatrixMode(GL_PROJECTION);	// Set up orthogonal volume.
	glLoadIdentity();
					// Set camera area in 'pixels'.
	glOrtho(0, new_width, -new_height, 0, 1, -16);
	glMatrixMode(GL_MODELVIEW);	// Use model-view matrix from now on.
	glLoadIdentity();
					// Clear screen & depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

class Paint
	{
	Xform_palette *xforms;
	int xfcnt;
public:
	Paint(Xform_palette *xf, int xc)
		: xforms(xf), xfcnt(xc)
		{  }
	void paint(Shape_frame *frame, Image_buffer8 *win,	int px, int py)
		{ frame->paint(win, px, py); }
	GL_texshape *newtex(Shape_frame *frame, unsigned char *pal,
			int first_rot, int last_rot, int alpha)
		{ return new GL_texshape(frame, pal,
					first_rot, last_rot, xforms, xfcnt, alpha); }
	};

template<typename Data, void (Shape_frame::*Fun)(Image_buffer8 *, int, int,
					Data)>
class Paint_trans
	{
private:
	Data data;
public:
	Paint_trans(Data d)
		: data(d)
		{  }
	void paint(Shape_frame *frame, Image_buffer8 *win, int px, int py)
		{ (frame->*Fun)(win, px, py, data); }
	GL_texshape *newtex(Shape_frame *frame, unsigned char *pal,
			int first_rot, int last_rot, int alpha)
		{ return new GL_texshape(frame, pal,
					first_rot, last_rot, data, alpha); }
	};


/*
 *	Paint an rgba image directly.
 */
void gl_paint_rgba_bitmap
	(
	unsigned char *pixels,
	int px, int py,
	int w, int h,
	int scale
	)
	{
	if (px < 0)			// Doesn't paint if off screen.
		{
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, -px);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
		w += px;
		px = 0;
		}
					//++++CHeck py too?
//	float x = static_cast<float>(px);
//	float y = static_cast<float>(py);
	glPixelZoom((float) scale, (float) -scale);	// Get right side up.
	glRasterPos2i(px, py);
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}

/*
 *	Paint an image directly using the given data for transform.
 */
template<typename Functor>
static void Paint_image
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	unsigned char *pal,		// 3*256 bytes (rgb).
	int first_rot, int last_rot,	// Palette rotations.
	int alpha,
	int scale,			// Scale factor.
	Functor& f
	)
	{
	px -= frame->get_xleft();	// Figure actual from hot-spot.
	py += frame->get_ybelow();
					// Game y-coord goes down from top.
	py = -py;
	int w = frame->get_width(), h = frame->get_height();
					// Render frame.
	Image_buffer8 buf8(w, h);
	w = buf8.get_width(); h = buf8.get_height();
	buf8.fill8(transp);		// Fill with transparent value.
	f.paint(frame, &buf8, frame->get_xleft(), frame->get_yabove());
					// Convert to rgba.
	bool rot;
	unsigned char *pixels = buf8.rgba(pal, transp, rot,
					first_rot, last_rot, alpha);
	gl_paint_rgba_bitmap(pixels, px, py, w, h, scale);
	delete pixels;
	}

/*
 *	Paint a shape using a translation table.
 */

template<typename Functor>
void GL_manager::paint_internal
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	int alpha,
	Functor& f
	)
	{
	GL_texshape *tex = frame->glshape;
	if (!tex)			// Need to create texture?
		{
		if (frame->get_width() > max_texsize || 
		    frame->get_height() > max_texsize)
			{		// Too big?  Just paint it now.
			Paint_image(frame, px, py, palette, first_rot, last_rot,
							alpha, scale, f);
			return;
			}
		frame->glshape = tex = f.newtex(frame, palette,
						first_rot, last_rot, alpha);
		num_shapes++;
		//++++++When 'too many', we'll free LRU here.
		}
	else if (tex != shapes)	// Remove from chain, but NOT if already the head!
		{
		if (tex->lru_next)
			tex->lru_next->lru_prev = tex->lru_prev;
		if (tex->lru_prev)
			tex->lru_prev->lru_next = tex->lru_next;
		tex->lru_prev = 0;	// It will go to the head.
		}
	if (tex != shapes)
		{		// NOT if already the head!
		tex->lru_next = shapes;		// Add to head of chain.
		if (shapes)
			shapes->lru_prev = tex;
		shapes = tex;
		}
	tex->paint(px, py);
	}

/*
 *	Paint a shape.
 */

void GL_manager::paint
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	Xform_palette *xforms,		// Transforms translucent colors if !0.
	int xfcnt			// Number of xforms.
	)
	{
	Paint f(xforms, xfcnt);
	paint_internal(frame, px, py, 255, f);
	}

/*
 *	Paint a shape using a translation table.
 */

void GL_manager::paint_remapped
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	unsigned char *trans	// Translation table.
	)
	{
	Paint_trans<unsigned char *, &Shape_frame::paint_rle_remapped> f(trans);
	paint_internal(frame, px, py, 255, f);
	}

/*
 *	Paint a shape using a xform table.
 */

void GL_manager::paint_transformed
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	Xform_palette& xform	// Translation table.
	)
	{
	// For all the purists out there:
#ifdef U7_INVISIBILITY
	const unsigned char transform_alpha = 64;
	Paint_trans<Xform_palette&, &Shape_frame::paint_rle_transformed> f(xform);
#else
	const unsigned char transform_alpha = 96;
	Paint f(0, 0);
#endif
	paint_internal(frame, px, py, transform_alpha, f);
	}

/*
 *	Paint a shape using a different outline color.
 */

void GL_manager::paint_outline
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	unsigned char color	// Outline color
	)
	{
	GL_texshape *tex = frame->gloutline;
	if (!tex || tex->outline < 0 || tex->outline != color)
		{
		delete tex;
		Paint_trans<unsigned char, &Shape_frame::paint_rle_outline> f(color);
		frame->gloutline = tex = new GL_texshape(frame, palette,
					first_rot, last_rot, color);
		}
	tex->paint(px, py);
	}

template<int bpp>
static inline void unflip_bitmap(unsigned char *pixels, int w, int h)
	{
	for (int j = 0; j < h/2; j++)
		{
		const int swapj = h - j - 1;
		for (int i = 0; i < w; i++)
			{
			const int off = bpp * (i + j * w);
			const int swapoff = bpp * (i + swapj * w);
			for (int b = 0; b < bpp; b++)
				std::swap(pixels[off + b], pixels[swapoff + b]);
			}
		}
	}

template<int bpp>
static inline unsigned char *get_pixels
	(
	int width,
	int height,
	GLenum format,
	bool unflip, int scale
	)
	{
	int w = width * scale, h = height * scale;
	unsigned char *bits = new unsigned char[bpp*w*h];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, w, h, format, GL_UNSIGNED_BYTE, bits);
		// glReadPixels reads from bottom-left to top-right.
		// Flip it back.
	if (unflip)
		unflip_bitmap<bpp>(bits, w, h);
	return bits;
	}

/*
 *	Grabs the (scaled) contents of the screen.
 *
 *	Output: -> to buffer allocated with new [] in RGBA format.
 */

unsigned char *GL_manager::get_screen_rgba
	(
	int width,
	int height,
	bool rgb,
	bool unflip
	)
	{
	return get_pixels<4>(width, height, rgb ? GL_RGBA : GL_BGRA, unflip, scale);
	}

/*
 *	Grabs the (scaled) contents of the screen.
 *
 *	Output: -> to buffer allocated with new [] in RGB format.
 */

unsigned char *GL_manager::get_screen_rgb
	(
	int width,
	int height,
	bool rgb,
	bool unflip
	)
	{
	return get_pixels<3>(width, height, rgb ? GL_RGB : GL_BGR, unflip, scale);
	}

/*
 *	Grabs the unscaled contents of the screen, as best as possible.
 *
 *	Output: -> to buffer allocated with new [] in RGB format.
 */

unsigned char *GL_manager::get_unscaled_rgb
	(
	int width,
	int height,
	bool rgb,
	bool unflip
	)
	{
	const int bpp = 3;	// Want RGB.
	unsigned char *bits = get_screen_rgb(width, height, rgb, unflip);
		// Scale down.
	unsigned char *destbits = new unsigned char[bpp*width*height];
	const unsigned int scalefactor = scale * scale,
			largeline = bpp * width * scale, smallline = bpp * width;
		// Scale down.
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			{
			unsigned int r = 0, g = 0, b = 0;
			for (int x = scale * i; x < scale * (i + 1); x++)
				for (int y = scale * j; y < scale * (j + 1); y++)
					{
					const int src = bpp * x + largeline * y;
					r += bits[src + 0];
					g += bits[src + 1];
					b += bits[src + 2];
					}
			const int dst = bpp * i + smallline * j;
			destbits[dst + 0] = r / scalefactor;
			destbits[dst + 1] = g / scalefactor;
			destbits[dst + 2] = b / scalefactor;
			}
	delete [] bits;
	return destbits;
	}


#endif	/* HAVE_OPENGL */
