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
#include "animate.h"
#include "objiter.h"
#include "ammoinf.h"
#include "armorinf.h"
#include "weaponinf.h"
#include "npcdollinf.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

#ifndef UNDER_EMBEDDED_CE
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
	Actor::ready_frame };	// Can't strech arms outward.

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
 *	Find (best) ammo of given type.
 *
 *	Output:	->object if found.
 */

Game_object *Actor::find_best_ammo
	(
	int family,
	int needed
	)
	{
	Game_object *best = 0;
	int best_strength = -20;
	Game_object_vector vec;		// Get list of all possessions.
	vec.reserve(50);
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		if (obj->inside_locked() || !In_ammo_family(obj->get_shapenum(), family))
			continue;
		Ammo_info *ainf = obj->get_info().get_ammo_info();
		if (!ainf)	// E.g., musket ammunition doesn't have it.
			continue;
			// Can't use it.
		if (obj->get_quantity() < needed)
			continue;
			// Calc ammo strength.
		int strength = ainf->get_base_strength();
			// Favor those with more shots remaining.
		if (obj->get_quantity() < 5*needed)
			strength /= 3;
		else if (obj->get_quantity() < 10*needed)
			strength /= 2;
		if (strength > best_strength)
			{
			best = obj;
			best_strength = strength;
			}
		}
	return best;
	}

/*
 *	Get effective maximum range for weapon taking in consideration
 *	the actor's strength and combat.
 */
int Actor::get_effective_range
	(
	const Weapon_info *winf,
	int reach
	)
	{
	if (reach < 0)
		{
		if (!winf)
			{
			Monster_info *minf = get_info().get_monster_info();
			return minf ? minf->get_reach()
					: Monster_info::get_default()->get_reach();
			}
		reach = winf->get_range();
		}
	int uses = winf ? winf->get_uses() : Weapon_info::melee;
	if (!uses || uses == Weapon_info::ranged)
		return reach;
	else
		{
		int eff_range;
		int str = get_effective_prop(static_cast<int>(Actor::strength));
		int combat = get_effective_prop(static_cast<int>(Actor::combat));
		if (str < combat)
			eff_range = str;
		else
			eff_range = combat;
		if (uses == Weapon_info::good_thrown)
			eff_range *= 2;
		if (eff_range < reach)
			eff_range = reach;
		if (eff_range > 31)
			eff_range = 31;
		return eff_range;
		}
	}

/*
 *	Find ammo used by weapon.
 *
 *	Output:	->object if found. Additionally, is_readied is set to
 *	true if the ammo is readied.
 */

Game_object *Actor::find_weapon_ammo
	(
	int weapon,			// Weapon shape.
	int needed,
	bool recursive
	)
	{
	if (weapon < 0)
		return 0;
	Weapon_info *winf = ShapeID::get_info(weapon).get_weapon_info();
	if (!winf)
		return 0;
	int family = winf->get_ammo_consumed();
	if (family >= 0)
		{
		Game_object *aobj = get_readied(Actor::ammo);
		if (aobj && In_ammo_family(aobj->get_shapenum(), family) &&
			aobj->get_quantity() >= needed)
			return aobj;		// Already readied.
		else if (recursive)
			return find_best_ammo(family, needed);
		return 0;
		}

	// Search readied weapons first.
	static enum Spots wspots[] = {lhand, rhand, back2h_spot, belt};
	const int num_weapon_spots = sizeof(wspots)/sizeof(wspots[0]);
	for (int i = 0; i < num_weapon_spots; i++)
		{
		Game_object *obj = spots[static_cast<int>(wspots[i])];
		if (!obj || obj->get_shapenum() != weapon)
			continue;
		Shape_info& inf = obj->get_info();
		if (family == -2)
			{
			if (!inf.has_quality() || obj->get_quality() > needed)
				return obj;
			}
			// Family -1 and family -3.
		else if (obj->get_quantity() > needed)
			return obj;
		}

	// Now recursively search all contents.
	return recursive ? Container_game_object::find_weapon_ammo(weapon) : 0;
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
 *	Recursivelly searches for ammunition for a given weapon, if needed.
 *	Output: true if the weapon can be used, ammo -> to best ammunition.
 */

static inline bool Is_weapon_usable
	(
	Actor *npc,
	Game_object *bobj,
	Game_object **ammo = 0,
	bool recursive = true
	)
	{
	if (ammo)
		*ammo = 0;
	Weapon_info *winf = bobj->get_info().get_weapon_info();
	if (!winf)
		return false;		// Not a weapon.
	Game_object *aobj = 0;	// Check ranged first.
	int need_ammo = npc->get_weapon_ammo(bobj->get_shapenum(),
			winf->get_ammo_consumed(), winf->get_projectile(),
			true, &aobj, GAME_BG, recursive);
	if (!need_ammo)
		return true;
		// Try melee if the weapon is not ranged.
	else if (!aobj && winf->get_uses() != Weapon_info::ranged)
		need_ammo = npc->get_weapon_ammo(bobj->get_shapenum(),
				winf->get_ammo_consumed(), winf->get_projectile(),
				false, &aobj, GAME_BG, recursive);
	if (need_ammo && !aobj)
		return false;
	if (ammo)
		*ammo = aobj;
	return true;
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
	if ((ammo = winf->get_ammo_consumed()) < 0)
		{			// Ammo not needed.
		if (winf->uses_charges() && info.has_quality() &&
					weapon->get_quality() <= 0)
			return false;	// Uses charges, but none left.
		else
			return true;
		}
	Game_object *found = 0;
		// Try non-recursive search for ammo first.
	bool usable = Is_weapon_usable(this, weapon, &found, false);
	if (usable)	// Ammo is available and ready.
		return true;
	else if (winf->get_ammo_consumed() < 0)
		return false;	// Weapon can't be used.
		// Try recursive search now.
	found = find_best_ammo(winf->get_ammo_consumed());
	if (!found)
		return false;
	swap_ammo(found);
	return true;
	}

/*
 *	If no shield readied, look through all possessions for the best one.
 *	Output:	true if successful.
 */

bool Actor::ready_best_shield
	(
	)
	{
	int points;
	// What about spell book????
	if (spots[rhand])
		{
		Shape_info& inf = spots[rhand]->get_info();
		return inf.get_armor() || inf.get_armor_immunity();
		}
	Game_object_vector vec;		// Get list of all possessions.
	vec.reserve(50);
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	Game_object *best = 0;
	int best_strength = -20;
	int wtype = other;
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		if (obj->inside_locked())
			continue;
		Shape_info& info = obj->get_info();
			// Only want those that can be readied in hand.
		int ready = info.get_ready_type();
		if (ready != one_handed_weapon && ready != other)
			continue;
		Armor_info *arinf = info.get_armor_info();
		if (!arinf)
			continue;	// Not a shield.
		int strength = arinf->get_base_strength();
		if (strength > best_strength)
			{
			wtype = ready;
			best = obj;
			best_strength = strength;
			}
		}
	if (!best)
		return false;
	// Spot is free already.
	best->remove_this(1);
	add(best, 1);			// Should go to the right place.
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
	int best_strength = -20;
	int wtype = other;
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *obj = *it, *ammo_obj = 0;
		if (obj->inside_locked())
			continue;
		Shape_info& info = obj->get_info();
		int ready = info.get_ready_type();
		if (ready != one_handed_weapon && ready != two_handed_weapon)
			continue;
		Weapon_info *winf = info.get_weapon_info();
		if (!winf)
			continue;	// Not a weapon.
		if (!Is_weapon_usable(this, obj, &ammo_obj))
			continue;
		int strength = winf->get_base_strength();
		strength += get_effective_range(winf);
		if (strength > best_strength)
			{
			wtype = ready;
			best = obj;
			best_ammo = ammo_obj != obj ? ammo_obj : 0;
			best_strength = strength;
			}
		}
	if (!best)
		return false;
		// If nothing is in left hand, nothing will happen.
	Game_object *remove1 = spots[lhand], *remove2 = 0;
	if (wtype == two_handed_weapon)
		remove2 = spots[rhand];
					// Free the spot(s).
	if (remove1)
		remove1->remove_this(1);
	if (remove2)
		remove2->remove_this(1);
	best->remove_this(1);
	add(best, 1);			// Should go to the right place.
	if (wtype == one_handed_weapon)
		ready_best_shield();	// Also add a shield for 1-handed weapons.
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
	Tile_coord *f,			// Step from here, or curpos if null.
	const int move_flags
	)
	{
	Shape_info& info = get_info();
					// Get dim. in tiles.
	int frame = get_framenum();
	int xtiles = info.get_3d_xtiles(frame), ytiles = info.get_3d_ytiles(frame);
	int ztiles = info.get_3d_height();
	if (xtiles == 1 && ytiles == 1)	// Simple case?
		{
		Map_chunk *nlist = gmap->get_chunk(
			t.tx/c_tiles_per_chunk, t.ty/c_tiles_per_chunk);
		nlist->setup_cache();
		int new_lift;
		int blocked = nlist->is_blocked(ztiles, t.tz,
			t.tx%c_tiles_per_chunk, t.ty%c_tiles_per_chunk,
					new_lift, move_flags | get_type_flags());
		t.tz = new_lift;
		return blocked;
		}
	return Map_chunk::is_blocked(xtiles, ytiles, ztiles,
			f ? *f : get_tile(), t, move_flags | get_type_flags());
	}

