/*
 *  Copyright (C) 2000-2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _MIXER_H_
#define _MIXER_H_

#include "SDL_mapping.h"
#include "pcb.h"
#include "common_types.h"

class Audio;

/*
 *	(Mostly) unique identification for a sound that's playing:
 */
class AudioID
	{
	ProducerConsumerBuf *pcb;	// The PCB.
	uint32 seq;			// PCB sequence #.
public:
	friend class Mixer;
	AudioID() : pcb(0), seq(0)	// Empty, inactive.
		{  }
	AudioID(ProducerConsumerBuf *p, uint32 s) : pcb(p), seq(s)
		{  }
	void set_volume(int v);		// 0-128.
	void set_dir(int d);		// 0-15
	void set_repeat(bool rep);
	bool is_active()
		{ return pcb && pcb->get_seq() == seq && pcb->is_active(); }
	};

//---- Mixer -----------------------------------------------------------

static const int Mixer_Sample_Magic_Number=0x55443322;
#define MAX_AUDIO_STREAMS  6

class Mixer 
{
private:
	ProducerConsumerBuf *streams[MAX_AUDIO_STREAMS];
	Mixer(const Mixer &m);	// Cannot call me
	uint8*		temp_buffer;
	Audio *audio;
public:
	//Mixer();
	Mixer(Audio*, uint32, uint32, uint8);
	//Mixer(size_t ringsize,size_t bufferlength);
	~Mixer();

	size_t	buffer_length;
	uint8	silence;
	SDL_mutex	*stream_mutex;
	void	stream_lock(void) { SDL_mutexP(stream_mutex); };
	void	stream_unlock(void) { SDL_mutexV(stream_mutex); };
	void	modify_stereo16(sint16 *data, int cnt, int dir16);
	void fill_audio_func(void *, uint8 *, int);
	AudioID play(uint8 *, uint32, int volume = SDL_MIX_MAXVOLUME,
					int dir = 0, bool repeat = false);
	ProducerConsumerBuf *Create_Audio_Stream(uint32 type);
	void	Destroy_Audio_Stream(uint32 type);
	void	cancel_streams(void);
	bool	is_playing(uint32 type);
};


#endif
