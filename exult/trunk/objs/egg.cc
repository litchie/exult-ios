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
#include "databuf.h"
#include "weaponinf.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::cerr;
using std::endl;
using std::rand;
using std::ostream;
using std::string;
#endif

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
		proj = new Projectile_effect(egg, dest, weapon, shapenum, shapenum);
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
					src, party[i], weapon, shapenum, shapenum);
		}
	if (proj)
		eman->add_effect(proj);
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + (delay > 0 ? delay : 1), this,udata);
	}

/*
 *	Each egg type:
 */
class Jukebox_egg : public Egg_object {
protected:
	unsigned char score;
	bool continuous;
public:
	Jukebox_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, 0),
		  score(d1&0xff), continuous(((d1>>8)&1) != 0)
		{  }
	virtual void hatch_now(Game_object *obj, bool must) {
#ifdef DEBUG
		cout << "Audio parameters might be: " << (data1&0xff) << 
				" and " << ((data1>>8)&0x01) << endl;
#endif
		Audio::get_ptr()->start_music(score, continuous);
	}
};

class Soundsfx_egg : public Jukebox_egg {
public:
	Soundsfx_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1)
		: Jukebox_egg(shnum, frnum, tx, ty, tz, itype, prob, d1)
		{  }
	virtual void hatch_now(Game_object *obj, bool must) {
		int dir = 0;
		if (obj)		// Get direction from obj. to egg.
			{
			Tile_coord epos = get_tile(), opos = obj->get_tile();
			dir = Get_direction16(opos.ty - epos.ty, 
							epos.tx - opos.tx);
			}
		Audio::get_ptr()->play_sound_effect(score, this, AUDIO_MAX_VOLUME, 
							continuous);
	}
};

class Voice_egg : public Egg_object {
	unsigned short speechnum;
public:
	Voice_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, 0),
		  speechnum(d1)
		{  }
	virtual void hatch_now(Game_object *obj, bool must) {
		ucmachine->do_speech(data1&0xff);
	}
};

class Monster_egg : public Egg_object {
	unsigned short mshape;		// For monster.
	unsigned char mframe;
	unsigned char sched, align, cnt;
	void create_monster(Monster_info *inf) {
		Tile_coord dest = Map_chunk::find_spot(
				get_tile(), 5, mshape, 0, 1);
		if (dest.tx != -1) {
			Monster_actor *monster = 
			    Monster_actor::create(mshape, dest, sched, align);
			monster->change_frame(mframe);
			gwin->add_dirty(monster);
			gwin->add_nearby_npc(monster);
		}
	}
public:
	Monster_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2, uint16 d3)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1,d2,d3),
		  sched(d1>>8), align(d1&3), cnt((d1&0xff)>>2) {

		if (d3 > 0) {		// Exult extension.
			mshape = d3;
			mframe = d2&0xff;
		} else {
			mshape = d2&1023;
			mframe = d2>>10;
		}
	}
	virtual void hatch_now(Game_object *obj, bool must) {
		Monster_info *inf = 
				ShapeID::get_info(mshape).get_monster_info();
		if (inf) {
			if (gwin->armageddon)
				return;
			int num = cnt;
			if (num > 1)	// Randomize.
				num = 1 + (rand()%num);
			while (num--)
				create_monster(inf);
		} else {		// Create item.
			Shape_info& info = ShapeID::get_info(mshape);
			Game_object *nobj = get_map()->create_ireg_object(info,
			    mshape, mframe, get_tx(), get_ty(), get_lift());
			if (nobj->is_egg())
				chunk->add_egg((Egg_object *) nobj);
			else
				chunk->add(nobj);
			gwin->add_dirty(nobj);
			nobj->set_flag(Obj_flags::okay_to_take);
					// Objects are created temporary
			nobj->set_flag(Obj_flags::is_temporary);
		}
	}
};

