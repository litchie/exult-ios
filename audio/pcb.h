/*
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

#ifndef _PCB_H_
#define _PCB_H_

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#  include <iostream>
#endif
#include <new>
#include "SDL_mapping.h"
#include "common_types.h"

class ByteBuffer
{
  public:
	void	reserve(size_t numbytes)    // Reserve space in advance
		{
		if (!data)
			{
			data = new char[numbytes];

			low  = high = 0;
			cap  = numbytes;

			return;
			}

		// Sanity check. Do we already have at least that much?
		if (numbytes <= cap)
			{
			std::memmove(data, (data + low), (high - low));

			high -= low;
			low  =  0;

			return;
			}

		// Allocate new store
		char   *tmp = new char[numbytes];

		// Copy the old store into it.
		std::memcpy(tmp, (data + low), (high - low));

		// Reset all our offsets
		high -= low;
		low  =  0;
		cap  =  numbytes;

		// Replace the old store with the new one
		delete [] data;
		data = tmp;
		}

	ByteBuffer() : data(0), cap(0), low(0), high(0)
		{  } 
	ByteBuffer(const ByteBuffer &b) :  data(0), cap(0), low(0), high(0)
		{
		if (b.data)
			{
			reserve(b.cap);

			low  = 0;              // Is this line redundant?
			high = b.high - b.low;

			std::memcpy(data, (b.data + b.low), high);
			}

		} 

	virtual ~ByteBuffer()
		{
		if (data)
			{
			delete [] data;
			}
		}

	typedef char   *iterator;    // Define an STL-compatible iterator
    
	ByteBuffer   &operator =(const ByteBuffer &b)    // assignment operator
		{
		if (b.data)
			{
			reserve(b.cap);

			low  = 0;             // Not redundant here, I think.
			high = b.high - b.low;

			std::memcpy(data, (b.data + b.low), high);
			}
		else
			{
			if (data)
				{
				delete [] data;
				}

			low  = cap = high = 0;
			data = 0;
			}

		return *this;
		}

	char	&operator [](size_t pos)    // index operator
		{
		// Index operator. Return the character at pos.
		// NO RANGE CHECKING!

		return *(data + low + pos);
		}
    
	void	push_back(char c)    // Add-to-end of buffer
		{
		if (!data)
			{
			reserve(4096);    // An arbitrary sort of amount
			}
		else
			{
			// Check if we're getting close to the end
			if (high >= (cap - 16))
				{
				reserve(cap + 4096);
				}
			}
		data[high++] = c;
		}

	void	push_back(const char *buf, size_t len)    // add block to end of buffer
		{
		// If there is no buffer, reserve space for at least double
		// this amount
		if (!data)
			{
			reserve(len * 2);
			}
		else
			{
			// Will this chunk fit at the end with current capacity ?
			if(cap-high<len)
				{
				// Maybe if we reshuffle things a bit ?
				if(cap-high+low>len)
					{
						std::memmove(data, (data+low), (high-low));
						high -= low;
						low = 0;
					}
				else	// No luck...
					reserve(cap + (len * 2));
				}
			}

		std::memcpy((data + high), buf, len);

		high += len;
		}
					// Pull block off the front
	size_t	pop_front(char *buf, size_t len, bool repeat = false)
		{
		if (!data)
			return 0;

		if (len > (high - low))
			{
			len = high - low;
			}

		std::memcpy(buf, (data + low), len);

		low += len;

		if (low == high)
			{
			if (repeat)	// Rewind?
				low = 0;
			else		// All done.
				low = high = 0;
			}
		return len;
		}

	iterator	begin()         // return iterator pointing to 
					//   beginning of buffer
		{
		return (data + low);
		}

	iterator	end()   	// return iterator pointing to one 
					//   past end of buffer
		{
			return (data + high);
		}
#if 0
	void	erase(iterator pos)   // erase from the front 
                                     //  ONLY WORKS IF iterator == begin() !
		{
		if (!data)
			return;    // Perhaps we should throw a range_error instead?

		// The rest of this really doesn't work if pos != begin()
		if (pos == (data + low))
			{
			low++;

			// Is it empty now? If so, throw everything away
			// Perhaps we should change this to preserve a buffer
			// for speed considerations
			if (low == high)
				{
				low = high = cap = 0;

				delete [] data;

				data = 0;

				return;
				}
			}

		// We could optionally throw a range_error here, as well,
		// as the erase simply doesn't happen if we get here.

		}

	void          erase(iterator pos, size_t count)    // As above.
		{
		if (!data)
			return;    // Perhaps we should throw a range_error instead?

		// The rest of this really doesn't work if pos != begin()
		if (pos == (data + low))
			{
			while (count-- && (low != high))
				{
				low++;
				}

			// Is it empty now? If so, throw everything away
			// Perhaps we should change this to preserve a buffer
			// for speed considerations
			if (low == high)
				{
				low = high = cap = 0;

				delete [] data;

				data = 0;

				return;
				}
			}

		// We could optionally throw a range_error here, as well,
		// as the erase simply doesn't happen if we get here.
		}
#endif
	void          clear(void)           // wipe the whole buffer
		{
		low = high = cap = 0;

		if (data)
			{
			delete [] data;
			}

		data = 0;

		}
	size_t        size(void)            // return number of bytes currently buffered
		{
		return (high - low);
		}
    
  private:

    char     *data;
    size_t    cap,
              low,
              high;
};










#define	MAX_PCB_SIZE	8192


class	ProducerConsumerBuf
	{
private:
	static  uint32 sequence_cnt;	// For generating (mostly) unique ID's.
	ByteBuffer Buffer;
	SDL_mutex	*mutex;
	size_t	window;
	bool	producing,consuming;
	bool	repeat;			// Keep consuming forever.
	uint32	type;			// 'Magic' # identifying type.
	uint32  seq;			// Sequence # assigned.
	int	volume;				// 0-128.
	int	dir;			// Dir. (0-15 from N, clockwise) from
					//   observer to sound source.
	inline 	void	lock(void)
		{
#ifdef MACOS
		SDL_mutexP(mutex);
#else
		if(SDL_mutexP(mutex)!=0)
			std::cerr << "ProducerConsumerBuf::lock() failed" << std::endl;
#endif
		}
	inline 	void	unlock(void)
		{
		SDL_mutexV(mutex);
		}
public:
#ifdef DEBUG
	static	int	counter;
	int	mycounter;
#endif
	bool is_consuming() const { return consuming; }
	bool is_active() const
		{ return consuming || producing; }
	void	produce(const void *p,size_t l)
		{
		if(!l||!consuming)
			return;	// No data? Do nothing
		while(1)
			{
			lock();
			if(!consuming)
				{
				unlock();
				return;
				}
			size_t	n=Buffer.size();
			if(n>window)
				{
				unlock();
				SDL::Delay(100);
				}
			else
				break;
			}
		Buffer.push_back(reinterpret_cast<const char *>(p),l);
		unlock();
		}
	size_t	consume(void *p,size_t l)
		{
		if(!l)
			return producing?0:-1;
		lock();
		l=Buffer.pop_front(reinterpret_cast<char *>(p), l, repeat);
		unlock();
		return l?l:(producing?0:-1);
		}
	uint32 init(uint32 t)		// Initialize prior to use/reuse.
					// Returns sequence #.
		{
		type = t;
		lock();
		seq = ++sequence_cnt;
		volume = SDL_MIX_MAXVOLUME;
		dir = 0;
		producing = consuming = true;
		repeat = false;
		Buffer.clear();
		unlock();
		return seq;
		}
	int get_volume()
		{ return volume; }
	void set_volume(int v)		// Should be 0-128.
		{ volume = v; }
	int get_dir()
		{ return dir; }
	void set_dir(int d)		// Should be 0-15.
		{ dir = d; }
	void set_repeat(bool rep)
		{ repeat = rep; }
	ProducerConsumerBuf() : Buffer(),mutex(SDL_CreateMutex()),
			window(32768),producing(false),consuming(false),
			repeat(false),
			type(0), seq(0), volume(SDL_MIX_MAXVOLUME), dir(0)
		{
#ifdef DEBUG
		mycounter = counter++;
		COUT("Created PCB " << mycounter);
#endif
		 }
	~ProducerConsumerBuf()
		{
#ifdef DEBUG
		COUT("::"<<mycounter<<" ProducerConsumerBuf going away");
#endif
		SDL_DestroyMutex(mutex);
		}
	uint32	get_type(void) const
		{ return type; }
	uint32  get_seq(void) const
		{ return seq; }
	void	end_production(void)
		{
#ifdef DEBUG
		COUT("::" << mycounter << " end_production");
#endif
		lock();
		producing=false;
		unlock();
		}
	void	end_consumption(void)
		{
#if !defined(MACOS) && defined(DEBUG)
		// As always, no use of cerr etc. on MacOS during Audio Interrupt!
		COUT("::"<<mycounter<<" end_consumption");
#endif
		lock();
		consuming=false;
		unlock();
		}
	size_t	size(void) { return Buffer.size(); }
	};

#endif

