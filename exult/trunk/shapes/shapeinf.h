/**	-*-mode: Fundamental; tab-width: 8; -*-
**
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

/*
 *	Specific information about weapons from 'weapons.dat':
 *	MAYBE:  Move this and ammo. to separate source file(s).
 */
class Weapon_info
	{
	char damage;			// Damage points (positive).
	unsigned char special_atts;	// Poison, sleep, charm. flags.
	short ammo;			// Shape # of ammo. consumed, or
					//   -1 = ?? (swords, also sling).
					//   -2 = ?? wands?
					//   -3 = throw weapon itself.
	short projectile;		// Projectile shape, or 0.
	short usecode;			// Usecode function, or 0.
	unsigned char range1;		// 1st possible range (striking for
					//   weapons with 2 uses; i.e.,
					//   daggers.
	unsigned char range2;		// Throwing range for i.e., daggers.
public:
	friend class Shape_info;
	Weapon_info(char d, unsigned char r1, unsigned char r2,
			unsigned char sp, short am, short pr, short uc) 
		: damage(d),  special_atts(sp), ammo(am), projectile(pr),
	          usecode(uc), range1(r1), range2(r2)
		{  }
	int get_damage()
		{ return damage; }
	unsigned char get_special_atts()// Test for special damage.
		{ return special_atts; }
	int get_ammo_consumed()
		{ return ammo > 0 ? ammo : 0; }
	int is_thrown()
		{ return ammo == -3 && (range2&7) != 0; }
	int get_striking_range()
		{ return range1 < 3 ? range1 : 0; }
	int get_projectile_range()
		{ return range1 >= 3 ? (range1 + 3) 
			: (is_thrown() ? ((range2&7) + 8 + 3) : 0); }
	int get_projectile()
		{ return projectile; }
	int get_usecode()
		{ return usecode; }
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
//	unsigned char unknown[6];	// ??
public:
	friend class Shapes_vga_file;
	Ammo_info() : shapenum(0)
		{  }
	Ammo_info(int shnum, int family, unsigned char dmge)
		{
		shapenum = shnum;
		family_shape = family;
		damage = dmge;
		}
	int get_shapenum()
		{ return shapenum; }
	int get_family_shape()
		{ return family_shape; }
	int get_damage()
		{ return damage; }
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
	unsigned char *weapon_offsets;	// From "wihh.dat": pixel offsets
					//   for drawing weapon in hand
	unsigned char armor;		// Armor, from armor.dat.
	Weapon_info *weapon;		// From weapon.dat, if a weapon.
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
		ready_type(0), weapon_offsets(0), armor(0), weapon(0)
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
	 *	+++++++Just guessing for now.
	 */
	enum Shape_class {
		unusable = 0,		// Trees.
		has_quality = 2,
		has_quantity = 3,	// Can have more than 1:  coins, arrs.
//		shutters = 4,		// Also mirrors.
//		wearable = 5,		// Includes wieldable weapons, food,
					//   table, curtain??
		container = 6,		// Includes NPC's.
		hatchable = 7,		// Eggs, traps, moongates.
		spellbook = 8,
		barge = 9,
		virtue_stone = 11,
					// 12, 13 for some NPC's.
		building = 14		// Roof, window, mountain.
		};
	Shape_class get_shape_class()
		{ return (Shape_class) (tfa[1]&15); }
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
