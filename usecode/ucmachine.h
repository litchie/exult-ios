/*
 *	ucmachine.cc - Interpreter for usecode.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef UCMACHINE_H
#define UCMACHINE_H

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

class Game_window;
class Usecode_machine;
class Conversation;
class Keyring;
class Game_object;
class Actor;

#include "exceptions.h"
#include "singles.h"

/*
 *	Here's our virtual machine for running usecode.  The actual internals
 *	are in Usecode_internal.
 */
class Usecode_machine : public Game_singletons
	{
	UNREPLICATABLE_CLASS(Usecode_machine);
protected:
	unsigned char gflags[1024];	// Global flags.
	Keyring* keyring;
	int party[8];			// NPC #'s of party members.
	int party_count;		// # of NPC's in party.
	int dead_party[16];		// NPC #'s of dead party members.
	int dead_party_count;

	Conversation *conv;		// Handles conversations
public:
	friend class Usecode_script;
					// Create Usecode_internal.
	static Usecode_machine *create();
	Usecode_machine();
	virtual ~Usecode_machine();
					// Read in usecode functions.
	virtual void read_usecode(std::istream& file) = 0;
					// Possible events:
	enum Usecode_events {
		npc_proximity = 0,
		double_click = 1,
		internal_exec = 2,	// Internal call via intr. 1 or 2.
		egg_proximity = 3,
		weapon = 4,		// From weapons.dat.
		readied = 5,		// Wear an item.
		unreadied = 6,		// Removed an item.
		died = 7,		// In SI only, I think.
		chat = 9	// When a NPC wants to talk to you in SI
		};
	enum Global_flag_names {
		did_first_scene = 0x3b,	// Went through 1st scene with Iolo.
		have_trinsic_password = 0x3d,
		found_stable_key = 0x48,
		left_trinsic = 0x57,
		avatar_is_thief = 0x2eb
		};
	int get_global_flag(int i)	// Get/set ith flag.
		{ return gflags[i]; }
	void set_global_flag(int i, int val = 1)
		{ gflags[i] = (val == 1); }
	int get_party_count()		// Get # party members.
		{ return party_count; }
	int get_party_member(int i)	// Get npc# of i'th party member.
		{ return party[i]; }
	int get_dead_party_count()	// Same for dead party members.
		{ return dead_party_count; }
	int get_dead_party_member(int i)
		{ return dead_party[i]; }
					// Update status of NPC that died or
					//   was resurrected.
	virtual void update_party_status(Actor *npc) = 0;
					// Start speech, or show text.
	virtual void do_speech(int num) = 0;
	virtual int in_usecode() = 0;	// Currently in a usecode function?
	Keyring* getKeyring() const { return keyring; }
					// Call desired function.
	virtual int call_usecode(int id, Game_object *obj, 
						Usecode_events event) = 0;
	virtual void write() = 0;	// Write out 'gamedat/usecode.dat'.
	virtual void read() = 0;	// Read in 'gamedat/usecode.dat'.

	void init_conversation();
	int get_num_faces_on_screen() const;

	// intercept the next click_on_item intrinsic
	virtual void intercept_click_on_item(Game_object *obj) = 0;

	};

#endif	/* INCL_USECODE */
