/**
 **	Text.h - Text handling (using FreeType).
 **
 **	Written: 11/20/98 - JSF
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

#ifndef INCL_TEXT
#define INCL_TEXT

#if !AUTOCONFIGURED
#include "autoconfigure.h"
#endif

#if HAVE_FREETYPE
#include <freetype.h>
#endif

/*
 *	A 'cached' glyph:
 */
class Glyph
	{
	TT_Raster_Map bitmap;		// Actual map.
	int bearingX, bearingY;		// Pixels from pen to start of glyph.
	int advance;			// Pixels to next character.
public:
	friend class Font_face;
	Glyph(Font_face *font, int index);
	~Glyph();
	int get_rows()			// Return #rows.
		{ return bitmap.rows; }
	int get_cols()			// Return #bytes/row.
		{ return bitmap.cols; }
	int get_bearingX()
		{ return bearingX; }
	int get_bearingY()
		{ return bearingY; }
	int get_advance()
		{ return advance; }
	char *get_bits()		// Return ->bitmap.
		{ return (char *) bitmap.bitmap; }
	};

/*
 *	Font instance (based on FreeType):
 */
class Font_face
	{
	static TT_Engine engine;	// Freetype engine.
	static unsigned char initialized;
	TT_Face face;			// Represents the font.
	TT_Face_Properties props;	// Properties.
	TT_Instance instance;
	TT_Instance_Metrics imetrics;
	TT_Glyph glyph;			// Holds the glyphs we load.
	TT_CharMap charmap;		// Unicode character mapping.
	int error;			// Gets error (if nonzero).
	Glyph **glyphs;		// A bitmap for each character.
	int get_index(short chr)	// Get glyph index of a character.
		{ return TT_Char_Index(charmap, chr); }
public:
	friend class Glyph;
	Font_face(char *fname, int pts);// Create for given .ttf file.
	~Font_face();
	Glyph *get_glyph(short chr)	// Get glyph for given char.
		{
		int index = get_index(chr);
		if (!glyphs[index])	// Look in cache.
			glyphs[index] = new Glyph(this, index);
		return (glyphs[index]);
		}
	int get_height()		// Get height in pixels.
		{ return imetrics.y_ppem; }
	};

#endif
