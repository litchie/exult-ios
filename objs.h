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
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID
	{
	short shapenum;			// Shape #.
	unsigned char framenum;		// Frame # within shape.
public:
					// Create from map data.
	ShapeID(unsigned char l, unsigned char h) 
		: shapenum(l + 256*(h&0x3)), framenum(h >> 2)
		{  }
	ShapeID(unsigned char *& data)	// Read from buffer & incr. ptr.
		{
		unsigned char l = *data++;
		unsigned char h = *data++;
		shapenum = l + 256*(h&0x3);
		framenum = h >> 2;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : shapenum(-1)
		{  }
	~ShapeID() {};
	int is_invalid() const		// End-of-list or invalid?
		{ return shapenum == -1; }
	int is_eol() const
		{ return is_invalid(); }
	int get_shapenum() const
		{ return shapenum; }
	int get_framenum() const
		{ return framenum; }
					// Set to given shape.
	void set_shape(int shnum, int frnum)
		{
		shapenum = shnum;
		framenum = frnum;
		}
	ShapeID(int shnum, int frnum) : shapenum(shnum), framenum(frnum)
		{  }
	void set_shape(int shnum)	// Set shape, but keep old frame #.
		{ shapenum = shnum; }
	void set_frame(int frnum)	// Set to new frame.
		{ framenum = frnum; }
	};

/*
 *	A game object is a shape from shapes.vga along with info. about its
 *	position within its chunk.
 */
class Game_object : public ShapeID
	{
	unsigned char shape_pos;	// (X,Y) of shape within chunk.
	unsigned char lift;		// Raise by 4* this number.
	short quality;			// Some sort of game attribute.
	Game_object *next;		// ->next in chunk list or container.
	Vector dependencies;		// Objects which must be painted before
					//   this can be rendered.
	static unsigned char rotate[8];	// For getting rotated frame #.
public:	//++++++Testing.  Maybe replace with a seq. number.
	unsigned char rendered;		// 1 when rendered.
protected:
	unsigned char cx, cy;		// (Absolute) chunk coords., or if this
					//   is in a container, coords. within
					//   gump's rectangle.
public:
	friend class Chunk_object_list;
	friend class Barge_object;
	friend class Gump_object;
	friend class Actor;
	friend class Actor_gump_object;
					// Create from ifix record.
	Game_object(unsigned char *ifix)
			: ShapeID(ifix[2], ifix[3]), shape_pos(ifix[0]),
			  lift(ifix[1] & 0xf), quality(0), cx(255), cy(255)
		{  }
					// Create from ireg. data.
	Game_object(unsigned char l, unsigned char h, unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: ShapeID(l, h), shape_pos((shapex << 4) + shapey), 
					lift(lft), quality(0), cx(255), cy(255)
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
	void set_next(Game_object *obj) // Set next in list.
		{ next = obj; }
	int lt(Game_object& obj2) const;// Is this less than another in pos.?
					// Return chunk coords.
	int get_cx() const
		{ return cx; }
	int get_cy() const
		{ return cy; }
					// Get frame for desired direction.
	int get_dir_framenum(int dir, int frnum) const
		{ return (frnum&0xf) + rotate[dir]; }
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	void move(Tile_coord t)
		{ move(t.tx, t.ty, t.tz); }
					// Move and change shape/frame.
	void move(Chunk_object_list *old_chunk, int new_cx, int new_cy, 
			Chunk_object_list *new_chunk, 
			int new_sx, int new_sy, int new_frame, 
			int new_lift = -1);
	int get_dependency_count()	// Get objs. to paint first.
		{ return dependencies.get_cnt(); }
	Game_object *get_dependency(int i)
		{ return (Game_object *) dependencies.get(i); }
					// Remove a dependency.
	void remove_dependency(Game_object *obj2)
		{
		int i = dependencies.find(obj2);
		if (i >= 0)
			dependencies.put(i, 0);
		}
	void clear_dependencies();	// Remove all dependencies.
					// Find nearby objects.
	int find_nearby(Vector& vec, int shapenum, int quality, int mask);
	Game_object *find_closest(int *shapenums, int num_shapes);
					// Find object blocking given tile.
	static Game_object *find_blocking(Tile_coord tile);
	int is_closed_door() const;	// Checking for a closed door.
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Set new NPC schedule.
	virtual void set_schedule_type(int new_schedule_type)
		{  }
					// Return NPC schedule.
	virtual int get_schedule_type()	const
		{ return 11; }		// Loiter.
	virtual string get_name() const;
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
	virtual int is_dragable() const;// Can this be dragged?
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Set/clear/get actor flag.
	virtual void set_flag(int flag) { }
	virtual void clear_flag(int flag) { }
	virtual int get_flag(int flag) const { return 0; }
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
	virtual int count_objects(int shapenum, int framenum = -359)
		{ return 0; }
					// Get contained objs.
	virtual int get_objects(Vector& vec, int shapenum, 
						int framenum = -359)
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
	virtual Game_object *remove_and_return(int shapenum, int qual,
								int framenum)
		{ return 0; }
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_abs_tile_coord(); }
					// Write out to IREG file.
	virtual void write_ireg(ostream& out)
		{  }
					// Write common IREG data.
	void write_common_ireg(unsigned char *buf);
	};

/*
 *	A moveable game object (from 'ireg' files):
 */
class Ireg_game_object : public Game_object
	{
	Container_game_object *owner;	// Container this is in, or 0.
public:
					// Create from ireg. data.
	Ireg_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Game_object(l, h, shapex, shapey, lft), owner(0)
		{  }
	Ireg_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft),
						owner(0)
		{  }
					// Copy constructor.
	Ireg_game_object(const Ireg_game_object& obj2)
		: Game_object(obj2), owner(0)
		{  }
	Ireg_game_object() : owner(0)	// Create fake entry.
		{  }
	virtual ~Ireg_game_object()
		{  }
					// Create a copy.
	virtual Game_object *clone() const
		{ return new Ireg_game_object(*this); }
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual Container_game_object *get_owner()
		{ return owner; }
	virtual void set_owner(Container_game_object *o)
		{ owner = o; }
	virtual int is_dragable() const;// Can this be dragged?
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
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
	Spellbook_object(unsigned char l, unsigned char h, unsigned int shapex,
		unsigned int shapey, unsigned int lft, unsigned char *c,
		unsigned long f);
	int add_spell(int spell);	// Add a spell.
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	};

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	int volume_used;		// Amount of volume occupied.
protected:
	Game_object *last_object;	// ->last obj., which pts. to first.
