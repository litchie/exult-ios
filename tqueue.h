/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Tqueue.h - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#ifndef INCL_TQUEUE
#define INCL_TQUEUE	1

/*
 *	An interface for entries in the queue:
 */
class Time_sensitive
	{
public:
	virtual void handle_event(unsigned long curtime, long udata) = 0;
	};

/*
 *	A queue entry:
 */
class Queue_entry
	{
	Queue_entry *next, *prev;	// Next, prev. in queue.
	unsigned long time;			// Time when this is due.
	Time_sensitive *handler;	// Object to activate.
	long udata;			// Data to pass to handler.
	friend class Time_queue;
	void set(unsigned long t, Time_sensitive *h, long ud)
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
					// Activate head + any others due.
	void activate0(unsigned long curtime);
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
	void add(unsigned long t, Time_sensitive *obj, long ud);
					// Remove object's entry.
	void remove(Time_sensitive *obj);
					// Activate entries that are 'due'.
	void activate(unsigned long curtime)
		{
		if (head && !(curtime < head->time))
			activate0(curtime);
		}
	};

#endif	/* INCL_TQUEUE */
