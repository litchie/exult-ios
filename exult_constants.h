/*
 *  Copyright (C) 2001  The Exult Team
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

#ifndef _EXULT_CONSTANTS_H
#define _EXULT_CONSTANTS_H

/*
 *	Sizes:
 */
const int tilesize = 8;			// A tile (shape) is 8x8 pixels.
const int tiles_per_chunk = 16;		// A chunk is 16x16 tiles.
const int chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int num_chunks = 12*16;		// Total # of chunks in each dir.
const int chunks_per_schunk = 16;	// # chunks in each superchunk.
const int tiles_per_schunk = 16*16;	// # tiles in each superchunk.
					// Total # tiles in each dir.:
const int num_tiles = tiles_per_chunk*num_chunks;


#endif
