/**
 **	Items.cc - Names of items.
 **
 **	Written: 11/5/98 - JSF
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

#include <fstream.h>
#include "items.h"
#include "utils.h"

//char *item_names[1024];			// Names of U7 items.
char **item_names;			// Names of U7 items.
int num_item_names;

#if 0
/*
 *	A few shapes have names for each frame.
 */
class Frame_names
	{
	int shapenum;			// The shape.
	char **names;			// ->name for each frame.
public:
	Frame_names(int shnum) : shapenum(shnum), names(0)
		{  }
	void 
#endif

/*
 *	Set up names of items.
 *
 *	Frame names start at entry 0x500 (reagents,medallions,food,etc.).
 */

void Setup_item_names (ifstream& items) {
	items.seekg(0x54);
	num_item_names = Read4(items);
	item_names = new char *[num_item_names];
	for(int i=0; i<num_item_names; i++) {
		items.seekg(0x80+i*8);
		int itemoffs = Read4(items);
		if(!itemoffs)
			continue;
		int itemlen = Read4(items);
		items.seekg(itemoffs);
		item_names[i] = new char[itemlen];
		items.read(item_names[i], itemlen);
//		cout << i << "\t" << item_names[i] << endl;
	}
} 
