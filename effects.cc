/*
 *	effects.h - Special effects.
 *
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

#include "gamewin.h"
#include "gamemap.h"
#include "gameclk.h"
#include "actors.h"
#include "effects.h"
#include "Zombie.h"
#include "dir.h"
#include "chunks.h"
#include "Audio.h"
#include "Gump_manager.h"
#include "game.h"
#include "Gump.h"

#include "SDL_timer.h"

using std::cout;
using std::endl;
using std::rand;
using std::string;
using std::strlen;

int Cloud::randcnt = 0;
int Lightning_effect::active = 0;

/*
 *	Clean up.
 */

Effects_manager::~Effects_manager()
	{ 
	remove_all_effects(false);
	}

/*
 *	Add text over a given item.
 */

void Effects_manager::add_text
	(
	const char *msg,
	Game_object *item		// Item text ID's, or null.
	)
	{
	if (!msg)			// Happens with edited games.
		return;
					// Don't duplicate for item.
	for (Special_effect *each = effects; each; each = each->next)
		if (each->is_text(item))
			return;		// Already have text on this.
	Text_effect *txt = new Text_effect(msg, item);
//	txt->paint(this);		// Draw it.
//	painted = 1;
	add_effect(txt);
	}

/*
 *	Add a text object at a given spot.
 */

void Effects_manager::add_text
	(
	const char *msg,
	int x, int y			// Pixel coord. on screen.
	)
	{
	Text_effect *txt = new Text_effect(msg,
		gwin->get_scrolltx() + x/c_tilesize, 
		gwin->get_scrollty() + y/c_tilesize);
	add_effect(txt);
	}

/*
 *	Add a text object in the center of the screen
 */

void Effects_manager::center_text
	(
	const char *msg
	)
	{
	remove_text_effects();
	Shape_manager *sman = Shape_manager::get_instance();
	add_text(msg, (gwin->get_width()-sman->get_text_width(0,msg))/2,
			 gwin->get_height()/2);
	}

/*
 *	Add an effect at the start of the chain.
 */

void Effects_manager::add_effect
	(
	Special_effect *effect
	)
	{
	effect->next = effects;		// Insert into chain.
	effect->prev = 0;
	if (effect->next)
		effect->next->prev = effect;
	effects = effect;
	}

/*
 *	Remove a given object's text effect.
 */

void Effects_manager::remove_text_effect
	(
	Game_object *item		// Item text was added for.
	)
	{
	for (Special_effect *each = effects; each; each = each->next)
		if (each->is_text(item))
			{		// Found it.
			remove_effect(each);
			gwin->paint();
			return;
			}
	}

/*
 *	Remove a text item/sprite from the chain and delete it.
 */

void Effects_manager::remove_effect
	(
	Special_effect *effect
	)
	{
	if (effect->in_queue())
		gwin->get_tqueue()->remove(effect);
	if (effect->next)
		effect->next->prev = effect->prev;
	if (effect->prev)
		effect->prev->next = effect->next;
	else				// Head of chain.
		effects = effect->next;
	delete effect;
	}

/*
 *	Remove all text items.
 */

void Effects_manager::remove_all_effects
	(
	 bool repaint
	)
	{
	if (!effects)
		return;
	while (effects)
		remove_effect(effects);
	if (repaint)
		gwin->paint();		// Just paint whole screen.
	}

/*
 *	Remove text effects.
 */

void Effects_manager::remove_text_effects
	(
	)
	{
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
		if (each->is_text())
			remove_effect(each);
		each = next;
		}
	gwin->set_all_dirty();
	}


/*
 *	Remove weather effects.
 */

void Effects_manager::remove_weather_effects
	(
	int dist			// Only remove those from eggs at
					//   least this far away.
	)
	{
	Actor *main_actor = gwin->get_main_actor();
	Tile_coord apos = main_actor ? main_actor->get_tile()
				: Tile_coord(-1, -1, -1);
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
					// See if we're far enough away.
		if (each->is_weather() && (!dist ||
		    ((Weather_effect *) each)->out_of_range(apos, dist)))
			remove_effect(each);
		each = next;
		}
	gwin->set_all_dirty();
	}

/*
 *	Find last numbered weather effect added.
 */

int Effects_manager::get_weather
	(
	)
	{
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
		if (each->is_weather())
			{
			Weather_effect *weather = (Weather_effect *) each;
			if (weather->get_num() >= 0)
				return weather->get_num();
			}
		each = next;
		}
	return 0;
	}

/*
 *	Paint them all.
 */

void Effects_manager::paint
	(
	)
	{
	for (Special_effect *txt = effects; txt; txt = txt->next)
		txt->paint();
	}

