/*
 *	actors.cc - Game actors.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <iostream>			/* Debugging. */
#  include <cstdlib>
#  include <cstring>
#endif
#include <algorithm>		/* swap. */
#include <set>
#include <map>
#include "chunks.h"
#include "gamemap.h"
#include "Astar.h"
#include "Audio.h"
#include "Gump_manager.h"
#include "Paperdoll_gump.h"
#include "Zombie.h"
#include "actions.h"
#include "actors.h"
#include "cheat.h"
#include "combat.h"
#include "combat_opts.h"
#include "dir.h"
#include "egg.h"
#include "exult.h"
#include "frameseq.h"
#include "game.h"
#include "gamewin.h"
#include "gameclk.h"
#include "imagewin.h"
#include "items.h"
#include "npctime.h"
#include "ready.h"
#include "ucmachine.h"
#include "party.h"
#include "monstinf.h"
#include "exult_constants.h"
#include "monsters.h"
#include "effects.h"
#include "palette.h"
#include "ucsched.h"
#include "ucscriptop.h"
#include "miscinf.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

#ifndef UNDER_CE
using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::rand;
using std::string;
using std::swap;
using std::set;
using std::map;
#endif

Actor *Actor::editing = 0;

extern bool combat_trace;

// Party positions
// Direction, Party num, xy (tile) from leader
//
// Please Don't Touch - Colourless
//
const short Actor::party_pos[4][10][2] = {
		// North Facing
	{
		{ -2, 2 },
		{ 2, 2 },
		{ 0, 4 },
		{ -4, 4 },
		{ 4, 4 },
		{ -2, 6 },
		{ 2, 6 },
		{ 0, 8 },
		{ -4, 8 },
		{ 4, 8 }
	},
		// East Facing,
	{
		{ -2, -2 },
		{ -2, 2 },
		{ -4, 0 },
		{ -4, -4 },
		{ -4, 4 },
		{ -6, -2 },
		{ -6, 2 },
		{ -8, 0 },
		{ -8, -4 },
		{ -8, 4 }
	},
		// South Facing
	{
		{ -2, -2 },
		{ 2, -2 },
		{ 0, -4 },
		{ -4, -4 },
		{ 4, -4 },
		{ -2, -6 },
		{ 2, -6 },
		{ 0, -8 },
		{ -4, -8 },
		{ 4, -8 }
	},
		// West Facing
	{
		{ 2, -2 },
		{ 2, 2 },
		{ 4, 0 },
		{ 4, -4 },
		{ 4, 4 },
		{ 6, -2 },
		{ 6, 2 },
		{ 8, 0 },
		{ 8, -4 },
		{ 8, 4 }
	}
};

//	Actor frame to substitute when a frame is empty (as some are):
uint8 visible_frames[16] = {
	Actor::standing,		// Standing.
	Actor::standing,		// Steps.
	Actor::standing,
	Actor::standing,		// Ready.
	Actor::raise2_frame,		// 1-handed strikes => 2-handed.
	Actor::reach2_frame,
	Actor::strike2_frame,
	Actor::raise1_frame,		// 2-handed => 1-handed.
	Actor::reach1_frame,
	Actor::strike1_frame,
	Actor::standing,		// When you can't sit...
	Actor::kneel_frame,		// When you can't bow.
	Actor::bow_frame,		// When you can't kneel.
	Actor::standing,		// Can't lie.
	Actor::strike2_frame,		// Can't raise hands.
	Actor::strike2_frame };

Frames_sequence *Actor::avatar_frames[4] = {0, 0, 0, 0};
Frames_sequence *Actor::npc_frames[4] = {0, 0, 0, 0};
const signed char sea_serpent_attack_frames[] = {13, 12, 11, 0, 1, 2, 3, 11, 12, 
								13, 14};
const signed char reach_attack_frames1[] = {3, 6};
const signed char raise_attack_frames1[] = {3, 4, 6};
const signed char fast_swing_attack_frames1[] = {3, 5, 6};
const signed char slow_swing_attack_frames1[] = {3, 4, 5, 6};
const signed char reach_attack_frames2[] = {3, 9};
const signed char raise_attack_frames2[] = {3, 7, 9};
const signed char fast_swing_attack_frames2[] = {3, 8, 9};
const signed char slow_swing_attack_frames2[] = {3, 7, 8, 9};

// inline int Is_attack_frame(int i) { return i >= 3 && i <= 9; }
inline int Is_attack_frame(int i) { return i == 6 || i == 9; }
inline int Get_dir_from_frame(int i)
	{ return ((((i&16)/8) - ((i&32)/32)) + 4)%4; }

/*
 *	Provide attribute/value pairs.
 */
class Actor_attributes
	{
	static set<string> *strings;	// So names get shared.
	typedef map<const char *,int> Att_map;
	Att_map map;
public:
	Actor_attributes()
		{
		if (!strings)
			strings = new std::set<string>;
		}
	void set(const char *nm, int val)
		{
		std::set<string>::iterator siter = strings->find(nm);
		if (siter == strings->end())
			siter = strings->insert(nm).first;
		nm = (*siter).c_str();
		map[nm] = val;
		}
	int get(const char *nm)		// Returns 0 if not set.
		{
		std::set<string>::const_iterator siter = strings->find(nm);
		if (siter == strings->end())
			return 0;
		nm = (*siter).c_str();
		Att_map::const_iterator it = map.find(nm);
		return it == map.end() ? 0 : (*it).second;
		}
	void get_all(std::vector<std::pair<const char *,int> >& attlist)
		{
		Att_map::const_iterator it;
		for (it = map.begin(); it != map.end(); ++it)
			attlist.push_back(*it);
		}
	};
set<string> *Actor_attributes::strings = 0;

/*
 *	Get/create timers.
 */

Npc_timer_list *Actor::need_timers
	(
	)
	{
	if (!timers)
		timers = new Npc_timer_list(this);
	return timers;
	}

/*
 *	Initialize.
 */

void Actor::init
	(
	)
	{
	if (!avatar_frames[0])
		init_default_frames();
	size_t i;
	for (i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		properties[i] = 0;
	for (i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		spots[i] = 0;
	}

/*
 *	Find desired ammo.
 *
 *	Output:	->object if found.
 */

Game_object *Actor::find_ammo
	(
	int ammo			// Ammo family shape.
	)
	{
					// See if already have ammo.
	Game_object *aobj = get_readied(Actor::ammo);
	if (aobj && In_ammo_family(aobj->get_shapenum(), ammo))
		return aobj;		// Already readied.
	Game_object_vector vec;		// Get list of all possessions.
	vec.reserve(50);
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		if (In_ammo_family(obj->get_shapenum(), ammo))
			return obj;
		}
	return 0;
	}

/*
 *	Swap new ammo with old.
 */

void Actor::swap_ammo
	(
	Game_object *newammo
	)
	{
	Game_object *aobj = get_readied(Actor::ammo);
	if (aobj == newammo)
		return;			// Already what we need.
	if (aobj)			// Something there already?
		aobj->remove_this(1);	// Remove it.
	newammo->remove_this(1);
	add(newammo, 1);		// Should go to the right place.
	if (aobj)			// Put back old ammo.
		add(aobj, 1);
	}

/*
 *	Ready ammo for weapon being carried.
 *
 *	Output:	true if successful.
 */

bool Actor::ready_ammo
	(
	)
	{
	int points;
	Game_object *weapon = spots[static_cast<int>(lhand)];
	if (!weapon)
		return false;
	Shape_info& info = weapon->get_info();
	Weapon_info *winf = info.get_weapon_info();
	if (!winf)
		return false;
	int ammo;
	if ((ammo = winf->get_ammo_consumed()) == 0)
		{			// Ammo not needed.
		if (winf->uses_charges() && info.has_quality() &&
					weapon->get_quality() <= 0)
			return false;	// Uses charges, but none left.
		else
			return true;
		}
	Game_object *found = find_ammo(ammo);
	if (!found)
		return false;
	swap_ammo(found);
	return true;
	}

/*
 *	If no weapon readied, look through all possessions for the best one.
 *	Output:	true if successful.
 */

bool Actor::ready_best_weapon
	(
	)
	{
	int points;
	// What about spell book????
	if (Actor::get_weapon(points) != 0 && ready_ammo())
		return true;		// Already have one.
	Game_object_vector vec;		// Get list of all possessions.
	vec.reserve(50);
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	Game_object *best = 0, *best_ammo = 0;
	int best_damage = -20;
	Ready_type wtype=other;
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *obj = *it, *ammo_obj = 0;
		if (obj->get_shapenum() == 595)
			continue;	// Don't pick the torch. (Don't under-
					//   stand weapons.dat well, yet!)
		Shape_info& info = obj->get_info();
		Weapon_info *winf = info.get_weapon_info();
		int ammo;
		if (!winf)
			continue;	// Not a weapon.
		if ((ammo = winf->get_ammo_consumed()) != 0)
			{
			ammo_obj = find_ammo(ammo);
			if (!ammo_obj)
				continue;
			}
		if (winf->uses_charges() && info.has_quality() &&
					obj->get_quality() <= 0)
			continue;	// No charges left.
					// +++Might be a class to check.
		int damage = winf->get_damage();
		if (damage > best_damage)
			{
			wtype = static_cast<Ready_type>(info.get_ready_type());
			best = obj;
			best_ammo = ammo_obj;
			best_damage = damage;
			}
		}
	if (!best)
		return false;
	Game_object *remove1 = 0, *remove2 = 0;
	if (wtype == two_handed_weapon)
		{
		remove1 = spots[lhand];
		remove2 = spots[rhand];
		}
	else
		if (free_hand() == -1)
			remove1 = spots[lhand];
					// Free the spot(s).
	if (remove1)
		remove1->remove_this(1);
	if (remove2)
		remove2->remove_this(1);
	best->remove_this(1);
	add(best, 1);			// Should go to the right place.
	if (remove1)			// Put back other things.
		add(remove1, 1);
	if (remove2)
		add(remove2, 1);
	if (best_ammo)
		swap_ammo(best_ammo);
	return true;
	}

/*
 *	Try to store the weapon.
 */

void Actor::unready_weapon
	(
	)
	{
	Game_object *obj = spots[lhand];
	if (!obj)
		return;
	Shape_info& info = obj->get_info();
	if (!info.get_weapon_info())	// A weapon?
		return;
	gwin->add_dirty(this);
	if (!spots[belt])		// Belt free?
		{
		obj->remove_this(1);
		add_readied(obj, belt);
		}
	}

int Actor::get_effective_weapon_shape
	(
	)
	{
	if (get_casting_mode() == Actor::show_casting_frames)
		return 859;		// Casting frames
	else
		{
		Game_object * weapon = spots[lhand];
		return weapon->get_shapenum();
		}
	}

/*
 *	Add dirty rectangle(s).
 *
 *	Output:	0 if not on screen.
 */
int Actor::add_dirty
	(
	int figure_rect			// Recompute weapon rectangle.
	)
	{
	if (!gwin->add_dirty(this))
		return 0;
	int weapon_x, weapon_y, weapon_frame;
	if (figure_rect || get_casting_mode() == Actor::show_casting_frames)
		if (figure_weapon_pos(weapon_x, weapon_y, weapon_frame))
			{
			int shnum = get_effective_weapon_shape();
			
			Shape_frame *wshape = ShapeID(shnum, weapon_frame).get_shape();

			if (wshape)	// Set dirty area rel. to NPC.
				weapon_rect = gwin->get_shape_rect(wshape, 
							weapon_x, weapon_y);
			else
				weapon_rect.w = 0;
			}
		else
			weapon_rect.w = 0;
	if (weapon_rect.w > 0)		// Repaint weapon area too.
		{
		Rectangle r = weapon_rect;
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		r.shift(xoff, yoff);
		r.enlarge(4);
		gwin->add_dirty(gwin->clip_to_win(r));
		}
	return 1;
	}

/*
 *	Change the frame and set to repaint areas.
 */

void Actor::change_frame
	(
	int frnum
	)
	{
	add_dirty();			// Set to repaint old area.
	ShapeID id(get_shapenum(), frnum, get_shapefile());
	Shape_frame *shape = id.get_shape();
	if (!shape || shape->is_empty())
		{		// Swap 1hand <=> 2hand frames.
		frnum = (frnum&48)|visible_frames[frnum&15];
		id.set_frame(frnum);
		if (!(shape = id.get_shape()) || shape->is_empty())
			frnum = (frnum&48)|Actor::standing;
		}
	rest_time = 0;
	set_frame(frnum);
	add_dirty(1);			// Set to repaint new.
	}

/*
 *	See if it's blocked when trying to move to a new tile.
 *
 *	Output: 1 if so, else 0.
 */

int Actor::is_blocked
	(
	Tile_coord& t,			// Tz possibly updated.
	Tile_coord *f			// Step from here, or curpos if null.
	)
	{
	Shape_info& info = get_info();
					// Get dim. in tiles.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int ztiles = info.get_3d_height();
	if (xtiles == 1 && ytiles == 1)	// Simple case?
		{
		Map_chunk *nlist = gmap->get_chunk(
			t.tx/c_tiles_per_chunk, t.ty/c_tiles_per_chunk);
		nlist->setup_cache();
		int new_lift;
		int blocked = nlist->is_blocked(ztiles, t.tz,
			t.tx%c_tiles_per_chunk, t.ty%c_tiles_per_chunk,
					new_lift, get_type_flags());
		t.tz = new_lift;
		return blocked;
		}
	return Map_chunk::is_blocked(xtiles, ytiles, ztiles,
			f ? *f : get_tile(), t, get_type_flags());
	}

/*
 *	Move an object, and possibly change its shape too.
 */
inline void Actor::movef
	(
	Map_chunk *old_chunk, 
	Map_chunk *new_chunk, 
	int new_sx, int new_sy, int new_frame, 
	int new_lift
	)
	{
	if (old_chunk)			// Remove from current chunk.
		old_chunk->remove(this);
	set_shape_pos(new_sx, new_sy);
	if (new_frame >= 0)
		set_frame(new_frame);
	if (new_lift >= 0)
		set_lift(new_lift);
	new_chunk->add(this);
	}

/*
 *	Create character.
 */

Actor::Actor
	(
	const std::string &nm, 
	int shapenum, 
	int num,			// NPC # from npc.dat.
	int uc				// Usecode #.
	) : Container_game_object(), name(nm),usecode(uc), 
	    usecode_assigned(false), unused(false),
	    npc_num(num), face_num(num), party_id(-1), shape_save(-1), 
	    oppressor(-1), target(0), attack_mode(nearest),
	    schedule_type(static_cast<int>(Schedule::loiter)), schedule(0),
	    dormant(true), hit(false), combat_protected(false), 
	    user_set_attack(false), alignment(0),
	    two_handed(false), two_fingered(false), light_sources(0),
	    usecode_dir(0), usecode_target(0), usecode_weapon(0),
	    siflags(0), type_flags(0), ident(0),
	    skin_color(-1), action(0), 
	    frame_time(0), step_index(0), timers(0),
	    weapon_rect(0, 0, 0, 0), rest_time(0), casting_mode(false),
	    atts(0), usecode_name("")
	{
	set_shape(shapenum, 0); 
	init();
	frames = &npc_frames[0];	// Default:  5-frame walking.
	}

/*
 *	Delete.
 */

Actor::~Actor
	(
	)
	{
	delete schedule;
	delete action;
	delete timers;
	delete atts;
	}

/*
 *	Decrement food level and print complaints if it gets too low.
 *	NOTE:  Should be called every hour.
 */

void Actor::use_food
	(
	)
	{
	if (Game::get_game_type() == SERPENT_ISLE)
		{			// Automatons don't eat.
		int shnum = get_shapenum();
		if (shnum == 658 || shnum == 734 || shnum == 747)
			return;
		}
	int food = get_property(static_cast<int>(food_level));
	food -= rand()%4;		// Average 1.5 level/hour.
	set_property(static_cast<int>(food_level), food);
	if (food <= 0)			// Really low?
		{
		if (rand()%4)
			say(first_starving, first_starving + 2);
		if (food < 0)		// Set timer for damage.
			need_timers()->start_hunger();
		}
	else if (food <= 4)
		{
		if (rand()%3)
			say(first_needfood, first_needfood + 2);
		}
	else if (food <= 8)
		if (rand()%2)
			say(first_hunger, first_hunger + 2);
	}

