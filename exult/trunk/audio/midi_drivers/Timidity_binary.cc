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

#ifdef XWIN

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <csignal>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <iostream>
#endif

#include "fnames.h"

using std::cerr;
using std::endl;

#if HAVE_TIMIDITY_BIN
#include "Timidity_binary.h"

static Mix_Music *mixermusic;

//
// Woohoo. This is easy with SDL_mixer! :)
//

Timidity_binary::Timidity_binary() 
{
	//Point to music finish cleanup code
	Mix_HookMusicFinished(music_complete_callback);
}

Timidity_binary::~Timidity_binary()
{
	// Stop any current player
	stop_track();
	stop_sfx();
}

void Timidity_binary::stop_track(void)
{
	Mix_HaltMusic();
	if(mixermusic)
	{
		Mix_FreeMusic(mixermusic);
		mixermusic = NULL;
	}
}
//Clean up last track played, freeing memory each time
void Timidity_binary::music_complete_callback(void)
{
	if(mixermusic)
	{
		Mix_FreeMusic(mixermusic);
		mixermusic = NULL;
	}
}

bool	Timidity_binary::is_playing(void)
{
	return Mix_PlayingMusic()!=0;
}

void	Timidity_binary::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

#if DEBUG
	cerr << "Starting midi sequence with Timidity_binary" << endl;
#endif
	stop_track();

	mixermusic = Mix_LoadMUS(name);
	Mix_PlayMusic(mixermusic, repeat);
	Mix_VolumeMusic(MIX_MAX_VOLUME - 50);	//Balance volume with other music types
}

void	Timidity_binary::start_sfx(XMIDIEventList *event_list)
{
}

void	Timidity_binary::stop_sfx(void)
{
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal SDL_mixer timidity synthesiser";
}

#endif // HAVE_TIMIDITY_BIN

#endif // XWIN