/*
 *	Some special effects may not need painting.
 */

void Special_effect::paint
	(
	)
	{
	}

/*
 *	Create an animation from the 'sprites.vga' file.
 */

Sprites_effect::Sprites_effect
	(
	int num,			// Index.
	Tile_coord p,			// Position within world.
	int dx, int dy,			// Add to offset for each frame.
	int delay			// Delay (msecs) before starting.
	) : sprite(num, 0, SF_SPRITES_VGA), item(0), pos(p), xoff(0), yoff(0),
						deltax(dx), deltay(dy)
	{
	Game_window *gwin = Game_window::get_instance();
	frames = sprite.get_num_frames();
					// Start.
	gwin->get_tqueue()->add(Game::get_ticks() + delay, this, 0L);
	}

/*
 *	Create an animation on an object.
 */

Sprites_effect::Sprites_effect
	(
	int num,			// Index.
	Game_object *it,		// Item to put effect by.
	int xf, int yf,			// Offset from actor in pixels.
	int dx, int dy			// Add to offset on each frame.
	) : sprite(num, 0, SF_SPRITES_VGA), item(it), xoff(xf), 
					yoff(yf), deltax(dx), deltay(dy)
	{
	pos = item->get_tile();
	Game_window *gwin = Game_window::get_instance();
	frames = sprite.get_num_frames();
					// Start immediately.
	gwin->get_tqueue()->add(Game::get_ticks(), this, 0L);
	}

/*
 *	Add a dirty rectangle for the current position and frame.
 */

inline void Sprites_effect::add_dirty
	(
	int frnum
	)
	{
	if (pos.tx == -1 || frnum == -1)
		return;			// Already at destination.
	Shape_frame *shape = sprite.get_shape();
	int lp = pos.tz/2;

	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(shape,
		xoff + (pos.tx - lp - gwin->get_scrolltx())*c_tilesize,
	    	yoff + (pos.ty - lp - 
			gwin->get_scrollty())*c_tilesize).enlarge(4)));
	}

/*
 *	Animation.
 */

void Sprites_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	int frame_num = sprite.get_framenum();
	Game_window *gwin = Game_window::get_instance();
	int delay = gwin->get_std_delay();// Delay between frames.  Needs to
					//   match usecode animations.
	if (frame_num == frames)	// At end?
		{			// Remove & delete this.
		eman->remove_effect(this);
		gwin->set_all_dirty();
		return;
		}
	add_dirty(frame_num - 1);	// Clear out old.
	gwin->set_painted();
	if (item)			// Following actor?
		pos = item->get_tile();
	xoff += deltax;			// Add deltas.
	yoff += deltay;
	add_dirty(frame_num);		// Want to paint new frame.
	frame_num++;			// Next frame.
	sprite.set_frame(frame_num);
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Sprites_effect::paint
	(
	)
	{
	if (sprite.get_framenum() >= frames)
		return;
	int lp = pos.tz/2;		// Account for lift.
	sprite.paint_shape(
		xoff + (pos.tx - lp - gwin->get_scrolltx())*c_tilesize,
		yoff + (pos.ty - lp - gwin->get_scrollty())*c_tilesize);
	}

/*
 *	Start explosion.
 */

Explosion_effect::Explosion_effect
	(
	Tile_coord p, 
	Game_object *exp,
	int delay			// Delay before starting (msecs).
	) : Sprites_effect(1, p, 0, 0, delay), explode(exp)
{
	if (exp && exp->get_shapenum() == 704) { // powderkeg
		exp->set_quality(1); // mark as detonating
	}
}


void Explosion_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	int frnum = sprite.get_framenum();
	if (!frnum)			// Max. volume, with stereo position.
		{
		Tile_coord apos = gwin->get_main_actor()->get_tile();
		int dir = Get_direction16(apos.ty - pos.ty, pos.tx - apos.tx);

		Audio::get_ptr()->play_sound_effect(
				Audio::game_sfx(9), SDL_MIX_MAXVOLUME, dir);
		}
	if (frnum == frames/4) {
		// this was in ~Explosion_effect before
		if (explode)
			{
			Game_window::get_instance()->add_dirty(explode);
			explode->remove_this();
			explode = 0;
			}
		Game_object_vector vec;	// Find objects near explosion.
		Game_object::find_nearby(vec, pos, c_any_shapenum, 5, 0);
		for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				(**it).attacked(0, 704, 0);
			}
	}
	Sprites_effect::handle_event(curtime, udata);
}

/*
 *	Get direction in 1/16's starting from North.
 */

