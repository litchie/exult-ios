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
#include "utils.h"

#if 0	/* +++++++++++++Finish later. */
/*
 *	Read in actor from a given file.
 */

Actor::Actor
	(
	istream& nfile,			// 'npc.dat', generally.
	int num,			// NPC #.
	int has_usecode			// 1 if a 'type1' NPC.
	) : Container_game_object(), npc_num(num), party_id(-1),
	    frame_time(0),
	    usecode(uc), flags(0), action(0), usecode_dir(0), two_handed(0)
	{
	init();				// Clear rest of stuff.
	unsigned locx = Read1(nfile);	// Get chunk/tile coords.
	unsigned locy = Read1(nfile);
					// Read & set shape #.
	set_shapenum(Read2(nfile) & 0x3f);
	int iflag1 = Read2(nfile);	// Inventory flag.
	int schunk = Read1(nfile);	// Superchunk #.
	Read1(nfile);			// Skip next byte.
	int usefun = Read2(nfile);	// Get usecode function #.
	set_lift(usefun >> 12);		// Lift is high 4 bits.
	usecode = usefun & 0xfff;
	if (!has_usecode)		// Type2?
		usecode = -1;		// Let's try this.
					// Guessing:  !!
	set_property((int) Actor::food_level, Read1(nfile));
	nfile.seekg(3, ios::cur);	// Skip 3 bytes.
	int iflag2 = Read2(nfile);	// Another inventory flag.
	nfile.seekg(2, ios::cur);	// Skip next 2.
					// Get char. atts.
	int strength_val = Read1(nfile);
	set_property((int) Actor::strength, strength_val);
					// Hits = strength, I think.
	actor->set_property((int) Actor::health, strength_val);
	set_property((int) Actor::dexterity, Read1(nfile));
	set_property((int) Actor::intelligence, Read1(nfile));
	set_property((int) Actor::combat, Read1(nfile));
	int schedtype = Read1(nfile);
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
	name = strdup(namebuf);		// Store copy of it.
	Game_window *gwin = Game_window::get_game_window();
	if (iflag1 && iflag2)		// Inventory?  Read.
		gwin->read_ireg_objects(nfile, scx, scy, this);
					// Get abs. chunk. coords. of schunk.
	int scy = 16*(schunk/12);
	int scx = 16*(schunk%12);
	int cx = locx >> 4;		// Get chunk indices within schunk.
	int cy = locy >> 4;
					// Get tile #'s.
	int tilex = locx & 0xf;
	int tiley = locy & 0xf;
+++++++++++++++++++++++++++++++++++++++++
	Chunk_object_list *olist = get_objects(scx + cx, scy + cy);
		actor->move(0, scx + cx, scy + cy, olist,
				tilex, tiley, (shape[1]>>2) & 0x1f, lift);
					// Put in chunk's NPC list.
		if (npc_actor)
			{
			npc_actor->switched_chunks(0, olist);
			npc_actor->set_schedule_type(schedtype);
			}
		else if (main_actor)
			main_actor->switched_chunks(0, olist);
					// Set attributes.
#if 0
cout << i << " Creating " << namebuf << ", shape = " << 
	actor->get_shapenum() <<
	", frame = " << actor->get_framenum() << ", usecode = " <<
				usefun << '\n';
cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
	<< lift << '\n';
#endif
	}
#endif

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
	nfile.write(buf4, sizeof(buf4));
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
	nfile.put(get_property(Actor::food_level));
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
	strncpy(namebuf, get_name(), 16);
	nfile.write(namebuf, 16);
	write_contents(nfile);		// Write what he holds.
	}
