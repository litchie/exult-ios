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
#include "gamewin.h"
#include "usecode.h"
#include "Audio.h"
#include <string.h>
#include <Audio.h>

extern	Audio audio;

/*
 *	Move to a new absolute location.
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
	Chunk_object_list *newchunk = gwin->get_objects(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
					// Remove from old.
	gwin->get_objects(cx, cy)->remove(this);
	set_lift(newlift);		// Set new values.
	shape_pos = ((newtx%tiles_per_chunk) << 4) + newty%tiles_per_chunk;
	newchunk->add(this);		// Updates cx, cy.
	}

/*
 *	Find nearby objects.
 *
 *	Output:	# found, appended to vec.
 */

int Game_object::find_nearby
	(
	Vector& vec,			// Objects appended to this.
	int shapenum,			// Shape to look for.  -1=any,
					//   -359=any NPC.
	int quality,			// +++Not used/understood.
	int mask			// +++Same here.
	)
	{
	int vecsize = vec.get_cnt();
	Game_window *gwin = Game_window::get_game_window();
	int atx, aty, atz;		// Get abs. tile coords.
	get_abs_tile(atx, aty, atz);
	const int delta = 8;		// Let's try 8 tiles each dir.
	Rectangle tiles(atx - delta, aty - delta, 2*delta, 2*delta);
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
			for (Game_object *obj = chunk->get_first(); obj;
						obj = chunk->get_next(obj))
				{	// Check shape.
				if (shapenum >= 0)
					{
					if (obj->get_shapenum() != shapenum)
						continue;
					}
				else if (shapenum == -359 &&
							!obj->get_npc_num())
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
 *	Get name.
 */

char *Game_object::get_name
	(
	)
	{
	extern char *item_names[];
	return item_names[get_shapenum()];
	}

/*
 *	Can this be dragged?
 */

int Game_object::is_dragable
	(
	)
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
	Game_object *obj
	)
	{
	return (0);
	}

/*
 *	Can this be dragged?
 */

int Ireg_game_object::is_dragable
	(
	)
	{
	return (1);			// Yes.
	}

/*
 *	Is a given tile within this egg's influence?
 */

int Egg_object::within_distance
	(
	int abs_tx, int abs_ty		// Tile coords. within entire world.
	)
	{
	int egg_tx = ((int) cx)*tiles_per_chunk + get_tx();
	int egg_ty = ((int) cy)*tiles_per_chunk + get_ty();
	int deltax = abs_tx - egg_tx;
	if (deltax >= distance || -deltax >= distance)
		return (0);
	int deltay = abs_ty - egg_ty;
	return (deltay < distance && -deltay < distance);
	}

/*
 *	Add an object.
 */

void Container_game_object::add
	(
	Game_object *obj
	)
	{
	if (!last_object)		// First one.
		{
		last_object = obj;
		obj->set_next(obj);
		}
	else
		{
		obj->set_next(last_object->get_next());
		last_object->set_next(obj);
		last_object = obj;
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
	if (!last_object)
		return;
	Game_object *prev = last_object;
	do
		{
		if (prev->get_next() == obj)
			{		// Found it.
			if (prev == obj)
				{	// Last one.
				last_object = 0;
				return;
				}
			prev->set_next(obj->get_next());
			if (obj == last_object)
				last_object = prev;
			return;
			}
		prev = prev->get_next();
		}
	while (prev != last_object);
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
	case 416:			// Chest of drawers.
		gwin->show_gump(this, 27);
		return;
	case 800:			// Chest.
		gwin->show_gump(this, 22);	// ???Guessing.
		return;
	case 801:			// Backpack.
		gwin->show_gump(this, 10);
		return;
	case 798:			// Bag.
	case 799:
	case 802:
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
	add(obj);			// We'll take it.
	return (1);
	}

/*
 *	Recursively count all objects of a given shape.
 */

int Container_game_object::count_objects
	(
	int shapenum			// Shape#, or -359 for any.
	)
	{
	int total = 0;
	Game_object *obj = last_object;
	do
		{
		obj = obj->get_next();
		if (shapenum == -359 || obj->get_shapenum() == shapenum)
			total++;
					// Count recursively.
		total += obj->count_objects(shapenum);
		}
	while (obj != last_object);
	return (total);
	}

/*
 *	Recursively get all objects of a given shape.
 */

int Container_game_object::get_objects
	(
	Vector& vec,			// Objects returned here.
	int shapenum			// Shape#, or -359 for any.
	)
	{
	int vecsize = vec.get_cnt();
	Game_object *obj = last_object;
	do
		{
		obj = obj->get_next();
		if (shapenum == -359 || obj->get_shapenum() == shapenum)
			vec.append(obj);
					// Search recursively.
		obj->get_objects(vec, shapenum);
		}
	while (obj != last_object);
	return (vec.get_cnt() - vecsize);
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine
	)
	{
	int roll = 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
cout << "Egg type is " << (int) type << ", prob = " << (int) probability <<
		", distance = " << (int) distance << ", crit = " <<
		(int) criteria << ", once = " <<
	((flags & (1<<(int)once) != 0)) << ", areset = " <<
	((flags & (1<<(int)auto_reset) != 0)) << '\n';
//TESTING:
	static int cnt = 0;
//	audio.start_speech(cnt++);//++++++++++++
	flags |= (1 << (int) hatched);	// Flag it as done.
	switch(type)
		{
		case jukebox:
			audio.start_music((data1)&0xff);
			break;
		case voice:
			audio.start_speech((data1)&0xff);
			break;
		case usecode:
			// Data2 is the usecode function.
			umachine->call_usecode(data2, this,
					Usecode_machine::egg_proximity);
			break;
		default:
			cout << "Egg not actioned" << endl;
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
	Shapes_vga_file& shapes = gwin->get_shapes();
	int shnum = obj->get_shapenum();
	Shape_info& info = shapes.get_info(shnum);
	int ztiles = info.get_3d_height(); 
	if (!ztiles)
		return;			// Skip if not an obstacle.
					// Get chunk coords.
	int cx = chunk->get_cx(), cy = chunk->get_cy();
					// Get lower-right corner of obj.
	int endx = obj->get_tx();
	int endy = obj->get_ty();
					// Get footprint dimensions.
	int xtiles = info.get_3d_xtiles();
	int ytiles = info.get_3d_ytiles();
	int lift = obj->get_lift();
	if (xtiles == 1 && ytiles == 1)	// Simplest case?
		{
		if (add)
			set_blocked_tile(endx, endy, lift, ztiles);
		else
			clear_blocked_tile(endx, endy, lift, ztiles);
		return;
		}
	int startx = endx - xtiles + 1, starty = endy - ytiles + 1;
					// First this chunk.
	int this_startx = startx < 0 ? 0 : startx;
	int this_starty = starty < 0 ? 0 : starty;
	set_blocked(this_startx, this_starty, endx, endy, lift, ztiles, add);
	if (startx < 0 && cx > 0)	// Overlaps chunk to the left?
		{
		gwin->get_objects(cx - 1, cy)->set_blocked(
				startx + tiles_per_chunk,
				this_starty, 15, endy, lift, ztiles, add);
					// Chunk to left and above?
		if (starty < 0 && cy > 0)
			gwin->get_objects(cx - 1, cy - 1)->set_blocked(
					startx + tiles_per_chunk,
					starty + tiles_per_chunk, 
					15, 15, lift, ztiles, add);
		}
	if (starty < 0 && cy > 0)	// Chunk directly above?
		gwin->get_objects(cx, cy - 1)->set_blocked(this_startx,
			starty + tiles_per_chunk, endx, 15, lift, ztiles, add);
	}

/*
 *	Set a rectangle of tiles within this chunk to be under the influence
 *	of a given egg.
 */

void Chunk_cache::set_egged
	(
	Egg_object *egg,
	Rectangle& tiles		// Range of tiles within chunk.
	)
	{
					// Egg already there?
	int eggnum = egg_objects.find(egg);
	if (eggnum < 0)			// No.  Is there a free spot?
		if ((eggnum = egg_objects.find(0)) >= 0)
			egg_objects.put(eggnum, egg);
		else			// No free spot.
			eggnum = egg_objects.append(egg);
	if (eggnum > 15)		// We only have 16 bits.
		eggnum = 15;
	short mask = (1<<eggnum);
	int stopx = tiles.x + tiles.w, stopy = tiles.y + tiles.h;
	for (int ty = tiles.y; ty < stopy; ty++)
		for (int tx = tiles.x; tx < stopx; tx++)
			eggs[ty*tiles_per_chunk + tx] |= mask;
	}

/*
 *	Add an egg to the cache.
 */

void Chunk_cache::add_egg
	(
	Chunk_object_list *chunk,
	Egg_object *egg
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get absolute tile coords.
	int tx = chunk->get_cx()*tiles_per_chunk + egg->get_tx(), 
	    ty = chunk->get_cy()*tiles_per_chunk + egg->get_ty();
	int dist = egg->get_distance();
					// Set up rect. with abs. tile range.
	Rectangle tiles(tx - dist, ty - dist, 2*dist + 1, 2*dist + 1);
					// Don't go outside the world.
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
			{
			Chunk_object_list *chunk = gwin->get_objects(cx, cy);
					// Figure intersection with egg's.
			Rectangle crect(cx*tiles_per_chunk, cy*tiles_per_chunk,
					tiles_per_chunk, tiles_per_chunk);
			crect = crect.intersect(tiles);
			if (crect.w > 0 && crect.h > 0)
				{
				crect.shift(-cx*tiles_per_chunk, 
							-cy*tiles_per_chunk);
				chunk->set_egged(egg, crect);
				}
			}
	}

/*
 *	Create the cached data for a chunk.
 */

void Chunk_cache::setup
	(
	Chunk_object_list *chunk
	)
	{
					// Set 'blocked' tiles.
	for (Game_object *obj = chunk->get_first(); obj; 
						obj = chunk->get_next(obj))
		if (obj->is_egg())
			add_egg(chunk, (Egg_object *) obj);
		else
			update_object(chunk, obj, 1);
	setup_done = 1;
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
	int lift,			// Given lift.
	int tx, int ty,			// Square to test.
	int& new_lift			// New lift returned.
	)
	{
					// Get bits.
	unsigned short tflags = blocked[ty*tiles_per_chunk + tx];
	if (tflags & (1<<lift))		// Something there?
		{
		new_lift = lift + 1;	// Maybe we can step up.
		if (new_lift > 15 || (tflags & (1<<new_lift)))
			return (1);	// Nope, next lift is blocked.
		else
			return (0);
		}
	int i;				// See if we're going down.
	for (i = lift - 1; i >= 0 && !(tflags & (1<<i)); i--)
		;
	new_lift = i + 1;
	return (0);
	}

/*
 *	Activate nearby eggs.
 */

void Chunk_cache::activate_eggs
	(
	Chunk_object_list *chunk,	// Chunk this is attached to.
	int tx, int ty,			// Tile.
	unsigned short eggbits		// Eggs[tile].
	)
	{
					// Get ->usecode machine.
	Usecode_machine *usecode = 
				Game_window::get_game_window()->get_usecode();
	int i;				// Go through eggs.
	for (i = 0; i < 8*sizeof(eggbits) - 1 && eggbits; 
						i++, eggbits = eggbits >> 1)
		{
		Egg_object *egg;
		if ((eggbits&1) && (egg = (Egg_object *) egg_objects.get(i)) &&
		    egg->is_active())
			egg->Egg_object::activate(usecode);
		}
	if (eggbits)			// Check 15th bit.
		{
					// Figure absolute tile coords.
		int atx = chunk->get_cx()*tiles_per_chunk + tx;
		int aty = chunk->get_cy()*tiles_per_chunk + ty;
		int num_eggs = get_num_eggs();
		for ( ; i < num_eggs; i++)
			{
			Egg_object *egg = (Egg_object *) egg_objects.get(i);
			if (egg && egg->within_distance(atx, aty) &&
			    egg->is_active())
				egg->Egg_object::activate(usecode);
			}
		}
	}

/*
 *	Create list for a given chunk.
 */

Chunk_object_list::Chunk_object_list
	(
	int chunkx, int chunky		// Absolute chunk coords.
	) : objects(0), roof(0), npcs(0),
	    cache(0), cx(chunkx), cy(chunky)
	{
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
					// Get x,y of shape within chunk.
	int x = newobj->get_tx(), y = newobj->get_ty();
	int num_entries = 0;		// Need to count as we sort.
	Game_object *obj;
	Game_object *prev = 0;
	for (obj = objects; obj && !newobj->lt(*obj); obj = obj->next)
		prev = obj;
	if (!prev)			// Goes in front?
		{
		newobj->next = objects;
		objects = newobj;
		}
	else
		{
		newobj->next = prev->next;
		prev->next = newobj;
		}
	if (cache)			// Add to cache.
		cache->update_object(this, newobj, 1);
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		roof = 1;
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
	if (cache)			// Add to cache.
		cache->add_egg(this, egg);
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
	if (remove == objects)		// First one?
		{
		objects = remove->next;
		return;
		}
	Game_object *obj;		// Got to find it.
	for (obj = objects; obj && obj->next != remove; obj = obj->next)
		;
	if (obj)			// This is before it.
		obj->next = remove->next;
	}

/*
 *	Create a sequence of frames.
 */

Frames_sequence::Frames_sequence
	(
	int cnt,			// # of frames.
	unsigned char *f		// List of frames.
	)
	{
	frames = new unsigned char[cnt + 1];
	memcpy(frames, f, cnt);		// Copy in the list.
	frames[cnt] = 0xff;		// Terminate it.
	}

/*
 *	Create a moveable sprite.
 */

Sprite::Sprite
	(
	int shapenum
	)  : Container_game_object(), chunk(0),
		x_dir(0), major_frame_incr(8), frames_seq(0)
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
	x_dir = 0;
	if (frames_seq)			// Set to "resting" frame.
		set_frame(frames_seq->get_resting());
	}

/*
 *	Start moving.
 */

void Sprite::start
	(
	Game_window *gwin,		// Game window.
	unsigned long destx,		// Move towards pt. within world.
	unsigned long desty,
	int speed			// # millisecs. between frames.
	)
	{
	if (!in_world())
		return;			// We can't start moving.
	frame_time = speed;
	Direction dir;			// Gets compass direction.++++++Get
					//  northeast, etc. too.
	if (!is_moving())		// Not already moving?
		{			// Start immediately.
		unsigned long curtime = SDL_GetTicks();
		gwin->get_tqueue()->add(curtime, this, (long) gwin);
		}
	curx = get_worldx();		// Get current coords.
	cury = get_worldy();
	sum = 0;			// Clear accumulator.
					// Get change at current lift.
	int liftpixels = 4*get_lift();
	long deltax = destx + liftpixels - curx;
	long deltay = desty + liftpixels - cury;
	unsigned long abs_deltax, abs_deltay;
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
		major_axis = yaxis;
		major_delta = abs_deltay;
		minor_delta = abs_deltax;
		}
	else				// Moving faster along x?
		{
		dir = x_dir > 0 ? east : west;
		major_axis = xaxis;
		major_delta = abs_deltax;
		minor_delta = abs_deltay;
		}
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
	)
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
	unsigned long time,		// Current time.
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_sx, int& new_sy,	// New shape coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
	if (!is_moving())
		return (0);
					// Figure change in faster axis.
	int new_major = major_frame_incr;
					// Accumulate change.
	sum += major_frame_incr * minor_delta;
					// Figure change in slower axis.
	int new_minor = sum/major_delta;
	sum = sum % major_delta;	// Remove what we used.
	if (major_axis == xaxis)	// Which axis?
		{
		curx += x_dir*new_major;
		cury += y_dir*new_minor;
		}
	else
		{
		cury += y_dir*new_major;
		curx += x_dir*new_minor;
		}
	new_cx = curx/chunksize;	// Return new chunk pos.
	new_cy = cury/chunksize;
	new_sx = (curx%chunksize)/8;
	new_sy = (cury%chunksize)/8;
	if (frames_seq)			// Got a sequence of frames?
		next_frame = frames_seq->get_next(frame_index);
	else
		next_frame = -1;
	return (1);
	}

