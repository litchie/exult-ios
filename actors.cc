/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actors.cc - Game actors.
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

#include <iostream.h>			/* Debugging. */
#include <stdlib.h>
#include <string.h>
#include "gamewin.h"
#include "imagewin.h"
#include "usecode.h"

/*
 *	Create character.
 */

Actor::Actor
	(
	char *nm, 
	int shapenum, 
	int num,			// NPC # from npc.dat.
	int uc				// Usecode #.
	) : Sprite(shapenum), npc_num(num), usecode(uc), flags(0)
	{
	set_default_frames();
	name = nm == 0 ? 0 : strdup(nm);
	for (int i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		properties[i] = 0;
	}

/*
 *	Set default set of frames.
 */

void Actor::set_default_frames
	(
	)
	{
					// Set up actor's frame lists.
					// These are rough guesses.
	static unsigned char 	north_frames[3] = {0, 1, 2},
				south_frames[3] = {16, 17, 18},
//				east_frames[3] = {3, 4, 5},
				east_frames[3] = {7, 8, 9},
//				west_frames[3] = {19, 20, 21};
				west_frames[3] = {23, 24, 25};
	set_frame_sequence(north, 3, north_frames);
	set_frame_sequence(south, 3, south_frames);
	set_frame_sequence(east, 3, east_frames);
	set_frame_sequence(west, 3, west_frames);
	}

/*
 *	Run usecode when double-clicked.
 */

void Actor::activate
	(
	Usecode_machine *umachine
	)
	{
	if (usecode == -1)
		umachine->call_usecode(get_shapenum(), this,
				Usecode_machine::double_click);
	else
		umachine->call_usecode(usecode, this, 
					Usecode_machine::double_click);
	}

/*
 *	Get name.
 */

char *Actor::get_name
	(
	)
	{
	return name ? name : Game_object::get_name();
	}

/*
 *	Set flag.
 */

void Actor::set_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags |= ((unsigned long) 1 << flag);
	}

/*
 *	Clear flag.
 */

void Actor::clear_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags &= ~((unsigned long) 1 << flag);
	}

/*
 *	Get flag.
 */

int Actor::get_flag
	(
	int flag
	)
	{
	return (flag >= 0 && flag < 32) ? (flags & ((unsigned long) 1 << flag))
			!= 0 : 0;
	}

/*
 *	Animation.
 */

void Main_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = (Game_window *) udata;
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	if (next_frame(curtime, cx, cy, sx, sy, frame))
		{
		Chunk_object_list *olist = gwin->get_objects(cx, cy);
#if 1 /* Need to fix renderer first. */
		olist->setup_cache();	//++++++++++++++++
//++++++++TESTING Remove the above!!!!!!!+++++++++++++++++++++++++++++
		int new_lift;		// Might climb/descend.
		if (olist->is_blocked(get_lift(), sx, sy, new_lift))
			{
			stop();
			return;
			}
		if (new_lift != get_lift())
			cout << "New lift is " << new_lift << '\n';
#else
		int new_lift = get_lift();
#endif
					// Add back to queue for next time.
		gwin->get_tqueue()->add(curtime + frame_time,
							this, udata);
					// Check for scrolling.
		int chunkx = gwin->get_chunkx(), chunky = gwin->get_chunky();
					// At left?
		if (cx - chunkx <= 0 && sx < 6)
			gwin->view_left();
					// At right?
		else if ((cx - chunkx)*16 + sx >= gwin->get_width()/8 - 4)
			gwin->view_right();
					// At top?
		if (cy - chunky <= 0 && sy < 6)
			gwin->view_up();
					// At bottom?
		else if ((cy - chunky)*16 + sy >= gwin->get_height()/8 - 4)
			gwin->view_down();
					// Get old chunk it's in.
		int old_cx = get_cx(), old_cy = get_cy();
					// Get old rectangle.
		Rectangle oldrect = gwin->get_shape_rect(this);
					// Move it.
		move(cx, cy, olist, sx, sy, frame, new_lift);
				// Near an egg?
		Egg_object *egg = olist->find_egg(sx, sy);
		if (egg)
			egg->activate(gwin->get_usecode());
		int inside;		// See if moved inside/outside.
					// In a new chunk?
		if ((get_cx() != old_cx || get_cy() != old_cy) &&
		    gwin->check_main_actor_inside())
			gwin->paint();
		else
			gwin->repaint_sprite(this, oldrect);
		}
	}

