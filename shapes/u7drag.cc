/**
 **	U7drag.cc - Common defines for drag-and-drop of U7 shapes.
 **
 **	Written: 12/13/00 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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


#include "u7drag.h"

/*
 *	Read/write 2-byte quantities, low-byte first.
 */
inline void Write2
	(
	unsigned char *& out,	// Write here and update.
	unsigned short val
	)
	{
	*out++ = val & 0xff;
	*out++ = (val>>8) & 0xff;
	}
inline unsigned short Read2
	(
	unsigned char *& in
	)
	{
	unsigned short val = *in++;
	return val + ((*in++)<<8);
	}


/*
 *	Store in char array.
 *
 *	Output:	Length of what's stored.
 */

int Store_u7_shapeid
	(
	unsigned char *data, 		// At least 4 bytes.
	int file, 			// 0-255.
	int shape,			// 0-0xffff.
	int frame			// 0-255.
	)
	{
	unsigned char *ptr = data;
	*ptr++ = file;
	Write2(ptr, shape);
	*ptr++ = frame;
	return (ptr - data);
	}

/*
 *	Retrieve shapeid.
 */

void Get_u7_shapeid
	(
	unsigned char *data, 		// At least 4 bytes.
	int& file, 			// 0-255.
	int& shape,			// 0-0xffff.
	int& frame			// 0-255.
	)
	{
	unsigned char *ptr = data;
	file = *ptr++;
	shape = Read2(ptr);
	frame = *ptr;
	}

/*
 *	Store in char array.
 *
 *	Output:	Length of what's stored.
 */

int Store_u7_chunkid
	(
	unsigned char *data, 		// At least 4 bytes.
	int cnum			// Chunk #.
	)
	{
	unsigned char *ptr = data;
	Write2(ptr, cnum);
	return (ptr - data);
	}

/*
 *	Retrieve cnum#.
 */

void Get_u7_chunkid
	(
	unsigned char *data, 		// At least 4 bytes.
	int& cnum			// 0-0xffff returned.
	)
	{
	unsigned char *ptr = data;
	cnum = Read2(ptr);
	}

/*
 *	Store combo.
 *
 *	Output:	Length of data stored.
 */

int Store_u7_comboid
	(
	unsigned char *data, 
	int cnt, 			// # members.
	U7_combo_data *ents		// The members, with locs. relative to
					//   hot-spot.
	)
	{
	unsigned char *ptr = data;
	Write2(ptr, cnt);
	for (int i = 0; i < cnt; i++)
		{
		Write2(ptr, ents[i].tx);
		Write2(ptr, ents[i].ty);
		Write2(ptr, ents[i].tz);
		Write2(ptr, ents[i].shape);
		*ptr++ = (unsigned char) ents[i].frame;
		}
	return (ptr - data);
	}

/*
 *	Retrieve a combo.
 *
 *	Output:	cnt = #elements.
 *		ents = ALLOCATED array of shapes with offsets rel. to hot-spot.
 */

void Get_u7_comboid
	(
	unsigned char *data, 
	int& cnt, 
	U7_combo_data *& ents
	)
	{
	unsigned char *ptr = data;
	cnt = Read2(ptr);
	ents = new U7_combo_data[cnt];
	for (int i = 0; i < cnt; i++)
		{			// Tiles can be negative!
		ents[i].tx = (int) (short) Read2(ptr);
		ents[i].ty = (int) (short) Read2(ptr);
		ents[i].tz = (int) (short) Read2(ptr);
		ents[i].shape = Read2(ptr);
		ents[i].frame = *ptr++;
		}
	}
