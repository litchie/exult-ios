/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Schedule.h - Schedules for characters.
 **
 **	Written: 6/6/2000 - JSF
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

#include "alpha_kludges.h"

#include "schedule.h"
#include "actors.h"
#include "Astar.h"
#include "Zombie.h"
#include "gamewin.h"
#include "actions.h"
#include "dir.h"
#include "items.h"
#include "game.h"
#include "paths.h"
#include "ucmachine.h"

using std::cout;
using std::endl;
using std::rand;

/*
 *	Create. 
 */
Schedule::Schedule
	(
	Actor *n
	) : npc(n), blocked(-1, -1, -1)
	{
	prev_type = npc ? npc->get_schedule_type() : -1;
	}

/*
 *	Set up an action to get an actor to a location (via pathfinding), and
 *	then execute another action when he gets there.
 */

void Schedule::set_action_sequence
	(
	Actor *actor,			// Whom to activate.
	Tile_coord dest,		// Where to walk to.
	Actor_action *when_there,	// What to do when he gets there.
	int from_off_screen		// Have actor walk from off-screen.
	)
	{
	Actor_action *act = when_there;
	Tile_coord actloc = actor->get_abs_tile_coord();
	if (from_off_screen)
		actloc.tx = actloc.ty = -1;
	if (dest != actloc)		// Get to destination.
		{
		Actor_action *w = new Path_walking_actor_action(new Astar());
		Actor_action *w2 = w->walk_to_tile(actloc, dest, 
						actor->get_type_flags());
		if (w2 != w)
			delete w;
		if (!w2)		// Failed?  Teleport.
			w2 = new Move_actor_action(dest);
					// And teleport if blocked walking.
		Actor_action *tel = new Move_actor_action(dest);
					// Walk there, then do whatever.
		act = new Sequence_actor_action(w2, tel, act);
		}
	actor->set_action(act);
	actor->start();			// Get into time queue.
	}

/*
 *	Create a horizontal pace schedule.
 */

Pace_schedule *Pace_schedule::create_horiz
	(
	Actor *n
	)
	{
	int tx, ty, tz;			// Get his position.
	n->get_abs_tile(tx, ty, tz);
	return (new Pace_schedule(n, Tile_coord(tx - 4, ty, tz),
					Tile_coord(tx + 4, ty, tz)));
	}

/*
 *	Create a vertical pace schedule.
 */

Pace_schedule *Pace_schedule::create_vert
	(
	Actor *n
	)
	{
	int tx, ty, tz;			// Get his position.
	n->get_abs_tile(tx, ty, tz);
	return (new Pace_schedule(n, Tile_coord(tx, ty - 4, tz),
					Tile_coord(tx, ty + 4, tz)));
	}

/*
 *	Schedule change for pacing:
 */

void Pace_schedule::now_what
	(
	)
	{
	which = !which;			// Flip direction.
	int delay = 750;		// Delay .75 secs.
	if (blocked.tx != -1 &&		// Blocked?
	    !npc->is_monster())
		{
		Game_window *gwin = Game_window::get_game_window();
		Game_object *obj = Game_object::find_blocking(blocked);
		blocked.tx = -1;
		if (obj && (obj->get_npc_num() > 0 || 
					obj == gwin->get_main_actor()))
			{
			npc->say(first_move_aside, last_move_aside);
			delay = 1200;	// Wait longer.
			}
		}
					// Wait 1 sec. before moving.
	npc->walk_to_tile(which ? p1 : p0, 250, delay);
	}

/*
 *	Eat at inn.
 */

void Eat_at_inn_schedule::now_what
	(
	)
	{
	int frnum = npc->get_framenum();
	if ((frnum&0xf) != Actor::sit_frame)
		{			// First have to sit down.
		static int chairs[] = {873,292};
		Game_object *chair = npc->find_closest(chairs, 
					sizeof(chairs)/sizeof(chairs[0]));
		if (!chair)
			{		// Try again in a while.
			npc->start(250, 20000);
			return;
			}
		Sit_schedule::set_action(npc, chair);
		return;
		}
	Game_window *gwin = Game_window::get_game_window();
	Game_object_vector foods;			// Food nearby?
	int cnt = npc->find_nearby(foods, 377, 2, 0);
	if (cnt)			// Found?
		{			// Find closest.
		Game_object *food = 0;
		int dist = 500;
		for (Game_object_vector::const_iterator it = foods.begin();
					it != foods.end(); ++it)
			{
			Game_object *obj = *it;
			int odist = obj->distance(npc);
			if (odist < dist)
				{
				dist = odist;
				food = obj;
				}
			}
		if (rand()%5 == 0)
			{
			gwin->add_dirty(food);
			food->remove_this();
			}
		const char *msgs[] = {"Gulp!", "Mmmmm.", "Yum!", ""};
		int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
		npc->say(msgs[n]);
		}
					// Wake up in a little while.
	npc->start(250, 5000 + rand()%12000);
	}