/*
 *	Periodic check for freezing.
 */

void Actor::check_temperature
	(
	bool freeze			// Avatar's flag applies to party.
	)
	{
	if (!freeze)			// Not in a cold area?
		{
		if (!temperature)	// 0 means warm.
			return;		// Nothing to do.
					// Warming up.
		temperature -= (temperature >= 5 ? 5 : temperature);
		if (rand()%3 == 0)
			if (temperature >= 30)
				say(1218, 1221);
			else
				say(1214, 1217);
		return;
		}
	int shnum = get_shapenum();
	if ((shnum == 658 || shnum == 734 || shnum == 747) && GAME_SI)
		return;			// Automatons don't get cold.
	if (get_schedule_type() == Schedule::wait)
		return;			// Not following leader?  Leave alone.
	int warmth = figure_warmth();	// (This could be saved for speed.)
	if (warmth >= 100)		// Enough clothing?
		{
		if (!temperature)
			return;		// Already warm.
		int decr = 1 + (warmth - 100)/10;
		decr = decr > temperature ? temperature : decr;
		temperature -= decr;
		if (rand()%3 == 0)
			if (temperature >= 30)
				say(1201, 1205);
			else
				say(1194, 1200);
		return;
		}
	int incr = 1 + (100 - warmth)/20;
	temperature += incr;
	if (temperature > 63)
		temperature = 63;
	if (rand()%3 == 0) switch (temperature/10)
		{
	case 0:
		say(1182, 1184);	// A bit chilly.
		break;
	case 1:
		say(1185, 1187);	// It's colder.
		break;
	case 2:
		say(1188, 1190);
		break;
	case 3:
		say(1191, 1193);	// Frostbite.
		break;
	case 4:
		say(1206, 1208);
		break;
	case 5:
		say(1209, 1211);
		break;
	case 6:
		say(1212, 1213);	// Frozen.
		reduce_health(1 + rand()%3);
		break;
		}
	}

/*
 *	Get sequence of frames for an attack.
 *
 *	Output:	# of frames stored.
 */

int Actor::get_attack_frames
	(
	int weapon,			// Weapon shape, or 0 for innate.
	bool projectile,		// Shooting/throwing.
	int dir,			// 0-7 (as in dir.h).
	signed char *frames			// Frames stored here.
	) const
	{
	signed char baseframes[4];
	const signed char *which = baseframes;
	int cnt = 4;
	switch (get_shapenum())		// Special cases.
		{
	case 525:			// Sea serpent.
		which = sea_serpent_attack_frames;
		cnt = sizeof(sea_serpent_attack_frames);
		break;
	case 529:			// Slimes.
		return 0;		// None, I believe.
	default:
		const signed char *reach_attack_frames;
		const signed char *raise_attack_frames;
		const signed char *fast_swing_attack_frames;
		const signed char *slow_swing_attack_frames;
		if (two_handed)
			{
			reach_attack_frames = reach_attack_frames2;
			raise_attack_frames = raise_attack_frames2;
			fast_swing_attack_frames = fast_swing_attack_frames2;
			slow_swing_attack_frames = slow_swing_attack_frames2;
			}
		else
			{
			reach_attack_frames = reach_attack_frames1;
			raise_attack_frames = raise_attack_frames1;
			fast_swing_attack_frames = fast_swing_attack_frames1;
			slow_swing_attack_frames = slow_swing_attack_frames1;
			}
		unsigned char frame_flags;	// Get Actor_frame flags.
		Weapon_info *winfo;
		if (weapon && 
		    (winfo = ShapeID::get_info(weapon).get_weapon_info()) != 0)
			frame_flags = winfo->get_actor_frames(projectile);
		else				// Default to normal swing.
			frame_flags = projectile ? Weapon_info::reach : Weapon_info::fast_swing;
		switch (frame_flags)
			{
			case Weapon_info::reach:
				which = reach_attack_frames;
				cnt = sizeof(reach_attack_frames1);
				break;
			case Weapon_info::raise:
				which = raise_attack_frames;
				cnt = sizeof(raise_attack_frames1);
				break;
			case Weapon_info::fast_swing:
				which = fast_swing_attack_frames;
				cnt = sizeof(fast_swing_attack_frames1);
				break;
			case Weapon_info::slow_swing:
				which = slow_swing_attack_frames;
				cnt = sizeof(slow_swing_attack_frames1);
				break;
			}
		break;
		}
	for (int i = 0; i < cnt; i++)	// Copy frames with correct dir.
		{
		int frame = get_dir_framenum(dir, *which++);
					// Check for empty shape.
		ShapeID id(get_shapenum(), frame, get_shapefile());
		Shape_frame *shape = id.get_shape();
		if (!shape || shape->is_empty())
			{		// Swap 1hand <=> 2hand frames.
			frame = get_dir_framenum(dir,visible_frames[frame&15]);
			id.set_frame(frame);
			if (!(shape = id.get_shape()) || shape->is_empty())
				frame = get_dir_framenum(dir, Actor::standing);
			}
		*frames++ = frame;
		}
	return (cnt);
	}		

/*
 *	Set default set of frames.
 */

void Actor::init_default_frames
	(
	)
	{
					// Set up actor's frame lists.
					// Most NPC's walk with a 'stand'
					//   frame between steps.
	const int FRAME_NUM = 5;
	uint8		npc_north_frames[FRAME_NUM] = { 0,  1,  0,  2,  0},
			npc_south_frames[FRAME_NUM] = {16, 17, 16, 18, 16},
			npc_east_frames[FRAME_NUM] = {48, 49, 48, 50, 48},
			npc_west_frames[FRAME_NUM] = {32, 33, 32, 34, 32};
	npc_frames[static_cast<int> (north)/2] = 
			new Frames_sequence(FRAME_NUM, npc_north_frames);
	npc_frames[static_cast<int> (south)/2] = 
			new Frames_sequence(FRAME_NUM, npc_south_frames);
	npc_frames[static_cast<int> (east)/2] = 
			new Frames_sequence(FRAME_NUM, npc_east_frames);
	npc_frames[static_cast<int> (west)/2] = 
			new Frames_sequence(FRAME_NUM, npc_west_frames);
					// Avatar just walks left, right.
	uint8		avatar_north_frames[3] = {0, 1, 2},
			avatar_south_frames[3] = {16, 17, 18},
			avatar_east_frames[3] = {48, 49, 50},
			avatar_west_frames[3] = {32, 33, 34};
	avatar_frames[static_cast<int> (north)/2] = 
			new Frames_sequence(3, avatar_north_frames);
	avatar_frames[static_cast<int> (south)/2] = 
			new Frames_sequence(3, avatar_south_frames);
	avatar_frames[static_cast<int> (east)/2] = 
			new Frames_sequence(3, avatar_east_frames);
	avatar_frames[static_cast<int> (west)/2] = 
			new Frames_sequence(3, avatar_west_frames);
	}

/*
 *	This is called for the Avatar to return to a normal standing position
 *	when not doing anything else.  It could work for other party members,
 *	but currently isn't called for them.
 */

void Actor::stand_at_rest
	(
	)
	{
	rest_time = 0;			// Reset timer.
	int frame = get_framenum()&0xff;// Base frame #.
	if (frame == standing || frame == sit_frame || frame == sleep_frame)
		return;			// Already standing/sitting/sleeping.
	if (!is_dead() && schedule_type == Schedule::follow_avatar &&
	    !get_flag(Obj_flags::asleep))
		change_frame(get_dir_framenum(standing));
	}

/*
 *	Set new action.
 */

void Actor::set_action
	(
	Actor_action *newact
	)
	{
	if (newact != action)
		{
		delete action;
		action = newact;
		}
	if (!action)			// No action?  We're stopped.
		frame_time = 0;
	}

/*
 *	Notify scheduler that an object it may be using has disappeared.
 */

void Actor::notify_object_gone
	(
	Game_object *obj
	)
	{
	if (schedule)
		schedule->notify_object_gone(obj);
	}

/*
 *	Get destination, or current spot if no destination.
 */

Tile_coord Actor::get_dest
	(
	)
	{
	Tile_coord dest;
	if (action && action->get_dest(dest))
		return dest;
	else
		return get_tile();
	}

/*
 *	Walk towards a given tile.
 */

void Actor::walk_to_tile
	(
	Tile_coord dest,		// Destination.
	int speed,			// Time between frames (msecs).
	int delay,			// Delay before starting (msecs) (only
					//   if not already moving).
	int maxblk			// Max. # retries if blocked.
	)
	{
	if (!action)
		action = new Path_walking_actor_action(new Zombie(), maxblk);
	set_action(action->walk_to_tile(this, get_tile(), dest));
	if (action)			// Successful at setting path?
		start(speed, delay);
	else
		frame_time = 0;		// Not moving.
	}

/*
 *	Find a path towards a given tile.
 *
 *	Output:	0 if failed.
 */

int Actor::walk_path_to_tile
	(
	Tile_coord src,			// Our location, or an off-screen
					//   location to try path from.
	Tile_coord dest,		// Destination.
	int speed,			// Time between frames (msecs).
	int delay,			// Delay before starting (msecs) (only
					//   if not already moving).
	int dist,			// Distance to get within dest.
	int maxblk			// Max. # retries if blocked.
	)
	{
	set_action(new Path_walking_actor_action(new Astar(), maxblk));
	set_action(action->walk_to_tile(this, src, dest, dist));
	if (action)			// Successful at setting path?
		{
		start(speed, delay);
		return (1);
		}
	frame_time = 0;			// Not moving.
	return (0);
	}

/*
 *	Begin animation.
 */

void Actor::start
	(
	int speed,			// Time between frames (msecs).
	int delay			// Delay before starting (msecs) (only
					//   if not already moving).
	)
	{
	dormant = false;		// 14-jan-2001 - JSF.
	frame_time = speed;
	if (!in_queue() || delay)	// Not already in queue?
		{
		if (delay)
			gwin->get_tqueue()->remove(this);
		uint32 curtime = Game::get_ticks();
		gwin->get_tqueue()->add(curtime + delay, this, 
					reinterpret_cast<long>(gwin));
		}
	}

/*
 *	Stop animation.
 */
void Actor::stop
	(
	)
	{
	/* +++ This might cause jerky walking. Needs to be done above? */
	if (action)
		{
		action->stop(this);
		add_dirty();
		}
	frame_time = 0;
	}

/*
 *	Want one value to approach another.
 */

inline int Approach
	(
	int from,
	int to,
	int dist			// Desired distance.
	)
	{
	if (from <= to)			// Going forwards?
		return (to - from <= dist ? from : to - dist);
	else				// Going backwards.
		return (from - to <= dist ? from : to + dist);
	}

/*
 *	Follow the leader.
 */

void Actor::follow
	(
	Actor *leader
	)
	{
	if (Actor::is_dead())
		return;			// Not when dead.
	int delay = 0;
	int dist;			// How close to aim for.
	Tile_coord leaderpos = leader->get_tile();
	Tile_coord pos = get_tile();
	Tile_coord goal;
	if (leader->is_moving())	// Figure where to aim.
		{			// Aim for leader's dest.
		dist = 2 + party_id/3;
		goal = leader->get_dest();
		goal.tx = Approach(pos.tx, goal.tx, dist);
		goal.ty = Approach(pos.ty, goal.ty, dist);
		}
	else				// Leader stopped?
		{
		goal = leaderpos;	// Aim for leader.
		if (gwin->walk_in_formation && pos.distance(leaderpos) <= 6)
			return;		// In formation, & close enough.
//		cout << "Follow:  Leader is stopped" << endl;
		// +++++For formation, why not get correct positions?
		static int xoffs[10] = {-1, 1, -2, 2, -3, 3, -4, 4, -5, 5},
			   yoffs[10] = {1, -1, 2, -2, 3, -3, 4, -4, 5, -5};
		goal.tx += xoffs[party_id] + 1 - rand()%3;
		goal.ty += yoffs[party_id] + 1 - rand()%3;
		dist = 1;
		}
					// Already aiming along a path?
	if (is_moving() && action && action->following_smart_path() &&
					// And leader moving, or dest ~= goal?
		(leader->is_moving() || goal.distance(get_dest()) <= 5))
		return;
					// Tiles to goal.
	int goaldist = goal.distance(pos);
	if (goaldist < dist)		// Already close enough?
		{
		if (!leader->is_moving())
			stop();
		return;
		}
					// Is leader following a path?
	bool leaderpath = leader->action && 
				leader->action->following_smart_path();
					// Get leader's distance from goal.
	int leaderdist = goal.distance(leaderpos);
					// Get his speed.
	int speed = leader->get_frame_time();
	if (!speed)			// Not moving?
		{
		speed = 100;
		if (goaldist < leaderdist)	// Closer than leader?
					// Delay a bit IF not moving.
			delay = (1 + leaderdist - goaldist)*100;
		}
	if (goaldist - leaderdist >= 5)
		speed -= 20;		// Speed up if too far.
					// Get window rect. in tiles.
	Rectangle wrect = gwin->get_win_tile_rect();
	int dist2lead = pos.distance(leaderpos);
					// Getting kind of far away?
	if (dist2lead > wrect.w + wrect.w/2 &&
	    party_id >= 0 &&		// And a member of the party.
	    !leaderpath)		// But leader is not following path.
		{			// Approach, or teleport.
					// Try to approach from offscreen.
		if (approach_another(leader))
			return;
					// Find a free spot.
		goal = Map_chunk::find_spot(
				leader->get_tile(), 2, this);
		if (goal.tx != -1)
			{
			move(goal.tx, goal.ty, goal.tz);
			if (rand()%2)
				say(first_catchup, last_catchup);
			gwin->paint();
			return;
			}
		}
					// NOTE:  Avoid delay when moving,
					//  as it creates jerkiness.  AND,
					//  0 retries if blocked.
	walk_to_tile(goal, speed, delay, 0);
	}

/*
 *	Approach another actor from offscreen.
 *
 *	Output:	0 if failed.
 */

int Actor::approach_another
	(
	Actor *other,
	bool wait			// If true, game hangs until arrival.
	)
	{
	Tile_coord dest = other->get_tile();
					// Look outwards for free spot.
	dest = Map_chunk::find_spot(dest, 8, get_shapenum(), get_framenum());
	if (dest.tx == -1)
		return 0;
					// Where are we now?
	Tile_coord src = get_tile();
	if (!gwin->get_win_tile_rect().has_point(src.tx - src.tz/2, 
							src.ty - src.tz/2))
					// Off-screen?
		src = Tile_coord(-1, -1, 0);
	int destmap = other->get_map_num();
	int srcmap = get_map_num();
	if (destmap != -1 && srcmap != -1 && srcmap != destmap)
	{
		src = Tile_coord(-1, -1, 0);
		move(src, destmap);
	}
	Actor_action *action = new Path_walking_actor_action();
	if (!action->walk_to_tile(this, src, dest))
		{
		delete action;
		return 0;
		}
	set_action(action);
	int speed = gwin->get_std_delay()/2;
	start(speed);			// Walk fairly fast.
	if (wait)			// Only wait ~1/5 sec.
		Wait_for_arrival(this, dest, 2*gwin->get_std_delay());
	return 1;
	}

/*
 *	Get information about a tile that an actor is about to step onto.
 */

