/*
Copyright (C) 2000 The Exult Team

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

#include "../alpha_kludges.h"

#include "actors.h"
#include "cheat.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "items.h"
#include "mouse.h"
#include "ucmachine.h"
#include "Spellbook_gump.h"
#include "spellbook.h"

extern Cheat cheat;

const int REAGANTS = 842;		// Shape #.

/*
 *	Defines in 'gumps.vga':
 */
const int SPELLS = 33;			// First group of 9 spells.
const int TURNINGPAGE = 41;		// Animation?? (4 frames).
const int BOOKMARK = 42;		// Red ribbon, 5 frames.
const int LEFTPAGE = 44;		// At top-left of left page.
const int RIGHTPAGE = 45;		// At top-right of right page.

/*
 *	Flags for required reagants.  Bits match shape #.
 */
const int bp = 1;			// Black pearl.
const int bm = 2;			// Blood moss.
const int ns = 4;			// Nightshade.
const int mr = 8;			// Mandrake root.
const int gr = 16;			// Garlic.
const int gn = 32;			// Ginseng.
const int ss = 64;			// Spider silk.
const int sa = 128;			// Sulphuras ash.
unsigned char Spellbook_gump::reagants[9*8] = {
	0, 0, 0, 0, 0, 0, 0, 0,		// Linear spells require no reagants.
					// Circle 1:
	gr|gn, gr|gn|mr, gr|gn, ns|ss, gr|ss, sa|ss, sa, ns,
					// Circle 2:
	bm|sa, bp|mr, bp|sa, mr|sa, gr|gn|mr, gr|gn|sa, bp|bm|mr,
							bm|ns|mr|sa|bp|ss,
					// Circle 3:
	gr|ns|sa, gr|gn|ss, ns|ss, ns|mr, ns|bm|bp, gr|gn|mr|sa,
						bp|ns|ss, ns|mr|bm,
					// Circle 4:
	ss|mr, bp|sa|mr, mr|bp|bm, gr|mr|ns|sa, mr|bp|bm, bm|sa,
						bm|mr|ns|ss|sa, bm|sa,
					// Circle 5:
	bp|ns|ss, mr|gr|bm, gr|bp|sa|ss, bm|bp|mr|sa, bp|ss|sa,
						gr|gn|mr|ss, bm|ns, gn|ns|ss,
					// Circle 6:
	gr|mr|ns, sa|ss|bm|gn|ns|mr, bp|mr|ss|sa, sa|bp|bm, mr|ns|sa|bm,
						ns|ss|bp, gn|ss|bp, bm|sa|mr,
					// Circle 7:
	mr|ss, bp|ns|sa, bm|bp|mr|ss|sa, bp|mr|ss|sa, bm|mr|ns|sa,
					bp|ns|ss|mr, bp|gn|mr, gr|gn|mr|sa,
					// Circle 8:
	bp|bm|gr|gn|mr|ns|ss|sa, bm|mr|ns|sa, bp|bm|mr|ns, bm|gr|gn|mr|ns,
				gr|gn|ss|sa, bm|gr|mr, bp|mr|ns, bm|gr|mr
	};

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
	Page_button(Gump *par, int px, int py, int lr)
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
	Spell_button(Gump *par, int px, int py, int sp)
		: Gump_button(par, SPELLS + sp%8, px, py), spell(sp)
		{
		framenum = sp/8;	// Frame # is circle.
		}
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	virtual void double_clicked(Game_window *gwin);
	};

/*
 *	Handle click.
 */

void Spell_button::activate
	(
	Game_window *gwin
	)
	{
	((Spellbook_gump *) parent)->set_bookmark(spell);
	}

/*
 *	Method for double-click.
 */

void Spell_button::double_clicked
	(
	Game_window *gwin
	)
	{
	((Spellbook_gump *) parent)->do_spell(spell);
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
	int reagant_counts[8];		// Count reagants.
	int r;
	for (r = 0; r < 8; r++)		// Count, by frame (frame==bit#).
		reagant_counts[r] = book_owner->count_objects(
							REAGANTS, c_any_qual, r);
	for (i = 0; i < 9*8; i++)	// Now figure what's available.
	{
		avail[i] = 10000;	// 'infinite'.
		unsigned char flags = reagants[i];
					// Go through bits.
		for (r = 0; flags; r++, flags = flags >> 1)
					// Take min. of req. reagant counts.
			if ((flags&1) && reagant_counts[r] < avail[i])
				avail[i] = reagant_counts[r];
	}
}

/*
 *	Create spellbook display.
 */

