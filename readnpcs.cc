/*
 *	readnpcs.cc - Read in NPC's from npc.dat & schedule.dat.  Also writes npc.dat back out.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "gamewin.h"
#include "game.h"
#include "monsters.h"
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
	npcs.resize(1);			// Create main actor.
	camera_actor = npcs[0] = main_actor = new Main_actor("", 0);
	ifstream nfile;
	int num_npcs;
	try
		{
		U7open(nfile, NPC_DAT);
		num_npcs1 = Read2(nfile);	// Get counts.
		num_npcs = num_npcs1 + Read2(nfile);
		main_actor->read(nfile, 0, 0);
		}
	catch(exult_exception &e)
		{
		if (!Game::is_editing())
			throw e;
		num_npcs1 = num_npcs = 1; 
		main_actor->set_invalid();	// Put in middle of world.
		main_actor->move(c_num_tiles/2, c_num_tiles/2, 0);
		}
	npcs.resize(num_npcs);
	bodies.resize(num_npcs);
	int i;

	// Don't like it... no i don't.
	center_view(main_actor->get_tile());
					// Set ranges to skip.
	int skip1f, skip1t, skip2f, skip2t;
	if (Game::get_game_type() == SERPENT_ISLE)
		{			// SI: 231-255 are bogus automatons(?).
		skip1f = 233; skip1t = 256;
					// SI: 296-355 are bogus trappers.
		skip2f = 296; skip2t = 356;
		}
	else if (Game::get_game_type() == BLACK_GATE)
		{			// BG: 293-297 are extra mages.
		skip1f = 293; skip1t = 298;
		skip2f = skip2t = 10000;
		}
	else
		skip1f = skip1t = skip2f = skip2t = 10000;
	for (i = 1; i < num_npcs; i++)	// Create the rest.
	{
		npcs[i] = new Npc_actor("", 0);
		npcs[i]->read(nfile, i, i < num_npcs1);
		if ((i >= skip1f && i < skip1t) || (i >= skip2f && i < skip2t))
			{
			npcs[i]->remove_this(1);
			npcs[i]->set_schedule_type(Schedule::wait);
			}
		else
			npcs[i]->restore_schedule();
		CYCLE_RED_PLASMA();
	}
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
		{
					// Read ahead to get shape.
			nfile.seekg(2, ios::cur);
			unsigned short shnum = Read2(nfile)&0x3ff;
			okay = nfile.good();
			nfile.seekg(-4, ios::cur);
			ShapeID sid(shnum, 0);
			if (!okay || sid.get_num_frames() < 16)
				break;	// Watch for corrupted file.
			Monster_actor *act = Monster_actor::create(shnum);
			act->read(nfile, -1, 1);
			act->restore_schedule();
			CYCLE_RED_PLASMA();
		}
	}
	catch(exult_exception &e) {
		cerr << "Error reading saved monsters.  Clearing list." << endl;
		Monster_actor::give_up();
	}
	if (moving_barge)		// Gather all NPC's on barge.
	{
		Barge_object *b = moving_barge;
		moving_barge = 0;
		set_moving_barge(b);
	}
	read_schedules();		// Now get their schedules.
	center_view(main_actor->get_tile());
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
	int num_npcs = npcs.size();
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
	int num_npcs = 0;
	try
	{
		U7open(sfile, GSCHEDULE);
	}
	catch(exult_exception e)
	{
		
		cerr << "Exult Exception: "<< e.what() << 
		endl << "Trying " << SCHEDULE_DAT << endl;
		try
		{
			U7open(sfile, SCHEDULE_DAT);
		}
		catch(exult_exception e1)
		{
		if (!Game::is_editing())
			throw e1;
		else
			return;
		}
	}
	num_npcs = Read4(sfile);	// # of NPC's, not include Avatar.

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
			sfile.read(reinterpret_cast<char*>(ent), 4);
			schedules[j].set(ent);
			}
					// Store in NPC.
		if (npc)
			npc->set_schedules(schedules, cnt);
		else
			delete schedules;
		CYCLE_RED_PLASMA();
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
	num = npcs.size();

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
			sfile.write(reinterpret_cast<char*>(ent), 4);
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
		sfile.read(reinterpret_cast<char*>(ent), 4);
		schedules[i].set(ent);
	}
	// Store in NPC.
	npc->set_schedules(schedules, cnt);

	// Done
	delete [] offsets;
}

