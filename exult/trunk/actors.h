/*
 *	actors.h - Game actors.
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

#ifndef INCL_ACTORS
#define INCL_ACTORS	1

#include "contain.h"
#include "utils.h"		// This is only included for Log2...
#include "flags.h"

class Image_window;
class Game_window;
class Npc_actor;
class Actor_action;
class Schedule;
class Schedule_change;
class Monster_info;
class Weapon_info;
class Dead_body;
class Npc_timer_list;
class Frames_sequence;
class Animator;
					// The range of actors' rect. gumps:
const int ACTOR_FIRST_GUMP = 57, ACTOR_LAST_GUMP = 68;

/*
 *	An actor:
 */
class Actor : public Container_game_object, public Time_sensitive
	{
	static Actor *editing;		// NPC being edited by ExultStudio.
protected:
	std::string name;			// Its name.
	int usecode;			// # of usecode function.
	short npc_num;			// # in Game_window::npcs list, or -1.
	short face_num;			// Which shape for conversations.
	short party_id;			// Index in party, or -1.
	short properties[12];		// Properties set/used in 'usecode'.
	unsigned char temperature;	// Measure of coldness (0-63).
	short shape_save;		// Our old shape, or -1.
	short oppressor;		// NPC ID (>= 0) of oppressor, or -1.
	Game_object *target;		// Who/what we're attacking.
public:
	enum Attack_mode {		// Setting from gump.+++++Save/restore.
		nearest = 0,
		weakest = 1,		// Attack weakest.
		strongest = 2,
		beserk = 3,		// Always attack, never retreat.
		protect = 4,		// Protect NPC with halo.
		defend = 5,
		flank = 6,		// Attempt to attack from side.
		flee = 7,
		random = 8,		// Choose target at random.
		manual = 9
		};
protected:
					// Party positions
	const static short party_pos[4][10][2];

	Attack_mode attack_mode;
					// A frame sequence for each dir.:
	static Frames_sequence *frames[8];
					// Draw weapon in hand
	void paint_weapon(Game_window *gwin);
	unsigned char schedule_type;	// Schedule type (Schedule_type).
	Tile_coord schedule_loc;	// Location (x,y) of Shedule
	unsigned char next_schedule;	// Used so correct schedule type 
					//   will be saved
	Schedule *schedule;		// Current schedule.
	bool dormant;			// I.e., off-screen.
	bool hit;			// Just hit in combat.
	bool combat_protected;		// 'Halo' on paperdoll screen.
	bool user_set_attack;		// True if player set attack_mode.
	short alignment;		// 'Feelings' towards Ava. See below.
	Game_object *spots[18];		// Where things can go.  See 'Spots'
					//   below for description.
	bool two_handed;		// Carrying a two-handed item.
	bool two_fingered;		// Carrying gauntlets (both fingers)
	unsigned char light_sources;	// # of light sources readied.
	unsigned char usecode_dir;	// Direction (0-7) for usecode anim.
	unsigned siflags:32;	// 32 flags used in 'usecode'.
	unsigned type_flags:32;	// 32 flags used in movement among other things

	unsigned char ident;

	int	skin_color;
	Actor_action *action;		// Controls current animation.
	int frame_time;			// Time between frames in msecs.  0 if
					//   actor not moving.
	unsigned long next_path_time;	// Next time we're allowed to use
					//   pathfinding to follow leader.
	Npc_timer_list *timers;		// Timers for poison, hunger, etc.
	Rectangle weapon_rect;		// Screen area weapon was drawn in.
	void init();			// Clear stuff during construction.
	int is_blocked(Tile_coord& t);	// Blocked moving onto tile 't'?
					// Move and change frame.
	void movef(Map_chunk *old_chunk, Map_chunk *new_chunk, 
		int new_sx, int new_sy, int new_frame, int new_lift);
					// Read from file.
	Actor(std::istream& nfile, int num, int has_usecode);
public:
	friend class Clear_hit;
	static void init_default_frames();	// Set usual frame sequence.
	Actor(const std::string &nm, int shapenum, int num = -1, int uc = -1);
	~Actor();
	int ready_ammo();		// Find and ready appropriate ammo.
	void ready_best_weapon();	// Find best weapon and ready it.
	void unready_weapon(int spot);	// Try to sheath weapon.
					// Force repaint of area taken.
	int add_dirty(Game_window *gwin, int figure_rect = 0);
	int figure_weapon_pos(int& weapon_x, int& weapon_y, int& weapon_frame);
	void use_food();		// Decrement food level.
					// Increment/decrement temperature.
	void check_temperature(bool freeze);
					// Get frame seq. for given dir.
	static Frames_sequence *get_frames(int dir)
		{ return frames[dir]; }
					// Get attack frames.
	int get_attack_frames(int dir, char *frames) const;
	enum Alignment {		// Describes alignment field.
		neutral = 0,
		friendly = 1,
		hostile = 2,
		unknown_align = 3 };	// Bees have this, & don't attack until
					// Spots where items are carried.
	enum Spots {			// Index of each spot, starting at
					//   upper, rt., going clkwise.
		head = 0,
		back = 1,
		belt = 2,
		lhand = 3,
		lfinger = 4,
		legs = 5,
		feet = 6,
		rfinger = 7,
		rhand = 8,
		torso = 9,
		neck = 10,
		ammo = 11,
		back2h_spot = 12,	// SI (2 Handed weapons, Bedroll, 
					//   Bodies)
		shield_spot = 13,	// SI (Sheild behind Backpack)
		ears_spot = 14,		// SI
		cloak_spot = 15,	// SI
		hands2_spot = 16,	// SI (gloves, gauntlets)
		ucont_spot = 17,	// SI Usecode Container
		lrhand = 100,		// Special:  uses lhand & rhand. - Used anymore?
		lrfinger = 101,		// Special:  uses lfinger & rfinger - Used anymore?
		special_spot = 102	// Special:  SI non placeable
		};
	int free_hand() const		// Get index of a free hand, or -1.
		{ 			// PREFER right hand.
		return two_handed ? -1 :
			(!spots[rhand] ? rhand : (!spots[lhand] ? lhand : -1));
		}
	int free_finger() const		// Get index of a free finger, or -1.
		{ 
		return two_fingered ? -1 :
			(!spots[lfinger] ? lfinger
			 	: (!spots[rfinger] ? rfinger : -1));
		}
	int has_light_source() const	// Carrying a torch?
		{ return light_sources > 0; }
	Attack_mode get_attack_mode()
		{ return attack_mode; }
	void set_attack_mode(Attack_mode amode, bool byuser = false)
		{ attack_mode = amode; user_set_attack = byuser; }
	bool did_user_set_attack() const
		{ return user_set_attack; }
	bool is_combat_protected() const
		{ return combat_protected; }
	void set_combat_protected(bool v)
		{ combat_protected = v; }
	int get_oppressor() const
		{ return oppressor; }
	void set_oppressor(int opp)
		{ oppressor = opp; }
	// This is not even a guess, it's a place holder
	enum Serpent_flags {		// Bit #'s of flags:
		// petra = 4,
		// met = 5,
		no_spell_casting = 6,
		zombie = 7,
		naked = 8,
		dont_move = 9
		};
	enum type_flags {
		tf_fly = 4,
		tf_walk = 5,
		tf_swim = 6,
		tf_ethereal = 7,
		tf_want_primary = 8,
		tf_sex = 9,
		tf_bleeding = 10,
		tf_in_party = 12,
		tf_in_action = 13,
		tf_conjured = 14,
		tf_summonned = 15
	};
	enum Item_properties {		// Trying to figure out properties.
		strength = 0,		// This is also max. health.
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
	enum Frames {			// Frames 0-15.  16-31 are the same,
					//   only S instead of N.
		standing = 0,
		sit_frame = 10,
		to_sit_frame = 11,
		sleep_frame = 13
		};
	enum FIS_Type {			// The types used in the call to fit_in_spot
		FIS_Other	= 0,
		FIS_2Hand	= 1,
		FIS_2Finger	= 2,
		FIS_Spell	= 3
		};

	int get_face_shapenum() const	// Get "portrait" shape #.
		{ return face_num; }	// It's the NPC's #.
	int get_usecode() const
		{ return usecode == -1 ? get_shapenum() : usecode; }
	Schedule *get_schedule() const
		{ return schedule; }
	int get_frame_time() const	// Return frame time if moving.
		{ return frame_time; }
	void set_frame_time(int ftime)	// Set walking speed.
		{ frame_time = ftime; }
	bool is_moving() const
		{ return frame_time != 0; }
	bool is_dormant() const		// Inactive (i.e., off-screen)?
		{ return dormant; }
	bool is_dead() const
		{ return (flags&(1<<Obj_flags::dead)) != 0; }
	void set_dormant()
		{ dormant = true; }
	Actor_action *get_action()	// Return action.
		{ return action; }
					// Set new action.
	void set_action(Actor_action *newact);
	Tile_coord get_dest();		// Get destination.
					// Walk to a desired spot.
	void walk_to_tile(Tile_coord dest, int speed = 250, int delay = 0);
	void walk_to_tile(int tx, int ty, int tz, int speed = 250, 
							int delay = 0)
		{ walk_to_tile(Tile_coord(tx, ty, tz), speed, delay); }
					// Get there, avoiding obstacles.
	int walk_path_to_tile(Tile_coord src, Tile_coord dest, 
				int speed = 250, int delay = 0, int dist = 0);
	int walk_path_to_tile(Tile_coord dest, 
				int speed = 250, int delay = 0, int dist = 0)
		{ return walk_path_to_tile(get_abs_tile_coord(), dest,
							speed, delay, dist); }
					// Start animation.
	void start(int speed = 250, int delay = 0);
	void stop();			// Stop animation.
	void follow(Actor *leader);	// Follow the leader.
					// Approach another from offscreen.
	int approach_another(Actor *other, bool wait = false);
					// Get info. about tile to step onto.
	static void get_tile_info(Actor *actor,
		Game_window *gwin, Map_chunk *nlist,
				int tx, int ty, int& water, int& poison);
					// Set combat opponent.
	void set_target(Game_object *obj, bool start_combat = false);
	Game_object *get_target()	// Get who/what we're attacking.
		{ return target; }
					// Works out if an object fits in a spot
	bool fits_in_spot (Game_object *obj, int spot, FIS_Type type = FIS_Other);
					// The prefered slot for an object
	void get_prefered_slots (Game_object *obj, int &prefered, int &alternate, FIS_Type &fistype);
					// Find where to put object.
	int find_best_spot(Game_object *obj);
	int get_prev_schedule_type();	// Get previous schedule.
	void restore_schedule();	// Set schedule after reading in.
					// Set new schedule.
	virtual void set_schedule_type(int new_schedule_type, 
						Schedule *newsched = 0);
					// Change to new schedule at loc
	virtual void set_schedule_and_loc(int new_schedule_type, 
					Tile_coord dest, int delay = -1);
	virtual int get_schedule_type() const
		{ return schedule_type; }
					// Get/set 'alignment'.
	virtual int get_alignment() const
		{ return alignment; }
	virtual void set_alignment(short a)
		{ alignment = a; }
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
					// Saved from ExultStudio.
	static void update_from_studio(unsigned char *data, int datalen);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
	virtual std::string get_name() const;
	std::string get_npc_name() const;
	void set_npc_name(const char *n);
	virtual void set_property(int prop, int val);
					// Lose HP's and check for death.
	bool reduce_health(int delta, Actor *attacker = 0);
	virtual int get_property(int prop) const
		{ return (prop >= 0 && prop < 12) ? properties[prop] : 0; }
	bool is_dying() const		// Dead when health below -1/3 str.
		{ return properties[(int) health] < 
					-(properties[(int) strength]/3); }
	int get_level() const		// Get experience level.
		{ return 1 + Log2(get_property(exp)/50); }
	Npc_timer_list *need_timers();
					// Set/clear/get actor flag.
	virtual void set_flag(int flag);
	virtual void set_siflag(int flag);
	void set_type_flag(int flag);
	virtual void clear_flag(int flag);
	virtual void clear_siflag(int flag);
	void clear_type_flag(int flag);
	virtual int get_siflag(int flag) const;
	int get_type_flag(int flag) const;
	void set_type_flags(unsigned short tflags);
	int get_skin_color () const { return skin_color; }
	void set_skin_color (int color) { skin_color = color; set_actor_shape();}
	virtual int get_type_flags() const
		{ return type_flags; }

	virtual unsigned char get_ident() { return is_dead() ? 0 : ident; }
	virtual void set_ident(unsigned char id) { ident = id; }

	int get_temperature() const	// Get/set measure of coldness.
		{ return temperature; }
	void set_temperature(int t);
	int figure_warmth();		// Based on what's worn.

	virtual int get_npc_num() const	// Get its ID (1-num_npcs).
		{ return npc_num; }
					// Get/set index within party.
	virtual int get_party_id() const
		{ return party_id; }
	virtual void set_party_id(int i)
		{ party_id = i; }
					// Set for Usecode animations.
	virtual void set_usecode_dir(int d)
		{ usecode_dir = d&7; }
	virtual int get_usecode_dir() const
		{ return usecode_dir; }
	void init_readied();		// Call Usecode to init. readied objs.
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Add to NPC 'readied' spot.
	virtual int add_readied(Game_object *obj, int index, int dont_check = 0, int force_pos = 0);
	virtual int find_readied(Game_object *obj);
	virtual Game_object *get_readied(int index) const
		{
		return index >= 0 && 
			index < (int)(sizeof(spots)/sizeof(spots[0])) ? 
				spots[index] : 0; 
		}
	virtual void call_readied_usecode(Game_window *gwin, int index,
					Game_object *obj, int eventid);
	virtual int get_max_weight();	// Get max. weight allowed.
					// Change member shape.
	virtual void change_member_shape(Game_object *obj, int newshape);
					// Move out of the way.
	virtual int move_aside(Actor *for_actor, int dir);
					// Get frame if rotated clockwise.
	virtual int get_rotated_frame(int quads);
	virtual int get_armor_points();	// Get total armor value.
					// Get total weapon value.
	virtual Weapon_info *get_weapon(int& points, int& shape);	
	Weapon_info *get_weapon(int& points)
		{ int sh; return get_weapon(points, sh); }
	static bool roll_to_win(int attacker, int defender);
					// Hit-point algorithm:
	bool figure_hit_points(Actor *attacker, int weapon_shape, 
							int ammo_shape);
					// Under attack.
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
					int ammo_shape = 0);
	virtual void die();		// We're dead.
	Actor *resurrect(Dead_body *body);// Bring back to life.
	void mend_hourly();		// Restore HP's hourly.
					// Don't write out to IREG file.
	virtual void write_ireg(std::ostream& out)
		{  }
	void write(std::ostream& nfile);		// Write out (to 'npc.dat').
	virtual void write_contents(std::ostream& out);	// Write contents
	void set_actor_shape(); 	// Set shape based on sex and skin color
	void set_polymorph(int shape);	// Set a polymorph shape
	void set_polymorph_default();	// Set the default shape
					// Get the polymorph shape
	int get_polymorph () { return shape_save; }
	int get_shape_real();		// Get the non polymorph shape

	// Set schedule list.
	virtual void set_schedules(Schedule_change *list, int cnt) { }
	virtual void set_schedule_time_type(int time, int type) { }
	virtual void set_schedule_time_location(int time, int x, int y) { }
	virtual void remove_schedule(int time) { }
	virtual void get_schedules(Schedule_change *&list, int &cnt)
	{ list = NULL, cnt = 0; }

	void show_inventory();
	int inventory_shapenum();

	bool was_hit() { return hit; }

	virtual bool update_forced_schedule() { return false; }
	};

