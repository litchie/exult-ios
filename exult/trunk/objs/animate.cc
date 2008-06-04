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
#include "aniinf.h"
#include "sfxinf.h"

#ifndef UNDER_EMBEDDED_CE
using std::map;
using std::ostream;
using std::rand;
using std::endl;
using std::cout;
#endif


static inline bool Get_sfx_volume
	(
	Game_window *gwin,
	Game_object *obj,
	int& distance,
	int& volume,
	int& dir
	)
	{
	//distance = gwin->get_main_actor()->distance(obj);
	distance = gwin->get_main_actor()->sound_distance(obj);

	if (distance)
		{			// 160/8 = 20 tiles. 20*20=400.
		volume = (MIX_MAX_VOLUME*64)/(3*distance);
		if (!volume)		// Dead?
			return true;	// Time to kill it.
		if (volume < 8)
			volume = 8;
		else if (volume > MIX_MAX_VOLUME)
			volume = MIX_MAX_VOLUME;
		Tile_coord apos = gwin->get_main_actor()->get_center_tile();
		Tile_coord opos = obj->get_center_tile();
		dir = Get_direction16(apos.ty - opos.ty, opos.tx - apos.tx);
		}
	return false;
	}

/*
 *	Play SFX.
 */

Object_sfx::Object_sfx(Game_object *o, int s, int delay)
	: obj(o), distance(-1), channel(-1), sfx(s)
	{	// Start immediatelly.
	gwin->get_tqueue()->add(Game::get_ticks() + delay, this, (long) gwin);
	}

void Object_sfx::stop()
	{
	while (gwin->get_tqueue()->remove(this))
		;
	if(channel >= 0)
		{
		if (Mix_Playing(channel))
			Mix_HaltChannel(channel);
		channel = -1;
		}
	delete this;
	}

