/**
 **	Chunks.cc - Chunks (16x16 tiles) on the map.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "chunks.h"
#include "chunkter.h"
#include "gamewin.h"
#include "gamemap.h"
#include "shapeinf.h"
#include "citerate.h"
#include "egg.h"
#include "objiter.h"
#include "objs.h"
#include "ordinfo.h"
#include "game.h"
#include "animate.h"
#include "dir.h"
#include "actors.h"

#if 0
#include <iostream>
using std::cout;
using std::endl;
#endif

using std::memset;
using std::rand;

/*
 *	Create the cached data storage for a chunk.
 */

Chunk_cache::Chunk_cache
	(
	) : egg_objects(4)
	{
	memset((char *) &blocked[0], 0, sizeof(blocked));
	memset((char *) &eggs[0], 0, sizeof(eggs));
	}

/*
 *	Delete cache.
 */

Chunk_cache::~Chunk_cache
	(
	)
	{
	}

/*
 *	This mask gives the low bits (b0) for a given # of ztiles.
 */
unsigned long tmasks[16] = {		0x0L,
				        0x1L,
				        0x5L,
				       0x15L,
				       0x55L,
				      0x155L,
				      0x555L,
				     0x1555L,
				     0x5555L,
				    0x15555L,
				    0x55555L,
				   0x155555L,
				   0x555555L,
				  0x1555555L,
				  0x5555555L,
				 0x15555555L
			};

/*
 *	Set (actually, increment count) for a given tile.
 *	Want:	00 => 01,	01 => 10,
 *		10 => 11,	11 => 11.
 *	So:	newb0 = !b0 OR b1,
 *		newb1 =  b1 OR b0
 */
inline void Set_blocked_tile
	(
	unsigned long *blocked,		// 16x16 flags,
	int tx, int ty,			// Tile #'s (0-15).
	int lift,			// Starting lift to set.
	int ztiles			// # tiles along z-axis.
	)
	{
	unsigned long& val = blocked[ty*c_tiles_per_chunk + tx];
					// Get mask for the bit0's:
	unsigned long mask0 = tmasks[ztiles]<<2*lift;
	unsigned long mask1 = mask0<<1;	// Mask for the bit1's.
	unsigned long val0s = val&mask0;
	unsigned long Nval0s = (~val)&mask0;
	unsigned long val1s = val&mask1;
	unsigned long newval = val1s | (val0s<<1) | Nval0s | (val1s>>1);
					// Replace old values with new.
	val = (val&~(mask0|mask1)) | newval;
	}

/*
 *	Clear (actually, decrement count) for a given tile.
 *	Want:	00 => 00,	01 => 00,
 *		10 => 01,	11 => 10.
 *	So:	newb0 =  b1 AND !b0
 *		newb1 =  b1 AND  b0
 */
inline void Clear_blocked_tile
	(
	unsigned long *blocked,		// 16x16 flags,
	int tx, int ty,			// Tile #'s (0-15).
	int lift,			// Starting lift to set.
	int ztiles			// # tiles along z-axis.
	)
	{
	unsigned long& val = blocked[ty*c_tiles_per_chunk + tx];
					// Get mask for the bit0's:
	unsigned long mask0 = tmasks[ztiles]<<2*lift;
	unsigned long mask1 = mask0<<1;	// Mask for the bit1's.
	unsigned long val0s = val&mask0;
	unsigned long Nval0s = (~val)&mask0;
	unsigned long val1s = val&mask1;
	unsigned long newval = (val1s & (val0s<<1)) | ((val1s>>1) & Nval0s);
					// Replace old values with new.
	val = (val&~(mask0|mask1)) | newval;
	}

/*
 *	Set/unset the blocked flags in a region.
 */

void Chunk_cache::set_blocked
	(
	int startx, int starty,		// Starting tile #'s.
	int endx, int endy,		// Ending tile #'s.
	int lift, int ztiles,		// Lift, height info.
	bool set				// 1 to add, 0 to remove.
	)
	{
	if (set)
		{
		for (int y = starty; y <= endy; y++)
			for (int x = startx; x <= endx; x++)
				Set_blocked_tile(blocked, x, y, lift, ztiles);
		}
	else
		{
		for (int y = starty; y <= endy; y++)
			for (int x = startx; x <= endx; x++)
				Clear_blocked_tile(blocked,x, y, lift, ztiles);
		}
	}

/*
 *	Add/remove an object to/from the cache.
 */

void Chunk_cache::update_object
	(
	Map_chunk *chunk,
	Game_object *obj,
	bool add				// 1 to add, 0 to remove.
	)
	{
	Shape_info& info = obj->get_info();
	if (info.is_door())		// Special door list.
		if (add)
			doors.append(obj);
		else
			doors.remove(obj);
	int ztiles = info.get_3d_height(); 
	if (!ztiles || !info.is_solid())
		return;			// Skip if not an obstacle.
					// Get lower-right corner of obj.
	int endx = obj->get_tx();
	int endy = obj->get_ty();
	int frame = obj->get_framenum();// Get footprint dimensions.
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	int lift = obj->get_lift();
	if (xtiles == 1 && ytiles == 1)	// Simplest case?
		{
		if (add)
			Set_blocked_tile(blocked, endx, endy, lift, ztiles);
		else
			Clear_blocked_tile(blocked, endx, endy, lift, ztiles);
		return;
		}
	Rectangle footprint = obj->get_footprint();
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(footprint);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		gmap->get_chunk(cx, cy)->set_blocked(tiles.x, tiles.y, 
			tiles.x + tiles.w - 1, tiles.y + tiles.h - 1, lift,
								ztiles, add);
	}

/*
 *	Set a rectangle of tiles within this chunk to be under the influence
 *	of a given egg, or clear it.
 */

