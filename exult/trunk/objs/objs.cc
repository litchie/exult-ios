/*
 *	objs.cc - Game objects.
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

#include "objs.h"
#include "chunks.h"
#include "objiter.h"
#include "egg.h"
#include "gamewin.h"
#include "gamemap.h"
#include "actors.h"
#include "ucmachine.h"
#include "items.h"
#include "dir.h"
#include "ordinfo.h"
#include "game.h"
#include "Gump_manager.h"
#include "effects.h"
#include "databuf.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#  include <cstdio>
#endif
#include <algorithm>       // STL function things

#ifdef USE_EXULTSTUDIO
#include "cheat.h"
#include "server.h"
#include "objserial.h"
#include "servemsg.h"
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::memset;
using std::rand;
using std::ostream;
using std::snprintf;
using std::strchr;
using std::string;

					// Offset to each neighbor, dir=0-7.
short Tile_coord::neighbors[16] = {0,-1, 1,-1, 1,0, 1,1, 0,1,
							-1,1, -1,0, -1,-1 };
Game_object *Game_object::editing = 0;
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
	Tile_coord t1 = get_tile();
	Tile_coord t2 = o2->get_tile();
					// Treat as cartesian coords.
	return (int) Get_direction(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Get direction to a given tile.
 */

int Game_object::get_direction
	(
	Tile_coord t2
	) const
	{
	Tile_coord t1 = get_tile();
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
	Game_window *gwin = Game_window::get_instance();
	Shape_info& info = ShapeID::get_info(shnum);
	return info.has_quantity();
	}

static int Has_hitpoints(int shnum)
{
	Game_window *gwin = Game_window::get_instance();
	Shape_info& info = ShapeID::get_info(shnum);
	return ((info.get_shape_class() == Shape_info::has_hp) ||
			(info.get_shape_class() == Shape_info::container));

	// containers have hitpoints too ('resistance')
}

const int MAX_QUANTITY = 100;		// Highest quantity possible.

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

int Game_object::get_obj_hp() const
{
	int shnum = get_shapenum();
	if (Has_hitpoints(shnum))
		return quality;
	else
		return 0;
}

void Game_object::set_obj_hp(int hp)
{
	int shnum = get_shapenum();
	if (Has_hitpoints(shnum))
		set_quality(hp);
}

/*
 *	Get the volume.
 */

