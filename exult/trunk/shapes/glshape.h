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

class Shape_frame;
class GL_texshape;
class Image_buffer8;

/*
 *	A 2D shape is rendered by painting a quad with the shape as a texture.
 */
class GL_texshape
	{
	Shape_frame *frame;		// Source for this (or null if it came
					//   from someplace else).
	unsigned int texture;		// Texture ID.
	unsigned int texsize;		// Width/ht of texture (power of 2).
					// Least-recently used chain:
	GL_texshape *lru_next, *lru_prev;
					// Create from this source.
	void create(Image_buffer8 *src, unsigned char *pal);
public:
	friend class GL_manager;
	GL_texshape(Shape_frame *f, unsigned char *pal);
	GL_texshape(Image_buffer8 *src, unsigned char *pal);
	~GL_texshape();
	void paint(int px, int py);	// Render at given position.
	};

/*
 *	Handle OpenGL rendering.
 */
class GL_manager
	{
	static GL_manager *instance;	// One one of these.
	int scale;			// Scale for drawing.
	GL_texshape *shapes;		// Shapes in LRU chain.
	int num_shapes;
	unsigned char *palette;		// 3*256 bytes (rgb).
public:
	friend class GL_texshape;
	GL_manager();
	~GL_manager();
	static GL_manager *get_instance()
		{ return instance; }
	void set_palette(unsigned char *pal)
		{ delete palette; palette = pal; }
					// Create from src (but don't add to
					//   chain).
	GL_texshape *create(Image_buffer8 *src)
		{ return new GL_texshape(src, palette); }
					// Window was resized.
	void resized(int new_width, int new_height, int new_scale);
					// Paint a shape & create GL_shape
					//   for it if necessary.
	void paint(Shape_frame *frame, int px, int py);
	};


#else

class GL_manager
{
 public:
	static GL_manager *get_instance()
		{ return 0; }
};

#endif	/* HAVE_OPENGL */


#endif	/* GLSHAPE_H */
