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
#include <string.h>

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
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine
	)
	{
cout << "Egg type is " << (int) type << ", prob = " << (int) probability <<
		", distance = " << (int) distance << '\n';
	if (type == (int) usecode)	// Data2 is the usecode function.
		umachine->call_usecode(data2, this,
					Usecode_machine::egg_proximity);
	}

/*
 *	Create the cached data storage for a chunk.
 */

Chunk_cache::Chunk_cache
	(
	) : setup_done(0)
	{
	memset((char *) &blocked[0], 0, sizeof(blocked));
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
 *	Set the blocked flags in a region.
 */

void Chunk_cache::set_blocked
	(
	int startx, int starty,		// Starting tile #'s.
	int endx, int endy,		// Ending tile #'s.
	int lift, int ztiles		// Lift, height info.
	)
	{
	for (int y = starty; y <= endy; y++)
		for (int x = startx; x <= endx; x++)
			set_blocked_tile(x, y, lift, ztiles);
	}

/*
 *	Create the cached data for a chunk.
 */

void Chunk_cache::setup
	(
	Game_window *gwin,
	int cx, int cy,			// This chunk's (absolute) coords.
	Chunk_object_list *chunk
	)
	{
	Shapes_vga_file& shapes = gwin->get_shapes();
					// Set 'blocked' tiles.
	for (Game_object *obj = chunk->get_first(); obj; 
						obj = chunk->get_next(obj))
		{
		int shnum = obj->get_shapenum();
		Shape_info& info = shapes.get_info(shnum);
		int ztiles = info.get_3d_height(); 
		if (!ztiles)
			continue;	// Skip if not an obstacle.
					// Get lower-right corner of obj.
		int endx = obj->get_shape_pos_x();
		int endy = obj->get_shape_pos_y();
					// Get footprint dimensions.
		int xtiles = info.get_3d_xtiles();
		int ytiles = info.get_3d_ytiles();
		int lift = obj->get_lift();
					// Simplest case?
		if (xtiles == 1 && ytiles == 1)
			{
			set_blocked_tile(endx, endy, lift, ztiles);
			continue;
			}
		int startx = endx - xtiles + 1, starty = endy - ytiles + 1;
					// First this chunk.
		int this_startx = startx < 0 ? 0 : startx;
		int this_starty = starty < 0 ? 0 : starty;
		set_blocked(this_startx, this_starty, endx, endy, 
								lift, ztiles);
					// Overlaps chunk to the left?
		if (startx < 0 && cx > 0)
			{
			gwin->get_objects(cx - 1, cy)->set_blocked(
				startx + tiles_per_chunk,
				this_starty, 15, endy, lift, ztiles);
					// Chunk to left and above?
			if (starty < 0 && cy > 0)
				gwin->get_objects(cx - 1, cy - 1)->set_blocked(
					startx + tiles_per_chunk,
					starty + tiles_per_chunk, 
					15, 15, lift, ztiles);
			}
					// Chunk directly above?
		if (starty < 0 && cy > 0)
			gwin->get_objects(cx, cy - 1)->set_blocked(
				this_startx,
				starty + tiles_per_chunk, endx, 15,
				lift, ztiles);

		}
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
	int tilex, int tiley,		// Square to test.
	int& new_lift			// New lift returned.
	)
	{
#if 0			/* ++++++Until we get the right footprint dims. */
	new_lift = lift;
	return (0);
#else
					// Get bits.
	unsigned short tflags = blocked[tiley*tiles_per_chunk + tilex];
	if (tflags & (1<<lift))		// Something there?
		{
		new_lift = lift + 1;	// Maybe we can step up.
		if (new_lift > 15 || (tflags & (1<<new_lift)))
			return (1);	// Nope, next lift is blocked.
		else
			return (0);
		}
	int i;				// See if we're going down.
	for (i = lift - 1; i >= 0 && !(tflags & (1<<lift)); i--)
		;
	new_lift = i + 1;
	return (0);
#endif
	}

/*
 *	Create list for a given chunk.
 */

Chunk_object_list::Chunk_object_list
	(
	) : objects(0), roof(0), egg_objects(0), num_eggs(0), npcs(0), cache(0)
	{
	memset((char *) &eggs[0], 0xff, sizeof(eggs));
	}

/*
 *	Add a game object to a chunk's list.
 */

void Chunk_object_list::add
	(
	Game_object *newobj		// Object to add.
	)
	{
					// Get x,y of shape within chunk.
	int x = newobj->get_shape_pos_x(), y = newobj->get_shape_pos_y();
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
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		roof = 1;
	}

/*
 *	Add a game object to a chunk's list.
 */

void Chunk_object_list::add_egg
	(
	Egg_object *egg			// Object to add.
	)
	{
					// Get x,y of shape within chunk.
	int x = egg->get_shape_pos_x(), y = egg->get_shape_pos_y();
	unsigned char& spot = eggs[y*16 + x];
	if (spot != 0xff)		// One already there?
		return;
	int new_cnt = num_eggs + 1;	// Create new list.
	Egg_object **newlist = new Egg_object*[new_cnt];
	for (int i = 0; i < num_eggs; i++)
		newlist[i] = egg_objects[i];
	spot = num_eggs;		// Store offset in list.
	newlist[num_eggs++] = egg;	// Store new egg at end.
	delete [] egg_objects;
	egg_objects = newlist;
					// ++++Testing:
	add(egg);			// So we can see it.
	}

/*
 *	Remove a game object from this list.
 */

void Chunk_object_list::remove
	(
	Game_object *remove
	)
	{
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
	)  : Container_game_object(), cx(-1), cy(-1), chunk(0),
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
	long deltax = destx - curx;	// Get changes.
	long deltay = desty - cury;
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
	long udata			// Ignored.
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

#if 0
/*
 *	Lookup arctangent in a table for degrees 5-85.
 */

static unsigned Lookup_atan
	(
	unsigned dydx
	)
	{
					// 1024*tan(x), where x ranges from
					//   5 deg to 85.
	static unsigned tans[17] = {90, 181, 274, 373, 477, 591, 717, 859,
			1024, 1220, 1462, 1774, 2196, 2813, 3822, 5807, 11704};
++++++++++++++++++++++++++++++++++++

/*
 *	Return the arctangent, in 5-degree increments as an angle counter-
 *	clockwise from the east.
 *
 *	Output: Arctangent/5 degrees.
 */

unsigned Arctangent
	(
	int deltay,
	int deltax
	)
	{
	if (!deltax)			// Vertical?
		return (deltax > 0 ? 90/5 : 270/5);
	if (deltay >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			{
#endif
