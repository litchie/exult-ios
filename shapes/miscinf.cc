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
using std::multimap;
using std::pair;

static map<int, int> *explosion_sprite_table = 0;
static map<int, int> *shape_sfx_table = 0;
static multimap<int, Animation_info> *animation_cycle_table = 0;
static map<int, bool> *usecode_event_table = 0;
static map<int, bool> *mountain_top_table = 0;

static map<int, pair<int, int> > *bodies_table = 0;
static map<int, bool> *body_shape_table = 0;

static map<int, Paperdoll_npc> *characters_table = 0;
static multimap<int, Paperdoll_item> *items_table = 0;

void Paperdoll_npc_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch
	)
	{
	Paperdoll_npc npc;
	int npc_shape = strtol(eptr, &eptr, 0);
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
	if (*eptr)	// BG-style gumps
		npc.gump_shape = strtol(eptr + 1, 0, 0);
	else
		npc.gump_shape = -1;
	(*table)[npc_shape] = npc;
	}

void Paperdoll_item_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch
	)
	{
	Paperdoll_item obj;
	int world_shape = strtol(eptr, &eptr, 0);
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
	if (for_patch)
		{
		Paperdoll_item *old = 0;
		typedef multimap<int, Paperdoll_item>::iterator objIterator;
		pair<objIterator, objIterator> itPair =
				items_table->equal_range(world_shape);
		for (objIterator it = itPair.first; it != itPair.second; ++it)
			{
			Paperdoll_item& inf = (*it).second;
			if (obj.world_frame == inf.world_frame
					&& obj.spot == inf.spot)
				{
				old = &inf;
				break;
				}
			}
		if (old)
			// Replace existing.
			*old = obj;
		else
			table->insert(pair<int, Paperdoll_item>(world_shape, obj));
		}
	else
		table->insert(pair<int, Paperdoll_item>(world_shape, obj));
	}

void Animation_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch
	)
	{
	Animation_info inf;
	int shapenum = strtol(eptr, &eptr, 0);
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
	if (for_patch)
		{
		Animation_info *old = Shapeinfo_lookup::get_animation_cycle_info(
				shapenum, inf.first_frame);
		if (old)
			// Replace existing.
			*old = inf;
		else
			table->insert(pair<int, Animation_info>(shapenum, inf));
		}
	else
		table->insert(pair<int, Animation_info>(shapenum, inf));
	}

/*
 *	Parses a shape data file.
 */
void Shapeinfo_lookup::Read_data_file
	(
	const char *fname,					// Name of file to read, sans extension
	const char *sections[],				// The names of the sections
	Shapeinfo_entry_parser *parsers[],	// Parsers to use for each section
	int numsections						// Number of sections
	)
	{
	vector<Readstrings> static_strings;
	vector<Readstrings> patch_strings;
	static_strings.resize(numsections);
	char buf[50];
	if (GAME_BG || GAME_SI)
		{
		snprintf(buf, 50, "config/%s", fname);
		str_int_pair resource = game->get_resource(buf);
		U7object txtobj(resource.str, resource.num);
		std::size_t len;
		char *txt = txtobj.retrieve(len);
		BufferDataSource ds(txt, len);
		for (int i=0; i<numsections; i++)
			Read_text_msg_file(&ds, static_strings[i], sections[i]);
		}
	else
		{
		try
			{
			snprintf(buf, 50, "<STATIC>/%s.txt", fname);
			ifstream in;
			U7open(in, buf, true);
			for (int i=0; i<numsections; i++)
				Read_text_msg_file(in, static_strings[i], sections[i]);
			in.close();
			}
		catch (std::exception &e)
			{
			if (!Game::is_editing())
				throw e;
			}
		}
	patch_strings.resize(numsections);
	snprintf(buf, 50, "<PATCH>/%s.txt", fname);
	if (U7exists(buf))
		{
		ifstream in;
		U7open(in, buf, true);
		for (int i=0; i<numsections; i++)
			Read_text_msg_file(in, patch_strings[i], sections[i]);
		in.close();
		}

	for (int i=0; i<static_strings.size(); i++)
		{
		Readstrings& section = static_strings[i];
		for (int j=0; j<section.size(); j++)
			{
			char *ptr = section[j];
			if (!ptr)
				continue;
			parsers[i]->parse_entry(j, ptr, false);
			delete[] section[j];
			}
		section.clear();
		}
	static_strings.clear();
	for (int i=0; i<patch_strings.size(); i++)
		{
		Readstrings& section = patch_strings[i];
		for (int j=0; j<section.size(); j++)
			{
			char *ptr = section[j];
			if (!ptr)
				continue;
			parsers[i]->parse_entry(j, ptr, true);
			delete[] section[j];
			}
		section.clear();
		}
	patch_strings.clear();
	}

/*
 *	Setup misc info tables.
 */
void Shapeinfo_lookup::setup_miscinf
	(
	)
	{
	explosion_sprite_table = new map<int, int>;
	shape_sfx_table = new map<int, int>;
	animation_cycle_table = new multimap<int, Animation_info>;
	usecode_event_table = new map<int, bool>;
	mountain_top_table = new map<int, bool>;
	const char *sections[5] = {"explosions", "shape_sfx",
			"animation", "usecode_events", "mountain_tops"};
	Shapeinfo_entry_parser *parsers[5] = {
			new Int_pair_parser(explosion_sprite_table),
			new Int_pair_parser(shape_sfx_table),
			new Animation_parser(animation_cycle_table),
			new Bool_parser(usecode_event_table),
			new Bool_parser(mountain_top_table)};
	Read_data_file("shape_info", sections, parsers, 5);
	}

/*
 *	Setup body info table.
 */
