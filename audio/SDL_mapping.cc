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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#include "SDL_mapping.h"
#include <SDL.h>

void (*SDL::Delay)(Uint32)=::SDL_Delay;
void (*SDL::PauseAudio)(int)=::SDL_PauseAudio;

void (*SDL::UnlockAudio)(void)=::SDL_UnlockAudio;
void (*SDL::LockAudio)(void)=::SDL_LockAudio;
void (*SDL::MixAudio)(Uint8 *dst, const Uint8 *src, Uint32 len,int)=::SDL_MixAudio;
void (*SDL::CloseAudio)(void)=::SDL_CloseAudio;
int (*SDL::OpenAudio)(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)=::SDL_OpenAudio;
void (*SDL::QuitSubSystem)(Uint32)=::SDL_QuitSubSystem;
int (*SDL::InitSubSystem)(Uint32)=::SDL_InitSubSystem;


// Thread functions
SDL::Thread *(*SDL::CreateThread)(int (*fn)(void *), void *data)=::SDL_CreateThread;

