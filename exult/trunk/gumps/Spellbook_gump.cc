/*
 *  Copyright (C) 2000-2002  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "actors.h"
#include "cheat.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "items.h"
#include "mouse.h"
#include "Spellbook_gump.h"
#include "spellbook.h"
#include "game.h"
#include "Gump_manager.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif

using std::snprintf;


const int REAGENTS = 842;		// Shape #.

/*
 *	Defines in 'gumps.vga':
 */
#define SPELLBOOK (GAME_BG ? 43 : 38)
#define SPELLS  (GAME_BG ? 33 : 28)		// First group of 9 spells.
#define TURNINGPAGE  (GAME_BG ? 41 : 36)	// Animation?? (4 frames).
#define BOOKMARK  (GAME_BG ? 42 : 37)	// Red ribbon, 5 frames.
#define LEFTPAGE  (GAME_BG ? 44 : 39)	// At top-left of left page.
#define RIGHTPAGE  (GAME_BG ? 45 : 40)	// At top-right of right page.
#define SCROLLSPELLS  66		// First group of scroll spells (SI).

/*
 *	And in 'text.flx':
 */
#define CIRCLE (GAME_BG ? 0x545 : 0x551)
#define CIRCLENUM (GAME_BG ? 0x545 : 0x552)

/*
 *	Get circle, given a spell #.
 */
inline int Get_circle(int spell)
	{ return spell/8; }

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
 *	A 'page-turner' button.
 */
class Page_button : public Gump_button
	{
	int leftright;			// 0=left, 1=right.
public:
	Page_button(Gump *par, int px, int py, int lr)
		: Gump_button(par, lr ? RIGHTPAGE : LEFTPAGE, px, py),
		  leftright(lr)
		{  }
					// What to do when 'clicked':
	virtual void activate();
	virtual void push() {}
	virtual void unpush() {}
	};

/*
 *	Handle click.
 */

void Page_button::activate
	(
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
	Spell_button(Gump *par, int px, int py, int sp, int shnum, int frnum)
		: Gump_button(par, shnum, px, py), spell(sp)
		{
		set_frame(frnum);	// Frame # is circle.
		}
					// What to do when 'clicked':
	virtual void activate();
	virtual void double_clicked(int x, int y);
	virtual void push() { }
	virtual void unpush() { }
	};

/*
 *	Handle click.
 */

void Spell_button::activate
	(
	)
	{
	((Spelltype_gump *) parent)->select_spell(spell);
	}

/*
 *	Method for double-click.
 */

void Spell_button::double_clicked
	(
	int x, int y
	)
	{
	((Spelltype_gump *) parent)->do_spell(spell);
	}

/*
 *	Figure the availability of the spells.
 */

void Spellbook_gump::set_avail
	(
	)
{
	int i;				// Init.
	for (i = 0; i < 9*8; i++)
		avail[i] = 0;
	if (book_owner == book)
		return;			// Nobody owns it.
	int reagent_counts[NREAGENTS];	// Count reagents.
	int r;
	for (r = 0; r < NREAGENTS; r++)	// Count, by frame (frame==bit#).
		reagent_counts[r] = book_owner->count_objects(
						REAGENTS, c_any_qual, r);
	bool has_ring = book->has_ring(gwin->get_main_actor());
	for (i = 0; i < 9*8; i++)	// Now figure what's available.
	{
		if (has_ring)
			{
			avail[i] = 10000;
			continue;
			}
		avail[i] = 10000;	// 'infinite'.
		unsigned short flags = book->reagents[i];
					// Go through bits.
		for (r = 0; flags; r++, flags = flags >> 1)
					// Take min. of req. reagent counts.
			if ((flags&1) && reagent_counts[r] < avail[i])
				avail[i] = reagent_counts[r];
	}
}

/*
 *	Create spellbook display.
 */

