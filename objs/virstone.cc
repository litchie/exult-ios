/**
 **	Virstone.cc - Virtue stones.
 **
 **	Written: 10/27/2000 - JSF
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "virstone.h"
#include <iostream>
#include "databuf.h"
#include "gamewin.h"
#include "ucsched.h"
#include "Gump_manager.h"

using std::ostream;


/*
 *	Set position from IREG data.
 */

void Virtue_stone_object::set_pos
	(
	unsigned char tilex,		// Tx within superchunk.
	unsigned char tiley,		// Ty within superchunk,
	unsigned char schunk,		// Superchunk
	unsigned char lift
	)
	{
	pos.tx = (schunk%12)*c_tiles_per_schunk + tilex;
	pos.ty = (schunk/12)*c_tiles_per_schunk + tiley;
	pos.tz = lift;
	}

/*
 *	Write out container and its members.
 */

void Virtue_stone_object::write_ireg
	(
	DataSource *out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	unsigned char *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
					// Write tilex, tiley.
	*ptr++ = pos.tx%c_tiles_per_schunk;
	*ptr++ = pos.ty%c_tiles_per_schunk;
					// Get superchunk index.
	int sx = pos.tx/c_tiles_per_schunk,
	    sy = pos.ty/c_tiles_per_schunk;
	*ptr++ = sy*12 + sx;		// Write superchunk #.
	*ptr++ = pos.tz;		// Finally, lift in entry[7].??Guess+++
	*ptr++ = 0;			// Entry[8] unknown.
	*ptr++ = (get_lift()&15)<<4;	// Stone's lift in entry[9].
	*ptr++ = 0;			// Entry[10].  Unknown.
	*ptr++ = 0;			// Entry[11].  Unknown.
	out->write((char*)buf, sizeof(buf));
	}


// Get size of IREG. Returns -1 if can't write to buffer
int Virtue_stone_object::get_ireg_size()
{
	// These shouldn't ever happen, but you never know
	if (gumpman->find_gump(this) || Usecode_script::find(this))
		return -1;

	return 13;
}
