/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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

#include "gamewin.h"
#include "actors.h"
#include "chunks.h"
#include "objiter.h"
#include "Gump.h"
#include "effects.h"

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
	int start_chunkx = (scrolltx + x/tilesize - 1)/tiles_per_chunk;
	if (start_chunkx < 0)
		start_chunkx = 0;
	int start_chunky = (scrollty + y/tilesize - 1)/tiles_per_chunk;
	if (start_chunky < 0)
		start_chunky = 0;
					// End 8 tiles to right.
	int stop_chunkx = 1 + (scrolltx + (x + w + tilesize - 2)/tilesize + 
					tiles_per_chunk/2)/tiles_per_chunk;
	int stop_chunky = 1 + (scrollty + (y + h + tilesize - 2)/tilesize + 
					tiles_per_chunk/2)/tiles_per_chunk;
	if (stop_chunkx > num_chunks)
		stop_chunkx = num_chunks;
	if (stop_chunky > num_chunks)
		stop_chunky = num_chunks;

	int cx, cy;			// Chunk #'s.
					// Paint all the flat scenery.
	for (cy = start_chunky; cy < stop_chunky; cy++)
		for (cx = start_chunkx; cx < stop_chunkx; cx++)
			if (in_dungeon)
				paint_dungeon_chunk_flats(cx, cy);
			else
				paint_chunk_flats(cx, cy);
					// Draw the chunks' objects
					//   diagonally NE.
	for (cy = start_chunky; cy < stop_chunky; cy++)
		for (int dx = start_chunkx, dy = cy;
			dx < stop_chunkx && dy >= start_chunky; dx++, dy--)
			light_sources += paint_chunk_objects(dx, dy);
	for (cx = start_chunkx + 1; cx < stop_chunkx; cx++)
		for (int dx = cx, dy = stop_chunky - 1; 
			dx < stop_chunkx && dy >= start_chunky; dx++, dy--)
			light_sources += paint_chunk_objects(dx, dy);
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
 *	Paint a flat tile.
 */

inline void Game_window::paint_tile
	(
	Chunk_object_list *olist,
	int tilex, int tiley,		// Tile within chunk.
	int xoff, int yoff		// Offset of chunk within window.
	)
	{
	ShapeID id = olist->get_flat(tilex, tiley);
	if (!id.is_invalid())
		{			// Draw flat.
		Shape_frame *shape = get_shape(id);
		win->copy8(shape->data, tilesize, tilesize, 
					xoff + tilex*tilesize,
					yoff + tiley*tilesize);
		}
	}

/*
 *	Paint the flat (non-rle) shapes in a chunk.
 */

void Game_window::paint_chunk_flats
	(
	int cx, int cy			// Chunk coords (0 - 12*16).
	)
	{
	int xoff = (cx*tiles_per_chunk - get_scrolltx())*tilesize;
	int yoff = (cy*tiles_per_chunk - get_scrollty())*tilesize;
	Chunk_object_list *olist = get_objects(cx, cy);
					// Go through array of tiles.
	for (int tiley = 0; tiley < tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < tiles_per_chunk; tilex++)
			paint_tile(olist, tilex, tiley, xoff, yoff);

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
	int cx, int cy			// Chunk coords (0 - 12*16).
	)
	{
	int xoff = (cx*tiles_per_chunk - get_scrolltx())*tilesize;
	int yoff = (cy*tiles_per_chunk - get_scrollty())*tilesize;
	Chunk_object_list *olist = get_objects(cx, cy);
	if (!olist->has_dungeon())	// No dungeon in this chunk?
		{
		const int w = tilesize*tiles_per_chunk;
		win->fill8(0, w, w, xoff, yoff);
		return;
		}
					// Go through array of tiles.
	for (int tiley = 0; tiley < tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < tiles_per_chunk; tilex++)
			if (olist->in_dungeon(tilex, tiley))
				paint_tile(olist, tilex, tiley, xoff, yoff);
			else		// Paint black if outside dungeon.
				win->fill8(0, tilesize, tilesize, 
					xoff + tilex*tilesize,
					yoff + tiley*tilesize);

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
	Chunk_object_list *olist = get_objects(cx, cy);
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
	Chunk_object_list *olist,	// Chunk being rendered.
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