public:
	Container_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Ireg_game_object(l, h, shapex, shapey, lft),
		  volume_used(0), last_object(0)
		{  }
	Container_game_object() : volume_used(0), last_object(0) {  }
	virtual ~Container_game_object();
	Game_object *get_last_object()
		{ return last_object; }
	Game_object *get_first_object()	// Get first inside.
		{ return last_object ? last_object->get_next() : 0; }
					// For when an obj's quantity changes:
	void modify_volume_used(int delta)
		{ volume_used += delta; }
					// Room for this object?
	int has_room(Game_object *obj) const
		{ return obj->get_volume() + volume_used <= get_volume(); }
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
	virtual Game_object *remove_and_return(int shapenum, int qual,
								int framenum);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Count contained objs.
	virtual int count_objects(int shapenum, int framenum = -359);
					// Get contained objs.
	virtual int get_objects(Vector& vec, int shapenum, 
							int framenum = -359);
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
					// Write contents in IREG format.
	void write_contents(ostream& out);
	};

/*
 *	A 'barge', such as a ship or horse-and-cart.  +++++For now, the
 *	objects aren't really added or kept track of, since they have to
 *	be rendered in the outside world.+++++
 */
class Barge_object : public Ireg_game_object
	{
public:
	Barge_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Ireg_game_object(l, h, shapex, shapey, lft)
		{  }
#if 0
	Barge_object() : Ireg_game_object()
		{  }
#endif
	virtual ~Barge_object()
		{  }
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Render.
	virtual void paint(Game_window *gwin);
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	};

