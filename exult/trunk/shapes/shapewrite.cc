/*
 *	shapewrite.cc - Write out the shape 'info' files.
 *
 *	Note:  Currently only used by ExultStudio.
 *
 *  Copyright (C) 2002  The Exult Team
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

#include <map>
#include <vector>
#include <iomanip>
#include "shapevga.h"
#include "shapeinf.h"
#include "ammoinf.h"
#include "aniinf.h"
#include "armorinf.h"
#include "bodyinf.h"
#include "continf.h"
#include "effhpinf.h"
#include "expinf.h"
#include "frnameinf.h"
#include "monstinf.h"
#include "npcdollinf.h"
#include "objdollinf.h"
#include "sfxinf.h"
#include "warminf.h"
#include "weaponinf.h"
#include "utils.h"
#include "exceptions.h"

using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::endl;
using std::map;
using std::vector;

static inline void Write_count
	(
	ofstream& out,
	int cnt
	)
	{
	if (cnt >= 255)
		{	// Exult extension.
		out.put(255);
		Write2(out, cnt);
		}
	else
		out.put(cnt);
	}

// Abstract base class.
class Abstract_base_writer
	{
protected:
	char *name;
	map<int, Shape_info>& info;
	const int num_shapes;
	const int version;
	int cnt;
	virtual bool need_write(Shape_info& inf)
		{ return false; }
	virtual void write_data(ostream& out, int index, bool bg)
		{  }
public:
	Abstract_base_writer(const char *s, map<int, Shape_info>& nfo, int n, int v=-1)
		:	name(strdup(s)), info(nfo), num_shapes(n), version(v), cnt(-1)
		{  }
	virtual ~Abstract_base_writer()
		{ delete name; }
	int check()
		{
		if (cnt > -1)	// Return cached value.
			return cnt;
		cnt = 0;
		for (int i = 0; i < num_shapes; i++)
			if (need_write(info[i]))
				cnt++;
		return cnt;
		}
	void write_text
		(
		ostream& out,
		bool bg
		)
		{
		if (cnt <= 0)	// Nothing to do.
			return;
			// Section is not empty.
		out << "%%section " << name << endl;
		for (int i = 0; i < num_shapes; i++)
			if (need_write(info[i]))
				write_data(out, i, bg);
		out << "%%endsection" << endl;
		}
	void write_binary
		(
		bool bg
		)
		{
		if (cnt <= 0)	// Nothing to do.
			return;
		ofstream fout;	// Open file.
		U7open(fout, name);
		if (version >= 0)	// container.dat has version #.
			fout.put(version);
		Write_count(fout, cnt);	// Object count, with Exult extension.
		for (int i = 0; i < num_shapes; i++)
			if (need_write(info[i]))
				write_data(fout, i, bg);
		fout.close();
		}
	};

template <typename T>
class Text_writer_functor
	{
public:
	void operator()(ostream& out, int shapenum, bool bg, T data)
		{ out << ':' << shapenum << '/' << (int)data << endl; }
	};

template <typename T, int bit>
class Bit_text_writer_functor
	{
public:
	void operator()(ostream& out, int shapenum, bool bg, T data)
		{
		bool val = (data & ((T)1 << bit));
		out << ':' << shapenum << '/' << val << endl;
		}
	};

template <typename T>
class Bit_field_text_writer_functor
	{
public:
	void operator()(ostream& out, int shapenum, bool bg, T data)
		{
		out << shapenum << '/';
		int size = 8*sizeof(T)-1;	// Bit count.
		int bit = 0;
		while (bit < size)
			{
			out << (bool)((data & ((T)1 << bit)) != 0) << '/';
			bit++;
			}
		out << (bool)((data & ((T)1 << size)) != 0) << endl;
		}
	};

template <typename T, int pad>
class Binary_writer_functor
	{
public:
	void operator()(ostream& out, int shapenum, bool bg, T data)
		{
		Write2(out, shapenum);
		out.write((char *)(&data), sizeof(T)/sizeof(char));
		for (int i = 0; i < pad; i++)
			out.put(0);
		}
	};

template <typename T, T Shape_info::*data, int flag,
          typename Functor = Text_writer_functor<T> >
class Functor_data_writer : public Abstract_base_writer
	{
protected:
	Functor writer;
	virtual bool need_write(Shape_info& inf)
		{ return (inf.modified_flags | inf.frompatch_flags) & flag; }
	virtual void write_data(ostream& out, int index, bool bg)
		{ writer(out, index, bg, info[index].*data); }
public:
	Functor_data_writer(const char *s, map<int, Shape_info>& nfo, int n, int v=-1)
		:	Abstract_base_writer(s, nfo, n, v)
		{ check(); }
	virtual ~Functor_data_writer() {  }
	};

template <typename T, T *Shape_info::*data>
class Class_data_writer : public Abstract_base_writer
	{
protected:
	virtual bool need_write(Shape_info& inf)
		{
		T *cls = inf.*data;
		if (!cls)
			return (inf.have_static_flags & T::get_info_flag()) != 0;
		return (cls->need_write());
		}
	virtual void write_data(ostream& out, int index, bool bg)
		{
		T *cls = info[index].*data;
		if (!cls)	// Write 'remove' block.
			{
			if (T::get_entry_size() < 0)	// Text entry.
				out << ':' << index << '/' << -255 << endl;
			else
				{	// T::get_entry_size() should be >= 3!
				#ifdef _MSC_VER
					unsigned char *buf = new unsigned char[T::get_entry_size()];
				#else
					unsigned char buf[T::get_entry_size()];
				#endif
				unsigned char *ptr = buf;
				Write2(ptr, index);
				if (T::get_entry_size() >= 4)
					memset(ptr, 0, T::get_entry_size()-3);
				buf[T::get_entry_size()-1] = 0xff;
				out.write((const char *)buf, T::get_entry_size());
				#ifdef _MSC_VER
					delete buf;
				#endif
				}
			}
		else
			cls->write(out, index, bg);
		}
public:
	Class_data_writer(const char *s, map<int, Shape_info>& nfo, int n, int v=-1)
		:	Abstract_base_writer(s, nfo, n, v)
		{ check(); }
	virtual ~Class_data_writer() {  }
	};

template <typename T, vector<T> Shape_info::*data>
class Vector_data_writer : public Abstract_base_writer
	{
protected:
	virtual bool need_write(Shape_info& inf)
		{
		vector<T>& vec = inf.*data;
		if (!vec.size())	// Nothing to do.
			return false;
		for (typename vector<T>::iterator it = vec.begin();
				it != vec.end(); ++it)
			if (it->need_write() || (it->is_invalid() && it->have_static()))
				return true;
		return false;
		}
	virtual void write_data(ostream& out, int index, bool bg)
		{
		vector<T>& vec = info[index].*data;
		for (typename vector<T>::iterator it = vec.begin();
				it != vec.end(); ++it)
			if (it->need_write() || (it->is_invalid() && it->have_static()))
				it->write(out, index, bg);
		}
public:
	Vector_data_writer(const char *s, map<int, Shape_info>& nfo, int n, int v=-1)
		:	Abstract_base_writer(s, nfo, n, v)
		{  }
	virtual ~Vector_data_writer()
		{ check(); }
	};

/*
 *	Writes a text data file.
 */
