/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Barge.cc - Ships, carts, flying-carpets.
 **
 **	Written: 7/13/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "barge.h"
#include "gamewin.h"
#include "actors.h"
#include "Zombie.h"
#include "citerate.h"
#include "dir.h"
#include "objiter.h"


using std::ostream;

/*
 *	Rotate a point 90 degrees to the right around a point.
 *
 *	In cartesian coords with 'c' as center, the rule is:
 *		(newx, newy) = (oldy, -oldx)
 */

inline Tile_coord Rotate90r
	(
	Tile_coord t,			// Tile to move.
	Tile_coord c			// Center to rotate around.
	)
	{
					// Get cart. coords. rel. to center.
	int rx = t.tx - c.tx, ry = c.ty - t.ty;
	return Tile_coord(c.tx + ry, c.ty + rx, t.tz);
	}

/*
 *	Rotate a point 90 degrees to the left around a point.
 *
 *	In cartesian coords with 'c' as center, the rule is:
 *		(newx, newy) = (-oldy, oldx)
 */

inline Tile_coord Rotate90l
	(
	Tile_coord t,			// Tile to move.
	Tile_coord c			// Center to rotate around.
	)
	{
					// Get cart. coords. rel. to center.
	int rx = t.tx - c.tx, ry = c.ty - t.ty;
	return Tile_coord(c.tx - ry, c.ty - rx, t.tz);
	}

/*
 *	Rotate a point 180 degrees around a point.
 *
 *	In cartesian coords with 'c' as center, the rule is:
 *		(newx, newy) = (-oldx, -oldy)
 */

inline Tile_coord Rotate180
	(
	Tile_coord t,			// Tile to move.
	Tile_coord c			// Center to rotate around.
	)
	{
					// Get cart. coords. rel. to center.
	int rx = t.tx - c.tx, ry = c.ty - t.ty;
	return Tile_coord(c.tx - rx, c.ty + ry, t.tz);
	}

/*
 *	Figure tile where an object will be if it's rotated 90 degrees around
 *	a point counterclockwise, assuming its 'hot spot' 
 *	is at its lower-right corner.
 */

inline Tile_coord Rotate90r
	(
	Game_window *gwin,
	Game_object *obj,
	int xtiles, int ytiles,		// Object dimensions.
	Tile_coord c			// Rotate around this.
	)
	{
					// Rotate hot spot.
	Tile_coord r = Rotate90r(obj->get_abs_tile_coord(), c);
					// New hotspot is what used to be the
					//   upper-right corner.
	r.tx += ytiles;
	return r;
	}

/*
 *	Figure tile where an object will be if it's rotated 90 degrees around
 *	a point, assuming its 'hot spot' is at its lower-right corner.
 */

inline Tile_coord Rotate90l
	(
	Game_window *gwin,
	Game_object *obj,
	int xtiles, int ytiles,		// Object dimensions.
	Tile_coord c			// Rotate around this.
	)
	{
					// Rotate hot spot.
	Tile_coord r = Rotate90l(obj->get_abs_tile_coord(), c);
					// New hot-spot is old lower-left.
	r.ty += xtiles;
	return r;
	}

/*
 *	Figure tile where an object will be if it's rotated 180 degrees around
 *	a point, assuming its 'hot spot' is at its lower-right corner.
 */

inline Tile_coord Rotate180
	(
	Game_window *gwin,
	Game_object *obj,
	int xtiles, int ytiles,		// Object dimensions.
	Tile_coord c			// Rotate around this.
	)
	{
					// Rotate hot spot.
	Tile_coord r = Rotate180(obj->get_abs_tile_coord(), c);
					// New hotspot is what used to be the
					//   upper-left corner.
	r.tx += xtiles;
	r.ty += ytiles;
	return r;
	}

/*
 *	Swap dimensions.
 */

inline void Barge_object::swap_dims
	(
	)
	{
	int tmp = xtiles;		// Swap dims.
	xtiles = ytiles;
	ytiles = tmp;
	}

/*
 *	Get footprint in tiles.
 */

inline Rectangle Barge_object::get_tile_footprint
	(
	)
	{
	Tile_coord pos = get_abs_tile_coord();
	int xts = get_xtiles(), yts = get_ytiles();
	Rectangle foot(pos.tx - xts + 1, pos.ty - yts + 1, xts, yts);
	return foot;
	}

/*
 *	Set center.
 */

