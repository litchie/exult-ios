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
 *	Paint at given spot in world.
 */

void Game_object::paint
	(
	Game_window *gwin
	)
	{
	int xoff = (cx - gwin->get_chunkx())*chunksize;
	int yoff = (cy - gwin->get_chunky())*chunksize;
	gwin->paint_shape(xoff + (1 + get_tx())*tilesize - 4*lift, 
				yoff + (1 + get_ty())*tilesize - 4*lift,
					get_shapenum(), get_framenum());
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

#if 1	/* +++++For new rendering scheme.+++++++*/

static void Lt_wall()		//+++++Debugging.
	{ cout << "Before the wall.\n"; }

/*
 *	Should this object be rendered before obj2?
 *
 *	Output:	1 if so, 0 if not, -1 if cannot compare.
 */
int Game_object::lt
	(
	Game_object& obj2
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shapes_vga_file& shapes = gwin->get_shapes();
	int shapenum1 = get_shapenum(), shapenum2 = obj2.get_shapenum();
	Shape_info& info1 = shapes.get_info(shapenum1),
		    info2 = shapes.get_info(shapenum2);
					// Get absolute tile positions.
	int atx1, aty1, atz1, atx2, aty2, atz2;
	int x1, x2, y1, y2, z1, z2;	// Dims. in tiles.
	get_abs_tile(atx1, aty1, atz1);
	obj2.get_abs_tile(atx2, aty2, atz2);
					// See if there's no overlap.
	Rectangle r1 = gwin->get_shape_rect(this),
		  r2 = gwin->get_shape_rect(&obj2);
	if (!r1.intersects(r2))
		return (-1);		// No overlap on screen.
	x1 = info1.get_3d_xtiles(), x2 = info2.get_3d_xtiles();
	y1 = info1.get_3d_ytiles(), y2 = info2.get_3d_ytiles();
	z1 = info1.get_3d_height(), z2 = info2.get_3d_height();
	if (atz1 != atz2)		// Is one obj. on top of another?
		{
		if (atz1 + z1 <= atz2)
			return (1);	// It's above us.
		if (atz2 + z2 <= atz1)
			return (0);	// We're above.
		}
	if (aty1 != aty2)		// Is one obj. in front of the other?
		{
		if (aty1 <= aty2 - y2)
			return (1);	// Obj2 is in front.
		if (aty2 <= aty1 - y1)
			return (0);	// We're in front.
		}
	if (atx1 != atx2)		// Is one obj. to right of the other?
		{
		if (atx1 <= atx2 - x2)
			return (1);	// Obj2 is to right of us.
		if (atx2 <= atx1 - x1)
			return (0);	// We're to the right.
		}
					// Handle intersecting objects.
					// If x's overlap, see if in front.
	if ((x1 > y1) && ((atx1 > atx2 - x2 && atx1 <= atx2) ||
	    (atx2 > atx1 - x1 && atx2 <= atx1)))
		{
		if (aty1 < aty2)
			return (1);
		else if (aty1 > aty2)
			return (0);
		else if (x1 < x2)	// Take the narrower one as greater.
			return (0);
		else if (x1 > x2)
			return (1);
		else if (z1 < z2)	// The narrower one?
			return (0);
		else if (z1 > z2)
			return (1);
		else
			return (-1);
		}
					// If y's overlap, see if to left.
	if ((aty1 > aty2 - y2 && aty1 <= aty2) ||
	    (aty2 > aty1 - y1 && aty2 <= aty1))
		{
		if (atx1 < atx2)
			return (1);
		else if (atx1 > atx2)
			return (0);
		else if (y1 < y2)	// Take narrower 2nd.
			return (0);
		else if (y2 > y1)
			return (1);
		else if (z1 < z2)	// The narrower one?
			return (0);
		else if (z1 > z2)
			return (1);
		}
	return (-1);
	}

#endif

/*
 *	Can this be dragged?
 */

int Ireg_game_object::is_dragable
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// 0 weight means 'too heavy'.
	return gwin->get_shapes().get_info(get_shapenum()).get_weight() > 0;
	}

#if 1
/*
 *	Create an animated object.
 */

Animated_object::Animated_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex,
	unsigned int shapey,
	unsigned int lft
	) : Ireg_game_object(l, h, shapex, shapey, lft), cycles(0),
		cycle_num(0), animating(0)
	{
	Game_window *gwin = Game_window::get_game_window();
	int delay = 100;		// Delay between frames.
	frames = gwin->get_shape_num_frames(get_shapenum());
	}

