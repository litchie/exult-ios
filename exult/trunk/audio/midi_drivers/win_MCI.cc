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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#ifdef WIN32

#include "win_MCI.h"

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include <string.h>
#include <stdio.h>

Windows_MCI::Windows_MCI()
{
  device_open = false;
}

void Windows_MCI::stop_track(void)
{
  int ret;
  if (device_open) {
    ret = mciSendString("close u7midi", 0, 0, 0);
    device_open = false;
  }
}

Windows_MCI::~Windows_MCI(void)
{
  stop_track();
}

bool	Windows_MCI::is_playing(void)
{
  int ret;
  char buf[512];

  if (!device_open)
    return false;

  ret = mciSendString("status u7midi mode", buf, 510, 0);

  if (strcmp(buf, "playing")==0)
    return true;
  
  return false;
}


void	Windows_MCI::start_track(const char *name,bool repeat)
{
  int ret;
  char buf[512];

  //#if DEBUG
  cerr << "Starting midi sequence with Windows_MCI" << endl;

  cerr << "Stopping any running track" << endl;
  //#endif
  stop_track();

// TODO: repeats not implemented yet

// TODO: use return value

  //open MCI device
  sprintf(buf, "open %s type sequencer alias u7midi", name);
  ret = mciSendString(buf, NULL, 0, 0);
  device_open = true;

  //start playing
  ret = mciSendString("play u7midi", 0, 0, 0);
}

const	char *Windows_MCI::copyright(void)
{
  return "Internal Windows MCI based MIDI player";
}


#endif
