/**
 **	A group of shapes/chunks that can be used as a 'palette'.
 **
 **	Written: 1/22/02 - JSF
 **/

#ifndef INCL_SHAPEGROUP
#define INCL_SHAPEGROUP	1

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

#include <vector>
#include <string>

class Shape_group_file;

/*
 *	A group of shape/chunk #'s:
 */
class Shape_group : std::vector<int>		// Not public on purpose.
	{
	std::string name;		// What this group is called.
	Shape_group_file *file;		// Where this comes from.
public:
	friend class Shape_group_file;
	Shape_group(char *nm, Shape_group_file *f);
	~Shape_group() {  }
	const char *get_name() const
		{ return name.c_str(); }
	void set_name(char *nm)
		{ name = nm; }
	void add(int id);		// Add ID, checking for duplicate 1st.
	};

/*
 *	This class represents the file containing groups for a given shape
 *	or chunks file.  It is read from the 'patch' or 'static' directory,
 *	and saved to the 'patch' directory.
 */
class Shape_group_file
	{
	std::string name;		// Base filename.
	std::vector<Shape_group *> groups;	// List of groups from the file.
	bool modified;			// Changed since last save.
public:
	Shape_group_file(const char *nm);
	~Shape_group_file();
	int size()
		{ return groups.size(); }
	Shape_group *get(int i)
		{ return groups[i]; }
	void add(Shape_group *grp)	// Add a new group.
		{ groups.push_back(grp); modified = true; }
	void remove(int index);		// Remove and delete group.
	void write();			// Write out (to 'patch' directory).
	};

#endif
