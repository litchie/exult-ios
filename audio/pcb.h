#include <new>
#include <cstring>
#include <iostream>
#include "SDL_mapping.h"

class ByteBuffer
{
  public:
	ByteBuffer() : data(0), cap(0), low(0), high(0)
		{  } 
	ByteBuffer(const ByteBuffer &b) :  data(0), cap(0), low(0), high(0)
		{
		if (b.data)
			{
			reserve(b.cap);

			low  = 0;              // Is this line redundant?
			high = b.high - b.low;

			memcpy(data, (b.data + b.low), high);
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

			memcpy(data, (b.data + b.low), high);
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
			memmove(data, (data + low), (high - low));

			high -= low;
			low  =  0;

			return;
			}

		// Allocate new store
		char   *tmp = new char[numbytes];

		// Copy the old store into it.
		memcpy(tmp, (data + low), (high - low));

		// Reset all our offsets
		high -= low;
		low  =  0;
		cap  =  numbytes;

		// Replace the old store with the new one
		delete [] data;
		data = tmp;
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
			// We have a buffer. Check if it will hold the data. If
			// not, add double the requested length to the size
			if(cap-high<len)
				{
				reserve(cap + (len * 2));
				}
			}

		memcpy((data + high), buf, len);

		high += len;
		}

	size_t	pop_front(char *buf, size_t len)    // Pull block off the front
		{
		if (!data)
			return 0;

		if (len > (high - low))
			{
			len = high - low;
			}

		memcpy(buf, (data + low), len);

		low += len;

		if (low == high)
			{
			low = high = 0;
			}
		return len;
		}

	iterator	begin()           // return iterator pointing to beginning of buffer
		{
		return (data + low);
		}

	iterator	end()             // return iterator pointing to one past end of buffer
		{
			return (data + high);
		}

	void	erase(iterator pos)   // erase from the front 
                                     //  ONLY WORKS IF THE iterator == begin() !
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
	ByteBuffer Buffer;
	SDL_mutex	*mutex;
	size_t	window;
	inline 	void	lock(void)
		{
		SDL_mutexP(mutex);
		}
	inline 	void	unlock(void)
		{
		SDL_mutexV(mutex);
		}
	bool	producing,consuming;
public:
	Uint32	id;
	void	produce(const void *p,size_t l)
		{
		if(!l||!consuming)
			return;	// No data? Do nothing
		while(1)
			{
			lock();
			size_t	n=Buffer.size();
			if(n>window)
				{
				unlock();
				SDL::Delay(100);
				}
			else
				break;
			}
		Buffer.push_back((const char *)p,l);
		unlock();
		}
	size_t	consume(void *p,size_t l)
		{
		if(!l)
			return producing?0:-1;
		unsigned char *data=(unsigned char *)p;
		lock();
		l=Buffer.pop_front((char *)p,l);
		unlock();
		if(l>0)
			window+=(l>>3);
		return l?l:(producing?0:-1);
		}
	ProducerConsumerBuf() : Buffer(),mutex(SDL_CreateMutex()),producing(true),consuming(true),id(0),window(2048)
		{  }
	~ProducerConsumerBuf()
		{
		SDL_DestroyMutex(mutex);
		}
	void	end_production(void)
		{
		cerr << " end_production" << endl;
		lock();
		producing=false;
		if(!consuming)
			delete this;
		else
			unlock();
		}
	void	end_consumption(void)
		{
		cerr << " end_consumption" << endl;
		lock();
		consuming=false;
		if(!producing)
			delete this;
		else
			unlock();
		}
	size_t	size(void) { return Buffer.size(); }
	};