void Object_sfx::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Game window.
	)
	{
	const int delay = 100;		// Guessing this will be enough.

	bool active = channel != -1 ? Mix_Playing(channel) : false;

	if (obj->is_pos_invalid() || distance >= 0 && !active)
		{	// Quitting time.
		stop();
		return;
		}

	dir = 0;
	int volume = MIX_MAX_VOLUME;	// Set volume based on distance.
	bool halt = Get_sfx_volume(gwin, obj, distance, volume, dir);

	if (!halt && channel == -1 && sfx > -1)		// First time?
					// Start playing.
		channel = Audio::get_ptr()->play_sound_effect(
					sfx, volume, dir, 0);
	else if (channel != -1)
		{
		if (halt)
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

	if (channel != -1)
		gwin->get_tqueue()->add(curtime + delay - (curtime%delay), this, udata);
	else
		stop();
	}

/*
 *	Stop playing the sound effect if needed.
 */
void Shape_sfx::stop()
	{
	for (int i = 0; i < sizeof(channel)/sizeof(channel[0]); i++)
		{
		if(channel[i] >= 0)
			{
			Mix_HaltChannel(channel[i]);
			channel[i] = -1;
			}
		}
	}

/*
 *	Update distance/direction information. Also starts playing
 *	the sound effect if needed.
 */
void Shape_sfx::update
	(
	bool play
	)
	{
	if (obj->is_pos_invalid())
		{			// Not on map.
		stop();
		return;
		}

	if (!sfxinf)
		return;

	int active[2] = {0, 0};
	for (int i = 0; i < sizeof(channel)/sizeof(channel[0]); i++)
		{
		if (channel[i] != -1)
	 		active[i] = Mix_Playing(channel[i]);
		if (!active[i] && channel[i] != -1)
			{
			Mix_HaltChannel(channel[i]);
			channel[i] = -1;
			}
		}
	// If neither channel is playing, and we are not going to
	// play anything now, we have nothing to do.
	if (!play && channel[0] == -1 && channel[1] == -1)
		return;

	int sfxnum[2] = {-1, -1};
	if (play && channel[0] == -1)
		{
		if (!sfxinf->time_to_play())
			return;
		sfxnum[0] = sfxinf->get_next_sfx(last_sfx);
		}
	int rep[2] = {0, 0};
	if (play && channel[1] == -1 && sfxinf->play_horly_ticks())
		{
		Game_clock *gclock = Game_window::get_instance()->get_clock();
		if (gclock->get_minute() == 0)
			{	// Play sfx->extra every hour for reps = hour
			int reps = gclock->get_hour()%12;
			rep[1] = (reps ? reps : 12) - 1;
			sfxnum[1] = sfxinf->get_extra_sfx();
			}
		}

	dir = 0;
	int volume = MIX_MAX_VOLUME;	// Set volume based on distance.
	bool halt = Get_sfx_volume(gwin, obj, distance, volume, dir);

	if (play && halt)
		play = false;

	for (int i = 0; i < sizeof(channel)/sizeof(channel[0]); i++)
		if (play && channel[i] == -1 && sfxnum[i] > -1)		// First time?
						// Start playing.
			channel[i] = Audio::get_ptr()->play_sound_effect(
						sfxnum[i], volume, dir, rep[i]);
		else if (channel[i] != -1)
			{
			if(halt)
				{
				Mix_HaltChannel(channel[i]);
				channel[i] = -1;
				}
			else
				{
				//Just change the "location" of the sound
				Mix_Volume(channel[i], volume);
				Mix_SetPosition(channel[i], (dir * 22), 0);
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
		{
		objsfx->stop();
		delete objsfx;
		}
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
	last_shape = obj->get_shapenum();
		// Catch rotated objects here.
	last_frame = obj->get_framenum() & ~(1<<5);
	int rotflag = obj->get_framenum() & (1<<5);

	ShapeID shp(last_shape, last_frame);
	aniinf = obj->get_info().get_animation_info_safe(last_shape,
				shp.get_num_frames());
	int cnt = aniinf->get_frame_count();
	if (cnt < 0)
		nframes = shp.get_num_frames();
	else
		nframes = cnt;
	if (nframes == shp.get_num_frames())
		first_frame = 0;
	else
		first_frame = last_frame - (last_frame%nframes);
		// Ensure proper bounds.
	if (first_frame + nframes >= shp.get_num_frames())
		nframes = shp.get_num_frames() - first_frame;
	assert(nframes > 0);

	frame_counter = aniinf->get_frame_delay();

	if (aniinf->get_type() == Animation_info::FA_TIMESYNCHED)
		created = currpos = last_frame%nframes;
	else
		created = currpos = 0;
		// Add rotate flag back.
	first_frame |= rotflag;
	last_frame |= rotflag;
	}

/*
 *	Retrieve current frame
 */

int Frame_animator::get_next_frame()
{
	// Re-init if it's outside the range.
	// ++++++Should we do this for the other cases (jsf)?
	// ++++++Seeing if it breaks anything (marzo)
	int curframe = obj->get_framenum();
	if (curframe < first_frame ||
		curframe >= first_frame + nframes)
		Initialize();

	if (nframes == 1)	// No reason to do anything else.
		return first_frame;

	int framenum;
	switch (aniinf->get_type())
	{
	case Animation_info::FA_HOURLY:
		framenum = gclock->get_hour() % nframes;  
		break;

	case Animation_info::FA_NON_LOOPING:
		currpos++;
		if (currpos >= nframes) 
			currpos = nframes - 1;
		framenum = first_frame + currpos;
		break;

	case Animation_info::FA_TIMESYNCHED:
		{
		unsigned int ticks = Game::get_ticks();
		const int delay = 100;
		currpos = (ticks / (delay * aniinf->get_frame_delay())) + created;
		currpos %= nframes;
		framenum = first_frame + currpos;
		break;
		}

	case Animation_info::FA_LOOPING:
		{
		int chance = aniinf->get_freeze_first_chance();
		if (currpos || chance == 100
			|| (chance && rand()%100 < chance))
			{
			currpos++;
			currpos %= nframes;
			if (!currpos)
				currpos += aniinf->get_recycle();
			}
		framenum = first_frame + currpos;
		break;
		}

	case Animation_info::FA_RANDOM_FRAMES:
		currpos = rand() % nframes;
		framenum = first_frame + currpos;
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
	const int delay = 100;
	Game_window *gwin = (Game_window *) udata;

	if (!--frame_counter)
		{
		frame_counter = aniinf->get_frame_delay();
		bool dirty_first = gwin->add_dirty(obj);
		int framenum = get_next_frame();
		obj->set_frame(last_frame = framenum);
		if (!dirty_first && !gwin->add_dirty(obj))
			{	// No longer on screen.
			animating = 0;
			// Stop playing sound.
			if (objsfx)
				objsfx->stop();
			return;
			}
		}

	if (objsfx)
		{	// Sound effect?
		bool play;
		if (frame_counter != aniinf->get_frame_delay())
			play = false;
		else if (aniinf->get_sfx_delay() < 0)
			{	// Only in synch with animation.
			if (aniinf->get_freeze_first_chance() < 100)
				// Not in (frozen) first frame.
				play = (currpos == 1);
			else
				play = (currpos == 0);
			}
		else if (aniinf->get_sfx_delay() > 1)
				// Skip (sfx_delay-1) frames.
			play = (currpos % aniinf->get_sfx_delay()) == 0;
		else
			// Continuous.
			play = true;
		objsfx->update(play);
		}

	// Add back to queue for next time.
	if (animating)
		// Ensure all animations are synched
		gwin->get_tqueue()->add(curtime + delay - (curtime%delay), this, udata);
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
		objsfx->update(true);
					// Add back to queue for next time.
	if (animating)
		gwin->get_tqueue()->add(curtime + delay - (curtime%delay), this, udata);
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
	const int delay = 100;		// Delay between frames.
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


