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
static bool		is_available;
	
static DWORD		thread_main(void *data);
static void		thread_play();
static HANDLE	 	*thread_handle;
static DWORD		thread_id;

// Thread communicatoins
static int		thread_com;
static int		sfx_com;

#define W32MO_THREAD_COM_READY		0
#define W32MO_THREAD_COM_PLAY		1
#define W32MO_THREAD_COM_STOP		2
#define W32MO_THREAD_COM_INIT		3
#define W32MO_THREAD_COM_INIT_FAILED	4
#define W32MO_THREAD_COM_EXIT		-1

#define W32MO_SFX_COM_READY		0
#define W32MO_SFX_COM_PLAY		1
#define W32MO_SFX_COM_PLAYING		2

static bool		in_use = 0;

struct mid_data {
	midi_event	*list;
	int 		ppqn;
	bool		repeat;
};

static mid_data *thread_data;
static mid_data *sfx_data;


Windows_MidiOut::Windows_MidiOut()
{
	if (in_use)
	{
		throw (0);
		return;
	}
	in_use = true;
	playing = false;
	is_available = false;
	init_device();
}

Windows_MidiOut::~Windows_MidiOut()
{
	in_use = false;
	if (!is_available) return;

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
	if (count == 100 && is_available)
		TerminateThread (thread_handle, 1);

	is_available = false;
}

static void init_device()
{
	// Opened, lets open the thread
	thread_com = W32MO_THREAD_COM_INIT;
	
	thread_handle = (HANDLE*) CreateThread (NULL, 0, thread_main, NULL, 0, &thread_id);
	
	while (thread_com == W32MO_THREAD_COM_INIT) SDL_Delay (1);
	
	if (thread_com == W32MO_THREAD_COM_INIT_FAILED) cout << "Failier to initialize midi playing thread" << endl;
}

