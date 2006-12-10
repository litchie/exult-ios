/**
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gamemap.h"
#include "chunks.h"
#include "barge.h"
#include "gamewin.h"
#include "actors.h"
#include "Zombie.h"
#include "citerate.h"
#include "dir.h"
#include "objiter.h"
#include "game.h"
#include "databuf.h"
#include "ucsched.h"
#include "cheat.h"
#include "exult.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

using std::ostream;
using std::cout;
using std::endl;

Barge_object *Barge_object::editing = 0;

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
	Tile_coord r = Rotate90r(obj->get_tile(), c);
					// New hotspot is what used to be the
					//   upper-right corner.
	r.tx = (r.tx + ytiles + c_num_tiles)%c_num_tiles;
	r.ty = (r.ty + c_num_tiles)%c_num_tiles;
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
	Tile_coord r = Rotate90l(obj->get_tile(), c);
					// New hot-spot is old lower-left.
	r.ty = (r.ty + xtiles + c_num_tiles)%c_num_tiles;
	r.tx = (r.tx + c_num_tiles)%c_num_tiles;
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
	Tile_coord r = Rotate180(obj->get_tile(), c);
					// New hotspot is what used to be the
					//   upper-left corner.
	r.tx = (r.tx + xtiles + c_num_tiles)%c_num_tiles;
	r.ty = (r.ty + ytiles + c_num_tiles)%c_num_tiles;
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

Rectangle Barge_object::get_tile_footprint
	(
	)
	{
	Tile_coord pos = get_tile();
	int xts = get_xtiles(), yts = get_ytiles();
	Rectangle foot((pos.tx - xts + 1 + c_num_tiles)%c_num_tiles, 
		(pos.ty - yts + 1 + c_num_tiles)%c_num_tiles, xts, yts);
	return foot;
	}

/*
 *	Set center.
 */

inline void Barge_object::set_center
	(
	)
	{
	center = get_tile();
	center.tx = (center.tx - xtiles/2 + c_num_tiles)%c_num_tiles;
	center.ty = (center.ty - ytiles/2 + c_num_tiles)%c_num_tiles;
	}

/*
 *	See if okay to rotate.
 +++++++++++Handle wrapping here+++++++++++
 */

