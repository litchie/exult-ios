/**
 **	Tqueue.cc - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#include "tqueue.h"

/*
 *	Compare times.
 */
inline int operator<(timeval &t1, timeval& t2)
	{
					// Check secs., but watch for new day.
	return (t1.tv_sec < t2.tv_sec && t2.tv_sec - t1.tv_sec < 10*3600) ||
		(t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
	}

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
				newent->next = head;
				newent->prev = head->prev;
				head = newent;
				return;
				}
			else
				prev = prev->prev;
		newent->next = prev->next;
		newent->prev = prev;
		prev->next = newent;
		}
	}
