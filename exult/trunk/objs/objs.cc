/*
 *	objs.cc - Game objects.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2002  The Exult Team
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

#include "objs.h"
#include "chunks.h"
#include "objiter.h"
#include "egg.h"
#include "gamewin.h"
#include "gamemap.h"
#include "actors.h"
#include "ucmachine.h"
#include "items.h"
#include "dir.h"
#include "ordinfo.h"
#include "game.h"
#include "Gump_manager.h"
#include "effects.h"
#include "databuf.h"
#include "animate.h"
#include "Audio.h"
#include "ready.h"
#include "ammoinf.h"
#include "weaponinf.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#  include <cstdio>
#endif
#include <algorithm>       // STL function things

#ifdef USE_EXULTSTUDIO
#include "cheat.h"
#include "server.h"
#include "objserial.h"
#include "servemsg.h"
#endif

#ifndef UNDER_EMBEDDED_CE
using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::memset;
using std::rand;
using std::ostream;
using std::strchr;
using std::string;
using std::set;
#endif

/*
 *	Figure attack points against an object, and also run weapon's usecode.
 */

inline int Game_object::get_usecode() const
	{
	return ucmachine->get_shape_fun(get_shapenum());
	}


					// Offset to each neighbor, dir=0-7.
short Tile_coord::neighbors[16] = {0,-1, 1,-1, 1,0, 1,1, 0,1,
							-1,1, -1,0, -1,-1 };
Game_object *Game_object::editing = 0;
					// Bit 5=S, Bit6=reflect. on diag.
unsigned char Game_object::rotate[8] = { 0, 0, 48, 48, 16, 16, 32, 32};

extern bool combat_trace;

/*
 *	Get chunk coords, or 255.
 */
int Game_object::get_cx() const
	{ return chunk ? chunk->cx : 255; }
int Game_object::get_cy() const
	{ return chunk ? chunk->cy : 255; }


Game_map* Game_object::get_map() const // Map we're on.
{
	return chunk ? chunk->get_map() : 0;
}
int Game_object::get_map_num() const // Get map number this is in.
{
	return chunk ? chunk->get_map()->get_num() : -1;
}

/*
 *	Get tile.
 */

Tile_coord Game_object::get_tile
	(
	) const
	{
	if (!chunk)
		{
#if DEBUG
		cout << "Asking tile for obj. " << get_shapenum()
				<< " not on map" << endl;
#endif
		return Tile_coord(255*c_tiles_per_chunk, 255*c_tiles_per_chunk,
								0);
		}
	return Tile_coord(chunk->cx*c_tiles_per_chunk + tx,
				chunk->cy*c_tiles_per_chunk + ty, lift);
	}

/*
 *	Get tile.
 */

Tile_coord Game_object::get_center_tile
	(
	) const
	{
	if (!chunk)
		{
#if DEBUG
		cout << "Asking center tile for obj. " << get_shapenum()
				<< " not on map" << endl;
#endif
		return Tile_coord(255*c_tiles_per_chunk, 255*c_tiles_per_chunk,
								0);
		}
	int frame = get_framenum();
	int dx = (get_info().get_3d_xtiles(frame)-1) >> 1,
	    dy = (get_info().get_3d_ytiles(frame)-1) >> 1,
		dz = (get_info().get_3d_height()*3)/4;
	int x = chunk->cx*c_tiles_per_chunk + tx - dx,
	    y = chunk->cy*c_tiles_per_chunk + ty - dy;
	return Tile_coord(x, y, lift + dz);
	}

Tile_coord Game_object::get_missile_tile
	(
	int dir
	) const
	{
	if (!chunk)
		{
#if DEBUG
		cout << "Asking missile tile for obj. " << get_shapenum()
				<< " not on map" << endl;
#endif
		return Tile_coord(255*c_tiles_per_chunk, 255*c_tiles_per_chunk,
								0);
		}
	int frame = get_framenum();
	int dx = get_info().get_3d_xtiles(frame)-1,
	    dy = get_info().get_3d_ytiles(frame)-1,
		dz = (get_info().get_3d_height()*3)/4;
	/*switch (dir)
		{
		case south:
			dy = -1;
		case north:
			dx /=2; break;
		case east:
			dx = -1;
		case west:
			dy /= 2; break;
		case southeast:
			dy = -1;
		case northeast:
			dx = -1; break;
		case southwest:
			dy = -1;
			break;
		}*/
	int x = chunk->cx*c_tiles_per_chunk + tx - dx/2,
	    y = chunk->cy*c_tiles_per_chunk + ty - dy/2;
	return Tile_coord(x, y, lift + dz);
	}

static inline void delta_check
	(
	int delta1,
	int size1,
	int size2,
	short& coord1,
	short& coord2
	)
	{
	if (delta1 < 0)
		{
		if (coord1 + size1 > coord2)
			coord1 = coord2;
		else
			coord1 += size1;
		}
	else if (delta1 > 0)
		{
		if (coord2 + size2 > coord1)
			coord2 = coord1;
		else
			coord2 += size2;
		}
	}

static inline void delta_wrap_check
	(
	int dir,			// Neg. if coord2 < coord1.
	int size1,
	int size2,
	short& coord1,
	short& coord2
	)
	{
	// NOTE: An obj's tile is it's lower-right corner.
	if (dir > 0)			// Coord2 > coord1.
		coord2 = (coord2 - size2 + c_num_tiles)%c_num_tiles;
	else if (dir < 0)
		coord1 = (coord1 - size1 + c_num_tiles)%c_num_tiles;
	}

/*
 *	Calculate effective distance for sound purposes.
 *	Gives a more realistic fall-off.
 */

