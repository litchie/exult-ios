//-*-Mode: C++;-*-
/*
Copyright (C) 2000  Ryan Nunn

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

#ifndef _MIDI_driver_win_midiout_h_
#define _MIDI_driver_win_midiout_h_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#ifdef WIN32

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include "Midi.h"
#include "xmidi.h"
#include "SDL_syswm.h"
#include "mmsystem.h"

class	Windows_MidiOut : virtual public MidiAbstract
{
public:
//	virtual void start_track(const char *,bool repeat) { }
	virtual void start_track(midi_event *evntlist, int ppqn, bool repeat);
	virtual void stop_track(void);
	virtual bool is_playing(void);
	virtual const char *copyright(void);

	Windows_MidiOut();
	virtual ~Windows_MidiOut();
};

#endif

#endif
