/*
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2003  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Audio.h"
#include "monsters.h"
#include "cheat.h"
#include "chunks.h"
#include "animate.h"
#include "effects.h"
#include "egg.h"
#include "exult.h"
#include "game.h"
#include "gameclk.h"
#include "gamewin.h"
#include "gamemap.h"
#include "npctime.h"
#include "paths.h"
#include "ucmachine.h"
#include "ucscriptop.h"
#include "ucsched.h"
#include "Gump_manager.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

using std::cout;
using std::endl;
using std::rand;
using std::ostream;

Egg_object *Egg_object::editing = 0;

/*
 *	Timer for a missile egg (type-6 egg).
 */
class Missile_launcher : public Time_sensitive, public Game_singletons
	{
	Egg_object *egg;		// Egg this came from.
	int weapon;			// Shape for weapon.
	int shapenum;			// Shape for missile.
	int dir;			// Direction (0-7).  (8==??).
	int delay;			// Delay (msecs) between launches.
public:
	Missile_launcher(Egg_object *e, int weap, int shnum, int di, int del)
		: egg(e), weapon(weap), shapenum(shnum), dir(di), delay(del)
		{  }
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Launch a missile.
 */

void Missile_launcher::handle_event
	(
	unsigned long curtime,
	long udata
	)
	{
	Tile_coord src = egg->get_tile();
					// Is egg off the screen?
	if (!gwin->get_win_tile_rect().has_point(src.tx, src.ty))
		return;			// Return w'out adding back to queue.
	Projectile_effect *proj = 0;
	if (dir < 8)			// Direction given?
		{			// Get adjacent tile in direction.
		Tile_coord adj = src.get_neighbor(dir%8);
					// Make it go 20 tiles.
		int dx = adj.tx - src.tx, dy = adj.ty - src.ty;
		Tile_coord dest = src;
		dest.tx += 20*dx;
		dest.ty += 20*dy;
		proj = new Projectile_effect(src, dest, shapenum, weapon);
		}
	else				// Target a party member.
		{
		Actor *party[9];
		int psize = gwin->get_party(party, 1);
		int cnt = psize;
		int n = rand()%psize;	// Pick one at random.
					// Find one we can hit.
		for (int i = n; !proj && cnt; cnt--, i = (i + 1)%psize)
			if (Fast_pathfinder_client::is_straight_path(src,
					party[i]->get_tile()))
				proj = new Projectile_effect(
					src, party[i], shapenum, weapon);
		}
	if (proj)
		eman->add_effect(proj);
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + (delay > 0 ? delay : 1), this, udata);
	}

/*
 *	Paint at given spot in world.
 */

void Egglike_game_object::paint
	(
	)
	{
	if(gwin->paint_eggs)
		Game_object::paint();
	}

/*
 *	Can this be clicked on?
 */

int Egglike_game_object::is_findable
	(
	)
	{
	return gwin->paint_eggs && Ireg_game_object::is_findable();
	}

/*
 *	Create an egg from IREG data.
 */

Egg_object::Egg_object
	(
	int shapenum, int framenum,
	unsigned int tilex, unsigned int tiley, 
	unsigned int lft, 
	unsigned short itype,
	unsigned char prob, 
	short d1, short d2
	) : Egglike_game_object(shapenum, framenum, tilex, tiley, lft),
	    probability(prob), data1(d1), data2(d2),
	    area(Rectangle(0, 0, 0, 0)), launcher(0)
	{
	type = itype&0xf;
					// Teleport destination?
	if (type == teleport && framenum == 6 && shapenum == 275)
		type = path;		// (Mountains N. of Vesper).
	criteria = (itype & (7<<4)) >> 4;
	distance = (itype >> 10) & 0x1f;
	unsigned char noct = (itype >> 7) & 1;
	unsigned char do_once = (itype >> 8) & 1;
					// Missile eggs can be rehatched
	unsigned char htch = (type == missile) ? 0 : ((itype >> 9) & 1);
	solid_area = (criteria == something_on || criteria == cached_in ||
					// Teleports need solid area.
						type == teleport) ? 1 : 0;
	unsigned char ar = (itype >> 15) & 1;
	flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
	if (type == usecode || type == teleport || type == path)
		set_quality(data1&0xff);
					// Party_near & auto_reset don't mix
					//   well.
	if (criteria == party_near && (flags&(1<<auto_reset)))
		criteria = avatar_near;
	}

