/**
 **	Gamerend.cc - Rendering methods.
 **
 **	Written: 7/22/98 - JSF
 **	Split off: 9/8/2000
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include <stdio.h>

using std::snprintf;

#include "gamewin.h"
#include "actors.h"
#include "chunks.h"
#include "objiter.h"
#include "Gump.h"
#include "effects.h"
#include "cheat.h"

/*
 *	Paint just the map with given top-left-corner tile.
 */

void Game_window::paint_map_at_tile
	(
	int x, int y, int w, int h,	// Clip to this area.
	int toptx, int topty,
	int skip_above			// Don't display above this lift.
	)
	{
	int savescrolltx = scrolltx, savescrollty = scrollty;
	int saveskip = skip_lift;
	scrolltx = toptx;
	scrollty = topty;
	skip_lift = skip_above;
	read_map_data();		// Gather in all objs., etc.
	win->set_clip(x, y, w, h);
	paint_map(0, 0, get_width(), get_height());
	win->clear_clip();
	scrolltx = savescrolltx;
	scrollty = savescrollty;
	skip_lift = saveskip;
	}

/*
 *	Figure offsets on screen.
 */

inline int Figure_screen_offset
	(
	int ch,				// Chunk #
	int scroll			// Top/left tile of screen.
	)
	{
					// Watch for wrapping.
	int t = ch*c_tiles_per_chunk - scroll;
	if (t < -c_num_tiles/2)
		t += c_num_tiles;
	t %= c_num_tiles;
	return t*c_tilesize;
	}

/*
 *	Paint just the map and its objects (no gumps, effects).
 *	(The caller should set/clear clip area.)
 *
 *	Output:	# light-sources found.
 */

int Game_window::paint_map
	(
	int x, int y, int w, int h	// Rectangle to cover.
	)
	{

	render_seq++;			// Increment sequence #.
	int light_sources = 0;		// Count light sources found.
					// Get chunks to start with, starting
					//   1 tile left/above.
	int start_chunkx = (scrolltx + x/c_tilesize - 1)/c_tiles_per_chunk;
					// Wrap around.
	start_chunkx = (start_chunkx + c_num_chunks)%c_num_chunks;
	int start_chunky = (scrollty + y/c_tilesize - 1)/c_tiles_per_chunk;
	start_chunky = (start_chunky + c_num_chunks)%c_num_chunks;
					// End 8 tiles to right.
	int stop_chunkx = 1 + (scrolltx + (x + w + c_tilesize - 2)/c_tilesize + 
					c_tiles_per_chunk/2)/c_tiles_per_chunk;
	int stop_chunky = 1 + (scrollty + (y + h + c_tilesize - 2)/c_tilesize + 
					c_tiles_per_chunk/2)/c_tiles_per_chunk;
					// Wrap around the world:
	stop_chunkx = (stop_chunkx + c_num_chunks)%c_num_chunks;
	stop_chunky = (stop_chunky + c_num_chunks)%c_num_chunks;

	int cx, cy;			// Chunk #'s.
					// Paint all the flat scenery.
	for (cy = start_chunky; cy != stop_chunky; cy = INCR_CHUNK(cy))
		{
		int yoff = Figure_screen_offset(cy, scrollty);
		for (cx = start_chunkx; cx != stop_chunkx; cx = INCR_CHUNK(cx))
			{
			int xoff = Figure_screen_offset(cx, scrolltx);
			if (in_dungeon)
				paint_dungeon_chunk_flats(cx, cy, xoff, yoff);
			else
				paint_chunk_flats(cx, cy, xoff, yoff);
			if (cheat.in_map_editor())
				{	// Show lines around chunks.
				win->fill8(hit_pixel, c_chunksize, 1, 
								xoff, yoff);
				win->fill8(hit_pixel, 1, c_chunksize, 
								xoff, yoff);
				char text[8];	// Show chunk #.
				snprintf(text, 8, "%d", terrain_map[cx][cy]);
				paint_text(7, text, xoff + 2, yoff + 2);
				}
			}
		}
					// Draw the chunks' objects
					//   diagonally NE.
	int tmp_stopy = DECR_CHUNK(start_chunky);
	for (cy = start_chunky; cy != stop_chunky; cy = INCR_CHUNK(cy))
		{
		for (int dx = start_chunkx, dy = cy;
			dx != stop_chunkx && dy != tmp_stopy; 
				dx = INCR_CHUNK(dx), dy = DECR_CHUNK(dy))
			light_sources += paint_chunk_objects(dx, dy);
		}
	for (cx = (start_chunkx + 1)%c_num_chunks; cx != stop_chunkx; 
							cx = INCR_CHUNK(cx))
		{
		for (int dx = cx, 
			dy = (stop_chunky - 1 + c_num_chunks)%c_num_chunks; 
			dx != stop_chunkx && dy != tmp_stopy; 
				dx = INCR_CHUNK(dx), dy = DECR_CHUNK(dy))
			light_sources += paint_chunk_objects(dx, dy);
		}
	painted = 1;
	return light_sources;
	}

/*
 *	Paint a rectangle in the window by pulling in vga chunks.
 */

