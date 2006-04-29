/*
 *  miscinf.cc - Information about several previously-hardcoded shape data.
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "miscinf.h"
#include "Audio.h"
#include "game.h"
#include "utils.h"
#include "msgfile.h"
#include "U7file.h"
#include "databuf.h"
#include <fstream>
#include <map>
#include <vector>
#include <iostream>

using std::vector;
using std::ifstream;
using std::map;
using std::pair;
using std::size_t;

static map<int, int> *explosion_sprite_table = 0;
static map<int, int> *shape_sfx_table = 0;
static vector<Shapeinfo_lookup::Animation_info> animation_cycle_table;
static map<int, bool> *usecode_event_table = 0;
static map<int, bool> *mountain_top_table = 0;
/*
 *	Setup tables.
 */
void Shapeinfo_lookup::setup
	(
	)
	{
	int i = -1, j = -1, k = -1, l = -1, m = -1;
	ifstream in;
	vector<char *> explosion_sprite_strings;
	vector<char *> shape_sfx_strings;
	vector<char *> animation_cycle_strings;
	vector<char *> usecode_event_strings;
	vector<char *> mountain_top_strings;
	try {
		ifstream in;
		U7open(in, "<PATCH>/shape_info.txt", true);
		i = Read_text_msg_file(in, explosion_sprite_strings, "explosions");
		j = Read_text_msg_file(in, shape_sfx_strings, "shape_sfx");
		k = Read_text_msg_file(in, animation_cycle_strings, "animation");
		l = Read_text_msg_file(in, usecode_event_strings, "usecode_events");
		m = Read_text_msg_file(in, mountain_top_strings, "mountain_tops");
		in.close();
	} catch (std::exception &) {
		if (GAME_BG || GAME_SI) {

			str_int_pair resource = game->get_resource("config/shape_info");

			U7object txtobj(resource.str, resource.num);
			size_t len;
			char *txt = txtobj.retrieve(len);
			BufferDataSource ds(txt, len);
			i = Read_text_msg_file(&ds, explosion_sprite_strings, "explosions");
			j = Read_text_msg_file(&ds, shape_sfx_strings, "shape_sfx");
			k = Read_text_msg_file(&ds, animation_cycle_strings, "animation");
			l = Read_text_msg_file(&ds, usecode_event_strings, "usecode_events");
			m = Read_text_msg_file(&ds, mountain_top_strings, "mountain_tops");
		} else {
			ifstream in;
			U7open(in, "<STATIC>/shape_info.txt", true);
			i = Read_text_msg_file(in, explosion_sprite_strings, "explosions");
			j = Read_text_msg_file(in, shape_sfx_strings, "shape_sfx");
			k = Read_text_msg_file(in, animation_cycle_strings, "animation");
			l = Read_text_msg_file(in, usecode_event_strings, "usecode_events");
			m = Read_text_msg_file(in, mountain_top_strings, "mountain_tops");
			in.close();
		}
	}
	explosion_sprite_table = new std::map<int, int>;
	int cnt = explosion_sprite_strings.size();
	if (i >= 0)
		{
		for ( ; i < cnt; ++i)
			{
			char *ptr = explosion_sprite_strings[i], *eptr;
			if (!ptr)
				continue;
			int shapenum = strtol(ptr, &eptr, 0);
			int spritenum = strtol(eptr + 1, 0, 0);
			(*explosion_sprite_table)[shapenum] = spritenum;
			}
		}
	for (i = 0; i < cnt; ++i)
		delete[] explosion_sprite_strings[i];
	
	shape_sfx_table = new std::map<int, int>;
	cnt = shape_sfx_strings.size();
	if (j >= 0)
		{
		for ( ; j < cnt; ++j)
			{
			char *ptr = shape_sfx_strings[j], *eptr;
			if (!ptr)
				continue;
			int shapenum = strtol(ptr, &eptr, 0);
			int sfxnum = strtol(eptr + 1, 0, 0);
			(*shape_sfx_table)[shapenum] = sfxnum;
			}
		}
	for (j = 0; j < cnt; ++j)
		delete[] shape_sfx_strings[j];

	cnt = animation_cycle_strings.size();
	if (k >= 0)
		{
		animation_cycle_table.resize(cnt);
		for ( ; k < cnt; ++k)
			{
			char *ptr = animation_cycle_strings[k], *eptr;
			if (!ptr)
				continue;
			Animation_info inf;
			inf.shapenum = strtol(ptr, &eptr, 0);
			inf.type = strtol(eptr + 1, &eptr, 0);
			if (inf.type == 0)	// FA_LOOPING
				{
				inf.first_frame = strtol(eptr + 1, &eptr, 0);
				inf.frame_count = strtol(eptr + 1, &eptr, 0);
				if (!*eptr)
					inf.offset_type = -1;
				else
					{
					eptr++;
					if (*eptr = '%')
						inf.offset_type = 1;
					else
						{
						inf.offset_type = 0;	// For safety.
						inf.offset = strtol(eptr, &eptr, 0);
						}
					}
				}
			animation_cycle_table[k] = inf;
			}
		}
	for (k = 0; k < cnt; ++k)
		delete[] animation_cycle_strings[k];

	usecode_event_table = new std::map<int, bool>;
	cnt = usecode_event_strings.size();
	if (l >= 0)
		{
		for ( ; l < cnt; ++l)
			{
			char *ptr = usecode_event_strings[l], *eptr;
			if (!ptr)
				continue;
			int shapenum = strtol(ptr, &eptr, 0);
			(*usecode_event_table)[shapenum] = true;
			}
		}
	for (l = 0; l < cnt; ++l)
		delete[] usecode_event_strings[l];

	mountain_top_table = new std::map<int, bool>;
	cnt = mountain_top_strings.size();
	if (m >= 0)
		{
		for ( ; m < cnt; ++m)
			{
			char *ptr = mountain_top_strings[m], *eptr;
			if (!ptr)
				continue;
			int shapenum = strtol(ptr, &eptr, 0);
			(*mountain_top_table)[shapenum] = true;
			}
		}
	for (m = 0; m < cnt; ++m)
		delete[] mountain_top_strings[m];
	}