/*
 *	Init. for a field.
 */

inline void Egg_object::init_field
	(
	unsigned char ty		// Egg (field) type.
	)
	{
	type = ty;
	probability = 100;
	data1 = data2 = 0;
	launcher = 0;
	area = Rectangle(0, 0, 0, 0);
	criteria = party_footpad;
	distance = 0;
	solid_area = 0;
	flags = (1 << auto_reset);
	}

/*
 *	Create an egg representing a field.
 */

Egg_object::Egg_object
	(
	int shapenum,
	int framenum,
	unsigned int tilex, unsigned int tiley, 
	unsigned int lft, 
	unsigned char ty		// Egg (field) type.
	) : Egglike_game_object(shapenum, framenum, tilex, tiley, lft)
	{
	init_field(ty);
	}

/*
 *	Destructor:
 */

Egg_object::~Egg_object
	(
	)
	{
	if (launcher)
		{
		gwin->get_tqueue()->remove(launcher);
		delete launcher;
		}
	}

/*
 *	Set active area after being added to its chunk.
 */

void Egg_object::set_area
	(
	)
	{
	if (!probability || type == path)// No chance of normal activation?
		{
		area = Rectangle(0, 0, 0, 0);
		return;
		}
	Tile_coord t = get_tile();	// Get absolute tile coords.
	switch (criteria)		// Set up active area.
		{
	case cached_in:			// Make it really large.
		area = Rectangle(t.tx - 32, t.ty - 32, 64, 64);
		break;
	case avatar_footpad:
	case party_footpad:
		{
		Shape_info& info = get_info();
		int xtiles = info.get_3d_xtiles(), 
		    ytiles = info.get_3d_ytiles();
		area = Rectangle(t.tx - xtiles + 1, t.ty - ytiles + 1,
							xtiles, ytiles);
		break;
		}
	case avatar_far:		// Make it 1 tile bigger each dir.
		area = Rectangle(t.tx - distance - 1, t.ty - distance - 1, 
					2*distance + 3, 2*distance + 3);
		break;
	default:
		{
		int width = 2*distance;
		width++;		// Added 8/1/01.
		if (distance <= 1)	// Small?
			{
					// More guesswork:
			if (criteria == external_criteria)
				width += 2;
			}
		area = Rectangle(t.tx - distance, t.ty - distance, 
					width, width);
		break;
		}
		}
					// Don't go outside the world.
	Rectangle world(0, 0, c_num_chunks*c_tiles_per_chunk,
						c_num_chunks*c_tiles_per_chunk);
	area = area.intersect(world);
	}

/*
 *	Is the egg active when stepping onto a given spot, or placing an obj.
 *	on the spot?
 */

