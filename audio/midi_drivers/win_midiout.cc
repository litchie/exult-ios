//-*-Mode: C++;-*-

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

//Windows-specific code
#ifdef WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include "win_midiout.h"
#include "exceptions.h"

#include "utils.h"
#include "xmidi.h"
#include <unistd.h>

#include "Configuration.h"
extern	Configuration	*config;

#define W32MO_THREAD_COM_READY		0
#define W32MO_THREAD_COM_PLAY		1
#define W32MO_THREAD_COM_STOP		2
#define W32MO_THREAD_COM_INIT		3
#define W32MO_THREAD_COM_INIT_FAILED	4
#define W32MO_THREAD_COM_EXIT		-1

const unsigned short Windows_MidiOut::centre_value = 0x2000;
const unsigned char Windows_MidiOut::fine_value = centre_value & 127;
const unsigned char Windows_MidiOut::coarse_value = centre_value >> 7;
const unsigned short Windows_MidiOut::combined_value = (coarse_value << 8) | fine_value;

//#define DO_SMP_TEST

#ifdef DO_SMP_TEST
#define giveinfo() std::cerr << __FILE__ << ":" << __LINE__ << std::endl; std::cerr.flush();
#else
#define giveinfo()
#endif

using std::string;
using std::cout;
using std::cerr;
using std::endl;

Windows_MidiOut::Windows_MidiOut()
{
	giveinfo();
	InterlockedExchange (&playing, false);
	InterlockedExchange (&s_playing, false);
	InterlockedExchange (&is_available, false);
	giveinfo();
	init_device();
	giveinfo();
}

Windows_MidiOut::~Windows_MidiOut()
{
	giveinfo();
	if (!is_available) return;

	giveinfo();
	while (thread_com != W32MO_THREAD_COM_READY) Sleep (1);
	
	giveinfo();
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_EXIT);

	giveinfo();
	int count = 0;
	
	giveinfo();
	while (count < 100)
	{
		giveinfo();
		DWORD code;
		GetExitCodeThread (thread_handle, &code);
		
		giveinfo();
		// Wait 1 MS before trying again
		if (code == STILL_ACTIVE) Sleep (10);
		else break;
		giveinfo();
		
		count++;
	}

	// We waited a second and it still didn't terminate
	giveinfo();
	if (count == 100 && is_available)
		TerminateThread (thread_handle, 1);

	giveinfo();
	InterlockedExchange (&is_available, false);
	giveinfo();
}

void Windows_MidiOut::init_device()
{
	string s;
		
	// Opened, lets open the thread
	giveinfo();
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_INIT);
	
	giveinfo();
	thread_handle = (HANDLE*) CreateThread (NULL, 0, thread_start, this, 0, &thread_id);
	
	giveinfo();
	while (thread_com == W32MO_THREAD_COM_INIT) Sleep (1);
	
	giveinfo();
	if (thread_com == W32MO_THREAD_COM_INIT_FAILED) cerr << "Failure to initialize midi playing thread" << endl;
	giveinfo();
}

DWORD __stdcall Windows_MidiOut::thread_start(void *data)
{
	giveinfo();
	Windows_MidiOut *ptr=static_cast<Windows_MidiOut *>(data);
	giveinfo();
	return ptr->thread_main();
}

