/**
 **	Gamemap.cc - X-windows Ultima7 map browser.
 **/

/*
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <cstring>
#  include <cstdarg>
#  include <cstdio>
#endif

#include "gamemap.h"
#include "objs.h"
#include "chunks.h"
#include "mappatch.h"
#include "fnames.h"
#include "utils.h"
#include "shapeinf.h"
#include "objiter.h"
#include "Flex.h"
#include "exceptions.h"
#include "animate.h"
#include "barge.h"
#include "spellbook.h"
#include "virstone.h"
#include "egg.h"
#include "jawbone.h"
#include "actors.h"	/* For Dead_body, which should be moved. */
#include "ucsched.h"
#include "gamewin.h"	/* With some work, could get rid of this. */
#include "bodies.h"
#include "game.h"
#include "effects.h"
#include "objiter.cc"	/* Yes we #include the .cc here on purpose! Please don't "fix" this */
#include "databuf.h"
#include <fstream>

#ifndef UNDER_CE
using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ifstream;
using std::ios;
using std::memcpy;
using std::memset;
using std::ofstream;
using std::rand;
using std::strcmp;
using std::strcpy;
using std::string;
using std::strlen;
using std::srand;
using std::vector;
#endif

/*
 *	Create a chunk.
 */

Map_chunk *Game_map::create_chunk
	(
	int cx, int cy
	)
	{
	return (objects[cx][cy] = new Map_chunk(cx, cy));
	}

/*
 *	Read in a terrain chunk.
 */

Chunk_terrain *Game_map::read_terrain
	(
	int chunk_num			// Want this one from u7chunks.
	)
	{
	assert(chunk_num >= 0 && chunk_num < chunk_terrains.size());
	unsigned char buf[16*16*2];	
	chunks->seekg(chunk_num * 512);
	chunks->read(reinterpret_cast<char*>(buf), sizeof(buf));
	Chunk_terrain *ter = new Chunk_terrain(&buf[0]);
	chunk_terrains.put(chunk_num, ter);
	return ter;
	}

/*
 *	Create game window.
 */

Game_map::Game_map
	(
	) : 
            chunk_terrains(0), read_all_terrain(false), 
	    map_modified(false),
	    map_patches(new Map_patch_collection), chunks(0)
	{
	}

/*
 *	Deleting map.
 */

Game_map::~Game_map
	(
	)
	{
	clear();			// Delete all objects, chunks.
	delete chunks;
	delete map_patches;
	}

/*
 *	Initialize for new/restored game.
 */

void Game_map::init
	(
	)
	{
	if (chunks)
		delete chunks;
	chunks = new ifstream;
	int num_chunk_terrains;
	bool patch_exists = is_system_path_defined("<PATCH>");
	if (patch_exists && U7exists(PATCH_U7CHUNKS))
		U7open(*chunks, PATCH_U7CHUNKS);
	else try
		{
		U7open(*chunks, U7CHUNKS);
		}
	catch(const file_exception & f)
		{
		if (!Game::is_editing() ||	// Ok if map-editing.
		    !patch_exists)	// But only if patch exists.
			throw f;
		ofstream ochunks;	// Create one in 'patch'.
		U7open(ochunks, PATCH_U7CHUNKS);
		unsigned char buf[16*16*2];	
		memset(&buf[0], 0, sizeof(buf));
		ochunks.write((char *) buf, sizeof(buf));
		ochunks.close();
		U7open(*chunks, PATCH_U7CHUNKS);
		}
					// Get to end so we can get length.
	chunks->seekg(0, ios::end);
					// 2 bytes/tile.
	num_chunk_terrains = chunks->tellg()/
				(c_tiles_per_chunk*c_tiles_per_chunk*2);
					// Resize list to hold all.
	chunk_terrains.resize(num_chunk_terrains);
	read_all_terrain = map_modified = false;
	std::ifstream u7map;		// Read in map.
	bool nomap = false;
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_U7MAP))
		U7open(u7map, PATCH_U7MAP);
	else try 
		{
		U7open(u7map, U7MAP);
		}
	catch(const file_exception & f)
		{
		if (!Game::is_editing())	// Ok if map-editing.
			throw f;
		nomap = true;
		}
	for (int schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
	{			// Read in the chunk #'s.
		unsigned char buf[16*16*2];
		if (nomap)
			memset(&buf[0], 0, sizeof(buf));
		else
			u7map.read(reinterpret_cast<char *>(buf), sizeof(buf));
		int scy = 16*(schunk/12);// Get abs. chunk coords.
		int scx = 16*(schunk%12);
		uint8 *mapdata = buf;
					// Go through chunks.
		for (int cy = 0; cy < 16; cy++)
			for (int cx = 0; cx < 16; cx++)
				terrain_map[scx+cx][scy+cy] = Read2(mapdata);
	}
	u7map.close();
					// Clear object lists, flags.
	// No casting _should_ be necessary at this point.
	// Who needs this?
	memset(reinterpret_cast<char*>(objects), 0, sizeof(objects));
	memset(reinterpret_cast<char*>(schunk_read), 0, sizeof(schunk_read));
	memset(reinterpret_cast<char*>(schunk_modified), 0, 
						sizeof(schunk_modified));

	memset(schunk_cache, 0, sizeof (schunk_cache));
	memset(schunk_cache_sizes, -1, sizeof(schunk_cache_sizes));
	}

/*
 *	Clear out world's contents.  Should be used during a 'restore'.
 */

void Game_map::clear
	(
	)
	{
					// Delete all chunks (& their objs).
	for (int y = 0; y < c_num_chunks; y++)
		for (int x = 0; x < c_num_chunks; x++)
			{
			delete objects[x][y];
			objects[x][y] = 0;
			}
	int cnt = chunk_terrains.size();
	int i;
	for (i = 0; i < cnt; i++)
		delete chunk_terrains[i];
	chunk_terrains.resize(0);
	delete chunks;			// Close 'u7chunks'.
	chunks = 0;
	map_modified = false;
					// Clear 'read' flags.
	memset(reinterpret_cast<char*>(schunk_read), 0, sizeof(schunk_read));
	memset(reinterpret_cast<char*>(schunk_modified), 0, 
						sizeof(schunk_modified));

	for (i = 0; i < 144; i++) delete [] schunk_cache[i];
	memset(schunk_cache, 0, sizeof (schunk_cache));
	memset(schunk_cache_sizes, -1, sizeof(schunk_cache_sizes));

	}

