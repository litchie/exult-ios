/**
 **	Tqueue.h - A queue of time-based events for animation.
 **
 **	Written: 2/15/00 - JSF
 **/

#ifndef INCL_TQUEUE
#define INCL_TQUEUE	1


//#include <stack.h>
#include <vector>
#include <list>

#include "exult_types.h"



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
	virtual ~Time_sensitive();
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
	uint32 time;			// Time when this is due.
	inline void set(uint32 t, Time_sensitive *h, long ud)
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
	typedef std::list<Queue_entry>	Temporal_sequence;
	Temporal_sequence data;
	uint32 pause_time;		// Time when paused.

	// Activate head + any others due.
	void activate0(uint32 curtime);
public:
	friend class Time_queue_iterator;
	// Time_queue() : head(0), free_entries(0)
	Time_queue() : pause_time(0)
		{  }
	void clear();			// Remove all entries.
					// Add an entry.
	void add(uint32 t, Time_sensitive *obj, long ud);
					// Remove object's entry.
	int remove(Time_sensitive *obj);
	int remove(Time_sensitive *obj, long udata);
	int find(Time_sensitive *obj);	// Find an entry.
					// Activate entries that are 'due'.
	inline void activate(uint32 curtime)
		{
		// if (head && !(curtime < head->time))
		if (data.size() && !(curtime < data.front().time))
			activate0(curtime);
		}
	void pause(uint32 curtime)	// Game paused.
		{
		if (!pause_time)
			pause_time = curtime;
		}
	void resume(uint32 curtime);
	};


class Time_queue_iterator
	{
	Time_queue::Temporal_sequence::iterator iter;
	Time_queue *tqueue;
	Time_sensitive *this_obj;	// Only return entries for this obj.
public:
	Time_queue_iterator(Time_queue *tq, Time_sensitive *obj)
			: iter(tq->data.begin()), tqueue(tq), this_obj(obj)
		{  }
	int operator()(Time_sensitive *& obj, long& data);
	};

#endif	/* INCL_TQUEUE */
