/**
 **	Gametxt.cc - Text-drawing methods for Game_window.
 **
 **	Written: 3/19/2000 - JSF
 **/

/*
Copyright (C) 2000 Jeffrey S. Freedman

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
#include "gamewin.h"

/*
 *	Fonts in 'fonts.vga':
 *
 *	0 = Normal yellow.
 *	1 = Large runes.
 *	2 = small black (as in zstats).
 *	3 = runes.
 *	4 = tiny black.
 *	5 = little white.
 *	6 = runes.
 *	7 = normal red.
 */

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

void Game_window::paint_text_box
	(
	int fontnum,			// Font # from fonts.vga (0-9).
	char *text,
	int x, int y,			// Top-left corner of box.
	int w, int h			// Dimensions.
	)
	{
	win->set_clip(x, y, w, h);
	int endx = x + w, endy = y + h;	// Figure where to stop.
	int curx = x, cury = y;
	int height = get_text_height(fontnum);
	while (*text)			// Go through it.
		{
		char *wrd;		// ->start of word.
		switch (*text)		// Special cases.
			{
		case '\n':		// Next line.
			curx = x;
			cury += height;
			text++;
			continue;
		case ' ':		// Space.
		case '\t':
					// Pass space.
			wrd = Pass_space(text);
			if (wrd != text)
				curx += get_text_width(fontnum, text, 
								wrd - text);
			text = wrd;
			break;
			}
					// Pass word & get its width.
		char *ewrd = Pass_word(text);
		int width = get_text_width(fontnum, text, ewrd - text);
		if (curx + width > endx)
			{		// Word-wrap.
			curx = x;
			cury += height;
			}
					// Draw word.
		curx += paint_text(fontnum, text, ewrd - text, curx, cury);
		text = ewrd;		// Continue past the word.
		}
	win->clear_clip();
	}

/*
 *	Draw text at a given location (which is the upper-left corner of the
 *	place to draw.
 *
 *	Output:	Width in pixels of what was drawn.
 */

int Game_window::paint_text
	(
	int fontnum,			// 0-9, from fonts.vga.
	const char *text,			// What to draw, 0-delimited.
	int xoff, int yoff		// Upper-left corner of where to start.
	)
	{
	int x = xoff;
	int chr;
	while ((chr = *text++) != 0)
		{
		Shape_frame *shape = fonts.get_shape(fontnum, chr);
		if (!shape)
			continue;
		paint_rle_shape(*shape, x, yoff + shape->get_yabove());
		x += shape->get_width();
		}
	return (x - xoff);
	}

/*
 *	Paint text using font from "fonts.vga".
 *
 *	Output:	Width in pixels of what was painted.
 */

int Game_window::paint_text
	(
	int fontnum,			// Font # in fonts.vga (0-9).
	const char *text,			// What to draw.
	int textlen,			// Length of text.
	int xoff, int yoff		// Upper-left corner of where to start.
	)
	{
	int x = xoff;
	while (textlen--)
		{
		Shape_frame *shape = fonts.get_shape(fontnum, (int) *text++);
		if (!shape)
			continue;
		paint_rle_shape(*shape, x, yoff + shape->get_yabove());
		x += shape->get_width();
		}
	return (x - xoff);
	}

/*
 *	Get the width in pixels of a 0-delimited string.
 */

int Game_window::get_text_width
	(
	int fontnum,
	const char *text
	)
	{
	int width = 0;
	short chr;
	while ((chr = *text++) != 0)
		width += fonts.get_shape(fontnum, chr)->get_width();
	return (width);
	}

/*
 *	Get the width in pixels of a string given by length.
 */

int Game_window::get_text_width
	(
	int fontnum,
	const char *text,
	int textlen			// Length of text.
	)
	{
	int width = 0;
	while (textlen--)
		width += fonts.get_shape(fontnum, *text++)->get_width();
	return (width);
	}

/*
 *	Get font line-height.
 */

int Game_window::get_text_height
	(
	int fontnum
	)
	{
	Shape_frame *A = fonts.get_shape(fontnum, 'A');
	Shape_frame *y = fonts.get_shape(fontnum, 'y');
	return A->get_yabove() + y->get_ybelow();	
	}