/*
 *	Animation.
 */

void Sprite::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
	{
	Game_window *gwin = (Game_window *) udata;
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	if (next_frame(curtime, cx, cy, sx, sy, frame))
		{
					// Add back to queue for next time.
		gwin->get_tqueue()->add(curtime + frame_time,
							this, udata);
					// Get old rectangle.
		Rectangle oldrect = gwin->get_shape_rect(this);
					// Move it.
		move(cx, cy, gwin->get_objects(cx, cy), sx, sy, frame);
					// Repaint.
		gwin->repaint_sprite(this, oldrect);
		}
	}

#if 0
/*
 *	Create a cyclic sprite.
 */

Frame_cycle_sprite::Frame_cycle_sprite
	(
	Game_window *gwin,
	int shnum, 
	int tx, int ty, 		// Abs. tile coords.
	int lft,			// Lift.
	int ncycs			// Num. cycles to do, or 0.
	) : cycles(ncycs), cycle_num(0), 
	    Game_object(shnum, 0, tx%tiles_per_chunk, ty%tiles_per_chunk, lft)
	{
	int delay = 100;		// Delay between frames.
	frames = gwin->get_shape_num_frames(shnum);
	gwin->get_objects(tx/tiles_per_chunk, ty/tiles_per_chunk)->add(this);
	gwin->get_tqueue()->add(SDL_GetTicks() + delay, this, (long) gwin);
	Rectangle rect = gwin->get_shape_rect(this);
	rect.enlarge(4);
	rect = gwin->clip_to_win(rect);
	gwin->paint(rect);
	}

