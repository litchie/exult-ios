/**
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "shapeinf.h"

Ammo_table *Ammo_info::table = 0;

#ifndef DONT_HAVE_HASH_MAP
#if __GNUG__ > 2
#  include <ext/hash_map>
using std::hash_map;
#else
#  include <hash_map>
#endif
#  ifdef MACOS
	using Metrowerks::hash_map;
#  endif
#else
#  include <map>
#endif


/*
 *	For looking up ammo entries:
 */

class Ammo_table
	{
#ifndef DONT_HAVE_HASH_MAP
	hash_map<int, Ammo_info> my_map;
#else
	std::map<int, Ammo_info> my_map;
#endif
public:

#ifndef DONT_HAVE_HASH_MAP
	Ammo_table() : my_map(53) {  }
#else
	Ammo_table() : my_map() {  }
#endif
	void insert(int shnum, Ammo_info& ent)
		{ my_map[shnum] = ent; }
	Ammo_info *find(int shnum)	// Look up given shape.
		{
#ifndef DONT_HAVE_HASH_MAP
		hash_map<int, Ammo_info>::iterator it = my_map.find(shnum);
#else
		std::map<int, Ammo_info>::iterator it = my_map.find(shnum);
#endif
		return it == my_map.end() ? 0 : &((*it).second);
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
