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

//
// Old Win32 MIDI driver. Changes required in Exult.cc, Midi.cc and Midi.h to use this
//

#ifndef _MIDI_driver_win_MCI_h_
#define _MIDI_driver_win_MCI_h_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#ifdef WIN32

#include "Midi.h"
#include "SDL_syswm.h"

class	Windows_MCI : virtual public MidiAbstract
{
public:
  virtual void start_track(const char *,bool repeat);
//virtual void start_track(midi_event *evntlist, int ppqn, bool repeat);
  virtual void stop_track(void);
  virtual bool is_playing(void);
  virtual const char *copyright(void);

  void callback(WPARAM wParam, HWND hWnd);
  
  Windows_MCI();
  virtual ~Windows_MCI();
private:
  bool device_open;
  bool repeating;
};

#endif

#endif
