/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Spells.h - Spellbook handling.
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

#ifndef INCL_SPELLS
#define INCL_SPELLS 1

#include "gumps.h"

/*
 *	Open spellbook.  The spells are drawn in the object area.
 */
class Spellbook_gump : public Gump_object
	{
	short avail[9*8];		// For each spell, # which the
					//   available reagants make possible.
	int page;			// Starting with 0 (= circle #).
	Spellbook_object *book;		// Book this shows.
					// Page turners:
	Gump_button *leftpage, *rightpage;
	Gump_button *spells[9*8];	// ->spell 'buttons'.
	int spwidth, spheight;		// Dimensions of a spell shape.
public:
	Spellbook_gump(Spellbook_object *b);
	~Spellbook_gump();
	void do_spell(int spell);	// Perform spell.
	void change_page(int delta);	// Page forward/backward.
	void set_bookmark(int spell);	// Set bookmark.
	virtual Game_object *get_owner()// Get object this belongs to.
		{ return book; }
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Paint button.
	virtual void paint_button(Game_window *gwin, Gump_button *btn);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

#endif
