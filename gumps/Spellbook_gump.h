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

#ifndef _SPELLBOOK_GUMP_H_
#define _SPELLBOOK_GUMP_H_

#include "Gump.h"

class Spellbook_object;

/*
 *	Abstract base class for spellbook, spell-scrolls:
 */
class Spelltype_gump : public Gump
	{
public:
	Spelltype_gump(int shnum) : Gump(0, shnum) {  }
	virtual ~Spelltype_gump() {  }
					// Perform spell.
	virtual void do_spell(int spell) = 0;
					// Set bookmark.
	virtual void select_spell(int spell) = 0;
	};

/*
 *	Open spellbook.  The spells are drawn in the object area.
 */
class Spellbook_gump : public Spelltype_gump
	{
	int page;			// Starting with 0 (= circle #).
	short avail[9*8];		// For each spell, # which the
					//   available reagents make possible.
	Spellbook_object *book;		// Book this shows.
	Game_object *book_owner;	// Top-owner of book.
					// Page turners:
	Gump_button *leftpage, *rightpage;
	Gump_button *spells[9*8];	// ->spell 'buttons'.
	int spwidth, spheight;		// Dimensions of a spell shape.
	void set_avail();		// Set up counts.
public:
	Spellbook_gump(Spellbook_object *b);
	~Spellbook_gump();
	virtual void do_spell(int spell);	// Perform spell.
	void change_page(int delta);	// Page forward/backward.
	virtual void select_spell(int spell);	// Set bookmark.
	virtual Game_object *get_owner();// Get object this belongs to.
					// Is a given point on a button?
	virtual Gump_button *on_button(int mx, int my);
					// Paint button.
	virtual void paint_button(Gump_button *btn);
					// Paint it and its contents.
	virtual void paint();
	};

/*
 *	Open spell-scroll in Serpent Isle.
 */
class Spellscroll_gump : public Spelltype_gump
	{
	Game_object *scroll;		// Scroll clicked on.
	Gump_button *spell;
	int spwidth, spheight;		// Dimensions of a spell shape.
public:
	Spellscroll_gump(Game_object *s);
	~Spellscroll_gump();
	virtual void do_spell(int spell);	// Perform spell.
	virtual void select_spell(int)
		{  }
	virtual Game_object *get_owner();// Get object this belongs to.
					// Is a given point on a button?
	virtual Gump_button *on_button(int mx, int my);
					// Paint button.
	virtual void paint_button(Gump_button *btn);
					// Paint it and its contents.
	virtual void paint();
	};

#endif
