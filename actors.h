/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actors.h - Game actors.
 **
 **	Written: 11/3/98 - JSF
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

#ifndef INCL_ACTORS
#define INCL_ACTORS	1

#include "objs.h"

class Image_window;
class Game_window;
class Npc_actor;
					// The range of actors' rect. gumps:
const int ACTOR_FIRST_GUMP = 57, ACTOR_LAST_GUMP = 68;

/*
 *	An actor:
 */
class Actor : public Sprite
	{
	char *name;			// Its name.
	int usecode;			// # of usecode function.
	int npc_num;			// # in Game_window::npcs list, or -1.
	short properties[12];		// Properties set/used in 'usecode'.
protected:
	unsigned long flags;		// 32 flags used in 'usecode'.
public:
	void set_default_frames();	// Set usual frame sequence.
	Actor(char *nm, int shapenum, int num = -1, int uc = -1);
	~Actor()
		{ delete name; }
	enum Item_flags {		// Bit #'s of flags:
		poisoned = 8,
		dont_render = 16	// Completely invisible.
		};
	enum Item_properties {		// Trying to figure out properties.
		strength = 0,		// Or is max_health 0????
		dexterity = 1,
		intelligence = 2,
		health = 3,
		combat = 4,
		mana = 5,
		magic = 6,		// Max. mana.
		training = 7,		// Training points.
		exp = 8,		// Experience.
		food_level = 9
		};
	int get_face_shapenum()		// Get "portrait" shape #.
		{ return npc_num; }	// It's the NPC's #.
	int get_usecode()
		{ return usecode; }
					// Walk to a desired spot.
	void walk_to_tile(int tx, int ty, int tz);
	void walk_to_tile(Tile_coord p)
		{ walk_to_tile(p.tx, p.ty, p.tz); }
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	virtual char *get_name();
	virtual void set_property(int prop, int val)
		{
		if (prop >= 0 && prop < 12)
			properties[prop] = (short) val;
		}
	virtual int get_property(int prop)
		{ return (prop >= 0 && prop < 12) ? properties[prop] : 0; }
					// Set/clear/get actor flag.
	virtual void set_flag(int flag);
	virtual void clear_flag(int flag);
	virtual int get_flag(int flag);
	virtual int get_npc_num()	// Get its ID (1-num_npcs).
		{ return npc_num; }
	struct	{
		int cx;
		int cy;
		Chunk_object_list *chunk;
		int sx;
		int sy;
		int frame;
		int lift;
		} initial_location;
	void set_initial_location(int new_cx,int new_cy,Chunk_object_list *new_chunk,int new_sx,int new_sy,int new_frame,int new_lift=-1)
		{
		initial_location.cx=new_cx;
		initial_location.cy=new_cy;
		initial_location.chunk=new_chunk;
		initial_location.sx=new_sx;
		initial_location.sy=new_sy;
		initial_location.frame=new_frame;
		initial_location.lift=new_lift;
		};
	};

/*
 *	The main actor.
 */
class Main_actor : public Actor
	{
public:
	Main_actor(char *nm, int shapenum, int num = -1, int uc = -1)
		: Actor(nm, shapenum, num, uc)
		{ 
		}
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
	};


/*
 *	A Schedule controls the NPC it is assigned to.
 */
class Schedule
	{
protected:
	Npc_actor *npc;			// Who this controls.
public:
	Schedule(Npc_actor *n) : npc(n)
		{  }
	enum Schedule_types {
		combat = 0,	horiz_pace = 1,
		vert_pace = 2,	talk = 3,
		dance = 4,	eat = 5,
		farm = 6,	tend_shop = 7,
		miner = 8,	hound = 9,
		stand = 10,	loiter = 11,
		wander = 12,	blacksmith = 13,
		sleep = 14,	wait = 15,
		sit = 16,	graze = 17,
		bake = 18,	sew = 19,
		shy = 20,	lab = 21,
		thief = 22,	waiter = 23,
		special = 24,	kid_games = 25,
		eat_at_inn = 26,duel = 27,
		preach = 28,	patrol = 29,
		desk_work = 30,	follow_avatar = 31
		};
	virtual void now_what() = 0;	// Npc calls this when it's done
					//   with its last task.
	};

