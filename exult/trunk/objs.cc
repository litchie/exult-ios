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
#include "game.h"
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
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(shnum);
	return info.get_shape_class() == Shape_info::has_quantity;
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
	if (Has_quantity(shnum))
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
 *	Swap positions with another object (of the same footprint).
 *
 *	Output: 1 if successful, else 0.
 */

int Game_object::swap_positions
	(
	Game_object *obj2
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& inf1 = gwin->get_info(this);
	Shape_info& inf2 = gwin->get_info(obj2);
	if (inf1.get_3d_xtiles() != inf2.get_3d_xtiles() ||
	    inf1.get_3d_ytiles() != inf2.get_3d_ytiles())
		return 0;		// Not the same size.
	Tile_coord p1 = get_abs_tile_coord();
	Tile_coord p2 = obj2->get_abs_tile_coord();
	remove_this(1);			// Remove (but don't delete) each.
	set_invalid();
	obj2->remove_this(1);
	obj2->set_invalid();
	move(p2.tx, p2.ty, p2.tz);	// Move to new locations.
	obj2->move(p1.tx, p1.ty, p1.tz);
	return (1);
	}

/*
 *	Remove all dependencies.
 */

void Game_object::clear_dependencies
	(
	)
	{
	int cnt = dependencies.get_cnt();// First do those we depend on.
	int i;
	for (i = 0; i < cnt; i++)
		{
		Game_object *dep = (Game_object *) dependencies.get(i);
		if (dep)
			{
			dependencies.put(i, 0);
			dep->dependors.remove(this);
			}
		}
	cnt = dependors.get_cnt();	// Now those who depend on this.
	for (i = 0; i < cnt; i++)
		{
		Game_object *dep = (Game_object *) dependors.get(i);
		if (dep)
			{
			dependors.put(i, 0);
			dep->dependencies.remove(this);
			}
		}
	}

/*
 *	Check an object in find_nearby() against the mask.
 *	+++++These don't seem to be acting like masks!!
 *
 *	Output:	1 if it passes.
 */
static int Check_mask
	(
	Game_window *gwin,
	Game_object *obj,
	int mask
	)
	{
	if (mask == 4)			// Party members.
		return (obj->get_party_id() >= 0 || 
					obj == gwin->get_main_actor());
	if (mask == 8)			// All NPCs.
		{
		if (obj->is_monster())
			return 1;
		if (obj->get_npc_num() <= 0 && obj != gwin->get_main_actor())
			return 0;	// Not an NPC & not the Avatar.
		return 1;
		}
	if (mask == 16)			// Egg or barge.
		return obj->is_egg() || obj->get_shapenum() == 0x3c1;
	if (mask == 32)
		return obj->is_monster();
	if (!mask)			// Guessing a bit here.
		return !obj->is_egg();	// Don't pass eggs if 0.
	return 1;
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
	int shapenum,			// Shape to look for.  
					//   -1=any (but always use mask?),
					//   -359=any.
	int delta,			// # tiles to look in each direction.
	int mask,			// Guessing+++:
					//   4 == party members only???
					//   8 == all NPC's.
					//  16 == egg or barge.
					//  32 == monsters? 
	int qual,			// Quality, or -359 for any.
	int framenum			// Frame #, or -359 for any.
	)
	{
	if (delta < 0)			// +++++Until we check all old callers.
		delta = 24;
	if (shapenum > 0 && mask == 4)	// Ignore mask=4 if shape given!
		mask = 0;
	int vecsize = vec.get_cnt();
	Game_window *gwin = Game_window::get_game_window();
	Rectangle tiles(pos.tx - delta, pos.ty - delta, 1 + 2*delta, 1 + 
								2*delta);
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
#if 0
				else if (shapenum == -359 &&
							!obj->get_npc_num() &&
						 obj != gwin->get_main_actor())
					continue;
#endif
				if (qual != -359 && obj->get_quality() != qual)
					continue;
				if ((mask || shapenum == -1) && 
						!Check_mask(gwin, obj, mask))
					continue;
				if (framenum !=  -359 &&
					obj->get_framenum() != framenum)
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
		cnt += find_nearby(vec, shapenums[i], 24, 0);
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
	Tile_coord pos,			// Position to look from.
	int dist,			// 1 means adjacent.
	int height,			// Height to check for unblocked.
	const int move_flags		// What sort of motion is allowed.
	)
	{
					// Get box to go through.
	Rectangle box(pos.tx - dist, pos.ty - dist, 2*dist + 1, 2*dist + 1);
	Rectangle world(0, 0, num_tiles, num_tiles);
	box = box.intersect(world);
	int stopx = box.x + box.w, stopy = box.y + box.h;
	for (int y = box.y; y < stopy; y++)
		for (int x = box.x; x < stopx; x++)
			{		// Check this spot.
			Tile_coord spot(x, y, pos.tz);
			if (!Chunk_object_list::is_blocked(spot, height, 
								move_flags))
				return spot;
			}
	return Tile_coord(-1, -1, -1);
	}

