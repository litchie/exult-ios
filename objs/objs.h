/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.h - Game objects.
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

#ifndef INCL_OBJS
#define INCL_OBJS	1

#ifdef WIN32
#define Rectangle RECTX
#endif

#ifdef MACOS
#  include "exult_types.h"
#else
#  include "../exult_types.h"
#endif
#include <string>	// STL string
#include "shapeid.h"
#include "rect.h"
#include "vec.h"
#include "tqueue.h"
#include "tiles.h"
#include "exult_constants.h"

class Usecode_machine;
class Vga_file;
class Game_window;
class Npc_actor;
class Rectangle;
class Container_game_object;
class Egg_object;
class PathFinder;
class Actor;
class Schedule;

#define MOVE_NODROP (1<<3)
#define MOVE_FLY (1<<4)
#define MOVE_LEVITATE (MOVE_FLY|MOVE_NODROP)
#define	MOVE_WALK (1<<5)
#define MOVE_SWIM (1<<6)
#define	MOVE_ALL_TERRAIN ((1<<5)|(1<<6))
#define MOVE_ETHEREAL (1<<7)
#define MOVE_ALL (MOVE_FLY|MOVE_WALK|MOVE_SWIM|MOVE_ETHEREAL)


/*
 *	A game object is a shape from shapes.vga along with info. about its
 *	position within its chunk.
 */
