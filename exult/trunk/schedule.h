/*
 *  Schedule.h - Schedules for characters.
 *
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef SCHEDULE_H
#define SCHEDULE_H  1

#include "tiles.h"
#include "singles.h"
#include "objclient.h"
#include <vector>

#ifdef WIN32
#include <windows.h>
#define Rectangle RECTX
#endif

class Game_object;
class Actor;
class Rectangle;
class Actor_action;
class Usecode_value;

using std::vector;

/*
 *  A Schedule controls the NPC it is assigned to.
 */
class Schedule : public Game_singletons, public Object_client {
protected:
	Actor *npc;         // Who this controls.
	Tile_coord blocked;     // Tile where actor was blocked.
	short prev_type;        // Actor's previous schedule.
	int street_maintenance_failures;// # times failed to find path.
	long street_maintenance_time;   // Time (msecs) when last tried.
public:
	Schedule(Actor *n);
	virtual ~Schedule() {
		remove_clients();
	}
	int get_prev_type() const {
		return prev_type;
	}
	void set_blocked(Tile_coord const &b) {
		blocked = b;
	}
	enum Schedule_types {
	    combat = 0, horiz_pace = 1,
	    vert_pace = 2,  talk = 3,
	    dance = 4,  eat = 5,
	    farm = 6,   tend_shop = 7,
	    miner = 8,  hound = 9,
	    stand = 10, loiter = 11,
	    wander = 12,    blacksmith = 13,
	    sleep = 14, wait = 15,
	    sit = 16,   graze = 17,
	    bake = 18,  sew = 19,
	    shy = 20,   lab = 21,
	    thief = 22, waiter = 23,
	    special = 24,   kid_games = 25,
	    eat_at_inn = 26, duel = 27,
	    preach = 28,    patrol = 29,
	    desk_work = 30, follow_avatar = 31,
	    // Our own:
	    walk_to_schedule = 32,
	    street_maintenance = 33,
	    arrest_avatar = 34,
	    first_scripted_schedule = 0x80
	};
	// Set actor to walk somewhere, then
	//   do something.
	static void set_action_sequence(Actor *actor, Tile_coord const &dest,
	                                Actor_action *when_there, bool from_off_screen = false,
	                                int delay = 0);
	static Game_object *set_procure_item_action(Actor *actor, Game_object *obj,
	        int dist, int shnum, int frnum);
	static bool set_pickup_item_action(Actor *actor, Game_object *obj,
	                                   int delay);
	static Game_object *find_nearest(Actor *actor, Game_object_vector &nearby);
	int try_street_maintenance();   // Handle street-lamps, shutters.
	virtual void now_what() = 0;    // Npc calls this when it's done
	//   with its last task.
	virtual void im_dormant();  // Npc calls this when it goes from
	//  being active to dormant.
	virtual void ending(int newtype)// Switching to another schedule.
	{  }
	virtual void set_weapon(bool removed = false)   // Set weapon info.
	{  }
	// Set where to sleep.
	virtual void set_bed(Game_object *b)
	{  }
	// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc) const;
	// Look for foes.
	bool seek_foes();
	/* For Object_client: +++++ Override in sub-classes. */
	// Notify that schedule's obj. has been moved or deleted.
	virtual void notify_object_gone(Game_object *obj)
	{  }
	bool try_proximity_usecode(int odds);
};

/*
 *  Schedule is implemented as Usecode functions.
 */
class Scripted_schedule : public Schedule {
	int type;           // Sched. type (so we can get name).
	Usecode_value *inst;        // Usecode schedule instance.
	// Usecode function #'s:
	int now_what_id, im_dormant_id, ending_id, set_weapon_id, set_bed_id,
	    notify_object_gone_id;
	void run(int id);
public:
	Scripted_schedule(Actor *n, int ty);
	virtual ~Scripted_schedule();
	virtual void now_what() {
		run(now_what_id);
	}
	virtual void im_dormant() {
		run(im_dormant_id);
	}
	virtual void ending(int newtype) {
		run(ending_id);
	}
	virtual void set_weapon(bool removed = false) {
		run(set_weapon_id);
	}
	virtual void set_bed(Game_object *b) {
		run(set_bed_id);
	}
	virtual void notify_object_gone(Game_object *obj) {
		run(notify_object_gone_id);
	}
};

/*
 *  Street maintenance (turn lamps on/off).
 */
