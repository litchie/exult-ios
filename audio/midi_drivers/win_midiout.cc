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

#define W32MO_THREAD_COM_READY		0
#define W32MO_THREAD_COM_PLAY		1
#define W32MO_THREAD_COM_STOP		2
#define W32MO_THREAD_COM_INIT		3
#define W32MO_THREAD_COM_INIT_FAILED	4
#define W32MO_THREAD_COM_EXIT		-1

Windows_MidiOut::Windows_MidiOut()
{
	InterlockedExchange (&playing, false);
	InterlockedExchange (&s_playing, false);
	InterlockedExchange (&is_available, false);
	init_device();
}

Windows_MidiOut::~Windows_MidiOut()
{
	if (!is_available) return;

	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_EXIT);

	int count = 0;
	
	while (count < 100)
	{
		DWORD code;
		GetExitCodeThread (thread_handle, &code);
		
		// Wait 1 MS before trying again
		if (code == STILL_ACTIVE) SDL_Delay (10);
		else break;
		
		count++;
	}

	// We waited a second and it still didn't terminate
	if (count == 100 && is_available)
		TerminateThread (thread_handle, 1);

	InterlockedExchange (&is_available, false);
}

void Windows_MidiOut::init_device()
{
	// Opened, lets open the thread
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_INIT);
	
	thread_handle = (HANDLE*) CreateThread (NULL, 0, thread_start, this, 0, &thread_id);
	
	while (thread_com == W32MO_THREAD_COM_INIT) SDL_Delay (1);
	
	if (thread_com == W32MO_THREAD_COM_INIT_FAILED) cerr << "Failier to initialize midi playing thread" << endl;
}

DWORD Windows_MidiOut::thread_start(void *data)
{
	Windows_MidiOut *ptr=static_cast<Windows_MidiOut *>(data);
	return ptr->thread_main();
}

DWORD Windows_MidiOut::thread_main()
{
	thread_data = NULL;
	InterlockedExchange (&playing, false);
	InterlockedExchange (&s_playing, false);

	UINT mmsys_err = midiOutOpen (&midi_port, MIDI_MAPPER, 0, 0, 0);

	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];

		mciGetErrorString(mmsys_err, buf, 512);
		cerr << "Unable to open device: " << buf << endl;
		InterlockedExchange (&thread_com, W32MO_THREAD_COM_INIT_FAILED);
		return 1;
	}
	InterlockedExchange (&is_available, true);
	
	SetThreadPriority (thread_handle, THREAD_PRIORITY_HIGHEST);
	
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);

	thread_play();

	midiOutClose (midi_port);
	InterlockedExchange (&is_available, false);
	return 0;
}

void Windows_MidiOut::thread_play ()
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
					InterlockedExchange (&playing, false);
					
					// If stop was requested, we are ready to receive another song
					if (thread_com == W32MO_THREAD_COM_STOP)
						InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
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
				InterlockedExchange (&playing, false);
			}
			
			// Reset pitch wheel
			for (int i = 0; i < 16; i++)
				midiOutShortMsg (midi_port, i | 0xE0 | (0x2000 << 2));
			
			// Make sure that the data exists
			while (!thread_data) SDL_Delay(1);
			
			evntlist = thread_data->list;
			repeat = thread_data->repeat;

			ppqn = thread_data->ppqn;
			InterlockedExchange ((LONG*) &thread_data, (LONG) NULL);
			InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
			
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
	 		if ((!s_event) || (thread_com == W32MO_THREAD_COM_EXIT) || (sfx_com != W32MO_THREAD_COM_READY))
		 	{
				XMIDI::DeleteEventList (s_evntlist);
				s_evntlist = NULL;
				s_event = NULL;
				InterlockedExchange (&s_playing, false);
				InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);
		 	}
		}


		// Got issued a sound effect play command
		// set up the sound effect playing routine
		if (sfx_com == W32MO_THREAD_COM_PLAY)
		{
			cout << "Play sfx command" << endl;
			// Make sure that the data exists
			while (!sfx_data) SDL_Delay(1);
			
			s_evntlist = sfx_data->list;

			s_ppqn = sfx_data->ppqn;
			InterlockedExchange ((LONG*) &sfx_data, (LONG) NULL);
			InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);
			
			s_event = s_evntlist;
			s_tempo = 0x07A120;
			
			s_Ippqn = 1.0/s_ppqn;
			s_tick = s_tempo*s_Ippqn;
	
			s_last_tick = 0;
			s_last_time = 0;
			
			wmoInitSFXClock ();

			InterlockedExchange (&s_playing, true);
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
	
	InterlockedExchange ((LONG*) &thread_data, (LONG) &data);
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_PLAY);
}

void Windows_MidiOut::start_sfx(midi_event *evntlist, int ppqn)
{
	if (!is_available)
		init_device();

	if (!is_available)
		return;
	
	while (sfx_com != W32MO_THREAD_COM_READY) SDL_Delay (1);

	sdata.list = evntlist;
	sdata.ppqn = ppqn;
	
	InterlockedExchange ((LONG*) &sfx_data, (LONG) &sdata);
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_PLAY);
}


void Windows_MidiOut::stop_track(void)
{
	if (!is_available)
		return;

	if (!playing) return;

	while (thread_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_STOP);
}

void Windows_MidiOut::stop_sfx(void)
{
	if (!is_available)
		return;

	if (!s_playing) return;

	while (sfx_com != W32MO_THREAD_COM_READY) SDL_Delay (1);
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_STOP);
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