/*
 *	Lookup a shape's body.
 *
 *	Output:	0 if not found.
 */

int Shapeinfo_lookup::get_explosion_sprite (int shapenum)
	{
	if (!explosion_sprite_table)		// First time?
		setup();
	std::map<int, int>::iterator it = explosion_sprite_table->find(shapenum);
	if (it != explosion_sprite_table->end())
		return (*it).second;
	else
		return 5;	// The default.
	}

int Shapeinfo_lookup::get_shape_sfx (int shapenum)
	{
	if (!shape_sfx_table)		// First time?
		setup();
	std::map<int, int>::iterator it = shape_sfx_table->find(shapenum);
	if (it != shape_sfx_table->end())
		return Audio::game_sfx((*it).second);
	else
		return -1;	// The default.
	}

bool Shapeinfo_lookup::get_usecode_events (int shapenum)
	{
	if (!usecode_event_table)		// First time?
		setup();
	std::map<int, bool>::iterator it = usecode_event_table->find(shapenum);
	if (it != usecode_event_table->end())
		return (*it).second;
	else
		return false;	// The default.
	}

bool Shapeinfo_lookup::get_mountain_top (int shapenum)
	{
	if (!mountain_top_table)		// First time?
		setup();
	std::map<int, bool>::iterator it = mountain_top_table->find(shapenum);
	if (it != mountain_top_table->end())
		return (*it).second;
	else
		return false;	// The default.
	}


Shapeinfo_lookup::Animation_info *Shapeinfo_lookup::get_animation_cycle_info
	(
	int shapenum,
	int init_frame
	)
	{

	if (animation_cycle_table.empty())		// First time?
		setup();
	for (vector<Animation_info>::iterator it=animation_cycle_table.begin();
			it!=animation_cycle_table.end(); ++it)
		{
		if ((*it).shapenum == shapenum && ((*it).type != 0 ||
			((*it).first_frame <= init_frame &&
			 (*it).first_frame + (*it).frame_count > init_frame)))
			return &*it;
		}
	return NULL;
	}
