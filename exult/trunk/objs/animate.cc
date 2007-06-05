/**
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
#include "game.h"
#include "gameclk.h"
#include "Audio.h"
#include "actors.h"			/* Only need this for Object_sfx. */
#include "dir.h"
#include "Flex.h"
#include <map>
#include <string>
#include "miscinf.h"

#ifndef UNDER_CE
using std::map;
using std::ostream;
using std::rand;
using std::endl;
using std::cout;
#endif



/*
 *	Stop playing the sound effect if needed.
 */
void Object_sfx::stop()
	{
	if(channel >= 0)
		{
		Mix_HaltChannel(channel);
		channel = -1;
		}
	}

/*
 *	Update distance/direction information. Also starts playing
 *	the sound effect if needed.
 */
void Object_sfx::update
	(
	)
	{
	if (obj->is_pos_invalid() || !sfx)
		return;			// Not on map.

	int active = 0;
	if (channel != -1)
	 	active = Mix_Playing(channel);

	int sfxnum = sfx->num;
	if (!active && !repeat && channel != -1)
		{
		Mix_HaltChannel(channel);
		channel = -1;
		}
	if (channel == -1 && sfx->range > 1)
		{
		if (sfx->rand)
			sfxnum += (rand() % sfx->range);
		else
			{
			last_sfx = ((last_sfx + 1) % sfx->range);
			sfxnum += last_sfx;
			}
		}

	dir = 0;
	bool halt = false;

	Tile_coord apos = gwin->get_main_actor()->get_tile();
	Tile_coord opos = obj->get_tile();
	distance = apos.distance(opos);
	int volume = MIX_MAX_VOLUME;	// Set volume based on distance.

	if (distance)
		{			// 160/8 = 20 tiles. 20*20=400.
		volume = (MIX_MAX_VOLUME*64)/(distance*distance);
		if (!volume)		// Dead?
			halt = true;	// Time to kill it.
		if (volume < 8)
			volume = 8;
		else if (volume > MIX_MAX_VOLUME)
			volume = MIX_MAX_VOLUME;
		dir = Get_direction16(apos.ty - opos.ty, opos.tx - apos.tx);
		}

	if (channel == -1)		// First time?
					// Start playing, and repeat.
		channel = Audio::get_ptr()->play_sound_effect(sfxnum, volume, dir, repeat);
	else
		{
		if(halt)
			{
			Mix_HaltChannel(channel);
			channel = -1;
			}
		else
			{
			//Just change the "location" of the sound
			Mix_Volume(channel, volume);
			Mix_SetPosition(channel, (dir * 22), 0);
			}
		}
	}

/*
 *	Create appropriate animator.
 */

Animator *Animator::create
	(
	Game_object *ob			// Animated object.
	)
	{
	int shnum = ob->get_shapenum();
	int frames = ob->get_num_frames();
	Shape_info& info = ob->get_info();
	if (!info.is_animated())	// Assume it's just SFX.
		return new Sfx_animator(ob);
	else if (frames > 1)
		return new Frame_animator(ob);
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
	while (gwin->get_tqueue()->remove(this))
		;
	if (objsfx)
		objsfx->stop();
	}

/*
 *	Start animation.
 */

void Animator::start_animation
	(
	)
	{
					// Clean out old entry if there.
	gwin->get_tqueue()->remove(this);
	gwin->get_tqueue()->add(Game::get_ticks() + 20, this, (long) gwin);
	animating = 1;
	}

/*
 *	Retrieve current frame
 */

int Animator::get_framenum()
{
	return obj->get_framenum();
}

/*
 *	Create a frame animator.
 */

Frame_animator::Frame_animator
	(
	Game_object *o
	) : Animator(o)
{
	Initialize();
}

/*
 *	Initialize a frame animator.
 */
void Frame_animator::Initialize()
{
	created = currpos = 0;
	delay = 100;

	last_shape = obj->get_shapenum();
	last_frame = obj->get_framenum();

	aniinf = Shapeinfo_lookup::get_animation_cycle_info(last_shape, last_frame);
	if (!aniinf)
		{	 // Default animation cycle.
		new_aniinf = true;
		aniinf = new Animation_info();
		aniinf->type = FA_LOOPING;
		aniinf->first_frame = 0;
		aniinf->frame_count = obj->get_num_frames();
		aniinf->offset = 0;
		aniinf->offset_type = 0;
		aniinf->frame_delay = 1;
		aniinf->sfx_info = 0;
		}
	else
		{
		new_aniinf = false;
		delay *= aniinf->frame_delay;

		if (aniinf->type == FA_LOOPING)
			{
			switch (aniinf->offset_type)
				{
				case -1:
					created = currpos = last_frame;
					currpos -= aniinf->first_frame;
					break;
				case 0:
					created = currpos = aniinf->offset;
					break;
				case 1:
					created = currpos = last_frame % aniinf->frame_count;
					break;
				}
			}
		else if (aniinf->type == FA_RANDOM_LOOP)
			currpos = aniinf->frame_count - 1;
		}
}

/*
 *	Retrieve current frame
 */

