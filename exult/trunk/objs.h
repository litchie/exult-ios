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

#include <string>	// STL string
#include "shapeid.h"
#include "rect.h"
#include "vec.h"
#include "tqueue.h"
#include "tiles.h"

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

enum Exult_Game {
	NONE,
	BLACK_GATE,
	SERPENT_ISLE
};

#define MOVE_FLY (1<<4)
#define	MOVE_WALK (1<<5)
#define MOVE_SWIM (1<<6)
#define	MOVE_ALL_TERRAIN ((1<<5)|(1<<6))
#define MOVE_ETHEREAL (1<<7)
#define MOVE_ALL (MOVE_FLY|MOVE_WALK|MOVE_SWIM|MOVE_ETHEREAL)


/*
 *	Sizes:
 */
const int tilesize = 8;			// A tile (shape) is 8x8 pixels.
const int tiles_per_chunk = 16;		// A chunk is 16x16 tiles.
const int chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int num_chunks = 12*16;		// Total # of chunks in each dir.
const int chunks_per_schunk = 16;	// # chunks in each superchunk.
const int tiles_per_schunk = 16*16;	// # tiles in each superchunk.
					// Total # tiles in each dir.:
const int num_tiles = tiles_per_chunk*num_chunks;

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
	GOVector dependencies;		// Objects which must be painted before
					//   this can be rendered.
	GOVector dependors;		// Objects which must be painted after.
	static unsigned char rotate[8];	// For getting rotated frame #.
public:
	unsigned long render_seq;	// Render sequence #.
protected:
	unsigned char cx, cy;		// (Absolute) chunk coords., or if this
					//   is in a container, coords. within
					//   gump's rectangle.
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
		unknown1 = 18,		// ??Used for Usecode-created items.
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
	static int find_nearby(GOVector& vec, Tile_coord pos,
			int shapenum, int delta, int mask, 
			int qual = -359, int framenum = -359);
	int find_nearby(GOVector& vec, int shapenum, int delta, int mask,
			int qual = -359, int framenum = -359)
		{ return find_nearby(vec, get_abs_tile_coord(), shapenum,
						delta, mask, qual, framenum); }
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
	virtual int get_objects(GOVector& vec, int shapenum, int qual,
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

/*
 *	A moveable game object (from 'ireg' files):
 */
class Ireg_game_object : public Game_object
	{
	Container_game_object *owner;	// Container this is in, or 0.
protected:
	unsigned flags:32;		// 32 flags used in 'usecode'.
	unsigned flags2:32;		// Another 32 flags used in 'usecode'.
public:
	Ireg_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft),
				owner(0), flags(0), flags2(0), lowlift(-1), 
				highshape (-1)
		{  }
					// Copy constructor.
	Ireg_game_object(const Ireg_game_object& obj2)
		: Game_object(obj2), owner(0), flags(0), flags2(0),
					lowlift(-1), highshape (-1)
		{  }
					// Create fake entry.
	Ireg_game_object() : owner(0), flags(0), flags2(0), lowlift(-1), highshape (-1)
		{  }
	virtual ~Ireg_game_object()
		{  }
	void set_flags(unsigned long f)	// For initialization.
		{ flags = f; }
					// Create a copy.
	virtual Game_object *clone() const
		{ return new Ireg_game_object(*this); }
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	void move(Tile_coord t)
		{ move(t.tx, t.ty, t.tz); }
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual Container_game_object *get_owner()
		{ return owner; }
	virtual void set_owner(Container_game_object *o)
		{ owner = o; }
	virtual int is_dragable() const;// Can this be dragged?
	virtual void set_flag(int flag)
		{
		if (flag >= 0 && flag < 32)
			flags |= ((unsigned long) 1 << flag);
		else if (flag >= 32 && flag < 64)
			flags2 |= ((unsigned long) 1 << (flag-32));
		}
	virtual void clear_flag(int flag)
		{
		if (flag >= 0 && flag < 32)
			flags &= ~((unsigned long) 1 << flag);
		else if (flag >= 32 && flag < 64)
			flags2 &= ~((unsigned long) 1 << (flag-32));
		}
	virtual int get_flag(int flag) const
		{
		if (flag >= 0 && flag < 32)
			return flags & ((unsigned long) 1 << flag);
		else if (flag >= 32 && flag < 64)
			return flags2 & ((unsigned long) 1 << (flag-32));
		return 0;
		}
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);
	int	lowlift;
	int highshape;
	virtual int get_high_shape() const { return highshape; };
	virtual void set_high_shape(int s) { highshape = s;};
	virtual int get_low_lift() const { return lowlift; };
	virtual void set_low_lift(int l) { lowlift = l;};
	};

/*
 *	A spellbook:
 */
