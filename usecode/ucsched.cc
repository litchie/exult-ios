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

#include "ucinternal.h"
#include "useval.h"
#include "ucsched.h"
#include "Audio.h"
#include "barge.h"
#include "game.h"
#include "frameseq.h"
#include "gamewin.h"
#include "egg.h"
#include "actors.h"
#include "ucscriptop.h"

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::hex;
using std::setfill;
using std::setw;

using namespace Ucscript;

int Usecode_script::count = 0;
Usecode_script *Usecode_script::first = 0;

/*
 *	Create for a 'restore'.
 */

Usecode_script::Usecode_script
	(
	Game_object *item, 
	Usecode_value *cd, 
	int findex,
	int nhalt, 
	int del
	) : obj(item), code(cd), i(0), frame_index(findex), 
	    no_halt(nhalt), delay(del)
	{
	cnt = code->get_array_size();
	count++;			// Keep track of total.
	next = first;			// Put in chain.
	prev = 0;
	if (first)
		first->prev = this;
	first = this;
	}

/*
 *	Create.
 */

Usecode_script::Usecode_script
	(
	Game_object *o,
	Usecode_value *cd		// May be NULL for empty script.
	) : obj(o), code(cd), cnt(0), i(0), frame_index(0), no_halt(0),
	    delay(0)
	{
	if (!code)			// Empty?
		code = new Usecode_value(0, 0);
	else
		{
		cnt = code->get_array_size();
		if (!cnt)		// Not an array??  (This happens.)
			{		// Create with single element.
			code = new Usecode_value(1, code);
			cnt = 1;
			}
					//++++This should be done in start():
		int opval0 = code->get_elem(0).get_int_value();
		if (opval0 == 0x23)		// PURE GUESS:
			no_halt = 1;
		}
	count++;			// Keep track of total.
	next = first;			// Put in chain.
	prev = 0;
	if (first)
		first->prev = this;
	first = this;
	}

/*
 *	Delete.
 */

Usecode_script::~Usecode_script()
	{
	delete code;
	count--;
	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;
	else
		first = next;
	}

/*
 *	Enter into the time-queue.
 */

void Usecode_script::start
	(
	long delay			// Start after this many msecs.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->get_tqueue()->add(delay + SDL_GetTicks(), this,
					(long) gwin->get_usecode());
	}

/*
 *	Append instructions.
 */
void Usecode_script::add(int v1)
	{
	code->append(&v1, 1);
	cnt++;
	}
void Usecode_script::add(int v1, int v2)
	{
	int vals[2];
	vals[0] = v1;
	vals[1] = v2;
	code->append(vals, 2);
	cnt += 2;
	}
void Usecode_script::add(int v1, char *str)
	{
	int sz = code->get_array_size();
	code->resize(sz + 2);
	code[sz] = v1;
	code[sz + 1] = str;
	cnt += 2;
	}
void Usecode_script::add(int *vals, int c)
	{
	code->append(vals, cnt);
	cnt += c;
	}

/*
 *	Search list for one for a given item.
 *
 *	Output:	->Usecode_script if found, else 0.
 */

Usecode_script *Usecode_script::find
	(
	Game_object *srch
	)
	{
	for (Usecode_script *each = first; each; each = each->next)
		if (each->obj == srch)
			return each;	// Found it.
	return (0);
	}

/*
 *	Remove all from global list (assuming they've already been cleared
 *	from the time queue).
 */

void Usecode_script::clear
	(
	)
	{
	while (first)
		delete first;
	}

inline void Usecode_script::activate_egg(Usecode_internal *usecode,
				     Game_object *e, int type)
{
	if (e && e->is_egg() && (type == -1 || 
			((Egg_object *) e)->get_type() == type))
		((Egg_object *) e)->activate(usecode,
			usecode->gwin->get_main_actor(), 1);
}

#if 0
/*
 *	Execute eggs.
 */

