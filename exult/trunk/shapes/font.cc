/*
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

#include "../alpha_kludges.h"

#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#endif
#include "files/U7file.h"
#include "files/databuf.h"
#include "font.h"
#include "ibuf8.h"
#include "vgafile.h"

using std::cout;
using std::endl;
using std::isspace;
using std::size_t;
using std::string;
using std::strncmp;

FontManager fontManager;

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

int Font::paint_text_box
	(
	Image_buffer8 *win,		// Buffer to paint in.
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
	int height = get_text_height() + vert_lead + ver_lead;
	int space_width = get_text_width(" ", 1);
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
				int w = get_text_width(text, wrd - text);
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
		int width = get_text_width(text, ewrd - text);
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
		paint_text(win, str, len, x, cury);
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

int Font::paint_text
	(
	Image_buffer8 *win,		// Buffer to paint in.
	const char *text,			// What to draw, 0-delimited.
	int xoff, int yoff		// Upper-left corner of where to start.
	)
	{
	int x = xoff;
	int chr;
	yoff += get_text_baseline();
	while ((chr = *text++) != 0)
		{
		Shape_frame *shape = font_shapes->get_frame(chr);
		if (!shape)
			continue;
		shape->paint_rle(win, x, yoff);
		x += shape->get_width() + hor_lead;
		}
	return (x - xoff);
	}

/*
 *	Paint text using font from "fonts.vga".
 *
 *	Output:	Width in pixels of what was painted.
 */

int Font::paint_text
	(
	Image_buffer8 *win,		// Buffer to paint in.
	const char *text,		// What to draw.
	int textlen,			// Length of text.
	int xoff, int yoff		// Upper-left corner of where to start.
	)
	{
	int x = xoff;
	yoff += get_text_baseline();
	while (textlen--)
		{
		Shape_frame *shape = font_shapes->get_frame((int) *text++);
		if (!shape)
			continue;
		shape->paint_rle(win, x, yoff);
		x += shape->get_width() + hor_lead;
		}
	return (x - xoff);
	}

/*
 *	Get the width in pixels of a 0-delimited string.
 */

int Font::get_text_width
	(
	const char *text
	)
	{
	int width = 0;
	short chr;
	while ((chr = *text++) != 0)
		width += font_shapes->get_frame(chr)->get_width() + hor_lead;
	return (width);
	}

/*
 *	Get the width in pixels of a string given by length.
 */

int Font::get_text_width
	(
	const char *text,
	int textlen			// Length of text.
	)
	{
	int width = 0;
	while (textlen--)
		width += font_shapes->get_frame(*text++)->get_width() + hor_lead;
	return (width);
	}

/*
 *	Get font line-height.
 */

int Font::get_text_height
	(
	)
	{
	Shape_frame *A = font_shapes->get_frame('A');
	Shape_frame *y = font_shapes->get_frame('y');
	return A->get_yabove() + y->get_ybelow() + 1;	
	}

/*
 *	Get font baseline as the distance from the top.
 */

int Font::get_text_baseline
	(
	)
	{
	Shape_frame *A = font_shapes->get_frame('A');
	return A->get_yabove();
	}

#if 0
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
#endif

Font::Font(): font_shapes(0)
{
}

Font::Font(const char *fname, int index, int hlead, int vlead): font_shapes(0)
{
	load(fname, index, hlead, vlead);
}

Font::~Font()
{
	if(font_shapes)
		delete font_shapes;
}

int Font::load(const char *fname, int index, int hlead, int vlead)
{
	if(font_shapes)
		delete font_shapes;
	char *font_buf;
	size_t len;
	U7object font_obj(fname, index);
	font_buf = font_obj.retrieve(len);
	if(!strncmp(font_buf,"font",4))	// If it's an IFF archive...
		font_buf += 8;		// Skip first 8 bytes
	BufferDataSource *font_data = new BufferDataSource(font_buf, len);
	font_shapes = new Shape_file(*font_data);
	hor_lead = hlead;
	ver_lead = vlead;
	return 0;
}

#if 0
int Font::draw_text(Game_window *gwin, int x, int y, const char *s)
{
	int xoff = x;
	int chr;
	int yoff = y+get_text_baseline();
	while ((chr = *s++) != 0) {
		Shape_frame *shape = font_shapes->get_frame(chr);
		if (!shape)
			continue;
		shape->paint_rle(gwin->get_win()->get_ib8(), x, yoff);
		x += shape->get_width() + hor_lead;
		}
	return (x - xoff);
}

int Font::get_text_width(const char *s)
{
	int width = 0;
	short chr;
	while ((chr = *s++) != 0)
		width += font_shapes->get_frame(chr)->get_width() + hor_lead;
	return width;
}

int Font::get_text_baseline() 
{
	return font_shapes->get_frame('A')->get_yabove();
}

int Font::get_text_height()
{
	return get_text_baseline()+font_shapes->get_frame('y')->get_ybelow();
}
#endif


int Font::center_text(Image_buffer8 *win, int x, int y, const char *s)
{
	return draw_text(win, x - get_text_width(s)/2, y, s);
}

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
	// FIXME: free all fonts
}

void FontManager::add_font(const char *name, const char *archive, int index, int hlead, int vlead)
{
#if DEBUG
	if(fonts[name]!=0)
		cout << "font " << name << " already here" << endl;
	else
		cout << "adding font " << name << endl;
#endif
	Font *font = new Font(archive, index, hlead, vlead);
	
	fonts[name] = font;
}

Font *FontManager::get_font(const char *name)
{
	return fonts[name];
}
