/*
 *  Gump_manager.cc - Object that manages all available gumps
 *
 *  Copyright (C) 2001  The Exult Team
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

#include "Configuration.h"
#include "exult.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "gamewin.h"

#include "Actor_gump.h"
#include "Paperdoll_gump.h"
#include "Spellbook_gump.h"
#include "Stats_gump.h"
#include "CombatStats_gump.h"
#include "Jawbone_gump.h"
#include "npcnear.h"
#include "actors.h"
#include "game.h"
#include "Audio.h"


Gump_manager::Gump_manager()
	: open_gumps(0), non_persistent_count(0), right_click_close(true)
{
	std::string str;
	config->value("config/gameplay/right_click_closes_gumps", str, "yes");
	if (str == "no")
		right_click_close = false;
	config->set("config/gameplay/right_click_closes_gumps", str, true);
}


/*
 *	Showing gumps.
 */

bool Gump_manager::showing_gumps(bool no_pers) const
{
	// If no gumps, or we do want to check for persistent, just check to see if any exist
	if (!no_pers || !open_gumps) return open_gumps != 0;

	// If we don't want to check for persistend
	for (Gump_list *gump = open_gumps; gump; gump = gump->next)
		if (!gump->gump->is_persistent()) return true;

	return false;
}


/*
 *	Find the highest gump that the mouse cursor is on.
 *
 *	Output:	->gump, or null if none.
 */

Gump *Gump_manager::find_gump
	(
	int x, int y,			// Pos. on screen.
	bool pers				// Persistent?
	)
{
	Gump_list *gmp;
	Gump *found = 0;		// We want last found in chain.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
	{
		Gump *gump = gmp->gump;
		if (gump->has_point(x,y) && (pers || !gump->is_persistent()))
			found = gump;
	}
	return (found);
}

/*
 *	Find gump containing a given object.
 */

Gump *Gump_manager::find_gump
	(
	Game_object *obj
	)
{
					// Get container object is in.
	Game_object *owner = obj->get_owner();
	if (!owner)
		return (0);
					// Look for container's gump.
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_container() == owner)
			return (gmp->gump);
	return (0);
}

/*
 *	Find gump with a given owner & shapenum.
 */

Gump *Gump_manager::find_gump
	(
	Game_object *owner,
	int shapenum
	)
{
	Gump_list *gmp;			// See if already open.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_owner() == owner &&
		    gmp->gump->get_shapenum() == shapenum)
			return gmp->gump;
	return (0);
}

/*
 *	Add a gump to the end of a chain.
 */

void Gump_manager::add_gump(Gump *gump)
{
	Gump_list *g = new Gump_list(gump);

	if (!open_gumps)
		open_gumps = g;		// First one.
	else
	{
		Gump_list *last = open_gumps;
		while (last->next) last = last->next;
		last->next = g;
	}
	if (!gump->is_persistent())	// Count 'gump mode' gumps.
		non_persistent_count++;
}

/*
 *	Close a gump and delete it
 */

bool Gump_manager::close_gump(Gump *gump)
{
	bool ret = remove_gump(gump);
	delete gump;
	return ret;
}

/*
 *	Remove a gump from the chain
 */

bool Gump_manager::remove_gump(Gump *gump)
{
	if (open_gumps)
	{
		if (open_gumps->gump == gump)
		{
			Gump_list *p = open_gumps->next;
			delete open_gumps;
			open_gumps = p;
		}
		else
		{
			Gump_list *p = open_gumps;		// Find prev. to this.
			while (p->next != 0 && p->next->gump != gump) p = p->next;

			if (p->next)
			{
				Gump_list *g = p->next->next;
				delete p->next;
				p->next = g;
			}
			else
				return true;
		}
		if (!gump->is_persistent())	// Count 'gump mode' gumps.
			non_persistent_count--;
	}

	return false;
}

/*
 *	Show a gump.
 */