void Actor::get_tile_info
	(
	Actor *actor,			// May be 0 if not known.
	Game_window *gwin,
	Map_chunk *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& water,			// Returns 1 if water.
	int& poison			// Returns 1 if poison.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (flat.get_shapenum() == -1)
		water = poison = 0;
	else
		{
		Shape_info& finfo = flat.get_info();
		water = finfo.is_water();
		poison = finfo.is_poisonous();
					// Check for swamp/swamp boots.
		if (poison && actor)
			{
			Game_object *boots = actor->Actor::get_readied(
							Actor::feet);
			if (boots != 0 &&
		    	    ((boots->get_shapenum() == 588 && 
					Game::get_game_type() == BLACK_GATE) ||
		    	     (boots->get_shapenum() == 587 && 
					// SI:  Swamp/magic boots save you.
				(boots->get_framenum() == 6 ||
				 boots->get_framenum() == 1) && 
				Game::get_game_type() == SERPENT_ISLE)))
				poison = 0;
			else
				{	// Safe from poisoning?
				Monster_info *minf = 
					actor->get_info().get_monster_info();
				if (minf && minf->poison_safe())
					poison = 0;
				}
			}
		}
	}

/*
 *	Set combat opponent.
 */

void Actor::set_target
	(
	Game_object *obj,
	bool start_combat		// If true, set sched. to combat.
	)
	{
	target = obj;
	if (start_combat && (schedule_type != Schedule::combat || !schedule))
		set_schedule_type(Schedule::combat);
	Actor *opponent = obj ? obj->as_actor() : 0;
	if (opponent)
		opponent->set_oppressor(get_npc_num());
	}

/*
 *	Works out if an item can be readied in a spot
 *
 *	Output:	true if it does fit, or false if it can't
 */

bool Actor::fits_in_spot (Game_object *obj, int spot, FIS_Type type)
{
	// If occupied, can't place
	if (spots[spot])
		return false;
	// If want to use 2h or a 2h is already equiped, can't go in right
	else if ((type == FIS_2Hand || two_handed) && spot == rhand)
		return false;
	// If want to use 2f or a 2f is already equiped, can't go in right
	else if ((type == FIS_2Finger || two_fingered) && spot == rfinger)
		return false;
	// Can't use 2h in left if right occupied
	else if (type == FIS_2Hand && spot == lhand && spots[rhand])
		return false;
	// Can't use 2f in left if right occupied
	else if (type == FIS_2Finger && spot == lfinger && spots[rfinger])
		return false;
	// If in left or right hand allow it
	else if (spot == lhand || spot == rhand)
		return true;
	// Special Checks for Belt
	else if (spot == belt)
	{
		if (type == FIS_Spell)
			return true;
		else if (Game::get_game_type() == BLACK_GATE)
		{
			if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), back2h_spot))
				return true;
			else if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), shield_spot))
				return true;
		}
	}

	// Lastly if we have gotten here, check the paperdoll table 
	return Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
			obj->get_framenum(), spot);
}

/*
 *	Find the spot(s) where an item would prefer to be readied
 *
 *	Output:	prefered slot, alternative slot, FIS_type
 */

void Actor::get_prefered_slots
	(
	Game_object *obj,
	int &prefered,
	int &alternate,
	FIS_Type &fistype
	)
{

	Shape_info& info = obj->get_info();

	// Defaults
	fistype = FIS_Other;
	prefered = lhand;
	alternate = lhand;

	if (Game::get_game_type() == BLACK_GATE)
	{
		Ready_type type = (Ready_type) info.get_ready_type();
		
		switch (type)
		{
			// Weapons, Sheilds, Spells, misc stuff
			default:
			if (type == spell || type == other_spell) fistype = FIS_Spell;
			else if (type == two_handed_weapon) fistype = FIS_2Hand;

			if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), rhand))
				prefered = rhand;
			else if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), back))
				prefered = back;
			else
				alternate = rhand;
			break;


			case gloves:
			fistype = FIS_2Finger;

			case ring:
			prefered = lfinger;
			alternate = rfinger;
			break;

			case neck_armor:
			prefered = neck;
			break;
				
			case torso_armor:
			prefered = torso;
			alternate = neck;	// This is a hack for cloaks. It shouldn't cause problems
			break;
				
			case ammunition:
			prefered = ammo;
			break;
			
			case head_armor:
			prefered = head;
			break;
				
			case leg_armor:
			prefered = legs;
			break;
				
			case foot_armor:
			prefered = feet;
			break;
			
			case usecode_container_bg:
			prefered = ucont_spot;
			break;
		}
	}
	else if (Game::get_game_type() == SERPENT_ISLE)	// Serpent Isle Types
	{
		Ready_type_SI type = (Ready_type_SI) info.get_ready_type();
		
		switch (type)
		{
			// Weapons, Sheilds, Spells, misc stuff
			default:
			if (type == spell_si|| type == other_spell_si) fistype = FIS_Spell;
			else if (type == two_handed_si) fistype = FIS_2Hand;

			if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), rhand))
				prefered = rhand;
			else
				alternate = rhand;
			break;

			case other:
			if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), back2h_spot))
				prefered = back2h_spot;
			else if (Shapeinfo_lookup::IsObjectAllowed(obj->get_shapenum(),
					obj->get_framenum(), back))
				prefered = back;
			else 
				alternate = rhand;
			break;


			case helm_si:
			prefered = head;
			break;

			case gloves_si:
			prefered = hands2_spot;
			break;

			case boots_si:
			prefered = feet;
			break;

			case leggings_si:
			prefered = legs;
			break;
			
			case amulet_si:
			prefered = neck;
			break;
				
			case armour_si:
			prefered = torso;
			break;

			case ring_si:
			prefered = lfinger;
			alternate = rfinger;
			break;

			case ammo_si:
			prefered = ammo;
			break;
	
			case cloak_si:
			prefered = cloak_spot;
			break;

			case usecode_container_si:
			prefered = ucont_spot;
			break;
			
			case earrings_si:
			prefered = ears_spot;
			break;

			case belt_si:
			prefered = belt;
			break;
			
			case backpack_si:
			prefered = back;
			break;
		}
	}
}


/*
 *	Find the best spot where an item may be readied.
 *
 *	Output:	Index, or -1 if none found.
 */

int Actor::find_best_spot
	(
	Game_object *obj
	)
{
	int prefered;
	int alternate;
	FIS_Type type;
	bool SI = Game::get_game_type() == SERPENT_ISLE;

	// Get the preferences
	get_prefered_slots (obj, prefered, alternate, type);

	// Check Prefered
	if (fits_in_spot (obj, prefered, type)) return prefered;
	// Alternate
	else if (fits_in_spot (obj, alternate, type)) return alternate;
	// Belt
	else if (fits_in_spot (obj, belt, type)) return belt;
	// Back - required???
	else if (fits_in_spot (obj, back, type)) return back;
	// Back2h
	else if (SI && fits_in_spot (obj, back2h_spot, type)) return back2h_spot;
	// Sheild Spot
	else if (SI && fits_in_spot (obj, shield_spot, type)) return shield_spot;
	// Left Hand
	else if (fits_in_spot (obj, lhand, type)) return lhand;
	// Right Hand
	else if (fits_in_spot (obj, rhand, type)) return rhand;

	return -1;
}

/*
 *	Get previous schedule type.
 *
 *	Output:	Prev. schedule #, or -1 if not known.
 */

int Actor::get_prev_schedule_type
	(
	)
	{
	return schedule ? schedule->get_prev_type() : -1;
	}

/*
 *	Restore actor's schedule after reading.  This CANNOT be called from
 *	a constructor, since it may call virtual methods when setting up
 *	the schedule.
 */

void Actor::restore_schedule
	(
	)
	{
					// Make sure it's in valid chunk.
	Map_chunk *olist = get_chunk();
					// Activate schedule if not in party.
	if (olist && party_id < 0)
		{
		if (next_schedule != 255 && 
				schedule_type == Schedule::walk_to_schedule)
			set_schedule_and_loc(next_schedule, schedule_loc);
		else
			set_schedule_type(schedule_type);
		}
	}

/*
 *	Set new schedule by type.
 */

void Actor::set_schedule_type
	(
	int new_schedule_type,
	Schedule *newsched		// New sched., or 0 to create here.
	)
	{
	stop();				// Stop moving.
	if (schedule)			// Finish up old if necessary.
		schedule->ending(new_schedule_type);
	set_action(0);			// Clear out old action.
					// Save old for a moment.
	Schedule::Schedule_types old_schedule = (Schedule::Schedule_types)
								schedule_type;
	delete schedule;		// Done with the old.
	schedule = newsched;
	if (!schedule)
		switch ((Schedule::Schedule_types) new_schedule_type)
			{
		case Schedule::combat:
			schedule = new Combat_schedule(this, old_schedule);
			break;
		case Schedule::horiz_pace:
			ready_best_weapon();
			schedule = Pace_schedule::create_horiz(this);
			break;
		case Schedule::vert_pace:
			ready_best_weapon();
			schedule = Pace_schedule::create_vert(this);
			break;
		case Schedule::talk:
			schedule = new Talk_schedule(this);
			break;
		case Schedule::dance:
			unready_weapon();
			schedule = new Dance_schedule(this);
			break;
		case Schedule::farm:	// Use a scythe.
			schedule = new Tool_schedule(this, 618);
			break;
		case Schedule::tend_shop:// For now.
			unready_weapon();
			schedule = new Loiter_schedule(this, 3);
			break;
		case Schedule::miner:	// Use a pick.
			schedule = new Tool_schedule(this, 624);
			break;
		case Schedule::hound:
			ready_best_weapon();
			schedule = new Hound_schedule(this);
			break;
		case Schedule::loiter:
		case Schedule::graze:	// For now.
			schedule = new Loiter_schedule(this);
			break;
		case Schedule::wander:
			schedule = new Wander_schedule(this);
			break;
		case Schedule::blacksmith:
			schedule = new Forge_schedule(this);
			break;
		case Schedule::sleep:
			unready_weapon();
			schedule = new Sleep_schedule(this);
			break;
		case Schedule::wait:
			schedule = new Wait_schedule(this);
			break;
		case Schedule::eat:		// For now.
		case Schedule::sit:
			unready_weapon();
			schedule = new Sit_schedule(this);
			break;
		case Schedule::bake:
			schedule = new Bake_schedule(this);
			break;
		case Schedule::sew:
			schedule = new Sew_schedule(this);
			break;
		case Schedule::shy:
			unready_weapon();
			schedule = new Shy_schedule(this);
			break;
		case Schedule::lab:
			schedule = new Lab_schedule(this);
			break;
		case Schedule::thief:
			unready_weapon();
			schedule = new Thief_schedule(this);
			break;
		case Schedule::waiter:
			unready_weapon();
			schedule = new Waiter_schedule(this);
			break;
		case Schedule::kid_games:
			unready_weapon();
			schedule = new Kid_games_schedule(this);
			break;
		case Schedule::eat_at_inn:
			unready_weapon();
			schedule = new Eat_at_inn_schedule(this);
			break;
		case Schedule::duel:
			schedule = new Duel_schedule(this);
			break;
		case Schedule::preach:
			ready_best_weapon();	// Fellowship staff.
			schedule = new Preach_schedule(this);
			break;
		case Schedule::patrol:
			ready_best_weapon();
			schedule = new Patrol_schedule(this);
			break;
		case Schedule::desk_work:
			unready_weapon();
			schedule = new Desk_schedule(this);
			break;
		case Schedule::follow_avatar:
			schedule = new Follow_avatar_schedule(this);
			break;
		case Schedule::walk_to_schedule:
			cerr << "Attempted to set a \"walk to schedule\" activity for NPC "<< get_npc_num() << endl;
			break;
		default:
			if (new_schedule_type >= 
					Schedule::first_scripted_schedule)
				schedule = new Scripted_schedule(this,
							new_schedule_type);
			break;
			}
					// Set AFTER creating new schedule.
	schedule_type = new_schedule_type;

	// Reset Next Schedule
	schedule_loc = Tile_coord(0,0,0);
	next_schedule = 255;

	if (!gmap->is_chunk_read(get_cx(), get_cy()))
		dormant = true;		// Chunk hasn't been read in yet.
	else if (schedule)		// Try to start it.
		{
		dormant = false;
		schedule->now_what();
		}
	}

/*
 *  Cache out an actor. 
 *  Resets the schedule, and makes the actor dormant
 */
void Actor::cache_out()
{
	// This is a bit of a hack, but it works well enough
	if (get_schedule_type() != Schedule::walk_to_schedule)
		set_schedule_type(get_schedule_type());
}


/*
 *	Set new schedule by type AND location.
 */

void Actor::set_schedule_and_loc (int new_schedule_type, Tile_coord dest,
				int delay)	// -1 for random delay.
{
	stop();				// Stop moving.
	if (schedule)			// End prev.
		schedule->ending(new_schedule_type);
	int mapnum = get_map_num();
	if (mapnum < 0) mapnum = gmap->get_num();
	if ((mapnum != gmap->get_num()) ||
	    (!gmap->is_chunk_read(get_cx(), get_cy()) &&
	     !gmap->is_chunk_read(dest.tx/c_tiles_per_chunk,
						dest.ty/c_tiles_per_chunk)))
		{			// Not on current map, or
					//   src, dest. are off the screen.
		move(dest.tx, dest.ty, dest.tz, mapnum);
		set_schedule_type(new_schedule_type);
		return;
		}
					// Going to walk there.
	schedule_loc = dest; 
	next_schedule = new_schedule_type;
	schedule_type = Schedule::walk_to_schedule;
	delete schedule;
	schedule = new Walk_to_schedule(this, dest, next_schedule, delay);
	dormant = false;
	schedule->now_what();
}

/*
 *	Get alignment, taking into account 'charmed' flag.
 */

int Actor::get_effective_alignment
	(
	) const
	{
	if (!(flags&(1<<Obj_flags::charmed)))
		return alignment;
	else switch(alignment)
		{
	case neutral:
		return unknown_align;
	case friendly:
		return hostile;
	case hostile:
		return friendly;
	case unknown_align:
		return neutral;
		}
	return neutral;
	}

/*
 *	Render.
 */

void Actor::paint
	(
	)
	{
					// In BG, dont_move means don't render.
	if (!(flags & (1L << Obj_flags::dont_move)) ||
	    Game::get_game_type() == SERPENT_ISLE)
		{
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		if (flags & (1L << Obj_flags::invisible))
			paint_invisible(xoff, yoff);
		else 
			paint_shape(xoff, yoff, true);

		paint_weapon();
		if (hit)		// Want a momentary red outline.
			ShapeID::paint_outline(xoff, yoff, HIT_PIXEL);
		else if (flags & ((1L<<Obj_flags::protection) | 
		    (1L << Obj_flags::poisoned) | (1 << Obj_flags::cursed) |
		    	(1 << Obj_flags::charmed)))
			{
			if (flags & (1L << Obj_flags::poisoned))
				ShapeID::paint_outline(xoff,yoff,POISON_PIXEL);
			else if (flags & (1L << Obj_flags::cursed))
				ShapeID::paint_outline(xoff,yoff,CURSED_PIXEL);
			else if (flags & (1L << Obj_flags::charmed))
				ShapeID::paint_outline(xoff, yoff,
								CHARMED_PIXEL);
			else
				ShapeID::paint_outline(xoff, yoff,
								PROTECT_PIXEL);
			}
		}
	}
/*
 *	Draw the weapon in the actor's hand (if any).
 */
void Actor::paint_weapon
	(
	)
	{
	int weapon_x, weapon_y, weapon_frame;
	if (figure_weapon_pos(weapon_x, weapon_y, weapon_frame))
		{
		int shnum = get_effective_weapon_shape();
		ShapeID wsid(shnum, weapon_frame);
		Shape_frame *wshape = wsid.get_shape();
		if (!wshape)
			{
			weapon_rect.w = 0;
			return;
			}
					// Set dirty area rel. to NPC.
		weapon_rect = gwin->get_shape_rect(wshape, weapon_x, weapon_y);
		// Paint the weapon shape using the actor's coordinates
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		xoff += weapon_x;
		yoff += weapon_y;

		if (flags & (1L<<Obj_flags::invisible))
			wsid.paint_invisible(xoff, yoff);
		else
			wsid.paint_shape(xoff, yoff);
		}
	else
		weapon_rect.w = 0;
	}

/*
 *	Figure weapon drawing info.  We need this in advance to set the dirty
 *	rectangle.
 *
 *	Output:	0 if don't need to paint weapon.
 */
