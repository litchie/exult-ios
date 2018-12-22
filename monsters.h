/*
 *  monsters.h - Monsters.
 *
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

#ifndef INCL_MONSTERS
#define INCL_MONSTERS   1

#include "actors.h"
#include "ignore_unused_variable_warning.h"

class Monster_actor;

/*
 *  Monsters get their own class because they have a bigger footprint
 *  than humans.
 */
class Monster_actor : public Npc_actor {
	static Game_object_shared in_world; // All monsters in the world.
	// Links for 'in_world' list.
    Game_object_shared next_monster;
	Monster_actor *prev_monster;
	Animator *animator;     // For wounded men.
	void link_in();         // Add to in_world list.
	void link_out();        // Remove from list.
	void equip(const Monster_info *inf, bool temporary);
public:
	Monster_actor(const std::string &nm, int shapenum, int num = -1,
	              int uc = -1);
	virtual ~Monster_actor();
	// Create an instance.
	static Game_object_shared create(int shnum);
	static Game_object_shared create(int shnum, Tile_coord pos,
	                             int sched = -1, int align = static_cast<int>(Actor::neutral),
	                             bool tempoary = true, bool equipment = true);
	// Methods to retrieve them all:
	static Monster_actor *get_first_in_world() {
		return in_world ? static_cast<Monster_actor *>(in_world.get())
			   			: nullptr;
	}
	Monster_actor *get_next_in_world() {
		return next_monster ? static_cast<Monster_actor *>(next_monster.get())
			   				: nullptr;
	}
	static void delete_all();   // Delete all monsters.
	static void give_up() {     // For file errors only!
		in_world = 0;
	}
	virtual int move_aside(Actor *for_actor, int dir) {
		ignore_unused_variable_warning(for_actor, dir);
		return 0;    // Monsters don't move aside.
	}
	// Render.
	virtual void paint();
	// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame, bool force = false);
	// Remove/delete this object.
	virtual void remove_this(Game_object_shared *keep = 0);
	// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
	                 bool combine = false, bool noset = false);
	virtual int get_armor_points(); // Get total armor value.
	// Get total weapon value.
	virtual const Weapon_info *get_weapon(int &points, int &shape,
	                                Game_object  *&obj);
	virtual int is_monster() {
		return 1;
	}
	virtual void die(Game_object *attacker);        // We're dead.
	void write(ODataSource *nfile);// Write out (to 'monsnpc.dat').
};

#endif
