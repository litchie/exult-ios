/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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
 *	4 = tiny black, used in books.
 *	5 = little white.
 *	6 = runes.
 *	7 = normal red.
 */

/*
 *	Horizontal leads, by fontnum:
 */
static int hlead[10] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};

/*
 *	Pass space.
 */

static const char *Pass_space
	(
	const char *text
	)
	{
	while (isspace(*text))
		text++;
	return (text);
	}

/*
 *	Pass a word.
 */

static const char *Pass_word
	(
	const char *text
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
 *
 *	Output:	If out of room, -offset of end of text painted.
 *		Else height of text painted.
 */

int Game_window::paint_text_box
	(
	int fontnum,			// Font # from fonts.vga (0-9).
	const char *text,
	int x, int y,			// Top-left corner of box.
	int w, int h,			// Dimensions.
	int vert_lead			// Extra spacing between lines.
	)
	{
	const char *start = text;		// Remember the start.
	win->set_clip(x, y, w, h);
	int endx = x + w, endy = y + h;	// Figure where to stop.
	int curx = x, cury = y;
	int height = get_text_height(fontnum) + vert_lead;
	while (*text)
		{
		const char *wrd;		// ->start of word.
		switch (*text)		// Special cases.
			{
		case '\n':		// Next line.
			curx = x;
			cury += height;
			text++;
			if (cury + height > endy)
				break;	// No more room.
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
		const char *ewrd = Pass_word(text);
		int width = get_text_width(fontnum, text, ewrd - text);
		if (curx + width > endx)
			{		// Word-wrap.
			curx = x;
			cury += height;
			if (cury + height > endy)
				break;	// No more room.
			}
					// Draw word.
		curx += paint_text(fontnum, text, ewrd - text, curx, cury);
		text = ewrd;		// Continue past the word.
		}
	win->clear_clip();
	if (*text)			// Out of room?
		return -(text - start);	// Return -offset of end.
	else				// Else return height, counting last
					//   partial line.
		return (cury - y) + (curx != 0);
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
	yoff += get_text_baseline(fontnum);
	while ((chr = *text++) != 0)
		{
		Shape_frame *shape = fonts.get_shape(fontnum, chr);
		if (!shape)
			continue;
		paint_rle_shape(*shape, x, yoff);
		x += shape->get_width() + hlead[fontnum];
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
	yoff += get_text_baseline(fontnum);
	while (textlen--)
		{
		Shape_frame *shape = fonts.get_shape(fontnum, (int) *text++);
		if (!shape)
			continue;
		paint_rle_shape(*shape, x, yoff);
		x += shape->get_width() + hlead[fontnum];
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
		width += fonts.get_shape(fontnum, chr)->get_width() + 
								hlead[fontnum];
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
		width += fonts.get_shape(fontnum, *text++)->get_width() + 
								hlead[fontnum];
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

/*
 *	Get font baseline as the distance from the top.
 */

int Game_window::get_text_baseline
	(
	int fontnum
	)
	{
	Shape_frame *A = fonts.get_shape(fontnum, 'A');
	return A->get_yabove();
	}