inline int Get_dir16
	(
	Tile_coord& t1,
	Tile_coord& t2
	)
	{
					// Treat as cartesian coords.
	return Get_direction16(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Initialize path & add to time queue.
 */

void Projectile_effect::init
	(
	Tile_coord s,			// Source,
	Tile_coord d			// Destination.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Shape_info& info = ShapeID::get_info(projectile_shape);
	Weapon_info *winfo = info.get_weapon_info();
	if (winfo && winfo->get_projectile())	// Different sprite to show?
		sprite.set_shape(winfo->get_projectile());
	frames = sprite.get_num_frames();
	pos = s;			// Get starting position.
	if (attacker)			// Try to set start better.
		{
		Shape_info& info = attacker->get_info();
					// Try for around the heat.
		pos.tz += info.get_3d_height() - 1;
		if (d.tx < pos.tx)	// Start on proper side.
			pos.tx -= info.get_3d_xtiles();
		else
			pos.tx++;
		if (d.ty < pos.ty)
			pos.ty -= info.get_3d_ytiles();
		else
			pos.ty++;
		}
	path = new Zombie();		// Create simple pathfinder.
					// Find path.  Should never fail.
	path->NewPath(pos, d, 0);
	if (frames >= 24)		// Use frames 8-23, for direction
		{			//   going clockwise from North.
		int dir = Get_dir16(s, d);
		sprite.set_frame(8 + dir);
		}
	else if (frames == 1 && sprite.get_shapenum() != 704)
		sprite.set_frame(0);	// (Don't show powder keg!)
	else
		sprite.set_frame(-1);	// We just won't show it.
					// Start immediately.
	gwin->get_tqueue()->add(Game::get_ticks(), this, 0L);
	}


/*
 *	Create a projectile animation.
 */

Projectile_effect::Projectile_effect
	(
	Actor *att,			// Source of spell/attack.
	Game_object *to,		// End here, 'attack' it with shape.
	int shnum,			// Projectile shape # in 'shapes.vga'.
	int weap			// Weapon (bow, gun, etc.) shape, or 0.
	) : attacker(att), target(to), projectile_shape(shnum),
	    sprite(shnum, 0), weapon(weap),
	    return_path(false)
	{
	init(attacker->get_tile(), to->get_tile());
	}

/*
 *	Constructor used by missile eggs.
 */

Projectile_effect::Projectile_effect
	(
	Tile_coord s,			// Start here.
	Tile_coord d,			// End here.
	int shnum,			// Projectile shape
	int weap			// Weapon (bow, gun, etc.) shape, or 0.
	) : attacker(0), target(0), projectile_shape(shnum),
	    sprite(shnum, 0), weapon(weap),
	    return_path(false)
	{
	init(s, d);
	}

/* 
 *	Another used by missile eggs.
 */

Projectile_effect::Projectile_effect
	(
	Tile_coord s,			// Start here.
	Game_object *to,		// End here, 'attack' it with shape.
	int shnum,			// Projectile shape
	int weap,			// Weapon (bow, gun, etc.) shape, or 0.
	bool retpath			// Return of a boomerang.
	) : attacker(0), target(to), projectile_shape(shnum),
	    sprite(shnum, 0), weapon(weap),
	    return_path(retpath)
	{
	init(s, to->get_tile());
	}

/*
 *	Delete.
 */

Projectile_effect::~Projectile_effect
	(
	)
	{
	delete path;
	}

/*
 *	Add a dirty rectangle for the current position and frame.
 */

inline void Projectile_effect::add_dirty
	(
	)
	{
	if (pos.tx == -1 || sprite.get_framenum() == -1)
		return;			// Already at destination.
	Shape_frame *shape = sprite.get_shape();
					// Force repaint of prev. position.
	int liftpix = pos.tz*c_tilesize/2;
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(shape,
			(pos.tx - gwin->get_scrolltx())*c_tilesize - liftpix,
	    (pos.ty - gwin->get_scrollty())*c_tilesize - liftpix).enlarge(4)));
	}

/*
 *	See if something was hit.
 *
 *	Output:	->target hit, or 0.
 */

inline Game_object *Find_target
	(
	Game_window *gwin,
	Tile_coord pos
	)
	{
	if (pos.tz%5 == 0)		// On floor?
		pos.tz++;		// Look up 1 tile.
	Tile_coord dest = pos;		// This gets modified.
	if (!Map_chunk::is_blocked(pos, 1, MOVE_FLY, 0) &&
	    dest == pos)
		return (0);
	return Game_object::find_blocking(pos);
	}

/*
 *	Animation.
 */

