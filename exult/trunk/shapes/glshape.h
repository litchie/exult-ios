/*
 *	glshape.h - Paint 2D shapes in OpenGL
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

#ifndef GLSHAPE_H
#define GLSHAPE_H	1

/* ++++++TESTING. */
#define HAVE_OPENGL	1
#ifdef HAVE_OPENGL

#include <GL/gl.h>

class Shape_frame;
class GL_texshape;

/*
 *	A 2D shape is rendered by painting a quad with the shape as a texture.
 */
class GL_texshape
	{
	Shape_frame *frame;		// Source for this.
	GLuint texture;			// Texture ID.
	GLuint texsize;			// Width/ht of texture (power of 2).
					// Least-recently used chain:
	GL_texshape *lru_next, *lru_prev;
public:
	friend class GL_manager;
	GL_texshape(Shape_frame *f, unsigned char *pal);
	~GL_texshape();
	void paint(int px, int py);	// Render at given position.
	};

/*
 *	Handle OpenGL rendering.
 */
class GL_manager
	{
	GL_texshape *shapes;		// Shapes in LRU chain.
	int num_shapes;
	unsigned char *palette;		// 3*256 bytes (rgb).
public:
	GL_manager();
	~GL_manager();
	void set_palette(unsigned char *pal)
		{ palette = pal; }
					// Window was resized.
	void resized(int new_width, int new_height);
					// Paint a shape.
	void paint(Shape_frame *frame, int px, int py);
	};

#endif	/* HAVE_OPENGL */
#endif	/* GLSHAPE_H */
