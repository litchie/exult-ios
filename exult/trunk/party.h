/**
 **	Party.h - Manage the party.
 **
 **	Written: 4/8/02 - JSF
 **/
/*  Copyright (C) 2000-2003  The Exult Team
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

#ifndef INCL_PARTY_H
#define INCL_PARTY_H 1

#include "singles.h"
#include "tiles.h"

class Actor;

#define EXULT_PARTY_MAX 8

/*
 *	Manage the party.
 */
class Party_manager : public Game_singletons
	{
	int party[EXULT_PARTY_MAX];	// NPC #'s of party members.
	int party_count;		// # of NPC's in party.
	int dead_party[16];		// NPC #'s of dead party members.
	int dead_party_count;
public:
	Party_manager();
	void set_count(int n)		// For initializing from file.
		{ party_count = n; }
	void set_member(int i, int npcnum)
		{ party[i] = npcnum; }
	int get_count()			// Get # party members.
		{ return party_count; }
	int get_member(int i)		// Get npc# of i'th party member.
		{ return party[i]; }
	int get_dead_count()		// Same for dead party members.
		{ return dead_party_count; }
	int get_dead_member(int i)
		{ return dead_party[i]; }
					// Add/remove party member.
	bool add_to_party(Actor *npc);
	bool remove_from_party(Actor *npc);
	int in_dead_party(Actor *npc);
	bool add_to_dead_party(Actor *npc);
	bool remove_from_dead_party(Actor *npc);
					// Update status of NPC that died or
					//   was resurrected.
	void update_party_status(Actor *npc);
	void link_party();		// Set party's id's.
					// Formation-walking:
	void move_followers(Actor *npc, int dir);
	bool step(Actor *npc, Actor *leader, int dir, Tile_coord dest);
	};


#endif
