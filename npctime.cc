/*
 *	Npctime.cc - Timed-even handlers for NPC's.
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "npctime.h"
#include "gamewin.h"
#include "gameclk.h"
#include "actors.h"
#include "items.h"
#include "schedule.h"
#include "game.h"

using std::rand;

extern bool god_mode;

/*
 *	Base class for keeping track of things like poison, protection, hunger.
 */
class Npc_timer : public Time_sensitive, public Game_singletons
	{
protected:
	Npc_timer_list *list;		// Where NPC stores ->this.
	uint32 get_minute();	// Get game minutes.
public:
	Npc_timer(Npc_timer_list *l, int start_delay = 0);
	virtual ~Npc_timer();
	};

/*
 *	Handle starvation.
 */
class Npc_hunger_timer : public Npc_timer
	{
	uint32 last_time;	// Last game minute when penalized.
public:
	Npc_hunger_timer(Npc_timer_list *l) : Npc_timer(l, 5000)
		{ last_time = get_minute(); }
	virtual ~Npc_hunger_timer();
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Handle poison.
 */
class Npc_poison_timer : public Npc_timer
	{
	uint32 end_time;		// Time when it wears off.
public:
	Npc_poison_timer(Npc_timer_list *l);
	virtual ~Npc_poison_timer();
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Handle sleep.
 */
class Npc_sleep_timer : public Npc_timer
	{
	uint32 end_time;		// Time when it wears off.
public:
	Npc_sleep_timer(Npc_timer_list *l) : Npc_timer(l)
		{			// Lasts 5-10 seconds..
		end_time = Game::get_ticks() + 5000 + rand()%5000;
		}
	virtual ~Npc_sleep_timer()
		{ list->sleep = 0; }
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Invisibility timer.
 */
class Npc_invisibility_timer : public Npc_timer
	{
	uint32 end_time;		// Time when it wears off.
public:
	Npc_invisibility_timer(Npc_timer_list *l) : Npc_timer(l)
		{			// Lasts 60-80 seconds..
		end_time = Game::get_ticks() + 60000 + rand()%20000;
		}
	virtual ~Npc_invisibility_timer()
		{ list->invisibility = 0; }
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Protection timer.
 */
class Npc_protection_timer : public Npc_timer
	{
	uint32 end_time;		// Time when it wears off.
public:
	Npc_protection_timer(Npc_timer_list *l) : Npc_timer(l)
		{			// Lasts 60-80 seconds..
		end_time = Game::get_ticks() + 60000 + rand()%20000;
		}
	virtual ~Npc_protection_timer()
		{ list->protection = 0; }
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Timer for flags that don't need any other checks.
 */
class Npc_flag_timer : public Npc_timer
	{
	int flag;			// Flag # in Obj_flags.
	uint32 end_time;		// Time when it wears off.
	Npc_flag_timer **listloc;	// Where it's stored in Npc_timer_list.
public:
	Npc_flag_timer(Npc_timer_list *l, int f, Npc_flag_timer **loc) 
		: Npc_timer(l), flag(f), listloc(loc)
		{			// Lasts 60-120 seconds..
		end_time = Game::get_ticks() + 60000 + rand()%60000;
		}
	virtual ~Npc_flag_timer()
		{ *listloc = 0; }
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};


/*
 *	Delete list.
 */

Npc_timer_list::~Npc_timer_list
	(
	)
	{
	delete hunger;
	delete poison;
	delete sleep;
	delete invisibility;
	delete protection;
	delete might;
	delete curse;
	delete paralyze;
	}

/*
 *	Start hunger (if not already there).
 */

void Npc_timer_list::start_hunger
	(
	)
	{
	if (!hunger)			// Not already there?
		hunger = new Npc_hunger_timer(this);
	}

/*
 *	Start poison.
 */

void Npc_timer_list::start_poison
	(
	)
	{
	if (poison)			// Remove old one.
		delete poison;
	poison = new Npc_poison_timer(this);
	}

/*
 *	Start sleep.
 */

void Npc_timer_list::start_sleep
	(
	)
	{
	if (sleep)			// Remove old one.
		delete sleep;
	sleep = new Npc_sleep_timer(this);
	}

/*
 *	Start invisibility.
 */

void Npc_timer_list::start_invisibility
	(
	)
	{
	if (invisibility)			// Remove old one.
		delete invisibility;
	invisibility = new Npc_invisibility_timer(this);
	}

/*
 *	Start protection.
 */

void Npc_timer_list::start_protection
	(
	)
	{
	if (protection)			// Remove old one.
		delete protection;
	protection = new Npc_protection_timer(this);
	}


/*
 *	Start might.
 */

void Npc_timer_list::start_might
	(
	)
	{
	if (might)			// Remove old one.
		delete might;
	might = new Npc_flag_timer(this, Obj_flags::might, &might);
	}

/*
 *	Start curse.
 */

void Npc_timer_list::start_curse
	(
	)
	{
	if (curse)			// Remove old one.
		delete curse;
	curse = new Npc_flag_timer(this, Obj_flags::cursed, &curse);
	}

/*
 *	Start paralyze.
 */

void Npc_timer_list::start_paralyze
	(
	)
	{
	if (paralyze)			// Remove old one.
		delete paralyze;
	paralyze = new Npc_flag_timer(this, Obj_flags::paralyzed, &paralyze);
	}

/*
 *	Get game minute from start.
 */

uint32 Npc_timer::get_minute
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	return 60*gclock->get_total_hours() + gclock->get_minute();
	}

/*
 *	Start timer.
 */

Npc_timer::Npc_timer
	(
	Npc_timer_list *l,
	int start_delay			// Time in msecs. before starting.
	) : list(l)
	{
	gwin->get_tqueue()->add(Game::get_ticks() + start_delay, this, 0L);
	}

/*
 *	Be sure we're no longer in the time queue.
 */

Npc_timer::~Npc_timer
	(
	)
	{
	if (in_queue())
		{
		Time_queue *tq = Game_window::get_instance()->get_tqueue();
		tq->remove(this);
		}
	}

/*
 *	Done with hunger timer.
 */

Npc_hunger_timer::~Npc_hunger_timer
	(
	)
	{
	list->hunger = 0;
	}

/*
 *	Time to penalize for hunger.
 */

void Npc_hunger_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
					// No longer a party member?
	if (!npc->is_in_party() ||
					//   or no longer hungry?
	    npc->get_property(static_cast<int>(Actor::food_level)) >= 0 ||
	    npc->is_dead())		// Obviously.
		{
		delete this;
		return;
		}
	uint32 minute = get_minute();
					// Once/hour.
	if (minute >= last_time + 60)
		{
		int hp = rand()%3;
		if (rand()%4)
			npc->say(first_starving, first_starving + 2);
		npc->reduce_health(hp);

//		npc->set_property(static_cast<int>(Actor::health),
//			npc->get_property(static_cast<int>(Actor::health)) - hp);
		last_time = minute;
		}
	gwin->get_tqueue()->add(curtime + 30000, this, 0L);
	}

/*
 *	Initialize poison timer.
 */

Npc_poison_timer::Npc_poison_timer
	(
	Npc_timer_list *l
	) : Npc_timer(l, 5000)
	{
					// Lasts 1-3 minutes.
	end_time = Game::get_ticks() + 60000 + rand()%120000;
	}

/*
 *	Done with poison timer.
 */

Npc_poison_timer::~Npc_poison_timer
	(
	)
	{
	list->poison = 0;
	}

/*
 *	Time to penalize for poison, or see if it's worn off.
 */

void Npc_poison_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
	if (curtime >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Obj_flags::poisoned) == 0 ||
	    npc->is_dead())		// Obviously.
		{
		npc->clear_flag(Obj_flags::poisoned);
		delete this;
		return;
		}
	int penalty = rand()%3;
	if (penalty && rand()%4)
		npc->say(first_ouch, last_ouch);
	npc->reduce_health(penalty);

//	npc->set_property(static_cast<int>(Actor::health),
//		npc->get_property(static_cast<int>(Actor::health)) - penalty);
					// Check again in 10-20 secs.
	gwin->get_tqueue()->add(curtime + 10000 + rand()%10000, this, 0L);
	}

/*
 *	Time to see if we should wake up.
 */

void Npc_sleep_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
	if (curtime >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Obj_flags::asleep) == 0)
		{
					// Avoid waking Penumbra.
		if (npc->get_schedule_type() != Schedule::sleep &&
		    !npc->is_dead())	// Don't wake the dead.
			{
			npc->clear_flag(Obj_flags::asleep);
			int frnum = npc->get_framenum();
			if ((frnum&0xf) == Actor::sleep_frame &&
					// Slimes don't change.
			    !npc->get_info().has_strange_movement())
					// Stand up.
				npc->change_frame(
					Actor::standing | (frnum&0x30));
			}
		delete this;
		return;
		}
					// Check again in 2 secs.
	gwin->get_tqueue()->add(curtime + 2000, this, 0L);
	}

