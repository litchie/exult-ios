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

using std::map;
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
	Object_sfx(int snum, Game_object *o) : sfxnum(snum), distance(0)
		{ set_obj(o); }
	bool is_active()		// Is sound still active?
		{ return sfx.is_active(); }
	int get_sfxnum()
		{ return sfxnum; }
	int get_distance()
		{ return distance; }
	void set_obj(Game_object *o);	// Set to new object.
	void stop(Game_object *o)	// Stop if from given object.
		{
		if (o == obj)
			sfx.set_repeat(false);
		}
					// Get sfx to play for given shape.
	static int get_shape_sfx(int shapenum);
	static void play(Game_object *o, int snum, bool stop = false);
	};

/*
 *	Set to new object if it's closer than the old object, or we're
 *	inactive.
 */
void Object_sfx::set_obj
	(
	Game_object *o
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord apos = gwin->get_main_actor()->get_abs_tile_coord();
	Tile_coord opos = o->get_abs_tile_coord();
	bool active = sfx.is_active();
	int new_distance = apos.distance(opos);
	if (active && new_distance >= distance && o != obj)
		return;			// Farther than current source.
	obj = o;
	dir = 0;
	bool repeat = true;
	distance = new_distance;
	int volume = SDL_MIX_MAXVOLUME;	// Set volume based on distance.
	if (distance)
		{			// 160/8 = 20 tiles. 20*20=400.
		volume = (SDL_MIX_MAXVOLUME*64)/(distance*distance);
		if (!volume)		// Dead?
			repeat = false;	// Time to kill it.
		if (volume < 8)
			volume = 8;
		else if (volume > SDL_MIX_MAXVOLUME)
			volume = SDL_MIX_MAXVOLUME;
		dir = Get_direction16(apos.ty - opos.ty, opos.tx - apos.tx);
		}
	if (!sfx.is_active())		// First time?
					// Start playing, and repeat.
		sfx = Audio::get_ptr()->play_wave_sfx(sfxnum, volume,
							dir, repeat);
	else				// Set new volume, position.
		{
		sfx.set_volume(volume);
		sfx.set_dir(dir);
		sfx.set_repeat(repeat);
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
	static int first = 1;

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
					// Moongates:
		table[776] = 77;
		table[777] = 77;
		}
	map<int, int>::iterator it = table.find(shapenum);
	if (it == table.end())
		return -1;
	int sfx = (*it).second;
	return Audio::game_sfx(sfx);
	return sfx;
	}

/*
 *	Play a sound, or modify its volume/position.
 */

void Object_sfx::play
	(
	Game_object *o,			// Object.
	int snum,			// Object's sound-effect #.
	bool stop			// Time to stop.
	)
	{
					// Play a given sfx only once.
	static map<int, Object_sfx*> play_table;
					// Already playing?
	map<int, Object_sfx*>::iterator it = play_table.find(snum);
	if (it == play_table.end())	// No.
		{			// Start new SFX for it.
		if (!stop)
			play_table[snum] = new Object_sfx(snum, o);
		return;
		}
	Object_sfx *sfx = (*it).second;
	if (stop)
		sfx->stop(o);
	else
		sfx->set_obj(o);	// Modify/restart.
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
	int shnum = ob->get_shapenum();
	int frames = gwin->get_shape_num_frames(shnum);
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
	bool ir
	) : Animator(o, Object_sfx::get_shape_sfx(o->get_shapenum())), ireg(ir)
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
					// Stop playing sound.
		Object_sfx::play(obj, sfxnum, true);
		return;
		}
	int framenum = obj->get_framenum();
	if (ireg)			// For IREG, increment frames.
		switch (obj->get_shapenum())
			{
		case 284:		// Sundial is a special case.
			framenum = gwin->get_hour(); 
			break;
		case 768:		// Energy field.  Stop at top.
			if (framenum < frames - 1)
				framenum++;
			break;
		case 222:		// Pennant (SI).
		case 289:		// Fire (SI).
		case 326:		// Fountain (SI).
		case 614:		// Magic music box (SI).
		case 655:		// Planets (SI). (Groups of 6).
		case 695:		// Grandfather clock (SI).
		case 794:		// Severed limb (SI).
		case 992:		// Burning urn (SI).
					// 0-5 face one way, 6-11 the other.
			framenum = (framenum + 1)%6 + 6*(framenum/6);
			break;
		default:
			framenum++;
			break;
			}
	else				// Want fixed shapes synchronized.
					// Testing -WJP
		framenum = (curtime / 100);
	framenum %= frames;
	obj->set_frame(framenum);
	gwin->add_dirty(obj);
	if (!framenum && sfxnum >= 0)	// Sound effect?
		Object_sfx::play(obj, sfxnum);
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
