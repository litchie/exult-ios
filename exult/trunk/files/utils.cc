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

#include <stdio.h>
#include <fstream>
#include <cctype>

#include "utils.h"


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
#ifndef BEOS
		*buf = std::toupper(*buf);
#else
//sigh...
        if ((*buf >= 'a') && (*buf <= 'z')) *buf -= 32;
#endif         
		buf++;
		}
	return (ret);
	}

static char *Switch_slash(
	char *name,
	const char *old_name
	)
	{
#ifdef WIN32
		std::strcpy(name, old_name);
		for(;*name!=0;name++)
			{
				if(*name=='/')
					*name = '\\';
			}
#elif defined(MACOS)
		name[0] = ':';
		std::strcpy(name+1, old_name);
		for(;*name!=0;name++)
			{
				if(*name=='/')
					*name = ':';
			}
#else
		std::strcpy(name, old_name);
#endif
		return name;
	}
/*
 *	Open a file for input, 
 *	trying the original name (lower case), and the upper case version 
 *	of the name.  
 *
 *	Output: 0 if couldn't open.
 */

int U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname			// May be converted to upper-case.
	)
	{
#ifdef MACOS
	std::ios_base::openmode mode = std::ios::in | std::ios::binary;
#elif defined(XWIN)
	int mode = ios::in;
#else
	int mode = ios::in | ios::binary;
#endif
	char name[512];
	Switch_slash(name, fname);
	in.open(name, mode);		// Try to open original name.
	if (!in.good())			// No good?  Try upper-case.
		{
		To_upper(name);
		in.open(name, mode);
		if (!in.good())
			return (0);
		}
	return (1);
	}

/*
 *	Open a file for output,
 *	trying the original name (lower case), and the upper case version 
 *	of the name.  
 *
 *	Output: 0 if couldn't open.
 */

int U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname			// May be converted to upper-case.
	)
	{
#ifdef MACOS
	std::ios_base::openmode mode = std::ios::out | std::ios::trunc | std::ios::binary;
#elif defined(XWIN)
	int mode = ios::out | ios::trunc;
#else
	int mode = ios::out | ios::trunc | ios::binary;
#endif
	char name[512];
	Switch_slash(name, fname);
	out.open(name, mode);		// Try to open original name.
	if (!out.good())		// No good?  Try upper-case.
		{
		char upper[512];
		out.open(To_upper(std::strcpy(upper, name)), mode);
		if (!out.good())
			return (0);
		}
	return (1);
	}

/*
 *	Open a file with the access rights specified in mode,
 *	works just like fopen but in a system independant fashion.  
 *
 *	Output: A pointer to a FILE
 */

std::FILE* U7open
	(
	const char *fname,			// May be converted to upper-case.
	const char *mode			// File access mode.
	)
	{
	char name[512];
	Switch_slash(name, fname);
	std::FILE *f = std::fopen(name, mode);
	if (!f)				// No good?  Try upper-case.
		{
		char upper[512];
		f = std::fopen(To_upper(std::strcpy(upper, name)), mode);
		if (!f)
			return (0);
		}
	return (f);
	}
	
/*
 *	Take log2 of a number.
 *
 *	Output:	Log2 of n (0 if n==0).
 */

int Log2
	(
	unsigned int n
	)
	{
	int result = 0;
	for (n = n>>1; n; n = n>>1)
		result++;
	return result;
	}