/*
 *	Actor frame descriptions:
	0	Standing
	1	Walk
	2	Walk
	3	Beginning to attack
	4-6	Attacking with one hand
	7-9	Attacking with two hands
	9	Also NPC shooting magic
	10	Sitting down
	11	Bending over (beginning to sit down)
	12	Kneeling
	11	Lying down
	14	Casting spell (hands raised)
	15	Casting spell (hands outstretched)

 */

/*
 *	The main actor.
 */
class Main_actor : public Actor
	{
public:
	Main_actor(const std::string &nm, int shapenum, int num = -1, int uc = -1)
		: Actor(nm, shapenum, num, uc)
		{  }
					// Read from file.
	Main_actor(std::istream& nfile, int num, int has_usecode);
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
	void get_followers();		// Get party to follow.
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame);
					// Update chunks after NPC moved.
	void switched_chunks(Map_chunk *olist,
					Map_chunk *nlist);
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	virtual void die();		// We're dead.
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
	unsigned char num_schedules;	// # entries below.
	bool force_update;		// Force schedule update
	Schedule_change *schedules;	// List of schedule changes.
	int find_schedule_change(int hour3);
public:
	Npc_actor(const std::string &nm, int shapenum, int num = -1, 
								int uc = -1);
					// Read from file.
	Npc_actor(std::istream& nfile, int num, int has_usecode);
	~Npc_actor();
					//   Usecode tells them to.
	Npc_actor *get_next()
		{ return next; }
	void set_nearby()		// Set/clear/test 'nearby' flag.
		{ nearby = true; }
	void clear_nearby()
		{ nearby = false; }
	bool is_nearby() const
		{ return nearby!=0; }
					// Set schedule list.
	virtual void set_schedules(Schedule_change *list, int cnt);
	virtual void set_schedule_time_type(int time, int type);
	virtual void set_schedule_time_location(int time, int x, int y);
	virtual void remove_schedule(int time);
	virtual void get_schedules(Schedule_change *&list, int &cnt);
					// Move and change frame.
	void movef(Map_chunk *old_chunk, Map_chunk *new_chunk, 
		int new_sx, int new_sy, int new_frame, int new_lift);
					// Update schedule for new 3-hour time.
	void update_schedule(Game_window *gwin, int hour3, int backwards = 0,
							int delay = -1);
	virtual bool update_forced_schedule();
	void set_force_update() { force_update = true; }
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame);
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
					// Update chunks after NPC moved.
	void switched_chunks(Map_chunk *olist,
					Map_chunk *nlist);
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	};