void Projectile_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int delay = gwin->get_std_delay()/2;
	add_dirty();			// Force repaint of old pos.
	Tile_coord epos = pos;		// Save pos.
	if (!path->GetNextStep(pos) ||	// Get next spot.
					// If missile egg, detect target.
			(!target && (target = Find_target(gwin, pos)) != 0))
		{			// Done? 
		int delay = 0;		// For explosions.
		switch (projectile_shape)
			{
		case 287:		// Swordstrike.
			eman->add_effect(new Sprites_effect(23, epos));
			break;
		case 554:		// Burst arrow.
			eman->add_effect(new Sprites_effect(19, epos));
			break;
		case 565:		// Starburst.
			eman->add_effect(new Sprites_effect(18, epos));
			break;
		case 639:		// Death Vortex.
			eman->add_effect(new Death_vortex(target, epos));
			target = 0;	// Takes care of attack.
			break;
		case 82:		// Delayed explosion.
		case 621:		//    "       "
			delay = 3000;	// Wait 3 secs.  FALL THROUGH!
		case 78:		// Explosion.
		case 702:		// Cannon.
		case 704:		// Powder keg.
			eman->add_effect(new Explosion_effect(epos, 0, delay));
			target = 0;	// Takes care of attack.
			break;
			}
		if (return_path)	// Returned a boomerang?
			target->add(gmap->create_ireg_object(weapon, 0));
		else
			{		// Not teleported away ?
			if (target && epos.distance(target->get_tile()) < 50)
				target->attacked(attacker, weapon, 
							projectile_shape);
			if (attacker && // Check for `boomerangs'
			    weapon == projectile_shape && 
			    epos.distance(attacker->get_tile() ) < 50)
				{ 	// not teleported away
				Weapon_info *winf = 
				   ShapeID::get_info(weapon).get_weapon_info();
				if (winf && winf->returns())
					eman->add_effect(new Projectile_effect(
						pos, attacker, weapon, weapon,
									true));
				}
			}
		add_dirty();
		pos.tx = -1;		// Signal we're done.
		eman->remove_effect(this);
		return;
		}
	add_dirty();			// Paint new spot/frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Projectile_effect::paint
	(
	)
	{
	if (pos.tx == -1 || sprite.get_framenum() == -1)
		return;			// Already at destination.
	int liftpix = pos.tz*c_tilesize/2;
	sprite.paint_shape(
		(pos.tx - gwin->get_scrolltx())*c_tilesize - liftpix,
		(pos.ty - gwin->get_scrollty())*c_tilesize - liftpix);
	}

/*
 *	Create a 'death vortex'.
 */

Death_vortex::Death_vortex
	(
	Game_object *trg,		// What to aim for.
	Tile_coord tp			// Target pos, if trg==0.
	) : vortex(8, 0, SF_SPRITES_VGA), next_damage_time(0)
	{
	pos = trg ? trg->get_tile() : tp;
	target = trg ? trg->as_actor() : 0;
	Game_window *gwin = Game_window::get_instance();
	frames = vortex.get_num_frames();
					// Go for 20 seconds.
	stop_time = Game::get_ticks() + 20*1000;
					// Start immediately.
	gwin->get_tqueue()->add(Game::get_ticks(), this, 0L);
	}

/*
 *	Add a dirty rectangle for the current position and frame.
 *
 *	Output:	Width in pixels.
 */

inline int Death_vortex::add_dirty
	(
	)
	{
	Shape_frame *shape = vortex.get_shape();
	int liftpix = pos.tz*c_tilesize/2;
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(shape,
		(pos.tx - gwin->get_scrolltx())*c_tilesize - liftpix,
	    (pos.ty - gwin->get_scrollty())*c_tilesize - liftpix).enlarge(4)));
	return shape->get_width();
	}

/*
 *	Animation.
 */

void Death_vortex::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int width = add_dirty();	// Repaint old area.
	if (target && !target->is_dead())	// Follow target.
		{
		Tile_coord tpos = target->get_tile();
		int deltax = tpos.tx - pos.tx, deltay = tpos.ty - pos.ty;
		int absx = deltax >= 0 ? deltax : -deltax;
		int absy = deltay >= 0 ? deltay : -deltay;
		if (absx > 2 || absy > 2)
			{
			if (deltax)
				pos.tx += deltax/absx;
			if (deltay)
				pos.ty += deltay/absy;
			}
		}
	if (curtime > next_damage_time)	// Time to cause damage.
		{			// Do it every second.
		next_damage_time = curtime + 1000;
		Actor_vector npcs;	// Find NPC's.
		Game_object::find_nearby(npcs, pos, -1, width/(2*c_tilesize), 
									8);
		for (Actor_vector::const_iterator it = npcs.begin();
							it != npcs.end(); ++it)
			{
			Actor *npc = *it;
			if (!npc->is_in_party())
				npc->reduce_health(40);
			}
		}
	vortex.set_frame((vortex.get_framenum() + 1)%frames);
	add_dirty();			// Paint new.
	if (curtime < stop_time)	// Keep going?
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		{
		gwin->set_all_dirty();
		eman->remove_effect(this);
		}
	}