Spellbook_gump::Spellbook_gump
	(
	Spellbook_object *b
	) : Gump(0, 43), page(0), book(b)
{
					// Where to paint page marks:
	const int lpagex = 38, rpagex = 142, lrpagey = 25;
					// Get book's top owner.
	book_owner = book->get_outermost();
	set_avail();			// Figure spell counts.
	Game_window *gwin = Game_window::get_game_window();
	if (book->bookmark >= 0)	// Set to bookmarked page.
		page = Get_circle(book->bookmark);
	leftpage = new Page_button(this, lpagex, lrpagey, 0);
	rightpage = new Page_button(this, rpagex, lrpagey, 1);
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
			if ((cflags & (1<<s)) || cheat.in_wizard_mode())
				spells[spindex + s] = new Spell_button(this,
					s < 4 ? object_area.x +
						spshape->get_xleft()
					: object_area.x + object_area.w - 
						spshape->get_xright(),
					object_area.y + spshape->get_yabove() +
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
	if ((spells[spell] && avail[spell]) || cheat.in_wizard_mode())
	{
		Game_window *gwin = Game_window::get_game_window();
		int circle = spell/8;	// Figure/subtract mana.
		if (cheat.in_wizard_mode())
			circle = 0;
		int mana = gwin->get_main_actor()->get_property(Actor::mana);
		int level = gwin->get_main_actor()->get_level();
		if ((mana < circle) || (level < circle))
			// Not enough mana or not yet at required level?
		{
			Mouse::mouse->flash_shape(Mouse::redx);
			return;
		}
		gwin->get_main_actor()->set_property(Actor::mana, mana-circle);
					// Figure what we used.
		unsigned char flags = reagants[spell];

		if (!cheat.in_wizard_mode())
		{
					// Go through bits.
			for (int r = 0; flags; r++, flags = flags >> 1)
					// Remove 1 of each required reagant.
				if (flags&1)
					book_owner->remove_quantity(1, REAGANTS, c_any_qual, r);
		}
		gwin->end_gump_mode();
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
 *	Set bookmark.
 */

void Spellbook_gump::set_bookmark
	(
	int spell
	)
{
	if (spells[spell])
	{
		book->bookmark = spell;
		paint(Game_window::get_game_window());
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
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
{
	Gump_button *btn = Gump::on_button(gwin, mx, my);
	if (btn)
		return btn;
	else if (leftpage->on_button(gwin, mx, my))
		return leftpage;
	else if (rightpage->on_button(gwin, mx, my))
		return rightpage;
	int spindex = page*8;		// Index into list.
	for (int s = 0; s < 8; s++)	// Check spells.
	{
		Gump_button *spell = spells[spindex + s];
		if (spell && spell->on_button(gwin, mx, my))
			return spell;
	}
	return 0;
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
	const int numx = 1, numy = -4;// Where to draw numbers on spells,
					//   with numx being the right edge.
	Gump::paint(gwin);	// Paint outside & checkmark.
	if (page > 0)			// Not the first?
		paint_button(gwin, leftpage);
	if (page < 8)			// Not the last?
		paint_button(gwin, rightpage);
	int spindex = page*8;		// Index into list.
	for (int s = 0; s < 8; s++)	// Paint spells.
		if (spells[spindex + s] || cheat.in_wizard_mode())
		{
			Gump_button *spell = spells[spindex + s];
			paint_button(gwin, spell);
			int num = avail[spindex + s];
			char text[6];
			sprintf(text, "%d", num);
			if (num > 0 && num < 1000)
				gwin->paint_text(4, text,
					x + spell->x + numx -
						gwin->get_text_width(4, text),
					y + spell->y + numy);
		}
	if (page > 0)			// Paint circle.
	{
		char *circ = item_names[0x545];
		char *cnum = item_names[0x545 + page];
		gwin->paint_text(4, cnum, x + 40 + 
			(44 - gwin->get_text_width(4, cnum))/2, y + 20);
		gwin->paint_text(4, circ, x + 92 +
			(44 - gwin->get_text_width(4, circ))/2, y + 20);
	}
	if (book->bookmark >= 0 &&	// Bookmark?
	    book->bookmark/8 == page)
	{
		int s = book->bookmark%8;// Get # within circle.
		int bx = s < 4 ? object_area.x + spwidth/2
			: object_area.x + object_area.w - spwidth/2 - 2;
		Shape_frame *bshape = gwin->get_gump_shape(BOOKMARK, 0);
		bx += bshape->get_xleft();
		int by = object_area.y - 14 + bshape->get_yabove();
		gwin->paint_gump(x + bx, y + by, BOOKMARK, 1 + s%4);
	}
	gwin->set_painted();
}