/*
 *	Preach:
 */

void Preach_schedule::now_what
	(
	)
	{
	if (first)			// Find podium.
		{
		first = 0;
		Game_object_vector vec;
		if (npc->find_nearby(vec, 697, 5, 0))
			{
			Game_object *podium = vec[0];
			npc->walk_to_tile(podium->get_abs_tile_coord());
			return;
			}
		}
	char frames[8];			// Frames.
	int cnt = 1 + rand()%(sizeof(frames) - 1);
					// Frames to choose from:
	static char choices[3] = {0, 9, 14};
	for (int i = 0; i < cnt - 1; i++)
		frames[i] = npc->get_dir_framenum(
					choices[rand()%(sizeof(choices))]);
					// Make last one standing.
	frames[cnt - 1] = npc->get_dir_framenum(Actor::standing);
	npc->set_action(new Frames_actor_action(frames, cnt, 250));
					// Do it in 3-?? seconds.
	npc->start(250, 3000 + rand()%8000);
	}

/*
 *	Schedule change for patroling:
 */

void Patrol_schedule::now_what
	(
	)
	{
	const int PATH_SHAPE = 607;
	pathnum++;			// Find next path.
					// Already know its location?
	Game_object *path =  pathnum < paths.size() ? paths[pathnum] : 0;
	if (!path)			// No, so look around.
		{
		Game_object_vector nearby;
		npc->find_nearby(nearby, PATH_SHAPE, 26, 0);
		for (Game_object_vector::const_iterator it = nearby.begin();
						it != nearby.end(); ++it)
			{
			Game_object *obj = *it;
			int framenum = obj->get_framenum();
					// Cache it.
			paths.put(framenum, obj);
			if (framenum == pathnum)
				{	// Found it.
				path = obj;
				break;
				}
			}
		if (!path)		// Turn back if at end.
			{
			pathnum = 0;
			path = paths.size() ? paths[0] : 0;
			}
		if (!path)
			{
#if 0
			cout << "Couldn't find patrol path " << pathnum
				<< " for obj. shape " << npc->get_shapenum()
								<< endl;
#endif
					// Wiggle a bit.
			Tile_coord pos = npc->get_abs_tile_coord();
			Tile_coord delta = Tile_coord(rand()%3 - 1,
					rand()%3 - 1, 0);
			npc->walk_to_tile(pos + delta, 250, 500);
			int pathcnt = paths.size();
			pathnum = rand()%(pathcnt < 4 ? 4 : pathcnt);
			return;
			}
		}
					// This works for passion play:
	npc->set_lift(path->get_lift());
					// Delay up to 2 secs.
	npc->walk_to_tile(path->get_abs_tile_coord(), 250, rand()%2000);
	}

/*
 *	Schedule change for 'talk':
 */

void Talk_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();

	// Switch to phase 3 if we are reasonable close
	if (phase != 0 && npc->distance(gwin->get_main_actor()) < 7)
		phase = 3;
		

	switch (phase)
		{
	case 0:				// Start by approaching Avatar.
		{
		if (npc->distance(gwin->get_main_actor()) > 50)
			return;		// But not if too far away.
		PathFinder *path = new Astar();
					// Aim for within 5 tiles.
		Fast_pathfinder_client cost(5);
		if (!path->NewPath(npc->get_abs_tile_coord(),
			gwin->get_main_actor()->get_abs_tile_coord(), &cost))
			{
			delete path;
			cout << "Talk: Failed to find path for " << 
						npc->get_name() << endl;
			npc->follow(gwin->get_main_actor());
			}
		else
			{
					// Walk there, and retry if
					//   blocked.
			npc->set_action(new Path_walking_actor_action(
								path, 1));
			npc->start(400, 250);	// Start walking.
			}
		phase++;
		return;
		}
	case 1:				// Wait a second.
	case 2:
		{
		int dx = 1 - 2*(rand()%2);
		npc->walk_to_tile(npc->get_abs_tile_coord() +
			Tile_coord(dx, -dx, 0), 300, 500);
					// Wait til conversation is over.
		if (gwin->get_usecode()->get_num_faces_on_screen() == 0)
//			phase++;
			phase = 3;
		return;
		}
	case 3:				// Talk.
		gwin->add_dirty(npc);	// But first face Avatar.
		npc->set_frame(npc->get_dir_framenum(npc->get_direction(
				gwin->get_main_actor()), Actor::standing));
		gwin->add_dirty(npc);
		phase++;
					// NOTE:  This could DESTROY us!
		if (Game::get_game_type() == SERPENT_ISLE)
			npc->activate(gwin->get_usecode(), 9);
		else
			npc->activate(gwin->get_usecode(), 1);
					// SO don't refer to any instance
					//   variables from here on.
		gwin->set_mode(Game_window::normal);
		gwin->paint();
		return;
	default:
		break;
		}
	}

