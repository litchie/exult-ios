/*
 *	actorio.cc - I/O for the Actor class.
 *
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "gamewin.h"
#include "gamemap.h"
#include "game.h"
#include "monsters.h"
#include "utils.h"
#include "egg.h"
#include "chunks.h"
#include "schedule.h"
#include "objiter.h"
#include "databuf.h"

using std::ios;
using std::cout;
using std::endl;
using std::istream;
using std::ostream;

#ifdef _MSC_VER
#pragma optimize("t", off)
#endif

/*
 *	Read in actor from a given file.
 */

void Actor::read
	(
	DataSource* nfile,		// 'npc.dat', generally.
	int num,			// NPC #, or -1.
	bool has_usecode,		// 1 if a 'type1' NPC.
	bool& fix_unused		// Old savegame, so 'unused' isn't
					//   valid.  Possibly set here.
		//June9,02:  +++++Fix_unused should go away in a few months.
	)
	{
	npc_num = num;

	// This is used to get around parts of the files that we don't know
	// what the uses are. 'fix_first' is used fix issues in the originals
	// files that cause problems for the extra info we save.
	bool fix_first = Game::is_new_game();
					
	init();				// Clear rest of stuff.
	unsigned locx = nfile->read1();	// Get chunk/tile coords.
	unsigned locy = nfile->read1();
					// Read & set shape #, frame #.
	unsigned short shnum = nfile->read2();

	if (num == 0 && Game::get_game_type() != BLACK_GATE && 
						(shnum & 0x3ff) < 12)
		set_shape((shnum & 0x3ff) | 0x400);
	else
		set_shape(shnum & 0x3ff);

	set_frame(shnum >> 10);
	
	int iflag1 = nfile->read2();	// Inventory flag.
					// We're going to use these bits.
					// iflag1:0 == has_contents.
					// iflag1:1 == sched. usecode follows,
					//   possibly empty.
					// iflag1:2 == usecode # assigned by
					//   ES, so always use it.
	bool read_sched_usecode = !fix_first && (iflag1&2);
	if (!fix_first && (iflag1&4))
		usecode_assigned = true;

	int schunk = nfile->read1();	// Superchunk #.
	nfile->read1();			// Skip next byte.
	int usefun = nfile->read2();	// Get usecode function #.
	set_lift(usefun >> 12);		// Lift is high 4 bits.
	usecode = usefun & 0xfff;
//	if (!npc_num)			// Avatar is always first.
//		usecode = 0x400;
					// Need this for BG. (Not sure if SI.)
	if (npc_num >= 0 && npc_num < 256)
		usecode = 0x400 + npc_num;
					// Watch for new NPC's added.
	else if ((!has_usecode && !usecode_assigned && 
						usecode != 0x400 + npc_num) ||
	    usecode == 0xfff)
		usecode = -1;		// Let's try this.
					// Guessing:  !!  (Want to get signed.)
	int health_val = static_cast<int>(static_cast<char>(nfile->read1()));
	set_property(static_cast<int>(Actor::health), health_val);
	nfile->skip(3);	// Skip 3 bytes.
	int iflag2 = nfile->read2();	// The 'used-in-game' flag.
	if (iflag2 == 0 && num >= 0 && !fix_unused)
		{
		if (num == 0)		// Old (bad) savegame?
			fix_unused = true;
		else
			unused = true;
		cout << "NPC #" << num << " is unused" << endl;
		}

	bool has_contents = fix_first ? (iflag1 && !unused) : (iflag1&1);
	// Read first set of flags
	const int rflags = nfile->read2();
	
	if ((rflags >> 0x7) & 1) set_flag (Obj_flags::asleep);
	if ((rflags >> 0x8) & 1) set_flag (Obj_flags::charmed);
	if ((rflags >> 0x9) & 1) set_flag (Obj_flags::cursed);
	if ((rflags >> 0xB) & 1) set_flag (Obj_flags::in_party);
	if ((rflags >> 0xC) & 1) set_flag (Obj_flags::paralyzed);
	if ((rflags >> 0xD) & 1) set_flag (Obj_flags::poisoned);
	if ((rflags >> 0xE) & 1) set_flag (Obj_flags::protection);
	if (!fix_first && ((rflags >> 0xF) & 1)) 
		set_flag (Obj_flags::dead);

	// Guess
	if (((rflags >> 0xA) & 1))
		set_flag (Obj_flags::on_moving_barge);
	alignment = (rflags >> 3)&3;

	// Unknown, using for is_temporary (only when not fix_first)
	if ((rflags >> 0x6) & 1 && !fix_first) set_flag (Obj_flags::is_temporary);

	/*	Not used by exult

	Unknown in U7tech
	if ((rflags >> 0x5) & 1) set_flag (Obj_flags::unknown);
*/
	
					// Get char. atts.

	// In BG - Strength (0-5), skin colour(6-7)
	// In SI - Strength (0-4), skin colour(5-6), freeze (7)
	int strength_val = nfile->read1();

	if (Game::get_game_type() == BLACK_GATE)
	{
		set_property(static_cast<int>(Actor::strength), strength_val & 0x3F);

		if (num == 0)
		{
			if (Game::get_avskin() >= 0 && Game::get_avskin() <= 3)
				set_skin_color (Game::get_avskin());
			else
				set_skin_color (((strength_val >> 6)-1) & 0x3);
		}
		else 
			set_skin_color (3);
	}
	else
	{
		set_property(static_cast<int>(Actor::strength), strength_val & 0x1F);
		
		if (num == 0)
		{
			if (Game::get_avskin() >= 0 && Game::get_avskin() <= 2)
				set_skin_color (Game::get_avskin());
			else
				set_skin_color ((strength_val >> 5) & 0x3);
		}
		else 
			set_skin_color (-1);

		if ((strength_val << 7) & 1) set_flag (Obj_flags::freeze);
	}

	if (is_dying() &&		// Now we know health, strength.
	    npc_num > 0)		// DON'T do this for Avatar!
		set_flag(Obj_flags::dead);	// Fixes older savegames.
	// Dexterity
	set_property(static_cast<int>(Actor::dexterity), nfile->read1());


	// Intelligence (0-4), read(5), Tournament (6), polymorph (7)
	int intel_val = nfile->read1();

	set_property(static_cast<int>(Actor::intelligence), intel_val & 0x1F);
	if ((intel_val >> 5) & 1) set_flag (Obj_flags::read);
					// Tournament.
	if ((intel_val >> 6) & 1) 
		set_flag (Obj_flags::si_tournament);
	if ((intel_val >> 7) & 1) set_flag (Obj_flags::polymorph);


	// Combat skill (0-6), Petra (7)
	int combat_val = nfile->read1();
	set_property(static_cast<int>(Actor::combat), combat_val & 0x7F);
	if ((combat_val << 7) & 1) set_flag (Obj_flags::petra);

	schedule_type = nfile->read1();
	int amode = nfile->read1();	// Default attack mode
					// Just stealing 2 spare bits:
	combat_protected = (amode&(1<<4)) != 0;
	user_set_attack = (amode&(1<<5)) != 0;
	attack_mode = (Attack_mode) (amode&0xf);

	nfile->skip(3); 	//Unknown

	// If NPC 0: MaxMagic (0-4), TempHigh (5-7) and Mana(0-4), TempLow (5-7)
	// Else: ID# (0-4) ???, TempHigh (5-7) and Mat (0), No Spell Casting (1), Zombie (3), TempLow (5-7)
	int magic_val = nfile->read1();
	int mana_val = nfile->read1();

	if (num == 0)
	{
		int magic = magic_val&0x1f, mana = mana_val&0x1f;
		set_property(static_cast<int>(Actor::magic), magic);
		
		// Need to make sure that mana is less than magic
		if (mana < magic)
			set_property(static_cast<int>(Actor::mana), mana);
		else
			set_property(static_cast<int>(Actor::mana), magic);

		set_flag (Obj_flags::met);
	}
	else
	{
		set_ident(magic_val&0x1F);
		if ((mana_val >> 0) & 1) set_flag (Obj_flags::met);
		if ((mana_val >> 1) & 1) set_flag(Obj_flags::no_spell_casting);
		if ((mana_val >> 2) & 1) set_flag (Obj_flags::si_zombie);
	}


	set_temperature (((magic_val >> 2) & 0x38) + ((mana_val >> 5) & 7));

//	nfile->skip(1	);	// Index2???? (refer to U7tech.txt)
	face_num = nfile->read1();
	if (!face_num && npc_num > 0)	// Fix older savegames.
		face_num = npc_num;
	nfile->skip(2	);	// Unknown

	set_property(static_cast<int>(Actor::exp), nfile->read4());
	set_property(static_cast<int>(Actor::training), nfile->read1());


	nfile->skip (2);	// Primary Attacker
	nfile->skip (2);	// Secondary Attacker
	oppressor = nfile->read2();	// Oppressor NPC id.

	nfile->skip (4);	//I-Vr ??? (refer to U7tech.txt)

	schedule_loc.tx = nfile->read2();	//S-Vr Where npc is supposed to 
	schedule_loc.ty = nfile->read2();	//be for schedule)
	
	// Type flags 2
	int tflags = nfile->read2();

	// First time round, all the flags are garbage
	if (fix_first)
		set_type_flags (1 << Actor::tf_walk);
	else
		set_type_flags (tflags);


	if (num == 0 && Game::get_avsex() == 0)
	{
		clear_type_flag (Actor::tf_sex);
	}
	else if (num == 0 && Game::get_avsex() == 1)
	{
		set_type_flag (Actor::tf_sex);
	}
	

	nfile->skip (5);	// Unknown

	next_schedule = nfile->read1();	// Acty ????? what is this??

	nfile->skip (1);	// SN ????? (refer to U7tech.txt)
	nfile->skip (2);	// V1 ????? (refer to U7tech.txt)
	nfile->skip (2);	// V2 ????? (refer to U7tech.txt)

	// 16 Bit Shape Numbers, allows for shapes > 1023
	shnum = nfile->read2();
	if (!fix_first && shnum)
	{
		if (GAME_BG && !sman->can_use_multiracial() && shnum > 1024 && npc_num == 0)
			set_actor_shape();
		else
			set_shape(shnum);		// 16 Bit Shape Number

		shnum = (sint16) nfile->read2();	// 16 Bit Polymorph Shape Number
		if (get_flag (Obj_flags::polymorph)) 
			{			// Try to fix messed-up flag.
			if (shnum != get_shapenum())
				set_polymorph(shnum);
			else
				clear_flag(Obj_flags::polymorph);
			}
	}
	else
	{
		nfile->skip (2);
		set_polymorph_default();
	}

	// More Exult stuff
	if (!fix_first)
	{
		uint32	f;

		// Flags
		f = nfile->read4();
		flags |= f;

		// SIFlags
		f = nfile->read2();
		siflags |= f;

		// Flags2	But don't set polymorph.
		bool polym = get_flag(Obj_flags::polymorph)!= false;
		f = nfile->read4();
		flags2 |= f;
		if (!polym && get_flag(Obj_flags::polymorph))
			clear_flag(Obj_flags::polymorph);
	}
	else
	{
		// Flags
		nfile->skip (4);

		// SIFlags
		nfile->skip (2);

		// Flags2 
		nfile->skip (4);
	}

	// Skip 15
	nfile->skip (15);

					// Get (signed) food level.
	int food_read = static_cast<int>(static_cast<char>(nfile->read1()));
	if (fix_first) food_read = 18;
	set_property(static_cast<int>(Actor::food_level), food_read);

	// Skip 7
	nfile->skip(7);

	char namebuf[17];
	nfile->read(namebuf, 16);
	
	for (int i = 0; i < 16; i++)
		if (namebuf[i] == 0) 
			i = 16;
		else if (namebuf[i] < ' ' || namebuf[i] >= 127)
		{
			namebuf[0] = 0;
			break;
		}

	namebuf[16] = 0;		// Be sure it's 0-delimited.
//	cout << "Actor " << namebuf << " has alignment " << align << endl;
	
	if (num == 0 && Game::get_avname())
	{
		cout << Game::get_avname() << endl;
		name = Game::get_avname();
	}
	else
		name = namebuf;		// Store copy of it.

					// Get abs. chunk. coords. of schunk.
	int scy = 16*(schunk/12);
	int scx = 16*(schunk%12);
	if (has_contents)		// Inventory?  Read.
		gwin->get_map()->read_ireg_objects(nfile, scx, scy, this);
	if (read_sched_usecode)		// Read in scheduled usecode.
		gwin->get_map()->read_special_ireg(nfile, this);
	int cx = locx >> 4;		// Get chunk indices within schunk.
	int cy = locy >> 4;
					// Get tile #'s.
	int tilex = locx & 0xf;
	int tiley = locy & 0xf;
	set_shape_pos(tilex, tiley);
	Map_chunk *olist = gmap->get_chunk_safely(scx + cx, scy + cy);
	set_invalid();			// Not in world yet.
	if (olist && !is_dead() &&	// Valid & alive?  Put into chunk list.
	    !unused)
		{
		move((scx + cx)*c_tiles_per_chunk + tilex,
		     (scy + cy)*c_tiles_per_chunk + tiley, get_lift());
//		olist->add(this);
//		switched_chunks(0, olist);	// Put in chunk's NPC list.
		}
	// Only do ready best weapon if we are in BG, this is the first time
	// and we are the Avatar or Iolo
	if (Game::get_game_type() == BLACK_GATE && Game::get_avname() && (num == 0 || num == 1))
		ready_best_weapon();
			
#if defined(DEBUG) && 0

	cout << get_npc_num() << " Creating ";

	if (name.empty()) cout << get_name();
	else cout << name;

	cout << ", shape = " << 
		get_shapenum() <<
		", frame = " << get_framenum() << ", usecode = " <<
				usefun << endl;

	cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
		<< get_lift() << endl;

	cout << "Type Flags: ";
	if (get_type_flag(tf_fly)) cout << "fly ";
	if (get_type_flag(tf_swim)) cout << "swim ";
	if (get_type_flag(tf_walk)) cout << "walk ";
	if (get_type_flag(tf_ethereal)) cout << "ethereal ";
	if (get_type_flag(tf_in_party)) cout << "in party ";
	cout << endl;

#endif
	}