/*
 *	An actor's dead body:
 */
class Dead_body : public Container_game_object
	{
	short npc_num;			// # of NPC it came from, or -1.
public:
	Dead_body(int shapenum, int framenum, unsigned int tilex, 
		unsigned int tiley, unsigned int lft, int n)
		: Container_game_object(shapenum, framenum, tilex, tiley, lft),
			npc_num(n)
		{
		}
	virtual ~Dead_body();
	virtual int get_live_npc_num();
					// Under attack.
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
					int ammo_shape = 0)
		{ return this; }	// Not affected.
	};

/*
 *	Monsters get their own class because they have a bigger footprint
 *	than humans.
 */
class Monster_actor : public Npc_actor
	{
	static Monster_actor *in_world;	// All monsters in the world.
	static int in_world_cnt;	// # in list.
					// Links for 'in_world' list.
	Monster_actor *next_monster, *prev_monster;
	Animator *animator;		// For wounded men.
	void init();			// For constructors.
public:
	Monster_actor(const std::string &nm, int shapenum, int num = -1, 
							int uc = -1);
					// Read from file.
	Monster_actor(std::istream& nfile, int num, int has_usecode);
	virtual ~Monster_actor();
					// Create an instance.
	static Monster_actor *create(int shnum, int chunkx, int chunky, 
		int tilex, int tiley, 
		int lift, int sched = -1, int align = (int) Actor::neutral, 
					bool tempoary = true, bool equipment = true);
					// Methods to retrieve them all:
	static Monster_actor *get_first_in_world()
		{ return in_world; }
	Monster_actor *get_next_in_world()
		{ return next_monster; }
	static int get_num_in_world()
		{ return in_world_cnt; }
	static void delete_all();	// Delete all monsters.
	static void give_up()		// For file errors only!
		{ in_world = 0; in_world_cnt = 0; }
	virtual int move_aside(Actor* for_actor, int dir)
		{ return 0; }		// Monsters don't move aside.
					// Render.
	virtual void paint(Game_window *gwin);
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
	virtual int get_armor_points();	// Get total armor value.
					// Get total weapon value.
	virtual Weapon_info *get_weapon(int& points, int& shape);
	virtual int is_monster()
		{ return 1; }
	virtual void die();		// We're dead.
	void write(std::ostream& nfile);// Write out (to 'monsnpc.dat').
	};

#endif
