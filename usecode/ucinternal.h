/*
 *	ucinternal.h - Interpreter for usecode.
 *
 *	Usecode_internal is the implementation, so this header should only
 *	be included within .cc's in the 'usecode' directory.
 *
 *
 *  Copyright (C) 2001-2002  The Exult Team
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

#ifndef _UCINTERNAL_H
#define _UCINTERNAL_H

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

#include "exult_types.h"

class Actor;
class Barge_object;
class Npc_actor;
class Usecode_value;
class Text_gump;
class Vector;
class Stack_frame;
class Usecode_function;

#ifdef MACOS
// With Metrowerks Codewarrior, we *have* to include useval.h, otherwise this will fail to compile:
// ...
// 	typedef Usecode_value (Usecode_internal::*UsecodeIntrinsicFn)(
//		int event,int intrinsic,int num_parms,Usecode_value parms[12]);
// ...
// This might be a problem on other compilers besides CodeWarrior (thoug it works with gcc 2.95)
// This is *not* a bug in CW! It is a "feature" of g++ that it can work without!
#include "useval.h"
#endif

#include "ucmachine.h"
#include "ucdebugging.h"
#include "tiles.h"
#include "vec.h"	// Includes STL vector.
#include <string>	// STL string
#include <deque>

/*
 *	Recursively look for a barge that an object is a part of, or on.
 *
 *	Output:	->barge if found, else 0.
 */

Barge_object *Get_barge
	(
	Game_object *obj
	);

#define	USECODE_INTRINSIC_DECL(NAME)	Usecode_value UI_## NAME (int event,int intrinsic,int num_parms,Usecode_value parms[12])

/*
 *	Here's our virtual machine for running usecode.
 */
