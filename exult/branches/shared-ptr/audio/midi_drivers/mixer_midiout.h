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

#ifndef _MIDI_driver_Mixer_MidiOut_h_
#define _MIDI_driver_Mixer_MidiOut_h_

//#define USE_MIXER_MIDI
#ifdef USE_MIXER_MIDI

#include "FileMidiDriver.h"

class	Mixer_MidiOut : public FileMidiDriver
{
	const static MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new Mixer_MidiOut();
	}
public:
	const static MidiDriverDesc* getDesc() { return &desc; }

	// FileMidiDriver Implementation
	virtual int			open();
	virtual void		close();
	virtual void		start_track(const char *filename, bool repeat, int vol);
	virtual void		stop_track();
	virtual bool		is_playing();
	virtual void		set_volume(int vol);

	static void music_complete_callback(void);

	Mixer_MidiOut();
	virtual ~Mixer_MidiOut();
};
#endif //USE_MIXER_MIDI

#endif //_MIDI_driver_Mixer_MidiOut_h_