class Usecode_egg : public Egg_object {
	short fun;
	string fun_name;		// Actual name in usecode source.
public:
	Usecode_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2, const char *fnm)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2),
		  fun(d2) {  
		set_quality(d1&0xff);
		Usecode_egg::set_str1(fnm);
	}
	virtual void set_str1(const char *s) {
		fun_name = s ? s : "";
		if (s && *s)
			fun = 0;	// Want to look this up.
	}
	virtual const char *get_str1()
		{ return fun_name.c_str(); }
	virtual void hatch_now(Game_object *obj, bool must) {
		if (!fun && !fun_name.empty())
			fun = ucmachine->find_function(fun_name.c_str());
		if (must)		// From script?  Do immediately.
			ucmachine->call_usecode(fun, this,
					Usecode_machine::egg_proximity);
		else {			// Do on next animation frame.
			Usecode_script *scr = new Usecode_script(this);
			scr->add(Ucscript::usecode, fun);
			if (flags & (1<<(int)once)) {
					// Don't remove until done.
				scr->add(Ucscript::remove);
				flags &= ~(1<<(int)once);
			}
			scr->start(gwin->get_std_delay());
		}
	}
};

class Missile_egg : public Egg_object {
	short weapon;
	unsigned char dir, delay;
	Missile_launcher *launcher;
public:
	Missile_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2),
		  weapon(d1), dir(d2&0xff), delay(d2>>8), launcher(0)
		{  }
	virtual ~Missile_egg() {
		if (launcher) {
			gwin->get_tqueue()->remove(launcher);
			delete launcher;
		}
	}
	virtual void remove_this(int nodel) {
		if (launcher) {		// Stop missiles.
			gwin->get_tqueue()->remove(launcher);
			delete launcher;
			launcher = 0;
		}
		Egg_object::remove_this(nodel);
	}
	virtual void paint() {
					// Make sure launcher is active.
		if (launcher && !launcher->in_queue())
			gwin->get_tqueue()->add(0L, launcher, 0);
		Egg_object::paint();
	}
	virtual void set(int crit, int dist) {
		if (crit == external_criteria && launcher) {	// Cancel trap.
			gwin->get_tqueue()->remove(launcher);
			delete launcher;
			launcher = 0;
		}
		Egg_object::set(crit, dist);
	}
	virtual void hatch_now(Game_object *obj, bool must) {
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
	}
};


class Teleport_egg : public Egg_object {
	short mapnum;			// If not -1.
	unsigned short destx, desty, destz;
public:
	Teleport_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2, uint16 d3)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2,d3),
		  mapnum(-1) {
		if (type == intermap)
			mapnum = d1&0xff;
		else
			set_quality(d1&0xff);	// Teleport egg.
		int schunk = d1 >> 8;
		destx = (schunk%12)*c_tiles_per_schunk + (d2&0xff);
		desty = (schunk/12)*c_tiles_per_schunk + (d2>>8);
		destz = d3&0xff;
	}
	virtual void hatch_now(Game_object *obj, bool must) {
		Tile_coord pos(-1, -1, -1);	// Get position to jump to.
		int eggnum = 255;
		if (mapnum == -1)
 			eggnum = get_quality();
		if (eggnum == 255) {		// Jump to coords.
			pos = Tile_coord(destx, desty, destz);
		} else {
			Egg_vector vec;	// Look for dest. egg (frame == 6).
			if (find_nearby_eggs(vec, 275, 256, eggnum, 6)) {
				Egg_object *path = vec[0];
				pos = path->get_tile();
			}
		}
		cout << "Should teleport to map " << mapnum << 
				", (" << pos.tx << ", " <<
					pos.ty << ')' << endl;
		if (pos.tx != -1 && obj && obj->get_flag(Obj_flags::in_party))
					// Teleport everyone!!!
			gwin->teleport_party(pos, false, mapnum, false);
	}
};

class Weather_egg : public Egg_object {
	unsigned char weather;		// 0-6
	unsigned char len;		// In game minutes.
public:
	Weather_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2),
		  weather(d1&0xff), len(d1>>8) {
		if (!len)		// Means continuous.
			len = 120;	// How about a couple game hours?
	}
	virtual void hatch_now(Game_object *obj, bool must) {
		set_weather(weather, len, this);
	}
};

