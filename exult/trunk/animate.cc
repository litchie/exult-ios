/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Animate.cc - Animated game objects.
 **
 **	Written: 7/27/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "animate.h"
#include "gamewin.h"

/*
 *	Create appropriate animator.
 */

Animator *Animator::create
	(
	Game_object *ob,		// Animated object.
	int ireg			// 1 if an IREG object.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int frames = gwin->get_shape_num_frames(ob->get_shapenum());
	if (frames > 1)
		return new Frame_animator(ob, ireg);
	else
		return new Wiggle_animator(ob);
	}


/*
 *	When we delete, better remove from queue.
 */

Animator::~Animator
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	while (gwin->get_tqueue()->remove(this))
		;
	}

/*
 *	Start animation.
 */

void Animator::start_animation
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Clean out old entry if there.
	gwin->get_tqueue()->remove(this);
	gwin->get_tqueue()->add(SDL_GetTicks() + 100, this, (long) gwin);
	animating = 1;
	}

/*
 *	Create a frame animator.
 */

Frame_animator::Frame_animator
	(
	Game_object *o,
	int ir
	) : Animator(o), ireg(ir)
	{
	Game_window *gwin = Game_window::get_game_window();
	int shapenum = obj->get_shapenum();
	frames = gwin->get_shape_num_frames(shapenum);
	}

/*
 *	Animation.
 */

void Frame_animator::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
	{
	int delay = 100;		// Delay between frames.
	Game_window *gwin = (Game_window *) udata;
	if (!gwin->add_dirty(obj))
		{			// No longer on screen.
		animating = 0;
		return;
		}
	int framenum;
	if (ireg)			// +++Another experiment -JSF
		framenum = obj->get_framenum() + 1;
	else				// Want fixed shapes synchronized.
					// Testing -WJP
		framenum = (curtime / 100);
	obj->set_frame(framenum % frames);
	gwin->add_dirty(obj);
					// Add back to queue for next time.
	if (animating)
		gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Animation.
 */

void Wiggle_animator::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
	{
	int delay = 100;		// Delay between frames.
	Game_window *gwin = (Game_window *) udata;
	if (!gwin->add_dirty(obj))
		{			// No longer on screen.
		animating = 0;
		return;
		}
	int tx, ty, tz;			// Get current position.
	obj->get_abs_tile(tx, ty, tz);
	int newdx = rand()%3;
	int newdy = rand()%3;
	tx += -deltax + newdx;
	ty += -deltay + newdy;
	deltax = newdx;
	deltay = newdy;
	obj->Game_object::move(tx, ty, tz);
					// Add back to queue for next time.
	if (animating)
		gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Create an animated object.
 */

Animated_object::Animated_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex,
	unsigned int shapey,
	unsigned int lft
	) : Game_object(l, h, shapex, shapey, lft)
	{
	animator = Animator::create(this, 0);
	}

/*
 *	Create at given position.
 */

Animated_object::Animated_object
	(
	int shapenum, 
	int framenum, 
	unsigned int tilex, unsigned int tiley, 
	unsigned int lft
	) : Game_object(shapenum, framenum, tilex, tiley, lft)
	{
	animator = Animator::create(this, 0);
	}

/*
 *	When we delete, better remove from queue.
 */

Animated_object::~Animated_object
	(
	)
	{
	delete animator;
	}

/*
 *	Render.
 */

void Animated_object::paint
	(
	Game_window *gwin
	)
	{
	Game_object::paint(gwin);
	animator->want_animation();	// Be sure animation is on.
	}

/*
 *	Create an animated object.
 */

Animated_ireg_object::Animated_ireg_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex,
	unsigned int shapey,
	unsigned int lft
	) : Ireg_game_object(l, h, shapex, shapey, lft)
	{
	animator = Animator::create(this, 1);
	}

/*
 *	Create at given position.
 */

Animated_ireg_object::Animated_ireg_object
	(
	int shapenum, 
	int framenum, 
	unsigned int tilex, unsigned int tiley, 
	unsigned int lft
	) : Ireg_game_object(shapenum, framenum, tilex, tiley, lft)
	{
	animator = Animator::create(this, 1);
	}

/*
 *	When we delete, better remove from queue.
 */

Animated_ireg_object::~Animated_ireg_object
	(
	)
	{
	delete animator;
	}

/*
 *	Render.
 */

void Animated_ireg_object::paint
	(
	Game_window *gwin
	)
	{
	Game_object::paint(gwin);
	animator->want_animation();	// Be sure animation is on.
	}

/*
 *	Write out.  (Same as Ireg_game_object::write_ireg().)
 */

void Animated_ireg_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[7];		// 6-byte entry + length-byte.
	buf[0] = 6;
	write_common_ireg(&buf[1]);
	buf[5] = (get_lift()&15)<<4;
	buf[6] = get_quality();
	out.write((char*)buf, sizeof(buf));
	}

