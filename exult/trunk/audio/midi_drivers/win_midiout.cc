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
#include "sdl.h"
#include "win_midiout.h"

static HMIDIOUT		midi_port;
static bool		playing;
	
static void		init_device();
static bool		is_avaliable;
	
static midi_event	*thread_list;
static int 		thread_ppqn;
static bool		thread_repeat;
static int		thread_com;

static int		thread_main(char *data);
static void		thread_play(midi_event *list, const int ppqn, bool repeat);
static SDL_Thread 	*thread_id;

Windows_MidiOut::Windows_MidiOut()
{
	playing = false;
	is_avaliable = false;
	init_device();
}

Windows_MidiOut::~Windows_MidiOut()
{
	if (!is_avaliable) return;

	while (thread_com != 0) SDL_Delay (1);
	thread_com = -1;
	SDL_WaitThread (thread_id, NULL);
}

static void init_device()
{
	// Opened, lets open the thread
	thread_com = 4;
	char	*data = " ";
	thread_id = SDL_CreateThread (thread_main, data);
	
	while (thread_com == 4) SDL_Delay (1);
	
	if (thread_com == 3) return;
	
	is_avaliable = true;
}


static int thread_main(char *data)
{
	midi_event*	evntlist;
	playing = false;
	thread_list = NULL;


	UINT mmsys_err = midiOutOpen (&midi_port, (unsigned) -1, 0, 0, 0);

	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];

		mciGetErrorString(mmsys_err, buf, 512);
		cerr << "Unable to open device: " << buf << endl;
		thread_com = 3;
		return 1;
	}
	thread_com = 0;

	// -1 = quit
	while (thread_com != -1)
	{
		if (thread_com == 1)
		{
			SDL_Delay (1);
			evntlist = thread_list;
			thread_com = 0;
			thread_play (evntlist, thread_ppqn, thread_repeat);
			XMIDI::DeleteEventList (evntlist);
		}
		else if (thread_com == 2)
			thread_com = 0;

		SDL_Delay (1);
	}
	midiOutClose (midi_port);
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
	while (1)
	{
		aim = last_time + (event->time-last_tick)*tick;
		
		while ((diff = aim - wmoGetTime ()) > 0) wmoDelay (1000);
		
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
		
		//Somethings been issued
		if (thread_com != 0) break;
		
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

void Windows_MidiOut::start_track (midi_event *evntlist, const int ppqn, bool repeat)
{
	if (!is_avaliable)
		init_device();

	if (!is_avaliable)
		return;
		
	while (thread_com != 0) SDL_Delay (1);
	
	thread_list = evntlist;
	thread_ppqn = ppqn;
	thread_repeat = repeat;
	thread_com = 1;
}

void Windows_MidiOut::stop_track(void)
{
	if (!is_avaliable)
		return;

	while (thread_com != 0) SDL_Delay (1);
	thread_com = 2;
}

bool Windows_MidiOut::is_playing(void)
{
	return playing;
}

const char *Windows_MidiOut::copyright(void)
{
	return "Internal Midiout midi player";
}


#endif
