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
 *  GNU Library General Public License for more details.
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
#include <fstream.h>

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
using std::snprintf;

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
 *	Create game window.
 */

Game_map::Game_map
	(
	) : 
            chunk_terrains(0), num_chunk_terrains(0),
	    map_patches(new Map_patch_collection), chunks(new ifstream)
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
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_U7CHUNKS))
		U7open(*chunks, PATCH_U7CHUNKS);
	else
		U7open(*chunks, U7CHUNKS);
	chunks->seekg(0, ios::end);	// Get to end so we can get length.
					// 2 bytes/tile.
	num_chunk_terrains = chunks->tellg()/(c_tiles_per_chunk*2);
	std::ifstream u7map;		// Read in map.
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_U7MAP))
		U7open(u7map, PATCH_U7MAP);
	else
		U7open(u7map, U7MAP);
	for (int schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
	{			// Read in the chunk #'s.
		unsigned char buf[16*16*2];
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
	for (int i = 0; i < cnt; i++)
		delete chunk_terrains[i];
	chunk_terrains.resize(0);
					// Clear 'read' flags.
	memset(reinterpret_cast<char*>(schunk_read), 0, sizeof(schunk_read));
	memset(reinterpret_cast<char*>(schunk_modified), 0, 
						sizeof(schunk_modified));
	}

/*
 *	Read in superchunk data to cover the screen.
 */

void Game_map::read_map_data
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
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
					// Already have this one?
	Chunk_terrain *ter = chunk_num < chunk_terrains.size() ?
					chunk_terrains[chunk_num] : 0;
	if (!ter)			// No?  Got to read it.
		{			// Read in 16x16 2-byte shape #'s.
		chunks->seekg(chunk_num * 512);
		unsigned char buf[16*16*2];	
		chunks->read(reinterpret_cast<char*>(buf), sizeof(buf));
		ter = new Chunk_terrain(&buf[0]);
		chunk_terrains.put(chunk_num, ter);
		}
	chunk->set_terrain(ter);
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
	U7mkdir(PATCHDAT, 0755);	// Create dir if not already there.
	int schunk;			// Write each superchunk to 'static'.
	for (schunk = 0; schunk < 12*12 - 1; schunk++)
					// Only write what we've modified.
		if (schunk_modified[schunk])
			write_ifix_objects(schunk);
#if 0	/* +++++Finish later. Don't delete this! */
	ofstream ochunks;		// Open file for chunks data.
	U7open(ochunks, PATCH_U7CHUNKS);
	int cnt = chunk_terrains.size();
	for (int i = 0; i < cnt; i++)
			// +++Won't work.  Write out as a FLEX file??
		{			// Write modified ones.
		Chunk_terrain *ter = chunk_terrains[i];
		if (ter && ter->is_modified())
			{
			ochunks.seekp(i*512);
			unsigned char data[512];
			ter->write_flats(data);
			ochunks.write(reinterpret_cast<char*>(data), 512);
			}
		}
	if (!ochunks.good())
		throw file_write_exception(U7CHUNKS);
	ochunks.close();
#endif
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
	ofstream ifix;			// There it is.
	U7open(ifix, get_schunk_file_name(PATCH_U7IFIX, schunk, fname));
					// +++++Use game title.
	const int count = c_chunks_per_schunk*c_chunks_per_schunk;
	Flex::write_header(ifix, "Exult",  count);
	uint8 table[2*count*4];
	uint8 *tptr = &table[0];
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Store file position in table.
			long start = ifix.tellp();
			Write4(tptr, start);
			Map_chunk *chunk = get_chunk(scx + cx,
							       scy + cy);
					// Restore original order (sort of).
			Object_iterator_backwards next(chunk);
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				obj->write_ifix(ifix);
					// Store IFIX data length.
			Write4(tptr, ifix.tellp() - start);
			}
	ifix.seekp(0x80, std::ios::beg);	// Write table.
	ifix.write(reinterpret_cast<char *>(&table[0]), sizeof(table));
	ifix.flush();
	int result = ifix.good();
	if (!result)
		throw file_write_exception(fname);
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
	ifstream ifix;			// There it is.
	if (is_system_path_defined("<PATCH>") &&
					// First check for patch.
	    U7exists(get_schunk_file_name(PATCH_U7IFIX, schunk, fname)))
		U7open(ifix, fname);
	else
		U7open(ifix, get_schunk_file_name(U7IFIX, schunk, fname));
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get to index entry for chunk.
			int chunk_num = cy*16 + cx;
			ifix.seekg(0x80 + chunk_num*8);
					// Get location, length.
			long shapesoff = Read4(ifix);
			if (!shapesoff) // Nothing there?
				continue;
			unsigned long shapeslen = Read4(ifix);
			get_ifix_chunk_objects(ifix, shapesoff, shapeslen/4,
				scx + cx, scy + cy);
			}
	}

/*
 *	Get the objects from one ifix chunk entry onto the screen.
 */