int Game_object::sound_distance
	(
	Game_object *o2
	) const
	{
	Tile_coord t1 = get_tile(), t2 = o2->get_tile();
	Shape_info& info1 = get_info(), info2 = o2->get_info();
	int f1 = get_framenum(), f2 = o2->get_framenum();
	int dx = Tile_coord::delta(t1.tx, t2.tx),
		dy = Tile_coord::delta(t1.ty, t2.ty),
		dz = t1.tz - t2.tz;
	delta_wrap_check(dx, info1.get_3d_xtiles(f1)-1,
			info2.get_3d_xtiles(f2)-1, t1.tx, t2.tx);
	delta_wrap_check(dy, info1.get_3d_ytiles(f1)-1,
			info2.get_3d_ytiles(f2)-1, t1.ty, t2.ty);
	delta_check(dz, info1.get_3d_height(),
			info2.get_3d_height(), t1.tz, t2.tz);
	return abs(t1.tx - t2.tx) + abs(t1.ty - t2.ty) + abs(t1.tz - t2.tz);
	}

/*
 *	Calculate distance to object taking 3D size in consideration.
 *	U7 & SI verified.
 */

int Game_object::distance
	(
	Game_object *o2
	) const
	{
	Tile_coord t1 = get_tile(), t2 = o2->get_tile();
	Shape_info& info1 = get_info(), info2 = o2->get_info();
	int f1 = get_framenum(), f2 = o2->get_framenum();
	int dx = Tile_coord::delta(t1.tx, t2.tx),
		dy = Tile_coord::delta(t1.ty, t2.ty),
		dz = t1.tz - t2.tz;
	delta_wrap_check(dx, info1.get_3d_xtiles(f1)-1,
			info2.get_3d_xtiles(f2)-1, t1.tx, t2.tx);
	delta_wrap_check(dy, info1.get_3d_ytiles(f1)-1,
			info2.get_3d_ytiles(f2)-1, t1.ty, t2.ty);
	delta_check(dz, info1.get_3d_height(),
			info2.get_3d_height(), t1.tz, t2.tz);
	return t1.distance(t2);
	}
/*
 *	Calculate distance to tile taking 3D size in consideration.
 *	U7 & SI verified.
 */

int Game_object::distance
	(
	Tile_coord t2
	) const
	{
	Tile_coord t1 = get_tile();
	Shape_info& info1 = get_info();
	int f1 = get_framenum();
	int dx = Tile_coord::delta(t1.tx, t2.tx),
		dy = Tile_coord::delta(t1.ty, t2.ty),
		dz = t1.tz - t2.tz;
	delta_wrap_check(dx, info1.get_3d_xtiles(f1)-1, 0, t1.tx, t2.tx);
	delta_wrap_check(dy, info1.get_3d_ytiles(f1)-1, 0, t1.ty, t2.ty);
	delta_check(dz, info1.get_3d_height(), 0, t1.tz, t2.tz);
	return t1.distance(t2);
	}

/*
 *	Get direction to another object.
 */

