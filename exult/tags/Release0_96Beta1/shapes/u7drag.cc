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
	*ptr++ = shape&0xff;		// Low-byte first.
	*ptr++ = (shape>>8)&0xff;
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
	shape = *ptr++;			// Low-byte first.
	shape += (*ptr++)<<8;
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
	*ptr++ = cnum&0xff;		// Low-byte first.
	*ptr++ = (cnum>>8)&0xff;
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
	cnum = *ptr++;			// Low-byte first.
	cnum += (*ptr++)<<8;
	}

