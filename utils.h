/**
 **	Utils.h - Common utility routines.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef INCL_UTILS
#define INCL_UTILS	1

class ifstream;

/*
 *	Read a 1-byte value.
 */

inline unsigned char Read1
	(
	istream& in
	)
	{
	unsigned char c;
	in.get((char&) c);
	return (c);
	}

/*
 *	Read a 2-byte value, lsb first.
 */

inline unsigned int Read2
	(
	istream& in
	)
	{
	unsigned char b0, b1;
	in.get((char&) b0);
	in.get((char&) b1);
	return (b0 + (b1 << 8));
	}

/*
 *	Read a 2-byte value from a buffer.
 */

inline unsigned int Read2
	(
	unsigned char *& in
	)
	{
	unsigned char b0 = *in++;
	unsigned char b1 = *in++;
	return (b0 + (b1 << 8));
	}

/*
 *	Read a 4-byte long value, lsb first.
 */

inline unsigned long Read4
	(
	istream& in
	)
	{
	unsigned char b0, b1, b2, b3;
	in.get((char&) b0);
	in.get((char&) b1);
	in.get((char&) b2);
	in.get((char&) b3);
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
	}

int U7open
	(
	ifstream& in,			// Input stream to open.
	char *fname			// May be converted to upper-case.
	);

int Log2
	(
	unsigned int n
	);

#endif