void Game_window::paint
	(
	int x, int y, int w, int h	// Rectangle to cover.
	)
	{
	if (!win->ready())
		return;
	win->set_clip(x, y, w, h);	// Clip to this area.
	int light_sources = paint_map(x, y, w, h);
					// Draw gumps.
	for (Gump *gmp = open_gumps; gmp; gmp = gmp->get_next())
		gmp->paint(this);
					// Draw text, sprites.
	for (Special_effect *txt = effects; txt; txt = txt->next)
		txt->paint(this);
	win->clear_clip();
					// Complete repaint?
	if (!x && !y && w == get_width() && h == get_height() && main_actor)
		{			// Look for lights.
		Actor *party[9];	// Get party, including Avatar.
		int cnt = get_party(party, 1);
		int carried_light = 0;
		for (int i = 0; !carried_light && i < cnt; i++)
			carried_light = party[i]->has_light_source();
					// Also check light spell.
		if (special_light && clock.get_total_minutes() > special_light)
			special_light = 0;
					// Set palette for lights.
		clock.set_light_source(carried_light + (light_sources > 0));
		}
	}

/*
 *	Paint the flat (non-rle) shapes in a chunk.
 */

void Game_window::paint_chunk_flats
	(
	int cx, int cy,			// Chunk coords (0 - 12*16).
	int xoff, int yoff		// Pixel offset of top-of-screen.
	)
	{
	Map_chunk *olist = get_chunk(cx, cy);
					// Paint flat tiles.
	Image_buffer8 *cflats = olist->get_rendered_flats();
	if (cflats)
		win->copy8(cflats->get_bits(), c_chunksize, c_chunksize, 
								xoff, yoff);

	Flat_object_iterator next(olist);// Now do flat RLE objects.
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
		obj->paint(this);
	}

/*
 *	Paint the flat (non-rle) shapes in a chunk when inside a dungeon.
 */

void Game_window::paint_dungeon_chunk_flats
	(
	int cx, int cy,			// Chunk coords (0 - 12*16).
	int xoff, int yoff		// Pixel offset of top-of-screen.
	)
	{
	Map_chunk *olist = get_chunk(cx, cy);
	if (!olist->has_dungeon())	// No dungeon in this chunk?
		{
		const int w = c_tilesize*c_tiles_per_chunk;
		win->fill8(0, w, w, xoff, yoff);
		return;
		}
					// Paint flat tiles.
	Image_buffer8 *cflats = olist->get_rendered_flats();
	if (cflats)
		win->copy8(cflats->get_bits(), c_chunksize, c_chunksize, 
								xoff, yoff);
					// Paint tiles outside dungeon black.
	for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
			if (!olist->in_dungeon(tilex, tiley))
				win->fill8(0, c_tilesize, c_tilesize, 
					xoff + tilex*c_tilesize, 
					yoff + tiley*c_tilesize);

	Flat_object_iterator next(olist);// Now do flat RLE objects.
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
		if (olist->in_dungeon(obj))
			obj->paint(this);
	}

/*
 *	Paint a chunk's objects, left-to-right, top-to-bottom.
 *
 *	Output:	# light sources found.
 */

int Game_window::paint_chunk_objects
	(
	int cx, int cy			// Chunk coords (0 - 12*16).
	)
	{
	Game_object *obj;
	Map_chunk *olist = get_chunk(cx, cy);
	if (in_dungeon && !olist->has_dungeon())
		return 0;		// Totally outside dungeon.
	int light_sources = 0;		// Also check for light sources.
//	if (is_main_actor_inside() && olist->is_roof()) +++++Correct??
		light_sources += olist->get_light_sources();
	int save_skip = skip_lift;
	if (skip_above_actor < skip_lift)
		skip_lift = skip_above_actor;
	Nonflat_object_iterator next(olist);
	if (in_dungeon)
		{
		while ((obj = next.get_next()) != 0)
			if (obj->render_seq != render_seq &&
			    olist->in_dungeon(obj))
				paint_dungeon_object(olist, obj);
		}
	else
		while ((obj = next.get_next()) != 0)
			if (obj->render_seq != render_seq)
				paint_object(obj);
	skip_lift = save_skip;
	return light_sources;
	}

/*
 *	Render an object after first rendering any that it depends on.
 */

void Game_window::paint_object
	(
	Game_object *obj
	)
	{
	int lift = obj->get_lift();
	if (lift >= skip_lift)
		return;
	obj->render_seq = render_seq;
	int cnt = obj->get_dependency_count();
	for (int i = 0; i < cnt; i++)
		{
		Game_object *dep = obj->get_dependency(i);
		if (dep && dep->render_seq != render_seq)
			paint_object(dep);
		}
	obj->paint(this);		// Finally, paint this one.
	}

/*
 *	Same thing as above, but when inside a dungeon.
 */

void Game_window::paint_dungeon_object
	(
	Map_chunk *olist,	// Chunk being rendered.
	Game_object *obj
	)
	{
	int lift = obj->get_lift();
	if (lift >= skip_lift)
		return;
	obj->render_seq = render_seq;
	int cnt = obj->get_dependency_count();
	for (int i = 0; i < cnt; i++)
		{
		Game_object *dep = obj->get_dependency(i);
		if (dep && dep->render_seq != render_seq &&
		    olist->in_dungeon(dep))
			paint_dungeon_object(olist, dep);
		}
	obj->paint(this);		// Finally, paint this one.
	}


