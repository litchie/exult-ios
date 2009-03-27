/*
 *	shapevga.cc - Handle the 'shapes.vga' file and associated info.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
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

#include <iomanip>			/* For debugging only. */
#include <sstream>
#include "shapevga.h"
#include "ammoinf.h"
#include "aniinf.h"
#include "armorinf.h"
#include "bodyinf.h"
#include "continf.h"
#include "effhpinf.h"
#include "expinf.h"
#include "frnameinf.h"
#include "frpowers.h"
#include "monstinf.h"
#include "npcdollinf.h"
#include "objdollinf.h"
#include "sfxinf.h"
#include "warminf.h"
#include "weaponinf.h"
#include "data/exult_bg_flx.h"
#include "data/exult_si_flx.h"
#include "utils.h"
#include "databuf.h"
#include "msgfile.h"
#include "U7file.h"
#include "exceptions.h"
#include "ready.h"

using std::ifstream;
using std::ios;
using namespace std;

// For convienience
#define patch_exists(p) (have_patch_path && U7exists(p))
#define patch_name(p) (patch_exists(p) ? (p) : 0)

/*
 *	Open, but don't quit if editing.  We first try the patch name if it's
 *	given.
 */

static bool U7open2
	(
	ifstream& in,			// Stream to open.
	const char *pname,		// Patch name, or null.
	const char *fname,		// File name.
	bool editing
	)
	{
	if (pname)
		{
		U7open(in, pname);
		return true;
		}
	try
		{
		U7open(in, fname);
		}
	catch(const file_exception & f)
		{
#if 0
		if (editing)
			return false;
		throw f;
#else	/* This is preferable. */
		return false;
#endif
		}
	return true;
	}

/*
 *	Get # entries of binary data file (with Exult extension).
 */

static inline int Read_count(ifstream& in)
	{
	int cnt = Read1(in);	// How the originals did it.
	if (cnt == 255)
		cnt = Read2(in);	// Exult extension.
	return cnt;
	}

// Base class.
class Base_reader
	{
protected:
	map<int, Shape_info>& info;
	bool haveversion;
	virtual int read_data(istream& in, int index, int version, bool patch, bool bg)
		{ return -1; }
public:
	Base_reader(map<int, Shape_info>& nfo, bool h)
		:	info(nfo), haveversion(h)
		{  }
	virtual ~Base_reader() {  }
		// Text data file.
	void parse(vector<char *> strings, int version, bool patch, bool bg)
		{
		for (int j=0; j<strings.size(); j++)
			{
			char *ptr = strings[j];
			if (!ptr)
				continue;
			istringstream strin(string(ptr), ios::in);
			read_data(strin, j, version, patch, bg);
			delete[] strings[j];
			}
		strings.clear();
		}
		// Binary data file.
	void read(const char *fname, bool patch, bool bg)
		{
		if (!U7exists(fname))
			return;
		ifstream fin;
		U7open(fin, fname);
		int vers = 0;
		if (haveversion)
			vers = Read1(fin);
		int cnt = Read_count(fin);
		for (int j = 0; j < cnt; j++)
			read_data(fin, j, vers, patch, bg);
		fin.close();
		}
	};

template <typename T, T Shape_info::*data>
class Text_reader_functor
	{
public:
	int operator()(std::istream& in, int index, int version, bool patch, 
			bool bg, std::map<int, Shape_info>& info)
		{
		int shapenum = ReadInt(in);
		info[shapenum].*data = ReadInt(in);
		return shapenum;
		}
	};

template <typename T, T Shape_info::*data, int bit>
class Bit_text_reader_functor
	{
public:
	int operator()(std::istream& in, int index, int version, bool patch, 
			bool bg, std::map<int, Shape_info>& info)
		{
		int shapenum = ReadInt(in);
			// For backwards compatibility.
		bool biton = ReadInt(in, 1) != 0;
		if (biton)
			info[shapenum].*data |= ((T)1 << bit);
		else
			info[shapenum].*data &= ~((T)1 << bit);
		return shapenum;
		}
	};

