/*
Copyright (C) 2000 The Exult team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucsched.h"
#include "Audio.h"
#include "barge.h"
#include "game.h"
#include "frameseq.h"
#include "gamewin.h"

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::hex;
using std::setfill;
using std::setw;

int Scheduled_usecode::count = 0;
Scheduled_usecode *Scheduled_usecode::first = 0;

/*
 *	Create.
 */

Scheduled_usecode::Scheduled_usecode
	(
	Usecode_machine *usecode,
	Usecode_value& oval, 
	Usecode_value& aval
	) : objval(oval), arrval(aval), i(0), frame_index(0), no_halt(0)
	{
	cnt = arrval.get_array_size();
	obj = usecode->get_item(objval);
					// Pure kludge for SI wells:
	if (objval.get_array_size() == 2 && 
	    Game::get_game_type() == SERPENT_ISLE &&
	    obj && obj->get_shapenum() == 470 && obj->get_lift() == 0)
		{			// We want the TOP of the well.
		Usecode_value v2 = objval.get_elem(1);
		Game_object *o2 = usecode->get_item(v2);
		if (o2->get_shapenum() == obj->get_shapenum() && 
		    o2->get_lift() == 2)
			{
			objval = v2;
			obj = o2;
			}
		}
	objpos = obj ? obj->get_abs_tile_coord() : Tile_coord(-1, -1, -1);
					// Not an array?
	if (!cnt && !arrval.is_array())
		cnt = 1;		// Get_elem(0) works for non-arrays.
	count++;			// Keep track of total.
	next = first;			// Put in chain.
	prev = 0;
	if (first)
		first->prev = this;
	first = this;
	int opval0 = arrval.get_elem(0).get_int_value();
	if (opval0 == 0x23)		// PURE GUESS:
		no_halt = 1;
	}

/*
 *	Search list for one for a given item.
 *
 *	Output:	->Scheduled_usecode if found, else 0.
 */

Scheduled_usecode *Scheduled_usecode::find
	(
	Game_object *srch
	)
	{
	for (Scheduled_usecode *each = first; each; each = each->next)
		if (each->obj == srch)
			return each;	// Found it.
	return (0);
	}


inline void Scheduled_usecode::activate_egg(Usecode_machine *usecode,
				     Game_object *e, int type)
{
	if (e && e->is_egg() && (type == -1 || 
			((Egg_object *) e)->get_type() == type))
		((Egg_object *) e)->activate(usecode,
			usecode->gwin->get_main_actor(), 1);
}

/*
 *	Execute eggs.
 */

void Scheduled_usecode::activate_eggs
	(
	Usecode_machine *usecode
	)
	{
	int size = objval.get_array_size();
	if (!size)			// Not an array?
		{
		activate_egg(usecode, obj, -1);
		return;
		}
	int i;
	for (i = 0; i < size; i++)	// First do monsters.
		{
		activate_egg(usecode, usecode->get_item(objval.get_elem(i)),
						(int) Egg_object::monster);
		Usecode_value z((Game_object*) NULL);
		objval.put_elem(i, z);
		}
	for (i = 0; i < size; i++)	// Do the rest.
		activate_egg(usecode, usecode->get_item(objval.get_elem(i)),
									-1);
	}

/*
 *	Execute an array of usecode, generally one instruction per tick.
 */