Spellbook_gump::Spellbook_gump
	(
	Spellbook_object *b
	) : Spelltype_gump(SPELLBOOK), page(0), book(b)
{
	set_object_area(Rectangle(36, 28, 102, 66), 7, 54);

					// Where to paint page marks:
	const int lpagex = 38, rpagex = 142, lrpagey = 25;
					// Get book's top owner.
	book_owner = book->get_outermost();
	set_avail();			// Figure spell counts.
	if (book->bookmark >= 0)	// Set to bookmarked page.
		page = Get_circle(book->bookmark);
	leftpage = new Page_button(this, lpagex, lrpagey, 0);
	rightpage = new Page_button(this, rpagex, lrpagey, 1);
					// Get dims. of a spell.
	Shape_frame *spshape = ShapeID(SPELLS, 0, SF_GUMPS_VGA).get_shape();
	spwidth = spshape->get_width();
	spheight = spshape->get_height();
	int vertspace = (object_area.h - 4*spheight)/4;
	int spells0 = SPELLS;
	for (int c = 0; c < 9; c++)	// Add each spell.
	{
		int spindex = c*8;
		unsigned char cflags = book->circles[c];
		for (int s = 0; s < 8; s++)
			if ((cflags & (1<<s)) || cheat.in_wizard_mode())
				{
				int spnum = spindex + s;
				spells[spnum] = new Spell_button(this,
					s < 4 ? object_area.x +
						spshape->get_xleft() + 1
					: object_area.x + object_area.w - 
						spshape->get_xright() - 2,
					object_area.y + spshape->get_yabove() +
						(spheight + vertspace)*(s%4),
					spnum,
					spells0 + spnum%8, spnum/8);
				}
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
	if (!book->can_do_spell(gwin->get_main_actor(), spell))
		Mouse::mouse->flash_shape(Mouse::redx);
	else
		{
		Spellbook_object *save_book = book;
		close();		// We've just been deleted!
		gwin->paint();
		gwin->show();
					// Don't need to check again.
		save_book->do_spell(gwin->get_main_actor(), spell, true);
					// Close all gumps so animations can
					//   start.
		gumpman->close_all_gumps();
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
	paint();
}

/*
 *	Set bookmark.
 */

void Spellbook_gump::select_spell
	(
	int spell
	)
{
	if (spells[spell])
	{
		book->bookmark = spell;
		paint();
	}
}

/*
 *	Get object that 'owns' this.
 */
Game_object *Spellbook_gump::get_owner()
{
	return book; 
}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Spellbook_gump::on_button
	(
	int mx, int my			// Point in window.
	)
{
	Gump_button *btn = Gump::on_button(mx, my);
	if (btn)
		return btn;
	else if (leftpage->on_button(mx, my))
		return leftpage;
	else if (rightpage->on_button(mx, my))
		return rightpage;
	int spindex = page*8;		// Index into list.
	for (int s = 0; s < 8; s++)	// Check spells.
	{
		Gump_button *spell = spells[spindex + s];
		if (spell && spell->on_button(mx, my))
			return spell;
	}
	return 0;
}

/*
 *	Our buttons are never drawn 'pushed'.
 */

void Spellbook_gump::paint_button
	(
	Gump_button *btn
	)
{
	btn->paint();
}

/*
 *	Render.
 */

void Spellbook_gump::paint
	(
	)
{
	const int numx = 1, numy = -4;// Where to draw numbers on spells,
					//   with numx being the right edge.
	Gump::paint();			// Paint outside & checkmark.
	if (page > 0)			// Not the first?
		paint_button(leftpage);
	if (page < 8)			// Not the last?
		paint_button(rightpage);
	int spindex = page*8;		// Index into list.
	for (int s = 0; s < 8; s++)	// Paint spells.
		if (spells[spindex + s])
		{
			Gump_button *spell = spells[spindex + s];
			paint_button(spell);
			int num = avail[spindex + s];
			char text[6];
			if (num > 0 && num < 1000)
				{
				snprintf(text, 6, "%d", num);
				sman->paint_text(4, text,
					x + spell->x + numx -
						sman->get_text_width(4, text),
					y + spell->y + numy);
				}
			else if (num >= 1000)	// Fake an 'infinity'.
				{
				std::strcpy(text, "oo");
				int px = x + spell->x + numx + 2 -
						sman->get_text_width(4, text);
				sman->paint_text(4, text + 1, px,
					y + spell->y + numy);
				sman->paint_text(4, text + 1, px + 3,
					y + spell->y + numy);
				}
		}
	if (page > 0 ||			// Paint circle.
	    Game::get_game_type() == SERPENT_ISLE)
	{
		char *circ = item_names[CIRCLE];
		char *cnum = item_names[CIRCLENUM + page];
		sman->paint_text(4, cnum, x + 40 + 
			(44 - sman->get_text_width(4, cnum))/2, y + 20);
		sman->paint_text(4, circ, x + 92 +
			(44 - sman->get_text_width(4, circ))/2, y + 20);
	}
	if (book->bookmark >= 0 &&	// Bookmark?
	    book->bookmark/8 == page)
	{
		int s = book->bookmark%8;// Get # within circle.
		int bx = s < 4 ? object_area.x + spwidth/2
			: object_area.x + object_area.w - spwidth/2 - 2;
		ShapeID bm(BOOKMARK, 1 + s%4, SF_GUMPS_VGA);
		Shape_frame *bshape = bm.get_shape();
		bx += bshape->get_xleft();
		int by = object_area.y - 14 + bshape->get_yabove();
		bm.paint_shape(x + bx, y + by);
	}
	gwin->set_painted();
}

/*
 *	Create spellscroll display.
 */

Spellscroll_gump::Spellscroll_gump
	(
	Game_object *s
	) : Spelltype_gump(65), scroll(s), spell(0)
	{
	set_object_area(Rectangle(30, 29, 50, 29), 8, 68);

					// Get dims. of a spell.
	Shape_frame *spshape = ShapeID(SCROLLSPELLS, 0, SF_GUMPS_VGA).get_shape();
	spwidth = spshape->get_width();
	spheight = spshape->get_height();
	int spellnum = scroll->get_quality();
	if (spellnum >= 0 && spellnum < 8*9)
		spell = new Spell_button(this, 
				object_area.x + 4 + spshape->get_xleft(), 
				object_area.y + 4 + spshape->get_yabove(), 
				spellnum, SCROLLSPELLS + spellnum/8,
				spellnum%8);
	}

/*
 *	Delete.
 */

Spellscroll_gump::~Spellscroll_gump
	(
	)
	{
	delete spell;
	}

/*
 *	Perform the spell.
 */

void Spellscroll_gump::do_spell
	(
	int spellnum
	)
	{
	scroll->remove_this();		// Scroll is gone.
	scroll = 0;
	close();			// We've just been deleted!
	gwin->paint();
	gwin->show();
	Spellbook_object::execute_spell(gwin->get_main_actor(), spellnum);
	}

/*
 *	Return scroll.
 */

Game_object *Spellscroll_gump::get_owner
	(
	)
	{
	return scroll;
	}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Spellscroll_gump::on_button
	(
	int mx, int my			// Point in window.
	)
	{
	Gump_button *btn = Gump::on_button(mx, my);
	if (btn)
		return btn;
	else if (spell && spell->on_button(mx, my))
		return spell;
	return 0;
	}

/*
 *	Our buttons are never drawn 'pushed'.
 */

void Spellscroll_gump::paint_button
	(
	Gump_button *btn
	)
	{
	btn->paint();
	}

/*
 *	Render.
 */

void Spellscroll_gump::paint
	(
	)
	{
	Gump::paint();			// Paint outside & checkmark.
	if (spell)
		paint_button(spell);
	gwin->set_painted();
	}

