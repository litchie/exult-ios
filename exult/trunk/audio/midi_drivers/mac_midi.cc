/*
Copyright (C) 2000  Max Horn

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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

//MacOS-specific code
#ifdef MACOS

#include "mac_midi.h"


Mac_QT_midi::Mac_QT_midi()
{
}

void Mac_QT_midi::stop_track(void)
{
}

Mac_QT_midi::~Mac_QT_midi(void)
{
  stop_track();
}

bool	Mac_QT_midi::is_playing(void)
{
}


void Mac_QT_midi::start_track(const char *name,bool repeat)
{
}

const	char *Mac_QT_midi::copyright(void)
{
  return "Internal QuickTime MIDI player";
}


#endif
