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

#ifndef FMOPL_MIDI_H
#define FMOPL_MIDI_H

#ifdef USE_FMOPL_MIDI

// 22050/OPL_NUM_SAMPLES_PER_PASS = (120*OPL_TICK_MULTIPLIER)/OPL_TIME_PER_PASS
#define OPL_TIME_PER_PASS			4
#define OPL_NUM_SAMPLES_PER_PASS	49
//#define OPL_TIME_PER_PASS			8
//#define OPL_NUM_SAMPLES_PER_PASS	98
//#define OPL_TIME_PER_PASS			32
//#define OPL_NUM_SAMPLES_PER_PASS	392
#define	OPL_TICK_MULTIPLIER			15

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#include "../Midi.h"
#include "exceptions.h"
#include "SDL_thread.h"
#include "SDL.h"

class OplDriver;
class RingBuffer16;

class	FMOpl_Midi : virtual public MidiAbstract
{
public:
	virtual void		start_track(XMIDIEventList *, bool repeat);
	virtual void		stop_track(void);
	virtual bool		is_playing(void);
	virtual const char	*copyright(void);
	virtual void		load_patches(bool force_xmidi);
	virtual bool		is_fm_synth();
	virtual bool		use_gs127();

	// PSMDEX - Pentagram Streaming Midi Driver Extensions
	virtual int			max_streams();
	virtual void		start_stream(int str_num, XMIDIEventList *, bool repeat, bool activate, int vol);
	virtual void		activate_stream(int str_num);
	virtual void		stop_stream(int str_num);
	virtual void		set_volume(int str_num, int level);
	virtual bool		is_playing(int str_num);
	virtual int			get_active();


	FMOpl_Midi();
	virtual ~FMOpl_Midi();

private:
	UNREPLICATABLE_CLASS(FMOpl_Midi);

	struct mid_data {
		XMIDIEventList	*list;
		bool			repeat;
	};

	static const unsigned short	centre_value;
	static const unsigned char	fine_value;
	static const unsigned char	coarse_value;
	static const unsigned short	combined_value;

	int						is_available;
	OplDriver				*opl;
	RingBuffer16			*buffer;

	// For Dual Opl2 Mode
	bool					dual;
	int						global_volume;
	uint8					volumes[16];
	uint8					balances[16];
	OplDriver				*opl_right;
	RingBuffer16			*buffer_right;
	
	// Communications
	int						&LockComs();
	void					UnlockComs();
	void					ClearComs();

	bool					playing;
	int						comMessage_priv;
	XMIDIEventList			*new_list;
	SDL_mutex				*mutex;
	SDL_mutex				*comMutex;

	// Methods
	static void				mixer_hook_static(void *udata, Uint8 *stream, int len);
	void					mixer_hook(Uint8 *stream, int len);

	void					init_device();
	void					deinit_device();
	void					reset_channel (int i);

	// Sample Clock
	unsigned long			total_sample_ticks;
	unsigned long			start;

	inline void wmoClockIncTime(unsigned long c)
	{ total_sample_ticks += c; }

	inline unsigned long wmoGetRealTime ()
	{ return total_sample_ticks*OPL_TIME_PER_PASS; }

	inline void wmoInitClock ()
	{ start = wmoGetRealTime(); }

	inline void wmoAddOffset (unsigned long offset)
	{ start += offset; }

	inline void wmoSubOffset (unsigned long offset)
	{ start -= offset; }

	inline unsigned long wmoGetTime ()
	{ return wmoGetRealTime() - start; }

	inline unsigned long wmoGetStart ()
	{ return start; }

	inline void				send(uint32 b);
	inline void				send_vol_or_balance(uint32 chan);
	void					PlayNotes();
	void					HandlePlay();
	void					HandleStop();

	uint32					GenerateSamples(uint32 count_required);
	void					GetSamples(uint32 samples);


	// This stuff is only actually used by the play thread. it should NOT be touched
	// by anything else
	int				repeat;
	uint32			aim;
	sint32			diff;
	uint32			last_tick;
	XMIDIEventList	*evntlist;
	midi_event		*event;
	NoteStack		notes_on;
	uint32			generate_rem;

	//
	// Xmidi Looping
	//

	// The for loop event
	midi_event	*loop_event[XMIDI_MAX_FOR_LOOP_COUNT];

	// The amount of times we have left that we can loop
	int		loop_count[XMIDI_MAX_FOR_LOOP_COUNT];

	// The level of the loop we are currently in
	int		loop_num;		


};

#endif //USE_FMOPL_MIDI

#endif //FMOPL_MIDI_H