template <typename T, T Shape_info::*data, unsigned pad>
class Binary_reader_functor
	{
public:
	int operator()(std::istream& in, int index, int version, bool patch, 
			bool bg, std::map<int, Shape_info>& info)
		{
		int shapenum = Read2(in);
		in.read((char *)(&(info[shapenum].*data)), sizeof(T)/sizeof(char));
		if (pad)	// Skip some bytes.
			in.seekg(pad, std::ios::cur);
		return shapenum;
		}
	};

// Deviating case #1.
class Readytype_reader_functor
	{
public:
	int operator()(std::istream& in, int index, int version, bool patch, 
			bool bg, std::map<int, Shape_info>& info)
		{
		int shapenum = Read2(in);
		unsigned char ready = Read1(in);
			// Skip some bytes.
		in.seekg(6, std::ios::cur);

		info[shapenum].spell_flag = ready&1;
		ready = ready >> 3;
		char spot = bg ? Ready_spot_from_BG(ready)
		               : Ready_spot_from_SI(ready);
		info[shapenum].ready_type = spot;
				// Init alternate spots.
		switch (spot)
			{
		case lfinger:
			info[shapenum].alt_ready1 = rfinger;
			break;
		case lhand:
			info[shapenum].alt_ready1 = rhand;
			info[shapenum].alt_ready2 = belt;
			break;
		case both_hands:
			info[shapenum].alt_ready1 = back_2h;
			break;
			}
		return shapenum;
		}
	};

// Deviating case #2.
class Altreadytype_reader_functor
	{
public:
	int operator()(std::istream& in, int index, int version, bool patch, 
			bool bg, std::map<int, Shape_info>& info)
		{
		int shapenum = ReadInt(in);
		info[shapenum].alt_ready1 = (char)ReadInt(in, -1);
		info[shapenum].alt_ready2 = (char)ReadInt(in, -1);
		return shapenum;
		}
	};

template <int flag, class Functor>
class Functor_data_reader : public Base_reader
	{
protected:
	Functor reader;
	virtual int read_data(istream& in, int index, int version, bool patch, bool bg)
		{
		int shapenum = reader(in, index, version, patch, bg, info);
		if (patch)
			info[shapenum].frompatch_flags |= flag;
		return shapenum;
		}
public:
	Functor_data_reader(map<int, Shape_info>& nfo, bool h=false)
		:	Base_reader(nfo, h)
		{  }
	virtual ~Functor_data_reader()
		{  }
	};

// Truly deviating case.
class Actor_flags_reader : public Base_reader
	{
protected:
	virtual int read_data(istream& in, int index, int version, bool patch, bool bg)
		{
		int shapenum = ReadInt(in);
		int size = 8*sizeof(unsigned char);	// Bit count.
		int bit = 0;
		unsigned char flags = 0;
		while (in.good() && bit < size)
			{
			if (ReadInt(in) != 0)
				flags |= (1U << bit);
			else
				flags &= ~(1U << bit);
			bit++;
			}
		// Here starts the deviating part.
		// We already have monster data by this point.
		Monster_info *minf= info[shapenum].monstinf;
		if (minf)
			{
			// Deprecating old Monster_info based flags for these powers:
			if (minf->can_teleport())
				flags |= Shape_info::teleports;
			if (minf->can_summon())
				flags |= Shape_info::summons;
			if (minf->can_be_invisible())
				flags |= Shape_info::turns_invisible;
			}
		info[shapenum].actor_flags = flags;
		if (patch)
			info[shapenum].frompatch_flags |= actor_flags_flag;
		return shapenum;
		}
public:
	Actor_flags_reader(map<int, Shape_info>& nfo, bool h=false)
		:	Base_reader(nfo, h)
		{  }
	virtual ~Actor_flags_reader() {  }
	};

