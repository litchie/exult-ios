/*
 *	objs.h - Game objects.
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef OBJS_H
#define OBJS_H

#include <string>	// STL string
#include "exult_constants.h"
#include "exult_types.h"
#include "flags.h"
#include "rect.h"
#include "shapeid.h"
#include "tqueue.h"
#include "tiles.h"
#include "vec.h"

#include "objlist.h"

class Actor;
class Map_chunk;
class Container_game_object;
class Terrain_game_object;
class Egg_object;
class Barge_object;
class Game_window;
class Npc_actor;
class PathFinder;
class Rectangle;
class Schedule;
class Usecode_machine;
class Vga_file;
class DataSource;

template<class T>
class T_Object_list;

/*
 *	A game object is a shape from shapes.vga along with info. about its
 *	position within its chunk.
 */
class Game_object : public ShapeID
	{
protected:
	static Game_object *editing;	// Obj. being edited by ExultStudio.
	Map_chunk *chunk;		// Chunk we're in, or NULL.
	unsigned char tx, ty;		// (X,Y) of shape within chunk, or if
					//   in a container, coords. within
					//   gump's rectangle.

	unsigned char lift;		// Raise by 4* this number.
	short quality;			// Some sort of game attribute.
	int get_cxi() const;
	int get_cyi() const;
private:
	Game_object *next, *prev;	// ->next in chunk list or container.
	Game_object_vector dependencies;// Objects which must be painted before
					//   this can be rendered.
	Game_object_vector dependors;	// Objects which must be painted after.
	static unsigned char rotate[8];	// For getting rotated frame #.
public:
	uint32 render_seq;		// Render sequence #.
protected:
					// Handle attack on an object.
	int attack_object(Actor *attacker, int weapon_shape, int ammo_shape);
					// Create from ifix record.
	Game_object(unsigned char *ifix)
			: ShapeID(ifix[2], ifix[3]), 
			  tx((ifix[0]>>4)&0xf), ty(ifix[0]&0xf),
			  lift(ifix[1] & 0xf), quality(0), chunk(0)
		{  }
public:
	friend class T_Object_list<Game_object *>;
	friend class T_Object_iterator<Game_object *>;
	friend class T_Flat_object_iterator<Game_object *,Map_chunk *>;
	friend class T_Object_iterator_backwards<Game_object *, 
							Map_chunk *>;
	friend class Map_chunk;
	Game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: ShapeID(shapenum, framenum), tx(tilex), ty(tiley),
			lift(lft), quality(0), chunk(0)
		{  }
					// Copy constructor.
	Game_object(const Game_object& obj2)
		: ShapeID(obj2), tx(obj2.tx), ty(obj2.ty), lift(obj2.lift),
		  quality(obj2.quality), chunk(obj2.chunk)
		{  }
	Game_object() : ShapeID()	// Create fake entry.
		{  }
	virtual ~Game_object()
		{  }
	int get_tx() const		// Get tile (0-15) within chunk.
		{ return tx; }
	int get_ty() const
		{ return ty; }
	int get_lift() const
		{ return lift; }
	Tile_coord get_tile() const;	// Get location in abs. tiles.
					// Get distance to another object.
	int distance(Game_object *o2) const
		{ return get_tile().distance(
					o2->get_tile()); }
					// Get direction to another object.
	int get_direction(Game_object *o2) const;
	int get_direction(Tile_coord t2) const;
	Map_chunk *get_chunk() const	// Get chunk this is in.
		{ return chunk; }
	int get_cx() const;
	int get_cy() const;
	int get_quality() const
		{ return quality; }
	void set_quality(int q)
		{ quality = q; }
	int get_quantity() const;	// Like # of coins.
	virtual int get_obj_hp() const;     // hitpoints for non-NPCs
	virtual void set_obj_hp(int hp);
	int get_volume() const;		// Get space taken.
					// Add/remove to/from quantity.
	int modify_quantity(int delta, bool *del = 0);
					// Set shape coord. in chunk/gump.
	void set_shape_pos(unsigned int shapex, unsigned int shapey)
		{ tx = shapex; ty = shapey; }
	void set_lift(int l)
		{ lift = l; }
	Game_object *get_next()
		{ return next; }
	Game_object *get_prev()
		{ return prev; }
					// Compare for render order.
	static int compare(class Ordering_info& inf1, Game_object *obj2);
	int lt(Game_object& obj2);	// Is this less than another in pos.?
	void set_invalid()		// Set to invalid position.
		{ chunk = 0; }
	bool is_pos_invalid() const
		{ return chunk == 0; }
	void set_chunk(Map_chunk *c)
		{ chunk = c; }
					// Get frame for desired direction.
	int get_dir_framenum(int dir, int frnum) const
		{ return (frnum&0xf) + rotate[dir]; }
					// Get it using current dir.
	int get_dir_framenum(int frnum) const
		{ return (frnum&0xf) + (get_framenum()&(16 | 32)); }
					// Get direction (NPC) is facing.
	int get_dir_facing() const;
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	void move(Tile_coord t, int newmap = -1)
		{ move(t.tx, t.ty, t.tz, newmap); }
	void change_frame(int frnum);	// Change frame & set to repaint.
					// Swap positions.
	int swap_positions(Game_object *obj2);
	int get_dependency_count()	// Get objs. to paint first.
		{ return dependencies.size(); }
	Game_object *get_dependency(int i)
		{ return dependencies[i]; }
	void clear_dependencies();	// Remove all dependencies.

					// Find nearby objects.
#define HDR_DECLARE_FIND_NEARBY(decl_type) \
	static int find_nearby(decl_type vec, Tile_coord pos, \
			int shapenum, int delta, int mask,  \
			int qual = c_any_qual, int framenum = c_any_framenum)

	HDR_DECLARE_FIND_NEARBY(Egg_vector&);
	HDR_DECLARE_FIND_NEARBY(Actor_vector&);
	HDR_DECLARE_FIND_NEARBY(Game_object_vector&);

#undef HDR_DECLARE_FIND_NEARBY

	int find_nearby_actors(Actor_vector& vec, int shapenum, int delta) 
									const;
	int find_nearby_eggs(Egg_vector& vec, int shapenum, int delta,
		int qual = c_any_qual, int frnum = c_any_framenum) const;
	int find_nearby(Game_object_vector& vec, int shapenum, int delta, 
		int mask, int qual = c_any_qual, 
					int framenum = c_any_framenum) const;

	Game_object *find_closest(Game_object_vector& vec,
				int *shapenums, int num_shapes, int dist = 24);
	Game_object *find_closest(int *shapenums, int num_shapes, 
							int dist = 24);
	Game_object *find_closest(int shapenum, int dist = 24)
		{ return find_closest(&shapenum, 1, dist); }
	Rectangle get_footprint();	// Get tile footprint.
	bool blocks(Tile_coord tile);	// Do we block a given tile?
					// Find object blocking given tile.
	static Game_object *find_blocking(Tile_coord tile);
	static Game_object *find_door(Tile_coord tile);
	int is_closed_door() const;	// Checking for a closed door.
	Game_object *get_outermost();	// Get top 'owner' of this object.
	void say(const char *text);		// Put text up by item.
	void say(int from, int to);	// Show random msg. from 'text.flx'.
					// Render.
	virtual void paint();
	void paint_outline(Pixel_colors pix);
					// Make this class abstract.
	virtual void paint_terrain() = 0;
					// Can this be clicked on?
	virtual int is_findable()
		{ return 1; }
					// Run usecode function.
	virtual void activate(int event = 1);
	virtual bool edit();		// Edit in ExultStudio.
					// Saved from ExultStudio.
	static void update_from_studio(unsigned char *data, int datalen);
	virtual std::string get_name() const;
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual Container_game_object *get_owner()
		{ return 0; }
	virtual void set_owner(Container_game_object *o)
		{  }
	static int get_weight(int shnum, int quant = 1);
	virtual int get_weight();
	virtual int get_max_weight();	// Get max. weight allowed.
	virtual int is_dragable() const;// Can this be dragged?
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Set/clear/get actor flag.
	virtual void set_flag(int flag) {  }
	virtual void set_siflag(int flag) { }
	virtual void clear_siflag(int flag) { }
	virtual void clear_flag(int flag) { }
	virtual int get_flag(int flag) const  { return 0; }
	virtual void set_flag_recursively(int flag) { }
	virtual int get_siflag(int flag) const { return 0; }
	virtual int get_type_flag(int flag) const { return 0; }

	virtual Actor *as_actor() { return 0; }
	virtual Npc_actor *as_npc() { return 0; }
	virtual Barge_object *as_barge() { return 0; }
	virtual Terrain_game_object *as_terrain() { return 0; }
	virtual Container_game_object *as_container() { return 0; }
	virtual Egg_object *as_egg() { return 0; }
	virtual int is_egg() const	// An egg?
		{ return 0; }
					// Count contained objs.
	virtual int count_objects(int shapenum, int qual = c_any_qual,
							int framenum = c_any_framenum)
		{ return 0; }
					// Get contained objs.
	virtual int get_objects(Game_object_vector& vec, int shapenum, int qual,
						int framenum)
		{ return 0; }
					// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
							bool combine = false);
					// Add to NPC 'ready' spot.
	virtual int add_readied(Game_object *obj, int index, int dont_check = 0, int force_pos = 0)
		{ return add(obj, dont_check!=0); }
	virtual int add_quantity(int delta, int shapenum, int qual = c_any_qual,
				int framenum = c_any_framenum, int dontcreate = 0)
		{ return delta; }
	virtual int create_quantity(int delta, int shapenum, int qual,
						int framenum, bool temporary = false)
		{ return delta; }
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum)
		{ return delta; }
	virtual Game_object *find_item(int shapenum, int qual, int framenum)
		{ return 0; }
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_tile(); }
					// Move out of the way.
	virtual int move_aside(Actor *for_actor, int dir)
		{ return 0; }		// For now.
					// Get frame if rotated clockwise.
	virtual int get_rotated_frame(int quads);
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame)
		{ return 0; }
	virtual int is_monster()
		{ return 0; }
					// Under attack.
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
					int ammo_shape = 0);
					// Write out to IREG file.
	virtual void write_ireg(DataSource* out)
		{  }
				// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size()
		{ return 0; }
					// Write out IFIX, CHUNKS.
	virtual void write_ifix(DataSource* ifix)
		{  }
	virtual void elements_read()	// Called when all member items read.
		{  }
					// Write common IREG data.
	void write_common_ireg(unsigned char *buf);
	virtual int get_live_npc_num()
		{ return -1; }

	virtual void delete_contents() { }

	};

/*
 *	Object from U7chunks.
 */
class Terrain_game_object : public Game_object
	{
public:
	Terrain_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft)
		{  }
	virtual Terrain_game_object *as_terrain() { return this; }
	virtual void paint_terrain();
	};

/*
 *	Object from an IFIXxx file.
 */
class Ifix_game_object : public Game_object
	{
public:
					// Create from ifix record.
	Ifix_game_object(unsigned char *ifix) : Game_object(ifix)
		{  }
	Ifix_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft)
		{  }
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual void paint_terrain() {  }
	virtual void write_ifix(DataSource* ifix);
	};

#endif
