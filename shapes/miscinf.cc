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

#include "exult_constants.h"
#include "miscinf.h"
#include "Audio.h"
#include "game.h"
#include "utils.h"
#include "msgfile.h"
#include "U7file.h"
#include "databuf.h"
#include "shapeid.h"
#include "gamewin.h"
#include "ucmachine.h"
#include "actors.h"

#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <string>

using std::vector;
using std::ifstream;
using std::map;
using std::multimap;
using std::pair;
using std::string;

using namespace std;

static map<int, Explosion_info> *explosion_sprite_table = 0;
static map<int, SFX_info> *shape_sfx_table = 0;
static multimap<int, Animation_info> *animation_cycle_table = 0;
static map<int, bool> *usecode_event_table = 0;
static map<int, bool> *mountain_top_table = 0;

static map<int, pair<int, int> > *bodies_table = 0;
static map<int, bool> *body_shape_table = 0;

static vector<pair<string, int> > *paperdoll_source_table = 0;
static vector<pair<int, int> > *imported_gump_shapes = 0;
static vector<pair<int, int> > *blue_shapes = 0;
static vector<pair<int, int> > *imported_skin_shapes = 0;
static map<string, int> *gumpvars = 0;
static map<string, int> *skinvars = 0;

static map<int, Paperdoll_npc> *characters_table = 0;
static multimap<int, Paperdoll_item> *items_table = 0;

static map<bool, Base_Avatar_info> *def_av_info = 0;
static Avatar_default_skin *base_av_info = 0;
static vector<Skin_data> *skins_table = 0;
static map<int, bool> *unselectable_skins = 0;
static map<int, int> *petra_table = 0;
static map<int, Usecode_function_data> *usecode_funs = 0;
static int last_skin = 0;


/*
 *	Base parser class shape data.
 */
class Shapeinfo_entry_parser
	{
public:
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version) = 0;
	virtual int ReadInt(char *&eptr, int off = 1)
		{
		int ret = strtol(eptr + off, &eptr, 0);
		while (isspace(*eptr))
			eptr++;
		return ret;
		}
	virtual string ReadStr(char *&eptr, int off = 1)
		{
		eptr += off;
		char *pos = strchr(eptr, '/');
		char buf[50];
		strncpy(buf, eptr, pos - eptr);
		buf[pos - eptr] = 0;
		eptr = pos;
		return string(buf);
		}
	};

class Int_pair_parser: public Shapeinfo_entry_parser
	{
	map<int, int> *table;
public:
	Int_pair_parser(map<int, int> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		int key = ReadInt(eptr, 0);
		int data = ReadInt(eptr);
		(*table)[key] = data;
		}
	};

class Bool_parser: public Shapeinfo_entry_parser
	{
	map<int, bool> *table;
public:
	Bool_parser(map<int, bool> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		int key = ReadInt(eptr, 0);
		(*table)[key] = true;
		}
	};

class Explosion_info_parser: public Shapeinfo_entry_parser
	{
	map<int, Explosion_info> *table;
public:
	Explosion_info_parser(map<int, Explosion_info> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		Explosion_info data = {5, -1};
		int shape = ReadInt(eptr, 0);
		data.sprite = ReadInt(eptr);
		if (version > 3 && *eptr)
			data.sfxnum = ReadInt(eptr);
		(*table)[shape] = data;
		}
	};

class Body_parser: public Shapeinfo_entry_parser
	{
	map<int, pair<int, int> > *table;
public:
	Body_parser(map<int, pair<int, int> > *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		int bshape = ReadInt(eptr, 0);
		int bframe = ReadInt(eptr);
		((*table)[index]).first = bshape;
		((*table)[index]).second = bframe;
		}
	};

class Animation_parser: public Shapeinfo_entry_parser
	{
	multimap<int, Animation_info> *table;
public:
	Animation_parser(multimap<int, Animation_info> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version);
	};