int Frame_animator::get_next_frame()
{
	int framenum = 0;

	switch ((AniType)aniinf->type)
	{
	case FA_HOURLY:
		framenum = gclock->get_hour() % aniinf->frame_count;  
		break;

	case FA_NON_LOOPING:
		currpos++;
		if (currpos >= aniinf->frame_count) 
			currpos = aniinf->frame_count - 1;
		framenum = aniinf->first_frame + currpos;
		break;

	case FA_LOOPING:
		{
		unsigned int ticks = Game::get_ticks();
		framenum = (ticks / delay) + created;
		framenum %= aniinf->frame_count;
		currpos = framenum;
		framenum += aniinf->first_frame;
		break;
		}
	case FA_RANDOM_LOOP:
		currpos++;
		currpos %= aniinf->frame_count;
		framenum = aniinf->first_frame + currpos;
		break;

	case FA_LOOP_RECYCLE:
		currpos++;
		currpos %= aniinf->frame_count;
		if (!currpos)
			currpos += aniinf->offset - aniinf->first_frame;
		framenum = aniinf->first_frame + currpos;
		break;
	}

	return framenum;
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
	unsigned int ticks = Game::get_ticks();

	Game_window *gwin = (Game_window *) udata;

	if (!gwin->add_dirty(obj))
	{	// No longer on screen.
		animating = 0;
		// Stop playing sound.
		if (objsfx)
			objsfx->stop();
		return;
	}

	int framenum = get_next_frame();
	obj->set_frame(last_frame = framenum);
	gwin->add_dirty(obj);

	if (objsfx)
	{	// Sound effect?
		if (aniinf->type == FA_RANDOM_LOOP)
			{
			if (currpos == 1)
				objsfx->update();
			}
		else
			{
			if ((!aniinf->sfx_info && !currpos) ||	// Synch with animation
					(aniinf->sfx_info &&
						(currpos % aniinf->sfx_info) == 0))	// skip n frames
				objsfx->update();
			}
	}

	// Add back to queue for next time.
	if (animating)
	{
		// Ensure all animations are synched
		int nextticks = ticks + delay - (ticks%delay);
		if (aniinf->type == FA_RANDOM_LOOP && !currpos)
			nextticks += delay * (rand() % 20);
		gwin->get_tqueue()->add(nextticks, this, udata);
	}
}

/*
 *	Create a pure SFX player.
 */

Sfx_animator::Sfx_animator
	(
	Game_object *o
	) : Animator(o)
{
}

/*
 *	Play SFX.
 */

void Sfx_animator::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
{
	const int delay = 100;		// Guessing this will be enough.
	unsigned int ticks = Game::get_ticks();

	Game_window *gwin = (Game_window *) udata;
	Rectangle rect = gwin->clip_to_win(gwin->get_shape_rect(obj));
	if (rect.w <= 0 || rect.h <= 0)
	{	// No longer on screen.
		animating = 0;
		// Stop playing sound.
		if (objsfx)
			objsfx->stop();
		return;
	}

	if (objsfx)		// Sound effect?
		objsfx->update();
					// Add back to queue for next time.
	if (animating)
		{
		int nextticks = ticks + delay - (ticks%delay);
		if (objsfx->get_has_delay())
			nextticks += delay * (10 + rand() % 30);
		gwin->get_tqueue()->add(nextticks, this, udata);
		}
}

/*
 *	Create a field frame animator.
 */

Field_frame_animator::Field_frame_animator
	(
	Game_object *o
	) : Frame_animator(o), activated(true)
	{
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
	Frame_animator::handle_event(curtime, udata);
	if (activated && rand()%10 == 0)// Check for damage?
		obj->activate(0);
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
	Tile_coord t = obj->get_tile();	// Get current position.
	int newdx = rand()%3;
	int newdy = rand()%3;
	t.tx += -deltax + newdx;
	t.ty += -deltay + newdy;
	deltax = newdx;
	deltay = newdy;
	obj->Game_object::move(t.tx, t.ty, t.tz);
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
	) : Terrain_game_object(shapenum, framenum, tilex, tiley, lft)
	{
	animator = Animator::create(this);
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
	)
	{
	animator->want_animation();	// Be sure animation is on.
	Game_object::paint();
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
	animator = Animator::create(this);
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
	)
	{
	animator->want_animation();	// Be sure animation is on.
	Ireg_game_object::paint();
	}

/*
 *	Write out.
 */

void Animated_ireg_object::write_ireg(DataSource *out)
{
	int oldframe = get_framenum();
	set_frame(animator->get_framenum());
	Ireg_game_object::write_ireg(out);
	set_frame(oldframe);
}

/*
 *	Create at given position.
 */

Animated_ifix_object::Animated_ifix_object
	(
	int shapenum, 
	int framenum, 
	unsigned int tilex, unsigned int tiley, 
	unsigned int lft
	) : Ifix_game_object(shapenum, framenum, tilex, tiley, lft)
	{
	animator = Animator::create(this);
	}

/*
 *	When we delete, better remove from queue.
 */

Animated_ifix_object::~Animated_ifix_object
	(
	)
	{
	delete animator;
	}

/*
 *	Render.
 */

void Animated_ifix_object::paint
	(
	)
	{
	animator->want_animation();	// Be sure animation is on.
	Ifix_game_object::paint();
	}

/*
 *	Write out an IFIX object.
 */

void Animated_ifix_object::write_ifix(DataSource *ifix,  bool v2)

{
	int oldframe = get_framenum();
	set_frame(animator->get_framenum());
	Ifix_game_object::write_ifix(ifix, v2);
	set_frame(oldframe);
}


