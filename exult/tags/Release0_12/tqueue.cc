/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Tqueue.cc - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#include "tqueue.h"

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
	Queue_entry *newent;
	if (free_entries)		// First try for a free entry.
		{
		newent = free_entries;
		free_entries = free_entries->next;
		}
	else
		newent = new Queue_entry();
	newent->set(t, obj, ud);	// Store data.
	if (!head)			// Empty queue?
		head = newent->next = newent->prev = newent;
	else
		{			// Find where to insert it.
		Queue_entry *prev = head->prev;
		while (t < prev->time)
					// Before head of chain?
			if (prev == head)
				{
				head = newent;
				prev = prev->prev;
				break;
				}
			else
				prev = prev->prev;
		newent->next = prev->next;
		newent->prev = prev;
		prev->next->prev = newent;
		prev->next = newent;
		}
	}

/*
 *	Remove first entry containing a given object.
 */

void Time_queue::remove
	(
	Time_sensitive *obj
	)
	{
	if (!head)
		return;			// Empty.
	Queue_entry *ent = head;
	do
		{
		if (ent->handler == obj)// Found it?
			{
			if (ent == head)
				remove_head();
			else
				remove_non_head(ent);
			return;
			}
		ent = ent->next;
		}
	while (ent != head);
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
	do
		{
		Queue_entry *ent = head;
		Time_sensitive *obj = head->handler;
		long udata = head->udata;
		remove_head();		// Remove from chain.
		obj->handle_event(curtime, udata);
		}
	while (head && !(curtime < head->time));
	}