/*
 *	Read in superchunk data to cover the screen.
 */

void Game_map::read_map_data
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int scrolltx = gwin->get_scrolltx(), scrollty = gwin->get_scrollty();
	int w = gwin->get_width(), h = gwin->get_height();
					// Start one tile to left.
	int firstsx = (scrolltx - 1)/c_tiles_per_schunk, 
	    firstsy = (scrollty - 1)/c_tiles_per_schunk;
					// End 8 tiles to right.
	int lastsx = (scrolltx + (w + c_tilesize - 2)/c_tilesize + 
				c_tiles_per_chunk/2)/c_tiles_per_schunk;
	int lastsy = (scrollty + (h + c_tilesize - 2)/c_tilesize + 
				c_tiles_per_chunk/2)/c_tiles_per_schunk;
					// Watch for wrapping.
	int stopsx = (lastsx + 1)%c_num_schunks,
	    stopsy = (lastsy + 1)%c_num_schunks;
					// Read in "map", "ifix" objects for
					//  all visible superchunks.
	for (int sy = firstsy; sy != stopsy; sy = (sy + 1)%c_num_schunks)
		for (int sx = firstsx; sx != stopsx; 
						sx = (sx + 1)%c_num_schunks)
			{
					// Figure superchunk #.
			int schunk = 12*sy + sx;
					// Read it if necessary.
			if (!schunk_read[schunk])
				get_superchunk_objects(schunk);
			}
	}	

/*
 *	Get the map objects and scenery for a superchunk.
 */

void Game_map::get_map_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			get_chunk_objects(scx + cx, scy + cy);
	}

/*
 *	Read in terrain graphics data into window's image.  (May also be
 *	called during map-editing if the chunknum changes.)
 */

void Game_map::get_chunk_objects
	(
	int cx, int cy			// Chunk index within map.
	)
	{
					// Get list we'll store into.
	Map_chunk *chunk = get_chunk(cx, cy);
	int chunk_num = terrain_map[cx][cy];
	Chunk_terrain *ter = get_terrain(chunk_num);
	chunk->set_terrain(ter);
	}

/*
 *	Read in all terrain chunks (for editing).
 */

void Game_map::get_all_terrain
	(
	)
	{
	if (read_all_terrain)
		return;			// Already done.
	int num_chunk_terrains = chunk_terrains.size();
	for (int i = 0; i < num_chunk_terrains; i++)
		if (!chunk_terrains[i])
			read_terrain(i);
	read_all_terrain = true;
	}

/*
 *	Set a chunk to a new terrain (during map-editing).
 */

void Game_map::set_chunk_terrain
	(
	int cx, int cy,			// Coords. of chunk to change.
	int chunknum			// New chunk #.
	)
	{
	terrain_map[cx][cy] = chunknum;	// Set map.
	get_chunk_objects(cx, cy);	// Set chunk to it.
	}

/*
 *	Get the name of an ireg or ifix file.
 *
 *	Output:	->fname, where name is stored.
 */

char *Game_map::get_schunk_file_name
	(
	char *prefix,			// "ireg" or "ifix".
	int schunk,			// Superchunk # (0-143).
	char *fname			// Name is stored here.
	)
	{
	strcpy(fname, prefix);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	return (fname);
	}

/*
 *	Write out the 'static' map files.
 */

void Game_map::write_static
	(
	)
	{
	U7mkdir("<PATCH>", 0755);		// Create dir if not already there. Don't
									// use PATCHDAT define cause it has a
									// trailing slash

	int schunk;			// Write each superchunk to 'static'.
	for (schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
					// Only write what we've modified.
		if (schunk_modified[schunk])
			write_ifix_objects(schunk);
	int cnt = chunk_terrains.size();
	int i;				// Any terrains modified?
	for (i = 0; i < cnt; i++)
		if (chunk_terrains[i] && chunk_terrains[i]->is_modified())
			break;
	if (i < cnt)			// Got to update.
		{
		get_all_terrain();	// IMPORTANT:  Get all in memory.
		ofstream ochunks;	// Open file for chunks data.
					// This truncates the file.
		U7open(ochunks, PATCH_U7CHUNKS);
		for (i = 0; i < cnt; i++)
			{
			Chunk_terrain *ter = chunk_terrains[i];
			unsigned char data[512];
			if (ter)
				{
				ter->write_flats(data);
				ter->set_modified(false);
				}
			else
				{
				memset(&data[0], 0, 512);
				cerr << "NULL terrain.  U7chunks may be bad."
								<< endl;
				}
			ochunks.write(reinterpret_cast<char*>(data), 512);
			}
		if (!ochunks.good())
			throw file_write_exception(U7CHUNKS);
		ochunks.close();
		}
	std::ofstream u7map;		// Write out map.
	U7open(u7map, PATCH_U7MAP);
	for (schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
		{
		int scy = 16*(schunk/12);// Get abs. chunk coords.
		int scx = 16*(schunk%12);
		uint8 buf[16*16*2];
		uint8 *mapdata = buf;
					// Go through chunks.
		for (int cy = 0; cy < 16; cy++)
			for (int cx = 0; cx < 16; cx++)
				Write2(mapdata, terrain_map[scx+cx][scy+cy]);
		u7map.write(reinterpret_cast<char*>(buf), sizeof(buf));
		}
	if (!u7map.good())
		throw file_write_exception(U7MAP);
	u7map.close();
	map_modified = false;
	}

/*
 *	Write out one of the "u7ifix" files.
 *
 *	Output:	Errors reported.
 */

void Game_map::write_ifix_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ofstream ifix_stream;	// There it is.
	U7open(ifix_stream, get_schunk_file_name(PATCH_U7IFIX, schunk, fname));
	StreamDataSource ifix(&ifix_stream);
					// +++++Use game title.
	const int count = c_chunks_per_schunk*c_chunks_per_schunk;
	Flex::write_header(&ifix, "Exult",  count);
	uint8 table[2*count*4];
	uint8 *tptr = &table[0];
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Store file position in table.
			long start = ifix.getPos();
			Write4(tptr, start);
			Map_chunk *chunk = get_chunk(scx + cx,
							       scy + cy);
					// Restore original order (sort of).
			Object_iterator_backwards next(chunk);
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				obj->write_ifix(&ifix);
					// Store IFIX data length.
			Write4(tptr, ifix.getPos() - start);
			}
	ifix.seek(0x80);	// Write table.
	ifix.write(reinterpret_cast<char *>(&table[0]), sizeof(table));
	ifix_stream.flush();
	int result = ifix_stream.good();
	if (!result)
		throw file_write_exception(fname);
	schunk_modified[schunk] = false;
	return;
	}