int Barge_object::okay_to_rotate
	(
	Tile_coord pos			// New position (bottom-right).
	)
	{
	int lift = get_lift();
					// Special case for carpet.
	int move_type = (lift >= 10) ? (MOVE_LEVITATE) : MOVE_ALL_TERRAIN;
					// Get footprint in tiles.
	Rectangle foot = get_tile_footprint();
	int xts = get_xtiles(), yts = get_ytiles();
					// Get where new footprint will be.
	Rectangle newfoot(pos.tx - yts + 1, pos.ty - xts + 1, yts, xts);
	int new_lift;
	if (newfoot.y < foot.y)		// Got a piece above the old one?
					// Check area.  (No dropping allowed.)
		if (Map_chunk::is_blocked(4, lift,
			newfoot.x, newfoot.y, newfoot.w, foot.y - newfoot.y,
				new_lift, move_type, 0) || new_lift != lift)
			return 0;
	if (foot.y + foot.h < newfoot.y + newfoot.h)
					// A piece below old one.
		if (Map_chunk::is_blocked(4, lift,
			newfoot.x, foot.y + foot.h, newfoot.w, 
			newfoot.y + newfoot.h - (foot.y + foot.h),
				new_lift, move_type, 0) || new_lift != lift)
			return 0;
	if (newfoot.x < foot.x)		// Piece to the left?
		if (Map_chunk::is_blocked(4, lift,
			newfoot.x, newfoot.y, foot.x - newfoot.x, newfoot.h,
				new_lift, move_type, 0) || new_lift != lift)
			return 0;
	if (foot.x + foot.w < newfoot.x + newfoot.w)
					// Piece to the right.
		if (Map_chunk::is_blocked(4, lift,
			foot.x + foot.w, newfoot.y,
			newfoot.x + newfoot.w - (foot.x + foot.w), newfoot.h,
				new_lift, move_type, 0) || new_lift != lift)
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
	if (!gmap->get_chunk_safely(get_cx(), get_cy()))
		return;			// Not set in world yet.
	ice_raft = false;		// We'll just detect it each time.
	objects.resize(perm_count);	// Start fresh.
					// Get footprint in tiles.
	Rectangle foot = get_tile_footprint();
	int lift = get_lift();		// How high we are.
					// Go through intersected chunks.
	Chunk_intersect_iterator next_chunk(foot);
	Rectangle tiles;
	int cx, cy;
	bool si = Game::get_game_type() == SERPENT_ISLE;
	while (next_chunk.get_next(tiles, cx, cy))
		{
		Map_chunk *chunk = gmap->get_chunk(cx, cy);
		tiles.x += cx*c_tiles_per_chunk;
		tiles.y += cy*c_tiles_per_chunk;
		Game_object *obj;
		Object_iterator next(chunk->get_objects());
		while ((obj = next.get_next()) != 0)
			{		// Look at each object.
			if (obj == this)
				continue;
			if (obj->is_egg()) // don't pick up eggs
				continue;
			Tile_coord t = obj->get_tile();
			if (!tiles.has_point(t.tx, t.ty) ||
				obj->get_owner() == this)
				continue;
			Shape_info& info = obj->get_info();
					// Above barge, within 5-tiles up?
			bool isbarge = info.is_barge_part() || !info.get_weight();
			if (t.tz + info.get_3d_height() > lift && 
			    ((isbarge && t.tz >= lift - 1) ||
				(t.tz < lift + 5 && t.tz >= lift + 1)))
				{
				objects.push_back(obj);
				if (si)
					{
					if (obj->get_shapenum() == 0x1f8)
						ice_raft = true;
					// Kludge for SI turtle.
					else if (obj->get_shapenum() == 0xd7)
						xtiles = 20;
					}
				}
			}
		}
	set_center();
					// Test for boat.
	Map_chunk *chunk = gmap->get_chunk_safely(
		center.tx/c_tiles_per_chunk, center.ty/c_tiles_per_chunk);
	if (boat == -1 && chunk != 0)
		{
		ShapeID flat = chunk->get_flat(center.tx%c_tiles_per_chunk,
						center.ty%c_tiles_per_chunk);
		if (flat.is_invalid())
			boat = 0;
		else
			{
			Shape_info& info = flat.get_info();
			boat = info.is_water();
			}
		}
	gathered = true;
	}

/*
 *	Add a dirty rectangle for our current position.
 */

void Barge_object::add_dirty
	(
	)
	{
	int x, y;			// Get lower-right corner.
	gwin->get_shape_location(this, x, y);
	int w = xtiles*c_tilesize, h = ytiles*c_tilesize;
	Rectangle box(x - w, y - h, w, h);
	box.enlarge(10);		// Make it a bit bigger.
	if (dir%2)			// Horizontal?  Stretch.
		{ box.x -= 18; box.w += 36; }
	else
		{ box.y -= 18; box.h += 36; }
	box = gwin->clip_to_win(box);	// Intersect with screen.
	gwin->add_dirty(box);
	}

/*
 *	Finish up moving all the objects by adding them back and deleting the
 *	saved list of positions.
 */

void Barge_object::finish_move
	(
	Tile_coord *positions,		// New positions.  Deleted when done.
	int newmap			// Map #, or -1 for current.
	)
	{
	set_center();			// Update center.
	int cnt = objects.size();	// We'll move each object.
	for (int i = 0; i < cnt; i++)	// Now add them back in new location.
		{
		Game_object *obj = get_object(i);
		if (i < perm_count)	// Restore us as owner.
			obj->set_owner(this);
		obj->move(positions[i], newmap);
		}
	delete [] positions;
					// Check for scrolling.
	gwin->scroll_if_needed(center);
	}

