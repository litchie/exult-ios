/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actorio.cc - I/O for the Actor class.
 **
 **	Written: 5/12/2000 - JSF
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

#include <string.h>
#include "gamewin.h"
#include "actors.h"
#include "utils.h"
#include "egg.h"

/*
 *	Read in actor from a given file.
 */

Actor::Actor
	(
	istream& nfile,			// 'npc.dat', generally.
	int num,			// NPC #, or -1.
	int has_usecode			// 1 if a 'type1' NPC.
	) : Container_game_object(), npc_num(num), party_id(-1),
	    attack_mode(nearest), schedule(0), dormant(1), two_handed(0),
	    two_fingered(0),		//+++++ Added this. Correct? -WJP
	    light_sources(0),
	    usecode_dir(0), flags(0), action(0), frame_time(0),
	    next_path_time(0)
	{
	init();				// Clear rest of stuff.
	unsigned locx = Read1(nfile);	// Get chunk/tile coords.
	unsigned locy = Read1(nfile);
					// Read & set shape #, frame #.
	unsigned short shnum = Read2(nfile);
	set_shape(shnum & 0x3ff);
	set_frame(shnum >> 10);
	int iflag1 = Read2(nfile);	// Inventory flag.
	int schunk = Read1(nfile);	// Superchunk #.
	Read1(nfile);			// Skip next byte.
	int usefun = Read2(nfile);	// Get usecode function #.
	set_lift(usefun >> 12);		// Lift is high 4 bits.
	usecode = usefun & 0xfff;
	if (!has_usecode)		// Type2?
		usecode = -1;		// Let's try this.
					// Guessing:  !!  (Want to get signed.)
	int health_val = (int) (char) Read1(nfile);
	set_property((int) Actor::health, health_val);
	nfile.seekg(3, ios::cur);	// Skip 3 bytes.
	int iflag2 = Read2(nfile);	// Another inventory flag.
	nfile.seekg(2, ios::cur);	// Skip next 2.
					// Get char. atts.
	int strength_val = Read1(nfile);
	set_property((int) Actor::strength, strength_val);
	set_property((int) Actor::dexterity, Read1(nfile));
	set_property((int) Actor::intelligence, Read1(nfile));
	set_property((int) Actor::combat, Read1(nfile));
					// +++++For now (cheating):
	set_property((int) Actor::food_level, 18);
	schedule_type = Read1(nfile);
	nfile.seekg(4, ios::cur); 	//??
	int mana_val = Read1(nfile);	// ??
	set_property((int) Actor::mana, mana_val);
	set_property((int) Actor::magic, mana_val);
	nfile.seekg(4, ios::cur);
	set_property((int) Actor::exp, Read2(nfile));
	nfile.seekg(2, ios::cur);
	set_property((int) Actor::training, Read1(nfile));
	nfile.seekg(0x40, ios::cur);	// Get name.
	char namebuf[17];
	nfile.read(namebuf, 16);
	namebuf[16] = 0;		// Be sure it's 0-delimited.
	name = namebuf;		// Store copy of it.
	Game_window *gwin = Game_window::get_game_window();
					// Get abs. chunk. coords. of schunk.
	int scy = 16*(schunk/12);
	int scx = 16*(schunk%12);
	if (iflag1 && iflag2)		// Inventory?  Read.
		gwin->read_ireg_objects(nfile, scx, scy, this);
	int cx = locx >> 4;		// Get chunk indices within schunk.
	int cy = locy >> 4;
					// Get tile #'s.
	int tilex = locx & 0xf;
	int tiley = locy & 0xf;
	Chunk_object_list *olist = gwin->get_objects(scx + cx, scy + cy);
					// Put into chunk list.
	if (!is_dead_npc())
		move(0, scx + cx, scy + cy, olist, tilex, tiley, -1, -1);
	ready_best_weapon();		// Get best weapon in hand.
#if 0
cout << i << " Creating " << namebuf << ", shape = " << 
	actor->get_shapenum() <<
	", frame = " << actor->get_framenum() << ", usecode = " <<
				usefun << endl;
cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
	<< lift << endl;
#endif
	}