/*
 *	Animation.
 */

void Frame_cycle_sprite::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
	{
	int delay = 100;		// Delay between frames.
	Game_window *gwin = (Game_window *) udata;
					// Get area we're taking.
	Rectangle rect = gwin->get_shape_rect(this);
	rect.enlarge(4);
	rect = gwin->clip_to_win(rect);
					// Get next frame.
	int framenum = get_framenum() + 1;
	if (framenum >= frames)		// End of cycle?
		{
		if (!cycles || ++cycle_num < cycles)
			framenum = 0;
		else
			{		// All done.
#if 1	/* ++++Not sure yet. */
			gwin->get_objects(get_cx(), get_cy())->remove(this);
			delete this;
			gwin->paint(rect);
			return;
#endif
//			framenum = 0;
			}
		}
	set_frame(framenum);		// Set new frame.
	gwin->paint(rect);		// Paint.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}
#endif

/*
 *	Remove from screen.
 */

void Text_object::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = (Game_window *) udata;
					// Repaint slightly bigger rectangle.
	Rectangle rect((cx - gwin->get_chunkx())*chunksize +
				sx*tilesize - tilesize,
		       (cy - gwin->get_chunky())*chunksize +
				sy*tilesize - tilesize,
			width + 2*tilesize, height + 2*tilesize);
					// Intersect with screen.
	rect = gwin->clip_to_win(rect);
	gwin->remove_text(this);	// Remove & delete this.
	if (rect.w > 0 && rect.h > 0)	// Watch for negatives.
		gwin->paint(rect.x, rect.y, rect.w, rect.h);

	}