void Shapeinfo_lookup::setup_bodies
	(
	)
	{
	body_shape_table = new map<int, bool>;
	bodies_table = new map<int, pair<int, int> >;
	const char *sections[2] = {"bodyshapes", "bodylist"};
	Shapeinfo_entry_parser *parsers[2] = {
			new Bool_parser(body_shape_table),
			new Body_parser(bodies_table)};
	Read_data_file("bodies", sections, parsers, 2);
	}

/*
 *	Setup paperdoll tables.
 */
void Shapeinfo_lookup::setup_paperdolls()
{
	characters_table = new map<int, Paperdoll_npc>;
	items_table = new multimap<int, Paperdoll_item>;
	const char *sections[2] = {"characters", "items"};
	Shapeinfo_entry_parser *parsers[2] = {
			new Paperdoll_npc_parser(characters_table),
			new Paperdoll_item_parser(items_table)};
	Read_data_file("paperdol_info", sections, parsers, 2);
}

/*
 *	Lookup a shape's explosion sprite.
 *
 *	Output:	5 if not found.
 */

int Shapeinfo_lookup::get_explosion_sprite (int shapenum)
	{
	if (!explosion_sprite_table)		// First time?
		setup_miscinf();
	map<int, int>::iterator it = explosion_sprite_table->find(shapenum);
	if (it != explosion_sprite_table->end())
		return (*it).second;
	else
		return 5;	// The default.
	}

/*
 *	Lookup a shape's sfx.
 *
 *	Output:	-1 if not found.
 */

int Shapeinfo_lookup::get_shape_sfx (int shapenum)
	{
	if (!shape_sfx_table)		// First time?
		setup_miscinf();
	map<int, int>::iterator it = shape_sfx_table->find(shapenum);
	if (it != shape_sfx_table->end())
		return Audio::game_sfx((*it).second);
	else
		return -1;	// The default.
	}

/*
 *	Lookup whether to call usecode when readying/unreadying a shape
 *
 *	Output:	false if not found.
 */

bool Shapeinfo_lookup::get_usecode_events (int shapenum)
	{
	if (!usecode_event_table)		// First time?
		setup_miscinf();
	map<int, bool>::iterator it = usecode_event_table->find(shapenum);
	if (it != usecode_event_table->end())
		return (*it).second;
	else
		return false;	// The default.
	}

/*
 *	Lookup if a shape is a mountain top.
 *
 *	Output:	0 if not found.
 */

bool Shapeinfo_lookup::get_mountain_top (int shapenum)
	{
	if (!mountain_top_table)		// First time?
		setup_miscinf();
	map<int, bool>::iterator it = mountain_top_table->find(shapenum);
	if (it != mountain_top_table->end())
		return (*it).second;
	else
		return false;	// The default.
	}

/*
 *	Get information about the animation cycle of a shape given
 *	the initial frame.
 *
 *	Output:	NULL if not found.
 */

Animation_info *Shapeinfo_lookup::get_animation_cycle_info
	(
	int shapenum,
	int init_frame
	)
	{
	if (!animation_cycle_table)		// First time?
		setup_miscinf();
	typedef multimap<int, Animation_info>::iterator aniIterator;
	pair<aniIterator, aniIterator> itPair =
			animation_cycle_table->equal_range(shapenum);
	for (aniIterator it = itPair.first; it != itPair.second; ++it)
		{
		Animation_info& inf = (*it).second;
		if (inf.type != 0 ||
			(inf.first_frame <= init_frame &&
			 inf.first_frame + inf.frame_count > init_frame))
			return &inf;
		}
	return NULL;
	}

/*
 *	Lookup a shape's body.
 *
 *	Output:	0 if not found.
 */

int Shapeinfo_lookup::find_body
	(
	int liveshape,			// Live actor's shape.
	int& deadshape,			// Dead shape returned.
	int& deadframe			// Dead frame returned.
	)
	{
	if (!bodies_table)		// First time?
		setup_bodies();
	map<int, pair<int, int> >::iterator it = bodies_table->find(liveshape);
	if (it != bodies_table->end())
		{
		deadshape = ((*it).second).first;
		deadframe = ((*it).second).second;
		return 1;
		}
	else
		return 0;
	}

/*
 *	Recognize dead body shapes.
 */

bool Shapeinfo_lookup::Is_body_shape
	(
	int shapeid
	)
	{
	if (!body_shape_table)		// First time?
		setup_bodies();
	map<int, bool>::iterator it = body_shape_table->find(shapeid);
	if (it != body_shape_table->end())
		return (*it).second;
	else
		return false;
	}

Paperdoll_npc *Shapeinfo_lookup::GetCharacterInfo(int shape)
{
	if (!characters_table)
		setup_paperdolls();
	map<int, Paperdoll_npc>::iterator it = characters_table->find(shape);
	if (it != characters_table->end())
		return &((*it).second);
	else
		return NULL;
}

Paperdoll_npc *Shapeinfo_lookup::GetCharacterInfoSafe(int shape)
{
	Paperdoll_npc *ch = GetCharacterInfo(shape);
	if (ch) return ch;
	else return &(*characters_table)[0];
}

Paperdoll_item *Shapeinfo_lookup::GetItemInfo(int shape, int frame, int spot)
{
	if (!items_table)
		setup_paperdolls();
	frame &= 0x1f;			// Mask off 'rotated' bit.
	typedef multimap<int, Paperdoll_item>::iterator objIterator;
	pair<objIterator, objIterator> itPair =
			items_table->equal_range(shape);
	for (objIterator it = itPair.first; it != itPair.second; ++it)
		{
		Paperdoll_item& obj = (*it).second;
		if ((frame == -1 || obj.world_frame == -1 || obj.world_frame == frame)
				&& (spot == -1 || obj.spot == spot))
			return &obj;
		}
	return NULL;
}
