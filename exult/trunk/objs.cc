/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.cc - Game objects.
 **
 **	Written: 10/1/98 - JSF
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

#include "objs.h"
#include "objiter.h"
#include "egg.h"
#include "gamewin.h"
#include "actors.h"
#include "usecode.h"
#include "items.h"
#include "dir.h"
#include "citerate.h"
#include <string.h>

					// Offset to each neighbor, dir=0-7.
short Tile_coord::neighbors[16] = {0,-1, 1,-1, 1,0, 1,1, 0,1,
							-1,1, -1,0, -1,-1 };

					// Bit 5=S, Bit6=reflect. on diag.
unsigned char Game_object::rotate[8] = { 0, 0, 48, 48, 16, 16, 32, 32};

/*
 *	Get direction to another object.
 */

int Game_object::get_direction
	(
	Game_object *o2
	) const
	{
	Tile_coord t1 = get_abs_tile_coord();
	Tile_coord t2 = o2->get_abs_tile_coord();
					// Treat as cartesian coords.
	return (int) Get_direction(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Does a given shape come in quantity.
 */
static int Has_quantity
	(
	int shnum			// Shape number.
	)
	{
#if 1
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(shnum);
	return info.get_shape_class() == Shape_info::has_quantity;
#else
	switch (shnum)
		{
	case 417:			// Magic bolts.
	case 554:			// Various arrows.
	case 556:
	case 558:
	case 560:
	case 565:
	case 568:
	case 592:			// Spears, axes, daggers.
	case 593:
	case 594:
	case 615:			// Knives.
	case 623:			// Hammers.
	case 627:			// Lockpicks.
	case 636:			// Serp. dagger.
	case 644:			// Gold.
	case 645:
	case 656:
	case 722:			// Arrows.
	case 723:
	case 769:			// Smokebombs.
	case 827:			// Bandages.
	case 842:			// Reagants.
	case 947:
	case 948:
		return 1;
	default:
		return 0;
		}
#endif
	}

const int MAX_QUANTITY = 0x7f;		// Highest quantity possible.

/*
 *	Get the quantity.
 */

int Game_object::get_quantity
	(
	) const
	{
	int shnum = get_shapenum();
	if (Has_quantity(shnum))	// +++++Until we find a flag.
		{
		int qual = quality & 0x7f;
		return qual ? qual : 1;
		}
	else
		return 1;
	}

/*
 *	Get the volume.
 */

int Game_object::get_volume
	(
	) const
	{
	int quant = get_quantity();
	quant = 1 + (quant - 1)/8;	// Be liberal about multiples.
	return (quant *
		Game_window::get_game_window()->get_info(this).get_volume());
	}

/*
 *	Add or remove from object's 'quantity', and delete if it goes to 0.
 *	Also, this sets the correct frame, even if delta == 0.
 *
 *	Output:	Delta decremented/incremented by # added/removed.
 *		Container's volume_used field is updated.
 */

int Game_object::modify_quantity
	(
	int delta			// >=0 to add, <0 to remove.
	)
	{
	if (!Has_quantity(get_shapenum()))
		{			// Can't do quantity here.
		if (delta > 0)
			return (delta);
		remove_this();		// Remove from container (or world).
		return (delta + 1);
		}
	int quant = quality&0x7f;	// Get current quality.
	if (!quant)
		quant = 1;		// Might not be set.
	int newquant = quant + delta;
	if (delta >= 0)			// Adding?
		{			// Too much?
		if (newquant > MAX_QUANTITY)
			newquant = MAX_QUANTITY;
		}
	else if (newquant <= 0)		// Subtracting.
		{
		remove_this();		// We're done for.
		return (delta + quant);
		}
	int oldvol = get_volume();	// Get old volume used.
	quality = 0x80|(char) newquant;	// Store new value.
	int shapenum = get_shapenum();
	Game_window *gwin = Game_window::get_game_window();
					// Set appropriate shape.
	int num_frames = gwin->get_shapes().get_num_frames(shapenum);
	int new_frame = newquant - 1;
	if (new_frame > 7)		// Range is 0-7.
		new_frame = 7;
	if (shapenum != 842)		// ++++Kludge:  reagants.
					// Guessing:  Works for ammo, arrows.
		set_frame(num_frames == 32 ? 24 + new_frame : new_frame);
	Container_game_object *owner = get_owner();
	if (owner)			// Update owner's volume.
		owner->modify_volume_used(get_volume() - oldvol);
	return (delta - (newquant - quant));
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (cx=cy=255).
 */

void Game_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Figure new chunk.
	int newcx = newtx/tiles_per_chunk, newcy = newty/tiles_per_chunk;
	Chunk_object_list *newchunk = gwin->get_objects_safely(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
					// Remove from old.
	Chunk_object_list *oldchunk = gwin->get_objects_safely(cx, cy);
	if (oldchunk)
		{
		gwin->add_dirty(this);	// Want to repaint old area.
		oldchunk->remove(this);
		}
	set_lift(newlift);		// Set new values.
	shape_pos = ((newtx%tiles_per_chunk) << 4) + newty%tiles_per_chunk;
	newchunk->add(this);		// Updates cx, cy.
	gwin->add_dirty(this);		// And repaint new area.
	}

/*
 *	Remove all dependencies.
 */

void Game_object::clear_dependencies
	(
	)
	{
	int cnt = get_dependency_count();
	for (int i = 0; i < cnt; i++)
		dependencies.put(i, 0);
	}

/*
 *	Find objects near a given position.
 *
 *	Output:	# found, appended to vec.
 */

int Game_object::find_nearby
	(
	Vector& vec,			// Objects appended to this.
	Tile_coord pos,			// Look near this point.
	int shapenum,			// Shape to look for.  -1=any,
					//   -359=any NPC.
	int quality,			// +++Not used/understood.
	int mask			// +++Same here.
	)
	{
	int vecsize = vec.get_cnt();
	Game_window *gwin = Game_window::get_game_window();
	const int delta = 16;		// Let's try 16 tiles each dir.
	Rectangle tiles(pos.tx - delta, pos.ty - delta, 2*delta, 2*delta);
					// Stay within world.
	Rectangle world(0, 0, num_chunks*tiles_per_chunk, 
						num_chunks*tiles_per_chunk);
	tiles = tiles.intersect(world);
					// Figure range of chunks.
	int start_cx = tiles.x/tiles_per_chunk,
	    end_cx = (tiles.x + tiles.w - 1)/tiles_per_chunk;
	int start_cy = tiles.y/tiles_per_chunk,
	    end_cy = (tiles.y + tiles.h - 1)/tiles_per_chunk;
					// Go through all covered chunks.
	for (int cy = start_cy; cy <= end_cy; cy++)
		for (int cx = start_cx; cx <= end_cx; cx++)
			{		// Go through objects.
			Chunk_object_list *chunk = gwin->get_objects(cx, cy);
			Object_iterator next(chunk);
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				{	// Check shape.
				if (shapenum >= 0)
					{
					if (obj->get_shapenum() != shapenum)
						continue;
					}
				else if (shapenum == -359 &&
							!obj->get_npc_num() &&
						 obj != gwin->get_main_actor())
					continue;
				int tx, ty, tz;
				obj->get_abs_tile(tx, ty, tz);
					// +++++Check tz too?
				if (tiles.has_point(tx, ty))
					vec.append(obj);
				}
			}
					// Return # added.
	return (vec.get_cnt() - vecsize);
	}

/*
 *	Find the closest nearby object with a shape in a given list.
 *
 *	Output:	->object, or 0 if none found.
 */

Game_object *Game_object::find_closest
	(
	int *shapenums,			// Shapes to look for. -359=any NPC.
	int num_shapes			// Size of shapenums.
	)
	{
	Vector vec;			// Gets objects found.
	int cnt = 0;
	int i;
	for (i = 0; i < num_shapes; i++)
		cnt += find_nearby(vec, shapenums[i], -359, 0);
	if (!cnt)
		return (0);
	Game_object *closest = 0;	// Get closest.
	int best_dist = 10000;		// In tiles.
					// Get our location.
	Tile_coord loc = get_abs_tile_coord();
	for (i = 0; i < cnt; i++)
		{
		Game_object *obj = (Game_object *) vec.get(i);
		int dist = obj->get_abs_tile_coord().distance(loc);
		if (dist < best_dist)
			{
			closest = obj;
			best_dist = dist;
			}
		}
	return (closest);
	}

/*
 *	Find a free tile within given distance.
 *
 *	Output:	Tile, or (-1, -1, -1) if failed.
 */

Tile_coord Game_object::find_unblocked_tile
	(
	int dist,			// 1 means adjacent.
	int height			// Height to check for unblocked.
	)
	{
	Tile_coord pos = get_abs_tile_coord();
					// Get box to go through.
	Rectangle box(pos.tx - dist, pos.ty - dist, 2*dist + 1, 2*dist + 1);
	Rectangle world(0, 0, num_tiles, num_tiles);
	box = box.intersect(world);
	int stopx = box.x + box.w, stopy = box.y + box.h;
	for (int y = box.y; y < stopy; y++)
		for (int x = box.x; x < stopx; x++)
			{		// Check this spot.
			Tile_coord spot(x, y, pos.tz);
			if (!Chunk_object_list::is_blocked(spot, height))
				return spot;
			}
	return Tile_coord(-1, -1, -1);
	}

/*
 *	Find the game object that's blocking a given tile.
 *
 *	Output:	->object, or 0 if not found.
 */

Game_object *Game_object::find_blocking
	(
	Tile_coord tile			// Tile to check.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Chunk_object_list *chunk = gwin->get_objects(tile.tx/tiles_per_chunk,
						     tile.ty/tiles_per_chunk);
	Game_object *obj;
	Object_iterator next(chunk);
	while ((obj = next.get_next()) != 0)
		{
		int tx, ty, tz;		// Get object's coords.
		obj->get_abs_tile(tx, ty, tz);
		if (tx < tile.tx || ty < tile.ty || tz > tile.tz)
			continue;	// Out of range.
		Shape_info& info = gwin->get_info(obj);
		int ztiles = info.get_3d_height(); 
		if (!ztiles || !info.is_solid())
			continue;	// Skip if not an obstacle.
					// Occupies desired tile?
		int frame = obj->get_framenum();
		if (tile.tx > tx - info.get_3d_xtiles(frame) &&
		    tile.ty > ty - info.get_3d_ytiles(frame) &&
		    tile.tz < tz + ztiles)
			return (obj);	// Found it.
		}
	return (0);
	}

/*
 *	Is this a closed door?
 */

int Game_object::is_closed_door
	(
	) const
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(this);
	if (!info.is_door())
		return 0;
					// Get door's footprint.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
					// Get its location.
	Tile_coord doortile = get_abs_tile_coord();
	Tile_coord before, after;	// Want tiles to both sides.
	if (xtiles > ytiles)		// Horizontal footprint?
		{
		before = doortile + Tile_coord(-xtiles, 0, 0);
		after = doortile + Tile_coord(1, 0, 0);
		}
	else				// Vertical footprint.
		{
		before = doortile + Tile_coord(0, -ytiles, 0);
		after = doortile + Tile_coord(0, 1, 0);
		}
					// Should be blocked before/after.
	return (Chunk_object_list::is_blocked(before) &&
	    	Chunk_object_list::is_blocked(after));
	}

/*
 *	Get the topmost owner of this object.
 *
 *	Output:	->topmost owner, or the object itself.
 */

Game_object *Game_object::get_outermost
	(
	)
	{
	Game_object *top = this;
	Game_object *above;
	while ((above = top->get_owner()) != 0)
		top = above;
	return top;
	}

/*
 *	Show text by the object on the screen.
 */

void Game_object::say
	(
	const char *text
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Rectangle box = gwin->get_shape_rect(this);
	gwin->add_text(text, box.x, box.y);
	}

/*
 *	Show a random string from 'text.flx' by the object.
 */

void Game_object::say
	(
	int from, int to		// Range (inclusive).
	)
	{
	int offset = rand()%(to - from + 1);
	say(item_names[from + offset]);
	}

/*
 *	Paint at given spot in world.
 */

void Game_object::paint
	(
	Game_window *gwin
	)
	{
	int tx, ty, tz;
	get_abs_tile(tx, ty, tz);
	int liftpix = 4*tz;
	gwin->paint_shape(
		(tx + 1 - gwin->get_scrolltx())*tilesize - 1 - liftpix,
		(ty + 1 - gwin->get_scrollty())*tilesize - 1 - liftpix,
					get_shapenum(), get_framenum());
#if 0	/* OLD WAY++++++++++++ */
	int xoff = (cx - gwin->get_chunkx())*chunksize;
	int yoff = (cy - gwin->get_chunky())*chunksize;
	gwin->paint_shape(xoff + (1 + get_tx())*tilesize - 1 - 4*lift, 
				yoff + (1 + get_ty())*tilesize - 1 - 4*lift,
					get_shapenum(), get_framenum());
#endif
	}

/*
 *	Run usecode when double-clicked.
 */

void Game_object::activate
	(
	Usecode_machine *umachine
	)
	{
	umachine->call_usecode(get_shapenum(), this,
				Usecode_machine::double_click);
	}

/*
 *	For objects that can have a quantity, the name is in the format:
 *		%1/%2/%3/%4
 *	Where
 *		%1 : singular prefix (e.g. "a")
 *		%2 : main part of name
 *		%3 : singular suffix
 *		%4 : plural suffix (e.g. "s")
 */

/*
 *	Extracts the first, second and third parts of the name string
 */
static void get_singular_name
	(
	const char *name,		// Raw name string from TEXT.FLX
	string& output_name		// Output string
	)
	{
	if(*name != '/')		// Output the first part
		{
		do
			output_name += *name++;
		while(*name != '/' && *name != '\0');
		if(*name == '\0')	// should not happen
			{
			output_name = "?";
			return;
			}
		// If there is a first part it is followed by a space
		output_name += ' ';
		}
	name++;

					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;

					// Output the third part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
}

/*
 *	Extracts the second and fourth parts of the name string
 */
static void get_plural_name
	(
	const char *name,
	int quantity,
	string& output_name
	)
	{
	char buf[20];

	sprintf(buf, "%d ", quantity);	// Output the quantity
	output_name = buf;

					// Skip the first part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Skip the third part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
	while(*name != '\0')		// Output the last part
		output_name += *name++;
}

/*
 *	Returns the string to be displayed when the item is clicked on
 */
string Game_object::get_name
	(
	) const
	{
	const char *name;
	int quantity;
	string display_name;
	int shnum = get_shapenum();
	switch (shnum)			// Some special cases!
		{
	case 0x34a:			// Reagants.
		name = item_names[0x500 + get_framenum()];
		break;
	case 0x3bb:			// Medallions?
		name = item_names[0x508 + get_framenum()];
		break;
	case 0x179:			// Food items.
		name = item_names[0x50b + get_framenum()];
		break;
	case 0x2a3:			// Desk item.
		name = item_names[0x52d + get_framenum()];
		break;
	default:
		name = item_names[shnum];
		break;
		}
	if(name == 0)
		return "?";

	if (Has_quantity(shnum))
		quantity = quality & 0x7f;
	else
		quantity = 1;

	// If there are no slashes then it is simpler
	if(strchr(name, '/') == 0)
		{
		if(quantity <= 1)
			display_name = name;
		else
			{
			char buf[50];

			sprintf(buf, "%d %s", quantity, name);
			display_name = buf;
			}
		}
	else if(quantity <= 1)		// quantity might be zero?
		get_singular_name(name, display_name);
	else
		get_plural_name(name, quantity, display_name);
	return display_name;
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	Chunk_object_list *chunk = 
			Game_window::get_game_window()->get_objects_safely(
								cx, cy);
	if (chunk)
		chunk->remove(this);
	if (!nodel)
		delete this;
	}

/*
 *	Can this be dragged?
 */

int Game_object::is_dragable
	(
	) const
	{
	return (0);			// Default is 'no'.
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Game_object::drop
	(
	Game_object *obj		// This may be deleted.
	)
	{
	int shapenum = get_shapenum();	// It's possible if shapes match.
	if (obj->get_shapenum() != shapenum ||
	    !Has_quantity(shapenum))
		return (0);
	int objq = obj->get_quantity();
	int total_quant = get_quantity() + objq;
	if (total_quant > MAX_QUANTITY)	// Too much?
		return (0);
	modify_quantity(objq);		// Add to our quantity.
	obj->remove_this();		// It's been used up.
	return (1);
	}

#ifdef DEBUGLT
static int rx1 = -1, ry1 = -1, rx2 = -1, ry2 = -1;

static void Debug_lt
	(
	int tx1, int ty1,		// 1st coord.
	int tx2, int ty2		// 2nd coord.
	)
	{
	if (tx1 == rx1 && ty1 == ry1)
		{
		if (tx2 == rx2 && ty2 == ry2)
			cout << "Debug_lt" << endl;
		}
	}
#endif

/*
 *	Should this object be rendered before obj2?
 *
 *	Output:	1 if so, 0 if not, -1 if cannot compare.
 */
int Game_object::lt
	(
	Game_object& obj2
	) const
	{
	Game_window *gwin = Game_window::get_game_window();
					// See if there's no overlap.
	Rectangle r1 = gwin->get_shape_rect(this),
		  r2 = gwin->get_shape_rect(&obj2);
	if (!r1.intersects(r2))
		return (-1);		// No overlap on screen.
	Shapes_vga_file& shapes = gwin->get_shapes();
	int shapenum1 = get_shapenum(), shapenum2 = obj2.get_shapenum();
	int framenum1 = get_framenum(), framenum2 = obj2.get_framenum();
	Shape_info& info1 = shapes.get_info(shapenum1);
	Shape_info& info2 = shapes.get_info(shapenum2);
					// Get absolute tile positions.
	int atx1, aty1, atz1, atx2, aty2, atz2;
	int x1, x2, y1, y2, z1, z2;	// Dims. in tiles.
	get_abs_tile(atx1, aty1, atz1);
	obj2.get_abs_tile(atx2, aty2, atz2);
#ifdef DEBUGLT
	Debug_lt(atx1, aty1, atx2, aty2);
#endif
	x1 = info1.get_3d_xtiles(framenum1);
	x2 = info2.get_3d_xtiles(framenum2);
	y1 = info1.get_3d_ytiles(framenum1); 
	y2 = info2.get_3d_ytiles(framenum2);
	z1 = info1.get_3d_height(), z2 = info2.get_3d_height();
	int result = -1;		// Watch for conflicts.
	if (atz1 != atz2)		// Is one obj. on top of another?
		{
		if (atz1 + z1 <= atz2)
			result = 1;	// It's above us.
		else if (atz2 + z2 <= atz1)
			result = 0;	// We're above.
		}
	if (aty1 != aty2)		// Is one obj. in front of the other?
		{
		if (aty1 <= aty2 - y2)
			{		// Obj2 is in front.
			if (result == 0)// Conflict, so return 'neither'.
				return -1;
			result = 1;
			}
		else if (aty2 <= aty1 - y1)
			{		// We're in front.
			if (result == 1)
				return -1;
			result = 0;
			}
		}
	if (atx1 != atx2)		// Is one obj. to right of the other?
		{
		if (atx1 <= atx2 - x2)
			{		// Obj2 is to right of us.
			if (result == 0)
				return -1;
			result = 1;
			}
		if (atx2 <= atx1 - x1)
			{		// We're to the right.
			if (result == 1)
				return -1;
			result = 0;
			}
		}
	if (result != -1)		// Consistent result?
		return result;
	if (!z1 && atz1 <= atz2)	// We're flat and below?
		return (1);
	if (!z2 && atz2 <= atz1)	// It's below us?
		return (0);
#if 0					// Below 2nd?
	if (atz1 < atz2 && atz1 + z1 <= atz2 + z2)
		return (1);
					// Above 2nd?
	else if (atz2 < atz1 && atz2 + z2 <= atz1 + z1)
		return (0);
#endif
					// Handle intersecting objects.
	if (atx1 == atx2 &&		// Watch for paintings on NS walls.
	    x1 == x2)
		if (y1 < y2)		// Take narrower 2nd.
			return (0);
		else if (y2 > y1)
			return (1);
		else if (z1 < z2)	// The shorter one?
			return (0);
		else if (z1 > z2)
			return (1);
					// If x's overlap, see if in front.
	if ((atx1 > atx2 - x2 && atx1 <= atx2) ||
	    (atx2 > atx1 - x1 && atx2 <= atx1))
		{
		if (aty1 < aty2)
			return (1);
		else if (aty1 > aty2)
			return (0);
		else if (z1 < z2)	// The shorter one?
			return (0);
		else if (z1 > z2)
			return (1);
		else if (x1 < x2)	// Take the narrower one as greater.
			return (0);
		else if (x1 > x2)
			return (1);
		}
					// If y's overlap, see if to left.
	if ((aty1 > aty2 - y2 && aty1 <= aty2) ||
	    (aty2 > aty1 - y1 && aty2 <= aty1))
		{
		if (atx1 < atx2)
			return (1);
		else if (atx1 > atx2)
			return (0);
		}
	return (-1);
	}

/*
 *	Get frame if rotated 1, 2, or 3 quadrants clockwise.  This is to
 *	support barges (ship, cart, flying carpet).
 */

int Game_object::get_rotated_frame
	(
	int quads			// 1=90, 2=180, 3=270.
	)
	{
	int curframe = get_framenum();
	switch (get_shapenum())		// Wish I knew a better way...
		{
	case 251:			// Sails.  Wind direction?
		{
		static char swaps180[8] = {3, 2, 1, 0, 7, 6, 5, 4};
		int subframe = curframe&7;
		switch (quads)
			{
		case 1:			// 90 right.
			return (curframe&32) ? swaps180[subframe]
					: (curframe|32);
		case 3:			// 90 left.
			return (curframe&32) ? subframe
					: (swaps180[subframe]|32);
		case 2:			// 180.
			return swaps180[subframe] | (curframe&32);
			}
		}
	case 301:			// Step onto cart.
	case 292:			// Seat.  Sequential frames for dirs.
		{
		int dir = curframe%4;	// Current dir (0-3).
		return (curframe - dir) + (dir + quads)%4;
		}
	case 665:			// Prow.  Frames 0,3 are N, 1,2 S.
		{
		static char dirs[4] = {0, 2, 2, 0};
		int subframe = curframe%4;
		int dir = (4 + dirs[subframe] - ((curframe>>5)&1) + quads)%4;
		static int subframes[4] = {0, 33, 1, 32};
		return ((curframe - subframe)&31) + subframes[dir];
		}
	case 700:			// Deck.
		{
		static char swaps180[12] = {2, 0, 0, 1, 5, 4, 7, 6, 8, 9, 
								10, 11};
		int subframe = curframe&15;
		switch (quads)
			{
		case 1:			// 90 left.
			return (curframe&32) ? (curframe&31) :
				(swaps180[subframe]|32);
		case 3:			// 90 right.
			return (curframe&32) ? swaps180[subframe] :
				(subframe|32);
		case 2:			// 180.  Flip around.
			return swaps180[subframe]|(curframe&32);
			}
		}
	case 775:			// Ship rails.
		{
		static char swaps180[8] = {2, 3, 0, 1, 6, 7, 4, 5};
		static char swaps90r[8] = {1, 0, 3, 2, 5, 4, 7, 6};
		int subframe = curframe&7;
		switch (quads)
			{
		case 1:			// 90 right?
			{
			int swapped = swaps90r[subframe];
			return (curframe&32) ? swaps180[swapped]
					     : (swapped|32);
			}
		case 3:			// 90 left?  Go 180 + 90 right.
			{
			int swapped = swaps90r[swaps180[subframe]];
			return (curframe&32) ? swaps180[swapped]
					     : (swapped|32);
			}
		case 2:			// 180?
			return swaps180[subframe] | (curframe&32);
		default:
			return curframe;
			}
		}
	case 781:			// Gang plank.  Odd frames are rot.
	case 1017:			// Ship.
		{
		int newframe = (curframe + quads)%4;
		return newframe | ((newframe&1)<<5);
		}
	case 791:			// Ship.
		{
		static char swaps180[4] = {2, 3, 0, 1};
		static char swaps90r[4] = {1, 0, 3, 2};
		int subframe = curframe&3;
		switch (quads)
			{
		case 3:			// 90 left.
			subframe = swaps180[subframe];
					// FALL through.
		case 1:			// 90 right.
			{
			int swapped = swaps90r[subframe];
			return (curframe&32) ? swaps180[swapped]
					     : (swapped|32);
			}
		case 2:
		default:
			return swaps180[subframe]|(curframe&32);
			}
		}
	case 660:
	case 652:
	case 757:			// Piece of the cart.
		{			// Groups of 4.
		static char swaps180[4] = {2, 3, 0, 1};
		static char swaps90r[4] = {1, 0, 3, 2};
		int subframe = curframe&3;
		switch (quads)
			{
		case 3:			// 90 left.
			subframe = swaps180[subframe];
					// FALL through.
		case 1:			// 90 right.
			{
			int swapped = swaps90r[subframe];
			return (curframe&32) ? swaps180[swapped]
					     : (swapped|32);
			}
		case 2:
		default:
			return swaps180[subframe]|(curframe&32);
			}
		}
		
	case 796:			// Draft horse.
		{
					// Groups of 4:  0, 3 are N, 1, 2 S.
		int subframe = curframe&3;
		int curdir = (4 + 2*(((subframe>>1)&1)^(subframe&1)) -
							((curframe>>5)&1))%4;
		int newdir = (curdir + quads)%4;
		static char frames[4] = {0, 33, 1, 32};
		return (curframe&31) - subframe + frames[newdir];
		}
	case 840:			// Magic carpet.
		{
		if (curframe >= 24)	// Plain square.
			return curframe;
		static char rot[24] = {	// Values for 90 degrees:
		//      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
			7, 4, 5, 6, 3, 0, 1, 2,14,12,13,15,11, 8,10, 9,22,
		//	17 18 19 20 21 22 23
			20,21,23,16,19,17,18 };
		while (quads--)
			curframe = rot[curframe];
		return curframe;
		}
	default:			// Reflect.  Bit 32==horizontal.
		return curframe ^ ((quads%2)<<5);
		}
	}

/*
 *	Being attacked.
 */

void Game_object::attacked
	(
	Actor *attacker,
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int wpoints;
	Weapon_info *winf;
	if (weapon_shape > 0)
		winf = gwin->get_info(weapon_shape).get_weapon_info();
	else if (ammo_shape > 0)	// Not sure about all this...
		winf = gwin->get_info(ammo_shape).get_weapon_info();
	else
		winf = attacker->get_weapon(wpoints);
	int usefun;			// Run usecode if present.
	if (winf && (usefun = winf->get_usecode()) != 0)
		gwin->get_usecode()->call_usecode(usefun, this,
					Usecode_machine::weapon);
	}

/*
 *	Write the common IREG data for an entry.
 */

void Game_object::write_common_ireg
	(
	unsigned char *buf		// 4-byte buffer to be filled.
	)
	{
					// Coords:
	buf[0] = ((get_cx()%16) << 4) | get_tx();
	buf[1] = ((get_cy()%16) << 4) | get_ty();
	int shapenum = get_shapenum(), framenum = get_framenum();
	buf[2] = shapenum&0xff;
	buf[3] = ((shapenum>>8)&3) | (framenum<<2);
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (cx=cy=255).
 */

void Ireg_game_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	if (owner)			// Watch for this.
		{
		owner->remove(this);
		set_invalid();		// So we can safely move it back.
		}
	Game_object::move(newtx, newty, newlift);
	}

/*
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Ireg_game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	if (owner)			// In a bag, box, or person.
		owner->remove(this);
	else				// In the outside world.
		{
		Chunk_object_list *chunk = 
			Game_window::get_game_window()->get_objects_safely(
								cx, cy);
		if (chunk)
			chunk->remove(this);
		}
	if (!nodel)
		delete this;
	}

/*
 *	Can this be dragged?
 */

int Ireg_game_object::is_dragable
	(
	) const
	{
	Game_window *gwin = Game_window::get_game_window();
					// 0 weight means 'too heavy'.
	return gwin->get_info(this).get_weight() > 0;
	}

/*
 *	Write out.
 */

void Ireg_game_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[7];		// 6-byte entry + length-byte.
	buf[0] = 6;
	write_common_ireg(&buf[1]);
	buf[5] = (get_lift()&15)<<4;
	buf[6] = get_quality();
	out.write((char*)buf, sizeof(buf));
	}

/*
 *	Create a spellbook from Ireg data.
 */

Spellbook_object::Spellbook_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex, unsigned int shapey, 
	unsigned int lft, 
	unsigned char *c,		// Circle spell flags.
	unsigned long f			// Flags (unknown).
	) : Ireg_game_object(l, h, shapex, shapey, lft), flags(f), bookmark(-1)
	{
	memcpy(circles, c, sizeof(circles));
	}