void Gump_manager::add_gump
	(
	Game_object *obj,		// Object gump represents.
	int shapenum			// Shape # in 'gumps.vga'.
	)
{
	Game_window *gwin = Game_window::get_game_window();
	Main_actor *main_actor = gwin->get_main_actor();
	int paperdoll = 0;
	
	if (shapenum >= ACTOR_FIRST_GUMP && shapenum <= ACTOR_LAST_GUMP
		&& Game::get_game_type() == BLACK_GATE)
		paperdoll = 1;

	// overide for paperdolls
	if (shapenum == 123 && (Game::get_game_type() == SERPENT_ISLE ||
		(gwin->can_use_paperdolls() && gwin->get_bg_paperdolls())))
		paperdoll=2;
	else if (paperdoll && obj == main_actor)
		shapenum += main_actor->get_type_flag(Actor::tf_sex);
	
	Gump *dragged = gwin->get_dragging_gump();
	
	// If we are dragging the same, just return
	if (dragged && dragged->get_owner() == obj && dragged->get_shapenum() == shapenum)
		return;

	static int cnt = 0;		// For staggering them.
	Gump_list *gmp;			// See if already open.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_owner() == obj &&
		    gmp->gump->get_shapenum() == shapenum)
			break;

	if (gmp)			// Found it?
	{			// Move it to end.
		if (gmp->next)
		{
			Gump *gump = gmp->gump;
			remove_gump(gump);
			add_gump(gump);
		}
		gwin->paint();
		return;
	}

	int x = (1 + cnt)*gwin->get_width()/10, 
	    y = (1 + cnt)*gwin->get_height()/10;

	ShapeID s_id(shapenum, 0, paperdoll == 2 ? SF_PAPERDOL_VGA : SF_GUMPS_VGA);
    	Shape_frame *shape = s_id.get_shape();
		
	if (x + shape->get_xright() > gwin->get_width() ||
	    y + shape->get_ybelow() > gwin->get_height())
	{
		cnt = 0;
		x = gwin->get_width()/10;
		y = gwin->get_width()/10;
	}

	Gump *new_gump = 0;
	Actor *npc = dynamic_cast<Actor *>(obj);
	if (npc && paperdoll == 2)
		new_gump = new Paperdoll_gump(npc, x, y, npc->get_npc_num());
	else if (npc && paperdoll)
		new_gump = new Actor_gump(npc, x, y, shapenum);
	else if (shapenum == game->get_shape("gumps/statsdisplay"))
		new_gump = new Stats_gump((Container_game_object *) obj, x, y);
	else if (shapenum == game->get_shape("gumps/spellbook"))
		new_gump = new Spellbook_gump((Spellbook_object *) obj);
	else if (Game::get_game_type() == SERPENT_ISLE)
	{
		if (shapenum == game->get_shape("gumps/spell_scroll"))
			new_gump = new Spellscroll_gump(obj);
		else if (shapenum >= game->get_shape("gumps/cstats/1")&&
				shapenum <= game->get_shape("gumps/cstats/6"))
			new_gump = new CombatStats_gump(x, y);
		else if (shapenum == game->get_shape("gumps/jawbone"))
			new_gump = new Jawbone_gump(
					(Jawbone_object*) obj, x, y);
	}

	if (!new_gump)
		new_gump = new Container_gump((Container_game_object *) obj, x, y, shapenum);

					// Paint new one last.
	add_gump(new_gump);
	if (++cnt == 8)
		cnt = 0;
	int sfx = Audio::game_sfx(14);
	Audio::get_ptr()->play_sound_effect(sfx);	// The weird noise.
	gwin->paint();			// Show everything.
}

/*
 *	End gump mode.
 */

void Gump_manager::close_all_gumps
	(
	bool pers
	)
{
	Game_window *gwin = Game_window::get_game_window();
	bool removed = false;

	Gump_list *prev = 0;
	Gump_list *next = open_gumps;

	while (next)		// Remove all gumps.
		{
		Gump_list *gump = next;
		next = gump->next;

		// Don't delete if persistant
		if (!gump->gump->is_persistent() || pers)
		{
			if (prev) prev->next = gump->next;
			else open_gumps = gump->next;
			delete gump->gump;
			delete gump;
			removed = true;
		}
		else
			prev = gump;
	}
	non_persistent_count = 0;
	gwin->get_npc_prox()->wait(4);		// Delay "barking" for 4 secs.
	if (removed) gwin->paint();
}


/*
 *	Handle a double-click.
 */

bool Gump_manager::double_clicked
	(
	int x, int y, 			// Coords in window.
	Game_object *&obj
	)
	{
	Gump *gump = find_gump(x, y);

	if (gump)
	{			// Find object in gump.
		obj = gump->find_object(x, y);
		if (!obj)		// Maybe it's a spell.
		{
			Game_window *gwin = Game_window::get_game_window();
		 	Gump_button *btn = gump->on_button(gwin, x, y);
			if (btn) btn->double_clicked(gwin, x, y);
			else if (Game_window::get_game_window()->get_double_click_closes_gumps())
			{
				gump->close(gwin);
				gwin->paint();
			}
		}
		return true;
	}

	return false;
}

/*
 *	Update the gumps
 */
void Gump_manager::update_gumps(Game_window *gwin)
{
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		gmp->gump->update_gump(gwin);
}

/*
 *	Paint the gumps
 */
void Gump_manager::paint(Game_window *gwin)
{
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		gmp->gump->paint(gwin);
}
