/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Spells.cc - Spellbook handling.
 **
 **	Written: 5/29/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "spells.h"
#include "gamewin.h"

/*
 *	Defines in 'gumps.vga':
 */
const int SPELLS = 33;			// First group of 9 spells.
const int TURNINGPAGE = 41;		// Animation?? (4 frames).
const int BOOKMARK = 42;		// Red ribbon, 5 frames.
const int LEFTPAGE = 44;		// At top-left of left page.
const int RIGHTPAGE = 45;		// At top-right of right page.

/*
 *	Get shape, frame for a given spell #.  There are 8 shapes, each
 *	containing 9 frames, where frame # = spell circle #.
 */
inline int Get_spell_gump_shape
	(
	int spell,			// Spell # (as used in Usecode).
	int& shape,			// Shape returned (gumps.vga).
	int& frame			// Frame returned.
	)
	{
	if (spell < 0 || spell >= 0x48)
		return 0;
	shape = spell%8;
	frame = spell/8;
	return (1);
	}

/*
 *	Get circle, given a spell #.
 */
inline int Get_circle(int spell)
	{ return spell/8; }

/*
 *	Get usecode function for a given spell:
 */
int Get_usecode(int spell)
	{ return 0x640 + spell; }

/*
 *	A 'page-turner' button.
 */
class Page_button : public Gump_button
	{
	int leftright;			// 0=left, 1=right.
public:
	Page_button(Gump_object *par, int px, int py, int lr)
		: Gump_button(par, lr ? RIGHTPAGE : LEFTPAGE, px, py),
		  leftright(lr)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Handle click.
 */

void Page_button::activate
	(
	Game_window *gwin
	)
	{
	((Spellbook_gump *) parent)->change_page(leftright ? 1 : -1);
	}

/*
 *	A spell button.
 */
class Spell_button : public Gump_button
	{
	int spell;			// Spell # (0 - 71).
public:
	Spell_button(Gump_object *par, int px, int py, int sp)
		: Gump_button(par, SPELLS + sp%8, px, py)
		{
		framenum = sp/8;	// Frame # is circle.
		}
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Handle click.
 */

void Spell_button::activate
	(
	Game_window *gwin
	)
	{
//+++++++++++++
	}

/*
 *	Create spellbook display.
 */

Spellbook_gump::Spellbook_gump
	(
	Spellbook_object *b
	) : Gump_object(0, 43), page(0), book(b)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (book->bookmark >= 0)	// Set to bookmarked page.
		page = Get_circle(book->bookmark);
	leftpage = new Page_button(this, 0, 0, 0);
					// Figure loc. of right pageturner.
	Shape_frame *bookshape = gwin->get_gump_shape(get_shapenum(), 0);
	Shape_frame *rpshape = gwin->get_gump_shape(RIGHTPAGE, 0);
	rightpage = new Page_button(this, 
		bookshape->get_width() - rpshape->get_width(), 0, 1);
					// Get dims. of a spell.
	Shape_frame *spshape = gwin->get_gump_shape(SPELLS, 0);
	spwidth = spshape->get_width();
	spheight = spshape->get_height();
	int vertspace = (object_area.h - 4*spheight)/4;
	for (int c = 0; c < 9; c++)	// Add each spell.
		{
		int spindex = c*8;
		unsigned char cflags = book->circles[c];
		for (int s = 0; s < 8; s++)
			if (cflags & (1<<s))
				spells[spindex + s] = new Spell_button(this,
					s < 4 ? object_area.x
					: object_area.x + object_area.w - 
								spwidth,
					(spheight + vertspace)*(s%4),
							spindex + s);
			else
				spells[spindex + s] = 0;
		}
	}

/*
 *	Delete.
 */

Spellbook_gump::~Spellbook_gump
	(
	)
	{
	delete leftpage;
	delete rightpage;
	for (int i = 0; i < 9*8; i++)
		delete spells[i];
	}

/*
 *	Perform a spell.
 */

void Spellbook_gump::do_spell
	(
	int spell
	)
	{
	if (spells[spell])
		{
		Game_window *gwin = Game_window::get_game_window();
		gwin->get_usecode()->call_usecode(Get_usecode(spell),
			gwin->get_main_actor(), Usecode_machine::double_click);
		}
	}

/*
 *	Change page.
 */

void Spellbook_gump::change_page
	(
	int delta
	)
	{
	page += delta;
	if (page < 0)
		page = 0;
	else if (page > 8)
		page = 8;
	paint(Game_window::get_game_window());
	}

/*
 *	Our buttons are never drawn 'pushed'.
 */

void Spellbook_gump::paint_button
	(
	Game_window *gwin,
	Gump_button *btn
	)
	{
	gwin->paint_gump(x + btn->x, y + btn->y, btn->shapenum, btn->framenum);
	}

/*
 *	Render.
 */

void Spellbook_gump::paint
	(
	Game_window *gwin
	)
	{
	Gump_object::paint(gwin);	// Paint outside & checkmark.
	if (page > 0)			// Not the first?
		paint_button(gwin, leftpage);
	if (page < 8)			// Not the last?
		paint_button(gwin, rightpage);
	int spindex = page*8;		// Index into list.
	for (int s = 0; s < 8; s++)	// Paint spells.
		if (spells[spindex + s])
			paint_button(gwin, spells[spindex + s]);
	if (book->bookmark >= 0 &&	// Bookmark?
	    book->bookmark/8 == page)
		{
		int s = book->bookmark%8;// Get # within circle.
		int bx = s < 4 ? object_area.x + spwidth/2
			: object_area.x + object_area.w - spwidth/2;
		gwin->paint_gump(x + bx, y, BOOKMARK, 1 + s%4);
		}
	gwin->set_painted();
	}