class SFX_parser: public Shapeinfo_entry_parser
	{
	map<int, SFX_info> *table;
public:
	SFX_parser(map<int, SFX_info> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		SFX_info data = {-1, false, 1, 100, -1};
		int key = ReadInt(eptr, 0);
		data.num = ReadInt(eptr);
		if (version >= 2)
			{
			if (*eptr)
				{
				data.chance = ReadInt(eptr);
				if (data.chance < 1 || data.chance > 100)
					data.chance = 100;
				if (*eptr)
					{
					data.range = ReadInt(eptr);
					if (*eptr)
						{
						data.rand = ReadInt(eptr) != 0;
						if (*eptr)
							data.extra = ReadInt(eptr);
						}
					}
				}
			}
		(*table)[key] = data;
		}
	};

class Shape_imports_parser: public Shapeinfo_entry_parser
	{
	vector<pair<int, int> > *table;
	map<string, int> *shapevars;
	int shape;
public:
	Shape_imports_parser
		(vector<pair<int, int> > *tbl, map<string, int> *sh)
		: table(tbl), shapevars(sh), shape(c_max_shapes)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		pair<int, int> data;
		data.second = ReadInt(eptr, 0);	// The real shape.
		for (vector<pair<int, int> >::iterator it = table->begin();
				it != table->end(); ++it)
			if ((*it).second == data.second)
				return;		// Do nothing for repeated entries.
		eptr++;
		if (*eptr == '%')
			{
			data.first = shape;		// The assigned shape.
			string key(strdup(eptr));
			(*shapevars)[key] = shape;
			shape++;	// Leave it ready for the next shape.
			}
		else
			data.first = ReadInt(eptr, 0);
		table->push_back(data);
		}
	};

class Shaperef_parser: public Shapeinfo_entry_parser
	{
	vector<pair<int, int> > *table;
	map<string, int> *shapevars;
public:
	Shaperef_parser
		(vector<pair<int, int> > *tbl, map<string, int> *sh)
		: table(tbl), shapevars(sh)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		pair<int, int> data;
		data.first = ReadInt(eptr, 0);	// The spot.
		eptr++;
		if (*eptr == '%')
			{
			string key(strdup(eptr));
			map<string, int>::iterator it = shapevars->find(key);
			if (it != shapevars->end())
				data.second = (*it).second;	// The shape #.
			else
				return;	// Invalid reference; bail out.
			}
		else
			data.second = ReadInt(eptr, 0);
		table->push_back(data);
		}
	};

class Paperdoll_source_parser: public Shapeinfo_entry_parser
	{
	vector<pair<string, int> > *table;
	bool erased_for_patch;
public:
	Paperdoll_source_parser(vector<pair<string, int> > *tbl)
		: table(tbl), erased_for_patch(false)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version);
	};

class Paperdoll_npc_parser: public Shapeinfo_entry_parser
	{
	map<int, Paperdoll_npc> *table;
	map<string, int> *shapevars;
public:
	Paperdoll_npc_parser(map<int, Paperdoll_npc> *tbl, map<string, int> *sh)
		: table(tbl), shapevars(sh)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version);
	};

class Paperdoll_item_parser: public Shapeinfo_entry_parser
	{
	multimap<int, Paperdoll_item> *table;
public:
	Paperdoll_item_parser(multimap<int, Paperdoll_item> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version);
	};

class Def_av_shape_parser: public Shapeinfo_entry_parser
	{
	map<bool, Base_Avatar_info> *table;
public:
	Def_av_shape_parser(map<bool, Base_Avatar_info> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		bool fmale = ReadInt(eptr, 0) != 0;
		Base_Avatar_info entry;
		entry.shape_num = ReadInt(eptr);
		entry.face_shape = ReadInt(eptr);
		entry.face_frame = ReadInt(eptr);
		(*table)[fmale] = entry;
		}
	};

class Base_av_race_parser: public Shapeinfo_entry_parser
	{
	Avatar_default_skin *table;
public:
	Base_av_race_parser(Avatar_default_skin *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		table->default_skin = ReadInt(eptr, 0);
		table->default_female = ReadInt(eptr);
		}
	};