template <typename T, T *Shape_info::*data>
class Class_data_reader : public Base_reader
	{
protected:
	int read_data(std::istream& in, int index, int version, bool patch, bool bg)
		{
		T *cls = new T();
		cls->set_patch(patch);	// Set patch flag.
		int shapenum = cls->read(in, index, version, bg);
		if (shapenum < 0)
			{
			delete cls;
			return -1;
			}
		T *&pt = info[shapenum].*data;
		if (cls->is_invalid() && pt)
			{	// 'Delete old' flag.
			delete pt;
			pt = 0;
			delete cls;
			return -1;
			}
		if (!patch)		// This is a static data file.
			info[shapenum].have_static_flags |= T::get_info_flag();
		// Delete old.
		delete pt;
		pt = cls;
		return shapenum;
		}
public:
	Class_data_reader(map<int, Shape_info>& nfo, bool h=false)
		:	Base_reader(nfo, h)
		{  }
	virtual ~Class_data_reader() {  }
	};

template <typename T, vector<T> Shape_info::*data>
class Vector_data_reader : public Base_reader
	{
protected:
	int read_data(std::istream& in, int index, int version, bool patch, bool bg)
		{
		T cls;
		cls.set_patch(patch);
		int shapenum = cls.read(in, index, version, bg);
		vector<T>& vec = info[shapenum].*data;
		Shape_info::add_vector_info(cls, vec);
		return shapenum;
		}
public:
	Vector_data_reader(map<int, Shape_info>& nfo, bool h=false)
		:	Base_reader(nfo, h)
		{  }
	virtual ~Vector_data_reader() {  }
	};

/*
 *	Reads text data file and parses it according to passed
 *	parser functions.
 */

static void Read_text_data_file
	(
	const char *fname,				// Name of file to read, sans extension
	Base_reader *parsers[],	// What to use to parse data.
	const char *sections[],			// The names of the sections
	int numsections,				// Number of sections
	bool editing,
	Exult_Game game_type,
	int resource
	)
	{
	int static_version = 1;
	int patch_version = 1;
	vector<vector<char *> > static_strings;
	vector<vector<char *> > patch_strings;
	char buf[50];
	if (game_type == BLACK_GATE || game_type == SERPENT_ISLE)
		{
		/*	++++ Not because of ES.
		snprintf(buf, 50, "config/%s", fname);
		str_int_pair resource = game->get_resource(buf);
		U7object txtobj(resource.str, resource.num);
		*/
		snprintf(buf, 50, "<DATA>/exult_%s.flx", 
				game_type == BLACK_GATE ? "bg" : "si");
		U7object txtobj(buf, resource);
		std::size_t len;
		char *txt = txtobj.retrieve(len);
		BufferDataSource ds(txt, len);
		static_version = Read_text_msg_file_sections(&ds,
				static_strings, sections, numsections);
		delete [] txt;
		}
	else
		{
		try
			{
			snprintf(buf, 50, "<STATIC>/%s.txt", fname);
			ifstream in;
			U7open(in, buf, false);
			static_version = Read_text_msg_file_sections(in,
					static_strings, sections, numsections);
			in.close();
			}
		catch (std::exception &e)
			{
			if (!editing)
				throw e;
			static_strings.resize(numsections);
			}
		}
	patch_strings.resize(numsections);
	snprintf(buf, 50, "<PATCH>/%s.txt", fname);
	if (U7exists(buf))
		{
		ifstream in;
		U7open(in, buf, false);
		patch_version = Read_text_msg_file_sections(in, patch_strings,
				sections, numsections);
		in.close();
		}
	bool bg = game_type == BLACK_GATE;
	for (int i = 0; i < numsections; i++)
		{
		parsers[i]->parse(static_strings[i], static_version, false, bg);
		parsers[i]->parse(patch_strings[i], patch_version, true, bg);
		delete parsers[i];
		}
	static_strings.clear();
	patch_strings.clear();
	}