static DWORD thread_main(void *data)
{
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
	is_available = true;
	
	SetThreadPriority (thread_handle, THREAD_PRIORITY_HIGHEST);
	
	thread_com = W32MO_THREAD_COM_READY;
	sfx_com = W32MO_SFX_COM_READY;

	thread_play();

	midiOutClose (midi_port);
	is_available = false;
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


// Inits the sfx microsecond clock
static Uint32 sfx_start;
inline void wmoInitSFXClock ()
{
	sfx_start = SDL_GetTicks();
}

// Gets time in microseconds since
inline double wmoGetSFXTime ()
{
	return (SDL_GetTicks() - sfx_start) * 1000.0;
}

static void thread_play ()
{
	int	ppqn = 1;
	int	repeat = false;
	int	tempo = 0x07A120;
	double	Ippqn = 1;
	double	tick = 1;
	double	last_tick = 0;
	double	last_time = 0;
	double	aim;
	double	diff;
	
	midi_event *evntlist = NULL;
	midi_event *event = NULL;


	int	s_ppqn = 1;
	int	s_tempo = 0x07A120;
	double	s_Ippqn = 1;
	double	s_tick = 1;
	double	s_last_tick = 0;
	double	s_last_time = 0;
	double	s_aim;
	double	s_diff;
	
	midi_event *s_evntlist = NULL;
	midi_event *s_event = NULL;

	bool	s_playing = false;

	// Play while there isn't a message waiting
	while (1)
	{
		if (thread_com == W32MO_THREAD_COM_EXIT && !playing && !s_playing) break;
		
	 	if (event)
	 	{
	 		aim = last_time + (event->time-last_tick)*tick;
			diff = aim - wmoGetTime ();
	 	}
	 	else 
	 		diff = 1;
	
		if (diff <= 0)
		{
			last_tick = event->time;
			last_time = aim;
		
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
	 		if (!event || (thread_com != W32MO_THREAD_COM_READY))
		 	{
		 		if (!repeat || (thread_com != W32MO_THREAD_COM_READY))
		 		{
		 						// Clean up
					midiOutReset (midi_port);
					XMIDI::DeleteEventList (evntlist);
					evntlist = NULL;
					event = NULL;
					playing = false;
					
					// If stop was requested, we are ready to receive another song
					if (thread_com == W32MO_THREAD_COM_STOP)
						thread_com = W32MO_THREAD_COM_READY;
		 		}
	 			else
	 			{
	 				event = evntlist;
					last_tick = 0;
					last_time = 0;
					wmoInitClock();
	 			}
		 	}
		}

		// Got issued a music play command
		// set up the music playing routine
		if (thread_com == W32MO_THREAD_COM_PLAY)
		{
			if (evntlist)
			{
				midiOutReset (midi_port);
				XMIDI::DeleteEventList (evntlist);
				evntlist = NULL;
				event = NULL;
				playing = false;
			}
			
			// Make sure that the data exists
			while (!thread_data) SDL_Delay(1);
			
			evntlist = thread_data->list;
			repeat = thread_data->repeat;

			ppqn = thread_data->ppqn;
			thread_data = NULL;
			thread_com = W32MO_THREAD_COM_READY;
			
			event = evntlist;
			tempo = 0x07A120;
			
			Ippqn = 1.0/ppqn;
			tick = tempo*Ippqn;
	
			last_tick = 0;
			last_time = 0;
			
			wmoInitClock ();

			playing = true;
		}



	 	if (s_event)
	 	{
	 		s_aim = s_last_time + (s_event->time-s_last_tick)*s_tick;
			s_diff = s_aim - wmoGetSFXTime ();
	 	}
	 	else 
	 		s_diff = 1;
	
		if (s_diff <= 0)
		{
			s_last_tick = s_event->time;
			s_last_time = s_aim;
		
			if (s_event->status < 0xF0)
			{
				midiOutShortMsg (midi_port, s_event->status + (s_event->data[0] << 8) + (s_event->data[1] << 16));
			}
			else if (s_event->status == 0xFF && s_event->data[0] == 0x51) // Tempo change
			{
				s_tempo = (s_event->buffer[0] << 16) +
					(s_event->buffer[1] << 8) +
					s_event->buffer[2];
					
				s_tick = s_tempo*s_Ippqn;
			}	
		
		 	s_event = s_event->next;
	 		if ((!s_event) || (thread_com == W32MO_THREAD_COM_EXIT))
		 	{
				XMIDI::DeleteEventList (s_evntlist);
				s_evntlist = NULL;
				s_event = NULL;
				s_playing = false;
				sfx_com = W32MO_SFX_COM_READY;
		 	}
		}


		// Got issued a sound effect play command
		// set up the sound effect playing routine
		if (sfx_com == W32MO_SFX_COM_PLAY)
		{
			cout << "Play sfx command" << endl;
			// Make sure that the data exists
			while (!sfx_data) SDL_Delay(1);
			
			s_evntlist = sfx_data->list;

			s_ppqn = sfx_data->ppqn;
			sfx_data = NULL;
			sfx_com = W32MO_SFX_COM_PLAYING;
			
			s_event = s_evntlist;
			s_tempo = 0x07A120;
			
			s_Ippqn = 1.0/s_ppqn;
			s_tick = s_tempo*s_Ippqn;
	
			s_last_tick = 0;
			s_last_time = 0;
			
			wmoInitSFXClock ();

			s_playing = true;
		}



	 	if (event)
	 	{
	 		aim = last_time + (event->time-last_tick)*tick;
			diff = aim - wmoGetTime ();
	 	}
	 	else 
	 		diff = 1;

	 	if (s_event)
	 	{
	 		s_aim = s_last_time + (s_event->time-s_last_tick)*s_tick;
			s_diff = s_aim - wmoGetSFXTime ();
	 	}
	 	else 
	 		s_diff = 1;


		if (diff > 0 && s_diff > 0) wmoDelay (1000);
	}
	midiOutReset (midi_port);
}

static mid_data data;

void Windows_MidiOut::start_track (midi_event *evntlist, const int ppqn, bool repeat)
{
	if (!is_available)
		init_device();

	if (!is_available)
		return;
		
	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	
	data.list = evntlist;
	data.ppqn = ppqn;
	data.repeat = repeat;
	
	thread_data = &data;
	
	thread_com = W32MO_THREAD_COM_PLAY;
}

static mid_data sdata;

void Windows_MidiOut::start_sfx(midi_event *evntlist, int ppqn)
{
	cout << "got sfx" << endl;

	if (!is_available)
		init_device();

	if (!is_available)
		return;
	
	// One sfx at a time at the moment, ok
	if (sfx_com == W32MO_SFX_COM_PLAYING) return;

	sdata.list = evntlist;
	sdata.ppqn = ppqn;
	
	sfx_data = &sdata;
	
	sfx_com = W32MO_SFX_COM_PLAY;
}


void Windows_MidiOut::stop_track(void)
{
	if (!is_available)
		return;

	if (!playing) return;

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