/*
 *	Get footprint in absolute tiles.
 */

Rectangle Game_object::get_footprint
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(this);
					// Get footprint.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	Tile_coord t = get_abs_tile_coord();
	Rectangle foot(t.tx - xtiles + 1, t.ty - ytiles + 1, xtiles, ytiles);
	return foot;
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
#if 0	/* +++++Old way */
	int tx, ty, tz;
	get_abs_tile(tx, ty, tz);
	int liftpix = 4*tz;
	gwin->paint_shape(
		(tx + 1 - gwin->get_scrolltx())*tilesize - 1 - liftpix,
		(ty + 1 - gwin->get_scrollty())*tilesize - 1 - liftpix,
					get_shapenum(), get_framenum());
#else
	int x, y;
	gwin->get_shape_location(this, x, y);
	gwin->paint_shape(x, y, get_shapenum(), get_framenum());
#endif
	}

/*
 *	Run usecode when double-clicked.
 */

void Game_object::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	int usefun = get_shapenum();
					// !!!Special case:  books in BG.
	if (usefun == 0x282 && get_quality() >= 100)
		usefun = 0x638;
	umachine->call_usecode(usefun, this,
			(Usecode_machine::Usecode_events) event);
#if 0	/* ++++I don't think this does any good. */
	if (Game::get_game_type() != BLACK_GATE)
		return;
	Vector barges;			// Look for nearby barge.
	if (!find_nearby(barges, 961, 16, 0))
		return;
					// Special usecode for barge pieces:
	umachine->call_usecode(0x634, this, 
				(Usecode_machine::Usecode_events) event);