class Spellbook_object : public Ireg_game_object
	{
	unsigned char circles[9];	// Spell-present flags for each circle.
	unsigned long flags;		// Unknown at present.
	int bookmark;			// Spell # that bookmark is on, or -1.
public:
	friend class Spellbook_gump;
					// Create from ireg. data.
	Spellbook_object(int shapenum, int framenum, unsigned int shapex,
		unsigned int shapey, unsigned int lft, unsigned char *c,
		unsigned long f);
	int add_spell(int spell);	// Add a spell.
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);
	};

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	int volume_used;		// Amount of volume occupied.
	unsigned char resistance;	// Resistance to attack.
protected:
	Object_list objects;		// ->first object.
	int get_max_volume() const	// Max. we'll hold. (0 == infinite).
		{ return 4*get_volume(); }
public:
	Container_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				unsigned char res = 0)
		: Ireg_game_object(shapenum, framenum, tilex, tiley, lft),
		  volume_used(0), resistance(res), objects(0)
		{  }
	Container_game_object() : volume_used(0), resistance(0),
		objects(0) {  }
	virtual ~Container_game_object();
	Object_list& get_objects()
		{ return objects; }
					// For when an obj's quantity changes:
	void modify_volume_used(int delta)
		{ volume_used += delta; }
					// Room for this object?
	int has_room(Game_object *obj) const
		{ return obj->get_volume() + volume_used <= get_max_volume(); }
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Add to NPC 'ready' spot.
	virtual int add_readied(Game_object *obj, int index)
		{ return add(obj); }
					// Change member shape.
	virtual void change_member_shape(Game_object *obj, int newshape);
					// Find object's spot.
	virtual int find_readied(Game_object *obj)
		{ return -1; }
	virtual Game_object *get_readied(int index) const
		{ return 0; }
					// Add/remove quantities of objs.
	virtual int add_quantity(int delta, int shapenum, int qual = -359,
				int framenum = -359, int dontcreate = 0);
	virtual int create_quantity(int delta, int shapenum, int qual,
							int framenum);
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum);
	virtual Game_object *find_item(int shapenum, int qual, int framenum);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
	virtual int get_weight();
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Count contained objs.
	virtual int count_objects(int shapenum, int qual = -359,
							int framenum = -359);
					// Get contained objs.
	virtual int get_objects(GOVector& vec, int shapenum, int qual,
						int framenum);
					// Under attack.
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
					int ammo_shape = 0);
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);
					// Write contents in IREG format.
	void write_contents(std::ostream& out);
	};

/*
 *	Data cached for a chunk to speed up processing, but which doesn't need
 *	to be saved to disk:
 */
class Chunk_cache
	{
	Chunk_object_list	*obj_list;
	unsigned char setup_done;	// Already setup.
	unsigned short blocked[256];	// For each tile, a bit for each lift
					//   level if it's blocked by an obj.
					// In the process of implementing:+++++
	EggVector egg_objects;		// ->eggs which influence this chunk.
	unsigned short eggs[256];	// Bit #i (0-14) set means that the
					//   tile is within egg_object[i]'s
					//   influence.  Bit 15 means it's 1 or
					//   more of 
					//   egg_objects[15-(num_eggs-1)].
	friend class Chunk_object_list;
	Chunk_cache();			// Create empty one.
	~Chunk_cache();
	int get_num_eggs()
		{ return egg_objects.size(); }
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, bool set);
					// Add/remove object.
	void update_object(Chunk_object_list *chunk,
						Game_object *obj, bool add);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, bool add);
					// Add egg.
	void update_egg(Chunk_object_list *chunk, Egg_object *egg, bool add);
					// Set up with chunk's data.
	void setup(Chunk_object_list *chunk);
					// Set blocked tile's bits.
	void set_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] |= 
						(((1 << ztiles) - 1) << lift);
		}
					// Clear blocked tile's bits.
	void clear_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] &= 
					~(((1 << ztiles) - 1) << lift);
		}
					// Get highest lift blocked below a
					//   given level for a desired tile.
	int get_highest_blocked(int lift, unsigned short tflags);
	int get_highest_blocked(int lift, int tx, int ty);
	int get_lowest_blocked(int lift, unsigned short tflags);
	int get_lowest_blocked(int lift, int tx, int ty);
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift,
				const int move_flags, int max_drop = 1);
					// Activate eggs nearby.
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
			int tx, int ty, int tz,
			int from_tx, int from_ty, unsigned short eggbits);
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
			int tx, int ty, int tz, int from_tx, int from_ty)
		{
		unsigned short eggbits = eggs[
			(ty%tiles_per_chunk)*tiles_per_chunk + 
							(tx%tiles_per_chunk)];
		if (eggbits)
			activate_eggs(obj, chunk, tx, ty, tz,
						from_tx, from_ty,  eggbits);
		}
	};

/*
 *	Game objects are stored in a list for each chunk, sorted from top-to-
 *	bottom, left-to-right.
 */