/*
 *	Turn to face a given direction.
 */

void Barge_object::face_direction
	(
	int ndir			// 0-7 0==North.
	)
	{
	ndir /= 2;			// Convert to 0-3.
	switch ((4 + ndir - dir)%4)
		{
	case 1:				// Rotate 90 degrees right.
		turn_right();
		break;
	case 2:
		turn_around();		// 180 degrees.
		break;
	case 3:
		turn_left();
		break;
	default:
		break;
		}
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
	if (path->NewPath(get_tile(), dest, 0))
		{
		frame_time = speed;
					// Figure new direction.
		Tile_coord cur = get_tile();
		int dy = Tile_coord::delta(cur.ty, dest.ty),
		    dx = Tile_coord::delta(cur.tx, dest.tx);
		int ndir = Get_direction4(-dy, dx);
		if (!ice_raft)		// Ice-raft doesn't rotate.
			face_direction(ndir);
		if (!in_queue())	// Not already in queue?
			gwin->get_tqueue()->add(Game::get_ticks(), this, 0L);
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
	add_dirty();			// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate90r(gwin, this, xtiles, ytiles, center);
	if (!okay_to_rotate(rot))	// Check for blockage.
		return;
	Container_game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 1)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = obj->get_info();
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
	add_dirty();			// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate90l(gwin, this, xtiles, ytiles, center);
	if (!okay_to_rotate(rot))	// Check for blockage.
		return;
	Container_game_object::move(rot.tx, rot.ty, rot.tz);
	swap_dims();			// Exchange xtiles, ytiles.
	dir = (dir + 3)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = obj->get_info();
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
	add_dirty();			// Want to repaint old position.
					// Move the barge itself.
	Tile_coord rot = Rotate180(gwin, this, xtiles, ytiles, center);
	Container_game_object::move(rot.tx, rot.ty, rot.tz);
	dir = (dir + 2)%4;		// Increment direction.
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		int frame = obj->get_framenum();
		Shape_info& info = obj->get_info();
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
 *	Ending 'barge mode'.
 */

void Barge_object::done
	(
	)
	{
	gathered = false;		// Clear for next time.
	static int norecurse = 0;	// Don't recurse on the code below.
	if (norecurse > 0)
		return;
	norecurse++;
	if (boat == 1)			// Look for sails on boat.
		{			// Pretend they were clicked on.
		int cnt = objects.size();	// Look for open sail.
		for (int i = 0; i < cnt; i++)
			{
			Game_object *obj = objects[i];
			if (obj->get_shapenum() == 251 &&
			    (obj->get_framenum()&7) < 4)
				{
				obj->activate();
				break;
				}
			}
		}
	norecurse--;
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
					// Go through intersected chunks.
	Chunk_intersect_iterator next_chunk(foot);
	Rectangle tiles;
	int cx, cy;
	while (next_chunk.get_next(tiles, cx, cy))
		{			// Check each tile.
		Map_chunk *chunk = gmap->get_chunk(cx, cy);
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
	if (!path || !frame_time || gwin->get_moving_barge() != this)
		return;			// We shouldn't be doing anything.
	Tile_coord tile;		// Get spot & walk there.	
					// Take two steps for speed.
	if (!path->GetNextStep(tile) || !Barge_object::step(tile))
		frame_time = 0;
	else if (!first_step)		// But not when just starting.
		{
		taking_2nd_step = true;
		if (!path->GetNextStep(tile) || !Barge_object::step(tile))
			frame_time = 0;
		taking_2nd_step = false;
		}
	if (frame_time)			// Still good?
		gwin->get_tqueue()->add(curtime + frame_time, this, udata);
	first_step = false;		// After 1st, move 2 at a time.
	}

/*
 *	Move to a new absolute location.
 */

void Barge_object::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
	if (!chunk)			// Not currently on map?
		{	// UNTIL drag-n-drop does the gather properly.
		Container_game_object::move(newtx, newty, newlift, newmap);
		return;
		}
	if (!gathered)			// Happens in SI with turtle.
		gather();
					// Want to repaint old position.
	add_dirty();
					// Get current location.
	Tile_coord old = get_tile();
	if (newmap == -1) newmap = get_map_num();
					// Move the barge itself.
	Container_game_object::move(newtx, newty, newlift, newmap);
					// Get deltas.
	int dx = newtx - old.tx, dy = newty - old.ty, dz = newlift - old.tz;
	int cnt = objects.size();	// We'll move each object.
					// But 1st, remove & save new pos.
	Tile_coord *positions = new Tile_coord[cnt];
	int i;
	for (i = 0; i < cnt; i++)
		{
		Game_object *obj = get_object(i);
		Tile_coord ot = obj->get_tile();
					// Watch for world-wrapping.
		positions[i] = Tile_coord(
				(ot.tx + dx + c_num_tiles)%c_num_tiles,
				(ot.ty + dy + c_num_tiles)%c_num_tiles, 
				ot.tz + dz);
		obj->remove_this(1);	// Remove object from world.
		obj->set_invalid();	// So it gets added back right.
		if (!taking_2nd_step)
			{		// Animate a few shapes.
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
		}
	finish_move(positions, newmap);	// Add back & del. positions.
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

bool Barge_object::add
	(
	Game_object *obj,
	bool dont_check,
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
	{
	objects.push_back(obj);		// Add to list.
	return (false);			// We want it added to the chunk.
	}

/*
 *	Is a given object part of the barge (after a 'gather')?
 */

bool Barge_object::contains
	(
	Game_object *obj
	)
	{
	for (Game_object_vector::const_iterator it = objects.begin();
						it != objects.end(); ++it)
		if (obj == *it)
			return true;
	return false;
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
	)
	{
					// DON'T paint barge shape itself.
					// The objects are in the chunk too.
	if(gwin->paint_eggs)
		{
		Container_game_object::paint();
		int pix = sman->get_special_pixel(CURSED_PIXEL);
		int rx, by, lx, ty;	// Right, bottom, left, top.
		gwin->get_shape_location(this, rx, by);
		lx = rx - xtiles*c_tilesize + 1;
		ty = by - ytiles*c_tilesize + 1;
					// Little square at lower-right.
		gwin->get_win()->fill8(pix, 4, 4, rx-2, by-2);
					// Little square at top.
		gwin->get_win()->fill8(pix, 4, 4, lx-1, ty-1);
					// Horiz. line along top, bottom.
		gwin->get_win()->fill8(pix, xtiles*c_tilesize, 1, lx, ty);
		gwin->get_win()->fill8(pix, xtiles*c_tilesize, 1, lx, by);
					// Vert. line to left, right.
		gwin->get_win()->fill8(pix, 1, ytiles*c_tilesize, lx, ty);
		gwin->get_win()->fill8(pix, 1, ytiles*c_tilesize, rx, ty);
		}
	}

/*
 *	Edit in ExultStudio.
 */

void Barge_object::activate
	(
	int /* event */
	)
	{
	edit();
	}

/*
 *	Edit in ExultStudio.
 *
 *	Output:	True if map-editing & ES is present.
 */

bool Barge_object::edit
	(
	)
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_tile();
		unsigned long addr = (unsigned long) this;
		if (Barge_object_out(client_socket, addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), 
			xtiles, ytiles, dir) != -1)
			{
			cout << "Sent barge data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending barge data to ExultStudio" <<
									endl;
		return true;
		}
#endif
	return false;
	}


/*
 *	Message to update from ExultStudio.
 */

void Barge_object::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame;
	int xtiles, ytiles, dir;
	if (!Barge_object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		xtiles, ytiles, dir))
		{
		cout << "Error decoding barge" << endl;
		return;
		}
	Barge_object *barge = (Barge_object *) addr;
	if (barge && barge != editing)
		{
		cout << "Barge from ExultStudio is not being edited" << endl;
		return;
		}
	editing = 0;
	if (!barge)			// Create a new one?
		{
		int x, y;
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, 
							Exult_server::cancel);
			return;
			}
		if (shape == -1)
			shape = 961;	// FOR NOW.
					// Create.  Gets initialized below.
		barge = new Barge_object(shape, 0, 0, 0, 0, 0, 0, 0);
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(barge, x, y, lift) == 1)
				break;
		if (lift == 12)
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, 
							Exult_server::cancel);
			delete barge;
			return;
			}
		if (client_socket >= 0)
			Exult_server::Send_data(client_socket, 
						Exult_server::user_responded);
		}
	barge->xtiles = xtiles;
	barge->ytiles = ytiles;
	barge->dir = dir;
	if (shape != -1)
		barge->set_shape(shape);
	gwin->add_dirty(barge);
	cout << "Barge updated" << endl;