class Street_maintenance_schedule : public Schedule {
	Game_object *obj;       // Lamp/shutters.
	int shapenum, framenum;     // Save original shapenum.
	Actor_action *paction;      // Path to follow to get there.
	Tile_coord oldloc;
public:
	Street_maintenance_schedule(Actor *n, Actor_action *p, Game_object *o);
	virtual void now_what();
	// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc) const;
	virtual void notify_object_gone(Game_object *obj);
	virtual void ending(int newtype);
};

/*
 *  For following the Avatar (by party members):
 */
class Follow_avatar_schedule : public Schedule {
	unsigned long next_path_time;   // Next time we're allowed to use
	//   pathfinding to follow leader.
public:
	Follow_avatar_schedule(Actor *n) : Schedule(n), next_path_time(0)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  A 'do nothing' schedule.
 */
class Wait_schedule : public Schedule {
public:
	Wait_schedule(Actor *n) : Schedule(n)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  A schedule for pacing between two obstacles:
 */
class Pace_schedule : public Schedule {
	char which;     // 0 for north-south, 1 for east-west
	Tile_coord loc; // The starting position of the schedule
	int phase;      // Current phase
public:
	Pace_schedule(Actor *n, char dir, Tile_coord const &pos)
		: Schedule(n), which(dir), loc(pos), phase(0)
	{  }
	// Create common schedules:
	static Pace_schedule *create_horiz(Actor *n);
	static Pace_schedule *create_vert(Actor *n);
	static void pace(Actor *npc, char &which, int &phase, Tile_coord &blocked, int delay);
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  A schedule for eating at an inn.
 */
class Eat_at_inn_schedule : public Schedule {
public:
	Eat_at_inn_schedule(Actor *n) : Schedule(n)
	{  }
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype); // Switching to another schedule
	virtual void im_dormant();  // Just went dormant.
};

/*
 *  A schedule for preaching.
 */
class Preach_schedule : public Schedule {
	enum {
	    find_podium,
	    at_podium,
	    exhort,
	    visit,
	    talk_member,
	    find_icon,
	    pray
	} state;
public:
	Preach_schedule(Actor *n) : Schedule(n), state(find_podium)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  A schedule for patrolling along 'path' objects.
 */
class Patrol_schedule : public Schedule {
	enum {PATH_SHAPE = 607};
	static int num_path_eggs;
	vector<Game_object *> paths; // Each 'path' object.
	int pathnum;            // # of next we're heading towards.
	int dir;                // 1 or -1;
	int failures;           // # of failures to find marker.
	int state;              // The patrol state.
	Tile_coord center;      // For 'loiter' and 'pace' path eggs.
	char whichdir;          // For 'pace' path eggs.
	int phase;              // For 'pace' path eggs.
	int pace_count;         // For 'pace' path eggs.
	Game_object *hammer;    // For 'hammer' path eggs.
	Game_object *book;      // For 'read' path eggs.
	bool seek_combat;       // The NPC should seek enemies while patrolling.
	bool forever;           // If should keep executing last path egg.
public:
	Patrol_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype); // Switching to another schedule
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Talk to avatar.
 */
class Talk_schedule : public Schedule {
	int firstbark, lastbark;
	int eventid;
protected:
	int phase;          // 0=walk to Av., 1=talk, 2=done.
	Talk_schedule(Actor *n, int fb, int lb, int eid);
public:
	Talk_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Arrest avatar.
 */
class Arrest_avatar_schedule : public Talk_schedule {
public:
	Arrest_avatar_schedule(Actor *n);
	// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc) const;
	virtual void ending(int newtype);// Switching to another schedule.
};

/*
 *  Loiter within a rectangle.
 */
class Loiter_schedule : public Schedule {
protected:
	Tile_coord center;      // Center of rectangle.
	int dist;           // Distance in tiles to roam in each
	//   dir.
public:
	Loiter_schedule(Actor *n, int d = 12);
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Graze within a rectangle.
 */
class Graze_schedule : public Loiter_schedule {
protected:
	int phase;
public:
	Graze_schedule(Actor *n, int d = 12) : Loiter_schedule(n, d), phase(0) {  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Kid games.
 */
class Kid_games_schedule : public Loiter_schedule {
	vector<Actor *> kids;           // Other kids playing.
public:
	Kid_games_schedule(Actor *n) : Loiter_schedule(n, 10)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Dance.
 */
class Dance_schedule : public Loiter_schedule {
public:
	Dance_schedule(Actor *n) : Loiter_schedule(n, 4)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Miner/farmer:
 */
class Tool_schedule : public Loiter_schedule {
protected:
	int toolshape;          // Pick/scythe shape.
	Game_object *tool;
	void get_tool();
public:
	Tool_schedule(Actor *n, int shnum) : Loiter_schedule(n, 12),
		toolshape(shnum), tool(0)
	{  }
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
};

/*
 *  Miner.
 */
class Miner_schedule : public Tool_schedule {
	Game_object *ore;
	enum {
	    find_ore,
	    attack_ore,
	    ore_attacked,
	    wander
	} state;
public:
	Miner_schedule(Actor *n) : Tool_schedule(n, 624),
		ore(0), state(find_ore)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Hound the Avatar.
 */
class Hound_schedule : public Schedule {
public:
	Hound_schedule(Actor *n) : Schedule(n)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Wander all over the place, using pathfinding.
 */
class Wander_schedule : public Loiter_schedule {
public:
	Wander_schedule(Actor *n) : Loiter_schedule(n, 128)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Sleep in a  bed.
 */
class Sleep_schedule : public Schedule {
	Tile_coord floorloc;        // Where NPC was standing before.
	Game_object *bed;       // Bed being slept on, or 0.
	int state;
	int spread0, spread1;       // Range of bedspread frames.
	bool for_nap_time;
public:
	Sleep_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	// Set where to sleep.
	virtual void set_bed(Game_object *b);
	virtual void notify_object_gone(Game_object *obj);
	virtual void im_dormant();  // Just went dormant.
	static bool is_bed_occupied(Game_object *bed, Actor *npc);
};

/*
 *  Sit in a chair.
 */
class Sit_schedule : public Schedule {
	Game_object *chair;     // What to sit in.
	bool sat;           // True if we already sat down.
	bool did_barge_usecode;     // So we only call it once.
public:
	Sit_schedule(Actor *n, Game_object *ch = 0);
	virtual void now_what();    // Now what should NPC do?
	virtual void notify_object_gone(Game_object *obj);
	virtual void im_dormant();  // Just went dormant.
	static bool is_occupied(Game_object *chairobj, Actor *actor);
	static bool set_action(Actor *actor, Game_object *chairobj = 0,
	                       int delay = 0, Game_object **chair_found = 0);
};

/*
 *  Desk work - Just sit in front of desk.
 */
class Desk_schedule : public Schedule {
	Game_object *chair;     // What to sit in.
	Game_object *desk;
	vector<Game_object *> tables;	// Other tables to work at.
	enum {
	    desk_setup,
	    sit_at_desk,
		work_at_table
	} state;
	void find_tables(int shapenum);
public:
	Desk_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	virtual void notify_object_gone(Game_object *obj);
	virtual void im_dormant();  // Just went dormant.
};

/*
 *  Shy away from Avatar.
 */
class Shy_schedule : public Schedule {
public:
	Shy_schedule(Actor *n) : Schedule(n)
	{  }
	virtual void now_what();    // Now what should NPC do?
};

/*
 *  Lab work.
 */
class Lab_schedule : public Schedule {
	vector<Game_object *> tables;
	Game_object *chair;     // Chair to sit in.
	Game_object *book;      // Book to read.
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
	virtual void now_what();    // Now what should NPC do?
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Be a thief.
 */
class Thief_schedule : public Schedule {
	unsigned long next_steal_time;  // Next time we can try to steal.
	void steal(Actor *from);
public:
	Thief_schedule(Actor *n) : Schedule(n), next_steal_time(0)
	{  }
	virtual void now_what();
};

/*
 *  Wait tables.
 */
class Waiter_schedule : public Schedule {
	Tile_coord startpos;        // Starting position.
	Actor *customer;        // Current customer.
	Game_object *prep_table;    // Table we're working at.
	vector<Actor *> customers;  // List of customers.
	vector<Game_object *> prep_tables; // Prep. tables.
	vector<Game_object *> eating_tables; // Tables with chairs around them.
	enum {
	    waiter_setup,
	    get_customer,
	    get_order,
	    prep_food,
	    serve_food
	} state;
	bool find_customer();
	void find_tables(int shapenum);
	bool walk_to_customer(int min_delay = 0);
	bool walk_to_prep();
	Game_object *find_serving_spot(Tile_coord &spot);
public:
	Waiter_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Sew/weave schedule.
 */
class Sew_schedule : public Schedule {
	Game_object *bale;      // Bale of wool.
	Game_object *spinwheel;
	Game_object *chair;     // In front of spinning wheel.
	Game_object *spindle;       // Spindle of thread.
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
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype);// Switching to another schedule.
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Bake schedule
 */
class Bake_schedule : public Schedule {
	Game_object *oven;
	Game_object *worktable;
	Game_object *displaytable;
	Game_object *flourbag;
	Game_object *dough;
	Game_object *dough_in_oven;
	bool clearing;
	enum {
	    find_leftovers,     // Look for misplaced dough already made by this schedule
	    to_flour,           // Looks for flourbag and walks to it if found
	    get_flour,          // Bend over flourbag and change the frame to zero if nonzero
	    to_table,           // Walk over to worktable and create flour
	    make_dough,         // Changes flour to flat dough then dough ball
	    remove_from_oven,   // Changes dough in oven to food %7 and picks it up
	    display_wares,      // Walk to displaytable. Put food on it. If table full, go to
	    // clear_display which eventualy comes back here to place food
	    clear_display,      // Mark food for deletion by remove_food
	    remove_food,        // Delete food on display table one by one with a slight delay
	    get_dough,          // Walk to work table and pick up dough
	    put_in_oven         // Walk to oven and put dough on in.
	} state;
public:
	Bake_schedule(Actor *n);
	virtual void now_what();
	virtual void ending(int newtype);
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Blacksmith schedule
 */
class Forge_schedule : public Schedule {
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
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype); // Switching to another schedule
	virtual void notify_object_gone(Game_object *obj);
};

/*
 *  Eat without a server
 */
class Eat_schedule : public Schedule {
	Game_object *plate;
	enum {
	    eat,        // eat food and say food barks
	    find_plate, // make sure there is a plate, create one if not
	    serve_food  // put food on the plate
	} state;
public:
	Eat_schedule(Actor *n);
	virtual void now_what();    // Now what should NPC do?
	virtual void ending(int newtype); // Switching to another schedule
	virtual void notify_object_gone(Game_object *obj);
	virtual void im_dormant();  // Just went dormant.
};

/*
 *  Walk to the destination for a new schedule.
 */
class Walk_to_schedule : public Schedule {
	Tile_coord dest;        // Where we're going.
	int first_delay;        // Starting delay (1/1000's sec.)
	int new_schedule;       // Schedule to set when we get there.
	int retries;            // # failures at finding path.
	int legs;           // # times restarted walk.
	// Set to walk off screen.
	void walk_off_screen(Rectangle &screen, Tile_coord &goal);
public:
	Walk_to_schedule(Actor *n, Tile_coord const &d, int new_sched,
	                 int delay = -1);
	virtual void now_what();    // Now what should NPC do?
	virtual void im_dormant();  // Just went dormant.
	// For Usecode intrinsic.
	virtual int get_actual_type(Actor *npc) const;
};

/*
 *  An NPC schedule change:
 */
class Schedule_change {
	static vector<char *> script_names; // For Scripted_schedule's.
	unsigned char time;     // Time*3hours when this takes effect.
	unsigned char type;     // Schedule_type value.
	unsigned char days;     // A bit for each day (0-6).  We don't
	//   yet use this.
	Tile_coord pos;         // Location.
public:
	Schedule_change() : time(0), type(0), days(0x7f)
	{  }
	static void clear();
	static vector<char *> &get_script_names() {
		return script_names;
	}
	void set4(unsigned char *ent);  // Create from 4-byte entry.
	void set8(unsigned char *ent);  // Create from Exult entry (v. -1).
	void write8(unsigned char *ent) const;// Write out 8-byte Exult entry.
	void set(int ax, int ay, int az,
	         unsigned char stype, unsigned char stime);
	int get_type() const {
		return type;
	}
	int get_time() const {
		return time;
	}
	Tile_coord get_pos() const {
		return pos;
	}
	static char *get_script_name(int ty) {
		return ty >= Schedule::first_scripted_schedule ?
		       script_names[ty - Schedule::first_scripted_schedule] : 0;
	}
};

#endif