inline void Barge_object::set_center
	(
	)
	{
	center = get_abs_tile_coord();
	center.tx -= xtiles/2;
	center.ty -= ytiles/2;
	}

/*
 *	See if okay to rotate.
 */

int Barge_object::okay_to_rotate
	(
	Tile_coord pos			// New position (bottom-right).
	)
	{
	int lift = get_lift();
	if (lift >= 11)			// Flying carpet?
		return 1;
					// Get footprint in tiles.
	Rectangle foot = get_tile_footprint();
	int xts = get_xtiles(), yts = get_ytiles();
					// Get where new footprint will be.
	Rectangle newfoot(pos.tx - yts + 1, pos.ty - xts + 1, yts, xts);
	int new_lift;			// Ignored.
	if (newfoot.y < foot.y)		// Got a piece above the old one?
					// Check area.  (No dropping allowed.)
		if (Chunk_object_list::is_blocked(4, lift,
			newfoot.x, newfoot.y, newfoot.w, foot.y - newfoot.y,
						new_lift, MOVE_ALL_TERRAIN, 0))
			return 0;
	if (foot.y + foot.h < newfoot.y + newfoot.h)
					// A piece below old one.
		if (Chunk_object_list::is_blocked(4, lift,
			newfoot.x, foot.y + foot.h, newfoot.w, 
				newfoot.y + newfoot.h - (foot.y + foot.h),
						new_lift, MOVE_ALL_TERRAIN, 0))
			return 0;
	if (newfoot.x < foot.x)		// Piece to the left?
		if (Chunk_object_list::is_blocked(4, lift,
			newfoot.x, newfoot.y, foot.x - newfoot.x, newfoot.h,
						new_lift, MOVE_ALL_TERRAIN, 0))
			return 0;
	if (foot.x + foot.w < newfoot.x + newfoot.w)
					// Piece to the right.
		if (Chunk_object_list::is_blocked(4, lift,
			foot.x + foot.w, newfoot.y,
			newfoot.x + newfoot.w - (foot.x + foot.w), newfoot.h,
						new_lift, MOVE_ALL_TERRAIN, 0))
			return 0;
	return 1;
	}

/*
 *	Delete.
 */

Barge_object::~Barge_object
	(
	)
	{
	delete path;
	}

/*
 *	Gather up all objects that appear to be on this barge.
 *	Also inits. 'center'.
 */

void Barge_object::gather
	(
	)
	{
	objects.resize(perm_count);	// Start fresh.
					// Get footprint in tiles.
	Rectangle foot = get_tile_footprint();
	int lift = get_lift();		// How high we are.
	Game_window *gwin = Game_window::get_game_window();
					// Go through intersected chunks.
	Chunk_intersect_iterator next_chunk(foot);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		Game_object *obj;
		Object_iterator next(chunk);
		while ((obj = next.get_next()) != 0)
			{		// Look at each object.
			if (obj == this)
				continue;
			Tile_coord t = obj->get_abs_tile_coord();
			Shape_info& info = gwin->get_info(obj);
					// Above barge, within 5-tiles up?
			if (foot.has_point(t.tx, t.ty) &&
			    t.tz + info.get_3d_height() > lift && 
			    (info.is_barge_part() || t.tz < lift + 5) &&
			    obj->get_owner() != this)
				objects.append(obj);
			}
		}
					// Test landscape under center.
	set_center();
	if (boat == -1)			// Test for boat the first time.
		{
		Chunk_object_list *chunk = gwin->get_objects(
			center.tx/tiles_per_chunk, center.ty/tiles_per_chunk);
		ShapeID flat = chunk->get_flat(center.tx%tiles_per_chunk,
						center.ty%tiles_per_chunk);
		if (flat.is_invalid())
			boat = 0;
		else
			{
			Shape_info& info = gwin->get_info(flat.get_shapenum());
			boat = info.is_water();
			}
		}
	}

/*
 *	Add a dirty rectangle for our current position.
 */

void Barge_object::add_dirty
	(
	Game_window *gwin
	)
	{
	int x, y;			// Get lower-right corner.
	gwin->get_shape_location(this, x, y);
	int w = xtiles*tilesize, h = ytiles*tilesize;
	Rectangle box(x - w, y - h, w, h);
	box.enlarge(10);		// Make it a bit bigger.
	if (dir%2)			// Horizontal?  Stretch.
		{ box.x -= 15; box.w += 30; }
	else
		{ box.y -= 15; box.h += 30; }
	box = gwin->clip_to_win(box);	// Intersect with screen.
	gwin->add_dirty(box);
	}

