/*
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Audio.h"
#include "Book_gump.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "Scroll_gump.h"
#include "Sign_gump.h"
// #include "actions.h"
#include "barge.h"
#include "bodies.h"
#include "cheat.h"
#include "chunks.h"
#include "conversation.h"
#include "effects.h"
#include "exult.h"
#include "game.h"
#include "gamewin.h"
#include "gamemap.h"
#include "gameclk.h"
#include "keyring.h"
#include "mouse.h"
#include "rect.h"
#include "schedule.h"
#include "spellbook.h"
#include "ucinternal.h"
#include "ucsched.h"
#include "useval.h"
#include "virstone.h"
#include "monsters.h"
#include "egg.h"
#include "monstinf.h"
#include "actions.h"
#include "ucscriptop.h"
#include "ucfunction.h"
#include "palette.h"

using std::cerr;
using std::cout;
using std::endl;
using std::rand;
using std::strchr;

Barge_object *Get_barge	(Game_object *obj);
extern Usecode_value no_ret;

#define PARTY_MAX (sizeof(party)/sizeof(party[0]))

#define	USECODE_INTRINSIC(NAME)	Usecode_value	Usecode_internal:: UI_## NAME (int event,int intrinsic,int num_parms,Usecode_value parms[12])

USECODE_INTRINSIC(NOP)
{
	return(no_ret);
}

USECODE_INTRINSIC(UNKNOWN)
{
//	Unhandled(intrinsic, num_parms, parms);
	return(no_ret);
}

USECODE_INTRINSIC(get_random)
{
	int range = parms[0].get_int_value();
	if (range == 0)
		{
		Usecode_value u(0);
		return(u);
		}
	Usecode_value u=(1 + (rand() % range));
	return(u);
}

USECODE_INTRINSIC(execute_usecode_array)
{
	COUT("Executing intrinsic 1");
#if 0	/* Old way. */
					// 9/17/00:  New guess to make it
					//   possible to heat Black sword.
	Game_object *item = get_item(parms[0]);
	Usecode_script *uc;
	if (item && (uc = Usecode_script::find(item)) != 0 &&
	    uc->is_activated())
		uc->halt();		// Stop current one.
#endif
					// Start on next tick.
	create_script(parms[0], parms[1], 1);

	Usecode_value u(1);
	return(u);
}

USECODE_INTRINSIC(delayed_execute_usecode_array)
{
	// Delay = .20 sec.?
					// +++++Special problem with inf. loop:
	if (Game::get_game_type() == BLACK_GATE &&
	    event == internal_exec && parms[1].get_array_size() == 3 &&
	    parms[1].get_elem(2).get_int_value() == 0x6f7)
		return(no_ret);
	int delay = parms[2].get_int_value();
	create_script(parms[0], parms[1], delay*gwin->get_std_delay());
	COUT("Executing intrinsic 2");

	Usecode_value u(1);
	return(u);
}