static void Write_text_data_file
	(
	map<int, Shape_info>& info,		// The info map.
	const char *fname,				// Name of file to read, sans extension
	Abstract_base_writer *writers[],	// What to use to write data.
			// Ugh.
	int numsections,				// Number of sections
	int version,
	Exult_Game game
	)
	{
	int cnt = 0;
	for (int i = 0; i < numsections; i++)
		cnt += writers[i]->check();
	if (!cnt)	// Nothing to do.
		return;
	ofstream out;
	char buf[50];
	snprintf(buf, 50, "<PATCH>/%s.txt", fname);
	U7open(out, buf, true);	// (It's a text file.)
	out << "#\tExult " << VERSION << " text message file."
		<< "\tWritten by ExultStudio." << endl;
	out << "%%section version" << endl
		<< ":" << version << endl
		<< "%%endsection" << endl;
	bool bg = game == BLACK_GATE;
	for (int i = 0; i < numsections; i++)
		{
		writers[i]->write_text(out, bg);
		delete writers[i];
		}
	out.close();
	}

void Shapes_vga_file::Write_Shapeinf_text_data_file(Exult_Game game)
	{
	Abstract_base_writer *writers[] = {
			new Class_data_writer<Explosion_info, &Shape_info::explosion>(
				"explosions", info, num_shapes),
			new Class_data_writer<SFX_info, &Shape_info::sfxinf>(
				"shape_sfx", info, num_shapes),
			new Class_data_writer<Animation_info, &Shape_info::aniinf>(
				"animation", info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				usecode_events_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::usecode_events> >("usecode_events",
					info, num_shapes),
			new Functor_data_writer<short, &Shape_info::mountain_top,
				mountain_top_flag>("mountain_tops", info, num_shapes),
			new Functor_data_writer<short, &Shape_info::monster_food,
				monster_food_flag>("monster_food", info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::actor_flags,
				actor_flags_flag, Bit_field_text_writer_functor<unsigned char> >(
				"actor_flags", info, num_shapes),
			new Vector_data_writer<Effective_hp_info, &Shape_info::hpinf>(
				"effective_hps", info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				lightweight_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::lightweight> >("lightweight_object",
					info, num_shapes),
			new Vector_data_writer<Warmth_info, &Shape_info::warminf>(
				"warmth_data", info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				quantity_frames_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::quantity_frames> >("quantity_frames",
					info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				is_locked_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::is_locked> >("locked_containers",
					info, num_shapes),
			new Vector_data_writer<Content_rules, &Shape_info::cntrules>(
				"content_rules", info, num_shapes),
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				is_volatile_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::is_volatile> >("volatile_explosive",
					info, num_shapes),
			new Vector_data_writer<Frame_name_info, &Shape_info::nameinf>(
				"framenames", info, num_shapes)
			};
	int numsections = sizeof(writers)/sizeof(writers[0]);
	Write_text_data_file(info, "shape_info", writers, numsections, 5, game);
	}

void Shapes_vga_file::Write_Bodies_text_data_file(Exult_Game game)
	{
	Abstract_base_writer *writers[] = {
			new Functor_data_writer<unsigned char, &Shape_info::shape_flags,
				is_body_flag, Bit_text_writer_functor<unsigned char,
						Shape_info::is_body> >("bodyshapes",
					info, num_shapes),
			new Class_data_writer<Body_info, &Shape_info::body>(
				"bodylist", info, num_shapes),
			};
	int numsections = sizeof(writers)/sizeof(writers[0]);
	Write_text_data_file(info, "bodies", writers, numsections, 2, game);
	}

void Shapes_vga_file::Write_Paperdoll_text_data_file(Exult_Game game)
	{
	Abstract_base_writer *writers[] = {
			new Class_data_writer<Paperdoll_npc, &Shape_info::npcpaperdoll>(
				"characters", info, num_shapes),
			new Vector_data_writer<Paperdoll_item, &Shape_info::objpaperdoll>(
				"items", info, num_shapes)
			};
	int numsections = sizeof(writers)/sizeof(writers[0]);
	Write_text_data_file(info, "paperdol_info", writers, numsections, 2, game);
	}


/*
 *	Write out data files about shapes.
 */

void Shapes_vga_file::write_info
	(
	Exult_Game game
	)
	{
	int i, cnt;
	bool have_patch_path = is_system_path_defined("<PATCH>");
	assert(have_patch_path);

	// ShapeDims
	// Starts at 0x96'th shape.
	ofstream shpdims;
	U7open(shpdims, PATCH_SHPDIMS);
	for (i = c_first_obj_shape; i < num_shapes; i++)
		{
		shpdims.put((char&) info[i].shpdims[0]);
		shpdims.put((char&) info[i].shpdims[1]);
		}

	// WGTVOL
	ofstream wgtvol;
	U7open(wgtvol, PATCH_WGTVOL);
	for (i = 0; i < num_shapes; i++)
		{
		wgtvol.put((char&) info[i].weight);
		wgtvol.put((char&) info[i].volume);
		}

	// TFA
	ofstream tfa;
	U7open(tfa, PATCH_TFA);
	for (i = 0; i < num_shapes; i++)
		tfa.write((char*)&info[i].tfa[0], 3);

	// Write data about drawing the weapon in an actor's hand
	ofstream wihh;
	U7open(wihh, PATCH_WIHH);
	cnt = 0;			// Keep track of actual entries.
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon_offsets == 0)
			Write2(wihh, 0);// None for this shape.
		else			// Write where it will go.
			Write2(wihh, 2*num_shapes + 64*(cnt++));
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon_offsets)
				// There are two bytes per frame: 64 total
			wihh.write((char *)(info[i].weapon_offsets), 64);
	wihh.close();

	ofstream(occ);			// Write occlude.dat.
	U7open(occ, PATCH_OCCLUDE);
	unsigned char occbits[c_occsize];	// c_max_shapes bit flags.
					// +++++This could be rewritten better!
	memset(&occbits[0], 0, sizeof(occbits));
	for (i = 0; i < sizeof(occbits); i++)
		{
		unsigned char bits = 0;
		int shnum = i*8;	// Check each bit.
		for (int b = 0; b < 8; b++)
			if (shnum + b >= num_shapes)
				break;
			else if (info[shnum + b].occludes_flag)
				bits |= (1<<b);
		occbits[i] = bits;	
		}
	occ.write((char *)occbits, sizeof(occbits));

	ofstream mfile;			// Now get monster info.
	U7open(mfile, PATCH_EQUIP);	// Write 'equip.dat'.
	cnt = Monster_info::get_equip_cnt();
	Write_count(mfile, cnt);	// Exult extension.
	for (i = 0; i < cnt; i++)
		{
		Equip_record& rec = Monster_info::get_equip(i);
					// 10 elements/record.
		for (int e = 0; e < 10; e++)
			{
			Equip_element& elem = rec.get(e);
			Write2(mfile, elem.get_shapenum());
			mfile.put(elem.get_probability());
			mfile.put(elem.get_quantity());
			Write2(mfile, 0);
			}
		}
	mfile.close();

	bool bg = game == BLACK_GATE;
	Class_data_writer<Armor_info, &Shape_info::armor> armor(
		PATCH_ARMOR, info, num_shapes);
	armor.write_binary(bg);

	Class_data_writer<Weapon_info, &Shape_info::weapon> weapon(
		PATCH_WEAPONS, info, num_shapes);
	weapon.write_binary(bg);

	Class_data_writer<Ammo_info, &Shape_info::ammo> ammo(
		PATCH_AMMO, info, num_shapes);
	ammo.write_binary(bg);

	Class_data_writer<Monster_info, &Shape_info::monstinf> monstinf(
		PATCH_MONSTERS, info, num_shapes);
	monstinf.write_binary(bg);

	Functor_data_writer<short, &Shape_info::container_gump, container_gump_flag,
			Binary_writer_functor<short, 0> > container_gump(
		PATCH_CONTAINER, info, num_shapes, 1);
	container_gump.write_binary(bg);

	Functor_data_writer<unsigned char, &Shape_info::ready_type, ready_type_flag, 
			Binary_writer_functor<unsigned char, 6> > ready_type(
		PATCH_READY, info, num_shapes);
	ready_type.write_binary(bg);

	Write_Shapeinf_text_data_file(game);
	Write_Bodies_text_data_file(game);
	Write_Paperdoll_text_data_file(game);
	}

void Animation_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, (int)type);
	WriteInt(out, frame_count, type == FA_HOURLY);
	if (type != FA_HOURLY)
		{	// We still have things to write.
		WriteInt(out, frame_delay);
		WriteInt(out, sfx_delay, type != FA_LOOPING);
		if (type == FA_LOOPING)
			{	// We *still* have things to write.
			WriteInt(out, freeze_first);
			WriteInt(out, recycle, true);
			}
		}
	}

void Body_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, bshape);
	WriteInt(out, bframe, true);
	}

