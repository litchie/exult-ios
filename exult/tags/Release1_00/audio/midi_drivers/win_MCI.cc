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



//Windows-specific code
#ifdef WIN32

#include "win_MCI.h"
#include "SDL_syswm.h"

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include <string.h>
#include <stdio.h>

UINT MCI_Command(LPCTSTR lpszCommand, LPTSTR lpszReturnString, 
		 UINT cchReturn, HWND hWndCallBack)
{
  DWORD code = mciSendString(lpszCommand, lpszReturnString, cchReturn, hWndCallBack);
  if (code) {
    char buf[128];

    mciGetErrorString(code, buf, 127);
    cerr << "On MCI command: \"" << lpszCommand << "\"" << endl;
    cerr << "MCI error code " << code << ": ";
    fprintf(stderr, "%s\n", buf);
  }
  return code;
}
  

Windows_MCI::Windows_MCI()
{
  device_open = false;
}

void Windows_MCI::stop_track(void)
{
  if (device_open) {
    MCI_Command("close u7midi", 0, 0, 0);
    device_open = false;
    repeating = false;
  }
}

Windows_MCI::~Windows_MCI(void)
{
  stop_track();
}

bool	Windows_MCI::is_playing(void)
{
  char buf[512];

  if (!device_open)
    return false;

  MCI_Command("status u7midi mode", buf, 510, 0);

#if DEBUG
  cerr << "MIDI status command returned: " << buf << endl;
#endif

  if (strcmp(buf, "playing")==0)
    return true;
  
  return false;
}


void Windows_MCI::start_track(const char *name,bool repeat)
{
  char buf[512];
  HWND hWnd = 0;
  SDL_SysWMinfo info;		// Get system info.
  
  if (repeat) {
    SDL_VERSION(&info.version);
    SDL_GetWMInfo(&info);

    hWnd = info.window;
  }

#if DEBUG
  cerr << "Stopping any running track" << endl;
#endif
  stop_track();

#if DEBUG
  cerr << "Starting midi sequence with Windows_MCI, repeat = " 
       << (repeat?"true":"false") << endl;
#endif

  //open MCI device
  snprintf(buf, 512, "open %s type sequencer alias u7midi", name);
  MCI_Command(buf, NULL, 0, 0);
  device_open = true;

  repeating = repeat;

  //start playing
  MCI_Command("play u7midi notify", 0, 0, hWnd);
}

void Windows_MCI::callback(WPARAM wParam, HWND hWnd)
{

#if DEBUG
  cerr << "Entering MCI_callback: ";
  switch(wParam) {
  case MCI_NOTIFY_ABORTED:
    cerr << "MCI_NOTIFY_ABORTED" << endl;
    break;
  case MCI_NOTIFY_FAILURE:
    cerr << "MCI_NOTIFY_FAILURE" << endl;
    break;
  case MCI_NOTIFY_SUCCESSFUL:
    cerr << "MCI_NOTIFY_SUCCESFUL" << endl;
    break;
  case MCI_NOTIFY_SUPERSEDED:
    cerr << "MCI_NOTIFY_SUPERSEDED" << endl;
    break;
  default:
    cerr << "Unknown flag!" << endl;
  }
#endif
  
  if (wParam == MCI_NOTIFY_SUCCESSFUL) {
    if (repeating) {
      //music stopped playing, so start over

#if DEBUG
      cerr << "Starting repeated MIDI playback" << endl;
#endif
      
      MCI_Command("seek u7midi to start", 0, 0, 0);  //rewind
      MCI_Command("play u7midi notify", 0, 0, hWnd); //play
    } else {

    //non-continuous playback stopped, so close MIDI device
      if (device_open) {
	MCI_Command("close u7midi", 0, 0, 0);
	device_open = false;
	repeating = false;
      }
    }
  } else if (wParam == MCI_NOTIFY_FAILURE) {
    //error. Close midi device
    if (device_open) {
      MCI_Command("close u7midi", 0, 0, 0);
      device_open = false;
      repeating = false;
    }
  }    
}

const	char *Windows_MCI::copyright(void)
{
  return "Internal Windows MCI based MIDI player";
}


#endif
