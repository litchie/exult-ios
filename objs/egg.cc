/**
 **	Objs.cc - Game objects.
 **
 **	Written: 10/1/98 - JSF
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "egg.h"
#include "gamewin.h"
#include "actors.h"
#include "animate.h"
#include "Audio.h"
#include "ucmachine.h"
#include "effects.h"
#include "game.h"
#include "items.h"
#include "npctime.h"
#include "paths.h"
#include "chunks.h"
#include "cheat.h"

#ifdef XWIN
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
class Missile_launcher : public Time_sensitive
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
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord src = egg->get_abs_tile_coord();
					// Is egg off the screen?
	if (!gwin->get_win_tile_rect().has_point(src.tx, src.ty))
		return;			// Return w'out adding back to queue.
	Projectile_effect *proj = 0;
	if (dir < 8)			// Direction given?
		{			// Get adjacent tile in direction.
		Tile_coord adj = src.get_neighbor(dir%8);
					// Make it go 12 tiles.
		int dx = adj.tx - src.tx, dy = adj.ty - src.ty;
		Tile_coord dest = src;
		dest.tx += 12*dx;
		dest.ty += 12*dy;
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
					party[i]->get_abs_tile_coord()))
				proj = new Projectile_effect(
					src, party[i], shapenum, weapon);
		}
	if (proj)
		gwin->add_effect(proj);
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Paint at given spot in world.
 */

void Egglike_game_object::paint
	(
	Game_window *gwin
	)
	{
	if(gwin->paint_eggs)
		Game_object::paint(gwin);
	}

/*
 *	Can this be clicked on?
 */

int Egglike_game_object::is_findable
	(
	Game_window *gwin
	)
	{
	return gwin->paint_eggs && Ireg_game_object::is_findable(gwin);
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
	criteria = (itype & (7<<4)) >> 4;
	distance = (itype >> 10) & 0x1f;
	unsigned char noct = (itype >> 7) & 1;
	unsigned char do_once = (itype >> 8) & 1;
					// Missile eggs can be rehatched
	unsigned char htch = (type == missile) ? 0 : ((itype >> 9) & 1);
	solid_area = (criteria == something_on || criteria == cached_in) ? 1 : 0;
	unsigned char ar = (itype >> 15) & 1;
	flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
	if (type == usecode || type == teleport || type == path)
		set_quality(data1&0xff);
	if (type == path)		// Store paths.
		Game_window::get_game_window()->add_path_egg(this);
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
		Game_window::get_game_window()->get_tqueue()->remove(launcher);
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
	Game_window *gwin = Game_window::get_game_window();
	int tx, ty, tz;			// Get absolute tile coords.
	get_abs_tile(tx, ty, tz);
	switch (criteria)		// Set up active area.
		{
	case cached_in:			// Make it really large.
		area = Rectangle(tx - 32, ty - 32, 64, 64);
		break;
	case avatar_footpad:
	case party_footpad:
		{
		Shape_info& info = gwin->get_info(this);
		int xtiles = info.get_3d_xtiles(), 
		    ytiles = info.get_3d_ytiles();
		area = Rectangle(tx - xtiles + 1, ty - ytiles + 1,
							xtiles, ytiles);
		break;
		}
	case avatar_far:		// Make it 1 tile bigger each dir.
		area = Rectangle(tx - distance - 1, ty - distance - 1, 
					2*distance + 3, 2*distance + 3);
		break;
	default:
		{
		int width = 2*distance;
		if (distance <= 1)	// Small?
			{
			width++;
					// More guesswork:
			if (criteria == external_criteria)
				width += 2;
			}
		area = Rectangle(tx - distance, ty - distance, 
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
	Game_window *gwin = Game_window::get_game_window();
	if (flags & (1 << (int) nocturnal))
		{			// Nocturnal.
		int hour = gwin->get_hour();
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
	case party_near:
		return (obj->get_party_id() >= 0 || 
					obj == gwin->get_main_actor()) &&
			area.has_point(tx, ty) && 
			(deltaz == 0 || deltaz == 1) &&
					!area.has_point(from_tx, from_ty);
	case avatar_near:		// New tile is in, old is out.
		return obj == gwin->get_main_actor() && 
			(deltaz == 0 || deltaz == 1 || 
					// Using trial&error here:
			 (Game::get_game_type() == SERPENT_ISLE &&
				(type != teleport || absdeltaz <= 3)) ||
				(type == missile && tz/5 == get_lift()/5)) &&
			area.has_point(tx, ty) &&
					!area.has_point(from_tx, from_ty);
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
			(obj->get_party_id() >= 0 || 
						obj == gwin->get_main_actor());
	case something_on:
		return obj != gwin->get_main_actor() && 
			tz >= get_lift() && deltaz <= 3 &&
			area.has_point(tx, ty) && obj->get_npc_num() <= 0;
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
	Game_window *gwin
	)
	{
	Egglike_game_object::paint(gwin);
					// Make sure launcher is active.
	if (launcher && !launcher->in_queue())
		gwin->get_tqueue()->add(0L, launcher, 0);
	}

/*
 *	Run usecode when double-clicked.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine,
	int /* event */
	)
	{
#ifdef XWIN
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_abs_tile_coord();
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
		return;
		}
#endif
	activate(umachine, 0, 0);
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
#ifdef XWIN
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
	Game_window *gwin = Game_window::get_game_window();
	if (!egg)			// Create a new one?
		{
		extern int Get_click(int&, int&, Mouse::Mouse_shapes, char *);
		int x, y;
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Send_data(client_socket, Exult_server::cancel);
			return;
			}
		if (shape == -1)
			shape = 275;	// FOR NOW.
		if (frame == -1)
			switch (type)
				{	// (These aren't perfect.)
			case monster: frame = 0; break;
			case jukebox: frame = 2; break;
			case soundsfx:frame = 1; break;
			case voice:   frame = 3; break;
			case weather: frame = 4; break;
			case teleport:frame = 5; break;
			case path:    frame = 6; break;
			default:      frame = 7; break;
				}
					// Create.  Gets initialized below.
		egg = new Egg_object(shape, frame, 0, 0, 0, 0, 0, 0, 0);
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(egg, x, y, lift))
				break;
		if (lift == 12)
			{
			if (client_socket >= 0)
				Send_data(client_socket, Exult_server::cancel);
			delete egg;
			return;
			}
		if (client_socket >= 0)
			Send_data(client_socket, Exult_server::user_responded);
		}
	egg->type = type;
	if (shape != -1)
		egg->set_shape(shape);
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
	Map_chunk *chunk = 
			gwin->get_chunk_safely(egg->get_cx(), egg->get_cy());
	chunk->remove_egg(egg);		// Got to add it back.
	chunk->add_egg(egg);
	cout << "Egg updated" << endl;
#endif
	}

