/**
 **	Shapeinf.h: Info. about shapes read from various data files.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifndef INCL_SHAPEINF
#define INCL_SHAPEINF	1

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

class Armor_info;
class Weapon_info;
class Ammo_info;
class Monster_info;
class SFX_info;
class Animation_info;
class Explosion_info;
class Body_info;
class Paperdoll_npc;
class Paperdoll_item;
class Effective_hp_info;
class Frame_name_info;
class Frame_powers_info;
class Warmth_info;
class Content_rules;
class Shapes_vga_file;

#include <iosfwd>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include "baseinf.h"
#include "utils.h"

template <int flag, class Functor>
class Functor_data_reader;
class Readytype_reader_functor;
class Altreadytype_reader_functor;
class Actor_flags_reader;
template <typename T, T *Shape_info::*data>
class Class_data_reader;
template <typename T, std::vector<T> Shape_info::*data>
class Vector_data_reader;
template <int flag, class Functor>
class Functor_data_writer;
class Readytype_writer_functor;
class Altreadytype_writer_functor;
template <typename T, T *Shape_info::*data>
class Class_data_writer;
template <typename T, std::vector<T> Shape_info::*data>
class Vector_data_writer;

enum Data_flag_names
	{
	ready_type_flag = 1,
	container_gump_flag = 2,
	monster_food_flag = 4,
	actor_flags_flag = 8,
	mountain_top_flag = 0x10,
	usecode_events_flag = 0x20,
	is_body_flag = 0x40,
	lightweight_flag = 0x80,
	quantity_frames_flag = 0x100,
	is_locked_flag = 0x200,
	is_volatile_flag = 0x400,
	altready_type_flag = 0x800,
	barge_type_flag = 0x1000,
	};

/*
 *	This class contains information only about shapes from "shapes.vga".
 */