class Button_egg : public Egg_object {
	unsigned char dist;
public:
	Button_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2),
		  dist(d1&0xff)
		{  }
	virtual void hatch_now(Game_object *obj, bool must) {
		Egg_vector eggs;
		find_nearby_eggs(eggs, 275, dist);
		for (Egg_vector::const_iterator it = eggs.begin();
						it != eggs.end(); ++it) {
			Egg_object *egg = *it;
			if (egg != this &&
			    egg->criteria == external_criteria && 
				// Attempting to fix problem in Silver Seed
			    !(egg->flags & (1 << (int) hatched))) 
				egg->hatch(obj, 0);
		}
	}
};

class Path_egg : public Egg_object {
public:
	Path_egg(int shnum, int frnum, unsigned int tx, unsigned int ty,
		unsigned int tz, unsigned short itype,
		unsigned char prob, uint16 d1, uint16 d2)
		: Egg_object(shnum, frnum, tx, ty, tz, itype, prob, d1, d2) {
		set_quality(d1&0xff);
	}
};

/*
 *	Create an "egg" from Ireg data.
 */

Egg_object *Egg_object::create_egg
	(
	unsigned char *entry,		// 12+ byte ireg entry.
	int entlen,
	bool animated,
	int shnum,
	int frnum,
	unsigned int tx,
	unsigned int ty,
	unsigned int tz
	)
{
	unsigned short type = entry[4] + 256*entry[5];
	int prob = entry[6];		// Probability (1-100).
	int data1 = entry[7] + 256*entry[8];
	int data2 = entry[10] + 256*entry[11];
	int data3 = entlen >= 14 ? (entry[12] + 256*entry[13]) : 0;
	return create_egg(animated, shnum, frnum, tx, ty, tz, type, prob, 
						data1, data2, data3);
}

Egg_object *Egg_object::create_egg
	(
	bool animated,
	int shnum, int frnum,
	unsigned int tx, unsigned int ty, unsigned int tz,
	unsigned short itype,		// Type + flags, etc.
	unsigned char prob, 
	short data1, short data2, short data3,
	const char *str1
	)
{
	int type = itype&0xf;
					// Teleport destination?
	if (type == teleport && frnum == 6 && shnum == 275)
		type = path;		// (Mountains N. of Vesper).

	Egg_object *obj = 0;
	switch (type) {		// The type:
	case monster:
		obj = new Monster_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2, data3);
		break;
	case jukebox:
		obj = new Jukebox_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1);
		break;
	case soundsfx:
		obj = new Soundsfx_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1);
		break;
	case voice:
		obj = new Voice_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1);
		break;
	case usecode:
		obj = new Usecode_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2, str1);
		break;
	case missile:
		obj = new Missile_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2);
		break;
	case teleport:
	case intermap:
		obj = new Teleport_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2, data3);
		break;
	case weather:
		obj = new Weather_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2);
		break;
	case path:
		obj = new Path_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2);
		break;
	case button:
		obj = new Button_egg(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2);
		break;
	default:
		cerr << "Illegal egg itype:  " << type << endl;
		obj = new Egg_object(shnum, frnum, tx, ty, tz, itype, prob,
						data1, data2, data3);
	}
	if (animated)
		obj->set_animator(new Frame_animator(obj));
	return (obj);
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
	short d1, short d2, short d3
	) : Egglike_game_object(shapenum, framenum, tilex, tiley, lft),
	    probability(prob), data1(d1), data2(d2), data3(d3),
	    area(Rectangle(0, 0, 0, 0)), animator(0)
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
		type == teleport || type == intermap) ? 1 : 0;
	unsigned char ar = (itype >> 15) & 1;
	flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
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
	area = Rectangle(0, 0, 0, 0);
	criteria = party_footpad;
	distance = 0;
	solid_area = 0;
	animator = 0;
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
	delete animator; 
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
		int frame = get_framenum();
		int xtiles = info.get_3d_xtiles(frame), 
		    ytiles = info.get_3d_ytiles(frame);
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
 *	Can this be clicked on?
 */
int Egg_object::is_findable()
{
	if (animator)
		return Ireg_game_object::is_findable();
	else
		return Egglike_game_object::is_findable();
}

