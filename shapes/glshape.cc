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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_OPENGL

#include <GL/gl.h>

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
	Xform_palette *xforms,		// Transforms translucent colors if !0.
	int xfcnt			// Number of xforms.
	)
	{
	assert(pal != 0);
	const int xfstart = 0xff - xfcnt;
					// Convert to rgba.
	unsigned char *pixels = xforms ? 
			src->rgba(pal, transp, xfstart, 0xfe, xforms)
			: src->rgba(pal, transp);
	GLuint tex;
	glGenTextures(1, &tex);		// Generate (empty) texture.
	texture = tex;
	glBindTexture(GL_TEXTURE_2D, texture);
					// +++++Might want GL_RGBA, depending
					//   on video card.
					// Need translucency?
	GLint iformat = xforms ? GL_RGBA4 : GL_RGB5_A1;
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texsize, texsize, 0, GL_RGBA,
	glTexImage2D(GL_TEXTURE_2D, 0, iformat, texsize, texsize, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);
	delete pixels;
					// Linear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
	Xform_palette *xforms,		// Transforms translucent colors if !0.
	int xfcnt			// Number of xforms.
	) : frame(f), lru_next(0), lru_prev(0)
	{
	int w = frame->get_width(), h = frame->get_height();
					// Figure texture size as 2^n, rounding
					//   up.
	int logw = Log2(2*w - 1), logh = Log2(2*h - 1);
	texsize = 1<<(logw > logh ? logw : logh);
					// Render frame.
	Image_buffer8 buf8(texsize, texsize);
	buf8.fill8(transp);		// Fill with transparent value.
					// ++++Guessing a bit on the offset:
	frame->paint(&buf8, texsize - frame->get_xright() - 1,
					texsize - frame->get_ybelow() - 1);
	create(&buf8, pal, xforms, xfcnt);
	}

/*
 *	Create from a given (square, size 2^n) image.
 */

GL_texshape::GL_texshape
	(
	Image_buffer8 *src,		// Must be square.
	unsigned char *pal		// 3*256 bytes (rgb).
	) : frame(0), lru_next(0), lru_prev(0)
	{
	int w = src->get_width(), h = src->get_height();
	assert (w == h);
	assert ((1<<Log2(w)) == w);
	texsize = w;
	create(src, pal);
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
		frame->glshape = 0;	// Tell owner.
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
	) : shapes(0), num_shapes(0), palette(0), scale(1)
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
	delete palette;
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

/*
 *	Paint an image directly.
 */

static void Paint_image
	(
	Shape_frame *frame,
	int px, int py,			// 'Pixel' position from top-left.
	unsigned char *pal,		// 3*256 bytes (rgb).
	int scale			// Scale factor.
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
	frame->paint(&buf8, frame->get_xleft(), frame->get_yabove());
					// Convert to rgba.
	unsigned char *pixels = buf8.rgba(pal, transp);
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
	glRasterPos2f(px, py);
	glPixelZoom(scale, scale);
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	delete pixels;
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
	GL_texshape *tex = frame->glshape;
	if (!tex)			// Need to create texture?
		{
		if (frame->get_width() > max_texsize || 
		    frame->get_height() > max_texsize)
			{		// Too big?  Just paint it now.
			Paint_image(frame, px, py, palette, scale);
			return;
			}
		frame->glshape = tex = new GL_texshape(frame, palette, 
							xforms, xfcnt);
		num_shapes++;
		//++++++When 'too many', we'll free LRU here.
		}
	else				// Remove from chain.
		{
		if (tex->lru_next)
			tex->lru_next->lru_prev = tex->lru_prev;
		if (tex->lru_prev)
			tex->lru_prev->lru_next = tex->lru_next;
		tex->lru_prev = 0;	// It will go to the head.
		}
	tex->lru_next = shapes;		// Add to head of chain.
	if (shapes)
		shapes->lru_prev = tex;
	shapes = tex;
	tex->paint(px, py);
	}

#endif	/* HAVE_OPENGL */