DWORD Windows_MidiOut::thread_main()
{
	thread_data = NULL;
	giveinfo();
	InterlockedExchange (&playing, false);
	InterlockedExchange (&s_playing, false);

	giveinfo();
	UINT mmsys_err = midiOutOpen (&midi_port, MIDI_MAPPER, 0, 0, 0);

	giveinfo();
	if (mmsys_err != MMSYSERR_NOERROR)
	{
		char buf[512];

		giveinfo();
		mciGetErrorString(mmsys_err, buf, 512);
		cerr << "Unable to open device: " << buf << endl;
		giveinfo();
		InterlockedExchange (&thread_com, W32MO_THREAD_COM_INIT_FAILED);
		giveinfo();
		return 1;
	}
	giveinfo();
	InterlockedExchange (&is_available, true);
	
//	SetThreadPriority (thread_handle, THREAD_PRIORITY_HIGHEST);
	giveinfo();
	SetThreadPriority (thread_handle, THREAD_PRIORITY_TIME_CRITICAL);
	
	giveinfo();
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);

	giveinfo();
	thread_play();
	giveinfo();

	giveinfo();
	midiOutClose (midi_port);
	giveinfo();
	InterlockedExchange (&is_available, false);
	giveinfo();
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
	double	aim = 0;
	double	diff = 0;
	
	midi_event *evntlist = NULL;
	midi_event *event = NULL;

	giveinfo();

	int	s_ppqn = 1;
	int	s_tempo = 0x07A120;
	double	s_Ippqn = 1;
	double	s_tick = 1;
	double	s_last_tick = 0;
	double	s_last_time = 0;
	double	s_aim = 0;
	double	s_diff = 0;
	int	s_track = 0;
	
	midi_event *s_evntlist = NULL;
	midi_event *s_event = NULL;

	giveinfo();

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
		
			if ((event->status >> 4) != MIDI_STATUS_SYSEX)
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
		}

	 	if ((diff <= 0 && !event) || (thread_com != W32MO_THREAD_COM_READY))
		{
		 	if (!repeat || thread_com != W32MO_THREAD_COM_READY || last_tick == 0)
		 	{
		 		// Clean up
				giveinfo();
				for (int i = 0; i < 16; i++) reset_channel (i); 
				midiOutReset (midi_port);
				giveinfo();
				XMIDI::DeleteEventList (evntlist);
				giveinfo();
				evntlist = NULL;
				giveinfo();
				event = NULL;
				giveinfo();
				InterlockedExchange (&playing, false);
				giveinfo();
				
				// If stop was requested, we are ready to receive another song
				giveinfo();
				if (thread_com == W32MO_THREAD_COM_STOP)
				{
					giveinfo();
					InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
				}
				giveinfo();
		 	}
	 		else
	 		{
	 			event = evntlist;
				last_tick = 0;
				last_time = 0;
				wmoInitClock();
	 		}
		}

		// Got issued a music play command
		// set up the music playing routine
		if (!evntlist && thread_com == W32MO_THREAD_COM_PLAY)
		{
			// Manual Reset since I don't trust midiOutReset()
			giveinfo();
			for (int i = 0; i < 16; i++) reset_channel (i);
			midiOutReset (midi_port);
			
			// Make sure that the data exists
			giveinfo();
			while (!thread_data) Sleep(1);
			
			giveinfo();
			evntlist = thread_data->list;
			repeat = thread_data->repeat;

			giveinfo();
			ppqn = thread_data->ppqn;
			InterlockedExchange ((LONG*) &thread_data, (LONG) NULL);
			giveinfo();
			InterlockedExchange (&thread_com, W32MO_THREAD_COM_READY);
			
			giveinfo();
			event = evntlist;
			tempo = 0x07A120;
			
			Ippqn = 1.0/ppqn;
			tick = tempo*Ippqn;
	
			giveinfo();
			last_tick = 0;
			last_time = 0;
			
			giveinfo();
			wmoInitClock ();
	
			giveinfo();
			InterlockedExchange (&playing, true);
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
		
			if ((s_event->status >> 4) != MIDI_STATUS_SYSEX)
			{
				midiOutShortMsg (midi_port, s_event->status + (s_event->data[0] << 8) + (s_event->data[1] << 16));
				s_track |= 1 << (s_event->status & 0xF);
			}
			else if (s_event->status == 0xFF && s_event->data[0] == 0x51) // Tempo change
			{
				s_tempo = (s_event->buffer[0] << 16) +
					(s_event->buffer[1] << 8) +
					s_event->buffer[2];
					
				s_tick = s_tempo*s_Ippqn;
			}	
		
		 	s_event = s_event->next;
		}
	 	if (s_evntlist && (!s_event || thread_com == W32MO_THREAD_COM_EXIT || sfx_com != W32MO_THREAD_COM_READY))
		{
		 	// Play all the remaining note offs 
		 	while (s_event)
		 	{
				if ((s_event->status >> 4) == MIDI_STATUS_NOTE_OFF || 
					((s_event->status >> 4) == MIDI_STATUS_NOTE_OFF && s_event->data[1] == 0 ))
					
					midiOutShortMsg (midi_port, s_event->status + (s_event->data[0] << 8) + (s_event->data[1] << 16));
				s_event = s_event->next;
		 	}
		 	
		 	// Also reset the played tracks
			for (int i = 0; i < 16; i++) if ((s_track >> i)&1) reset_channel (i);

			XMIDI::DeleteEventList (s_evntlist);
			s_evntlist = NULL;
			s_event = NULL;
			InterlockedExchange (&s_playing, false);
			if (sfx_com != W32MO_THREAD_COM_PLAY) InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);
		}


		// Got issued a sound effect play command
		// set up the sound effect playing routine
		if (!s_evntlist && sfx_com == W32MO_THREAD_COM_PLAY)
		{
			giveinfo();
			cout << "Play sfx command" << endl;
			// Make sure that the data exists
			while (!sfx_data) Sleep(1);
			
			giveinfo();
			s_evntlist = sfx_data->list;

			giveinfo();
			s_ppqn = sfx_data->ppqn;
			InterlockedExchange ((LONG*) &sfx_data, (LONG) NULL);
			InterlockedExchange (&sfx_com, W32MO_THREAD_COM_READY);
			giveinfo();
			
			s_event = s_evntlist;
			s_tempo = 0x07A120;
			
			giveinfo();
			s_Ippqn = 1.0/s_ppqn;
			s_tick = s_tempo*s_Ippqn;
	
			s_last_tick = 0;
			s_last_time = 0;
			
			giveinfo();
			wmoInitSFXClock ();

			giveinfo();
			InterlockedExchange (&s_playing, true);
			
			giveinfo();
			// Reset thet track counter
			s_track = 0;
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
	if (evntlist) XMIDI::DeleteEventList (evntlist);
	if (s_evntlist) XMIDI::DeleteEventList (s_evntlist);
	for (int i = 0; i < 16; i++) reset_channel (i); 
	midiOutReset (midi_port);
}

