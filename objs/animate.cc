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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "animate.h"
#include "gamewin.h"
#include "SDL_timer.h"
#include "Audio.h"
#include "actors.h"			/* Only need this for Object_sfx. */
#include "dir.h"
#include <map>

using std::ostream;
using std::rand;

/*
 *	A class for playing sound effects when certain objects are nearby.
 */
class Object_sfx
	{
	int sfxnum;			// Sound effect #.
	AudioID sfx;			// ID of sound effect being played.
	Game_object *obj;		// Object that caused the sound.
	int distance;			// Distance in tiles from Avatar.
	int dir;			// Direction (0-15) from Avatar.
public:
					// Create & start playing sound.
	Object_sfx(int snum, Game_object *o) : sfxnum(snum)
		{ set_obj(o); }
	bool is_active()		// Is sound still active?
		{ return sfx.is_active(); }
	int get_sfxnum()
		{ return sfxnum; }
	int get_distance()
		{ return distance; }
	void set_obj(Game_object *o);	// Set to new object.
					// Get sfx to play for given shape.
	static int get_shape_sfx(int shapenum);
	};

/*
 *	Set to new object.
 */
void Object_sfx::set_obj
	(
	Game_object *o
	)
	{
	obj = o;
	dir = 0;
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord apos = gwin->get_main_actor()->get_abs_tile_coord();
	Tile_coord opos = obj->get_abs_tile_coord();
	distance = apos.distance(opos);
	int volume = SDL_MIX_MAXVOLUME;	// Set volume based on distance.
	if (distance)
		{			// 160/8 = 20 tiles. 20*20=400.
		volume = (SDL_MIX_MAXVOLUME*64)/(distance*distance);
		if (volume < 8)
			volume = 8;
		else if (volume > SDL_MIX_MAXVOLUME)
			volume = SDL_MIX_MAXVOLUME;
		dir = Get_direction16(apos.ty - opos.ty, opos.tx - apos.tx);
		}
	if (!sfx.is_active())		// First time?
					// Start playing, and repeat.
		sfx = Audio::get_ptr()->play_wave_sfx(sfxnum, volume,
								dir, true);
	else				// Set new volume, position.
		{
		sfx.set_volume(volume);
		sfx.set_dir(dir);
		sfx.set_repeat(true);
		}
	}

/*
 *	Get the sound-effect # for a given shape, or -1 if not found.
 */

int Object_sfx::get_shape_sfx
	(
	int shapenum
	)
	{
	static map<int, int> table;
	static int first = 0;

	if (first)			// First time?
		{
		first = 0;
					// Surf: (47 or 49)
		table[612] = 49;
		table[613] = 49;
		table[632] = 49;
		table[736] = 49;
		table[737] = 49;
		table[751] = 49;
		table[808] = 49;
		table[834] = 49;
		table[875] = 49;
		table[907] = 49;
		table[911] = 49;
		table[918] = 49;
		table[1012] = 49;
		table[1020] = 49;
		table[1022] = 49;
					// Bubbles: (54, 56).
		table[334] = 56;
		table[335] = 56;
		table[780] = 56;
					// Fountains:
		table[893] = 36;
		}
	map<int, int>::iterator it = table.find(shapenum);
	return it == table.end() ? -1 : (*it).second;
	}

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
	if (ireg)			// For IREG, increment frames.
					// Sundial is a special case.
		framenum = obj->get_shapenum() == 284 ? gwin->get_hour()
				: obj->get_framenum() + 1;
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
 *	Create a field frame animator.
 */

Field_frame_animator::Field_frame_animator
	(
	Game_object *o,
	int rcy				// Frame to start recycling at.
	) : Animator(o), recycle(rcy)
	{
	Game_window *gwin = Game_window::get_game_window();
	int shapenum = obj->get_shapenum();
	frames = gwin->get_shape_num_frames(shapenum);
	}

/*
 *	Animation.
 */

void Field_frame_animator::handle_event
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
	int framenum = obj->get_framenum() + 1;
	if (framenum == frames)
		framenum = recycle;	// Restart cycle here.
	obj->set_frame(framenum);
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

