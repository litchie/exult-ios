/**
 **	Monstinf.h - Information (about NPC's, really) from 'monster.dat'.
 **
 **	Written: 8/13/01 - JSF
 **/

#ifndef INCL_MONSTINF
#define INCL_MONSTINF	1

/*
Copyright (C) 2000-2001 The Exult Team

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

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

#include <cassert>

/*
 *	An element from 'equip.dat', describing a monster's equipment:
 */
class Equip_element
	{
	unsigned short shapenum;	// What to create, or 0 for nothing.
	unsigned char probability;	// 0-100:  probabilit of creation.
	unsigned char quantity;		// # to create.
public:
	friend class Monster_info;
	friend class Monster_actor;
	Equip_element()
		{  }
	void set(int shnum, int prob, int quant)
		{
		shapenum = shnum;
		probability = prob;
		quantity = quant;
		}
	int get_shapenum() const
		{ return shapenum; }
	int get_probability() const
		{ return probability; }
	int get_quantity() const
		{ return quantity; }
	};

/*
 *	A record from 'equip.dat' consists of 10 elements.
 */
class Equip_record
	{
	Equip_element elements[10];
public:
	friend class Monster_info;
	friend class Monster_actor;
	Equip_record()
		{  }
					// Set i'th element.
	void set(int i, int shnum, int prob, int quant)
		{ elements[i].set(shnum, prob, quant); }
	Equip_element& get(int i)
		{ assert(i >= 0 && i < 10); return elements[i]; }
	};

/*
 *	Monster info. from 'monsters.dat':
 */
class Monster_info
	{
	static Equip_record *equip;	// ->equipment info.
	static int equip_cnt;		// # entries in equip.
	static Monster_info default_info;	// For shapes not found.
	unsigned char strength;		// Attributes.
	unsigned char dexterity;
	unsigned char intelligence;
	unsigned char alignment;	// Default alignment.
	unsigned char combat;
	unsigned char armor;		// These are unarmed stats.
	unsigned char weapon;
	unsigned char reach;
	unsigned char flags;		// Defined below.
					// The following are bits corresponding
					//   to Weapon_info::Damage_type.
	unsigned char vulnerable, immune;
	unsigned char equip_offset;	// Offset in 'equip.dat' (1 based;
					//   if 0, there's none.)
	bool m_splits;			// For slimes.
	bool m_cant_die;
	bool m_cant_yell;		// Can't yell during combat.
	bool m_cant_bleed;
	bool m_poison_safe;		// Can't be poisoned.
public:
	friend class Monster_actor;
	Monster_info() {  }
	int read(std::istream& mfile);	// Read in from file.
					// Write out.
	void write(int shapenum, std::ostream& mfile);
	static const Monster_info *get_default();
					// Done by Game_window:
	static void set_equip(Equip_record *eq, int cnt)
		{
		equip = eq;
		equip_cnt = cnt;
		}
	static int get_equip_cnt()
		{ return equip_cnt; }
	static Equip_record& get_equip(int i)
		{ assert(i >= 0 && i < equip_cnt); return equip[i]; }
	bool splits() const
		{ return m_splits; }
	void set_splits(bool tf)
		{ m_splits = tf; }
	bool cant_die() const
		{ return m_cant_die; }
	void set_cant_die(bool tf)
		{ m_cant_die = tf; }
	bool cant_yell() const
		{ return m_cant_yell; }
	void set_cant_yell(bool tf)
		{ m_cant_yell = tf; }
	bool cant_bleed() const
		{ return m_cant_bleed; }
	void set_cant_bleed(bool tf)
		{ m_cant_bleed = tf; }
	bool poison_safe() const
		{ return m_poison_safe; }
	void set_poison_safe(bool tf)
		{ m_poison_safe = tf; }
					// Get bits indicating
					//   Weapon_info::damage_type:
	unsigned char get_vulnerable() const
		{ return vulnerable; }
	void set_vulnerable(unsigned char v)
		{ vulnerable = v; }
	unsigned char get_immune() const
		{ return immune; }
	void set_immune(unsigned char v)
		{ immune = v; }
	enum Flags {
		fly = 0,
		swim = 1,
		walk = 2,
		ethereal = 3,		// Can walk through walls.
		no_body = 4		// Don't create body.
					// 5:  gazer, hook only.
//Don't think so magic_only = 7,	// Can only be hurt by magic weapons.
					// 8:  bat only.
//		slow = 9		// E.g., slime, corpser.
					// 10:  skeleton only.
		};
	unsigned char get_flags() const	// Get above set of flags.
		{ return flags; }
	void set_flags(unsigned char f)
		{ flags = f; }
	bool has_no_body() const	// No dead body?
		{ return (flags>>no_body)&1; }
	int get_strength() const	
		{ return strength; }
	int get_dexterity() const
		{ return dexterity; }
	int get_intelligence() const
		{ return intelligence; }
	int get_alignment() const
		{ return alignment; }
	void set_alignment(int a)
		{ alignment = a; }
	int get_combat() const
		{ return combat; }
	int get_armor() const
		{ return armor; }
	int get_weapon() const
		{ return weapon; }
	int get_reach() const
		{ return reach; }
	void set_stats(int str, int dex, int intel, int cmb, int armour,
			int wpn, int rch);
	int get_equip_offset() const
		{ return equip_offset; }
	void set_equip_offset(int o)
		{ equip_offset = o; }
	};



#endif