void Frame_name_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, frame < 0 ? -1 : (frame & 0xff));
	WriteInt(out, quality < 0 ? -1 : (quality & 0xff));
	WriteInt(out, type, type < 0);
	if (type < 0)
		return;
	WriteInt(out, msgid, type == 0);
	if (type == 0)
		return;
	WriteInt(out, othermsg, true);
	}

void Effective_hp_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, frame < 0 ? -1 : (frame & 0xff));
	WriteInt(out, quality < 0 ? -1 : (quality & 0xff));
	WriteInt(out, hps, true);
	}

void Warmth_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, frame < 0 ? -1 : (frame & 0xff));
	WriteInt(out, warmth, true);
	}

void Content_rules::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, shape < 0 ? -1 : shape);
	WriteInt(out, accept, true);
	}

void Paperdoll_npc::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, is_female);
	WriteInt(out, translucent);
	WriteInt(out, body_shape);
	WriteInt(out, body_frame);
	WriteInt(out, head_shape);
	WriteInt(out, head_frame);
	WriteInt(out, head_frame_helm);
	WriteInt(out, arms_shape);
	WriteInt(out, arms_frame[0]);
	WriteInt(out, arms_frame[1]);
	WriteInt(out, arms_frame[2], !bg);
	if (bg)
		WriteInt(out, gump_shape, true);
	}