/*
 *	An object which blocks the destination tile.
 */

Game_object *Actor::find_blocking
	(
	Tile_coord tile,
	int dir
	)
	{
	Rectangle footprint = get_footprint();
	Rectangle base = get_footprint();
	switch (dir)
		{
		case north:
			footprint.shift(0, -1); break;
		case northeast:
			footprint.shift(1, -1); break;
		case east:
			footprint.shift(1, 0); break;
		case southeast:
			footprint.shift(1, 1); break;
		case south:
			footprint.shift(0, 1); break;
		case southwest:
			footprint.shift(-1, 1); break;
		case west:
			footprint.shift(-1, 0); break;
		case northwest:
			footprint.shift(-1, -1); break;
		}
	Game_object *block;
	for (int i = footprint.x; i < footprint.x+footprint.w; i++)
		for (int j = footprint.y; j < footprint.y+footprint.h; j++)
			if (base.has_point(i, j))
				continue;
			else if ((block = Game_object::find_blocking(
					Tile_coord(i, j, get_tile().tz)))!=0)
				return block;
	return 0;
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
	    usecode_dir(0), type_flags(0), ident(0),
	    skin_color(-1), action(0), 
	    frame_time(0), step_index(0), timers(0),
	    weapon_rect(0, 0, 0, 0), temperature(0), rest_time(0),
	    casting_mode(false), atts(0), usecode_name(""),
		target_object(0), attack_weapon(-1), target_tile(Tile_coord(-1, -1, 0))
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
	if (get_info().does_not_eat())
		return;
	int food = get_property(static_cast<int>(food_level));
	food -= (rand()%4);		// Average 1.5 level/hour.
	set_property(static_cast<int>(food_level), food);
	if (food <= 0)			// Really low?
		{
		if (rand()%4)
			say(first_starving, first_starving + 2);
					// Set timer for damage.
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
		if (GAME_SI && rand()%3 == 0)
			if (temperature >= 30)
				say(1218, 1221);
			else
				say(1214, 1217);
		return;
		}
	if (get_info().is_cold_immune())
		return;			// Automatons don't get cold.
	int shnum = get_shapenum();
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
		if (GAME_SI && rand()%3 == 0)
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
	if (GAME_SI && rand()%3 == 0)
		switch (temperature/10)
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
			reduce_health(1 + rand()%3, Weapon_data::sonic_damage);
			break;
			}
	else if (rand()%3 == 0 && temperature/10 == 6)
		reduce_health(1 + rand()%3, Weapon_data::sonic_damage);
	}

/*
 *	Get sequence of frames for an attack.
 *
 *	Output:	# of frames stored.
 */