void Chunk_cache::set_egged
	(
	Egg_object *egg,
	Rectangle& tiles,		// Range of tiles within chunk.
	bool add				// 1 to add, 0 to remove.
	)
	{
					// Egg already there?
	int eggnum = egg_objects.find(egg);
	if (add)
		{
		if (eggnum < 0)		// No, so add it.
			eggnum = egg_objects.put(egg);
		if (eggnum > 15)	// We only have 16 bits.
			eggnum = 15;
		short mask = (1<<eggnum);
		int stopx = tiles.x + tiles.w, stopy = tiles.y + tiles.h;
		for (int ty = tiles.y; ty < stopy; ++ty)
			for (int tx = tiles.x; tx < stopx; ++tx)
				eggs[ty*c_tiles_per_chunk + tx] |= mask;
		}
	else				// Remove.
		{
		if (eggnum < 0 || eggnum >= egg_objects.size())
			return;		// Not there.
		egg_objects[eggnum] = NULL;
		if (eggnum >= 15)	// We only have 16 bits.
			{		// Last one at 15 or above?
			for (Egg_vector::const_iterator it = 
				egg_objects.begin() + 15; 
					it != egg_objects.end(); ++it)
				if (*it != 0)
					// No, so leave bits alone.
					return;
			eggnum = 15;
			}
		short mask = ~(1<<eggnum);
		int stopx = tiles.x + tiles.w, stopy = tiles.y + tiles.h;
		for (int ty = tiles.y; ty < stopy; ty++)
			for (int tx = tiles.x; tx < stopx; tx++)
				eggs[ty*c_tiles_per_chunk + tx] &= mask;
		}
	}

/*
 *	Add/remove an egg to the cache.
 */

