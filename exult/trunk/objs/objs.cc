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
#include "chunks.h"
#include "objiter.h"
#include "egg.h"
#include "gamewin.h"
#include "actors.h"
#include "ucmachine.h"
#include "items.h"
#include "dir.h"
// #include "game.h"
#include "ordinfo.h"
#include <cstring>

using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::memset;
using std::rand;
using std::ostream;
using std::strchr;
using std::string;

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
	Game_object_vector::const_iterator	X;
	
	// First do those we depend on.
	for(X = dependencies.begin(); X != dependencies.end(); ++X )
		(**X).dependors.remove(this);
	dependencies.clear();
	
	// Now those who depend on us.
	for(X = dependors.begin(); X != dependors.end(); ++X )
		(**X).dependencies.remove(this);
	dependors.clear();
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

template <class T>
int Game_object::find_nearby
	(
	Exult_vector<T*>& vec,	// Objects appended to this.
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
	int vecsize = vec.size();
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
					vec.push_back(dynamic_cast<T*>(obj));
				}
			}
					// Return # added.
	return (vec.size() - vecsize);
	}

int Game_object::find_nearby_actors
	(
	Actor_vector& vec,
	int shapenum,
	int delta
	) const
	{
	return Game_object::find_nearby(vec, get_abs_tile_coord(), shapenum,
						delta, 8, -359, -359);
	}

int Game_object::find_nearby_eggs
	(
	Egg_vector& vec,
	int shapenum,
	int delta
	) const
	{
	return Game_object::find_nearby(vec, get_abs_tile_coord(), shapenum,
						delta, 16, -359, -359);
	}

int Game_object::find_nearby
	(
	Game_object_vector& vec,
	int shapenum,
	int delta,
	int mask,
	int qual,
	int framenum
	) const
	{
	return Game_object::find_nearby(vec, get_abs_tile_coord(), shapenum,
					delta, mask, qual, framenum);
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
	Game_object_vector vec;			// Gets objects found.
	int i;
	for (i = 0; i < num_shapes; i++)
		find_nearby(vec, shapenums[i], 24, 0);
	int cnt = vec.size();
	if (!cnt)
		return (0);
	Game_object *closest = 0;	// Get closest.
	int best_dist = 10000;		// In tiles.
					// Get our location.
	Tile_coord loc = get_abs_tile_coord();
	for (Game_object_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
		Game_object *obj = *it;
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

int Game_object::attack_object
	(
	Game_window *gwin,
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
		gwin->get_usecode()->call_usecode(usefun, this,
					Usecode_machine::weapon);
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
	int wpoints = attack_object(gwin,
					attacker, weapon_shape, ammo_shape);
	if (wpoints < 8)
		return this;		// Fail.
	if (wpoints < 16)
		wpoints = wpoints/2;	// Unlikely.
	int shnum = get_shapenum();	// Only do doors for now.
	if (shnum != 433 && shnum != 432 && shnum != 270 && shnum != 376)
		return this;
					// Guessing:
	if (rand()%90 < wpoints)
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