/* Weapon frames:
	0 - normal item
	1 - in hand, actor facing north/south
	2 - attacking (pointing north)
	3 - attacking (pointing east)
	4 - attacking (pointing south)
*/

int Actor::figure_weapon_pos
	(
	int& weapon_x, int& weapon_y,	// Pos. rel. to NPC.
	int& weapon_frame
	)
	{
	unsigned char actor_x, actor_y;
	unsigned char wx, wy;
	if((spots[lhand] == 0) && (get_casting_mode() != Actor::show_casting_frames))
		return 0;
	// Get offsets for actor shape
	int myframe = get_framenum();
	get_info().get_weapon_offset(myframe & 0x1f, actor_x,
			actor_y);
	// Get weapon frames for actor frame:
	switch (myframe)
	{
		case 4:
		case 7:
		case 22:
		case 25:
		case 36:
		case 39:
		case 54:
		case 57:
			weapon_frame = 4;
			break;
		case 5:
		case 8:
		case 21:
		case 24:
		case 37:
		case 40:
		case 53:
		case 56:
			weapon_frame = 3;
			break;
		case 6:
		case 9:
		case 20:
		case 23:
		case 38:
		case 41:
		case 52:
		case 55:
			weapon_frame = 2;
			break;
		//The next cases (before the default) are here to make use of all
		//the frames of the "casting frames" shape (shape 859):
		case 14:
		case 30:
		case 46:
		case 62:
			weapon_frame = 5;
			break;
		case 15:
		case 47:
			weapon_frame = 6;
			break;
		case 31:
		case 63:
			weapon_frame = 7;
			break;
		
		default:
			weapon_frame = 1;
	}
	weapon_frame |= (myframe & 32);

	// Get offsets for weapon shape
	int shnum = get_effective_weapon_shape();
	Shape_info& info = ShapeID::get_info(shnum);
	info.get_weapon_offset(weapon_frame&0xf, wx, wy);

	// actor_x will be 255 if (for example) the actor is lying down
	// wx will be 255 if the actor is not holding a proper weapon
	if(actor_x != 255 && wx != 255)
		{			// Store offsets rel. to NPC.
		weapon_x = wx - actor_x;
		weapon_y = wy - actor_y;
		// Need to swap offsets if actor's shape is reflected
		if(myframe & 32)
			{
			swap(weapon_x, weapon_y);
					// Combat frames are already done.
			}
#if 0	/* +++++Philanderer's Wand looks strange. */
					// Watch for valid frame.
		int nframes = gwin->get_shape_num_frames(
						weapon->get_shapenum());
		if ((weapon_frame&31) >= nframes)
			weapon_frame = (nframes - 1)|(weapon_frame&32);
#endif
		return 1;
		}
	else
		return 0;
	}

/*
 *	Run usecode when double-clicked.
 */
void Actor::activate
	(
	int event
	)
	{
	if (edit())
		return;
	// We are serpent if we can use serpent isle paperdolls
	bool serpent = Game::get_game_type()==SERPENT_ISLE||
		(sman->can_use_paperdolls() && sman->get_bg_paperdolls());
	
	bool show_party_inv = gumpman->showing_gumps(true) || 
							gwin->in_combat();
	Schedule::Schedule_types sched = 
				(Schedule::Schedule_types) get_schedule_type();
	if (!npc_num ||		// Avatar
			(show_party_inv && party_id >= 0 && // Party
			(serpent || (npc_num >= 1 && npc_num <= 10))) ||
					// Pickpocket cheat && double click
			(cheat.in_pickpocket() && event == 1))
		show_inventory();
					// Asleep (but not awakened)?
	else if ((sched == Schedule::sleep &&
		(get_framenum()&0xf) == Actor::sleep_frame) ||
		 get_flag(Obj_flags::asleep))
		return;
	else if (sched == Schedule::combat && party_id < 0)
		return;			// Too busy fighting.
					// Usecode
					// Failed copy-protection?
	else if (serpent &&
		 gwin->get_main_actor()->get_flag(Obj_flags::confused))
		ucmachine->call_usecode(0x63d, this,
			(Usecode_machine::Usecode_events) event);	
	else if (usecode == -1)
		ucmachine->call_usecode(get_usecode(), this,
			(Usecode_machine::Usecode_events) event);
	else if (party_id >= 0 || !gwin->is_time_stopped())
		ucmachine->call_usecode(usecode, this, 
			(Usecode_machine::Usecode_events) event);
	
	}

/*
 *	Edit in ExultStudio.
 *
 *	Output:	True if map-editing & ES is present.
 */

bool Actor::edit
	(
	)
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_tile();
		unsigned long addr = reinterpret_cast<unsigned long>(this);
		int num_schedules;	// Set up schedule-change list.
		Schedule_change *changes;
		get_schedules(changes, num_schedules);
		Serial_schedule schedules[8];
		for (int i = 0; i < num_schedules; i++)
			{
			schedules[i].time = changes[i].get_time();
			schedules[i].type = changes[i].get_type();
			Tile_coord p = changes[i].get_pos();
			schedules[i].tx = p.tx;
			schedules[i].ty = p.ty;
			schedules[i].tz = p.tz;
			}
		if (Npc_actor_out(client_socket, addr, t.tx, t.ty, t.tz,
				get_shapenum(), get_framenum(), get_face_shapenum(),
				name, npc_num, ident, usecode, usecode_name, properties,
				attack_mode, alignment, flags, siflags, type_flags,
				num_schedules, schedules) != -1)
			{
			cout << "Sent npc data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending npc data to ExultStudio" <<endl;
		return true;
		}
#endif
	return false;
	}

/*
 *	Message to update from ExultStudio.
 */

void Actor::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, face;
	std::string name;
	short npc_num, ident;
	int usecode;
	std::string usecodefun;
	int properties[12];
	short attack_mode, alignment;
	unsigned long oflags;		// Object flags.
	unsigned long siflags;		// Extra flags for SI.
	unsigned long type_flags;	// Movement flags.
	short num_schedules;
	Serial_schedule schedules[8];
	if (!Npc_actor_in(data, datalen, addr, tx, ty, tz, shape, frame,
			face, name, npc_num, ident, usecode, usecodefun,
			properties, attack_mode, alignment,
			oflags, siflags, type_flags, num_schedules, schedules))
		{
		cout << "Error decoding npc" << endl;
		return;
		}
	Actor *npc = (Actor *) addr;
	if (npc && npc != editing)
		{
		cout << "Npc from ExultStudio is not being edited" << endl;
		return;
		}
	editing = 0;
	if (!npc)			// Create a new one?
		{
		int x, y;
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, Exult_server::cancel);
			return;
			}
					// Create.  Gets initialized below.
		npc = new Npc_actor(name, shape, npc_num, usecode);
		npc->usecode_name = usecodefun;
		if (usecodefun.size())
			npc->usecode = ucmachine->find_function(usecodefun.c_str(), true);
		if (npc_num >= 256 && npc->usecode != -1 &&
				npc->usecode != 0x400 + npc_num)
			npc->usecode_assigned = true;
		else
			npc->usecode_assigned = false;

		npc->set_invalid();	// Set to invalid position.
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(npc, x, y, lift))
				break;
		if (lift == 12)
			{
			if (client_socket >= 0)
				Exult_server::Send_data(client_socket, Exult_server::cancel);
			delete npc;
			return;
			}
		gwin->add_npc(npc, npc_num);
		if (client_socket >= 0)
			Exult_server::Send_data(client_socket, Exult_server::user_responded);
		}
	else				// Old.
		{
		npc->add_dirty();
		npc->usecode_name = usecodefun;
		if (usecodefun.size())
			npc->usecode = ucmachine->find_function(usecodefun.c_str(), true);
		else
			npc->usecode = usecode;
		if (npc_num >= 256 && npc->usecode != -1 &&
				npc->usecode != 0x400 + npc_num)
			npc->usecode_assigned = true;
		else
			npc->usecode_assigned = false;

		npc->set_npc_name(name.c_str());
		}
	// Ensure proper initialization of frame #:
	npc->set_shape(shape, frame);
	npc->add_dirty();
	npc->face_num = face;
	npc->set_ident(ident);
	int i;
	for (i = 0; i < 12; i++)
		npc->set_property(i, properties[i]);
	npc->set_attack_mode((Actor::Attack_mode) attack_mode);
	npc->set_alignment(alignment);
	npc->flags = oflags;
	npc->siflags = siflags;
	npc->type_flags = type_flags;
	Schedule_change *scheds = num_schedules ? 
				new Schedule_change[num_schedules] : 0;
	for (i = 0; i < num_schedules; i++)
		scheds[i].set(schedules[i].tx, schedules[i].ty,
				schedules[i].tz,
				schedules[i].type, schedules[i].time);
	npc->set_schedules(scheds, num_schedules);
	cout << "Npc updated" << endl;
#endif
	}


void Actor::show_inventory()
{
	Gump_manager *gump_man = gumpman;

	int shapenum = inventory_shapenum();
	if (shapenum)
		gump_man->add_gump(this, shapenum, true);
}

int Actor::inventory_shapenum()
{
	// We are serpent if we can use serpent isle paperdolls
	bool serpent = Game::get_game_type()==SERPENT_ISLE||(sman->can_use_paperdolls() && sman->get_bg_paperdolls());
	
	if (!serpent)
		{	// Can't display paperdolls (or they are disabled)
			// Use BG gumps
		Paperdoll_npc *npcinfo =
			Shapeinfo_lookup::GetCharacterInfo(get_shapenum());

		if (!npcinfo) npcinfo = Shapeinfo_lookup::GetCharacterInfo(get_sexed_coloured_shape());
		if (!npcinfo) npcinfo = Shapeinfo_lookup::GetCharacterInfoSafe(get_shape_real());
		if (!npcinfo)		// No paperdoll info at ALL; should never happen...
			return (65);	// Default to male (Pickpocket Cheat)
		
		return npcinfo->gump_shape;
		}
	else /* if (serpent) */
		return (123);		// Show paperdolls
}


/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Actor::drop
	(
	Game_object *obj		// MAY be deleted (if combined).
	)
	{
	if (is_in_party())	// In party?
		{
		int res = add(obj, false, true);// We'll take it, and combine.
		int ind = find_readied(obj);
		if (ind >= 0)
			call_readied_usecode(ind,obj,Usecode_machine::readied);
		return res;
		}
	else
		return 0;
	}

/*
 *	Get name.
 */

string Actor::get_name
	(
	) const
	{
	return !get_flag(Obj_flags::met)?Game_object::get_name():get_npc_name();
	}

/*
 *	Get npc name.
 */

string Actor::get_npc_name
	(
	) const
	{
	return name.empty() ? Game_object::get_name() : name;
	}

/*
 *	Set npc name.
 */

void Actor::set_npc_name(const char *n)
{
	name = n;
}

/*
 *	Set property.
 */
void Actor::set_property
	(
	int prop, 
	int val
	)
	{
	if (prop == health && ((party_id != -1) || (npc_num == 0)) && 
		cheat.in_god_mode() && val < properties[prop])
		return;
	switch (static_cast<Item_properties>(prop))
		{
	case exp:
		{			// Experience?  Check for new level.
		int old_level = get_level();
		properties[static_cast<int>(exp)] = val;
		int delta = get_level() - old_level;
		if (delta > 0)
			properties[static_cast<int>(training)] += 3*delta;
		break;
		}
	case food_level:
		if (val > 36)		// Looks like max. in usecode.
			val = 36;
		else if (val < 0)
			val = 0;
		properties[prop] = val;
		break;
	case combat:			// These two are limited to 30.
	case magic:
		properties[prop] = val > 30 ? 30 : val;
		break;
	case training:			// Don't let this go negative.
		properties[prop] = val < 0 ? 0 : val;
		break;
	default:
		if (prop >= 0 && prop < 12)
			properties[prop] = val;
		break;
		}
	if (gumpman->showing_gumps())
		gwin->set_all_dirty();
	}

/*
 *	A class whose whole purpose is to stop casting mode.
 */
class Clear_casting : public Time_sensitive
	{
public:
	Clear_casting()
		{  }
	virtual void handle_event(unsigned long curtime, long udata);
	};
void Clear_casting::handle_event(unsigned long curtime, long udata)
	{ 
	Actor *a = reinterpret_cast<Actor*>(udata);
	a->set_casting_mode(Actor::not_casting);
	a->add_dirty();
	delete this;
	}

void Actor::end_casting_mode (int delay)
	{
	Clear_casting *c = new Clear_casting();
	gwin->get_tqueue()->add(Game::get_ticks() + 2 * delay, c, 
				reinterpret_cast<long>(this));
	}

/*
 *	A class whose whole purpose is to clear the 'hit' flag.
 */
class Clear_hit : public Time_sensitive
	{
public:
	Clear_hit()
		{  }
	virtual void handle_event(unsigned long curtime, long udata);
	};
void Clear_hit::handle_event(unsigned long curtime, long udata)
	{ 
	Actor *a = reinterpret_cast<Actor*>(udata);
	a->hit = false;
	a->add_dirty();
	delete this;
	}

/*
 *	This method should be called to decrement health from attacks, traps.
 *
 *	Output:	true if defeated.
 */

bool Actor::reduce_health
	(
	int delta,			// # points to lose.
	Actor *attacker			// Attacker, or null.
	)
	{
	if (cheat.in_god_mode() && ((party_id != -1) || (npc_num == 0)))
		return false;
	Monster_info *minf = get_info().get_monster_info();
	if (minf && minf->cant_die())	// In BG, this is Batlin/LB.
		delta = 0;		// Let them appear to be wounded.
					// Watch for Skara Brae ghosts.
	if (npc_num > 0 && Game::get_game_type() == BLACK_GATE &&
			get_info().has_translucency() &&
			party_id < 0)	// Don't include Spark here!!
		return false;
					// Being a bully (in BG)?
	if (attacker && attacker->is_in_party() && GAME_BG &&
	    npc_num > 0 &&
	    (alignment == Actor::friendly || alignment == Actor::neutral) &&
	    !(flags & (1<<Obj_flags::charmed)) && !is_in_party() &&
	    get_info().get_shape_class() == Shape_info::human)
		{
		static long lastcall = 0L;	// Last time yelled.
		long curtime = SDL_GetTicks();
		long delta = curtime - lastcall;
		if (delta > 10000)	// Call if 10 secs. has passed.
			{
			eman->remove_text_effect(this);
			say(first_call_police, last_call_police);
			lastcall = curtime;
			gwin->attack_avatar(1 + rand()%2);
			}
		else if (rand()%4 == 0)
			{
			cout << "Rand()%4" << endl;
			gwin->attack_avatar(1 + rand()%2);
			}
		}
	bool defeated = false;
	int oldhp = properties[static_cast<int>(health)];
	int maxhp = properties[static_cast<int>(strength)];
	int val = oldhp - delta;
	properties[static_cast<int>(health)] = val;
	if (this == gwin->get_main_actor() && val < maxhp/8 &&
					// Flash red if Avatar badly hurt.
	    rand()%2)
		gwin->get_pal()->flash_red();
	else
		{
		hit = true;		// Flash red outline.
		add_dirty();
		Clear_hit *c = new Clear_hit();
		gwin->get_tqueue()->add(Game::get_ticks() + 200, c, 
					reinterpret_cast<long>(this));
		}
	if (oldhp >= maxhp/2 && val < maxhp/2 && rand()%2 != 0)
		{			// A little oomph.
					// Goblin?
		if (GAME_SI &&
			 (get_shapenum() == 0x1de ||
			  get_shapenum() == 0x2b3 ||
			  get_shapenum() == 0x2d5 ||
			  get_shapenum() == 0x2e8))
			say(0x4d2, 0x4da);
		else if (!minf || !minf->cant_yell())
			say(first_ouch, last_ouch);
		}
	Game_object_vector vec;		// Create blood.
	const int blood = 912;
	if (delta >= 3 && (!minf || !minf->cant_bleed()) &&
	    rand()%2 && find_nearby(vec, blood, 1, 0) < 2)
		{			// Create blood where actor stands.
		Game_object *bobj = gmap->create_ireg_object(blood, 0);
		bobj->set_flag(Obj_flags::is_temporary);
		bobj->move(get_tile());
		}
	if (Actor::is_dying())
		{
		if (get_flag(Obj_flags::si_tournament))
			{
			ucmachine->call_usecode(get_usecode(), this, 
							Usecode_machine::died);
				// Still 'tournament'?  Set hp = 1.
			if (!is_dead() && get_flag(Obj_flags::si_tournament) &&
			    get_property(static_cast<int>(health)) < 1)
				{
				set_property(static_cast<int>(health), 1);
				if (get_attack_mode() == Actor::flee)
					defeated = true;
				}
			}
		else
			die(attacker);
		defeated = defeated || is_dead();
		}
	else if (val < 0 && !get_flag(Obj_flags::asleep) &&
					!get_flag(Obj_flags::si_tournament))
		set_flag(Obj_flags::asleep);
	return defeated;
	}