#endif
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
	int frnum = get_framenum();
	switch (shnum)			// Some special cases!
		{
	case 0x34a:			// Reagants.
		name = item_names[0x500 + frnum];
		break;
	case 0x3bb:			// Medallions?
		if (frnum >= 3)
			name = item_names[shnum];
		else
			name = item_names[0x508 + frnum];
		break;
	case 0x179:			// Food items.
		name = item_names[0x50b + frnum];
		break;
	case 0x2a3:			// Desk item.
		name = item_names[0x52d + frnum];
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
 *	Get weight in 1/10 stones.  0 means infinite.
 */

int Game_object::get_weight
	(
	)
	{
	int quant = get_quantity();
	int wt = quant *
		Game_window::get_game_window()->get_info(this).get_weight();
	int shnum = get_shapenum();	// Special case:  reagants, coins.
	if (shnum == 842 || shnum == 644)
		wt /= 10;
	return wt;
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
	    !Has_quantity(shapenum) ||
					// Reagants are a special case.
	    (shapenum == 842 && get_framenum() != obj->get_framenum()))
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
 *	Information about an object used during render-order comparison (lt()):
 */
class Ordering_info
	{
public:
	Rectangle area;			// Area (pixels) rel. to screen.
	Shape_info& info;		// Info. about shape.
	int tx, ty, tz;			// Absolute tile coords.
	int xs, ys, zs;			// Tile dimensions.
private:
	void init(const Game_object *obj)
		{
		obj->get_abs_tile(tx, ty, tz);
		int frnum = obj->get_framenum();
		xs = info.get_3d_xtiles(frnum);
		ys = info.get_3d_ytiles(frnum);
		zs = info.get_3d_height();
		}
public:
	friend class Game_object;
	friend class Chunk_object_list;
					// Create from scratch.
	Ordering_info(Game_window *gwin, const Game_object *obj)
		: area(gwin->get_shape_rect(obj)),
		  info(gwin->get_shapes().get_info(obj->get_shapenum()))
		{ init(obj); }
	Ordering_info(Game_window *gwin, const Game_object *obj, Rectangle& a)
		: area(a),
		  info(gwin->get_shapes().get_info(obj->get_shapenum()))
		{ init(obj); }
	};

/*
 *	Should obj1 be rendered before obj2?
 *
 *	Output:	1 if so, 0 if not, -1 if cannot compare.
 */
int Game_object::lt
	(
	Ordering_info& inf1,		// Info. for object 1.
	Game_object *obj2
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// See if there's no overlap.
	Rectangle r2 = gwin->get_shape_rect(obj2);
	if (!inf1.area.intersects(r2))
		return (-1);		// No overlap on screen.
	Ordering_info inf2(Game_window::get_game_window(), obj2, r2);
#ifdef DEBUGLT
	Debug_lt(inf1.tx, inf1.ty, inf2.tx, inf2.ty);
#endif
	int result = -1;		// Watch for conflicts.
	if (inf1.tz != inf2.tz)		// Is one obj. on top of another?
		{
		if (inf1.tz + inf1.zs <= inf2.tz)
			{		// It's above us.
			if (inf1.zs >= 4)	// Like roof/statue?
				return 1;
			result = 1;
			}
		else if (inf2.tz + inf2.zs <= inf1.tz)
			{		// We're above.
			if (inf2.zs >= 4)	// Like roof/statue?
				return 0;
			result = 0;
			}
		}
	if (inf1.ty != inf2.ty)		// Is one obj. in front of the other?
		{
		if (inf1.ty <= inf2.ty - inf2.ys)
			{		// Obj2 is in front.
			if (result == 0)// Conflict, so return 'neither'.
				return -1;
			result = 1;
			}
		else if (inf2.ty <= inf1.ty - inf1.ys)
			{		// We're in front.
			if (result == 1)
				return -1;
			result = 0;
			}
		}
	if (inf1.tx != inf2.tx)		// Is one obj. to right of the other?
		{
		if (inf1.tx <= inf2.tx - inf2.xs)
			{		// Obj2 is to right of us.
			if (result == 0)
				return -1;
			result = 1;
			}
		if (inf2.tx <= inf1.tx - inf1.xs)
			{		// We're to the right.
			if (result == 1)
				return -1;
			result = 0;
			}
		}
	if (result != -1)		// Consistent result?
		return result;
	if (!inf1.zs && inf1.tz <= inf2.tz)	// We're flat and below?
		return (1);
	if (!inf2.zs && inf2.tz <= inf1.tz)	// It's below us?
		return (0);
#if 0					// Below 2nd?
	if (inf1.tz < inf2.tz && inf1.tz + inf1.zs <= inf2.tz + inf2.zs)
		return (1);
					// Above 2nd?
	else if (inf2.tz < inf1.tz && inf2.tz + inf2.zs <= inf1.tz + inf1.zs)
		return (0);
#endif
					// Handle intersecting objects.
	if (inf1.tx == inf2.tx &&	// Watch for paintings on NS walls.
	    inf1.xs == inf2.xs)
		if (inf1.ys < inf2.ys)	// Take narrower 2nd.
			return (0);
		else if (inf2.ys > inf1.ys)
			return (1);
		else if (inf1.zs < inf2.zs)// The shorter one?
			return (0);
		else if (inf1.zs > inf2.zs)
			return (1);
#if 1	/* +++++Added 8/14/00: */
					// Item on table?
	if (inf1.zs > 1 && inf2.tz == inf1.tz + inf1.zs - 1 &&
	    inf2.tx - inf2.xs >= inf1.tx - inf1.xs && inf2.tx <= inf1.tx &&
	    inf2.ty - inf2.ys >= inf1.ty - inf1.ys && inf2.ty <= inf1.ty)
		return 1;
	if (inf2.zs > 1 && inf1.tz == inf2.tz + inf2.zs - 1 &&
	    inf1.tx - inf1.xs >= inf2.tx - inf2.xs && inf1.tx <= inf2.tx &&
	    inf1.ty - inf1.ys >= inf2.ty - inf2.ys && inf1.ty <= inf2.ty)
		return 0;
#endif
					// If x's overlap, see if in front.
	if ((inf1.tx > inf2.tx - inf2.xs && inf1.tx <= inf2.tx) ||
	    (inf2.tx > inf1.tx - inf1.xs && inf2.tx <= inf1.tx))
		{
		if (inf1.ty < inf2.ty)
			return (1);
		else if (inf1.ty > inf2.ty)
			return (0);
		else if (inf1.zs < inf2.zs)// The shorter one?
			return (0);
		else if (inf1.zs > inf2.zs)
			return (1);
		else if (inf1.xs < inf2.xs)// Take the narrower one as greater.
			return (0);
		else if (inf1.xs > inf2.xs)
			return (1);
		}
					// If y's overlap, see if to left.
	if ((inf1.ty > inf2.ty - inf2.ys && inf1.ty <= inf2.ty) ||
	    (inf2.ty > inf1.ty - inf1.ys && inf2.ty <= inf1.ty))
		{
		if (inf1.tx < inf2.tx)
			return (1);
		else if (inf1.tx > inf2.tx)
			return (0);
		}
	return (-1);
	}

/*
 *	Should this object be rendered before obj2?
 *	NOTE:  This older interface isn't as efficient.
 *
 *	Output:	1 if so, 0 if not, -1 if cannot compare.
 */
int Game_object::lt
	(
	Game_object& obj2
	) const
	{
	Game_window *gwin = Game_window::get_game_window();
	Ordering_info ord(gwin, this);
	return lt(ord, &obj2);
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
		{
		static char swaps180[4] = {2, 3, 0, 1};
		int subframe = curframe&3;
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
 *	Figure attack points against an object, and also run weapon's usecode.
 */

static int Attack_object
	(
	Game_window *gwin,
	Game_object *obj,		// What's being attacked.
	Actor *attacker,
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape
	)
	{
	int wpoints = 0;
	Weapon_info *winf;
	if (weapon_shape > 0)
		winf = gwin->get_info(weapon_shape).get_weapon_info();
	else if (ammo_shape > 0)	// Not sure about all this...
		winf = gwin->get_info(ammo_shape).get_weapon_info();
	else
		winf = attacker->get_weapon(wpoints);
	int usefun;			// Run usecode if present.
	if (winf && (usefun = winf->get_usecode()) != 0)
		gwin->get_usecode()->call_usecode(usefun, obj,
					Usecode_machine::weapon);
	int shnum = obj->get_shapenum();	// Only do doors for now.
	if (!wpoints && winf)
		wpoints = winf->get_damage();
	return wpoints;
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if destroyed, else object itself.
 */

Game_object *Game_object::attacked
	(
	Actor *attacker,
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int wpoints = Attack_object(gwin, this, 
					attacker, weapon_shape, ammo_shape);
	if (wpoints < 10)
		return this;		// Fail.
	if (wpoints < 20)
		wpoints = wpoints/2;	// Unlikely.
	int shnum = get_shapenum();	// Only do doors for now.
	if (shnum != 433 && shnum != 432 && shnum != 270 && shnum != 376)
		return this;
					// Let's guess points = percentage.
	if (rand()%100 < wpoints)
		{
		gwin->add_dirty(this);
		remove_this();
		return 0;
		}
	return this;
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
 *	Delete the chain.
 */

Object_list::~Object_list
	(
	)
	{
	if (!first)
		return;
	Game_object *objects = first;
	Game_object *obj;
	do
		{
		obj = objects;
		objects = obj->get_next();
		delete obj;
		}
	while (obj != first);
	}

/*
 *	Report iterator problem.
 */

void Object_list::report_problem
	(
	)
	{
	cerr << "Danger! Danger! Object list modified while being iterated." 
						<< endl;
	cerr.flush();
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
	int shapenum, int framenum,
	unsigned int shapex, unsigned int shapey, 
	unsigned int lft, 
	unsigned char *c,		// Circle spell flags.
	unsigned long f			// Flags (unknown).
	) : Ireg_game_object(shapenum, framenum,
			shapex, shapey, lft), flags(f), bookmark(-1)
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
	Usecode_machine *umachine,
	int event
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
	}

/*
 *	Remove an object.
 */

void Container_game_object::remove
	(
	Game_object *obj
	)
	{
	if (objects.is_empty())
		return;
	volume_used -= obj->get_volume();
	obj->set_owner(0);
	objects.remove(obj);
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
	objects.append(obj);		// Append to chain.
					// Guessing:
	if (get_flag(okay_to_take))
		obj->set_flag(okay_to_take);
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
	if (!objects.is_empty())
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
	if (objects.is_empty())
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
	if (objects.is_empty())
		return delta;		// Empty.
	Game_object *obj = objects.get_first();
	Game_object *next;
	int done = 0;
	while (!done && delta)
		{
		next = obj->get_next();	// Might be deleting obj.
		if (obj->get_shapenum() == shapenum &&
		    (qual == -359 || obj->get_quality() == qual) &&
		    (framenum == -359 || obj->get_framenum() == framenum))
			delta = -obj->modify_quantity(-delta);
					// Still there?
		if (next->get_prev() == obj)
					// Do it recursively.
			delta = obj->remove_quantity(delta, shapenum, 
							qual, framenum);
		obj = next;
		done = (!obj || obj == objects.get_first());
		}
	return (delta);
	}

/*
 *	Find and return a desired item.
 *
 *	Output:	->object if found, else 0.
 */

Game_object *Container_game_object::find_item
	(
	int shapenum,			// Shape #.
	int qual,			// Quality, or -359 for any. ???+++++
	int framenum			// Frame, or -359 for any.
	)
	{
	if (objects.is_empty())
		return 0;		// Empty.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if (obj->get_shapenum() == shapenum &&
		    (framenum == -359 || obj->get_framenum() == framenum))
					// ++++++Quality???
			return (obj);

					// Do it recursively.
		Game_object *found = obj->find_item(shapenum, qual, framenum);
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
	Usecode_machine *umachine,
	int event
	)
	{
	int shnum = get_shapenum();
	Game_window *gwin = Game_window::get_game_window();
	switch(shnum)			// Watch for gumps.
		{
	case 405:			// Ship's hold
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/shipshold"));
		return;
	case 406:			// Nightstand.
	case 407:			// Desk.
	case 283:
	case 203:
	case 416:			// Chest of drawers.
	case 679:
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/drawer"));
		return;
	case 400:			// Bodies.
	case 414:
	case 762:
	case 778:
	case 892:
	case 507: 			// Bones
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/body"));
		return;
	case 800:			// Chest.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/chest"));	// ???Guessing.
		return;
	case 801:			// Backpack.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/backpack"));
		return;
	case 799:			// Unsealed box
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/box"));
		return;
	case 802:			// Bag.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/bag"));
		return;
	case 803:			// Basket.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/basket"));
		return;
	case 804:			// Crate.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/crate"));
		return;
	case 819:			// Barrel.
		gwin->show_gump(this, Game::get_game()->get_shape("gumps/barrel"));
		return;
		}
					// Try to run normal usecode fun.
	umachine->call_usecode(shnum, this,
				(Usecode_machine::Usecode_events) event);
	}

/*
 *	Get (total) weight.
 */

int Container_game_object::get_weight
	(
	)
	{
	int wt = Game_object::get_weight();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		wt += obj->get_weight();
	return wt;
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
	int qual,			// Quality, or -359 for any.
	int framenum			// Frame#, or -359 for any.
	)
	{
	int vecsize = vec.get_cnt();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if ((shapenum == -359 || obj->get_shapenum() == shapenum) &&
		    (qual == -359 || obj->get_quality() == qual) &&
		    (framenum == -359 || obj->get_framenum() == framenum))
			vec.append(obj);
					// Search recursively.
		obj->get_objects(vec, shapenum, qual, framenum);
		}
	return (vec.get_cnt() - vecsize);
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if destroyed, else object itself.
 */

Game_object *Container_game_object::attacked
	(
	Actor *attacker,
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int wpoints = Attack_object(gwin, this, 
					attacker, weapon_shape, ammo_shape);
	if (wpoints < 8)
		return this;		// Fail.
	if (wpoints < 16)
		wpoints = wpoints/2;	// Unlikely.
					// Let's guess points = percentage.
	if (rand()%100 >= wpoints)
		return this;		// Failed.
	Tile_coord pos = get_abs_tile_coord();
	gwin->add_dirty(this);
	remove_this(1);			// Remove, but don't delete yet.
	Game_object *obj;		// Remove objs. inside.
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		Shape_info& info = gwin->get_info(obj);
		Tile_coord p(-1, -1, -1);
		for (int i = 1; p.tx < 0 && i < 8; i++)
			p = Game_object::find_unblocked_tile(pos,
					i, info.get_3d_height());
		if (p.tx == -1)
			obj->remove_this();
		else
			obj->move(p.tx, p.ty, p.tz);
		}
	delete this;
	return 0;
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
	Game_object *first = objects.get_first(); // Guessing: +++++
	unsigned short tword = first ? first->get_prev()->get_shapenum() 
									: 0;
	Write2(ptr, tword);
	*ptr++ = 0;			// Unknown.
	*ptr++ = get_quality();
	int npc = get_live_npc_num();	// If body, get source.
	int quant = (npc >= 0 && npc <= 127) ? (npc + 0x80) : 0;
	*ptr++ = quant&0xff;		// "Quantity".
	*ptr++ = (get_lift()&15)<<4;
	*ptr++ = resistance;		// Resistance.
					// Flags:  B0=invis. B3=okay_to_take.
	*ptr++ = get_flag((Game_object::invisible) != 0) +
		 ((get_flag(Game_object::okay_to_take) != 0) << 3);
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
	if (!objects.is_empty())	// Now write out what's inside.
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
	if (!foot.w)
		return;			// Empty (probability = 0).
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
			
	obj_list = chunk;
	
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
 *	See if a tile is water or land.
 */

inline void Check_terrain
	(
	Game_window *gwin,
	Chunk_object_list *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& terrain			// Sets: bit0 if land, bit1 if water,
					//   bit2 if solid.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (!flat.is_invalid())
		{
		if (gwin->get_info(flat.get_shapenum()).is_water())
			terrain |= 2;
		else if (gwin->get_info(flat.get_shapenum()).is_solid())
			terrain |= 4;
		else
			terrain |= 1;
		}

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
	const int move_flags,
	int max_drop			// Max. drop allowed.
	)
{

	// Ethereal beings always return not blocked
	// and can only move horizontally
	if (move_flags & MOVE_ETHEREAL)
	{
		new_lift = lift;
		return 0;
	}
					// Get bits.
	unsigned short tflags = blocked[ty*tiles_per_chunk + tx];

	int new_high;
					// Something there? and not flying
	if (tflags & (1 << lift) && !(move_flags & MOVE_FLY))		
	{
		new_lift = lift + 1;	// Maybe we can step up.
		new_high = get_lowest_blocked (new_lift, tflags);
		if (new_lift > 15)
			return (1);	// In sky
		else if (tflags & (1 << new_lift))
			return (1);	// Next step up also blocked
		else if (new_high != -1 && new_high < (new_lift + height))
			return (1);	// Blocked by something above
	}
	else if (tflags & (1 << lift))
	{
		new_lift = lift;
		while (tflags & (1 << new_lift) && new_lift <= lift+max_drop)
		{
			new_lift++;		// Maybe we can step up.
			new_high = get_lowest_blocked (new_lift, tflags);
			if (new_lift > 15)
				return (1);	// In sky
			else if (new_high != -1 && new_high < 
							(new_lift + height))
				return (1);	// Blocked by something above
		}
	}
	else
	{
					// See if we're going down.
		new_lift = get_highest_blocked(lift, tflags) + 1;
		new_high = get_lowest_blocked (new_lift, tflags);
	
		// Make sure that where we want to go is tall enough for us
		if (new_high != -1 && new_high < (new_lift + height)) return 1;
	
					// Don't allow fall of > max_drop.
		if (lift - new_lift > max_drop) return 1;
	}
		
	
	// Found a new place to go, lets test if we can actually move there
	
	// Lift 0 tests
	if (new_lift == 0)
	{
		int ter = 0;
		Check_terrain (Game_window::get_game_window(), obj_list, 
								tx, ty, ter);
		if (ter & 2)	// Water
		{
			if (move_flags & (MOVE_FLY+MOVE_SWIM))
				return 0;
			else
				return 1;
		}
		else if (ter & 1)	// Land
		{
			if (move_flags & (MOVE_FLY|MOVE_WALK))
				return 0;
			else
				return 1;
		}
		else if (ter & 4)	// Blocked
			return (move_flags & MOVE_FLY) ? 0 : 1;
		else	// Other
			return 0;
	}
	else if (move_flags & (MOVE_FLY|MOVE_WALK))
		return 0;

	return 1;
}

/*
 *	Activate nearby eggs.
 */

void Chunk_cache::activate_eggs
	(
	Game_object *obj,		// Object (actor) that's near.
	Chunk_object_list *chunk,	// Chunk this is attached to.
	int tx, int ty, int tz,		// Tile (absolute).
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
		    egg->is_active(obj, tx, ty, tz, from_tx, from_ty))
			egg->activate(usecode, obj);
		}
	if (eggbits)			// Check 15th bit.
		{
		int num_eggs = egg_objects.get_cnt();
		for ( ; i < num_eggs; i++)
			{
			Egg_object *egg = (Egg_object *) egg_objects.get(i);
			if (egg && egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
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
	) : objects(0), first_nonflat(0), dungeon_bits(0),
	    npcs(0), cache(0), roof(0), light_sources(0),
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
	delete cache;
	delete dungeon_bits;
	}

/*
 *	Add rendering dependencies for a new object.
 */

void Chunk_object_list::add_dependencies
	(
	Game_object *newobj,		// Object to add.
	Ordering_info& newinfo		// Info. for new object's ordering.
	)
	{
	Game_object *obj;		// Figure dependencies.
	Nonflat_object_iterator next(this);
	while ((obj = next.get_next()) != 0)
		{
		//cout << "Here " << __LINE__ << " " << obj << endl;
		int cmp = Game_object::lt(newinfo, obj);
		if (!cmp)		// Bigger than this object?
			{
			if (newobj->cx == obj->cx && newobj->cy == obj->cy)
				{
				newobj->dependencies.put(obj);
				obj->dependors.put(newobj);
				}
			}
		else if (cmp == 1)	// Smaller than?
			{
			obj->dependencies.put(newobj);
			newobj->dependors.put(obj);
			}
		}
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
	Game_window *gwin = Game_window::get_game_window();
	Ordering_info ord(gwin, newobj);
					// Put past flats.
	if (first_nonflat)
		objects.insert_before(newobj, first_nonflat);
	else
		objects.append(newobj);
					// Not flat?
	if (newobj->get_lift() || ord.info.get_3d_height())
		{			// Deal with dependencies.
		if (ord.xs == 1 && ord.ys == 1)	// Simplest case?
			add_dependencies(newobj, ord);
		else
			{
			Rectangle footprint(ord.tx - ord.xs + 1, 
					ord.ty - ord.ys + 1, ord.xs, ord.ys);
					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(footprint);
			Rectangle tiles;// (Ignored).
			int eachcx, eachcy;
			while (next_chunk.get_next(tiles, eachcx, eachcy))
				if (eachcx <= cx && eachcy <= cy)
					gwin->get_objects(eachcx, eachcy)->
						add_dependencies(newobj, ord);
			}
		first_nonflat = newobj;	// Inserted before old first_nonflat.
		}
			// +++++Maybe should skip test, do update_object(...).
	if (cache)			// Add to cache.
		cache->update_object(this, newobj, 1);
	if (ord.info.is_light_source())	// Count light sources.
		light_sources++;
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		{
		if (ord.info.get_shape_class() == Shape_info::building)
			roof = 1;
		}
	}

#if 0
/*
 *	Add a flat, fixed object.
 */

void Chunk_object_list::add_flat
	(
	Game_object *newobj		// Should be 0 height, lift=0.
	)
	{
	newobj->cx = get_cx();		// Set object's chunk.
	newobj->cy = get_cy();
					// Just put in front.
	objects = newobj->insert_in_chain(objects);
	}
#endif
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
	if (remove == first_nonflat)	// First nonflat?
		{			// Update.
		first_nonflat = remove->get_next();
		if (first_nonflat == objects.get_first())
			first_nonflat = 0;
		}
	objects.remove(remove);		// Remove from list.
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
	const int move_flags,
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
						rty, this_lift, move_flags, max_drop))
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
	const int move_flags,
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
				tile.ty%tiles_per_chunk, new_lift, move_flags, max_drop))
		return (1);
	tile.tz = new_lift;
	return (0);
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
	const int move_flags
	)
	{
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
							new_lift, move_flags) ||
			    new_lift != from.tz)
				return (1);
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
						new_lift, move_flags) ||
			    new_lift != from.tz)
				return (1);
			}
		}
	return (0);			// All clear.
	}

