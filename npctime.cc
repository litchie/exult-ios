/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npctime.cc - Timed-even handlers for NPC's.
 **
 **	Written: 8/4/00 - JSF
 **/

#include "npctime.h"
#include "gamewin.h"
#include "actors.h"
#include "items.h"
#include "schedule.h"

extern bool god_mode;

/*
 *	Base class for keeping track of things like poison, protection, hunger.
 */
class Npc_timer : public Time_sensitive
	{
protected:
	Npc_timer_list *list;		// Where NPC stores ->this.
	unsigned long get_minute();	// Get game minutes.
public:
	Npc_timer(Npc_timer_list *l, int start_delay = 0);
	virtual ~Npc_timer();
	};

/*
 *	Handle starvation.
 */
class Npc_hunger_timer : public Npc_timer
	{
	unsigned long last_time;	// Last game minute when penalized.
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
	unsigned long end_time;		// Time when it wears off.
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
	unsigned long end_time;		// Time when it wears off.
public:
	Npc_sleep_timer(Npc_timer_list *l);
	virtual ~Npc_sleep_timer();
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};
#if 0
/*
 *	Invisibility timer.
 */
class Invisibility_timer : public Npc_timer
	{
public:
	
#endif
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
 *	Get game minute from start.
 */

unsigned long Npc_timer::get_minute
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	return 60*gwin->get_total_hours() + gwin->get_minute();
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
	Game_window *gwin = Game_window::get_game_window();
	gwin->get_tqueue()->add(SDL_GetTicks() + start_delay, this, 0L);
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
		Time_queue *tq = Game_window::get_game_window()->get_tqueue();
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
	Game_window *gwin = Game_window::get_game_window();
	Actor *npc = list->npc;
					// No longer a party member?
	if ((npc != gwin->get_main_actor() && npc->get_party_id() < 0) ||
					//   or no longer hungry?
	    npc->get_property((int) Actor::food_level) >= 0)
		{
		delete this;
		return;
		}
	unsigned long minute = get_minute();
					// Once/hour.
	if (minute >= last_time + 60)
		{
		int hp = rand()%3;
		if (rand()%4)
			npc->say(first_starving, first_starving + 2);
//++++Problems		npc->reduce_health(hp);

		npc->set_property((int) Actor::health,
			npc->get_property((int) Actor::health) - hp);
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
	end_time = SDL_GetTicks() + 60000 + rand()%120000;
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
	Game_window *gwin = Game_window::get_game_window();
	Actor *npc = list->npc;
	if (curtime >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Actor::poisoned) == 0)
		{
		npc->clear_flag(Actor::poisoned);
		delete this;
		return;
		}
	int penalty = rand()%3;
	if (penalty && rand()%4)
		npc->say(first_ouch, last_ouch);
//+++Problems	npc->reduce_health(penalty);

	npc->set_property((int) Actor::health,
		npc->get_property((int) Actor::health) - penalty);
					// Check again in 10-20 secs.
	gwin->get_tqueue()->add(curtime + 10000 + rand()%10000, this, 0L);
	}

/*
 *	Initialize sleep timer.
 */

Npc_sleep_timer::Npc_sleep_timer
	(
	Npc_timer_list *l
	) : Npc_timer(l)
	{
					// Lasts 5-10 seconds..
	end_time = SDL_GetTicks() + 5000 + rand()%5000;
	}

/*
 *	Done with sleep timer.
 */

Npc_sleep_timer::~Npc_sleep_timer
	(
	)
	{
	list->sleep = 0;
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
	Game_window *gwin = Game_window::get_game_window();
	Actor *npc = list->npc;
	if (curtime >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Actor::asleep) == 0)
		{
					// Avoid waking Penumbra.
		if (npc->get_schedule_type() != Schedule::sleep)
			{
			npc->clear_flag(Actor::asleep);
			int frnum = npc->get_framenum();
			if ((frnum&0xf) == Actor::sleep_frame)
				{	// Stand up.
				gwin->add_dirty(npc);
				npc->set_frame(Actor::standing | (frnum&0x30));
				gwin->add_dirty(npc);
				}
			}
		delete this;
		return;
		}
					// Check again in 2 secs.
	gwin->get_tqueue()->add(curtime + 2000, this, 0L);
	}

