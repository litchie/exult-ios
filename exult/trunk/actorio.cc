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
	istream& nfile,			// 'npc.dat', generally.
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
	unsigned locx = Read1(nfile);	// Get chunk/tile coords.
	unsigned locy = Read1(nfile);
					// Read & set shape #, frame #.
	unsigned short shnum = Read2(nfile);

	if (num == 0 && Game::get_game_type() != BLACK_GATE && 
						(shnum & 0x3ff) < 12)
		set_shape((shnum & 0x3ff) | 0x400);
	else
		set_shape(shnum & 0x3ff);

	set_frame(shnum >> 10);
	
	int iflag1 = Read2(nfile);	// Inventory flag.
					// We're going to use these bits.
					// iflag1:0 == has_contents.
					// iflag1:1 == sched. usecode follows,
					//   possibly empty.
					// iflag1:2 == usecode # assigned by
					//   ES, so always use it.
	bool read_sched_usecode = !fix_first && (iflag1&2);
	if (!fix_first && (iflag1&4))
		usecode_assigned = true;

	int schunk = Read1(nfile);	// Superchunk #.
	Read1(nfile);			// Skip next byte.
	int usefun = Read2(nfile);	// Get usecode function #.
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
	int health_val = static_cast<int>(static_cast<char>(Read1(nfile)));
	set_property(static_cast<int>(Actor::health), health_val);
	nfile.seekg(3, ios::cur);	// Skip 3 bytes.
	int iflag2 = Read2(nfile);	// The 'used-in-game' flag.
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
	const int rflags = Read2(nfile);
	
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
	int strength_val = Read1(nfile);

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
	set_property(static_cast<int>(Actor::dexterity), Read1(nfile));


	// Intelligence (0-4), read(5), Tournament (6), polymorph (7)
	int intel_val = Read1(nfile);

	set_property(static_cast<int>(Actor::intelligence), intel_val & 0x1F);
	if ((intel_val >> 5) & 1) set_flag (Obj_flags::read);
					// Tournament.
	if ((intel_val >> 6) & 1) 
		set_flag (Obj_flags::si_tournament);
	if ((intel_val >> 7) & 1) set_flag (Obj_flags::polymorph);


	// Combat skill (0-6), Petra (7)
	int combat_val = Read1(nfile);
	set_property(static_cast<int>(Actor::combat), combat_val & 0x7F);
	if ((combat_val << 7) & 1) set_flag (Obj_flags::petra);

	schedule_type = Read1(nfile);
	int amode = Read1(nfile);	// Default attack mode
					// Just stealing 2 spare bits:
	combat_protected = (amode&(1<<4)) != 0;
	user_set_attack = (amode&(1<<5)) != 0;
	attack_mode = (Attack_mode) (amode&0xf);

	nfile.seekg(3, ios::cur); 	//Unknown

	// If NPC 0: MaxMagic (0-4), TempHigh (5-7) and Mana(0-4), TempLow (5-7)
	// Else: ID# (0-4) ???, TempHigh (5-7) and Mat (0), No Spell Casting (1), Zombie (3), TempLow (5-7)
	int magic_val = Read1(nfile);
	int mana_val = Read1(nfile);

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
		if ((mana_val >> 1) & 1) set_siflag (Actor::no_spell_casting);
		if ((mana_val >> 2) & 1) set_siflag (Actor::zombie);
	}


	set_temperature (((magic_val >> 2) & 0x38) + ((mana_val >> 5) & 7));

