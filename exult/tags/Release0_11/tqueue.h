/**
 **	Tqueue.h - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#ifndef INCL_TQUEUE
#define INCL_TQUEUE	1

#include <sys/time.h>

/*
 *	Compare times.
 */
inline int operator<(timeval &t1, timeval& t2)
	{
					// Check secs.
	return (t1.tv_sec < t2.tv_sec) ||
		(t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
	}

/*
 *	Add a given # of microseconds to a time.
 */
inline timeval Add_usecs
	(
	timeval t,
	int usecs
	)
	{
	timeval result;
	result.tv_usec = t.tv_usec + usecs;
	result.tv_sec = t.tv_sec + result.tv_usec/1000000;
	result.tv_usec %= 1000000;
	return result;
	}

/*
 *	An interface for entries in the queue:
 */
class Time_sensitive
	{
public:
	virtual void handle_event(timeval curtime, long udata) = 0;
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
	void activate0(timeval curtime);// Activate head + any others due.
	void add_freed(Queue_entry *ent)
		{
		ent->next = free_entries;
		free_entries = ent;
		}		
	void remove_head()		// Remove head of chain.
		{
		Queue_entry *ent = head;
		if (head == head->next)
			head = 0;
		else
			{
			head->prev->next = head->next;
			head->next->prev = head->prev;
			head = head->next;
			}
		add_freed(ent);		// Add to free list.
		}
					// Remove non-head.
	void remove_non_head(Queue_entry *ent)
		{
		ent->prev->next = ent->next;
		ent->next->prev = ent->prev;
		add_freed(ent);
		}
public:
	Time_queue() : head(0), free_entries(0)
		{  }
					// Add an entry.
	void add(timeval t, Time_sensitive *obj, long ud);
					// Remove object's entry.
	void remove(Time_sensitive *obj);
	void activate(timeval curtime)	// Activate entries that are 'due'.
		{
		if (head && !(curtime < head->time))
			activate0(curtime);
		}
	};

#endif	/* INCL_TQUEUE */