class Chunk_object_list
	{
	ShapeID flats[256];		// Flat (non-RLE) shapes.  The unused
					//   are "invalid" entries.
	Object_list objects;		// ->first in list of all objs.  'Flat'
					//   obs. (lift=0,ht=0) stored 1st.
	Game_object *first_nonflat;	// ->first nonflat in 'objects'.
	unsigned char *dungeon_bits;	// A 'dungeon' bit flag for each tile.
	Npc_actor *npcs;		// List of NPC's in this chunk.
					//   (Managed by Npc_actor class.)
	Chunk_cache *cache;		// Data for chunks near player.
	unsigned char roof;		// 1 if a roof present.
	unsigned char light_sources;	// # light sources in chunk.
	unsigned char cx, cy;		// Absolute chunk coords. of this.
	void add_dungeon_bits(Rectangle& tiles);
	void add_dependencies(Game_object *newobj,
					class Ordering_info& newinfo);
public:
	friend class Npc_actor;
	friend class Object_iterator;
	friend class Flat_object_iterator;
	friend class Nonflat_object_iterator;
	friend class Object_iterator_backwards;
	Chunk_object_list(int chunkx, int chunky);
	~Chunk_object_list();		// Delete everything in chunk.
	void add(Game_object *obj);	// Add an object.
	void add_egg(Egg_object *egg);	// Add/remove an egg.
	void remove_egg(Egg_object *egg);
	void remove(Game_object *obj);	// Remove an object.
					// Apply gravity over given area.
	static void gravity(Rectangle area, int lift);
					// Is there a roof? Returns height
	int is_roof(int tx, int ty, int lift);
	int get_cx() const
		{ return cx; }
	int get_cy() const
		{ return cy; }
	Npc_actor *get_npcs()		// Get ->first npc in chunk.
		{ return npcs; }
	int get_light_sources() const	// Get #lights.
		{ return light_sources; }
					// Set/get flat shape.
	void set_flat(int tilex, int tiley, ShapeID id)
		{ flats[16*tiley + tilex] = id; }
	ShapeID get_flat(int tilex, int tiley) const
		{ return flats[16*tiley + tilex]; }
					// Get/create cache.
	Chunk_cache *need_cache()
		{ 
		return cache ? cache 
				: (cache = new Chunk_cache()); 
		}
	void setup_cache()
		{ 
		if (!cache || !cache->setup_done)
			need_cache()->setup(this);
		}
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, bool set)
		{ need_cache()->set_blocked(startx, starty, endx, endy,
							lift, ztiles, set); }
					// Get highest lift blocked.
	int get_highest_blocked(int lift, int tx, int ty)
		{ return need_cache()->get_highest_blocked(lift, tx, ty); }
	int get_lowest_blocked(int lift, int tx, int ty)
		{ return need_cache()->get_lowest_blocked(lift, tx, ty); }
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift,
					const int move_flags, int max_drop = 1)
		{ return cache->is_blocked(height, lift, tx, ty, new_lift,
						move_flags, max_drop); }
					// Check range.
	static int is_blocked(int height, int lift, int startx, int starty,
		int xtiles, int ytiles, int& new_lift, const int move_flags, 
							int max_drop = 1);
					// Check absolute tile.
	static int is_blocked(Tile_coord& tile, int height = 1,
		const int move_flags = MOVE_ALL_TERRAIN, int max_drop = 1);
					// Check for > 1x1 object.
	static int is_blocked(int xtiles, int ytiles, int ztiles,
			Tile_coord from, Tile_coord to, const int move_flags);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, bool add)
		{ need_cache()->set_egged(egg, tiles, add); }
	void activate_eggs(Game_object *obj, int tx, int ty, int tz,
						int from_tx, int from_ty)
		{ need_cache()->activate_eggs(obj, 
					this, tx, ty, tz, from_tx, from_ty);}
					// Use this when teleported in.
	static void try_all_eggs(Game_object *obj, int tx, int ty, int tz,
						int from_tx, int from_ty);
	void setup_dungeon_bits();	// Set up after IFIX objs. read.
	int has_dungeon()		// Any tiles within dungeon?
		{ return dungeon_bits != 0; }
					// NOTE:  The following should only be
					//   called if has_dungeon()==1.
	int in_dungeon(int tx, int ty)	// Is object within dungeon?
		{
		int tnum = ty*tiles_per_chunk + tx;
		return dungeon_bits[tnum/8] & (1 << (tnum%8));
		}
	int in_dungeon(Game_object *obj)// Is object within dungeon?
		{ return in_dungeon(obj->get_tx(), obj->get_ty()); }
	};

/*
 *	A sequence of frames.  Frame 0 is the resting state.
 */
class Frames_sequence
	{
	unsigned char *frames;
	int num_frames;
public:
	Frames_sequence(int cnt, unsigned char *f);
	~Frames_sequence()
		{ delete [] frames; }
					// Get resting frame.
	unsigned char get_resting() const
		{ return frames[0]; }
					// Get next frame.  Call
					//   with index = 0 for first one.
	unsigned char get_next(int& index) const
		{
		if (++index >= num_frames)
			index = 1;
		return frames[index];
		}
	};
#endif