/*
 *	Check for a given ring.
 */

inline int Wearing_ring
	(
	Actor *actor,
	int shnum,			// Ring shape to look for.
	int frnum
	)
	{
					// See if wearing ring.
	Game_object *ring = actor->get_readied(Actor::lfinger);
	if (ring && ring->get_shapenum() == shnum &&
	    ring->get_framenum() == frnum)
		return 1;
	ring = actor->get_readied(Actor::rfinger);
	if (ring && ring->get_shapenum() == shnum &&
	    ring->get_framenum() == frnum)
		return 1;
	return 0;
	}

/*
 *	See if invisibility wore off.
 */

void Npc_invisibility_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
	if (Wearing_ring(npc, 296, 0))	// (Works for SI and BG.)
		{			// Wearing invisibility ring.
		delete this;		// Don't need timer.
		return;
		}
	if (curtime >= end_time ||	// Long enough?  Or cleared.
	    npc->get_flag(Obj_flags::invisible) == 0)
		{
		npc->clear_flag(Obj_flags::invisible);
		if (!npc->is_dead())
			gwin->add_dirty(npc);
		delete this;
		return;
		}
					// Check again in 2 secs.
	gwin->get_tqueue()->add(curtime + 2000, this, 0L);
	}

/*
 *	See if protection wore off.
 */

void Npc_protection_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
	if (Wearing_ring(npc, 297, 0))	// ++++SI has an Amulet.
		{			// Wearing protection ring.
		delete this;		// Don't need timer.
		return;
		}
	if (curtime >= end_time ||	// Long enough?  Or cleared.
	    npc->get_flag(Obj_flags::protection) == 0)
		{
		npc->clear_flag(Obj_flags::protection);
		if (!npc->is_dead())
			gwin->add_dirty(npc);
		delete this;
		return;
		}
					// Check again in 2 secs.
	gwin->get_tqueue()->add(curtime + 2000, this, 0L);
	}

/*
 *	Might/curse/paralyze wore off.
 */

void Npc_flag_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Actor *npc = list->npc;
	if (curtime >= end_time ||	// Long enough?  Or cleared.
	    npc->get_flag(flag) == 0)
		{
		npc->clear_flag(flag);
		delete this;
		}
	else				// Check again in 10 secs.
		gwin->get_tqueue()->add(curtime + 10000, this, 0L);
	}