/*
 *	Finish up moving all the objects by adding them back and deleting the
 *	saved list of positions.
 */

void Barge_object::finish_move
	(
	Tile_coord *positions		// New positions.  Deleted when done.
	)
	{
	set_center();			// Update center.
	int cnt = objects.size();	// We'll move each object.
	for (int i = 0; i < cnt; i++)	// Now add them back in new location.
		{
		Game_object *obj = get_object(i);
		if (i < perm_count)	// Restore us as owner.
			obj->set_owner(this);
		obj->move(positions[i]);
		}
	delete [] positions;
	Game_window *gwin = Game_window::get_game_window();
					// Check for scrolling.
	gwin->scroll_if_needed(center);
	}


/*
 *	Travel towards a given tile.
 */

void Barge_object::travel_to_tile
	(
	Tile_coord dest,		// Destination.
	int speed			// Time between frames (msecs).
	)
	{
	if (!path)
		path = new Zombie();
					// Set up new path.
	if (path->NewPath(get_abs_tile_coord(), dest, 0))
		{
		frame_time = speed;
		Game_window *gwin = Game_window::get_game_window();
					// Figure new direction.
		Tile_coord cur = get_abs_tile_coord();
		int ndir = Get_direction4(cur.ty - dest.ty, dest.tx - cur.tx);
		ndir /= 2;		// Convert to 0-3.
		switch ((4 + ndir - dir)%4)
			{
		case 1:			// Rotate 90 degrees right.
			turn_right();
			break;
		case 2:
			turn_around();	// 180 degrees.
			break;
		case 3:
			turn_left();
			break;
		default:
			break;
			}
		if (!in_queue())	// Not already in queue?
			gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
		}
	else
		frame_time = 0;		// Not moving.
	}

/*
 *	Turn 90 degrees to the right.
 */

void Barge_object::turn_right
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	add_dirty(gwin);		// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate90r(gwin, this, xtiles, ytiles, center);
	if (!okay_to_rotate(rot))	// Check for blockage.
		return;
	Game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 1)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = gwin->get_info(obj);
		positions[i] = Rotate90r(gwin, obj, info.get_3d_xtiles(frame),
					info.get_3d_ytiles(frame), center);
		obj->remove_this(1);	// Remove object from world.
					// Set to rotated frame.
		obj->set_frame(obj->get_rotated_frame(1));
		obj->set_invalid();	// So it gets added back right.
		}
	finish_move(positions);		// Add back & del. positions.
	}

/*
 *	Turn 90 degrees to the left.
 */

void Barge_object::turn_left
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	add_dirty(gwin);		// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate90l(gwin, this, xtiles, ytiles, center);
	if (!okay_to_rotate(rot))	// Check for blockage.
		return;
	Game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 3)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = gwin->get_info(obj);
		positions[i] = Rotate90l(gwin, obj, info.get_3d_xtiles(frame),
					info.get_3d_ytiles(frame), center);
		obj->remove_this(1);	// Remove object from world.
					// Set to rotated frame.
		obj->set_frame(obj->get_rotated_frame(3));
		obj->set_invalid();	// So it gets added back right.
		}
	finish_move(positions);		// Add back & del. positions.
	}

/*
 *	Turn 180 degrees.
 */

void Barge_object::turn_around
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	add_dirty(gwin);		// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate180(gwin, this, xtiles, ytiles, center);
	Game_object::move(rot.tx, rot.ty, rot.tz);
	dir = (dir + 2)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = gwin->get_info(obj);
		positions[i] = Rotate180(gwin, obj, info.get_3d_xtiles(frame),
					info.get_3d_ytiles(frame), center);
		obj->remove_this(1);	// Remove object from world.
					// Set to rotated frame.
		obj->set_frame(obj->get_rotated_frame(2));
		obj->set_invalid();	// So it gets added back right.
		}
	finish_move(positions);		// Add back & del. positions.
	}

/*
 *	Is it okay to land?
 */

int Barge_object::okay_to_land
	(
	)
	{
	Rectangle foot = get_tile_footprint();
	int lift = get_lift();		// How high we are.
	Game_window *gwin = Game_window::get_game_window();
					// Go through intersected chunks.
	Chunk_intersect_iterator next_chunk(foot);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{			// Check each tile.
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		for (int ty = tiles.y; ty < tiles.y + tiles.h; ty++)
			for (int tx = tiles.x; tx < tiles.x + tiles.w; tx++)
				if (chunk->get_highest_blocked(lift, tx, ty)
								!= -1)
					return (0);
		}
	return (1);
	}

