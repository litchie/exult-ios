/**
 **	Tqueue.h - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#ifndef INCL_TQUEUE
#define INCL_TQUEUE	1

#include <sys/time.h>

/*
 *	An interface for entries in the queue:
 */
class Time_sensitive
	{
public:
	virtual void activate(timeval curtime, long udata) = 0;
	};

/*
 *	A queue entry:
 */
class Queue_entry
	{
	Queue_entry *next, *prev;	// Next, prev. in queue.
	timeval time;			// Time when this is due.
	Time_sensitive *handler;	// Object to activate.
	long udata;			// Data to pass to handler.
	friend class Time_queue;
	void set(timeval t, Time_sensitive *h, long ud)
		{
		time = t;
		handler = h;
		udata = ud;
		}
	};

/*
 *	Time-based queue.  The entries are kept sorted in increasing order
 *	by time.
 */
class Time_queue
	{
	Queue_entry *head;		// Head of queue.  Head->prev = tail.
	Queue_entry *free_entries;	// ->list of freed entries.
public:
	Time_queue() : head(0), free_entries(0)
		{  }
					// Add an entry.
	void add(timeval t, Time_sensitive *obj, long ud);
	void activate(timeval curtime);	// Activate entries that are 'due'.
	};

#endif	/* INCL_TQUEUE */