/*
 *	Read in the objects for a superchunk from one of the "u7ifix" files.
 */

void Game_map::get_ifix_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ifstream ifix_stream;	// There it is.
	if (is_system_path_defined("<PATCH>") &&
					// First check for patch.
	    U7exists(get_schunk_file_name(PATCH_U7IFIX, schunk, fname)))
		U7open(ifix_stream, fname);
	else try
		{
		U7open(ifix_stream, get_schunk_file_name(U7IFIX, schunk, fname));
		}
		catch(const file_exception & f)
		{
		if (!Game::is_editing())	// Ok if map-editing.
			throw f;
		return;
		}
	StreamDataSource ifix(&ifix_stream);
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get to index entry for chunk.
			int chunk_num = cy*16 + cx;
			ifix.seek(0x80 + chunk_num*8);
					// Get location, length.
			long shapesoff = ifix.read4();
			if (!shapesoff) // Nothing there?
				continue;
			unsigned long shapeslen = ifix.read4();
			get_ifix_chunk_objects(&ifix, shapesoff, shapeslen/4,
				scx + cx, scy + cy);
			}
	}

/*
 *	Get the objects from one ifix chunk entry onto the screen.
 */

void Game_map::get_ifix_chunk_objects
	(
	DataSource* ifix,
	long filepos,			// Where chunk's data lies.
	int cnt,			// # entries (objects).
	int cx, int cy			// Absolute chunk #'s.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	ifix->seek(filepos);		// Get to actual shape.
					// Get buffer to hold entries' indices.
	unsigned char *entries = new unsigned char[4*cnt];
	unsigned char *ent = entries;	// Read them in.
	ifix->read(reinterpret_cast<char*>(entries), 4*cnt);
					// Get object list for chunk.
	Map_chunk *olist = get_chunk(cx, cy);
	for (int i = 0; i < cnt; i++, ent += 4)
		{
		Ifix_game_object *obj;
		int shnum = ent[2]+256*(ent[3]&3);
		Shape_info& info = ShapeID::get_info(shnum);
		if (info.is_animated() || info.has_sfx())
			obj = new Animated_ifix_object(ent);
		else
			obj = new Ifix_game_object(ent);

		olist->add(obj);
		}
	delete[] entries;		// Done with buffer.
	olist->setup_dungeon_levels();	// Should have all dungeon pieces now.
	}

/*
 *	Constants for IREG files:
 */
#define IREG_SPECIAL	255		// Precedes special entries.
#define IREG_UCSCRIPT	1		// Saved Usecode_script for object.
#define IREG_ENDMARK	2		// Just an 'end' mark.

/*
 *	Write out scheduled usecode for an object.
 */

void Game_map::write_scheduled
	(
	DataSource* ireg,
	Game_object *obj,
	bool write_mark			// Write an IREG_ENDMARK if true.
	)
	{
	for (Usecode_script *scr = Usecode_script::find(obj); scr;
					scr = Usecode_script::find(obj, scr))
		{
		unsigned char buf[256];
		int len = scr->save(buf, sizeof(buf));
		if (len < 0)
			cerr << "Error saving Usecode script" << endl;
		else if (len > 0)
			{
			ireg->write1(IREG_SPECIAL);
			ireg->write1(IREG_UCSCRIPT);
			ireg->write2(len);	// Store length.
			ireg->write(reinterpret_cast<char*>(buf), len);
			}
		}
	if (write_mark)
		{
		ireg->write1(IREG_SPECIAL);
		ireg->write1(IREG_ENDMARK);
		}
	}

/*
 *	Write modified 'u7ireg' files.
 */

void Game_map::write_ireg
	(
	)
	{

					// Write each superchunk to Iregxx.
	for (int schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
					// Only write what we've read.
		if (schunk_cache[schunk] && schunk_cache_sizes[schunk] >= 0) {
			// It's loaded in a memory buffer
			char fname[128];		// Set up name.
			ofstream ireg_stream;
			U7open(ireg_stream, get_schunk_file_name(U7IREG, schunk, fname));
			ireg_stream.write(schunk_cache[schunk], schunk_cache_sizes[schunk]);
		}
		else if (schunk_read[schunk]) {
			// It's active
			write_ireg_objects(schunk);
		}
	}

/*
 *	Write out one of the "u7ireg" files.
 *
 *	Output:	0 if error, which is reported.
 */

void Game_map::write_ireg_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ofstream ireg_stream;			// There it is.
	U7open(ireg_stream, get_schunk_file_name(U7IREG, schunk, fname));
	StreamDataSource ireg(&ireg_stream);
	write_ireg_objects (schunk, &ireg);
	ireg_stream.flush();
	int result = ireg_stream.good();
	if (!result) throw file_write_exception(fname);
	return;
	}


/*
 *	Write out one of the "u7ireg" files.
 *
 *	Output:	0 if error, which is reported.
 */

