/*
Copyright (C) 2000, 2001, 2002  Ryan Nunn

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

#ifdef USE_FMOPL_MIDI

#include <string>

#include "fmopl_midi.h"
#include "fmopldrv.h"
#include "xmidi.h"
#include "utils.h"
#include "Configuration.h"
#include "SDL_mixer.h"

extern	Configuration	*config;

#define FMOPL_MESSAGE_NO_MESSAGE		0
#define FMOPL_MESSAGE_PLAY				1
#define FMOPL_MESSAGE_PLAY_REPEAT		2
#define FMOPL_MESSAGE_STOP				3
#define FMOPL_MESSAGE_INIT				4

const unsigned short FMOpl_Midi::centre_value = 0x2000;
const unsigned char FMOpl_Midi::fine_value = centre_value & 127;
const unsigned char FMOpl_Midi::coarse_value = centre_value >> 7;
const unsigned short FMOpl_Midi::combined_value = (coarse_value << 8) | fine_value;

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

class RingBuffer16
{
	uint32		write_pos;
	uint32		read_pos;
	sint16		*buf;
	uint32		size;
	SDL_mutex	*mutex;

public:
	void LockWrite(uint32 total, sint16 *&buf1, uint32 &size1, sint16 *&buf2, uint32 &size2);
	void LockRead(uint32 total, sint16 *&buf1, uint32 &size1, sint16 *&buf2, uint32 &size2);
	void Unlock();

	RingBuffer16(uint32 s);
	~RingBuffer16();
};

RingBuffer16::RingBuffer16(uint32 s) : write_pos(0), read_pos(0), size(s)
{
	buf = new sint16[size];
	std::memset(buf, 0, sizeof(sint16)*size);
	mutex = SDL_CreateMutex();
}

RingBuffer16::~RingBuffer16()
{
	SDL_DestroyMutex(mutex);
	delete [] buf;
}

void RingBuffer16::Unlock()
{
	// Unlock it
	SDL_mutexV(mutex);
}

void RingBuffer16::LockRead(uint32 total, sint16 *&buf1, uint32 &size1, sint16 *&buf2, uint32 &size2)
{
	// Lock it
	SDL_mutexP(mutex);

	// Too many samples
	if (size < total) {
		size1 = 0;
		buf1 = 0;
		size2 = 0;
		buf2 = 0;
		return;
	}

	// It will run over the end of the ring buffer. Need to return end and start
	if (read_pos + total >= size) {

		// Buffer 1
		buf1 = buf + read_pos;
		size1 = size - read_pos;

		// Increment read_pos
		read_pos += total;
		read_pos -= size;

		// Buffer 2
		buf2 = buf;
		size2 = read_pos;

		return;
	}

	// Handle it normally
	buf1 = buf + read_pos;
	size1 = total;
	buf2 = 0;
	size2 = 0;
	read_pos += total;

}

void RingBuffer16::LockWrite(uint32 total, sint16 *&buf1, uint32 &size1, sint16 *&buf2, uint32 &size2)
{
	// Lock it
	SDL_mutexP(mutex);

	// Too many samples
	if (size < total) {
		size1 = 0;
		buf1 = 0;
		size2 = 0;
		buf2 = 0;
		return;
	}

	// It will run over the end of the ring buffer. Need to return end and start
	if (write_pos + total >= size) {

		// Buffer 1
		buf1 = buf + write_pos;
		size1 = size - write_pos;

		// Increment read_pos
		write_pos += total;
		write_pos -= size;

		// Buffer 2
		buf2 = buf;
		size2 = write_pos;

		return;
	}

	// Handle it normally
	buf1 = buf + write_pos;
	size1 = total;
	buf2 = 0;
	size2 = 0;
	write_pos += total;
}

FMOpl_Midi::FMOpl_Midi()
{
	giveinfo();
	playing = false;
	is_available = false;
	giveinfo();
	init_device();
	giveinfo();
}

FMOpl_Midi::~FMOpl_Midi()
{
	giveinfo();
	if (!is_available) return;

	deinit_device();
}

void FMOpl_Midi::deinit_device()
{
	// Un Hook SDL
	is_available = false;
	Mix_HookMusic (NULL, NULL);
#ifdef PENTAGRAM
	Mix_CloseAudio();
#endif

	// Delay a fraction
	SDL_Delay(10);

	// Lock it
	SDL_mutexP(mutex);

	// Free everything
	if (evntlist) evntlist->DecerementCounter();
	evntlist = NULL;

	// Free new list if it exists
	if (new_list) new_list->DecerementCounter();
	new_list = NULL;
	comMessage_priv = FMOPL_MESSAGE_NO_MESSAGE;

	// Reset the reasonable defaults
	repeat = false;
	aim = 0;
	diff = 0;
	last_tick = 0;
	evntlist = NULL;
	event = NULL;
	notes_on.clear();
	loop_num = -1;		
	std::memset(volumes, 64, sizeof(volumes));
	std::memset(balances, 64, sizeof(balances));
	generate_rem = 0;
	total_sample_ticks = 0;

	// Left
	delete opl;
	opl = 0;
	delete buffer;
	buffer = 0;

	// Right
	delete opl_right;
	opl_right = 0;
	delete buffer_right;
	buffer_right = 0;

	// Unlock it
	SDL_mutexV(mutex);

	// Kill it
	SDL_DestroyMutex(mutex);
	SDL_DestroyMutex(comMutex);
	mutex = NULL;
	comMutex = NULL;
}

void FMOpl_Midi::init_device()
{
	string s;
		
	// Dual Opl mode
	config->value("config/audio/midi/dual_opl",s,"yes");
	if (s == "yes") dual = true;
	else {
		s = "no";
		dual = false;
	}

	std::cout << "Dual OPL mode: " << s << endl;

	// Make sure it's in the config
	config->set("config/audio/midi/dual_opl",s,true);

	// Now setup all our settings
	playing = false;
	generate_rem = 0;
	is_available = true;
	comMessage_priv = FMOPL_MESSAGE_NO_MESSAGE;
	new_list = NULL;

	// Create Left
	buffer = new RingBuffer16(44096);
	opl = new OplDriver;
	opl->open(22050);

	// Create right, if required
	if (dual) {
		buffer_right = new RingBuffer16(44096);
		opl_right = new OplDriver;
		opl_right->open(22050);
	}
	else {
		opl_right = 0;
		buffer_right = 0;
	}

	// Setup some reasonable defaults
	repeat = false;
	aim = 0;
	diff = 0;
	last_tick = 0;
	evntlist = NULL;
	event = NULL;
	notes_on.clear();
	loop_num = -1;		
	std::memset(volumes, 64, sizeof(volumes));
	std::memset(balances, 64, sizeof(balances));

	generate_rem = 0;
	total_sample_ticks = 0;
	global_volume = 255;

	// Create the Mutexes
	mutex = SDL_CreateMutex();
	comMutex = SDL_CreateMutex();

	// Hook the process
#ifdef PENTAGRAM
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 4096);
#endif
	Mix_HookMusic (mixer_hook_static, (void*) this);
}

inline void FMOpl_Midi::send_vol_or_balance(uint32 chan) {

	// Single OPL mode only
	if (!dual) return;

	int left = volumes[chan];
	int right = volumes[chan];

	// is left
	if (balances[chan] < 64) {
		right *= balances[chan];
		right >>= 6;	// right /= 64;
	}
	// is right
	else if (balances[chan] > 64) {
		left *= 127 - balances[chan];
		left >>= 6;		// left /= 64;
	}

	opl->send(chan | (MIDI_STATUS_CONTROLLER << 4)|(7 << 8) | (left<<16));
	opl_right->send(chan | (MIDI_STATUS_CONTROLLER << 4)|(7 << 8) | (right<<16));
}

inline void FMOpl_Midi::send(uint32 b) {

	// Volume
	if (dual && (b&0xFFF0) == ((MIDI_STATUS_CONTROLLER << 4)|(7 << 8)))
	{
		int chan = b&0xF;
		volumes[chan] = (b >> 16) & 0x7F;
		send_vol_or_balance(chan);
	}
	// Balance
	else if (dual && (b&0xFFF0) == ((MIDI_STATUS_CONTROLLER << 4)|(10 << 8)))
	{
		int chan = b&0xF;
		balances[chan] = (b >> 16) & 0x7F;
		send_vol_or_balance(chan);
	}
	else {
		opl->send(b);
		if (opl_right) opl_right->send(b);
	}
}

uint32 FMOpl_Midi::GenerateSamples(uint32 count_required) {

	uint32 amount_generated = 0;

	while (amount_generated < count_required) {

		HandleStop();
		PlayNotes();
		HandlePlay();

		GetSamples(OPL_NUM_SAMPLES_PER_PASS);
		amount_generated += OPL_NUM_SAMPLES_PER_PASS;
		wmoClockIncTime(1);
	}

	return amount_generated - count_required;
}

void FMOpl_Midi::PlayNotes()
{
	// Handle note off's here
	while (midi_event *note = notes_on.PopTime(wmoGetRealTime()))
		send(note->status | (note->data[0] << 8));

	while (event)
	{
	 	aim = (event->time-last_tick)*OPL_TICK_MULTIPLIER;

		//printf ("aim %i time %i\n", aim, wmoGetTime ());

		diff = aim - wmoGetTime ();

		if (diff > 0) break;

		last_tick = event->time;
		wmoAddOffset(aim);
	
			// XMIDI For Loop
		if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_FOR_LOOP)
		{
			if (loop_num < XMIDI_MAX_FOR_LOOP_COUNT) loop_num++;

			loop_count[loop_num] = event->data[1];
			loop_event[loop_num] = event;

		}	// XMIDI Next/Break
		else if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_NEXT_BREAK)
		{
			if (loop_num != -1)
			{
				if (event->data[1] < 64)
				{
					loop_num--;
				}
			}
			event = NULL;

		}	// XMIDI Callback Trigger
		else if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_CALLBACK_TRIG)
		{
			// TODO
		}	// Not SysEx
		else if (event->status < 0xF0)
		{
			int type = event->status >> 4;

			if ((type != MIDI_STATUS_NOTE_ON || event->data[1]) && type != MIDI_STATUS_NOTE_OFF) {
				if (type == MIDI_STATUS_NOTE_ON) {
					notes_on.Remove(event);
					notes_on.Push (event, event->duration * OPL_TICK_MULTIPLIER + wmoGetStart());
				}

				send(event->status | (event->data[0] << 8) | (event->data[1] << 16));
			}
		}
	
		if (event) event = event->next;

		// Lock com system
		int &comMessage = LockComs();

	 	if (!event || comMessage != FMOPL_MESSAGE_NO_MESSAGE)
		{
			bool clean = !repeat || (comMessage != FMOPL_MESSAGE_NO_MESSAGE) || last_tick == 0;

		 	if (clean)
		 	{
				playing = false;
				if (comMessage == FMOPL_MESSAGE_STOP)
					comMessage = FMOPL_MESSAGE_NO_MESSAGE;

				// Handle note off's here
				while (midi_event *note = notes_on.Pop())
					send(note->status | (note->data[0] << 8));

		 		// Clean up
				for (int i = 0; i < 16; i++) reset_channel (i); 
				//midiOutReset (midi_port);
				if (evntlist) evntlist->DecerementCounter();
				evntlist = NULL;
				event = NULL;

				loop_num = -1;
				wmoInitClock ();
		 	}

			last_tick = 0;

			if (evntlist)
			{
	 			if (loop_num == -1) event = evntlist->events;
				else
				{
					event = loop_event[loop_num]->next;
					last_tick = loop_event[loop_num]->time;

					if (loop_count[loop_num])
						if (!--loop_count[loop_num])
							loop_num--;
				}
			}
		}

		// UnLock com system
		UnlockComs();
	}

}

void FMOpl_Midi::HandlePlay()
{
	// Lock com system
	int &comMessage = LockComs();

	// Got issued a music play command
	// set up the music playing routine
	if (comMessage == FMOPL_MESSAGE_PLAY || comMessage == FMOPL_MESSAGE_PLAY_REPEAT)
	{
		// Handle note off's here
		while (midi_event *note = notes_on.Pop())
			send(note->status | (note->data[0] << 8));

		// Manual Reset since I don't trust midiOutReset()
		giveinfo();
		for (int i = 0; i < 16; i++) reset_channel (i);

		if (evntlist) evntlist->DecerementCounter();
		evntlist = NULL;
		event = NULL;
		playing = false;

	
		giveinfo();
		evntlist = new_list;
		repeat = (comMessage == FMOPL_MESSAGE_PLAY_REPEAT);

		giveinfo();
		new_list = NULL;
		giveinfo();
		comMessage = FMOPL_MESSAGE_NO_MESSAGE;
		
		giveinfo();
		if (evntlist) event = evntlist->events;
		else event = 0;

		giveinfo();
		last_tick = 0;
		
		giveinfo();
		wmoInitClock ();

		// Reset XMIDI Looping
		loop_num = -1;

		giveinfo();
		playing = true;
	}

	// Unlock com system
	UnlockComs();
}

void FMOpl_Midi::HandleStop()
{
	// Lock com system
	int &comMessage = LockComs();

	if (comMessage == FMOPL_MESSAGE_STOP)
	{
		giveinfo();
		playing = false;
		comMessage = FMOPL_MESSAGE_NO_MESSAGE;

		// Handle note off's here
		while (midi_event *note = notes_on.Pop())
			send(note->status | (note->data[0] << 8));

		giveinfo();
		// Clean up
		for (int i = 0; i < 16; i++) reset_channel (i); 
		//midiOutReset (midi_port);
		giveinfo();
		if (evntlist) evntlist->DecerementCounter();
		giveinfo();
		evntlist = NULL;
		event = NULL;
		giveinfo();

		// If stop was requested, we are ready to receive another song

		loop_num = -1;

		wmoInitClock ();
		last_tick = 0;
	}

	// Unlock com system
	UnlockComs();
}

void FMOpl_Midi::reset_channel (int i)
{
	// Bank Select (General Midi)
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (0 << 8) | (127 << 16));

	// Modulation Wheel
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (1 << 8) | (coarse_value << 16));
	
	// Volume 
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (7 << 8) | (coarse_value << 16));

	// Balance
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (10 << 8) | (coarse_value << 16));

	// Expression
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (11 << 8) | (127 << 16));

	// XMIDI Bank Select
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (XMIDI_CONTROLLER_BANK_CHANGE << 8) | (0 << 16));

	// All notes off
	send(i | (MIDI_STATUS_CONTROLLER << 4) | (123 << 8));

	// Instrument
	send(i | (MIDI_STATUS_PROG_CHANGE << 4) | (0 << 8));

	// Pitch Wheel
	send(i | (MIDI_STATUS_PITCH_WHEEL << 4) | (combined_value << 8));
	
}

// Lock the coms
int	&FMOpl_Midi::LockComs()
{
	SDL_mutexP(comMutex);
	return comMessage_priv;
}

// Unlock the coms
void FMOpl_Midi::UnlockComs()
{
	SDL_mutexV(comMutex);
}

// Clead coms
void FMOpl_Midi::ClearComs()
{
	comMessage_priv = FMOPL_MESSAGE_NO_MESSAGE;

	// Free everything
	if (new_list) new_list->DecerementCounter();
	new_list = NULL;
}

void FMOpl_Midi::start_track (XMIDIEventList *xmidi, bool repeat)
{
	giveinfo();
	if (!is_available)
		init_device();

	giveinfo();
	if (!is_available)
		return;

	giveinfo();

	// Lock and clear the com system
	int &comMessage = LockComs();
	ClearComs();

	// Increment the list counter
	giveinfo();
	xmidi->IncerementCounter();
	new_list = xmidi;
	
	// Set the com message
	if (repeat) comMessage = FMOPL_MESSAGE_PLAY_REPEAT;
	else comMessage = FMOPL_MESSAGE_PLAY;

	// Unlock the coms
	UnlockComs();
}

void FMOpl_Midi::stop_track(void)
{
	giveinfo();
	if (!is_available) return;

	giveinfo();

	// Lock and clear the com system
	int &comMessage = LockComs();
	ClearComs();

	// If we are playing, we need to issue a stop com
	if (playing) comMessage = FMOPL_MESSAGE_STOP;

	// Unlock the coms
	UnlockComs();
}

bool FMOpl_Midi::is_playing(void)
{
	giveinfo();

	int &comMessage = LockComs();

	// If a stop command has been issued, we'll return and say that we have actually stopped
	// even though, there may still be some output.
	bool ret = playing && comMessage != FMOPL_MESSAGE_STOP;

	UnlockComs();

	return ret;
}

const char *FMOpl_Midi::copyright(void)
{
	giveinfo();
	return "Internal Emulated FM Opl Midi Synth.";
}

void FMOpl_Midi::mixer_hook_static(void *udata, Uint8 *stream, int len)
{
	((FMOpl_Midi *)udata)->mixer_hook(stream, len);
}

void FMOpl_Midi::mixer_hook(Uint8 *stream, int len)
{
	// Lock the mutex
	SDL_mutexP(mutex);

	// Looks like we are shutting down
	if (!is_available) return;

	// Generate the music
	generate_rem = GenerateSamples(len/4-generate_rem);

	// Copy the buffers into SDL_Mixer's buffer
	sint16* stream16 = (sint16*) stream;

	sint16* b1 = 0;
	sint16* b2 = 0;
	uint32	size1 = 0;
	uint32	size2 = 0;
	int i = 0;

	// Lock it (Left or Mono)
	buffer->LockRead(len/4, b1, size1, b2, size2);

	sint16* b1_r = b1;
	sint16* b2_r = b2;
	uint32	size1_r = size1;
	uint32	size2_r = size2;

	// Right, if in dual mode
	if (dual) buffer_right->LockRead(len/4, b1_r, size1_r, b2_r, size2_r);

	// Copy the samples
	if (global_volume >= 255) {
		for (i = 0; i < size1; ++i) {
			stream16[i<<1] = b1[i];
			stream16[(i<<1)+1] = b1_r[i];
		}

		for (i = 0; i < size2; ++i) {
			stream16[(i+size1)<<1] = b2[i];
			stream16[((i+size1)<<1)+1] = b2_r[i];
		}
	}
	else if (global_volume && dual) {
		for (i = 0; i < size1; ++i) {
			stream16[i<<1] = (b1[i]*global_volume)/255;
			stream16[(i<<1)+1] = (b1_r[i]*global_volume)/255;
		}

		for (i = 0; i < size2; ++i) {
			stream16[(i+size1)<<1] = (b2[i]*global_volume)/255;
			stream16[((i+size1)<<1)+1] = (b2_r[i]*global_volume)/255;
		}
	}
	else if (global_volume) {
		for (i = 0; i < size1; ++i) {
			stream16[(i<<1)+1] = stream16[i<<1] = (b1[i]*global_volume)/255;
		}

		for (i = 0; i < size2; ++i) {
			stream16[((i+size1)<<1)+1] = stream16[(i+size1)<<1] = (b2[i]*global_volume)/255;
		}
	}

	// Unlock right first
	if (dual) buffer_right->Unlock();

	// Unlock it
	buffer->Unlock();

	// Unlock the mutex
	SDL_mutexV(mutex);

}

void FMOpl_Midi::GetSamples(uint32 samples)
{
	// Got samples
	if (samples) {

		// Do the lock and generate the samples
		sint16* b1 = 0;
		sint16* b2 = 0;
		uint32	size1 = 0;
		uint32	size2 = 0;

		// Lock left
		buffer->LockWrite(samples, b1, size1, b2, size2);

		//	printf ("Generating %i samples\n", samples);

		if (size1 && b1) opl->generate_samples(b1, size1);
		if (size2 && b2) opl->generate_samples(b2, size2);

		if (dual) {
			// Lock it (right)
			buffer_right->LockWrite(samples, b1, size1, b2, size2);

			if (size1 && b1) opl_right->generate_samples(b1, size1);
			if (size2 && b2) opl_right->generate_samples(b2, size2);

			// Unlock (right)
			buffer_right->Unlock();
		}

		// Unlock left
		buffer->Unlock();

	}
}

void FMOpl_Midi::load_patches(bool force_xmidi)
{
	// Lock the mutex
	SDL_mutexP(mutex);

	if (opl) opl->LoadMT32Bank(force_xmidi);
	if (opl_right) opl_right->LoadMT32Bank(force_xmidi);

	// Unlock the mutex
	SDL_mutexV(mutex);
}

bool FMOpl_Midi::is_fm_synth()
{
	return true;
}

bool FMOpl_Midi::use_gs127() 
{ 
	return true; 
}

//
// PSMDEX - Pentagram Streaming Midi Driver Extensions
//

int FMOpl_Midi::max_streams()
{
	return 1;
}

void FMOpl_Midi::start_stream(int str_num, XMIDIEventList *eventlist, bool repeat, bool activate, int vol)
{
	stop_track();
	set_volume(0, vol);
	start_track(eventlist, repeat);
}

void FMOpl_Midi::activate_stream(int str_num)
{

}

void FMOpl_Midi::stop_stream(int str_num)
{
	stop_track();
}

void FMOpl_Midi::set_volume(int str_num, int level)
{
	if (!is_available) return;

	// Lock the mutex
	SDL_mutexP(mutex);

	// Set the volume
	global_volume = level;

	// Unlock the mutex
	SDL_mutexV(mutex);
}

bool FMOpl_Midi::is_playing(int str_num)
{
	return is_playing();
}

int FMOpl_Midi::get_active()
{ 
	return 0;
}

/*

what we need to do

  play notes and generate 5512 samples

  delay till we are past the amount of time it should have taken to generate those samples

  repeat

what we will do for now

  play notes and generate 5513 samples (250ms)

  delay till we are 250ms ahead

  play notes and generate 5512 samples (250ms)

  delay till we are 250ms ahead

  repeat

at 22050 KHz there are 735 samples for every 4 ticks.
at 22050 KHz there are 147 samples for every 4/5 ticks
at 22050 KHz there are  98 samples for every 8/15 ticks
at 22050 KHz there are  49 samples for every 4/15 ticks

  The wmo clock 'should' be based on the 98 samples number. For every 98 samples,
  we increment the clock 15 units. Working out the midi tick is unit/8. The unit
  from the midi tick is tick*8

  I could base it on the 49 samples number too. For every 49 samples we increment
  the clock 15 units aslso. The midi tick being then is unit/4. The unit from the
  midi tick is tick*4


The generate samples function:

  uint32 generate_sample (uint32 count_required) {

	uint32 amount_generated = 0;

	while (amount_generated < count_required) {

		while_event_loop();

		get_opls_to_generate_samples(NUM_SAMPLES_PER_PASS);

		amount_generated += NUM_SAMPLES_PER_PASS;
		wmoClockIncTime(TIME_PER_PASS);
	}

	return amount_generated;
  }

*/
#endif //USE_FMOPL_MIDI