/*
 *	A schedule for pacing between two points:
 */
class Pace_schedule : public Schedule
	{
	Tile_coord p0;			// Point 0 tile coords.
	Tile_coord p1;			// Point 1 tile coords.
	char which;			// Which he's going to (0 or 1).
public:
	Pace_schedule(Npc_actor *n, Tile_coord pt0, Tile_coord pt1)
		: Schedule(n), p0(pt0), p1(pt1), which(0)
		{  }
					// Create common schedules:
	static Pace_schedule *create_horiz(Npc_actor *n);
	static Pace_schedule *create_vert(Npc_actor *n);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	A schedule for patrolling along 'path' objects.
 */
class Patrol_schedule : public Schedule
	{
	Vector paths;			// Each 'path' object.
	int pathnum;			// # of next we're heading towards.
public:
	Patrol_schedule(Npc_actor *n)
		: Schedule(n), pathnum(-1)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	An NPC schedule change:
 */
class Schedule_change
	{
	unsigned char time;		// Time*3hours when this takes effect.
	unsigned char type;		// Schedule_type value.
	unsigned char x, y;		// Location within superchunk.
	unsigned char superchunk;	// 0-143.
public:
	Schedule_change() : time(0), type(0), x(0), y(0), superchunk(0)
		{  }
	void set(unsigned char *ent);	// Create from 5-byte entry.
	int get_type()
		{ return type; }
	int get_time()
		{ return time; }
					// Get position chunk, tile.
	void get_pos(int& cx, int& cy, int& tx, int& ty)
		{
		cx = 16*(superchunk%12) + x/16;
		cy = 16*(superchunk/12) + y/16;
		tx = x%16;
		ty = y%16;
		}
	};

/*
 *	A non-player-character that one can converse with:
 */
class Npc_actor : public Actor
	{
	Npc_actor *next;		// Next in same chunk.
	unsigned char nearby;		// Queued as a 'nearby' NPC.  This is
					//   to avoid being added twice.
	unsigned char dormant;		// I.e., off-screen.
	unsigned char schedule_type;	// Schedule type (Schedule_type).
	unsigned char num_schedules;	// # entries below.
	Schedule *schedule;		// Current schedule.
	Schedule_change *schedules;	// List of schedule changes.
public:
	Npc_actor(char *nm, int shapenum, int fshape = -1, int uc = -1);
	~Npc_actor();
	Npc_actor *get_next()
		{ return next; }
	void set_nearby()		// Set/clear/test 'nearby' flag.
		{ nearby = 1; }
	void clear_nearby()
		{ nearby = 0; }
	int is_nearby()
		{ return nearby != 0; }
					// Set schedule list.
	void set_schedules(Schedule_change *list, int cnt)
		{
		delete [] schedules;
		schedules = list;
		num_schedules = cnt;
		}
					// Update schedule for new 3-hour time.
	void update_schedule(Game_window *gwin, int hour3);
					// Set new schedule.
	virtual void set_schedule_type(int new_schedule_type);
	virtual int get_schedule_type()
		{ return schedule_type; }
					// Render.
	virtual void paint(Game_window *gwin);
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
					// Update chunks after NPC moved.
	void switched_chunks(Chunk_object_list *olist,
					Chunk_object_list *nlist);
	};
#if 0
/*
 *	Here's an actor that's just hanging around an area.
 */
class Area_actor : public Npc_actor
	{
	unsigned long next_change;	// When to change motion.
public:
	Area_actor(char *nm, int shapenum, int fshape = -1) : Npc_actor(nm, shapenum, fshape)
		{
		next_change.tv_sec = next_change.tv_usec = 0;
		}
					// Figure next frame location.
	virtual int next_frame(unsigned long time,
		int& new_cx, int& new_cy, int& new_sx, int& new_sy,
		int& new_frame);
	};
#endif
#endif