/*
 *	Get a property, modified by flags.
 */

int Actor::get_effective_prop
	(
	int prop				// Property #.
	) const
	{
	int val = properties[prop];
	switch (prop)
		{
	case Actor::dexterity:
	case Actor::intelligence:
	case Actor::combat:
	case Actor::strength:
		if (get_flag(Obj_flags::might))
			val += (val < 15 ? val : 15);	// Add up to 15.
		if (get_flag(Obj_flags::cursed))
			val /= 2;
		break;
		}
	return val;
	}

/*
 *	For mods and new games:  Set generic attribute keyed by a name.
 */

void Actor::set_attribute
	(
	const char *nm,
	int val
	)
	{
	if (!atts)
		atts = new Actor_attributes;
	atts->set(nm, val);
	}

/*
 *	Get generic attribute.  Returns 0 if not set.
 */

int Actor::get_attribute
	(
	const char *nm
	)
	{
	return atts ? atts->get(nm) : 0;
	}

/*
 *	Get them all.
 */

void Actor::get_attributes
	(
	Atts_vector& attlist
	)
	{
	attlist.resize(0);
	if (atts)
		atts->get_all(attlist);
	}

/*
 *	Read in attributes (from savegame file).
 */

void Actor::read_attributes
	(
	unsigned char *buf,		// Attribute/value pairs.
	int len
	)
	{
	unsigned char *ptr = buf, *endbuf = buf + len;
	while (ptr < endbuf)
		{
		char *att = (char *) ptr;
		ptr += strlen(att) + 1;
		assert(ptr + 2 <= endbuf);
		int val = Read2(ptr);
		set_attribute(att, val);
		}
	}

/*
 *	Set flag.
 */

void Actor::set_flag
	(
	int flag
	)
	{
//	cout << "Set flag for NPC " << get_npc_num() << " = " << flag << endl;

	if (flag >= 0 && flag < 32)
		flags |= ((uint32) 1 << flag);
	else if (flag >= 32 && flag < 64)
		flags2 |= ((uint32) 1 << (flag-32));
	switch (flag)
		{
	case Obj_flags::asleep:
					// Check sched. to avoid waking
					//   Penumbra.
		if (schedule_type == Schedule::sleep)
			break;
					// Set timer to wake in a few secs.
		need_timers()->start_sleep();
		if ((get_framenum()&0xf) != Actor::sleep_frame &&
					// Watch for slimes.
		    !get_info().has_strange_movement() &&
		    get_shapenum() > 0)	// (In case not initialized.)
					// Lie down.
			change_frame(Actor::sleep_frame + ((rand()%4)<<4));
		set_action(0);		// Stop what you're doing.
		break;
	case Obj_flags::poisoned:
		need_timers()->start_poison();
		break;
	case Obj_flags::protection:
		need_timers()->start_protection();
		break;
	case Obj_flags::might:
		need_timers()->start_might();
		break;
	case Obj_flags::cursed:
		need_timers()->start_curse();
		break;
	case Obj_flags::charmed:
		need_timers()->start_charm();
		set_target(0);		// Need new opponent if in combat.
		break;
	case Obj_flags::paralyzed:
		need_timers()->start_paralyze();
		break;
	case Obj_flags::invisible:
		need_timers()->start_invisibility();
		gclock->set_palette();
		break;
	case Obj_flags::dont_move:
	case Obj_flags::bg_dont_move:
		stop();			// Added 7/6/03.
		set_action(0);	// Force actor to stop current action.
		break;
		}
					// Update stats if open.
	if (gumpman->showing_gumps())
		gwin->set_all_dirty();
	set_actor_shape();
	}

void Actor::set_siflag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		siflags |= (static_cast<uint32>(1) << flag);

	set_actor_shape();
	}

void Actor::set_type_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 16)
		type_flags |= (static_cast<uint32>(1) << flag);

	set_actor_shape();
	}

/*
 *	Clear flag.
 */

void Actor::clear_flag
	(
	int flag
	)
	{
//	cout << "Clear flag for NPC " << get_npc_num() << " = " << flag << endl;
	if (flag >= 0 && flag < 32)
		flags &= ~(static_cast<uint32>(1) << flag);
	else if (flag >= 32 && flag < 64)
		flags2 &= ~(static_cast<uint32>(1) << (flag-32));
	if (flag == Obj_flags::invisible)	// Restore normal palette.
		gclock->set_palette();
	else if (flag == Obj_flags::asleep)
		{
		if (schedule_type == Schedule::sleep)
			set_schedule_type(Schedule::stand);
		else if ((get_framenum()&0xf) == Actor::sleep_frame)
			{		// Find spot to stand.
			Tile_coord pos = get_tile();
			pos.tz -= pos.tz%5;	// Want floor level.
			pos = Map_chunk::find_spot(pos, 6, get_shapenum(),
				Actor::standing, 0);
			if (pos.tx >= 0)
				move(pos);
			change_frame(Actor::standing);
			}
		}
	else if (flag == Obj_flags::charmed)
		set_target(0);			// Need new opponent.
	else if ((GAME_BG && flag == Obj_flags::bg_dont_move) ||
			flag == Obj_flags::dont_move)
		// Start again after a little while
		start(gwin->get_std_delay(), gwin->get_std_delay());
	
	set_actor_shape();
	}

void Actor::clear_siflag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		siflags &= ~(static_cast<uint32>(1) << flag);

	set_actor_shape();
	}

void Actor::clear_type_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 16)
		type_flags &= ~(static_cast<uint32>(1) << flag);

	set_actor_shape();
	}

/*
 *	Get flags.
 */

int Actor::get_siflag
	(
	int flag
	) const
	{
	return (flag >= 0 && flag < 32) ? (siflags & (static_cast<uint32>(1) << flag))
			!= 0 : 0;
	}

int Actor::get_type_flag
	(
	int flag
	) const
	{
	return (flag >= 0 && flag < 16) ? (type_flags & (static_cast<uint32>(1) << flag))
			!= 0 : 0;
	}
/*
 *	SetFlags
 */

void Actor::set_type_flags
	(
	unsigned short tflags
	)
	{
	type_flags = tflags;
	set_actor_shape();
	}

/*
 *	Set temperature.
 */

void Actor::set_temperature
	(
	int v				// Should be 0-63.
	)
	{
	if (v < 0)
		v = 0;
	else if (v > 63)
		v = 63;
	temperature = v;
	}

/*
 *	Figure warmth based on what's worn.  In trying to mimic the original
 *	SI, the base value is -75.
 */

int Actor::figure_warmth
	(
	)
	{
	static short hats[5] = {10, 35, -8, 20, 35};
	static short cloaks[5] = {30, 70, 70, 70, 70};
	static short boots[7] = {85, 65, 50, 90, 85, 75, 80};

	int warmth = -75;		// Base value.
	int frnum;
	Game_object *worn = spots[static_cast<int>(head)];
	if (worn && worn->get_shapenum() == 1004 &&
	    (frnum = worn->get_framenum()) < sizeof(hats)/sizeof(hats[0]))
		warmth += hats[frnum];
	if (worn && worn->get_shapenum() == 1013)
		warmth += hats[1]; // Helm of Light behaves like fur hat
	worn = spots[static_cast<int>(cloak_spot)];	// Cloak.
	if (worn && worn->get_shapenum() == 227 &&
	    (frnum = worn->get_framenum()) < sizeof(cloaks)/sizeof(cloaks[0]))
		warmth += cloaks[frnum];
	worn = spots[static_cast<int>(feet)];
	if (worn && worn->get_shapenum() == 587 &&
	    (frnum = worn->get_framenum()) < sizeof(boots)/sizeof(boots[0]))
		warmth += boots[frnum];
					// Leather armor?
	worn = spots[static_cast<int>(torso)];
	if (worn && worn->get_shapenum() == 569)
		warmth += 20;
	worn = spots[static_cast<int>(hands2_spot)];// Gloves?
	if (worn && worn->get_shapenum() == 579)
		warmth += 7;
	worn = spots[static_cast<int>(legs)];	// Legs?
	if (worn)
		switch (worn->get_shapenum())
			{
		case 686:		// Magic leggings.
			warmth += 5; break;
		case 574:		// Leather.
			warmth += 10; break;
			}
	return warmth;
	}

/*
 *	Get maximum weight in stones that can be held.
 *
 *	Output:	Max. allowed, or 0 if no limit (i.e., not carried by an NPC).
 */

int Actor::get_max_weight
	(
	)
	{
	return 2*get_effective_prop(static_cast<int>(Actor::strength));
	}

/*
 *	Call usecode function for an object that's readied/unreadied.
 */

void Actor::call_readied_usecode
	(
	int index,
	Game_object *obj,
	int eventid
	)
	{
					// Limit to certain types.
	if (!Shapeinfo_lookup::get_usecode_events(obj->get_shapenum()))
		return;

	Shape_info& info = obj->get_info();
	if (info.get_shape_class() != Shape_info::container)
		{
		Ready_type type = (Ready_type) info.get_ready_type();
		if (type != other)
			ucmachine->call_usecode(obj->get_usecode(),
			    obj, (Usecode_machine::Usecode_events) eventid);
		}
	}

/*
 *	Attack using the usecode_target and usecode_weapon fields set by
 *	the 'set_to_attack' intrinsic.
 *	Note:	I think this is only for weapons that fire (jsf).
 */
bool Actor::in_usecode_control() const
	{
	if (GAME_BG && get_flag(Obj_flags::bg_dont_move)
			|| get_flag(Obj_flags::dont_move))
		return true;
	Usecode_script *scr = 0;
	Actor *act = const_cast<Actor *>(this);
	while ((scr = Usecode_script::find(act, scr)) != 0)
		// no_halt scripts seem not to prevent movement.
		if (scr->is_activated() && !scr->is_no_halt())
			return true;
	return false;
	}

/*
 *	Attack using the usecode_target and usecode_weapon fields set by
 *	the 'set_to_attack' intrinsic.
 *	Note:	I think this is only for weapons that fire (jsf).
 */
void Actor::usecode_attack
	(
	)
	{
	if (!usecode_target)
		return;
	Shape_info& info = ShapeID::get_info(usecode_weapon);
	Weapon_info *winfo = info.get_weapon_info();
	Game_object *trg = usecode_target;
	usecode_target = 0;
	if (!winfo)
		return;
	int projectile_shape = winfo->get_projectile();
	int ammo_shape = winfo->get_ammo_consumed();
	// Not sure if we need all these.
	bool uses_charges = winfo->uses_charges() && info.has_quality();
	int strike_range = winfo->get_striking_range();
	int projectile_range = winfo->get_projectile_range();
	bool returns = winfo->returns();
	bool is_thrown = winfo->is_thrown();
	bool skip_render = false;
	if (ammo_shape)
		{
		if (!(ammo_shape = Combat_schedule::use_ammo(this, ammo_shape,
							projectile_shape)))
			{
			Mouse::mouse->flash_shape(Mouse::outofammo);
			return;
			}
		}
	else if (uses_charges)
		{
		Game_object *weapon = spots[static_cast<int>(lhand)];
		if (!weapon || weapon->get_shapenum() != usecode_weapon ||
					!weapon->get_quality())
			{
			weapon = spots[static_cast<int>(rhand)];
			if (!weapon || weapon->get_shapenum() != usecode_weapon||
					!weapon->get_quality())
				{
				Mouse::mouse->flash_shape(Mouse::outofammo);
				return;
				}
			}
		weapon->set_quality(weapon->get_quality() - 1);
		ammo_shape = projectile_shape;
		}
	else if (projectile_shape)
		ammo_shape = projectile_shape;
	else if (winfo->get_uses() == 3)
		ammo_shape = usecode_weapon;
	else
		{	// ammo_shape is still zero; see if usecode_weapon looks
			// like a projectile:
		ShapeID id(usecode_weapon, 0, SF_SHAPES_VGA);
		if (id.get_num_frames() >= 24)
			{
			ammo_shape = usecode_weapon;
			// I am interpreting the lack of projectile info as meaning
			// not to render the projectile:
			skip_render = true;
			}
		}

	if (ammo_shape)
		gwin->get_effects()->add_effect(
				new Projectile_effect(this, trg,
					ammo_shape, usecode_weapon, skip_render));
	}

/*
 *	Should be called after actors and usecode are initialized.
 */

void Actor::init_readied
	(
	)
	{
	if (spots[lfinger])
		call_readied_usecode(lfinger, spots[lfinger],
						Usecode_machine::readied);
	if (spots[rfinger])
		call_readied_usecode(rfinger, spots[rfinger],
						Usecode_machine::readied);
	if (spots[belt])
		call_readied_usecode(belt, spots[belt],
						Usecode_machine::readied);
	if (spots[neck])
		call_readied_usecode(neck, spots[neck],
						Usecode_machine::readied);
	if (spots[head])
		call_readied_usecode(head, spots[head],
						Usecode_machine::readied);
	if (spots[hands2_spot])
		call_readied_usecode(hands2_spot, 
				spots[hands2_spot], Usecode_machine::readied);
	if (spots[lhand])
		call_readied_usecode(lhand, spots[lhand],
						Usecode_machine::readied);
	if (spots[rhand])
		call_readied_usecode(rhand, spots[rhand],
						Usecode_machine::readied);
	}

/*
 *	Remove an object.
 */

