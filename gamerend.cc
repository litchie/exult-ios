/*
 *	gamerend.cc - Rendering methods.
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

#include <cstdio>

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
namespace std {
using ::snprintf;
}
#else
using std::snprintf;
#endif

#include "gamewin.h"
#include "gamerend.h"
#include "gameclk.h"
#include "gamemap.h"
#include "actors.h"
#include "chunks.h"
#include "objiter.h"
#include "Gump_manager.h"
#include "Gump.h"
#include "effects.h"
#include "cheat.h"
#include "drag.h"

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
	map->read_map_data();		// Gather in all objs., etc.
	win->set_clip(x, y, w, h);
	render->paint_map(0, 0, get_width(), get_height());
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
 *	Show the outline around a chunk.
 */

inline void Paint_chunk_outline
	(
	Game_window *gwin,
	int pixel,			// Pixel value to use.
	int cx, int cy,			// Chunk coords.
	int tnum,			// Terrain #.
	int xoff, int yoff		// Where chunk was painted.
	)
	{
	gwin->get_win()->fill8(pixel, c_chunksize, 1, xoff, yoff);
	gwin->get_win()->fill8(pixel, 1, c_chunksize, xoff, yoff);
	char text[40];			// Show chunk #.
	snprintf(text, sizeof(text), "(%d,%d)T%d", cx, cy, tnum);
	Shape_manager::get_instance()->paint_text(2, text, xoff + 2, yoff + 2);
	}

/*
 *	Paint tile grid.
 */

static void Paint_grid
	(
	Game_window *gwin,
	Xform_palette xform		// For transparency.
	)
	{
	Image_window8 *win = gwin->get_win();
					// Paint grid at edit height.
	int xtiles = gwin->get_width()/c_tilesize,
	    ytiles = gwin->get_height()/c_tilesize;
	int lift = cheat.get_edit_lift();
	int liftpixels = lift*(c_tilesize/2) + 1;
	for (int y = 0; y < ytiles; y++)
		win->fill_translucent8(0, xtiles*c_tilesize, 1, 
				-liftpixels, y*c_tilesize - liftpixels, xform);
	for (int x = 0; x < xtiles; x++)
		win->fill_translucent8(0, 1, ytiles*c_tilesize,
				x*c_tilesize - liftpixels, -liftpixels, xform);
	}


/*
 *	Just paint terrain.  This is for terrain_editing mode.
 */

void Game_render::paint_terrain_only
	(
	int start_chunkx, int start_chunky,
	int stop_chunkx, int stop_chunky
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Game_map *map = gwin->map;
	Shape_manager *sman = Shape_manager::get_instance();
	int cx, cy;			// Chunk #'s.
					// Paint all the flat scenery.
	for (cy = start_chunky; cy != stop_chunky; cy = INCR_CHUNK(cy))
		{
		int yoff = Figure_screen_offset(cy, gwin->scrollty);
		for (cx = start_chunkx; cx != stop_chunkx; cx = INCR_CHUNK(cx))
			{
			int xoff = Figure_screen_offset(cx, gwin->scrolltx);
			Map_chunk *chunk = map->get_chunk(cx, cy);
			chunk->get_terrain()->render_all(cx, cy);
			if (cheat.in_map_editor())
				Paint_chunk_outline(gwin, 
				    sman->get_special_pixel(HIT_PIXEL), cx, cy,
				    map->get_terrain_num(cx, cy), xoff, yoff);
			}
		}
					// Paint tile grid if desired.
	if (cheat.show_tile_grid())
		Paint_grid(gwin, sman->get_xform(10));
	}

/*
 *	Paint just the map and its objects (no gumps, effects).
 *	(The caller should set/clear clip area.)
 *
 *	Output:	# light-sources found.
 */

