/*
 *	Schedule.h - Schedules for characters.
 *
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

#ifndef SCHEDULE_H
#define SCHEDULE_H	1

#include "tiles.h"
#include "vec.h"
#include "lists.h"
#include "singles.h"

#ifdef WIN32
#define Rectangle RECTX
#endif

class Game_object;
class Actor;
class Rectangle;
class Actor_action;

/*
 *	A Schedule controls the NPC it is assigned to.
 */
class Schedule : public Game_singletons
	{
protected:
	Actor *npc;			// Who this controls.
	Tile_coord blocked;		// Tile where actor was blocked.
	short prev_type;		// Actor's previous schedule.
	int street_maintenance_failures;// # times failed to find path.
	long street_maintenance_time;	// Time (msecs) when last tried.
public:
	Schedule(Actor *n);
	virtual ~Schedule()
		{  }
	int get_prev_type() const
		{ return prev_type; }
	void set_blocked(Tile_coord b)
		{ blocked = b; }
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
		desk_work = 30,	follow_avatar = 31,
					// Our own:
		walk_to_schedule = 32,
		street_maintenance = 33
		};
					// Set actor to walk somewhere, then
					//   do something.
	static void set_action_sequence(Actor *actor, Tile_coord dest,
		Actor_action *when_there, bool from_off_screen = false,
							int delay = 0);
	int try_street_maintenance();	// Handle street-lamps, shutters.
	virtual void now_what() = 0;	// Npc calls this when it's done
					//   with its last task.
	virtual void im_dormant()	// Npc calls this when it goes from
		{  }			//   being active to dormant.
	virtual void ending(int newtype)// Switching to another schedule.
		{  }
	virtual void set_weapon()	// Set weapon info.
		{  }
					// Set where to sleep.
	virtual void set_bed(Game_object *b)
		{  }
					// Notify that schedule's obj. has
					//   been moved.
	virtual void notify_object_gone(Game_object *obj)
		{  }
					// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc);
	};

/*
 *	Street maintenance (turn lamps on/off).
 */
class Street_maintenance_schedule : public Schedule
	{
	Game_object *obj;		// Lamp/shutters.
	int shapenum, framenum;		// Save original shapenum.
	Actor_action *paction;		// Path to follow to get there.
public:
	Street_maintenance_schedule(Actor *n, Actor_action *p, Game_object *o);
	virtual void now_what();
					// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc);
	};

/*
 *	For following the Avatar (by party members):
 */