/*
 *	Write out to given file.
 */

void Actor::write
	(
	DataSource* nfile			// Generally 'npc.dat'.
	)
	{
	int i;
	unsigned char buf4[4];		// Write coords., shape, frame.

	int old_shape = get_shapenum();	// Backup shape because we might change it
	set_shape( get_shape_real() );	// Change the shape out non polymorph one
	
	Game_object::write_common_ireg(buf4);
	nfile->write(reinterpret_cast<char*>(buf4), sizeof(buf4));

	set_shape( old_shape );		// Revert the shape to what it was

					// Inventory flag.
					// Bit0 = has_contents (our use).
					// Bit1 = our savegame, with sched.
					//   usecode script following this.
					// iflag1:2 == usecode # assigned by
					//   ES, so always use it.
	int iflag1 = objects.is_empty() ? 0 : 1;
	iflag1 |= 2;			// We're always doing write_scheduled()
	if (usecode_assigned)		// # assigned by EStudio?
		iflag1 |= 4;		// Set bit 2.
	nfile->write2(iflag1);
			// Superchunk #.
	nfile->write1((get_cy()/16)*12 + get_cx()/16);
	nfile->write1(0);			// Unknown.
					// Usecode.
	int usefun = get_usecode() & 0xfff;
					// Lift is in high 4 bits.
	usefun |= ((get_lift()&15) << 12);
	nfile->write2(usefun);
	nfile->write1(get_property(Actor::health));
	nfile->write1(0);			// Unknown 3 bytes.
	nfile->write2(0);
	nfile->write2(unused ? 0 : 1);	// Write 0 if unused.

	//Write first set of flags
	int iout = 0;
	
	if (get_flag (Obj_flags::asleep)) iout |= 1 << 0x7;
	if (get_flag (Obj_flags::charmed)) iout |= 1 << 0x8;
	if (get_flag (Obj_flags::cursed)) iout |= 1 << 0x9;
	if (get_flag (Obj_flags::in_party)) iout |= 1 << 0xB;
	if (get_flag (Obj_flags::paralyzed)) iout |= 1 << 0xC;
	if (get_flag (Obj_flags::poisoned)) iout |= 1 << 0xD;
	if (get_flag (Obj_flags::protection)) iout |= 1 << 0xE;

	// Guess
	if (get_flag (Obj_flags::on_moving_barge)) iout |= 1 << 0xA;
					// Alignment is bits 3-4.

	// Unknownm using for is_temp
	if (get_flag (Obj_flags::is_temporary)) iout |= 1 << 0x6;

	iout |= ((alignment&3) << 3);

	nfile->write2(iout);
	
					// Write char. attributes.
	iout = get_property(Actor::strength);
	if (Game::get_game_type() != BLACK_GATE && npc_num == 0) iout |= (get_skin_color () & 3) << 5;
	else if (npc_num == 0) iout |= ((get_skin_color()+1) & 3) << 6;

	if (get_flag (Obj_flags::freeze)) iout |= 1 << 7;
	nfile->write1(iout);
	
	nfile->write1(get_property(Actor::dexterity));
	
	iout = get_property(Actor::intelligence);
	if (get_flag (Obj_flags::read)) iout |= 1 << 5;
					// Tournament
	if (get_flag (Obj_flags::si_tournament)) iout |= 1 << 6;
	if (get_flag (Obj_flags::polymorph)) iout |= 1 << 7;
	nfile->write1(iout);


	iout = get_property(Actor::combat);
	if (get_flag (Obj_flags::petra)) iout |= 1 << 7;
	nfile->write1(iout);
	
	nfile->write1(get_schedule_type());
	unsigned char amode = attack_mode | 
		(combat_protected ? (1<<4) : 0) |
		(user_set_attack ? (1<<5) : 0);
	nfile->write1(amode);
	nfile->write1(0);		// Skip 3.
	nfile->write2(0);

	// Magic
	int mana_val = 0;
	int magic_val = 0;
	
	if (get_npc_num() == 0)
	{
		mana_val = get_property(static_cast<int>(Actor::mana));
		magic_val = get_property(static_cast<int>(Actor::magic));
	}
	else
	{
		magic_val = get_ident() & 0x1F;
		if (get_flag (Obj_flags::met)) mana_val |= 1;
		if (get_flag (Obj_flags::no_spell_casting)) mana_val |= 1 << 1;
		if (get_flag (Obj_flags::si_zombie)) mana_val |= 1 << 2;
	}

	// Tempertures
	mana_val |= (get_temperature () & 0x1F) << 5;
	magic_val |= (get_temperature () & 0x38) << 2;

	nfile->write1 (magic_val);
	nfile->write1 (mana_val);

	nfile->write1((face_num >= 0 && face_num <= 255) ? face_num : 0);
	nfile->write2(0);		// Skip 2

	nfile->write4(get_property(Actor::exp));
	nfile->write1(get_property(Actor::training));
			// 0x40 unknown.

	nfile->write2(0);	// Skip 2*2
	nfile->write2(0);
	nfile->write2(oppressor);	// Oppressor.

	nfile->write4(0);	// Skip 2*2
	
	nfile->write2(schedule_loc.tx);	//S-Vr Where npc is supposed to 
	nfile->write2(schedule_loc.ty);	//be for schedule)
	//nfile->write4(0);

	nfile->write2(get_type_flags());	// Typeflags
	
	nfile->write4(0);	// Skip 5
	nfile->write1(0);

	nfile->write1(next_schedule);	// Acty ????? what is this??

	nfile->write1(0);		// Skip 1
	nfile->write2(0);	// Skip 2
	nfile->write2(0);	// Skip 2

	// 16 Bit Shapes
	if (get_flag (Obj_flags::polymorph))
	{
		nfile->write2(shape_save);	// 16 Bit Shape Num
		nfile->write2(get_shapenum());	// 16 Bit Polymorph Shape
	}
	else
	{
		nfile->write2(get_shapenum());	// 16 Bit Shape Num
		nfile->write2(0);		// 16 Bit Polymorph Shape
	}

	// Flags
	nfile->write4(flags);

	// SIFlags 
	nfile->write2(siflags);

	// flags2
	nfile->write4(flags2);

	// Skip 15
	for (i = 0; i < 15; i++)
		nfile->write1(0);
	
	// Food
	nfile->write1(get_property (Actor::food_level));

	// Skip 7
	for (i = 0; i < 7; i++)
		nfile->write1(0);

	char namebuf[17];		// Write 16-byte name.
	std::memset(namebuf, 0, 16);
	if (name.empty())
		std::strncpy(namebuf, Game_object::get_name().c_str(), 16);
	else
		std::strncpy(namebuf, name.c_str(), 16);
	nfile->write(namebuf, 16);
	write_contents(nfile);		// Write what he holds.
	namebuf[16] = 0;
					// Write scheduled usecode.
	Game_map::write_scheduled(nfile, this, true);
	}