void Game_map::write_ireg_objects
	(
	int schunk,			// Superchunk # (0-143).
	DataSource *ireg
	)
	{
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
			Map_chunk *chunk = get_chunk(scx + cx,
							       scy + cy);
			Game_object *obj;
					// Restore original order (sort of).
			Object_iterator_backwards next(chunk);
			while ((obj = next.get_next()) != 0)
				obj->write_ireg(ireg);
			ireg->write2(0);// End with 2 0's.
			}
	}

/*
 *	Read in the objects for a superchunk from one of the "u7ireg" files.
 *	(These are the moveable objects.)
 */

void Game_map::get_ireg_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ifstream ireg_stream;			// There it is.
	DataSource *ireg = 0;

	if (schunk_cache[schunk] && schunk_cache_sizes[schunk] >= 0) {
		// No items
		if (schunk_cache_sizes[schunk] == 0) return;
		ireg = new BufferDataSource (schunk_cache[schunk], schunk_cache_sizes[schunk]);
#ifdef DEBUG
		std::cout << "Reading " << get_schunk_file_name(U7IREG, schunk, fname) << " from memory" << std::endl;
#endif
	}
	else {
		try
		{
			U7open(ireg_stream, get_schunk_file_name(U7IREG, schunk, fname));
		}
		catch(const file_exception & f)
		{
			return;			// Just don't show them.
		}
		ireg = new StreamDataSource (&ireg_stream);
	}
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	read_ireg_objects(ireg, scx, scy);
					// A fixup:
	if (schunk == 10*12 + 11 && Game::get_game_type() == SERPENT_ISLE)
		{			// Lever in SilverSeed:
		Game_object_vector vec;
		if (Game_object::find_nearby(vec, Tile_coord(2936, 2726, 0),
					787, 0, 0, c_any_qual, 5))
			vec[0]->move(2937, 2727, 2);
		}
	delete ireg;
	if (schunk_cache[schunk]) {
		delete [] schunk_cache[schunk];
		schunk_cache[schunk] = 0;
		schunk_cache_sizes[schunk] = -1;
	}

	}

/*
 *	Read in a 'special' IREG entry (one starting with 255).
 */

void Read_special_ireg
	(
	DataSource *ireg,
	Game_object *obj		// Last object read.
	)
	{
	int type = ireg->read1();		// Get type.
	int len = ireg->read2();		// Length of rest.
	unsigned char *buf = new unsigned char[len];
	ireg->read(reinterpret_cast<char*>(buf), len);
	if (type == IREG_UCSCRIPT)	// Usecode script?
		{
		Usecode_script *scr = Usecode_script::restore(obj, buf, len);
		if (scr)
			{
			scr->start(scr->get_delay());
#if 0
			COUT("Restored script for '" << 
				item_names[obj->get_shapenum()]
							<< "'" << endl);
			scr->print(cout); cout << endl;
#endif
			}
		}
	else
		cerr << "Unknown special IREG entry: " << type << endl;
	delete [] buf;
	}

/*
 *	Read in a 'special' IREG entry (one starting with 255).
 */

void Game_map::read_special_ireg
	(
	DataSource *ireg,
	Game_object *obj		// Last object read.
	)
	{
	unsigned char entlen;
	while ((entlen = ireg->peek()) == IREG_SPECIAL && !ireg->eof())
		{
		ireg->read1();		// Eat the IREG_SPECIAL.
		unsigned char type = ireg->peek();
		if (type == IREG_ENDMARK)
			{		// End of list.
			ireg->read1();
			return;
			}
		Read_special_ireg(ireg, obj);
		}
	}

/*
 *	Create an "egg".
 */

static Egg_object *Create_egg
	(
	unsigned char *entry,		// 1-byte ireg entry.
	bool animated
	)
	{
	int shnum = entry[2]+256*(entry[3]&3);
	int frnum = entry[3] >> 2;
	unsigned short type = entry[4] + 256*entry[5];
	int prob = entry[6];		// Probability (1-100).
	int data1 = entry[7] + 256*entry[8];
	int lift = entry[9] >> 4;
	int data2 = entry[10] + 256*entry[11];
	Egg_object *obj = animated ?
		new Animated_egg_object(shnum, frnum,
			entry[0]&0xf, entry[1]&0xf, lift, type, prob,
						data1, data2)
		: new Egg_object(shnum, frnum,
			entry[0]&0xf, entry[1]&0xf, lift, type, prob,
						data1, data2);
	return (obj);
	}

/*
 *	Containers and items classed as 'quality_flags' have a byte of flags.
 *	This routine returns them converted into Object_flags.
 */
inline unsigned long Get_quality_flags
	(
	unsigned char qualbyte		// Quality byte containing flags.
	)
	{
	return 	((qualbyte&1) << Obj_flags::invisible) |
		(((qualbyte>>3)&1) << Obj_flags::okay_to_take);
	}

/*
 *	Read a list of ireg objects.  They are either placed in the desired
 *	game chunk, or added to their container.
 */