int Game_object::get_volume
	(
	) const
	{
	int vol = get_info().get_volume();
	return vol;			// I think U7 ignores quantity!
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
	int delta,			// >=0 to add, <0 to remove.
	bool *del			// If !null, true ret'd if deleted.
	)
	{
	if (del)
		*del = false;
	if (!Has_quantity(get_shapenum()))
		{			// Can't do quantity here.
		if (delta > 0)
			return (delta);
		remove_this();		// Remove from container (or world).
		if (del)
			*del = true;
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
		if (del)
			*del = true;
		return (delta + quant);
		}
	int oldvol = get_volume();	// Get old volume used.
	quality = (char) newquant;	// Store new value.
	int shapenum = get_shapenum();
					// Set appropriate shape.
	int num_frames = get_num_frames();
	int new_frame = newquant - 1;
	if (new_frame > 7)		// Range is 0-7.
		new_frame = 7;
	if (shapenum == 565)		// Starbursts are special.
		set_frame(0);		// (Fixes messed-up games.)
	else if (shapenum != 842)	// Leave reagants alone.
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
	Game_window *gwin = Game_window::get_instance();
	Game_map *gmap = gwin->get_map();
					// Figure new chunk.
	int newcx = newtx/c_tiles_per_chunk, newcy = newty/c_tiles_per_chunk;
	Map_chunk *newchunk = gmap->get_chunk_safely(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
					// Remove from old.
	Map_chunk *oldchunk = gmap->get_chunk_safely(cx, cy);
	if (oldchunk)
		{
		gwin->add_dirty(this);	// Want to repaint old area.
		oldchunk->remove(this);
		}
	set_lift(newlift);		// Set new values.
	shape_pos = ((newtx%c_tiles_per_chunk) << 4) + newty%c_tiles_per_chunk;
	newchunk->add(this);		// Updates cx, cy.
	gwin->add_dirty(this);		// And repaint new area.
	}

/*
 *	Change the frame and set to repaint areas.
 */

void Game_object::change_frame
	(
	int frnum
	)
	{
	Game_window *gwin = Game_window::get_instance();
	gwin->add_dirty(this);		// Set to repaint old area.
	set_frame(frnum);
	gwin->add_dirty(this);		// Set to repaint new.
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
	Game_window *gwin = Game_window::get_instance();
	Shape_info& inf1 = get_info();
	Shape_info& inf2 = obj2->get_info();
	if (inf1.get_3d_xtiles() != inf2.get_3d_xtiles() ||
	    inf1.get_3d_ytiles() != inf2.get_3d_ytiles())
		return 0;		// Not the same size.
	Tile_coord p1 = get_tile();
	Tile_coord p2 = obj2->get_tile();
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
	Shape_info& info = obj->get_info();
	if ((mask&(4|8)) &&		// Both seem to be all NPC's.
	    !info.is_npc())
		return 0;
	Shape_info::Shape_class sclass = info.get_shape_class();
					// Egg/barge?
	if ((sclass == Shape_info::hatchable || sclass == Shape_info::barge) &&
	    !(mask&0x10))		// Only accept if bit 16 set.
		return 0;
	if (info.is_transparent() &&	// Transparent?
	    !(mask&0x80))
		return 0;
					// Invisible object?
	if (obj->get_flag(Obj_flags::invisible))
		if (!(mask&20))	// Guess:  0x20 == invisible.
			{
			if (!(mask&0x40))	// Guess:  Inv. party member.
				return 0;
			if (!obj->get_flag(Obj_flags::in_party))
				return 0;
			}
	return 1;			// Passed all tests.
}

/*
 *	Find objects near a given position.
 *
 *	Output:	# found, appended to vec.
 */


#ifndef MSVC_FIND_NEARBY_KLUDGE

template <class T>
#ifdef ALPHA_LINUX_CXX
int Game_object::find_nearby_static
#else
int Game_object::find_nearby
#endif
	(
	Exult_vector<T*>& vec,	// Objects appended to this.
	Tile_coord pos,			// Look near this point.
	int shapenum,			// Shape to look for.  
					//   -1=any (but always use mask?),
					//   c_any_shapenum=any.
	int delta,			// # tiles to look in each direction.
	int mask,			// See Check_mask() above.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame #, or c_any_framenum for any.
	)
	{
	if (delta < 0)			// +++++Until we check all old callers.
		delta = 24;
	if (shapenum > 0 && mask == 4)	// Ignore mask=4 if shape given!
		mask = 0;
	int vecsize = vec.size();
	Game_window *gwin = Game_window::get_instance();
	Game_map *gmap = gwin->get_map();
	Rectangle tiles(pos.tx - delta, pos.ty - delta, 1 + 2*delta, 1 + 
								2*delta);
					// Stay within world.
	Rectangle world(0, 0, c_num_chunks*c_tiles_per_chunk, 
					c_num_chunks*c_tiles_per_chunk);
	tiles = tiles.intersect(world);
					// Figure range of chunks.
	int start_cx = tiles.x/c_tiles_per_chunk,
	    end_cx = (tiles.x + tiles.w - 1)/c_tiles_per_chunk;
	int start_cy = tiles.y/c_tiles_per_chunk,
	    end_cy = (tiles.y + tiles.h - 1)/c_tiles_per_chunk;
					// Go through all covered chunks.
	for (int cy = start_cy; cy <= end_cy; cy++)
		for (int cx = start_cx; cx <= end_cx; cx++)
			{		// Go through objects.
			Map_chunk *chunk = gmap->get_chunk(cx, cy);
			Object_iterator next(chunk->get_objects());
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				{	// Check shape.
				if (shapenum >= 0)
					{
					if (obj->get_shapenum() != shapenum)
						continue;
					}
				if (qual != c_any_qual && obj->get_quality() 
								!= qual)
					continue;
				if (framenum !=  c_any_framenum &&
					obj->get_framenum() != framenum)
					continue;
				if (!Check_mask(gwin, obj, mask))
					continue;
				Tile_coord t = obj->get_tile();
				if (tiles.has_point(t.tx, t.ty)) {
					T* castobj = dynamic_cast<T*>(obj);
					if (castobj)
						vec.push_back(castobj);
				}
				}
			}
					// Return # added.
	return (vec.size() - vecsize);
	}
 
#ifdef ALPHA_LINUX_CXX
#define DEFINE_FIND_NEARBY(decl_type, decl_conttype) \
int Game_object::find_nearby(decl_type vec, Tile_coord pos, int shapenum, int delta, int mask, int qual, int framenum) \
{  \
  return find_nearby_static(vec, pos, shapenum, delta, mask, qual, framenum); \
}

DEFINE_FIND_NEARBY(Egg_vector&);
DEFINE_FIND_NEARBY(Actor_vector&);
DEFINE_FIND_NEARBY(Game_object_vector&);
#endif //ALPHA_LINUX_CXX


#else //MSVC_FIND_NEARBY_KLUDGE

#define FN_VECTOR Egg_vector
#define FN_OBJECT Egg_object
#include "find_nearby.h"

#define FN_VECTOR Game_object_vector
#define FN_OBJECT Game_object
#include "find_nearby.h"

#define FN_VECTOR Actor_vector
#define FN_OBJECT Actor
#include "find_nearby.h"

#endif //MSVC_FIND_NEARBY_KLUDGE

int Game_object::find_nearby_eggs
	(
	Egg_vector& vec,
	int shapenum,
	int delta,
	int qual,
	int frnum
	) const
	{
	return Game_object::find_nearby (vec, get_tile(), shapenum,
					delta, 16, qual, frnum);
	}

int Game_object::find_nearby_actors
	(
	Actor_vector& vec,
	int shapenum,
	int delta
	) const
	{
	return Game_object::find_nearby(vec, get_tile(), shapenum,
						delta, 8, c_any_qual, c_any_framenum);
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
	return Game_object::find_nearby(vec, get_tile(), shapenum,
					delta, mask, qual, framenum);
	}

/*
 *	For sorting closest to a given spot.
 */
class Object_closest_sorter
	{
	Tile_coord pos;			// Pos. to get closest to.
public:
	Object_closest_sorter(Tile_coord p) : pos(p)
		{  }
	bool operator()(const Game_object *o1, const Game_object *o2)
		{
		Tile_coord t1 = o1->get_tile(),
			   t2 = o2->get_tile();
		return t1.distance(pos) < t2.distance(pos);
		}
	};

/*
 *	Find the closest nearby objects with a shape in a given list.
 *
 *	Output:	->closest object, or 0 if none found.
 */

Game_object *Game_object::find_closest
	(
	Game_object_vector& vec,	// List returned here, closest 1st.
	int *shapenums,			// Shapes to look for. 
					//   c_any_shapenum=any NPC.
	int num_shapes,			// Size of shapenums.
	int dist			// Distance to look (tiles).
	)
	{
	int i;
	for (i = 0; i < num_shapes; i++)
					// 0xb0 mask finds anything.
		find_nearby(vec, shapenums[i], dist, 0xb0);
	int cnt = vec.size();
	if (!cnt)
		return (0);
	if (cnt > 1)
		std::sort(vec.begin(), vec.end(), 
				Object_closest_sorter(get_tile()));
	return *(vec.begin());
	}

/*
 *	Find the closest nearby object with a shape in a given list.
 *
 *	Output:	->object, or 0 if none found.
 */

Game_object *Game_object::find_closest
	(
	int *shapenums,			// Shapes to look for. 
					//   c_any_shapenum=any NPC.
	int num_shapes,			// Size of shapenums.
	int dist			// Distance to look (tiles).
	)
	{
	Game_object_vector vec;			// Gets objects found.
	int i;
	for (i = 0; i < num_shapes; i++)
					// 0xb0 mask finds anything.
		find_nearby(vec, shapenums[i], dist, 0xb0);
	int cnt = vec.size();
	if (!cnt)
		return (0);
	Game_object *closest = 0;	// Get closest.
	int best_dist = 10000;		// In tiles.
					// Get our location.
	Tile_coord loc = get_tile();
	for (Game_object_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		int dist = obj->get_tile().distance(loc);
		if (dist < best_dist)
			{
			closest = obj;
			best_dist = dist;
			}
		}
	return (closest);
	}

/*
 *	Get footprint in absolute tiles.
 */

Rectangle Game_object::get_footprint
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Shape_info& info = get_info();
					// Get footprint.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame);
	int ytiles = info.get_3d_ytiles(frame);
	Tile_coord t = get_tile();
	Rectangle foot((t.tx - xtiles + 1 + c_num_tiles)%c_num_tiles, 
		       (t.ty - ytiles + 1 + c_num_tiles)%c_num_tiles, 
							xtiles, ytiles);
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
	Game_window *gwin = Game_window::get_instance();
	Map_chunk *chunk = gwin->get_chunk(tile.tx/c_tiles_per_chunk,
						    tile.ty/c_tiles_per_chunk);
	Game_object *obj;
	Object_iterator next(chunk->get_objects());
	while ((obj = next.get_next()) != 0)
		{
					// Get object's coords.
		Tile_coord t = obj->get_tile();
		if (t.tx < tile.tx || t.ty < tile.ty || t.tz > tile.tz)
			continue;	// Out of range.
		Shape_info& info = obj->get_info();
		int ztiles = info.get_3d_height(); 
		if (!ztiles || !info.is_solid())
			continue;	// Skip if not an obstacle.
					// Occupies desired tile?
		int frame = obj->get_framenum();
		if (tile.tx > t.tx - info.get_3d_xtiles(frame) &&
		    tile.ty > t.ty - info.get_3d_ytiles(frame) &&
		    tile.tz < t.tz + ztiles)
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
	Game_window *gwin = Game_window::get_instance();
	Shape_info& info = get_info();
	if (!info.is_door())
		return 0;
					// Get door's footprint.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
					// Get its location.
	Tile_coord doortile = get_tile();
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
	return (Map_chunk::is_blocked(before) &&
	    	Map_chunk::is_blocked(after));
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
	Game_window *gwin = Game_window::get_instance();
	gwin->add_text(text, this);
	}

/*
 *	Show a random string from 'text.flx' by the object.
 */

void Game_object::say
	(
	int from, int to		// Range (inclusive).
	)
{
	if (from > to) return;
	int offset = rand()%(to - from + 1);
	if (from + offset < num_item_names)
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
	int x, y;
	gwin->get_shape_location(this, x, y);
	gwin->paint_shape(x, y, *this);
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
	if (edit())
		return;			// Map-editing.
	int usefun = get_shapenum();
					// Serpent Isle spell scrolls:
	if (usefun == 0x2cb && Game::get_game_type() == SERPENT_ISLE)
		{
		Game_window *gwin = Game_window::get_instance();
		gwin->get_gump_man()->add_gump(this, 65);
		return;
		}
					// !!!Special case:  books
	if (usefun == 0x282 && get_quality() >= 100 && get_quality() < 180)
		usefun = 0x638;
	else if (usefun == 0x282 && get_quality() >= 180 && 
			 Game::get_game_type() == SERPENT_ISLE )
		usefun = 0x63b;
	else if (usefun == 0x2c1 && get_quality() >= 213 &&
			 Game::get_game_type() == SERPENT_ISLE )
		usefun = 0x62a;
	umachine->call_usecode(usefun, this,
			(Usecode_machine::Usecode_events) event);
	}

/*
 *	Edit in ExultStudio.
 */

bool Game_object::edit
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
		std::string name = get_name();
		if (Object_out(client_socket, Exult_server::obj, 
			addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), get_quality(),
								name) != -1)
			{
			cout << "Sent object data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending object to ExultStudio" <<endl;
		return true;
		}