int Egg_object::is_active
	(
	Game_object *obj,		// Object placed (or Actor).
	int tx, int ty, int tz,		// Tile stepped onto.
	int from_tx, int from_ty	// Tile stepped from.
	)
	{
	if ((flags & (1 << (int) hatched)) &&
			!(flags & (1 << (int) auto_reset)))
		return (0);		// For now... Already hatched.
	if (flags & (1 << (int) nocturnal))
		{			// Nocturnal.
		int hour = gclock->get_hour();
		if (!(hour >= 9 || hour <= 5))
			return (0);	// It's not night.
		}
	if (cheat.in_map_editor())
		return 0;		// Disable in map-editor.
	Egg_criteria cri = (Egg_criteria) get_criteria();

	int deltaz = tz - get_lift();
	int absdeltaz = deltaz < 0 ? -deltaz : deltaz;
	switch (cri)
		{
	case cached_in:			// Anywhere in square.
		{
		if (obj != gwin->get_main_actor() || !area.has_point(tx, ty))
			return 0;	// Not in square.
		if (!(flags & (1 << (int) hatched)))
			return 1;	// First time.
					// Must have autoreset.
					// Just activate when reentering.
		return !area.has_point(from_tx, from_ty);
		}
	case avatar_near:
		if (obj != gwin->get_main_actor())
			return 0;
		print_debug();
		// fall through
	case party_near:		// Avatar or party member.
		if (!obj->get_flag(Obj_flags::in_party))
			return 0;
		if (type == teleport)	// Teleports:  Any tile, exact lift.
			return absdeltaz == 0 && area.has_point(tx, ty);
		if (!((absdeltaz <= 1 || 
					// Using trial&error here:
			 (Game::get_game_type() == SERPENT_ISLE &&
						type != missile) ||
				(type == missile && tz/5 == get_lift()/5)) &&
					// New tile is in, old is out.
			area.has_point(tx, ty) &&
					!area.has_point(from_tx, from_ty)))
			return 0;
		return 1;
	case avatar_far:		// New tile is outside, old is inside.
		{
		if (obj != gwin->get_main_actor() || !area.has_point(tx, ty))
			return (0);
		Rectangle inside(area.x + 1, area.y + 1, 
						area.w - 2, area.h - 2);
		return inside.has_point(from_tx, from_ty) &&
			!inside.has_point(tx, ty);
		}
	case avatar_footpad:
		return obj == gwin->get_main_actor() && deltaz == 0 &&
						area.has_point(tx, ty);
	case party_footpad:
		return area.has_point(tx, ty) && deltaz == 0 &&
					obj->get_flag(Obj_flags::in_party);
	case something_on:
		return	 		// Guessing.  At SI end, deltaz == -1.
			deltaz >= -1 && deltaz <= 3 &&
			area.has_point(tx, ty) && !obj->as_actor();
	case external_criteria:
	default:
		return 0;
		}
	}

/*
 *	Paint at given spot in world.
 */

void Egg_object::paint
	(
	)
	{
	Egglike_game_object::paint();
					// Make sure launcher is active.
	if (launcher && !launcher->in_queue())
		gwin->get_tqueue()->add(0L, launcher, 0);
	}

/*
 *	Run usecode when double-clicked.
 */

void Egg_object::activate
	(
	int /* event */
	)
	{
	if (!edit())
		activate(0, 0);
	}

/*
 *	Edit in ExultStudio.
 *
 *	Output:	True if map-editing & ES is present.
 */

bool Egg_object::edit
	(
	)
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_tile();
		unsigned long addr = (unsigned long) this;
		if (Egg_object_out(client_socket, addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), 
			type, criteria, probability, distance,
			(flags>>nocturnal)&1, (flags>>once)&1, 
			(flags>>hatched)&1, (flags>>auto_reset)&1, 
			data1, data2) != -1)
			{
			cout << "Sent egg data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending egg data to ExultStudio" <<endl;
		return true;
		}
#endif
	return false;
	}


/*
 *	Message to update from ExultStudio.
 */