#endif
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
	int frame,				// Frame (ignored).
	bool force				// Forces the step to happen.
	)
	{
	if (!gathered)			// Happens in SI with turtle.
		gather();
	Tile_coord cur = get_tile();
					// Blocked? (Assume ht.=4, for now.)
	int move_type;
	if (cur.tz > 0)
		move_type = MOVE_LEVITATE;
	else if (force)
		move_type = MOVE_ALL;
	else if (boat) 
		move_type = MOVE_SWIM;
	else
		move_type = MOVE_WALK;
					// No rising/dropping.
	if (Map_chunk::is_blocked(get_xtiles(), get_ytiles(), 
						4, cur, t, move_type, 0, 0))
		return (0);		// Done.
	move(t.tx, t.ty, t.tz);		// Move it & its objects.
					// Near an egg?
	Map_chunk *nlist = gmap->get_chunk(get_cx(), get_cy());
	nlist->activate_eggs(gwin->get_main_actor(), t.tx, t.ty, t.tz, 
						cur.tx, cur.ty);
	return (1);			// Add back to queue for next time.
	}

/*
 *	Write out.
 */

void Barge_object::write_ireg
	(
	DataSource *out
	)
	{
	unsigned char buf[20];		// 13-byte entry + length-byte.
	unsigned char *ptr = write_common_ireg(12, buf);
					// Write size.
	*ptr++ = xtiles;
	*ptr++ = ytiles;
	*ptr++ = 0;			// Unknown.
					// Flags (quality).  Taking B3 to in-
					//   dicate barge mode.
	*ptr++ = (dir<<1) | ((gwin->get_moving_barge() == this)<<3);
	*ptr++ = 0;			// (Quantity).
	*ptr++ = (get_lift()&15)<<4;
	*ptr++ = 0;			// Data2.
	*ptr++ = 0;			// 
	out->write((char*)buf, ptr - buf);
					// Write permanent objects.
	for (int i = 0; i < perm_count; i++)
		{
		Game_object *obj = get_object(i);
		obj->write_ireg(out);
		}
	out->write1(0x01);			// A 01 terminates the list.
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
	}

// Get size of IREG. Returns -1 if can't write to buffer
int Barge_object::get_ireg_size()
{
	// These shouldn't ever happen, but you never know
	if (gwin->get_moving_barge() == this || Usecode_script::find(this))
		return -1;

	int total_size = 8 + get_common_ireg_size();

	for (int i = 0; i < perm_count; i++)
	{
		Game_object *obj = get_object(i);
		int size = obj->get_ireg_size();

		if (size < 0) return -1;

		total_size += size;
	}

	total_size += 1;

	return total_size;
}

/*
 *	This is called after all elements have been read in and added.
 */

void Barge_object::elements_read
	(
	)
	{
	perm_count = 0;			// So we don't get haystack!
	complete = true;
	}

