/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <SDL_timer.h>
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
#define SCROLLSPELLS  game->get_shape("gumps/scroll_spells") // First group of scroll spells

/*
 *	And in 'text.flx' (indices are offset from 0x500):
 */
#define CIRCLE ((GAME_BG ? 0x45 : 0x51))
#define CIRCLENUM ((GAME_BG ? 0x45 : 0x52))

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
	virtual bool activate(int button=1);
	virtual bool push(int button) {return button == 1;}
	virtual void unpush(int button) {}
	};

/*
 *	Handle click.
 */

bool Page_button::activate
	(
	int button
	)
	{
	if (button != 1) return false;
	((Spellbook_gump *) parent)->change_page(leftright ? 1 : -1);
	return true;
	}

/*
 *	The bookmark.
 */
class Bookmark_button : public Gump_button
	{
public:
	Bookmark_button(Gump *par)
		: Gump_button(par, BOOKMARK, 0, 0)
		{  }
	void set();			// Call this to set properly.
					// What to do when 'clicked':
	virtual bool activate(int button=1);
	virtual bool push(int button) { return button == 1;}
	virtual void unpush(int button) { }
	};

/*
 *	Set position and frame.
 */

void Bookmark_button::set
	(
	)
	{
	Spellbook_gump *sgump = (Spellbook_gump *) parent;
	Rectangle& object_area = sgump->object_area;
	int spwidth = sgump->spwidth;	// Spell width.
	Spellbook_object *book = sgump->book;
	int page = sgump->page;		// Page (circle) we're on.
	int bmpage = book->bookmark/8;	// Bookmark's page.
	int s = book->bookmark%8;	// Get # within circle.
					// Which side for bookmark?
	bool left = bmpage == page ? (s < 4) : bmpage < page;
					// Figure coords.
	x = left ? object_area.x + spwidth/2
		: object_area.x + object_area.w - spwidth/2 - 2;
	Shape_frame *bshape = get_shape();
	x += bshape->get_xleft();
	y = object_area.y - 14 + bshape->get_yabove();
	set_frame(bmpage == page ? (1 + s%4) : 0);
	}

/*
 *	Handle click.
 */

bool Bookmark_button::activate
	(
	int button
	)
	{
	if (button != 1) return false;
	Spellbook_gump *sgump = (Spellbook_gump *) parent;
	int bmpage = sgump->book->bookmark/8;	// Bookmark's page.
					// On a different, valid page?
	if (bmpage >= 0 && bmpage != sgump->page)
		sgump->change_page(bmpage - sgump->page);
	return true;
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
	virtual bool activate(int button=1);
	virtual void double_clicked(int x, int y);
	virtual bool push(int button) { return button == 1; }
	virtual void unpush(int button) { }
	};

/*
 *	Handle click.
 */

bool Spell_button::activate
	(
	int button
	)
	{
	if (button != 1) return false;
	((Spelltype_gump *) parent)->select_spell(spell);
	return true;
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
		avail[i] = 10000;	// 'infinite'.
		if (has_ring)
			continue;
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
	) : Spelltype_gump(SPELLBOOK), page(0), turning_page(0), book(b)
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
	bookmark = new Bookmark_button(this);
					// Get dims. of a spell.
	Shape_frame *spshape = ShapeID(SPELLS, 0, SF_GUMPS_VGA).get_shape();
	spwidth = spshape->get_width();
	spheight = spshape->get_height();
	bookmark->set();		// Set to correct position, frame.
	int vertspace = (object_area.h - 4*spheight)/4;
	int spells0 = SPELLS;
	for (int c = 0; c < 9; c++)	// Add each spell.
	{
		int spindex = c*8;
		unsigned char cflags = book->circles[c];
		for (int s = 0; s < 8; s++)
			if ((cflags & (1<<s)) || cheat.in_wizard_mode() || cheat.in_map_editor())
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
	delete bookmark;
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
#ifdef USE_EXULTSTUDIO
	if (cheat.in_map_editor()){
		if (book->has_spell(spell))
			book->remove_spell(spell);
		else
			book->add_spell(spell);
		gwin->paint();
		}
	else
#endif
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
	if (delta > 0)
		{
		if (page == 8)
			return;
		turning_page = -1;
		}
	else if (delta < 0)
		{
		if (page == 0)
			return;
		turning_page = 1;
		}
	ShapeID shape(TURNINGPAGE, 0, SF_GUMPS_VGA);
	int nframes = shape.get_num_frames();
	int i;
	turning_frame = turning_page == 1 ? 0 : nframes - 1;
	for (i = 0; i < nframes; i++)	// Animate.
		{
		if (i == nframes/2)
			{
			page += delta;	// Change page halfway through.
			bookmark->set();// Update bookmark for new page.
			}
		gwin->add_dirty(get_rect());
		gwin->paint_dirty();
		gwin->show();
		SDL_Delay(50);		// 1/20 sec.
		}
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
		bookmark->set();	// Update bookmark's position/frame.
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
	if (bookmark->on_button(mx, my))
		return bookmark;
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
			if (GAME_BG && page == 0 && !cheat.in_map_editor())	// No quantities for 0th circle in BG.
				continue;
			int num = avail[spindex + s];
			char text[7];
#ifdef USE_EXULTSTUDIO
			if (cheat.in_map_editor()){
				unsigned char cflags = book->circles[page];
				if (cflags & (1<<s)) // has spell
					std::strcpy(text, "remove");
				else
					std::strcpy(text, "add");
			}
			else
#endif
			if (num > 0 || cheat.in_wizard_mode()) {
				if ((num >= 1000 || cheat.in_wizard_mode()) && GAME_SI)
					std::strcpy(text, "#"); // # = infinity in SI's font 5
				else if (num > 99 || cheat.in_wizard_mode())
					std::strcpy(text, "99");
				else
					snprintf(text, 6, "%d", num);
			}
			else	// prevent garbage text
				std::strcpy(text, "");
			sman->paint_text(5, text,
					x + spell->x + numx - sman->get_text_width(5, text),
					y + spell->y + numy);
		}
	if (page > 0 ||	GAME_SI)		// Paint circle.
	{
		char *circ = misc_names[CIRCLE];
		char *cnum = misc_names[CIRCLENUM + page];
		sman->paint_text(5, cnum, x + 40 + 
			(44 - sman->get_text_width(5, cnum))/2, y + 20);
		sman->paint_text(5, circ, x + 92 +
			(44 - sman->get_text_width(5, circ))/2, y + 20);
	}
	if (book->bookmark >= 0)	// Bookmark?
		paint_button(bookmark);
	if (turning_page)		// Animate turning page.
		{
		const int TPXOFF = 5, TPYOFF = 3;
		ShapeID shape(TURNINGPAGE, turning_frame, SF_GUMPS_VGA);
		Shape_frame *fr = shape.get_shape();
		int spritex = x + object_area.x + fr->get_xleft() + TPXOFF;
		int spritey = y + fr->get_yabove() + TPYOFF;
		shape.paint_shape(spritex, spritey);
		turning_frame += turning_page;
		if (turning_frame < 0 || turning_frame >= 
						shape.get_num_frames())
			turning_page = 0;	// Last one.
		}
	gwin->set_painted();
}

/*
 *	Create spellscroll display.
 */

Spellscroll_gump::Spellscroll_gump
	(
	Game_object *s
	) : Spelltype_gump(game->get_shape("gumps/spell_scroll")), scroll(s), spell(0)
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

