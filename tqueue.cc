/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Tqueue.cc - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#include "tqueue.h"
#include <algorithm>

/*
 *	Add an entry to the queue.
 */

void Time_queue::add
	(
	unsigned long t,		// When entry is to be activated.
	Time_sensitive *obj,		// Object to be added.
	long ud				// User data.
	)
	{
	Queue_entry	newent;
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
 *	Remove & activate entries that are due, starting with head (already
 *	known to be due).
 */

void Time_queue::activate0
	(
	unsigned long curtime		// Current time.
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
		data.erase(data.begin());	// Remove from chain.
		obj->handle_event(curtime, udata);
		}
	while (data.size() && !(curtime < data.front().time));
	}