class Shape_info
	{
	unsigned char tfa[3];		// From "tfa.dat".+++++Keep for
					//   debugging, for now.
					// 3D dimensions in tiles:
	unsigned char dims[3];		//   (x, y, z)
	unsigned char weight, volume;	// From "wgtvol.dat".
	unsigned char shpdims[2];	// From "shpdims.dat".
	char ready_type;	// From "ready.dat": where item can be worn.
	char alt_ready1;	// Alternate spot where item can be worn.
	char alt_ready2;	// Second alternate spot where item can be worn.
	bool spell_flag;		// Flagged as epsll in 'ready.dat'.
	bool occludes_flag;		// Flagged in 'occlude.dat'.  Roof.
	unsigned char *weapon_offsets;	// From "wihh.dat": pixel offsets
					//   for drawing weapon in hand
	Armor_info *armor;		// From armor.dat.
	Weapon_info *weapon;		// From weapon.dat, if a weapon.
	Ammo_info *ammo;		// From ammo.dat, if ammo.
	Monster_info *monstinf;		// From monster.dat.
	SFX_info *sfxinf;
	Animation_info *aniinf;
	Explosion_info *explosion;
	Body_info *body;
	Paperdoll_npc *npcpaperdoll;
	// These vectors should be totally ordered by the strict-weak
	// order operator defined for the classes.
	std::vector<Paperdoll_item> objpaperdoll;
	std::vector<Effective_hp_info> hpinf;
	std::vector<Frame_name_info> nameinf;
	std::vector<Frame_powers_info> frpowerinf;
	std::vector<Warmth_info> warminf;
	std::vector<Content_rules> cntrules;
	short container_gump;		// From container.dat.
	short monster_food;
	short mountain_top;
	short barge_type;
	unsigned char actor_flags;
	unsigned char shape_flags;
		// For some non-class data (see Data_flag_names enum).
	unsigned short modified_flags;
	unsigned short frompatch_flags;
		// For class data (to indicate an invalid entry should
		// be written by ES).
	unsigned short have_static_flags;
	void set_tfa_data()		// Set fields from tfa.
		{
		dims[0] = 1 + (tfa[2]&7);
		dims[1] = 1 + ((tfa[2]>>3)&7);
		dims[2] = (tfa[0] >> 5);
		}
					// Set/clear tfa bit.
	void set_tfa(int i, int bit, bool tf)
		{ tfa[i] = tf ? (tfa[i]|(1<<bit)) : (tfa[i]&~(1<<bit)); }

	/*
	 *	Generic vector data handler routines.
	 *	They all assume that the template class has the following
	 *	operators defined:
	 *		(1) operator< (which must be a strict weak order)
	 *		(2) operator==
	 *		(3) operator!=
	 *		(4) operator= that sets modified flag if needed.
	 *	They also assume that the vector is totally ordered
	 *	with the operator< -- using these functions will ensure
	 *	that this is the case.
	 */
	template <typename T>
	static void add_vector_info(const T& inf, std::vector<T>& vec)
		{
		typename std::vector<T>::iterator it;
			// Find using operator<.
		it = std::lower_bound(vec.begin(), vec.end(), inf);
		if (it == vec.end() || *it != inf)	// Not found.
			vec.insert(it, inf);	// Add new.
		else	// Already exists.
			*it = inf;	// Replace information.
		}
	template <typename T>
	static void copy_vector_info(const std::vector<T>& from, std::vector<T>& to)
		{
		if (from.size())
			{
			to.resize(from.size());
			std::copy(from.begin(), from.end(), to.begin());
			}
		else
			to.clear();
		}
	template <typename T>
	static std::vector<T>& set_vector_info(bool tf, std::vector<T>& vec)
		{
		invalidate_vector(vec);
		if (!tf)
			clean_vector(vec);
		return vec;
		}
	template <typename T>
	static void invalidate_vector(std::vector<T>& vec)
		{
		typename std::vector<T>::iterator it;
		for (it = vec.begin(); it != vec.end(); ++it)
			it->invalidate();
		}

	template <typename T>
	static void clean_vector(std::vector<T>& vec)
		{
		unsigned int i = 0;
		while (i < vec.size())
			{
			typename std::vector<T>::iterator it = vec.begin() + i;
			if (!it->is_invalid() || it->have_static())
				i++;
			else
				vec.erase(it);
			}
		}
	// Generic data handler routine.
	template <typename T>
	static T *set_info(bool tf, T *&pt)
		{
		if (!tf)
			{
			delete pt;
			pt = 0;
			}
		else if (!pt)
			pt = new T();
		return pt;
		}
public:
	enum Actor_flags
		{
		cold_immune = 0,
		doesnt_eat,
		teleports,
		summons,
		turns_invisible,
		armageddon_safe,
		quake_walk
		};
	enum Shape_flags
		{
		usecode_events = 0,
		is_body,
		lightweight,
		quantity_frames,
		is_locked,
		is_volatile
		};
	enum Mountain_tops
		{
		not_mountain_top = 0,
		normal_mountain_top,
		snow_mountain_top
		};
	enum Barge_types
		{
		barge_generic = 0,
		barge_raft,
		barge_seat,
		barge_sails,
		barge_wheel,
		barge_draftanimal,
		barge_turtle
		};
	friend class Shapes_vga_file;	// Class that reads in data.
	template <int flag, class Functor>
	friend class Functor_data_reader;
	friend class Readytype_reader_functor;
	friend class Altreadytype_reader_functor;
	friend class Actor_flags_reader;
	template <typename T, T *Shape_info::*data>
	friend class Class_data_reader;
	template <typename T, std::vector<T> Shape_info::*data>
	friend class Vector_data_reader;
	template <int flag, class Functor>
	friend class Functor_data_writer;
	friend class Readytype_writer_functor;
	friend class Altreadytype_writer_functor;
	template <typename T, T *Shape_info::*data>
	friend class Class_data_writer;
	template <typename T, std::vector<T> Shape_info::*data>
	friend class Vector_data_writer;
	Shape_info();
	// This copy constructor and assignment operator intentionally cause
	// errors.
	Shape_info(const Shape_info & other);
	const Shape_info & operator = (const Shape_info & other);
	virtual ~Shape_info();
	void copy(const Shape_info& inf2, bool skip_dolls = false);

	int get_weight() const		// Get weight, volume.
		{ return weight; }
	int get_volume() const
		{ return volume; }
	void set_weight_volume(int w, int v)
		{ weight = w; volume = v; }

	int get_armor() const;
	int get_armor_immunity() const;

	int get_explosion_sprite() const;
	int get_explosion_sfx() const;

	int get_body_shape() const;
	int get_body_frame() const;

	bool has_weapon_info() const
		{ return weapon != 0; }
	Weapon_info *get_weapon_info_safe() const;
	Weapon_info *get_weapon_info() const
		{ return weapon; }
	Weapon_info *set_weapon_info(bool tf);

	bool has_ammo_info() const
		{ return ammo != 0; }
	Ammo_info *get_ammo_info_safe() const;
	Ammo_info *get_ammo_info() const
		{ return ammo; }
	Ammo_info *set_ammo_info(bool tf);

	bool has_armor_info() const
		{ return armor != 0; }
	Armor_info *get_armor_info() const
		{ return armor; }
	Armor_info *set_armor_info(bool tf);

	bool has_monster_info() const
		{ return monstinf != 0; }
	Monster_info *get_monster_info_safe() const;
	Monster_info *get_monster_info() const
		{ return monstinf; }
	Monster_info *set_monster_info(bool tf);

	bool has_npc_paperdoll_info() const
		{ return npcpaperdoll != 0; }
	Paperdoll_npc *get_npc_paperdoll() const
		{ return npcpaperdoll; }
	Paperdoll_npc *set_npc_paperdoll_info(bool tf);
	Paperdoll_npc *get_npc_paperdoll_safe(bool sex) const;

	bool has_sfx_info() const
		{ return sfxinf != 0; }
	SFX_info *get_sfx_info() const
		{ return sfxinf; }
	SFX_info *set_sfx_info(bool tf);

	bool has_explosion_info() const
		{ return explosion != 0; }
	Explosion_info *get_explosion_info() const
		{ return explosion; }
	Explosion_info *set_explosion_info(bool tf);

	bool has_animation_info() const
		{ return aniinf != 0; }
	Animation_info *get_animation_info() const
		{ return aniinf; }
	Animation_info *get_animation_info_safe(int shnum, int nframes);
	Animation_info *set_animation_info(bool tf);

	bool has_body_info() const
		{ return body != 0; }
	Body_info *get_body_info() const
		{ return body; }
	Body_info *set_body_info(bool tf);

	bool has_paperdoll_info() const;
	std::vector<Paperdoll_item>& get_paperdoll_info()
		{ return objpaperdoll; }
	std::vector<Paperdoll_item>& set_paperdoll_info(bool tf);
	void clean_invalid_paperdolls();
	void clear_paperdoll_info();
	void add_paperdoll_info(Paperdoll_item& add);
	Paperdoll_item *get_item_paperdoll(int frame, int spot);
	bool is_object_allowed(int frame, int spot)
		{ return get_item_paperdoll(frame, spot) != 0; }

	bool has_content_rules() const;
	std::vector<Content_rules>& get_content_rules()
		{ return cntrules; }
	std::vector<Content_rules>& set_content_rules(bool tf);
	void clean_invalid_content_rules();
	void clear_content_rules();
	void add_content_rule(Content_rules& add);
	bool is_shape_accepted(int shape);

	bool has_effective_hp_info() const;
	std::vector<Effective_hp_info>& get_effective_hp_info()
		{ return hpinf; }
	std::vector<Effective_hp_info>& set_effective_hp_info(bool tf);
	void clean_invalid_hp_info();
	void clear_effective_hp_info();
	void add_effective_hp_info(Effective_hp_info& add);
	int get_effective_hps(int frame, int quality);

	bool has_frame_name_info() const;
	std::vector<Frame_name_info>& get_frame_name_info()
		{ return nameinf; }
	std::vector<Frame_name_info>& set_frame_name_info(bool tf);
	void clean_invalid_name_info();
	void clear_frame_name_info();
	void add_frame_name_info(Frame_name_info& add);
	Frame_name_info *get_frame_name(int frame, int quality);

	bool has_frame_powers() const;
	std::vector<Frame_powers_info>& get_frame_powers()
		{ return frpowerinf; }
	std::vector<Frame_powers_info>& set_frame_powers(bool tf);
	void clean_invalid_frame_powers();
	void clear_frame_powers();
	void add_frame_powers(Frame_powers_info& add);
	int get_object_powers(int frame);
	int has_object_power(int frame, int p)
		{ return (get_object_powers(frame)&(1 << p)) != 0; }

	bool has_warmth_info() const;
	std::vector<Warmth_info>& get_warmth_info()
		{ return warminf; }
	std::vector<Warmth_info>& set_warmth_info(bool tf);
	void clean_invalid_warmth_info();
	void clear_warmth_info();
	void add_warmth_info(Warmth_info& add);
	int get_object_warmth(int frame);

	int get_monster_food() const
		{ return monster_food; }
	void set_monster_food(int sh)
		{
		if (monster_food != (short)sh)
			{
			modified_flags |= monster_food_flag;
			monster_food = (short)sh;
			}
		}

	int get_mountain_top_type() const
		{ return mountain_top; }
	void set_mountain_top(int sh)
		{
		if (mountain_top != (short)sh)
			{
			modified_flags |= mountain_top_flag;
			mountain_top = (short)sh;
			}
		}

	int get_barge_type() const
		{ return barge_type; }
	void set_barge_type(int sh)
		{
		if (barge_type != (short)sh)
			{
			modified_flags |= barge_type_flag;
			barge_type = (short)sh;
			}
		}

	int get_container_gump() const
		{ return container_gump; }
	void set_container_gump(int sh)
		{
		if (container_gump != (short)sh)
			{
			modified_flags |= container_gump_flag;
			container_gump = (short) sh;
			}
		}

	unsigned char get_shape_flags() const
		{ return shape_flags; }
	void set_shape_flags(char flags)
		{
		if (shape_flags != flags)
			{
			int diff = (shape_flags ^ flags) * usecode_events_flag;
			modified_flags |= diff;
			shape_flags = flags;
			}
		}
	bool get_shape_flag(int tf) const
		{ return (shape_flags & (1 << tf)) != 0; }
	void set_shape_flag(int tf, int mod)
		{
		if (!(shape_flags & (1 << tf)))
			{
			modified_flags |= (1 << mod);
			shape_flags |= (1 << tf);
			}
		}
	void clear_shape_flag(int tf, int mod)
		{
		if (shape_flags & (1 << tf))
			{
			modified_flags |= (1 << mod);
			shape_flags &= ~(1 << tf);
			}
		}

	bool has_usecode_events() const
		{ return get_shape_flag(usecode_events); }
	bool is_body_shape() const
		{ return get_shape_flag(is_body); }
	bool is_lightweight() const
		{ return get_shape_flag(lightweight); }
	bool has_quantity_frames() const
		{ return get_shape_flag(quantity_frames); }
	bool is_container_locked() const
		{ return get_shape_flag(is_locked); }
	bool is_explosive() const
		{ return get_shape_flag(is_volatile); }

	unsigned char get_actor_flags() const
		{ return actor_flags; }
	void set_actor_flags(char flags)
		{
		if (actor_flags != flags)
			{
			modified_flags |= actor_flags_flag;
			actor_flags = flags;
			}
		}
	bool get_actor_flag(int tf) const
		{ return (actor_flags & (1 << tf)) != 0; }
	void set_actor_flag(int tf)
		{
		if (!(actor_flags & (1 << tf)))
			{
			modified_flags |= actor_flags_flag;
			actor_flags |= (1 << tf);
			}
		}
	void clear_actor_flag(int tf)
		{
		if (actor_flags & (1 << tf))
			{
			modified_flags |= actor_flags_flag;
			actor_flags &= ~(1 << tf);
			}
		}
	
	bool is_cold_immune() const
		{ return get_actor_flag(cold_immune); }
	bool does_not_eat() const
		{ return get_actor_flag(doesnt_eat); }
	bool can_teleport() const
		{ return get_actor_flag(teleports); }
	bool can_summon() const
		{ return get_actor_flag(summons); }
	bool can_be_invisible() const
		{ return get_actor_flag(turns_invisible); }
	bool survives_armageddon() const
		{ return get_actor_flag(armageddon_safe); }
	bool quake_on_walk() const
		{ return get_actor_flag(quake_walk); }
					// Get tile dims., flipped for
					//   reflected (bit 5) frames.
	int get_3d_xtiles(unsigned int framenum)
		{ return dims[(framenum >> 5)&1]; }
	int get_3d_ytiles(unsigned int framenum)
		{ return dims[1 ^ ((framenum >> 5)&1)]; }
	int get_3d_height()		// Height (in lifts?).
		{ return dims[2]; }
	void set_3d(int xt, int yt, int zt);
	unsigned char get_tfa(int i)	// For debugging:
		{ return tfa[i]; }
	int has_sfx()			// Has a sound effect (guessing).
		{ return (tfa[0] & (1<<0)) != 0; }
	void set_sfx(bool tf)
		{ set_tfa(0, 0, tf); }
	int has_strange_movement()	// Slimes, sea monsters.
		{ return (tfa[0] & (1<<1)) != 0; }
	void set_strange_movement(bool tf)
		{ set_tfa(0, 1, tf); }
	int is_animated()
		{ return (tfa[0] & (1<<2)) != 0; }
	void set_animated(bool tf)
		{ set_tfa(0, 2, tf); }
	int is_solid()			// Guessing.  Means can't walk through.
		{ return (tfa[0] & (1<<3)) != 0; }
	void set_solid(bool tf)
		{ set_tfa(0, 3, tf); }
	int is_water()			// Guessing.
		{ return (tfa[0] & (1<<4)) != 0; }
	void set_water(bool tf)
		{ set_tfa(0, 4, tf); }
	int is_poisonous()		// Swamps.  Applies to tiles.
		{ return (tfa[1] & (1<<4)) != 0; }
	int is_field()			// Applies to Game_objects??
		{ return (tfa[1] & (1<<4)) != 0; }
	void set_field(bool tf)
		{ set_tfa(1, 4, tf); }
	int is_door()
		{ return (tfa[1] & (1<<5)) != 0; }
	void set_door(bool tf)
		{ set_tfa(1, 5, tf); }
	int is_barge_part()
		{ return (tfa[1] & (1<<6)) != 0; }
	void set_barge_part(bool tf)
		{ set_tfa(1, 6, tf); }
	int is_transparent()		// ??
		{ return (tfa[1] & (1<<7)) != 0; }
	void set_transparent(bool tf)
		{ set_tfa(1, 7, tf); }
	int is_light_source()
		{ return (tfa[2] & (1<<6)) != 0; }
	void set_light_source(bool tf)
		{ set_tfa(2, 6, tf); }
	int has_translucency()
		{ return (tfa[2] & (1<<7)) != 0; }
	void set_translucency(bool tf)
		{ set_tfa(2, 7, tf); }
	int is_xobstacle()		// Obstacle in x-dir.???
		{ return (shpdims[1] & 1) != 0; }
	int is_yobstacle()		// Obstacle in y-dir.???
		{ return (shpdims[0] & 1) != 0; }
	void set_obstacle(bool x, bool y)
		{
		shpdims[1] = x ? (shpdims[1]|1) : (shpdims[1]&~1);
		shpdims[0] = y ? (shpdims[0]|1) : (shpdims[0]&~1);
		}
	/*
	 *	TFA[1][b0-b3] seems to indicate object types:
	 */
	enum Shape_class {
		unusable = 0,		// Trees.
		quality = 2,
		quantity = 3,		// Can have more than 1:  coins, arrs.
		has_hp = 4,	    // Breakable items (if hp != 0, that is)
		quality_flags = 5,	// Item quality is set of flags:
					// Bit 3 = okay-to-take.
		container = 6,
		hatchable = 7,		// Eggs, traps, moongates.
		spellbook = 8,
		barge = 9,
		virtue_stone = 11,
		monster = 12,		// Non-human's.
		human = 13,		// Human NPC's.
		building = 14		// Roof, window, mountain.
		};
	Shape_class get_shape_class()
		{ return (Shape_class) (tfa[1]&15); }
	void set_shape_class(Shape_class c)
		{ tfa[1] = (tfa[1]&~15)|(int) c; }
	bool is_npc()
		{
		Shape_class c = get_shape_class();
		return c == human || c == monster;
		}
	bool has_quantity()
		{ return get_shape_class() == quantity; }
	bool has_quality_flags()	// Might be more...
		{ return get_shape_class() == quality_flags; }
	bool has_quality()
		{
#if 0
		static bool qual[16] = 	// Ugly, but quick.
		//			quality
		      { false,	false,	true,	false,	false, 	false,
		//	ctainer	egg				virtue stone
			true,	true,	false,	false,	false,	true,
		//	monst	human
			true,	true,	false,	false };
#endif
		Shape_class c = get_shape_class();
		return (c == 2 || c == 6 || c == 7 || c == 11 || c == 12 || c == 13);
		//		return qual[(int) c];
		}
	bool occludes() const
		{ return occludes_flag; }
	void set_occludes(bool tf)
		{ occludes_flag = tf; }
	char get_ready_type()
		{ return ready_type; }
	void set_ready_type(char t)
		{
		if (ready_type != t)
			{
			modified_flags |= ready_type_flag;
			ready_type = t;
			}
		}
	bool is_spell()
		{ return spell_flag; }
	void set_is_spell(bool tf)
		{
		if (spell_flag != tf)
			{
			modified_flags |= ready_type_flag;
			spell_flag = tf;
			}
		}
	char get_alt_ready1()
		{ return alt_ready1; }
	char get_alt_ready2()
		{ return alt_ready2; }
	void set_alt_ready(unsigned char t1, unsigned char t2)
		{
		if (alt_ready1 != t1 || alt_ready2 != t2)
			{
			modified_flags |= altready_type_flag;
			alt_ready1 = t1;
			alt_ready2 = t2;
			}
		}
	// Sets x to 255 if there is no weapon offset
	void get_weapon_offset(int frame, unsigned char& x, unsigned char& y)
		{
		if(!weapon_offsets)
			x = 255;
		else
			{
			// x could be 255 (see read_info())
			x = weapon_offsets[frame * 2];
			y = weapon_offsets[frame * 2 + 1];
			}
		}
	void set_weapon_offset(int frame, unsigned char x, unsigned char y);
	int get_rotated_frame(int curframe, int quads);
	};

#endif
