/**
 **	Utils.cc - Common utility routines.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

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

#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <fstream>
#endif
#include <map>
#include <list>
#ifdef MACOS
  #include <stat.h>
#else
  #include <sys/stat.h>
#endif
#include <unistd.h>

#include "exceptions.h"
#include "utils.h"


using std::string;

// Function prototypes

static void switch_slashes(string & name);

// Ugly hack for supporting different paths

static std::map<string, string> path_map;

void add_system_path(const string& key, const string& value)
{
	path_map[key] = value;
}

/*
 *  Convert an exult path (e.g. "<DATA>/exult.flx") into a system path
 */

string get_system_path(const string &path)
{
	string new_path;
	string::size_type pos;
	
	pos = path.find('/');
	// If there is no separator, return the path as is
	if(pos == string::npos)
		new_path = path;
	else
	{
		// See if we can translate this prefix
		string new_prefix(path_map[path.substr(0, pos).c_str()]);

		// If the prefix path is not recognised, return the path as is
		if(new_prefix.empty())
			new_path = path;
		else
			new_path = new_prefix + path.substr(pos);
	}
	switch_slashes(new_path);
	return new_path;
}


/*
 *	Convert a buffer to upper-case.
 *
 *	Output: ->original buffer, changed to upper case.
 */

void to_uppercase
	(
	string &str
	)
{
	for(string::iterator X = str.begin(); X != str.end(); ++X)
	{
#ifndef BEOS
		*X = std::toupper(*X);
#else
//sigh...
        if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#endif         
	}
}

static void switch_slashes(
	string & name
	)
{
#ifdef WIN32
	for(string::iterator X = name.begin(); X != name.end(); ++X)
	{
		if(*X == '/' )
			*X =  '\\';
	}
#elif defined(MACOS)
	// We use a component-wise algorithm (suggested by Yorick)
	// Basically, split the path along the "/" seperators
	// If a component is empty or '.', remove it. If it's '..', replace it
	// with the empty string. convert all / to :
	string::size_type	begIdx, endIdx;;
	string	component;
	string	new_name;
	
	if( name.at(0) != '/' )
		new_name = ":";
	
	begIdx = name.find_first_not_of('/');
	while( begIdx != string::npos )
	{
		endIdx = name.find_first_of('/', begIdx);
		if( endIdx == std::string::npos )
			component = name.substr(begIdx);
		else
			component = name.substr(begIdx, endIdx-begIdx);
		if( component == ".." )
			new_name += ":";
		else if( !component.empty() && component != "." )
		{
			new_name += component;
			if( endIdx != std::string::npos )
				new_name += ":";
		}
		begIdx = name.find_first_not_of('/', endIdx);
	}

	name = new_name;
#else
	// do nothing
#endif
}

/*
 *	Open a file for input, 
 *	trying the original name (lower case), and the upper case version 
 *	of the name.  
 *
 *	Output: 0 if couldn't open.
 */

void U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
#ifdef MACOS
	std::ios_base::openmode mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = ios::in;
#else
	int mode = ios::in;
	if (!is_text) mode |= ios::binary;
#endif
	string name = get_system_path(fname);

	in.open(name.c_str(), mode);		// Try to open original name.
	if (!in.good())			// No good?  Try upper-case.
	{
		to_uppercase(name);
		in.open(name.c_str(), mode);
		if (!in.good())
			throw file_open_exception(name);
	}
}

#ifdef ALPHA_LINUX_CXX
/*
 * Wraps around a bug in Compaq's cxx, which doesn't generate an external
 * symbol for this one implicitly because of is_text = false
 *
 * See function above for a functional description
 */
void U7open(std::ifstream& in,
	    const char *fname)
{
	U7open(in, fname, false);
}
#endif

/*
 *	Open a file for output,
 *	trying the original name (lower case), and the upper case version 
 *	of the name.  
 *
 *	Output: 0 if couldn't open.
 */

void U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
#ifdef MACOS
	std::ios_base::openmode mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = ios::out | ios::trunc;
#else
	int mode = ios::out | ios::trunc;
	if (!is_text) mode |= ios::binary;
#endif
	string name = get_system_path(fname);

	out.open(name.c_str(), mode);		// Try to open original name.
	if (!out.good())		// No good?  Try upper-case.
	{
		to_uppercase(name);
		out.open(name.c_str(), mode);
		if (!out.good())
			throw file_open_exception(name);
	}
}

#ifdef ALPHA_LINUX_CXX
/*
 * Wraps around a bug in Compaq's cxx, which doesn't generate an external
 * symbol for this one implicitly because of is_text = false
 *
 * See function above for a functional description
 */
void U7open(std::ofstream& out,
	    const char *fname)
{
	U7open(out, fname, false);
}
#endif

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
	string name = get_system_path(fname);

	std::FILE *f = std::fopen(name.c_str(), mode);
	if (!f)				// No good?  Try upper-case.
	{
		to_uppercase(name);
		f = std::fopen(name.c_str(), mode);
	}
	if (!f)
		throw file_open_exception(name);
	return (f);
}

/*
 *	Remove a file taking care of paths etc.
 *
 */

void U7remove
	(
	const char *fname			// May be converted to upper-case.
	)
{
	string name = get_system_path(fname);

	std::remove(name.c_str());
}

/*
 *	See if a file exists.
 */

int U7exists
	(
	const char *fname			// May be converted to upper-case.
	)
{
	bool	exists;
	struct stat sbuf;
	
	string name = get_system_path(fname);
	
	exists = (stat(name.c_str(), &sbuf) == 0);
	if( !exists )
	{
		to_uppercase(name);
		exists = (stat(name.c_str(), &sbuf) == 0);
	}
	return exists;
}

/*
 *	See if a file exists.
 */

int U7mkdir
	(
	const char *dirname,	// May be converted to upper-case.
	int mode
	)
{
#ifdef WIN32
	return mkdir(dirname);
#else
	return mkdir(dirname, mode);		// Create dir. if not already there.
#endif
}

/*
 *	See if a file exists.
 */

int U7chdir
	(
	const char *dirname	// May be converted to upper-case.
	)
{
#ifdef MACOS
	string name(dirname);
	switch_slashes(name);
	return chdir(name.c_str());
#else
	return chdir(dirname);
#endif
}

/*
 *	Take log2 of a number.
 *
 *	Output:	Log2 of n (0 if n==0).
 */

int Log2
	(
	uint32 n
	)
{
	int result = 0;
	for (n = n>>1; n; n = n>>1)
		result++;
	return result;
}
