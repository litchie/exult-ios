/*
 *	monsters.cc - Monsters.
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

#include "monsters.h"
#include "monstinf.h"
#include "gamewin.h"
#include "animate.h"
#include "schedule.h"
#include "chunks.h"
#include "Audio.h"

Monster_actor *Monster_actor::in_world = 0;
int Monster_actor::in_world_cnt = 0;

/*
 *	Initialize a new monster.
 */

void Monster_actor::init
	(
	)
	{
	if (in_world)
		in_world->prev_monster = this;
	next_monster = in_world;
	in_world = this;
	in_world_cnt++;
					// Check for animated shape.
	Shape_info& info = Game_window::get_game_window()->get_info(this);
	if (info.is_animated())
		animator = Animator::create(this, 1);
	}

/*
 *	Create monster.
 */

Monster_actor::Monster_actor
	(
	const std::string &nm, 
	int shapenum, 
	int num,			// Generally -1.
	int uc
	) : Npc_actor(nm, shapenum, num, uc), prev_monster(0),
	    animator(0)
	{
	init();
	}

/*
 *	Delete.
 */

Monster_actor::~Monster_actor
	(
	)
	{
	delete animator;
					// Remove from chain.
	if (next_monster)
		next_monster->prev_monster = prev_monster;
	if (prev_monster)
		prev_monster->next_monster = next_monster;
	else				// We're at start of list.
		in_world = next_monster;
	if (!is_dead())			// Dying decrements count.
		in_world_cnt--;
	}

/*
 *	Another set of constants that should be in a data file (or that's
 *	probably already in one of the U7 data files), this one correlating
 *	a 'monster' shape with the food frame you get when you kill it.
 */
static int Monster_food[] = {
	498, 10,			// Chicken.
	500,  9,			// Cow - beef steaks.
	502,  14,			// Deer - meat.
	509, 12,			// Fish.
	811, 14,			// Rabbit - small leg.
	970, 8,				// Sheep - mutton.
	727, 23				// Horse - ribs.
	};

/*
 *	Find food frame for given monster shape.
 *
 *	Output:	Frame if found, else a random frame (0-31).
 */

static int Find_monster_food
	(
	int shnum			// Monster shape.
	)
	{
	const int cnt = sizeof(Monster_food)/(2*sizeof(Monster_food[0]));
	for (int i = 0; i < cnt; i++)
		if (Monster_food[2*i] == shnum)
			return Monster_food[2*i + 1];
	return rand()%32;
	}

/*
 *	Create an instance of a monster.
 */

Monster_actor *Monster_actor::create
	(
	int shnum			// Shape to use.
	)
	{
	if (shnum == 529)		// Slime?
					// Usecode = shape.
		return new Slime_actor("", shnum, -1, shnum);
	else
		return new Monster_actor("", shnum, -1, shnum);
	}

/*
 *	Create an instance of a monster and initialize from monstinf.dat.
 */

Monster_actor *Monster_actor::create
	(
	int shnum,			// Shape to use.
	int chunkx, int chunky,		// Chunk to place it in, or 255's to
					//   not place in world yet.
	int tilex, int tiley,		// Tile within chunk.
	int lift,			// Lift.
	int sched,			// Schedule type.
	int align,			// Alignment.
	bool temporary,
	bool equipment
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get 'monsters.dat' info.
	const Monster_info *inf = gwin->get_info(shnum).get_monster_info();
	if (!inf)
		inf = Monster_info::get_default();
	Monster_actor *monster = create(shnum);
	monster->set_alignment(align == static_cast<int>(Actor::neutral)
						? inf->alignment : align);
	// Movement flags
	if ((inf->flags >> Monster_info::fly)&1)
	{
		monster->set_type_flag(Actor::tf_fly);
	}
	if ((inf->flags >> Monster_info::swim)&1)
	{
		monster->set_type_flag(Actor::tf_swim);
	}
	if ((inf->flags >> Monster_info::walk)&1)
	{
		monster->set_type_flag(Actor::tf_walk);
	}
	if ((inf->flags >> Monster_info::ethereal)&1)
	{
		monster->set_type_flag(Actor::tf_ethereal);
	}
	monster->set_property(Actor::strength, inf->strength);
					// Max. health = strength.
	monster->set_property(Actor::health, inf->strength);
	monster->set_property(Actor::dexterity, inf->dexterity);
	monster->set_property(Actor::intelligence, inf->intelligence);
	monster->set_property(Actor::combat, inf->combat);

	// Set temporary
	if (temporary) monster->set_flag (Obj_flags::is_temporary);
					// Place in world.
	Map_chunk *olist = gwin->get_chunk_safely(chunkx, chunky);
	if (olist)
		monster->movef(0, olist, tilex, tiley, 0, lift);
	else
		monster->set_invalid();
	if (equipment) {
					// Get equipment.
		int equip_offset = inf->equip_offset;
		Equip_record *equip = inf->equip;
		if (equip_offset && equip_offset - 1 < inf->equip_cnt)
		{
			Equip_record& rec = equip[equip_offset - 1];
			for (size_t i = 0;
				 i < sizeof(equip->elements)/sizeof(equip->elements[0]);
				 i++)
			{		// Give equipment.
				Equip_element& elem = rec.elements[i];
				if (!elem.shapenum || 1 + rand()%100 > 
						elem.probability)
					continue;// You lose.
				int frnum = (elem.shapenum == 377) ? 
					Find_monster_food(shnum) : 0;
				monster->create_quantity(elem.quantity, elem.shapenum, 
										 c_any_qual, frnum, temporary);
			}
		}
	}

	if (sched < 0)			// Set sched. AFTER equipping.
		sched = (int) Schedule::loiter;
	monster->set_schedule_type(sched);
	return (monster);
	}

