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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <cstdio>
#endif
#include <new>
#if 0
// Some people are having trouble with this
#include <pthread_alloc>	// This allocator defines memset (we think)
#else
#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#endif

unsigned long exult_mem = 0L;		// Keep track of total memory allocated

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
	exult_mem += n;
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
	exult_mem += n;
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