void Scheduled_usecode::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->usecode machine.
	)
	{
	Usecode_machine *usecode = (Usecode_machine *) udata;
	Game_window *gwin = usecode->gwin;
	int delay = 200;			// Trying default delay.
	int do_another = 1;			// Flag to keep going.
	int opcode;
					// If a 1 follows, keep going.
	for (; i < cnt && ((opcode = arrval.get_elem(i).get_int_value()) 
						== 0x1 || do_another); i++)
		{
		do_another = 0;
		switch (opcode)
			{
		case 0x1:		// Means keep going without painting.
			do_another = 1;
			gwin->set_painted();	// Want to paint when done.
			break;
		case 0x0b:		// ?? 2 parms, 1st one < 0.
			{		// Loop(offset, cnt).
			Usecode_value& cntval = arrval.get_elem(i + 2);
			int cnt = cntval.get_int_value();
			if (cnt <= 0)
					// Done.
				i += 2;
			else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = arrval.get_elem(i + 1);
				i += offval.get_int_value() - 1;
				do_another = 1;
				}
			break;
			}
		case 0x0c:		// Loop with 3 parms.???
			{		// Loop(offset, cnt1, cnt2?).++++
			do_another = 1;
			Usecode_value& cntval = arrval.get_elem(i + 2);
			int cnt = cntval.get_int_value();
			if (cnt <= 0)
					// Done.
				i += 3;
			else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = arrval.get_elem(i + 1);
				i += offval.get_int_value() - 1;
				}
			break;
			}
		case 0x23:		// ?? Always appears first.
					// Maybe means "don't let
					//    intrinsic 5c stop it".
			no_halt = 1;	// PURE GUESS.
			do_another = 1;
			break;
		case 0x27:		// ?? 1 parm. Guessing:
			{		//   delay before next instruction.
			Usecode_value& delayval = arrval.get_elem(++i);
					// ?? Guessing at time.
//			delay = 200*(delayval.get_int_value()); 9/17/00:
// NOTE: Changing this can have a major impact!
//			delay = 400*(delayval.get_int_value());
// Changed Apr. 16, 2001 to:
			delay = 250*(delayval.get_int_value());
			break;		
			}
#if 0
		case 0x2c:		// Quit if there's already scheduled
					//   code for item?
					// Or supercede the existing one?
			break;
#endif
		case 0x2d:		// Remove itemref.
			usecode->remove_item(obj);
			break;
		case 0x39:		// Rise?  (For flying carpet.
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz < 10)
				t.tz++;
			obj->move(t);
			break;
			}
		case 0x38:		// Descend?
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz > 0)
				t.tz--;
			obj->move(t);
			break;
			}
		case 0x46:
			{		// Set frame.
			Usecode_value& fval = arrval.get_elem(++i);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x48:		// Guessing:  activate egg.
			activate_eggs(usecode);
			break;
		case 0x4d:		// ??Also show next frame?
		case 0x4e:		// Show next frame.
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			Usecode_value fval((1 + obj->get_framenum())%nframes);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x50:		// Guessing: Show prev. frame.
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			int pframe = obj->get_framenum() - 1;
			Usecode_value fval((pframe + nframes)%nframes);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x52:		// Say string.
			{
			Usecode_value& strval = arrval.get_elem(++i);
			usecode->item_say(objval, strval);
			break;
			}
		case 0x55:		// Call?
			{
			Usecode_value& val = arrval.get_elem(++i);
			int fun = val.get_int_value();
					// REALLY guessing (for Forge):
			Usecode_machine::Usecode_events ev = 
					Usecode_machine::internal_exec;
			if (obj && obj->is_egg() && 
				((Egg_object *)obj)->get_type() ==
			    				Egg_object::usecode)
				{
				ev = Usecode_machine::egg_proximity;
				cout << "0x55:  guessing with egg" << endl;
				}
			usecode->call_usecode(fun, obj, ev);
			break;
			}
		case 0x56:		// Play speech track.
			{
			Usecode_value& val = arrval.get_elem(++i);
			int track = val.get_int_value();
			if (track >= 0)
				Audio::get_ptr()->start_speech(track);
			}
		case 0x58:		// Play sound effect!
			{
			Usecode_value& val = arrval.get_elem(++i);
			Audio::get_ptr()->play_sound_effect(val.get_int_value());
					// Or play sound effect??
			break;
			}
		case 0x59:		// Parm. is dir. (0-7).  0=north?
			{
					// Look in that dir.
			Usecode_value& val = arrval.get_elem(++i);
					// It may be 0x3x.  Face dir?
			int dir = val.get_int_value()&7;
			obj->set_usecode_dir(dir);
			Usecode_value v(obj->get_dir_framenum(
							dir, Actor::standing));
			usecode->set_item_frame(objval, v);
			frame_index = 0;// Reset walking frame index.
			break;
			}
		default:
					// Frames with dir.  U7-verified!
			if (opcode >= 0x61 && opcode <= 0x70)
				{	// But don't show empty frames.
				Usecode_value v(obj->get_dir_framenum(
					obj->get_usecode_dir(), 
					opcode - 0x61));
				usecode->set_item_frame(objval, v, 1);
				}
					// ++++Guessing:
			else if (opcode >= 0x30 && opcode < 0x38)
					// Step in dir. opcode&7.
				step(usecode, opcode&7);
			else
				{
			        cout << "Und sched. opcode " << hex << 
			"0x" << setfill((char)0x30) << setw(2) << opcode << endl;
				do_another = 1; // Don't let it delay us.
				}
			break;
			}
		}
	if (i < cnt)			// More to do?
		{
		gwin->get_tqueue()->add(curtime + delay, this, udata);
		return;
		}
					// Don't get stuck in conv. mode.
	if (gwin->get_mode() == Game_window::conversation)
		{
//		gwin->set_mode(Game_window::normal);
		gwin->end_gump_mode();	// This also sets mode=normal.
		gwin->paint();
		}
	if (count == 1 &&		// Last one?  GUESSING:
	    objpos.tx != -1)		// And valid pos.
		usecode->activate_cached(objpos);
	delete this;			// Hope this is safe.
	}

/*
 *	Step in given direction.
 */

void Scheduled_usecode::step
	(
	Usecode_machine *usecode,
	int dir				// 0-7.
	)
	{
	int frame = obj->get_framenum();
	Barge_object *barge;
	Actor *act = usecode->as_actor(obj);
	if (act)
		{
		Frames_sequence *frames = act->get_frames(dir);
					// Get frame (updates frame_index).
		frame = frames->get_next(frame_index);
		Tile_coord tile = obj->get_abs_tile_coord().get_neighbor(dir);
		obj->step(tile, frame);
		}
	else if ((barge = dynamic_cast<Barge_object *> (obj)) != 0)
		{
		barge->face_direction(dir);
		for (int i = 0; i < 4; i++)
			{
			Tile_coord t = obj->get_abs_tile_coord().get_neighbor(dir);
			obj->step(t, 0);
#if 0	/* ++++Doesn't help SI.  Definitely not for BG. */
			if (!obj->step(t, 0))
				{	// Blocked, so try to turn.
				barge->face_direction((dir - 2 + 8)%8);
				obj->step(t, 0);
				}
#endif
			}
		}
	}
