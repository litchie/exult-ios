/**
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
	timeval t,			// When entry is to be activated.
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
 *	Remove & activate entries that are due, starting with head (already
 *	known to be due).
 */

void Time_queue::activate0
	(
	timeval curtime			// Current time.
	)
	{
	do
		{
		Queue_entry *ent = head;
		ent->handler->handle_event(curtime, ent->udata);
					// Remove head of chain.
		if (head == head->next)
			head = 0;
		else
			{
			head->prev->next = head->next;
			head->next->prev = head->prev;
			head = head->next;
			}
					// Add to free list.
		ent->next = free_entries;
		free_entries = ent;
		}
	while (head && !(curtime < head->time));
	}