void Usecode_script::activate_eggs
	(
	Usecode_internal *usecode
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
#endif

/*
 *	Execute an array of usecode, generally one instruction per tick.
 */

void Usecode_script::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->usecode machine.
	)
	{
	Usecode_internal *usecode = (Usecode_internal *) udata;
	Game_window *gwin = usecode->gwin;
	int delay = c_std_delay;		// Trying default delay.
	int do_another = 1;			// Flag to keep going.
	int opcode;
					// If a 1 follows, keep going.
	for (; i < cnt && ((opcode = code->get_elem(i).get_int_value()) 
						== 0x1 || do_another); i++)
		{
		do_another = 0;
		switch (opcode)
			{
		case cont:		// Means keep going without painting.
			do_another = 1;
			gwin->set_painted();	// Want to paint when done.
			break;
		case repeat:		// ?? 2 parms, 1st one < 0.
			{		// Loop(offset, cnt).
			Usecode_value& cntval = code->get_elem(i + 2);
			int cnt = cntval.get_int_value();
			if (cnt <= 0)
					// Done.
				i += 2;
			else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = code->get_elem(i + 1);
				i += offval.get_int_value() - 1;
				do_another = 1;
				}
			break;
			}
		case repeat2:		// Loop with 3 parms.???
			{		// Loop(offset, cnt1, cnt2?).++++
				//+++ guessing: loop cnt1 each round. use cnt2 as loop var.
				//This is necessary for loop nesting.
				//(used in mining machine, orb of the moons)

				// maybe cnt1 and cnt2 should be swapped... not sure

			do_another = 1;
			Usecode_value& cntval = code->get_elem(i + 3);
			Usecode_value& origval = code->get_elem(i + 2);
			int cnt = cntval.get_int_value();
			int orig = origval.get_int_value();
			if (cnt > orig) { // ++++ First pass? Set to orig or not?
				cntval = origval;
				cnt = orig;
			}
			if (cnt <= 0) {
					// Done.
				i += 3;
				cntval = origval; // restore counter
			} else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = code->get_elem(i + 1);
				i += offval.get_int_value() - 1;
				}
			break;
			}
		case nop:		// Just a nop.
			break;
		case dont_halt:		// ?? Always appears first.
					// Maybe means "don't let
					//    intrinsic 5c stop it".
			no_halt = 1;	// PURE GUESS.
			do_another = 1;
			break;
		case delay_ticks:	// 1 parm.
			{		//   delay before next instruction.
			Usecode_value& delayval = code->get_elem(++i);
					// ?? Guessing at time.
// NOTE: Changing this can have a major impact!
//			delay = 250*(delayval.get_int_value());
// Changed Jul. 30, 2001 to:
			delay = c_std_delay*(delayval.get_int_value());
			break;		
			}
#if 0
		case finish:		// Quit if there's already scheduled
					//   code for item?
					// Or supercede the existing one?
			break;
#endif
		case Ucscript::remove:	// Remove obj.
			usecode->remove_item(obj);
			break;
		case rise:		// (For flying carpet.
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz < 10)
				t.tz++;
			obj->move(t);
			break;
			}
		case descend:
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz > 0)
				t.tz--;
			obj->move(t);
			break;
			}
		case frame:		// Set frame.
			usecode->set_item_frame(obj, 
					code->get_elem(++i).get_int_value());
			break;
		case egg:		// Guessing:  activate egg.
			activate_egg(usecode, obj, -1);
