/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Fontvga.cc - Handle the 'fonts.vga' file and text rendering.
 **
 **	Written: 4/29/99 - JSF
 **/

#include "fontvga.h"
#include <cctype>
#include "ibuf8.h"

using std::cout;
using std::endl;
using std::isspace;
using std::string;

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
 *
 *	This must include the Endgame fonts (currently 32-35)!!
 *      And the MAINSHP font (36)
 *	However, their values are set elsewhere
 */
static int hlead[NUM_FONTS] = {-1, 0, 1, 0, 1, 0, 0, -1, 0, 0};

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

int Fonts_vga_file::paint_text_box
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int fontnum,			// Font # from fonts.vga (0-9).
	const char *text,
	int x, int y,			// Top-left corner of box.
	int w, int h,			// Dimensions.
	int vert_lead,			// Extra spacing between lines.
	int pbreak			// End at punctuation.
	)
	{
	const char *start = text;	// Remember the start.
	win->set_clip(x, y, w, h);
	int endx = x + w;		// Figure where to stop.
	int curx = x, cury = y;
	int height = get_text_height(fontnum) + vert_lead;
	int space_width = get_text_width(fontnum, " ", 1);
	int max_lines = h/height;	// # lines that can be shown.
	string *lines = new string[max_lines + 1];
	int cur_line = 0;
	const char *last_punct_end = 0;// ->last period, qmark, etc.
					// Last punct in 'lines':
	int last_punct_line = -1, last_punct_offset = -1;

	while (*text)
		{
		switch (*text)		// Special cases.
			{
		case '\n':		// Next line.
			curx = x;
			text++;
			cur_line++;
			if (cur_line >= max_lines)
				break;	// No more room.
			continue;
		case ' ':		// Space.
		case '\t':
			{		// Pass space.
			const char *wrd = Pass_space(text);
			if (wrd != text)
				{
				int w = get_text_width(fontnum, text, 
								wrd - text);
				if (!w)
					w = space_width;
				int nsp = w/space_width;
				lines[cur_line].append(nsp, ' ');
				curx += nsp*space_width;
				}
			text = wrd;
			break;
			}
			}

		if (cur_line >= max_lines)
			break;
					// Pass word & get its width.
		const char *ewrd = Pass_word(text);
		int width = get_text_width(fontnum, text, ewrd - text);
		if (curx + width > endx)
			{		// Word-wrap.
			curx = x;
			cur_line++;
			if (cur_line >= max_lines)
				break;	// No more room.
			}

					// Store word.
		lines[cur_line].append(text, ewrd - text);
		curx += width;
		text = ewrd;		// Continue past the word.
					// Keep loc. of punct. endings.
		if (text[-1] == '.' || text[-1] == '?' || text[-1] == '!' ||
		    text[-1] == ',')
			{
			last_punct_end = text;
			last_punct_line = cur_line;
			last_punct_offset = lines[cur_line].length();
			}
		}
	if (*text &&			// Out of room?
					// Break off at end of punct.
	     pbreak && last_punct_end)
		text = Pass_space(last_punct_end);
	else
		last_punct_line = -1;
					// Render text.
	for (int i = 0; i <= cur_line; i++)
		{
		const char *str = lines[i].data();
		int len = lines[i].length();
		if (i == last_punct_line)
			len = last_punct_offset;
		paint_text(win, fontnum, str, len, x, cury);
		cury += height;
		if (i == last_punct_line)
			break;
		}
	win->clear_clip();
	delete [] lines;
	if (*text)			// Out of room?
		return -(text - start);	// Return -offset of end.
	else				// Else return height.
		return (cury - y);
	}

/*
 *	Draw text at a given location (which is the upper-left corner of the
 *	place to draw.
 *
 *	Output:	Width in pixels of what was drawn.
 */

int Fonts_vga_file::paint_text
	(
	Image_buffer8 *win,		// Buffer to paint in.
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
		Shape_frame *shape = font_get_shape(fontnum, chr);
		if (!shape)
			continue;
		shape->paint_rle(win, x, yoff);
		x += shape->get_width() + hlead[fontnum];
		}
	return (x - xoff);
	}

/*
 *	Paint text using font from "fonts.vga".
 *
 *	Output:	Width in pixels of what was painted.
 */

int Fonts_vga_file::paint_text
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int fontnum,			// Font # in fonts.vga (0-9).
	const char *text,		// What to draw.
	int textlen,			// Length of text.
	int xoff, int yoff		// Upper-left corner of where to start.
	)
	{
	int x = xoff;
	yoff += get_text_baseline(fontnum);
	while (textlen--)
		{
		Shape_frame *shape = font_get_shape(fontnum, (int) *text++);
		if (!shape)
			continue;
		shape->paint_rle(win, x, yoff);
		x += shape->get_width() + hlead[fontnum];
		}
	return (x - xoff);
	}

/*
 *	Get the width in pixels of a 0-delimited string.
 */

int Fonts_vga_file::get_text_width
	(
	int fontnum,
	const char *text
	)
	{
	int width = 0;
	short chr;
	while ((chr = *text++) != 0)
		width += font_get_shape(fontnum, chr)->get_width() + 
								hlead[fontnum];
	return (width);
	}

/*
 *	Get the width in pixels of a string given by length.
 */

int Fonts_vga_file::get_text_width
	(
	int fontnum,
	const char *text,
	int textlen			// Length of text.
	)
	{
	int width = 0;
	while (textlen--)
		width += font_get_shape(fontnum, *text++)->get_width() + 
								hlead[fontnum];
	return (width);
	}

/*
 *	Get font line-height.
 */

int Fonts_vga_file::get_text_height
	(
	int fontnum
	)
	{
	Shape_frame *A = font_get_shape(fontnum, 'A');
	Shape_frame *y = font_get_shape(fontnum, 'y');
	return A->get_yabove() + y->get_ybelow();	
	}

/*
 *	Get font baseline as the distance from the top.
 */

int Fonts_vga_file::get_text_baseline
	(
	int fontnum
	)
	{
	Shape_frame *A = font_get_shape(fontnum, 'A');
	return A->get_yabove();
	}