/*
 *	Test all nearby eggs when you've teleported in.
 */

void Chunk_object_list::try_all_eggs
	(
	Game_object *obj,		// Object (actor) that's near.
	int tx, int ty, int tz,		// Tile (absolute).
	int from_tx, int from_ty	// Tile walked from.
	)
	{
	static int norecurse = 0;	// NO recursion here.
	if (norecurse)
		return;
	norecurse++;
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord pos = obj->get_abs_tile_coord();
	const int dist = 32;		// See if this works okay.
	Rectangle area(pos.tx - dist, pos.ty - dist, 2*dist, 2*dist);
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// (Ignored).
	int eachcx, eachcy;
	Vector eggs(0, 40);		// Get them here first, as activating
					//   an egg could affect chunk's list.
	while (next_chunk.get_next(tiles, eachcx, eachcy))
		{
		Chunk_object_list *chunk = gwin->get_objects_safely(
							eachcx, eachcy);
		if (!chunk)
			continue;
		chunk->setup_cache();	// I think we should do this.
		Object_iterator next(chunk);
		Game_object *each;
		while ((each = next.get_next()) != 0)
			if (each->is_egg())
				{
				Egg_object *egg = (Egg_object *) each;
					// Music eggs are causing problems.
				if (egg->get_type() != Egg_object::jukebox &&
			    	    egg->is_active(obj,
						tx, ty, tz, from_tx, from_ty))
					eggs.append(egg);
				}
		}
	int eggcnt = eggs.get_cnt();
	for (int i = 0; i < eggcnt; i++)
		{
		Egg_object *egg = (Egg_object *) eggs.get(i);
		egg->activate(gwin->get_usecode(), obj);
		}
	norecurse--;
	}

