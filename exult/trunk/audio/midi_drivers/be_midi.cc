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

#include "pent_include.h"
#include "be_midi.h"

#ifdef USE_BEOS_MIDI

#include "fnames.h"

#include <string.h>
#include <stdio.h>

const MidiDriver::MidiDriverDesc Be_midi::desc = 
		MidiDriver::MidiDriverDesc ("Be_midi", createInstance);


int Be_midi::open()
{
  FileOpen = false;
  return 0;
}

void Be_midi::close()
{
  stop_track();
}

void Be_midi::stop_track(void)
{
  if (FileOpen) {
    midiSynthFile.UnloadFile();
    FileOpen = false;
  }
}

bool	Be_midi::is_playing(void)
{
  if (!FileOpen)
    return false;

  return !midiSynthFile.IsFinished();
}


void Be_midi::start_track(const char *name,bool repeat,int vol)
{
#if DEBUG
  cerr << "Stopping any running track" << endl;
#endif
  stop_track();

#if DEBUG
  cerr << "Starting midi sequence with Be_midi, repeat = " 
       << (repeat?"true":"false") << endl;
#endif

  //open file
  get_ref_for_path(name, &midiRef);
  midiSynthFile.LoadFile(&midiRef);
  FileOpen = true;

  //set repeating
  midiSynthFile.EnableLooping(repeat);

  //play file
  midiSynthFile.Start();
}

#endif // USE_BEOS_MIDI