/*
 *	Create a loiter schedule.
 */

Loiter_schedule::Loiter_schedule
	(
	Actor *n,
	int d				// Distance in tiles to roam.
	) : Schedule(n), center(n->get_abs_tile_coord()), dist(d)
	{
	}

/*
 *	Schedule change for 'loiter':
 */

void Loiter_schedule::now_what
	(
	)
	{
	int newx = center.tx - dist + rand()%(2*dist);
	int newy = center.ty - dist + rand()%(2*dist);
					// Wait a bit.
	npc->walk_to_tile(newx, newy, center.tz, 350, rand()%2000);
	}

/*
 *	Schedule change for kid games.
 */

void Kid_games_schedule::now_what
	(
	)
	{
	Tile_coord pos = npc->get_abs_tile_coord();
	Actor *kid = 0;			// Get a kid to chase.
					// But don't run too far.
	while (!kids.empty())
	{
		kid = kids.pop();
		if (npc->distance(kid) < 16)
			break;
		kid = 0;
	}

	if (kid)
	{
		Fast_pathfinder_client cost(1);
		PathFinder *path = new Astar();
		if (path->NewPath(pos, kid->get_abs_tile_coord(), &cost))
		{
			npc->set_action(new Path_walking_actor_action(
								path, 0));
			npc->start(100, 250); // Run.
			return;
		}
	}
	else				// No more kids?  Search.
	{
		Actor_vector vec;
		npc->find_nearby_actors(vec, c_any_shapenum, 16);
		for (Actor_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
			Actor *act = *it;
			if (act->get_schedule_type() == kid_games)
				kids.push(act);
		}
	}
	Loiter_schedule::now_what();	// Wander around the start.
}

/*
 *	Schedule change for 'dance':
 */

void Dance_schedule::now_what
	(
	)
	{
	Tile_coord dest = center;	// Pick new spot to walk to.
	dest.tx += center.tx -dist + rand()%(2*dist);
	dest.ty += -dist + rand()%(2*dist);
	Tile_coord cur = npc->get_abs_tile_coord();
	int dir = (int) Get_direction4(cur.ty - dest.ty, dest.tx - cur.tx);
	char frames[4];
	for (int i = 0; i < 4; i++)
					// Spin with 'hands outstretched'.
		frames[i] = npc->get_dir_framenum((2*(dir + i))%8, 15);
					// Create action to walk.
	Actor_action *walk = new Path_walking_actor_action(new Zombie());
	walk->walk_to_tile(cur, dest, npc->get_type_flags());
					// Walk, then spin.
	npc->set_action(new Sequence_actor_action(walk,
		new Frames_actor_action(frames, sizeof(frames), 100)));
	npc->start(200, 500);		// Start in 1/2 sec.
	}

/*
 *	Schedule change for mining/farming:
 */

void Tool_schedule::now_what
	(
	)
	{
	if (!tool)			// First time?
		{
		tool = new Ireg_game_object(toolshape, 0, 0, 0, 0);
					// Free up both hands.
		Game_object *obj = npc->get_readied(Actor::rhand);
		if (obj)
			obj->remove_this();
		if ((obj = npc->get_readied(Actor::lhand)) != 0)
			obj->remove_this();
		if (!npc->add_readied(tool, Actor::lrhand))
			npc->add_readied(tool, Actor::lhand);
		}
	if (rand()%4 == 0)		// 1/4 time, walk somewhere.
		{
		Loiter_schedule::now_what();
		return;
		}
	char frames[12];		// Use pick.
	int cnt = npc->get_attack_frames(rand()%8, frames);
	npc->set_action(new Frames_actor_action(frames, cnt));
	npc->start();			// Get back into time queue.
	}

/*
 *	End of mining/farming:
 */

