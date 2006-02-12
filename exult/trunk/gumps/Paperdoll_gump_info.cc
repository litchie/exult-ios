/*
Copyright (C) 2000-2005 The Exult Team

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

#include "game.h"
#include "gamewin.h"
#include "Paperdoll_gump.h"
#include "actors.h"
#include "msgfile.h"
#include "U7file.h"
#include "databuf.h"
#include <fstream>
#include <vector>
#include <iostream>

using std::vector;
using std::ifstream;

/*
 *
 *	SERPENT ISLE GUMPS
 *
 */

static vector<Paperdoll_gump::Paperdoll_npc> characters_table;
static vector<Paperdoll_gump::Paperdoll_item> items_table;

/*
 *	Setup paperdoll tables.
 */
void Paperdoll_gump::Setup_paperdolls()
{
	int i = -1, j = -1;
	ifstream in;
	vector<char *> char_strings;
	vector<char *> item_strings;
	try {
		ifstream in;
		U7open(in, "<PATCH>/paperdol_info.txt", true);
		i = Read_text_msg_file(in, char_strings, "characters");
		j = Read_text_msg_file(in, item_strings, "items");
		in.close();
	} catch (std::exception &) {
		if (GAME_BG || GAME_SI) {

			str_int_pair resource = game->get_resource("config/paperdol_info");

			U7object txtobj(resource.str, resource.num);
			size_t len;
			char *txt = txtobj.retrieve(len);
			BufferDataSource ds(txt, len);
			i = Read_text_msg_file(&ds, char_strings, "characters");
			j = Read_text_msg_file(&ds, item_strings, "items");
		} else {
			ifstream in;
			U7open(in, "<STATIC>/paperdol_info.txt", true);
			i = Read_text_msg_file(in, char_strings, "characters");
			j = Read_text_msg_file(in, item_strings, "items");
			in.close();
		}
	}

	int cnt = char_strings.size();
	if (i >= 0)
		{
		characters_table.resize(cnt);
		for ( ; i < cnt; ++i)
			{
			char *ptr = char_strings[i], *eptr;
			if (!ptr)
				continue;
			Paperdoll_npc npc;
			npc.npc_shape = strtol(ptr, &eptr, 0);
			npc.is_female = strtol(eptr + 1, &eptr, 0) ? true : false;
			npc.body_shape = strtol(eptr + 1, &eptr, 0);
			npc.body_frame = strtol(eptr + 1, &eptr, 0);
			npc.head_shape = strtol(eptr + 1, &eptr, 0);
			npc.head_frame = strtol(eptr + 1, &eptr, 0);
			npc.head_frame_helm = strtol(eptr + 1, &eptr, 0);
			npc.arms_shape = strtol(eptr + 1, &eptr, 0);
			npc.arms_frame = strtol(eptr + 1, &eptr, 0);
			npc.arms_frame_2h = strtol(eptr + 1, &eptr, 0);
			npc.arms_frame_staff = strtol(eptr + 1, &eptr, 0);
			if (*eptr)	// BG Gumps
				npc.gump_shape = strtol(eptr + 1, 0, 0);
			else
				npc.gump_shape = -1;
			characters_table[i] = npc;
			}
		}
	for (i = 0; i < cnt; ++i)
		delete char_strings[i];

	cnt = item_strings.size();
	if (j >= 0)
		{
		items_table.resize(cnt);
		for ( ; j < cnt; ++j)
			{
			char *ptr = item_strings[j], *eptr;
			if (!ptr)
				continue;
			Paperdoll_item obj;
			obj.world_shape = strtol(ptr, &eptr, 0);
			obj.world_frame = strtol(eptr + 1, &eptr, 0);
			obj.spot = strtol(eptr + 1, &eptr, 0);
			obj.type = (Object_type)strtol(eptr + 1, &eptr, 0);
			obj.gender = strtol(eptr + 1, &eptr, 0) ? true : false;
			obj.shape = strtol(eptr + 1, &eptr, 0);
			obj.frame = strtol(eptr + 1, &eptr, 0);
			// Not all items have all entries, so check first
			if (*eptr)
				obj.frame2 = strtol(eptr + 1, &eptr, 0);
			if (*eptr)
				obj.frame3 = strtol(eptr + 1, &eptr, 0);
			if (*eptr)
				obj.frame4 = strtol(eptr + 1, 0, 0);
			items_table[j] = obj;
			}
		}
	for (j = 0; j < cnt; ++j)
		delete item_strings[j];
}

Paperdoll_gump::Paperdoll_npc *Paperdoll_gump::GetCharacterInfo(int shape)
{
	if (characters_table.empty())
		Setup_paperdolls();

	for (vector<Paperdoll_npc>::iterator it=characters_table.begin();
			it!=characters_table.end(); ++it)
		if ((*it).npc_shape == shape)
			return &*it;

	return NULL;
}

Paperdoll_gump::Paperdoll_npc *Paperdoll_gump::GetCharacterInfoSafe(int shape)
{
	Paperdoll_npc *ch = GetCharacterInfo(shape);

	if (ch) return ch;
	else return &characters_table[0];
}

Paperdoll_gump::Paperdoll_item *Paperdoll_gump::GetItemInfo(int shape, int frame, int spot)
{
	if (items_table.empty())
		Setup_paperdolls();

	int i=0;

	frame &= 0x1f;			// Mask off 'rotated' bit.

	for (vector<Paperdoll_item>::iterator it=items_table.begin();
			it!=items_table.end(); ++it)
		if ((*it).world_shape == shape
			&& (frame == -1 || (*it).world_frame == -1 || (*it).world_frame == frame)
			&& (spot == -1 || (*it).spot == spot))
			return &*it;
	
	return NULL;
}