void Egg_object::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame;
	int type;
	int criteria;
	int probability;
	int distance;
	bool nocturnal, once, hatched, auto_reset;
	int data1, data2;
	if (!Egg_object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset,
		data1, data2))
		{
		cout << "Error decoding egg" << endl;
		return;
		}
	Egg_object *egg = (Egg_object *) addr;
	if (egg && egg != editing)
		{
		cout << "Egg from ExultStudio is not being edited" << endl;
		return;
		}
	editing = 0;
	if (!egg)			// Create a new one?
		{
		int x, y;
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, Exult_server::cancel);
			return;
			}
		if (shape == -1)
			shape = 275;	// FOR NOW.
					// Create.  Gets initialized below.
		egg = new Egg_object(shape, 0, 0, 0, 0, 0, 0, 0, 0);
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(egg, x, y, lift))
				break;
		if (lift == 12)
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, 
							Exult_server::cancel);
			delete egg;
			return;
			}
		if (client_socket >= 0)
			Exult_server::Send_data(client_socket, 
						Exult_server::user_responded);
		}
	egg->type = type;
	if (shape != -1)
		egg->set_shape(shape);
	if (frame == -1)
		switch (type)
			{		// (These aren't perfect.)
		case monster: frame = 0; break;
		case jukebox: frame = 2; break;
		case soundsfx:frame = 1; break;
		case voice:   frame = 3; break;
		case weather: frame = 4; break;
		case teleport:frame = 5; break;
		case path:    frame = 6; break;
        case missile:
            egg->set_shape(200); 
            if ((data2 & 0xFF) < 8)
                frame = 2 + ((data2 & 0xFF) / 2);
            else
                frame = 1;
            break;
		default:      frame = 7; break;
			}
	if (frame != -1)
		egg->set_frame(frame);
	gwin->add_dirty(egg);
	egg->criteria = criteria&7;
	egg->distance = distance&31;
	egg->probability = probability;
	egg->flags = ((nocturnal?1:0)<<Egg_object::nocturnal) +
		((once?1:0)<<Egg_object::once) +
		((hatched?1:0)<<Egg_object::hatched) +
		((auto_reset?1:0)<<Egg_object::auto_reset);
	egg->data1 = data1;
	egg->data2 = data2;
	if (type == usecode || type == teleport || type == path)
		egg->set_quality(data1&0xff);
	Map_chunk *chunk = 
			gmap->get_chunk_safely(egg->get_cx(), egg->get_cy());
	chunk->remove_egg(egg);		// Got to add it back.
	chunk->add_egg(egg);
	cout << "Egg updated" << endl;
#endif
	}

/*
 *	Create a monster nearby.
 */

static void Create_monster
	(
	Game_window *gwin,
	Egg_object *egg,
	int shnum,			// Monster shape.
	Monster_info *inf,		// Info.
	int sched,
	int align
	)
	{
	Tile_coord dest = Map_chunk::find_spot(
				egg->get_tile(), 5, shnum, 0, 1);
	if (dest.tx != -1)
		{
		Monster_actor *monster = Monster_actor::create(shnum, dest,
								sched, align);
		gwin->add_dirty(monster);
		gwin->add_nearby_npc(monster);
		}
	}

/*
 *	Handle a teleport egg.
 */

void Egg_object::activate_teleport
	(
	Game_object *obj		// Object (actor) that came near it.
	)
	{
	Tile_coord pos(-1, -1, -1);	// Get position to jump to.
	int qual = get_quality();
	if (qual == 255)
		{			// Jump to coords.
		int schunk = data1 >> 8;
		pos = Tile_coord(
			(schunk%12)*c_tiles_per_schunk + (data2&0xff), 
			(schunk/12)*c_tiles_per_schunk + (data2>>8), 0);
		}
	else
		{
		Egg_vector vec;		// Look for dest. egg (frame == 6).
		if (find_nearby_eggs(vec, 275, 256, qual, 6))
			{
			Egg_object *path = vec[0];
			pos = path->get_tile();
			}
		}
	cout << "Should teleport to (" << pos.tx << ", " <<
					pos.ty << ')' << endl;
	if (pos.tx != -1 && obj && obj->get_flag(Obj_flags::in_party))
					// Teleport everyone!!!
		gwin->teleport_party(pos);
#if 0	/* ++++I'm almost sure this is wrong. */
					// Can keep doing it.
	flags &= ~((1 << (int) hatched));
#endif
	}

/*
 *	Hatch egg.
 */

