/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Readnpcs.cc - Read in NPC's from npc.dat & schedule.dat.  Also writes
 **		npc.dat back out.
 **
 **	Written: 5/13/99 - JSF
 **/

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "gamewin.h"
#include "game.h"
#include "actors.h"
#include "ucmachine.h"
#include "utils.h"
#include "fnames.h"
#include "schedule.h"
//#include "items.h"			/* Debugging only */

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::memset;
using std::ofstream;

/*
 *	Read in the NPC's, plus the monster info.
 */

void Game_window::read_npcs
	(
	)
	{
	ifstream nfile;
	U7open(nfile, NPC_DAT);
	num_npcs1 = Read2(nfile);	// Get counts.
	int cnt2 = Read2(nfile);
	num_npcs = num_npcs1 + cnt2;
	npcs = new Actor *[num_npcs];
	memset(npcs, 0, num_npcs*sizeof(npcs[0]));
					// Create main actor.
	camera_actor = npcs[0] = main_actor = new Main_actor(nfile, 0, 0);
	if (Game::get_game_type() == BLACK_GATE)
		{
		if (usecode->get_global_flag(Usecode_machine::did_first_scene))
			main_actor->clear_flag(Obj_flags::dont_render);
		else
			main_actor->set_flag(Obj_flags::dont_render);
		}
	int i;

	// Don't like it... no i don't.
	center_view(main_actor->get_abs_tile_coord());

	for (i = 1; i < num_npcs; i++)	// Create the rest.
		npcs[i] = new Npc_actor(nfile, i, i < num_npcs1);
	nfile.close();
	main_actor->set_actor_shape();
	try
	{
		U7open(nfile, MONSNPCS);	// Monsters.
		// (Won't exist the first time; in this case U7open throws
		int cnt = Read2(nfile);
		char tmp = Read1(nfile);// Read 1 ahead to test.
		int okay = nfile.good();
		nfile.seekg(-1, ios::cur);
		while (okay && cnt--)
			{		// (Placed automatically.)
			Monster_actor *act = new Monster_actor(nfile, -1, 1);
					// Watch for corrupted file.
			if (get_shape_num_frames(act->get_shapenum()) < 16)
				act->remove_this();
			}
	}
	catch(...)
	{
	}
	read_schedules();		// Now get their schedules.
	if (!monster_info)		// Might be a 'restore'.
		{
		ifstream mfile;		// Now get monster info.
		U7open(mfile, MONSTERS);
		num_monsters = Read1(mfile);
					// Create list, and read it in.
		monster_info = new Monster_info[num_monsters];
		unsigned char monster[25];
		for (i = 0; i < num_monsters; i++)
			{
			int shape = Read2(mfile);
			mfile.read((char*)monster, 23);// Get the rest.
					// Point to flags.
			uint8 *ptr = &monster[7];
			unsigned short flags = Read2(ptr);
			ptr += 3;	// Get equip.dat offset.
			unsigned int equip = *ptr;
			monster_info[i].set(shape, monster[0], monster[1],
				monster[2], monster[3], monster[4], monster[5],
				flags, equip);
			}
		mfile.close();
		U7open(mfile, EQUIP);	// Get 'equip.dat'.
		int num_recs = Read1(mfile);
		Equip_record *equip = new Equip_record[num_recs];
		for (i = 0; i < num_recs; i++)
			{
			Equip_record& rec = equip[i];
					// 10 elements/record.
			for (int elem = 0; elem < 10; elem++)
				{
				int shnum = Read2(mfile);
				unsigned prob = Read1(mfile);
				unsigned quant = Read1(mfile);
				Read2(mfile);
				rec.set(elem, shnum, prob, quant);
				}
			}
					// Monster_info owns this.
		Monster_info::set_equip(equip, num_recs);
		}
	center_view(main_actor->get_abs_tile_coord());
	}

/*
 *	Write NPC (and monster) data back out.
 *
 *	Output:	false if error, already reported.
 */