/*
 *	Add a spell.
 *
 *	Output:	0 if already have it, 1 if successful.
 */

int Spellbook_object::add_spell
	(
	int spell			// 0-71
	)
	{
	int circle = spell/8;
	int num = spell%8;		// # within circle.
	if (circles[circle] & (1<<num))
		return 0;		// Already have it.
	circles[circle] |= (1<<num);
	return 1;
	}

/*
 *	Show book when double-clicked.
 */

void Spellbook_object::activate
	(
	Usecode_machine *umachine
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->show_gump(this, 43);
	}

/*
 *	Write out.
 */

void Spellbook_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[19];		// 18-byte entry + length-byte.
	buf[0] = 18;
	unsigned char *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
	memcpy(ptr, &circles[0], 5);	// Store the way U7 does it.
	ptr += 5;
	*ptr++ = (get_lift()&15)<<4;	// Low bits?++++++
	memcpy(ptr, &circles[5], 4);	// Rest of spell circles.
	ptr += 4;
	Write4(ptr, flags);
	out.write((char*)buf, sizeof(buf));
	}

/*
 *	Delete all contents.
 */

Container_game_object::~Container_game_object
	(
	)
	{
	if (objects)
		{
		Game_object *first = objects;
		Game_object *obj;
		do
			{
			obj = objects;
			objects = obj->get_next();
			delete obj;
			}
		while (obj != first);
		}
	}

