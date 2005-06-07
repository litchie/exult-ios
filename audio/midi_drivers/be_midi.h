//-*-Mode: C++;-*-
/*
Copyright (C) 2000  Willem Jan Palenstijn

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

#ifndef _MIDI_driver_be_midi_h_
#define _MIDI_driver_be_midi_h_

#ifdef BEOS

#define USE_BEOS_MIDI

#include "FileMidiDriver.h"
#include <be/midi/Midi.h>   //name clash; MidiSynthFile.h includes Exult's Midi.h 'accidently'
#include <be/midi/MidiSynthFile.h>

class	Be_midi : public FileMidiDriver
{
	const static MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new Be_midi();
	}
public:
	const static MidiDriverDesc* getDesc() { return &desc; }

  virtual void start_track(const char *name,bool repeat,int vol);
  virtual void stop_track(void);
  virtual bool is_playing(void);

  Be_midi();
  virtual ~Be_midi();
private:
  BMidiSynthFile midiSynthFile;
  entry_ref midiRef;

  bool FileOpen;
};

#endif

#endif
