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
	r.tx += ytiles - 1;
	r.ty++;
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
	Tile_coord c			// Rotate around this.
	)
	{
					// Rotate hot spot.
	Tile_coord r = Rotate90l(obj->get_abs_tile_coord(), c);
					// New hot-spot is old lower-left.
	r.tx--;
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
	r.tx += xtiles - 2;
	r.ty -= ytiles;
	return r;
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
 */

void Barge_object::gather
	(
	)
	{
	objects.truncate(perm_count);	// Start fresh.
					// Get footprint in tiles.
	Tile_coord pos = get_abs_tile_coord();
	int xts = get_xtiles(), yts = get_ytiles();
	Rectangle foot(pos.tx - xts + 1, pos.ty - yts + 1, xts, yts);
	Game_window *gwin = Game_window::get_game_window();
					// Go through intersected chunks.
	Chunk_intersect_iterator next_chunk(foot);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		for (Game_object *obj = chunk->get_first(); obj;
						obj = chunk->get_next(obj))
			{		// Look at each object.
			if (obj == this)
				continue;
			Tile_coord t = obj->get_abs_tile_coord();
			Shape_info& info = gwin->get_info(obj);
					// Above barge, within 5-tiles up?
			if (foot.has_point(t.tx, t.ty) &&
			    t.tz + info.get_3d_height() > pos.tz && 
			    (info.is_barge_part() || t.tz < pos.tz + 5) &&
			    obj->get_owner() != this)
				objects.append(obj);
			}
		}
	if (boat == -1)			// Test for boat the first time.
		{
					// Test landscape under hot-spot.
		Chunk_object_list *chunk = gwin->get_objects(cx, cy);
		ShapeID flat = chunk->get_flat(get_tx(), get_ty());
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
	box.enlarge(6);			// Make it a bit bigger.
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
	int cnt = objects.get_cnt();	// We'll move each object.
	for (int i = 0; i < cnt; i++)	// Now add them back in new location.
		{
		Game_object *obj = get_object(i);
		if (i < perm_count)	// Restore us as owner.
			obj->set_owner(this);
		obj->move(positions[i]);
		}
	delete [] positions;
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
					// Get center to rotate around.
	Tile_coord center = get_abs_tile_coord();
	center.tx -= xtiles/2;
	center.ty -= ytiles/2;
	//+++++++Need to check for blocked squares.
					// Move the barge itself.
	Tile_coord rot = Rotate90r(gwin, this, xtiles, ytiles, center);
	Game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 1)%4;		// Increment direction.
	int cnt = objects.get_cnt();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		Shape_info& info = gwin->get_info(obj);
		positions[i] = Rotate90r(gwin, obj, info.get_3d_xtiles(),
						info.get_3d_ytiles(), center);
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
					// Get center to rotate around.
	Tile_coord center = get_abs_tile_coord();
	center.tx -= xtiles/2;
	center.ty -= ytiles/2;
	//+++++++Need to check for blocked squares.
					// Move the barge itself.
	Tile_coord rot = Rotate90l(gwin, this, center);
	Game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 3)%4;		// Increment direction.
	int cnt = objects.get_cnt();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		positions[i] = Rotate90l(gwin, obj, center);
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
					// Get center to rotate around.
	Tile_coord center = get_abs_tile_coord();
	center.tx -= xtiles/2;
	center.ty -= ytiles/2;
					// Move the barge itself.
	Tile_coord rot = Rotate180(gwin, this, xtiles, ytiles, center);
	Game_object::move(rot.tx, rot.ty, rot.tz);
	dir = (dir + 2)%4;		// Increment direction.
	int cnt = objects.get_cnt();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		Shape_info& info = gwin->get_info(obj);
		positions[i] = Rotate180(gwin, obj, info.get_3d_xtiles(),
						info.get_3d_ytiles(), center);
		obj->remove_this(1);	// Remove object from world.
					// Set to rotated frame.
		obj->set_frame(obj->get_rotated_frame(2));
		obj->set_invalid();	// So it gets added back right.
		}
	finish_move(positions);		// Add back & del. positions.
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
	int cnt = objects.get_cnt();	// We'll move each object.
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
#if 0	/* ++++Rake & bucket are showing up as cart's members! */
	if (!complete)			// Permanent member?
		{
		perm_count++;
		obj->set_owner(this);
		}
#endif
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
	Tile_coord t			// Tile to step onto.
	)
	{
	Tile_coord cur = get_abs_tile_coord();
					// Blocked? (Assume ht.=4, for now.)
	if (cur.tz < 11 && 		// But don't check flying carpet.
            Chunk_object_list::is_blocked(get_xtiles(), get_ytiles(), 
								4, cur, t))
		{
		return (0);		// Done.
		}
	move(t.tx, t.ty, t.tz);		// Move it & its objects.
	Game_window *gwin = Game_window::get_game_window();
					// Check for scrolling.
	gwin->scroll_if_needed(gwin->get_main_actor()->get_abs_tile_coord());
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
					// Flags (quality):
	*ptr++ = (dir<<1);
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
	perm_count = objects.get_cnt();
#endif
	perm_count = 0;			// ++++So we don't get haystack!
	complete = 1;
	}