void Actor::remove
	(
	Game_object *obj
	)
	{
	int index = Actor::find_readied(obj);	// Remove from spot.
					// Note:  gwin->drop() also does this,
					//   but it needs to be done before
					//   removal too.
					// Definitely DO NOT call if dead!
	if (!is_dead() && !ucmachine->in_usecode_for(
					obj, Usecode_machine::unreadied))
		call_readied_usecode(index, obj, Usecode_machine::unreadied);
	Container_game_object::remove(obj);
	if (index >= 0)
		{			// Update light-source count.
		if (obj->get_info().is_light_source())
			light_sources--;
		spots[index] = 0;
		if (index == rhand || index == lhand)
			two_handed = false;
		if (index == rfinger || index == lfinger)
			two_fingered = false;
		if (index == lhand && schedule)
			schedule->set_weapon(true);	
		}
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

bool Actor::add
	(
	Game_object *obj,
	bool dont_check,		// 1 to skip volume check (AND also
					//   to skip usecode call).
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
	{

	int index, a; 
	FIS_Type type;
	get_prefered_slots (obj, index, a, type);
	index = find_best_spot(obj);// Where should it go?
		
	if (index < 0)			// No free spot?  Look for a bag.
	{
		if (spots[back] && spots[back]->add(obj, false, combine))
			return true;
		if (spots[belt] && spots[belt]->add(obj, false, combine))
			return true;
		if (spots[lhand] && spots[lhand]->add(obj, false, combine))
			return true;
		if (spots[rhand] && spots[rhand]->add(obj, false, combine))
			return true;
		if (!dont_check)
			return false;

		// try again without checking volume/weight
		if (spots[back] && spots[back]->add(obj, true, combine))
			return true;
		if (spots[belt] && spots[belt]->add(obj, true, combine))
			return true;
		if (spots[lhand] && spots[lhand]->add(obj, true, combine))
			return true;
		if (spots[rhand] && spots[rhand]->add(obj, true, combine))
			return true;

		if (party_id != -1 || npc_num==0) {
			CERR("Warning: adding object (" << obj << ", sh. " << obj->get_shapenum() << ", " << obj->get_name() << ") to actor container (npc " << npc_num << ")"); 
		}
		return Container_game_object::add(obj, dont_check, combine);
	}
					// Add to ourself (DON'T combine).
	if (!Container_game_object::add(obj, true))
		return false;

	if (type == FIS_2Hand)		// Two-handed?
		two_handed = true;
	if (type == FIS_2Finger) {	// Gloves?
		index = lfinger;
		two_fingered = true;
	}

	spots[index] = obj;		// Store in correct spot.
	if (index == lhand && schedule)
		schedule->set_weapon();	// Tell combat-schedule about it.
	obj->set_shape_pos(0, 0);	// Clear coords. (set by gump).
	if (!dont_check)
		call_readied_usecode(index, obj, Usecode_machine::readied);
					// (Readied usecode now in drop().)
	if (obj->get_info().is_light_source())
		light_sources++;
	return true;
	}

/*
 *	Add to given spot.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::add_readied
	(
	Game_object *obj,
	int index,			// Spot #.
	int dont_check,
	int force_pos
	)
{
	// Is Out of range?
	if (index < 0 || index >= static_cast<int>(sizeof(spots)/sizeof(spots[0])))
		return (0);		

	// Already something there? Try to drop into it.
	// +++++Danger:  drop() can potentially delete the object.
//	if (spots[index]) return (spots[index]->drop(obj));
	if (spots[index]) return (spots[index]->add(obj));

	int prefered;
	int alternate;
	FIS_Type type;

	// Get the preferences
	get_prefered_slots (obj, prefered, alternate, type);
	
	// Check Prefered
	if (!fits_in_spot (obj, index, type) && !force_pos) return 0;

	// No room, or too heavy.
	if (!Container_game_object::add(obj, 1)) return 0;

	// Set the spot to this object
	spots[index] = obj;

	// Clear coords. (set by gump).
	obj->set_shape_pos(0, 0);

	// Must be a two-handed weapon.
	if (type == FIS_2Hand && index == lhand) two_handed = true;

	// Must be gloves
	if (type == FIS_2Finger && index == lfinger) two_fingered = true;

	if (!dont_check)
		call_readied_usecode(index, obj, Usecode_machine::readied);
	// Lightsource?
	if (obj->get_info().is_light_source()) light_sources++;

	if (index == lhand && schedule)
		schedule->set_weapon();	// Tell combat-schedule about it.
	return 1;
}

/*
 *	Find index of spot where an object is readied.
 *
 *	Output:	Index, or -1 if not found.
 */

int Actor::find_readied
	(
	Game_object *obj
	)
	{
	for (size_t i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		if (spots[i] == obj)
			return (i);
	return (-1);
	}

/*
 *	Change shape of a member.
 */

void Actor::change_member_shape
	(
	Game_object *obj,
	int newshape
	)
	{
	if (obj->get_info().is_light_source())
		light_sources--;
	Container_game_object::change_member_shape(obj, newshape);
	if (obj->get_info().is_light_source())
		light_sources++;
	}

/*
 *	Step aside to a free tile, or try to swap places.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::move_aside
	(
	Actor *for_actor,		// Moving aside for this one.
	int dir				// Direction to avoid (0-7).
	)
	{
	Tile_coord cur = get_tile();
	int opp = (dir + 4)%8;		// Don't go in opposite dir. either.
	Tile_coord to(-1, -1, -1);
	int i;
	for (i = 0; i < 8; i++)		// Go through directions.
		if (i == dir || i == opp)
			continue;	// Don't go that way.
		else
			{
			to = cur.get_neighbor(i);
					// Assume height = 3.
			if (!Map_chunk::is_blocked(
						to, 3, get_type_flags()))
				break;
			}
	int stepdir = i;		// This is the direction.
	if (i == 8 || to.tx < 0)	// Failed?  Try to swap places.
		return swap_positions(for_actor);
					// Step, and face direction.
	step(to, get_dir_framenum(stepdir,static_cast<int>(Actor::standing)));
	Tile_coord newpos = get_tile();
	return (newpos.tx == to.tx && newpos.ty == to.ty);
	}

/*
 *	Get frame if rotated 1, 2, or 3 quadrants clockwise.
 */

int Actor::get_rotated_frame
	(
	int quads			// 1=90, 2=180, 3=270.
	)
	{
	int curframe = get_framenum();
					// Bit 4=rotate180, 5=rotate-90.
	int curdir = (4 + 2*((curframe>>4)&1) - ((curframe>>5)&1))%4;
	int newdir = (curdir + quads)%4;
					// Convert to 8-value dir & get frame.
	return get_dir_framenum(2*newdir, curframe);
	}

/*
 *	Get total value of armor being worn.
 */

int Actor::get_armor_points
	(
	)
	{
	int points = 0;
	static enum Spots aspots[] = {head, neck, torso, cloak_spot, belt,
					lhand, rhand, lfinger, rfinger, legs, feet, ears_spot,
					hands2_spot};
	const int num_armor_spots = sizeof(aspots)/sizeof(aspots[0]);
	for (int i = 0; i < num_armor_spots; i++)
		{
		Game_object *armor = spots[static_cast<int>(aspots[i])];
		if (armor)
			points += armor->get_info().get_armor();
		}
	return points;
	}

/*
 *	Get whether or not the actor is immune or vulnerable to a form of damage.
 *
 *	Input is damage_type for a weapon.
 *
 *	Returns 1 if the actor is immune, -1 if vulnerable or 0 otherwise.
 */

int Actor::is_immune
	(
	int type
	)
	{
	Monster_info *minf = get_info().get_monster_info();
	int is_immune = 0;
	if (minf && minf->get_immune()&(1<<type))
		return 1;
	static enum Spots aspots[] = {head, neck, torso, cloak_spot, belt,
					lhand, rhand, lfinger, rfinger, legs, feet, ears_spot,
					hands2_spot};
	const int num_armor_spots = sizeof(aspots)/sizeof(aspots[0]);
	for (int i = 0; i < num_armor_spots; i++)
		{
		Game_object *armor = spots[static_cast<int>(aspots[i])];
		Armor_info *arinfo = armor ? armor->get_info().get_armor_info() : 0;
		if (arinfo && arinfo->get_immune()&(1<<type))
			return 1;
		}
	if (minf && minf->get_vulnerable()&(1<<type))
		return -1;
	else
		return 0;
	}

/*
 *	Get weapon value.
 */

Weapon_info *Actor::get_weapon
	(
	int& points,
	int& shape,
	Game_object *& obj		// ->weapon itself returned, or 0.
	)
	{
	points = 1;			// Bare hands = 1.
	Weapon_info *winf = 0;
	Game_object *weapon = spots[static_cast<int>(lhand)];
	obj = weapon;
	if (weapon)
		if ((winf = weapon->get_info().get_weapon_info()) != 0)
			{
			points = winf->get_damage();
			shape = weapon->get_shapenum();
			return winf;
			}
#if 0	/* (jsf: 17july2005) I don't think we should look at right hand. */
					// Try right hand.
	weapon = spots[static_cast<int>(rhand)];
	if (weapon)
		{
		Weapon_info *rwinf = weapon->get_info().get_weapon_info();
		int rpoints;
		if (rwinf && (rpoints = rwinf->get_damage()) > points)
			{
			winf = rwinf;
			points = rpoints;
			shape = weapon->get_shapenum();
			obj = weapon;
			}
		}
#endif
	return winf;
	}

/*
 *	Roll a 25-sided die to determine a win-lose outcome, adding a
 *	bias for the attacker and for the defender.
 */

bool Actor::roll_to_win
	(
	int attacker,			// Points added.
	int defender			// Points subtracted.
	)
	{
	const int sides = 25;
	int roll = rand()%sides;
	if (roll == 0)			// Always lose.
		return false;
	else if (roll == sides - 1)	// High?  Always win.
		return true;
	else
		return roll + attacker - defender >= sides/2;
	}

/*
 *	Get effective property, or default value.
 */

inline int Get_effective_prop
	(
	Actor *npc,			// ...or NULL.
	Actor::Item_properties prop,	// Property #.
	int defval = 0			// Default val if npc==0.
	)
	{
	return npc ? npc->get_effective_prop(prop) : defval;
	}

/*
 *	Figure hit points lost from an attack, and subtract from total.
 *
 *	Output:	True if defeated (dead, or lost battle on List Field).
 */

bool Actor::figure_hit_points
	(
	Actor *attacker,		// 0 if hit from a missile egg.
	int weapon_shape,		// Negative if from an explosion.
	int ammo_shape
	)
	{
	bool were_party = party_id != -1 || npc_num == 0;
	// godmode effects:
	if (were_party && cheat.in_god_mode())
		return false;
	bool theyre_party = attacker &&
			(attacker->party_id != -1 || attacker->npc_num == 0);
	bool instant_death = (cheat.in_god_mode() && theyre_party);
					// Modify using combat difficulty.
	int bias = were_party ? Combat::difficulty :
			(theyre_party ? -Combat::difficulty : 0);
	int armor = get_armor_points() - bias;
	int wpoints;
	Weapon_info *winf;
	if (weapon_shape > 0)
		{
		winf = ShapeID::get_info(weapon_shape).get_weapon_info();
		wpoints = winf ? winf->get_damage() : 0;
		}
	else if (ammo_shape > 0)
		{
		winf = ShapeID::get_info(ammo_shape).get_weapon_info();
		wpoints = winf ? winf->get_damage() : 0;
		}
	else
		{
		Game_object *w;
		winf = attacker->get_weapon(wpoints, weapon_shape, w);
		if (!winf)
			wpoints = 1;	// Give at least one, but only if there's no weapon
		}
	bool explosion = winf ? winf->explodes() : false;
					// Get bonus ammo points.
	Ammo_info *ainf = ammo_shape > 0 ? 
			ShapeID::get_info(ammo_shape).get_ammo_info() : 0;
	bool special_behaviour = false;
	if (ainf)
		{
		wpoints += ainf->get_damage();
		special_behaviour = ainf->has_special_behaviour();
		}
	int usefun;			// See if there's usecode for it.
	if (winf && (usefun = winf->get_usecode()) != 0)
		ucmachine->call_usecode(usefun, this,
					Usecode_machine::weapon);
					// Same for ammo.
	unsigned char powers = winf ? winf->get_powers() : 0;
	if (ainf)
		powers |= ainf->get_powers();
	if (attacker && (usefun ||
			(wpoints || powers) && !powers&Weapon_info::si_no_damage))
		{ 
		if (is_combat_protected() && party_id >= 0 &&
		    rand()%5 == 0)
			say(first_need_help, last_need_help);
					// Attack back, but not if in party.
		if (!target && !is_in_party())
			set_target(attacker,
			    attacker->get_schedule_type() != Schedule::duel);
		}
	if (!wpoints && !powers)
		return false;		// No harm can be done.
	
	int prob;
	if (weapon_shape == 621)
		//Give a better base chance for delayed blast spell to hit as
		//it does not depend on the attacker's stats:
		prob = 80;
	else 
		prob = 55;
	
	prob += 8*bias +
		2*Get_effective_prop(attacker, combat, 10) +
		Get_effective_prop(attacker, dexterity, 10) -
		2*Get_effective_prop(this, combat, 10) -
		Get_effective_prop(this, dexterity, 10);
	if (get_flag(Obj_flags::protection))// Defender is protected?
		prob -= (40 + rand()%20);
	if (prob < 4)			// Always give some chance.
		prob = 4;
	else if (prob > 96)
		prob = 96;
					// Attacked by Vesculio in SI?
	if (GAME_SI && attacker && attacker->npc_num == 294)
		{
		int pts, sh;		// Do we have Magebane?
		Game_object *w;
	    	if (get_weapon(pts, sh, w) && sh == 0xe7)
			{
			prob -= (70 + rand()%20);
			eman->remove_text_effect(attacker);
			attacker->say(magebane_struck);
			}
		}
	if (instant_death || special_behaviour)
		prob = 200;	// always hits

	if (combat_trace) {
		cout << "Hit probability is " << prob << endl;
	}
	bool missed = false;
	if (rand()%100 > prob)
		missed = true;			// Missed.
	if (!missed && (Game::get_game_type() == SERPENT_ISLE))
		{	// putting Draygan to sleep (better kludge).
		if (powers&Weapon_info::si_no_damage)
			{	// if ainf exists, may be SI sleep arrows.
				// Just in case, see if it is set to sleep:
			if (ainf && ainf->get_powers()&Weapon_info::sleep)
				ucmachine->call_usecode(0x7e1, this,
							Usecode_machine::weapon);
			return false;	//Causes no harm
			}
		}
	// See if we should drop ammo.
	unsigned char drop = ainf ? ainf->get_drop_type() : 0;
	if ((drop == Ammo_info::always_drop) || 
			((drop != Ammo_info::never_drop) && (missed)))
		{
		if (winf && ammo_shape && attacker &&
		    ((winf->is_thrown() && !winf->returns()) ||
			winf->get_ammo_consumed() > 0))
			{
			Tile_coord pos = Map_chunk::find_spot(get_tile(), 3,
							ammo_shape, 0, 1);
			if (pos.tx != -1)
				{
				Game_object *aobj = gmap->create_ireg_object(
									ammo_shape, 0);
				if (attacker->get_flag(	Obj_flags::is_temporary))
					aobj->set_flag(	Obj_flags::is_temporary);
				aobj->set_flag(Obj_flags::okay_to_take);
				aobj->move(pos);
				}
			}
		}
	if (missed)
		return false;
					// Compute hit points to lose.
	int attacker_str;
	if (explosion)	//Explosions shouldn't depend on strength
		attacker_str = 8;
	else
		attacker_str = Get_effective_prop(attacker, strength, 8);
		
	int hp;
	wpoints += 2*bias;		// Apply user's preference.
	if (wpoints > 0)		// Some ('curse') do no damage.
		{
		if ((wpoints == 127)	// Glass sword?
			|| (special_behaviour))	//Death Vortex/Energy Mist
			hp = wpoints;	// A killer.
		else {
			hp = 1 + rand()%wpoints;
			if (attacker_str > 2)
				hp += 1 + rand()%(attacker_str/3);
			if (armor > 0)
				hp -= 1 + rand()%armor;
		}
		if (hp < 1)
			hp = 1;
		}
	else
		hp = 0;
	Monster_info *minf = get_info().get_monster_info();
	int damage_type = winf ? winf->get_damage_type() : 0;
	if (ainf)
		{
		if (winf && winf->get_uses() == 3)	// Bows, crossbows, muskets
			damage_type = ainf->get_damage_type();
		else if (!winf)
			damage_type = ainf->get_damage_type();
		}
	switch (is_immune(damage_type))
		{
		case 1:		hp = 0; break;	// Is immune
		case -1:	hp *= 2; break;	// Is vulnerable
		default:	break;
		}
	if (powers)			// Special attacks?
		{
		if ((powers&Weapon_info::poison) && roll_to_win(
			Get_effective_prop(attacker, Actor::strength),
			Get_effective_prop(this, Actor::dexterity)) &&
			!(minf && minf->poison_safe()))
			set_flag(Obj_flags::poisoned);
		if (powers&Weapon_info::magebane)
			{
			int mana = properties[static_cast<int>(Actor::mana)];
			set_property(static_cast<int>(Actor::mana), 
					mana > 1 ? rand()%(mana - 1) : 0);
					// Vasculio the Vampire?
			if (npc_num == 294 && GAME_SI)
				hp *= 3;
			}
		if ((powers&Weapon_info::curse) && roll_to_win(
			Get_effective_prop(attacker, Actor::intelligence),
			Get_effective_prop(this, Actor::intelligence)))
			set_flag(Obj_flags::cursed);
		if ((powers&Weapon_info::charm) && roll_to_win(
			Get_effective_prop(attacker, Actor::intelligence),
			Get_effective_prop(this, Actor::intelligence)))
			set_flag(Obj_flags::charmed);
		if ((powers&Weapon_info::sleep) && roll_to_win(
			Get_effective_prop(attacker, Actor::intelligence),
			Get_effective_prop(this, Actor::intelligence)))
			set_flag(Obj_flags::asleep);
		if ((powers&Weapon_info::paralyze) && roll_to_win(
			Get_effective_prop(attacker, Actor::intelligence),
			Get_effective_prop(this, Actor::intelligence)))
			set_flag(Obj_flags::paralyzed);
		}
	int sfx;			// Play 'hit' sfx.
	if (winf && (sfx = winf->get_hitsfx()) >= 0 &&
					// But only if Ava. involved.
		(this == gwin->get_main_actor() || 
				attacker == gwin->get_main_actor()))
		Audio::get_ptr()->play_sound_effect(sfx);

	int oldhealth = properties[static_cast<int>(health)];
	int maxhealth = properties[static_cast<int>(strength)];

	if (instant_death)		//instant death
		hp = properties[static_cast<int>(health)] + 
				properties[static_cast<int>(strength)] + 1;
	int newhp = oldhealth - hp;	// Subtract from health.

	bool defeated = hp > 0 ? reduce_health(hp, attacker) : false;
	if (Combat::show_hits)
		{
		eman->remove_text_effect(this);
		char hpmsg[50];
		sprintf(hpmsg, "-%d(%d)", hp, newhp);
		eman->add_text(hpmsg, this);
		}
	string name = "<trap>";
	if (attacker)
		name = attacker->get_name();

	if (combat_trace) {
		cout << name << " hits " << get_name()
			 << " for " << hp << " hit points, leaving "
			 <<	properties[static_cast<int>(health)] << " remaining" << endl;
	}
	if (!defeated && minf && minf->splits() && rand()%2 == 0 && 
	    properties[static_cast<int>(health)] > 0)
		clone();

	return defeated;
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if defeated, else object itself.
 */

Game_object *Actor::attacked
	(
	Actor *attacker,		// 0 if from a trap.
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape			// Also may be 0.
	)
	{
	if (is_dead() ||		// Already dead?
					// Or party member of dead Avatar?
	    (party_id >= 0 && gwin->get_main_actor()->is_dead()))
		return 0;
	if (attacker)
		set_oppressor(attacker->get_npc_num());
	if (attacker && attacker->get_schedule_type() == Schedule::duel)
		return this;	// Just play-fighting.
					// Watch for Skara Brae ghosts.
	if (npc_num > 0 && Game::get_game_type() == BLACK_GATE &&
		get_info().has_translucency() && 
			party_id < 0)	// But don't include Spark!!
		return this;
	bool defeated = figure_hit_points(attacker, weapon_shape, ammo_shape);
	if (attacker && defeated)
		{	// ++++++++This should be in reduce_health()+++++++
					// Experience gained = strength???
		int expval = get_property(static_cast<int>(strength)) +
				get_property(static_cast<int>(combat))/4 +
				get_property(static_cast<int>(dexterity))/4 +
				get_property(static_cast<int>(intelligence))/4;
		expval /= 2;	// Users complain we're giving too much.
					// Attacker gains experience.
		attacker->set_property(static_cast<int>(exp),
		    attacker->get_property(static_cast<int>(exp)) + expval);
		return 0;
		}
	return this;
	}

/*
 *	There's probably a smarter way to do this, but this routine checks
 *	for the dragon Draco.
 */

static int Is_draco
	(
	Actor *dragon
	)
	{
	Game_object_vector vec;		// Gets list.
						// Should have a special scroll.
	int cnt = dragon->get_objects(vec, 797, 241, 4);
	return cnt > 0;
	}

/*
 *	We're dead.  We're removed from the world, but not deleted.
 */

void Actor::die
	(
	Actor *attacker
	)
	{
	// If the actor is already dead, we shouldn't do anything
	//(fixes a resurrection bug).
	if (Actor::get_flag(Obj_flags::dead))
		return;
					// Get location.
	Tile_coord pos = get_tile();
	set_action(0);
	delete schedule;
	schedule = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
	Actor::set_flag(Obj_flags::dead);// IMPORTANT:  Set this before moving
					//   objs. so Usecode(eventid=6) isn't
					//   called.
	int shnum = get_shapenum();
					// Special case:  Hook, Dracothraxus.
	if (((shnum == 0x1fa || (shnum == 0x1f8 && Is_draco(this))) && 
	    Game::get_game_type() == BLACK_GATE))
		{			// Exec. usecode before dying.
		ucmachine->call_usecode(shnum, this, 
					Usecode_machine::internal_exec);
		if (is_pos_invalid())	// Invalid now?
			return;
		}
	properties[static_cast<int>(health)] = -50;
	Shape_info& info = get_info();
	Monster_info *minfo = info.get_monster_info();
	remove_this(1);			// Remove (but don't delete this).
	set_invalid();
	Dead_body *body;		// See if we need a body.
	if (!minfo || !minfo->has_no_body())
		{
		int frnum;			// Lookup body shape/frame.
		if (!Shapeinfo_lookup::find_body(get_shapenum(), shnum, frnum))
			{
			shnum = 400;
			frnum = 3;
			}
					// Reflect if NPC reflected.
		frnum |= (get_framenum()&32);
		body = new Dead_body(shnum, frnum, 0, 0, 0, 
					npc_num > 0 ? npc_num : -1);
		if (npc_num > 0)
			{
			body->set_quality(1);	// Flag for dead body of NPC.
			gwin->set_body(npc_num, body);
			}
					// Tmp. monster => tmp. body.
		if (get_flag(Obj_flags::is_temporary))
			body->set_flag(Obj_flags::is_temporary);
					// Okay to take its contents.
		body->set_flag_recursively(Obj_flags::okay_to_take);
					// Find a spot within 1 tile.
		Tile_coord bp = Map_chunk::find_spot(pos, 1, shnum, frnum, 2);
		if (bp.tx == -1)
			bp = pos;	// Default to NPC pos, even if blocked.
		body->move(bp);
		}
	else
		body = 0;
	Game_object *item;		// Move/remove all the items.
	Game_object_vector tooheavy;	// Some shouldn't be moved.
	while ((item = objects.get_first()) != 0)
		{
		remove(item);
		item->set_invalid();
		if (!item->is_dragable())
			{
			tooheavy.push_back(item);
			continue;
			}
		if (body)
			body->add(item, 1);// Always succeed at adding.
		else			// No body?  Drop on ground.
			{
			item->set_flag_recursively(Obj_flags::okay_to_take);
			Tile_coord pos2 = Map_chunk::find_spot(pos, 5,
				item->get_shapenum(), item->get_framenum(), 1);
			if (pos.tx != -1)
				item->move(pos2);
			else		// No room anywhere.
				tooheavy.push_back(item);
			}
		}
					// Put the heavy ones back.
	for (Game_object_vector::const_iterator it = tooheavy.begin(); 
						it != tooheavy.end(); ++it)
		add(*it, 1);
	if (body)
		gwin->add_dirty(body);
	add_dirty();			// Want to repaint area.
	delete_contents();		// remove what's left of inventory
					// Is this a bad guy?
					// Party defeated an evil monster?
	if (attacker && attacker->is_in_party() && !is_in_party() && 
	    alignment != neutral && alignment != friendly)
		Combat_schedule::monster_died();
					// Move party member to 'dead' list.
	partyman->update_party_status(this);
	}

/*
 *	Create another monster of the same type as this, and adjacent.
 *
 *	Output:	->monster, or 0 if failed.
 */

Monster_actor *Actor::clone
	(
	)
	{
	Shape_info& info = get_info();
					// Base distance on greater dim.
	int xs = info.get_3d_xtiles(), ys = info.get_3d_ytiles();
					// Find spot.
	Tile_coord pos = Map_chunk::find_spot(get_tile(), 
		xs > ys ? xs : ys, get_shapenum(), 0, 1);
	if (pos.tx < 0)
		return 0;		// Failed.
					// Create, temporary & with equip.
	Monster_actor *monst = Monster_actor::create(
			get_shapenum(), pos, get_schedule_type(),
			get_alignment(), true, true);
	return monst;
	}

/*
 *	Restore HP's on the hour.
 */

void Actor::mend_hourly
	(
	)
	{
	if (is_dead())
		return;
	int maxhp = properties[static_cast<int>(strength)];
	int hp = properties[static_cast<int>(health)];
	if (maxhp > 0 && hp < maxhp)
		{
		if (maxhp >= 3)  
			hp += 1 + rand()%(maxhp/3);
		else
			hp += 1;
		if (hp > maxhp)
			hp = maxhp;
		properties[static_cast<int>(health)] = hp;
					// ??If asleep & hps now >= 0, should
					//   we awaken?
		}
					// Restore some mana also.
	int maxmana = properties[static_cast<int>(magic)];
	int curmana = properties[static_cast<int>(mana)];
	if (maxmana > 0 && curmana < maxmana)
		{
		if (maxmana >= 3)	
			curmana += 1 + rand()%(maxmana/3);
		else
			curmana += 1;
		properties[static_cast<int>(mana)] = curmana <= maxmana ? curmana 
								: maxmana;
		}
	}

/*
 *	Restore from body.  It must not be owned by anyone.
 *
 *	Output:	->actor if successful, else 0.
 */

Actor *Actor::resurrect
	(
	Dead_body *body			// Must be this actor's body.
	)
	{
	if (body->get_owner() ||	// Must be on ground.
	    npc_num <= 0 || gwin->get_body(npc_num) != body)
		return (0);
	gwin->set_body(npc_num, 0);	// Clear from gwin's list.
	Game_object *item;		// Get back all the items.
	while ((item = body->get_objects().get_first()) != 0)
		{
		body->remove(item);
		add(item, 1);		// Always succeed at adding.
		}
	gwin->add_dirty(body);		// Need to repaint here.
	Tile_coord pos = body->get_tile();
	body->remove_this();		// Remove and delete body.
	move(pos);			// Move back to life.
					// Restore health to max.
	properties[static_cast<int>(health)] = 
					properties[static_cast<int>(strength)];
	Actor::clear_flag(Obj_flags::dead);
	Actor::clear_flag(Obj_flags::poisoned);
	Actor::clear_flag(Obj_flags::paralyzed);
	Actor::clear_flag(Obj_flags::asleep);
					// Restore to party if possible.
	partyman->update_party_status(this);
					// Give a reasonable schedule.
	set_schedule_type(is_in_party() ? Schedule::follow_avatar
					: Schedule::loiter);
					// Stand up.
	Usecode_script *scr = new Usecode_script(this);
	(*scr) << (Ucscript::npc_frame + Actor::sleep_frame)
		<< (Ucscript::npc_frame + Actor::kneel_frame)
		<< (Ucscript::npc_frame + Actor::standing);
	scr->start(1);
	return (this);
	}

/*
 *	Handle a time event (for animation).
 */

void Main_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	if (action)			// Doing anything?
		{			// Do what we should.
		int delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			gwin->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			{
			set_action(0);
			if (schedule)
				schedule->now_what();
			}
		}
	else if (schedule)
		schedule->now_what();
	}

/*
 *	Get the party to follow.
 */

void Main_actor::get_followers
	(
	)
	{
	int cnt = partyman->get_count();
	for (int i = 0; i < cnt; i++)
		{
		Actor *npc = gwin->get_npc(partyman->get_member(i));
		if (!npc || npc->get_flag(Obj_flags::asleep) ||
		    npc->is_dead())
			continue;
		int sched = npc->get_schedule_type();
					// Skip if in combat or set to 'wait'.
		if (sched != Schedule::combat &&
		    sched != Schedule::wait &&
					// Loiter added for SI.
		    sched != Schedule::loiter)
			{
			if (sched != Schedule::follow_avatar)
				npc->set_schedule_type(
						Schedule::follow_avatar);
			else
				npc->follow(this);
			}
		}
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 */

int Main_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	rest_time = 0;			// Reset counter.
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
	Map_chunk *nlist = gmap->get_chunk(cx, cy);
	int old_lift = get_lift();
	int water, poison;		// Get tile info.
	get_tile_info(this, gwin, nlist, tx, ty, water, poison);
	Game_object *block;
	if (is_blocked(t) &&
	   (!(block = Game_object::find_blocking(t)) || block == this ||
					// Try to get blocker to move aside.
	        !block->move_aside(this, get_direction(block)) ||
					// (May have swapped places.)
		(t != get_tile() &&
					// If okay, try one last time.
		 is_blocked(t))))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		return (0);
		}
	if (poison && t.tz == 0)
		Actor::set_flag(static_cast<int>(Obj_flags::poisoned));
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty();			// Set to update old location.
					// Get old chunk, old tile.
	Map_chunk *olist = get_chunk();
	Tile_coord oldtile = get_tile();
					// Move it.
	Actor::movef(olist, nlist, tx, ty, frame, t.tz);
	add_dirty(1);			// Set to update new.
					// In a new chunk?
	if (olist != nlist)
		Main_actor::switched_chunks(olist, nlist);
	int roof_height = nlist->is_roof (tx, ty, t.tz);
	gwin->set_ice_dungeon(nlist->is_ice_dungeon(tx, ty));
	if (gwin->set_above_main_actor (roof_height))
		{
		gwin->set_in_dungeon(nlist->has_dungeon()?
					nlist->is_dungeon(tx, ty):0);
		gwin->set_all_dirty();
		}
	else if (roof_height < 31 && gwin->set_in_dungeon(nlist->has_dungeon()?
 					nlist->is_dungeon(tx, ty):0))
		gwin->set_all_dirty();
					// Near an egg?  (Do this last, since
					//   it may teleport.)
	nlist->activate_eggs(this, t.tx, t.ty, t.tz,
						oldtile.tx, oldtile.ty);
	return (1);
	}

/*
 *	Setup cache after a change in chunks.
 */

void Main_actor::switched_chunks
	(
	Map_chunk *olist,	// Old chunk, or null.
	Map_chunk *nlist	// New chunk.
	)
	{
	int newcx = nlist->get_cx(), newcy = nlist->get_cy();
	int xfrom, xto, yfrom, yto;	// Get range of chunks.
	if (!olist ||			// No old, or new map?  Use all 9.
	     olist->get_map() != nlist->get_map())
		{
		xfrom = newcx > 0 ? newcx - 1 : newcx;
		xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
		yfrom = newcy > 0 ? newcy - 1 : newcy;
		yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
		}
	else
		{
		int oldcx = olist->get_cx(), oldcy = olist->get_cy();
		if (newcx == oldcx + 1)
			{
			xfrom = newcx;
			xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
			}
		else if (newcx == oldcx - 1)
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx;
			}
		else
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
			}
		if (newcy == oldcy + 1)
			{
			yfrom = newcy;
			yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
			}
		else if (newcy == oldcy - 1)
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy;
			}
		else
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
			}
		}
	for (int y = yfrom; y <= yto; y++)
		for (int x = xfrom; x <= xto; x++)
			nlist->get_map()->get_chunk(x, y)->setup_cache();

	// If change in Superchunk number, apply Old Style caching emulation
	gwin->emulate_cache(olist, nlist);
	}