int Actor::get_attack_frames
	(
	int weapon,			// Weapon shape, or -1 for innate.
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
		if (weapon >= 0 && 
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
		if (gwin->walk_in_formation && distance(leader) <= 6)
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
	int leaderdist = leader->distance(goal);
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
	int dist2lead = leader->distance(pos);
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
	Shape_info& inf = obj->get_info();
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
			if (inf.is_object_allowed(obj->get_framenum(), back2h_spot))
				return true;
			else if (inf.is_object_allowed(obj->get_framenum(), shield_spot))
				return true;
		}
	}

	// Lastly if we have gotten here, check the paperdoll table 
	return inf.is_object_allowed(obj->get_framenum(), spot);
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
				{
			if (info.is_spell()) fistype = FIS_Spell;
			else if (type == two_handed_weapon) fistype = FIS_2Hand;

			Shape_info& inf = obj->get_info();
			if (inf.is_object_allowed(obj->get_framenum(), rhand))
				if (!inf.is_object_allowed(obj->get_framenum(), lhand))
					prefered = rhand;
				else
					alternate = rhand;
			else if (inf.is_object_allowed(obj->get_framenum(), back))
				prefered = back;
			else
				alternate = rhand;
			break;
				}

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
		
		Shape_info& inf = obj->get_info();
		switch (type)
		{
			// Weapons, Sheilds, Spells, misc stuff
			default:
			if (info.is_spell()) fistype = FIS_Spell;
			else if (type == two_handed_si) fistype = FIS_2Hand;

			if (inf.is_object_allowed(obj->get_framenum(), rhand))
				if (!inf.is_object_allowed(obj->get_framenum(), lhand))
					prefered = rhand;
				else
					alternate = rhand;
			else
				alternate = rhand;
			break;

			case other:
			if (inf.is_object_allowed(obj->get_framenum(), back2h_spot))
				prefered = back2h_spot;
			else if (inf.is_object_allowed(obj->get_framenum(), back))
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
			schedule = new Miner_schedule(this);
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
	int flag = GAME_BG ? Obj_flags::bg_dont_render : Obj_flags::dont_render;
	if (cheat.in_map_editor() || !(flags & (1L << flag)))
		{
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		bool invis = flags & (1L << Obj_flags::invisible);
		if (invis && party_id < 0)
			return;	// Don't render invisible NPCs not in party.
		else if (invis)
			paint_invisible(xoff, yoff);
		else 
			paint_shape(xoff, yoff, true);

		paint_weapon();
		if (hit)		// Want a momentary red outline.
			ShapeID::paint_outline(xoff, yoff, HIT_PIXEL);
		else if (flags & ((1L<<Obj_flags::protection) | 
		    (1L << Obj_flags::poisoned) | (1 << Obj_flags::cursed) |
		    	(1 << Obj_flags::charmed) | (1 << Obj_flags::paralyzed)))
			{
			if (flags & (1L << Obj_flags::poisoned))
				ShapeID::paint_outline(xoff,yoff,POISON_PIXEL);
			else if (flags & (1L << Obj_flags::cursed))
				ShapeID::paint_outline(xoff,yoff,CURSED_PIXEL);
			else if (flags & (1L << Obj_flags::charmed))
				ShapeID::paint_outline(xoff, yoff,
								CHARMED_PIXEL);
			else if (flags & (1L << Obj_flags::paralyzed))
				ShapeID::paint_outline(xoff, yoff,
								PARALYZE_PIXEL);
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
	switch (myframe & 0x1f)
	{
		case 4:
		case 7:
		case 22:
		case 25:
			weapon_frame = 4;
			break;
		case 5:
		case 8:
		case 21:
		case 24:
			weapon_frame = 3;
			break;
		case 6:
		case 9:
		case 20:
		case 23:
			weapon_frame = 2;
			break;
		//The next cases (before the default) are here to make use of all
		//the frames of the "casting frames" shape (shape 859):
		case 14:
		case 30:
			weapon_frame = 5;
			break;
		case 15:
			weapon_frame = 6;
			break;
		case 31:
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
			swap(weapon_x, weapon_y);
					// Combat frames are already done.
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
	bool serpent = (sman->can_use_paperdolls() && sman->are_paperdolls_enabled());
	
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
				attack_mode, alignment, flags, flags2, type_flags,
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
	unsigned long xflags;		// Extra object flags.
	unsigned long type_flags;	// Movement flags.
	short num_schedules;
	Serial_schedule schedules[8];
	if (!Npc_actor_in(data, datalen, addr, tx, ty, tz, shape, frame,
			face, name, npc_num, ident, usecode, usecodefun,
			properties, attack_mode, alignment,
			oflags, xflags, type_flags, num_schedules, schedules))
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
	npc->flags2 = xflags;
	npc->type_flags = type_flags;
	npc->set_actor_shape();
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
	bool serpent = (sman->can_use_paperdolls() && sman->are_paperdolls_enabled());
	
	if (!serpent)
		{	// Can't display paperdolls (or they are disabled)
			// Use BG gumps
		Paperdoll_npc *npcinfo = get_info().get_npc_paperdoll();

		if (!npcinfo)
			{
			Shape_info& inf = ShapeID::get_info(get_sexed_coloured_shape());
			npcinfo = inf.get_npc_paperdoll();
			}
		if (!npcinfo)
			{
			Shape_info& inf = ShapeID::get_info(get_shape_real());
			npcinfo = inf.get_npc_paperdoll_safe(get_type_flag(tf_sex));
			}
		if (!npcinfo)		// No paperdoll info at ALL; should never happen...
			return (65);	// Default to male (Pickpocket Cheat)
		
		return npcinfo->get_gump_shape();
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
#if 0
		if (val > 36)		// Looks like max. in usecode.
			val = 36;		// ++++Never seems to get this high, though (marzo)
#else
		if (val > 31)		// Never seems to get above this.
			val = 31;
#endif
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
	case sex_flag:
		// Doesn't seem to be settable in original BG except by hex-editing
		// the save game, but there is no problem in allowing it in Exult.
		if (val > 0)
			set_type_flag(tf_sex);
		else
			clear_type_flag(tf_sex);
		break;
	default:
		if (prop >= 0 && prop < missile_weapon)
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
bool Actor::can_act()
	{
	return !(get_flag(Obj_flags::paralyzed) || get_flag(Obj_flags::asleep)
			|| is_dead() || get_property(static_cast<int>(health)) <= 0);
	}

void Actor::fight_back
	(
	Game_object *attacker
	)
	{
	// ++++ TODO: I think that nearby NPCs will help NPCs (or party members,
	// as the case may be) of the same alignment when attacked by other NPCs,
	// not just when the avatar & party attack. Although this is tricky to
	// test (except, maybe, by exploiting the agressive U7 & SI duel schedule.
	Actor *npc = attacker ? attacker->as_actor() : 0;
	if (!npc)
		return;
	if (!target && !is_in_party())
		set_target(npc, npc->get_schedule_type() != Schedule::duel);
	// Being a bully?
	if (npc->is_in_party() &&
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
		else if (rand()%20 == 0)
			{
			cout << "Rand()%20" << endl;
			gwin->attack_avatar(1 + rand()%2);
			// To reduce the guard pile-up.
			lastcall = curtime;
			}
		}
	}

/*
 *	This method should be called to cause damage from traps, attacks.
 *
 *	Output:	Hits taken. If exp is nonzero, experience value if defeated.
 */

int Actor::apply_damage
	(
	Game_object *attacker,	// Attacker, or null.
	int str,		// Attack strength.
	int wpoints,	// Weapon bonus.
	int type,		// Damage type.
	int bias,		// Different combat difficulty.
	int *exp
	)
	{
	if (exp)
		*exp = 0;
	int damage = bias;
	str /= 3;

		// In the original, wpoints == 127 does fixed 127 damage.
		// Allowing for >= 127 in Exult, as the original seems to
		// use only a byte for damage/health.
	if (wpoints >= 127)
		damage = 127;
	else
		{
			// Lightning damage ignores strength.
		if (type != Weapon_data::lightning_damage && str > 0)
			damage += (1 + rand()%str);
		if (wpoints > 0)
			damage += (1 + rand()%wpoints);
		}

	int armor = -bias;
	Monster_info *minf = get_info().get_monster_info();
		// Monster armor protects only in UI_apply_damage.
	if (minf)
		armor += minf->get_armor();

		// Armor defense and immunities only affect UI_apply_damage.
	const int num_spots = sizeof(spots)/sizeof(spots[0]);
	for (int i = 0; i < num_spots; i++)
		{
		Game_object *obj = spots[i];
		if (obj)
			{
			Shape_info& info = obj->get_info();
			armor += info.get_armor();
			if ((info.get_armor_immunity() & (1 << type)) != 0)
				{	// Armor gives immunity.
					// Metal clang.
				int sfx = Audio::game_sfx(5);
				new Object_sfx(this, sfx);
					// Attack back anyway.
				fight_back(attacker);
				return 0;	// No damage == no powers.
				}
			}
		}

		// Some attacks ignore armor (unless the armor gives immunity).
	if (wpoints == 127 || type == Weapon_data::lightning_damage ||
			type == Weapon_data::ethereal_damage ||
			type == Weapon_data::sonic_damage ||
			armor < 0)	// Armor should never help the attacker.
		armor = 0;

	if (armor)
		damage -= (1 + rand()%armor);

		// Paralyzed/defenseless foes may take damage even if
		// the armor protects them. This code is guesswork,
		// but it matches statistical tests.
	if (damage <= 0 && !can_act())
		{
		if (str > 0)
			damage = 1 + rand()%str;
		else
			damage = 0;
		}

	if (damage <= 0)
		{	// No damage caused.
		int sfx = Audio::game_sfx(5);
		new Object_sfx(this, sfx);

			// Flash red outline.
		hit = true;
		add_dirty();
		Clear_hit *c = new Clear_hit();
		gwin->get_tqueue()->add(Game::get_ticks() + 200, c, 
					reinterpret_cast<long>(this));
			// Attack back.
		fight_back(attacker);
		return 0;	// No damage == no powers (usually).
		}

	return reduce_health(damage, type, attacker, exp);
	}

/*
 *	This method should be called to decrement health directly.
 *
 *	Output:	Hits taken. If exp is nonzero, experience value if defeated.
 */

int Actor::reduce_health
	(
	int delta,			// # points to lose.
	int type,		// Type of damage
	Game_object *attacker,		// Attacker, or null.
	int *exp
	)
	{
	if (exp)
		*exp = 0;
		// Cheater, cheater.
	if (cheat.in_god_mode() && ((party_id != -1) || (npc_num == 0)))
		return 0;

	Monster_info *minf = get_info().get_monster_info_safe();
	Actor *npc = attacker ? attacker->as_actor() : 0;

		// Monster immunities DO affect UI_reduce_health, unlike
		// armor immunities.
	if (is_dead() || minf->cant_die() ||
		(minf->get_immune() & (1 << type)) != 0)
		{	// Monster data gives immunity to damage.
			// Attack back.
		fight_back(attacker);
		return 0;
		}

		// Monsters vulnerable to a damage type take 2x the damage.
		// The originals seem to limit damage to 127 points; we
		// set a lower bound on final health below.
	if ((minf->get_vulnerable() & (1 << type)) != 0)
		delta *= 2;

	int oldhp = properties[static_cast<int>(health)];
	int maxhp = properties[static_cast<int>(strength)];
	int val = oldhp - delta;
	if (val < -50)
		{	// Limit how low it goes for safety.
		val = -50;
		delta = oldhp + 50;
		}
		// Don't set health yet!! (see tournament below for why)
		// The following thresholds are exact.
	if (this == gwin->get_main_actor() &&
					// Flash red if Avatar badly hurt.
			(delta >= maxhp/3 || oldhp < maxhp/4 ||
					// Or if lightning damage.
			type == Weapon_data::lightning_damage))
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
			say(0x4d2, 0x4da);	// ++++ TODO: Not sure they use all these.
		else if (!minf || !minf->cant_yell())
			say(first_ouch, last_ouch);
		}
	Game_object_vector vec;		// Create blood.
	const int blood = 912;		// ++++TAG for future de-hard-coding.
			// Bleed only for normal damage.
	if (type == Weapon_data::normal_damage && !minf->cant_bleed()
#if 0
		delta >= 3 && rand()%2
#else	// Trying something new. Seems to match originals better, but
		// it is hard to judge accurately (although 10 or more hits
		// *always* cause bleeding).
		&& rand()%10 < delta
#endif
		&& find_nearby(vec, blood, 1, 0) < 2)
		{			// Create blood where actor stands.
		Game_object *bobj = gmap->create_ireg_object(blood, 0);
		bobj->set_flag(Obj_flags::is_temporary);
		bobj->move(get_tile());
		}
	if (val <= 0 && oldhp > 0 && get_flag(Obj_flags::tournament))
		{	// HPs are never reduced before tournament usecode
			// (this can be checked on weapon usecode, tournament
			// usecode or even UI_reduce_health directly followed
			// by UI_get_npc_prop).
			// THIS is why we haven't reduced health yet.
			// This makes foes with tournament flag EXTREMELLY
			// tough, particularly if they have high hit points
			// to begin with!
			// No more pushover banes!
		if (npc)	// Just to be sure.
			set_oppressor(npc->get_npc_num());
		ucmachine->call_usecode(get_usecode(), this, 
						Usecode_machine::died);
		return 0;	// If needed, NPC usecode does the killing (possibly
					// by calling this function again).
		}

		// We do slimes here; they DO split through reduce_health intrinsic.
		// They also do *not* split if hit by damage they are vulnerable to.
	if (minf->splits() && val > 0 && 
		(minf->get_vulnerable() & (1 << type)) == 0 && rand()%2 == 0)
		clone();
	
		// Doing this here simplifies the tournament code, above.
	properties[static_cast<int>(health)] = val;
	bool defeated = is_dying() || (val <= 0 && oldhp > 0);
	
	if (defeated && exp)
		{
			// Verified: No experience for killing sleeping people.
		if (!get_flag(Obj_flags::asleep))
			{
			int expval = 0;
				// Except for 2 details mentioned below, this formula
				// is an exact match to what the originals give.
				// We also have to do this *here*, before we kill the
				// NPC, because the equipment is deleted (spells) or
				// transferred to the dead body it leaves.
			int combval = properties[static_cast<int>(combat)];
			expval = properties[static_cast<int>(strength)] + combval + 
					properties[static_cast<int>(dexterity)]/3 + 
					properties[static_cast<int>(intelligence)]/5;
			Monster_info *minf = get_info().get_monster_info();
			int immune = minf ? minf->get_immune() : 0;
			int vuln = minf ? minf->get_vulnerable() : 0;
			if (minf)
				expval += minf->get_base_xp_value();

			if (!objects.is_empty())
				{
				Game_object_vector vec;		// Get list of all possessions.
				vec.reserve(50);
				get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
				for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
					{
					Game_object *obj = *it;
						// This matches the original, but maybe
						// we should iterate through all items.
						// After all, a death bolt in the backpack
						// can still be dangerous...
					if (obj->get_owner() != this)
						continue;
					Shape_info& inf = obj->get_info();
					expval += inf.get_armor();
						// Strictly speaking, the original does not give
						// XP for armor immunities; but I guess this is
						// mostly because very few armors give immunities
						// (ethereal ring, cadellite helm) and no monsters
						// use them anyway.
						// I decided to have them give a (non-cumulative)
						// increase in experience.
					immune |= inf.get_armor_immunity();
					Weapon_info *winf = inf.get_weapon_info();
					if (!winf)
						continue;
					expval += winf->get_base_xp_value();
						// This was tough to figure out, but figured out it was;
						// it is a perfect match to what the original does.
					switch (winf->get_uses())
						{
						case Weapon_info::melee:
							{
							int range = winf->get_range();
							expval += range > 5 ? 2 : (range > 3 ? 1 : 0);
							break;
							}
						case Weapon_info::poor_thrown:
							expval += combval/5; break;
						case Weapon_info::good_thrown:
							expval += combval/3; break;
						case Weapon_info::ranged:
							expval += winf->get_range()/2; break;
						}
					}
				}
				// Originals don't do this, but hey... they *should*.
				// Also, being vulnerable to something you are immune
				// should not matter because immunities are checked first;
				// the originals don't do this check, but neither do they
				// have a monster vulnerable and immune to the same thing.
			vuln &= ~immune;
			expval += bitcount(immune);
			expval -= bitcount(vuln);
				// And the final touch (verified).
			expval /= 2;
			*exp = expval;
			}
		}

	if (is_dying())
		die(attacker);
	else if (val <= 0 && !get_flag(Obj_flags::asleep))
		set_flag(Obj_flags::asleep);
	else if (npc && !target && !is_in_party())
		set_target(npc, npc->get_schedule_type() != Schedule::duel);
	return delta;
	}

/*
 *	Causes the actor to fall to the ground and take damage.
 */

void Actor::fall_down()
	{
	if (!get_type_flag(tf_fly))
		return;
	int savetz = get_lift();
	Tile_coord start = get_tile();
	if (!Map_chunk::is_blocked(start, 1, MOVE_WALK, 100) && 
			start.tz < savetz)
		{
		move(start.tx, start.ty, start.tz);
		reduce_health(1 + rand()%5, Weapon_data::normal_damage);
		}
	}

/*
 *	Causes the actor to lie down to sleep (or die).
 */

void Actor::lay_down(bool die)
	{
	if (!die && get_flag(Obj_flags::asleep))
		return;
		// Fall to the ground.
	fall_down();
	bool frost_serp = GAME_SI && get_shapenum() == 832;
		// Don't do it if already in sleeping frame.
	if (frost_serp && (get_framenum() & 0xf) == Actor::sit_frame)
		return;		// SI Frost serpents are weird.
	else if ((get_framenum() & 0xf) == Actor::sleep_frame)
		return;

	set_action(0);
	Usecode_script *scr = new Usecode_script(this);
	(*scr) << Ucscript::finish << (Ucscript::npc_frame + Actor::standing);
	if (GAME_SI && get_shapenum() == 832)	// SI Frost serpent
		{		// Frames are in reversed order. This results in a
				// strangeanimation in the original, which we fix here.
		(*scr) << (Ucscript::npc_frame + Actor::sleep_frame)
			<< (Ucscript::npc_frame + Actor::kneel_frame)
			<< (Ucscript::npc_frame + Actor::bow_frame)
			<< (Ucscript::npc_frame + Actor::sit_frame);
		}
	else if (GAME_BG && get_shapenum() == 525)	// BG Sea serpent
		{
		(*scr) << (Ucscript::npc_frame + Actor::bow_frame)
			<< (Ucscript::npc_frame + Actor::kneel_frame)
			<< (Ucscript::npc_frame + Actor::sleep_frame);
		}
	else
		{	// Slimes are done elsewhere.
		(*scr) << (Ucscript::npc_frame + Actor::kneel_frame)
			<< Ucscript::sfx << Audio::game_sfx(86);
		if (!die)
			(*scr) << (Ucscript::npc_frame + Actor::sleep_frame);
		}
	if (die)	// If dying, remove.
		(*scr) << Ucscript::remove;
	scr->start();
	}

/*
 *	Get a property.
 */

int Actor::get_property(int prop) const
	{
	if (prop == Actor::sex_flag)
		// Correct in BG and SI, but the flag is never normally set
		// for anyone but avatar in BG.
		return get_type_flag(Actor::tf_sex);
	else if (prop == Actor::missile_weapon)
		{
		// Seems to give the same results as in the originals.
		Game_object *weapon = get_readied(lhand);
		Weapon_info *winf = weapon ? weapon->get_info().get_weapon_info() : 0;
		if (!winf)
			return 0;
		return (winf->get_uses() >= 2);
		}
	return (prop >= 0 && prop < Actor::sex_flag) ? properties[prop] : 0;
	}

/*
 *	Get a property, modified by flags.
 */

int Actor::get_effective_prop
	(
	int prop				// Property #.
	) const
	{
	int val = get_property(prop);
	switch (prop)
		{
	case Actor::dexterity:
	case Actor::combat:
		if (get_flag(Obj_flags::paralyzed) || get_flag(Obj_flags::asleep) ||
			get_property(Actor::health) < 0)
			return prop == Actor::dexterity ? 0 : 1;
	case Actor::intelligence:
		if (is_dead())
			return prop == Actor::dexterity ? 0 : 1;
		else if (get_flag(Obj_flags::charmed) || get_flag(Obj_flags::asleep))
			val--;
	case Actor::strength:
		if (get_flag(Obj_flags::might))
			val += (val < 15 ? val : 15);	// Add up to 15.
		if (get_flag(Obj_flags::cursed))
			val -= 3;	// Matches originals.
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
	Monster_info *minf = get_info().get_monster_info_safe();
	switch (flag)
		{
	case Obj_flags::asleep:
		if (minf->sleep_safe() || minf->power_safe())
			return;		// Don't do anything.
					// Check sched. to avoid waking
					//   Penumbra.
		if (schedule_type == Schedule::sleep)
			break;
					// Set timer to wake in a few secs.
		need_timers()->start_sleep();
		set_action(0);		// Stop what you're doing.
		lay_down(false);	// Lie down.
		break;
	case Obj_flags::poisoned:
		if (minf->poison_safe())
			return;		// Don't do anything.
		need_timers()->start_poison();
		break;
	case Obj_flags::protection:
		need_timers()->start_protection();
		break;
	case Obj_flags::might:
		need_timers()->start_might();
		break;
	case Obj_flags::cursed:
		if (minf->curse_safe() || minf->power_safe())
			return;		// Don't do anything.
		need_timers()->start_curse();
		break;
	case Obj_flags::charmed:
		if (minf->charm_safe() || minf->power_safe())
			return;		// Don't do anything.
		need_timers()->start_charm();
		set_target(0);		// Need new opponent if in combat.
		break;
	case Obj_flags::paralyzed:
		if (minf->paralysis_safe() || minf->power_safe())
			return;		// Don't do anything.
		fall_down();
		need_timers()->start_paralyze();
		break;
	case Obj_flags::invisible:
		need_timers()->start_invisibility();
		Combat_schedule::stop_attacking_invisible(this);
		gclock->set_palette();
		break;
	case Obj_flags::dont_move:
	case Obj_flags::bg_dont_move:
		stop();			// Added 7/6/03.
		set_action(0);	// Force actor to stop current action.
		break;
	case Obj_flags::naked:
		{
		// set_polymorph needs this, and there are no problems
		// in setting this twice.
		flags2 |= ((uint32) 1 << (flag-32));
		if (get_npc_num() != 0)	// Ignore for all but avatar.
			break;
		int sn;
		int female = get_type_flag(tf_sex)?1:0;
		Skin_data *skin = Shapeinfo_lookup::GetSkinInfoSafe(this);
		
		if (!skin ||	// Should never happen, but hey...
			(!sman->have_si_shapes() &&
				Shapeinfo_lookup::IsSkinImported(skin->naked_shape)))
			sn = Shapeinfo_lookup::GetBaseAvInfo(female)->shape_num;
		else
			sn = skin->naked_shape;
		set_polymorph(sn);
		break;
		}
		}

	// Doing it here to prevent problems with immunities.
	if (flag >= 0 && flag < 32)
		flags |= ((uint32) 1 << flag);
	else if (flag >= 32 && flag < 64)
		flags2 |= ((uint32) 1 << (flag-32));
					// Update stats if open.
	if (gumpman->showing_gumps())
		gwin->set_all_dirty();
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
	else if (flag == Obj_flags::bg_dont_move || flag == Obj_flags::dont_move)
		// Start again after a little while
		start(gwin->get_std_delay(), gwin->get_std_delay());
	else if (flag == Obj_flags::polymorph && get_flag(Obj_flags::naked))
		clear_flag(Obj_flags::naked);
	else if (flag = Obj_flags::naked && get_flag(Obj_flags::polymorph))
		clear_flag(Obj_flags::polymorph);
	
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
	int warmth = -75;		// Base value.

	static Spots locs[] = {head, cloak_spot, feet, torso, hands2_spot, legs};
	for (int i = 0; i < sizeof(locs)/sizeof(locs[0]); i++)
		{
		Game_object *worn = spots[static_cast<int>(locs[i])];
		if (worn)
			warmth += worn->get_info().get_object_warmth(worn->get_framenum());
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
	if (!obj->get_info().has_usecode_events())
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
 *	Returns true if NPC is in a non-no_halt usecode script or if
 *	dont_move flag is set.
 */
bool Actor::in_usecode_control() const
	{
	if (get_flag(Obj_flags::dont_render) || get_flag(Obj_flags::dont_move))
		return true;
	Usecode_script *scr = 0;
	Actor *act = const_cast<Actor *>(this);
	while ((scr = Usecode_script::find_active(act, scr)) != 0)
		// no_halt scripts seem not to prevent movement.
		if (!scr->is_no_halt())
			return true;
	return false;
	}

/*
 *	Attack preset target/tile using preset weapon shape.
 *
 *	Output: True if attack was realized and hit target (or is
 *	a missile flying towards target), false otherwise
 */
bool Actor::usecode_attack()
	{
	return Combat_schedule::attack_target(
			this, target_object, target_tile, attack_weapon);
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
	if (spots[index]) return (spots[index]->add(obj, dont_check));

	int prefered;
	int alternate;
	FIS_Type type;

	// Get the preferences
	get_prefered_slots (obj, prefered, alternate, type);
	
	// Check Prefered
	if (!fits_in_spot (obj, index, type) && !force_pos) return 0;

	// No room, or too heavy.
	if (!Container_game_object::add(obj, true)) return 0;

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
	int d = 8;
	// Try orthogonal directions first.
	to = cur.get_neighbor((dir + 2)%8);
	if (!is_blocked(to, 0, get_type_flags()))
		d = (dir + 2)%8;
	else
		{
		to = cur.get_neighbor((dir + 6)%8);
		if (!is_blocked(to, 0, get_type_flags()))
			d = (dir + 6)%8;
		else
			{
			for (i = 0; i < 4; i++)		// Try diagonals now.
				{
				to = cur.get_neighbor(2*i+1);
				if (!is_blocked(to, 0, get_type_flags()))
					{
					d = 2*i+1;
					break;
					}
				}
			}
		}

	int stepdir = d;		// This is the direction.
	if (d == 8 || to.tx < 0)	// Failed?  Try to swap places.
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
	shape = -1;			// Bare hands.
	Weapon_info *winf = 0;
	Game_object *weapon = spots[static_cast<int>(lhand)];
	obj = weapon;
	if (weapon)
		{
		if ((winf = weapon->get_info().get_weapon_info()) != 0)
			{
			points = winf->get_damage();
			shape = weapon->get_shapenum();
			return winf;
			}
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
	const int sides = 30;
	int roll = rand()%sides;
	if (roll == 0)			// Always lose.
		return false;
	else if (roll == sides - 1)	// High?  Always win.
		return true;
	else
		return roll + attacker - defender >= sides/2 - 1;
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
 *	Output:	Hits taken or < 0 for explosion.
 */

int Actor::figure_hit_points
	(
	Game_object *attacker,
	int weapon_shape,		// < 0 for readied weapon.
	int ammo_shape,			// < 0 for no ammo shape.
	bool explosion			// If this is an explosion attacking.
	)
	{
	bool were_party = party_id != -1 || npc_num == 0;
	// godmode effects:
	if (were_party && cheat.in_god_mode())
		return 0;
	Actor *npc = attacker ? attacker->as_actor() : 0;
	bool theyre_party = npc &&
			(npc->party_id != -1 || npc->npc_num == 0);
	bool instant_death = (cheat.in_god_mode() && theyre_party);
					// Modify using combat difficulty.
	int bias = were_party ? Combat::difficulty :
			(theyre_party ? -Combat::difficulty : 0);

	const Weapon_info *winf;
	Ammo_info *ainf;

	int wpoints = 0;
	if (weapon_shape >= 0)
		winf = ShapeID::get_info(weapon_shape).get_weapon_info();
	else
		winf = 0;
	if (ammo_shape >= 0)
		ainf = ShapeID::get_info(ammo_shape).get_ammo_info();
	else
		ainf = 0;
	if (!winf && weapon_shape < 0)
		winf = npc ? npc->get_weapon(wpoints) : 0;

	int usecode = -1, powers = 0;
	int type = Weapon_data::normal_damage;
	bool explodes = false;

	if (winf)
		{
		wpoints = winf->get_damage();
		usecode = winf->get_usecode();
		type = winf->get_damage_type();
		powers = winf->get_powers();
		explodes = winf->explodes();
		}
	else
		wpoints = 1;	// Give at least one, but only if there's no weapon
	if (ainf)
		{
		wpoints += ainf->get_damage();
			// Replace damage type.
		if (ainf->get_damage_type() != Weapon_data::normal_damage)
			type = ainf->get_damage_type();
		powers |= ainf->get_powers();
		explodes = explodes || ainf->explodes();
		}

	if (explodes && !explosion)	// Explosions shouldn't explode again.
		{	// Time to explode.
		Tile_coord offset(0, 0, get_info().get_3d_height()/2);
		eman->add_effect(new Explosion_effect(get_tile() + offset,
				0, 0, weapon_shape, ammo_shape, attacker));
			// The explosion will handle the damage.
		return -1;
		}

	int expval = 0, hits = 0;
	bool nodamage = (powers & (Weapon_data::no_damage)) != 0;
	if (instant_death)
		wpoints = 127;
	if (!nodamage)
		// This may kill the NPC; this comes before powers because no
		// damage means no powers -- except for the no_damage flag.
		hits = apply_damage(attacker, Get_effective_prop(npc, Actor::strength, 0),
				wpoints, type, bias, &expval);
		// Apply weapon powers if needed.
	if (powers && (hits || nodamage))
		{
			// Protection prevents powers.
		if (!get_flag(Obj_flags::protection))
			{
			int attint = Get_effective_prop(npc, Actor::intelligence, 16),
				defstr = Get_effective_prop(this, Actor::strength),
				defint = Get_effective_prop(this, Actor::intelligence);

				// These rolls are bourne from statistical analisys and are,
				// as far as I can tell, how the game works.
			if ((powers & Weapon_data::poison) && roll_to_win(attint, defstr))
				set_flag(Obj_flags::poisoned);
			if ((powers & Weapon_data::curse) && roll_to_win(attint, defint))
				set_flag(Obj_flags::cursed);
			if ((powers & Weapon_data::charm) && roll_to_win(attint, defint))
				set_flag(Obj_flags::charmed);
			if ((powers & Weapon_data::sleep) && roll_to_win(attint, defint))
				set_flag(Obj_flags::asleep);
			if ((powers & Weapon_data::paralyze) && roll_to_win(attint, defstr))
				set_flag(Obj_flags::paralyzed);
			if (powers & Weapon_data::magebane)
				{	// Magebane weapons (magebane sword, death scythe).
					// Take away all mana.
				int mana = properties[static_cast<int>(Actor::mana)];
				set_property(static_cast<int>(Actor::mana), 0);
				int num_spells = 0;
				Game_object_vector vec, spells;
				vec.reserve(50);
				spells.reserve(50);
				get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
					// Gather all spells...
				for (Game_object_vector::iterator it = vec.begin(); 
							it != vec.end(); ++it)
					if ((*it)->get_info().is_spell())	// Seems to be right.
						spells.push_back(*it);
				vec.clear();
					// ... and take them all away.
				while (!spells.empty())
					{
					++num_spells;
					Game_object *obj = spells.back();
					spells.pop_back();
					obj->remove_this();
					}
				if (num_spells)
					{	// Display magebane struck string and set
						// no_spell_casting flag. This is only done
						// to prevent monsters from teleporting or
						// doing other such things.
					set_flag(Obj_flags::no_spell_casting);
					if (GAME_SI)
						{
						eman->remove_text_effect(this);
						say(first_magebane_struck, last_magebane_struck);
						}
					}
				}
			}
		if (nodamage && ammo_shape == 568)
			{	// This is *only* done for SI sleep arrows, and all other
				// powers have had their effect by now (as can be verified
				// by using the called usecode function).
			if (npc)	// Just to be sure.
				set_oppressor(npc->get_npc_num());
			// Allowing for BG too, as it doesn't have a function 0x7e1.
			ucmachine->call_usecode(0x7e1, this,
						Usecode_machine::weapon);
			}
		}

	if (expval > 0 && npc)	// Give experience.
		npc->set_property(static_cast<int>(exp),
		    npc->get_property(static_cast<int>(exp)) + expval);

		// Weapon usecode comes last of all.
	if (usecode > 0)
		{
		if (npc)	// Just to be sure.
			set_oppressor(npc->get_npc_num());
		ucmachine->call_usecode(usecode, this,
					Usecode_machine::weapon);
		}
	return hits;
	}

/*
 *	Trying to hit NPC with an attack.
 *
 *	Output:	true if attack hit, false otherwise.
 */

bool Actor::try_to_hit
	(
	Game_object *attacker,
	int attval
	)
	{
	int defval = get_effective_prop(static_cast<int>(combat)) +
			get_flag(Obj_flags::protection) ? 3 : 0;
	if (combat_trace)
		{
		string name = "<trap>";
		if (attacker)
			name = attacker->get_name();
		int prob = 30 - (15 + defval - attval) + 1;
		if (prob >= 30)
			prob = 29;	// 1 always misses.
		else if (prob <= 1)
			prob = 1;	// 30 always hits.
		prob *= 100;
		cout << name << " is attacking " << get_name()
			<< " with hit probability " << (float)prob/30 << "%" << endl;
		}

	return Actor::roll_to_win(attval, defval);
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if defeated, else object itself.
 */

Game_object *Actor::attacked
	(
	Game_object *attacker,
	int weapon_shape,		// < 0 for readied weapon.
	int ammo_shape,			// < 0 for no ammo shape.
	bool explosion			// If this is an explosion attacking.
	)
	{
	if (is_dead() ||		// Already dead?
					// Or party member of dead Avatar?
	    (party_id >= 0 && gwin->get_main_actor()->is_dead()))
		return 0;
	Actor *npc = attacker ? attacker->as_actor() : 0;
	if (npc)
		set_oppressor(npc->get_npc_num());
	if (npc && npc->get_schedule_type() == Schedule::duel)
		return this;	// Just play-fighting.

	int oldhp = properties[static_cast<int>(health)];
	int delta = figure_hit_points(attacker, weapon_shape, ammo_shape, explosion);

	if (Combat::show_hits && !is_dead() && delta >= 0)
		{
		eman->remove_text_effect(this);
		char hpmsg[50];
		sprintf(hpmsg, "-%d(%d)", delta, oldhp - delta);
		eman->add_text(hpmsg, this);
		}
	if (combat_trace)
		{
		string name = "<trap>";
		if (attacker)
			name = attacker->get_name();
		cout << name << " hits " << get_name();
		if (delta > 0)
			{
			cout << " for " << delta << " hit points; ";
			if (oldhp > 0 && oldhp < delta)
				cout << get_name() << " is defeated." << endl;
			else
				cout << oldhp - delta << " hit points are left." << endl;
			}
		else if (!delta)
			cout << " to no damage." << endl;
		else
			cout << " causing an explosion." << endl;
		}

	if (attacker && (is_dead() || properties[(int) health] < 0))
		return 0;
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
	Game_object *attacker
	)
	{
	// If the actor is already dead, we shouldn't do anything
	//(fixes a resurrection bug).
	if (is_dead())
		return;
	set_action(0);
	delete schedule;
	schedule = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
	Actor::set_flag(Obj_flags::dead);// IMPORTANT:  Set this before moving
					//   objs. so Usecode(eventid=6) isn't called.
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
					// Get location.
	Tile_coord pos = get_tile();
	//properties[static_cast<int>(health)] = -50;
	Shape_info& info = get_info();
	Monster_info *minfo = info.get_monster_info();
#if 0	// ++++ Trying new thing.
	remove_this(1);			// Remove (but don't delete this).
	set_invalid();
#else
	bool frost_serp = GAME_SI && get_shapenum() == 832;
	if (frost_serp && (get_framenum() & 0xf) == Actor::sit_frame
		|| (get_framenum() & 0xf) == Actor::sleep_frame)
		{
		Usecode_script *scr = new Usecode_script(this);
		(*scr) << Ucscript::delay_ticks << 4 << Ucscript::remove;
		scr->start();
		}
	else	// Laying down to die.
		lay_down(true);
#endif

	Dead_body *body;		// See if we need a body.
	if (!minfo || !minfo->has_no_body())
		{
				// Get body shape/frame.
		shnum = info.get_body_shape();		// Default 400.
		int frnum = info.get_body_frame();	// Default 3.
					// Reflect if NPC reflected.
		frnum |= (get_framenum()&32);
#if 0	// ++++ Trying new thing.
		body = new Dead_body(shnum, frnum, 0, 0, 0, 
					npc_num > 0 ? npc_num : -1);
#else
		body = new Dead_body(shnum, 0, 0, 0, 0, 
					npc_num > 0 ? npc_num : -1);
		Usecode_script *scr = new Usecode_script(body);
		(*scr) << Ucscript::delay_ticks << 4 << Ucscript::frame << frnum;
		scr->start();
#endif
		if (npc_num > 0)
			{	// Originals would use body->set_quality(2) instead
				// for bodies of dead monsters. What we must do for
				// backwards compatibility...
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
#if 1		// Guessing it is spells that get deleted.
		if (item->get_info().is_spell())
#else
		if (!item->is_dragable())
#endif
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
		// Set oppressor and cause nearby NPCs to attack avatar.
	fight_back(attacker);
			// Is this a bad guy?
			// Party defeated an evil monster?
	Actor *npc = attacker ? attacker->as_actor() : 0;
	if (npc && npc->is_in_party() && !is_in_party() && 
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
	int frame = get_framenum();
	int xs = info.get_3d_xtiles(frame), ys = info.get_3d_ytiles(frame);
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
	clear_flag(Obj_flags::no_spell_casting);
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
	Tile_coord pos;
	if (body)
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
		pos = body->get_tile();
		body->remove_this();		// Remove and delete body.
		}
	else
		pos = Tile_coord(-1, -1, 0);
	move(pos);			// Move back to life.
					// Restore health to max.
	properties[static_cast<int>(health)] = 
					properties[static_cast<int>(strength)];
	Actor::clear_flag(Obj_flags::dead);
	Actor::clear_flag(Obj_flags::poisoned);
	Actor::clear_flag(Obj_flags::paralyzed);
	Actor::clear_flag(Obj_flags::asleep);
	Actor::clear_flag(Obj_flags::protection);
	Actor::clear_flag(Obj_flags::cursed);
	Actor::clear_flag(Obj_flags::charmed);
					// Restore to party if possible.
	partyman->update_party_status(this);
					// Give a reasonable schedule.
	set_schedule_type(is_in_party() ? Schedule::follow_avatar
					: Schedule::loiter);
					// Stand up.
	if (!body)
		return (this);
	Usecode_script *scr = new Usecode_script(this);
	(*scr) << (Ucscript::npc_frame + Actor::sleep_frame)
		<< (Ucscript::npc_frame + Actor::kneel_frame)
		<< (Ucscript::npc_frame + Actor::standing);
	scr->start(1);
	return (this);
	}

/*
 *	Check to see if an actor taking a step is really blocked.
 */

bool Actor::is_really_blocked
	(
	Tile_coord& t,
	bool force
	)
	{
	if (abs(t.tz - get_tile().tz) > 1)
		return true;
	Game_object *block = find_blocking(t, get_direction(t));
	if (!block)
		return true;		// IE, water.
	if (block == this)
		return false;
					// Try to get blocker to move aside.
	if (block->move_aside(this, get_direction(block)))
		return false;
	// (May have swapped places.)  If okay, try one last time.
	return (t != get_tile() && is_blocked(t, 0, force ? MOVE_ALL : 0));
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
		int speed = action->get_speed();
		int delay = action->handle_event(this);
		if (!delay)
			{	// Action finished.
				// This makes for a smoother scrolling and prevents the
				// avatar from skipping a step when walking.
			frame_time = speed;
			if (!frame_time)	// Not a path. Add a delay anyway.
				frame_time = gwin->get_std_delay();
			delay = frame_time;
			set_action(0);
			}

		gwin->get_tqueue()->add(
				curtime + delay, this, udata);
		}
	else if (in_usecode_control() || get_flag(Obj_flags::paralyzed))
		// Keep trying if we are in usecode control.
		gwin->get_tqueue()->add(
				curtime + gwin->get_std_delay(), this, udata);
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
	int frame,			// New frame #.
	bool force
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
	if (is_blocked(t, 0, force ? MOVE_ALL : 0))
		{
		if (is_really_blocked(t, force))
			{
			if (schedule)		// Tell scheduler.
				schedule->set_blocked(t);
			stop();
			return (0);
			}
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
	Game_object * /* attacker */
	)
	{
	if (gwin->in_combat())
		gwin->toggle_combat();	// Hope this is safe....
	Actor::set_flag(Obj_flags::dead);
	gumpman->close_all_gumps();	// Obviously.
					// Special function for dying:
	Usecode_function_data *info = Shapeinfo_lookup::GetAvUsecode(0);
	ucmachine->call_usecode(info->fun_id, this,
			(Usecode_machine::Usecode_events)info->event_id);
	}

/*
 *	Set the shapenum based on skin color, sex, naked flag and petra and polymorph flag
 */
void Actor::set_actor_shape()
{
	if (get_npc_num() != 0 || get_skin_color() < 0 ||
		(get_flag(Obj_flags::polymorph) && !get_flag(Obj_flags::naked)))
		return;

	int sn;
	int female = get_type_flag(tf_sex)?1:0;
	Skin_data *skin = Shapeinfo_lookup::GetSkinInfoSafe(this);
	
	if (!skin ||	// Should never happen, but hey...
		(!sman->have_si_shapes() &&
			Shapeinfo_lookup::IsSkinImported(
			get_flag(Obj_flags::naked) ? skin->naked_shape : skin->shape_num)))
		sn = Shapeinfo_lookup::GetBaseAvInfo(female)->shape_num;
	else
		sn = get_flag(Obj_flags::naked) ? skin->naked_shape : skin->shape_num;

#ifdef DEBUG
	cerr << "Setting Shape to " << sn << endl;
#endif
	set_shape(sn, get_framenum());
	set_file(SF_SHAPES_VGA);
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
	if (shape == Shapeinfo_lookup::GetMaleAvShape() ||
		shape == Shapeinfo_lookup::GetFemaleAvShape())
	{
		Actor *avatar = gwin->get_main_actor();
		if (!avatar) return;

		int female = avatar->get_type_flag(tf_sex)?1:0;

		Skin_data *skin = Shapeinfo_lookup::GetSkinInfoSafe(this);
		shape = avatar->get_flag(Obj_flags::naked) ? skin->naked_shape : skin->shape_num;
	}
	set_file(SF_SHAPES_VGA);

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
		set_polymorph (Shapeinfo_lookup::GetMaleAvShape());
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
		return Shapeinfo_lookup::GetFemaleAvShape();
	else
		return Shapeinfo_lookup::GetMaleAvShape();
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
	if (is_dead() && !cheat.in_map_editor())
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
	if (cheat.in_map_editor() && party_id < 0 || !can_act())
		{
		gwin->get_tqueue()->add(
				curtime + gwin->get_std_delay(), this, udata);
		return;
		}
	// Prevent actor from doing anything if not in the active map.
	// ... but not if the NPC is not on the map (breaks pathfinding
	// from offscreen if NPC not on map).
	if (get_map() && get_map() != gwin->get_map())
		{
		set_action(0);
		dormant = true;
		if (schedule)
			schedule->im_dormant();
		return;
		}
	if (schedule && party_id < 0 && can_act() && 
			(schedule_type != Schedule::combat ||	// Not if already in combat.
						// Patrol schedule already does this.
				schedule_type != Schedule::patrol) &&
			!rand()%3)	// Don't do it every time.
		schedule->seek_foes();

	if (!action)			// Not doing anything?
		{
		if (in_usecode_control() || !can_act())
				// Can't move on our own. Keep trying.
			gwin->get_tqueue()->add(
					curtime + gwin->get_std_delay(), this, udata);
		else if (schedule)
			{
				// Should try seeking foes?
			if (party_id < 0 && can_act() &&
						// Not if already in combat.
					(schedule_type != Schedule::combat ||
						// Patrol schedule already does this.
						schedule_type != Schedule::patrol) &&
					!rand()%4)	// Don't do it every time.
				{
				schedule->seek_foes();
					// Get back into queue.
				gwin->get_tqueue()->add(
						curtime + gwin->get_std_delay(), this, udata);
				}
			else
				schedule->now_what();
			}
		}
	else
		{			// Do what we should.
		int delay = party_id < 0 ? gwin->is_time_stopped() : 0;
		if (delay <= 0)		// Time not stopped?
			{
			int speed = action->get_speed();
			delay = action->handle_event(this);
			if (!delay)
				{	// Action finished. Add a slight delay.
				frame_time = speed;
				if (!frame_time)	// Not a path. Add a delay anyway.
					frame_time = gwin->get_std_delay();
				delay = frame_time;
				set_action(0);
				}
			}
		gwin->get_tqueue()->add(
				curtime + delay, this, udata);
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
	int frame,			// New frame #.
	bool force
	)
	{
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
	Game_object *block;
	if (is_blocked(t, 0, force ? MOVE_ALL : 0))
		{
		if (is_really_blocked(t, force))
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
	set_invalid();
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
	) const
	{
	return npc_num;
	}

/*
 *	Write out body and its members.
 */

void Dead_body::write_ireg
	(
	DataSource *out
	)
	{
	unsigned char buf[21];		// 13-byte entry - Exult extension.
	uint8 *ptr = write_common_ireg(13, buf);
	Game_object *first = objects.get_first(); // Guessing: +++++
	unsigned short tword = first ? first->get_prev()->get_shapenum() 
									: 0;
	Write2(ptr, tword);
	*ptr++ = 0;			// Unknown.
	*ptr++ = get_quality();
	int npc = get_live_npc_num();	// If body, get source.
		// Here, store NPC # more simply.
	Write2(ptr, npc);	// Allowing larger range of NPC bodies.
	*ptr++ = (get_lift()&15)<<4;	// Lift 
	*ptr++ = (unsigned char)get_obj_hp();		// Resistance.
					// Flags:  B0=invis. B3=okay_to_take.
	*ptr++ = (get_flag(Obj_flags::invisible) != 0) +
		 ((get_flag(Obj_flags::okay_to_take) != 0) << 3);
	out->write((char*)buf, ptr - buf);
	write_contents(out);		// Write what's contained within.
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
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


