/*
Copyright (C) 2000-2004 The Exult Team

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

#include "Notebook_gump.h"
#include "exult_flx.h"
#include "game.h"
// #include "gamewin.h"

vector<One_note *> Notebook_gump::notes;
bool Notebook_gump::initialized = false;	// Set when read in.
vector<Notebook_top_left> Notebook_gump::page_info;

/*
 *	setup one note (from already-allocated text).
 */

void One_note::set
	(
	int d, int h, 
	int la, int ln, 
	char *txt
	)
	{
	day = d;
	hour = h;
	lat = la;
	lng = ln;
	delete text;
	text = txt;
	textlen = text ? strlen(text) : 0;
	}

/*
 *	Create notebook gump.
 */

Notebook_gump::Notebook_gump
	(
	) : Gump(0, EXULT_FLX_NOTEBOOK_SHP, SF_EXULT_FLX), curpage(0)
	{
	// +++++++++Guessing obj. area.
	set_object_area(Rectangle(36, 10, 100, 100), 7, 40);
	page_info.push_back(Notebook_top_left(0, 0));
	// ++++TESTING:
	notes.push_back(new One_note(1, 1, 10, 10, strdup("Note #1\nHello")));
	notes.push_back(new One_note(2, 2, 20, 20, strdup(
				"Note #2\nworld.\n\nHow are you?")));
	notes.push_back(new One_note(3, 3, 30, 30, strdup(
				"Note #3")));
	}
Notebook_gump *Notebook_gump::create
	(
	)
	{
	// ++++++Initialize.
	return new Notebook_gump;
	}

/*
 *	Paint a page and find where its text ends.
 *
 *	Output:	Index past end of displayed page.
 */

int Notebook_gump::paint_page
	(
	Rectangle box,			// Display box rel. to gump.
	One_note *note,			// Note to print.
	int start			// Starting offset into text.
	)
{
	const int font = 4;		// Black.
	const int vlead = 1;		// Extra inter-line spacing.
	int ypos = 0;
	int textheight = sman->get_text_height(font) + vlead;
	char *str = note->text + start;
	while (*str && ypos + textheight <= box.h)
		{
		if (*str == '\n')	// Empty paragraph?
			{
			ypos += textheight;
			str++;
			continue;
			}
		int endoff = sman->paint_text_box(font, str, x + box.x,
				y + box.y + ypos, box.w, box.h - ypos, vlead);
		if (endoff > 0)		// All painted?
			{		// Value returned is height.
			ypos += endoff;	// ????Need to do this?
			str = note->text + note->textlen;
			}
		else			// Out of room.
			{
			str += -endoff;
			break;
			}
		}
	return (str - note->text);	// Return offset past end.
}

/*
 *	Paint notebook.
 */

void Notebook_gump::paint
	(
	)
{
	Gump::paint();
	int curnote = page_info[curpage/2].notenum;
	if (curnote < 0)
		return;
	int offset = page_info[curpage/2].offset;
	One_note *note = notes[curnote];
					// Paint left page.
	offset = paint_page(Rectangle(36, 10, 122, 130), note, offset);
	if (offset >= note->textlen)	// Finished note?
		{
		if (curnote == notes.size() - 1)
			return;
		++curnote;
		note = notes[curnote];
		offset = 0;
		}
					// Paint right page.
	offset = paint_page(Rectangle(174, 10, 122, 130), note, offset);
	if (offset >= note->textlen)	// Finished note?
		{
		if (curnote == notes.size() - 1)
			return;		// No more.
		++curnote;
		offset = 0;
		}
	int nxt = curpage/2 + 1;	// For next pair of pages.
	if (nxt >= page_info.size())
		page_info.resize(nxt + 1);
	page_info[nxt].notenum = curnote;
	page_info[nxt].offset = offset;
}
