/**
 **	Readnpcs.cc - Read in NPC's from u7nbuf.dat & schedule.dat.
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

#include <string.h>
#include "gamewin.h"
#include "utils.h"
#include "fnames.h"

/*
 *	Read in the NPC's.
 */

void Game_window::read_npcs
	(
	)
	{
	ifstream nfile;
	u7open(nfile, NPC_DAT);
	int cnt1 = Read2(nfile);		// Get counts.
	int cnt2 = Read2(nfile);
cout << "cnt1 = " << cnt1 << ", cnt2 = " << cnt2 << '\n';
	num_npcs = cnt1 + cnt2;
	npcs = new Actor *[num_npcs];
	for (int i = 0; i < num_npcs; i++)
		{
					// Not sure about these.
		unsigned locx = Read1(nfile);
		unsigned locy = Read1(nfile);
					// Get shape, frame #.
		unsigned char shape[2];
		nfile.read(shape, 2);
		int iflag1 = Read2(nfile);// Inventory flag.
					// Superchunk #.
		int schunk = Read1(nfile);
		Read1(nfile);		// Skip next byte.
					// Get usecode function #.
		int usecode = Read2(nfile);
					// Lift is high 4 bits.
		int lift = usecode >> 12;
		usecode &= 0xfff;
		if (i >= cnt1)		// Type2?
			usecode = -1;	// Let's try this.
		nfile.seekg(4, ios::cur);// Skip 4 bytes.
					// Another inventory flag.
		int iflag2 = Read2(nfile);
					// Get name.
		nfile.seekg(85, ios::cur);
		char namebuf[17];
		nfile.read(namebuf, 16);
		namebuf[16] = 0;	// Be sure it's 0-delimited.
		int namelen = strlen(namebuf);
					// Get abs. chunk. coords. of schunk.
		int scy = 16*(schunk/12);
		int scx = 16*(schunk%12);
		int cx = locx >> 4;	// Get chunk indices within schunk.
		int cy = locy >> 4;
					// Get shape #'s.
		int shapex = locx & 0xf;
		int shapey = locy & 0xf;
		Actor *actor;
		Npc_actor *npc_actor = 0;
		if (i == 0)		// Main character?
			actor = main_actor =
				new Main_actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usecode);
		else			// Create NPC.
			actor = npc_actor = new Npc_actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usecode);
		npcs[i] = actor;	// Store in list.
		Chunk_object_list *olist = get_objects(scx + cx, scy + cy);
		actor->move(scx + cx, scy + cy, olist,
				shapex, shapey, (shape[1]>>2) & 0x1f, lift);
					// Put in chunk's NPC list.
		if (npc_actor)
			npc_actor->switched_chunks(0, olist);
#if 0
cout << i << " Creating " << namebuf << ", shape = " << 
	actor->get_shapenum() <<
	", frame = " << actor->get_framenum() << ", usecode = " <<
				usecode << '\n';
cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
	<< lift << '\n';
#endif
		if (iflag1 && iflag2)	// Inventory?  Read (but ignore++++).
			read_ireg_objects(nfile, scx, scy, actor);
		}
	read_schedules();		// Now get their schedules.
	}
/*
 *	Read NPC schedules.
 */

void Game_window::read_schedules
	(
	)
	{
	ifstream sfile;
	u7open(sfile, SCHEDULE_DAT);
	int num_npcs = Read4(sfile);	// # of NPC's, not include Avatar.
	short *offsets = new short[num_npcs];
	int i;				// Read offsets with list of scheds.
	for (i = 0; i < num_npcs; i++)
		offsets[i] = Read2(sfile);
	for (i = 0; i < num_npcs - 1; i++)	// Do each NPC, except Avatar.
		{
					// Avatar isn't included here.
		Npc_actor *npc = (Npc_actor *) npcs[i + 1];
		int cnt = offsets[i + 1] - offsets[i];
#if 0
		cout << "Schedule for " << npc->get_name() << ":\n";
#endif
					// Read schedules into this array.
		Schedule *schedules = new Schedule[cnt];
		for (int j = 0; j < cnt; j++)
			{
			unsigned char ent[4];
			sfile.read(ent, 4);
			schedules[j].set(ent);
#if 0
			cout << "    " << sched->get_type() << 
				", time = " << sched->get_time() << '\n';
#endif
			}
					// Store in NPC.
		npc->set_schedules(schedules, cnt);
		}
	delete [] offsets;		// Done with this.
	}