void Egg_object::activate
	(
	Game_object *obj,		// Object (actor) that came near it.
	bool must			// If 1, skip dice roll & execute
					//   usecode eggs immediately.
	)
	{
#ifdef DEBUG
	print_debug();
#endif
					// Flag it as done.
	flags |= (1 << (int) hatched);
/*	+++++ I just move the 'hatched' line here from below the 'return' in
an effort to fix the 'monsters spawning too often' bug.   NEED to see if this
breaks anything!  */

	int roll = must ? 0 : 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
	switch(type)
		{
	case jukebox:
#ifdef DEBUG
		cout << "Audio parameters might be: " << (data1&0xff) << " and " << ((data1>>8)&0x01) << endl;
#endif
		Audio::get_ptr()->start_music((data1)&0xff,(data1>>8)&0x01);
		break;
	case soundsfx:
		{
		int dir = 0;
		if (obj)		// Get direction from obj. to egg.
			{
			Tile_coord epos = get_tile(), opos = obj->get_tile();
			dir = Get_direction16(opos.ty - epos.ty, 
							epos.tx - opos.tx);
			}
		Audio::get_ptr()->play_sound_effect(
			Audio::game_sfx(data1&0xff), SDL_MIX_MAXVOLUME, dir,
								(data1>>8)&1);
		break;
		}
	case voice:
		ucmachine->do_speech(data1&0xff);
		break;
	case monster:			// Also creates other objects.
		{
		int shnum = data2&1023;
		int frnum = data2>>10;
		Monster_info *inf = 
				ShapeID::get_info(shnum).get_monster_info();
		if (inf)
			{		// Armageddon spell cast?
			if (gwin->armageddon)
				break;
			int sched = data1>>8;
			int align = data1&3;
			int cnt = (data1&0xff)>>2;
			if (cnt > 1)	// Randomize.
				cnt = 1 + (rand()%cnt);
			while (cnt--)
				Create_monster(gwin, this, shnum, inf,
							sched, align);
			}
		else			// Create item.
			{
			Shape_info& info = ShapeID::get_info(shnum);
			Game_object *nobj =gmap->create_ireg_object(info,
				shnum, frnum, get_tx(), get_ty(), get_lift());
			Map_chunk *chunk = 
					gmap->get_chunk(get_cx(), get_cy());
			if (nobj->is_egg())
				chunk->add_egg((Egg_object *) nobj);
			else
				chunk->add(nobj);
			gwin->add_dirty(nobj);
			nobj->set_flag(Obj_flags::okay_to_take);
					// Objects are created temporary
			nobj->set_flag(Obj_flags::is_temporary);
			}
		break;
		}
	case usecode:
		{			// Data2 is the usecode function.
		if (must)		// From script?  Do immediately.
			ucmachine->call_usecode(data2, this,
					Usecode_machine::egg_proximity);
		else			// Do on next animation frame.
			{
			Usecode_script *scr = new Usecode_script(this);
			scr->add(Ucscript::usecode, data2);
			if (flags & (1<<(int)once))
				{	// Don't remove until done.
				scr->add(Ucscript::remove);
				flags &= ~(1<<(int)once);
				}
			scr->start(gwin->get_std_delay());
			}
		break;
		}
	case missile:
		{
					// Get data.  Not sure about delay.
		int weapon = data1, dir = data2&0xff, delay = data2>>8;
		Shape_info& info = ShapeID::get_info(weapon);
		Weapon_info *winf = info.get_weapon_info();
		int proj;
		if (winf && winf->get_projectile())
			proj = winf->get_projectile();
		else
			proj = 856;	// Fireball.  Shouldn't get here.
		if (!launcher)
			launcher = new Missile_launcher(this, weapon,
			    proj, dir, gwin->get_std_delay()*delay);
		if (!launcher->in_queue())
			gwin->get_tqueue()->add(0L, launcher, 0);
		break;
		}
	case teleport:
		activate_teleport(obj);
		break;
	case weather:
		{
		set_weather(data1&0xff, data1>>8, this);
		break;
		}
	case button:		// Set off all in given area.
		{
		int dist = data1&0xff;
		Egg_vector eggs;
		find_nearby_eggs(eggs, 275, dist);
		for (Egg_vector::const_iterator it = eggs.begin();
					it != eggs.end(); ++it)
			{
			Egg_object *egg = *it;
			if (egg != this &&
			    egg->criteria == external_criteria && 
			    !(egg->flags & (1 << (int) hatched))) // Experimental attempting to fix problem in Silver Seed
				egg->activate(obj, 0);
			}
		break;
		}
	default:
		cout << "Egg not actioned" << endl;
		}
	if (flags & (1 << (int) once))
		remove_this(0);
	}