void Game_map::read_ireg_objects
	(
	DataSource *ireg,			// File to read from.
	int scx, int scy,		// Abs. chunk coords. of superchunk.
	Game_object *container,		// Container, or null.
	unsigned long flags		// Usecode item flags.
	)
	{
	int entlen;			// Gets entry length.
	sint8 index_id = -1;
	Game_object *last_obj = 0;	// Last one read in this call.
	Game_window *gwin = Game_window::get_instance();
					// Go through entries.
	while (((entlen = ireg->read1(), !ireg->eof())))
		{

		// Skip 0's & ends of containers.

		if (!entlen || entlen == 1)
		{
			if (container)
				return;	// Skip 0's & ends of containers.
			else
				continue;
		}
		// Detect the 2 byte index id
		else if (entlen == 2)
		{
			index_id = (sint8) ireg->read2();
			continue;
		}
		else if (entlen == IREG_SPECIAL)
			{
			Read_special_ireg(ireg, last_obj);
			continue;
			}
					// Get copy of flags.
		unsigned long oflags = flags & ~(1<<Obj_flags::is_temporary);
		if (entlen != 6 && entlen != 10 && entlen != 12 && 
								entlen != 18)
			{
			long pos = ireg->getPos();
			cout << "Unknown entlen " << entlen << " at pos. " <<
					pos << endl;
			ireg->seek(pos + entlen);
			continue;	// Only know these two types.
			}
		unsigned char entry[18];// Get entry.
		ireg->read(reinterpret_cast<char*>(entry), entlen);
		int cx = entry[0] >> 4; // Get chunk indices within schunk.
		int cy = entry[1] >> 4;
					// Get coord. #'s where shape goes.
		int tilex = entry[0] & 0xf;
		int tiley = entry[1] & 0xf;
					// Get shape #, frame #.
		int shnum = entry[2]+256*(entry[3]&3);
		int frnum = entry[3] >> 2;

		Shape_info& info = ShapeID::get_info(shnum);
		unsigned int lift, quality, type;
		Ireg_game_object *obj;
		int is_egg = 0;		// Fields are eggs.
					// An "egg"?

		// Has flag byte(s)
		if (entlen == 10)
		{
			// Temporary
			if (entry[6] & 1) oflags |= 1<<Obj_flags::is_temporary;
		}
		
		if (info.get_shape_class() == Shape_info::hatchable)
			{
			bool anim = info.is_animated() || info.has_sfx();
			Egg_object *egg = Create_egg(entry, anim);
			get_chunk(scx + cx, scy + cy)->add_egg(egg);
			last_obj = egg;
			continue;
			}
		else if (entlen == 6 || entlen == 10)	// Simple entry?
			{
			type = 0;
			lift = entry[4] >> 4;
			quality = entry[5];
			obj = create_ireg_object(info, shnum, frnum,
							tilex, tiley, lift);
			is_egg = obj->is_egg();

					// Wierd use of flag:
			if (info.has_quantity())
				{
				if (!(quality&0x80))
					oflags &= 
						~(1<<Obj_flags::okay_to_take);
				else
					quality &= 0x7f;
				}
			else if (info.has_quality_flags())
				{	// Use those flags instead of deflt.
				oflags = Get_quality_flags(quality);
				quality = 0;
				}
			}
		else if (entlen == 12)	// Container?
			{
			type = entry[4] + 256*entry[5];
			lift = entry[9] >> 4;
			quality = entry[7];
			oflags =	// Override flags (I think).
				Get_quality_flags(entry[11]);
			if (shnum == 330)// Virtue stone?
				{
				Virtue_stone_object *v = 
				   new Virtue_stone_object(shnum, frnum, tilex,
						tiley, lift);
				v->set_pos(entry[4], entry[5], entry[6],
								entry[7]);
				obj = v;
				type = 0;
				}
			else if (shnum == 961)
				{
				Barge_object *b = new Barge_object(
				    shnum, frnum, tilex, tiley, lift,
					entry[4], entry[5],
					(quality>>1)&3);
				obj = b;
				if (!gwin->get_moving_barge() && 
							(quality&(1<<3)))
					gwin->set_moving_barge(b);
				}
			else if (Game::get_game_type() == SERPENT_ISLE &&
				shnum == 555) // serpent jawbone
				{
				obj = new Jawbone_object(shnum, frnum,
					tilex, tiley, lift, entry[10]);
				}
			else if (Game::get_game_type() == SERPENT_ISLE && 
				 shnum == 400 && frnum == 8 && quality == 1)
				// Gwenno. Ugly hack to fix bug without having to start
				// a new game. Remove someday... (added 20010820)
				{
				Dead_body *b = new Dead_body(400, 8, 
						tilex, tiley, lift, 149);
				obj = b;
				gwin->set_body(149, b);
				}
			else if (quality == 1 && 
					 (entry[8] >= 0x80 || 
				  Game::get_game_type() == SERPENT_ISLE)) 
				{		// NPC's body.
				int npc_num = (entry[8] - 0x80) & 0xFF;
				Dead_body *b = new Dead_body(shnum, frnum, 
						tilex, tiley, lift, npc_num);
				obj = b;
				gwin->set_body(npc_num, b);
				}
			else if (Is_body(shnum)) {
				obj = new Dead_body(
				    shnum, frnum, tilex, tiley, lift, -1);
			}
			else
				obj = new Container_game_object(
				    shnum, frnum, tilex, tiley, lift,
							entry[10]);
					// Read container's objects.
			if (type)	// (0 if empty.)
				{	// Don't pass along invisibility!
				read_ireg_objects(ireg, scx, scy, obj, 
					oflags & ~(1<<Obj_flags::invisible));
				obj->elements_read();
				}
			}
		else			// Length 18 means it's a spellbook.
			{		// Get all 9 spell bytes.
			quality = 0;
			unsigned char circles[9];
			memcpy(&circles[0], &entry[4], 5);
			lift = entry[9] >> 4;
			memcpy(&circles[5], &entry[10], 4);
			uint8 *ptr = &entry[14];
					// 3 unknowns, then bookmark.
			unsigned char bmark = ptr[3];
			obj = new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], bmark);
			}
		obj->set_quality(quality);
		obj->set_flags(oflags);
		last_obj = obj;		// Save as last read.
					// Add, but skip volume check.
		if (container)
			{
			if (index_id != -1 && 
			    container->add_readied(obj, index_id, 1, 1))
				continue;
			else if (container->add(obj, 1))
				continue;
			}
		Map_chunk *chunk = get_chunk(scx + cx, scy + cy);
		if (is_egg)
			chunk->add_egg((Egg_object *) obj);
		else
			chunk->add(obj);
		}
	}

/*
 *	Create non-container IREG objects.
 */