/*
 *	Write out to given file.
 */

void Monster_actor::write
	(
	DataSource* nfile			// Generally 'npc.dat'.
	)
	{
	if (Actor::is_dead())		// Not alive?
		return;
	Actor::write(nfile);		// Now write.
	}

/*
 *	Write Inventory
 */

/*
 *	Write contents (if there is any).
 */

void Actor::write_contents
	(
	DataSource* out
	)
{
	if (!objects.is_empty())	// Now write out what's inside.
	{
		const int num_spots = static_cast<int>(sizeof(spots)/sizeof(spots[0]));
		sint8 i;

		for (i = 0; i < num_spots; ++i)
		{
			// Spot Increment
			if (spots[i])
			{
				// Write 2 byte index id
				out->write1(0x02);
				out->write2(static_cast<uint8>(i));
				spots[i]->write_ireg(out);
			}
		}

		Game_object *obj;
		Object_iterator next(objects);

		while ((obj = next.get_next()) != 0)
		{
			for (i = 0; i < num_spots; ++i)
				if (spots[i] == obj)
					break;

			if (i == num_spots)
			{
				// Write 2 byte index id (-1 = no slot)
				i = -1;
				out->write1(0x02);
				out->write2(static_cast<uint8>(i));
				obj->write_ireg(out);
			}
		}
		out->write1(0x01);		// A 01 terminates the list.
	}
}

