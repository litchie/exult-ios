/**
 **	Shapeinf.h: Info. about shapes read from various 'static' data files.
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

class Monster_info;

#include <iosfwd>

/*
 *	Specific information about weapons from 'weapons.dat':
 *	MAYBE:  Move this and ammo. to separate source file(s).
 */
class Weapon_info
	{
	char damage;			// Damage points (positive).
public:
	enum Powers			// Special weapon powers.
		{			// Guesses from printout:
		sleep = 1,
		charm = 2,
		curse = 4,
		poison = 8,
		paralyze = 16,
		magebane = 32		// Takes away mana.
		};
	enum Damage_type		// Type of damage.  These also are the
					//   bit #'s in Monster_info's 
					//   immune and vulerable fields.
		{
		normal_damage = 0,
		fire_damage = 1,
		magic_damage = 2,
		poison_damage = 3	// Not sure about rest.
		};
private:
	unsigned char powers;		// Poison, sleep, charm. flags.
	unsigned char damage_type;	// See Damage_type above.
	short ammo;			// Shape # of ammo. consumed, or
					//   -1 = ?? (swords, also sling).
					//   -2 = ?? wands?
					//   -3 = throw weapon itself.
	short projectile;		// Projectile shape, or 0.
	bool m_explodes;		// Projectile explodes on impact.
	bool m_returns;			// Boomerang, magic axe.
	short usecode;			// Usecode function, or 0.
	unsigned char uses;		// 0 = hand-hand, 1,2 = throwable,
					//   3 = missile-firing.
	unsigned char range;		// Distance weapon can be used.
	short sfx, hitsfx;		// Sound when using/hit, or -1.
public:
	friend class Shape_info;
	Weapon_info() {  }
	int read(std::istream& mfile, bool bg);	// Read in from file.
	int get_damage() const
		{ return damage; }
	int get_damage_type() const
		{ return damage_type; }
	unsigned char get_powers() const
		{ return powers; }
	int get_ammo_consumed()
		{ return ammo > 0 ? ammo : 0; }
	bool is_thrown() const
		{ return uses == 1 || uses == 2 || m_returns; }
	bool returns() const
		{ return m_returns; }
	int get_striking_range()	// Guessing about div. by 2.
		{ return uses < 3 ? range/2 : 0; }
	int get_projectile_range()	// +++Guess for thrown weapons.
		{ return uses == 3 ? range : is_thrown() ? 20 : 0; }
	int get_projectile()
		{ return projectile; }
	int get_usecode()
		{ return usecode; }
	int get_sfx()			// Return sound-effects #, or -1.
		{ return sfx; }
	int get_hitsfx()
		{ return hitsfx; }
	};

/*
 *	Info. from 'ammo.dat':
 */
class Ammo_info
	{
	static class Ammo_table *table;	// For looking up by shape #.
	int shapenum;			// Ammo's shape.
	int family_shape;		// I.e., burst-arrow's is 'arrow'.
	unsigned char damage;		// Extra damage points.
	unsigned char powers;		// Same as for weapons.
	unsigned char damage_type;	// Same as for weapons.
//	unsigned char unknown[6];	// ??
public:
	friend class Shapes_vga_file;
	Ammo_info() : shapenum(0)
		{  }
	int read(std::istream& mfile);	// Read in from file.
	int get_shapenum()
		{ return shapenum; }
	int get_family_shape()
		{ return family_shape; }
	int get_damage()
		{ return damage; }
	unsigned char get_powers() const
		{ return powers; }
	int get_damage_type() const
		{ return damage_type; }
	static void create();		// Create table.
	static void insert(int shnum, Ammo_info& ent);
	static Ammo_info *find(int shnum);// Find given ammo's entry.
					// Is given shape in desired family.
	static int is_in_family(int shnum, int family)
		{
		Ammo_info *ainf;
		return (shnum == family) ||
			((ainf = find(shnum)) != 0 && 
					ainf->family_shape == family);
		}
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
	unsigned char ready_type;	// From "ready.dat":  where item can
					//   be worn.
	bool occludes_flag;		// Flagged in 'occlude.dat'.  Roof.
	unsigned char *weapon_offsets;	// From "wihh.dat": pixel offsets
					//   for drawing weapon in hand
	unsigned char armor;		// Armor, from armor.dat.
	Weapon_info *weapon;		// From weapon.dat, if a weapon.
	Monster_info *monstinf;		// From monster.dat.
	void set_tfa_data()		// Set fields from tfa.
		{
		dims[0] = 1 + (tfa[2]&7);
		dims[1] = 1 + ((tfa[2]>>3)&7);
		dims[2] = (tfa[0] >> 5);
		}
	// This private copy constructor and assignment operator are never
	// defined so copying will cause a link error (intentional)
	Shape_info(const Shape_info & other);
	const Shape_info & operator = (const Shape_info & other);
public:
	friend class Shapes_vga_file;	// Class that reads in data.
	Shape_info() : weight(0), volume(0),
		ready_type(0), weapon_offsets(0), armor(0), weapon(0),
		monstinf(0), occludes_flag(false)
		{
		tfa[0] = tfa[1] = tfa[2] = shpdims[0] = shpdims[1] = 0;
		dims[0] = dims[1] = dims[2] = 0;
		}
	virtual ~Shape_info();
	int get_weight()		// Get weight, volume.
		{ return weight; }
	int get_volume()
		{ return volume; }
	int get_armor()			// Get armor protection.
		{ return armor; }
	Weapon_info *get_weapon_info()
		{ return weapon; }
	Monster_info *get_monster_info()
		{ return monstinf; }
					// Get tile dims., flipped for
					//   reflected (bit 5) frames.
	int get_3d_xtiles(unsigned int framenum = 0)
		{ return dims[(framenum >> 5)&1]; }
	int get_3d_ytiles(unsigned int framenum = 0)
		{ return dims[1 ^ ((framenum >> 5)&1)]; }
	int get_3d_height()		// Height (in lifts?).
		{ return dims[2]; }
	unsigned char get_tfa(int i)	// For debugging:
		{ return tfa[i]; }
	int has_sfx()			// Has a sound effect (guessing).
		{ return (tfa[0] & (1<<0)) != 0; }
	int has_strange_movement()	// Slimes, sea monsters.
		{ return (tfa[0] & (1<<1)) != 0; }
	int is_animated()
		{ return (tfa[0] & (1<<2)) != 0; }
	int is_solid()			// Guessing.  Means can't walk through.
		{ return (tfa[0] & (1<<3)) != 0; }
	int is_water()			// Guessing.
		{ return (tfa[0] & (1<<4)) != 0; }
	int is_poisonous()		// Swamps.  Applies to tiles.
		{ return (tfa[1] & (1<<4)) != 0; }
	int is_field()			// Applies to Game_objects??
		{ return (tfa[1] & (1<<4)) != 0; }
	int is_door()
		{ return (tfa[1] & (1<<5)) != 0; }
	int is_barge_part()
		{ return (tfa[1] & (1<<6)) != 0; }
	int is_transparent()		// ??
		{ return (tfa[1] & (1<<7)) != 0; }
	int is_light_source()
		{ return (tfa[2] & (1<<6)) != 0; }
	int has_translucency()
		{ return (tfa[2] & (1<<7)) != 0; }
	int is_xobstacle()		// Obstacle in x-dir.???
		{ return (shpdims[1] & 1) != 0; }
	int is_yobstacle()		// Obstacle in y-dir.???
		{ return (shpdims[0] & 1) != 0; }
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
	unsigned char get_ready_type()
		{ return ready_type; }
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
	};

#endif
