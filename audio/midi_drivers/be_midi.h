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

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#ifdef BEOS

#include "Midi.h"
#include <be/midi/Midi.h>   //name clash; MidiSynthFile.h includes Exult's Midi.h 'accidently'
#include <be/midi/MidiSynthFile.h>

class	Be_midi : virtual public MidiAbstract
{
public:
  virtual void start_track(XMIDIEventList *,bool repeat);
//virtual void start_sfx(XMIDIEventList *);
  virtual void stop_track(void);
  virtual bool is_playing(void);
  virtual const char *copyright(void);

  Be_midi();
  virtual ~Be_midi();
private:
  BMidiSynthFile midiSynthFile;
  entry_ref midiRef;

  bool FileOpen;
};

#endif

#endif