void Game_window::write_npcs
	(
	)
	{
	ofstream nfile;
	U7open(nfile, NPC_DAT);
	Write2(nfile, num_npcs1);	// Start with counts.
	Write2(nfile, num_npcs - num_npcs1);
	int i;
	for (i = 0; i < num_npcs; i++)
		npcs[i]->write(nfile);
	nfile.flush();
	bool result = nfile.good();
	if (!result)
		throw file_write_exception(NPC_DAT);
	nfile.close();
	write_schedules();		// Write schedules
					// Now write out monsters in world.
	U7open(nfile, MONSNPCS);
	int cnt = 0;
	Write2(nfile, 0);		// Write 0 as a place holder.
	for (Monster_actor *mact = Monster_actor::get_first_in_world();
					mact; mact = mact->get_next_in_world())
		if (!mact->is_dead())	// Alive?
			{
			mact->write(nfile);
			cnt++;
			}
	nfile.seekp(0);			// Back to start.
	Write2(nfile, cnt);		// Write actual count.
	nfile.flush();
	result = nfile.good();
	nfile.close();
	if (!result)
		throw file_write_exception(NPC_DAT);
	}

/*
 *	Read NPC schedules.
 */

void Game_window::read_schedules
	(
	)
	{
	ifstream sfile;

	try
	{
		U7open(sfile, GSCHEDULE);
	}
	catch(exult_exception e)
	{
		
		cerr << "Exult Exception: "<< e.what() << 
		endl << "Trying " << SCHEDULE_DAT << endl;

		U7open(sfile, SCHEDULE_DAT);
	}

	int num_npcs = Read4(sfile);	// # of NPC's, not include Avatar.

	short *offsets = new short[num_npcs];
	int i;				// Read offsets with list of scheds.
	for (i = 0; i < num_npcs; i++)
		offsets[i] = Read2(sfile);
	for (i = 0; i < num_npcs - 1; i++)	// Do each NPC, except Avatar.
		{
					// Avatar isn't included here.
		Actor *npc = npcs[i + 1];
		int cnt = offsets[i + 1] - offsets[i];
					// Read schedules into this array.
		Schedule_change *schedules = cnt?new Schedule_change[cnt]:0;
		
		for (int j = 0; j < cnt; j++)
			{
			unsigned char ent[4];
			sfile.read((char*)ent, 4);
			schedules[j].set(ent);
			}
					// Store in NPC.
		npc->set_schedules(schedules, cnt);
		}
	delete [] offsets;		// Done with this.
	cout.flush();
	}


/*
 *	Write NPC schedules.
 */

void Game_window::write_schedules ()
{

	ofstream sfile;
	Schedule_change *schedules;
	int cnt;
	short offset = 0;
	int i;
	int num;

	// So do I allow for all NPCs (type1 and type2) - Yes i will
	num = num_npcs;

	U7open(sfile, GSCHEDULE);

	Write4(sfile, num);		// # of NPC's, not include Avatar.
	Write2(sfile, 0);		// First offfset

	for (i = 1; i < num; i++)	// write offsets with list of scheds.
	{
		npcs[i]->get_schedules(schedules, cnt);
		offset += cnt;
		Write2(sfile, offset);
	}

	for (i = 1; i < num; i++)	// Do each NPC, except Avatar.
	{
		npcs[i]->get_schedules(schedules, cnt);
		for (int j = 0; j < cnt; j++)
		{
			unsigned char ent[4];
			schedules[j].get(ent);
			sfile.write((char*)ent, 4);
		}
	}
}

void Game_window::revert_schedules(Actor *npc)
{
	// Can't do this if <= 0
	if (npc->get_npc_num() <= 0) return;

	int i;
	ifstream sfile;

	U7open(sfile, SCHEDULE_DAT);

	// # of NPC's, not include Avatar.
	int num_npcs = Read4(sfile);
	short *offsets = new short[num_npcs];
	for (i = 0; i < num_npcs; i++) offsets[i] = Read2(sfile);

	// Seek to the right place
	sfile.seekg(offsets[npc->get_npc_num()-1]*4, ios::cur);

	// Get the count that we want to use
	int cnt = offsets[npc->get_npc_num()] - offsets[npc->get_npc_num()-1];

	// Read schedules into this array.
	Schedule_change *schedules = cnt?new Schedule_change[cnt]:0;

	for (i = 0; i < cnt; i++)
	{
		unsigned char ent[4];
		sfile.read((char*)ent, 4);
		schedules[i].set(ent);
	}
	// Store in NPC.
	npc->set_schedules(schedules, cnt);

	// Done
	delete [] offsets;
}

