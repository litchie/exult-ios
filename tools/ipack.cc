/**
 **	Ipack.cc - Create/extract image Flex files using the .png format.
 **
 **	Written: 2/19/2002 - JSF
 **/

/*
 *  Copyright (C) 2002  The Exult Team
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
#  include <unistd.h>
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#endif
#include <iostream>
#include <vector>
#include "Flex.h"
#include "utils.h"
#include "exceptions.h"
//#include "vgafile.h"

using std::atoi;
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::size_t;
using std::strdup;
using std::strlen;
using std::strncmp;
using std::vector;

/*
 *	A shape specification:
 */
class Shape_spec
	{
public:
	char *filename;			// Should be allocated.
	int nframes;			// # frames in shape.
public:
	Shape_spec() : filename(0), nframes(0)
		{  }
	~Shape_spec()
		{ delete filename; }
	};
typedef vector<Shape_spec> Shape_specs;

/*
 *	Skip white space.
 */

static char *Skip_space
	(
	char *ptr
	)
	{
	while (isspace(*ptr))
		ptr++;
	return ptr;
	}

/*
 *	Find white space (or null).
 */

static char *Find_space
	(
	char *ptr
	)
	{
	while (*ptr && !isspace(*ptr))
		ptr++;
	return ptr;
	}

/*
 *	Parse a number, and quit with an error msg. if not found.
 */

static long Get_number
	(
	int linenum,			// For printing errors.
	char *errmsg,
	char *ptr,
	char *& endptr			// ->past number and spaces returned.
	)
	{
	long num = strtol(ptr, &endptr, 0);
	if (endptr == ptr)		// No #?
		{
		cerr << "Line " << linenum << ":  " << errmsg << endl;
		exit(1);
		}
	endptr = Skip_space(endptr);
	return num;
	}

/*
 *	Read in script, with the following format:
 *		Max. text length is 1024.
 *		A line beginning with a '#' is a comment.
 *		'archive imgfile' specifies name of the image archive.
 *		'nnn/fff:filename' indicates shape #nnn will consist of fff
 *			frames in files "filename-iii.png", where iii is the
 *			frame #.
 *		Filename may be followed by 'flat' to indicate 8x8 non-RLE
 *			shape.
 */

static void Read_script
	(
	istream& in,
	char *& archname,		// Archive name returned.
	Shape_specs& specs		// Shape specs. returned here.
	)
	{
	archname = 0;
	specs.resize(0);		// Initialize.
	specs.reserve(1200);
	char buf[1024];
	int linenum = 0;
	while (!in.eof())
		{
		++linenum;
		in.get(buf, sizeof(buf));
		char delim;		// Check for end-of-line.
		in.get(delim);
		if (delim != '\n' && !in.eof())
			{
			cerr << "Line #" << linenum << " is too long" << endl;
			exit(1);
			}
		if (!buf[0])
			continue;	// Empty line.
		char *ptr = Skip_space(&buf[0]);
		if (*ptr == '#')
			continue;	// Comment.
		if (strncmp(ptr, "archive", 7) == 0)
			{		// Archive name.
			if (archname)
				{
				cerr << "Line #" << linenum << 
					":  Archname already given" << endl;
				exit(1);
				}
			ptr = Skip_space(ptr + 7);
			char *endptr = Find_space(ptr);
			if (endptr == ptr)
				{
				cerr << "Line #" << linenum <<
					":  No archive name given" << endl;
				exit(1);
				}
			*endptr = 0;
			archname = strdup(ptr);
			continue;
			}
		char *endptr;		// Get shape# in decimal, hex, or oct.
		long shnum = Get_number(linenum, "Shape # missing",
								ptr, endptr);
		if (*endptr != '/')
			{
			cerr << "Line #" << linenum << 
				":  Missing '/' after shape number" << endl;
			exit(1);
			}
		ptr = endptr + 1;
		long nframes = Get_number(linenum, "Frame count missing",
								ptr, endptr);
		if (*endptr != ':')
			{
			cerr << "Line #" << linenum << 
				":  Missing ':' after frame count" << endl;
			exit(1);
			}
		ptr = Skip_space(endptr + 1);
		endptr = Find_space(ptr);
		if (endptr == ptr)
			{
			cerr << "Line #" << linenum <<
				":  Missing filename" << endl;
			exit(1);
			}
		*endptr = 0;
		if (shnum >= specs.size())
			specs.resize(shnum + 1);
		specs[shnum].filename = strdup(ptr);
		specs[shnum].nframes = nframes;
		}
	}

/*
 *	Print usage and exit.
 */

static void Usage()
	{
	cerr << "Usage: ipack -[x|c|u] script" << endl;
	exit(1);
	}

/*
 *	Create or extract from Flex files consisting of shapes.
 */

int main
	(
	int argc,
	char **argv
	)
	{
	if (argc < 3 || argv[1][0] != '-')
		Usage();		// (Exits.)
	char *scriptname = argv[2];
	char *flexname = 0;
	Shape_specs specs;		// Shape specs. stored here.
	ifstream specin;
	try {
		U7open(specin, scriptname, true);
	} catch (exult_exception& e) {
		cerr << e.what() << endl;
		exit(1);
	}
	Read_script(specin, flexname, specs);
	specin.close();
	switch (argv[1][1])		// Which function?
		{
	case 'c':			// Create.
#if 0
		try {
			Write_flex(flexname, "Flex created by Exult", strings);
		} catch (exult_exception& e){
			cerr << e.what() << endl;
			exit(1);
		}
#endif
		break;
	case 'x':			// Extract .png files.
#if 0
		try {
			Read_flex(flexname, strings);
		} catch (exult_exception& e){
			cerr << e.what() << endl;
			exit(1);
		}
		if (argc >= 4)		// Text file given?
			{
			ofstream out;
			try {
				U7open(out, argv[3],  true);
			} catch(exult_exception& e) {
				cerr << e.what() << endl;
				exit(1);
			}
			Write_text(out, strings);
			}
		else
			Write_text(cout, strings);
#endif
		break;
	default:
		Usage();
		}
	return 0;
	}