/*
 *	Lookup arctangent in a table for degrees 0-85.
 */

static unsigned Lookup_atan
	(
	unsigned dydx
	)
	{
					// 1024*tan(x), where x ranges from
					//   5 deg to 85.
	static unsigned tans[18] = {0, 90, 181, 274, 373, 477, 591, 717, 859,
			1024, 1220, 1462, 1774, 2196, 2813, 3822, 5807, 11704};
	static int cnt = sizeof(tans)/sizeof(tans[0]);
	for (int i = 1; i < cnt; i++)	// Don't bother with 0.
		if (dydx < tans[i])
			return (5*(i - 1));
	return (5*(cnt - 1));
	}

/*
 *	Return the arctangent, rounded to 5-degree increments as an 
 *	angle counter-clockwise from the east.
 *
 *	Output: Arctangent in degrees.
 */

unsigned Arctangent
	(
	int deltay,
	int deltax
	)
	{
	unsigned angle;			// Gets angle in degrees.
	int absx = deltax >= 0 ? deltax : -deltax;
	int absy = deltay >= 0 ? deltay : -deltay;
	if (absy > 23*absx)		// Vertical?
		angle = 90;
	else
		angle = Lookup_atan((1024*absy)/absx);
	if (deltay >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			return angle;
		else			// Top-left?
			return 180 - angle;
	else
		if (deltax >= 0)	// Lower-right.
			return 360 - angle;
		else			// Lower-left.
			return 180 + angle;
	}