void Paperdoll_item::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, world_frame < 0 ? -1 : world_frame);
	WriteInt(out, translucent);
	WriteInt(out, spot);
	WriteInt(out, type, type < 0);
	if (type < 0)	// 'Invalid' entry; we are done.
		return;
	WriteInt(out, gender);
	WriteInt(out, shape);
	int i = 0;
	for (; i < 3; i++)
		{
		if (frame[i] == -1)
			return;		// Done writing.
		WriteInt(out, frame[i], frame[i+1] == -1);
		}
	if (frame[3] != -1)
		WriteInt(out, frame[3], true);
	}

void Explosion_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, sprite, sfxnum < 0);
	if (sfxnum >= 0)
		WriteInt(out, sfxnum, true);
	}

void SFX_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	out << ":";
	WriteInt(out, shapenum);
	WriteInt(out, sfxnum);
	WriteInt(out, chance);
	WriteInt(out, range);
	WriteInt(out, random, extra == -1);
	if (extra >= 0)
		WriteInt(out, extra, true);
	}

/*
 *	Write out a weapon-info entry to 'weapons.dat'.
 */

void Weapon_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	uint8 buf[21];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);		// Bytes 0-1.
	Write2(ptr, ammo);
	Write2(ptr, projectile);
	*ptr++ = damage;
	unsigned char flags0 = (damage_type<<4) | (m_delete_depleted<<3) |
				(m_no_blocking<<2) | (m_explodes<<1) | m_lucky;
	*ptr++ = flags0;
	*ptr++ = (range<<3) | (uses<<1) | m_autohit;
	unsigned char flags1 = m_returns|(m_need_target<<1)|(rotation_speed<<4)|
		((missile_speed == 4 ? 1 : 0)<<2);
	*ptr++ = flags1;
	int delay = missile_speed >= 3 ? 0 : (missile_speed == 2 ? 2 : 3);
	unsigned char flags2 = actor_frames|(delay<<5);
	*ptr++ = flags2;
	*ptr++ = powers;
	*ptr++ = 0;			// ??
	Write2(ptr, usecode);
					// BG:  Subtracted 1 from each sfx.
	int sfx_delta = bg ? -1 : 0;
	Write2(ptr, sfx - sfx_delta);
	Write2(ptr, hitsfx - sfx_delta);
					// Last 2 bytes unknown/unused.
	Write2(ptr, 0);
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out an ammo-info entry to 'ammo.dat'.
 */

