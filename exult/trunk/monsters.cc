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
#include "gamemap.h"

using std::rand;

Monster_actor *Monster_actor::in_world = 0;

/*
 *	Add to global list (if not already there).
 */

void Monster_actor::link_in
	(
	)
	{
	if (prev_monster || in_world == this)
		return;			// Already in list.
	if (in_world)			// Add to head.
		in_world->prev_monster = this;
	next_monster = in_world;
	in_world = this;
	}

/*
 *	Remove from global list (if in list).
 */

void Monster_actor::link_out
	(
	)
	{
	if (next_monster)
		next_monster->prev_monster = prev_monster;
	if (prev_monster)
		prev_monster->next_monster = next_monster;
	else				// We're at start of list.
		if (in_world == this)
			in_world = next_monster;
	next_monster = prev_monster = 0;
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
	    next_monster(0), animator(0)
	{
					// Check for animated shape.
	Shape_info& info = get_info();
	if (info.is_animated())
		animator = Animator::create(this, 1);
	}

/*
 *	Delete.
 */

Monster_actor::~Monster_actor
	(
	)
	{
	delete animator;
	link_out();			// Remove from chain.
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
	Tile_coord pos,			// Where to place it.  If pos.tx < 0,
					//   it's not placed in the world.
	int sched,			// Schedule type.
	int align,			// Alignment.
	bool temporary,
	bool equipment
	)
	{
					// Get 'monsters.dat' info.
	const Monster_info *inf = ShapeID::get_info(shnum).get_monster_info();
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
	monster->set_invalid();		// Place in world.
	if (pos.tx >= 0)
		monster->move(pos.tx, pos.ty, pos.tz);
	if (equipment) {
					// Get equipment.
		int equip_offset = inf->equip_offset;
		Equip_record *equip = inf->equip;
		if (equip_offset && equip_offset - 1 < inf->equip_cnt)
		{
			Equip_record& rec = equip[equip_offset - 1];
			for (size_t i = 0;
				 i < sizeof(equip->elements)/sizeof(
							equip->elements[0]);
				 i++)
			{		// Give equipment.
				Equip_element& elem = rec.elements[i];
				if (!elem.shapenum || 1 + rand()%100 > 
						elem.probability)
					continue;// You lose.
				int frnum = (elem.shapenum == 377) ? 
					Find_monster_food(shnum) : 0;
				monster->create_quantity(elem.quantity, 
						elem.shapenum, c_any_qual, 
							frnum, temporary);
			}
		}
	}

	if (sched < 0)			// Set sched. AFTER equipping.
		sched = static_cast<int>(Schedule::loiter);
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
	}

/*
 *	Render.
 */

void Monster_actor::paint
	(
	)
	{
	// Animate first
	if (animator)			// Be sure animation is on.
		animator->want_animation();
	Npc_actor::paint();		// Draw on screen.
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
	// If move not allowed do I remove or change destination?
	// I'll do nothing for now
	if (!gwin->emulate_is_move_allowed(t.tx, t.ty))
		return (0);
	if (get_flag(Obj_flags::paralyzed))
		return 0;
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get ->new chunk.
	Map_chunk *nlist = gmap->get_chunk(cx, cy);
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
	add_dirty();			// Set to repaint old area.
					// Get old chunk.
	Map_chunk *olist = gmap->get_chunk(old_cx, old_cy);
					// Move it.
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
	movef(olist, nlist, tx, ty, frame, t.tz);
	if (!add_dirty(1) &&
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
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Monster_actor::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	link_out();			// Remove from list.
	Npc_actor::remove_this(nodel);
	}

/*
 *	Move (teleport) to a new spot.
 */

void Monster_actor::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	Npc_actor::move(newtx, newty, newlift);
	link_in();			// Insure it's in global list.
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

bool Monster_actor::add
	(
	Game_object *obj,
	bool dont_check,		// 1 to skip volume check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
	{
					// Try to add to 'readied' spot.
	if (Npc_actor::add(obj, true, combine))
		return (true);		// Successful.
					// Just add anything.
	return Container_game_object::add(obj, true, combine);
	}

/*
 *	Get total value of armor being worn.
 */

int Monster_actor::get_armor_points
	(
	)
	{
	Monster_info *inf = get_info().get_monster_info();
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
	Monster_info *inf = get_info().get_monster_info();
					// Kind of guessing here.
	Weapon_info *winf = Actor::get_weapon(points, shape);
	if (!winf)			// No readied weapon?
		{			// Look up monster itself.
		shape = 0;
		winf = get_info().get_weapon_info();
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
	}

/*
 *	Get the tiles where slimes adjacent to one in a given position should
 *	be found.
 */

static void Get_slime_neighbors
	(
	Tile_coord pos,			// Position to look around.
	Tile_coord *neighbors		// N,E,S,W tiles returned.
	)
	{
					// Offsets to neighbors 2 tiles away.
	static int offsets[8] = {0,-2, 2,0, 0,2, -2,0};
	for (int dir = 0; dir < 4; dir++)
		neighbors[dir] = pos +Tile_coord(offsets[2*dir],
						    offsets[2*dir + 1], 0);
	}

/*
 *	Find whether a slime is a neighbor of a given spot.
 *
 *	Output:	Direction (0-3 for N,E,S,W), or -1 if not found.
 */

int Find_neighbor
	(
	Game_object *slime,
	Tile_coord *neighbors		// Neighboring spots to check.
	)
	{
	Tile_coord pos = slime->get_tile();
	for (int dir = 0; dir < 4; dir++)
		if (pos == neighbors[dir])
			return dir;
	return -1;			// Not found.
	}

/*
 *	Update the frame of a slime and its neighbors after it has been moved.
 *	The assumption is that slimes are 2x2 tiles, and that framenum/2 is
 *	based on whether there are adjoining slimes to the N, W, S, or E, with
 *	bit 0 being random.
 */

void Slime_actor::update_frames
	(
	Tile_coord src,			// May be invalid (tx = -1).
	Tile_coord dest			// May be invalid.  If src & dest are
					//   both valid, we assume they're at
					//   most 2 tiles apart.
	)
	{
	Tile_coord neighbors[4];	// Gets surrounding spots for slimes.
	int dir;			// Get direction of neighbor.
	Game_object_vector nearby;		// Get nearby slimes.
	if (src.tx != -1)
		if (dest.tx != -1)	// Assume within 2 tiles.
			Game_object::find_nearby(nearby, dest, 529, 4, 8);
		else
			Game_object::find_nearby(nearby, src, 529, 2, 8);
	else				// Assume they're both not invalid.
		Game_object::find_nearby(nearby, dest, 529, 2, 8);
	if (src.tx != -1)		// Update neighbors we moved from.
		{
		Get_slime_neighbors(src, neighbors);
		for (Game_object_vector::const_iterator it = nearby.begin();
						it != nearby.end(); ++it)
			{
			Game_object *slime = *it;
			if (slime != this && 
			    (dir = Find_neighbor(slime, neighbors)) >= 0)
				{
				int ndir = (dir+2)%4;
					// Turn off bit (1<<ndir)*2, and set
					//   bit 0 randomly.
				slime->change_frame((slime->get_framenum()&
					~(((1<<ndir)*2)|1)) |(rand()%2));
				}
			}
		}
	if (dest.tx != -1)		// Update neighbors we moved to.
		{
		int frnum = 0;		// Figure our new frame too.
		Get_slime_neighbors(dest, neighbors);
		for (Game_object_vector::const_iterator it = nearby.begin();
						it != nearby.end(); ++it)
			{
			Game_object *slime = *it;
			if (slime != this && 
			    (dir = Find_neighbor(slime, neighbors)) >= 0)
				{	// In a neighboring spot?
				frnum |= (1<<dir)*2;
				int ndir = (dir+2)%4;
					// Turn on bit (1<<ndir)*2, and set
					//   bit 0 randomly.
				slime->change_frame((slime->get_framenum()&~1)|
						((1<<ndir)*2)|(rand()%2));
				}
			}
		change_frame(frnum|(rand()%2));
		}
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
	int /* frame */			// New frame # (ignored).
	)
	{
					// Save old pos.
	Tile_coord oldpos = get_tile();
	int ret = Monster_actor::step(t, -1);
					// Update surrounding frames (& this).
	Tile_coord newpos = get_tile();
	update_frames(oldpos, newpos);
	Game_object_vector blood;	// Place blood in old spot.
	if (newpos != oldpos && rand()%9 == 0 &&
	    !find_nearby(blood, oldpos, 912, 1, 0))
		{
					// Frames 4-11 are green.
		Game_object *b = gmap->create_ireg_object(912, 4 + rand()%8);
		b->set_flag(Obj_flags::is_temporary);
		b->move(oldpos);
		}
	return ret;
	}

/*
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Slime_actor::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	Tile_coord pos = get_tile();
	Monster_actor::remove_this(nodel);
					// Update surrounding slimes.
	update_frames(pos, Tile_coord(-1, -1, -1));
	}

/*
 *	Move (teleport) to a new spot.  I assume slimes only use this when
 *	being added to the world.
 */

void Slime_actor::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
					// Save old pos.
	Tile_coord pos = get_tile();
	if (get_cx() == 255)		// Invalid?
		pos = Tile_coord(-1, -1, -1);
	Monster_actor::move(newtx, newty, newlift);
					// Update surrounding frames (& this).
	update_frames(pos, get_tile());
	}