/*
 *	Change the criteria and distance. 
 */

void Egg_object::set
	(
	int crit, 
	int dist
	)
	{ 
	Map_chunk *echunk = get_chunk();
	echunk->remove_egg(this);	// Got to add it back.
	criteria = crit;
	distance = dist; 
	echunk->add_egg(this);
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
	if (cheat.in_map_editor())
		return 0;		// Disable in map-editor.
	if ((flags & (1 << (int) hatched)) &&
			!(flags & (1 << (int) auto_reset)))
		return (0);		// For now... Already hatched.
	if (flags & (1 << (int) nocturnal))
		{			// Nocturnal.
		int hour = gclock->get_hour();
		if (!(hour >= 21 || hour <= 4))
			return (0);	// It's not night.
		}
	Egg_criteria cri = (Egg_criteria) get_criteria();

	int deltaz = tz - get_lift();
	switch (cri)
		{
	case cached_in:			// Anywhere in square.
		{
		// This seems to be true for SI in general. It has the side effect
		// of "fixing" Fawn Tower goblins.
		// It does NOT happen in BG, though.
		if (GAME_SI && deltaz/5 != 0 && type == monster)
			{
			// Mark hatched if not auto-reset.
			if (!(flags & (1 << (int) auto_reset)))
				flags |= (1 << (int) hatched);
			return 0;
			}
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
#ifdef DEBUG
		print_debug();
#endif
		// fall through
	case party_near:		// Avatar or party member.
		if (!obj->get_flag(Obj_flags::in_party))
			return 0;
		if (type == teleport ||	// Teleports:  Any tile, exact lift.
		    type == intermap)
			return deltaz == 0 && area.has_point(tx, ty);
		else if (type == jukebox || type == soundsfx || type == voice)
			// Guessing. Fixes shrine of Spirituality and Sacrifice.
			return area.has_point(tx, ty);
		if (!((deltaz/2 == 0 || 
					// Using trial&error here:
			 (Game::get_game_type() == SERPENT_ISLE &&
						type != missile) ||
				(type == missile && deltaz/5 == 0)) &&
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
			deltaz/4 == 0 && area.has_point(tx, ty) && !obj->as_actor();
	case external_criteria:
	default:
		return 0;
		}
	}

/*
 *	Animate.
 */
void Egg_object::set_animator(Animator *a)
{
	delete animator;
	animator = a;
}

/*
 *	Stop animation.
 */

void Egg_object::stop_animation
	(
	)
	{
	if (animator)
		animator->stop_animation();
	}

/*
 *	Paint at given spot in world.
 */

void Egg_object::paint
	(
	)
	{
	if (animator) {
		animator->want_animation();	// Be sure animation is on.
		Ireg_game_object::paint();	// Always paint these.
	} else
		Egglike_game_object::paint();
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
		hatch(0, 0);
	if (animator)
		flags &= ~(1 << (int) hatched);	// Moongate:  reset always.
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
		// Usecode function name.
		string str1 = get_str1();
		if (Egg_object_out(client_socket, addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), 
			type, criteria, probability, distance,
			(flags>>nocturnal)&1, (flags>>once)&1, 
			(flags>>hatched)&1, (flags>>auto_reset)&1, 
			data1, data2, data3, str1) != -1)
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
	int x, y;			// Mouse click for new egg.
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame;
	int type;
	int criteria;
	int probability;
	int distance;
	bool nocturnal, once, hatched, auto_reset;
	int data1, data2, data3;
	string str1;
	if (!Egg_object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset,
		data1, data2, data3, str1))
		{
		cout << "Error decoding egg" << endl;
		return;
		}
	Egg_object *oldegg = (Egg_object *) addr;
	if (oldegg && oldegg != editing)
		{
		cout << "Egg from ExultStudio is not being edited" << endl;
		return;
		}
	editing = 0;
	if (!oldegg)			// Creating a new one?  Get loc.
		{
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, 
						Exult_server::cancel);
			return;
			}
		if (shape == -1)
			shape = 275;	// FOR NOW.
		}
	else if (shape == -1)
		shape = oldegg->get_shapenum();
	if (frame == -1)
		switch (type)
			{		// (These aren't perfect.)
		case monster: frame = 0; break;
		case jukebox: frame = 2; break;
		case soundsfx:frame = 1; break;
		case voice:   frame = 3; break;
		case weather: frame = 4; break;
		case intermap:
		case teleport:frame = 5; break;
		case path:    frame = 6; break;
	        case missile:
        		shape = 200; 
            		if ((data2 & 0xFF) < 8)
                		frame = 2 + ((data2 & 0xFF) / 2);
            		else
                		frame = 1;
            		break;
		default:      frame = 7; break;
			}
	
	Shape_info& info = ShapeID::get_info(shape);
	bool anim = info.is_animated() || info.has_sfx();
	Egg_object *egg = create_egg(anim, shape, frame, tx, ty, tz, type,
			probability, data1, data2, data3, str1.c_str());
	if (!oldegg)
		{
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(egg, x, y, lift) == 1)
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
	else
		{
		Tile_coord pos = oldegg->get_tile();
		egg->move(pos.tx, pos.ty, pos.tz);
		}
	gwin->add_dirty(egg);
	egg->criteria = criteria&7;
	egg->distance = distance&31;
	egg->probability = probability;
	egg->flags = ((nocturnal?1:0)<<Egg_object::nocturnal) +
		((once?1:0)<<Egg_object::once) +
		((hatched?1:0)<<Egg_object::hatched) +
		((auto_reset?1:0)<<Egg_object::auto_reset);

	if (oldegg)
		oldegg->remove_this();
	Map_chunk *echunk = egg->get_chunk();
	echunk->remove_egg(egg);	// Got to add it back.
	echunk->add_egg(egg);
	cout << "Egg updated" << endl;