void Ammo_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	uint8 buf[13];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	Write2(ptr, family_shape);
	Write2(ptr, sprite);
	*ptr++ = damage;
	unsigned char flags0;
	flags0 = (m_explodes<<6) | ((homing ? 3 : drop_type)<<4) | m_lucky |
				(m_autohit<<1) | (m_returns<<2) | (m_no_blocking<<3);
	*ptr++ = flags0;
	*ptr++ = 0;			// Unknown.
	*ptr++ = damage_type<<4;
	*ptr++ = powers;
	Write2(ptr, 0);			// Unknown.
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out an armor-info entry to 'armor.dat'.
 */

void Armor_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	uint8 buf[10];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	*ptr++ = prot;			// Protection value.
	*ptr++ = 0;			// Unknown.
	*ptr++ = immune;		// Immunity flags.
	Write4(ptr, 0);			// Last 5 are unknown/unused.
	*ptr = 0;
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out monster info. to 'monsters.dat'.
 */

void Monster_info::write
	(
	ostream& out,		// Output stream.
	int shapenum,		// Shape number.
	bool bg				// Writing BG file.
	)
	{
	uint8 buf[25];		// Entry length.
	memset(&buf[0], 0, sizeof(buf));
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	*ptr++ = (strength << 2) | (m_charm_safe ? 2 : 0) |
			(m_sleep_safe ? 1 : 0);
	*ptr++ = (dexterity << 2) | (m_paralysis_safe ? 2 : 0) |
			(m_curse_safe ? 1 : 0);
	*ptr++ = (intelligence << 2) | (m_int_b1 ? 2 : 0) |
			(m_poison_safe ? 1 : 0);
	*ptr++ = (combat << 2) | alignment;
	*ptr++ = (armor << 4) | (m_splits ? 1 : 0) | (m_cant_die ? 2 : 0) |
			 (m_power_safe ? 4 : 0) | (m_death_safe ? 8 : 0);
	*ptr++ = 0;			// Unknown.
	*ptr++ = (weapon << 4) | reach;
	*ptr++ = flags;			// Byte 9.
	*ptr++ = vulnerable;
	*ptr++ = immune;
	*ptr++ = (m_cant_yell ? (1<<5) : 0) |
		 (m_cant_bleed ? (1<<6) : 0);
	*ptr++ = m_byte13 | (m_attackmode+1);
	*ptr++ = equip_offset;
	*ptr++ = (m_can_teleport ? (1<<0) : 0) |
		 (m_can_summon ? (1<<1) : 0) |
		 (m_can_be_invisible ? (1<<2) : 0);
	*ptr++ = 0;			// Unknown.
	int sfx_delta = bg ? -1 : 0;
	*ptr++ = (signed char)(sfx&0xff - sfx_delta);
	out.write((char *) buf, sizeof(buf));
	}