/*
 *	Render.
 */

void Death_vortex::paint
	(
	)
	{
	int liftpix = pos.tz*c_tilesize/2;
	vortex.paint_shape(
		(pos.tx - gwin->get_scrolltx())*c_tilesize - liftpix,
		(pos.ty - gwin->get_scrollty())*c_tilesize - liftpix);
	}

/*
 *	Figure text position.
 */

static inline Tile_coord Figure_text_pos
	(
	Game_object *item
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Rectangle box;
					// See if it's in a gump.
	Gump *gump = gwin->get_gump_man()->find_gump(item);
	if (gump)
		box = gump->get_shape_rect(item);
	else
		box = gwin->get_shape_rect(item->get_outermost());
	return Tile_coord(gwin->get_scrolltx() + box.x/c_tilesize, 
			  gwin->get_scrollty() + box.y/c_tilesize, 0);
	}

/*
 *	Add dirty rectangle for current position.
 */

void Text_effect::add_dirty
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
					// Repaint slightly bigger rectangle.
	Rectangle rect((pos.tx - gwin->get_scrolltx() - 1)*c_tilesize,
		       (pos.ty - gwin->get_scrollty() - 1)*c_tilesize,
			width + 2*c_tilesize, height + 2*c_tilesize);
	gwin->add_dirty(gwin->clip_to_win(rect));
	}

/*
 *	Initialize.
 */

void Text_effect::init
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	width = 8 + sman->get_text_width(0, msg.c_str());
	height = 8 + sman->get_text_height(0);
	add_dirty();			// Force first paint.
					// Start immediately.
	gwin->get_tqueue()->add(Game::get_ticks(), this, 0L);
	if (msg[0] == '@')
		msg[0] = '"';
	int len = msg.size();
	if (msg[len - 1] == '@')
		msg[len - 1] = '"';
	}

/*
 *	Create a text effect for a given object.
 */

Text_effect::Text_effect
	(
	const string &m, 		// A copy is made.
	Game_object *it			// Item text is on, or null.
	) : msg(m), item(it), pos(Figure_text_pos(it)), num_ticks(0)
	{
	init();
	}

/*
 *	Create a text object.
 */

Text_effect::Text_effect
	(
	const string &m, 		// A copy is made.
	int t_x, int t_y		// Abs. tile coords.
	) : msg(m), item(0), pos(t_x, t_y, 0), num_ticks(0)
	{
	init();
	}

/*
 *	Reposition if necessary.
 */

void Text_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	if (++num_ticks == 10)		// About 1-2 seconds.
		{			// All done.
		add_dirty();
		eman->remove_effect(this);
		return;
		}
					// Back into queue.
	gwin->get_tqueue()->add(Game::get_ticks() + gwin->get_std_delay(), 
								this, 0L);
	if (!item)			// Nothing to move?
		return;
					// See if moved.
	Tile_coord npos = Figure_text_pos(item);
	if (npos == pos)		// No change?
		return;
	add_dirty();			// Force repaint of old area.
	pos = npos;			// Then set to repaint new.
	add_dirty();
	}

/*
 *	Render.
 */

void Text_effect::paint
	(
	)
	{
	const char *ptr = msg.c_str();
	int len = strlen(ptr);
	sman->paint_text(0, ptr, len, 
		(pos.tx - gwin->get_scrolltx())*c_tilesize,
				(pos.ty - gwin->get_scrollty())*c_tilesize);
	}

/*
 *	Init. a weather effect.
 */

Weather_effect::Weather_effect
	(
	int duration,			// Length in game minutes.
	int delay,			// Delay before starting.
	int n,				// Weather number.
	Game_object *egg		// Egg that started it, or null.
	) : num(n)
	{
	Game_window *gwin = Game_window::get_instance();
	if (egg)
		eggloc = egg->get_tile();
	else
		eggloc = Tile_coord(-1, -1, -1);
	stop_time = Game::get_ticks() + delay + 1000*((duration*60)/time_factor);
					// Start immediately.
	gwin->get_tqueue()->add(Game::get_ticks() + delay, this, 0L);
	}

/*
 *	Are we far enough away from this to cancel it?
 */

int Weather_effect::out_of_range
	(
	Tile_coord& avpos,		// Avatar's position.
	int dist			// Distance in tiles to cancel at.
	)
	{
	if (eggloc.tx == -1)		// Not created by an egg?
		return 0;
	return eggloc.distance(avpos) >= dist;
	}

