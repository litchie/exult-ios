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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

//BEOS-specific code
#ifdef BEOS

#include "be_midi.h"

#include <string.h>
#include <stdio.h>

Be_midi::Be_midi()
{
  FileOpen = false;
}

void Be_midi::stop_track(void)
{
  if (FileOpen) {
    midiSynthFile.UnloadFile();
    FileOpen = false;
  }
}

Be_midi::~Be_midi(void)
{
  stop_track();
}

bool	Be_midi::is_playing(void)
{
  if (!FileOpen)
    return false;

  return !midiSynthFile.IsFinished();
}


void Be_midi::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

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

const	char *Be_midi::copyright(void)
{
  return "Internal BeOS MIDI player";
}


#endif
