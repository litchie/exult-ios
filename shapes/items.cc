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
#include "fnames.h"

using std::ifstream;
using std::ofstream;
using std::cerr;
using std::endl;
using std::vector;

char **item_names = 0;			// Names of U7 items.
int num_item_names = 0;
char **text_msgs = 0;			// Msgs. (0x400 - in text.flx).
int num_text_msgs = 0;
char **misc_names = 0;			// Frames, etc (0x500 - in text.flx).
int num_misc_names = 0;


/*
 *	Set up names of items.
 *
 *	Msg. names start at 0x400.
 *	Frame names start at entry 0x500 (reagents,medallions,food,etc.).
 */

static void Setup_item_names (ifstream& items, ifstream& msgs) {
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
			first_msg -= 0x400;
			if (first_msg < num_text_msgs) {
				cerr << "Exult msg. # " << first_msg <<
					" conflicts with 'text.flx'" << endl;
				first_msg = num_text_msgs;
			}
			total_msgs = msglist.size() - 0x400;
		} else
			first_msg = num_text_msgs;
	}
	item_names = new char *[num_item_names];
	memset(item_names, 0, num_item_names*sizeof(item_names[0]));
	text_msgs = new char *[total_msgs];
	memset(text_msgs, 0, total_msgs*sizeof(text_msgs[0]));
	misc_names = new char *[num_misc_names];
	memset(misc_names, 0, num_misc_names*sizeof(misc_names[0]));
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
		text_msgs[i] = msglist[i + 0x400];
	num_text_msgs = total_msgs;
} 

#define SHAPES_SECT 	"shapes"
#define MSGS_SECT	"msgs"
#define MISC_SECT	"miscnames"

/*
 *	This sets up item names and messages from Exult's new file,
 *	"textmsgs.txt".
 */

static void Setup_text
	(
	ifstream& txtfile		// All text.
	)
	{
	Read_text_msg_file(txtfile, item_names, num_item_names, SHAPES_SECT);
	Read_text_msg_file(txtfile, text_msgs, num_text_msgs, MSGS_SECT);
	Read_text_msg_file(txtfile, misc_names, num_misc_names, MISC_SECT);
	}

/*
 *	Setup item names and text messages.
 */

void Setup_text()
	{
	bool is_patch = is_system_path_defined("<PATCH>");

					// Exult new-style messages?
	if (is_patch && U7exists(PATCH_TEXTMSGS))
		{
		ifstream txtfile;
		U7open(txtfile, PATCH_TEXTMSGS);
		Setup_text(txtfile);
		}
	else if (U7exists(TEXTMSGS))
		{
		ifstream txtfile;
		U7open(txtfile, TEXTMSGS);
		Setup_text(txtfile);
		}
	else 
		{
		ifstream textflx, exultmsg;
		if (is_patch && U7exists(PATCH_TEXT))
			U7open(textflx, PATCH_TEXT);
		else
  			U7open(textflx, TEXT_FLX);
		if (is_patch && U7exists(PATCH_EXULTMSG))
			U7open(exultmsg, PATCH_EXULTMSG, true);
		else
			U7open(exultmsg, EXULTMSG, true);
		Setup_item_names(textflx, exultmsg);
		}
	}

/*
 *	Free memory.
 */

static void Free_text_list
	(
	char **& items,
	int& num_items
	)
	{
	for (int i = 0; i < num_items; ++i)
		delete items[i];
	delete [] items;
	items = 0;
	num_items = 0;
	}

void Free_text
	(
	)
	{
	Free_text_list(item_names, num_item_names);
	Free_text_list(text_msgs, num_text_msgs);
	Free_text_list(misc_names, num_misc_names);
	}

/*
 *	Write out new-style Exult text file.
 */

void Write_text_file
	(
	)
	{
	ofstream out;
	int i, cnt;

	U7open(out, PATCH_TEXTMSGS, true);	// (It's a text file.)
	out << "Exult " << VERSION << " text message file." <<
		"  Written by ExultStudio." << endl;
	Write_msg_file_section(out, SHAPES_SECT, item_names, num_item_names);
	Write_msg_file_section(out, MSGS_SECT, text_msgs, num_text_msgs);
	Write_msg_file_section(out, MISC_SECT, misc_names, num_misc_names);
	out.close();
	}

/*
 *	Update/add an item name.
 */

void Set_item_name
	(
	int num,
	const char *name
	)
	{
	if (num >= num_item_names)
		{
		char **newlist = new char*[num + 1];
		int i;
		memcpy(newlist, item_names, num_item_names*sizeof(char *));
		if (num > num_item_names)
			memchr(newlist + num_item_names, 0, 
				(num - num_item_names)*sizeof(char *));
		delete [] item_names;
		item_names = newlist;
		num_item_names = num + 1;
		}
	delete item_names[num];
	item_names[num] = newstrdup(name);
	}