/*
 *	Render.
 */

void Animated_object::paint
	(
	Game_window *gwin
	)
	{
	Ireg_game_object::paint(gwin);
	if (!animating)			// Turn on animation.
		{
		gwin->get_tqueue()->add(SDL_GetTicks() + 100, 
							this, (long) gwin);
		animating = 1;
		}
	}

/*
 *	Animation.
 */

void Animated_object::handle_event
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
	if (rect.w <= 0 || rect.h <= 0)	// No longer on screen?
		{
		animating = 0;
		return;
		}
					// Get next frame.
	int framenum = get_framenum() + 1;
	if (framenum >= frames)		// End of cycle?
		{
		framenum = 0;
		if (cycles && ++cycle_num >= cycles)
			animating = 0;
		}
	set_frame(framenum);		// Set new frame.
	gwin->add_dirty(rect);		// Paint.
					// Add back to queue for next time.
	if (animating)
		gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

#endif

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
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this.
 */

int Container_game_object::add
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
	return 1;
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
 *	Add an object.
 *
 *	Output:	0, meaning object should also be added to chunk.
 */

int Barge_object::add
	(
	Game_object *obj
	)
	{
	return (0);			//+++++++Later, dude.
	}

/*
 *	Paint at given spot in world.
 */

void Barge_object::paint
	(
	Game_window *gwin
	)
	{
					// DON'T paint barge shape itself.
					// The objects are in the chunk too.
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine
	)
	{
#if DEBUG
cout << "Egg type is " << (int) type << ", prob = " << (int) probability <<
		", distance = " << (int) distance << ", crit = " <<
		(int) criteria << ", once = " <<
	((flags & (1<<(int)once) != 0)) << ", areset = " <<
	((flags & (1<<(int)auto_reset) != 0)) << ", data2 = " << data2
		<< '\n';
#endif
	int roll = 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
//TESTING:
	static int cnt = 0;
//	audio.start_speech(cnt++);//++++++++++++
	flags |= (1 << (int) hatched);	// Flag it as done.
	switch(type)
		{
		case jukebox:
#if DEBUG
			cout << "Audio parameters might be: " << (data1&0xff) << " and " << ((data1>>8)&0xff) << endl;
#endif
			audio.start_music((data1)&0xff,(data1>>8)&0xff);
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
	if (!ztiles || !info.is_solid())
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
#if 0
	for (obj = objects; obj && !newobj->lt(*obj); obj = obj->next)
		prev = obj;
#else	/* ++++++Testing */
#if 0
	for (obj = objects; obj; obj = obj->next)
		{
		int cmp = newobj->lt(*obj);
		if (!cmp)		// Bigger than this object?
			prev = obj;	// Let's find last that we're bigger
					//   than.
		}
#else	/* Just sort by lift. */
	for (obj = objects; obj && newobj->get_lift() > obj->get_lift(); 
							obj = obj->next)
		prev = obj;

#endif
#endif
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
#if 1
					// Figure dependencies.
	for (obj = objects; obj; obj = obj->next)
		{
		int cmp = newobj->lt(*obj);
		if (!cmp)		// Bigger than this object?
			newobj->dependencies.put(obj);
		else if (cmp == 1)	// Smaller than?
			obj->dependencies.put(newobj);
		}
#endif
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
	remove->clear_dependencies();	// Remove all dependencies.
	Game_object *obj;
	for (obj = objects; obj; obj = obj->next)
		obj->remove_dependency(remove);
	if (remove == objects)		// First one?
		{
		objects = remove->next;
		return;
		}
					// Find obj. in list.
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
 *	Return the direction for a given slope.
 */

Direction Get_direction
	(
	int deltay,
	int deltax
	)
	{
	if (deltax == 0)
		return deltay > 0 ? north : south;
	int dydx = (1024*deltay)/deltax;// Figure 1024*tan.
	if (dydx >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			return dydx <= 424 ? east : dydx <= 2472 ? northeast
								: north;
		else			// Lower-left.
			return dydx <= 424 ? west : dydx <= 2472 ? southwest
								: south;
	else
		if (deltax >= 0)	// Lower-right.
			return dydx >= -424 ? east : dydx >= -2472 ? southeast
								: south;
		else			// Top-left?
			return dydx >= -424 ? west : dydx >= -2472 ? northwest
								: north;
	}

#if 0
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
#endif
