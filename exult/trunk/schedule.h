/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Schedule.h - Schedules for characters.
 **
 **	Written: 6/6/2000 - JSF
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

#ifndef INCL_SCHEDULE
#define INCL_SCHEDULE	1

#include "tiles.h"
#include "vec.h"
#include "lists.h"

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
class Schedule
	{
protected:
	Actor *npc;			// Who this controls.
	Tile_coord blocked;		// Tile where actor was blocked.
					// Set actor to walk somewhere, then
					//   do something.
	static void set_action_sequence(Actor *actor, Tile_coord dest,
						Actor_action *when_there);
public:
	Schedule(Actor *n) : npc(n), blocked(-1, -1, -1)
		{  }
	virtual ~Schedule()
		{  }
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
		walk_to_schedule = 32
		};
	virtual void now_what() = 0;	// Npc calls this when it's done
					//   with its last task.
	virtual void im_dormant()	// Npc calls this when it goes from
		{  }			//   being active to dormant.
	virtual void ending(int newtype)// Switching to another schedule.
		{  }
					// Set opponent in combat.
	virtual void set_opponent(Game_object *)
		{  }
	virtual Game_object *get_opponent()	// Get opponent.
		{ return 0; }
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
public:
	Preach_schedule(Actor *n) : Schedule(n)
		{  }
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
	Patrol_schedule(Actor *n)
		: Schedule(n), pathnum(-1)
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
	Tile_coord center;		// Center of rectangle.
	int dist;			// Distance in tiles to roam in each
					//   dir.
public:
	Loiter_schedule(Actor *n, int d = 12);
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Sleep in a  bed.
 */
class Sleep_schedule : public Schedule
	{
	Tile_coord floorloc;		// Where NPC was standing before.
	Game_object *bed;		// Bed being slept on, or 0.
public:
	Sleep_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	};

/*
 *	Sit in a chair.
 */
class Sit_schedule : public Schedule
	{
	Game_object *chair;		// What to sit in.
public:
	Sit_schedule(Actor *n, Game_object *ch = 0);
	virtual void now_what();	// Now what should NPC do?
	static void set_action(Actor *actor, Game_object *chairobj);
	};

/*
 *	Wait tables.
 */
class Waiter_schedule : public Schedule
	{
	int first;			// 1 if first 'what_next()' called.
	Tile_coord startpos;		// Starting position.
	Actor *customer;		// Current customer.
	Slist customers;		// List of customers.
	Vector prep_tables;		// Prep. tables.
	Vector eating_tables;		// Tables with chairs around them.
	void get_customer();
	void find_tables(int shapenum);
	int find_serving_spot(Tile_coord& spot);
public:
	Waiter_schedule(Actor *n);
	virtual void now_what();	// Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
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
					// Open door blocking NPC.
	void open_door(Game_object *door);
					// Set to walk off screen.
	void walk_off_screen(Rectangle& screen, Tile_coord& goal);
public:
	Walk_to_schedule(Actor *n, Tile_coord d, int new_sched);
	virtual void now_what();	// Now what should NPC do?
	virtual void im_dormant();	// Just went dormant.
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
	int get_type() const
		{ return type; }
	int get_time() const
		{ return time; }
	Tile_coord get_pos() const;	// Get position chunk, tile.
	};

#endif