/*
 *	An animator:
 */
class Animator : public Time_sensitive
	{
protected:
	Game_object *obj;		// Object we're controlling.
	unsigned char deltax, deltay;	// If wiggling, deltas from
					//   original position.
	unsigned char animating;	// 1 if animation turned on.
	void start_animation();
public:
	Animator(Game_object *o) : obj(o), deltax(0), deltay(0), animating(0)
		{  }
	~Animator();
	void want_animation()		// Want animation on.
		{
		if (!animating)
			start_animation();
		}
	int get_deltax()
		{ return deltax; }
	int get_deltay()
		{ return deltay; }
	};

/*
 *	Animate by going through frames.
 */
class Frame_animator : public Animator
	{
	unsigned char frames;		// # of frames.
	unsigned char ireg;		// 1 if from an IREG file.
public:
	Frame_animator(Game_object *o, int ir);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	Animate by wiggling.
 */
class Wiggle_animator : public Animator
	{
public:
	Wiggle_animator(Game_object *o) : Animator(o)
		{  }
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.  The base class is for non-Ireg ones.
 */
class Animated_object : public Game_object
	{
	Animator *animator;		// Controls animation.
	unsigned char ireg;		// 1 if from an IREG file.
public:
	Animated_object(unsigned char l, unsigned char h, 
				unsigned int shapex, unsigned int shapey, 
				unsigned int lft = 0, unsigned char ir = 0);
	Animated_object(int shapenum, int framenum, unsigned int tilex, 
	       unsigned int tiley, unsigned int lft = 0, unsigned char ir = 0);
	virtual ~Animated_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_abs_tile_coord() + 
			Tile_coord(-animator->get_deltax(), 
				   -animator->get_deltay(), 0); }
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	};


/*
 *	Data cached for a chunk to speed up processing, but which doesn't need
 *	to be saved to disk:
 */
class Chunk_cache
	{
	unsigned char setup_done;	// Already setup.
	unsigned short blocked[256];	// For each tile, a bit for each lift
					//   level if it's blocked by an obj.
					// In the process of implementing:+++++
	Vector egg_objects;		// ->eggs which influence this chunk.
	unsigned short eggs[256];	// Bit #i (0-14) set means that the
					//   tile is within egg_object[i]'s
					//   influence.  Bit 15 means it's 1 or
					//   more of 
					//   egg_objects[15-(num_eggs-1)].
	friend class Chunk_object_list;
	Chunk_cache();			// Create empty one.
	~Chunk_cache();
	int get_num_eggs()
		{ return egg_objects.get_cnt(); }
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, int set);
					// Add/remove object.
	void update_object(Chunk_object_list *chunk,
						Game_object *obj, int add);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, int add);
					// Add egg.
	void update_egg(Chunk_object_list *chunk, Egg_object *egg, int add);
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
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift);
					// Activate eggs nearby.
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
			int tx, int ty, 
			int from_tx, int from_ty, unsigned short eggbits);
	void activate_eggs(Game_object *obj, Chunk_object_list *chunk, 
				int tx, int ty, int from_tx, int from_ty)
		{
		unsigned short eggbits = eggs[ty*tiles_per_chunk + tx];
		if (eggbits)
			activate_eggs(obj, chunk, tx, ty, 
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
	Game_object *objects;		// ->first in ordered list.
	Npc_actor *npcs;		// List of NPC's in this chunk.
					//   (Managed by Npc_actor class.)
	Chunk_cache *cache;		// Data for chunks near player.
	unsigned char roof;		// 1 if a roof present.
	unsigned char light_sources;	// # light sources in chunk.
	unsigned char cx, cy;		// Absolute chunk coords. of this.
public:
	friend class Npc_actor;
	Chunk_object_list(int chunkx, int chunky);
	~Chunk_object_list();		// Delete everything in chunk.
	void add(Game_object *obj);	// Add an object.
	void add_egg(Egg_object *egg);	// Add/remove an egg.
	void remove_egg(Egg_object *egg);
	void remove(Game_object *obj);	// Remove an object.
	int is_roof() const		// Is there a roof?
		{ return roof; }
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
	Game_object *get_first()	// Return first object.
		{ return objects; }
					// Return next object.
	Game_object *get_next(Game_object *obj)
		{ return obj->next; }
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
						int lift, int ztiles, int set)
		{ need_cache()->set_blocked(startx, starty, endx, endy,
							lift, ztiles, set); }
					// Is a spot occupied?
	int is_blocked(int height, int lift, int tx, int ty, int& new_lift)
		{ return cache->is_blocked(height, lift, tx, ty, new_lift); }
					// Old entry:
	int is_blocked(int lift, int tx, int ty, int& new_lift)
		{ return cache->is_blocked(1, lift, tx, ty, new_lift); }
					// Check range.
	static int is_blocked(int height, int lift, int startx, int starty,
					int xtiles, int ytiles, int& new_lift);
					// Check absolute tile.
	static int is_blocked(Tile_coord& tile);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles, int add)
		{ need_cache()->set_egged(egg, tiles, add); }
	void activate_eggs(Game_object *obj, int tx, int ty, 
						int from_tx, int from_ty)
		{ need_cache()->activate_eggs(obj, 
					this, tx, ty, from_tx, from_ty);}
	};

