/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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
#include "gamewin.h"
#include "shapeinf.h"
#include "citerate.h"
#include "egg.h"
#include "objiter.h"
#include "objs.h"
#include "ordinfo.h"
#include "game.h"

using std::memset;

/*
 *	Create the cached data storage for a chunk.
 */

Chunk_cache::Chunk_cache
	(
	) : setup_done(0), egg_objects(4)
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
				set_blocked_tile(x, y, lift, ztiles);
		}
	else
		{
		for (int y = starty; y <= endy; y++)
			for (int x = startx; x <= endx; x++)
				clear_blocked_tile(x, y, lift, ztiles);
		}
	}

/*
 *	Add/remove an object to/from the cache.
 */

void Chunk_cache::update_object
	(
	Chunk_object_list *chunk,
	Game_object *obj,
	bool add				// 1 to add, 0 to remove.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(obj);
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
			set_blocked_tile(endx, endy, lift, ztiles);
		else
			clear_blocked_tile(endx, endy, lift, ztiles);
		return;
		}
	Tile_coord endpt = obj->get_abs_tile_coord();
	Rectangle footprint(endpt.tx - xtiles + 1, endpt.ty - ytiles + 1, 
							xtiles, ytiles);
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(footprint);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		gwin->get_objects(cx, cy)->set_blocked(tiles.x, tiles.y, 
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
	Chunk_object_list *chunk,
	Egg_object *egg,
	bool add				// 1 to add, 0 to remove.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
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
			gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
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
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator bottoms(bottom);
	while (bottoms.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator lefts(left);
	while (lefts.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator rights(right);
	while (rights.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);

	}

/*
 *	Create the cached data for a chunk.
 */

void Chunk_cache::setup
	(
	Chunk_object_list *chunk
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
	
	setup_done = 1;
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

inline int Chunk_cache::get_highest_blocked
	(
	int lift,			// Look below this lift.
	unsigned short tflags		// Flags for desired tile.
	)
	{
	int i;				// Look downwards.
	for (i = lift - 1; i >= 0 && !(tflags & (1<<i)); i--)
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
	unsigned short tflags		// Flags for desired tile.
	)
	{
	int i;				// Look upward.
	for (i = lift; i < 16 && !(tflags & (1<<i)); i++)
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
	Game_window *gwin,
	Chunk_object_list *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& terrain			// Sets: bit0 if land, bit1 if water,
					//   bit2 if solid.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (!flat.is_invalid())
		{
		if (gwin->get_info(flat.get_shapenum()).is_water())
			terrain |= 2;
		else if (gwin->get_info(flat.get_shapenum()).is_solid())
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
	int max_drop			// Max. drop allowed.
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
	unsigned short tflags = blocked[ty*c_tiles_per_chunk + tx];

	int new_high;
	if (tflags & (1 << lift))	// Something there?
		{			// Not flying?
		if (!(move_flags & MOVE_FLY))		
			{
			new_lift = lift + 1;	// Maybe we can step up.
			new_high = get_lowest_blocked (new_lift, tflags);
			if (new_lift > 15)
				return (1);	// In sky
			else if (tflags & (1 << new_lift))
				return (1);	// Next step up also blocked
			else if (new_high != -1 && 
					new_high < (new_lift + height))
				return (1);	// Blocked by something above
			}
		else			// Flying.
			{
			new_lift = lift;
			while (tflags & (1 << new_lift) && 
						new_lift <= lift+max_drop)
				{
				new_lift++;	// Maybe we can step up.
				new_high = get_lowest_blocked(
							new_lift, tflags);
				if (new_lift > 15)
					return (1);	// In sky
				else if (new_high != -1 && new_high < 
							(new_lift + height))
					// Blocked by something above
					return (1);
				}
			}
		}
	else
		{			// Not blocked.
					// See if going down if not levitating.
		new_lift =  (move_flags & MOVE_NODROP) ? lift :
				get_highest_blocked(lift, tflags) + 1;
		new_high = get_lowest_blocked (new_lift, tflags);
	
		// Make sure that where we want to go is tall enough for us
		if (new_high != -1 && new_high < (new_lift + height)) return 1;
	
					// Don't allow fall of > max_drop.
		if (lift - new_lift > max_drop) return 1;
		}
		
	
	// Found a new place to go, lets test if we can actually move there
	
	// Lift 0 tests
	if (new_lift == 0)
	{
		int ter = 0;
		Check_terrain (Game_window::get_game_window(), obj_list, 
								tx, ty, ter);
		if (ter & 2)	// Water
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
			return (move_flags & MOVE_FLY) ? 0 : 1;
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
	Chunk_object_list *chunk,	// Chunk this is attached to.
	int tx, int ty, int tz,		// Tile (absolute).
	int from_tx, int from_ty,	// Tile walked from.
	unsigned short eggbits		// Eggs[tile].
	)
	{
					// Get ->usecode machine.
	Usecode_machine *usecode = 
				Game_window::get_game_window()->get_usecode();
	int i;				// Go through eggs.
	for (i = 0; i < 8*(int)sizeof(eggbits) - 1 && eggbits; 
						i++, eggbits = eggbits >> 1)
		{
		Egg_object *egg;
		if ((eggbits&1) && i < egg_objects.size() &&
		    (egg = egg_objects[i]) &&
		    egg->is_active(obj, tx, ty, tz, from_tx, from_ty))
			egg->activate(usecode, obj);
		}
	if (eggbits)			// Check 15th bit.
		{
		for (Egg_vector::const_iterator it = egg_objects.begin() + i;
					it != egg_objects.end(); ++it)
			{
			Egg_object *egg = *it;
			if (egg && egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
				egg->activate(usecode, obj);
			}
		}
	}

/*
 *	Create list for a given chunk.
 */

Chunk_object_list::Chunk_object_list
	(
	int chunkx, int chunky		// Absolute chunk coords.
	) : objects(0), first_nonflat(0), dungeon_bits(0),
	    npcs(0), cache(0), roof(0), light_sources(0),
	    cx(chunkx), cy(chunky)
	{
	}

/*
 *	Delete all objects contained within.
 */

Chunk_object_list::~Chunk_object_list
	(
	)
	{
	delete cache;
	delete [] dungeon_bits;
	}

/*
 *	Add rendering dependencies for a new object.
 */

void Chunk_object_list::add_dependencies
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
		int cmp = Game_object::lt(newinfo, obj);
		if (!cmp)		// Bigger than this object?
			{
			if (newobj->cx == obj->cx && newobj->cy == obj->cy)
				{
				newobj->dependencies.put(obj);
				obj->dependors.put(newobj);
				}
			}
		else if (cmp == 1)	// Smaller than?
			{
			obj->dependencies.put(newobj);
			newobj->dependors.put(obj);
			}
		}
	}

/*
 *	Add a game object to a chunk's list.
 *
 *	Newobj's cx and cy fields are set to this chunk.
 */

void Chunk_object_list::add
	(
	Game_object *newobj		// Object to add.
	)
	{
	newobj->cx = get_cx();		// Set object's chunk.
	newobj->cy = get_cy();
	Game_window *gwin = Game_window::get_game_window();
	Ordering_info ord(gwin, newobj);
					// Put past flats.
	if (first_nonflat)
		objects.insert_before(newobj, first_nonflat);
	else
		objects.append(newobj);
					// Not flat?
	if (newobj->get_lift() || ord.info.get_3d_height())
		{			// Deal with dependencies.
		if (ord.xs == 1 && ord.ys == 1)	// Simplest case?
			add_dependencies(newobj, ord);
		else
			{
			Rectangle footprint(ord.tx - ord.xs + 1, 
					ord.ty - ord.ys + 1, ord.xs, ord.ys);
					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(footprint);
			Rectangle tiles;// (Ignored).
			int eachcx, eachcy;
			while (next_chunk.get_next(tiles, eachcx, eachcy))
				if (eachcx <= cx && eachcy <= cy)
					gwin->get_objects(eachcx, eachcy)->
						add_dependencies(newobj, ord);
			}
		first_nonflat = newobj;	// Inserted before old first_nonflat.
		}
			// +++++Maybe should skip test, do update_object(...).
	if (cache)			// Add to cache.
		cache->update_object(this, newobj, 1);
	if (ord.info.is_light_source())	// Count light sources.
		light_sources++;
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		{
		if (ord.info.get_shape_class() == Shape_info::building)
			roof = 1;
		}
	}

#if 0
/*
 *	Add a flat, fixed object.
 */

void Chunk_object_list::add_flat
	(
	Game_object *newobj		// Should be 0 height, lift=0.
	)
	{
	newobj->cx = get_cx();		// Set object's chunk.
	newobj->cy = get_cy();
					// Just put in front.
	objects = newobj->insert_in_chain(objects);
	}
#endif
/*
 *	Add an egg.
 */

void Chunk_object_list::add_egg
	(
	Egg_object *egg
	)
	{
	add(egg);			// Add it normally.
	egg->set_area();
	if (cache)			// Add to cache.
		cache->update_egg(this, egg, 1);
	}

/*
 *	Remove an egg.
 */

void Chunk_object_list::remove_egg
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
 *	are left set to this chunk.
 */

void Chunk_object_list::remove
	(
	Game_object *remove
	)
	{
	if (cache)			// Remove from cache.
		cache->update_object(this, remove, 0);
	remove->clear_dependencies();	// Remove all dependencies.
	Shape_info& info = Game_window::get_game_window()->get_info(remove);
	if (info.is_light_source())	// Count light sources.
		light_sources--;
	if (remove == first_nonflat)	// First nonflat?
		{			// Update.
		first_nonflat = remove->get_next();
		if (first_nonflat == objects.get_first())
			first_nonflat = 0;
		}
	objects.remove(remove);		// Remove from list.
	}

/*
 *	Is a given rectangle of tiles blocked at a given lift?
 *
 *	Output: 1 if so, else 0.
 *		If 0 (tile is free), new_lift contains the new height that
 *		   an actor will be at if he walks onto the tile.
 */

int Chunk_object_list::is_blocked
	(
	int height,			// Height (along lift) to check.
	int lift,			// Starting lift.
	int startx, int starty,		// Starting tile coords.
	int xtiles, int ytiles,		// Width, height in tiles.
	int& new_lift,			// New lift returned.
	const int move_flags,
	int max_drop			// Max. drop allowed.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int tx, ty;
	new_lift = 0;
	int stopy = starty + ytiles, stopx = startx + xtiles;
	for (ty = starty; ty < stopy; ty++)
		{			// Get y chunk, tile-in-chunk.
		int cy = ty/c_tiles_per_chunk, rty = ty%c_tiles_per_chunk;
		for (tx = startx; tx < stopx; tx++)
			{
			int this_lift;
			Chunk_object_list *olist = gwin->get_objects(
					tx/c_tiles_per_chunk, cy);
			olist->setup_cache();
			if (olist->is_blocked(height, lift, tx%c_tiles_per_chunk,
						rty, this_lift, move_flags, max_drop))
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

int Chunk_object_list::is_blocked
	(
	Tile_coord& tile,
	int height,			// Height in tiles to check.
	const int move_flags,
	int max_drop
	)
	{
					// Get chunk tile is in.
	Game_window *gwin = Game_window::get_game_window();
	Chunk_object_list *chunk = gwin->get_objects(
			tile.tx/c_tiles_per_chunk, tile.ty/c_tiles_per_chunk);
	chunk->setup_cache();		// Be sure cache is present.
	int new_lift;			// Check it within chunk.
	if (chunk->is_blocked(height, tile.tz, tile.tx%c_tiles_per_chunk,
				tile.ty%c_tiles_per_chunk, new_lift, move_flags, max_drop))
		return (1);
	tile.tz = new_lift;
	return (0);
	}

/*
 *	This one is used to see if an object of dims. possibly > 1X1 can
 *	step onto an adjacent square.
 */

int Chunk_object_list::is_blocked
	(
					// Object dims:
	int xtiles, int ytiles, int ztiles,
	Tile_coord from,		// Stepping from here.
	Tile_coord& to,			// Stepping to here.  Tz updated.
	const int move_flags,
	int max_drop			// Max drop allowed.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int vertx0, vertx1;		// Get x-coords. of vert. block
					//   to right/left.
	int horizx0, horizx1;		// Get x-coords of horiz. block
					//   above/below.
	int verty0, verty1;		// Get y-coords of horiz. block
					//   above/below.
	int horizy0, horizy1;		// Get y-coords of vert. block
					//   to right/left.
	horizx0 = to.tx + 1 - xtiles;
	horizx1 = to.tx;
	if (to.tx >= from.tx)		// Moving right?
		{
		vertx0 = from.tx + 1;	// Start to right of hot spot.
		vertx1 = to.tx;		// End at dest.
		}
	else				// Moving left?
		{
		vertx0 = to.tx + 1 - xtiles;
		vertx1 = from.tx - xtiles;
		}
	verty0 = to.ty + 1 - ytiles;
	verty1 = to.ty;
	if (to.ty >= from.ty)		// Moving down?
		{
		horizy0 = from.ty + 1;	// Start below hot spot.
		horizy1 = to.ty;	// End at dest.
		if (to.ty != from.ty)
			verty1--;	// Includes bottom of vert. area.
		}
	else				// Moving up?
		{
		horizy0 = to.ty + 1 - ytiles;
		horizy1 = from.ty - ytiles;
		verty0++;		// Includes top of vert. area.
		}
	int x, y;			// Go through horiz. part.
	int new_lift = from.tz;
	int new_lift0 = -1;		// All lift changes must be same.
	for (y = horizy0; y <= horizy1; y++)
		{			// Get y chunk, tile-in-chunk.
		int cy = y/c_tiles_per_chunk, rty = y%c_tiles_per_chunk;
		for (x = horizx0; x <= horizx1; x++)
			{
			Chunk_object_list *olist = gwin->get_objects(
					x/c_tiles_per_chunk, cy);
			olist->setup_cache();
			int rtx = x%c_tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty,
					new_lift, move_flags, max_drop))
				return 1;
			if (new_lift != from.tz)
				if (new_lift0 == -1)
					new_lift0 = new_lift;
				else if (new_lift != new_lift0)
					return (1);
			}
		}
					// Do vert. block.
	for (x = vertx0; x <= vertx1; x++)
		{			// Get x chunk, tile-in-chunk.
		int cx = x/c_tiles_per_chunk, rtx = x%c_tiles_per_chunk;
		for (y = verty0; y <= verty1; y++)
			{
			Chunk_object_list *olist = gwin->get_objects(
					cx, y/c_tiles_per_chunk);
			olist->setup_cache();
			int rty = y%c_tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty,
					new_lift, move_flags, max_drop))
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
 *	Test all nearby eggs when you've teleported in.
 */

void Chunk_object_list::try_all_eggs
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
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord pos = obj->get_abs_tile_coord();
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
		Chunk_object_list *chunk = gwin->get_objects_safely(
							eachcx, eachcy);
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
			    	    egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
					eggs.push_back(egg);
				}
		}
	for (Egg_vector::const_iterator it = eggs.begin(); it != eggs.end();
									++it)
		(*it)->activate(gwin->get_usecode(), obj);
	norecurse--;
	}

/*
 *	Add a rectangle of dungeon tiles.
 */

void Chunk_object_list::add_dungeon_bits
	(
	Rectangle& tiles
	)
	{
	if (!dungeon_bits)
		{			// First one found.
		dungeon_bits = new unsigned char[256/8];
		memset(dungeon_bits, 0, 256/8);
		}
	int endy = tiles.y + tiles.h, endx = tiles.x + tiles.w;
	for (int ty = tiles.y; ty < endy; ty++)
		for (int tx = tiles.x; tx < endx; tx++)
			{
			int tnum = ty*c_tiles_per_chunk + tx;
			dungeon_bits[tnum/8] |= (1 << (tnum%8));
			}
	}

/*
 *	Set up the dungeon flags (after IFIX objects read).
 */

void Chunk_object_list::setup_dungeon_bits
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Special SI weirdness:
	if (Game::get_game_type() == SERPENT_ISLE &&
					// Knight's Test dungeon:
	    cx >= 54 && cx <= 60 && cy >= 134 && cy <= 139)
		{			// Set whole thing.
		if (!dungeon_bits)
			dungeon_bits = new unsigned char[256/8];
		memset(dungeon_bits, 0xff, 256/8);
		return;
		}
	Object_iterator next(objects);
	Game_object *each;
	while ((each = next.get_next()) != 0)
		{
		int shnum = each->get_shapenum();
					// Test for mountain-tops.
		if (shnum == 983 || shnum == 969 || shnum == 183 ||
		    shnum == 182 || shnum == 180 || shnum == 324)
			{
			Rectangle area = each->get_footprint();
					// Try to fix Courage Test:
			if (shnum == 969 && each->get_framenum() == 12)
				area.enlarge(1);
					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(area);
			Rectangle tiles;// Rel. tiles.
			int cx, cy;
			while (next_chunk.get_next(tiles, cx, cy))
				gwin->get_objects(cx, cy)->add_dungeon_bits(
								tiles);
			}
		}
	}

/*
 *	Recursively apply gravity over a given rectangle that is known to be
 *	unblocked below a given lift.
 */

void Chunk_object_list::gravity
	(
	Rectangle area,			// Unblocked tiles (in abs. coords).
	int lift			// Lift where tiles are free.
	)
	{
	Game_object_vector dropped(20);		// Gets list of objs. that dropped.
	Game_window *gwin = Game_window::get_game_window();
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// Rel. tiles.  Not used.
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		Object_iterator objs(chunk->objects);
		Game_object *obj;
		while ((obj = objs.get_next()) != 0)
			{
			if (!obj->is_dragable())
				continue;
			Tile_coord t = obj->get_abs_tile_coord();
					// Get footprint.
			Rectangle foot = obj->get_footprint();
			int new_lift;
					// Above area?
			if (t.tz >= lift && foot.intersects(area) &&
					// Unblocked below itself?  Let drop.
			    !is_blocked(1, t.tz - 1, foot.x, foot.y,
					foot.w, foot.h, new_lift,
						MOVE_ALL_TERRAIN, 100))
				{	// Save it, and drop it.
				dropped.push_back(obj);
				obj->move(t.tx, t.ty, new_lift);
				}
			}
		}
	for (Game_object_vector::const_iterator it = dropped.begin(); 
						it != dropped.end(); ++it)
		{			// Recurse on each one.
		Game_object *obj = *it;
					// Get footprint.
		Rectangle foot = obj->get_footprint();
		gravity(foot, obj->get_lift() +
					gwin->get_info(obj).get_3d_height());
		}
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
int Chunk_object_list::is_roof(int tx, int ty, int lift)
{
#if 1		/* Might be lying on bed at lift==2. */
	int height = get_lowest_blocked (lift+4, tx, ty);
#else		/* But this is even worse! */
	int height = get_lowest_blocked (lift+2, tx, ty);
#endif
	if (height == -1) return 31;
	return height;
}


/*
 *  Is object within dungeon?
 */
int Chunk_object_list::in_dungeon(Game_object *obj) // Is object within dungeon?
{
	return in_dungeon(obj->get_tx(), obj->get_ty());
}