/*
 *	Print debug information.
 */

void Egg_object::print_debug
	(
	)
	{
	cout << "Egg type is " << (int) type << ", prob = " << 
		(int) probability <<
		", distance = " << (int) distance << ", crit = " <<
		(int) criteria << ", once = " <<
	((flags & (1<<(int)once)) != 0) << ", hatched = " <<
	((flags & (1<<(int)hatched)) != 0) <<
	", areset = " <<
	((flags & (1<<(int)auto_reset)) != 0) << ", data1 = " << data1
		<< ", data2 = " << data2 << endl;
	}

/*
 *	Set the weather (static).
 */

void Egg_object::set_weather
	(
	int weather,			// 0-6.
	int len,			// In game minutes (I think).
	Game_object *egg		// Egg this came from, or null.
	)
	{
	if (!len)			// Means continuous.
		len = 120;		// How about a couple game hours?
	int cur = eman->get_weather();
	cout << "Current weather is " << cur << "; setting " << weather
							<< endl;
	switch (weather)
		{
	case 0:		// Back to normal.
		eman->remove_weather_effects();
		break;
	case 2:		// Storm.
		if (cur != weather)
			eman->add_effect(new Storm_effect(len, 0, egg));
		break;
	case 3:		// (On Ambrosia).
		eman->remove_weather_effects();
		eman->add_effect(new Sparkle_effect(len, 0, egg));
		break;
	case 6:		// Clouds.
		eman->add_effect(new Clouds_effect(len, 0, egg));
		break;
	default:
		break;
		}
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (cx=cy=255).
 */

void Egg_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
					// Figure new chunk.
	int newcx = newtx/c_tiles_per_chunk, newcy = newty/c_tiles_per_chunk;
	Map_chunk *newchunk = gmap->get_chunk_safely(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
	remove_this(1);			// Remove from old.
	set_lift(newlift);		// Set new values.
	shape_pos = ((newtx%c_tiles_per_chunk) << 4) + newty%c_tiles_per_chunk;
	newchunk->add_egg(this);	// Updates cx, cy.
	gwin->add_dirty(this);		// And repaint new area.
	}

/*
 *	This is needed since it calls remove_egg().
 */

void Egg_object::remove_this
         (
         int nodel                       // 1 to not delete.
         )
	{
	if (get_owner())		// Watch for this.
		get_owner()->remove(this);
	else
		{
		Map_chunk *chunk = gmap->get_chunk_safely(cx, cy);
	 	if (chunk)
			{
			gwin->add_dirty(this);	// (Make's ::move() simpler.).
			chunk->remove_egg(this);
			}
		}
	if (launcher)			// Stop missiles.
		{
		gwin->get_tqueue()->remove(launcher);
		delete launcher;
		launcher = 0;
		}
	 if (!nodel)
		 gwin->delete_object(this);
	 }

/*
 *	Write out.
 */

void Egg_object::write_ireg
	(
	DataSource *out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	uint8 *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
	unsigned short tword = type&0xf;// Set up 'type' word.
	tword |= ((criteria&7)<<4);
	tword |= (((flags>>nocturnal)&1)<<7);
	tword |= (((flags>>once)&1)<<8);
	tword |= (((flags>>hatched)&1)<<9);
	tword |= ((distance&0x1f)<<10);
	tword |= (((flags>>auto_reset)&1)<<15);
	Write2(ptr, tword);
	*ptr++ = probability;
	Write2(ptr, data1);
	*ptr++ = (get_lift()&15)<<4;
	Write2(ptr, data2);
	out->write((char*)buf, sizeof(buf));
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
	}

// Get size of IREG. Returns -1 if can't write to buffer
int Egg_object::get_ireg_size()
{
	// These shouldn't ever happen, but you never know
	if (gumpman->find_gump(this) || Usecode_script::find(this))
		return -1;

	return 13;
}

/*
 *	Create from IREG data.
 */
Animated_egg_object::Animated_egg_object
	(
	int shapenum, int framenum,
	unsigned int tilex,
	unsigned int tiley, unsigned int lft, 
	unsigned short itype,
	unsigned char prob, short d1, short d2
	) : Egg_object(shapenum, framenum, 
				tilex, tiley, lft, itype, prob, d1, d2)
	{ 
	animator = new Frame_animator(this); 
	}

/*
 *	Create for fields.
 */

Animated_egg_object::Animated_egg_object
	(
	int shapenum, int framenum, unsigned int tilex, 
	unsigned int tiley, unsigned int lft,
	unsigned char ty
	) : Egg_object(shapenum, framenum, tilex, tiley, lft, ty)
	{
	int recycle = 0;		// Frame to begin new cycles after 1st.
	switch (type)
		{
	case poison_field:
		recycle = 6; break;
	case sleep_field:
		recycle = 1; break;
	case fire_field:
		recycle = 8; break;
	case caltrops_field:
		animator = 0; return;	// This doesn't get animated.
		}
	animator = new Field_frame_animator(this, recycle); 
	}

/*
 *	Delete.
 */
Animated_egg_object::~Animated_egg_object()
	{ 
	delete animator; 
	}

/*
 *	Render.
 */

void Animated_egg_object::paint
	(
	)
	{
	if (animator)
		animator->want_animation();	// Be sure animation is on.
	Ireg_game_object::paint();	// Always paint these.
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Animated_egg_object::activate
	(
	int event
	)
	{
	Egg_object::activate(event);
	flags &= ~(1 << (int) hatched);	// Moongate:  reset always.
	}

/*
 *	Stop animation.
 */

void Animated_egg_object::stop_animation
	(
	)
	{
	delete animator;
	animator = 0;
	}

/*
 *	Apply field.
 *
 *	Output:	True to delete field.
 */

bool Field_object::field_effect
	(
	Actor *actor
	)
	{
	bool del = false;		// Only delete poison, sleep fields.
	switch (type)
		{
	case poison_field:
		if (rand()%2)
			{
			actor->set_flag(Obj_flags::poisoned);
			del = true;
			}
		break;
	case sleep_field:
		if (rand()%2)
			{
			actor->set_flag(Obj_flags::asleep);
			del = true;
			}
		break;
	case fire_field:
					// Blue fire (serpent isle)?
		if (get_shapenum() == 561)
			{
			actor->reduce_health(5 + rand()%4);
			}
		else if (rand()%2)
			{
			actor->reduce_health(1);
			}
					// But no sleeping here.
		actor->clear_flag(Obj_flags::asleep);
		break;
	case caltrops_field:
		if (actor->get_property(Actor::intelligence)*
		    (actor->get_flag(Obj_flags::might) ? 2 : 1) < rand()%40)
			{
			actor->reduce_health(2 + rand()%3);
			}
		return false;
		}
	if (!del)			// Tell animator to keep checking.
		((Field_frame_animator *) animator)->activated = true;
	return del;
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 *	(Generally, nothing will happen.)
 */

void Field_object::activate
	(
	int event
	)
	{
					// Field_frame_animator calls us with
					//   event==0 to check for damage.
	if (event != Usecode_machine::npc_proximity)
		{
		Ireg_game_object::activate(event);
		return;
		}
	Actor_queue npcs;		// Find all nearby NPC's.
	gwin->get_nearby_npcs(npcs);
	npcs.push(gwin->get_main_actor());	// Include Avatar.
	Rectangle eggfoot = get_footprint();
					// Clear flag to check.
	((Field_frame_animator *) animator)->activated = false;
	for (Actor_queue::const_iterator it = npcs.begin(); 
						it != npcs.end(); ++it)
		{
		Actor *actor = *it;
		if (actor->is_dead() || Game_object::distance(actor) > 4)
			continue;
		if (actor->get_footprint().intersects(eggfoot))
			Field_object::activate(actor);
		}
	}

/*
 *	Someone stepped on it.
 */

void Field_object::activate
	(
	Game_object *obj,		// Object (actor) that came near it.
	bool /* must */			// If 1, skip dice roll.
	)
	{
	if (field_effect((Actor *) obj))// Apply field.
		remove_this(0);		// Delete sleep/poison if applied.
	}

/*
 *	Write out.  These are stored as normal game objects.
 */

void Field_object::write_ireg
	(
	DataSource *out
	)
	{
	Ireg_game_object::write_ireg(out);
	}

// Get size of IREG. Returns -1 if can't write to buffer
int Field_object::get_ireg_size()
{
	return Ireg_game_object::get_ireg_size();
}


/*
 *	It's a Mirror
 */

Mirror_object::Mirror_object(int shapenum, int framenum, unsigned int tilex, 
		     unsigned int tiley, unsigned int lft) :
	Egg_object(shapenum, framenum, tilex, tiley, lft, Egg_object::mirror_object)
{
	solid_area = 1;
}

void Mirror_object::activate(int event)
{
	Ireg_game_object::activate(event);
}

void Mirror_object::activate(Game_object *obj, bool must)
{
	// These are broken, so dont touch
	if ((get_framenum()%3) == 2)  return;

	int wanted_frame = get_framenum()/3;
	wanted_frame *= 3;

	// Find upperleft or our area
	Tile_coord t = get_tile();

	// To left or above?
	if (get_shapenum()==268)	// Left
	{
		t.tx++;
		t.ty--;
	}
	else				// Above
	{
		t.tx--;
		t.ty++;
	}

	// We just want to know if the area is blocked
	int nl = 0;
	if (Map_chunk::is_blocked(1, t.tz, t.tx, t.ty, 2, 2, nl, MOVE_WALK, 0))
	{
		wanted_frame++;
	}

	// Only if it changed update the shape
	if (get_framenum()!=wanted_frame)
		change_frame(wanted_frame);
}

// Can it be activated?
int Mirror_object::is_active(Game_object *obj, int tx, int ty, int tz, int from_tx, int from_ty)
{
	// These are broken, so dont touch
	int frnum = get_framenum();
	if (frnum%3 == 2)  return 0;
	if (frnum >= 3 && GAME_BG)	// Demon mirror in FOV.
		return 0;

	return 1;
}

// Set up active area.
void Mirror_object::set_area()
{
	// These are broken, so dont touch
	if ((get_framenum()%3) == 2) area = Rectangle(0, 0, 0, 0);

	Tile_coord t = get_tile();	// Get absolute tile coords.

	// To left or above?
	if (get_shapenum()==268) area = Rectangle(t.tx-1, t.ty-3, 6, 6);
	else  area = Rectangle(t.tx-3 , t.ty-1, 6, 6);
}

void Mirror_object::paint()
{
	Ireg_game_object::paint();	// Always paint these.
}

/*
 *	Write out.  These are stored as normal game objects.
 */

void Mirror_object::write_ireg(DataSource *out)
{
	Ireg_game_object::write_ireg(out);
}

// Get size of IREG. Returns -1 if can't write to buffer
int Mirror_object::get_ireg_size()
{
	// TODO!!!!!!!
	return Ireg_game_object::get_ireg_size();
}
