/**
 **	Imagetxt.cc - Text-drawing methods for Image_window.
 **
 **	Written: 11/25/98 - JSF
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

#include <ctype.h>
#include "imagewin.h"
#include "text.h"

/*
 *	Open a font.
 *
 *	Output:	->font, or 0 if error.
 */

Font_face *Image_buffer_base::open_font
	(
	char *fname,			// Filename (*.ttf).
	int points			// Desired size in points.
	)
	{
	Font_face *font = new Font_face(fname, points);
					// +++++++Check for errors.
	return (font);
	}

/*
 *	Close font.
 */

void Image_buffer_base::close_font
	(
	Font_face *font
	)
	{
	delete font;
	}

/*
 *	Pass space.
 */

static char *Pass_space
	(
	char *text
	)
	{
	while (isspace(*text))
		text++;
	return (text);
	}

/*
 *	Pass a word.
 */

static char *Pass_word
	(
	char *text
	)
	{
	while (*text && !isspace(*text))
		text++;
	return (text);
	}

/*
 *	Draw text within a rectangular area.
 *	Special characters handled are:
 *		\n	New line.
 *		space	Word break.
 *		tab	Treated like a space for now.
 */

void Image_buffer_base::draw_text_box
	(
	Font_face *font,
	char *text,
	int x, int y,			// Top-left corner of box.
	int w, int h			// Dimensions.
	)
	{
	set_clip(x, y, w, h);
	int endx = x + w, endy = y + h;	// Figure where to stop.
	int curx = x, cury = y;
	while (*text)			// Go through it.
		{
		char *wrd;		// ->start of word.
		switch (*text)		// Special cases.
			{
		case '\n':		// Next line.
			curx = x;
			cury += font->get_height();
			text++;
			continue;
		case ' ':		// Space.
		case '\t':
					// Pass space.
			wrd = Pass_space(text);
			if (wrd != text)
				curx += get_text_width(font, text, wrd - text);
			text = wrd;
			break;
			}
					// Pass word & get its width.
		char *ewrd = Pass_word(text);
		int width = get_text_width(font, text, ewrd - text);
		if (curx + width > endx)
			{		// Word-wrap.
			curx = x;
			cury += font->get_height();
			}
					// Draw word.
		curx += draw_text(font, text, ewrd - text, curx, cury);
		text = ewrd;		// Continue past the word.
		}
	clear_clip();
	}

/*
 *	Draw text at a given location (which is the upper-left corner of the
 *	place to draw.
 *
 *	Output:	Width in pixels of what was drawn.
 */

int Image_buffer_base::draw_text
	(
	Font_face *font,
	char *text,			// What to draw, 0-delimited.
	int x, int y			// Upper-left corner of where to start.
	)
	{
	int startx = x;
	short chr;
	while ((chr = *text++) != 0)
		x += draw_char(font, chr, x, y);
	return (x - startx);
	}

/*
 *	Draw text at a given location (which is the upper-left corner of the
 *	place to draw.
 *
 *	Output:	Width in pixels of what was drawn.
 */

int Image_buffer_base::draw_text
	(
	Font_face *font,
	char *text,			// What to draw.
	int textlen,			// Length of text.
	int x, int y			// Upper-left corner of where to start.
	)
	{
	int startx = x;
	while (textlen--)
		x += draw_char(font, *text++, x, y);
	return (x - startx);
	}

/*
 *	Draw a single character.
 *
 *	Output:	# of pixels to advance past the char.
 */

int Image_buffer_base::draw_char
	(
	Font_face *font,
	short chr,
	int x, int y			// Upper-left corner of wher to draw.
	)
	{
	int color = 1;			// ++++++++Testing.
	unsigned char buf[4000];	// Should be big enough.
	unsigned char *out = &buf[0];
	Glyph *glyph = font->get_glyph(chr);
	char *bits = glyph->get_bits();	// Get bit-map.
	int rows = glyph->get_rows();	// Get metrics.
	int cols = glyph->get_cols();
	for (int r = 0; r < rows; r++)	// Go through rows.
					// Go through cols.
		for (int c = 0; c < cols; c++)
			{		// Get next 8 pixels.
			char pix = *bits++;
			for (int b = 8; b; b--, pix = pix << 1)
					// Look at each bit.
				*out++ = (pix&128) ? color : 0;
			}
	copy_transparent8(buf, 8*cols, rows, x + glyph->get_bearingX(),
			y + font->get_height() - glyph->get_bearingY());
	return (glyph->get_advance());	// Return width to next char.
	}

/*
 *	Get the width in pixels of a 0-delimited string.
 */

int Image_buffer_base::get_text_width
	(
	Font_face *font,
	char *text
	)
	{
	int width = 0;
	short chr;
	while ((chr = *text++) != 0)
		width += font->get_glyph(chr)->get_advance();
	return (width);
	}

/*
 *	Get the width in pixels of a string given by length.
 */

int Image_buffer_base::get_text_width
	(
	Font_face *font,
	char *text,
	int textlen			// Length of text.
	)
	{
	int width = 0;
	short chr;
	while (textlen--)
		width += font->get_glyph(*text++)->get_advance();
	return (width);
	}

/*
 *	Get font line-height.
 */

int Image_buffer_base::get_text_height
	(
	Font_face *font
	)
	{
	return (font->get_height());
	}