/*
 *	Paint raindrop.
 */

inline void Raindrop::paint
	(
	Image_window8 *iwin,		// Where to draw.
	int scrolltx, int scrollty,	// Tile at top-left corner.
	Xform_palette xform		// Transform array.
	)
	{
	uint32 ascrollx = scrolltx*(uint32)c_tilesize,
		      ascrolly = scrollty*(uint32)c_tilesize;
	int x = ax - ascrollx, y = ay - ascrolly;
	if (x < 0 || y < 0 || 
			x >= iwin->get_width() || y >= iwin->get_height())
		return;
	if (oldpix == 255)
		oldpix = iwin->get_pixel8(x, y);	// Get pixel.
	iwin->put_pixel8(xform[oldpix], x, y);
	}

/*
 *	Move raindrop.
 */

inline void Raindrop::next
	(
	Image_window8 *iwin,		// Where to draw.	
	int scrolltx, int scrollty,	// Tile at top-left corner.
	Xform_palette xform,		// Transform array.
	int w, int h			// Dims. of window.
	)
	{
	uint32 ascrollx = scrolltx*(uint32)c_tilesize,
		      ascrolly = scrollty*(uint32)c_tilesize;
	int x = ax - ascrollx, y = ay - ascrolly;
					// Still on screen?  Restore pix.
	if (x >= 0 && y >= 0 && x < w && y < h && oldpix != 255)
		iwin->put_pixel8(oldpix, x, y);
	oldpix = 255;

					// Time to restart?
	if (x < 0 || x >= w || y < 0 || y >= h)
		{			
		int r = rand();
					// Have a few fall faster.
		yperx = (r%4) ? 1 : 2;
		ax = ascrollx + r%(w - w/8);
		ay = ascrolly + r%(h - h/4);
		}
	else				// Next spot.
		{
		int delta = 1 + rand()%4;
		ax += delta;
		ay += delta + yperx;
		}
					// Save old pixel & paint new.
	paint(iwin, scrolltx, scrollty, xform);
	}

/*
 *	Move raindrop.
 */

inline void Raindrop::next_random
	(
	Image_window8 *iwin,		// Where to draw.	
	int scrolltx, int scrollty,	// Tile at top-left corner.
	Xform_palette xform,		// Transform array.
	int w, int h			// Dims. of window.
	)
	{
	uint32 ascrollx = scrolltx*(uint32)c_tilesize,
		      ascrolly = scrollty*(uint32)c_tilesize;
	int x = ax - ascrollx, y = ay - ascrolly;
					// Still on screen?  Restore pix.
	if (x >= 0 && y >= 0 && x < w && y < h && oldpix != 255)
		iwin->put_pixel8(oldpix, x, y);
	oldpix = 255;
					// Pick new spot randomly.
	int newx = rand()%w, newy = rand()%h;
	ax = ascrollx + newx;
	ay = ascrolly + newy;
					// Save old pixel & paint new.
	paint(iwin, scrolltx, scrollty, xform);
	}

/*
 *	Rain.
 */

void Rain_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	if (!gwin->is_main_actor_inside() &&
	    !gwin->get_gump_man()->showing_gumps(true))
		{			// Don't show rain inside buildings!
		Image_window8 *win = gwin->get_win();
		int w = win->get_width(), h = win->get_height();
					// Get transform table.
		Xform_palette xform = sman->get_xform(8);//++++Experiment.
		int scrolltx = gwin->get_scrolltx(),
		    scrollty = gwin->get_scrollty();
					// Move drops.
		for (int i = 0; i < num_drops; i++)
			drops[i].next(win, scrolltx, scrollty, xform, w, h);
		gwin->set_painted();
		}
	if (curtime < stop_time)	// Keep going?
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		{
		gwin->set_all_dirty();
		eman->remove_effect(this);
		}
	}

/*
 *	Paint rain.
 */

void Rain_effect::paint
	(
	)
	{
	if (gwin->is_main_actor_inside() || gwin->get_gump_man()->showing_gumps(true))
		return;			// Inside.
					// Get transform table.
	Xform_palette xform = sman->get_xform(8);//++++Experiment.
	int scrolltx = gwin->get_scrolltx(),
	    scrollty = gwin->get_scrollty();
	Image_window8 *win = gwin->get_win();
	for (int i = 0; i < num_drops; i++)
		drops[i].paint(win, scrolltx, scrollty, xform);
	gwin->set_painted();
	}

/*
 *	End of lightning.
 */