/*
 *	Delete all monsters.  (Should only be called after deleting chunks.)
 */

void Monster_actor::delete_all
	(
	)
	{
	while (in_world)
		delete in_world;
	in_world_cnt = 0;
	}

/*
 *	Render.
 */

void Monster_actor::paint
	(
	Game_window *gwin
	)
	{
	// Animate first
	if (animator)			// Be sure animation is on.
		animator->want_animation();
	Npc_actor::paint(gwin);		// Draw on screen.
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 *		Dormant is set if off screen.
 */

int Monster_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	
	// If move not allowed do I remove or change destination?
	// I'll do nothing for now
	if (!gwin->emulate_is_move_allowed(t.tx, t.ty))
		return (0);

					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get ->new chunk.
	Map_chunk *nlist = gwin->get_chunk(cx, cy);
	nlist->setup_cache();		// Setup cache if necessary.
					// Blocked?
	if (is_blocked(t))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		if (!gwin->add_dirty(this))
			dormant = true;	// Off-screen.
		return (0);		// Done.
		}
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty(gwin);		// Set to repaint old area.
					// Get old chunk.
	Map_chunk *olist = gwin->get_chunk(old_cx, old_cy);
					// Move it.
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
	movef(olist, nlist, tx, ty, frame, t.tz);
	if (!add_dirty(gwin, 1) &&
					// And > a screenful away?
	    distance(gwin->get_camera_actor()) > 1 + 320/c_tilesize)
		{			// No longer on screen.
		stop();
		dormant = true;
		return (0);
		}
	return (1);			// Add back to queue for next time.
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Monster_actor::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume check.
	)
	{
	if (Npc_actor::add(obj, 1))	// Try to add to 'readied' spot.
		return (1);		// Successful.
					// Just add anything.
	return Container_game_object::add(obj, 1);
	}

/*
 *	Get total value of armor being worn.
 */

int Monster_actor::get_armor_points
	(
	)
	{
	Monster_info *inf =
	    Game_window::get_game_window()->get_info(this).get_monster_info();
					// Kind of guessing here.
	return Actor::get_armor_points() + (inf ? inf->armor : 0);
	}

/*
 *	Get weapon value.
 */

Weapon_info *Monster_actor::get_weapon
	(
	int& points,
	int& shape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Monster_info *inf = gwin->get_info(this).get_monster_info();
					// Kind of guessing here.
	Weapon_info *winf = Actor::get_weapon(points, shape);
	if (!winf)			// No readied weapon?
		{			// Look up monster itself.
		shape = 0;
		winf = gwin->get_info(get_shapenum()).get_weapon_info();
		if (winf)
			points = winf->get_damage();
		else			// Guessing:
			points = inf ? inf->weapon : 0;
		}
	return winf;
	}

/*
 *	We're dead.  We're removed from the world, but not deleted.
 */

void Monster_actor::die
	(
	)
	{
	Actor::die();
	Audio::get_ptr()->start_music_combat ( CSVictory, 0);
					// Got to delete this somewhere, but
					//   doing it here crashes.
	in_world_cnt--;			// So... Decrement 'live' count here.
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 *		Dormant is set if off screen.
 */

int Slime_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
		//+++++++Figure frame.
	int ret = Monster_actor::step(t, frame);
		// ++++++Create trail.
	return ret;
	}