//			activate_eggs(usecode);
			break;
		case next_frame_max:	// Stop at last frame.
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			if (obj->get_framenum() < nframes)
				usecode->set_item_frame(obj,
							1+obj->get_framenum());
			break;
			}
		case next_frame:
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			usecode->set_item_frame(obj, 
					(1 + obj->get_framenum())%nframes);
			break;
			}
		case prev_frame:
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			int pframe = obj->get_framenum() - 1;
			usecode->set_item_frame(obj, 
						(pframe + nframes)%nframes);
			break;
			}
		case say:		// Say string.
			{
			Usecode_value& strval = code->get_elem(++i);
			Usecode_value objval(obj);
			usecode->item_say(objval, strval);
			break;
			}
		case music:		// Unknown.
		        cout << "Und sched. opcode " << hex << 
				"0x" << setfill((char)0x30) << setw(2) 
						<< opcode << std::dec << endl;
			++i;		// Takes 1 parameter!
			break;
		case Ucscript::usecode:	// Call?
			{
			Usecode_value& val = code->get_elem(++i);
			int fun = val.get_int_value();
					// Watch for eggs:
			Usecode_internal::Usecode_events ev = 
					Usecode_internal::internal_exec;
			if (obj && obj->is_egg() && 
				((Egg_object *)obj)->get_type() ==
			    				Egg_object::usecode)
				ev = Usecode_internal::egg_proximity;
			usecode->call_usecode(fun, obj, ev);
			break;
			}
		case Ucscript::usecode2:// Call(fun, eventid).
			{
			Usecode_value& val = code->get_elem(++i);
			int evid = code->get_elem(++i).get_int_value();
			usecode->call_usecode(val.get_int_value(), obj, 
				(Usecode_internal::Usecode_events) evid);
			break;
			}
		case speech:		// Play speech track.
			{
			Usecode_value& val = code->get_elem(++i);
			int track = val.get_int_value();
			if (track >= 0)
				Audio::get_ptr()->start_speech(track);
			}
		case sfx:		// Play sound effect!
			{
			Usecode_value& val = code->get_elem(++i);
			Audio::get_ptr()->play_sound_effect(val.get_int_value());
					// Or play sound effect??
			break;
			}
		case face_dir:		// Parm. is dir. (0-7).  0=north.
			{
					// Look in that dir.
			Usecode_value& val = code->get_elem(++i);
					// It may be 0x3x.  Face dir?
			int dir = val.get_int_value()&7;
			obj->set_usecode_dir(dir);
			usecode->set_item_frame(obj, obj->get_dir_framenum(
							dir, Actor::standing));
			frame_index = 0;// Reset walking frame index.
			break;
			}
		case hit:		// Hit(hps, ??).
			{
			Usecode_value hps = code->get_elem(++i);
			Usecode_value unk = code->get_elem(++i);
			Actor *act = usecode->as_actor(obj);
			if (act)	// ++++Should apply to any object.
				act->reduce_health(hps.get_int_value());
			break;
			}
		default:
					// Frames with dir.  U7-verified!
			if (opcode >= 0x61 && opcode <= 0x70)
				{	// But don't show empty frames.
				int v = obj->get_dir_framenum(
					obj->get_usecode_dir(), 
					opcode - 0x61);
				usecode->set_item_frame(obj, v, 1);
				}
					// ++++Guessing:
			else if (opcode >= 0x30 && opcode < 0x38)
					// Step in dir. opcode&7.
				step(usecode, opcode&7);
			else
				{
			        cout << "Und sched. opcode " << hex << 
					"0x" << setfill((char)0x30) << setw(2) << opcode << std::dec << endl;
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
#if 0	/* ++++Might need this for Test of Love!! */
	if (count == 1 &&		// Last one?  GUESSING:
	    objpos.tx != -1)		// And valid pos.
	{
		usecode->activate_cached(objpos);
	}
#endif
	delete this;			// Hope this is safe.
	}

/*
 *	Step in given direction.
 */

void Usecode_script::step
	(
	Usecode_internal *usecode,
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
			Tile_coord t = obj->get_abs_tile_coord().get_neighbor(
									dir);
			obj->step(t, 0);
			}
		}
	}

/*
 *	Save (serialize).
 *
 *	Output:	Length written, or -1 if error.
 */

int Usecode_script::save
	(
	unsigned char *buf,
	int buflen
	)
	{
	unsigned char *ptr = buf;
	int remaining = cnt - i;
	Write2(ptr, remaining);		// # of values we'll store.
	for (int j = i; j < cnt; j++)
		{
		Usecode_value& val = code->get_elem(j);
		int len = val.save(ptr, buflen - (ptr - buf));
		if (len < 0)
			return -1;
		ptr += len;
		}
	if (ptr - buf < 8)		// Enough room left?
		return -1;
	Write2(ptr, frame_index);
	Write2(ptr, no_halt);
	Write4(ptr, delay);
	return (ptr - buf);
	}

/*
 *	Restore (serialize).
 *
 *	Output:	->entry, which is also stored in our global chain, but is NOT
 *		added to the time queue yet.
 */

Usecode_script *Usecode_script::restore
	(
	Game_object *item,		// Object this is executed for.
	unsigned char *buf,
	int buflen
	)
	{
	unsigned char *ptr = buf;
	int cnt = Read2(ptr);		// Get # instructions.
					// Create empty array.
	Usecode_value *code = new Usecode_value(cnt, 0);
	for (int i = 0; i < cnt; i++)
		{
		Usecode_value& val = code->get_elem(i);
		if (!val.restore(ptr, buflen - (ptr - buf)))
			{
			delete code;
			return 0;
			}
		}
	if (ptr - buf < 8)		// Enough room left?
		{
		delete code;
		return 0;
		}
	int frame_index = Read2(ptr);
	int no_halt = Read2(ptr);
	int delay = Read4(ptr);
	return new Usecode_script(item, code, frame_index, no_halt, delay);
	}