/*
 *	Write out to given file.
 */

void Actor::write
	(
	ostream& nfile			// Generally 'npc.dat'.
	)
	{
	unsigned char buf4[4];		// Write coords., shape, frame.
	Game_object::write_common_ireg(buf4);
	nfile.write((char*)buf4, sizeof(buf4));
					// Inventory flag.
	Write2(nfile, get_last_object() ? 1 : 0);
			// Superchunk #.
	nfile.put((get_cy()/16)*12 + get_cx()/16);
	nfile.put(0);			// Unknown.
					// Usecode.
	int usefun = get_usecode() & 0xfff;
					// Lift is in high 4 bits.
	usefun |= ((get_lift()&15) << 12);
	Write2(nfile, usefun);
	nfile.put(get_property(Actor::health));
	nfile.put(0);			// Unknown 3 bytes.
	Write2(nfile, 0);
					// ??Another inventory flag.
	Write2(nfile, get_last_object() ? 1 : 0);
	Write2(nfile, 0);		// 2 more unknown bytes.
					// Write char. attributes.
	nfile.put(get_property(Actor::strength));
	nfile.put(get_property(Actor::dexterity));
	nfile.put(get_property(Actor::intelligence));
	nfile.put(get_property(Actor::combat));
	nfile.put(get_schedule_type());
	Write2(nfile, 0);		// Skip 4.
	Write2(nfile, 0);
	nfile.put(get_property(Actor::mana));
	Write2(nfile, 0);		// Skip 4 again.
	Write2(nfile, 0);
	Write2(nfile, get_property(Actor::exp));
	Write2(nfile, 0);		// Skip 2.
	nfile.put(get_property(Actor::training));
			// 0x40 unknown.
	for (int i = 0; i < 0x40; i++)
		nfile.put(0);
	char namebuf[16];		// Write 16-byte name.
	memset(namebuf, 0, 16);
	strncpy(namebuf, get_name().c_str(), 16);
	nfile.write(namebuf, 16);
	write_contents(nfile);		// Write what he holds.
	}

/*
 *	Read in main actor from a given file.
 */

Main_actor::Main_actor
	(
	istream& nfile,			// 'npc.dat', generally.
	int num,			// NPC #.
	int has_usecode			// 1 if a 'type1' NPC.
	) : Actor(nfile, num, has_usecode)
	{
	Chunk_object_list *olist = Game_window::get_game_window()->
				get_objects_safely(get_cx(), get_cy());
	if (olist)
		switched_chunks(0, olist);
	}

/*
 *	Read in npc actor from a given file.
 */

Npc_actor::Npc_actor
	(
	istream& nfile,			// 'npc.dat', generally.
	int num,			// NPC #.
	int has_usecode			// 1 if a 'type1' NPC.
	) : Actor(nfile, num, has_usecode), nearby(0),
		num_schedules(0), 
		schedules(0), alignment(0)
	{
	Chunk_object_list *olist = Game_window::get_game_window()->
				get_objects_safely(get_cx(), get_cy());
	if (olist)
		switched_chunks(0, olist);	// Put in chunk's NPC list.
					// Activate schedule if not in party.
	if (Npc_actor::get_party_id() < 0)
		set_schedule_type(schedule_type);
	}

/*
 *	Read in monster actor from a given file.
 */

Monster_actor::Monster_actor
	(
	istream& nfile,			// 'monster.dat', generally.
	int num,			// MONSTER #.
	int has_usecode			// 1 if a 'type1' MONSTER.
	) : Npc_actor(nfile, num, has_usecode), prev_monster(0), creator(0),
	    info(0)
	{
	if (in_world)
		in_world->prev_monster = this;
	next_monster = in_world;
	in_world = this;
	in_world_cnt++;
	}

/*
 *	Write out to given file.
 */

void Monster_actor::write
	(
	ostream& nfile			// Generally 'npc.dat'.
	)
	{
	if (Actor::is_dead_npc())	// Not alive?
		return;
					// Created from eggs which get re-
					//   hatched when read in?
	if (creator && creator->get_criteria() == Egg_object::cached_in)
		return;
	Actor::write(nfile);		// Now write.
	}