/*
 *	Remove an object.
 */

void Container_game_object::remove
	(
	Game_object *obj
	)
	{
	if (!objects)
		return;
	volume_used -= obj->get_volume();
	obj->set_owner(0);
	if (obj == objects)		// Head of list?
		{
		objects = obj->get_next();
		if (obj == objects)	// Last in chain?
			objects = 0;
		}
	obj->remove_from_chain();
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Container_game_object::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume check.
	)
	{
	if (obj->get_shapenum() == get_shapenum())
		return (0);		// Can't put a bag in a bag.
	int maxvol;			// Note:  NPC's have 0 volume.
	if (!dont_check && (maxvol = get_max_volume()) > 0)
		{
		int objvol = obj->get_volume();
		if (objvol + volume_used > maxvol)
			return (0);	// Doesn't fit.
		volume_used += objvol;
		}
	obj->set_owner(this);		// Set us as the owner.
					// Append to chain.
	objects = obj->append_to_chain(objects);
	return 1;
	}

/*
 *	Change shape of a member.
 */

void Container_game_object::change_member_shape
	(
	Game_object *obj,
	int newshape
	)
	{
	int oldvol = obj->get_volume();
	obj->set_shape(newshape);
					// Update total volume.
	volume_used += obj->get_volume() - oldvol;
	}