/*
 *	Hatch egg.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine,
	Game_object *obj,		// Object (actor) that came near it.
	int must			// If 1, skip dice roll.
	)
	{
#ifdef DEBUG
	print_debug();
#endif
	int roll = must ? 0 : 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
					// Flag it as done.
	flags |= (1 << (int) hatched);
	Game_window *gwin = Game_window::get_game_window();
	switch(type)
		{
		case jukebox:
#ifdef DEBUG
			cout << "Audio parameters might be: " << (data1&0xff) << " and " << ((data1>>8)&0x01) << endl;
#endif
			Audio::get_ptr()->start_music((data1)&0xff,(data1>>8)&0x01);
			break;
		case voice:
			Audio::get_ptr()->start_speech((data1)&0xff);
			break;
		case monster:		// Also creates other objects.
			{
			int shnum = data2&1023;
			int frnum = data2>>10;
			Monster_info *inf = gwin->get_monster_info(shnum);
			if (inf)
				{
				int sched = data1>>8;
				int align = data1&3;
				int cnt = (data1&0xff)>>2;
				while (cnt--)
					{
					Monster_actor *monster = 
						inf->create(get_cx(),
						get_cy(), get_tx(), get_ty(),
						get_lift(), sched, align);
					gwin->add_dirty(monster);
					gwin->add_nearby_npc(monster);
					}
				}
			else		// Create item.
				{
				Shape_info& info = gwin->get_info(shnum);
				Game_object *nobj =
					gwin->create_ireg_object(info,
						shnum, frnum, get_tx(),
						get_ty(), get_lift());
				Map_chunk *chunk = 
					gwin->get_chunk(get_cx(), get_cy());
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
			{		// Data2 is the usecode function.
			umachine->call_usecode(data2, this,
					Usecode_machine::egg_proximity);
			break;
			}
		case missile:
			{
					// Get data.  Not sure about delay.
			int weapon = data1, dir = data2&0xff, delay = data2>>8;
			cout << "Missile egg:  " << item_names[weapon]
				<< endl;
			Shape_info& info = gwin->get_info(weapon);
			Weapon_info *winf = info.get_weapon_info();
			int proj;
			if (winf && winf->get_projectile())
				proj = winf->get_projectile();
			else
				proj = 856;// Fireball.  Shouldn't get here.
			if (!launcher)
				launcher = new Missile_launcher(this, weapon,
						proj, dir, 1000*delay);
			if (!launcher->in_queue())
				gwin->get_tqueue()->add(0L, launcher, 0);
			break;
			}
		case teleport:
			{
			Tile_coord pos;	// Get position to jump to.
			if (get_quality() == 255)
				{	// Jump to coords.
				int schunk = data1 >> 8;
				pos = Tile_coord((schunk%12)*c_tiles_per_schunk +
								(data2&0xff), 
					(schunk/12)*c_tiles_per_schunk +
								(data2>>8), 0);
				}
			else
				{
				Egg_object *path =
					gwin->get_path_egg(get_quality());
				if (!path)
					break;
				pos = path->get_abs_tile_coord();
				}
			cout << "Should teleport to (" << pos.tx << ", " <<
					pos.ty << ')' << endl;
			if (obj && (obj == gwin->get_main_actor() ||
					obj->get_party_id() >= 0))
					// Teleport everyone!!!
				gwin->teleport_party(pos);
					// Can keep doing it.
			flags &= ~((1 << (int) hatched));
			break;
			}
		case weather:
			{
			set_weather(gwin, data1&0xff, data1>>8, this);
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
					egg->activate(umachine, obj, 0);
				}
			break;
			}
		default:
			cout << "Egg not actioned" << endl;
                }
	if (flags & (1 << (int) once))
		remove_this();		// All done, so go away.
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
	Game_window *gwin,
	int weather,			// 0-6.
	int len,			// In game minutes (I think).
	Game_object *egg		// Egg this came from, or null.
	)
	{
	if (!len)			// Means continuous.
		len = 120;		// How about a couple game hours?
	int cur = gwin->get_weather();
	cout << "Current weather is " << cur << "; setting " << weather
							<< endl;
	switch (weather)
		{
	case 0:		// Back to normal.
		gwin->remove_weather_effects();
		break;
	case 2:		// Storm.
		if (cur != weather)
			gwin->add_effect(new Storm_effect(len, 0, egg));
		break;
	case 3:		// (On Ambrosia).
		gwin->remove_weather_effects();
		gwin->add_effect(new Sparkle_effect(len, 0, egg));
		break;
	case 6:		// Clouds.
		gwin->add_effect(new Clouds_effect(len, 0, egg));
		break;
	default:
		break;
		}
	}


void Egg_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	Map_chunk *chunk = 
			Game_window::get_game_window()->get_chunk_safely(
								cx, cy);
	if (chunk)
		chunk->remove_egg(this);
	if (!nodel)
		Game_window::get_game_window()->delete_object(this);
	}

/*
 *	Write out.
 */

