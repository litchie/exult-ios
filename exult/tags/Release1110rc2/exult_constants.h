/*
 *	exult_constants.h - Some constants/macros that are used all over the code.
 *
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

#ifndef EXULT_CONSTANTS_H
#define EXULT_CONSTANTS_H

#include <string>

/*
 *	Sizes:
 */
const int c_tilesize = 8;		// A tile (shape) is 8x8 pixels.
const int c_tiles_per_chunk = 16;	// A chunk is 16x16 tiles.
const int c_chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int c_num_schunks = 12;
const int c_num_chunks = 12*16;		// Total # of chunks in each dir.
const int c_chunks_per_schunk = 16;	// # chunks in each superchunk.
const int c_tiles_per_schunk = 16*16;	// # tiles in each superchunk.
					// Total # tiles in each dir.:
const int c_num_tiles = c_tiles_per_chunk*c_num_chunks;

const int c_fade_in_time = 30;	// Time for fade in
const int c_fade_out_time = 30;	// Time for fade out
const int c_std_delay = 200;	// Standard animation delay.  May want to
				//   make this settable!

const int c_any_shapenum = -359;
const int c_any_qual = -359;
const int c_any_framenum = -359;

/*
 * Empty string
 */
extern const std::string c_empty_string;


#define MOVE_NODROP (1<<3)
#define MOVE_FLY (1<<4)
#define MOVE_LEVITATE (MOVE_FLY|MOVE_NODROP)
#define	MOVE_WALK (1<<5)
#define MOVE_SWIM (1<<6)
#define	MOVE_ALL_TERRAIN ((1<<5)|(1<<6))
#define MOVE_ETHEREAL (1<<7)
#define MOVE_ALL (MOVE_FLY|MOVE_WALK|MOVE_SWIM|MOVE_ETHEREAL)
#define MOVE_MAPEDIT (1<<8)

//	Wrapping:
#define INCR_CHUNK(x) (((x) + 1)%c_num_chunks)
#define DECR_CHUNK(x) (((x) - 1 + c_num_chunks)%c_num_chunks)
#define INCR_TILE(x) (((x) + 1)%c_num_tiles)
inline int DECR_TILE(int x, int amt = 1)
	{ return (x - amt + c_num_tiles)%c_num_tiles; }
				// Return x - y with wrapping.
inline int SUB_TILE(int x, int y)
	{
	int delta = x - y;
	return delta < -c_num_tiles/2 ? delta + c_num_tiles :
	       delta >= c_num_tiles/2 ? delta - c_num_tiles : delta;
	}

// Debug
#ifdef DEBUG
#  define COUT(x)		do { std::cout << x << std::endl; std::cout.flush(); } while (0)
#  define CERR(x)		do { std::cerr << x << std::endl; std::cerr.flush(); } while (0)
#else
#  define COUT(x)		do { } while(0)
#  define CERR(x)		do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they will additionally
// set the old object pointer to 0, thus helping prevent double deletes (not that "delete 0"
// is a no-op.
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


#endif