/*
 *	Recursively add a quantity of an item to those existing in
 *	this container, and create new objects if necessary.
 *
 *	Output:	Delta decremented # added.
 */

int Container_game_object::add_quantity
	(
	int delta,			// Quantity to add.
	int shapenum,			// Shape #.
	int qual,			// Quality, or -359 for any.
	int framenum,			// Frame, or -359 for any.
	int dontcreate			// If 1, don't create new objs.
	)
	{
					// Get volume of 1 object.
	int objvol = Game_window::get_game_window()->get_info(
			shapenum).get_volume();
	int maxvol = get_max_volume();	// 0 means anything (NPC's?).
	int roomfor = maxvol ? (maxvol - volume_used)/objvol : 20000;
	int todo = delta < roomfor ? delta : roomfor;
	Game_object *obj;
	if (objects)
		{			// First try existing items.
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			{
			if (obj->get_shapenum() == shapenum &&
		    	 (framenum == -359 || obj->get_framenum() == framenum))
					// ++++++Quality???
				{
				int used = 
				    todo - obj->modify_quantity(todo);
				todo -= used;
				delta -= used;
				}
			}
		next.reset();			// Now try recursively.
		while ((obj = next.get_next()) != 0)
			delta = obj->add_quantity(
					delta, shapenum, qual, framenum, 1);
		}
	if (!delta || dontcreate)	// All added?
		return (delta);
	else
		return (create_quantity(delta, shapenum, qual,
				framenum == -359 ? 0 : framenum));
	}

