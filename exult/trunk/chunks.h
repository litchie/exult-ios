/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Chunks.h - Chunks (16x16 tiles) on the map.
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

#ifndef INCL_CHUNKS
#define INCL_CHUNKS	1

#include "objs.h"

/*
 *	Data cached for a chunk to speed up processing, but which doesn't need
 *	to be saved to disk:
 */
class Chunk_cache
	{
	Chunk_object_list	*obj_list;
	unsigned char setup_done;	// Already setup.
	unsigned short blocked[256];	// For each tile, a bit for each lift
					//   level if it's blocked by an obj.
					// In the process of implementing:+++++
	EggVector egg_objects;		// ->eggs which influence this chunk.
	unsigned short eggs[256];	// Bit #i (0-14) set means that the
					//   tile is within egg_object[i]'s
					//   influence.  Bit 15 means it's 1 or
					//   more of 
					//   egg_objects[15-(num_eggs-1)].
	friend class Chunk_object_list;
	Chunk_cache();			// Create empty one.
	~Chunk_cache();
	int get_num_eggs()
		{ return egg_objects.size(); }
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, bool set);
					// Add/remove object.
	void update_object(Chunk_object_list *chunk,
						Game_object *obj, bool add);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, bool add);
					// Add egg.
	void update_egg(Chunk_object_list *chunk, Egg_object *egg, bool add);
					// Set up with chunk's data.
	void setup(Chunk_object_list *chunk);
					// Set blocked tile's bits.
	void set_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] |= 
						(((1 << ztiles) - 1) << lift);
		}
					// Clear blocked tile's bits.
	void clear_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] &= 
					~(((1 << ztiles) - 1) << lift);
		}
					// Get highest lift blocked below a
					//   given level for a desired tile.
	int get_highest_blocked(int lift, unsigned short tflags);
	int get_highest_blocked(int lift, int tx, int ty);
	int get_lowest_blocked(int lift, unsigned short tflags);
	int get_lowest_blocked(int lift, int tx, int ty);
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift,
				const int move_flags, int max_drop = 1);
					// Activate eggs nearby.
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
			int tx, int ty, int tz,
			int from_tx, int from_ty, unsigned short eggbits);
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
			int tx, int ty, int tz, int from_tx, int from_ty)
		{
		unsigned short eggbits = eggs[
			(ty%tiles_per_chunk)*tiles_per_chunk + 
							(tx%tiles_per_chunk)];
		if (eggbits)
			activate_eggs(obj, chunk, tx, ty, tz,
						from_tx, from_ty,  eggbits);
		}
	};

/*
 *	Game objects are stored in a list for each chunk, sorted from top-to-
 *	bottom, left-to-right.
 */
class Chunk_object_list
	{
	ShapeID flats[256];		// Flat (non-RLE) shapes.  The unused
					//   are "invalid" entries.
	Object_list objects;		// ->first in list of all objs.  'Flat'
					//   obs. (lift=0,ht=0) stored 1st.
	Game_object *first_nonflat;	// ->first nonflat in 'objects'.
	unsigned char *dungeon_bits;	// A 'dungeon' bit flag for each tile.
	Npc_actor *npcs;		// List of NPC's in this chunk.
					//   (Managed by Npc_actor class.)
	Chunk_cache *cache;		// Data for chunks near player.
	unsigned char roof;		// 1 if a roof present.
	unsigned char light_sources;	// # light sources in chunk.
	unsigned char cx, cy;		// Absolute chunk coords. of this.
	void add_dungeon_bits(Rectangle& tiles);
	void add_dependencies(Game_object *newobj,
					class Ordering_info& newinfo);
public:
	friend class Npc_actor;
	friend class Object_iterator;
	friend class Flat_object_iterator;
	friend class Nonflat_object_iterator;
	friend class Object_iterator_backwards;
	Chunk_object_list(int chunkx, int chunky);
	~Chunk_object_list();		// Delete everything in chunk.
	void add(Game_object *obj);	// Add an object.
	void add_egg(Egg_object *egg);	// Add/remove an egg.
	void remove_egg(Egg_object *egg);
	void remove(Game_object *obj);	// Remove an object.
					// Apply gravity over given area.
	static void gravity(Rectangle area, int lift);
					// Is there a roof? Returns height
	int is_roof(int tx, int ty, int lift);
	int get_cx() const
		{ return cx; }
	int get_cy() const
		{ return cy; }
	Npc_actor *get_npcs()		// Get ->first npc in chunk.
		{ return npcs; }
	int get_light_sources() const	// Get #lights.
		{ return light_sources; }
					// Set/get flat shape.
	void set_flat(int tilex, int tiley, ShapeID id)
		{ flats[16*tiley + tilex] = id; }
	ShapeID get_flat(int tilex, int tiley) const
		{ return flats[16*tiley + tilex]; }
					// Get/create cache.
	Chunk_cache *need_cache()
		{ 
		return cache ? cache 
				: (cache = new Chunk_cache()); 
		}
	void setup_cache()
		{ 
		if (!cache || !cache->setup_done)
			need_cache()->setup(this);
		}
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, bool set)
		{ need_cache()->set_blocked(startx, starty, endx, endy,
							lift, ztiles, set); }
					// Get highest lift blocked.
	int get_highest_blocked(int lift, int tx, int ty)
		{ return need_cache()->get_highest_blocked(lift, tx, ty); }
	int get_lowest_blocked(int lift, int tx, int ty)
		{ return need_cache()->get_lowest_blocked(lift, tx, ty); }
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift,
					const int move_flags, int max_drop = 1)
		{ return cache->is_blocked(height, lift, tx, ty, new_lift,
						move_flags, max_drop); }
					// Check range.
	static int is_blocked(int height, int lift, int startx, int starty,
		int xtiles, int ytiles, int& new_lift, const int move_flags, 
							int max_drop = 1);
					// Check absolute tile.
	static int is_blocked(Tile_coord& tile, int height = 1,
		const int move_flags = MOVE_ALL_TERRAIN, int max_drop = 1);
					// Check for > 1x1 object.
	static int is_blocked(int xtiles, int ytiles, int ztiles,
			Tile_coord from, Tile_coord to, const int move_flags);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, bool add)
		{ need_cache()->set_egged(egg, tiles, add); }
	void activate_eggs(Game_object *obj, int tx, int ty, int tz,
						int from_tx, int from_ty)
		{ need_cache()->activate_eggs(obj, 
					this, tx, ty, tz, from_tx, from_ty);}
					// Use this when teleported in.
	static void try_all_eggs(Game_object *obj, int tx, int ty, int tz,
						int from_tx, int from_ty);
	void setup_dungeon_bits();	// Set up after IFIX objs. read.
	int has_dungeon()		// Any tiles within dungeon?
		{ return dungeon_bits != 0; }
					// NOTE:  The following should only be
					//   called if has_dungeon()==1.
	int in_dungeon(int tx, int ty)	// Is object within dungeon?
		{
		int tnum = ty*tiles_per_chunk + tx;
		return dungeon_bits[tnum/8] & (1 << (tnum%8));
		}
	int in_dungeon(Game_object *obj)// Is object within dungeon?
		{ return in_dungeon(obj->get_tx(), obj->get_ty()); }
	};

#endif
