//-*-Mode: C++;-*-
/*
Copyright (C) 2000  Dancer A.L Vesperman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _SDL_mapping_h_
#define _SDL_mapping_h_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#include <SDL_audio.h>
#include <SDL_timer.h>
#include <SDL_thread.h>

/*
 * Include the SDL headers and provide wrappers inside an SDL namespace
 *
 */

namespace	SDL	{

#if 0
	typedef	SDL_Surface	Surface;
	typedef	SDL_Rect	Rect;
	typedef	SDL_Event	Event;
#endif

	typedef SDL_Thread	Thread;

extern	void (*Delay)(Uint32);

// Audio functions
extern	void (*PauseAudio)(int pause);
extern	void (*UnlockAudio)(void);
extern	void (*LockAudio)(void);
extern	void (*MixAudio)(Uint8 *dst, Uint8 *src, Uint32 len,int);
extern	void (*CloseAudio)(void);
extern int (*OpenAudio)(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);


// Thread functions
extern	Thread *(*CreateThread)(int (*fn)(void *), void *data);



}; // Namespace
#endif
