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

#ifndef INCL_UCLABEL
#define INCL_UCLABEL

#include <string>
#include <vector>

class Uc_label
{
 private:
	std::string name;			// This will be the key.
	std::vector<int> references; // jumps in the output to this label
	                                      // contains absolute offsets
	int offset;  // location of the label
	bool valid;
 public:
	Uc_label(std::string n) : name(n), offset(0), valid(false)
		{ }
	Uc_label(std::string n, int o) : name(n), offset(o), valid(true)
		{ }

	std::string& get_name() { return name; }
	std::vector<int>& get_references() { return references; }
	int get_offset() const { return offset; }
	bool is_valid() const { return valid; }

	bool set_offset(int o)
	{
		if (valid)
			return false;
		valid = true;
		offset = o;
	}

	void add_reference(int offset)
	{
		references.push_back(offset);
	}
};


#endif
