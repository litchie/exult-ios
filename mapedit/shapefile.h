/**
 **	Information about a shapes file.
 **
 **	Written: 1/23/02 - JSF
 **/

#ifndef INCL_SHAPEFILE
#define INCL_SHAPEFILE	1

/*
Copyright (C) 2002 The Exult Team

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

#include <string>
#include <vector>

class Vga_file;
class Shape_group_file;
class ifstream;
class Object_browser;
class Shape_group;
class Shape;

/*
 *	A shape file:
 */
class Shape_file_info
	{
	std::string basename;		// Base filename.
	std::string pathname;		// Full pathname.
	Vga_file *ifile;		// Contains the images.
	std::ifstream *file;		// For 'chunks'; ifile is NULL.
	Shape_group_file *groups;	// Groups within ifile.
	bool modified;			// Ifile was modified.
public:
	friend class Shape_file_set;
					// We will own ifile and groups.
	Shape_file_info(const char *bnm, const char *pnm, Vga_file *i, 
					std::ifstream *f, Shape_group_file *g)
		: basename(bnm), pathname(pnm), ifile(i), file(f), groups(g),
		  modified(false)
		{  }
	~Shape_file_info();
	const char *get_basename()
		{ return basename.c_str(); }
	const char *get_pathname()
		{ return pathname.c_str(); }
	Vga_file *get_ifile()
		{ return ifile; }
	std::ifstream *get_file()
		{ return file; }
	Shape_group_file *get_groups()
		{ return groups; }
	void set_modified()
		{ modified = true; }
	Object_browser *create_browser(Shape_file_info *vgafile,
				unsigned char *palbuf, Shape_group *g = 0);
	void flush();			// Write if modified.
	static void write_file(const char *pathname, Shape **shapes,
						int nshapes, bool single);
	};

/*
 *	A set of Shape_file's.
 */
class Shape_file_set
	{
	std::vector<Shape_file_info *> files;
public:
	Shape_file_set() {  }
	~Shape_file_set();
					// Create, or return existing one.
	Shape_file_info *create(const char *basename);
	int size()
		{ return files.size(); }
	Shape_file_info *operator[](int i)
		{ return files[i]; }
	void flush();			// Write if modified.
	bool is_modified();		// Any modified?
	};

#endif