USECODE_INTRINSIC(show_npc_face)
{
	show_npc_face(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(remove_npc_face)
{
	remove_npc_face(parms[0]);
	return(no_ret);
}

USECODE_INTRINSIC(add_answer)
{
	conv->add_answer(parms[0]);
	//	user_choice = 0;
	return(no_ret);
}

USECODE_INTRINSIC(remove_answer)
{
	conv->remove_answer(parms[0]);
// Commented out 'user_choice = 0' 8/3/00 for Tseramed conversation.
//	user_choice = 0;
	return(no_ret);
}

USECODE_INTRINSIC(push_answers)
{
	conv->push_answers();
	return(no_ret);
}

USECODE_INTRINSIC(pop_answers)
{
	if(!conv->stack_empty())
	{
		conv->pop_answers();
		user_choice = 0;	// Added 7/24/2000.
	}
	return(no_ret);
}

USECODE_INTRINSIC(clear_answers)
{
	conv->clear_answers();
	return(no_ret);
}

USECODE_INTRINSIC(select_from_menu)
{
	user_choice = 0;
	const char *choice = get_user_choice();
	user_choice = 0;
	Usecode_value u(choice);
	return(u);
}

USECODE_INTRINSIC(select_from_menu2)
{
	// Return index (1-n) of choice.
	user_choice = 0;
	Usecode_value val(get_user_choice_num() + 1);
	user_choice = 0;
	return(val);
}

USECODE_INTRINSIC(input_numeric_value)
{
	// Ask for # (min, max, step, default).
	extern int Prompt_for_number(int minval, int maxval, 
							int step, int def);
	Usecode_value ret(Prompt_for_number(
		parms[0].get_int_value(), parms[1].get_int_value(),
		parms[2].get_int_value(), parms[3].get_int_value()));
	conv->clear_text_pending();	// Answered a question.
	return(ret);
}

USECODE_INTRINSIC(set_item_shape)
{
	// Set item shape.
	set_item_shape(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(find_nearest)
{
	// Think it rets. nearest obj. near parm0.
	Usecode_value u(find_nearest(parms[0], parms[1], parms[2]));
	return(u);
}

USECODE_INTRINSIC(die_roll)
{
	// Rand. # within range.
	int low = parms[0].get_int_value();
	int high = parms[1].get_int_value();
	if (low > high)
		{
		int tmp = low;
		low = high;
		high = tmp;
		}
	int val = (rand() % (high - low + 1)) + low;
	Usecode_value u(val);
	return(u);
}

USECODE_INTRINSIC(get_item_shape)
{
	Game_object *item = get_item(parms[0]);
					// Want the actual, not polymorph'd.
	Actor *act = as_actor(item);
	return Usecode_value(item == 0 ? 0 : 
		(act ? act->get_shape_real() : item->get_shapenum()));
}

USECODE_INTRINSIC(get_item_frame)
{
	// Returns frame without rotated bit.
	Game_object *item = get_item(parms[0]);
					// Don't count rotated frames.
	return Usecode_value(item == 0 ? 0 : item->get_framenum()&31);
}

USECODE_INTRINSIC(set_item_frame)
{	// Set frame, but don't change rotated bit.
//++++++++Seems like in BG, this should be the same as set_item_frame_rot()??
	set_item_frame(get_item(parms[0]), parms[1].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(get_item_quality)
{
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	Shape_info& info = obj->get_info();
	return Usecode_value(info.has_quality() ? obj->get_quality() : 0);
}

USECODE_INTRINSIC(set_item_quality)
{
	// Guessing it's 
	//  set_quality(item, value).
	int qual = parms[1].get_int_value();
	if (qual == c_any_qual)		// Leave alone (happens in SI)?
		return Usecode_value(1);
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		Shape_info& info = obj->get_info();
		if (info.has_quality())
			{
			obj->set_quality((unsigned int) qual);
			return Usecode_value(1);
			}
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(get_item_quantity)
{
	// Get quantity of an item.
	//   Get_quantity(item, mystery).
	Usecode_value ret(0);
	Game_object *obj = get_item(parms[0]);
	if (obj)
		ret = Usecode_value(obj->get_quantity());
	return(ret);
}

USECODE_INTRINSIC(set_item_quantity)
{
	// Set_quantity (item, newcount).  Rets 1 iff item.has_quantity().
	Usecode_value ret(0);
	Game_object *obj = get_item(parms[0]);
	int newquant = parms[1].get_int_value();
	if (obj && obj->get_info().has_quantity())
		{
		ret = Usecode_value(1);
					// If not in world, don't delete!
		if (newquant == 0 && obj->get_cx() == 255)
			return ret;
		int oldquant = obj->get_quantity();
		int delta = newquant - oldquant;
					// Note:  This can delete the obj.
		int newdelta = obj->modify_quantity(delta);
					// ++++Maybe repaint?
		}
	return(ret);
}

USECODE_INTRINSIC(get_object_position)
{
	// Takes itemref.  ?Think it rets.
	//  hotspot coords: (x, y, z).
	Game_object *obj = get_item(parms[0]);
	Tile_coord c(0, 0, 0);
	if (obj)		// (Watch for animated objs' wiggles.)
		c = obj->get_outermost()->get_original_tile_coord();
	Usecode_value vx(c.tx), vy(c.ty), vz(c.tz);
	Usecode_value arr(3, &vx);
	arr.put_elem(1, vy);
	arr.put_elem(2, vz);
	return(arr);
}

USECODE_INTRINSIC(get_distance)
{
	// Distance from parm[0] -> parm[1].  Guessing how it's computed.
	Game_object *obj0 = get_item(parms[0]);
	Game_object *obj1 = get_item(parms[1]);
	Usecode_value u((obj0 && obj1) ? obj0->get_tile().distance(
					obj1->get_tile()) : 0);
	return(u);
}

USECODE_INTRINSIC(find_direction)
{
	// Direction from parm[0] -> parm[1].
	// Rets. 0-7.  Is 0 east?
	Usecode_value u=find_direction(parms[0], parms[1]);
	return(u);
}

USECODE_INTRINSIC(get_npc_object)
{
	// Takes -npc.  Returns object, or array of objects.
	Usecode_value& v = parms[0];
	if (v.is_array())		// Do it for each element of array.
		{
		int sz = v.get_array_size();
		Usecode_value ret(sz, 0);
		for (int i = 0; i < sz; i++)
			{
			Usecode_value elem(get_item(v.get_elem(i)));
			ret.put_elem(i, elem);
			}
		return ret;
		}
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj);
	return(u);
}

USECODE_INTRINSIC(get_schedule_type)
{
	// GetSchedule(npc).  Rets. schedtype.
	Actor *npc = as_actor(get_item(parms[0]));
	if (!npc)
		return Usecode_value(0);
	Schedule *schedule = npc->get_schedule();
	int sched = schedule ? schedule->get_actual_type(npc) 
			     : npc->get_schedule_type();
					// Path_run_usecode?  (This is to fix
					//   a bug in the Fawn Trial.)
					//+++++Should be a better way to check.
	if (Game::get_game_type() == SERPENT_ISLE &&
	    npc->get_action() && npc->get_action()->as_usecode_path())
					// Give a 'fake' schedule.
		sched = Schedule::walk_to_schedule;
	Usecode_value u(sched);
	return(u);
}

USECODE_INTRINSIC(set_schedule_type)
{
	// SetSchedule?(npc, schedtype).
	// Looks like 15=wait here, 11=go home, 0=train/fight... This is the
	// 'bNum' field in schedules.
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		{
		int newsched = parms[1].get_int_value();
		npc->set_schedule_type(newsched);
					// Taking Avatar out of combat?
		if (npc == gwin->get_main_actor() && gwin->in_combat() &&
		    newsched != Schedule::combat)
					// End combat mode (for L.Field).
			{
			Audio::get_ptr()->stop_music();
			gwin->toggle_combat();
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(add_to_party)
{
	// NPC joins party.
	Actor *npc = as_actor(get_item(parms[0]));
	if (!add_to_party(npc))
		return no_ret;		// Can't add.
	npc->set_schedule_type(Schedule::follow_avatar);
// cout << "NPC " << npc->get_npc_num() << " added to party." << endl;
	return no_ret;
}

USECODE_INTRINSIC(remove_from_party)
{
	// NPC leaves party.
	Game_object *npc = get_item(parms[0]);
	remove_from_party(as_actor(npc));
	return no_ret;
}

USECODE_INTRINSIC(get_npc_prop)
{
	// Get NPC prop (item, prop_id).
	//   (9 is food level).
	Actor *npc = as_actor(get_item(parms[0]));
	Usecode_value u(npc ? 
		npc->get_property(parms[1].get_int_value()) : 0);
	return(u);
}

USECODE_INTRINSIC(set_npc_prop)
{
	// Set NPC prop (item, prop_id, delta_value).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		{			// NOTE: 3rd parm. is a delta!
		int prop = parms[1].get_int_value();
		npc->set_property(prop, npc->get_property(prop) +
						parms[2].get_int_value());
		return Usecode_value(1);// SI needs return.
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(get_avatar_ref)
{
	// Guessing it's Avatar's itemref.
	Usecode_value u(gwin->get_main_actor());
	return(u);
}

USECODE_INTRINSIC(get_party_list)
{
	// Return array with party members.
	Usecode_value u(get_party());
	return(u);
}

USECODE_INTRINSIC(create_new_object)
{
	// create_new_object(shapenum).   Stores it in 'last_created' (which
	//   maybe should be a stack.
	int shapenum = parms[0].get_int_value();

	Game_object *obj;		// Create to be written to Ireg.
	Monster_info *inf = ShapeID::get_info(shapenum).get_monster_info();

	if (inf)
	{
					// (Wait sched. added for FOV.)
		// don't add equipment (Erethian's transform sequence)
		Monster_actor *monster = Monster_actor::create(shapenum,
			Tile_coord(-1, -1, -1), Schedule::wait, 
					(int) Actor::neutral, true, false);
					// FORCE it to be neutral (dec04,01).
		monster->set_alignment((int) Actor::neutral);
		gwin->add_dirty(monster);
		gwin->add_nearby_npc(monster);
		gwin->show();
		last_created.push_back(monster);
		return Usecode_value(monster);
	}
	else
	{
		if (Is_body(shapenum))
		{
			obj = new Dead_body(shapenum, 0, 0, 0, 0, -1);
			cout << " body " << endl;
		}
		else
		{
			obj = gmap->create_ireg_object(shapenum, 0);
					// Be liberal about taking stuff.
			obj->set_flag(Obj_flags::okay_to_take);
			cout << " ireg object " << endl;
		}
	}
	obj->set_invalid();		// Not in world yet.
	obj->set_flag(Obj_flags::okay_to_take);
	last_created.push_back(obj);
	Usecode_value u(obj);
	return(u);
}

USECODE_INTRINSIC(create_new_object2)
{
	// create_new_object(shapenum, loc).
	Usecode_value ret = UI_create_new_object(event, intrinsic, 1, parms);
	if (ret == 0)
		return ret;		// Failed.
	UI_update_last_created(event, intrinsic, 1, &parms[1]);
	return ret;
}

USECODE_INTRINSIC(set_last_created)
{
	// Take itemref off map and set last_created to it.
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		last_created.push_back(obj);
		obj->remove_this(1);	// Remove, but don't delete.
		}
	Usecode_value u(obj);
	return(u);
}

USECODE_INTRINSIC(update_last_created)
{
	// Think it takes array from 0x18,
	//   updates last-created object.
	//   ??guessing??
	if (last_created.empty())
		{
		Usecode_value u((Game_object*) NULL);
		return(u);
		}
	Game_object *obj = last_created.back();
	last_created.pop_back();
	Usecode_value& arr = parms[0];
	int sz = arr.get_array_size();
	if (sz == 3 || sz == 2)
		{
		Tile_coord dest(arr.get_elem(0).get_int_value(),
			  arr.get_elem(1).get_int_value(),
			  sz == 3 ? arr.get_elem(2).get_int_value() : 0);
		Tile_coord pos = dest;
					// Skip 'blocked' check if it looks
					//   structural. (For SI maze).
		Shape_info& info = obj->get_info();
		if (info.get_3d_height() < 5 &&
					// Weed out drawbridge, etc:
		    info.get_3d_xtiles() + info.get_3d_ytiles() < 8 &&
					// And skip if BG.  Causes FoV probs.
		    Game::get_game_type() != BLACK_GATE)
			while (Map_chunk::is_blocked(pos, 1,
					MOVE_ALL_TERRAIN | MOVE_NODROP))
				{	// Try up to ceiling.
				if (dest.tz >= (dest.tz + 5) - dest.tz%5 - 1)
					{
//					cerr << " Failed to find space"<< endl;
					obj->remove_this(0);
					return Usecode_value(0);
					}
				dest.tz++;
				pos.tz = dest.tz;
				}
		obj->move(dest.tx, dest.ty, dest.tz);
		if (GAME_BG) {
			Usecode_value u(1);
			return u;
		} else {
			Usecode_value u(obj);
			return u;
		}
					// Taking a guess here:
	} else if (sz == 1)
		{
		obj->remove_this();
		}
#ifdef DEBUG
	else
		{
		cout << " { Intrinsic 0x26:  "; arr.print(cout); cout << endl << "} ";
		}
#endif
//	gwin->paint_dirty();	// Problems in conversations.
//	gwin->show();		// ??
	Usecode_value u(1);
	return(u);
}

USECODE_INTRINSIC(get_npc_name)
{
	// Get NPC name(s).  Works on arrays, too.
	static const char *unknown = "??name??";
	int cnt = parms[0].get_array_size();
	if (cnt)
		{			// Do array.
		Usecode_value arr(cnt, 0);
		for (int i = 0; i < cnt; i++)
			{
			Game_object *obj = get_item(parms[0].get_elem(i));
			Usecode_value v(obj->get_name().c_str());
			arr.put_elem(i, v);
			}
		return(arr);
		}
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? obj->get_name().c_str() : unknown);
	return(u);
}

USECODE_INTRINSIC(count_objects)
{
	// How many?
	// ((npc?-357==party, -356=avatar), 
	//   item, quality, frame (c_any_framenum = any)).
	// Quality/frame -359 means any.
	Usecode_value u(count_objects(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}

USECODE_INTRINSIC(find_object)
{
	// Find_object(container(-357=party) OR loc, shapenum, qual?? (-359=any), 
	//						frame??(-359=any)).
	int shnum = parms[1].get_int_value(),
	    qual  = parms[2].get_int_value(),
	    frnum = parms[3].get_int_value();
	if (parms[0].get_array_size() == 3)
		{			// Location (x, y).
		Game_object_vector vec;
		Game_object::find_nearby(vec,
			Tile_coord(parms[0].get_elem(0).get_int_value(),
				   parms[0].get_elem(1).get_int_value(),
				   parms[0].get_elem(2).get_int_value()),
			shnum, 1, 0, qual, frnum);
		if (vec.empty())
			return Usecode_value((Game_object *) 0);
		else
			return Usecode_value(vec.front());
		}
	int oval  = parms[0].get_int_value();
	if (oval != -357)		// Not the whole party?
		{			// Find inside owner.
		Game_object *obj = get_item(parms[0]);
		if (!obj)
			return Usecode_value((Game_object*) NULL);
		Game_object *f = obj->find_item(shnum, qual, frnum);
		return Usecode_value(f);
		}
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (obj)
			{
			Game_object *f = obj->find_item(shnum, qual, frnum);
			if (f)
				return Usecode_value(f);
			}
		}
	return Usecode_value((Game_object*) NULL);
}

USECODE_INTRINSIC(get_cont_items)
{
	// Get cont. items(container, shape, qual, frame).
	// recursively find items in container
	Usecode_value u(get_objects(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}


USECODE_INTRINSIC(remove_party_items)
{
	// Remove items(quantity, item, ??quality?? (-359), frame(-359), T/F).
	return remove_party_items(parms[0], parms[1], parms[2],
						parms[3], parms[4]);
}

USECODE_INTRINSIC(add_party_items)
{
	// Add items(num, item, ??quality?? (-359), frame (or -359), T/F).
	// Returns array of NPC's (->'s) who got the items.
	Usecode_value u(add_party_items(parms[0], parms[1], parms[2],
						parms[3], parms[4]));
	return(u);
}

USECODE_INTRINSIC(play_music)
{
	// Play music(item, songnum).
	// ??Show notes by item?
#ifdef DEBUG
	cout << "Music request in usecode" << endl;
	cout << "Parameter data follows" << endl;
	cout << "0: " << ((parms[0].get_int_value()>>8)&0xff) << " " <<  ((parms[0].get_int_value())&0xff) << endl;
	cout << "1: " << ((parms[1].get_int_value()>>8)&0x01) << " " <<  ((parms[1].get_int_value())&0x01) << endl;
#endif
	int track = parms[0].get_int_value()&0xff;
	if (track == 0xff)		// I think this is right:
		Audio::get_ptr()->cancel_streams();	// Stop playing.
	else
		Audio::get_ptr()->start_music(track, (parms[0].get_int_value()>>8)&0x01);
	return(no_ret);
}

USECODE_INTRINSIC(npc_nearby)
{
	// NPC nearby? (item).
	Game_object *npc = get_item(parms[0]);
	int is_near = (npc != 0 && 
		npc->get_tile().distance(gwin->get_main_actor()->get_tile()) 
								< 12 &&
					// FALSE if asleep.
		!npc->get_flag(Obj_flags::asleep));
	Usecode_value u(is_near);
	return(u);
}

USECODE_INTRINSIC(find_nearby_avatar)
{
	// Find objs. with given shape near Avatar?
	Usecode_value av(gwin->get_main_actor());
					// Try bigger # for Test of Love tree.
	Usecode_value dist(/* 64 */ 192), mask(0);
	Usecode_value u(find_nearby(av, parms[0], dist, mask));
	return(u);
}

USECODE_INTRINSIC(is_npc)
{
	// Is item an NPC?
	Actor *npc = as_actor(get_item(parms[0]));
	Usecode_value u(npc != 0);
	return(u);
}

USECODE_INTRINSIC(display_runes)
{
	// Render text into runes for signs, tombstones, plaques and the like
	// Display sign (gump #, array_of_text).
	int cnt = parms[1].get_array_size();
	if (!cnt)
		cnt = 1;		// Try with 1 element.
	Sign_gump *sign = new Sign_gump(parms[0].get_int_value(), cnt);
	bool si = Game::get_game_type()== SERPENT_ISLE;
	for (int i = 0; i < cnt; i++)
		{			// Paint each line.
		Usecode_value& lval = !i ? parms[1].get_elem0() 
					: parms[1].get_elem(i);
		const char *str = lval.get_str_value();
#if 0	/* ++++Not sure about this yet.  Compare with orig. */
		if (si)			// SI:  Add 0x20 to each chr.
			{
			char *newstr = strdup(str);
			for (char *ptr = newstr; *ptr; ptr++)
				if (*ptr >= 'A' && *ptr <= 'Z')
					*ptr += 0x20;
			sign->add_text(i, newstr);
			delete newstr;
			}
		else
#endif
			sign->add_text(i, str);
		}
	sign->paint();			// Paint it, and wait for click.
	int x, y;
	Get_click(x, y, Mouse::hand);
	delete sign;
	gwin->paint();
	return(no_ret);
}

USECODE_INTRINSIC(click_on_item)
{
	// Doesn't ret. until user single-
	//   clicks on an item.  Rets. item.
	Game_object *obj;
	Tile_coord t;

	// intercept this click?
	if (intercept_item) {
		obj = intercept_item;
		intercept_item = 0;
		t = obj->get_tile();

					// Special case for weapon hit:
	} else if (event == weapon && caller_item)
		{
		obj = caller_item;
		t = obj->get_tile();
		}
	else
		{
		int x, y;		// Allow dragging while here:
		if (!Get_click(x, y, Mouse::greenselect, 0, true))
			return Usecode_value(0);
					// Get abs. tile coords. clicked on.
		t = Tile_coord(gwin->get_scrolltx() + x/c_tilesize,
				gwin->get_scrollty() + y/c_tilesize, 0);
					// Look for obj. in open gump.
		Gump *gump = gwin->get_gump_man()->find_gump(x, y);
		if (gump)
		{
			obj = gump->find_object(x, y);
			if (!obj) obj = gump->find_actor(x, y);
		}
		else			// Search rest of world.
			{
			obj = gwin->find_object(x, y);
			if (obj)	// Found object?  Use its coords.
				t = obj->get_tile();
			}
		}
	Usecode_value oval(obj);	// Ret. array with obj as 1st elem.
	Usecode_value ret(4, &oval);
	Usecode_value xval(t.tx), yval(t.ty), zval(t.tz);
	ret.put_elem(1, xval);
	ret.put_elem(2, yval);
	ret.put_elem(3, zval);
	return (ret);
}

USECODE_INTRINSIC(find_nearby)
{
	// Think it rets. objs. near parm0.
	Usecode_value u(find_nearby(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}

USECODE_INTRINSIC(give_last_created)
{
	// Think it's give_last_created(container).
	Game_object *cont = get_item(parms[0]);
	int ret = 0;
	if (cont && !last_created.empty())
		{
					// Get object, but don't pop yet.
		Game_object *obj = last_created.back();
					// Don't check vol.  Causes failures.
		ret = cont->add(obj, 1);
		if (ret)		// Pop only if added.  Fixes chest/
					//   tooth bug in SI.
			last_created.pop_back();
		}
	Usecode_value u(ret);
	return(u);
}

USECODE_INTRINSIC(is_dead)
{
	// Return 1 if parm0 is a dead NPC.
	Actor *npc = as_actor(get_item(parms[0]));
	Usecode_value u(npc && npc->is_dead());
	return(u);
}

USECODE_INTRINSIC(game_hour)
{
	// Return. game time hour (0-23).
	Usecode_value u(gclock->get_hour());
	return(u);
}

USECODE_INTRINSIC(game_minute)
{
	// Return minute (0-59).
	Usecode_value u(gclock->get_minute());
	return(u);
}

USECODE_INTRINSIC(get_npc_number)
{
	// Returns NPC# of item. (-356 =
	//   avatar).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc == gwin->get_main_actor())
		{
		Usecode_value u(-356);
		return(u);
		}
	int num = npc ? npc->get_npc_num() : 0;
	Usecode_value u(-num);
	return(u);
}

USECODE_INTRINSIC(part_of_day)
{
	// Return 3-hour # (0-7, 0=midnight).
	Usecode_value u(gclock->get_hour()/3);
	return(u);
}

USECODE_INTRINSIC(get_alignment)
{
	// Get npc's alignment.
	Actor *npc = as_actor(get_item(parms[0]));
	Usecode_value u(npc ? npc->get_alignment() : 0);
	return(u);
}

USECODE_INTRINSIC(set_alignment)
{
	// Set npc's alignment.
	// 2,3==bad towards Ava. 0==good.
	Actor *npc = as_actor(get_item(parms[0]));
	int val = parms[1].get_int_value();
	if (npc)
		{
		int oldalign = npc->get_alignment();
		npc->set_alignment(val);
		if (oldalign != val)	// Changed?  Force search for new opp.
			npc->set_target(0);
					// For fixing List Field fleeing:
		if (npc->get_attack_mode() == Actor::flee)
			npc->set_attack_mode(Actor::nearest);
		}
	return(no_ret);
}

USECODE_INTRINSIC(move_object)
{
	// move_object(obj(-357=party), (tx, ty, tz)).
	Usecode_value& p = parms[1];
	Tile_coord tile(p.get_elem(0).get_int_value(),
			p.get_elem(1).get_int_value(),
			p.get_elem(2).get_int_value());
	Actor *ava = gwin->get_main_actor();
	if (parms[0].get_int_value() == -357)
		{			// Move whole party.
					// If Freedom exit teleport, don't ac-
					//   tivate eggs when you arrive.
		gwin->teleport_party(tile, Game::get_game_type() ==
			SERPENT_ISLE && cur_function->id == 0x7df && 
				caller_item->get_quality() == 0xcf);
		return (no_ret);
		}
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return (no_ret);
	Tile_coord oldpos = obj->get_tile();
	obj->move(tile.tx, tile.ty, tile.tz);
	Actor *act = as_actor(obj);
	if (act)
		{
		act->set_action(0);
		if (act == ava)
			{		// Teleported Avatar?
					// Make new loc. visible, test eggs.
			gwin->center_view(tile);
			Map_chunk::try_all_eggs(ava, tile.tx, 
				tile.ty, tile.tz, oldpos.tx, oldpos.ty);
			}
					// Close?  Add to 'nearby' list.
		else if (ava->distance(act) < gwin->get_width()/c_tilesize)
			{
			Npc_actor *npc = act->as_npc();
			if (npc) gwin->add_nearby_npc(npc);
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(remove_npc)
{
	// Remove_npc(npc) - Remove npc from world.
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		{
					// Don't want him/her coming back!
		npc->set_schedule_type(Schedule::wait);
		gwin->add_dirty(npc);
		npc->remove_this(1);	// Remove, but don't delete.
		}
	return (no_ret);
}

USECODE_INTRINSIC(item_say)
{
	// Show str. near item (item, str).
	if (!conv->is_npc_text_pending())
		item_say(parms[0], parms[1]);	// Do it now.
#if 0	/* ++++Crashes. */
	else
		{
		Game_object *obj = get_item(parms[0]);
		const char *str = parms[1].get_str_value();
		if (obj && str)
			{
			Usecode_script *scr = new Usecode_script(obj);
			scr->add(Ucscript::say, str);
			scr->start();
			}
		}
#endif
	return(no_ret);
}

USECODE_INTRINSIC(clear_item_say)
{
	// Clear str. near item (item).
	Game_object *item = get_item(parms[0]);
	if (item)
		gwin->get_effects()->remove_text_effect(item);
	return(no_ret);
}

USECODE_INTRINSIC(projectile_effect)
{
	// animate(fromitem, toitem, anim_shape_in_shapesdotvga).
	// ???? When it reaches toitem, toitem is 'attacked' by anim_shape.
	//   Returns??}
	Game_object *from = get_item(parms[0]),
		    *to = get_item(parms[1]);
	if (!from || !to)
		return Usecode_value(0);
	Actor *attacker = as_actor(from);
	if (!attacker)
		return Usecode_value(0);
	int shnum = parms[2].get_int_value();
	gwin->get_effects()->add_effect(
				new Projectile_effect(attacker, to, shnum));

	return Usecode_value(0);	// Not sure what this should be.
}

USECODE_INTRINSIC(get_lift)
{
	// ?? Guessing rets. lift(item).
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? Usecode_value(obj->get_lift())
					: Usecode_value(0));
	return(u);
}

USECODE_INTRINSIC(set_lift)
{
	// ?? Guessing setlift(item, lift).
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		Tile_coord t = obj->get_tile();
		int lift = parms[1].get_int_value();
		if (lift >= 0 && lift < 20)
			obj->move(t.tx, t.ty, lift);
		gwin->paint();
		gwin->show();
		}
	return(no_ret);
}

USECODE_INTRINSIC(get_weather)
{
	// Get_weather()
	return Usecode_value(gwin->get_effects()->get_weather());
}

USECODE_INTRINSIC(set_weather)
{
	// Set_weather(i)
	Egg_object::set_weather(parms[0].get_int_value());
	return no_ret;
}


USECODE_INTRINSIC(sit_down)
{
	// Sit_down(npc, chair).
	Game_object *nobj = get_item(parms[0]);
	Actor *npc = as_actor(nobj);
	if (!npc)
		return (no_ret);	// Doesn't look like an NPC.
	Game_object *chair = get_item(parms[1]);
					// See if someone already there.
	if (!chair || Sit_schedule::is_occupied(chair, npc))
		return(no_ret);
	npc->set_schedule_type(Schedule::sit, new Sit_schedule(npc, chair));
	return(no_ret);
}

USECODE_INTRINSIC(summon)
{
	// summon(shape, flag??).  Create monster of desired shape.

	int shapenum = parms[0].get_int_value();
	Monster_info *info = ShapeID::get_info(shapenum).get_monster_info();
	if (!info)
		return Usecode_value(0);
	Tile_coord start = gwin->get_main_actor()->get_tile();
	Tile_coord dest = Map_chunk::find_spot(start, 5, shapenum, 0, 1,
			-1, gwin->is_main_actor_inside() ?
				Map_chunk::inside : Map_chunk::outside);
	if (dest.tx == -1)
		return Usecode_value(0);
	Monster_actor *monst = Monster_actor::create(shapenum, dest,
					Schedule::combat, Actor::friendly);
	return Usecode_value(monst);
}

USECODE_INTRINSIC(display_map)
{
	// Display map.
	ShapeID msid(game->get_shape("sprites/map"), 0, SF_SPRITES_VGA);
	Shape_frame *map = msid.get_shape();

					// Get coords. for centered view.
	int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
	int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
	msid.paint_shape(x, y);

	//count all sextants in party
	Usecode_value v_357(-357), v650(650), v_359(-359);
	long sextants = count_objects(v_357, v650, v_359, v_359).get_int_value();
	if ((!gwin->is_main_actor_inside()) && (sextants > 0)) {
		// mark location
		int xx, yy;
		Tile_coord t = gwin->get_main_actor()->get_tile();

		if (Game::get_game_type()==BLACK_GATE) {
			xx = (int)(t.tx/16.05 + 5 + 0.5);
			yy = (int)(t.ty/15.95 + 4 + 0.5);
		} else if (Game::get_game_type()==SERPENT_ISLE) {
			xx = (int)(t.tx/16.0 + 18 + 0.5);
			yy = (int)(t.ty/16.0 + 9.4 + 0.5);
		}

		xx += x - map->get_xleft();
		yy += y - map->get_yabove();

		gwin->get_win()->fill8(255, 1, 5, xx, yy - 2);
		gwin->get_win()->fill8(255, 5, 1, xx - 2, yy);
	}

	gwin->show(1);
	int xx, yy;
	Get_click(xx, yy, Mouse::hand);
	gwin->paint();
	return(no_ret);
}

USECODE_INTRINSIC(si_display_map)
{
	int mapnum = parms[0].get_int_value();
	int shapenum;

	switch (mapnum) {
		case 0:	return UI_display_map(event,intrinsic,num_parms,parms);
		case 1:	shapenum = 57; break;
		case 2: shapenum = 58; break;
		case 3: shapenum = 59; break;
		case 4: shapenum = 60; break;
		case 5: shapenum = 52; break;
		default: return no_ret;
	}
			
	// Display map.
	
	// Display map.
	ShapeID msid(shapenum, 0, SF_SPRITES_VGA);
	Shape_frame *map = msid.get_shape();
				// Get coords. for centered view.
	int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
	int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
	msid.paint_shape(x, y);

	gwin->show(1);
	int xx, yy;
	Get_click(xx, yy, Mouse::hand);
	gwin->paint();

	return no_ret;
}

USECODE_INTRINSIC(kill_npc)
{
	// kill_npc(npc).
	Game_object *item = get_item(parms[0]);
	Actor *npc = as_actor(item);
	if (npc)
		npc->die();
	return (no_ret);
}

USECODE_INTRINSIC(roll_to_win)
{
	// roll_to_win(attackpts, defendpts)
	int attack = parms[0].get_int_value();
	int defend = parms[1].get_int_value();
	return Usecode_value((int) Actor::roll_to_win(attack, defend));
}

USECODE_INTRINSIC(set_attack_mode)
{
	// set_attack_mode(npc, mode).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		npc->set_attack_mode((Actor::Attack_mode) 
					parms[1].need_int_value());
	return (no_ret);
}

USECODE_INTRINSIC(get_attack_mode)
{
	// get_attack_mode(npc).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		return Usecode_value((int) npc->get_attack_mode());
	return Usecode_value(0);
}

USECODE_INTRINSIC(set_opponent)
{
	// set_opponent(npc, new_opponent).
	Actor *npc = as_actor(get_item(parms[0]));
	Game_object *opponent = get_item(parms[1]);
	if (npc && opponent)
		npc->set_target(opponent);
	return (no_ret);
}

USECODE_INTRINSIC(clone)
{
	// clone(npc)
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		{
		npc->set_alignment(Actor::friendly);
		npc->set_schedule_type(Schedule::combat);
		return Usecode_value(npc->clone());
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(get_oppressor)
{
	// get_oppressor(npc) Returns 0-n, NPC # (0=avatar).
	Actor *npc = as_actor(get_item(parms[0]));
	return Usecode_value(npc ? npc->get_oppressor() : 0);
}

USECODE_INTRINSIC(set_oppressor)
{
	// set_oppressor(npc, opp)
	Actor *npc = as_actor(get_item(parms[0]));
	Actor *opp = as_actor(get_item(parms[1]));
	if (npc && opp)
		{
		if (opp == gwin->get_main_actor())
			npc->set_oppressor(0);
		else
			npc->set_oppressor(opp->get_npc_num());
					// Need this for SI ListField training:
		npc->set_target(opp);
		opp->set_target(npc);
		}
	return no_ret;
}

USECODE_INTRINSIC(get_weapon)
{
	// get_weapon(npc).  Returns shape.
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		{
		int shape, points;
		if (npc->get_weapon(points, shape))
			return Usecode_value(shape);
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(display_area)
{
	// display_area(tilepos) - used for crystal balls.
	int size = parms[0].get_array_size();
	if (size >= 3)
		{
		int tx = parms[0].get_elem(0).get_int_value();
		int ty = parms[0].get_elem(1).get_int_value();
		int unknown = parms[0].get_elem(2).get_int_value();
					// Figure in tiles.
		int tw = gwin->get_width()/c_tilesize, 
		    th = gwin->get_height()/c_tilesize;
		gwin->clear_screen();	// Fill with black.
		Shape_frame *sprite = ShapeID(10, 0, SF_SPRITES_VGA).get_shape();
					// Center it.
		int topx = (gwin->get_width() - sprite->get_width())/2,
		    topy = (gwin->get_height() - sprite->get_height())/2;
					// Get area to fill.
		int x = topx, y = topy, w = sprite->get_width(),
					h = sprite->get_height();
		if (w > gwin->get_width())
			{ x = 0; w = gwin->get_width(); }
		if (h > gwin->get_height())
			{ y = 0; h = gwin->get_height(); }
					// Paint game area.
		gwin->paint_map_at_tile(x, y, w, h, tx - tw/2, ty - th/2, 4);
					// Paint sprite #10 (black gate!)
					//   over it, transparently.
		sman->paint_shape(topx + sprite->get_xleft(),
				topy + sprite->get_yabove(), sprite, 1);
		gwin->show();
					// Wait for click.
		Get_click(x, y, Mouse::hand);
		gwin->paint();		// Repaint normal area.
		}
	return (no_ret);
}

USECODE_INTRINSIC(wizard_eye)
{
	// wizard_eye(#ticks, ??);
	extern void Wizard_eye(long);
					// Let's give 50% longer.
	Wizard_eye(parms[0].get_int_value()*(3*gwin->get_std_delay())/2);
	return no_ret;
}

USECODE_INTRINSIC(resurrect)
{
	// resurrect(body).  Returns actor if successful.
	Game_object *body = get_item(parms[0]);
	int npc_num = body ? body->get_live_npc_num() : -1;
	if (npc_num < 0)
		return Usecode_value((Game_object*) NULL);
	Actor *actor = gwin->get_npc(npc_num);
	if (actor)
		actor = actor->resurrect((Dead_body *) body);
	return Usecode_value(actor);
}

USECODE_INTRINSIC(get_body_npc)
{
	// get_body_npc(body).  Returns npc # (negative).
	Game_object *obj = get_item(parms[0]);
	int num = obj ? obj->get_live_npc_num() : -1;
	return Usecode_value(num > 0 ? -num : 0);
}

USECODE_INTRINSIC(add_spell)
{
	// add_spell(spell# (0-71), ??, spoolbook).
	// Returns 0 if book already has that spell.
	Game_object *obj = get_item(parms[2]);
	if (!obj || obj->get_info().get_shape_class() != Shape_info::spellbook)
		return Usecode_value(0);
	Spellbook_object *book = (Spellbook_object *) (obj);
	if (!book)
		{
		cout << "Add_spell - Not a spellbook!" << endl;
		return Usecode_value(0);
		}
	return Usecode_value(book->add_spell(parms[0].get_int_value()));
}

USECODE_INTRINSIC(sprite_effect)
{
	// Display animation from sprites.vga.
	// show_sprite(sprite#, tx, ty, dx, dy, frame, length??);
					// ++++++Pass frame, length+++++++
	gwin->get_effects()->add_effect(
		new Sprites_effect(parms[0].get_int_value(),
		Tile_coord(parms[1].get_int_value(), parms[2].get_int_value(),
									0),
			parms[3].get_int_value(), parms[4].get_int_value()));
	return(no_ret);
}

USECODE_INTRINSIC(obj_sprite_effect)
{
	// obj_sprite_effect(obj, sprite#, -xoff, -yoff, dx, dy, 
	//						frame, length??)
	Game_object *obj = get_item(parms[0]);
	if (obj)
					// ++++++Pass frame, length+++++++
		gwin->get_effects()->add_effect(
			new Sprites_effect(parms[1].get_int_value(), obj,
			-parms[2].get_int_value(), -parms[3].get_int_value(),
			parms[4].get_int_value(), parms[5].get_int_value()));
	return(no_ret);
}

USECODE_INTRINSIC(explode)
{
	// Explode(obj??, item-to-explode, powder-keg-shape).
	Game_object *exp = get_item(parms[1]);
	if (!exp)
		return Usecode_value(0);
	Tile_coord pos = exp->get_tile();
					// Sprite 1,4,5 look like explosions.
	gwin->get_effects()->add_effect(new Explosion_effect(pos, exp));
	return Usecode_value(1);
}

USECODE_INTRINSIC(book_mode)
{
	// Display book or scroll.
	Text_gump *gump;
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		{
		return(no_ret);
		}

	// check for avatar read here
	bool do_serp = gwin->get_main_actor()->get_flag(Obj_flags::read) == false;
	
	if (obj->get_shapenum() == 707)		// Serpentine Scroll - Make SI only???
		gump = new Scroll_gump(do_serp);
	else if (obj->get_shapenum() == 705)	// Serpentine Book - Make SI only???
		gump = new Book_gump(do_serp);
	else if (obj->get_shapenum() == 797)
		gump = new Scroll_gump();
	else
		gump = new Book_gump();
	set_book(gump);
	return(no_ret);
}

USECODE_INTRINSIC(stop_time)
{	// stop_time(.25 secs).

	int length = parms[0].get_int_value();
	gwin->set_time_stopped(length*250);
	return no_ret;
}

USECODE_INTRINSIC(cause_light)
{
	// Cause_light(game_minutes??)

	gwin->add_special_light(parms[0].get_int_value());
	return no_ret;
}

USECODE_INTRINSIC(get_barge)
{
	// get_barge(obj) - returns barge object is part of or lying on.

	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value((Game_object*) NULL);
	return Usecode_value(Get_barge(obj));
}

USECODE_INTRINSIC(earthquake)
{
	int len = parms[0].get_int_value();
	gwin->get_tqueue()->add(Game::get_ticks() + 10,
		new Earthquake(len), (long) this);
	return(no_ret);
}

USECODE_INTRINSIC(is_pc_female)
{
	// Is player female?
	Usecode_value u(gwin->get_main_actor()->get_type_flag(Actor::tf_sex));
	return(u);
}

USECODE_INTRINSIC(armageddon)
{
	int cnt = gwin->get_num_npcs();
	Rectangle screen = gwin->get_win_tile_rect();
	for (int i = 1; i < cnt; i++)	// Most everyone dies.
		{			// Leave LB, Batlin, Hook.
		Actor *npc = gwin->get_npc(i);
		if (npc && i != 26 && i != 23 && npc->get_shapenum() != 506 &&
		    !npc->is_dead())
			{
			const char *text[] = {"Aiiiieee!", "Noooo!", "#!?*#%!"};
			const int numtext = sizeof(text)/sizeof(text[0]);
			Tile_coord loc = npc->get_tile();
			if (screen.has_point(loc.tx, loc.ty))
				npc->say(text[rand()%numtext]);
			npc->die();
			}
		}
	Actor_vector vec;		// Get any monsters nearby.
	gwin->get_main_actor()->find_nearby_actors(vec, c_any_shapenum, 40);
	for (Actor_vector::const_iterator it = vec.begin(); it != vec.end();
									++it)
		{
		Actor *act = *it;
		if (act->is_monster())
			act->die();
		}
	gwin->armageddon = true;
	return no_ret;
}

USECODE_INTRINSIC(halt_scheduled)
{
	// Halt_scheduled(item)
	Game_object *obj = get_item(parms[0]);
	if (obj)
		Usecode_script::terminate(obj);
	return(no_ret);
}

USECODE_INTRINSIC(lightning)
{
					// 1 sec. is long enough for 1 flash.
	gwin->get_effects()->add_effect(new Lightning_effect(1000));
	return no_ret;
}

USECODE_INTRINSIC(get_array_size)
{
	int cnt;
	if (parms[0].is_array())	// An array?  We might return 0.
		cnt = parms[0].get_array_size();
	else				// Not an array?  Usecode wants a 1.
		cnt = 1;
	Usecode_value u(cnt);
	return(u);
}

USECODE_INTRINSIC(mark_virtue_stone)
{
	Game_object *obj = get_item(parms[0]);
	if (obj->get_info().get_shape_class() == Shape_info::virtue_stone)
		{
		Virtue_stone_object *vs = (Virtue_stone_object *) (obj);
		vs->set_pos(obj->get_outermost()->get_tile());
		}
	return no_ret;
}

USECODE_INTRINSIC(recall_virtue_stone)
{
	Game_object *obj = get_item(parms[0]);
	if (obj->get_info().get_shape_class() == Shape_info::virtue_stone)
		{
		Virtue_stone_object *vs = (Virtue_stone_object *) (obj);
		gwin->get_gump_man()->close_all_gumps();
					// Pick it up if necessary.
		if (!obj->get_owner())
			{		// Go through whole party.
			obj->remove_this(1);
			Usecode_value party = get_party();
			int cnt = party.get_array_size();
			int i;
			for (i = 0; i < cnt; i++)
				{
				Game_object *npc = get_item(party.get_elem(i));
				if (npc && npc->add(obj))
					break;
				}
			if (i == cnt)	// Failed?  Force it on Avatar.
				gwin->get_main_actor()->add(obj, 1);
			}
		Tile_coord t = vs->get_pos();
		if (t.tx > 0 || t.ty > 0)
			gwin->teleport_party(t);
		}
	return no_ret;
}

USECODE_INTRINSIC(apply_damage)
{
	// apply_damage(??str?, hps, ??type?, NPC);
	int hps = parms[1].get_int_value();
	Actor *npc = as_actor(get_item(parms[3]));
	if (npc)			// Treat avatar as attacker.
		npc->reduce_health(hps, gwin->get_main_actor());
	return Usecode_value(1);	// ?? Guessing.
}

USECODE_INTRINSIC(is_pc_inside)
{
	Usecode_value u(gwin->is_main_actor_inside());
	return(u);
}

USECODE_INTRINSIC(get_timer)
{
	int tnum = parms[0].get_int_value();
	int ret;
	if (tnum >= 0 && tnum < (int)(sizeof(timers)/sizeof(timers[0])))
					// Return 0 if not set.
		ret = timers[tnum] > 0 ?
			(gclock->get_total_hours() - timers[tnum]) : 0;
	else
		{
		cerr << "Attempt to use invalid timer " << tnum << endl;
		ret = 0;
		}
	return Usecode_value(ret);
}

USECODE_INTRINSIC(set_timer)
{
	int tnum = parms[0].get_int_value();
	if (tnum >= 0 && tnum < (int)(sizeof(timers)/sizeof(timers[0])))
		timers[tnum] = gclock->get_total_hours();
	else
		cerr << "Attempt to use invalid timer " << tnum << endl;
	return(no_ret);
}

USECODE_INTRINSIC(wearing_fellowship)
{
	Game_object *obj = gwin->get_main_actor()->get_readied(Actor::neck);
	if (obj && obj->get_shapenum() == 955 && obj->get_framenum() == 1)
		return Usecode_value(1);
	else
		return Usecode_value(0);
}

USECODE_INTRINSIC(mouse_exists)
{
	Usecode_value u(1);
	return(u);
}

USECODE_INTRINSIC(get_speech_track)
{
	// Get speech track set by 0x74 or 0x8f.
	return Usecode_value(speech_track);
}

USECODE_INTRINSIC(flash_mouse)
{
	// flash_mouse(code)
	Mouse::Mouse_shapes shape;
	switch (parms[0].need_int_value())
		{
	case 2:
		shape = Mouse::outofrange; break;
	case 3:
		shape = Mouse::outofammo; break;
	case 4:
		shape = Mouse::tooheavy; break;
	case 5:
		shape = Mouse::wontfit; break;
	case 0:
	case 1:
	default:
		shape = Mouse::redx; break;
		}
	Mouse::mouse->flash_shape(shape);
	return (no_ret);
}

USECODE_INTRINSIC(get_item_frame_rot)
{
	// Same as get_item_frame, but (guessing!) include rotated bit.
	Game_object *obj = get_item(parms[0]);
	return Usecode_value(obj ? obj->get_framenum() : 0);
}

USECODE_INTRINSIC(set_item_frame_rot)
{	// Set entire frame, including rotated bit.
	set_item_frame(get_item(parms[0]), parms[1].get_int_value(), 0, 1);
	return(no_ret);
}

USECODE_INTRINSIC(on_barge)
{
	// Only used once for BG, in usecode for magic-carpet.
	// For SI, used for turtle.
	// on_barge()
	Barge_object *barge = Get_barge(gwin->get_main_actor());
	if (barge)
		{			// See if party is on barge.
		Rectangle foot = barge->get_tile_footprint();
		Actor *party[9];
		int cnt = gwin->get_party(party, 1);
		for (int i = 0; i < cnt; i++)
			{
			Actor *act = party[i];
			Tile_coord t = act->get_tile();
			if (!foot.has_point(t.tx, t.ty))
				return Usecode_value(0);
			}
					// Force 'gather()' for turtle.
		if (Game::get_game_type() == SERPENT_ISLE)
			barge->done();
		return Usecode_value(1);
		} 
	return Usecode_value(0);
//	return Usecode_value(1);
}

USECODE_INTRINSIC(get_container)
{
	// Takes itemref, returns container.
	Game_object *obj = get_item(parms[0]);
	Usecode_value u((Game_object *) NULL);
	if (obj)
		u = Usecode_value(obj->get_owner());
	return(u);
}

USECODE_INTRINSIC(remove_item)
{
	// Think it's 'delete object'.
	remove_item(get_item(parms[0]));
	return no_ret;
}

USECODE_INTRINSIC(reduce_health)
{
	// Reduce_health(npc, amount, ??property/flags??0?).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)			// Dies if health goes too low.
		npc->reduce_health(parms[1].get_int_value());
	return no_ret;
}

/*
 *	Convert Usecode spot # to ours (or -1 if not found).
 */

static int Get_spot(int ucspot)
	{
	int spot;
	switch (ucspot)
		{
	case 1:
		spot = Actor::lhand; break;
	case 2:
		spot = Actor::rhand; break;
	case 3:
		spot = Actor::neck; break;
	case 5:
		spot = Actor::hands2_spot; break;
	case 6:
		spot = Actor::lfinger; break;
	case 7:
		spot = Actor::rfinger; break;
	case 9:
		spot = Actor::head; break; 
	case 11:
		spot = Actor::belt; break;
	default:
		cerr << "Readied: spot #" << ucspot <<
						" not known yet" << endl;
		spot = -1;
		}
	return spot;
	}

USECODE_INTRINSIC(is_readied)
{
	// is_readied(npc, where, itemshape, frame (-359=any)).
	// Where:
	//   1=weapon hand, 
	//   2=other hand,
	//   6=one finger, 
	//   7=other finger,
	//   9=head
	//  20=???

	Actor *npc = as_actor(get_item(parms[0]));
	if (!npc)
		return Usecode_value(0);
	int where = parms[1].get_int_value();
	int shnum = parms[2].get_int_value();
	int frnum = parms[3].get_int_value();
					// Spot defined in Actor class.
	int spot = Get_spot(where);
	if (spot >= 0)
		{			// See if it's the right one.
		Game_object *obj = npc->get_readied(spot);
		if (obj && obj->get_shapenum() == shnum &&
		    (frnum == c_any_framenum || obj->get_framenum() == frnum))
			return Usecode_value(1);
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(get_readied)
{
	// get_readied(npc, where)
	// Where:
	//   1=weapon hand, 
	//   2=other hand,
	//   6=one finger, 
	//   7=other finger,
	//   9=head
	//  11=belt
	//  20=???

	Actor *npc = as_actor(get_item(parms[0]));
	if (!npc)
		return Usecode_value(0);
	int where = parms[1].get_int_value();
					// Spot defined in Actor class.
	int spot = Get_spot(where);
	if (spot >= 0)
		return Usecode_value(npc->get_readied(spot));
	return Usecode_value(0);
}

USECODE_INTRINSIC(restart_game)
{
	// Think it's 'restart game'.  
	// Happens if you die before leaving trinsic.
	Audio::get_ptr()->stop_music();
	quitting_time = QUIT_TIME_RESTART;		// Quit & restart.
	return(no_ret);
}

USECODE_INTRINSIC(start_speech)
{
	// Start_speech(num).  Also sets speech_track.
	bool okay = false;
	speech_track = parms[0].get_int_value();
	if (speech_track >= 0)
		okay = Audio::get_ptr()->start_speech(speech_track);
	if (!okay)			// Failed?  Clear faces.  (Fixes SI).
		init_conversation();
	return(Usecode_value(okay ? 1 : 0));
}

USECODE_INTRINSIC(is_water)
{
	// Is_water(pos).
	int size = parms[0].get_array_size();
	if (size >= 3)
		{
		Tile_coord t(parms[0].get_elem(0).get_int_value(),
			     parms[0].get_elem(1).get_int_value(),
			     parms[0].get_elem(2).get_int_value());
					// Didn't click on an object?
		int x = (t.tx - gwin->get_scrolltx())*c_tilesize,
		    y = (t.ty - gwin->get_scrollty())*c_tilesize;
		if (t.tz != 0 || gwin->find_object(x, y))
			return Usecode_value(0);
		ShapeID sid = gwin->get_flat(x, y);
		Shape_info& info = sid.get_info();
		return Usecode_value(info.is_water());
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(run_endgame)
{
	game->end_game(parms[0].get_int_value() != 0);
	// If successful enable menu entry and play credits afterwards
	if(parms[0].get_int_value() != 0) {
		std::ofstream endgameflg;
                U7open(endgameflg, "<SAVEGAME>/endgame.flg");
                endgameflg.close();
		game->show_credits();
	}
	quitting_time = QUIT_TIME_YES;
	return(no_ret);
}

USECODE_INTRINSIC(fire_cannon)
{
	// fire_cannon(cannon, dir, ballshape, dist?, cannonshape, cannonshape)

	Game_object *cannon = get_item(parms[0]);
					// Get direction (0,2,4, or 6).
	int dir = parms[1].get_int_value();
	dir = dir/2;			// 0, 1, 2, 3.
	int ball = parms[2].get_int_value();
	int dist = parms[3].get_int_value();
	int cshape = parms[4].get_int_value();
	Tile_coord pos = cannon->get_tile();
	short blastoff[8] = {-2, -5, 1, -2, -2, 1, -5, -2};
	Tile_coord blastpos = pos + Tile_coord(
				blastoff[2*dir], blastoff[2*dir + 1], 0);
					// Sprite 5 is a small explosion.
	gwin->get_effects()->add_effect(new Sprites_effect(5, blastpos));
	Tile_coord dest = pos;
	switch (dir)			// Figure where to aim.
		{
	case 0:	dest.ty -= dist; break;
	case 1: dest.tx += dist; break;
	case 2: dest.ty += dist; break;
	case 3: dest.tx -= dist; break;
		}
					// Shoot cannonball.
	gwin->get_effects()->add_effect(
			new Projectile_effect(blastpos, dest, ball, cshape));
	return no_ret;
}

USECODE_INTRINSIC(nap_time)
{
	// nap_time(bed)
	const char *msgs[] = {"Avatar!  Please restrain thyself!",
			"Hast thou noticed that this bed is occupied?",
			"Please, Avatar, the resident of this bed may not be desirouth of company at the moment."
			};
	const int nummsgs = sizeof(msgs)/sizeof(msgs[0]);
	Game_object *bed = get_item(parms[0]);
	if (!bed)
		return no_ret;
					// !!! Seems 622 handles sleeping.
	Actor_vector npcs;		// See if bed is occupied by an NPC.
	int cnt = bed->find_nearby_actors(npcs, c_any_shapenum, 0);
	if (cnt > 0)
		{
		Actor_vector::const_iterator it;
		for (it = npcs.begin(); it != npcs.end(); ++it)
			{
			Game_object *npc = *it;
			int zdiff = npc->get_lift() - bed->get_lift();
			if (npc != gwin->get_main_actor() &&
			    (npc->get_framenum()&0xf) == Actor::sleep_frame &&
						zdiff <= 2 && zdiff >= -2)
				break;	// Found one.
			}
		if (it != npcs.end())
			{		// Show party member's face.
			int party_cnt = get_party_count();
			int npcnum = party_cnt ? get_party_member(
						rand()%party_cnt) : 356;
			Usecode_value actval(-npcnum), frval(0);
			show_npc_face(actval, frval);
			conv->show_npc_message(msgs[rand()%nummsgs]);
			remove_npc_face(actval);
			gwin->get_main_actor()->set_schedule_type(
						Schedule::follow_avatar);
			return no_ret;
			}
		}
					// Give him a chance to get there (at
					//   most 5 seconds.)
	Wait_for_arrival(gwin->get_main_actor(), bed->get_tile(),
								5000);
	call_usecode(0x622, bed, double_click);
	return(no_ret);
}

USECODE_INTRINSIC(advance_time)
{
	// Incr. clock by (parm[0]*.04min.).
	gclock->increment(parms[0].get_int_value()/25);
	return(no_ret);
}

USECODE_INTRINSIC(in_usecode)
{
	// in_usecode(item):  Return 1 if executing usecode on parms[0].

	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	return Usecode_value(Usecode_script::find(obj) != 0);
}

USECODE_INTRINSIC(attack_avatar)
{
	// Attack thieving Avatar.
	gwin->attack_avatar();
	return no_ret;
}

USECODE_INTRINSIC(path_run_usecode)
{
	// exec(loc(x,y,z)?, usecode#, itemref, eventid).
	// Think it should have Avatar walk path to loc, return 0
	//  if he can't get there (and return), 1 if he can.
	Usecode_value ava(gwin->get_main_actor());
	return Usecode_value(path_run_usecode(ava, parms[0], parms[1],
				parms[2], parms[3],
					// SI:  Look for free spot. (Guess).
			Game::get_game_type() == SERPENT_ISLE));
}

USECODE_INTRINSIC(close_gumps)
{
	// Guessing+++++ close all gumps.
	if (!gwin->is_dragging())	// NOT while dragging stuff.
		gwin->get_gump_man()->close_all_gumps();
	return(no_ret);
}

USECODE_INTRINSIC(in_gump_mode)
{
								// No persistent
	return Usecode_value(gwin->get_gump_man()->showing_gumps(true));
}

USECODE_INTRINSIC(is_not_blocked)
{
	// Is_not_blocked(tile, shape, frame (or -359).
	Usecode_value fail(0);
					// Parm. 0 should be tile coords.
	Usecode_value& pval = parms[0];
	if (pval.get_array_size() < 3)
		return fail;
	Tile_coord tile(pval.get_elem(0).get_int_value(),
			pval.get_elem(1).get_int_value(),
			pval.get_elem(2).get_int_value());
	int shapenum = parms[1].get_int_value();
	int framenum = parms[2].get_int_value();
					// Find out about given shape.
	Shape_info& info = ShapeID::get_info(shapenum);
	Rectangle footprint(
		tile.tx - info.get_3d_xtiles(framenum) + 1,
		tile.ty - info.get_3d_ytiles(framenum) + 1,
		info.get_3d_xtiles(framenum), info.get_3d_ytiles(framenum));
	int new_lift;
	int blocked = Map_chunk::is_blocked(
		info.get_3d_height(), tile.tz, 
		footprint.x, footprint.y, footprint.w, footprint.h,
		new_lift, MOVE_ALL_TERRAIN);
					// Okay?
	if (!blocked && new_lift == tile.tz)
		return Usecode_value(1);
	else
		return Usecode_value(0);
}

USECODE_INTRINSIC(direction_from)
{
	// ?Direction from parm[0] -> parm[1].
	// Rets. 0-7, with 0 = North, 1 = Northeast, etc.
	// Same as 0x1a??
	Usecode_value u=find_direction(parms[0], parms[1]);
	return(u);
}

/*
 *	Test for a 'moving barge' flag.
 */

static int Is_moving_barge_flag
	(
	int fnum
	)
	{
	if (Game::get_game_type() == BLACK_GATE)
		{
		return fnum == (int) Obj_flags::on_moving_barge ||
			fnum == (int) Obj_flags::in_motion;
		}
	else				// SI.
		{
		return fnum == (int) Obj_flags::si_on_moving_barge ||
					// Ice raft needs this one:
			fnum == (int) Obj_flags::on_moving_barge ||
			fnum == (int) Obj_flags::in_motion;
		}
	}

USECODE_INTRINSIC(get_item_flag)
{
	// Get npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	int fnum = parms[1].get_int_value();
					// Special cases:
	if (Is_moving_barge_flag(fnum))
		{			// Test for moving barge.
		Barge_object *barge;
		if (!gwin->get_moving_barge() || !(barge = Get_barge(obj)))
			return Usecode_value(0);
		return Usecode_value(barge == gwin->get_moving_barge());
		}
	else if (fnum == (int) Obj_flags::okay_to_land)
		{			// Okay to land flying carpet?
		Barge_object *barge = Get_barge(obj);
		if (!barge)
			return Usecode_value(0);
		return Usecode_value(barge->okay_to_land());
		}
	else if (fnum == (int) Obj_flags::cant_die)
		{
		Monster_info *inf = obj->get_info().get_monster_info();
		return Usecode_value(inf != 0 && inf->cant_die());
		}
					// +++++0x18 is used in testing for
					//   blocked gangplank. What is it?????
	else if (fnum == 0x18 && Game::get_game_type() == BLACK_GATE)
		return Usecode_value(1);

	Usecode_value u(obj->get_flag(fnum) != 0);
	return(u);
}

USECODE_INTRINSIC(set_item_flag)
{
	// Set npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	int flag = parms[1].get_int_value();
	if (!obj)
		return no_ret;
	switch (flag)
		{
	case Obj_flags::dont_render:
		obj->set_flag(flag);
					// Get out of combat mode.
		if (obj == gwin->get_main_actor() && 
			Game::get_game_type() == SERPENT_ISLE &&
							gwin->in_combat())
			gwin->toggle_combat();
					// Show change in status.
		gwin->set_all_dirty();
		break;
	case Obj_flags::invisible:
		if (as_actor(obj))	// Only NPC's for now.
			{
			obj->set_flag(flag);
			gwin->add_dirty(obj);
			}
		break;
	default:
		obj->set_flag(flag);
		if (Is_moving_barge_flag(flag))
			{		// Set barge in motion.
			Barge_object *barge = Get_barge(obj);
			if (barge)
				gwin->set_moving_barge(barge);
			}
			break;
		}
	return(no_ret);
}

USECODE_INTRINSIC(clear_item_flag)
{
	// Clear npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	int flag = parms[1].get_int_value();
	if (obj)
		{
		obj->clear_flag(flag);
		if (flag == Obj_flags::dont_render)
			{	// Show change in status.
			show_pending_text();	// Fixes Lydia-tatoo.
			gwin->set_all_dirty();
			}
		else if (Is_moving_barge_flag(flag))
			{	// Stop barge object is on or part of.
			Barge_object *barge = Get_barge(obj);
			if (barge && barge == gwin->get_moving_barge())
				gwin->set_moving_barge(0);
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(set_path_failure)
{
	// set_path_failure(fun, itemref, eventid) for the last NPC in
	//  a path_run_usecode() call.

	int fun = parms[0].get_int_value(),
	    eventid = parms[2].get_int_value();
	Game_object *item = get_item(parms[1]);
	if (path_npc && item)		// Set in path_run_usecode().
		{
		If_else_path_actor_action *action = 
			path_npc->get_action() ?
			path_npc->get_action()->as_usecode_path() : 0;
		if (action)		// Set in in path action.
			action->set_failure(
				new Usecode_actor_action(fun, item, eventid));
		}
	return no_ret;

#if 0	/* ++++++Think it's 'set failure'. */
	// run_usecode(fun, itemref, eventid)
	Game_object *obj = get_item(parms[1]);
	int ucfun = parms[0].get_int_value();
#if 1	/* ++++++++Another guess to try to fix SI problems. */
	if (obj && Game::get_game_type() == SERPENT_ISLE &&
	    Usecode_script::find(obj))	// Usecode scheduled?
		return no_ret;
#endif
	if (obj)
		call_usecode(ucfun, obj, 
				(Usecode_events) parms[2].get_int_value());
#endif
	return(no_ret);
}

USECODE_INTRINSIC(fade_palette)
{
	// Fade(cycles?, ??(always 1), in_out (0=fade to black, 1=fade in)).
	int cycles = parms[0].get_int_value();
	int inout = parms[2].get_int_value();
	if (inout == 0)
		show_pending_text();	// Make sure prev. text was seen.
	gwin->get_pal()->fade(cycles, inout);
	return(no_ret);
}

USECODE_INTRINSIC(get_party_list2)
{
	// Return party.  Same as 0x23
	// Probably returns a list of everyone with (or without) some flag
	// List of live chars? Dead chars?
	Usecode_value u(get_party());
	return(u);
}

//+++++++Remove this if things work okay.  It's just get_party_list().
USECODE_INTRINSIC(get_party_ids)
{
	Usecode_value u(get_party());
	return u;
#if 0	/* ++++++Pretty sure this is wrong.  Just return party objs. */
	// Return list of party ID's, including -356 for Avatar.
	Usecode_value arr(1 + party_count, 0);
					// Add avatar.
	Usecode_value aval(-356);
	arr.put_elem(0, aval);	
	int num_added = 1;
	for (int i = 0; i < party_count; i++)
		{
		Usecode_value val(party[i]);
		arr.put_elem(num_added++, val);
		}
	return arr;
#endif
}

USECODE_INTRINSIC(set_camera)
{
	// Set_camera(actor)
	Actor *actor = as_actor(get_item(parms[0]));
	if (actor)
		gwin->set_camera_actor(actor);
	return no_ret;
}

USECODE_INTRINSIC(in_combat)
{
	// Are we in combat mode?
	return Usecode_value(gwin->in_combat());
}

USECODE_INTRINSIC(center_view)
{
	// Center view around given item.
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		Tile_coord t = obj->get_tile();
		gwin->center_view(t);
		activate_cached(t);	// Mar-10-01 - For Test of Love.
		}
	return no_ret;
}

USECODE_INTRINSIC(get_dead_party)
{
	// Return list of dead companions' bodies.
	Usecode_value ret(dead_party_count, 0);
	for (int i = 0; i < dead_party_count; i++)
		{
		Game_object *body = gwin->get_body(dead_party[i]);
					// Body within 50 tiles (a guess)?
		if (body && body->distance(gwin->get_main_actor()) < 50)
			{
			Usecode_value v(body);
			ret.put_elem(i, v);
			}
		}
	return ret;
}

USECODE_INTRINSIC(play_sound_effect)
{
	if (num_parms < 1) return(no_ret);
	// Play music(isongnum).
	COUT("Sound effect " << parms[0].get_int_value() << " request in usecode");

	Audio::get_ptr()->play_sound_effect (parms[0].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(play_sound_effect2)
{
	if (num_parms < 2) return(no_ret);
	// Play music(songnum, item).
	Game_object *obj = get_item(parms[1]);
	int volume = SDL_MIX_MAXVOLUME;	// Set volume based on distance.
	int dir = 0;
	if (obj)
		{
		Tile_coord apos = gwin->get_main_actor()->get_tile();
		Tile_coord opos = obj->get_tile();
		int dist = apos.distance(opos);
		if (dist)
			{		// 160/8 = 20 tiles. 20*20=400.
			volume = (SDL_MIX_MAXVOLUME*64)/(dist*dist);
			if (volume < 8)
				volume = 8;
			else if (volume > SDL_MIX_MAXVOLUME)
				volume = SDL_MIX_MAXVOLUME;
			dir = Get_direction16(apos.ty - opos.ty,
						opos.tx - apos.tx);
			}
		}
#ifdef DEBUG
	cout << "Sound effect(2) " << parms[0].get_int_value() << 
		" request in usecode with volume = " << volume 
		<< ", dir = " << dir << endl;
#endif
	Audio::get_ptr()->play_sound_effect (parms[0].get_int_value(), volume,
									dir);
	return(no_ret);
}

USECODE_INTRINSIC(get_npc_id)
{
	Actor *actor = as_actor(get_item(parms[0]));
	if (!actor) return(no_ret);
	return Usecode_value (actor->get_ident());
}

USECODE_INTRINSIC(set_npc_id)
{
	Actor *actor = as_actor(get_item(parms[0]));
	if (actor) actor->set_ident(parms[1].get_int_value());
	return(no_ret);
}


USECODE_INTRINSIC(add_cont_items)
{
	// Add items(num, item, ??quality?? (-359), frame (or -359), T/F).
	return add_cont_items(parms[0], parms[1], parms[2],
					parms[3], parms[4], parms[5]);
}

// Is this SI Only
USECODE_INTRINSIC(remove_cont_items)
{
	// Add items(num, item, ??quality?? (-359), frame (or -359), T/F).
	return remove_cont_items(parms[0], parms[1], parms[2],
					parms[3], parms[4], parms[5]);
}

/*
 *	SI-specific functions.
 */

USECODE_INTRINSIC(show_npc_face0)
{
	// Show_npc_face0(npc, frame).  Show in position 0.
	show_npc_face(parms[0], parms[1], 0);
	return no_ret;
}

USECODE_INTRINSIC(show_npc_face1)
{
	// Show_npc_face1(npc, frame).  Show in position 1.
	show_npc_face(parms[0], parms[1], 1);
	return no_ret;
}

USECODE_INTRINSIC(remove_npc_face0)
{
	show_pending_text();
	conv->remove_slot_face(0);
	return no_ret;
}

USECODE_INTRINSIC(remove_npc_face1)
{
	show_pending_text();
	conv->remove_slot_face(1);
	return no_ret;
}

USECODE_INTRINSIC(set_conversation_slot)
{
	// set_conversation_slot(0 or 1) - Choose which face is talking.
	conv->set_slot(parms[0].get_int_value());
	return no_ret;
}

USECODE_INTRINSIC(init_conversation)
{
	init_conversation();
	return no_ret;
}

USECODE_INTRINSIC(end_conversation)
{
	show_pending_text();		// Wait for click if needed.
	conv->init_faces();		// Removes faces from screen.
	gwin->set_all_dirty();
	return no_ret;
}

USECODE_INTRINSIC(si_path_run_usecode)
{
	// exec(npc, loc(x,y,z)?, eventid, itemref, usecode#, ??true/false).
	// Schedule Npc to walk to loc and then execute usecode.
					// Guessing:
	int always = parms[5].get_int_value();
	path_run_usecode(parms[0], parms[1], parms[4], parms[3], parms[2], 1,
							always);
	return no_ret;

#if 0	/* Old way++++++++ */
	Actor *npc = as_actor(get_item(parms[0]));
	Game_object *obj = get_item(parms[3]);
	int sz = parms[1].get_array_size();
	if (!npc || !obj || sz < 2)
		{
		CERR("Si_path_run_usecode: bad inputs");
		return no_ret;		// Bad data.
		}
	Tile_coord src = npc->get_tile();
	Tile_coord dest(parms[1].get_elem(0).get_int_value(),
			parms[1].get_elem(1).get_int_value(),
			sz == 3 ? parms[1].get_elem(2).get_int_value() : 0);
	Tile_coord start = dest;
	if (dest != src)
		{
		dest.tx = -1;		// Look outwards (with npc ht=4).
		for (int i = 0; dest.tx == -1 && i < 4; i++)
			dest = Game_object::find_unblocked_tile(start, i, 4);
		}
	if (dest.tx == -1)
		{			// Try again at npc's level.
		start.tz = src.tz;
		for (int i = 0; dest.tx == -1 && i < 4; i++)
			dest = Game_object::find_unblocked_tile(start, i, 4);
		if (dest.tx == -1)
			{
			cout << "Couldn't find free tile" << endl;
			return no_ret;
			}
		}
					// Walk there and execute.
		//++++++Wrong!  Mustn't teleport if no path!
	npc->set_action(Actor_action::create_action_sequence(
			npc, dest, new Usecode_actor_action(
					parms[4].get_elem0().get_int_value(), 
					obj, parms[2].get_int_value()),
			false, true));	// Don't teleport if path not found.
	npc->start(200, 0);		// Get into time queue.
	return no_ret;
#endif
}

USECODE_INTRINSIC(error_message)
{
	// exec(array)
	// Output everything to stdout

	for (int i = 0; i < num_parms; i++)
	{
		if (parms[i].is_int()) std::cout << parms[i].get_int_value() ;
		else if (parms[i].is_ptr()) std::cout << parms[i].get_ptr_value();
		else if (!parms[i].is_array()) std::cout << parms[i].get_str_value();
		else for (int j = 0; j < parms[i].get_array_size(); j++)
		{
			if (parms[i].get_elem(j).is_int()) std::cout << parms[i].get_elem(j).get_int_value() ;
			else if (parms[i].get_elem(j).is_ptr()) std::cout << parms[i].get_elem(j).get_ptr_value();
			else if (!parms[i].get_elem(j).is_array()) std::cout << parms[i].get_elem(j).get_str_value();
		}
	}

	std::cout << std::endl;
	return no_ret;
}

USECODE_INTRINSIC(set_polymorph)
{
	// exec(npc, shape).
	// Npc's shape is change to shape.
	Actor *actor = as_actor(get_item(parms[0]));
	if (actor) actor->set_polymorph(parms[1].get_int_value());
	return no_ret;
}

USECODE_INTRINSIC(set_new_schedules)
{
	// set_new_schedules ( npc, time, activity, [x, y] )
	//
	// or
	//
	// set_new_schedules ( npc, [	time1, time2, ...],
	//			[activity1, activity2, ...],
	//			[x1,y1, x2, y2, ...] )
	//

	Actor *actor = as_actor(get_item(parms[0]));

	// If no actor return
	if (!actor) return no_ret;

	int count = parms[1].is_array()?parms[1].get_array_size():1;
	Schedule_change *list = new Schedule_change[count];

	if (!parms[1].is_array())
	{
		int time = parms[1].get_int_value();
		int sched = parms[2].get_int_value();
		int tx = parms[3].get_elem(0).get_int_value();
		int ty = parms[3].get_elem(1).get_int_value();
		list[0].set(tx, ty, sched, time);
	}
	else for (int i = 0; i < count; i++)
	{
		int time = parms[1].get_elem(i).get_int_value();
		int sched = parms[2].get_elem(i).get_int_value();
		int tx = parms[3].get_elem(i*2).get_int_value();
		int ty = parms[3].get_elem(i*2+1).get_int_value();
		list[i].set(tx, ty, sched, time);
	}

	actor->set_schedules(list, count);

	return no_ret;
}

USECODE_INTRINSIC(revert_schedule)
{
	// revert_schedule(npc)
	// Reverts the schedule of the npc to the saved state in
	// <STATIC>/schedule.dat

	Actor *actor = as_actor(get_item(parms[0]));
	if (actor) gwin->revert_schedules(actor);

	return no_ret;
}

USECODE_INTRINSIC(run_schedule)
{
	// run_schedule(npc)
	// I think this is actually reset activity to current
	// scheduled activity - Colourless
	Actor *actor = as_actor(get_item(parms[0]));
	
	if (actor)
	{
		actor->update_schedule(gclock->get_hour()/3, 7);

	}

	return no_ret;
}

USECODE_INTRINSIC(get_temperature)
{
	Actor *npc = as_actor(get_item(parms[0]));
	return Usecode_value(npc ? npc->get_temperature() : 0);
}

USECODE_INTRINSIC(set_temperature)
{
	// set_temperature(npc, value (0-63)).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		npc->set_temperature(parms[1].get_int_value());
	return no_ret;
}

#if 0	/* +++++Not used at the moment. */
USECODE_INTRINSIC(add_removed_npc)
{
	// move_offscreen(npc, x, y) - I think (seems good) I think
	//
	// returns false if not moved
	// else, moves the object to the closest position off 

	// Actor we want to move
	Actor *actor = as_actor(get_item(parms[0]));

	// Need to check superchunk
	int cx = actor->get_cx();
	int cy = actor->get_cx();
	int scx = cx / c_chunks_per_schunk;
	int scy = cy / c_chunks_per_schunk;
	int scx2 = parms[1].get_int_value() / c_tiles_per_schunk;
	int scy2 = parms[2].get_int_value() / c_tiles_per_schunk;

	// Are the coords are good
	if ( (cx == 0xff && scx2) || (cx != 0xff && scx != scx2) ||
		(cy == 0xff && scy2) || (cy != 0xff && scy != scy2) ) 
		return (Usecode_value(false));


	// Ok they were good, now move it


	// Get the tiles around the edge of the screen
	Rectangle rect = gwin->get_win_tile_rect();

	int sx = rect.x;		// Tile coord of x start
	int ex = rect.x + rect.w;	// Tile coord of x end
	int sy = rect.y;		// y start
	int ey = rect.y + rect.h;	// x end

	// The height of the Actor we are checking
	int height = actor->get_info().get_3d_height();

	int i = 0, nlift = 0;
	int tx, ty;

	// Avatars coords
	Tile_coord av = gwin->get_main_actor()->get_tile();;

	Tile_coord close;	// The tile coords of the closest tile
	int dist = -1;		// The distance

	cy = sy/c_tiles_per_chunk;
	ty = sy%c_tiles_per_chunk;
	cout << "1" << endl;
	for (i = 0; i < rect.w; i++)
	{
		cx = (sx+i)/c_tiles_per_chunk;
		tx = (sx+i)%c_tiles_per_chunk;

		Map_chunk *clist = gmap->get_chunk_safely(cx, cy);
		clist->setup_cache();
		if (!clist->is_blocked (height, 0, tx, ty, nlift, actor->get_type_flags(), 1))
		{
			Tile_coord cur(tx+cx*c_tiles_per_chunk, ty+cy*c_tiles_per_chunk, nlift);
			if (cur.distance(av) < dist || dist == -1)
			{
				dist = cur.distance(av);
				cout << "(" << cur.tx << ", " << cur.ty << ") " << dist << endl;
				close = cur;
			}
		}
	}

	cx = ex/c_tiles_per_chunk;
	tx = ex%c_tiles_per_chunk;
	cout << "2" << endl;
	for (i = 0; i < rect.h; i++)
	{
		cy = (sy+i)/c_tiles_per_chunk;
		ty = (sy+i)%c_tiles_per_chunk;

		Map_chunk *clist = gmap->get_chunk_safely(cx, cy);
		clist->setup_cache();
		if (!clist->is_blocked (height, 0, tx, ty, nlift, actor->get_type_flags(), 1))
		{
			Tile_coord cur(tx+cx*c_tiles_per_chunk, ty+cy*c_tiles_per_chunk, nlift);
			if (cur.distance(av) < dist || dist == -1)
			{
				dist = cur.distance(av);
				cout << "(" << cur.tx << ", " << cur.ty << ") " << dist << endl;
				close = cur;
			}
		}
	}

	cy = ey/c_tiles_per_chunk;
	ty = ey%c_tiles_per_chunk;
	cout << "3" << endl;
	for (i = 0; i < rect.w; i++)
	{
		cx = (ex-i)/c_tiles_per_chunk;
		tx = (ex-i)%c_tiles_per_chunk;

		Map_chunk *clist = gmap->get_chunk_safely(cx, cy);
		clist->setup_cache();
		if (!clist->is_blocked (height, 0, tx, ty, nlift, actor->get_type_flags(), 1))
		{
			Tile_coord cur(tx+cx*c_tiles_per_chunk, ty+cy*c_tiles_per_chunk, nlift);
			if (cur.distance(av) < dist || dist == -1)
			{
				dist = cur.distance(av);
				cout << "(" << cur.tx << ", " << cur.ty << ") " << dist << endl;
				close = cur;
			}
		}
	}

	cx = sx/c_tiles_per_chunk;
	tx = sx%c_tiles_per_chunk;
	cout << "4" << endl;
	for (i = 0; i < rect.h; i++)
	{
		cy = (ey-i)/c_tiles_per_chunk;
		ty = (ey-i)%c_tiles_per_chunk;

		Map_chunk *clist = gmap->get_chunk_safely(cx, cy);
		clist->setup_cache();
		if (!clist->is_blocked (height, 0, tx, ty, nlift, actor->get_type_flags(), 1))
		{
			Tile_coord cur(tx+cx*c_tiles_per_chunk, ty+cy*c_tiles_per_chunk, nlift);
			if (cur.distance(av) < dist || dist == -1)
			{
				dist = cur.distance(av);
				cout << "(" << cur.tx << ", " << cur.ty << ") " << dist << endl;
				close = cur;
			}
		}
	}

	if (dist != -1)
	{
		actor->move(close);
		return (Usecode_value(true));
	}

	return (Usecode_value(false));
}
#endif

USECODE_INTRINSIC(approach_avatar)
{
	// Approach_avatar(npc, ?, ?).
	// Actor we want to move
	Actor *actor = as_actor(get_item(parms[0]));
	if (!actor || actor->is_dead())
		return Usecode_value(0);
					// Guessing!! If already close...
	if (actor->distance(gwin->get_main_actor()) < 10)
		return Usecode_value(1);
					// Approach, and wait.
	if (!actor->approach_another(gwin->get_main_actor(), true))
		return Usecode_value(0);
	return Usecode_value(1);
	}

USECODE_INTRINSIC(telekenesis)
{
	// telekenesis(fun#) - Save item for executing Usecode on.
	telekenesis_fun = parms[0].get_int_value();
	return no_ret;
}

USECODE_INTRINSIC(a_or_an)
{
	// a_or_an (word)
	// return a/an depending on 'word'

	if (strchr("aeiouyAEIOUY", (parms[0].get_str_value())[0]) == 0)
		return (Usecode_value("a"));
	else
		return (Usecode_value("an"));

}

USECODE_INTRINSIC(remove_from_area)
{
	// Remove_from_area(shapenum, framenum, [x,y]from, [x,y]to).
	int shnum = parms[0].get_int_value(), frnum = parms[1].get_int_value();
	int fromx = parms[2].get_elem(0).get_int_value(),
	    fromy = parms[2].get_elem(1).get_int_value(),
	    tox   = parms[3].get_elem(0).get_int_value(),
	    toy   = parms[3].get_elem(1).get_int_value();
	Rectangle area(fromx, fromy, tox - fromx + 1, toy - fromy + 1);
	if (area.w <= 0 || area.h <= 0)
		return no_ret;
	Game_object_vector vec;		// Find objects.
	Map_chunk::find_in_area(vec, area, shnum, frnum);
					// Remove them.
	for (Game_object_vector::iterator it = vec.begin(); it != vec.end();
								it++)
		{
		Game_object *obj = *it;
		gwin->add_dirty(obj);
		obj->remove_this(0);
		}
	return no_ret;
}

USECODE_INTRINSIC(infravision)
{
	// infravision(npc, onoff)
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc && npc->is_in_party())
		{
		if (parms[1].get_int_value())
			{		// On?
			cheat.set_infravision(true);
			gwin->get_pal()->set(0);
			}
		else
			{
			cheat.set_infravision(false);
			gclock->set_palette();
			}
		}
	return no_ret;
}

// parms[0] = quality of key to be added
USECODE_INTRINSIC(add_to_keyring)
{
	getKeyring()->addkey(parms[0].get_int_value());

	return no_ret;
}

// parms[0] = quality of key to check
// returns true if key is on keyring

USECODE_INTRINSIC(is_on_keyring)
{
	if (getKeyring()->checkkey(parms[0].get_int_value()))
		return Usecode_value(true);
	else
		return Usecode_value(false);
}

USECODE_INTRINSIC(save_pos)
{
	// save_pos(item).
	Game_object *item = get_item(parms[0]);
	if (item)
		saved_pos = item->get_tile();
	return no_ret;
}

USECODE_INTRINSIC(teleport_to_saved_pos)
{
	// teleport_to_saved_pos(actor).  Only supported for Avatar for now.
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc == gwin->get_main_actor())
		{			// Bad value?
		if (saved_pos.tx < 0 || saved_pos.tx >= c_num_tiles)
					// Fix old games.  Send to Monitor.
			saved_pos = Tile_coord(719, 2608, 1);
		gwin->teleport_party(saved_pos);
		}
	return no_ret;
}

USECODE_INTRINSIC(get_item_usability)
{
	// Complete guess:  Returns 1 for grabbable items (for fetch spell).
	Game_object *obj = get_item(parms[0]);
	if (!obj || !obj->is_dragable())
		return Usecode_value(0);
	else
		return Usecode_value(1);// Only 1 I'm pretty sure of.
}

USECODE_INTRINSIC(get_skin_colour)
{
	// Gets skin colour of avatar. 0 (wh), 1 (br) or 2 (bl)
	Main_actor *av = gwin->get_main_actor();
	return Usecode_value(av->get_skin_color());
}