/*
 *	Set a schedule.
 */

void Schedule::set
	(
	unsigned char *entry		// 4 bytes read from schedule.dat.
	)
	{
	time = entry[0]&7;
	type = entry[0]>>3;
	x = entry[1];
	y = entry[2];
	superchunk = entry[3];
	}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	char *nm, 			// Name.  A copy is made.
	int shapenum, 
	int fshape, 
	int uc
	) : Actor(nm, shapenum, fshape, uc), next(0), nearby(0),
		schedule((int) Schedule::loiter), num_schedules(0), 
		schedules(0)
	{
	}

/*
 *	Kill an actor.
 */

Npc_actor::~Npc_actor
	(
	)
	{
	}

/*
 *	Update schedule at a 3-hour time change.
 */

void Npc_actor::update_schedule
	(
	Game_window *gwin,
	int hour3			// 0=midnight, 1=3am, etc.
	)
	{
	for (int i = 0; i < num_schedules; i++)
		if (schedules[i].get_time() == hour3)
			{		// Found entry.
			schedule = schedules[i].get_type();
					// Store old chunk list.
			Chunk_object_list *olist = gwin->get_objects(
							get_cx(), get_cy());
			int new_cx, new_cy, tx, ty;
			schedules[i].get_pos(new_cx, new_cy, tx, ty);
cout << "Npc " << get_name() << " has new schedule " << schedule << '\n';
			Chunk_object_list *nlist = 
					gwin->get_objects(new_cx, new_cy);
					// Move it.
			move(new_cx, new_cy, nlist, tx, ty, -1);
			if (nlist != olist)
				switched_chunks(olist, nlist);
			return;
			}
	}

/*
 *	Animation.
 */

void Npc_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
	Sprite::handle_event(curtime, udata);
					// In new chunk?
	if (get_cx() != old_cx || get_cy() != old_cy)
		{
		Game_window *gwin = (Game_window *) udata;
		Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
		Chunk_object_list *nlist = gwin->get_objects(get_cx(),
								get_cy());
		switched_chunks(olist, nlist);
		}
	}

/*
 *	Update chunks' npc lists after this has moved.
 */

void Npc_actor::switched_chunks
	(
	Chunk_object_list *olist,	// Old chunk, or null.
	Chunk_object_list *nlist	// New chunk, or null.
	)
	{
	Npc_actor *each;
	if (olist)			// Remove from old list.
		{
		if (this == olist->npcs)
			olist->npcs = next;
		else
			{
			Npc_actor *each, *prev = olist->npcs;
			while ((each = prev->next) != 0)
				if (each == this)
					{
					prev->next = next;
					break;
					}
				else
					prev = each;
			}
		}
	if (nlist)			// Add to new list.
		{
		next = nlist->npcs;
		nlist->npcs = this;
		}
	}
#if 0
/*
 *	Figure where the sprite will be in the next frame.
 *
 *	Output:	0 if don't need to move.
 */

int Area_actor::next_frame
	(
	unsigned long time,		// Current time.
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_sx, int& new_sy,	// New shape coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
					// See if we should change motion.
	if (time < next_change)
		return (Actor::next_frame(time, new_cx, new_cy,
						new_sx, new_sy, next_frame));
	if (is_moving())
		{
		stop();
		new_cx = get_cx();
		new_cy = get_cy();
		new_sx = get_shape_pos_x();
		new_sy = get_shape_pos_y();
		next_frame = get_framenum();
					// Wait 0 to 5 seconds.
		next_change.tv_sec = time.tv_sec +
			(long) (5.0*rand()/(RAND_MAX + 1.0));
		return (1);
		}
	else
		{			// Where should we aim for?
		long deltax = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long deltay = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long newx = get_worldx() + deltax;
		long newy = get_worldy() + deltay;
					// Move every 1/4 second.
		start(newx, newy, 250);
					// Go 1 to 4 seconds.
		next_change.tv_sec = time.tv_sec + 1 +
			(long) (4.0*rand()/(RAND_MAX + 1.0));
		return (0);
		}
	}
#endif