#endif
	return false;
	}

/*
 *	Message to update from ExultStudio.
 */

void Game_object::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, quality;
	std::string name;
	if (!Object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		quality, name))
		{
		cout << "Error decoding object" << endl;
		return;
		}
	Game_object *obj = (Game_object *) addr;
	if (!editing || obj != editing)
		{
		cout << "Obj from ExultStudio is not being edited" << endl;
		return;
		}
//	editing = 0;	// He may have chosen 'Apply', so still editing.
	Game_window *gwin = Game_window::get_instance();
	gwin->add_dirty(obj);
	obj->set_shape(shape, frame);
	gwin->add_dirty(obj);
	obj->set_quality(quality);
					// See if it moved.
	Tile_coord oldt = obj->get_tile();
	if (oldt.tx != tx || oldt.ty != ty || oldt.tz != tz)
		obj->move(tx, ty, tz);
	cout << "Object updated" << endl;
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

	snprintf(buf, 20, "%d ", quantity);	// Output the quantity
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
	const char *name = 0;
	int quantity;
	string display_name;
	int shnum = get_shapenum();
	int frnum = get_framenum();
	int qual = get_quality();
	if (Game::get_game_type() == BLACK_GATE) {
		//TODO: yourself 
		switch (shnum)			// Some special cases!
		{
		case 0x34a:			// Reagents
			name = item_names[0x500 + frnum];
			break;
		case 0x3bb:			// Amulets
			if (frnum < 3)
				name = item_names[0x508 + frnum];
			else
				name = item_names[shnum];
			break;
		case 0x179:			// Food items
			name = item_names[0x50b + frnum];
			break;
		case 0x28a:			// Sextants
			name = item_names[0x52c - frnum];
			break;
		case 0x2a3:			// Desk items
			name = item_names[0x52d + frnum];
			break;
		default:
			name = item_names[shnum];
			break;
		}


	} else if (Game::get_game_type() == SERPENT_ISLE) {
		//TODO: yourself, oilskin, broken glass, throne
		//TODO: Dave, >= text.flx nr. 1540
		
		switch (shnum)			// More special cases!
		{
		case 0x34a:			// Reagents
			name = item_names[0x500 + frnum];
			break;
		case 0x32a:			// Bucket
			if (frnum == 1 && qual >= 10 && qual <= 15)
				name = item_names[0x55b + qual];
			else if (frnum == 2 && qual == 9)
				name = item_names[0x55b + qual];
			else
				name = item_names[0x55b + frnum];
			break;
		case 0x179:			// Food items
			name = item_names[0x510 + frnum];
			break;
		case 0x2a3:			// Desk items
			name = item_names[0x532 + frnum];
			break;
		case 0x28a:			// Sextants
			name = item_names[0x531 - frnum];
			break;
		case 0x1bd:			// Soul prisms
			name = item_names[0x56b + frnum];
			break;
		case 0x289:			// Artifacts
			name = item_names[0x573 + frnum];
			break;
		case 0x1d3:			// Magic plants
			name = item_names[0x581 + frnum/2];
			// not sure about 'catnip'
			break;
		case 0x1c2:			// Orbs
			name = item_names[0x585 + frnum];
			break;
		case 0x106:			// Blackrock Serpents
			if (frnum < 4)
				name = item_names[0x592 + frnum];
			else
				name = item_names[shnum];
			break;
		case 0x0b2:			// Cloth maps
			name = item_names[0x596 + frnum];
			break;
		case 0x24b:			// Boots
			name = item_names[0x59c + frnum];
			break;
		case 0x21e:			// Crested Helmets
			name = item_names[0x5a3 + frnum];
			break;
		case 0x3ec:			// Helmets
			name = item_names[0x5a6 + frnum];
			break;
		case 0x241:			// Nests
			if (frnum < 6)
				name = item_names[0x5ab + frnum/3];
			else
				name = item_names[shnum];
			break;
		case 0x3bb:			// Amulets
			name = item_names[0x5ae + frnum];
			break;
		case 0x128:			// Rings
			name = item_names[0x5b8 + frnum];
			break;
		case 0x377:			// More rings
			name = item_names[0x5bc + frnum];
			break;
		case 0x2b4:			// Lute
			if (frnum == 2)
				name = item_names[0x5be];
			else
				name = item_names[shnum];
			break;
		case 0x347:			// Hourglass
			if (frnum == 1)
				name = item_names[0x5bf];
			else
				name = item_names[shnum];
			break;
		case 0x0d1:			// Artifacts
			name = item_names[0x5c0 + frnum];
			break;
		case 0x0f4:			// Large skulls
			if (frnum < 2)
				name = item_names[0x5d8 + frnum];
			else
				name = item_names[shnum];
			break;
		case 0x3f3:			// Beds
			if (frnum >= 1 && frnum <= 4)
				name = item_names[0x5da + frnum - 1];
			else if (frnum == 5)
				name = item_names[0x5dd];
			else if (frnum == 6)
				name = item_names[0x5da];
			else
				name = item_names[shnum];
			break;
		case 0x390:         // Acid
			if (frnum == 24)
				name = item_names[0x5de];
			else
				name = item_names[shnum];
			break;
		case 0x31F:        // Body parts
			name = item_names[0x5df + frnum];
			break;
		case 0x258:        // Bottles
			switch (frnum) {
			case 1:
				name = item_names[0x5f2]; // wine decanter
				break;
			case 9:
				name = item_names[0x5f3]; // fawnish ale
				break;
			case 16:
				name = item_names[0x5f4]; // ice wine
				break;
			case 17:
				name = item_names[0x5f5]; // vintage wine
				break;
			case 18:
				name = item_names[0x5f6]; // wineskin
				break;
		    case 20:
				name = item_names[0x5f7]; // everlasting goblet
				break;
			default:
				name = item_names[shnum];
			}
			break;
		case 0x1df:      // claw + gwani amulet
			name = item_names[0x5f8 + frnum/2];
			break;
		case 0x11d:      // brush
			if (frnum == 6)
				name = item_names[0x5fa];
			else
				name = item_names[shnum];
			break;
#if 0
			// probably wrong
		case 0x222:      // broken dish
			switch(frnum) {
			case 4: case 6: case 7: case 8: case 9:
			case 11: case 12: case 14: case 15:
				name = item_names[0x5fb];  // broken glass
			default:
				name = item_names[shnum];
			}
			break;
#endif
#if 0
			// not sure
		case 0x124:      // seat
			if (frnum == 17 || frnum == 18 || frnum == 21 || frnum == 22)
				name = item_names[0x5fc];
			else
				name = item_names[shnum];
#endif
		case 0x320:      // chest
			if (frnum == 4 || frnum == 5)
				name = item_names[0x5fd];
			else if (frnum == 6 || frnum == 7)
				name = item_names[0x5fe];
			else
				name = item_names[shnum];
			break;
		case 0x0e3:      // cloak
			if (frnum <= 4)
				name = item_names[0x5ff + frnum];
			else
				name = item_names[shnum];
			break;
		default:
			name = item_names[shnum];
			break;
		}

	} else {
		name = item_names[shnum];
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

			snprintf(buf, 50, "%d %s", quantity, name);
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
	Map_chunk *chunk = 
			Game_window::get_instance()->get_chunk_safely(
								cx, cy);
	if (chunk)
		chunk->remove(this);
	if (!nodel)
		Game_window::get_instance()->delete_object(this);
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
 *	Static method to get shape's weight in 1/10 stones.  0 means infinite.
 */

int Game_object::get_weight
	(
	int shnum,			// Shape #,
	int quant			// Quantity.
	)
	{
	int wt = quant * ShapeID::get_info(shnum).get_weight();
					// Special case:  reagents, coins.
	if (shnum == 842 || shnum == 644 || 
	    (Game::get_game_type() == SERPENT_ISLE &&
					// Monetari/guilders/filari:
	     (shnum == 951 || shnum == 952 || shnum == 948)))
	{
		wt /= 10;
		if (wt <= 0) wt = 1;
	}

	if (Has_quantity(shnum))
		if (wt <= 0) wt = 1;

	return wt;
	}

/* 
 *	Get weight of object in 1/10 stones.
 */

int Game_object::get_weight
	(
	)
	{
	return get_weight(get_shapenum(), get_quantity());
	}

/*
 *	Get maximum weight in stones that can be held.
 *
 *	Output:	Max. allowed, or 0 if no limit (i.e., not carried by an NPC).
 */

int Game_object::get_max_weight
	(
	)
	{
					// Looking outwards for NPC.
	Container_game_object *own = get_owner();
	return own ? own->get_max_weight() : 0;
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
					// ++++Really should use 
					//   Get_combine_info in contain.cc
					// Reagents are a special case.
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

//#define DEBUGLT
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
 *	Compare ranges along a given dimension.
 */
inline void Compare_ranges
	(
	int from1, int to1,		// First object's range.
	int from2, int to2,		// Second object's range.
					// Returns:
	int& cmp,			// -1 if 1st < 2nd, 1 if 1st > 2nd,
					//   0 if equal.
	bool& overlap			// true returned if they overlap.
	)
	{
	if (to1 < from2)
		{
		overlap = false;
		cmp = -1;
		}
	else if (to2 < from1)
		{
		overlap = false;
		cmp = 1;
		}
	else				// X's overlap.
		{
		overlap = true;
		if (from1 < from2)
			cmp = -1;
		else if (from1 > from2)
			cmp = 1;
		else if (to1 - from1 < to2 - from2)
			cmp = 1;
		else if (to1 - from1 > to2 - from2)
			cmp = -1;
		else
			cmp = 0;
		}
	}

/*
 *	Compare two objects.
 *
 *	Output:	-1 if 1st < 2nd, 0 if dont_care, 1 if 1st > 2nd.
 */

int Game_object::compare
	(
	Ordering_info& inf1,		// Info. for object 1.
	Game_object *obj2
	)
	{
	Game_window *gwin = Game_window::get_instance();
					// See if there's no overlap.
	Rectangle r2 = gwin->get_shape_rect(obj2);
	if (!inf1.area.intersects(r2))
		return (0);		// No overlap on screen.
	Ordering_info inf2(Game_window::get_instance(), obj2, r2);
#ifdef DEBUGLT
	Debug_lt(inf1.tx, inf1.ty, inf2.tx, inf2.ty);
#endif
	int xcmp, ycmp, zcmp;		// Comparisons for a given dimension:
					//   -1 if o1<o2, 0 if o1==o2,
					//    1 if o1>o2.
	bool xover, yover, zover;	// True if dim's overlap.
	Compare_ranges(inf1.xleft, inf1.xright, inf2.xleft, inf2.xright,
							xcmp, xover);
	Compare_ranges(inf1.yfar, inf1.ynear, inf2.yfar, inf2.ynear,
							ycmp, yover);
	Compare_ranges(inf1.zbot, inf1.ztop, inf2.zbot, inf2.ztop,
							zcmp, zover);
	if (!xcmp && !ycmp && !zcmp)
					// Same space?
					// Paint biggest area sec. (Fixes 
					//   plaque at Penumbra's.)
		return (inf1.area.w < inf2.area.w  && 
			inf1.area.h < inf2.area.h) ? -1 : 
			(inf1.area.w > inf2.area.w &&
			inf1.area.h > inf2.area.h) ? 1 : 0;
//		return 0;		// Equal.
	if (xover & yover & zover)	// Complete overlap?
		if (!inf1.zs)		// Flat one is always drawn first.
			return !inf2.zs ? 0 : -1;
		else if (!inf2.zs)
			return 1;
	if (xcmp >= 0 && ycmp >= 0 && zcmp >= 0)
		return 1;		// GTE in all dimensions.
	if (xcmp <= 0 && ycmp <= 0 && zcmp <= 0)
		return -1;		// LTE in all dimensions.
	if (yover)			// Y's overlap.
		{
		if (xover)		// X's too?
			return zcmp;
		else if (zover)		// Y's and Z's?
			return xcmp;
					// Just Y's overlap.
		else if (!zcmp)		// Z's equal?
			return xcmp;
		else			// See if X and Z dirs. agree.
			if (xcmp == zcmp)
				return xcmp;
#if 1 /* Woohoo!  Seems to work without messing up N. Trinsic gate. */
					// Experiment:  Fixes Trinsic mayor
					//   statue-through-roof.
		else if (inf1.ztop/5 < inf2.zbot/5 && inf2.info.occludes())
			return -1;	// A floor above/below.
		else if (inf2.ztop/5 < inf1.zbot/5 && inf1.info.occludes())
			return 1;
#endif
		else
			return 0;
		}
	else if (xover)			// X's overlap.
		{
		if (zover)		// X's and Z's?
			return ycmp;
		else if (!zcmp)		// Z's equal?
			return ycmp;
		else
			return ycmp == zcmp ? ycmp : 0;
		}
					// Neither X nor Y overlap.
	else if (xcmp == -1)		// o1 X before o2 X?
		{
		if (ycmp == -1)		// o1 Y before o2 Y?
					// If Z agrees or overlaps, it's LT.
			return (zover || zcmp <= 0) ? -1 : 0;
		}
	else if (ycmp == 1)		// o1 Y after o2 Y?
		if (zover || zcmp >= 0)
			return 1;
#if 1	/* So far, this seems to work without causing problems: */
					// Experiment:  Fixes Brit. museum
					//   statue-through-roof.
		else if (inf1.ztop/5 < inf2.zbot/5)
			return -1;	// A floor above.
		else
#endif
			return 0;
	return 0;
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
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Ordering_info ord(gwin, this);
	int cmp = compare(ord, &obj2);
	return cmp == -1 ? 1 : cmp == 1 ? 0 : -1;
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
	Game_window *gwin = Game_window::get_instance();
	int curframe = get_framenum();
	int shapenum = get_shapenum();
	Shape_info& info = get_info();
	if (shapenum == 292)		// Seat is a special case.
		{
		int dir = curframe%4;	// Current dir (0-3).
		return (curframe - dir) + (dir + quads)%4;
		}
	else if (info.is_barge_part())	// Piece of a barge?
		switch (quads)
			{
		case 1:
			return (curframe^32)^((curframe&32) ? 3 : 1);
		case 2:
			return curframe^2;
		case 3:
			return (curframe^32)^((curframe&32) ? 1 : 3);
		default:
			return curframe;
			}
	else
					// Reflect.  Bit 32==horizontal.
		return curframe ^ ((quads%2)<<5);
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
		winf = ShapeID::get_info(weapon_shape).get_weapon_info();
	else if (ammo_shape > 0)	// Not sure about all this...
		winf = ShapeID::get_info(ammo_shape).get_weapon_info();
	else
		winf = attacker->get_weapon(wpoints);
	int usefun;			// Run usecode if present.
	if (winf && (usefun = winf->get_usecode()) != 0)
		gwin->get_usecode()->call_usecode(usefun, this,
					Usecode_machine::weapon);
	if (!wpoints && winf)
		wpoints = winf->get_damage();
	if (attacker)
		wpoints += attacker->get_level() +
			attacker->get_property((int) Actor::strength);
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
	Game_window *gwin = Game_window::get_instance();
	int wpoints = attack_object(gwin,
					attacker, weapon_shape, ammo_shape);
	int shnum = get_shapenum();
	int frnum = get_framenum();

	int hp = get_obj_hp();		// Returns 0 if doesn't have HP's or is
					//   indestructible,
	if (!hp) {			//   with exceptions:
		// some special cases
		// guessing these don't have hitpoints by default because
		// doors need their 'quality' field for something else

		if (shnum == 432 || shnum == 433) // doors
			{
			if (get_quality()==0 || weapon_shape==704)
				// only 'normal' doors (or powderkeg)
				if (frnum != 3 && frnum < 7) // no magic-locked or steel doors
					hp = 6;
			}
		else if (shnum == 270 || shnum == 376) // more doors
			{
			if (get_quality()==0 || weapon_shape==704)
				// only 'normal' doors (or powderkeg)
				if (frnum < 3 || (frnum >= 8 && frnum <= 10) ||
					(frnum >= 16 && frnum <= 18)) // no magic or steel doors
					hp = 6;
			}
					// Serpent statue at end of SI:
		else if (shnum == 743 && Game::get_game_type() == SERPENT_ISLE)
			hp = 1;
		else if (shnum == 704 && weapon_shape == 704) { // Powder keg...
			// cause chain reaction

			// marked already detonating powderkegs with quality
			if (get_quality()==0) {
				Tile_coord pos = get_tile();
				gwin->add_effect(new Explosion_effect(pos, 
									this));
			}
		}
					// Arrow hitting practice targt?
		else if (shnum == 735 && ammo_shape == 722) {
			int newframe = !frnum ? (3*(rand()%8) + 1)
					: ((frnum%3) != 0 ? frnum + 1 : frnum);
			change_frame(newframe);
		}
#if 0
		else if (shnum == 522 && frnum < 2) { // locked normal chest
			if (get_quality() == 0 || get_quality() == 255)
				hp = 6;
		}
#endif

	}

	string name = "<trap>";
	if (attacker)
		name = attacker->get_name();


	if (hp == 0) { // indestructible
		cout << name << " attacks " << get_name() <<
			". No effect." << endl;
		return this;
	}

	cout << name << " hits " << get_name() <<
		" for " << wpoints << " hit points, leaving " <<
		 hp - wpoints << " remaining" << endl;

	if (wpoints >= hp) {
		// object destroyed
		gwin->remove_text_effect(this);	// Avoids crash.
		gwin->get_usecode()->call_usecode(0x626, this,Usecode_machine::weapon);

		return 0;
	} else {
		set_obj_hp(hp - wpoints);

		return this;
	}
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
 *	Paint terrain objects only.
 */

void Terrain_game_object::paint_terrain
	(
	Game_window *gwin
	)
	{
	paint(gwin);
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (cx=cy=255).
 */

void Ifix_game_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	Game_object::move(newtx, newty, newlift);
					// Mark superchunk as 'modified'.
	int cx = get_cx(), cy = get_cy();
	if (cx >= 0 && cx < c_num_chunks &&
	    cy >= 0 && cy < c_num_chunks)
		Game_window::get_instance()->get_map()->
					set_ifix_modified(cx, cy);
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Ifix_game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
					// Mark superchunk as 'modified'.
	int cx = get_cx(), cy = get_cy();
	if (cx >= 0 && cx < c_num_chunks &&
	    cy >= 0 && cy < c_num_chunks)
		Game_window::get_instance()->get_map()->
					set_ifix_modified(cx, cy);
	Game_object::remove_this(nodel);
	}

/*
 *	Write out an IFIX object.
 */

void Ifix_game_object::write_ifix
	(
	DataSource *ifix			// Where to write.
	)
	{
	unsigned char buf[4];
	buf[0] = shape_pos;
	buf[1] = lift;
	int shapenum = get_shapenum(), framenum = get_framenum();
	buf[2] = shapenum&0xff;
	buf[3] = ((shapenum>>8)&3) | (framenum<<2);
	ifix->write((char*)buf, sizeof(buf));
	}