void Tool_schedule::ending
	(
	int
	)
	{
	if (tool)
		tool->remove_this();	// Should safely remove from NPC.
	}

/*
 *	Schedule change for hounding the Avatar.
 */

void Hound_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_abs_tile_coord(),
		   npcpos = npc->get_abs_tile_coord();
					// How far away is Avatar?
	int dist = npcpos.distance(avpos);
	if (dist > 16 || dist < 3)	// Too far, or close enough?
		{			// Check again in a few seconds.
		npc->start(250, 500 + rand()%2000);
		return;
		}
	int newdist = 1 + rand()%2;	// Aim for about 3 tiles from Avatar.
	Fast_pathfinder_client cost(newdist);
	PathFinder *path = new Astar();
	avpos.tx += rand()%3 - 1;	// Vary a bit randomly.
	avpos.ty += rand()%3 - 1;
	if (path->NewPath(npcpos, avpos, &cost))
		{
		npc->set_action(new Path_walking_actor_action(path, 0));
		npc->start(250, 50);
		}
	else				// Try again.
		npc->start(250, 2000 + rand()%3000);
	}

/*
 *	Schedule change for 'wander':
 */

void Wander_schedule::now_what
	(
	)
	{
	Tile_coord pos = npc->get_abs_tile_coord();
	const int legdist = 32;
					// Go a ways from current pos.
	pos.tx += -legdist + rand()%(2*legdist);
	pos.ty += -legdist + rand()%(2*legdist);
					// Don't go too far from center.
	if (pos.tx - center.tx > dist)
		pos.tx = center.tx + dist;
	else if (center.tx - pos.tx > dist)
		pos.tx = center.tx - dist;
	if (pos.ty - center.ty > dist)
		pos.ty = center.ty + dist;
	else if (center.ty - pos.ty > dist)
		pos.ty = center.ty - dist;
	Tile_coord dest(-1, -1, -1);	// Find a free spot.
	for (int i = 0; i < 4 && dest.tx == -1; i++)
		dest = npc->find_unblocked_tile(pos, i, 4);
	if (dest.tx == -1 || !npc->walk_path_to_tile(dest, 250, rand()%2000))
					// Failed?  Try again a little later.
		npc->start(250, rand()%3000);
	}

/*
 *	Create a sleep schedule.
 */

Sleep_schedule::Sleep_schedule
	(
	Actor *n
	) : Schedule(n), bed(0)
	{
	floorloc.tx = -1;		// Set to invalid loc.
	}

/*
 *	Schedule change for 'sleep':
 */

void Sleep_schedule::now_what
	(
	)
	{
	int frnum = npc->get_framenum();
	if ((frnum&0xf) == Actor::sleep_frame)
		return;			// Already sleeping.
					// Find closest EW or NS bed.
	static int bedshapes[2] = {696, 1011};
	Game_window *gwin = Game_window::get_game_window();
	bed = npc->find_closest(bedshapes, 2);
	if (!bed)
		{			// Just lie down at current spot.
		gwin->add_dirty(npc);
		int dirbits = npc->get_framenum()&(0x30);
		npc->set_frame(Actor::sleep_frame|dirbits);
		gwin->add_dirty(npc);
		return;
		}
	Game_object_vector tops;			// Want to find top of bed.
	bed->find_nearby(tops, bed->get_shapenum(), 1, 0);
	for (Game_object_vector::const_iterator it = tops.begin(); it != tops.end();
									++it)
			{
			Game_object *top = *it;
			int frnum = top->get_framenum();
			if (frnum >= 3 && frnum <= 16)
				{
				bed = top;
				break;
				}
			}
	int dir = bed->get_shapenum() == 696 ? west : north;
	npc->set_frame(npc->get_dir_framenum(dir, Actor::sleep_frame));
					// Get bed info.
	Shape_info& info = Game_window::get_game_window()->get_info(bed);
	Tile_coord bedloc = bed->get_abs_tile_coord();
	floorloc = npc->get_abs_tile_coord();
	int bedframe = bed->get_framenum();// Unmake bed.
	if (bedframe >= 3 && bedframe < 16 && (bedframe%2))
		{
		bedframe++;
		bed->set_frame(bedframe);
		gwin->add_dirty(bed);
		}
	int bedspread = (bedframe >= 4 && !(bedframe%2));
					// Put NPC on top of bed.
	npc->move(bedloc.tx, bedloc.ty, bedloc.tz + 
				(bedspread ? 0 : info.get_3d_height()));
	}

/*
 *	Wakeup time.
 */

