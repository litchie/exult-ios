/*
 *	gamemap.h - Game map data.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "exult_constants.h"
#include "vec.h"
#include "flags.h"
#ifndef ALPHA_LINUX_CXX
#  include <cassert>
#endif
#include <string>	// STL string
#include <iostream>
#include <fstream>

class Map_chunk;
class Chunk_terrain;
class Map_patch_collection;
class Ireg_game_object;
class Egg_object;
class Shape_info;
class Shapes_vga_file;

/*
 *	The game map:
 */
class Game_map
	{
					// Flat chunk areas:
	Exult_vector<Chunk_terrain *> chunk_terrains;
	bool read_all_terrain;		// True if we've read them all.
	bool map_modified;		// True if any map changes from
					//   map-editing.
					// Chunk_terrain index for each chunk:
	short terrain_map[c_num_chunks][c_num_chunks];
					// A list of objects in each chunk:
	Map_chunk *objects[c_num_chunks][c_num_chunks];
	bool schunk_read[144]; 		// Flag for reading in each "ifix".
	bool schunk_modified[144];	// Flag for modified "ifix".
	std::ifstream *chunks;		// "u7chunks" file.
	Map_patch_collection *map_patches;

	Map_chunk *create_chunk(int cx, int cy);
	Chunk_terrain *read_terrain(int chunk_num);
public:
	Game_map();
	~Game_map();
	void init();			// Set up map.
	void clear();			// Clear out old map.
	void read_map_data();		// Read in 'ifix', 'ireg', etc.
	inline short get_terrain_num(int cx, int cy) const
		{ return terrain_map[cx][cy]; }
	inline Map_patch_collection *get_map_patches()
		{ return map_patches; }
	void set_map_modified()
		{ map_modified = true; }
	bool was_map_modified() const
		{ return map_modified; }
	bool is_chunk_read(int cx, int cy)
		{ return cx < c_num_chunks && cy < c_num_chunks &&
			schunk_read[12*(cy/c_chunks_per_schunk) +
						cx/c_chunks_per_schunk]; }
	void set_ifix_modified(int cx, int cy)
		{ 
		map_modified = true;
		schunk_modified[12*(cy/c_chunks_per_schunk) +
					cx/c_chunks_per_schunk] = true;
		}
					// Get/create objs. list for a chunk.
	Map_chunk *get_chunk(int cx, int cy)
		{
		assert((cx >= 0) && (cx < c_num_chunks) && 
		        (cy >= 0) && (cy < c_num_chunks));
		Map_chunk *list = objects[cx][cy];
		return list ? list : create_chunk(cx, cy);
		}
	Map_chunk *get_chunk_safely(int cx, int cy)
		{
		Map_chunk *list;
		return (cx >= 0 && cx < c_num_chunks && 
			cy >= 0 && cy < c_num_chunks ? 
			((list = objects[cx][cy]) != 0 ? list : 
						create_chunk(cx, cy)) : 0);
		}
					// Get "map" superchunk objs/scenery.
	void get_map_objects(int schunk);
					// Get "chunk" objects/scenery.
	void get_chunk_objects(int cx, int cy);
	void get_all_terrain();		// Read in all terrains.
					// Get desired terrain.
	Chunk_terrain *get_terrain(int tnum)
		{
		Chunk_terrain *ter = chunk_terrains[tnum];
		return ter ? ter : read_terrain(tnum);
		}
	inline int get_num_chunk_terrains() const
		{ return chunk_terrains.size(); }
					// Set new terrain chunk.
	void set_chunk_terrain(int cx, int cy, int chunknum);
					// Get ifixxxx/iregxx name.
	static char *get_schunk_file_name(char *prefix,
						int schunk, char *fname);
	void write_static();		// Write to 'static' directory.
					// Write (static) map objects.
	void write_ifix_objects(int schunk);
					// Get "ifix" objects for a superchunk.
	void get_ifix_objects(int schunk);
					// Get "ifix" objs. for given chunk.
	void get_ifix_chunk_objects(std::ifstream& ifix, long filepos, int cnt,
							int cx, int cy);
					// Write scheduled script for obj.
	static void write_scheduled(std::ostream& ireg, Game_object *obj,
						bool write_mark = false);
	void write_ireg();		// Write modified ireg files.
					// Write moveable objects to file.
	void write_ireg_objects(int schunk);
					// Get moveable objects.
	void get_ireg_objects(int schunk);
					// Read scheduled script(s) for obj.
	void read_special_ireg(std::istream& ireg, Game_object *obj);
	void read_ireg_objects(std::istream& ireg, int scx, int scy,
					Game_object *container = 0,
			unsigned long flags = (1<<Obj_flags::okay_to_take));
	Ireg_game_object *create_ireg_object(Shape_info& info, int shnum, 
			int frnum, int tilex, int tiley, int lift);
	Ireg_game_object *create_ireg_object(int shnum, int frnum);
					// Get all superchunk objects.
	void get_superchunk_objects(int schunk);
					// Locate chunk with desired terrain.
	bool locate_terrain(int tnum, int& cx, int& cy, bool upwards = false);
	bool swap_terrains(int tnum);	// Swap adjacent terrain #'s.
					// Insert new terrain after 'tnum'.
	bool insert_terrain(int tnum, bool dup = false);
	void commit_terrain_edits();	// End terrain-editing mode.
	void abort_terrain_edits();
					// Search entire game for unused.
	void find_unused_shapes(unsigned char *found, int foundlen);
	};

#endif