int Game_object::get_direction
	(
	Game_object *o2
	) const
	{
	Tile_coord t1 = get_center_tile();
	Tile_coord t2 = o2->get_center_tile();
					// Treat as cartesian coords.
	return (int) Get_direction(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Get direction to a given tile.
 */

int Game_object::get_direction
	(
	Tile_coord t2
	) const
	{
	Tile_coord t1 = get_center_tile();
					// Treat as cartesian coords.
	return (int) Get_direction(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Get direction to best face an object.
 */

int Game_object::get_facing_direction
	(
	Game_object *o2
	) const
	{
	Tile_coord t1 = get_tile();
	Rectangle torect = o2->get_footprint();
	if (torect.x + torect.w <= t1.tx && 
	    t1.ty >= torect.y && t1.ty < torect.y + torect.h)
		return (int) west;
	else if (t1.tx < torect.x &&
	    t1.ty >= torect.y && t1.ty < torect.y + torect.h)
		return (int) east;
	else if (torect.y + torect.h <= t1.ty &&
	    t1.tx >= torect.x && t1.tx < torect.w + torect.h)
		return (int) south;
	else if (t1.ty < torect.y &&
	    t1.tx >= torect.x && t1.tx < torect.w + torect.h)
		return (int) north;
	else
		return get_direction(o2);
	}

/*
 *	Does a given shape come in quantity.
 */
static int Has_quantity
	(
	int shnum			// Shape number.
	)
	{
	Shape_info& info = ShapeID::get_info(shnum);
	return info.has_quantity();
	}

static int Has_hitpoints(int shnum)
{
	Shape_info& info = ShapeID::get_info(shnum);
	return ((info.get_shape_class() == Shape_info::has_hp) ||
			(info.get_shape_class() == Shape_info::container));

	// containers have hitpoints too ('resistance')
}

const int MAX_QUANTITY = 100;		// Highest quantity possible.

/*
 *	Get the quantity.
 */

int Game_object::get_quantity
	(
	) const
	{
	int shnum = get_shapenum();
	if (Has_quantity(shnum))
		{
		int qual = quality & 0x7f;
		return qual ? qual : 1;
		}
	else
		return 1;
	}

/*
 *	Get effective maximum range for weapon.
 */
int Game_object::get_effective_range
	(
	const Weapon_info *winf,
	int reach
	)
	{
	if (reach < 0)
		{
		if (!winf)
			return 3;
		reach = winf->get_range();
		}
	int uses = winf ? winf->get_uses() : Weapon_info::melee;
	if (!uses || uses == Weapon_info::ranged)
		return reach;
	else
		return 31;
	}

/*
 *	Checks to see if the object has ammo for a weapon.
 *	Output is ammount of ammo needed and -> to ammo
 *	object, if the argument is not null.
 */
int Game_object::get_weapon_ammo
	(
	int weapon,
	int family,
	int proj,
	bool ranged,
	Game_object **ammo,
	bool bg,
	bool recursive
	)
	{
	if (ammo)
		*ammo = 0;
	if (weapon < 0)
		return false;	// Bare hands.
	// See if we need ammo.
	Weapon_info *winf = ShapeID::get_info(weapon).get_weapon_info();
	if (!winf)
		// No ammo needed.
		return 0;
	int uses = winf->get_uses();
	int need_ammo = 0;
		// This seems to match perfectly the originals.
	if (family == -1 || !ranged)
		need_ammo = !uses && winf->uses_charges();
	else
		need_ammo = 1;
	if (need_ammo && family >= 0 && proj >= 0)
		{
		// BG triple crossbows uses 3x ammo.
		Shape_info& info = ShapeID::get_info(winf->get_projectile());
		if (info.get_ready_type() == triple_bolts)
			need_ammo = 3;
		}

	if (ammo)
		*ammo = find_weapon_ammo(weapon, need_ammo, recursive);
	return need_ammo;
	}

int Game_object::get_effective_obj_hp(int weapon_shape) const
{
 	int hps = get_obj_hp();
	if (!hps)
		{
		Shape_info& inf = get_info();
		int qual = inf.has_quality() ? get_quality() : -1;
		hps = inf.get_effective_hps(get_framenum(), qual);
		}
	return hps;
}

int Game_object::get_obj_hp() const
{
	if (Has_hitpoints(get_shapenum()))
		return quality;
	else
		return 0;
}

void Game_object::set_obj_hp(int hp)
{
	int shnum = get_shapenum();
	if (Has_hitpoints(shnum))
		set_quality(hp);
}

/*
 *	Get the volume.
 */

int Game_object::get_volume
	(
	) const
	{
	int vol = get_info().get_volume();
	return vol;			// I think U7 ignores quantity!
	}

/*
 *	Returns true if the object is inside a locked container.
 */
bool Game_object::inside_locked() const
	{
	const Game_object *top = this;
	const Game_object *above;
	while ((above = top->get_owner()) != 0)
		{
		if (above->get_info().is_container_locked())
			return true;
		top = above;
		}
	return false;
	}


/*
 *	Add or remove from object's 'quantity', and delete if it goes to 0.
 *	Also, this sets the correct frame, even if delta == 0.
 *
 *	Output:	Delta decremented/incremented by # added/removed.
 *		Container's volume_used field is updated.
 */

int Game_object::modify_quantity
	(
	int delta,			// >=0 to add, <0 to remove.
	bool *del			// If !null, true ret'd if deleted.
	)
	{
	if (del)
		*del = false;
	if (!Has_quantity(get_shapenum()))
		{			// Can't do quantity here.
		if (delta > 0)
			return (delta);
		remove_this();		// Remove from container (or world).
		if (del)
			*del = true;
		return (delta + 1);
		}
	int quant = quality&0x7f;	// Get current quantity.
	if (!quant)
		quant = 1;		// Might not be set.
	int newquant = quant + delta;
	if (delta >= 0)			// Adding?
		{			// Too much?
		if (newquant > MAX_QUANTITY)
			newquant = MAX_QUANTITY;
		}
	else if (newquant <= 0)		// Subtracting.
		{
		remove_this();		// We're done for.
		if (del)
			*del = true;
		return (delta + quant);
		}
	int oldvol = get_volume();	// Get old volume used.
	quality = (char) newquant;	// Store new value.
					// Set appropriate frame.
	if (get_info().has_weapon_info())	// Starbursts, serpent(ine) daggers, knives.
		set_frame(0);		// (Fixes messed-up games.)
	else if (get_info().has_quantity_frames())
		{
			// This is actually hard-coded in the originals, but doing
			// it this way is consistent with musket ammo.
		int base = get_info().has_ammo_info() ? 24 : 0;
			// Verified.
		int new_frame = newquant > 12 ? 7 : (newquant > 6 ? 6 : newquant - 1);
		set_frame(base + new_frame);
		}

	Container_game_object *owner = get_owner();
	if (owner)			// Update owner's volume.
		owner->modify_volume_used(get_volume() - oldvol);
	return (delta - (newquant - quant));
	}

/*
 *	Based on frame #, get direction (N, S, E, W, 0-7), this (generally an
 *	NPC) is facing.
 */

int Game_object::get_dir_facing
	(
	) const
	{
	int reflect = get_framenum()&(16 | 32);
	switch (reflect)
		{
	case 0:
		return (int) north;
	case 48:
		return (int) east;
	case 16:
		return (int) south;
	case 32:
	default:
		return (int) west;
		}
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (chunk = 0).
 */

void Game_object::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
					// Figure new chunk.
	int newcx = newtx/c_tiles_per_chunk, newcy = newty/c_tiles_per_chunk;
	Game_map *objmap = newmap >= 0 ? gwin->get_map(newmap) : get_map();
	if (!objmap) objmap = gmap;
	Map_chunk *newchunk = objmap->get_chunk_safely(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
	Map_chunk *oldchunk = chunk;	// Remove from old.
	if (oldchunk)
		{
		gwin->add_dirty(this);	// Want to repaint old area.
		oldchunk->remove(this);
		}
	set_lift(newlift);		// Set new values.
	tx = newtx%c_tiles_per_chunk;
	ty = newty%c_tiles_per_chunk;
	newchunk->add(this);		// Updates 'chunk'.
	gwin->add_dirty(this);		// And repaint new area.
	}

/*
 *	Change the frame and set to repaint areas.
 */

void Game_object::change_frame
	(
	int frnum
	)
	{
	gwin->add_dirty(this);		// Set to repaint old area.
	set_frame(frnum);
	gwin->add_dirty(this);		// Set to repaint new.
	}

/*
 *	Swap positions with another object (of the same footprint).
 *
 *	Output: 1 if successful, else 0.
 */

int Game_object::swap_positions
	(
	Game_object *obj2
	)
	{
	Shape_info& inf1 = get_info();
	Shape_info& inf2 = obj2->get_info();
	int frame1 = get_framenum();
	int frame2 = obj2->get_framenum();
	if (inf1.get_3d_xtiles(frame1) != inf2.get_3d_xtiles(frame2) ||
	    inf1.get_3d_ytiles(frame1) != inf2.get_3d_ytiles(frame2))
		return 0;		// Not the same size.
	Tile_coord p1 = get_tile();
	Tile_coord p2 = obj2->get_tile();
	remove_this(1);			// Remove (but don't delete) each.
	set_invalid();
	obj2->remove_this(1);
	obj2->set_invalid();
	move(p2.tx, p2.ty, p2.tz);	// Move to new locations.
	obj2->move(p1.tx, p1.ty, p1.tz);
	return (1);
	}

/*
 *	Remove all dependencies.
 */

void Game_object::clear_dependencies
	(
	)
	{
	Game_object_set::const_iterator	X;
	
	// First do those we depend on.
	for(X = dependencies.begin(); X != dependencies.end(); ++X )
		(**X).dependors.erase(this);
	dependencies.clear();
	
	// Now those who depend on us.
	for(X = dependors.begin(); X != dependors.end(); ++X )
		(**X).dependencies.erase(this);
	dependors.clear();
	}

/*
 *	Check an object in find_nearby() against the mask.
 *
 *	Output:	1 if it passes.
 */
static int Check_mask
	(
	Game_window *gwin,
	Game_object *obj,
	int mask
	)
	{
	Shape_info& info = obj->get_info();
	if ((mask&(4|8)) &&		// Both seem to be all NPC's.
	    !info.is_npc())
		return 0;
	Shape_info::Shape_class sclass = info.get_shape_class();
					// Egg/barge?
	if ((sclass == Shape_info::hatchable || sclass == Shape_info::barge) &&
	    !(mask&0x10))		// Only accept if bit 16 set.
		return 0;
	if (info.is_transparent() &&	// Transparent?
	    !(mask&0x80))
		return 0;
					// Invisible object?
	if (obj->get_flag(Obj_flags::invisible))
		if (!(mask&0x20))	// Guess:  0x20 == invisible.
			{
			if (!(mask&0x40))	// Guess:  Inv. party member.
				return 0;
			if (!obj->get_flag(Obj_flags::in_party))
				return 0;
			}
	return 1;			// Passed all tests.
}

/*
 *	Find objects near a given position.
 *
 *	Output:	# found, appended to vec.
 */

#define FN_VECTOR Egg_vector
#define FN_OBJECT Egg_object
#define FN_CAST ->as_egg()
#include "find_nearby.h"

int Game_object::find_nearby_eggs
	(
	Egg_vector& vec,
	int shapenum,
	int delta,
	int qual,
	int frnum
	) const
	{
	return Game_object::find_nearby (vec, get_tile(), shapenum,
					delta, 16, qual, frnum);
	}

#define FN_VECTOR Actor_vector
#define FN_OBJECT Actor
#define FN_CAST ->as_actor()
#include "find_nearby.h"

int Game_object::find_nearby_actors
	(
	Actor_vector& vec,
	int shapenum,
	int delta,
	int mask
	) const
	{
	return Game_object::find_nearby(vec, get_tile(), shapenum,
						delta, mask|8, c_any_qual, c_any_framenum);
	}

#define FN_VECTOR Game_object_vector
#define FN_OBJECT Game_object
#define FN_CAST
#include "find_nearby.h"

int Game_object::find_nearby
	(
	Game_object_vector& vec,
	int shapenum,
	int delta,
	int mask,
	int qual,
	int framenum
	) const
	{
	return Game_object::find_nearby(vec, get_tile(), shapenum,
					delta, mask, qual, framenum);
	}

/*
 *	For sorting closest to a given spot.
 */
class Object_closest_sorter
	{
	Tile_coord pos;			// Pos. to get closest to.
public:
	Object_closest_sorter(Tile_coord p) : pos(p)
		{  }
	bool operator()(const Game_object *o1, const Game_object *o2)
		{
		Tile_coord t1 = o1->get_tile(),
			   t2 = o2->get_tile();
		return t1.distance(pos) < t2.distance(pos);
		}
	};

/*
 *	Find the closest nearby objects with a shape in a given list.
 *
 *	Output:	->closest object, or 0 if none found.
 */

Game_object *Game_object::find_closest
	(
	Game_object_vector& vec,	// List returned here, closest 1st.
	int *shapenums,			// Shapes to look for. 
					//   c_any_shapenum=any NPC.
	int num_shapes,			// Size of shapenums.
	int dist			// Distance to look (tiles).
	)
	{
	int i;
	for (i = 0; i < num_shapes; i++)
					// 0xb0 mask finds anything.
		find_nearby(vec, shapenums[i], dist, 0xb0);
	int cnt = vec.size();
	if (!cnt)
		return (0);
	if (cnt > 1)
		std::sort(vec.begin(), vec.end(), 
				Object_closest_sorter(get_tile()));
	return *(vec.begin());
	}

/*
 *	Find the closest nearby object with a shape in a given list.
 *
 *	Output:	->object, or 0 if none found.
 */

Game_object *Game_object::find_closest
	(
	Tile_coord pos,			// Where to look from.
	int *shapenums,			// Shapes to look for. 
					//   c_any_shapenum=any NPC.
	int num_shapes,			// Size of shapenums.
	int dist			// Distance to look (tiles).
	)
	{
	Game_object_vector vec;			// Gets objects found.
	int i;
	for (i = 0; i < num_shapes; i++)
					// 0xb0 mask finds anything.
		find_nearby(vec, pos, shapenums[i], dist, 0xb0);
	int cnt = vec.size();
	if (!cnt)
		return (0);
	Game_object *closest = 0;	// Get closest.
	int best_dist = 10000;		// In tiles.
					// Get our location.
	for (Game_object_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		int dist = obj->get_tile().distance(pos);
		if (dist < best_dist)
			{
			closest = obj;
			best_dist = dist;
			}
		}
	return (closest);
	}

/*
 *	Get footprint in absolute tiles.
 */

Rectangle Game_object::get_footprint
	(
	)
	{
	Shape_info& info = get_info();
					// Get footprint.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	Tile_coord t = get_tile();
	Rectangle foot((t.tx - xtiles + 1 + c_num_tiles)%c_num_tiles, 
		       (t.ty - ytiles + 1 + c_num_tiles)%c_num_tiles, 
							xtiles, ytiles);
	return foot;
	}

/*
 *	Get volume in absolute tiles.
 */

Block Game_object::get_block
	(
	)
	{
	Shape_info& info = get_info();
					// Get footprint.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	int ztiles = info.get_3d_height();
	Tile_coord t = get_tile();
	Block vol((t.tx - xtiles + 1 + c_num_tiles)%c_num_tiles, 
		  (t.ty - ytiles + 1 + c_num_tiles)%c_num_tiles, 
		   t.tz, 
		  xtiles, ytiles, ztiles);
	return vol;
	}

/*
 *	Does this object block a given tile?
 */

bool Game_object::blocks
	(
	Tile_coord tile
	)
	{
	Tile_coord t = get_tile();
	if (t.tx < tile.tx || t.ty < tile.ty || t.tz > tile.tz)
		return false;		// Out of range.
	Shape_info& info = get_info();
	int ztiles = info.get_3d_height(); 
	if (!ztiles || !info.is_solid())
		return false;		// Skip if not an obstacle.
					// Occupies desired tile?
	int frame = get_framenum();
	if (tile.tx > t.tx - info.get_3d_xtiles(frame) &&
	    tile.ty > t.ty - info.get_3d_ytiles(frame) &&
	    tile.tz < t.tz + ztiles)
		return true;
	return false;
	}

/*
 *	Find the game object that's blocking a given tile.
 *
 *	Output:	->object, or 0 if not found.
 */

Game_object *Game_object::find_blocking
	(
	Tile_coord tile			// Tile to check.
	)
	{
	tile.fixme();
	Map_chunk *chunk = gmap->get_chunk(tile.tx/c_tiles_per_chunk,
						    tile.ty/c_tiles_per_chunk);
	Game_object *obj;
	Object_iterator next(chunk->get_objects());
	while ((obj = next.get_next()) != 0)
		if (obj->blocks(tile))
			return obj;
	return (0);
	}

/*
 *	Find door blocking a given tile.
 *
 *	Output:	->door, or 0 if not found.
 */

Game_object *Game_object::find_door
	(
	Tile_coord tile
	)
	{
	tile.fixme();
	Map_chunk *chunk = gmap->get_chunk(tile.tx/c_tiles_per_chunk,
						    tile.ty/c_tiles_per_chunk);
	return chunk->find_door(tile);
	}

/*
 *	Is this a closed door?
 */

int Game_object::is_closed_door
	(
	) const
	{
	Shape_info& info = get_info();
	if (!info.is_door())
		return 0;
					// Get door's footprint.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame), ytiles = info.get_3d_ytiles(frame);
					// Get its location.
	Tile_coord doortile = get_tile();
	Tile_coord before, after;	// Want tiles to both sides.
	if (xtiles > ytiles)		// Horizontal footprint?
		{
		before = doortile + Tile_coord(-xtiles, 0, 0);
		after = doortile + Tile_coord(1, 0, 0);
		}
	else				// Vertical footprint.
		{
		before = doortile + Tile_coord(0, -ytiles, 0);
		after = doortile + Tile_coord(0, 1, 0);
		}
					// Should be blocked before/after.
	return (gmap->is_tile_occupied(before) &&
	    	gmap->is_tile_occupied(after));
	}

/*
 *	Get the topmost owner of this object.
 *
 *	Output:	->topmost owner, or the object itself.
 */

Game_object *Game_object::get_outermost
	(
	)
	{
	Game_object *top = this;
	Game_object *above;
	while ((above = top->get_owner()) != 0)
		top = above;
	return top;
	}

/*
 *	Show text by the object on the screen.
 */

void Game_object::say
	(
	const char *text
	)
	{
	eman->add_text(text, this);
	}

/*
 *	Show a message
 *	(Msg. #'s start from 0, and are stored from 0x400 in 'text.flx'.)
 */

void Game_object::say
	(
	int msgnum
	)
	{
	say(text_msgs[msgnum]);
	}

/*
 *	Show a random msg. from 'text.flx' by the object.
 *	(Msg. #'s start from 0, and are stored from 0x400 in 'text.flx'.)
 */

void Game_object::say
	(
	int from, int to		// Range (inclusive).
	)
{
	if (from > to) return;
	int offset = rand()%(to - from + 1);
	if (from + offset < num_text_msgs)
		say(text_msgs[from + offset]);
}

/*
 *	Paint at given spot in world.
 */

void Game_object::paint
	(
	)
	{
	int x, y;
	gwin->get_shape_location(this, x, y);
	paint_shape(x, y);
	}

/*
 *	Paint outline.
 */

void Game_object::paint_outline
	(
	Pixel_colors pix		// Color to use.
	)
	{
	int x, y;
	gwin->get_shape_location(this, x, y);
	ShapeID::paint_outline(x, y, pix);
	}

/*
 *	Run usecode when double-clicked.
 */

void Game_object::activate
	(
	int event
	)
	{
	if (edit())
		return;			// Map-editing.
	int usefun = get_usecode();
					// Serpent Isle spell scrolls:
	if (usefun == 0x2cb && Game::get_game_type() == SERPENT_ISLE)
		{
		gumpman->add_gump(this, 65);
		return;
		}
					// !!!Special case:  books
	if (usefun == 0x282 && get_quality() >= 100 && get_quality() < 180)
		usefun = 0x638;
	else if (usefun == 0x282 && get_quality() >= 180 && 
			 Game::get_game_type() == SERPENT_ISLE )
		usefun = 0x63b;
	else if (usefun == 0x2c1 && get_quality() >= 213 &&
			 Game::get_game_type() == SERPENT_ISLE )
		usefun = 0x62a;
	ucmachine->call_usecode(usefun, this,
			(Usecode_machine::Usecode_events) event);
	}

/*
 *	Edit in ExultStudio.
 */

bool Game_object::edit
	(
	)
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_tile();
		unsigned long addr = (unsigned long) this;
		std::string name = get_name();
		if (Object_out(client_socket, Exult_server::obj, 
			addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), get_quality(),
								name) != -1)
			{
			cout << "Sent object data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending object to ExultStudio" <<endl;
		return true;
		}
#endif
	return false;
	}

/*
 *	Message to update from ExultStudio.
 */

void Game_object::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, quality;
	std::string name;
	if (!Object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		quality, name))
		{
		cout << "Error decoding object" << endl;
		return;
		}
	Game_object *obj = (Game_object *) addr;
	if (!editing || obj != editing)
		{
		cout << "Obj from ExultStudio is not being edited" << endl;
		return;
		}
//	editing = 0;	// He may have chosen 'Apply', so still editing.
	gwin->add_dirty(obj);
	obj->set_shape(shape, frame);
	gwin->add_dirty(obj);
	obj->set_quality(quality);
	Container_game_object *owner = obj->get_owner();
	if (!owner)	// See if it moved -- but only if not inside something!
			// Don't skip this even if coords. are the same, since
			//   it will mark the chunk as modified.
		obj->move(tx, ty, tz);
#endif
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	if (chunk)
		chunk->remove(this);
	if (!nodel)
		gwin->delete_object(this);
	}

/*
 *	Can this be dragged?
 */

int Game_object::is_dragable
	(
	) const
	{
	return (0);			// Default is 'no'.
	}

/*
 *	Static method to get shape's weight in 1/10 stones.  0 means infinite.
 */

int Game_object::get_weight
	(
	int shnum,			// Shape #,
	int quant			// Quantity.
	)
	{
	Shape_info& inf = ShapeID::get_info(shnum);
	int wt = quant * inf.get_weight();
	if (inf.is_lightweight())
	{			// Special case:  reagents, coins.
		wt /= 10;
		if (wt <= 0) wt = 1;
	}

	if (Has_quantity(shnum))
		if (wt <= 0) wt = 1;

	return wt;
	}

/* 
 *	Get weight of object in 1/10 stones.
 */

int Game_object::get_weight
	(
	)
	{
	return get_weight(get_shapenum(), get_quantity());
	}

/*
 *	Get maximum weight in stones that can be held.
 *
 *	Output:	Max. allowed, or 0 if no limit (i.e., not carried by an NPC).
 */

int Game_object::get_max_weight
	(
	)
	{
					// Looking outwards for NPC.
	Container_game_object *own = get_owner();
	return own ? own->get_max_weight() : 0;
	}

/*
 *	Add an object to this one by combining.
 *
 *	Output:	1, meaning object is completely combined to this.  Obj. is
 *			deleted in this case.
 *		0 otherwise, although obj's quantity may be
 *			reduced if combine==true.
 */

bool Game_object::add
	(
	Game_object *obj,
	bool dont_check,		// 1 to skip volume/recursion check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
	{
	return combine ? drop(obj)!=0 : false;
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Game_object::drop
	(
	Game_object *obj		// This may be deleted.
	)
	{
	Shape_info& inf = get_info();
	int shapenum = get_shapenum();	// It's possible if shapes match.
	if (obj->get_shapenum() != shapenum || !inf.has_quantity() ||
	    (!inf.has_quantity_frames() && get_framenum() != obj->get_framenum()))
		return (0);
	int objq = obj->get_quantity();
	int total_quant = get_quantity() + objq;
	if (total_quant > MAX_QUANTITY)	// Too much?
		return (0);
	modify_quantity(objq);		// Add to our quantity.
	obj->remove_this();		// It's been used up.
	return (1);
	}

//#define DEBUGLT
#ifdef DEBUGLT
static int rx1 = -1, ry1 = -1, rx2 = -1, ry2 = -1;

static void Debug_lt
	(
	int tx1, int ty1,		// 1st coord.
	int tx2, int ty2		// 2nd coord.
	)
	{
	if (tx1 == rx1 && ty1 == ry1)
		{
		if (tx2 == rx2 && ty2 == ry2)
			cout << "Debug_lt" << endl;
		}
	}
#endif

/*
 *	Compare ranges along a given dimension.
 */
inline void Compare_ranges
	(
	int from1, int to1,		// First object's range.
	int from2, int to2,		// Second object's range.
					// Returns:
	int& cmp,			// -1 if 1st < 2nd, 1 if 1st > 2nd,
					//   0 if equal.
	bool& overlap			// true returned if they overlap.
	)
	{
	if (to1 < from2)
		{
		overlap = false;
		cmp = -1;
		}
	else if (to2 < from1)
		{
		overlap = false;
		cmp = 1;
		}
	else				// X's overlap.
		{
		overlap = true;
		if (from1 < from2)
			cmp = -1;
		else if (from1 > from2)
			cmp = 1;
		else if (to1 - from1 < to2 - from2)
			cmp = 1;
		else if (to1 - from1 > to2 - from2)
			cmp = -1;
		else
			cmp = 0;
		}
	}

/*
 *	Compare two objects.
 *
 *	Output:	-1 if 1st < 2nd, 0 if dont_care, 1 if 1st > 2nd.
 */

int Game_object::compare
	(
	Ordering_info& inf1,		// Info. for object 1.
	Game_object *obj2
	)
	{
					// See if there's no overlap.
	Rectangle r2 = gwin->get_shape_rect(obj2);
	if (!inf1.area.intersects(r2))
		return (0);		// No overlap on screen.
	Ordering_info inf2(gwin, obj2, r2);
#ifdef DEBUGLT
	Debug_lt(inf1.tx, inf1.ty, inf2.tx, inf2.ty);
#endif
	int xcmp, ycmp, zcmp;		// Comparisons for a given dimension:
					//   -1 if o1<o2, 0 if o1==o2,
					//    1 if o1>o2.
	bool xover, yover, zover;	// True if dim's overlap.
	Compare_ranges(inf1.xleft, inf1.xright, inf2.xleft, inf2.xright,
							xcmp, xover);
	Compare_ranges(inf1.yfar, inf1.ynear, inf2.yfar, inf2.ynear,
							ycmp, yover);
	Compare_ranges(inf1.zbot, inf1.ztop, inf2.zbot, inf2.ztop,
							zcmp, zover);
	if (!xcmp && !ycmp && !zcmp)
					// Same space?
					// Paint biggest area sec. (Fixes 
					//   plaque at Penumbra's.)
		return (inf1.area.w < inf2.area.w  && 
			inf1.area.h < inf2.area.h) ? -1 : 
			(inf1.area.w > inf2.area.w &&
			inf1.area.h > inf2.area.h) ? 1 : 0;
//		return 0;		// Equal.
	if (xover & yover & zover)	// Complete overlap?
		{
		if (!inf1.zs)		// Flat one is always drawn first.
			return !inf2.zs ? 0 : -1;
		else if (!inf2.zs)
			return 1;
		}
	if (xcmp >= 0 && ycmp >= 0 && zcmp >= 0)
		return 1;		// GTE in all dimensions.
	if (xcmp <= 0 && ycmp <= 0 && zcmp <= 0)
		return -1;		// LTE in all dimensions.
	if (yover)			// Y's overlap.
		{
		if (xover)		// X's too?
			return zcmp;
		else if (zover)		// Y's and Z's?
			return xcmp;
					// Just Y's overlap.
		else if (!zcmp)		// Z's equal?
			return xcmp;
		else			// See if X and Z dirs. agree.
			if (xcmp == zcmp)
				return xcmp;
#if 1 /* Woohoo!  Seems to work without messing up N. Trinsic gate. */
					// Experiment:  Fixes Trinsic mayor
					//   statue-through-roof.
		else if (inf1.ztop/5 < inf2.zbot/5 && inf2.info.occludes())
			return -1;	// A floor above/below.
		else if (inf2.ztop/5 < inf1.zbot/5 && inf1.info.occludes())
			return 1;
#endif
		else
			return 0;
		}
	else if (xover)			// X's overlap.
		{
		if (zover)		// X's and Z's?
			return ycmp;
		else if (!zcmp)		// Z's equal?
			return ycmp;
		else
			return ycmp == zcmp ? ycmp : 0;
		}
					// Neither X nor Y overlap.
	else if (xcmp == -1)		// o1 X before o2 X?
		{
		if (ycmp == -1)		// o1 Y before o2 Y?
					// If Z agrees or overlaps, it's LT.
			return (zover || zcmp <= 0) ? -1 : 0;
		}
	else if (ycmp == 1)		// o1 Y after o2 Y?
		{
		if (zover || zcmp >= 0)
			return 1;
#if 1	/* So far, this seems to work without causing problems: */
					// Experiment:  Fixes Brit. museum
					//   statue-through-roof.
		else if (inf1.ztop/5 < inf2.zbot/5)
			return -1;	// A floor above.
		else
#endif
			return 0;
		}
	return 0;
	}


/*
 *	Should this object be rendered before obj2?
 *	NOTE:  This older interface isn't as efficient.
 *
 *	Output:	1 if so, 0 if not, -1 if cannot compare.
 */
int Game_object::lt
	(
	Game_object& obj2
	)
	{
	Ordering_info ord(gwin, this);
	int cmp = compare(ord, &obj2);
	return cmp == -1 ? 1 : cmp == 1 ? 0 : -1;
	}


/*
 *	Get frame if rotated 1, 2, or 3 quadrants clockwise.  This is to
 *	support barges (ship, cart, flying carpet).
 */

int Game_object::get_rotated_frame
	(
	int quads			// 1=90, 2=180, 3=270.
	)
	{
	int curframe = get_framenum();
	return get_info().get_rotated_frame(curframe, quads);
	}

/*
 *	This method should be called to cause damage from traps, attacks.
 *
 *	Output:	Hits taken.
 */
int Game_object::apply_damage
	(
	Game_object *attacker,	// Attacker, or null.
	int str,		// Attack strength.
	int wpoints,	// Weapon bonus.
	int type,		// Damage type.
	int bias,		// Different combat difficulty.
	int *exp
	)
	{
	if (exp)
		exp = 0;
	int damage = 0;
	if (wpoints == 127)
		damage = 127;
	else
		{
		if (type != Weapon_data::lightning_damage && str > 0)
			{
			int base = str/3;
			damage = base ? 1 + rand()%base : 0;
			}
		if (wpoints > 0)
			damage += (1 + rand()%wpoints);
		}

	if (damage <= 0)
		{
		int sfx = Audio::game_sfx(5);
		new Object_sfx(this, sfx);
		return 0;
		}
	return reduce_health(damage, type, attacker);
	}

/*
 *	This method should be called to decrement health directly.
 *
 *	Output:	Hits taken.
 */

int Game_object::reduce_health
	(
	int delta,			// # points to lose.
	int type,		// Type of damage.
	Game_object *attacker,	// Attacker, or null.
	int *exp
	)
	{
	if (exp)
		*exp = 0;
		// Returns 0 if doesn't have HP's or is indestructible.
	int hp = get_effective_obj_hp();
	if (!hp	|| 	// Object is indestructible.
				// These damage types do not affect objects.
			type == Weapon_data::lightning_damage ||
			type == Weapon_data::ethereal_damage)
		return 0;
	if (get_info().is_explosive() &&
			(type == Weapon_data::fire_damage || delta >= hp))
		{	// Cause chain reaction.
		set_quality(1);	// Make it indestructible.
		Tile_coord offset(0, 0, get_info().get_3d_height()/2);
		eman->add_effect(new Explosion_effect(get_tile() + offset,
				this, 0, get_shapenum(), -1, attacker));
		return delta;	// Will be destroyed in Explosion_effect.
		}
	if (delta < hp)
		set_obj_hp(hp - delta);
	else
		{
		// object destroyed
		eman->remove_text_effect(this);
		ucmachine->call_usecode(0x626, this, Usecode_machine::weapon);
		}
	return delta;
	}

/*
 *	Being attacked.
 *
 *	Output:	Hits taken or < 0 for explosion.
 */

int Game_object::figure_hit_points
	(
	Game_object *attacker,
	int weapon_shape,		// < 0 for readied weapon.
	int ammo_shape,			// < 0 for no ammo shape.
	bool explosion			// If this is an explosion attacking.
	)
	{
	const Weapon_info *winf;
	Ammo_info *ainf;

	int wpoints = 0;
	if (weapon_shape >= 0)
		winf = ShapeID::get_info(weapon_shape).get_weapon_info();
	else
		winf = 0;
	if (ammo_shape >= 0)
		ainf = ShapeID::get_info(ammo_shape).get_ammo_info();
	else
		ainf = 0;
	if (!winf && weapon_shape < 0)
		{
		Actor *npc = attacker ? attacker->as_actor() : 0;
		winf = npc ? npc->get_weapon(wpoints) : 0;
		}

	int usecode = -1;
	int type = Weapon_data::normal_damage;
	bool explodes = false;

	if (winf)
		{
		wpoints = winf->get_damage();
		usecode = winf->get_usecode();
		type = winf->get_damage_type();
		explodes = winf->explodes();
		}
	else
		wpoints = 1;	// Give at least one, but only if there's no weapon
	if (ainf)
		{
		wpoints += ainf->get_damage();
			// Replace damage type.
		if (ainf->get_damage_type() != Weapon_data::normal_damage)
			type = ainf->get_damage_type();
		explodes = explodes || ainf->explodes();
		}

	if (explodes && !explosion)	// Explosions shouldn't explode again.
		{	// Time to explode.
		Tile_coord offset(0, 0, get_info().get_3d_height()/2);
		eman->add_effect(new Explosion_effect(get_tile() + offset,
				0, 0, weapon_shape, ammo_shape, attacker));
		return -1;
		}

	int delta = 0;
	int effstr = attacker && attacker->as_actor()
		? attacker->as_actor()->get_effective_prop(Actor::strength) : 0;
	if (winf && (winf->get_powers() & Weapon_data::no_damage) == 0)
		delta = apply_damage(attacker, effstr, wpoints, type);
		
		// Objects are not affected by weapon powers.

		// Object may be in the remove list by this point.
	if (usecode >= 0)
		ucmachine->call_usecode(usecode, this,
					Usecode_machine::weapon);
	return delta;
	}

void Game_object::play_hit_sfx(int weapon, bool ranged)
	{
	Weapon_info *winf = weapon >= 0 ?
			ShapeID::get_info(weapon).get_weapon_info() : 0;
	if (winf && winf->get_damage())
		{
		int sfx;
		if (ranged)
			sfx = Audio::game_sfx(1);
		else if (weapon == 604)	// Glass sword.
			sfx = Audio::game_sfx(37);
		else
			sfx = Audio::game_sfx(4);
		new Object_sfx(this, sfx);
		}
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if destroyed, else object itself.
 */

Game_object *Game_object::attacked
	(
	Game_object *attacker,
	int weapon_shape,		// < 0 for readied weapon.
	int ammo_shape,			// < 0 for no ammo shape.
	bool explosion			// If this is an explosion attacking.
	)
	{
	int shnum = get_shapenum();

	if (shnum == 735 && ammo_shape == 722)
		{	// Arrows hitting archery practice target.
		int frnum = get_framenum();
		int newframe = !frnum ? (3*(rand()%8) + 1)
				: ((frnum%3) != 0 ? frnum + 1 : frnum);
		change_frame(newframe);
		}

	int oldhp = get_effective_obj_hp();
	int delta = figure_hit_points(attacker, weapon_shape, ammo_shape, explosion);
	int newhp = get_effective_obj_hp();

	if (combat_trace)
		{
		string name = "<trap>";
		if (attacker)
			name = attacker->get_name();
		cout << name << " attacks " << get_name();
		if (oldhp < delta)
			{
			cout << ", destroying it." << endl;
			return 0;
			}
		else if (!delta || oldhp == newhp)
			{	// undamaged/indestructible
			cout << " to no effect." << endl;
			return this;
			}
		else if (delta < 0)
			cout << " causing an explosion." << endl;
		else
			cout << " for " << delta << " hit points, leaving "
				<< newhp << " remaining." << endl;
		}
	return this;
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (chunk = 0).
 */

void Terrain_game_object::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
	bool caching_out = chunk ? chunk->get_map()->is_caching_out() : false;
	if (!caching_out)
		{
		gwin->get_map()->set_map_modified();
		if (chunk)
			{
			Chunk_terrain *ter = chunk->get_terrain();
			if (!prev_flat.is_invalid())
				ter->set_flat(get_tx(), get_ty(), prev_flat);
			else
				ter->set_flat(get_tx(), get_ty(), ShapeID(12, 0));
			gwin->get_map()->set_chunk_terrains_modified();
			}
		}
	Game_object::move(newtx, newty, newlift, newmap);
	if (chunk && !caching_out)
		{
		Chunk_terrain *ter = chunk->get_terrain();
		prev_flat = ter->get_flat(get_tx(), get_ty());
		ter->set_flat(get_tx(), get_ty(), *this);
		gwin->get_map()->set_chunk_terrains_modified();
		}
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Terrain_game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	if (chunk && !nodel)
		{
		// Do NOT remove object if the map is being cached out.
		if (chunk->get_map()->is_caching_out())
			return;
		chunk->get_map()->set_map_modified();
		Chunk_terrain *ter = chunk->get_terrain();
		if (!prev_flat.is_invalid())
			ter->set_flat(get_tx(), get_ty(), prev_flat);
		else
			ter->set_flat(get_tx(), get_ty(), ShapeID(12, 0));
		gwin->get_map()->set_chunk_terrains_modified();
		}
	Game_object::remove_this(nodel);
	}

/*
 *	Paint terrain objects only.
 */

void Terrain_game_object::paint_terrain
	(
	)
	{
	paint();
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (chunk = 0).
 */

void Ifix_game_object::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
	bool caching_out = gwin->get_map()->is_caching_out();
	if (chunk && !caching_out)	// Mark superchunk as 'modified'.
		get_map()->set_ifix_modified(get_cx(), get_cy());
	Game_object::move(newtx, newty, newlift, newmap);
	if (chunk && !caching_out)
		get_map()->set_ifix_modified(get_cx(), get_cy());
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Ifix_game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	if (chunk)			// Mark superchunk as 'modified'.
		{
		int cx = get_cx(), cy = get_cy();
		get_map()->set_ifix_modified(cx, cy);
		}
	Game_object::remove_this(nodel);
	}

/*
 *	Write out an IFIX object.
 */

void Ifix_game_object::write_ifix
	(
	DataSource *ifix,		// Where to write.
	bool v2				// More shapes/frames allowed.
	)
	{
	unsigned char buf[5];
	buf[0] = (tx<<4)|ty;
	buf[1] = lift;
	int shapenum = get_shapenum(), framenum = get_framenum();
	if (v2)
		{
		buf[2] = shapenum&0xff;
		buf[3] = (shapenum>>8)&0xff;
		buf[4] = framenum;
		ifix->write((char*)buf, 5);
		}
	else
		{
		buf[2] = shapenum&0xff;
		buf[3] = ((shapenum>>8)&3) | (framenum<<2);
		ifix->write((char*)buf, 4);
		}
	}