void Sleep_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	if (new_type == (int) wait)	// Needed for Skara Brae.
		return;			// ++++Does this leave NPC's stuck?++++
	if (bed)			// Locate free spot.
		{		//++++++Got to look on floor.
		for (int dist = 1; dist < 8 && floorloc.tx == -1; dist++)
			floorloc = npc->find_unblocked_tile(dist, 4);
					// Make bed.
		int frnum = bed->get_framenum();
		Actor_vector occ;		// Unless there's another occupant.
		if (frnum >= 4 && frnum <= 16 && !(frnum%2) &&
				bed->find_nearby_actors(occ, c_any_shapenum, 0) < 2)
			bed->set_frame(frnum - 1);
		}
	if (floorloc.tx >= 0)		// Get back on floor.
		npc->move(floorloc);
	npc->set_frame(Actor::standing);
	Game_window *gwin = Game_window::get_game_window();
	gwin->set_all_dirty();		// Update all, since Av. stands up.
	}

/*
 *	Create a 'sit' schedule.
 */

Sit_schedule::Sit_schedule
	(
	Actor *n,
	Game_object *ch			// Chair, or null to find one.
	) : Schedule(n), chair(ch)
	{
	}

/*
 *	Schedule change for 'sit':
 */

void Sit_schedule::now_what
	(
	)
	{
	int frnum = npc->get_framenum();
	if ((frnum&0xf) == Actor::sit_frame)
		{			// Already sitting.
					// Seat on barge?
		if (!chair || chair->get_shapenum() != 292)
			return;
		Game_window *gwin = Game_window::get_game_window();
		if (gwin->get_moving_barge())
			return;		// Already moving.
		if (npc->get_party_id() < 0 && npc != gwin->get_main_actor())
			return;		// Not a party member.
		Actor *party[9];	// See if all sitting.
		int cnt = gwin->get_party(&party[0], 1);
		for (int i = 0; i < cnt; i++)
			if ((party[i]->get_framenum()&0xf) != Actor::sit_frame)
				return;	// Nope.
		int bargeshape = 961;	// Find barge.
		Game_object *barge = chair->find_closest(&bargeshape, 1);
		if (!barge)
			return;
		Game_object_vector fman;// See if Ferryman nearby.
		int usefun = 0x634;	// I hate using constants like this.
		if (chair->find_nearby(fman, 155, 8, 0) == 1)
			{
			usefun = 0x61c;
			}
					// Special usecode for barge pieces:
		gwin->get_usecode()->call_usecode(usefun, barge,
					Usecode_machine::double_click);
		return;
		}
	static int chairs[] = {873,292};
	if (!chair)			// Find chair if not given.
		if (!(chair = npc->find_closest(chairs, 
					sizeof(chairs)/sizeof(chairs[0]))))
			return;
	set_action(npc, chair);
	}

/*
 *	Set up action.
 */

void Sit_schedule::set_action
	(
	Actor *actor,
	Game_object *chairobj
	)
	{
	Tile_coord chairloc = chairobj->get_abs_tile_coord();
	switch (chairobj->get_framenum()%4)
		{			// Figure where to sit.
	case 0:				// North.
		chairloc.ty--; break;
	case 1:				// East.
		chairloc.tx++; break;
	case 2:				// South.
		chairloc.ty++; break;
	case 3:				// West.
		chairloc.tx--; break;
		}
	char frames[2];
					// Frame 0 faces N, 1 E, etc.
	int dir = 2*(chairobj->get_framenum()%4);
	frames[0] = actor->get_dir_framenum(dir, Actor::to_sit_frame);
	frames[1] = actor->get_dir_framenum(dir, Actor::sit_frame);
	Actor_action *act = new Frames_actor_action(frames, sizeof(frames));
					// Walk there, then sit.
	set_action_sequence(actor, chairloc, act);
	}

/*
 *	Schedule change for 'shy':
 */

void Shy_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_abs_tile_coord(),
		   npcpos = npc->get_abs_tile_coord();
					// How far away is Avatar?
	int dist = npcpos.distance(avpos);
	if (dist > 9)			// Far enough?
		{			// Check again in a few seconds.
		if (rand()%3)		// Just wait.
			npc->start(250, 1000 + rand()%2000);
		else			// Sometimes wander.
			npc->walk_to_tile(
				Tile_coord(npcpos.tx + rand()%6 - 3,
					npcpos.ty + rand()%6 - 3, npcpos.tz));
		return;
		}
					// Get deltas.
	int dx = npcpos.tx - avpos.tx, dy = npcpos.ty - avpos.ty;
	int adx = dx < 0 ? -dx : dx;
	int ady = dy < 0 ? -dy : dy;
					// Which is farthest?
	int farthest = adx < ady ? ady : adx;
	int factor = farthest < 3 ? 3 : farthest < 5 ? 2 : 1;
					// Walk away.
	Tile_coord dest = npcpos + Tile_coord(dx*factor, dy*factor, 0);
	Tile_coord delta = Tile_coord(rand()%3 - 1, rand()%3 - 1, 0);
	npc->walk_to_tile(dest + delta, 250, 250);
	}


