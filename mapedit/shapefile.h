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
class Flex;

/*
 *	A shape file:
 */
class Shape_file_info
	{
protected:
	std::string basename;		// Base filename.
	std::string pathname;		// Full pathname.
	Shape_group_file *groups;	// Groups within ifile.
	bool modified;			// Ifile was modified.
public:
	friend class Shape_file_set;
					// We will own files and groups.
	Shape_file_info(const char *bnm, const char *pnm, Shape_group_file *g)
		: basename(bnm), pathname(pnm), groups(g), modified(false)
		{  }
	virtual ~Shape_file_info();
	const char *get_basename()
		{ return basename.c_str(); }
	const char *get_pathname()
		{ return pathname.c_str(); }
	Shape_group_file *get_groups()
		{ return groups; }
	void set_modified()
		{ modified = true; }
	virtual Vga_file *get_ifile()
		{ return 0; }
	virtual std::ifstream *get_file()
		{ return 0; }
	virtual Flex *get_flex()
		{ return 0; }
	virtual Object_browser *create_browser(Shape_file_info *vgafile,
				unsigned char *palbuf, Shape_group *g = 0)
		{ return 0; }
	virtual void flush()		// Write if modified.
		{ modified = false; }
	};

/*
 *	Image file:
 */
class Image_file_info : public Shape_file_info
	{
	Vga_file *ifile;		// Contains the images.
public:
					// We will own ifile.
	Image_file_info(const char *bnm, const char *pnm, Vga_file *i, 
							Shape_group_file *g)
		: Shape_file_info(bnm, pnm, g), ifile(i)
		{  }
	virtual ~Image_file_info();
	virtual Vga_file *get_ifile()
		{ return ifile; }
	virtual Object_browser *create_browser(Shape_file_info *vgafile,
				unsigned char *palbuf, Shape_group *g = 0);
	virtual void flush();		// Write if modified.
	static void write_file(const char *pathname, Shape **shapes,
						int nshapes, bool single);
	};

/*
 *	Chunks file:
 */
class Chunks_file_info : public Shape_file_info
	{
	std::ifstream *file;		// For 'chunks'; ifile is NULL.
public:
					// We will own file.
	Chunks_file_info(const char *bnm, const char *pnm,
			std::ifstream *f, Shape_group_file *g)
		: Shape_file_info(bnm, pnm, g), file(f)
		{  }
	virtual ~Chunks_file_info();
	virtual std::ifstream *get_file()
		{ return file; }
	virtual Object_browser *create_browser(Shape_file_info *vgafile,
				unsigned char *palbuf, Shape_group *g = 0);
	virtual void flush();		// Write if modified.
	};

/*
 *	Flex file (used for combos, palettes):
 */
class Flex_file_info : public Shape_file_info
	{

	Flex *flex;			// NULL if just 1 entry.
	std::vector<char *> entries;	// Entries are stored here.
	std::vector<int> lengths;	// Lengths here.
	bool write_flat;		// Write flat file if just 1 entry.
public:
					// We will own flex.
	Flex_file_info(const char *bnm, const char *pnm,
				Flex *fl, Shape_group_file *g);
					// Create for single-palette.
	Flex_file_info(const char *bnm, const char *pnm, int size);
	int size()			// Get # flex entries.
		{ return entries.size(); }
	char *get(int i, size_t& len);	// Get i'th entry.
					// Set i'th entry.
	void set(int i, char *newentry, int entlen);
	void swap(int i);		// Swap entries i, i+1.
	void remove(int i);		// Remove i'th entry.
	virtual ~Flex_file_info();
	virtual Flex *get_flex()
		{ return flex; }
	virtual Object_browser *create_browser(Shape_file_info *vgafile,
				unsigned char *palbuf, Shape_group *g = 0);
	virtual void flush();		// Write if modified.
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
