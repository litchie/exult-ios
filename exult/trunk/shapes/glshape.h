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

#ifdef HAVE_OPENGL

#include <GL/gl.h>

class Shape_frame;

/*
 *	A 2D shape is rendered by painting a quad with the shape as a texture.
 */
class Gl_texshape
	{
	Shape_frame *frame;		// Source for this.
	GLuint texture;			// Texture ID.
	GLuint texsize;			// Width/ht of texture (power of 2).
					// Least-recently used chain:
	Gl_texshape *lru_next *lru_prev;
public:
	friend class GL_manager;
	Gl_texshape(Shape_frame *f);
	~Gl_texshape();
	void paint(int px, int py);	// Render at given position.
	};

/*
 *	Handle OpenGL rendering.
 */
class GL_manager
	{
	Gl_texshape *shapes;		// Shapes in LRU chain.
	int num_shapes;
public:
	GL_manager();
	~GL_manager();
					// Window was resized.
	void resized(int new_width, int new_height);
					// Paint a shape.
	void paint(Shape_frame *frame, int px, int py);
	};

#endif	/* HAVE_OPENGL */
#endif	/* GLSHAPE_H */