/*
 *	Create a 'waiter' schedule.
 */

Waiter_schedule::Waiter_schedule
	(
	Actor *n
	) : Schedule(n), first(1), startpos(n->get_abs_tile_coord()), customer(0)
		
	{
	}

/*
 *	Get a new customer & walk to a prep. table.
 */

void Waiter_schedule::get_customer
	(
	)
{
	if (customers.empty())			// Got to search?
	{
		Actor_vector vec;		// Look within 32 tiles;
		npc->find_nearby_actors(vec, c_any_shapenum, 32);
		for (Actor_vector::const_iterator it = vec.begin();
							it != vec.end(); ++it)
		{		// Filter them.
			Actor *npc = (Actor *) *it;
			if (npc->get_schedule_type() == Schedule::eat_at_inn)
				customers.push(npc);
		}
	}

	if (!customers.empty())
		customer = customers.pop();
	
	if (prep_tables.size())	// Walk to a 'prep' table.
	{
		Game_object *table = prep_tables[rand()%prep_tables.size()];
		Tile_coord pos = table->find_unblocked_tile(1, 3);
		if (pos.tx != -1 &&
		    npc->walk_path_to_tile(pos, 200, 1000 + rand()%1000))
			return;
	}
	const int dist = 8;		// Bad luck?  Walk randomly.
	int newx = startpos.tx - dist + rand()%(2*dist);
	int newy = startpos.ty - dist + rand()%(2*dist);
	npc->walk_to_tile(newx, newy, startpos.tz, 350, rand()%2000);
}

/*
 *	Find tables and categorize them.
 */

void Waiter_schedule::find_tables
	(
	int shapenum
	)
	{
	Game_object_vector vec;
	npc->find_nearby(vec, shapenum, 32, 0);
	int floor = npc->get_lift()/5;	// Make sure it's on same floor.
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end();
								++it)
		{
		Game_object *table = *it;
		if (table->get_lift()/5 != floor)
			continue;
		Game_object_vector chairs;		// No chairs by it?
		if (!table->find_nearby(chairs, 873, 3, 0) &&
		    !table->find_nearby(chairs, 292, 3, 0))
			prep_tables.append(table);
		else
			eating_tables.append(table);
		}
	}

/*
 *	Find serving spot for a customer.
 *
 *	Output:	1 if found, with spot set.
 */

int Waiter_schedule::find_serving_spot
	(
	Tile_coord& spot
	)
	{
	Game_object_vector plates;		// First look for a nearby plate.
	int cnt = npc->find_nearby(plates, 717, 1, 0);
	if (!cnt)
		cnt = npc->find_nearby(plates, 717, 2, 0);
	int floor = npc->get_lift()/5;	// Make sure it's on same floor.
	for (Game_object_vector::const_iterator it = plates.begin();
					it != plates.end(); ++it)
		{
		Game_object *plate = *it;
		if (plate->get_lift()/5 == floor)
			{
			spot = plate->get_abs_tile_coord();
			spot.tz++;	// Just above plate.
			return 1;
			}
		}
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord cpos = customer->get_abs_tile_coord();
					// Go through tables.
	for (Game_object_vector::const_iterator it = eating_tables.begin();
					it != eating_tables.end(); ++it)
		{
		Game_object *table = *it;
		Rectangle foot = table->get_footprint();
		if (foot.distance(cpos.tx, cpos.ty) <= 2)
			{		// Found it.
			spot = cpos;	// Start here.
					// East/West of table?
			if (cpos.ty >= foot.y && cpos.ty < foot.y + foot.h)
				spot.tx = cpos.tx <= foot.x ? foot.x
							: foot.x + foot.w - 1;
			else		// North/south.
				spot.ty = cpos.ty <= foot.y ? foot.y
							: foot.y + foot.h - 1;
			if (foot.has_point(spot.tx, spot.ty))
				{	// Passes test.
				Shape_info& info = gwin->get_info(table);
				spot.tz = table->get_lift() +
						info.get_3d_height();
				return 1;
				}
			}
		}
	return 0;			// Failed.
	}