class Multiracial_parser: public Shapeinfo_entry_parser
	{
	vector<Skin_data> *table;
	map<string, int> *shapevars;
public:
	Multiracial_parser(vector<Skin_data> *tbl, map<string, int> *sh)
		: table(tbl), shapevars(sh)
		{  }
	int ReadVar(char *&eptr)
		{
		eptr++;
		if (*eptr == '%')
			{
			string key = ReadStr(eptr, 0);
			map<string, int>::iterator it = shapevars->find(key);
			if (it != shapevars->end())
				return (*it).second;	// The var value.
			else
				return -1;	// Invalid reference; bail out.
			}
		else
			return ReadInt(eptr, 0);
		}
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		Skin_data entry;
		entry.skin_id = ReadInt(eptr, 0);
		if (entry.skin_id > last_skin)
			last_skin = entry.skin_id;
		entry.is_female = ReadInt(eptr) != 0;
		if ((entry.shape_num = ReadVar(eptr)) < 0)
			return;
		if ((entry.naked_shape = ReadVar(eptr)) < 0)
			return;
		entry.face_shape = ReadInt(eptr);
		entry.face_frame = ReadInt(eptr);
		entry.alter_face_shape = ReadInt(eptr);
		entry.alter_face_frame = ReadInt(eptr);
		entry.copy_info = !(version == 2 && *eptr && !(bool)ReadInt(eptr));
		if (for_patch && !table->empty())
			{
			int i;
			int found = -1;
			for (i = 0; i < table->size(); i++)
				if ((*table)[i].skin_id == entry.skin_id &&
					(*table)[i].is_female == entry.is_female)
					{
					found = i;
					break;
					}
			if (found > -1)
				{
				(*table)[i] = entry;
				return;
				}
			}
		table->push_back(entry);
		}
	};

class Avatar_usecode_parser: public Shapeinfo_entry_parser
	{
	map<int, Usecode_function_data> *table;
	Usecode_machine *usecode;
public:
	Avatar_usecode_parser(map<int, Usecode_function_data> *tbl)
		: table(tbl), usecode(Game_window::get_instance()->get_usecode())
		{  }
	virtual void parse_entry(int index, char *eptr,
			bool for_patch, int version)
		{
		Usecode_function_data entry;
		int type = ReadInt(eptr);
		if (*eptr == ':')
			{
			string name = ReadStr(eptr);
			entry.fun_id = usecode->find_function(name.c_str(), true);
			}
		else
			entry.fun_id = ReadInt(eptr);
		entry.event_id = ReadInt(eptr);
		(*table)[type] = entry;
		}
	};





void Paperdoll_source_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch,
	int version
	)
	{
	if (!erased_for_patch && for_patch)
		table->clear();
	string line(eptr);
	if (line == "static" ||
			(GAME_BG && line == "bg") ||
			(GAME_SI && line == "si"))
		table->push_back(pair<string, int>(string(PAPERDOL), -1));
	else if (line == "si")
		table->push_back(pair<string, int>(string("<SERPENTISLE_STATIC>/paperdol.vga"), -1));
	else if (GAME_SI && line == "flx")
		// ++++ FIMXME: Implement in the future for SI paperdoll patches.
		CERR("Paperdoll source file '" << line << "' is not implemented yet.");
	else if (GAME_BG && line == "flx")
		{
		str_int_pair resource = game->get_resource("files/paperdolvga");
		table->push_back(pair<string, int>(string(resource.str), resource.num));
		}
	else
		CERR("Unknown paperdoll source file '" << line << "' was specified.");
	}

void Paperdoll_npc_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch,
	int version
	)
	{
	Paperdoll_npc npc;
	int npc_shape;
	if (*eptr == '%')
		{
		string key = ReadStr(eptr, 0);
		map<string, int>::iterator it = shapevars->find(key);
		if (it != shapevars->end())
			npc_shape = (*it).second;	// The shape #.
		else
			return;	// Invalid reference; bail out.
		}
	else
		npc_shape = ReadInt(eptr, 0);
	
	npc.is_female = ReadInt(eptr) ? true : false;
	npc.translucent = ReadInt(eptr) ? true : false;
	npc.body_shape = ReadInt(eptr);
	npc.body_frame = ReadInt(eptr);
	npc.head_shape = ReadInt(eptr);
	npc.head_frame = ReadInt(eptr);
	npc.head_frame_helm = ReadInt(eptr);
	npc.arms_shape = ReadInt(eptr);
	npc.arms_frame = ReadInt(eptr);
	npc.arms_frame_2h = ReadInt(eptr);
	npc.arms_frame_staff = ReadInt(eptr);
	if (*eptr)	// BG-style gumps
		npc.gump_shape = ReadInt(eptr);
	else
		npc.gump_shape = -1;
	(*table)[npc_shape] = npc;
	}

