/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Chunkter.cc - Chunk terrain (16x16 flat tiles) on the map.
 **
 **	Written: 7/6/01 - JSF
 **/

/*
Copyright (C) 2001 The Exult Team

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

#include "chunkter.h"
#include "gamewin.h"

Chunk_terrain *Chunk_terrain::render_queue = 0;
int Chunk_terrain::queue_size = 0;

/*
 *	Insert at start of render queue.  It may already be there, but it's
 *	assumed that it's already tested as not being at the start.
 */

void Chunk_terrain::insert_in_queue
	(
	)
	{
	if (render_queue_next)		// In queue already?
		{			// !!Assuming it's not at head!!
		render_queue_next->render_queue_prev = render_queue_prev;
		render_queue_prev->render_queue_next = render_queue_next;
		}
	else
		render_queue_size++;	// Adding, so increment count.
	if (!render_queue)		// First?
		render_queue_next = render_queue_prev = this;
	else
		{
		render_queue_next = render_queue;
		render_queue_prev = render_queue->render_queue_prev;
		render_queue_prev->render_queue_next = this;
		render_queue->render_queue_prev = this;
		}
	render_queue = this;
	}

/*
 *	Remove from render queue.
 */

void Chunk_terrain::remove_from_queue
	(
	)
	{
	if (!render_queue_next)
		return;			// Not in queue.
	queue_size--;
	if (render_queue_next == this)	// Only element?
		render_queue = 0;
	else
		{
		if (render_queue == this)
			render_queue = render_queue_next;
		render_queue_next->render_queue_prev = render_queue_prev;
		render_queue_prev->render_queue_next = render_queue_next;
		}
	render_queue_next = render_queue_prev = 0;
	}

/*
 *	Paint a flat tile into our cached buffer.
 */

inline void Chunk_terrain::paint_tile
	(
	int tilex, int tiley		// Tile within chunk.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_frame *shape = gwin->get_shape(get_flat(tilex, tiley));
	if (!shape->is_rle())		// Only do flat tiles.
		rendered_flats->copy8(shape->get_data(), 
			c_tilesize, c_tilesize, tilex*c_tilesize,
						tiley*c_tilesize);
	}

/*
 *	Create list for a given chunk.
 */

Chunk_terrain::Chunk_terrain
	(
	unsigned char *data		// Chunk data.
	) : rendered_flats(0), num_clients(0), render_queue_next(0),
	    render_queue_prev(0)
	{
	for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
			{
			ShapeID id(data[0], (unsigned char) (data[1]&0x7f));
			flats[16*tiley + tilex] = id;
			data += 2;
			}
	}

/*
 *	Delete all objects contained within.
 */

Chunk_terrain::~Chunk_terrain
	(
	)
	{
	delete rendered_flats;
	remove_from_queue();
	}

/*
 *	Figure max. queue size for given game window.
 */
static int Figure_queue_size
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int w = gwin->get_width(), h = gwin->get_height();
					// Figure # chunks, rounding up.
	int cw = (w + c_chunksize - 1)/c_chunksize,
	    ch = (h + c_chunksize - 1)/c_chunksize;
					// Add extra in each dir.
	return (cw + 3)*(ch + 3);
	}

/*
 *	Create rendered_flats buffer.
 */

Image_buffer8 *Chunk_terrain::render_flats
	(
	bool in_dungeon
	)
	{
	rendered_dungeon = in_dungeon;	// Save type of rendering.
	if (!rendered_flats)
		{
		if (queue_size > Figure_queue_size())
			{		// Grown too big.  Remove last.
			Chunk_terrain *last = render_queue->render_queue_prev;
			last->free_rendered_flats();
			render_queue->render_queue_prev = render_queue_prev;
			render_queue_prev->render_queue_next = render_queue;
			render_queue_next = render_queue_prev = 0;
			queue_size--;
			}
		rendered_flats = new Image_buffer8(c_chunksize, c_chunksize);
		}
					// Go through array of tiles.
	for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
			if (!in_dungeon || this->in_dungeon(tilex, tiley))
				paint_tile(tilex, tiley);
			else		// Paint black if outside dungeon.
				rendered_flats->fill8(0, 
					c_tilesize, c_tilesize, 
					tilex*c_tilesize, tiley*c_tilesize);
	return rendered_flats;
	}

/*
 *	Free pre-rendered landscape.
 */

void Chunk_terrain::free_rendered_flats
	(
	)
	{
	delete rendered_flats; 
	rendered_flats = 0; 
	}

/*
 *	Write out to a chunk.
 */
	
void Chunk_terrain::write_flats
	(
	unsigned char *chunk_data
	)
	{
	for (int ty = 0; ty < c_tiles_per_chunk; ty++)
		for (int tx = 0; tx < c_tiles_per_chunk; tx++)
			{
			ShapeID id = get_flat(tx, ty);
			//++++++++++Skip RLE's???
			int shapenum = id.get_shapenum(), 
			    framenum = id.get_framenum();
			*chunk_data++ = shapenum&0xff;
			*chunk_data++ = ((shapenum>>8)&3) | (framenum<<2);
			}
	}

