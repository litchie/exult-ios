/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Tqueue.h - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#ifndef INCL_TQUEUE
#define INCL_TQUEUE	1


#include <stack.h>
#include <vector>
#include <list>



/*
 *	An interface for entries in the queue:
 */
class Time_sensitive
	{
	int queue_cnt;			// # of entries for this in queue.
public:
	friend class Time_queue;
	Time_sensitive() : queue_cnt(0)
		{  }
	int in_queue()
		{ return queue_cnt > 0; }
	virtual void handle_event(unsigned long curtime, long udata) = 0;
	};

class Time_queue;

/*
 *	A queue entry:
 */
class Queue_entry
	{
	public:
	// Queue_entry *next, *prev;	// Next, prev. in queue.
	Time_sensitive *handler;	// Object to activate.
	long udata;			// Data to pass to handler.
	unsigned long time;			// Time when this is due.
	void set(unsigned long t, Time_sensitive *h, long ud)
		{
		time = t;
		handler = h;
		udata = ud;
		}
	};

bool	operator <(const Queue_entry &q1,const Queue_entry &q2);

/*
 *	Time-based queue.  The entries are kept sorted in increasing order
 *	by time.
 */
class Time_queue
	{
	typedef list<Queue_entry>	Temporal_sequence;
	Temporal_sequence data;

	// Activate head + any others due.
	void activate0(unsigned long curtime);
#if 0
	void add_freed(Queue_entry *ent)
		{
		return;
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
#endif
public:
	// Time_queue() : head(0), free_entries(0)
	Time_queue() 
		{  }
					// Add an entry.
	void add(unsigned long t, Time_sensitive *obj, long ud);
					// Remove object's entry.
	int remove(Time_sensitive *obj);
	int find(Time_sensitive *obj);	// Find an entry.
					// Activate entries that are 'due'.
	void activate(unsigned long curtime)
		{
		// if (head && !(curtime < head->time))
		if (data.size() && !(curtime < data.front().time))
			activate0(curtime);
		}
	};

#endif	/* INCL_TQUEUE */