/*
 *	Recursively create a quantity of an item.
 *
 *	Output:	Delta decremented # added.
 */

int Container_game_object::create_quantity
	(
	int delta,			// Quantity to add.
	int shapenum,			// Shape #.
	int qual,			// Quality, or -359 for any.
	int framenum			// Frame.
	)
	{
					// Get volume of 1 object.
	int objvol = Game_window::get_game_window()->get_info(
			shapenum).get_volume();
	int maxvol = get_max_volume();	// 0 means anything (NPC's?).
	int roomfor = maxvol ? (maxvol - volume_used)/objvol : 20000;
	int todo = delta < roomfor ? delta : roomfor;
	while (todo)			// Create them here first.
		{
		Game_object *newobj = new Ireg_game_object(shapenum, framenum,
								0, 0, 0);
		if (!add(newobj))
			{
			delete newobj;
			break;
			}
		if (qual != -359)	// Set desired quality.
			newobj->set_quality(qual);
		todo--; delta--;
		if (todo > 0)
			{
			int used = 
				todo - newobj->modify_quantity(todo);
			todo -= used;
			delta -= used;
			}
		}
	if (!delta)			// All done?
		return (0);
					// Now try those below.
	Game_object *obj;
	if (!objects)
		return (delta);
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		delta = obj->create_quantity(delta, shapenum, qual, framenum);
	return (delta);
	}		

/*
 *	Recursively remove a quantity of an item from those existing in
 *	this container.
 *
 *	Output:	Delta decremented by # removed.
 */

int Container_game_object::remove_quantity
	(
	int delta,			// Quantity to remove.
	int shapenum,			// Shape #.
	int qual,			// Quality, or -359 for any.
	int framenum			// Frame, or -359 for any.
	)
	{
	if (!objects)
		return delta;		// Empty.
	Game_object *obj = objects;
	Game_object *next;
	int done = 0;
	while (!done && delta)
		{
		next = obj->get_next();	// Might be deleting obj.
		if (obj->get_shapenum() == shapenum &&
		    (framenum == -359 || obj->get_framenum() == framenum))
					// ++++++Quality???
			delta = -obj->modify_quantity(-delta);
					// Do it recursively.
		delta = obj->remove_quantity(delta, shapenum, qual, framenum);
		obj = next;
		done = (!obj || obj == objects);
		}
	return (delta);
	}

/*
 *	Remove and return a desired item.
 *
 *	Output:	->object if found, else 0.
 */

Game_object *Container_game_object::remove_and_return
	(
	int shapenum,			// Shape #.
	int qual,			// Quality, or -359 for any. ???+++++
	int framenum			// Frame, or -359 for any.
	)
	{
	if (!objects)
		return 0;		// Empty.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if (obj->get_shapenum() == shapenum &&
		    (framenum == -359 || obj->get_framenum() == framenum))
					// ++++++Quality???
			{		// Found.
			remove(obj);
			return (obj);
			}
					// Do it recursively.
		Game_object *found = 
			obj->remove_and_return(shapenum, qual, framenum);
		if (found)
			return (found);
		}
	return (0);
	}

/*
 *	Run usecode when double-clicked.
 */

void Container_game_object::activate
	(
	Usecode_machine *umachine
	)
	{
	int shnum = get_shapenum();
	Game_window *gwin = Game_window::get_game_window();
	switch(shnum)			// Watch for gumps.
		{
	case 406:			// Nightstand.
		gwin->show_gump(this, 27);
		return;
	case 407:			// Desk.
	case 283:
	case 203:
		gwin->show_gump(this, 27);
		return;
	case 400:			// Bodies.
	case 414:
	case 762:
	case 778:
	case 892:
	case 507: 			// Bones
		gwin->show_gump(this, 53);
		return;
	case 416:			// Chest of drawers.
	case 679:
		gwin->show_gump(this, 27);
		return;
	case 800:			// Chest.
		gwin->show_gump(this, 22);	// ???Guessing.
		return;
	case 801:			// Backpack.
		gwin->show_gump(this, 10);
		return;
	case 799:			// Unsealed box
		gwin->show_gump(this, 0);
		return;
	case 802:			// Bag.
		gwin->show_gump(this, 9);
		return;
	case 803:			// Basket.
		gwin->show_gump(this, 11);
		return;
	case 804:			// Crate.
		gwin->show_gump(this, 1);
		return;
	case 819:			// Barrel.
		gwin->show_gump(this, 8);
		return;
		}
					// Try to run normal usecode fun.
	umachine->call_usecode(shnum, this,
				Usecode_machine::double_click);
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Container_game_object::drop
	(
	Game_object *obj
	)
	{
	if (!get_owner())		// Only accept if inside another.
		return (0);
	return (add(obj));		// We'll take it.
	}

/*
 *	Recursively count all objects of a given shape.
 */

int Container_game_object::count_objects
	(
	int shapenum,			// Shape#, or -359 for any.
	int qual,			// Quality, or -359 for any.
	int framenum			// Frame#, or -359 for any.
	)
	{
	int total = 0;
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if ((shapenum == -359 || obj->get_shapenum() == shapenum) &&
		    (framenum == -359 || obj->get_framenum() == framenum) &&
		    (qual == -359 || obj->get_quality() == qual))
			{		// Check quantity.
			int quant = obj->get_quantity();
			total += quant;
			}
					// Count recursively.
		total += obj->count_objects(shapenum, qual, framenum);
		}
	return (total);
	}

/*
 *	Recursively get all objects of a given shape.
 */

int Container_game_object::get_objects
	(
	Vector& vec,			// Objects returned here.
	int shapenum,			// Shape#, or -359 for any.
	int framenum			// Frame#, or -359 for any.
	)
	{
	int vecsize = vec.get_cnt();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if ((shapenum == -359 || obj->get_shapenum() == shapenum) &&
		    (framenum == -359 || obj->get_framenum() == framenum))
			vec.append(obj);
					// Search recursively.
		obj->get_objects(vec, shapenum);
		}
	return (vec.get_cnt() - vecsize);
	}

/*
 *	Write out container and its members.
 */

void Container_game_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	unsigned char *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
					// Guessing: +++++
	unsigned short tword = objects ? objects->get_prev()->get_shapenum() 
									: 0;
	Write2(ptr, tword);
	*ptr++ = 0;			// Unknown.
	*ptr++ = get_quality();
	int npc = get_live_npc_num();	// If body, get source.
	int quant = (npc >= 0 && npc <= 127) ? (npc + 0x80) : 0;
	*ptr++ = quant&0xff;		// "Quantity".
	*ptr++ = (get_lift()&15)<<4;
	*ptr++ = 0;			// Resistance+++++
	*ptr++ = 0;			// Flags++++++
	out.write((char*)buf, sizeof(buf));
	write_contents(out);		// Write what's contained within.
	}