/*
 *	Move (teleport) to a new spot.
 */

void Main_actor::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
					// Store old chunk list.
	Map_chunk *olist = get_chunk();
					// Move it.
	Actor::move(newtx, newty, newlift, newmap);
	Map_chunk *nlist = get_chunk();
	if (nlist != olist)
		Main_actor::switched_chunks(olist, nlist);
	int tx = get_tx(), ty = get_ty();
	gwin->set_ice_dungeon(nlist->is_ice_dungeon(tx, ty));
	if (gwin->set_above_main_actor(nlist->is_roof(tx, ty, newlift)))
		gwin->set_in_dungeon(nlist->has_dungeon()?
		nlist->is_dungeon(tx, ty):0);
	}

/*
 *	We're dead.
 */

void Main_actor::die
	(
	Actor * /* attacker */
	)
	{
	if (gwin->in_combat())
		gwin->toggle_combat();	// Hope this is safe....
	Actor::set_flag(Obj_flags::dead);
	gumpman->close_all_gumps();	// Obviously.
					// Special function for dying:
	if (Game::get_game_type() == BLACK_GATE)
		ucmachine->call_usecode(0x60e, this, Usecode_machine::weapon);

		else
			ucmachine->call_usecode(0x400, this,
					Usecode_machine::died);
	}