#endif
	}

/*
 *	Hatch egg.
 */

void Egg_object::hatch
	(
	Game_object *obj,		// Object (actor) that came near it.
	bool must			// If 1, skip dice roll & execute
					//   usecode eggs immediately.
	)
	{
#ifdef DEBUG
	print_debug();
#endif
	/*
	  MAJOR HACK!
	  This is an attempt at a work-around of a potential bug in the original
	  Serpent Isle. See SourceForge bug #879253

	  Prevent the Serpent Staff egg from hatching only once
	*/
	Tile_coord eggpos = get_tile();
	if (GAME_SI && eggpos.tx == 1287 && eggpos.ty == 2568 && eggpos.tz == 0) {
		flags &= ~(1 << (int) hatched);
	}
	/* end hack */


	int roll = must ? 0 : 1 + rand()%100;
	if (roll <= probability)
		{
		// Time to hatch the egg.
		hatch_now(obj, must);
		if (flags & (1 << (int) once))
			{
			remove_this(0);
			return;
			}
		}
		// Flag it as done, whether or not it has been hatched.
	flags |= (1 << (int) hatched);
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
		<< ", data2 = " << data2 
		<< ", data3 = " << data3 << endl;
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
		len = 6000;		// Confirmed from originals.
	int cur = eman->get_weather();
	cout << "Current weather is " << cur << "; setting " << weather
							<< endl;
	// Experimenting.
	if (weather != 4 && (weather == 3 || cur != weather))
		eman->remove_weather_effects();

	switch (weather)
		{
	case 0:		// Back to normal.
		eman->remove_weather_effects();
		break;
	case 1:		// Snow.
		eman->add_effect(new Snowstorm_effect(len, 0, egg));
		break;
	case 2:		// Storm.
		eman->add_effect(new Storm_effect(len, 0, egg));
		break;
	case 3:		// (On Ambrosia).
		eman->remove_weather_effects();
		eman->add_effect(new Sparkle_effect(len, 0, egg));
		break;
	case 4:		// Fog.
		// ++++ Disabling this.
		//eman->add_effect(new Fog_effect(len, 0, egg));
		break;
	case 5:		// Overcast.
	case 6:		// Clouds.
		eman->add_effect(new Clouds_effect(len, 0, egg, weather));
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
	int newlift,
	int newmap
	)
	{
					// Figure new chunk.
	int newcx = newtx/c_tiles_per_chunk, newcy = newty/c_tiles_per_chunk;
	Game_map *eggmap = newmap >= 0 ? gwin->get_map(newmap) : get_map();
	if (!eggmap) eggmap = gmap;
	Map_chunk *newchunk = eggmap->get_chunk_safely(newcx, newcy);
	if (!newchunk)
		return;			// Bad loc.
	remove_this(1);			// Remove from old.
	set_lift(newlift);		// Set new values.
	set_shape_pos(newtx%c_tiles_per_chunk, newty%c_tiles_per_chunk);
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
	 	if (chunk)
			{
			gwin->add_dirty(this);	// (Make's ::move() simpler.).
			chunk->remove_egg(this);
			}
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
	unsigned char buf[30];		// 12-14 byte entry.
	int sz = data3 > 0 ? 14 : 12;
	uint8 *ptr = write_common_ireg(sz, buf);
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
	if (data3 > 0)
		Write2(ptr, data3);
	out->write((char*)buf, ptr - buf);
	const char *str1 = get_str1();
	if (*str1)			// This will be usecode fun. name.
		Game_map::write_string(out, str1);
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
	}

// Get size of IREG. Returns -1 if can't write to buffer
int Egg_object::get_ireg_size()
{
	// These shouldn't ever happen, but you never know
	if (gumpman->find_gump(this) || Usecode_script::find(this))
		return -1;
	const char *str1 = get_str1();
	return 8 + get_common_ireg_size() + ((data3 > 0) ? 2 : 0)
		+ (*str1 ? Game_map::write_string(0, str1) : 0);
}

/*
 *	Create.
 */
Field_object::Field_object(int shapenum, int framenum, unsigned int tilex, 
		unsigned int tiley, unsigned int lft, unsigned char ty)
		: Egg_object(shapenum, framenum, tilex, tiley, lft, ty)
{
	Shape_info& info = get_info();
	if (info.is_animated())
		set_animator(new Field_frame_animator(this));
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
		if (rand()%2 && !actor->get_flag(Obj_flags::poisoned))
			{
			actor->set_flag(Obj_flags::poisoned);
			del = true;
			}
		break;
	case sleep_field:
		if (rand()%2 && !actor->get_flag(Obj_flags::asleep))
			{
			actor->set_flag(Obj_flags::asleep);
			del = true;
			}
		break;
	case fire_field:
		actor->reduce_health(2 + rand()%3, Weapon_data::fire_damage);
					// But no sleeping here.
		actor->clear_flag(Obj_flags::asleep);
		break;
	case caltrops_field:
		if (actor->get_effective_prop(Actor::intelligence) < rand()%40)
			//actor->reduce_health(2 + rand()%3, Weapon_info::normal_damage);
			// Caltrops don't seem to cause much damage.
			actor->reduce_health(1 + rand()%1, Weapon_data::normal_damage);
		return false;
		}
	if (!del)			// Tell animator to keep checking.
		((Field_frame_animator *) animator)->activated = true;
	return del;
	}

/*
 *	Render.
 */

void Field_object::paint
	(
	)
	{
	if (animator)
		animator->want_animation();	// Be sure animation is on.
	Ireg_game_object::paint();	// Always paint these.
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
	Actor_vector npcs;		// Find all nearby NPC's.
	gwin->get_nearby_npcs(npcs);
	npcs.push_back(gwin->get_main_actor());	// Include Avatar.
	Rectangle eggfoot = get_footprint();
					// Clear flag to check.
	((Field_frame_animator *) animator)->activated = false;
	for (Actor_vector::const_iterator it = npcs.begin(); 
						it != npcs.end(); ++it)
		{
		Actor *actor = *it;
		if (actor->is_dead() || Game_object::distance(actor) > 4)
			continue;
		if (actor->get_footprint().intersects(eggfoot))
			Field_object::hatch(actor);
		}
	}

/*
 *	Someone stepped on it.
 */

void Field_object::hatch
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

void Mirror_object::hatch(Game_object *obj, bool must)
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
