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
#include "utils.h"

class Image_window;
class Game_window;
class Npc_actor;
class Actor_action;
					// The range of actors' rect. gumps:
const int ACTOR_FIRST_GUMP = 57, ACTOR_LAST_GUMP = 68;

/*
 *	An actor:
 */
class Actor : public Sprite
	{
	char *name;			// Its name.
	int usecode;			// # of usecode function.
	short npc_num;			// # in Game_window::npcs list, or -1.
	short party_id;			// Index in party, or -1.
	short properties[12];		// Properties set/used in 'usecode'.
protected:
	Game_object *spots[12];		// Where things can go.  See 'Spots'
					//   below for description.
	unsigned char two_handed;	// Carrying a two-handed item.
	unsigned char usecode_dir;	// Direction (0-7) for usecode anim.
	unsigned long flags;		// 32 flags used in 'usecode'.
	Actor_action *action;		// Controls current animation.
public:
	void set_default_frames();	// Set usual frame sequence.
	Actor(char *nm, int shapenum, int num = -1, int uc = -1);
	~Actor();
					// Spots where items are carried.
	enum Spots {			// Index of each spot, starting at
					//   upper, rt., going clkwise.
		head = 0,
		torso = 1,
		belt = 2,
		lhand = 3,
		lfinger = 4,
		legs = 5,
		feet = 6,
		rfinger = 7,
		rhand = 8,
		arms = 9,
		neck = 10,
		back = 11,
		lrhand = 100		// Special:  uses lhand & rhand.
		};
	int free_hand()			// Get index of a free hand, or -1.
		{ 
		return two_handed ? -1 :
			(!spots[rhand] ? rhand : (!spots[lhand] ? lhand : -1));
		}
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
					// Set new action.
	void set_action(Actor_action *newact);
					// Walk to a desired spot.
	void walk_to_tile(int tx, int ty, int tz, int speed = 250, 
							int delay = 0);
	void walk_to_tile(Tile_coord p, int speed = 250, int delay = 0)
		{ walk_to_tile(p.tx, p.ty, p.tz, speed, delay); }
					// Walk to desired point.
	void walk_to_point(unsigned long destx, unsigned long desty, 
								int speed);
					// Find where to put object.
	int find_best_spot(Game_object *obj);
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
	int get_level()			// Get experience level.
		{ return 1 + Log2(get_property(exp)/50); }
					// Set/clear/get actor flag.
	virtual void set_flag(int flag);
	virtual void clear_flag(int flag);
	virtual int get_flag(int flag);
	virtual int get_npc_num()	// Get its ID (1-num_npcs).
		{ return npc_num; }
	virtual int get_party_id()	// Get/set index within party.
		{ return party_id; }
	virtual void set_party_id(int i)
		{ party_id = i; }
					// Set for Usecode animations.
	virtual void set_usecode_dir(int d)
		{ usecode_dir = d&7; }
	virtual int get_usecode_dir()
		{ return usecode_dir; }
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Add to NPC 'readied' spot.
	virtual int add_readied(Game_object *obj, int index);
	virtual int find_readied(Game_object *obj);
	virtual Game_object *get_readied(int index)
		{
		return index >= 0 && index < sizeof(spots)/sizeof(spots[0]) ? 
				spots[index] : 0; 
		}
#if 0	/* ++++++ Trying to init. 1st-day schedules in gameclk.cc. */
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
#endif
	virtual int walk()		// Walk towards a direction.
		{ return 0; }
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame)
		{ return 0; }
	};

/*
 *	Actor frame descriptions:
	0	Standing N.
	1	Walk N.
	2	Walk N.
	3-4	Walk NW
	5-6	Walk NE.
	9	Facing N, hands raised.
	10	Sitting down N.
	11-13	Lying down N.
	14	Hands raised, N.
	15	Hands outstretched, N.
	16	Standing S.
	17,18	Walking S.
	19,20	Walking SW.
	21,22	Walking SE.
	26	Sitting S.
	27-29	Lying down S.
	30	Hands raised, S.
	31	Hands outstretched, S.

These were contributed by a user:
0-2: walking
3-9: attacking
10: sitting
11: beginning to sit down
12: kneeling
13: sleeping
14-15: casting spells

 */

/*
 *	The main actor.
 */
class Main_actor : public Actor
	{
public:
	Main_actor(char *nm, int shapenum, int num = -1, int uc = -1)
		: Actor(nm, shapenum, num, uc)
		{  }
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
	void get_followers();		// Get party to follow.
	virtual int walk();		// Walk towards a direction.
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame);
					// Update chunks after NPC moved.
	void switched_chunks(Chunk_object_list *olist,
					Chunk_object_list *nlist);
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
	virtual ~Patrol_schedule();
	};

/*
 *	Talk to avatar.
 */
class Talk_schedule : public Schedule
	{
	int phase;			// 0=walk to Av., 1=talk, 2=done.
public:
	Talk_schedule(Npc_actor *n) : Schedule(n), phase(0)
		{  }
	virtual void now_what();	// Now what should NPC do?
	};

/*
 *	Loiter within a rectangle.
 */
class Loiter_schedule : public Schedule
	{
	Tile_coord center;		// Center of rectangle.
public:
	Loiter_schedule(Npc_actor *n);
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
 *	A non-player-character that one can converse (or fight) with:
 */
class Npc_actor : public Actor
	{
	Npc_actor *next;		// Next in same chunk.
	unsigned char nearby;		// Queued as a 'nearby' NPC.  This is
					//   to avoid being added twice.
protected:
	unsigned char dormant;		// I.e., off-screen.
	unsigned char schedule_type;	// Schedule type (Schedule_type).
	unsigned char num_schedules;	// # entries below.
	Schedule *schedule;		// Current schedule.
	Schedule_change *schedules;	// List of schedule changes.
	short alignment;		// 'Feelings' towards Avatar.
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
	virtual int get_alignment()	// Get/set 'alignment'.
		{ return alignment; }
	virtual void set_alignment(short a)
		{ alignment = a; }
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
	virtual int walk();		// Walk towards a direction.
	void follow(Actor *leader);	// Follow the leader.
					// Update chunks after NPC moved.
	void switched_chunks(Chunk_object_list *olist,
					Chunk_object_list *nlist);
	};

/*
 *	Monsters get their own class because they have a bigger footprint
 *	than humans.
 */
class Monster_actor : public Npc_actor
	{
					// Are new tiles blocked?
	int is_blocked(int destx, int desty);
public:
	Monster_actor(char *nm, int shapenum, int fshape = -1, int uc = -1)
		: Npc_actor(nm, shapenum, fshape, uc)
		{  }
	virtual int walk();		// Walk towards a direction.
	};

/*
 *	Monster info. from 'monsters.dat':
 */
class Monster_info
	{
	int shapenum;			// Shape #.
	unsigned char strength;		// Attributes.
	unsigned char dexterity;
	unsigned char intelligence;
	unsigned char combat;
	unsigned char armor;
public:
	Monster_info() {  }
	int get_shapenum()
		{ return shapenum; }
	void set(int sh, int str, int dex, int intel, int comb, int ar)
		{
		shapenum = sh;
		strength = str;
		dexterity = dex;
		intelligence = intel;
		combat = comb;
		armor = ar;
		}
					// Create an instance.
	Npc_actor *create(int chunkx, int chunky, int tilex, int tiley, 
								int lift);
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