/*
 *	Directions:
 */
enum Direction
	{
	north = 0,
	northeast = 1,
	east = 2,
	southeast = 3,
	south = 4,
	southwest = 5,
	west = 6,
	northwest = 7
	};

Direction Get_direction
	(
	int deltay,
	int deltax
	);
Direction Get_direction4
	(
	int deltay,
	int deltax
	);

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
		{ delete frames; }
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

/*
 *	A rectangle:
 */
class Rectangle
	{
public:					// Let's make it all public.
	int x, y;			// Position.
	int w, h;			// Dimensions.
	Rectangle(int xin, int yin, int win, int hin)
			: x(xin), y(yin), w(win), h(hin)
		{  }
	Rectangle() { }			// An uninitialized one.
					// Is this point in it?
	int has_point(int px, int py) const
		{ return (px >= x && px < x + w && py >= y && py < y + h); }
					// Add another to this one to get
					//  a rect. that encloses both.
	Rectangle add(Rectangle& r2) const
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x < r2.x ? x : r2.x;
		r.y = y < r2.y ? y : r2.y;
		r.w = (xend > xend2 ? xend : xend2) - r.x;
		r.h = (yend > yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Intersect another with this.
	Rectangle intersect(Rectangle& r2) const
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x >= r2.x ? x : r2.x;
		r.y = y >= r2.y ? y : r2.y;
		r.w = (xend <= xend2 ? xend : xend2) - r.x;
		r.h = (yend <= yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Does it intersect another?
	int intersects(Rectangle r2) const
		{
		return (x >= r2.x + r2.w ? 0 : r2.x >= x + w ? 0 :
			y >= r2.y + r2.h ? 0 : r2.y >= y + h ? 0 : 1);
		}
	void shift(int deltax, int deltay)
		{
		x += deltax;
		y += deltay;
		}		
	void enlarge(int delta)		// Add delta in each dir.
		{ x -= delta; y -= delta; w += 2*delta; h += 2*delta; }
	};

/*
 *	Move an object, and possibly change its shape too.
 */
inline void Game_object::move
	(
	Chunk_object_list *old_chunk, 
	int new_cx, int new_cy, 
	Chunk_object_list *new_chunk, 
	int new_sx, int new_sy, int new_frame, 
	int new_lift
	)
	{
	if (old_chunk)			// Remove from current chunk.
		old_chunk->remove(this);
	set_shape_pos(new_sx, new_sy);
	if (new_frame >= 0)
		set_frame(new_frame);
	if (new_lift >= 0)
		set_lift(new_lift);
	new_chunk->add(this);
	}

#endif