void Windows_MidiOut::reset_channel (int i)
{
	// Pitch Wheel
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_PITCH_WHEEL << 4) | (combined_value << 8));
	
	// All controllers off
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (121 << 8));

	// All notes off
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (123 << 8));

	// Bank Select
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_PROG_CHANGE << 4) | (0 << 8));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (0 << 8));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (32 << 8));

	// Modulation Wheel
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (1 << 8) | (coarse_value << 16));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (33 << 8) | (fine_value << 16));
	
	// Volume
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (7 << 8) | (coarse_value << 16));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (39 << 8) | (fine_value << 16));

	// Pan
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (8 << 8) | (coarse_value << 16));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (40 << 8) | (fine_value << 16));

	// Balance
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (10 << 8) | (coarse_value << 16));
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (42 << 8) | (fine_value << 16));

	// Effects (Reverb)
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (91 << 8));

	// Chorus
	midiOutShortMsg (midi_port, i | (MIDI_STATUS_CONTROLLER << 4) | (93 << 8));
}

void Windows_MidiOut::start_track (midi_event *evntlist, const int ppqn, bool repeat)
{
	giveinfo();
	if (!is_available)
		init_device();

	giveinfo();
	if (!is_available)
		return;
		
	giveinfo();
	while (thread_com != W32MO_THREAD_COM_READY) Sleep (1);
	
	giveinfo();
	data.list = evntlist;
	data.ppqn = ppqn;
	data.repeat = repeat;
	
	giveinfo();
	InterlockedExchange ((LONG*) &thread_data, (LONG) &data);
	giveinfo();
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_PLAY);
	giveinfo();
}

void Windows_MidiOut::start_sfx(midi_event *evntlist, int ppqn)
{
	giveinfo();
	if (!is_available)
		init_device();

	giveinfo();
	if (!is_available)
		return;
	
	giveinfo();
	while (sfx_com != W32MO_THREAD_COM_READY) Sleep (1);

	giveinfo();
	sdata.list = evntlist;
	sdata.ppqn = ppqn;
	
	giveinfo();
	InterlockedExchange ((LONG*) &sfx_data, (LONG) &sdata);
	giveinfo();
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_PLAY);
	giveinfo();
}


void Windows_MidiOut::stop_track(void)
{
	giveinfo();
	if (!is_available)
		return;

	giveinfo();
	if (!playing) return;

	giveinfo();
	while (thread_com != W32MO_THREAD_COM_READY) Sleep (1);
	giveinfo();
	InterlockedExchange (&thread_com, W32MO_THREAD_COM_STOP);
	giveinfo();
}

void Windows_MidiOut::stop_sfx(void)
{
	giveinfo();
	if (!is_available)
		return;

	giveinfo();
	if (!s_playing) return;

	giveinfo();
	while (sfx_com != W32MO_THREAD_COM_READY) Sleep (1);
	giveinfo();
	InterlockedExchange (&sfx_com, W32MO_THREAD_COM_STOP);
	giveinfo();
}

bool Windows_MidiOut::is_playing(void)
{
	giveinfo();
	return playing!=0;
}

const char *Windows_MidiOut::copyright(void)
{
	giveinfo();
	return "Internal Win32 Midiout Midi Player for Exult.";
}


#endif