Lightning_effect::~Lightning_effect
	(
	)
	{
	if (flashing)			// Be sure palette is restored.
		Game_window::get_instance()->get_clock()->set_palette();
	}

/*
 *	Lightning.
 */

void Lightning_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int r = rand();			// Get a random #.
	int delay = 100;		// Delay for next time.
	if (flashing)			// Just turned white?  Restore.
		{
		gclock->set_palette();
		flashing = false;
		active = false;
		if (curtime >= stop_time)
			{		// Time to stop.
			eman->remove_effect(this);
			return;
			}
		if (r%5 == 0)		// Occassionally flash again.
			delay = (1 + r%7)*40;
		else			// Otherwise, wait several secs.
			delay = (4000 + r%3000);
		}
	else if (!gwin->is_in_dungeon() && !active)// Time to flash.
		{
					// Play thunder.
		Audio::get_ptr()->play_sound_effect(Audio::game_sfx(62));
		active = true;
		flashing = true;
		gwin->get_pal()->set(PALETTE_LIGHTNING);
		delay = (1 + r%2)*25;
		}
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Start a storm.
 */

Storm_effect::Storm_effect
	(
	int duration,			// In game minutes.
	int delay,			// In msecs.
	Game_object *egg		// Egg that caused it, or null.
	) : Weather_effect(duration, delay, 2, egg), start(1)
	{
	Game_window *gwin = Game_window::get_instance();
					// Start raining soon.
	int rain_delay = 20 + rand()%1000;
	eman->add_effect(new Rain_effect(duration - 1, rain_delay));
	int lightning_delay = rain_delay + rand()%500;
	eman->add_effect(new Lightning_effect(duration - 1, lightning_delay));
	}

/*
 *	Start/end of storm.
 */
void Storm_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	if (start)
		{
		start = 0;
					// Darken sky.
		gwin->get_clock()->set_storm(true);
					// Nothing more to do but end.
		gwin->get_tqueue()->add(stop_time, this, udata);
		}
	else				// Must be time to stop.
		eman->remove_effect(this);
	}

/*
 *	Storm ended.
 */

Storm_effect::~Storm_effect
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
					// Restore palette.
	gwin->get_clock()->set_storm(false);
	}

/*
 *	Sparkles (in Ambrosia).
 */

void Sparkle_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_instance();
	if (!gwin->is_main_actor_inside())
		{			// Don't show rain inside buildings!
		Image_window8 *win = gwin->get_win();
		int w = win->get_width(), h = win->get_height();
					// Get transform table.
		Xform_palette xform = sman->get_xform(8);
		int scrolltx = gwin->get_scrolltx(),
		    scrollty = gwin->get_scrollty();
					// Move drops to random spots.
		for (int i = 0; i < num_drops; i++)
			drops[i].next_random(win, scrolltx, scrollty, 
								xform, w, h);
		gwin->set_painted();
		}
	if (curtime < stop_time)	// Keep going?
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		{
		gwin->set_all_dirty();
		eman->remove_effect(this);
		}
	}

/*
 *	Create a cloud.
 */

const int CLOUD = 2;		// Shape #.

Cloud::Cloud
	(
	short dx, short dy		// Deltas for movement.
	) : cloud(CLOUD, 0, SF_SPRITES_VGA), wx(0), wy(0), deltax(dx), deltay(dy), count(-1)
	{
	Game_window *gwin = Game_window::get_instance();
					// Get abs. values.
	int adx = deltax > 0 ? deltax : -deltax;
	int ady = deltay > 0 ? deltay : -deltay;
	if (adx < ady)
		max_count = 2*gwin->get_height()/ady;
	else
		max_count = 2*gwin->get_width()/adx;
	start_time = 0;
	}

/*
 *	Set starting screen position according to direction.
 */

void Cloud::set_start_pos
	(
	Shape_frame *shape,
	int w, int h,			// Dims. of window.
	int& x, int& y			// Screen pos. returned.
	)
	{
	if (!deltax)			// Special cases first.
		{
		x = rand()%w;
		y = deltay > 0 ? -shape->get_ybelow() : 
						h + shape->get_yabove();
		return;
		}
	if (!deltay)
		{
		y = rand()%h;
		x = deltax > 0 ? -shape->get_xright() : w + shape->get_xleft();
		return;
		}
	int halfp = w + h;		// 1/2 perimeter.
	int r = rand()%halfp;		// Start on one of two sides.
	if (r > h)			// Start on top/bottom.
		{
		x = r - h;
		y = deltay > 0 ? -shape->get_ybelow() : 
						h + shape->get_yabove();
		return;
		}
	y = r;				// On left or right side.
	if (deltax > 0)			// Going right?
		x = -shape->get_xright();
	else				// Going left?
		x = w + shape->get_xleft();
	}