/*
 *	Add a rectangle of dungeon tiles.
 */

void Chunk_object_list::add_dungeon_bits
	(
	Rectangle& tiles
	)
	{
	if (!dungeon_bits)
		{			// First one found.
		dungeon_bits = new unsigned char[256/8];
		memset(dungeon_bits, 0, 256/8);
		}
	int endy = tiles.y + tiles.h, endx = tiles.x + tiles.w;
	for (int ty = tiles.y; ty < endy; ty++)
		for (int tx = tiles.x; tx < endx; tx++)
			{
			int tnum = ty*tiles_per_chunk + tx;
			dungeon_bits[tnum/8] |= (1 << (tnum%8));
			}
	}

/*
 *	Set up the dungeon flags (after IFIX objects read).
 */

void Chunk_object_list::setup_dungeon_bits
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Object_iterator next(this);
	Game_object *each;
	while ((each = next.get_next()) != 0)
		{
		int shnum = each->get_shapenum();
					// Test for mountain-tops.
		if (shnum == 983 || shnum == 969 || shnum == 183 ||
		    shnum == 182 || shnum == 180 || shnum == 324)
			{
			Rectangle area = each->get_footprint();
					// Try to fix Courage Test:
			if (shnum == 969 && each->get_framenum() == 12)
				area.enlarge(1);
					// Go through interesected chunks.
			Chunk_intersect_iterator next_chunk(area);
			Rectangle tiles;// Rel. tiles.
			int cx, cy;
			while (next_chunk.get_next(tiles, cx, cy))
				gwin->get_objects(cx, cy)->add_dungeon_bits(
								tiles);
			}
		}
	}

