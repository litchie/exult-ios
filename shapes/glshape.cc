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
	unsigned char *pal		// 3*256 bytes (rgb).
	)
	{
	assert(pal != 0);
#if 0	/* +++++Testing */
	Image_buffer8 test(128, 128);
	for (int r = 0; r < 16; r++)
	for (int c = 0; c < 16; c++)
		test.fill8(16*r + c, 8, 8, c*8, r*8);
	unsigned char *pixels = test.rgba(pal, transp);
#else
					// Convert to rgba.
	unsigned char *pixels = src->rgba(pal, transp);
#endif
	GLuint tex;
	glGenTextures(1, &tex);		// Generate (empty) texture.
	texture = tex;
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texsize, texsize, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pixels);
	delete pixels;
					// Linear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

/*
 *	Create for a given frame.
 */

GL_texshape::GL_texshape
	(
	Shape_frame *f,
	unsigned char *pal		// 3*256 bytes (rgb).
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
	create(&buf8, pal);
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
 *	Paint it.  Assumes that GL_TEXTURE_2D is already enabled.
 */

void GL_texshape::paint
	(
	int px, int py			// Location in 'pixels' from top-left
					//   of screen.
	)
	{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPushMatrix();
					// Convert to tile position.
	float x = static_cast<float>(px);
	float y = static_cast<float>(py) + texsize;
	if (frame)
		{
		x += frame->get_xright() - (int) texsize;
		y += frame->get_ybelow() - (int) texsize;
		}
					// Game y-coord goes down from top.
	y = -y;
	x /= c_tilesize;
	y /= c_tilesize;
	float w = static_cast<float>(texsize)/(c_tilesize), 
	      h = static_cast<float>(texsize)/(c_tilesize);
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
	glShadeModel(GL_SMOOTH);	// Smooth shading.
	glClearColor(1, 1, 1, 0);	// Background is white.
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);	// Enable depth-testing.
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// ??
	glEnable(GL_BLEND);		// !These two calls do the trick.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);	// Enable texture-mapping.
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
					// Set camera.
	glOrtho(0, new_width/c_tilesize, -new_height/c_tilesize, 0, 1, -16);
	glMatrixMode(GL_MODELVIEW);	// Use model-view matrix from now on.
	glLoadIdentity();
					// Clear screen & depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

/*
 *	Paint a shape.
 */

void GL_manager::paint
	(
	Shape_frame *frame,
	int px, int py			// 'Pixel' position from top-left.
	)
	{
	GL_texshape *tex = frame->glshape;
	if (!tex)			// Need to create texture?
		{
		frame->glshape = tex = new GL_texshape(frame, palette);
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