void Paperdoll_item_parser::parse_entry
	(
	int index,
	char *eptr,
	bool for_patch,
	int version
	)
	{
	Paperdoll_item obj;
	int world_shape = ReadInt(eptr, 0);
	obj.world_frame = ReadInt(eptr);
	obj.translucent = ReadInt(eptr) ? true : false;
	obj.spot = ReadInt(eptr);
	obj.type = (Object_type)ReadInt(eptr);
	obj.gender = ReadInt(eptr) ? true : false;
	obj.shape = ReadInt(eptr);
	obj.frame = ReadInt(eptr);
	// Not all items have all entries, so check first.
	// Also, ensure sensible defaults.
	if (*eptr)
		obj.frame2 = ReadInt(eptr);
	else
		obj.frame2 = -1;
	if (*eptr)
		obj.frame3 = ReadInt(eptr);
	else
		obj.frame3 = -1;
	if (*eptr)
		obj.frame4 = ReadInt(eptr);
	else
		obj.frame4 = -1;
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
	bool for_patch,
	int version
	)
	{
	Animation_info inf = {FA_LOOPING, 0, 0, 0, -1, 1, 1};
	int shapenum = ReadInt(eptr, 0);
	inf.type = (AniType)ReadInt(eptr);
	if (inf.type != FA_HOURLY && inf.type != FA_NON_LOOPING)
		{
		inf.first_frame = ReadInt(eptr);
		inf.frame_count = ReadInt(eptr);
		if (inf.type == FA_LOOP_RECYCLE)
			{
			inf.offset = ReadInt(eptr);
			if (inf.offset < 0)
				inf.offset = 0;
			if (*eptr)
				inf.sfx_info = ReadInt(eptr);
			if (inf.sfx_info < 1)
				inf.sfx_info = 1;
			}
		else if (inf.type == FA_LOOPING && *eptr)
			{
			eptr++;
			if (*eptr == '%')
				{
				inf.offset_type = 1;
				eptr++;
				}
			else
				{
				int off = ReadInt(eptr, 0);
				inf.offset_type = off < 0 ? -1 : 0;
				inf.offset = off < 0 ? 0 : off;
				}
			if (version >= 2 && *eptr)
				{
				inf.frame_delay = ReadInt(eptr) + 1;
				if (inf.frame_delay < 1)
					inf.frame_delay = 1;
				if (*eptr)
					inf.sfx_info = ReadInt(eptr) + 1;
				if (inf.sfx_info < 0)
					inf.sfx_info = 1;
				}
			}
		}
	else if (inf.type == FA_NON_LOOPING && version >= 3)
		{
		inf.first_frame = ReadInt(eptr);
		inf.frame_count = ReadInt(eptr);
		}
	if (inf.frame_count == 0)
		{	// Sane default.
		ShapeID shp(shapenum, 0);
		inf.frame_count = shp.get_num_frames();
		}

	Animation_info *old = Shapeinfo_lookup::get_animation_cycle_info(
			shapenum, inf.first_frame);
	if (old)
		// Replace existing.
		*old = inf;
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
	int static_version = 1;
	int patch_version = 1;
	const char *version = "version";
	char buf[50];
	if (GAME_BG || GAME_SI)
		{
		snprintf(buf, 50, "config/%s", fname);
		str_int_pair resource = game->get_resource(buf);
		U7object txtobj(resource.str, resource.num);
		std::size_t len;
		char *txt = txtobj.retrieve(len);
		BufferDataSource ds(txt, len);
		// Read version.
		Readstrings versioninfo;
		Read_text_msg_file(&ds, versioninfo, version);
		static_version = strtol(versioninfo[0], 0, 0);
		for (int j=0; j<versioninfo.size(); j++)
			delete[] versioninfo[j];
		for (int i=0; i<numsections; i++)
			Read_text_msg_file(&ds, static_strings[i], sections[i]);
		}
	else
		{
		try
			{
			snprintf(buf, 50, "<STATIC>/%s.txt", fname);
			ifstream in;
			U7open(in, buf, false);
			in.seekg(0, std::ios::end);
			int size = in.tellg();	// Get file size.
			in.seekg(0);
			Readstrings versioninfo;
			// Read version.
			if (Read_text_msg_file(in, versioninfo, version) != -1)
				{
				static_version = strtol(versioninfo[0], 0, 0);
				for (int j=0; j<versioninfo.size(); j++)
					delete[] versioninfo[j];
				}
			else
				in.seekg(0);
			for (int i=0; i<numsections; i++)
				{
				std::size_t loc = in.tellg();
				if (loc < size &&
					Read_text_msg_file(in, static_strings[i], sections[i]) == -1)
					in.seekg(loc);
				}
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
		U7open(in, buf, false);
		in.seekg(0, std::ios::end);
		int size = in.tellg();	// Get file size.
		in.seekg(0);
		Readstrings versioninfo;
		// Read version.
		if (Read_text_msg_file(in, versioninfo, version) != -1)
			{
			patch_version = strtol(versioninfo[0], 0, 0);
			for (int j=0; j<versioninfo.size(); j++)
				delete[] versioninfo[j];
			}
		else
			in.seekg(0);
		for (int i=0; i<numsections; i++)
			{
			std::size_t loc = in.tellg();
			if (loc < size &&
				Read_text_msg_file(in, patch_strings[i], sections[i]) == -1)
				in.seekg(loc);
			}
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
			parsers[i]->parse_entry(j, ptr, false, static_version);
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
			parsers[i]->parse_entry(j, ptr, true, patch_version);
			delete[] section[j];
			}
		section.clear();
		}
	patch_strings.clear();
	}