/*
 *	Schedule change for 'waiter':
 */

void Waiter_schedule::now_what
	(
	)
	{
	if (first)			// First time?
		{
		first = 0;		// Find tables.
		find_tables(971);
		find_tables(633);
		find_tables(847);
		find_tables(1003);
		find_tables(1018);
		find_tables(890);
		find_tables(964);
		}
	int dist = customer ? npc->distance(customer) : 5000;
	if (dist > 32)			// Need a new customer?
		{
		get_customer();		// Find one, and walk to a prep. table.
		return;
		}
	Game_window *gwin = Game_window::get_game_window();
	if (dist < 3)			// Close enough to customer?
		{
		Game_object_vector foods;
		if (customer->find_nearby(foods, 377, 2, 0) > 0)
			{
			const char *msgs[] = {"You look like you're doing fine.",
					"Everything okay?",
					"Ready for dessert?",
					""
					};
			int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
			npc->say(msgs[n]);
			}
		else			// Needs food.
			{
			Game_object *food = npc->get_readied(Actor::lhand);
			Tile_coord spot;
			if (food && food->get_shapenum() == 377 &&
			    find_serving_spot(spot))
				{
				const char *msgs[] = {"Enjoy!",
						"Specialty of the house!",
						""
						};
				gwin->add_dirty(npc);
				npc->set_frame(npc->get_dir_framenum(
					npc->get_direction(customer),
							Actor::standing));
				gwin->add_dirty(npc);
				npc->remove(food);
				food->set_invalid();
				food->move(spot);
				int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
				npc->say(msgs[n]);
				}
			}
		customer = 0;		// Done with this one.
		npc->start(250, rand()%3000);
		return;
		}
					// Walk to customer with food.
	if (!npc->get_readied(Actor::lhand))
		{			// Acquire some food.
		int nfoods = gwin->get_shape_num_frames(377);
		int frame = rand()%nfoods;
		Game_object *food = new Ireg_game_object(377, frame, 0, 0, 0);
		npc->add_readied(food, Actor::lhand);
		}
	for (int i = 1; i < 3; i++)
		{
		Tile_coord dest = customer->find_unblocked_tile(i, 3);
		if (dest.tx != -1 && npc->walk_path_to_tile(dest, 250,
								rand()%1000))
			return;			// Walking there.
		}
	npc->start(200, 2000 + rand()%4000);	// Failed so try again later.
	}

/*
 *	Waiter schedule is done.
 */

void Waiter_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
					// Remove what he/she is carrying.
	Game_object *obj = npc->get_readied(Actor::lhand);
	if (obj)
		obj->remove_this();
	obj = npc->get_readied(Actor::rhand);
	if (obj)
		obj->remove_this();
	}



/*
 *	Open door that's blocking the NPC, and set action to walk past and
 *	close it.
 */

void Walk_to_schedule::open_door
	(
	Game_object *door
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord cur = npc->get_abs_tile_coord();
					// Get door's footprint in tiles.
	Rectangle foot = door->get_footprint();
	Tile_coord past;		// Tile on other side of door.
	past.tz = cur.tz;
	int dir;			// Get dir to face door afterwards.
	if (foot.w > foot.h)		// Horizontal?
		{
		past.tx = foot.x + foot.w/2;
		if (cur.ty <= foot.y)	// N. of door?
			{
			past.ty = foot.y + foot.h;
			dir = 0;
			}
		else			// S. of door?
			{
			past.ty = foot.y - 1;
			dir = 4;
			}
		}
	else				// Vertical.
		{
		past.ty = foot.y + foot.h/2;
		if (cur.tx <= foot.x)	// W. of door?
			{
			past.tx = foot.x + foot.w;
			dir = 6;
			}
		else			// E. of door?
			{
			past.tx = foot.x - 1;
			dir = 2;
			}
		}
	past = Game_object::find_unblocked_tile(past, 1, 3);
					// Open it.
	door->activate(gwin->get_usecode());
	if (past.tx != -1)		// Succeeded.  Walk past and close it.
		{
		char frames[2];
		frames[0] = npc->get_dir_framenum(dir, Actor::standing);
		frames[1] = npc->get_dir_framenum(dir, 3);
		set_action_sequence(npc, past,
			new Sequence_actor_action(
				new Frames_actor_action(frames, 
							sizeof(frames)),
				new Activate_actor_action(door)));

		}
	else
		npc->start();		// Else just put back in queue.
	}

