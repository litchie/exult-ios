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
#include "Audio.h"
#include "actors.h"			/* Only need this for Object_sfx. */
#include "dir.h"
#include "Flex.h"
#include <map>
#include <string>

using std::map;
using std::ostream;
using std::rand;

using std::endl;
using std::cout;




/*
 *	A class for playing sound effects when certain objects are nearby.
 */
class Object_sfx
	{
	int sfxnum;			// Sound effect #.
	int sfx;			// ID of sound effect being played.
	Game_object *obj;		// Object that caused the sound.
	int distance;			// Distance in tiles from Avatar.
	int dir;			// Direction (0-15) from Avatar.
public:
					// Create & start playing sound.
	Object_sfx(int snum, Game_object *o) : sfxnum(snum), distance(0), sfx(-1)
		{ set_obj(o); }
//	bool is_active()		// Is sound still active?
//		{ return sfx.is_active(); }
	int get_sfxnum()
		{ return sfxnum; }
	int get_distance()
		{ return distance; }
	void set_obj(Game_object *o);	// Set to new object.
	void stop(Game_object *o)	// Stop if from given object.
		{
		if (o == obj)
			if(sfx >= 0)
				{
				Mix_HaltChannel(sfx);
#ifdef DEBUG
				cout << "AUDIO channel "<< sfx << " stopped" << endl;
#endif
				sfx = -1;
				}
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
	Tile_coord apos = gwin->get_main_actor()->get_tile();
	Tile_coord opos = o->get_tile();
	int active = 0;
	if(sfx != -1)
	 	active = Mix_Playing(sfx);
	int new_distance = apos.distance(opos);
	if (active && new_distance >= distance && o != obj)
		return;			// Farther than current source.
	obj = o;
	dir = 0;
	bool repeat = true;
	distance = new_distance;
	int volume = MIX_MAX_VOLUME;	// Set volume based on distance.
	if (distance)
		{			// 160/8 = 20 tiles. 20*20=400.
		volume = (MIX_MAX_VOLUME*64)/(distance*distance);
		if (!volume)		// Dead?
			repeat = false;	// Time to kill it.
		if (volume < 8)
			volume = 8;
		else if (volume > MIX_MAX_VOLUME)
			volume = MIX_MAX_VOLUME;
		dir = Get_direction16(apos.ty - opos.ty, opos.tx - apos.tx);
		}
	if (sfx == -1)		// First time?
		{	

					// Start playing, and repeat.
		sfx = Audio::get_ptr()->play_sound_effect(sfxnum, MIX_MAX_VOLUME, dir, -1);
		Mix_Volume(sfx, volume);
#ifdef DEBUG
		cout << "AUDIO created repeating object " << sfxnum << ", channel " << sfx << endl;
#endif
		}
	else				// Set new volume, position.
		{
		//Just change the "location" of the sound
		if(!repeat)
			{
			Mix_HaltChannel(sfx);
#ifdef DEBUG
			cout << "AUDIO halted repeat channel "<< sfx << endl;
#endif
			sfx = -1;
			}
		else
			{
			Mix_Volume(sfx, volume);
			Mix_SetPosition(sfx, (dir * 22), 0);
			}
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

		// Grandfather clock tick tock, only in the SQSFX files,
		 if (Audio::get_ptr()->get_sfx_file() != 0)
			{
			std::string s = 
				Audio::get_ptr()->get_sfx_file()->filename;
			to_uppercase(s);
			if(s.find("SQSFX") != std::string::npos)
				{
				table[252] = 116;	// Grandfather clock 
				table[695] = 116;	// Grandfather clock 
	 			}
			}	
				
		}
	std::map<int, int>::iterator it = table.find(shapenum);
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
	std::map<int, Object_sfx*>::iterator it = play_table.find(snum);
	if (it == play_table.end())	// No.
		{			// Start new SFX for it.
		if (!stop)
			play_table[snum] = new Object_sfx(snum, o);
		return;
		}
	Object_sfx *sfx = (*it).second;
	if (stop)
	{
		sfx->stop(o);
	}
	else
		sfx->set_obj(o);	// Modify/restart.
	}

/*
 *	Create appropriate animator.
 */

Animator *Animator::create
	(
	Game_object *ob,		// Animated object.
	bool ireg			// 1 if an IREG object.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int shnum = ob->get_shapenum();
	int frames = ob->get_num_frames();
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
	Game_object *o,
	bool ir
	) : Animator(o, Object_sfx::get_shape_sfx(o->get_shapenum())),
		ireg(ir)
{
	Initialize();
}

/*
 *	Initialize a frame animator.
 */
void Frame_animator::Initialize()
{
	Game_window *gwin = Game_window::get_game_window();

	first_frame = 0;
	created = 0;
	delay = 100;
	type = FA_LOOPING;

	last_shape = obj->get_shapenum();
	last_frame = obj->get_framenum();
	frames = obj->get_num_frames();

	// Serpent Isle
	if (Game::get_game_type() == SERPENT_ISLE)
	{
		switch (last_shape)
		{
		
		case 284:		// Sundial is a special case.
			type = FA_SUNDIAL;
			break;
		
		case 768:		// Energy field.  Stop at top.
			type = FA_ENERGY_FIELD;
			created = Game::get_ticks();
			first_frame = last_frame;
			break;
		
		case 153:		// Fountain
		case 184:		// Lava
		case 222:		// Pennant
		case 289:		// Fire 
		case 305:		// Serpent Statue
		case 326:		// Fountain
		case 456:		// Flux Analyzer
		case 614:		// Magic music box
		case 655:		// Planets 
		case 695:		// Grandfather clock
		case 726:		// Pulsating Object
		case 743:		// Statue??
		case 794:		// Severed limb
		case 992:		// Burning urn
			first_frame = 6*(last_frame/6);
			frames = 6;
			created = last_frame%6;
			break;

		case 779:		// Magic Wave.
			first_frame = 6*(last_frame/6);
			frames = 6;
			if (last_frame < 6) created = 1;
			else created = 0;
			break;

		case 335:		// Bubbles
			created = last_frame;
			if (last_frame < 6)
			{
				first_frame = 0;
				frames = 6;
			}
			else
			{
				first_frame = 6;
				frames = frames-6;
			}
			break;

		case 322:		// Basin
		case 714:		// Basin
			created = last_frame;
			if (last_frame != frames-1)
			{
				first_frame = 0;
				frames = frames - 1;
			}
			else
			{
				first_frame = frames -1;
				frames = 1;
			}
			break;


		default:
			break;
		}
	}
	// Black Gate
	else
	{
		switch (last_shape)
		{
		
		case 284:		// Sundial is a special case.
			type = FA_SUNDIAL;
			break;
		
		case 768:		// Energy field.  Stop at top.
			type = FA_ENERGY_FIELD;
			created = Game::get_ticks();
			first_frame = last_frame;
			break;
		
					// First frame isn't animated
		case 862:		// Shafts
		case 880:
		case 933:		// Millsaw
			if (last_frame == 0)
			{
				first_frame = 0;
				frames = 1;
			}
			else
			{
				frames = frames -1;
				first_frame = 1;
			}
			break;

		default:
			break;
		}
	}
}

/*
 *	Retrieve current frame
 */

int Frame_animator::get_framenum()
{
	unsigned int ticks = Game::get_ticks();
	int framenum = 0;

	if (last_shape != obj->get_shapenum() || last_frame != obj->get_framenum())
		Initialize();

	Game_window *gwin = Game_window::get_game_window();

	bool dirty_first = gwin->add_dirty(obj);

	if (last_shape != obj->get_shapenum() || last_frame != obj->get_framenum())
		Initialize();

	switch (type)
	{
	case FA_SUNDIAL:
		framenum = gwin->get_hour() % frames;  
		break;

	case FA_ENERGY_FIELD:
		framenum = (ticks - created) / delay + first_frame;
		if (framenum >= frames) framenum = frames-1;
		break;

	case FA_LOOPING:
		framenum = (ticks / delay) + created;
		framenum %= frames;
		framenum += first_frame;
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

	bool dirty_first = gwin->add_dirty(obj);

	int framenum = get_framenum();

	obj->set_frame(last_frame = framenum);

	if (!dirty_first && !gwin->add_dirty(obj))
	{				// No longer on screen.
		animating = 0;
					// Stop playing sound.
		Object_sfx::play(obj, sfxnum, true);
		return;
	}

	if (!framenum && sfxnum >= 0)	// Sound effect?
		Object_sfx::play(obj, sfxnum);
					// Add back to queue for next time.
	if (animating)
	{
		// Ensure all animations are synced
		gwin->get_tqueue()->add(ticks  + delay- (ticks%delay), this, udata);
	}
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
	frames = obj->get_num_frames();
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
	animator->want_animation();	// Be sure animation is on.
	Game_object::paint(gwin);
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
	animator->want_animation();	// Be sure animation is on.
	Ireg_game_object::paint(gwin);
	}

/*
 *	Write out.
 */

void Animated_ireg_object::write_ireg(ostream& out)
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
	animator = Animator::create(this, 0);
	}

/*
 *	Create from IFIX
 */

Animated_ifix_object::Animated_ifix_object(unsigned char *ifix) : Ifix_game_object(ifix)
	{
	animator = Animator::create(this, 0);
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
	Game_window *gwin
	)
	{
	animator->want_animation();	// Be sure animation is on.
	Ifix_game_object::paint(gwin);
	}

/*
 *	Write out an IFIX object.
 */

void Animated_ifix_object::write_ifix(ostream& ifix)

{
	int oldframe = get_framenum();
	set_frame(animator->get_framenum());
	Ifix_game_object::write_ifix(ifix);
	set_frame(oldframe);
}