/*
 *	Write contents (if there is any).
 */

void Container_game_object::write_contents
	(
	ostream& out
	)
	{
	if (objects)			// Now write out what's inside.
		{
		Game_object *obj;
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			obj->write_ireg(out);
		out.put(0x01);		// A 01 terminates the list.
		}
	}

/*
 *	Create the cached data storage for a chunk.
 */

Chunk_cache::Chunk_cache
	(
	) : setup_done(0), egg_objects(0, 4)
	{
	memset((char *) &blocked[0], 0, sizeof(blocked));
	memset((char *) &eggs[0], 0, sizeof(eggs));
	}

/*
 *	Delete cache.
 */

Chunk_cache::~Chunk_cache
	(
	)
	{
	}

/*
 *	Set/unset the blocked flags in a region.
 */

void Chunk_cache::set_blocked
	(
	int startx, int starty,		// Starting tile #'s.
	int endx, int endy,		// Ending tile #'s.
	int lift, int ztiles,		// Lift, height info.
	int set				// 1 to add, 0 to remove.
	)
	{
	if (set)
		{
		for (int y = starty; y <= endy; y++)
			for (int x = startx; x <= endx; x++)
				set_blocked_tile(x, y, lift, ztiles);
		}
	else
		{
		for (int y = starty; y <= endy; y++)
			for (int x = startx; x <= endx; x++)
				clear_blocked_tile(x, y, lift, ztiles);
		}
	}

/*
 *	Add/remove an object to/from the cache.
 */

void Chunk_cache::update_object
	(
	Chunk_object_list *chunk,
	Game_object *obj,
	int add				// 1 to add, 0 to remove.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(obj);
	int ztiles = info.get_3d_height(); 
	if (!ztiles || !info.is_solid())
		return;			// Skip if not an obstacle.
					// Get lower-right corner of obj.
	int endx = obj->get_tx();
	int endy = obj->get_ty();
	int frame = obj->get_framenum();// Get footprint dimensions.
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	int lift = obj->get_lift();
	if (xtiles == 1 && ytiles == 1)	// Simplest case?
		{
		if (add)
			set_blocked_tile(endx, endy, lift, ztiles);
		else
			clear_blocked_tile(endx, endy, lift, ztiles);
		return;
		}
	Tile_coord endpt = obj->get_abs_tile_coord();
	Rectangle footprint(endpt.tx - xtiles + 1, endpt.ty - ytiles + 1, 
							xtiles, ytiles);
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(footprint);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		gwin->get_objects(cx, cy)->set_blocked(tiles.x, tiles.y, 
			tiles.x + tiles.w - 1, tiles.y + tiles.h - 1, lift,
								ztiles, add);
	}

/*
 *	Set a rectangle of tiles within this chunk to be under the influence
 *	of a given egg, or clear it.
 */

void Chunk_cache::set_egged
	(
	Egg_object *egg,
	Rectangle& tiles,		// Range of tiles within chunk.
	int add				// 1 to add, 0 to remove.
	)
	{
					// Egg already there?
	int eggnum = egg_objects.find(egg);
	if (add)
		{
		if (eggnum < 0)		// No, so add it.
			eggnum = egg_objects.put(egg);
		if (eggnum > 15)	// We only have 16 bits.
			eggnum = 15;
		short mask = (1<<eggnum);
		int stopx = tiles.x + tiles.w, stopy = tiles.y + tiles.h;
		for (int ty = tiles.y; ty < stopy; ty++)
			for (int tx = tiles.x; tx < stopx; tx++)
				eggs[ty*tiles_per_chunk + tx] |= mask;
		}
	else				// Remove.
		{
		if (eggnum < 0)
			return;		// Not there.
		egg_objects.put(eggnum, 0);
		if (eggnum >= 15)	// We only have 16 bits.
			{		// Last one at 15 or above?
			int num_eggs = get_num_eggs();
			for (int i = 15; i < num_eggs; i++)
				if (egg_objects.get(i))
					// No, so leave bits alone.
					return;
			eggnum = 15;
			}
		short mask = ~(1<<eggnum);
		int stopx = tiles.x + tiles.w, stopy = tiles.y + tiles.h;
		for (int ty = tiles.y; ty < stopy; ty++)
			for (int tx = tiles.x; tx < stopx; tx++)
				eggs[ty*tiles_per_chunk + tx] &= mask;
		}
	}

/*
 *	Add/remove an egg to the cache.
 */

