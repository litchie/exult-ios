/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Ready.cc - Information from the 'ready.dat' file.
 **
 **	Written: 5/1/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "ready.h"
#include <fstream.h>
#include "utils.h"
#include <hash_map>


/*
 *	Define the table:
 */
typedef hash_map<unsigned short, unsigned char, struct hash<unsigned short>, 
				struct equal_to<unsigned short> > Short_lookup;
class Ready_table : public Short_lookup
	{
public:
	Ready_table() : Short_lookup(250)
		{  }
	};

/*
 *	Read in the info. from the 'ready.dat' file.
 */

Ready_info::Ready_info
	(
	char *fname
	)
	{
	table = new Ready_table();	// Create lookup table.
	ifstream file;
	if (!U7open(file, fname))
		return;
	int cnt = Read1(file);		// Get # entries.
	for (int i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(file);
		unsigned char type = Read1(file);
		(*table)[shapenum] = type;
		file.seekg(6, ios::cur);// Skip 9 bytes.
		}
	}

/*
 *	Delete table.
 */

Ready_info::~Ready_info
	(
	)
	{
	table->clear();
	delete table;
	}

/*
 *	Look up a shape and return its type ('other' if not found).
 */

Ready_info::Ready_type Ready_info::get_type
	(
	int shapenum
	)
	{
	Ready_table::iterator it = (*table).find(shapenum);
	if (it->first == shapenum)
		return (Ready_type) it->second;
	else
		return other;
	}


