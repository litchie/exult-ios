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
	ifstream nfile, sched;		// 2 files we read from.
	u7open(nfile, NPC_DAT);
//	u7open(sched, SCHEDULE_DAT);
					// Get header size of "schedule.dat".
//	long hdrsize = Read4(sched) * 2 + 4;
	int cnt1 = Read2(nfile);		// Get counts.
	int cnt2 = Read2(nfile);
cout << "cnt1 = " << cnt1 << ", cnt2 = " << cnt2 << '\n';
	int num_npcs = cnt1 + cnt2;
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
		if (i == 0)		// Main character?
			actor = main_actor =
				new Actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usecode);
		else			// Create NPC.
			actor = new Npc_actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usecode);
		actor->move(scx + cx, scy + cy, 
				get_objects(scx + cx, scy + cy),
				shapex, shapey, (shape[1]>>2) & 0x1f, lift);
#if 0
cout << i << " Creating " << namebuf << ", shape = " << 
	actor->get_shapenum() <<
	", frame = " << actor->get_framenum() << '\n';
cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
	<< lift << '\n';
#endif
		if (iflag1 && iflag2)	// Inventory?  Read (but ignore++++).
			read_ireg_objects(nfile, scx, scy, actor);
		}
	}



