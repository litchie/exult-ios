/**
 **	Utils.cc - Common utility routines.
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

#include <fstream.h>
#include <ctype.h>

/*
 *	Convert a buffer to upper-case.
 *
 *	Output: ->original buffer, changed to upper case.
 */

static char *To_upper
	(
	char *buf
	)
	{
	char *ret = buf;
	while (*buf)
		{
		*buf = toupper(*buf);
		buf++;
		}
	return (ret);
	}

/*
 *	Open a file, trying the original name (lower case), and the upper
 *	case version of the name.  
 *
 *	Output: 0 if couldn't open.
 */

int U7open
	(
	ifstream& in,			// Input stream to open.
	char *fname			// May be converted to upper-case.
	)
	{
#ifndef XWIN
	int mode = ios::in | ios::binary;
#else
	int mode = ios::in;
#endif
	in.open(fname, mode);		// Try to open original name.
	if (!in.good())			// No good?  Try upper-case.
		{
		char upper[512];
		in.open(To_upper(strcpy(upper, fname)), mode);
		if (!in.good())
			return (0);
		}
	return (1);
	}