void Chunk_cache::update_egg
	(
	Chunk_object_list *chunk,
	Egg_object *egg,
	int add				// 1 to add, 0 to remove.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get footprint with abs. tiles.
	Rectangle foot = egg->get_area();
	Rectangle crect;		// Gets tiles within each chunk.
	int cx, cy;
	if (egg->is_solid_area())
		{			// Do solid rectangle.
		Chunk_intersect_iterator all(foot);
		while (all.get_next(crect, cx, cy))
			gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
		return;
		}
					// Just do the perimeter.
	Rectangle top(foot.x, foot.y, foot.w, 1);
	Rectangle bottom(foot.x, foot.y + foot.h - 1, foot.w, 1);
	Rectangle left(foot.x, foot.y + 1, 1, foot.h - 2);
	Rectangle right(foot.x + foot.w - 1, foot.y + 1, 1, foot.h - 2);
					// Go through intersected chunks.
	Chunk_intersect_iterator tops(top);
	while (tops.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator bottoms(bottom);
	while (bottoms.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator lefts(left);
	while (lefts.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);
	Chunk_intersect_iterator rights(right);
	while (rights.get_next(crect, cx, cy))
		gwin->get_objects(cx, cy)->set_egged(egg, crect, add);

	}

/*
 *	Create the cached data for a chunk.
 */

void Chunk_cache::setup
	(
	Chunk_object_list *chunk
	)
	{
	Game_object *obj;		// Set 'blocked' tiles.
	Object_iterator next(chunk);
	while ((obj = next.get_next()) != 0)
		if (obj->is_egg())
			update_egg(chunk, (Egg_object *) obj, 1);
		else
			update_object(chunk, obj, 1);
	setup_done = 1;
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

inline int Chunk_cache::get_highest_blocked
	(
	int lift,			// Look below this lift.
	unsigned short tflags		// Flags for desired tile.
	)
	{
	int i;				// Look downwards.
	for (i = lift - 1; i >= 0 && !(tflags & (1<<i)); i--)
		;
	return i;
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

int Chunk_cache::get_highest_blocked
	(
	int lift,			// Look below this lift.
	int tx, int ty			// Square to test.
	)
	{
	return get_highest_blocked(lift, blocked[ty*tiles_per_chunk + tx]);
	}

/*
 *	Get highest blocked lift below a given level for a given tile.
 *
 *	Output:	Highest lift that's blocked by an object, or -1 if none.
 */

inline int Chunk_cache::get_lowest_blocked
	(
	int lift,			// Look above this lift.
	unsigned short tflags		// Flags for desired tile.
	)
	{
	int i;				// Look upward.
	for (i = lift; i < 16 && !(tflags & (1<<i)); i++)
		;
	if (i == 16) return -1;
	return i;
	}

/*
 *	Get lowest blocked lift above a given level for a given tile.
 *
 *	Output:	Lowest lift that's blocked by an object, or -1 if none.
 */

int Chunk_cache::get_lowest_blocked
	(
	int lift,			// Look below this lift.
	int tx, int ty			// Square to test.
	)
	{
	return get_lowest_blocked(lift, blocked[ty*tiles_per_chunk + tx]);
	}

/*
 *	Is a given square occupied at a given lift?
 *
 *	Output: 1 if so, else 0.
 *		If 0 (tile is free), new_lift contains the new height that
 *		   an actor will be at if he walks onto the tile.
 */

int Chunk_cache::is_blocked
	(
	int height,			// Height (in tiles) of obj. being
					//   tested.
	int lift,			// Given lift.
	int tx, int ty,			// Square to test.
	int& new_lift,			// New lift returned.
	int max_drop			// Max. drop allowed.
	)
	{
					// Get bits.
	unsigned short tflags = blocked[ty*tiles_per_chunk + tx];

	int new_high;
					// Something there?
	if (tflags & (1 << lift))		
		{
		new_lift = lift + 1;	// Maybe we can step up.
		new_high = get_lowest_blocked (new_lift, tflags);
		if (new_lift > 15)
			return (1);	// In sky
		else if (tflags & (1 << new_lift))
			return (1);	// Next step up also blocked
		else if (new_high != -1 && new_high < (new_lift + height))
			return (1);	// Blocked by something above
		else
			return (0);
		}
					// See if we're going down.
	new_lift = get_highest_blocked(lift, tflags) + 1;
	new_high = get_lowest_blocked (new_lift, tflags);
	
	if (new_high != -1 && new_high < (new_lift + height)) return 1;
	
					// Don't allow fall of > 1.
	return (lift - new_lift > max_drop ? 1 : 0);
	}

/*
 *	Activate nearby eggs.
 */

void Chunk_cache::activate_eggs
	(
	Game_object *obj,		// Object (actor) that's near.
	Chunk_object_list *chunk,	// Chunk this is attached to.
	int tx, int ty,			// Tile (absolute).
	int from_tx, int from_ty,	// Tile walked from.
	unsigned short eggbits		// Eggs[tile].
	)
	{
					// Get ->usecode machine.
	Usecode_machine *usecode = 
				Game_window::get_game_window()->get_usecode();
	int i;				// Go through eggs.
	for (i = 0; i < 8*(int)sizeof(eggbits) - 1 && eggbits; 
						i++, eggbits = eggbits >> 1)
		{
		Egg_object *egg;
		if ((eggbits&1) && (egg = (Egg_object *) egg_objects.get(i)) &&
		    egg->is_active(obj, tx, ty, from_tx, from_ty))
			egg->activate(usecode, obj);
		}
	if (eggbits)			// Check 15th bit.
		{
		int num_eggs = egg_objects.get_cnt();
		for ( ; i < num_eggs; i++)
			{
			Egg_object *egg = (Egg_object *) egg_objects.get(i);
			if (egg && egg->is_active(obj,
						tx, ty, from_tx, from_ty))
				egg->activate(usecode, obj);
			}
		}
	}

/*
 *	Create list for a given chunk.
 */

Chunk_object_list::Chunk_object_list
	(
	int chunkx, int chunky		// Absolute chunk coords.
	) : objects(0), npcs(0), cache(0), roof(0), light_sources(0),
	    cx(chunkx), cy(chunky)
	{
	}

/*
 *	Delete all objects contained within.
 */

Chunk_object_list::~Chunk_object_list
	(
	)
	{
	if (objects)
		{
		Game_object *first = objects;
		Game_object *obj;
		do
			{
			obj = objects;
			objects = obj->get_next();
			delete obj;
			}
		while (obj != first);
		}
	delete cache;
	}

/*
 *	Add a game object to a chunk's list.
 *
 *	Newobj's cx and cy fields are set to this chunk.
 */

void Chunk_object_list::add
	(
	Game_object *newobj		// Object to add.
	)
	{
	newobj->cx = get_cx();		// Set object's chunk.
	newobj->cy = get_cy();
					// Just put in front.
	objects = newobj->insert_in_chain(objects);
	Game_object *obj;		// Figure dependencies.
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		int cmp = newobj->lt(*obj);
		if (!cmp)		// Bigger than this object?
			newobj->dependencies.put(obj);
		else if (cmp == 1)	// Smaller than?
			obj->dependencies.put(newobj);
		}
	if (cache)			// Add to cache.
		cache->update_object(this, newobj, 1);
	Shape_info& info = Game_window::get_game_window()->get_info(newobj);
	if (info.is_light_source())	// Count light sources.
		light_sources++;
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		{
#if 1 /* Not sure yet. */
		if (info.get_shape_class() == Shape_info::building)
#endif
			roof = 1;
		}
	}

/*
 *	Add an egg.
 */

void Chunk_object_list::add_egg
	(
	Egg_object *egg
	)
	{
	add(egg);			// Add it normally.
	egg->set_area();
	if (cache)			// Add to cache.
		cache->update_egg(this, egg, 1);
	}

/*
 *	Remove an egg.
 */

void Chunk_object_list::remove_egg
	(
	Egg_object *egg
	)
	{
	remove(egg);			// Remove it normally.
	if (cache)			// Remove from cache.
		cache->update_egg(this, egg, 0);
	}

/*
 *	Remove a game object from this list.  The object's cx and cy fields
 *	are left set to this chunk.
 */

void Chunk_object_list::remove
	(
	Game_object *remove
	)
	{
	if (cache)			// Remove from cache.
		cache->update_object(this, remove, 0);
	remove->clear_dependencies();	// Remove all dependencies.
	Shape_info& info = Game_window::get_game_window()->get_info(remove);
	if (info.is_light_source())	// Count light sources.
		light_sources--;
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		obj->remove_dependency(remove);

	if (remove == objects)		// First one?
		{
		objects = remove->get_next();
		if (objects == remove)	// Very last?
			objects = 0;
		}
	remove->remove_from_chain();
	}

/*
 *	Is a given rectangle of tiles blocked at a given lift?
 *
 *	Output: 1 if so, else 0.
 *		If 0 (tile is free), new_lift contains the new height that
 *		   an actor will be at if he walks onto the tile.
 */

int Chunk_object_list::is_blocked
	(
	int height,			// Height (along lift) to check.
	int lift,			// Starting lift.
	int startx, int starty,		// Starting tile coords.
	int xtiles, int ytiles,		// Width, height in tiles.
	int& new_lift,			// New lift returned.
	int max_drop			// Max. drop allowed.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int tx, ty;
	new_lift = 0;
	int stopy = starty + ytiles, stopx = startx + xtiles;
	for (ty = starty; ty < stopy; ty++)
		{			// Get y chunk, tile-in-chunk.
		int cy = ty/tiles_per_chunk, rty = ty%tiles_per_chunk;
		for (tx = startx; tx < stopx; tx++)
			{
			int this_lift;
			Chunk_object_list *olist = gwin->get_objects(
					tx/tiles_per_chunk, cy);
			olist->setup_cache();
			if (olist->is_blocked(height, lift, tx%tiles_per_chunk,
						rty, this_lift, max_drop))
				return (1);
					// Take highest one.
			new_lift = this_lift > new_lift ?
					this_lift : new_lift;
			}
		}
	return (0);
	}

/*
 *	Check an absolute tile position.
 *
 *	Output:	1 if blocked, 0 otherwise.
 *		Tile.tz may be updated for stepping onto square.
 */

int Chunk_object_list::is_blocked
	(
	Tile_coord& tile,
	int height,			// Height in tiles to check.
	int max_drop
	)
	{
					// Get chunk tile is in.
	Game_window *gwin = Game_window::get_game_window();
	Chunk_object_list *chunk = gwin->get_objects(
			tile.tx/tiles_per_chunk, tile.ty/tiles_per_chunk);
	chunk->setup_cache();		// Be sure cache is present.
	int new_lift;			// Check it within chunk.
	if (chunk->is_blocked(height, tile.tz, tile.tx%tiles_per_chunk,
				tile.ty%tiles_per_chunk, new_lift, max_drop))
		return (1);
	tile.tz = new_lift;
	return (0);
	}

/*
 *	See if a tile is water or land.
 */

inline void Check_terrain
	(
	Game_window *gwin,
	Chunk_object_list *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& terrain			// Sets: bit0 if land, bit1 if water.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (!flat.is_invalid())
		{
		if (gwin->get_info(flat.get_shapenum()).is_water())
			terrain |= 2;
		else
			terrain |= 1;
		}
	}

/*
 *	This one is used to see if an object of dims. possibly > 1X1 can
 *	step onto an adjacent square.  For now, changes in lift aren't
 *	allowed.
 */

int Chunk_object_list::is_blocked
	(
					// Object dims:
	int xtiles, int ytiles, int ztiles,
	Tile_coord from,		// Stepping from here.
	Tile_coord to,			// Stepping to here.
	int& terrain			// Returns: 1==land, 2==sea,
					// 	    3==both.
	)
	{
	terrain = 0;
	Game_window *gwin = Game_window::get_game_window();
	int vertx0, vertx1;		// Get x-coords. of vert. block
					//   to right/left.
	int horizx0, horizx1;		// Get x-coords of horiz. block
					//   above/below.
	int verty0, verty1;		// Get y-coords of horiz. block
					//   above/below.
	int horizy0, horizy1;		// Get y-coords of vert. block
					//   to right/left.
	horizx0 = to.tx + 1 - xtiles;
	horizx1 = to.tx;
	if (to.tx >= from.tx)		// Moving right?
		{
		vertx0 = from.tx + 1;	// Start to right of hot spot.
		vertx1 = to.tx;		// End at dest.
		}
	else				// Moving left?
		{
		vertx0 = to.tx + 1 - xtiles;
		vertx1 = from.tx - xtiles;
		}
	verty0 = to.ty + 1 - ytiles;
	verty1 = to.ty;
	if (to.ty >= from.ty)		// Moving down?
		{
		horizy0 = from.ty + 1;	// Start below hot spot.
		horizy1 = to.ty;	// End at dest.
		if (to.ty != from.ty)
			verty1--;	// Includes bottom of vert. area.
		}
	else				// Moving up?
		{
		horizy0 = to.ty + 1 - ytiles;
		horizy1 = from.ty - ytiles;
		verty0++;		// Includes top of vert. area.
		}
	int x, y;			// Go through horiz. part.
	for (y = horizy0; y <= horizy1; y++)
		{			// Get y chunk, tile-in-chunk.
		int cy = y/tiles_per_chunk, rty = y%tiles_per_chunk;
		for (x = horizx0; x <= horizx1; x++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					x/tiles_per_chunk, cy);
			olist->setup_cache();
			int rtx = x%tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty, 
								new_lift) ||
			    new_lift != from.tz)
				return (1);
			if (new_lift == 0)
				Check_terrain(gwin, olist, rtx, rty, terrain);
			else
				terrain |= 1;
			}
		}
					// Do vert. block.
	for (x = vertx0; x <= vertx1; x++)
		{			// Get x chunk, tile-in-chunk.
		int cx = x/tiles_per_chunk, rtx = x%tiles_per_chunk;
		for (y = verty0; y <= verty1; y++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					cx, y/tiles_per_chunk);
			olist->setup_cache();
			int rty = y%tiles_per_chunk;
			if (olist->is_blocked(ztiles, from.tz, rtx, rty, 
								new_lift) ||
			    new_lift != from.tz)
				return (1);
			if (new_lift == 0)
				Check_terrain(gwin, olist, rtx, rty, terrain);
			else
				terrain |= 1;
			}
		}
	return (0);			// All clear.
	}