class Game_object : public ShapeID
	{
	unsigned char shape_pos;	// (X,Y) of shape within chunk.
	unsigned char lift;		// Raise by 4* this number.
	short quality;			// Some sort of game attribute.
	Game_object *next, *prev;	// ->next in chunk list or container.
	Game_object_vector dependencies;		// Objects which must be painted before
					//   this can be rendered.
	Game_object_vector dependors;		// Objects which must be painted after.
	static unsigned char rotate[8];	// For getting rotated frame #.
public:
	uint32 render_seq;	// Render sequence #.
protected:
	unsigned char cx, cy;		// (Absolute) chunk coords., or if this
					//   is in a container, coords. within
					//   gump's rectangle.
					// Handle attack on an object.
	int attack_object(Game_window *gwin, Actor *attacker, int weapon_shape,
							int ammo_shape);
public:
	friend class Object_list;
	friend class Object_iterator;
	friend class Object_iterator_backwards;
	friend class Chunk_object_list;
					// Create from ifix record.
	Game_object(unsigned char *ifix)
			: ShapeID(ifix[2], ifix[3]), shape_pos(ifix[0]),
			  lift(ifix[1] & 0xf), quality(0), cx(255), cy(255)
		{  }
	Game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: ShapeID(shapenum, framenum),
				shape_pos((tilex << 4) + tiley),
			lift(lft), quality(0), cx(255), cy(255)
		{  }
					// Copy constructor.
	Game_object(const Game_object& obj2)
		: ShapeID(obj2), shape_pos(obj2.shape_pos), lift(obj2.lift),
		  quality(obj2.quality), cx(obj2.cx), cy(obj2.cy)
		{  }
	Game_object() : ShapeID()	// Create fake entry.
		{  }
	virtual ~Game_object()
		{  }
	enum Item_flags {		// Bit #'s of flags:
		invisible = 0,
		asleep = 1,
		charmed = 2,
		cursed = 3,
		paralyzed = 7,
		poisoned = 8,
		protection = 9,
		on_moving_barge = 10,	// ??Guessing.
		okay_to_take = 11,	// Okay to take??
		tremor = 12,		// ??Earthquake??
		dancing = 15,		// ??Not sure.
		dont_render = 16,	// Completely invisible.
		okay_to_move = 18,	// ??Guess: for Usecode-created items.
		okay_to_land = 21,	// Used for flying-carpet.
		confused = 25,		// ??Guessing.
		in_motion = 26,		// ??Guessing (cart, boat)??
		met = 28,			// Has the npc been met
		// Flags > 31
		petra = 35			// Guess
		};
	int get_tx() const		// Get tile (0-15) within chunk.
		{ return (shape_pos >> 4) & 0xf; }
	int get_ty() const
		{ return shape_pos & 0xf; }
	int get_lift() const
		{ return lift; }
	int get_worldx() const		// Get x-coord. within world.
		{ return cx*chunksize + get_tx()*tilesize; }
	int get_worldy() const		// Get y-coord. within world.
		{ return cy*chunksize + get_ty()*tilesize; }
					// Get location in abs. tiles.
	void get_abs_tile(int& atx, int& aty, int& atz) const
		{
		atz = get_lift();
		atx = cx*tiles_per_chunk + get_tx();
		aty = cy*tiles_per_chunk + get_ty();
		}
					// Same thing.
	Tile_coord get_abs_tile_coord() const
		{
		int x, y, z;
		get_abs_tile(x, y, z);
		return Tile_coord(x, y, z);
		}
					// Get distance to another object.
	int distance(Game_object *o2) const
		{ return get_abs_tile_coord().distance(
					o2->get_abs_tile_coord()); }
					// Get direction to another object.
	int get_direction(Game_object *o2) const;
	int get_quality() const
		{ return quality; }
	void set_quality(int q)
		{ quality = q; }
	int get_quantity() const;	// Like # of coins.
	int get_volume() const;		// Get space taken.
					// Add/remove to/from quantity.
	int modify_quantity(int delta);
					// Set shape coord. within chunk.
	void set_shape_pos(unsigned int shapex, unsigned int shapey)
		{ shape_pos = (shapex << 4) + shapey; }
	void set_lift(int l)
		{ lift = l; }
	Game_object *get_next()
		{ return next; }
	Game_object *get_prev()
		{ return prev; }
	static int lt(class Ordering_info& inf1, Game_object *obj2);
	int lt(Game_object& obj2) const;// Is this less than another in pos.?
					// Return chunk coords.
	int get_cx() const
		{ return cx; }
	int get_cy() const
		{ return cy; }
	void set_invalid()		// Set to invalid position.
		{ cx = cy = 255; }
	void set_chunk(int newcx, int newcy)
		{ cx = newcx; cy = newcy; }
					// Get frame for desired direction.
	int get_dir_framenum(int dir, int frnum) const
		{ return (frnum&0xf) + rotate[dir]; }
					// Get it using current dir.
	int get_dir_framenum(int frnum) const
		{ return (frnum&0xf) + (get_framenum()&(16 | 32)); }
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	void move(Tile_coord t)
		{ move(t.tx, t.ty, t.tz); }
					// Swap positions.
	int swap_positions(Game_object *obj2);
	int get_dependency_count()	// Get objs. to paint first.
		{ return dependencies.size(); }
	Game_object *get_dependency(int i)
		{ return dependencies[i]; }
	void clear_dependencies();	// Remove all dependencies.

					// Find nearby objects.
#ifdef ALPHA_LINUX_CXX
	template<class T>
	static int find_nearby_static(Exult_vector<T*>& vec, Tile_coord pos,
			int shapenum, int delta, int mask, 
			int qual = -359, int framenum = -359);

#define HDR_DECLARE_FIND_NEARBY(decl_type) \
	static int find_nearby(decl_type vec, Tile_coord pos, \
			int shapenum, int delta, int mask,  \
			int qual = -359, int framenum = -359)

	HDR_DECLARE_FIND_NEARBY(Egg_vector&);
	HDR_DECLARE_FIND_NEARBY(Actor_vector&);
	HDR_DECLARE_FIND_NEARBY(Game_object_vector&);

#undef HDR_DECLARE_FIND_NEARBY

#else
	template<class T>
	static int find_nearby(Exult_vector<T*>& vec, Tile_coord pos,
			int shapenum, int delta, int mask, 
			int qual = -359, int framenum = -359);
#endif

	int find_nearby_actors(Actor_vector& vec, int shapenum, int delta) const;
	int find_nearby_eggs(Egg_vector& vec, int shapenum, int delta) const;
	int find_nearby(Game_object_vector& vec, int shapenum, int delta, int mask,
			int qual = -359, int framenum = -359) const;

	Game_object *find_closest(int *shapenums, int num_shapes);
					// Find nearby unblocked tile.
	static Tile_coord find_unblocked_tile(Tile_coord pos,
				int dist, int height = 1,
				const int move_flags = MOVE_WALK);
	Tile_coord find_unblocked_tile(int dist, int height = 1,
				const int move_flags = MOVE_WALK)
		{ return find_unblocked_tile(get_abs_tile_coord(), dist, 
						height, move_flags); }
	Rectangle get_footprint();	// Get tile footprint.
					// Find object blocking given tile.
	static Game_object *find_blocking(Tile_coord tile);
	int is_closed_door() const;	// Checking for a closed door.
	Game_object *get_outermost();	// Get top 'owner' of this object.
	void say(const char *text);		// Put text up by item.
	void say(int from, int to);	// Show random msg. from 'text.flx'.
					// Render.
	virtual void paint(Game_window *gwin);
					// Can this be clicked on?
	virtual int is_findable(Game_window *gwin)
		{ return 1; }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
					// Set new NPC schedule.
	virtual void set_schedule_type(int new_schedule_type,
						Schedule *newsched = 0)
		{  }
					// Return NPC schedule.
	virtual int get_schedule_type()	const
		{ return 11; }		// Loiter.
	virtual std::string get_name() const;
					// Create a copy.
	virtual Game_object *clone() const
		{ return new Game_object(*this); }
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual void set_property(int prop, int val)
		{  }
	virtual int get_property(int prop) const
		{ return 0; }
	virtual int is_dead_npc() const
		{ return 0; }
					// Get/set 'alignment'.
	virtual int get_alignment() const
		{ return 0; }
	virtual void set_alignment(short)
		{  }
	virtual Container_game_object *get_owner()
		{ return 0; }
	virtual void set_owner(Container_game_object *o)
		{  }
	virtual int get_weight();
	virtual int is_dragable() const;// Can this be dragged?
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Set/clear/get actor flag.
	virtual void set_flag(int flag) {  }
	virtual void set_siflag(int flag) { }
	virtual void clear_siflag(int flag) { }
	virtual void clear_flag(int flag) { }
	virtual int get_flag(int flag) const  { return 0; }
	virtual int get_siflag(int flag) const { return 0; }
	virtual int get_type_flag(int flag) const { return 0; }

	virtual unsigned char get_ident() { return 0; }
	virtual void set_ident(unsigned char id) {  }

	virtual int get_npc_num() const	// Get its ID (1-num_npcs).
		{ return 0; }
	virtual int get_party_id() const// Get/set index within party.
		{ return -1; }
	virtual void set_party_id(int i)
		{  }
					// Set for Usecode animations.
	virtual void set_usecode_dir(int d)
		{  }
	virtual int get_usecode_dir() const
		{ return 0; }
	virtual int is_egg() const	// An egg?
		{ return 0; }
					// Count contained objs.
	virtual int count_objects(int shapenum, int qual = -359,
							int framenum = -359)
		{ return 0; }
					// Get contained objs.
	virtual int get_objects(Game_object_vector& vec, int shapenum, int qual,
						int framenum)
		{ return 0; }
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0)
		{ return 0; }
	virtual int add_quantity(int delta, int shapenum, int qual = -359,
				int framenum = -359, int dontcreate = 0)
		{ return delta; }
	virtual int create_quantity(int delta, int shapenum, int qual,
								int framenum)
		{ return delta; }
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum)
		{ return delta; }
	virtual Game_object *find_item(int shapenum, int qual, int framenum)
		{ return 0; }
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_abs_tile_coord(); }
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
	virtual void write_ireg(std::ostream& out)
		{  }
	virtual void elements_read()	// Called when all member items read.
		{  }
					// Write common IREG data.
	void write_common_ireg(unsigned char *buf);
	virtual int get_live_npc_num()
		{ return -1; }

	virtual int get_high_shape() const { return -1; }
	virtual void set_high_shape(int s) { }
	virtual int get_low_lift() const { return -1; }
	virtual void set_low_lift(int l) { }

	};

