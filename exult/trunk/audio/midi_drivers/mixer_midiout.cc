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

//#ifndef ALPHA_LINUX_CXX
//#  include <unistd.h>
//#  include <csignal>
//#  include <sys/types.h>
//#  include <sys/stat.h>
//#  include <fcntl.h>
//#endif

#include <iostream>

#include "fnames.h"

using std::cerr;
using std::endl;

#include "mixer_midiout.h"

static Mix_Music *mixermusic;

//
// Woohoo. This is easy with SDL_mixer! :)
//

Mixer_MidiOut::Mixer_MidiOut() 
{
	//Point to music finish cleanup code
	Mix_HookMusicFinished(music_complete_callback);
}

Mixer_MidiOut::~Mixer_MidiOut()
{
	// Stop any current player
	stop_track();
	stop_sfx();
}

void Mixer_MidiOut::stop_track(void)
{
	Mix_HaltMusic();
	if(mixermusic)
	{
		Mix_FreeMusic(mixermusic);
		mixermusic = NULL;
	}
}

//Clean up last track played, freeing memory each time
void Mixer_MidiOut::music_complete_callback(void)
{
	if(mixermusic)
	{
		Mix_FreeMusic(mixermusic);
		mixermusic = NULL;
	}
}

bool	Mixer_MidiOut::is_playing(void)
{
	return Mix_PlayingMusic()!=0;
}

void	Mixer_MidiOut::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

#if DEBUG
	cerr << "Starting midi sequence with Mixer_MidiOut" << endl;
#endif
	stop_track();

	mixermusic = Mix_LoadMUS(name);
	Mix_PlayMusic(mixermusic, repeat);
	Mix_VolumeMusic(MIX_MAX_VOLUME);	//Balance volume with other music types
}

void	Mixer_MidiOut::start_sfx(XMIDIEventList *event_list)
{
//Defunct?
}

void	Mixer_MidiOut::stop_sfx(void)
{
//Defunct?
}

const	char *Mixer_MidiOut::copyright(void)
{
	return "Internal SDL_mixer midi out";
}