/*
 *  Finds if there is a 'roof' above lift in tile (tx, ty)
 *  of the chunk. Point is taken 4 above lift
 *
 *  Roof can be any object, not just a literal roof
 *
 *  Output: height of the roof.
 *  A return of 31 means no roof
 *
 */
int Chunk_object_list::is_roof(int tx, int ty, int lift)
{
#if 1		/* Might be lying on bed at lift==2. */
	int height = get_lowest_blocked (lift+4, tx, ty);
#else		/* But this is even worse! */
	int height = get_lowest_blocked (lift+2, tx, ty);
#endif
	if (height == -1) return 31;
	return height;
}

/*
 *	Create a sequence of frames.
 */

Frames_sequence::Frames_sequence
	(
	int cnt,			// # of frames.
	unsigned char *f		// List of frames.
	) : num_frames(cnt)
	{
	frames = new unsigned char[cnt];
	memcpy(frames, f, cnt);		// Copy in the list.
	}

#if 0
/*
 *	Create a moveable sprite.
 */

Sprite::Sprite
	(
	int shapenum
	)  : Container_game_object(),
		major_dir(0), major_frame_incr(8), frames_seq(0)
	{
	set_shape(shapenum, 0); 
	for (int i = 0; i < 8; i++)
		frames[i] = 0;
	}

/*
 *	Stop moving.
 */

void Sprite::stop
	(
	)
	{
	major_dir = 0;
	if (frames_seq)			// Set to "resting" frame.
		set_frame(frames_seq->get_resting());
	}

/*
 *	Start moving.
 */

void Sprite::start
	(
	unsigned long destx,		// Move towards pt. within world.
	unsigned long desty,
	int speed,			// # millisecs. between frames.
	int delay			// Delay before starting.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	frame_time = speed;
	Direction dir;			// Gets compass direction.++++++Get
					//  northeast, etc. too.
	if (!is_walking())		// Not already moving?
		{			// Start.
		unsigned long curtime = SDL_GetTicks();
		gwin->get_tqueue()->add(curtime + delay, this, (long) gwin);
		}
	curx = get_worldx();		// Get current coords.
	cury = get_worldy();
	sum = 0;			// Clear accumulator.
					// Get change at current lift.
	int liftpixels = 4*get_lift();
	long deltax = destx + liftpixels - curx;
	long deltay = desty + liftpixels - cury;
	if (!deltax && !deltay)		// Going nowhere?
		{
		stop();
		return;
		}		
	unsigned long abs_deltax, abs_deltay;
	int x_dir, y_dir;
	if (deltay >= 0)		// Figure directions.
		{
		y_dir = 1;
		abs_deltay = deltay;
		}
	else
		{
		y_dir = -1;
		abs_deltay = -deltay;
		}
	if (deltax >= 0)
		{
		x_dir = 1;
		abs_deltax = deltax;
		}
	else
		{
		x_dir = -1;
		abs_deltax = -deltax;
		}
	if (abs_deltay >= abs_deltax)	// Moving faster along y?
		{
		dir = y_dir > 0 ? south : north;
		major_coord = &cury;
		minor_coord = &curx;
		major_dir = y_dir;
		minor_dir = x_dir;
		major_delta = abs_deltay;
		minor_delta = abs_deltax;
		}
	else				// Moving faster along x?
		{
		dir = x_dir > 0 ? east : west;
		major_coord = &curx;
		minor_coord = &cury;
		major_dir = x_dir;
		minor_dir = y_dir;
		major_delta = abs_deltax;
		minor_delta = abs_deltay;
		}
	major_distance = major_delta;	// How far to go.
					// Different dir. than before?
	if (frames[(int) dir] != frames_seq)
		{			// Set frames sequence.
		frames_seq = frames[(int) dir];
		frame_index = -1;
		}
	}

/*
 *	Can this be dragged?
 */

int Sprite::is_dragable
	(
	) const
	{
	return (0);			// No.
	}

/*
 *	Figure where the sprite will be in the next frame.
 *
 *	Output: 0 if don't need to move.
 */

int Sprite::next_frame
	(
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_tx, int& new_ty,	// New tile coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
	if (!is_walking())
		return (0);
					// Figure change in faster axis.
	int new_major = major_frame_incr;
					// Subtract from distance to go.
	major_distance -= major_frame_incr;
					// Accumulate change.
	sum += major_frame_incr * minor_delta;
					// Figure change in slower axis.
	int new_minor = sum/major_delta;
	sum = sum % major_delta;	// Remove what we used.
					// Update coords. within world.
	*major_coord += major_dir*new_major;
	*minor_coord += minor_dir*new_minor;
	new_cx = curx/chunksize;	// Return new chunk pos.
	new_cy = cury/chunksize;
	new_tx = (curx%chunksize)/tilesize;
	new_ty = (cury%chunksize)/tilesize;
	if (frames_seq)			// Got a sequence of frames?
		next_frame = frames_seq->get_next(frame_index);
	else
		next_frame = -1;
	return (1);
	}
#endif