Ireg_game_object *Game_map::create_ireg_object
	(
	Shape_info& info,		// Info. about shape.
	int shnum, int frnum,		// Shape, frame.
	int tilex, int tiley,		// Tile within chunk.
	int lift			// Desired lift.
	)
	{
	if (info.is_field())		// (These are all animated.)
		{			// Check shapes.
		if (shnum == 895 ||	// Fire.
		    shnum == 561)	// SI - blue flame.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::fire_field);
		else if (shnum == 900)	// Poison.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::poison_field);
		else if (shnum == 902)	// Sleep.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::sleep_field);
		else if (shnum == 756)	// Caltrops.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::caltrops_field);
		}
	if (info.is_animated() || info.has_sfx())
		return new Animated_ireg_object(
				   shnum, frnum, tilex, tiley, lift);
	if (shnum == 607)		// Path.
		return new Egglike_game_object(
					shnum, frnum, tilex, tiley, lift);
	if (shnum == 848 || shnum == 268)	// Mirror
		return new Mirror_object(shnum, frnum, tilex, tiley, lift);
	else if (shnum == 761)		// Spellbook.
		{
		static unsigned char circles[9] = {0};
		return new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], 0);
		}
	else if (info.get_shape_class() == Shape_info::container)
		{
		if (shnum == 555 && GAME_SI)
			return new Jawbone_object(shnum, frnum, tilex, tiley,
									lift);
		else
			return new Container_game_object(shnum, frnum, 
							tilex, tiley, lift);
		}
	else
		return new Ireg_game_object(shnum, frnum, tilex, tiley, lift);
	}

/*
 *	Create non-container IREG objects.
 */

Ireg_game_object *Game_map::create_ireg_object
	(
	int shnum, int frnum		// Shape, frame.
	)
	{
	return create_ireg_object(ShapeID::get_info(shnum), 
						shnum, frnum, 0, 0, 0);
	}

/*
 *	Create 'fixed' (landscape, building) objects.
 */

Ifix_game_object *Game_map::create_ifix_object
	(
	int shnum, int frnum		// Shape, frame.
	)
	{
	Shape_info& info = ShapeID::get_info(shnum);
	return (info.is_animated() || info.has_sfx())
			? new Animated_ifix_object(shnum, frnum, 0, 0, 0)
			: new Ifix_game_object(shnum, frnum, 0, 0, 0);
	}

/*
 *	Read in the objects in a superchunk.
 */

void Game_map::get_superchunk_objects
	(
	int schunk			// Superchunk #.
	)
	{
//	CYCLE_RED_PLASMA();
	get_map_objects(schunk);	// Get map objects/scenery.
//	CYCLE_RED_PLASMA();
	get_ifix_objects(schunk);	// Get objects from ifix.
//	CYCLE_RED_PLASMA();
	get_ireg_objects(schunk);	// Get moveable objects.
//	CYCLE_RED_PLASMA();
	schunk_read[schunk] = 1;	// Done this one now.
	map_patches->apply(schunk);	// Move/delete objects.
	}

/*
 *	Locate a chunk with a given terrain # and center the view on it.
 *
 *	Output:	true if found, else 0.
 */

bool Game_map::locate_terrain
	(
	int tnum,			// # in u7chunks.
	int& cx, int& cy,		// Chunk to start at, or (-1,-1).
					//   Updated with chunk found.
	bool upwards			// If true, search upwards.
	)
	{
	int cnum;			// Chunk #, counting L-R, T-B.
	int cstop;			// Stop when cnum == cstop.
	int dir;
	if (upwards)
		{
		cstop = -1;
		dir = -1;
		if (cx == -1)		// Start at end?
			cnum = c_num_chunks*c_num_chunks - 1;
		else
			cnum = cy*c_num_chunks + cx - 1;
		}
	else
		{
		cstop = c_num_chunks*c_num_chunks;
		dir = 1;
		cnum = (cx == -1) ? 0 : cy*c_num_chunks + cx + 1;
		}
	while (cnum != cstop)
		{
		int chunky = cnum/c_num_chunks;
		int chunkx = cnum%c_num_chunks;
		if (terrain_map[chunkx][chunky] == tnum)
			{		// Return chunk # found.
			cx = chunkx;
			cy = chunky;
					// Center window over chunk found.
			Game_window::get_instance()->center_view(Tile_coord(
				cx*c_tiles_per_chunk + c_tiles_per_chunk/2,
				cy*c_tiles_per_chunk + c_tiles_per_chunk/2, 
									0));
			return true;
			}
		cnum += dir;
		}
	return false;			// Failed.
	}

/*
 *	Swap two adjacent terrain #'s, keeping the map looking the same.
 *
 *	Output:	false if unsuccessful.
 */

bool Game_map::swap_terrains
	(
	int tnum			// Swap tnum and tnum + 1.
	)
	{
	if (tnum < 0 || tnum >= chunk_terrains.size() - 1)
		return false;		// Out of bounds.
	map_modified = true;
					// Swap in list.
	Chunk_terrain *tmp = get_terrain(tnum);
	tmp->set_modified();
	chunk_terrains[tnum] = get_terrain(tnum + 1);
	chunk_terrains[tnum]->set_modified();
	chunk_terrains[tnum + 1] = tmp;
					// Update terrain map.
	for (int cy = 0; cy < c_num_chunks; cy++)
		for (int cx = 0; cx < c_num_chunks; cx++)
			{
			if (terrain_map[cx][cy] == tnum)
				terrain_map[cx][cy]++;
			else if (terrain_map[cx][cy] == tnum + 1)
				terrain_map[cx][cy]--;
			}
	Game_window::get_instance()->set_all_dirty();
	return true;
	}

/*
 *	Insert a new terrain after a given one, and push all the others up
 *	so the map looks the same.  The new terrain is filled with
 *	(shape, frame) == (0, 0) unless 'dup' is passed 'true'.
 *
 *	Output:	False if unsuccessful.
 */