//	nfile.seekg(1	, ios::cur);	// Index2???? (refer to U7tech.txt)
	face_num = Read1(nfile);
	if (!face_num && npc_num > 0)	// Fix older savegames.
		face_num = npc_num;
	nfile.seekg(2	, ios::cur);	// Unknown

	set_property(static_cast<int>(Actor::exp), Read4(nfile));
	set_property(static_cast<int>(Actor::training), Read1(nfile));


	nfile.seekg (2, ios::cur);	// Primary Attacker
	nfile.seekg (2, ios::cur);	// Secondary Attacker
	oppressor = Read2(nfile);	// Oppressor NPC id.

	nfile.seekg (4, ios::cur);	//I-Vr ??? (refer to U7tech.txt)

	schedule_loc.tx = Read2(nfile);	//S-Vr Where npc is supposed to 
	schedule_loc.ty = Read2(nfile);	//be for schedule)
	
	// Type flags 2
	int tflags = Read2 (nfile);

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
	

	nfile.seekg (5, ios::cur);	// Unknown

	next_schedule = Read1(nfile);	// Acty ????? what is this??

	nfile.seekg (1, ios::cur);	// SN ????? (refer to U7tech.txt)
	nfile.seekg (2, ios::cur);	// V1 ????? (refer to U7tech.txt)
	nfile.seekg (2, ios::cur);	// V2 ????? (refer to U7tech.txt)

	Game_window *gwin = Game_window::get_game_window();

	// 16 Bit Shape Numbers, allows for shapes > 1023
	shnum = Read2(nfile);
	if (!fix_first && shnum)
	{
		if (GAME_BG && !gwin->can_use_multiracial() && shnum > 1024 && npc_num == 0)
			set_actor_shape();
		else
			set_shape(shnum);		// 16 Bit Shape Number

		shnum = (sint16) Read2(nfile);	// 16 Bit Polymorph Shape Number
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
		nfile.seekg (2, ios::cur);
		set_polymorph_default();
	}

	// More Exult stuff
	if (!fix_first)
	{
		uint32	f;

		// Flags
		f = Read4(nfile);
		flags |= f;

		// SIFlags
		f = Read2(nfile);
		siflags |= f;

		// Flags2	But don't set polymorph.
		bool polym = get_flag(Obj_flags::polymorph)!= false;
		f = Read4(nfile);
		flags2 |= f;
		if (!polym && get_flag(Obj_flags::polymorph))
			clear_flag(Obj_flags::polymorph);
	}
	else
	{
		// Flags
		nfile.seekg (4, ios::cur);

		// SIFlags
		nfile.seekg (2, ios::cur);

		// Flags2 
		nfile.seekg (4, ios::cur);
	}

	// Skip 15
	nfile.seekg (15, ios::cur);

					// Get (signed) food level.
	int food_read = static_cast<int>(static_cast<char>(Read1(nfile)));
	if (fix_first) food_read = 18;
	set_property(static_cast<int>(Actor::food_level), food_read);

	// Skip 7
	nfile.seekg(7, ios::cur);

	char namebuf[17];
	nfile.read(namebuf, 16);
	
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
	Map_chunk *olist = gwin->get_chunk_safely(scx + cx, scy + cy);
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
	ostream& nfile			// Generally 'npc.dat'.
	)
	{
	int i;
	unsigned char buf4[4];		// Write coords., shape, frame.

	int old_shape = get_shapenum();	// Backup shape because we might change it
	set_shape( get_shape_real() );	// Change the shape out non polymorph one
	
	Game_object::write_common_ireg(buf4);
	nfile.write(reinterpret_cast<char*>(buf4), sizeof(buf4));

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
	Write2(nfile, iflag1);
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
	Write2(nfile, unused ? 0 : 1);	// Write 0 if unused.

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

	Write2(nfile, iout);
	
					// Write char. attributes.
	iout = get_property(Actor::strength);
	if (Game::get_game_type() != BLACK_GATE && npc_num == 0) iout |= (get_skin_color () & 3) << 5;
	else if (npc_num == 0) iout |= ((get_skin_color()+1) & 3) << 6;

	if (get_flag (Obj_flags::freeze)) iout |= 1 << 7;
	nfile.put(iout);
	
	nfile.put(get_property(Actor::dexterity));
	
	iout = get_property(Actor::intelligence);
	if (get_flag (Obj_flags::read)) iout |= 1 << 5;
					// Tournament
	if (get_flag (Obj_flags::si_tournament)) iout |= 1 << 6;
	if (get_flag (Obj_flags::polymorph)) iout |= 1 << 7;
	nfile.put(iout);


	iout = get_property(Actor::combat);
	if (get_flag (Obj_flags::petra)) iout |= 1 << 7;
	nfile.put(iout);
	
	// Make sure schedule is correct
	update_forced_schedule();

	nfile.put(get_schedule_type());
	unsigned char amode = attack_mode | 
		(combat_protected ? (1<<4) : 0) |
		(user_set_attack ? (1<<5) : 0);
	nfile.put(amode);
	nfile.put(0);		// Skip 3.
	Write2(nfile, 0);

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
		if (get_siflag (Actor::no_spell_casting)) mana_val |= 1 << 1;
		if (get_siflag (Actor::zombie)) mana_val |= 1 << 2;
	}

	// Tempertures
	mana_val |= (get_temperature () & 0x1F) << 5;
	magic_val |= (get_temperature () & 0x38) << 2;

	nfile.put (magic_val);
	nfile.put (mana_val);

	nfile.put((face_num >= 0 && face_num <= 255) ? face_num : 0);
	Write2(nfile, 0);		// Skip 2

	Write4(nfile, get_property(Actor::exp));
	nfile.put(get_property(Actor::training));
			// 0x40 unknown.

	Write2(nfile, 0);	// Skip 2*2
	Write2(nfile, 0);
	Write2(nfile, oppressor);	// Oppressor.

	Write4(nfile, 0);	// Skip 2*2
	
	Write2(nfile, schedule_loc.tx);	//S-Vr Where npc is supposed to 
	Write2(nfile, schedule_loc.ty);	//be for schedule)
	//Write4(nfile, 0);

	Write2(nfile, get_type_flags());	// Typeflags
	
	Write4(nfile, 0);	// Skip 5
	nfile.put(0);

	nfile.put(next_schedule);	// Acty ????? what is this??

	nfile.put(0);		// Skip 1
	Write2(nfile,0);	// Skip 2
	Write2(nfile,0);	// Skip 2

	// 16 Bit Shapes
	if (get_flag (Obj_flags::polymorph))
	{
		Write2 (nfile, shape_save);	// 16 Bit Shape Num
		Write2 (nfile, get_shapenum());	// 16 Bit Polymorph Shape
	}
	else
	{
		Write2 (nfile, get_shapenum());	// 16 Bit Shape Num
		Write2 (nfile, 0);		// 16 Bit Polymorph Shape
	}

	// Flags
	Write4(nfile, flags);

	// SIFlags 
	Write2(nfile, siflags);

	// flags2
	Write4(nfile, flags2);

	// Skip 15
	for (i = 0; i < 15; i++)
		nfile.put(0);
	
	// Food
	nfile.put(get_property (Actor::food_level));

	// Skip 7
	for (i = 0; i < 7; i++)
		nfile.put(0);

	char namebuf[17];		// Write 16-byte name.
	std::memset(namebuf, 0, 16);
	if (name.empty())
		std::strncpy(namebuf, Game_object::get_name().c_str(), 16);
	else
		std::strncpy(namebuf, name.c_str(), 16);
	nfile.write(namebuf, 16);
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
	ostream& nfile			// Generally 'npc.dat'.
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
	ostream& out
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
				out.put(0x02);
				Write2 (out, static_cast<uint8>(i));
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
				out.put(0x02);
				Write2 (out, static_cast<uint8>(i));
				obj->write_ireg(out);
			}
		}
		out.put(0x01);		// A 01 terminates the list.
	}
}