/*
 *	Handle a time event (for animation).
 */

void Barge_object::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (!path || !frame_time || gwin->get_moving_barge() != this)
		return;			// We shouldn't be doing anything.
	Tile_coord tile;		// Get spot & walk there.
	if (!path->GetNextStep(tile) || !step(tile))
		frame_time = 0;
	else
		gwin->get_tqueue()->add(curtime + frame_time, this, udata);
	}

/*
 *	Move to a new absolute location.
 */

void Barge_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
					// Want to repaint old position.
	add_dirty(Game_window::get_game_window());
					// Get current location.
	Tile_coord old = get_abs_tile_coord();
					// Move the barge itself.
	Game_object::move(newtx, newty, newlift);
					// Get deltas.
	int dx = newtx - old.tx, dy = newty - old.ty, dz = newlift - old.tz;
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	int i;
	for (i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int ox, oy, oz;
		obj->get_abs_tile(ox, oy, oz);
		positions[i] = Tile_coord(ox + dx, oy + dy, oz + dz);
		obj->remove_this(1);	// Remove object from world.
		obj->set_invalid();	// So it gets added back right.
					// Animate a few shapes.
		int frame = obj->get_framenum();
		switch (obj->get_shapenum())
			{
		case 774:		// Cart wheel.
			obj->set_frame(((frame + 1)&3)|(frame&32));
			break;
		case 796:		// Draft horse.
			obj->set_frame(((frame + 4)&15)|(frame&32));
			break;
			}
		}
	finish_move(positions);		// Add back & del. positions.
	}

/*
 *	Remove an object.
 */

void Barge_object::remove
	(
	Game_object *obj
	)
	{
	obj->set_owner(0);
	obj->remove_this(1);		// Now remove from outside world.
	}

/*
 *	Add an object.
 *
 *	Output:	0, meaning object should also be added to chunk.
 */

int Barge_object::add
	(
	Game_object *obj,
	int dont_check
	)
	{
	objects.append(obj);		// Add to list.
	return (0);			// We want it added to the chunk.
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Barge_object::drop
	(
	Game_object *obj
	)
	{
	return 0;			//++++++Later.
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
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 *		Dormant is set if off screen.
 */

int Barge_object::step
	(
	Tile_coord t,			// Tile to step onto.
	int				// Frame (ignored).
	)
	{
	Tile_coord cur = get_abs_tile_coord();
					// Blocked? (Assume ht.=4, for now.)
	if (cur.tz < 11) 		// But don't check flying carpet.
		{
		int move_type;
		if (boat) move_type = MOVE_SWIM;
		else move_type = MOVE_WALK;
        	if (Chunk_object_list::is_blocked(get_xtiles(), get_ytiles(), 
						4, cur, t, move_type))
			return (0);	// Done.
		}
	move(t.tx, t.ty, t.tz);		// Move it & its objects.
	Game_window *gwin = Game_window::get_game_window();
					// Near an egg?
	Chunk_object_list *nlist = gwin->get_objects(get_cx(), get_cy());
	nlist->activate_eggs(gwin->get_main_actor(), t.tx, t.ty, t.tz, 
						cur.tx, cur.ty);
	return (1);			// Add back to queue for next time.
	}

/*
 *	Write out.
 */

void Barge_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	unsigned char *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
					// Write size.
	*ptr++ = xtiles;
	*ptr++ = ytiles;
	*ptr++ = 0;			// Unknown.
					// Flags (quality).  Taking B3 to in-
					//   dicate barge mode.
	*ptr++ = (dir<<1) | 
		((Game_window::get_game_window()->get_moving_barge() == this)
								<<3);
	*ptr++ = 0;			// (Quantity).
	*ptr++ = (get_lift()&15)<<4;
	*ptr++ = 0;			// Data2.
	*ptr++ = 0;			// 
	out.write((char*)buf, sizeof(buf));
					// Write permanent objects.
	for (int i = 0; i < perm_count; i++)
		{
		Game_object *obj = get_object(i);
		obj->write_ireg(out);
		}
	out.put(0x01);			// A 01 terminates the list.
	}

/*
 *	This is called after all elements have been read in and added.
 */

void Barge_object::elements_read
	(
	)
	{
#if 0
	perm_count = objects.size();
#endif
	perm_count = 0;			// So we don't get haystack!
	complete = 1;
	}