bool Game_map::insert_terrain
	(
	int tnum,			// Insert after this one (may be -1).
	bool dup			// If true, duplicate #tnum.
	)
	{
	if (tnum < -1 || tnum >= chunk_terrains.size())
		return false;		// Invalid #.
	get_all_terrain();		// Need all of 'u7chunks' read in.
	map_modified = true;
	unsigned char buf[16*16*2];	// Set up buffer with shape #'s.
	if (dup && tnum >= 0)
		{			// Want to duplicate given terrain.
		Chunk_terrain *ter = chunk_terrains[tnum];
		unsigned char *data = &buf[0];
		for (int ty = 0; ty < c_tiles_per_chunk; ty++)
			for (int tx = 0; tx < c_tiles_per_chunk; tx++)
			{
			ShapeID id = ter->get_flat(tx, ty);
			*data++ = id.get_shapenum()&0xff;
			*data++ = ((id.get_shapenum()>>8)&3) | 
						(id.get_framenum()<<2);
			}
		}
	else
		memset(reinterpret_cast<char*>(buf), 0, sizeof(buf));
	Chunk_terrain *new_terrain = new Chunk_terrain(&buf[0]);
					// Insert in list.
	chunk_terrains.insert(chunk_terrains.begin() + tnum + 1, new_terrain);
					// Indicate terrains are modified.
	int num_chunk_terrains = chunk_terrains.size();
	for (int i = tnum + 1; i < num_chunk_terrains; i++)
		chunk_terrains[i]->set_modified();
	if (tnum + 1 == num_chunk_terrains - 1)
		return true;		// Inserted at end of list.
					// Update terrain map.
	for (int cy = 0; cy < c_num_chunks; cy++)
		for (int cx = 0; cx < c_num_chunks; cx++)
			{
			if (terrain_map[cx][cy] > tnum)
				terrain_map[cx][cy]++;
			}
	Game_window::get_instance()->set_all_dirty();
	return true;
	}

/*
 *	Remove a terrain, updating the map.
 *
 *	Output:	false if unsuccessful.
 */

bool Game_map::delete_terrain
	(
	int tnum
	)
	{
	if (tnum < 0 || tnum >= chunk_terrains.size())
		return false;		// Out of bounds.
	map_modified = true;
	int sz = chunk_terrains.size();
	delete chunk_terrains[tnum];
	for (int i = tnum + 1; i < sz; i++)
		{			// Move the rest downwards.
		Chunk_terrain *tmp = get_terrain(i);
		tmp->set_modified();
		chunk_terrains[i - 1] = tmp;
		}
	chunk_terrains.resize(sz - 1);
					// Update terrain map.
	for (int cy = 0; cy < c_num_chunks; cy++)
		for (int cx = 0; cx < c_num_chunks; cx++)
			{
			if (terrain_map[cx][cy] >= tnum)
				terrain_map[cx][cy]--;
			}
	Game_window::get_instance()->set_all_dirty();
	return true;
	}

/*
 *	Commit edits made to terrain chunks.
 */

void Game_map::commit_terrain_edits
	(
	)
	{
	int num_terrains = chunk_terrains.size();
					// Create list of flags.
	unsigned char *ters = new unsigned char[num_terrains];
	memset(ters, 0, num_terrains);
					// Commit edits.
	for (int i = 0; i < num_terrains; i++)
		if (chunk_terrains[i] && chunk_terrains[i]->commit_edits())
			ters[i] = 1;
					// Update terrain map.
	for (int cy = 0; cy < c_num_chunks; cy++)
		for (int cx = 0; cx < c_num_chunks; cx++)
			{
			Map_chunk *chunk = objects[cx][cy];
			if (chunk && ters[terrain_map[cx][cy]] != 0 &&
			    chunk->get_terrain())
					// Reload objects.
				chunk->set_terrain(chunk->get_terrain());
			}
	delete [] ters;
	}

/*
 *	Abort edits made to terrain chunks.
 */

void Game_map::abort_terrain_edits
	(
	)
	{
	int num_terrains = chunk_terrains.size();
					// Abort edits.
	for (int i = 0; i < num_terrains; i++)
		if (chunk_terrains[i])
			chunk_terrains[i]->abort_edits();
	}

/*
 *	Find all unused shapes in game.  This can take a while!!
 */

void Game_map::find_unused_shapes
	(
	unsigned char *found,		// Bits set for shapes found.
	int foundlen			// # bytes.
	)
	{
	memset(found, 0, foundlen);
	Game_window *gwin = Game_window::get_instance();
	Shape_manager *sman = Shape_manager::get_instance();
	cout << "Reading all chunks";
					// Read in EVERYTHING!
	for (int sc = 0; sc < c_num_schunks*c_num_schunks; sc++)
		{
		cout << '.';
		cout.flush();
		char msg[80];
		snprintf(msg, sizeof(msg), "Scanning superchunk %d", sc);
		gwin->get_effects()->center_text(msg);
		gwin->paint();
		gwin->show();
		if (!schunk_read[sc])
			get_superchunk_objects(sc);
		}
	cout << endl;
	int maxbits = foundlen*8;	// Total #bits in 'found'.
	int nshapes = sman->get_shapes().get_num_shapes();
	if (maxbits > nshapes)
		maxbits = nshapes;
					// Go through chunks.
	for (int cy = 0; cy < c_num_chunks; cy++)
		for (int cx = 0; cx < c_num_chunks; cx++)
			{
			Map_chunk *chunk = get_chunk(cx, cy);
			Recursive_object_iterator all(chunk->get_objects());
			Game_object *obj;
			while ((obj = all.get_next()) != 0)
				{
				int shnum = obj->get_shapenum();
				if (shnum >= 0 && shnum < maxbits)
					found[shnum/8] |= (1<<(shnum%8));
				}
			}
	int i;
	for (i = 0; i < maxbits; i++)	// Add all possible monsters.
		{
		Shape_info& info = ShapeID::get_info(i);
		Monster_info *minf = info.get_monster_info();
		if (minf)
			found[i/8] |= (1<<(i%8));
		Weapon_info *winf = info.get_weapon_info();
		if (winf)		// Get projectiles for weapons.
			{
			int proj = winf->get_projectile();
			if (proj > 0 && proj < maxbits)
				found[proj/8] |= (1<<(proj%8));
			}
		}
	for (i = 0x96; i < maxbits; i++)	// Ignore flats (<0x96).
		if (!(found[i/8]&(1<<(i%8))))
			cout << "Shape " << i << " not found in game" << endl;
	}

