/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Shapeinf.cc: Info. about shapes read from various 'static' data files.
 **
 **	Written: 4/29/99 - JSF
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


#include "shapeinf.h"

Ammo_table *Ammo_info::table = 0;

#include <hash_map>
#ifdef MACOS
  using Metrowerks::hash_map;
#endif

/*
 *	For looking up ammo entries:
 */

class Ammo_table
	{
	hash_map<int, Ammo_info> map;
public:
	Ammo_table() : map(53) {  }
	void insert(int shnum, Ammo_info& ent)
		{ map[shnum] = ent; }
	Ammo_info *find(int shnum)	// Look up given shape.
		{
		hash_map<int, Ammo_info>::iterator it = map.find(shnum);
		return it == map.end() ? 0 : &((*it).second);
		}
	};

/*
 *	Create ammo table.
 */

void Ammo_info::create
	(
	)
	{
	table = new Ammo_table;
	}

/*
 *	Insert an entry.
 */

void Ammo_info::insert(int shnum, Ammo_info& ent)
	{ table->insert(shnum, ent); }


/*
 *	Look up ammo's entry.
 */

Ammo_info *Ammo_info::find
	(
	int shnum			// Shape #.
	)
	{
	return Ammo_info::table->find(shnum);
	}


Shape_info::~Shape_info()
	{
	delete weapon;
	if(weapon_offsets)
		delete [] weapon_offsets;
	}