/*
 *	A list of objects chained together with the 'next' and 'prev'
 *	fields:
 */
class Object_list
	{
	Game_object *first;		// ->first in (circular) chain.
	unsigned short iter_count;	// # of iterators.
public:
	friend class Object_iterator;
	friend class Flat_object_iterator;
	friend class Object_iterator_backwards;
	Object_list(Game_object *f = 0) : first(f), iter_count(0)
		{  }
	~Object_list();
	void report_problem();		// Message if iterators exist.
	int is_empty()
		{ return first == 0; }
	void add_iterator()
		{ iter_count++; }
	void remove_iterator()
		{ iter_count--; }
	Game_object *get_first()
		{ return first; }
					// Insert at head of chain.
	void insert(Game_object *nobj)
		{
		if (iter_count)
			report_problem();
		if (!first)		// First one.
			nobj->next = nobj->prev = nobj;
		else
			{
			nobj->next = first;
			nobj->prev = first->prev;
			first->prev->next = nobj;
			first->prev = nobj;
			}
		first = nobj;
		}
					// Insert before given obj.
	void insert_before(Game_object *nobj, Game_object *before)
		{
		if (iter_count)
			report_problem();
		nobj->next = before;
		nobj->prev = before->prev;
		before->prev->next = nobj;
		before->prev = nobj;
		first = before == first ? nobj : first;
		}
					// Append.
	void append(Game_object *nobj)
		{ insert(nobj); first = nobj->next; }
	void remove(Game_object *dobj)
		{
		if (iter_count)
			report_problem();
		if (dobj == first)
			first = dobj->next != first ? dobj->next : 0;
		dobj->next->prev = dobj->prev;
		dobj->prev->next = dobj->next;
		}
	};

#endif