void Chunk_cache::update_egg
	(
	Map_chunk *chunk,
	Egg_object *egg,
	bool add				// 1 to add, 0 to remove.
	)
	{
					// Get footprint with abs. tiles.
	Rectangle foot = egg->get_area();
	if (!foot.w)
		return;			// Empty (probability = 0).
	Rectangle crect;		// Gets tiles within each chunk.
	int cx, cy;
	if (egg->is_solid_area())
		{			// Do solid rectangle.
		Chunk_intersect_iterator all(foot);
		while (all.get_next(crect, cx, cy))
			gmap->get_chunk(cx, cy)->set_egged(egg, crect, add);
		return;
		}
					// Just do the perimeter.
	Rectangle top(foot.x, foot.y, foot.w, 1);
	Rectangle bottom(foot.x, foot.y + foot.h - 1, foot.w, 1);
	Rectangle left(foot.x, foot.y + 1, 1, foot.h - 2);
	Rectangle right(foot.x + foot.w - 1, foot.y + 1, 1, foot.h - 2);
					// Go through intersected chunks.
	Chunk_intersect_iterator tops(top);
	while (tops.get_next(crect, cx, cy))
		gmap->get_chunk(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator bottoms(bottom);
	while (bottoms.get_next(crect, cx, cy))
		gmap->get_chunk(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator lefts(left);
	while (lefts.get_next(crect, cx, cy))
		gmap->get_chunk(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator rights(right);
	while (rights.get_next(crect, cx, cy))
		gmap->get_chunk(cx, cy)->set_egged(egg, crect, add);

	}

/*
 *	Create the cached data for a chunk.
 */

void Chunk_cache::setup
	(
	Map_chunk *chunk
	)
	{
	Game_object *obj;		// Set 'blocked' tiles.
	Object_iterator next(chunk->get_objects());
	while ((obj = next.get_next()) != 0)
		if (obj->is_egg())
			update_egg(chunk, (Egg_object *) obj, 1);
		else
			update_object(chunk, obj, 1);
			
	obj_list = chunk;
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

inline int Chunk_cache::get_highest_blocked
	(
	int lift,			// Look below this lift.
	unsigned long tflags		// Flags for desired tile.
	)
	{
	int i;				// Look downwards.
	for (i = lift - 1; i >= 0 && !(tflags & (3<<(2*i))); i--)
		;
	return i;
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

int Chunk_cache::get_highest_blocked
	(
	int lift,			// Look below this lift.
	int tx, int ty			// Square to test.
	)
	{
	return get_highest_blocked(lift, blocked[ty*c_tiles_per_chunk + tx]);
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

inline int Chunk_cache::get_lowest_blocked
	(
	int lift,			// Look above this lift.
	unsigned long tflags		// Flags for desired tile.
	)
	{
	int i;				// Look upward.
	for (i = lift; i < 16 && !(tflags & (3<<(2*i))); i++)
		;
	if (i == 16) return -1;
	return i;
	}

/*
 *	Get lowest blocked lift above a given level for a given tile.
 *
 *	Output:	Lowest lift that's blocked by an object, or -1 if none.
 */

int Chunk_cache::get_lowest_blocked
	(
	int lift,			// Look below this lift.
	int tx, int ty			// Square to test.
	)
	{
	return get_lowest_blocked(lift, blocked[ty*c_tiles_per_chunk + tx]);
	}

/*
 *	See if a tile is water or land.
 */

inline void Check_terrain
	(
	Map_chunk *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& terrain			// Sets: bit0 if land, bit1 if water,
					//   bit2 if solid.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (!flat.is_invalid())
		{
		if (flat.get_info().is_water())
			terrain |= 2;
		else if (flat.get_info().is_solid())
			terrain |= 4;
		else
			terrain |= 1;
		}

	}

/*
 *	Is a given square occupied at a given lift?
 *
 *	Output: 1 if so, else 0.
 *		If 0 (tile is free), new_lift contains the new height that
 *		   an actor will be at if he walks onto the tile.
 */

int Chunk_cache::is_blocked
	(
	int height,			// Height (in tiles) of obj. being
					//   tested.
	int lift,			// Given lift.
	int tx, int ty,			// Square to test.
	int& new_lift,			// New lift returned.
	const int move_flags,
	int max_drop,			// Max. drop/rise allowed.
	int max_rise			// Max. rise, or -1 to use old beha-
					//   viour (max_drop if FLY, else 1).
	)
{

	// Ethereal beings always return not blocked
	// and can only move horizontally
	if (move_flags & MOVE_ETHEREAL)
	{
		new_lift = lift;
		return 0;
	}
					// Get bits.
	unsigned long tflags = blocked[ty*c_tiles_per_chunk + tx];
					// Figure max lift allowed.
	if (max_rise == -1)
		max_rise = (move_flags & MOVE_FLY) ? max_drop : 1;
	int max_lift = lift + max_rise;
	if (max_lift > 15)
		max_lift = 15;		// As high as we can go.
	for (new_lift = lift; new_lift <= max_lift; new_lift++)
		{
		if ((tflags & (3 << (2*new_lift))) == 0)
			{		// Not blocked?
			int new_high = get_lowest_blocked(new_lift, tflags);
					// Not blocked above?
			if (new_high == -1 || new_high >= (new_lift + height))
				break;	// Okay.
			}
		}
	if (new_lift > max_lift)	// Spot not found at lift or higher?
		{			// Look downwards.
		new_lift = get_highest_blocked(lift, tflags) + 1;
		if (new_lift >= lift)	// Couldn't drop?
			return 1;
		int new_high = get_lowest_blocked(new_lift, tflags);
		if (new_high != -1 && new_high < new_lift + height)
			return 1;	// Still blocked above.
		}
	if (new_lift <= lift)		// Not going up?  See if falling.
		{
		new_lift =  (move_flags & MOVE_NODROP) ? lift :
				get_highest_blocked(lift, tflags) + 1;
					// Don't allow fall of > max_drop.
		if (lift - new_lift > max_drop)
			{		// Map-editing?  Suspend in air there.
			if (move_flags & MOVE_MAPEDIT)
				new_lift = lift - max_drop;
			else
				return 1;
			}
		int new_high = get_lowest_blocked (new_lift, tflags);
	
		// Make sure that where we want to go is tall enough for us
		if (new_high != -1 && new_high < (new_lift + height)) 
			return 1;
		}
	
	// Found a new place to go, lets test if we can actually move there
	
	// Lift 0 tests
	if (new_lift == 0)
	{
		if (move_flags & MOVE_MAPEDIT)
			return 0;	// Map-editor, so anything is okay.
		int ter = 0;
		Check_terrain (obj_list, tx, ty, ter);
		if (ter & 2)		// Water
		{
			if (move_flags & (MOVE_FLY+MOVE_SWIM))
				return 0;
			else
				return 1;
		}
		else if (ter & 1)	// Land
		{
			if (move_flags & (MOVE_FLY|MOVE_WALK))
				return 0;
			else
				return 1;
		}
		else if (ter & 4)	// Blocked
		{
			if (move_flags & MOVE_FLY)
				return 0;
			else
				return 1;
		}
		else	// Other
			return 0;
	}
	else if (move_flags & (MOVE_FLY|MOVE_WALK))
		return 0;

	return 1;
}

/*
 *	Activate nearby eggs.
 */

void Chunk_cache::activate_eggs
	(
	Game_object *obj,		// Object (actor) that's near.
	Map_chunk *chunk,	// Chunk this is attached to.
	int tx, int ty, int tz,		// Tile (absolute).
	int from_tx, int from_ty,	// Tile walked from.
	unsigned short eggbits,		// Eggs[tile].
	bool now			// Do them immediately.
	)
	{
	int i;				// Go through eggs.
	for (i = 0; i < 8*(int)sizeof(eggbits) - 1 && eggbits; 
						i++, eggbits = eggbits >> 1)
		{
		Egg_object *egg;
		if ((eggbits&1) && i < egg_objects.size() &&
		    (egg = egg_objects[i]) &&
		    egg->is_active(obj, tx, ty, tz, from_tx, from_ty))
			egg->activate(obj, now);
		}
	if (eggbits)			// Check 15th bit.
		{			// DON'T use an iterator here, since
					//   the list can change as eggs are
					//   activated, causing a CRASH!
		int sz = egg_objects.size();
		for (  ; i < sz; i++)
			{
			Egg_object *egg = egg_objects[i];
			if (egg && egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
				egg->activate(obj, now);
			}
		}
	}

/*
 *	Find door blocking a given tile.
 */

Game_object *Chunk_cache::find_door
	(
	Tile_coord tile
	)
	{
	for (Game_object_vector::iterator it = doors.begin();
						it != doors.end(); ++it)
		if ((*it)->blocks(tile))
			return *it;	// Found it.
	return 0;
	}

/*
 *	Create list for a given chunk.
 */

Map_chunk::Map_chunk
	(
	int chunkx, int chunky		// Absolute chunk coords.
	) : objects(0), terrain(0), first_nonflat(0), ice_dungeon(0x00),
	    dungeon_levels(0), cache(0), roof(0),
	    dungeon_lights(0), non_dungeon_lights(0),
	    cx(chunkx), cy(chunky), from_below(0), from_right(0),
	    from_below_right(0)
	{
	}

/*
 *	Delete all objects contained within.
 */

Map_chunk::~Map_chunk
	(
	)
	{
	delete cache;
	delete [] dungeon_levels;
	}

/*
 *	Set terrain.  Even if the terrain is the same, it still reloads the
 *	'flat' objects.
 */

void Map_chunk::set_terrain
	(
	Chunk_terrain *ter
	)
	{
	if (terrain)
		{
		terrain->remove_client();
					// Remove objs. from terrain.
		Game_object_vector removes;
		{			// Separate scope for Object_iterator.
		Object_iterator it(get_objects());
		Game_object *each;
		while ((each = it.get_next()) != 0)
					// Kind of nasty, I know:
			if (each->as_terrain())
				removes.push_back(each);
		}
		for (Game_object_vector::const_iterator it=removes.begin(); 
						it!=removes.end(); ++it)
			(*it)->remove_this();
		}
	terrain = ter;
	terrain->add_client();
					// Get RLE objects in chunk.
	for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
			{
			ShapeID id = ter->get_flat(tilex, tiley);
			Shape_frame *shape = id.get_shape();
			if (shape && shape->is_rle())
				{
				int shapenum = id.get_shapenum(),
				    framenum = id.get_framenum();
				Shape_info& info = id.get_info();
				Game_object *obj = info.is_animated() ?
					new Animated_object(shapenum,
					    	framenum, tilex, tiley)
					: new Terrain_game_object(shapenum,
					    	framenum, tilex, tiley);
				add(obj);
				}
			}
	}

/*
 *	Add rendering dependencies for a new object.
 */

void Map_chunk::add_dependencies
	(
	Game_object *newobj,		// Object to add.
	Ordering_info& newinfo		// Info. for new object's ordering.
	)
	{
	Game_object *obj;		// Figure dependencies.
	Nonflat_object_iterator next(this);
	while ((obj = next.get_next()) != 0)
		{
		//cout << "Here " << __LINE__ << " " << obj << endl;
		/* Compare returns -1 if lt, 0 if dont_care, 1 if gt. */
		int newcmp = Game_object::compare(newinfo, obj);
		int cmp = newcmp == -1 ? 1 : newcmp == 1 ? 0 : -1;
		if (!cmp)		// Bigger than this object?
			{
			newobj->dependencies.put(obj);
			obj->dependors.put(newobj);
			}
		else if (cmp == 1)	// Smaller than?
			{
			obj->dependencies.put(newobj);
			newobj->dependors.put(obj);
			}
		}
	}

/*
 *	Add rendering dependencies for a new object to another chunk.
 *	NOTE:  This is static.
 *
 *	Output:	->chunk that was checked.
 */

inline Map_chunk *Map_chunk::add_outside_dependencies
	(
	int cx, int cy,			// Chunk to check.
	Game_object *newobj,		// Object to add.
	Ordering_info& newinfo		// Info. for new object's ordering.
	)
	{
	Map_chunk *chunk = gmap->get_chunk(cx, cy);
	chunk->add_dependencies(newobj, newinfo);
	return chunk;
	}

/*
 *	Add a game object to a chunk's list.
 *
 *	Newobj's cx and cy fields are set to this chunk.
 */

void Map_chunk::add
	(
	Game_object *newobj		// Object to add.
	)
	{
	newobj->cx = get_cx();		// Set object's chunk.
	newobj->cy = get_cy();
	Ordering_info ord(gwin, newobj);
					// Put past flats.
	if (first_nonflat)
		objects.insert_before(newobj, first_nonflat);
	else
		objects.append(newobj);
					// Not flat?
	if (newobj->get_lift() || ord.info.get_3d_height())
		{			// Deal with dependencies.
		int ty = newobj->get_ty();
					// First this chunk.
		add_dependencies(newobj, ord);
		if (from_below)		// Overlaps from below?
			add_outside_dependencies(cx, INCR_CHUNK(cy), 
							newobj, ord);
		if (from_right)		// Overlaps from right?
			add_outside_dependencies(INCR_CHUNK(cx), cy, 
								newobj, ord);
		if (from_below_right)
			add_outside_dependencies(INCR_CHUNK(cx), 
					INCR_CHUNK(cy), newobj, ord);
					// See if newobj extends outside.
#if 0
		bool ext_left = (newobj->get_tx() - ord.xs) < -1 && cx > 0;
		bool ext_above = (newobj->get_ty() - ord.ys) < -1 && cy > 0;
#else	/* Let's try boundary. YES.  This helps with statues through roofs!*/
		bool ext_left = (newobj->get_tx() - ord.xs) < 0 && cx > 0;
		bool ext_above = (newobj->get_ty() - ord.ys) < 0 && cy > 0;
#endif
		if (ext_left)
			{
			add_outside_dependencies(DECR_CHUNK(cx), cy, 
						newobj, ord)->from_right++;
			if (ext_above)
				add_outside_dependencies(DECR_CHUNK(cx), 
							 DECR_CHUNK(cy),
					newobj, ord)->from_below_right++;
			}
		if (ext_above)
			add_outside_dependencies(cx, DECR_CHUNK(cy),
					newobj, ord)->from_below++;
		first_nonflat = newobj;	// Inserted before old first_nonflat.
		}
	if (cache)			// Add to cache.
		cache->update_object(this, newobj, 1);
	if (ord.info.is_light_source())	// Count light sources.
		if (dungeon_levels && is_dungeon(newobj->get_tx(),
							newobj->get_ty()))
			dungeon_lights++;
		else
			non_dungeon_lights++;
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		{
		if (ord.info.get_shape_class() == Shape_info::building)
			roof = 1;
		}
	}

/*
 *	Add an egg.
 */

void Map_chunk::add_egg
	(
	Egg_object *egg
	)
	{
	add(egg);			// Add it normally.
	egg->set_area();
// Messed up Moonshade after Banes if (cache)		// Add to cache.
	need_cache()->update_egg(this, egg, 1);
	}

/*
 *	Remove an egg.
 */

void Map_chunk::remove_egg
	(
	Egg_object *egg
	)
	{
	remove(egg);			// Remove it normally.
	if (cache)			// Remove from cache.
		cache->update_egg(this, egg, 0);
	}

/*
 *	Remove a game object from this list.  The object's cx and cy fields
 *	are set to invalid #'s (255,255).
 */

void Map_chunk::remove
	(
	Game_object *remove
	)
	{
	if (cache)			// Remove from cache.
		cache->update_object(this, remove, 0);
	remove->clear_dependencies();	// Remove all dependencies.
	Game_map *gmap = gwin->get_map();
	Shape_info& info = remove->get_info();
					// See if it extends outside.
	int frame = remove->get_framenum(), tx = remove->get_tx(),
					ty = remove->get_ty();
#if 0
	bool ext_left = (tx - info.get_3d_xtiles(frame)) < -1 && cx > 0;
	bool ext_above = (ty - info.get_3d_ytiles(frame)) < -1 && cy > 0;
#else	/* Let's try boundary. YES.  Helps with statues through roofs. */
	bool ext_left = (tx - info.get_3d_xtiles(frame)) < 0 && cx > 0;
	bool ext_above = (ty - info.get_3d_ytiles(frame)) < 0 && cy > 0;
#endif
	if (ext_left)
		{
		gmap->get_chunk(cx - 1, cy)->from_below_right--;
		if (ext_above)
			gmap->get_chunk(cx - 1, cy - 1)->from_below_right--;
		}
	if (ext_above)
		gmap->get_chunk(cx, cy - 1)->from_below--;
	if (info.is_light_source())	// Count light sources.
		if (dungeon_levels && is_dungeon(tx, ty))
			dungeon_lights--;
		else
			non_dungeon_lights--;
	if (remove == first_nonflat)	// First nonflat?
		{			// Update.
		first_nonflat = remove->get_next();
		if (first_nonflat == objects.get_first())
			first_nonflat = 0;
		}
	objects.remove(remove);		// Remove from list.
	remove->set_invalid();		// No longer part of world.
	}

/*
 *	Is a given rectangle of tiles blocked at a given lift?
 *
 *	Output: 1 if so, else 0.
 *		If 0 (tile is free), new_lift contains the new height that
 *		   an actor will be at if he walks onto the tile.
 */

int Map_chunk::is_blocked
	(
	int height,			// Height (along lift) to check.
	int lift,			// Starting lift.
	int startx, int starty,		// Starting tile coords.
	int xtiles, int ytiles,		// Width, height in tiles.
	int& new_lift,			// New lift returned.
	const int move_flags,
	int max_drop,			// Max. drop/rise allowed.
	int max_rise			// Max. rise, or -1 to use old beha-
					//   viour (max_drop if FLY, else 1).
	)
	{
	Game_map *gmap = gwin->get_map();
	int tx, ty;
	new_lift = 0;
	startx %= c_num_tiles;		// Watch for wrapping.
	starty %= c_num_tiles;
	int stopy = (starty + ytiles)%c_num_tiles, 
	    stopx = (startx + xtiles)%c_num_tiles;
	for (ty = starty; ty != stopy; ty = INCR_TILE(ty))
		{			// Get y chunk, tile-in-chunk.
		int cy = ty/c_tiles_per_chunk, rty = ty%c_tiles_per_chunk;
		for (tx = startx; tx != stopx; tx = INCR_TILE(tx))
			{
			int this_lift;
			Map_chunk *olist = gmap->get_chunk(
					tx/c_tiles_per_chunk, cy);
			olist->setup_cache();
			if (olist->is_blocked(height, lift, 
				tx%c_tiles_per_chunk,
				rty, this_lift, move_flags, max_drop,max_rise))
				return (1);
					// Take highest one.
			new_lift = this_lift > new_lift ?
					this_lift : new_lift;
			}
		}
	return (0);
	}

/*
 *	Check an absolute tile position.
 *
 *	Output:	1 if blocked, 0 otherwise.
 *		Tile.tz may be updated for stepping onto square.
 */

int Map_chunk::is_blocked
	(
	Tile_coord& tile,
	int height,			// Height in tiles to check.
	const int move_flags,
	int max_drop,			// Max. drop/rise allowed.
	int max_rise			// Max. rise, or -1 to use old beha-
					//   viour (max_drop if FLY, else 1).
	)
	{
					// Get chunk tile is in.
	Game_map *gmap = gwin->get_map();
	Map_chunk *chunk = gmap->get_chunk_safely(
			tile.tx/c_tiles_per_chunk, tile.ty/c_tiles_per_chunk);
	if (!chunk)			// Outside the world?
		return 0;		// Then it's not blocked.
	chunk->setup_cache();		// Be sure cache is present.
	int new_lift;			// Check it within chunk.
	if (chunk->is_blocked(height, tile.tz, tile.tx%c_tiles_per_chunk,
		    tile.ty%c_tiles_per_chunk, new_lift, move_flags, max_drop,
								max_rise))
		return (1);
	tile.tz = new_lift;
	return (0);
	}

/*
 *	This one is used to see if an object of dims. possibly > 1X1 can
 *	step onto an adjacent square.
 */

int Map_chunk::is_blocked
	(
					// Object dims:
	int xtiles, int ytiles, int ztiles,
	Tile_coord from,		// Stepping from here.
	Tile_coord& to,			// Stepping to here.  Tz updated.
	const int move_flags,
	int max_drop,			// Max drop/rise allowed.
	int max_rise			// Max. rise, or -1 to use old beha-
					//   viour (max_drop if FLY, else 1).
	)
	{
	int vertx0, vertx1;		// Get x-coords. of vert. block
					//   to right/left.
	int horizx0, horizx1;		// Get x-coords of horiz. block
					//   above/below.
	int verty0, verty1;		// Get y-coords of horiz. block
					//   above/below.
	int horizy0, horizy1;		// Get y-coords of vert. block
					//   to right/left.
					// !Watch for wrapping.
	horizx0 = (to.tx + 1 - xtiles + c_num_tiles)%c_num_tiles;
	horizx1 = INCR_TILE(to.tx);
	if (Tile_coord::gte(to.tx, from.tx))		// Moving right?
		{			// Start to right of hot spot.
		vertx0 = INCR_TILE(from.tx);
		vertx1 = INCR_TILE(to.tx);	// Stop past dest.
		}
	else				// Moving left?
		{
		vertx0 = (to.tx + 1 - xtiles + c_num_tiles)%c_num_tiles;
		vertx1 = (from.tx + 1 - xtiles + c_num_tiles)%c_num_tiles;
		}
	verty0 = (to.ty + 1 - ytiles + c_num_tiles)%c_num_tiles;
	verty1 = INCR_TILE(to.ty);
	if (Tile_coord::gte(to.ty, from.ty))		// Moving down?
		{			// Start below hot spot.
		horizy0 = INCR_TILE(from.ty);	
		horizy1 = INCR_TILE(to.ty);	// End past dest.
		if (to.ty != from.ty)	// Includes bottom of vert. area.
			verty1 = DECR_TILE(verty1);
		}
	else				// Moving up?
		{
		horizy0 = (to.ty + 1 - ytiles + c_num_tiles)%c_num_tiles;
		horizy1 = (from.ty + 1 - ytiles + c_num_tiles)%c_num_tiles;
					// Includes top of vert. area.
		verty0 = INCR_TILE(verty0);
		}
	int x, y;			// Go through horiz. part.
	int new_lift = from.tz;
	int new_lift0 = -1;		// All lift changes must be same.
#ifdef DEBUG
	assert(Tile_coord::gte(horizy1, horizy0));
	assert(Tile_coord::gte(horizx1, horizx0));
	assert(Tile_coord::gte(verty1, verty0));
	assert(Tile_coord::gte(vertx1, vertx0));
#endif
	for (y = horizy0; y != horizy1; y = INCR_TILE(y))
		{			// Get y chunk, tile-in-chunk.
		int cy = y/c_tiles_per_chunk, rty = y%c_tiles_per_chunk;
		for (x = horizx0; x != horizx1; x = INCR_TILE(x))
			{
			Map_chunk *olist = gmap->get_chunk(
					x/c_tiles_per_chunk, cy);
			olist->setup_cache();
			int rtx = x%c_tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty,
				new_lift, move_flags, max_drop, max_rise))
				return 1;
			if (new_lift != from.tz)
				if (new_lift0 == -1)
					new_lift0 = new_lift;
				else if (new_lift != new_lift0)
					return (1);
			}
		}
					// Do vert. block.
	for (x = vertx0; x != vertx1; x = INCR_TILE(x))
		{			// Get x chunk, tile-in-chunk.
		int cx = x/c_tiles_per_chunk, rtx = x%c_tiles_per_chunk;
		for (y = verty0; y != verty1; y = INCR_TILE(y))
			{
			Map_chunk *olist = gmap->get_chunk(
					cx, y/c_tiles_per_chunk);
			olist->setup_cache();
			int rty = y%c_tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty,
				new_lift, move_flags, max_drop, max_rise))
				return 1;
			if (new_lift != from.tz)
				if (new_lift0 == -1)
					new_lift0 = new_lift;
				else if (new_lift != new_lift0)
					return (1);
			}
		}
	to.tz = new_lift;
	return (0);			// All clear.
	}

/*
 *	Get the list of tiles in a square perimeter around a given tile.
 *
 *	Output:	List (8*dist) of tiles, starting in Northwest corner and going
 *		   clockwise.  List is on heap.
 */

static Tile_coord *Get_square
	(
	Tile_coord& pos,		// Center of square.
	int dist			// Distance to perimeter (>0)
	)
	{
	Tile_coord *square = new Tile_coord[8*dist];
					// Upper left corner:
	square[0] = Tile_coord(DECR_TILE(pos.tx, dist), 
					DECR_TILE(pos.ty, dist), pos.tz);
	int i;				// Start with top row.
	int len = 2*dist + 1;
	int out = 1;
	for (i = 1; i < len; i++, out++)
		square[out] = Tile_coord(INCR_TILE(square[out - 1].tx),
			square[out - 1].ty, pos.tz);
					// Down right side.
	for (i = 1; i < len; i++, out++)
		square[out] = Tile_coord(square[out - 1].tx,
			INCR_TILE(square[out - 1].ty), pos.tz);
					// Bottom, going back to left.
	for (i = 1; i < len; i++, out++)
		square[out] = Tile_coord(DECR_TILE(square[out - 1].tx),
			square[out - 1].ty, pos.tz);
					// Left side, going up.
	for (i = 1; i < len - 1; i++, out++)
		square[out] = Tile_coord(square[out - 1].tx,
			DECR_TILE(square[out - 1].ty), pos.tz);
	return square;
	}

/*
 *	Check a spot against the 'where' paramater to find_spot.
 *
 *	Output:	true if it passes.
 */

inline bool Check_spot
	(
	Map_chunk::Find_spot_where where,
	int tx, int ty, int tz
	)
	{
	Game_map *gmap = Game_window::get_instance()->get_map();
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	Map_chunk *chunk = gmap->get_chunk_safely(cx, cy);
	return (where == Map_chunk::inside) == 
				(chunk->is_roof(tx % c_tiles_per_chunk, 
					ty % c_tiles_per_chunk, tz) < 31);
	}

/*
 *	Find a free area for an object of a given shape, looking outwards.
 *
 *	Output:	Tile if successful, else (-1, -1, -1).
 */

Tile_coord Map_chunk::find_spot
	(
	Tile_coord pos,			// Starting point.
	int dist,			// Distance to look outwards.  (0 means
					//   only check 'pos'.
	int shapenum,			// Shape, frame to find spot for.
	int framenum,
	int max_drop,			// Allow to drop by this much.
	int dir,			// Preferred direction (0-7), or -1 for
					//   random.
	Find_spot_where where		// Inside/outside.
	)
	{
	Shape_info& info = ShapeID::get_info(shapenum);
	int xs = info.get_3d_xtiles(framenum);
	int ys = info.get_3d_ytiles(framenum);
	int zs = info.get_3d_height();
					// The 'MOVE_FLY' flag really means
					//   we can look upwards by max_drop.
	const int mflags = MOVE_WALK|MOVE_FLY;
	int new_lift;
					// Start with original position.
	if (!Map_chunk::is_blocked(zs, pos.tz, pos.tx - xs + 1,
		pos.ty - ys + 1, xs, ys, new_lift, mflags, max_drop))
		return Tile_coord(pos.tx, pos.ty, new_lift);
	if (dir < 0)
		dir = rand()%8;		// Choose dir. randomly.
	dir = (dir + 1)%8;		// Make NW the 0 point.
	for (int d = 1; d <= dist; d++)	// Look outwards.
		{
		int square_cnt = 8*d	;// # tiles in square's perim.
					// Get square (starting in NW).
		Tile_coord *square = Get_square(pos, d);
		int index = dir*d;	// Get index of preferred spot.
					// Get start of preferred range.
		index = (index - d/2 + square_cnt)%square_cnt;
		for (int cnt = square_cnt; cnt; cnt--, index++)
			{
			Tile_coord& p = square[index%square_cnt];
			if (!Map_chunk::is_blocked(zs, p.tz, p.tx - xs + 1,
				p.ty - ys + 1, xs, ys, new_lift, mflags,
								max_drop) &&
			    (where == anywhere || 
				  Check_spot(where, p.tx, p.ty, new_lift)))
				{	// Use tile before deleting.
				Tile_coord ret(p.tx, p.ty, new_lift);
				delete [] square;
				return ret;
				}
			}
		delete [] square;
		}
	return Tile_coord(-1, -1, -1);
	}

/*
 *	Find a free area for an object (usually an NPC) that we want to
 *	approach a given position.
 *
 *	Output:	Tile if successful, else (-1, -1, -1).
 */

Tile_coord Map_chunk::find_spot
	(
	Tile_coord pos,			// Starting point.
	int dist,			// Distance to look outwards.  (0 means
					//   only check 'pos'.
	Game_object *obj,		// Object that we want to move.
	int max_drop,			// Allow to drop by this much.
	Find_spot_where where		// Inside/outside.
	)
	{
	Tile_coord t2 = obj->get_tile();
					// Get direction from pos. to object.
	int dir = (int) Get_direction(pos.ty - t2.ty, t2.tx - pos.tx);
	return find_spot(pos, dist, obj->get_shapenum(), obj->get_framenum(),
			max_drop, dir, where);
	}

/*
 *	Find all desired objects within a given rectangle.
 *
 *	Output:	# found, appended to vec.
 */

int Map_chunk::find_in_area
	(
	Game_object_vector& vec,	// Returned here.
	Rectangle area,			// Area to search.
	int shapenum,
	int framenum
	)
	{
	int savesize = vec.size();
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// (Tiles within intersected chunk).
	int eachcx, eachcy;
	Game_map *gmap = gwin->get_map();
	while (next_chunk.get_next(tiles, eachcx, eachcy))
		{
		Map_chunk *chunk = gmap->get_chunk_safely(eachcx, eachcy);
		if (!chunk)
			continue;
		Object_iterator next(chunk->objects);
		Game_object *each;
		while ((each = next.get_next()) != 0)
			if (each->get_shapenum() == shapenum &&
			    each->get_framenum() == framenum &&
			    tiles.has_point(each->get_tx(), each->get_ty()))
				vec.append(each);
		}
	return vec.size() - savesize;
	}



/*
 *	Test all nearby eggs when you've teleported in.
 */

void Map_chunk::try_all_eggs
	(
	Game_object *obj,		// Object (actor) that's near.
	int tx, int ty, int tz,		// Tile (absolute).
	int from_tx, int from_ty	// Tile walked from.
	)
	{
	static int norecurse = 0;	// NO recursion here.
	if (norecurse)
		return;
	norecurse++;
	Game_map *gmap = gwin->get_map();
	Tile_coord pos = obj->get_tile();
	const int dist = 32;		// See if this works okay.
	Rectangle area(pos.tx - dist, pos.ty - dist, 2*dist, 2*dist);
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// (Ignored).
	int eachcx, eachcy;
	Egg_vector eggs(40);		// Get them here first, as activating
					//   an egg could affect chunk's list.
	while (next_chunk.get_next(tiles, eachcx, eachcy))
		{
		Map_chunk *chunk = gmap->get_chunk_safely(eachcx, eachcy);
		if (!chunk)
			continue;
		chunk->setup_cache();	// I think we should do this.
		Object_iterator next(chunk->objects);
		Game_object *each;
		while ((each = next.get_next()) != 0)
			if (each->is_egg())
				{
				Egg_object *egg = (Egg_object *) each;
					// Music eggs are causing problems.
				if (egg->get_type() != Egg_object::jukebox &&
					// And don't teleport a 2nd time.
				    egg->get_type() != Egg_object::teleport &&
			    	    egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
					eggs.push_back(egg);
				}
		}
	for (Egg_vector::const_iterator it = eggs.begin(); it != eggs.end();
									++it)
		(*it)->activate(obj);
	norecurse--;
	}

/*
 *	Add a rectangle of dungeon tiles (but only if higher!).
 */

void Map_chunk::add_dungeon_levels
	(
	Rectangle& tiles, unsigned int lift
	)
{
	if (!dungeon_levels)
	{			// First one found.
		dungeon_levels = new unsigned char[256/2];
		memset(dungeon_levels, 0, 256/2);
	}
	int endy = tiles.y + tiles.h, endx = tiles.x + tiles.w;
	for (int ty = tiles.y; ty < endy; ty++)
	{
		for (int tx = tiles.x; tx < endx; tx++)
		{
			int tnum = (ty*c_tiles_per_chunk + tx)/2;

			if (tx % 2)
			{
				dungeon_levels[tnum] &= 0x0F;
				dungeon_levels[tnum] |= lift << 4;
			}
			else
			{
				dungeon_levels[tnum] &= 0xF0;
				dungeon_levels[tnum] |= lift;
			}
		}
	}
}

/*
 *	Set up the dungeon levels (after IFIX objects read).
 */

void Map_chunk::setup_dungeon_levels
	(
	)
{
	Game_map *gmap = gwin->get_map();

	Object_iterator next(objects);
	Game_object *each;
	while ((each = next.get_next()) != 0)
	{
		int shnum = each->get_shapenum();
					// Test for mountain-tops.
		if (shnum == 983 || shnum == 969 || shnum == 183 ||
				shnum == 182 || shnum == 180 || shnum == 324 || 
				(shnum == 941 && Game::get_game_type() == SERPENT_ISLE))
		{
			Rectangle area = each->get_footprint();

					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(area);
			Rectangle tiles;// Rel. tiles.
			int cx, cy;
			while (next_chunk.get_next(tiles, cx, cy))
				gmap->get_chunk(cx, cy)->add_dungeon_levels(
								tiles, each->get_lift());
		}			// Ice Dungeon Pieces in SI
		else if (Game::get_game_type() == SERPENT_ISLE && (
			shnum == 436 || shnum == 437 || shnum == 444 ||
			shnum == 448  || shnum == 466 || shnum == 477))
		{
			// HACK ALERT! This gets 320x200 to work, but it is a hack
			// This is not exactly accurate.
			ice_dungeon |= 1 << ( (each->get_tx()>>3) + 2*(each->get_ty()>>3) );

			Rectangle area = each->get_footprint();

					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(area);
			Rectangle tiles;// Rel. tiles.
			int cx, cy;
			while (next_chunk.get_next(tiles, cx, cy))
				gmap->get_chunk(cx, cy)->add_dungeon_levels(
								tiles, each->get_lift());
		}
	}
	if (dungeon_levels)		// Recount lights.
		{
		dungeon_lights = non_dungeon_lights = 0;
		next.reset();
		while ((each = next.get_next()) != 0)
			if (each->get_info().is_light_source())
				if (is_dungeon(each->get_tx(), each->get_ty()))
					dungeon_lights++;
				else
					non_dungeon_lights++;
		}
}

/*
 *	Recursively apply gravity over a given rectangle that is known to be
 *	unblocked below a given lift.
 */

void Map_chunk::gravity
	(
	Rectangle area,			// Unblocked tiles (in abs. coords).
	int lift			// Lift where tiles are free.
	)
	{
	Game_object_vector dropped(20);	// Gets list of objs. that dropped.
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// Rel. tiles.  Not used.
	int cx, cy, new_lift;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Map_chunk *chunk = gmap->get_chunk(cx, cy);
		Object_iterator objs(chunk->objects);
		Game_object *obj;
		while ((obj = objs.get_next()) != 0)
			{		// We DO want NPC's to fall.
			if (!obj->is_dragable() && 
						!obj->get_info().is_npc())
				continue;
			Tile_coord t = obj->get_tile();
					// Get footprint.
			Rectangle foot = obj->get_footprint();
					// Above area?
			if (t.tz >= lift && foot.intersects(area) &&
					// Unblocked below itself?
			    !is_blocked(1, t.tz - 1, foot.x, foot.y,
					foot.w, foot.h, new_lift,
						MOVE_ALL_TERRAIN, 0) &&
			    new_lift < t.tz)
				dropped.push_back(obj);
			}
		}
	Game_object_vector::const_iterator it;
					// Drop each one found.
	for (it = dropped.begin(); it != dropped.end(); ++it)
		{
		Game_object *obj = *it;
		Tile_coord t = obj->get_tile();
					// Get footprint.
		Rectangle foot = obj->get_footprint();
					// Let drop as far as possible.
		if (!is_blocked(1, t.tz - 1, foot.x, foot.y,
			foot.w, foot.h, new_lift, MOVE_ALL_TERRAIN, 100) &&
			    				new_lift < t.tz)
			{		// Drop & recurse.
			obj->move(t.tx, t.ty, new_lift);
			gravity(foot, obj->get_lift() +
					obj->get_info().get_3d_height());
			}
		}
#if 0
	for (it = dropped.begin(); it != dropped.end(); ++it)
		{			// Recurse on each one.
		Game_object *obj = *it;
					// Get footprint.
		Rectangle foot = obj->get_footprint();
		gravity(foot, obj->get_lift() +
					obj->get_info().get_3d_height());
		}
#endif
	}

/*
 *  Finds if there is a 'roof' above lift in tile (tx, ty)
 *  of the chunk. Point is taken 4 above lift
 *
 *  Roof can be any object, not just a literal roof
 *
 *  Output: height of the roof.
 *  A return of 31 means no roof
 *
 */
int Map_chunk::is_roof(int tx, int ty, int lift)
{
#if 1		/* Might be lying on bed at lift==2. */
	int height = get_lowest_blocked (lift+4, tx, ty);
#else		/* But this is even worse! */
	int height = get_lowest_blocked (lift+2, tx, ty);
#endif
	if (height == -1) return 31;
	return height;
}

void Map_chunk::kill_cache()
{
	// Get rid of terrain
	if (terrain) terrain->remove_client();
	terrain = 0;

	// Now remove the cachce
	delete cache;
	cache = 0;

	// Delete dungeon bits
	delete [] dungeon_levels;
	dungeon_levels = 0;
}

int Map_chunk::get_obj_actors(Game_object_vector &removes, Actor_vector &actors)
{
	int buf_size = 0;
	bool failed = false;

	// Separate scope for Object_iterator.
	Object_iterator it(get_objects());
	Game_object *each;
	while ((each = it.get_next()) != 0)
	{
		Actor *actor = each->as_actor();

		// Normal objects and monsters
		if (actor == 0 || (each->is_monster() && each->get_flag(Obj_flags::is_temporary))) {
			removes.push_back(each);
			int ireg_size = each->get_ireg_size();

			if (ireg_size < 0) failed = true;
			else buf_size += ireg_size;
		}
			// Actors/NPCs here
		else {
			actors.push_back(actor);
		}
	}

	return failed?-1:buf_size;
}
