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

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

//Windows-specific code
#ifdef WIN32

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include "xmidi.h"
#include <unistd.h>
#include "utils.h"
#include "SDL.h"
#include "win_midiout.h"

static HMIDIOUT		midi_port;
static bool		playing;
	
static void		init_device();
static bool		is_avaliable;
	
static DWORD		thread_main(void *data);
static void		thread_play(midi_event *list, const int ppqn, bool repeat);
static HANDLE	 	*thread_handle;
static DWORD		thread_id;

// Thread communicatoins
static int		thread_com;

#define W32MO_THREAD_COM_READY		0
#define W32MO_THREAD_COM_PLAY		1
#define W32MO_THREAD_COM_STOP		2
#define W32MO_THREAD_COM_INIT		3
#define W32MO_THREAD_COM_INIT_FAILED	4
#define W32MO_THREAD_COM_EXIT		-1

static bool		in_use = 0;

struct mid_data {
	midi_event	*list;
	int 		ppqn;
	bool		repeat;
};

static mid_data *thread_data;


Windows_MidiOut::Windows_MidiOut()
{
	if (in_use)
	{
		throw (0);
		return;
	}
	in_use = true;
	playing = false;
	is_avaliable = false;
	init_device();
}

Windows_MidiOut::~Windows_MidiOut()
{
	in_use = false;
	if (!is_avaliable) return;

	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	thread_com = W32MO_THREAD_COM_EXIT;

	int count = 0;
	
	while (count < 100)
	{
		DWORD code;
		GetExitCodeThread (thread_handle, &code);
		
		// Wait 1 MS before trying again
		if (code == STILL_ACTIVE) SDL_Delay (10);
		else break;
	}

	// We waited a second and it still didn't terminate
	if (count == 100 && is_avaliable)
		TerminateThread (thread_handle, 1);

	is_avaliable = false;
}

static void init_device()
{
	// Opened, lets open the thread
	thread_com = W32MO_THREAD_COM_INIT;
	
	thread_handle = (HANDLE*) CreateThread (NULL, 0, thread_main, NULL, 0, &thread_id);
	
	while (thread_com == W32MO_THREAD_COM_INIT) SDL_Delay (1);
	
	if (thread_com == W32MO_THREAD_COM_INIT_FAILED) return;
}

static DWORD thread_main(void *data)
{
	midi_event*	evntlist;
	playing = false;
	thread_data = NULL;


	UINT mmsys_err = midiOutOpen (&midi_port, MIDI_MAPPER, 0, 0, 0);

	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];

		mciGetErrorString(mmsys_err, buf, 512);
		cerr << "Unable to open device: " << buf << endl;
		thread_com = W32MO_THREAD_COM_INIT_FAILED;
		return 1;
	}
	is_avaliable = true;
	
	SetThreadPriority (thread_handle, THREAD_PRIORITY_HIGHEST);
	
	thread_com = W32MO_THREAD_COM_READY;

	// -1 = quit
	while (thread_com != W32MO_THREAD_COM_EXIT)
	{
		if (thread_com == W32MO_THREAD_COM_PLAY)
		{
			// Make sure that the data exists
			while (!thread_data) SDL_Delay(1);
			
			evntlist = thread_data->list;
			int repeat = thread_data->repeat;
			int ppqn = thread_data->ppqn;
			
			thread_data = NULL;
			thread_com = W32MO_THREAD_COM_READY;
			
			thread_play (evntlist, ppqn, repeat);

			XMIDI::DeleteEventList (evntlist);
		}
		else if (thread_com == W32MO_THREAD_COM_STOP)
			thread_com = W32MO_THREAD_COM_READY;

		SDL_Delay (1);
	}
	midiOutClose (midi_port);
	is_avaliable = false;
	return 0;
}


// Inits the microsecond clock
static Uint32 start;
inline void wmoInitClock ()
{
	start = SDL_GetTicks();
}

// Gets time in microseconds since
inline double wmoGetTime ()
{
	return (SDL_GetTicks() - start) * 1000.0;
}

// Delays for a certain amount of microseconds
inline void wmoDelay (const double mcs_delay)
{
	if (mcs_delay < 0) return;
	SDL_Delay ((int) (mcs_delay / 1000.0));
}

static void thread_play (midi_event *evntlist, const int ppqn, bool repeat)
{
	int	tempo = 0x07A120;
	double	Ippqn = 1.0/ppqn;
	double	tick = tempo*Ippqn;
	double	last_tick = 0;
	double	last_time = 0;
	double	aim;
	double	diff;
	
	wmoInitClock ();
		
	midi_event *event = evntlist;

	playing = true;
	
	// Play while there isn't a message waiting
	while (thread_com == W32MO_THREAD_COM_READY)
	{
		aim = last_time + (event->time-last_tick)*tick;
		
		while (((diff = aim - wmoGetTime ()) > 0) && thread_com == W32MO_THREAD_COM_READY) wmoDelay (1000);
		
		if (thread_com != W32MO_THREAD_COM_READY) break;
		
		last_tick = event->time;
		last_time = aim;
		
		// Time to go!!!!!!!!
		if (event->status < 0xF0)
		{
			midiOutShortMsg (midi_port, event->status + (event->data[0] << 8) + (event->data[1] << 16));
		}
		else if (event->status == 0xFF && event->data[0] == 0x51) // Tempo change
		{
			tempo = (event->buffer[0] << 16) +
				(event->buffer[1] << 8) +
				event->buffer[2];
				
			tick = tempo*Ippqn;
		}
		
	 	event = event->next;
	 	if (!event)
	 	{
	 		if (!repeat) break;
	 		else
	 		{
	 			event = evntlist;
				last_tick = 0;
				last_time = 0;
				wmoInitClock();
	 		}
	 	}
	 	
	}
	playing = false;
	midiOutReset (midi_port);
}

static mid_data data;

void Windows_MidiOut::start_track (midi_event *evntlist, const int ppqn, bool repeat)
{
	if (!is_avaliable)
		init_device();

	if (!is_avaliable)
		return;
		
	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	
	data.list = evntlist;
	data.ppqn = ppqn;
	data.repeat = repeat;
	
	thread_data = &data;
	thread_com = W32MO_THREAD_COM_PLAY;
}

void Windows_MidiOut::stop_track(void)
{
	if (!is_avaliable)
		return;

	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	thread_com = W32MO_THREAD_COM_STOP;
}

bool Windows_MidiOut::is_playing(void)
{
	return playing;
}

const char *Windows_MidiOut::copyright(void)
{
	return "Internal Win32 Midiout Midi Player for Exult.";
}


#endif
