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
#include "game.h"
#include "actors.h"
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

/*
 *	Read in actor from a given file.
 */

Actor::Actor
	(
	istream& nfile,			// 'npc.dat', generally.
	int num,			// NPC #, or -1.
	int has_usecode			// 1 if a 'type1' NPC.
	) : Container_game_object(), npc_num(num), party_id(-1), 
	    shape_save(-1), oppressor(-1), target(0),
	    attack_mode(nearest), schedule(0), schedule_loc(0,0,0),
	    next_schedule(255), dormant(true), hit(false), alignment(0),
	    two_handed(0),
	    two_fingered(0),		//+++++ Added this. Correct? -WJP
	    light_sources(0),
	    usecode_dir(0), siflags(0), type_flags(0), ident(0),
	    action(0), frame_time(0), next_path_time(0), timers(0),
	    weapon_rect(0, 0, 0, 0)
	{
	init();				// Clear rest of stuff.
	unsigned locx = Read1(nfile);	// Get chunk/tile coords.
	unsigned locy = Read1(nfile);
					// Read & set shape #, frame #.
	unsigned short shnum = Read2(nfile);

	if (num == 0 && Game::get_game_type() != BLACK_GATE && (shnum & 0x3ff) < 12)
		set_shape((shnum & 0x3ff) | 0x400);
	else
		set_shape(shnum & 0x3ff);

	set_frame(shnum >> 10);
	
	int iflag1 = Read2(nfile);	// Inventory flag.
	int schunk = Read1(nfile);	// Superchunk #.
	Read1(nfile);			// Skip next byte.
	int usefun = Read2(nfile);	// Get usecode function #.
	set_lift(usefun >> 12);		// Lift is high 4 bits.
	usecode = usefun & 0xfff;
					// Watch for new NPC's added.
	if ((!has_usecode && usecode != 0x400 + npc_num) || !npc_num ||
	    usecode == 0xfff)
		usecode = -1;		// Let's try this.
					// Guessing:  !!  (Want to get signed.)
	int health_val = (int) (char) Read1(nfile);
	set_property((int) Actor::health, health_val);
	nfile.seekg(3, ios::cur);	// Skip 3 bytes.
	int iflag2 = Read2(nfile);	// Another inventory flag.

	// Read first set of flags
	const int rflags = Read2(nfile);
	
	if ((rflags >> 0x7) & 1) set_flag (Obj_flags::asleep);
	if ((rflags >> 0x8) & 1) set_flag (Obj_flags::charmed);
	if ((rflags >> 0x9) & 1) set_flag (Obj_flags::cursed);
	if ((rflags >> 0xB) & 1) set_flag (Obj_flags::in_party);
	if ((rflags >> 0xC) & 1) set_flag (Obj_flags::paralyzed);
	if ((rflags >> 0xD) & 1) set_flag (Obj_flags::poisoned);
	if ((rflags >> 0xE) & 1) set_flag (Obj_flags::protection);

	// Guess
	if ((rflags >> 0xA) & 1) set_flag (Obj_flags::on_moving_barge);
	alignment = (rflags >> 3)&3;

	// Unknown, using for is_temporary
	if ((rflags >> 0x6) & 1) set_flag (Obj_flags::is_temporary);

	/*	Not used by exult
	if ((rflags >> 0xF) & 1) set_flag (Obj_flags::dead);

	Unknown in U7tech
	if ((rflags >> 0x5) & 1) set_flag (Obj_flags::unknown);
*/
	
					// Get char. atts.

	// Strength (0-4), skin colour(5-6), freeze (7)
	int strength_val = Read1(nfile);

	if (Game::get_game_type() == BLACK_GATE)
	{
		set_property((int) Actor::strength, strength_val);
	}
	else
	{
		set_property((int) Actor::strength, strength_val & 0x1F);
		
		if (num == 0)
		{
			if (Game::get_avskin() >= 0 && Game::get_avskin() <= 2)
				set_skin_color (Game::get_avskin());
			else
				set_skin_color ((strength_val >> 5) & 0x3);
		}
		else 
			set_skin_color (-1);

		if ((strength_val << 7) & 1) set_siflag (Actor::freeze);
	}

	dead = is_dying();		// Now we know health, strength.

	// Dexterity
	set_property((int) Actor::dexterity, Read1(nfile));


	// Intelligence (0-4), read(5), Tournament (6), polymorph (7)
	int intel_val = Read1(nfile);

	set_property((int) Actor::intelligence, intel_val & 0x1F);
	if ((intel_val >> 5) & 1) set_siflag (Actor::read);
					// Tournament.
	if ((intel_val >> 6) & 1) 
		set_flag (Obj_flags::si_tournament);
	if ((intel_val >> 7) & 1) set_siflag (Actor::polymorph);


	// Combat skill (0-6), Petra (7)
	int combat_val = Read1(nfile);
	set_property((int) Actor::combat, combat_val & 0x7F);
	if ((combat_val << 7) & 1) set_flag (Obj_flags::petra);

	schedule_type = Read1(nfile);
	int amode = Read1(nfile);	// Default attack mode
					// Just stealing a spare bit:
	combat_protected = (amode&(1<<4)) != 0;
	attack_mode = (Attack_mode) (amode&0xf);

	nfile.seekg(3, ios::cur); 	//Unknown

	// If NPC 0: MaxMagic (0-4), TempHigh (5-7) and Mana(0-4), TempLow (5-7)
	// Else: ID# (0-4) ???, TempHigh (5-7) and Mat (0), No Spell Casting (1), Zombie (3), TempLow (5-7)
	int magic_val = Read1(nfile);
	int mana_val = Read1(nfile);


	if (num == 0)
	{
		set_property((int) Actor::magic, magic_val);
		
		// Need to make sure that mana is less than magic
		if ((mana_val & 0x1F) < (magic_val & 0x1F))
			set_property((int) Actor::mana, mana_val);
		else
			set_property((int) Actor::mana, magic_val);

		set_flag (Obj_flags::met);
	}
	else
	{
		set_ident(magic_val&0x1F);
		if ((mana_val >> 0) & 1) set_flag (Obj_flags::met);
		if ((mana_val >> 1) & 1) set_siflag (Actor::no_spell_casting);
		if ((mana_val >> 2) & 1) set_siflag (Actor::zombie);
	}


//	set_temperature (((magic_val >> 2) & 0x38) + (mana_val >> 5));

//	nfile.seekg(1	, ios::cur);	// Index2???? (refer to U7tech.txt)
	face_num = Read1(nfile);
	if (!face_num && npc_num > 0)	// Fix older savegames.
		face_num = npc_num;
	nfile.seekg(2	, ios::cur);	// Unknown

	set_property((int) Actor::exp, Read4(nfile));
	set_property((int) Actor::training, Read1(nfile));


	nfile.seekg (2, ios::cur);	// Primary Attacker
	nfile.seekg (2, ios::cur);	// Secondary Attacker
	oppressor = Read2(nfile);	// Oppressor NPC id.

	nfile.seekg (4, ios::cur);	//I-Vr ??? (refer to U7tech.txt)

	schedule_loc.tx = Read2(nfile);	//S-Vr Where npc is supposed to 
	schedule_loc.ty = Read2(nfile);	//be for schedule)
	
	// Type flags 2
	int tflags = Read2 (nfile);

	// First time round, all the flags are garbage
	int first_time = Game::is_new_game();
	if (first_time)
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

	// 16 Bit Shape Numbers, allows for shapes > 1023
	shnum = Read2(nfile);
	if (!first_time && shnum)
	{
		set_shape(shnum);		// 16 Bit Shape Number
		shnum = (sint16) Read2(nfile);	// 16 Bit Polymorph Shape Number
		if (get_siflag (polymorph)) set_polymorph(shnum);
	}
	else
	{
		nfile.seekg (2, ios::cur);
		set_polymorph_default();
	}

	// Skip 25
	nfile.seekg (25, ios::cur);

					// Get (signed) food level.
	int food_read = (int) (char) Read1(nfile);
	if (first_time)
		food_read = 18;
	set_property((int) Actor::food_level, food_read);

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
	set_shape_pos(tilex, tiley);
	Map_chunk *olist = gwin->get_chunk_safely(
							scx + cx, scy + cy);
	if (olist && !is_dead())	// Put into chunk list.
		olist->add(this);
	else
		set_invalid();		// Or set to invalid chunk.

	// Only do ready best weapon if we are in BG, this is the first time
	// and we are the Avatar or Iolo
	if (Game::get_game_type() == BLACK_GATE && Game::get_avname() && (num == 0 || num == 1))
		ready_best_weapon();
			
#ifdef DEBUG

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
	nfile.write((char*)buf4, sizeof(buf4));

	set_shape( old_shape );		// Revert the shape to what it was

					// Inventory flag.
	Write2(nfile, !objects.is_empty() ? 1 : 0);
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
	Write2(nfile, !objects.is_empty() ? 1 : 0);


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
	if (Game::get_game_type() != BLACK_GATE) iout |= (get_skin_color () & 3) << 5;
	if (get_siflag (Actor::freeze)) iout |= 1 << 7;
	nfile.put(iout);
	
	nfile.put(get_property(Actor::dexterity));
	
	iout = get_property(Actor::intelligence);
	if (get_siflag (Actor::read)) iout |= 1 << 5;
					// Tournament
	if (get_flag (Obj_flags::si_tournament)) iout |= 1 << 6;
	if (get_siflag (Actor::polymorph)) iout |= 1 << 7;
	nfile.put(iout);


	iout = get_property(Actor::combat);
	if (get_flag (Obj_flags::petra)) iout |= 1 << 7;
	nfile.put(iout);
	

	nfile.put(get_schedule_type());
	nfile.put(attack_mode | (combat_protected ? (1<<4) : 0));
	nfile.put(0);		// Skip 3.
	Write2(nfile, 0);

	// Magic
	int mana_val = 0;
	int magic_val = 0;
	
	if (get_npc_num() == 0)
	{
		mana_val = get_property((int) Actor::mana);
		magic_val = get_property((int) Actor::magic);
	}
	else
	{
		magic_val = get_ident() & 0x1F;
		if (get_flag (Obj_flags::met)) mana_val |= 1;
		if (get_siflag (Actor::no_spell_casting)) mana_val |= 1 << 1;
		if (get_siflag (Actor::zombie)) mana_val |= 1 << 2;
	}

	// Tempertures
//	mana_val |= (get_temperature () & 0x1F) << 5;
//	magic_val |= (get_temperature () & 0x38) << 2;

	nfile.put (magic_val);
	nfile.put (mana_val);

	nfile.put((face_num >= 0 && face_num <= 255) ? face_num : 0);
	Write2(nfile, 0);		// Skip 2

	Write2(nfile, get_property(Actor::exp));
	Write2(nfile, 0);		// Skip 2.
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
	if (get_siflag (polymorph))
	{
		Write2 (nfile, shape_save);	// 16 Bit Shape Num
		Write2 (nfile, get_shapenum());	// 16 Bit Polymorph Shape
	}
	else
	{
		Write2 (nfile, get_shapenum());	// 16 Bit Shape Num
		Write2 (nfile, 0);		// 16 Bit Polymorph Shape
	}

	// Skip 25
	for (i = 0; i < 25; i++)
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
	Map_chunk *olist = Game_window::get_game_window()->
				get_chunk_safely(get_cx(), get_cy());
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
		schedules(0)
	{
	Map_chunk *olist = Game_window::get_game_window()->
				get_chunk_safely(get_cx(), get_cy());
	if (olist)			// Might be invalide (a bug).
		switched_chunks(0, olist);	// Put in chunk's NPC list.
	}

/*
 *	Read in monster actor from a given file.
 */

Monster_actor::Monster_actor
	(
	istream& nfile,			// 'monster.dat', generally.
	int num,			// MONSTER #.
	int has_usecode			// 1 if a 'type1' MONSTER.
	) : Npc_actor(nfile, num, has_usecode), prev_monster(0), info(0),
	    animator(0)
	{
	init();
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
		const int num_spots = (int)(sizeof(spots)/sizeof(spots[0]));
		sint8 i;

		for (i = 0; i < num_spots; ++i)
		{
			// Spot Increment
			if (spots[i])
			{
				// Write 2 byte index id
				out.put(0x02);
				Write2 (out, (uint8) i);
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
				Write2 (out, (uint8) i);
				obj->write_ireg(out);
			}
		}
		out.put(0x01);		// A 01 terminates the list.
	}
}