/*
 *	Look throughout the map for a given shape.  The search starts at
 *	the first currently-selected shape, if possible.
 *
 *	Output:	->object if found, else 0.
 */

Game_object *Game_map::locate_shape
	(
	int shapenum,			// Desired shape.
	bool upwards,			// If true, search upwards.
	Game_object *start		// Start here if !0.
	)
	{
	int cx = -1, cy = 0;		// Before chunk to search.
	int dir = 1;			// Direction to increment.
	int stop = c_num_chunks;
	if (upwards)
		{
		dir = -1;
		stop = -1;
		cx = c_num_chunks;	// Past last chunk.
		cy = c_num_chunks - 1;
		}
	Game_object *obj = 0;
	if (start)			// Start here.
		{
		Game_object *owner = start->get_outermost();
		cx = owner->get_cx();
		cy = owner->get_cy();
		if (upwards)
			{
			Recursive_object_iterator_backwards next(start);
			while ((obj = next.get_next()) != 0)
				if (obj->get_shapenum() == shapenum)
					break;
			}
		else
			{
			Recursive_object_iterator next(start);
			while ((obj = next.get_next()) != 0)
				if (obj->get_shapenum() == shapenum)
					break;
			}
		}
	while (!obj)			// Not found yet?
		{
		cx += dir;		// Next chunk.
		if (cx == stop)		// Past (either) end?
			{
			cy += dir;
			if (cy == stop)
				break;	// All done.
			cx -= dir*c_num_chunks;
			}
		Map_chunk *chunk = get_chunk(cx, cy);
					// Make sure objs. are read.
		int sx = cx/c_chunks_per_schunk, sy = cy/c_chunks_per_schunk;
		int schunk = sy*c_num_schunks + sx;
		if (!schunk_read[schunk])
			get_superchunk_objects(schunk);
		if (upwards)
			{
			Recursive_object_iterator_backwards next(
							chunk->get_objects());
			while ((obj = next.get_next()) != 0)
				if (obj->get_shapenum() == shapenum)
					break;
			}
		else
			{
			Recursive_object_iterator next(chunk->get_objects());
			while ((obj = next.get_next()) != 0)
				if (obj->get_shapenum() == shapenum)
					break;
			}
		}
	return obj;
	}

/*
 *	Do a cache out. (x, y) is the center
 */

void Game_map::cache_out(int cx, int cy)
{
	int sx = cx / c_chunks_per_schunk;
	int sy = cy / c_chunks_per_schunk;
	bool chunk_flags[12][12];

#ifdef DEBUG
	std::cout << "Want to cache out around super chunk: " << (sy*12 + sx) << " = "  << sx << ", " << sy << std::endl;
#endif

	std::memset(chunk_flags, 0, sizeof(chunk_flags));

	// We cache out all but the 9 directly around the pov

	chunk_flags[(sy+11)%12][(sx+11)%12] = true;
	chunk_flags[(sy+11)%12][sx] = true;
	chunk_flags[(sy+11)%12][(sx+1)%12] = true;

	chunk_flags[sy][(sx+11)%12] = true;
	chunk_flags[sy][sx] = true;
	chunk_flags[sy][(sx+1)%12] = true;

	chunk_flags[(sy+1)%12][(sx+11)%12] = true;
	chunk_flags[(sy+1)%12][sx] = true;
	chunk_flags[(sy+1)%12][(sx+1)%12] = true;

	for (sy = 0; sy < 12; sy++) for (sx = 0; sx < 12; sx++) {
		if (chunk_flags[sy][sx]) continue;

		int schunk = sy*12 + sx;
		if (schunk_read[schunk] && !schunk_modified[schunk]) cache_out_schunk(schunk);
	}
}

void Game_map::cache_out_schunk(int schunk)
{
	// Get abs. chunk coords.
	const int scy = 16*(schunk/12);
	const int scx = 16*(schunk%12);
	int cy, cx;
	bool save_map_modified = map_modified;

	if (schunk_modified[schunk])
		return;			// NEVER cache out modified chunks.
	// Our vectors
	Game_object_vector removes;
	Actor_vector actors;

	int buf_size = 0;

#ifdef DEBUG
	std::cout << "Killing superchunk: " << schunk << std::endl;
#endif
	// Go through chunks and get all the items
	for (cy = 0; cy < 16; cy++) for (cx = 0; cx < 16; cx++) {

		int size = objects[scx + cx][scy + cy]->get_obj_actors(removes, actors);

		if (size < 0) {
#ifdef DEBUG
			std::cerr << "Failed attempting to kill superchunk" << std::endl;
#endif
			return;
		}

		buf_size += size + 2;
	}

	schunk_read[schunk] = false;

#ifdef DEBUG
	std::cout << "Buffer size of " << buf_size << " bytes required to store super chunk" << std::endl;
#endif

	// Clear old (this shouldn't happen)
	if (schunk_cache[schunk]) {
		delete [] schunk_cache[schunk];
		schunk_cache[schunk] = 0;
		schunk_cache_sizes[schunk] = -1;
	}

	// Create new
	schunk_cache[schunk] = new char[buf_size];
	schunk_cache_sizes[schunk] = buf_size;

	BufferDataSource ds(schunk_cache[schunk], schunk_cache_sizes[schunk]);

	write_ireg_objects(schunk, &ds);

#ifdef DEBUG
	std::cout << "Wrote " << ds.getPos() << " bytes" << std::endl;
#endif

	// Now remove the objects
	for (Game_object_vector::const_iterator it=removes.begin(); it!=removes.end(); ++it) {
		(*it)->delete_contents();
		(*it)->remove_this();
	}

	// Now disable the actors
	for (Actor_vector::const_iterator act=actors.begin(); act!=actors.end(); ++act) {
		(*act)->cache_out();
	}

	// Go through chunks and finish up
	for (cy = 0; cy < 16; cy++) for (cx = 0; cx < 16; cx++) {

		objects[scx + cx][scy + cy]->kill_cache();
	}
					// Removing objs. sets these flags.
	schunk_modified[schunk] = false;
	map_modified = save_map_modified;
}