void Egg_object::write_ireg
	(
	ostream& out
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
	out.write((char*)buf, sizeof(buf));
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
	animator = new Frame_animator(this, 1); 
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
	Game_window *gwin
	)
	{
	Ireg_game_object::paint(gwin);	// Always paint these.
	if (animator)
		animator->want_animation();	// Be sure animation is on.
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Animated_egg_object::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	Egg_object::activate(umachine, event);
	flags &= ~(1 << (int) hatched);	// Moongate:  reset always.
	}

/*
 *	Apply field.
 */

void Field_object::field_effect
	(
	Actor *actor
	)
	{
	switch (type)
		{
	case poison_field:
		if (rand()%2)
			actor->set_flag(Obj_flags::poisoned);
		break;
	case sleep_field:
		if (rand()%2)
			actor->set_flag(Obj_flags::asleep);
		break;
	case fire_field:
					// Blue fire (serpent isle)?
		if (get_shapenum() == 561)
			{
			actor->reduce_health(5 + rand()%4);
			say(first_ouch, last_ouch);
			}
		else if (rand()%2)
			{
			actor->reduce_health(1);
			if (rand()%2)
				say(first_ouch, last_ouch);
			}
					// But no sleeping here.
		actor->clear_flag(Obj_flags::asleep);
		break;
	case caltrops_field:
		if (actor->get_property(Actor::intelligence) < rand()%40)
			{
			actor->reduce_health(2 + rand()%3);
			say(first_ouch, last_ouch);
			}
		break;
		}
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 *	(Generally, nothing will happen.)
 */

void Field_object::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	Ireg_game_object::activate(umachine, event);
	}

/*
 *	Avatar stepped on it.
 */

void Field_object::activate
	(
	Usecode_machine *umachine,
	Game_object *obj,		// Object (actor) that came near it.
	int /* must */			// If 1, skip dice roll.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Main_actor *av = gwin->get_main_actor();
	if (obj != av && obj->get_party_id() < 0)
		return;			// Not a party member.
	field_effect((Actor *) obj);	// Apply field.
	}

/*
 *	Write out.  These are stored as normal game objects.
 */

void Field_object::write_ireg
	(
	ostream& out
	)
	{
	Ireg_game_object::write_ireg(out);
	}