int Game_render::paint_map
	(
	int x, int y, int w, int h	// Rectangle to cover.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Game_map *map = gwin->map;
	Shape_manager *sman = gwin->shape_man;
	render_seq++;			// Increment sequence #.
	gwin->painted = 1;
	int scrolltx = gwin->scrolltx, scrollty = gwin->scrollty;
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
	if (!gwin->skip_lift)		// Special mode for editing?
		{
		paint_terrain_only(start_chunkx, start_chunky,
						stop_chunkx, stop_chunky);
		return 10;		// Pretend there's lots of light!
		}
	int cx, cy;			// Chunk #'s.
					// Paint all the flat scenery.
	for (cy = start_chunky; cy != stop_chunky; cy = INCR_CHUNK(cy))
		{
		int yoff = Figure_screen_offset(cy, scrollty);
		for (cx = start_chunkx; cx != stop_chunkx; cx = INCR_CHUNK(cx))
			{
			int xoff = Figure_screen_offset(cx, scrolltx);
			paint_chunk_flats(cx, cy, xoff, yoff);

			if (cheat.in_map_editor())
				Paint_chunk_outline(gwin, 
				    sman->get_special_pixel(HIT_PIXEL), cx, cy,
				    map->get_terrain_num(cx, cy), xoff, yoff);
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

	/// Dungeon Blackness (but disable in map editor mode)
	if (gwin->in_dungeon >= gwin->skip_above_actor && 
							!cheat.in_map_editor())
		paint_blackness (start_chunkx, start_chunky, stop_chunkx, 
					stop_chunky, gwin->ice_dungeon?73:0);

					// Outline selected objects.
	const Game_object_vector& sel = cheat.get_selected();
	for (Game_object_vector::const_iterator it = sel.begin();
						it != sel.end(); ++it)
		{
		Game_object *obj = *it;
		int x, y;
		if (!obj->get_owner() && obj->get_lift() < gwin->skip_lift &&
				obj->get_lift() < gwin->skip_above_actor)
			{
			gwin->get_shape_location(obj, x, y);
			obj->paint_outline(x, y, HIT_PIXEL);
			}
		}
					// Paint tile grid if desired.
	if (cheat.in_map_editor() && cheat.show_tile_grid())
		Paint_grid(gwin, sman->get_xform(10));

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
	if (!win->ready()) return;

	win->set_clip(x, y, w, h);	// Clip to this area.
	int light_sources = render->paint_map(x, y, w, h);

					// Draw gumps unless in dont_move mode.
	if (!main_actor_dont_move())
        	gump_man->paint();
	effects->paint();		// Draw text, sprites.
	if (dragging)
		dragging->paint();	// Paint what user is dragging.
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
		if (special_light && clock->get_total_minutes() >special_light)
			special_light = 0;
					// Set palette for lights.
		clock->set_light_source(carried_light + (light_sources > 0));
		}
	}

/*
 *	Paint whole window.
 */
void Game_window::paint()
	{
	map->read_map_data();		// Gather in all objs., etc.
	set_all_dirty();
	paint_dirty();
	}

/*
 *	Paint the flat (non-rle) shapes in a chunk.
 */

void Game_render::paint_chunk_flats
	(
	int cx, int cy,			// Chunk coords (0 - 12*16).
	int xoff, int yoff		// Pixel offset of top-of-screen.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Map_chunk *olist = gwin->map->get_chunk(cx, cy);
					// Paint flat tiles.
#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())	// OpenGL rendering?
		{
		Chunk_terrain *terrain = olist->get_terrain();
		if (terrain)
			terrain->get_glflats()->paint(xoff, yoff);
		}
	else
#endif
		{
		Image_buffer8 *cflats = olist->get_rendered_flats();
		if (cflats)
			gwin->win->copy8(cflats->get_bits(), 
				c_chunksize, c_chunksize, xoff, yoff);
		}
	Flat_object_iterator next(olist);// Now do flat RLE objects.
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
		obj->paint();
	}

/*
 *	Paint a chunk's objects, left-to-right, top-to-bottom.
 *
 *	Output:	# light sources found.
 */

int Game_render::paint_chunk_objects
	(
	int cx, int cy			// Chunk coords (0 - 12*16).
	)
	{
	Game_object *obj;
	Game_window *gwin = Game_window::get_instance();
	Map_chunk *olist = gwin->map->get_chunk(cx, cy);
	int light_sources = 0;		// Also check for light sources.
//	if (is_main_actor_inside() && olist->is_roof()) +++++Correct??
		light_sources += olist->get_light_sources();
	skip = gwin->skip_lift;
	if (gwin->skip_above_actor < skip)
		skip = gwin->skip_above_actor;
	Nonflat_object_iterator next(olist);

	while ((obj = next.get_next()) != 0)
		if (obj->render_seq != render_seq)
			paint_object(obj);

	skip = 31;			// Back to a safe #.
	return light_sources;
	}

/*
 *	Render an object after first rendering any that it depends on.
 */

void Game_render::paint_object
	(
	Game_object *obj
	)
	{
	int lift = obj->get_lift();
	if (lift >= skip)
		return;
	obj->render_seq = render_seq;
	int cnt = obj->get_dependency_count();
	for (int i = 0; i < cnt; i++)
		{
		Game_object *dep = obj->get_dependency(i);
		if (dep && dep->render_seq != render_seq)
			paint_object(dep);
		}
	obj->paint();			// Finally, paint this one.
	}

/*
 *	Paint 'dirty' rectangle.
 */

void Game_window::paint_dirty()
{
	// Update the gumps before painting, unless in dont_move mode (may change dirty area)
    if (!main_actor_dont_move())
        gump_man->update_gumps();

	Rectangle box = clip_to_win(dirty);
	if (box.w > 0 && box.h > 0)
		paint(box);	// (Could create new dirty rects.)

	clear_dirty();
}

/*
 *	Dungeon Blacking
 *
 *	This is really simple. If there is a dungeon roof over our head	we
 *	black out every tile on screen that doens't have a roof at the height
 *	of the roof that is directly over our head. The tiles are blacked out
 *	at the height of the the roof. 
 *
 *	I've done some simple optimizations. Generally all the blackness will
 *	cover entire chunks. So, instead of drawing each tile individually, I
 *	work out home many tiles in a row that need to be blacked out, and then
 *	black them all out at the same time.
 */

void Game_render::paint_blackness(int start_chunkx, int start_chunky, int stop_chunkx, int stop_chunky, int index)
{
	Game_window *gwin = Game_window::get_instance();
	// Calculate the offset due to the lift (4x the lift).
	//const int off = skip_above_actor << 2;//in_dungeon << 2;
	const int off = gwin->in_dungeon << 2;

	// For each chunk that might be renderable
	for (int cy = start_chunky; cy != stop_chunky; cy = INCR_CHUNK(cy))
	{
		for (int cx = start_chunkx; cx != stop_chunkx; cx = INCR_CHUNK(cx))
		{
			// Coord of the left edge
			const int xoff = 
				Figure_screen_offset(cx, gwin->scrolltx) - off;
			// Coord of the top edge 
			int y = Figure_screen_offset(cy, gwin->scrollty) - off;

			// Need the chunk cache (needs to be setup!)
			Map_chunk *mc = gwin->map->get_chunk(cx, cy);
			mc->setup_cache();
			Chunk_cache *chunk = mc->need_cache();

			// For each line in the chunk
			for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
			{
				// Start and width of the area to black out
				int x = xoff;
				int w = 0;

				// For each tile in the line
				for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
				{
					// If the tile is blocked by 'roof'
					if (!chunk->is_blocked_fast(
						tilex, tiley,
						gwin->skip_above_actor))
					{
						// Add to the width of the area
						w += c_tilesize;
					}
					// If not blocked and have area,
					else if (w)
					{
						// Draw blackness
						gwin->win->fill8(index, w, c_tilesize, x, y);	

						// Set the start of the area to the next tile
						x += w + c_tilesize;

						// Clear the width
						w = 0;	
					}
					// Not blocked, and no area
					else
					{
						// Increment the start of the area to the next tile
						x += c_tilesize;
					}

				}

				// If we have an area, paint it.
				if (w) 
					gwin->win->fill8(index, w, c_tilesize,
									 x, y);

				// Increment the y coord for the next line
				y += c_tilesize;
			}
		}
	}
}