class Usecode_internal : public Usecode_machine
	{
					// I'th entry contains funs for ID's
					//    256*i + n.
	Exult_vector<Usecode_function*> funs[16];

	std::deque<Stack_frame*> call_stack; // the call stack
	Usecode_function *cur_function;	// Current function being executed.
	unsigned long timers[20];	// Each has time in hours when set.
	int speech_track;		// Set/read by some intrinsics.
	Text_gump *book;		// Book/scroll being displayed.
	Game_object *caller_item;	// Item this is being called on.
	Game_object_vector last_created;// Stack of last items created with 
					//   intrins. x24.
	Actor *path_npc;		// Last NPC in path_run_usecode().
	const char *user_choice;	// String user clicked on.
	bool found_answer;		// Did we already handle the 
					//   conversation option?
	Tile_coord saved_pos;		// For a couple SI intrinsics.
	char *String;			// The single string register.
	int telekenesis_fun;		// For next Usecode call from spell.
	void append_string(const char *txt);	// Append to string.
	void show_pending_text();	// Make sure user's seen all text.
	void show_book();		// "Say" book/scroll text.
	void say_string();		// "Say" the string.
	Usecode_value *stack;		// Stack.
	Usecode_value *sp;		// Stack ptr.  Grows upwards.
	void stack_error(int under);
	void push(Usecode_value& val);	// Push/pop stack.
	Usecode_value pop();
	void pushref(Game_object* obj); // Push itemref
	void pushi(long val);		// Push/pop integers.
	int popi();
					// Push/pop strings.
	void pushs(char *s);
					// Get ->obj. from 'itemref'.
	Game_object *get_item(Usecode_value& itemref);
					// "Safe" cast to Actor and Npc_actor.
	Actor *as_actor(Game_object *obj);
					// Get position.
	Tile_coord get_position(Usecode_value& itemref);
	/*
	 *	Built-in usecode functions:
	 */
	typedef Usecode_value (Usecode_internal::*UsecodeIntrinsicFn)(
		int event,int intrinsic,int num_parms,Usecode_value parms[12]);

	void show_npc_face(Usecode_value& arg1, Usecode_value& arg2,
							int slot = -1);
	void remove_npc_face(Usecode_value& arg1);
	void set_item_shape(Usecode_value& item_arg, Usecode_value& shape_arg);
	void set_item_frame(Game_object *item, int frame, int check_empty = 0,
							int set_rotated = 0);
	void remove_item(Game_object *obj);
	Usecode_value get_party();
	void item_say(Usecode_value& objval, Usecode_value& strval);
	void activate_cached(Tile_coord pos);
	Usecode_value find_nearby(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qval,
							Usecode_value& mval);
	Usecode_value find_nearest(Usecode_value& objval,
			Usecode_value& shapeval, Usecode_value& unknown);
	Usecode_value find_direction(Usecode_value& from, Usecode_value& to);
	Usecode_value count_objects(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qualval,
						Usecode_value& frameval);
	Usecode_value get_objects(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qualval,
						Usecode_value& frameval);
	Usecode_value remove_party_items(Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);
	Usecode_value add_party_items(Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);
	Usecode_value add_cont_items(Usecode_value& container, Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);
	Usecode_value remove_cont_items(Usecode_value& container, Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);

	int path_run_usecode(Usecode_value& npcval, Usecode_value& locval,
		Usecode_value& useval, Usecode_value& itemval,
		Usecode_value& eventval, int find_free = 0, int always = 0);
	void create_script(Usecode_value& objval, Usecode_value& codeval,
								long delay);

	/*
         *	Embedded intrinsics
	 */

	static struct IntrinsicTableEntry
		{
		UsecodeIntrinsicFn	func;
		const char *name;
		} intrinsic_table[], serpent_table[];
	Usecode_value	Execute_Intrinsic(UsecodeIntrinsicFn func,const char *name,int event,int intrinsic,int num_parms,Usecode_value parms[12]);
	USECODE_INTRINSIC_DECL(NOP);
	USECODE_INTRINSIC_DECL(UNKNOWN);
	USECODE_INTRINSIC_DECL(get_random);
	USECODE_INTRINSIC_DECL(execute_usecode_array);
	USECODE_INTRINSIC_DECL(delayed_execute_usecode_array);
	USECODE_INTRINSIC_DECL(show_npc_face);
	USECODE_INTRINSIC_DECL(remove_npc_face);
	USECODE_INTRINSIC_DECL(add_answer);
	USECODE_INTRINSIC_DECL(remove_answer);
	USECODE_INTRINSIC_DECL(push_answers);
	USECODE_INTRINSIC_DECL(pop_answers);
	USECODE_INTRINSIC_DECL(clear_answers);
	USECODE_INTRINSIC_DECL(select_from_menu);
	USECODE_INTRINSIC_DECL(select_from_menu2);
	USECODE_INTRINSIC_DECL(input_numeric_value);
	USECODE_INTRINSIC_DECL(set_item_shape);
	USECODE_INTRINSIC_DECL(find_nearest);
	USECODE_INTRINSIC_DECL(die_roll);
	USECODE_INTRINSIC_DECL(get_item_shape);
	USECODE_INTRINSIC_DECL(get_item_frame);
	USECODE_INTRINSIC_DECL(set_item_frame);
	USECODE_INTRINSIC_DECL(get_item_quality);
	USECODE_INTRINSIC_DECL(set_item_quality);
        USECODE_INTRINSIC_DECL(get_item_quantity);
        USECODE_INTRINSIC_DECL(set_item_quantity);
        USECODE_INTRINSIC_DECL(get_object_position);
	USECODE_INTRINSIC_DECL(get_distance);
        USECODE_INTRINSIC_DECL(find_direction);
        USECODE_INTRINSIC_DECL(get_npc_object);
        USECODE_INTRINSIC_DECL(get_schedule_type);
        USECODE_INTRINSIC_DECL(set_schedule_type);
        USECODE_INTRINSIC_DECL(add_to_party);
        USECODE_INTRINSIC_DECL(remove_from_party);
        USECODE_INTRINSIC_DECL(get_npc_prop);
        USECODE_INTRINSIC_DECL(set_npc_prop);
        USECODE_INTRINSIC_DECL(get_avatar_ref);
        USECODE_INTRINSIC_DECL(get_party_list);
        USECODE_INTRINSIC_DECL(create_new_object);
        USECODE_INTRINSIC_DECL(create_new_object2);
        USECODE_INTRINSIC_DECL(set_last_created);
        USECODE_INTRINSIC_DECL(update_last_created);
        USECODE_INTRINSIC_DECL(get_npc_name);
        USECODE_INTRINSIC_DECL(count_objects);
	USECODE_INTRINSIC_DECL(find_object);
        USECODE_INTRINSIC_DECL(get_cont_items);
        USECODE_INTRINSIC_DECL(remove_party_items);
        USECODE_INTRINSIC_DECL(add_party_items);
        USECODE_INTRINSIC_DECL(play_music);
        USECODE_INTRINSIC_DECL(npc_nearby);
        USECODE_INTRINSIC_DECL(find_nearby_avatar);
        USECODE_INTRINSIC_DECL(is_npc);
        USECODE_INTRINSIC_DECL(display_runes);
        USECODE_INTRINSIC_DECL(click_on_item);
        USECODE_INTRINSIC_DECL(find_nearby);
        USECODE_INTRINSIC_DECL(give_last_created);
	USECODE_INTRINSIC_DECL(is_dead);
        USECODE_INTRINSIC_DECL(game_hour);
        USECODE_INTRINSIC_DECL(game_minute);
	USECODE_INTRINSIC_DECL(get_npc_number);
	USECODE_INTRINSIC_DECL(part_of_day);
	USECODE_INTRINSIC_DECL(get_alignment);
	USECODE_INTRINSIC_DECL(set_alignment);
	USECODE_INTRINSIC_DECL(move_object);
	USECODE_INTRINSIC_DECL(remove_npc);
	USECODE_INTRINSIC_DECL(item_say);
	USECODE_INTRINSIC_DECL(clear_item_say);
	USECODE_INTRINSIC_DECL(projectile_effect);
	USECODE_INTRINSIC_DECL(get_lift);
	USECODE_INTRINSIC_DECL(set_lift);
	USECODE_INTRINSIC_DECL(get_weather);
	USECODE_INTRINSIC_DECL(set_weather);
	USECODE_INTRINSIC_DECL(sit_down);
	USECODE_INTRINSIC_DECL(summon);
	USECODE_INTRINSIC_DECL(display_map);
	USECODE_INTRINSIC_DECL(si_display_map);
	USECODE_INTRINSIC_DECL(kill_npc);
	USECODE_INTRINSIC_DECL(roll_to_win);
	USECODE_INTRINSIC_DECL(set_attack_mode);
	USECODE_INTRINSIC_DECL(get_attack_mode);
	USECODE_INTRINSIC_DECL(set_opponent);
	USECODE_INTRINSIC_DECL(clone);
	USECODE_INTRINSIC_DECL(get_oppressor);
	USECODE_INTRINSIC_DECL(set_oppressor);
	USECODE_INTRINSIC_DECL(get_weapon);
	USECODE_INTRINSIC_DECL(display_area);
	USECODE_INTRINSIC_DECL(wizard_eye);
	USECODE_INTRINSIC_DECL(resurrect);
	USECODE_INTRINSIC_DECL(get_body_npc);
	USECODE_INTRINSIC_DECL(add_spell);
	USECODE_INTRINSIC_DECL(sprite_effect);
	USECODE_INTRINSIC_DECL(obj_sprite_effect);
	USECODE_INTRINSIC_DECL(explode);
	USECODE_INTRINSIC_DECL(book_mode);
	USECODE_INTRINSIC_DECL(stop_time);
	USECODE_INTRINSIC_DECL(cause_light);
	USECODE_INTRINSIC_DECL(get_barge);
	USECODE_INTRINSIC_DECL(earthquake);
	USECODE_INTRINSIC_DECL(is_pc_female);
	USECODE_INTRINSIC_DECL(armageddon);
	USECODE_INTRINSIC_DECL(halt_scheduled);
	USECODE_INTRINSIC_DECL(lightning);
	USECODE_INTRINSIC_DECL(get_array_size);
	USECODE_INTRINSIC_DECL(mark_virtue_stone);
	USECODE_INTRINSIC_DECL(recall_virtue_stone);
	USECODE_INTRINSIC_DECL(apply_damage);
	USECODE_INTRINSIC_DECL(is_pc_inside);
	USECODE_INTRINSIC_DECL(get_timer);
	USECODE_INTRINSIC_DECL(set_timer);
	USECODE_INTRINSIC_DECL(wearing_fellowship);
	USECODE_INTRINSIC_DECL(mouse_exists);
	USECODE_INTRINSIC_DECL(get_speech_track);
	USECODE_INTRINSIC_DECL(flash_mouse);
	USECODE_INTRINSIC_DECL(get_item_frame_rot);
	USECODE_INTRINSIC_DECL(set_item_frame_rot);
	USECODE_INTRINSIC_DECL(on_barge);
	USECODE_INTRINSIC_DECL(get_container);
	USECODE_INTRINSIC_DECL(remove_item);
	USECODE_INTRINSIC_DECL(reduce_health);
	USECODE_INTRINSIC_DECL(is_readied);
	USECODE_INTRINSIC_DECL(get_readied);
	USECODE_INTRINSIC_DECL(restart_game);
	USECODE_INTRINSIC_DECL(start_speech);
	USECODE_INTRINSIC_DECL(is_water);
	USECODE_INTRINSIC_DECL(run_endgame);
	USECODE_INTRINSIC_DECL(fire_cannon);
	USECODE_INTRINSIC_DECL(nap_time);
	USECODE_INTRINSIC_DECL(advance_time);
	USECODE_INTRINSIC_DECL(in_usecode);
	USECODE_INTRINSIC_DECL(attack_avatar);
	USECODE_INTRINSIC_DECL(path_run_usecode);
	USECODE_INTRINSIC_DECL(close_gumps);
	USECODE_INTRINSIC_DECL(in_gump_mode);
	USECODE_INTRINSIC_DECL(is_not_blocked);
	USECODE_INTRINSIC_DECL(direction_from);
	USECODE_INTRINSIC_DECL(get_item_flag);
	USECODE_INTRINSIC_DECL(set_item_flag);
	USECODE_INTRINSIC_DECL(clear_item_flag);
	USECODE_INTRINSIC_DECL(set_path_failure);
	USECODE_INTRINSIC_DECL(fade_palette);
	USECODE_INTRINSIC_DECL(get_party_list2);
	USECODE_INTRINSIC_DECL(get_party_ids);
	USECODE_INTRINSIC_DECL(set_camera);
	USECODE_INTRINSIC_DECL(in_combat);
	USECODE_INTRINSIC_DECL(center_view);
	USECODE_INTRINSIC_DECL(get_dead_party);
	USECODE_INTRINSIC_DECL(play_sound_effect);
	USECODE_INTRINSIC_DECL(play_sound_effect2);
	USECODE_INTRINSIC_DECL(get_npc_id);
	USECODE_INTRINSIC_DECL(set_npc_id);
	USECODE_INTRINSIC_DECL(add_cont_items);
	USECODE_INTRINSIC_DECL(remove_cont_items);
					// Serpent Isle:
	USECODE_INTRINSIC_DECL(si_path_run_usecode);
	USECODE_INTRINSIC_DECL(remove_from_area);
	USECODE_INTRINSIC_DECL(infravision);
	USECODE_INTRINSIC_DECL(error_message);
	USECODE_INTRINSIC_DECL(set_polymorph);
	USECODE_INTRINSIC_DECL(show_npc_face0);
	USECODE_INTRINSIC_DECL(show_npc_face1);
	USECODE_INTRINSIC_DECL(remove_npc_face0);
	USECODE_INTRINSIC_DECL(remove_npc_face1);
	USECODE_INTRINSIC_DECL(set_conversation_slot);
	USECODE_INTRINSIC_DECL(init_conversation);
	USECODE_INTRINSIC_DECL(end_conversation);
	USECODE_INTRINSIC_DECL(set_new_schedules);
	USECODE_INTRINSIC_DECL(revert_schedule);
	USECODE_INTRINSIC_DECL(run_schedule);
	USECODE_INTRINSIC_DECL(get_temperature);
	USECODE_INTRINSIC_DECL(set_temperature);
//	USECODE_INTRINSIC_DECL(add_removed_npc);
	USECODE_INTRINSIC_DECL(approach_avatar);
	USECODE_INTRINSIC_DECL(telekenesis);
	USECODE_INTRINSIC_DECL(a_or_an);
	USECODE_INTRINSIC_DECL(add_to_keyring);
	USECODE_INTRINSIC_DECL(is_on_keyring);
	USECODE_INTRINSIC_DECL(save_pos);
	USECODE_INTRINSIC_DECL(teleport_to_saved_pos);
	USECODE_INTRINSIC_DECL(get_item_usability);
	USECODE_INTRINSIC_DECL(get_skin_colour);

	/*
	 *	Other private methods:
	 */
					// Call instrinsic function.
	Usecode_value call_intrinsic(int event, int intrinsic, int num_parms);
	void click_to_continue();	// Wait for user to click.
	void set_book(Text_gump *b);	// Set book/scroll to display.
	const char *get_user_choice();	// Get user's choice.
	int get_user_choice_num();
	void link_party();		// Set party's id's.

	Game_object *intercept_item;
	Game_object *temp_to_be_deleted;

	// execution functions
	bool call_function(int funcid, int event, Game_object *caller = 0,
					   bool entrypoint = false);
	void previous_stack_frame();
	void return_from_function(Usecode_value& retval);
	void return_from_procedure();
	void abort_function();
	int run();

	// debugging functions
	void uc_trace_disasm(Stack_frame* frame);
	void uc_trace_disasm(Usecode_value* locals, int num_locals,
						 uint8* data, uint8* externals, uint8* code,
						 uint8* ip);
	static int get_opcode_length(int opcode);
	void stack_trace(std::ostream& out);

#ifdef USECODE_DEBUGGER

	Breakpoints breakpoints;

	bool on_breakpoint; // are we on a breakpoint?
	int breakpoint_action; // stay on breakpoint/continue/abort?

public:
	bool is_on_breakpoint() const { return on_breakpoint; }
	void set_breakpoint_action(int a) { breakpoint_action = a; }

	void set_breakpoint();
	int set_location_breakpoint(int funcid, int ip);
	bool clear_breakpoint(int id) { return breakpoints.remove(id); }

	void transmit_breakpoints(int fd) { breakpoints.transmit(fd); }

	void dbg_stepover();
	void dbg_finish();

	int get_callstack_size() const;
	Stack_frame* get_stackframe(int i);

	int get_stack_size() const;
	Usecode_value* peek_stack(int depth) const;
	void poke_stack(int depth, Usecode_value& val);
#endif

private:

					// Add/remove party member.
	bool add_to_party(Actor *npc);
	bool remove_from_party(Actor *npc);
	int in_dead_party(Actor *npc);
	bool add_to_dead_party(Actor *npc);
	bool remove_from_dead_party(Actor *npc);
public:
	friend class Usecode_script;
	Usecode_internal(Game_window *gw);
	~Usecode_internal();
					// Read in usecode functions.
	virtual void read_usecode(std::istream& file);
					// Call desired function.
	virtual int call_usecode(int id, Game_object *obj, 
							Usecode_events event);
	virtual void update_party_status(Actor *npc);
	virtual void do_speech(int num);// Start speech, or show text.
	virtual void write();		// Write out 'gamedat/usecode.dat'.
	virtual void read();		// Read in 'gamedat/usecode.dat'.

	virtual void intercept_click_on_item(Game_object *obj) 
		{ intercept_item = obj; }

};


#endif