static void create_original_cycles
	(
	int type,
	int shapenum,
	multimap<int, Animation_info> *table
	)
	{
	ShapeID shp(shapenum, 0);
	Animation_info inf = {FA_LOOPING, 0, shp.get_num_frames(), 0, -1, 1, 1};

	switch (type)
		{
		case 0:
		case 1:
			break;
		case 5:
			inf.type = FA_RANDOM_LOOP;
			break;
		case 6:
			inf.type = FA_RANDOM_FRAMES;
			break;
		case 8:
			inf.type = FA_HOURLY;
			break;
		case 9:
			inf.type = FA_LOOP_RECYCLE;
			inf.offset = inf.frame_count - 8;
			break;
		case 10:
			inf.type = FA_LOOP_RECYCLE;
			inf.offset = inf.frame_count - 6;
			break;
		case 11:
			{
			// First frame doesn't animate.
			inf.type = FA_LOOPING;
			int nframes = inf.frame_count-1;
			inf.frame_count = 1;
			table->insert(pair<int, Animation_info>(shapenum, inf));
			inf.first_frame = 1;
			inf.frame_count = nframes;
			break;
			}
		case 13:
			inf.type = FA_NON_LOOPING;
			break;
		case 14:
			inf.frame_delay = 4;
			inf.sfx_info = 0;
			break;
		case 15:
			{
			int nframes = inf.frame_count;
			inf.frame_count = 6;
			inf.offset_type = 1;
			for (int i = 0; i < nframes/6; i++)
				{
				table->insert(pair<int, Animation_info>(shapenum, inf));
				inf.first_frame += 6;
				}
			// There may be one last cycle.
			inf.frame_count = nframes%6;
			if (!inf.frame_count)
				return;
			break;
			}
		default:
			// Not handled yet. These would be cases 2, 3, 4, 7 and 12:
			// case 2 seems to be equal to case 0/1 except that
			//		frames advance randomly (maybe 1 in 4 chance)
			// case 3 is very strange. I have noted no pattern yet.
			// case 4 seems to be for case 3 what case 2 is for case 0/1.
			// case 7 toggles bit 0 of the frame (i.e., new_frame == frame ^ 1)
			// case 12 seems to advance frames at 1/4 rate.
			// None of these are used for any animated shape, which is why
			// I haven't bothered implementing them.
			return;
		}
	table->insert(pair<int, Animation_info>(shapenum, inf));
	}

/*
 *	Setup misc info tables.
 */
