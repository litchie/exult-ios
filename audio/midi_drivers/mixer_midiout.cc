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

#include "pent_include.h"
#include "mixer_midiout.h"

#ifdef USE_MIXER_MIDI

//#ifndef ALPHA_LINUX_CXX
//#  include <unistd.h>
//#  include <csignal>
//#  include <sys/types.h>
//#  include <sys/stat.h>
//#  include <fcntl.h>
//#endif

#include "SDL_mixer.h"

#include <iostream>
#include <cstdio>

#ifdef PENTAGRAM_IN_EXULT
#include "fnames.h"
#include "utils.h"
#endif

using std::cerr;
using std::endl;

#include "XMidiEventList.h"

static Mix_Music *mixermusic;

//
// Woohoo. This is easy with SDL_mixer! :)
//

const MidiDriver::MidiDriverDesc Mixer_MidiOut::desc = 
		MidiDriver::MidiDriverDesc ("Mixer", createInstance);


Mixer_MidiOut::Mixer_MidiOut() 
{
}

Mixer_MidiOut::~Mixer_MidiOut()
{
}

int Mixer_MidiOut::open()
{
	//Point to music finish cleanup code
	Mix_HookMusicFinished(music_complete_callback);
	return 0;
}

void Mixer_MidiOut::close()
{
	// Stop any current player
	stop_track();
	Mix_HookMusicFinished(NULL);
}

void Mixer_MidiOut::stop_track()
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

bool	Mixer_MidiOut::is_playing()
{
	return Mix_PlayingMusic()!=0;
}

void	Mixer_MidiOut::start_track(const char *filename, bool repeat, int vol)
{
#if DEBUG
	cerr << "Starting midi sequence with Mixer_MidiOut" << endl;
#endif
	mixermusic = Mix_LoadMUS(filename);
	Mix_PlayMusic(mixermusic, repeat);
	Mix_VolumeMusic((vol*(MIX_MAX_VOLUME-50))/255);	
}

void Mixer_MidiOut::set_volume(int vol)
{
	Mix_VolumeMusic((vol*(MIX_MAX_VOLUME-50))/255);	
}

#endif // USE_MIXER_MIDIOUT