void Shapes_vga_file::Read_Shapeinf_text_data_file
	(
	bool editing,
	Exult_Game game_type
	)
	{
	const char *sections[] = {"explosions", "shape_sfx", "animation",
			"usecode_events", "mountain_tops", "monster_food", "actor_flags",
			"effective_hps", "lightweight_object", "warmth_data",
			"quantity_frames", "locked_containers", "content_rules",
			"volatile_explosive", "framenames", "altready", "barge_type",
			"frame_powers"
			};
	Base_reader *readers[] = {
			new Class_data_reader<Explosion_info, &Shape_info::explosion>(info),
			new Class_data_reader<SFX_info, &Shape_info::sfxinf>(info),
			new Class_data_reader<Animation_info, &Shape_info::aniinf>(info),
			new Functor_data_reader<usecode_events_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::usecode_events> >(info),
			new Functor_data_reader<mountain_top_flag,
				Text_reader_functor<short, &Shape_info::mountain_top> >(info),
			new Functor_data_reader<monster_food_flag,
				Text_reader_functor<short, &Shape_info::monster_food> >(info),
			new Actor_flags_reader(info),
			new Vector_data_reader<Effective_hp_info, &Shape_info::hpinf>(info),
			new Functor_data_reader<lightweight_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::lightweight> >(info),
			new Vector_data_reader<Warmth_info, &Shape_info::warminf>(info),
			new Functor_data_reader<quantity_frames_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::quantity_frames> >(info),
			new Functor_data_reader<is_locked_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::is_locked> >(info),
			new Vector_data_reader<Content_rules, &Shape_info::cntrules>(info),
			new Functor_data_reader<is_volatile_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::is_volatile> >(info),
			new Vector_data_reader<Frame_name_info, &Shape_info::nameinf>(info),
			new Functor_data_reader<altready_type_flag,
				Altreadytype_reader_functor >(info),
			new Functor_data_reader<barge_type_flag,
				Text_reader_functor<short, &Shape_info::barge_type> >(info),
			new Vector_data_reader<Frame_powers_info, &Shape_info::frpowerinf>(info)
			};
	int numsections = sizeof(sections)/sizeof(sections[0]);
	int numreaders = sizeof(readers)/sizeof(readers[0]);
	assert(numsections == numreaders);
	int flxres = game_type == BLACK_GATE ?
			EXULT_BG_FLX_SHAPE_INFO_TXT : EXULT_SI_FLX_SHAPE_INFO_TXT;

	Read_text_data_file("shape_info", readers, sections,
			numsections, editing, game_type, flxres);
	}

void Shapes_vga_file::Read_Bodies_text_data_file
	(
	bool editing,
	Exult_Game game_type
	)
	{
	const char *sections[] = {"bodyshapes", "bodylist"};
	Base_reader *readers[] = {
			new Functor_data_reader<is_body_flag,
				Bit_text_reader_functor<unsigned char, &Shape_info::shape_flags,
						Shape_info::is_body> >(info),
			new Class_data_reader<Body_info, &Shape_info::body>(info)
			};
	int numsections = sizeof(sections)/sizeof(sections[0]);
	int numreaders = sizeof(readers)/sizeof(readers[0]);
	assert(numsections == numreaders);
	int flxres = game_type == BLACK_GATE ?
			EXULT_BG_FLX_BODIES_TXT : EXULT_SI_FLX_BODIES_TXT;

	Read_text_data_file("bodies", readers, sections,
			numsections, editing, game_type, flxres);
	}

void Shapes_vga_file::Read_Paperdoll_text_data_file
	(
	bool editing,
	Exult_Game game_type
	)
	{
	const char *sections[] = {"characters", "items"};
	Base_reader *readers[] = {
			new Class_data_reader<Paperdoll_npc, &Shape_info::npcpaperdoll>(info),
			new Vector_data_reader<Paperdoll_item, &Shape_info::objpaperdoll>(info)
			};
	int numsections = sizeof(sections)/sizeof(sections[0]);
	int numreaders = sizeof(readers)/sizeof(readers[0]);
	assert(numsections == numreaders);
	int flxres = game_type == BLACK_GATE ?
			EXULT_BG_FLX_PAPERDOL_INFO_TXT : EXULT_SI_FLX_PAPERDOL_INFO_TXT;

	Read_text_data_file("paperdol_info", readers, sections,
			numsections, editing, game_type, flxres);
	}

/*
 *	Reload static data for weapons, ammo and mosters to
 *	fix data that was lost by earlier versions of ES.
 */