/*
 *	Move cloud
 */

inline void Cloud::next
	(
	Game_window *gwin,		// Game window.
	unsigned long curtime,		// Current time of day.
	int w, int h			// Dims. of window.
	)
	{
	if (curtime < start_time)
		return;			// Not yet.
					// Get top-left world pos.
	long scrollx = gwin->get_scrolltx()*c_tilesize;
	long scrolly = gwin->get_scrollty()*c_tilesize;
	Shape_frame *shape = cloud.get_shape();
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(
			shape, wx - scrollx, wy - scrolly).enlarge(4)));
	if (count <= 0)			// Time to restart?
		{
					// Set start time randomly.
		start_time = curtime + 2000*randcnt + rand()%2000;
		randcnt = (randcnt + 1)%4;
		start_time = Game::get_ticks() + 2000*randcnt + rand()%500;
cout << "Cloud: start_time = " << start_time << endl;
		count = max_count;
		cloud.set_frame(rand()%cloud.get_num_frames());
		int x, y;		// Get screen pos.
		set_start_pos(shape, w, h, x, y);
		wx = x + scrollx;
		wy = y + scrolly;
		}
	else
		{
		wx += deltax;
		wy += deltay;
		count--;
		}
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(
			shape, wx - scrollx, wy - scrolly).enlarge(4)));
	}

/*
 *	Paint cloud.
 */

void Cloud::paint
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	if (count > 0)			// Might not have been started.
		cloud.paint_shape(wx - gwin->get_scrolltx()*c_tilesize, 
			wy - gwin->get_scrollty()*c_tilesize);
	}

/*
 *	Create a few clouds to float across the screen.
 */

Clouds_effect::Clouds_effect
	(
	int duration,			// In game minutes.
	int delay,			// In msecs.
	Game_object *egg		// Egg that caused it, or null.
	) : Weather_effect(duration, delay, 6, egg)
	{
	num_clouds = 2 + rand()%5;	// Pick #.
	clouds = new Cloud *[num_clouds];
					// Figure wind direction.
	int dx = rand()%5 - 2;
	int dy = rand()%5 - 2;
	if (!dx && !dy)
		{
		dx = 1 + rand()%2;
		dy = 1 - rand()%3;
		}
#if 0
	if (dx != 0)			// Move a bit faster.
		dx += dx > 0 ? 1 : -1;
	if (dy != 0)
		dy += dy > 0 ? 1 : -1;
#endif
	for (int i = 0; i < num_clouds; i++)
		{			// Modify speed of some.
		int deltax = dx, deltay = dy;
		if (rand()%2 == 0)
			{
			deltax += deltax/2;
			deltay += deltay/2;
			}
		clouds[i] = new Cloud(deltax, deltay);
		}
	}

/*
 *	Cloud drift.
 */

void Clouds_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 100;
	Game_window *gwin = Game_window::get_instance();
	if (curtime >= stop_time)
		{			// Time to stop.
		eman->remove_effect(this);
		gwin->set_all_dirty();
		return;
		}
	int w = gwin->get_width(), h = gwin->get_height();
	for (int i = 0; i < num_clouds; i++)
		clouds[i]->next(gwin, curtime, w, h);
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Clouds_effect::paint
	(
	)
	{
	if (!gwin->is_main_actor_inside() && !gwin->get_gump_man()->showing_gumps(true))
		for (int i = 0; i < num_clouds; i++)
			clouds[i]->paint();
	}

/*
 *	Shake the screen.
 */

void Earthquake::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	static int eqsoundonce;

	if(eqsoundonce != 1)
	{
		eqsoundonce = 1;
		// Play earthquake SFX once
  		Audio::get_ptr()->play_sound_effect(Audio::game_sfx(60));
	}

	Game_window *gwin = Game_window::get_instance();
	Image_window *win = gwin->get_win();
	int w = win->get_width(), h = win->get_height();
	int sx = 0, sy = 0;
	int dx = rand()%9 - 4;
	int dy = rand()%9 - 4;
	if (dx > 0)
		w -= dx;
	else
		{
		w += dx;
		sx -= dx;
		dx = 0;
		}
	if (dy > 0)
		h -= dy;
	else
		{
		h += dy;
		sy -= dy;
		dy = 0;
		}
	win->copy(sx, sy, w, h, dx, dy);
	gwin->set_painted();
	gwin->show();
					// Shake back.
	win->copy(dx, dy, w, h, sx, sy);
	if (++i < len)			// More to do?  Put back in queue.
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		{
		eqsoundonce = 0;	
		delete this;
		}

	}