void Shapeinfo_lookup::setup_miscinf
	(
	)
	{
	explosion_sprite_table = new map<int, Explosion_info>;
	shape_sfx_table = new map<int, SFX_info>;
	animation_cycle_table = new multimap<int, Animation_info>;
	usecode_event_table = new map<int, bool>;
	mountain_top_table = new map<int, bool>;

	// Read original animation info first if there.
	if (GAME_BG || GAME_SI)
		{
		ifstream tfa;
			// We *should* blow up if TFA not there.
		U7open(tfa, TFA);
		tfa.seekg(3*1024);
		unsigned char buf[512];
		tfa.read((char *)buf, 512);
		tfa.close();
		unsigned char *ptr = buf;
		for (int i = 0; i < 512; i++, ptr++)
			{
			if ((*ptr)&0xf)
				create_original_cycles((*ptr)&0xf, 2*i, animation_cycle_table);
			if ((*ptr)&0xf0)
				create_original_cycles((*ptr) >> 4, 2*i+1, animation_cycle_table);
			}
		}

	const char *sections[5] = {"explosions", "shape_sfx",
			"animation", "usecode_events", "mountain_tops"};
	Shapeinfo_entry_parser *parsers[5] = {
			new Explosion_info_parser(explosion_sprite_table),
			new SFX_parser(shape_sfx_table),
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
	if (!paperdoll_source_table || !imported_gump_shapes ||
		!blue_shapes || !imported_skin_shapes)
		setup_shape_files();
	characters_table = new map<int, Paperdoll_npc>;
	items_table = new multimap<int, Paperdoll_item>;
	const int size = 2;
	const char *sections[size] = {
		"characters",
		"items"
		};
	Shapeinfo_entry_parser *parsers[size] = {
		new Paperdoll_npc_parser(characters_table, skinvars),
		new Paperdoll_item_parser(items_table)
		};
	Read_data_file("paperdol_info", sections, parsers, size);
	// For safety.
	if (paperdoll_source_table->size() == 0)
		paperdoll_source_table->push_back(pair<string, int>(string(PAPERDOL), -1));
	// Add in patch paperdolls too.
	paperdoll_source_table->push_back(pair<string, int>(string(PATCH_PAPERDOL), -1));
}

/*
 *	Setup shape file tables.
 */
void Shapeinfo_lookup::setup_shape_files()
{
	paperdoll_source_table = new vector<pair<string, int> >;
	imported_gump_shapes = new vector<pair<int, int> >;
	gumpvars = new map<string, int>;
	blue_shapes = new vector<pair<int, int> >;
	imported_skin_shapes = new vector<pair<int, int> >;
	skinvars = new map<string, int>;
	const int size = 4;
	const char *sections[size] = {
		"paperdoll_source",
		"gump_imports",
		"blue_shapes",
		"multiracial_imports"
		};
	Shapeinfo_entry_parser *parsers[size] = {
		new Paperdoll_source_parser(paperdoll_source_table),
		new Shape_imports_parser(imported_gump_shapes, gumpvars),
		new Shaperef_parser(blue_shapes, gumpvars),
		new Shape_imports_parser(imported_skin_shapes, skinvars)
		};
	Read_data_file("shape_files", sections, parsers, size);
	// For safety.
	if (paperdoll_source_table->size() == 0)
		paperdoll_source_table->push_back(pair<string, int>(string(PAPERDOL), -1));
	// Add in patch paperdolls too.
	paperdoll_source_table->push_back(pair<string, int>(string(PATCH_PAPERDOL), -1));
}

/*
 *	Setup avatar data tables.
 */
void Shapeinfo_lookup::setup_avatar_data()
{
	if (!skinvars)
		setup_shape_files();
	def_av_info = new map<bool, Base_Avatar_info>;
	base_av_info = new Avatar_default_skin;
	skins_table = new vector<Skin_data>;
	unselectable_skins = new map<int, bool>;
	petra_table = new map<int, int>;
	usecode_funs = new map<int, Usecode_function_data>;
	const int size = 6;
	const char *sections[size] = {
		"defaultshape",
		"baseracesex",
		"multiracial_table",
		"unselectable_races_table",
		"petra_face_table",
		"usecode_info"
		};
	Shapeinfo_entry_parser *parsers[size] = {
		new Def_av_shape_parser(def_av_info),
		new Base_av_race_parser(base_av_info),
		new Multiracial_parser(skins_table, skinvars),
		new Bool_parser(unselectable_skins),
		new Int_pair_parser(petra_table),
		new Avatar_usecode_parser(usecode_funs)
		};
	Read_data_file("avatar_data", sections, parsers, size);
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
	map<int, Explosion_info>::iterator it = explosion_sprite_table->find(shapenum);
	if (it != explosion_sprite_table->end())
		return ((*it).second).sprite;
	else
		return 5;	// The default.
	}

/*
 *	Lookup a shape's explosion sfx.
 *
 *	Output:	5 if not found.
 */

int Shapeinfo_lookup::get_explosion_sfx (int shapenum)
	{
	if (!explosion_sprite_table)		// First time?
		setup_miscinf();
	map<int, Explosion_info>::iterator it = explosion_sprite_table->find(shapenum);
	if (it != explosion_sprite_table->end())
		return ((*it).second).sfxnum;
	else
		return -1;	// The default.
	}

/*
 *	Lookup a shape's sfx.
 *
 *	Output:	-1 if not found.
 */

SFX_info *Shapeinfo_lookup::get_sfx_info (int shapenum)
	{
	if (!shape_sfx_table)		// First time?
		setup_miscinf();
	map<int, SFX_info>::iterator it = shape_sfx_table->find(shapenum);
	if (it != shape_sfx_table->end())
		return &((*it).second);
	else
		return NULL;	// The default.
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
		if (inf.first_frame <= init_frame &&
				inf.first_frame + inf.frame_count > init_frame)
			return &inf;
		}
	ShapeID shp(shapenum, 0);
	Animation_info inf = {FA_LOOPING, 0, shp.get_num_frames(), 0, -1, 1, 1};
	animation_cycle_table->insert(pair<int, Animation_info>(shapenum, inf));
	// Return the one in the table.
	return get_animation_cycle_info(shapenum, init_frame);
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

vector<pair<string, int> > *Shapeinfo_lookup::GetPaperdollSources()
{
	if (!paperdoll_source_table)
		setup_shape_files();
	return paperdoll_source_table;
}

vector<pair<int, int> > *Shapeinfo_lookup::GetImportedSkins()
{
	if (!imported_skin_shapes)
		setup_shape_files();
	return imported_skin_shapes;
}

bool Shapeinfo_lookup::IsSkinImported(int shape)
{
	if (!imported_skin_shapes)
		setup_shape_files();
	assert(imported_skin_shapes);
	for (vector<pair<int, int> >::iterator it = imported_skin_shapes->begin();
			it != imported_skin_shapes->end(); ++it)
		{
		if ((*it).first == shape)
			return true;
		}
	return false;
}

vector<pair<int, int> > *Shapeinfo_lookup::GetImportedGumpShapes()
{
	if (!imported_gump_shapes)
		setup_shape_files();
	return imported_gump_shapes;
}

int Shapeinfo_lookup::GetBlueShapeData(int spot)
{
	if (!blue_shapes)
		setup_shape_files();
	
	int ret;
	for (vector<pair<int,int> >::iterator it = blue_shapes->begin();
			it != blue_shapes->end(); ++it)
		{
		if ((*it).first == -1)
			ret = (*it).second;
		else if ((*it).first == spot)
			{
			ret = (*it).second;
			break;
			}
		}
	return ret;
}

Base_Avatar_info *Shapeinfo_lookup::GetBaseAvInfo(bool sex)
{
	if (!def_av_info)
		setup_avatar_data();
	map<bool, Base_Avatar_info>::iterator it = def_av_info->find(sex);
	if (it != def_av_info->end())
		return &((*it).second);
	else
		return NULL;
}

int Shapeinfo_lookup::GetMaleAvShape()
{
	if (!def_av_info)
		setup_avatar_data();
	return (*def_av_info)[false].shape_num;
}

int Shapeinfo_lookup::GetFemaleAvShape()
{
	if (!def_av_info)
		setup_avatar_data();
	return (*def_av_info)[true].shape_num;
}

Avatar_default_skin *Shapeinfo_lookup::GetDefaultAvSkin()
{
	if (!base_av_info)
		setup_avatar_data();
	return base_av_info;
}

vector<Skin_data> *Shapeinfo_lookup::GetSkinList()
{
	if (!skins_table)
		setup_avatar_data();
	return skins_table;
}

Skin_data *Shapeinfo_lookup::GetSkinInfo(int skin, bool sex)
{
	if (!skins_table)
		setup_avatar_data();
	for (vector<Skin_data>::iterator it = skins_table->begin();
			it != skins_table->end(); ++it)
		if ((*it).skin_id == skin && (*it).is_female == sex)
			return &(*it);
	return NULL;
}

Skin_data *Shapeinfo_lookup::GetSkinInfoSafe(int skin, bool sex, bool sishapes)
{
	Skin_data *sk = GetSkinInfo(skin, sex);
	if (sk && (sishapes ||
				(!IsSkinImported(sk->shape_num) && 
					!IsSkinImported(sk->naked_shape))))
		return sk;
	sk = GetSkinInfo(GetDefaultAvSkin()->default_skin, sex);
	// Prevent unavoidable problems. *Should* never be needed.
	assert(sk && (sishapes ||
			(!IsSkinImported(sk->shape_num) && 
				!IsSkinImported(sk->naked_shape))));
	return sk;
}

Skin_data *Shapeinfo_lookup::GetSkinInfoSafe(Actor *npc)
{
	int skin = npc->get_skin_color();
	bool sex = npc->get_type_flag(Actor::tf_sex);
	return GetSkinInfoSafe(skin, sex, Shape_manager::get_instance()->have_si_shapes());
}

Skin_data *Shapeinfo_lookup::ScrollSkins
	(
	int skin, bool sex, bool sishapes, bool ignoresex, bool prev, bool sel
	)
{
	if (!base_av_info)
		setup_avatar_data();
	bool nsex = sex;
	int nskin = skin;
	bool chskin = true;
	while (true)
		{
		if (ignoresex)
			{
			nsex = !nsex;
			chskin = (nsex == base_av_info->default_female);
			}
		nskin = (nskin + (prev ? last_skin : 0) + chskin) % (last_skin+1);
		if (sel && !IsSkinSelectable(nskin))
			continue;
		Skin_data *newskin = GetSkinInfo(nskin, nsex);
		if (newskin && (sishapes ||
				(!IsSkinImported(newskin->shape_num) &&
					!IsSkinImported(newskin->naked_shape))))
			return newskin;
		}
}

int Shapeinfo_lookup::GetNumSkins(bool sex)
{
	if (!skins_table)
		setup_avatar_data();
	int cnt = 0;
	for (vector<Skin_data>::iterator it = skins_table->begin();
			it != skins_table->end(); ++it)
		if ((*it).is_female == sex)
			cnt++;
	return cnt;
}

Usecode_function_data *Shapeinfo_lookup::GetAvUsecode(int type)
{
	if (!usecode_funs)
		setup_avatar_data();
	map<int, Usecode_function_data>::iterator it = usecode_funs->find(type);
	if (it != usecode_funs->end())
		return &(*it).second;
	else
		return NULL;
}

bool Shapeinfo_lookup::IsSkinSelectable(int skin)
{
	if (!unselectable_skins)
		setup_avatar_data();
	map<int, bool>::iterator it = unselectable_skins->find(skin);
	if (it != unselectable_skins->end())
		return false;
	else
		return true;
}

bool Shapeinfo_lookup::HasFaceReplacement(int npcid)
{
	if (!petra_table)
		setup_avatar_data();
	map<int, int>::iterator it = petra_table->find(npcid);
	if (it != petra_table->end())
		return (*it).second;
	else
		return npcid;
}

int Shapeinfo_lookup::GetFaceReplacement(int facenum)
{
	if (!petra_table)
		setup_avatar_data();
	Game_window *gwin = Game_window::get_instance();
	if (gwin->get_main_actor()->get_flag(Obj_flags::petra))
		{
		map<int, int>::iterator it = petra_table->find(facenum);
		if (it != petra_table->end())
			return (*it).second;
		}
	return facenum;
}