void Shapes_vga_file::fix_old_shape_info
	(
	Exult_Game game		// Which game.
	)
	{
	if (!info_read)	// Read info first.
		read_info(game, true);
	bool bg = game == BLACK_GATE;
	Class_data_reader<Weapon_info, &Shape_info::weapon> weapon(info);
	weapon.read(WEAPONS, false, bg);
	Class_data_reader<Ammo_info, &Shape_info::ammo> ammo(info);
	ammo.read(AMMO, false, bg);
	Class_data_reader<Monster_info, &Shape_info::monstinf> monstinf(info);
	monstinf.read(MONSTERS, false, bg);
	}

/*
 *	Reload info.
 */

void Shapes_vga_file::reload_info
	(
	Exult_Game game		// Which game.
	)
	{
	info_read = false;
	info.clear();
	read_info(game);
	}	

/*
 *	Read in data files about shapes.
 *
 *	Output:	0 if error.
 */

void Shapes_vga_file::read_info
	(
	Exult_Game game,		// Which game.
	bool editing			// True to allow files to not exist.
	)
	{
	if (info_read)
		return;
	info_read = true;
	int i, cnt;
	bool have_patch_path = is_system_path_defined("<PATCH>");

	// ShapeDims

	// Starts at 0x96'th shape.
	ifstream shpdims;
	if (U7open2(shpdims, patch_name(PATCH_SHPDIMS), SHPDIMS, editing))
		for (i = c_first_obj_shape; 
					i < num_shapes && !shpdims.eof(); i++)
			{
			shpdims.get((char&) info[i].shpdims[0]);
			shpdims.get((char&) info[i].shpdims[1]);
			}

	// WGTVOL
	ifstream wgtvol;
	if (U7open2(wgtvol, patch_name(PATCH_WGTVOL), WGTVOL, editing))
		for (i = 0; i < num_shapes && !wgtvol.eof(); i++)
			{
			wgtvol.get((char&) info[i].weight);
			wgtvol.get((char&) info[i].volume);
			}

	// TFA
	ifstream tfa;
	if (U7open2(tfa, patch_name(PATCH_TFA), TFA, editing))
		for (i = 0; i < num_shapes && !tfa.eof(); i++)
			{
			tfa.read((char*)&info[i].tfa[0], 3);
			info[i].set_tfa_data();
			}

	if (game == BLACK_GATE || game == SERPENT_ISLE)
		{	// Animation data at the end of BG and SI TFA.DAT
		ifstream stfa;
			// We *should* blow up if TFA not there.
		U7open(stfa, TFA);
		stfa.seekg(3*1024);
		unsigned char buf[512];
		stfa.read((char *)buf, 512);
		stfa.close();
		unsigned char *ptr = buf;
		for (int i = 0; i < 512; i++, ptr++)
			{
			int val = *ptr;
			int shape = 2*i;
			while (val)
				{
				if (val&0xf)
					{
					delete info[shape].aniinf;
					info[shape].aniinf = Animation_info::create_from_tfa(
								val&0xf, get_num_frames(shape));
					}
				val >>= 4;
				shape++;
				}
			}
		}

	// Load data about drawing the weapon in an actor's hand
	ifstream wihh;
	unsigned short offsets[c_max_shapes];
	if (U7open2(wihh, patch_name(PATCH_WIHH), WIHH, editing))
		{
		cnt = num_shapes;
		for (i = 0; i < cnt; i++)
			offsets[i] = Read2(wihh);
		for (i = 0; i < cnt; i++)
			// A zero offset means there is no record
			if(offsets[i] == 0)
				info[i].weapon_offsets = 0;
			else
				{
				wihh.seekg(offsets[i]);
				// There are two bytes per frame: 64 total
				info[i].weapon_offsets = new unsigned char[64];
				for(int j = 0; j < 32; j++)
					{
					unsigned char x = Read1(wihh);
					unsigned char y = Read1(wihh);
					// Set x/y to 255 if weapon is not to be drawn
					// In the file x/y are either 64 or 255:
					// I am assuming that they mean the same
					if(x > 63 || y > 63)
						x = y = 255;
					info[i].weapon_offsets[j * 2] = x;
					info[i].weapon_offsets[j * 2 + 1] = y;
					}
				}
		wihh.close();
		}

	ifstream(occ);			// Read flags from occlude.dat.
	if (U7open2(occ, patch_name(PATCH_OCCLUDE), OCCLUDE, editing))
		{
		unsigned char occbits[c_occsize];	// c_max_shapes bit flags.
			// Ensure sensible defaults.
		memset(&occbits[0], 0, sizeof(occbits));
		occ.read((char *)occbits, sizeof(occbits));
		for (i = 0; i < occ.gcount(); i++)
			{
			unsigned char bits = occbits[i];
			int shnum = i*8;	// Check each bit.
			for (int b = 0; bits; b++, bits = bits>>1)
				if (bits&1)
					info[shnum + b].occludes_flag = true;
			}
		}

			// Get 'equip.dat'.
	ifstream mfile;
	if (U7open2(mfile, patch_name(PATCH_EQUIP), EQUIP, editing))
		{
			// Get # entries (with Exult extension).
		int num_recs = Read_count(mfile);
		Monster_info::reserve_equip(num_recs);
		for (i = 0; i < num_recs; i++)
			{
			Equip_record equip;
					// 10 elements/record.
			for (int elem = 0; elem < 10; elem++)
				{
				int shnum = Read2(mfile);
				unsigned prob = Read1(mfile);
				unsigned quant = Read1(mfile);
				Read2(mfile);
				equip.set(elem, shnum, prob, quant);
				}
			Monster_info::add_equip(equip);
			}
		mfile.close();
		}

	bool bg = game == BLACK_GATE;
	Class_data_reader<Armor_info, &Shape_info::armor> armor(info);
	armor.read(ARMOR, false, bg);
	armor.read(PATCH_ARMOR, true, bg);

	Class_data_reader<Weapon_info, &Shape_info::weapon> weapon(info);
	weapon.read(WEAPONS, false, bg);
	weapon.read(PATCH_WEAPONS, true, bg);

	Class_data_reader<Ammo_info, &Shape_info::ammo> ammo(info);
	ammo.read(AMMO, false, bg);
	ammo.read(PATCH_AMMO, true, bg);

	Class_data_reader<Monster_info, &Shape_info::monstinf> monstinf(info);
	monstinf.read(MONSTERS, false, bg);
	monstinf.read(PATCH_MONSTERS, true, bg);

	Functor_data_reader<container_gump_flag, Binary_reader_functor<short,
		&Shape_info::container_gump, 0> > container(info, true);
	container.read(CONTAINER, false, bg);
	container.read(PATCH_CONTAINER, true, bg);
	Functor_data_reader<ready_type_flag, Readytype_reader_functor> ready(info);
	ready.read(READY, false, bg);
	ready.read(PATCH_READY, true, bg);

	Read_Shapeinf_text_data_file(editing, game);
	Read_Bodies_text_data_file(editing, game);
	Read_Paperdoll_text_data_file(editing, game);

	// Ensure valid ready spots for all shapes.
	zinfo.ready_type = bg ? backpack : rhand;
	for (std::map<int, Shape_info>::iterator it = info.begin();
			it != info.end(); ++it)
		{
		Shape_info& inf = it->second;
		if (inf.ready_type < 0)
			inf.ready_type = bg ? backpack : rhand;
		}
	}

/*
 *	Open/close file.
 */

Shapes_vga_file::Shapes_vga_file
	(
	const char *nm,			// Path to file.
	int u7drag,			// # from u7drag.h, or -1.
	const char *nm2			// Path to patch version, or 0.
	) : Vga_file(nm, u7drag, nm2), info_read(false)
	{
	}

Shapes_vga_file::~Shapes_vga_file()
	{
	}


void Shapes_vga_file::init()
{
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_SHAPES))
		load(SHAPES_VGA, PATCH_SHAPES);
	else
		load(SHAPES_VGA);
	info_read = false;
}

/*
 *	Make a spot for a new shape, and delete frames in existing shape.
 *
 *	Output:	->shape, or 0 if invalid shapenum.
 */

Shape *Shapes_vga_file::new_shape
	(
	int shapenum
	)
	{
	Shape *newshape = Vga_file::new_shape(shapenum);
	return newshape;
	}
