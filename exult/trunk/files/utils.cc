/*
 *	utils.cc - Common utility routines.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#ifdef WIN32
#include <windows.h>
#endif

#include <exception>
#include "exceptions.h"
#include "utils.h"


using std::cerr;
using std::string;
using std::ios;

// Function prototypes

static void switch_slashes(string & name);
static bool base_to_uppercase(string& str, int count);


// Wrap a few functions
inline int stat(const std::string &file_name, struct stat *buf)
{
	return stat(file_name.c_str(),buf);
}

// Ugly hack for supporting different paths

static std::map<string, string> path_map;
static std::map<string, string> stored_path_map;

void store_system_paths()
{
	stored_path_map = path_map;
}

void reset_system_paths()
{
	path_map = stored_path_map;
}

void add_system_path(const string& key, const string& value)
{
	path_map[key] = value;
}

void clear_system_path(const string& key)
{
	std::map<string, string>::iterator iter = path_map.find(key);
	if (iter != path_map.end())
		path_map.erase(iter);
}

/*
 *	Has a path been entered?
 */
bool is_system_path_defined(const string& path)
	{
	return path_map.find(path) != path_map.end();
	}

/*
 *  Convert an exult path (e.g. "<DATA>/exult.flx") into a system path
 */

string get_system_path(const string &path)
{
	string new_path;
	string::size_type pos;
	string::size_type pos2;
	
#if defined(__MORPHOS__) || defined(AMIGA)
  pos = path.find( "../" );
  if( pos != string::npos )
	  new_path = path.substr( 0, pos )+path.substr( pos+2 );
  else
	  new_path = path;

  pos = new_path.find( "./" );
  if( pos != string::npos )
	  new_path = new_path.substr( 0, pos )+new_path.substr( pos+2 );

  pos = new_path.find('>');
  pos2 = new_path.find('<');
  // If there is no separator, return the path as is
  if(pos != string::npos && pos2 == 0)
  {
	  pos += 1;
	  // See if we can translate this prefix
	  string new_prefix(path_map[new_path.substr(0, pos).c_str()]);

	  // If the prefix path is not recognised, return the path as is
	  if(!new_prefix.empty())
		  new_path = new_prefix + new_path.substr(pos);
  }
#else
	pos = path.find('>');
	pos2 = path.find('<');
	// If there is no separator, return the path as is
	if(pos == string::npos || pos2 != 0) {
		new_path = path;
	}
	else
	{
		pos += 1;
		// See if we can translate this prefix
		string new_prefix(path_map[path.substr(0, pos).c_str()]);

		// If the prefix path is not recognised, return the path as is
		if(new_prefix.empty()) {
			new_path = path;
		} else {
			new_path = new_prefix + path.substr(pos);
		}
	}
#endif
	switch_slashes(new_path);
#ifdef WIN32
	int num_chars = GetShortPathName(new_path.c_str(), NULL, 0);
	if (num_chars > 0) {
		char *short_path = new char [num_chars+1];
		GetShortPathName(new_path.c_str(), short_path, num_chars+1);
		new_path = short_path;
		delete [] short_path;
	}
	//else std::cerr << "Warning unable to get short path for: " << new_path << std::endl;
#endif
	return new_path;
}


/*
 *	Convert a buffer to upper-case.
 *
 *	Output: ->original buffer, changed to upper case.
 */

void to_uppercase(string &str)
{
	for(string::iterator X = str.begin(); X != str.end(); ++X)
	{
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = std::toupper(*X);
#endif         
	}
}

string to_uppercase(const std::string &str)
{
	string s(str);
	to_uppercase(s);
	return s;
}

/*
 *	Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

static bool base_to_uppercase(string& str, int count)
{
	if (count <= 0) return true;

	int todo = count;
					// Go backwards.
	string::reverse_iterator X;
	for(X = str.rbegin(); X != str.rend(); ++X)
		{
					// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = std::toupper(*X);
#endif         
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
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
#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = std::ios::in;
#else
	int mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#endif
	string name = get_system_path(fname);
	
	int uppercasecount = 0;
	do {
		// We first "clear" the stream object. This is done to prevent
		// problems when re-using stream objects
		in.clear();
		try {
			in.open(name.c_str(), mode);		// Try to open
		} catch (std::exception &)
		{}
		if (in.good() && !in.fail())
			return; // found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	throw file_open_exception(get_system_path(fname));
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
#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = std::ios::out | std::ios::trunc;
#else
	int mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#endif
	string name = get_system_path(fname);
	
	// We first "clear" the stream object. This is done to prevent
	// problems when re-using stream objects
	out.clear();

	int uppercasecount = 0;
	do {
		out.open(name.c_str(), mode);		// Try to open
		if (out.good())
			return; // found it!
		out.clear();	// Forget ye not
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	throw file_open_exception(get_system_path(fname));
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
	const char *fname,		   // May be converted to upper-case.
	const char *mode		   // File access mode.
	)
{
	std::FILE* f;
	string name = get_system_path(fname);
	
	int uppercasecount = 0;
	do {
		f = std::fopen(name.c_str(), mode); // Try to open
		if (f)
			return f; // found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	throw file_open_exception(get_system_path(fname));
}

/*
 *	Remove a file taking care of paths etc.
 *
 */

void U7remove
	(
	const char *fname		  // May be converted to upper-case.
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
	const char *fname		  // May be converted to upper-case.
	)
{
	bool	exists;
	struct stat sbuf;
	
	string name = get_system_path(fname);
	
	int uppercasecount = 0;
	do {
		exists = (stat(name, &sbuf) == 0);
		if (exists)
			return true; // found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found
	return false;
}

/*
 *	Create a directory
 */

int U7mkdir
	(
	const char *dirname,	// May be converted to upper-case.
	int mode
	)
{
	string name = get_system_path(dirname);
#if (defined(MACOSX) || defined(BEOS))
	// remove any trailing slashes
	string::size_type pos = name.find_last_not_of('/');
	if (pos != string::npos)
	  name.resize(pos+1);
#endif
#ifdef WIN32
	return mkdir(name.c_str());
#else
	return mkdir(name.c_str(), mode); // Create dir. if not already there.
#endif
}

/*
 *	Change the current directory
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
 *	Copy a file.  May throw an exception.
 */

void U7copy
	(
	const char *src,
	const char *dest
	)
	{
	std::ifstream in;
	std::ofstream out;
	try {
		U7open(in, src);
		U7open(out, dest);
	} catch (exult_exception& e) {
		in.close();
		out.close();
		throw e;
	}
	const int bufsize = 0x8000;
	unsigned char *buf = new unsigned char[0x8000];
	in.seekg(0, ios::end);		// Get filesize.
	int filesize = in.tellg();
	in.seekg(0, ios::beg);
	while (filesize > 0)		// Copy.
		{
		int toread = bufsize < filesize ? bufsize : filesize;
		in.read(reinterpret_cast<char *>(buf), toread);
		out.write(reinterpret_cast<char *>(buf), toread);
		filesize -= toread;
		}
	out.flush();
	delete [] buf;
	bool inok = in.good(), outok = out.good();
	in.close();
	out.close();
	if (!inok)
		throw file_read_exception(src);
	if (!outok)
		throw file_write_exception(dest);
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

/*
 *	Replacement for non-standard strdup function.
 */

char *newstrdup(const char *s)
{
	if(!s)
		throw std::invalid_argument("NULL pointer passed to newstrdup");
	char *ret=new char[std::strlen(s)+1];
	std::strcpy(ret,s);
	return ret;
}