class Follow_avatar_schedule : public Schedule
	{
	unsigned long next_path_time;	// Next time we're allowed to use
					//   pathfinding to follow leader.
public:
	Follow_avatar_schedule(Actor *n) : Schedule(n), next_path_time(0)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	A 'do nothing' schedule.
 */
class Wait_schedule : public Schedule
	{
public:
	Wait_schedule(Actor *n) : Schedule(n)
		{  }
	virtual void now_what();	// Now what should NPC do?
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
	Pace_schedule(Actor *n, Tile_coord pt0, Tile_coord pt1)
		: Schedule(n), p0(pt0), p1(pt1), which(0)
		{  }
					// Create common schedules:
	static Pace_schedule *create_horiz(Actor *n);
	static Pace_schedule *create_vert(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	A schedule for eating at an inn.
 */
class Eat_at_inn_schedule : public Schedule
	{
public:
	Eat_at_inn_schedule(Actor *n) : Schedule(n)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	A schedule for preaching.
 */
class Preach_schedule : public Schedule
	{
	int first;
public:
	Preach_schedule(Actor *n) : Schedule(n), first(1)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	A schedule for patrolling along 'path' objects.
 */
class Patrol_schedule : public Schedule
	{
	Game_object_vector paths;	// Each 'path' object.
	int pathnum;			// # of next we're heading towards.
	int dir;			// 1 or -1;
	int failures;			// # of failures to find marker.
	bool sitting;			// Sat down for one of the paths.
	bool find_next;			// Search for next path object.
public:
	Patrol_schedule(Actor *n)
		: Schedule(n), pathnum(-1), dir(1), failures(0),
		  sitting(false), find_next(true)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Talk to avatar.
 */
class Talk_schedule : public Schedule
	{
	int phase;			// 0=walk to Av., 1=talk, 2=done.
public:
	Talk_schedule(Actor *n) : Schedule(n), phase(0)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Loiter within a rectangle.
 */
class Loiter_schedule : public Schedule
	{
protected:
	Tile_coord center;		// Center of rectangle.
	int dist;			// Distance in tiles to roam in each
					//   dir.
public:
	Loiter_schedule(Actor *n, int d = 12);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Kid games.
 */
class Kid_games_schedule : public Loiter_schedule
	{
	Actor_queue kids;			// Other kids playing.
public:
	Kid_games_schedule(Actor *n) : Loiter_schedule(n, 10)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Dance.
 */
class Dance_schedule : public Loiter_schedule
	{
public:
	Dance_schedule(Actor *n) : Loiter_schedule(n, 4)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Miner/farmer:
 */
class Tool_schedule : public Loiter_schedule
	{
	int toolshape;			// Pick/scythe shape.
	Game_object *tool;
public:
	Tool_schedule(Actor *n, int shnum) : Loiter_schedule(n, 12), 
					toolshape(shnum), tool(0)
		{  }
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	};

/*
 *	Hound the Avatar.
 */
class Hound_schedule : public Schedule
	{
public:
	Hound_schedule(Actor *n) : Schedule(n)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Wander all over the place, using pathfinding.
 */
class Wander_schedule : public Loiter_schedule
	{
public:
	Wander_schedule(Actor *n) : Loiter_schedule(n, 128)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Sleep in a  bed.
 */
class Sleep_schedule : public Schedule
	{
	Tile_coord floorloc;		// Where NPC was standing before.
	Game_object *bed;		// Bed being slept on, or 0.
	int state;
	int spread0, spread1;		// Range of bedspread frames.
public:
	Sleep_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
					// Set where to sleep.
	virtual void set_bed(Game_object *b)
		{ bed = b; state = 0; }
	};

/*
 *	Sit in a chair.
 */
class Sit_schedule : public Schedule
	{
	Game_object *chair;		// What to sit in.
	bool sat;			// True if we already sat down.
	bool did_barge_usecode;		// So we only call it once.
public:
	Sit_schedule(Actor *n, Game_object *ch = 0);
	virtual void now_what();	// Now what should NPC do?
	static bool is_occupied(Game_object *chairobj, Actor *actor);
	static bool set_action(Actor *actor, Game_object *chairobj = 0,
				int delay = 0, Game_object **chair_found = 0);
	};

/*
 *	Desk work - Just sit in front of desk.
 */
class Desk_schedule : public Schedule
	{
	Game_object *chair;		// What to sit in.
public:
	Desk_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Shy away from Avatar.
 */
class Shy_schedule : public Schedule
	{
public:
	Shy_schedule(Actor *n) : Schedule(n)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Lab work.
 */
class Lab_schedule : public Schedule
	{
	Game_object_vector tables;
	Game_object *chair;		// Chair to sit in.
	Game_object *book;		// Book to read.
	Game_object *cauldron;
	Tile_coord spot_on_table;
 	enum {
		start,
		walk_to_cauldron,
		use_cauldron,
		sit_down,
		read_book,
		stand_up,
		walk_to_table,
		use_potion
	} state;
	void init();
public:
	Lab_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Wait tables.
 */
class Waiter_schedule : public Schedule
	{
	int first;			// 1 if first 'what_next()' called.
	Tile_coord startpos;		// Starting position.
	Actor *customer;		// Current customer.
	Actor_queue customers;		// List of customers.
	Game_object_vector prep_tables;		// Prep. tables.
	Game_object_vector eating_tables;		// Tables with chairs around them.
	void get_customer();
	void find_tables(int shapenum);
	int find_serving_spot(Tile_coord& spot);
public:
	Waiter_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	};

/*
 *	Sew/weave schedule.
 */
class Sew_schedule : public Schedule
	{
	Game_object *bale;		// Bale of wool.
	Game_object *spinwheel;
	Game_object *chair;		// In front of spinning wheel.
	Game_object *spindle;		// Spindle of thread.
	Game_object *loom;
	Game_object *cloth;
	Game_object *work_table, *wares_table;
	int sew_clothes_cnt;
 	enum {
		get_wool,
		sit_at_wheel,
		spin_wool,
		get_thread,
		weave_cloth,
		get_cloth,
		to_work_table,
		set_to_sew,
		sew_clothes,
		get_clothes,
		display_clothes,
		done
	} state;
public:
	Sew_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	};

/*
 *	Bake schedule
 */
class Bake_schedule : public Schedule
{
	Game_object *oven;
	Game_object *worktable;
	Game_object *displaytable;
	Game_object *flourbag;
	Game_object *dough;
	Game_object *dough_in_oven;
	int baked_count;
	enum {
		to_flour,
		get_flour,
		to_table,
		make_dough,
		remove_from_oven,
		display_wares,
		get_dough,
		put_in_oven
	} state;
public:
	Bake_schedule(Actor *n);
	virtual void now_what();
	virtual void ending(int newtype);
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *	Blacksmith schedule
 */
class Forge_schedule : public Schedule
{
	Game_object *tongs;
	Game_object *hammer;
	Game_object *blank;
	Game_object *firepit;
	Game_object *anvil;
	Game_object *trough;
	Game_object *bellows;
	enum {
		put_sword_on_firepit,
		use_bellows,
		get_tongs,
		sword_on_anvil,
		get_hammer,
		use_hammer,
		walk_to_trough,
		fill_trough,
		get_tongs2,
		use_trough,
		done
	} state;
public:
	Forge_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype); // Switching to another schedule
};

/*
 *	Walk to the destination for a new schedule.
 */
class Walk_to_schedule : public Schedule
	{
	Tile_coord dest;		// Where we're going.
	int first_delay;		// Starting delay (1/1000's sec.)
	int new_schedule;		// Schedule to set when we get there.
	int retries;			// # failures at finding path.
	int legs;			// # times restarted walk.
					// Set to walk off screen.
	void walk_off_screen(Rectangle& screen, Tile_coord& goal);
public:
	Walk_to_schedule(Actor *n, Tile_coord d, int new_sched,
							int delay = -1);
	virtual void now_what();	// Now what should NPC do?
	virtual void im_dormant();	// Just went dormant.
					// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc);
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
	void set(unsigned char *ent);	// Create from 4-byte entry.
	void get(unsigned char *ent);	// Get 4-byte entry.
	void set(int ax, int ay, unsigned char stype, unsigned char stime);
	int get_type() const
		{ return type; }
	int get_time() const
		{ return time; }
	Tile_coord get_pos() const;	// Get position chunk, tile.
	};

#endif
