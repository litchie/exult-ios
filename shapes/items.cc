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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// #include <iomanip>			/* Debugging */
#include <fstream>
#include <vector>
#include "items.h"
#include "utils.h"
#include "msgfile.h"

using std::ifstream;
using std::cerr;
using std::endl;
using std::vector;

char **item_names;			// Names of U7 items.
int num_item_names = 0;
char **text_msgs;			// Msgs. (0x400 - in text.flx).
int num_text_msgs = 0;
char **misc_names;			// Frames, etc (0x500 - in text.flx).
int num_misc_names = 0;


/*
 *	Set up names of items.
 *
 *	Msg. names start at 0x400.
 *	Frame names start at entry 0x500 (reagents,medallions,food,etc.).
 */

void Setup_item_names (ifstream& items, ifstream& msgs) {
	vector<char *> msglist;
	int first_msg;			// First in exultmsg.txt.  Should
					//   follow those in text.flx.
	int total_msgs = 0;

	items.seekg(0x54);
	int flxcnt = Read4(items);
	first_msg = num_item_names = flxcnt;
	if (flxcnt > 0x400)
		{
		num_item_names = 0x400;
		num_text_msgs = flxcnt - 0x400;
		if (flxcnt > 0x500)
			{
			num_text_msgs = 0x100;
			num_misc_names = flxcnt - 0x500;
			}
		total_msgs = num_text_msgs;
		}
	if (msgs.good()) {		// Exult msgs. too?
		first_msg = Read_text_msg_file(msgs, msglist);
		if (first_msg >= 0) {
			if (first_msg < num_text_msgs) {
				cerr << "Exult msg. # " << first_msg <<
					" conflicts with 'text.flx'" << endl;
				first_msg = num_text_msgs;
			}
			total_msgs = msglist.size();
		} else
			first_msg = num_text_msgs;
	}
	item_names = new char *[num_item_names];
	text_msgs = new char *[total_msgs - 0x400];
	misc_names = new char *[num_misc_names];
	int i;
	for(i=0; i < flxcnt; i++) {
		items.seekg(0x80+i*8);
		int itemoffs = Read4(items);
		if(!itemoffs)
			continue;
		int itemlen = Read4(items);
		items.seekg(itemoffs);
		char *& loc = i < num_item_names ? item_names[i]
			: (i - num_item_names < num_text_msgs) 
			? text_msgs[i - num_item_names]
			: misc_names[i - num_item_names - num_text_msgs];
		loc = new char[itemlen];
		items.read(loc, itemlen);
#if 0
		cout << dec << i << " 0x" << hex << i << dec
			<< "\t" << item_names[i] << endl;
#endif
	}
	for (i = first_msg; i < total_msgs; i++)
		text_msgs[i - num_item_names] = msglist[i];
	num_text_msgs = total_msgs - num_item_names;
} 