/*
 *	Recursively apply gravity over a given rectangle that is known to be
 *	unblocked below a given lift.
 */

void Chunk_object_list::gravity
	(
	Rectangle area,			// Unblocked tiles (in abs. coords).
	int lift			// Lift where tiles are free.
	)
	{
	Vector list(0, 20);		// Gets list of objs. that dropped.
	Game_window *gwin = Game_window::get_game_window();
					// Go through interesected chunks.
	Chunk_intersect_iterator next_chunk(area);
	Rectangle tiles;		// Rel. tiles.  Not used.
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		Object_iterator objs(chunk);
		Game_object *obj;
		while ((obj = objs.get_next()) != 0)
			{
			if (!obj->is_dragable())
				continue;
			Tile_coord t = obj->get_abs_tile_coord();
					// Get footprint.
			Rectangle foot = obj->get_footprint();
			int new_lift;
					// Above area?
			if (t.tz >= lift && foot.intersects(area) &&
					// Unblocked below itself?  Let drop.
			    !is_blocked(1, t.tz - 1, foot.x, foot.y,
					foot.w, foot.h, new_lift,
						MOVE_ALL_TERRAIN, 100))
				{	// Save it, and drop it.
				list.append(obj);
				obj->move(t.tx, t.ty, new_lift);
				}
			}
		}
	int cnt = list.get_cnt();	// Get # objs. that dropped.
	for (int i = 0; i < cnt; i++)	// Recurse on each one.
		{
		Game_object *obj = (Game_object *) list.get(i);
					// Get footprint.
		Rectangle foot = obj->get_footprint();
		gravity(foot, obj->get_lift() +
					gwin->get_info(obj).get_3d_height());
		}
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