/*
 *	Modify goal to walk off the screen.
 */

void Walk_to_schedule::walk_off_screen
	(
	Rectangle& screen,		// In tiles, area slightly larger than
					//   actual screen.
	Tile_coord& goal		// Modified for path offscreen.
	)
	{
					// Destination.
	if (goal.tx >= screen.x + screen.w)
		{
		goal.tx = screen.x + screen.w - 1;
		goal.ty = -1;
		}
	else if (goal.tx < screen.x)
		{
		goal.tx = screen.x;
		goal.ty = -1;
		}
	else if (goal.ty >= screen.y + screen.h)
		{
		goal.ty = screen.y + screen.h - 1;
		goal.tx = -1;
		}
	else if (goal.ty < screen.y)
		{
		goal.ty = screen.y;
		goal.tx = -1;
		}
	}

/*
 *	Create schedule for walking to the next schedule's destination.
 */

Walk_to_schedule::Walk_to_schedule
	(
	Actor *n,
	Tile_coord d,			// Destination.
	int new_sched			// Schedule when we get there.
	) : Schedule(n), dest(d), new_schedule(new_sched), retries(0), legs(0)
	{
	first_delay = 2*(rand()%10000);	// Delay 0-20 secs.
	}

/*
 *	Schedule change for walking to another schedule destination.
 */

void Walk_to_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (npc->get_abs_tile_coord().distance(dest) <= 3)
		{			// Close enough!
		npc->set_schedule_type(new_schedule);
		return;
		}
	if (legs >= 12 || retries >= 2)	// Trying too hard?
		{			// Going to jump there.
		npc->move(dest.tx, dest.ty, dest.tz);
		npc->set_schedule_type(new_schedule);
		return;
		}
					// Get screen rect. in tiles.
	Rectangle screen = gwin->get_win_tile_rect();
	screen.enlarge(1);		// Enlarge in all dirs.
					// Might do part of it first.
	Tile_coord from = npc->get_abs_tile_coord(),
		   to = dest;
					// Destination off the screen?
	if (!screen.has_point(to.tx, to.ty))
		{
		if (!screen.has_point(from.tx, from.ty))
			{		// Force teleport on next tick.
			retries = 100;
			npc->walk_to_tile(dest, 200, 100);
			return;
			}
					// Modify 'dest'. to walk off.
		walk_off_screen(screen, to);
		}
	else if (!screen.has_point(from.tx, from.ty))
					// Modify src. to walk from off-screen.
		walk_off_screen(screen, from);
					// Blocked by a door?
	if (legs > 0 && !retries &&
	    npc->get_abs_tile_coord().distance(blocked) == 1)
		{
		Game_object *door = Game_object::find_blocking(blocked);
		if (door != 0 && door->is_closed_door())
					// Try to open it.
			{
			open_door(door);
			blocked = Tile_coord(-1, -1, -1);
			return;		// Action is set.
			}
		}
	blocked = Tile_coord(-1, -1, -1);
	cout << "Finding path to schedule for " << npc->get_name() << endl;
					// Create path to dest., delaying
					//   0 to 2 seconds.
	if (!npc->walk_path_to_tile(from, to, 200, 
						first_delay + rand()%2000))
		{			// Wait 1 sec., then try again.
		cout << "Failed to find path for " << npc->get_name() << endl;
		npc->walk_to_tile(dest, 200, 1000);
		retries++;		// Failed.  Try again next tick.
		}
	else				// Okay.  He's walking there.
		{
		legs++;
		retries = 0;
		}
	first_delay = 0;
	}

/*
 *	Don't go dormant when walking to a schedule.
 */

void Walk_to_schedule::im_dormant
	(
	)
	{
	Walk_to_schedule::now_what();	// Get there by any means.
	}

/*
 *	Set a schedule.
 */

void Schedule_change::set
	(
	unsigned char *entry		// 4 bytes read from schedule.dat.
	)
	{
	time = entry[0]&7;
	type = entry[0]>>3;
	x = entry[1];
	y = entry[2];
	superchunk = entry[3];
	}

/*
 *	Get position.
 */

Tile_coord Schedule_change::get_pos() const
	{
	int cx = 16*(superchunk%12) + x/16,
	    cy = 16*(superchunk/12) + y/16,
	    tx = x%16,
	    ty = y%16;
	return Tile_coord(cx*c_tiles_per_chunk + tx, cy*c_tiles_per_chunk + ty, 0);
	}



