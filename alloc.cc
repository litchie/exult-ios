#if !AUTOCONFIGURED
#  include "autoconfig.h"
#endif
#ifdef __DECCXX
#  include "alpha_kludges.h"
#else
#  include <cstdlib>
#  include <cstdio>
#endif
#include <new>
#if 0
// Some people are having trouble with this
#include <pthread_alloc>	// This allocator defines memset (we think)
#else
#ifndef __DECCXX
#  include <cstring>
#endif
#endif


#ifdef WANT_ALTERNATE_ALLOCATOR
#ifdef POISON_ALLOCATED_BLOCKS
#undef INITIALISE_ALLOCATED_BLOCKS
#define INITIALISE_ALLOCATED_BLOCKS 0xf1
#endif

void *operator new (size_t) throw (std::bad_alloc);
void *operator new[] (size_t) throw (std::bad_alloc);

void	*operator new(size_t n) throw (std::bad_alloc)
{
	void	*r;
	r=malloc(n);
	if(!r)
		throw std::bad_alloc();
#ifdef INITIALISE_ALLOCATED_BLOCKS
	memset(r,INITIALISE_ALLOCATED_BLOCKS,n);
#endif
	return r;
}

void	*operator new[](size_t n) throw (std::bad_alloc)
{
	void	*r=malloc(n);
	if(!r)
		throw std::bad_alloc();
#ifdef INITIALISE_ALLOCATED_BLOCKS
	memset(r,INITIALISE_ALLOCATED_BLOCKS,n);
#endif
	return r;
}

void	operator delete(void *p) throw()
{
	if(p)
		free(p);
	// C++ doesn't throw if it's asked to delete
	// a null pointer
}
void	operator delete[](void *p) throw()
{
	if(p)
		free(p);
}

#endif
