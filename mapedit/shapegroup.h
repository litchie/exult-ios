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

class Shape_group_file;

/*
 *	A group of shape/chunk #'s:
 */
class Shape_group : vector<int>
	{
	vector<int> ids;		// The shape/chunk #'s.
	char *name;			// What this group is called.
	Shape_group_file *file;		// Where this comes from.
public:
	Shape_group(char *nm, Shape_group_file *f);
	~Shape_group();
	const char *get_name() const
		{ return name; }
	void set_name(char *nm);
	};

/*
 *	This class represents the file containing groups for a given shape
 *	or chunks file.
 */
class Shape_group_file
	{
	std::iofstream file;		// The file this represents.
	vector<Shape_group *> groups;	// List of groups from the file.
public:
	Shape_group_file(char *nm);
	~Shape_group_file();


#endif