void Game_map::get_ifix_chunk_objects
	(
	ifstream& ifix,
	long filepos,			// Where chunk's data lies.
	int cnt,			// # entries (objects).
	int cx, int cy			// Absolute chunk #'s.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	ifix.seekg(filepos);		// Get to actual shape.
					// Get buffer to hold entries' indices.
	unsigned char *entries = new unsigned char[4*cnt];
	unsigned char *ent = entries;	// Read them in.
	ifix.read(reinterpret_cast<char*>(entries), 4*cnt);
					// Get object list for chunk.
	Map_chunk *olist = get_chunk(cx, cy);
	for (int i = 0; i < cnt; i++, ent += 4)
		{
		Ifix_game_object *obj;
		int shnum = ent[2]+256*(ent[3]&3);
		Shape_info& info = gwin->get_info(shnum);
		if (info.is_animated())
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
	std::ostream& ireg,
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
			ireg.put(IREG_SPECIAL);
			ireg.put(IREG_UCSCRIPT);
			Write2(ireg, len);	// Store length.
			ireg.write(reinterpret_cast<char*>(buf), len);
			}
		}
	if (write_mark)
		{
		ireg.put(IREG_SPECIAL);
		ireg.put(IREG_ENDMARK);
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
	for (int schunk = 0; schunk < 12*12 - 1; schunk++)
					// Only write what we've read.
		if (schunk_read[schunk])
			write_ireg_objects(schunk);
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
	ofstream ireg;			// There it is.
	U7open(ireg, get_schunk_file_name(U7IREG, schunk, fname));
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
			Write2(ireg, 0);// End with 2 0's.
			}
	ireg.flush();
	int result = ireg.good();
	if (!result)
		throw file_write_exception(fname);
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
	ifstream ireg;			// There it is.
	try
	{
		U7open(ireg, get_schunk_file_name(U7IREG, schunk, fname));
	}
	catch(const file_exception & f)
	{
		return;			// Just don't show them.
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
	}

/*
 *	Read in a 'special' IREG entry (one starting with 255).
 */

void Read_special_ireg
	(
	istream& ireg,
	Game_object *obj		// Last object read.
	)
	{
	int type = Read1(ireg);		// Get type.
	int len = Read2(ireg);		// Length of rest.
	unsigned char *buf = new unsigned char[len];
	ireg.read(reinterpret_cast<char*>(buf), len);
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
	istream& ireg,
	Game_object *obj		// Last object read.
	)
	{
	unsigned char entlen;
	while ((entlen = ireg.peek()) == IREG_SPECIAL && !ireg.eof())
		{
		Read1(ireg);		// Eat the IREG_SPECIAL.
		unsigned char type = ireg.peek();
		if (type == IREG_ENDMARK)
			{		// End of list.
			Read1(ireg);
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
 *	Read a list of ireg objects.  They are either placed in the desired
 *	game chunk, or added to their container.
 */

void Game_map::read_ireg_objects
	(
	istream& ireg,			// File to read from.
	int scx, int scy,		// Abs. chunk coords. of superchunk.
	Game_object *container,		// Container, or null.
	unsigned long flags		// Usecode item flags.
	)
	{
	int entlen;			// Gets entry length.
	sint8 index_id = -1;
	Game_object *last_obj = 0;	// Last one read in this call.
	Game_window *gwin = Game_window::get_game_window();
					// Go through entries.
	while (((entlen = Read1(ireg), ireg.good())))
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
			index_id = (sint8) Read2 (ireg);
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
			long pos = ireg.tellg();
			cout << "Unknown entlen " << entlen << " at pos. " <<
					pos << endl;
			ireg.seekg(pos + entlen);
			continue;	// Only know these two types.
			}
		unsigned char entry[18];// Get entry.
		ireg.read(reinterpret_cast<char*>(entry), entlen);
		int cx = entry[0] >> 4; // Get chunk indices within schunk.
		int cy = entry[1] >> 4;
					// Get coord. #'s where shape goes.
		int tilex = entry[0] & 0xf;
		int tiley = entry[1] & 0xf;
					// Get shape #, frame #.
		int shnum = entry[2]+256*(entry[3]&3);
		int frnum = entry[3] >> 2;

		Shape_info& info = gwin->get_info(shnum);
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
			bool anim = info.is_animated() ||
					// Watch for BG itself.
				(Game::get_game_type() == BLACK_GATE &&
						shnum == 305);
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
			obj->set_low_lift (entry[4] & 0xF);
			obj->set_high_shape (entry[3] >> 7);

					// Wierd use of flag:
			if (info.has_quantity())
				if (!(quality&0x80))
					oflags &= 
						~(1<<Obj_flags::okay_to_take);
				else
					quality &= 0x7f;
			}
		else if (entlen == 12)	// Container?
			{
			type = entry[4] + 256*entry[5];
			lift = entry[9] >> 4;
			quality = entry[7];
			oflags =	// Override flags (I think).
			    ((entry[11]&1) << Obj_flags::invisible) |
			    (((entry[11]>>3)&1) << Obj_flags::okay_to_take);
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
				obj = new Dead_body(400, 8, tilex, tiley, 
								lift, 149);
				gwin->set_body(149, obj);
				}
			else if (quality == 1 && 
					 (entry[8] >= 0x80 || 
				  Game::get_game_type() == SERPENT_ISLE)) 
				{		// NPC's body.
				int npc_num = (entry[8] - 0x80) & 0xFF;
				obj = new Dead_body(shnum, frnum, tilex, 
						tiley, lift, npc_num);
				gwin->set_body(npc_num, obj);
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
			unsigned long flags = Read4(ptr);
			obj = new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], flags);
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
	if (info.is_animated())
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
				&circles[0], 0L);
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

