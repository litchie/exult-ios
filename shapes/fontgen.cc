/**
 **	Create RLE font shapes from a given font.
 **
 **	Written: 4/8/2002 - JSF
 **/

/*
Copyright (C) 2002  The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_FREETYPE2

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string.h>
#include "vgafile.h"

/*
 *	Create a shape with each frame containing the glyph for its ASCII
 *	code.  The shape has 128 frames.
 *
 *	Output:	->new Shape, or 0 if error.
 */

Shape *Gen_font_shape
	(
	const char *fontfile,		// Filename of font.
	int pixels_ht,			// Desired height in pixels.
	unsigned char fg,		// Foreground color index.
	unsigned char bg		// Background color index.
	)
	{
	FT_Library library;		// Initialize.
	int error = FT_Init_FreeType(&library);
	if (error)
		return 0;
	FT_Face face;			// Gets the font.
	error = FT_New_Face(library, fontfile, 0, &face);
	if (error)
		return 0;
	error = FT_Set_Pixel_Sizes(face, 0, pixels_ht);
					// Glyphs are rendered here:
	FT_GlyphSlot glyph = face->glyph;
	if (error)
		return 0;
	Shape *shape = new Shape(128);
	for (int chr = 0; chr < 128; chr++)
		{			// Get each glyph.
		error = FT_Load_Char(face, chr, 
				FT_LOAD_RENDER|FT_LOAD_MONOCHROME);
		if (error)
			{
			//+++++Do we need to store an empty frame?
			continue;
			}
		int w = glyph->bitmap.width, h = glyph->bitmap.rows;
					// Allocate our buffer.
		int cnt = w*h;		// Total #pixels.
		unsigned char *pixels = new unsigned char[cnt];
		memset(pixels, bg, cnt);// Fill with background.
					// I believe this is 1 bit/pixel:
		unsigned char *monobits = glyph->bitmap.buffer;
		for (int b = 0; b < cnt; b++)
			if (monobits[b/8]&(0x80>>(b%8)))
				pixels[b] = fg;
					// Not sure about dims here+++++
		Shape_frame *frame = new Shape_frame(pixels,
			w, h, glyph->bitmap_left, glyph->bitmap_top, true);
		delete pixels;
		shape->set_frame(frame, chr);
		}
	FT_Done_FreeType(library);
	return shape;
	}

#endif	/* HAVE_FREETYPE2 */

