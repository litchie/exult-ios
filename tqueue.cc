/*
 *	tqueue.cc - A queue of time-based events for animation.
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

#include "tqueue.h"
#include <algorithm>
#include <SDL_timer.h>

/*
 *	Be sure no copies are still in queue when deleted.
 */

Time_sensitive::~Time_sensitive
	(
	)
	{
#if 0	/*+++++++For finding bugs. */
	if (queue_cnt > 0)
		{
		char *p = 0;
		char c = *p;		// Force crash.
		}
#endif
	}

/*
 *	Remove all entries.
 */

void Time_queue::clear
	(
	)
	{
	Temporal_sequence::iterator it;
	while ((it=data.begin()) != data.end())
		{
		Queue_entry ent = *it;
		Time_sensitive *obj = ent.handler;
		data.erase(it);
		obj->queue_cnt--;
		}
	}

/*
 *	Add an entry to the queue.
 */

void Time_queue::add
	(
	uint32 t,		// When entry is to be activated.
	Time_sensitive *obj,		// Object to be added.
	long ud				// User data.
	)
	{
	obj->queue_cnt++;		// It's going in, no matter what.
	Queue_entry	newent;
	if (paused && !obj->always)	// Paused?
					// Messy, but we need to fix time.
		t -= SDL_GetTicks() - pause_time;
	newent.set(t,obj,ud);
	if(!data.size())
		{
		data.push_back(newent);
		return;
		}
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if(newent<*it)
			{
			data.insert(it,newent);
			return;
			}
		}
	data.push_back(newent);
	}

bool    operator <(const Queue_entry &q1,const Queue_entry &q2)
{
        if(q1.time<q2.time)
                return true;
        return false;
}

/*
 *	Remove first entry containing a given object.
 *
 *	Output:	1 if found, else 0.
 */

int Time_queue::remove
	(
	Time_sensitive *obj
	)
	{
	if(data.size()==0)
		return 0;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if(it->handler==obj)
			{
			obj->queue_cnt--;
			data.erase(it);
			return 1;
			}
		}
	return (0);			// Not found.
	}

/*
 *	Remove first entry containing a given object and data.
 *
 *	Output:	1 if found, else 0.
 */

int Time_queue::remove
	(
	Time_sensitive *obj,
	long udata
	)
	{
	if(data.size()==0)
		return 0;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if(it->handler==obj && it->udata == udata)
			{
			obj->queue_cnt--;
			data.erase(it);
			return 1;
			}
		}
	return (0);			// Not found.
	}

/*
 *	See if a given entry is in the queue.
 *
 *	Output:	1 if found, else 0.
 */

int Time_queue::find
	(
	Time_sensitive *obj
	)
	{
	if(data.size()==0)
		return 0;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if(it->handler==obj)
			return 1;
		}
	return 0;
	}

/*
 *	Find when an entry is due.
 *
 *	Output:	delay in msecs. when due, or -1 if not in queue.
 */

long Time_queue::find_delay
	(
	Time_sensitive *obj,
	uint32 curtime
	)
	{
	if(data.size()==0)
		return -1;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if(it->handler==obj)
			{
			if (pause_time)	// Watch for case when paused.
				curtime = pause_time;
			long delay = it->time - curtime;
			return delay >= 0 ? delay : 0;
			}
		}
	return -1;
	}

/*
 *	Remove & activate entries that are due, starting with head (already
 *	known to be due).
 */

void Time_queue::activate0
	(
	uint32 curtime		// Current time.
	)
	{
	if(data.size()==0)
		return;
	Queue_entry ent;
	do
		{
		ent=data.front();
		Time_sensitive *obj = ent.handler;
		long udata = ent.udata;
		data.pop_front();	// Remove from chain.
		obj->queue_cnt--;
		obj->handle_event(curtime, udata);
		}
	while (data.size() && !(curtime < data.front().time));
	}

/*
 *	Remove & activate entries marked 'always'.  This is called when
 *	the queue is paused.
 */

void Time_queue::activate_always
	(
	uint32 curtime		// Current time.
	)
	{
	if(data.size()==0)
		return;
	Queue_entry ent;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end() && !(curtime < (*it).time); )
		{
		Temporal_sequence::iterator next = it;
		++next;			// Get ->next in case we erase.
		ent = *it;
		Time_sensitive *obj = ent.handler;
		if (obj->always)
			{
			obj->queue_cnt--;
			long udata = ent.udata;
			data.erase(it);
			obj->handle_event(curtime, udata);
			}
		it = next;
		}
	}

/*
 *	Resume after a pause.
 */

void Time_queue::resume
	(
	uint32 curtime
	)
	{
	if (!paused || --paused > 0)	// Only unpause when stack empty.
		return;			// Not paused.
	int diff = curtime - pause_time;
	pause_time = 0;
	if (diff < 0)			// Should not happen.
		return;
	for(Temporal_sequence::iterator it=data.begin();
		it!=data.end(); ++it)
		{
		if (!(*it).handler->always)
			it->time += diff;	// Push entries ahead.
		}
	}

/*
 *	Get next element in queue.
 */

int Time_queue_iterator::operator()
	(
	Time_sensitive *& obj,		// Main object.
	long& data			// Data that was added with it.
	)
	{
	while (iter != tqueue->data.end() && this_obj &&
						(*iter).handler != this_obj)
		++iter;
	if (iter == tqueue->data.end())
		return (0);
	obj = (*iter).handler;		// Return fields.
	data = (*iter).udata;
	++iter;				// On to the next.
	return (1);
	}