/*
 *	Set the shapenum based on skin color, sex, naked flag and petra and polymorph flag
 */
void Actor::set_actor_shape()
{
	if (get_npc_num() != 0 || get_flag (Obj_flags::polymorph))
		return;

	int sn;

	ShapeFile the_file = SF_SHAPES_VGA;
	int female = get_type_flag(tf_sex)?1:0;

	if (Game::get_game_type() == SERPENT_ISLE||sman->can_use_multiracial())
	{
		if (Game::get_game_type() == BLACK_GATE) the_file = SF_BG_SISHAPES_VGA;

		if (get_skin_color() == 0) // WH
		{
			sn = 1028+female+6*get_siflag(naked);
		}
		else if (get_skin_color() == 1) // BN
		{
			sn = 1026+female+6*get_siflag(naked);
		}
		else if (get_skin_color() == 2) // BK
		{
			sn = 1024+female+6*get_siflag(naked);
		}
		else if (Game::get_game_type() == SERPENT_ISLE)
		{
			sn = female?658:747;
		}
		else
		{
			the_file = SF_SHAPES_VGA;
			sn = female?989:721;
		}
	}
	else if (female)
		sn = 989;
	else
		sn = 721;
#ifdef DEBUG
	cerr << "Setting Shape to " << sn << endl;
#endif
	set_shape (sn, get_framenum());
	set_file(the_file);
}

// Sets the polymorph to shape
void Actor::set_polymorph (int shape)
{
#ifdef DEBUG
	cerr << "Setting polymorph for " << get_npc_num() << endl;
	cerr << "Shape " << shape << endl;
	cerr << "Save shape " << shape_save << endl;
#endif
	
	// Want to set to Avatar
	ShapeFile the_file = SF_SHAPES_VGA;
	if (shape == 721 || shape == 989)
	{
		Actor *avatar = gwin->get_main_actor();
		if (!avatar) return;

		int female = get_type_flag(tf_sex)?1:0;

		if (Game::get_game_type() == SERPENT_ISLE||sman->can_use_multiracial())
		{
			if (Game::get_game_type() == BLACK_GATE) the_file = SF_BG_SISHAPES_VGA;

			if (get_skin_color() == 0) // WH
				shape = 1028+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
			else if (get_skin_color() == 1) // BN
				shape = 1026+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
			else if (get_skin_color() == 2) // BK
				shape = 1024+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
		}
	}
	set_file(the_file);

	if (shape == shape_save)
	{
		set_shape (shape_save);
		shape_save = -1;
		clear_flag(Obj_flags::polymorph);
		return;
	}
	if (shape_save == -1) shape_save = get_shapenum();
//	set_shape (shape, get_framenum());
					// ++++Taking a guess for SI amulet:
	set_shape (shape, get_dir_framenum(Actor::standing));
	set_flag (Obj_flags::polymorph);
}

// Sets polymorph shape to defaults based on flags and npc num
void Actor::set_polymorph_default()
{
	if (!get_flag(Obj_flags::polymorph)
			|| (get_npc_num() != 0 &&
				(GAME_SI && get_npc_num() != 28)))
		return;

	set_actor_shape();

	shape_save = get_shapenum();

	if (get_npc_num() == 28)		// Handle Petra First
		set_polymorph (721);
	else if (get_flag(Obj_flags::petra))	// Avatar as petra
		set_polymorph (658);
	else	// Snake
		set_polymorph (530);
}

/*
 *	Get 'real' shape for Usecode.
 */

int Actor::get_shape_real
	(
	)
	{
#if 0	/* Messes up start of BG after earthquake if you use an SI shape */
	if (Game::get_game_type() == BLACK_GATE)
		return get_shapenum();
#endif
	if (npc_num != 0)		// Not the avatar?
		return shape_save!=-1?shape_save:get_shapenum();
					// Taking guess (6/18/01):
	if (get_type_flag(Actor::tf_sex))
		return 989;
	else
		return 721;
	}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	const std::string &nm, 			// Name.  A copy is made.
	int shapenum, 
	int num, 
	int uc
	) : Actor(nm, shapenum, num, uc), nearby(false),
		num_schedules(0),
		schedules(0)
	{
	}

/*
 *	Kill an actor.
 */

Npc_actor::~Npc_actor
	(
	)
	{
	delete [] schedules;
	}

/*
 *	Set schedule list.
 */

void Npc_actor::set_schedules
	(
	Schedule_change *sc_list, 
	int cnt
	)
	{
	delete [] schedules;
	schedules = sc_list;
	num_schedules = cnt;
	}

/*
 *	Set schedule type.
 */

void Npc_actor::set_schedule_time_type (int time, int type)
{
	Tile_coord tile;
	int i;

	for (i = 0; i < num_schedules; i++) if (schedules[i].get_time() == time) break;
	
	if (i == num_schedules)	// Didn't find it
	{
		Schedule_change *scheds = new Schedule_change[num_schedules+1];

		for (i = 0; i < num_schedules; i++)
		{
			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, tile.tz,
			    schedules[i].get_type(), schedules[i].get_time());
		}

		scheds[num_schedules].set(0, 0, 0,
			static_cast<unsigned char>(type),
			static_cast<unsigned char>(time));
		set_schedules(scheds, num_schedules+1);
	}
	else	// Did find it
	{
		tile = schedules[i].get_pos();
		schedules[i].set(tile.tx, tile.ty, tile.tz,
					static_cast<unsigned char>(type),
					static_cast<unsigned char>(time));
	}
}

/*
 *	Set schedule location.
 */

void Npc_actor::set_schedule_time_location (int time, int x, int y)
{
	int i;

	for (i = 0; i < num_schedules; i++) if (schedules[i].get_time() == time) break;
	
	if (i == num_schedules)	// Didn't find it
	{
		Tile_coord tile;
		Schedule_change *scheds = new Schedule_change[num_schedules+1];

		for (i = 0; i < num_schedules; i++)
		{
			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, tile.tz,
			    schedules[i].get_type(), schedules[i].get_time());
		}

		scheds[num_schedules].set(x, y, 0, 0, 
					static_cast<unsigned char>(time));
		set_schedules(scheds, num_schedules+1);
	}
	else	// Did find it
	{
		schedules[i].set(x, y, 0,
		  schedules[i].get_type(), static_cast<unsigned char>(time));
	}
}

/*
 *	Remove schedule
 */

void Npc_actor::remove_schedule (int time)
{
	int i;

	for (i = 0; i < num_schedules; i++) 
		if (schedules[i].get_time() == time) break;
	if (i != num_schedules)	// Found it
	{
		int todel = i;
		Tile_coord tile;
		Schedule_change *scheds = new Schedule_change[num_schedules-1];

		for (i = 0; i < todel; i++)
		{
			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, tile.tz,
			    schedules[i].get_type(), schedules[i].get_time());
		}

		for (; i < num_schedules - 1; i++)
		{
			tile = schedules[i+1].get_pos();
			scheds[i].set(tile.tx, tile.ty, tile.tz,
					schedules[i+1].get_type(), 
					schedules[i+1].get_time());
		}

		set_schedules(scheds, num_schedules-1);
	}
}

/*
 *	Set schedule list.
 */

void Npc_actor::get_schedules
	(
	Schedule_change *&sc_list, 
	int &cnt
	)
	{
	sc_list = schedules;
	cnt = num_schedules;
	}
/*
 *	Move and change frame.
 */

void Npc_actor::movef
	(
	Map_chunk *old_chunk,
	Map_chunk *new_chunk, 
	int new_sx, int new_sy,
	int new_frame, 
	int new_lift
	)
	{
	Actor::movef(old_chunk, new_chunk,
				new_sx, new_sy, new_frame, new_lift);
	if (old_chunk != new_chunk)	// In new chunk?
		Npc_actor::switched_chunks(old_chunk, new_chunk);
	}

/*
 *	Find day's schedule for a given time-of-day.
 *
 *	Output:	index of schedule change.
 *		-1 if not found, or if a party member.
 */

int Npc_actor::find_schedule_change
	(
	int hour3			// 0=midnight, 1=3am, etc.
	)
	{
	if (party_id >= 0 || is_dead())
		return (-1);		// Fail if a party member or dead.
	for (int i = 0; i < num_schedules; i++)
		if (schedules[i].get_time() == hour3)
			return i;
	return -1;
	}

/*
 *	Update schedule at a 3-hour time change.
 */

void Npc_actor::update_schedule
	(
	int hour3,			// 0=midnight, 1=3am, etc.
	int backwards,			// Extra periods to look backwards.
	int delay			// Delay in msecs, or -1 for random.
	)
	{
	int i = find_schedule_change(hour3);
	if (i < 0)
		{			// Not found?  Look at prev.?
					// Always if noon of first day.
		long hour = gclock->get_total_hours();
		if (hour == 12 && !backwards)
			backwards++;
		while (backwards-- && i < 0)
			i = find_schedule_change((--hour3 + 8)%8);
		if (i < 0)
			return;
		// This is bad, not always true
		// location might be different
		//if (schedule_type == schedules[i].get_type())
		//	return;		// Already in it.
		}
	set_schedule_and_loc (schedules[i].get_type(), schedules[i].get_pos(),
								delay);
	}

/*
 *	Render.
 */

void Npc_actor::paint
	(
	)
	{
	Actor::paint();			// Draw on screen.
	if (dormant && schedule &&	// Resume schedule.
					// FOR NOW:  Not when in formation.
	    (party_id < 0 || !gwin->walk_in_formation || 
				schedule_type != Schedule::follow_avatar))
		{
		dormant = false;	// But clear out old entries first.??
		gwin->get_tqueue()->remove(this);
					// Force schedule->now_what() in .5secs
					// DO NOT call now_what here!!!
		uint32 curtime = Game::get_ticks();
		gwin->get_tqueue()->add(curtime + 500, this, reinterpret_cast<long>(gwin));
		}
	if (!nearby)			// Make sure we're in 'nearby' list.
		gwin->add_nearby_npc(this);
	}

/*
 *	Run usecode when double-clicked.
 */
void Npc_actor::activate
	(
	int event
	)
	{
	if (is_dead())
		return;
					// Converse, etc.
	Actor::activate(event);
#if 0	/* +++Sometimes causes former companions to wander off. */
	//++++++ This might no longer be needed.  Need to test.++++++ (jsf)
					// Want to get BG actors from start
					//   to their regular schedules:
	int i;				// Past 6:00pm first day?
	if (gclock->get_total_hours() >= 18 || 
	    Game::get_game_type() == SERPENT_ISLE ||
					// Or no schedule change.
	    (i = find_schedule_change(gclock->get_hour()/3)) < 0 ||
					// Or schedule is already correct?
	    schedules[i].get_type() == schedule_type)
		return;
	cout << "Setting '" << get_name() << "' to 1st schedule" << endl;
					// Maybe a delay here?  Okay for now.
	update_schedule(gclock->get_hour()/3);
#endif
	}

/*
 *	Handle a time event (for animation).
 */

void Npc_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	if (!action)			// Not doing anything?
		{			// Stop if not on current map.
		if (get_map() != gwin->get_map())
			dormant = true;
		else if (schedule && !in_usecode_control())
			schedule->now_what();
		}
	else
		{			// Do what we should.
		int delay = party_id < 0 ? gwin->is_time_stopped() : 0;
		if (delay <= 0)		// Time not stopped?
			delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			gwin->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			{
			set_action(0);
			if (get_map() != gwin->get_map())
				dormant = true;
			if (schedule && !in_usecode_control())
				if (dormant)
					schedule->im_dormant();
				else
					schedule->now_what();
			}
		}
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked (or paralyzed).
 *		Dormant is set if off screen.
 */

int Npc_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	if (cheat.in_map_editor() && party_id < 0)
		return 0;
	if (get_flag(Obj_flags::paralyzed) || get_map() != gmap)
		return 0;
	Tile_coord oldtile = get_tile();
					// Get old chunk.
	Map_chunk *olist = get_chunk();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
					// Get ->new chunk.
	Map_chunk *nlist = gmap->get_chunk_safely(cx, cy);
	if (!nlist)			// Shouldn't happen!
		{
		stop();
		return (0);
		}
	int water, poison;		// Get tile info.
	get_tile_info(this, gwin, nlist, tx, ty, water, poison);
	if (is_blocked(t))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
					// Offscreen, but not in party?
		if (!gwin->add_dirty(this) && party_id < 0 &&
					// And > a screenful away?
		    distance(gwin->get_camera_actor()) > 1 + 320/c_tilesize)
			dormant = true;	// Go dormant.
		return (0);		// Done.
		}
	if (poison && t.tz == 0)
		Actor::set_flag(static_cast<int>(Obj_flags::poisoned));
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty();			// Set to repaint old area.
					// Move it.
	movef(olist, nlist, tx, ty, frame, t.tz);

					// Near an egg?  (Do this last, since
					//   it may teleport.)
	nlist->activate_eggs(this, t.tx, t.ty, t.tz, oldtile.tx, oldtile.ty);

					// Offscreen, but not in party?
	if (!add_dirty(1) && party_id < 0 &&
					// And > a screenful away?
	    distance(gwin->get_camera_actor()) > 1 + 320/c_tilesize &&
			//++++++++Try getting rid of the 'talk' line:
	    get_schedule_type() != Schedule::talk &&
	    get_schedule_type() != Schedule::street_maintenance)
		{			// No longer on screen.
		stop();
		dormant = true;
		return (0);
		}
	return (1);			// Add back to queue for next time.
	}

/*
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Npc_actor::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	set_action(0);
//	delete schedule;	// Problems in SI monster creation.
//	schedule = 0;
// Messes up resurrection	num_schedules = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
	gwin->remove_nearby_npc(this);	// Remove from nearby list.
					// Store old chunk list.
	Map_chunk *olist = get_chunk();
	Actor::remove_this(1);	// Remove, but don't ever delete an NPC
	Npc_actor::switched_chunks(olist, 0);
	if (!nodel && npc_num > 0)	// Really going?
		unused = true;		// Mark unused if a numbered NPC.
	}

/*
 *	Update chunks' npc lists after this has moved.
 */

void Npc_actor::switched_chunks
	(
	Map_chunk *olist,	// Old chunk, or null.
	Map_chunk *nlist	// New chunk, or null.
	)
	{
	//++++++++++No longer needed.  Maybe it should go away.
	}

/*
 *	Move (teleport) to a new spot.
 */

void Npc_actor::move
	(
	int newtx, 
	int newty, 
	int newlift,
	int newmap
	)
	{
					// Store old chunk list.
	Map_chunk *olist = get_chunk();
					// Move it.
	Actor::move(newtx, newty, newlift, newmap);
	Map_chunk *nlist = get_chunk();
	if (nlist != olist)
		{
		Npc_actor::switched_chunks(olist, nlist);
		if (!olist)		// Moving back into world?
			dormant = true;	// Cause activation if painted.
		}
	}

/*
 *	Delete.
 */

Dead_body::~Dead_body
	(
	)
	{
	}

/*
 *	Get # of NPC a body came from (or -1 if not known).
 */

int Dead_body::get_live_npc_num
	(
	)
	{
	return npc_num;
	}

/*
 *	Create a sequence of frames.
 */

Frames_sequence::Frames_sequence
	(
	int cnt,			// # of frames.
	unsigned char *f		// List of frames.
	) : num_frames(cnt)
	{
	frames = new unsigned char[cnt];
	memcpy(frames, f, cnt);		// Copy in the list.
	}


