/*
Copyright (C) 2000  Dancer A.L Vesperman

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

#ifndef PENTAGRAM // Exult only at this stage. 

#include "Mixer.h"

#include <SDL_audio.h>
#include <SDL_error.h>

#include <unistd.h>

#define	TRAILING_VOC_SLOP 32

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#  include <iostream>
#  include <cstdlib>
#  include <cstring>
#  include <csignal>
#endif

#include <SDL_audio.h>
#include <SDL_timer.h>
#if defined(MACOS)
  #include <stat.h>
#else
  #include <sys/stat.h>
  #include <sys/types.h>
#endif
#include <fcntl.h>

#include "Audio.h"

using std::cerr;
using std::endl;
using std::memcpy;
using std::memset;
using std::vector;


//---- AudioID ---------------------------------------------------------

/*
 *	Set volume, direction, repeat.
 */
void AudioID::set_volume
	(
	int v				// 0-128.
	)
	{
	if (pcb && pcb->get_seq() == seq)
		pcb->set_volume(v);
	}
void AudioID::set_dir
	(
	int d				// 0-15 from North, clockwise.
	)
	{
	if (pcb && pcb->get_seq() == seq)
		pcb->set_dir(d);
	}
void AudioID::set_repeat
	(
	bool rep
	)
	{
	if (pcb && pcb->get_seq() == seq)
		pcb->set_repeat(rep);
	}


//---- Mixer ---------------------------------------------------------

Mixer::Mixer(Audio *a, uint32 __buffer_size,uint32 channels, 
	uint8 silence_value) : audio(a), stream_mutex(SDL_CreateMutex())
{
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		streams[i] = new ProducerConsumerBuf();
	buffer_length=__buffer_size;
	silence=silence_value;
	
	temp_buffer=new uint8[buffer_length];
}


Mixer::~Mixer()
{
	delete [] temp_buffer;
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		delete streams[i];
	SDL_DestroyMutex(stream_mutex);
}


/*
 *	Modify sound data for the sound source's position relative to the
 *	observer.
 */
				// These are factors * 1/128 for
				//   left, right channels:
static int factors[32] = {
				// North - East.
	128,128,    110,128,    90,128,    70,128,
				// East - South.
	50,128,     70,128,     90,128,    110,128,
				// South - West.
	128,128,    128,110,    128,90,    128,70,
				// West - North.
	128,50,     128,70,     128,90,    128,110
};

void Mixer::modify_stereo16
	(
	sint16 *data,			// 2-channels, 16-bit.
	int cnt,			// # samples.
	int dir16			// 0-15, clockwise from North.
	)
{
	int lfact = factors[(dir16%16)*2];
	int rfact = factors[(dir16%16)*2 + 1];
#if 0 && defined(DEBUG) && !defined(MACOS)
	cout << "Mixer::modify_stereo16:  lfact = " << lfact <<
		", rfact = " << rfact << endl;
#endif


	for (int i = 0; i < cnt; i++)
	{
		*data = (*data*lfact)/128;
		data++;
		*data = (*data*rfact)/128;
		data++;
	}
}		

/*
 *	Fill SDL_audio's request for sound.
 */
void Mixer::fill_audio_func(void *udata,uint8 *stream,int len)
{
#ifdef MACOS
	// WARNING WARNING WARNING
	// On MacOS, this function is called at *interrupt* time! That means several
	// functions are forbiden, including the following:
	// * new, delete, malloc, free etc.
	// * I/O like using cerr/cout, fputc etc.
	// * more!
#endif

	if( len > buffer_length ) {
#if defined(DEBUG) && !defined(MACOS)
		cerr << "Audio callback length too big! (" << len << ">" 
		     << buffer_length << ")" << endl;
#endif
		return;		// This should never happen, but just to 
		                // keep on the safe side we check anyway...
	}
	stream_lock();
	int format = Audio::get_ptr()->actual.format;
	int active_cnt=0;
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
	{
		ProducerConsumerBuf *buf = streams[i];
		if (!buf->is_active())
			continue;
		++active_cnt;
		int	ret=0;
		size_t	sofar=0;
		memset(temp_buffer,silence,len);
		while((len-sofar))
		{
			ret=buf->consume(reinterpret_cast<char*>(temp_buffer+sofar),len-sofar);
			if(ret<=0)
				break;
			sofar+=ret;
		}
		if (len - sofar && ret == -1)	// This one is done.
			buf->end_consumption();
#if 0 && !defined(MACOS)
		cerr << "(" << active_cnt <<"/"<<audio_streams.size()<< ")" << 
						" Mixing auxilliary data " ;
		cerr << sofar << " of " << (*it)->size() << endl;
#endif
		if (!sofar)
			continue;	// Nothing read.

		// the following code is not working under MacOS - it results in garbled sound
		// Propably an endianess problem?
		int dir = buf->get_dir();
		if (dir != 0 && dir != 8 && format == AUDIO_S16SYS && len%4 == 0)
			modify_stereo16(reinterpret_cast<sint16 *>(temp_buffer), len/4, dir);

		SDL::MixAudio(stream, temp_buffer, len, buf->get_volume());
	}
	if (!active_cnt)		// Nothing found?
		SDL::PauseAudio(1);	// Stop asking.
	stream_unlock();
}

AudioID	Mixer::play(uint8 *sound_data,uint32 len, int volume, int dir,
							bool repeat)
{
	ProducerConsumerBuf *audiostream=Create_Audio_Stream(
						Mixer_Sample_Magic_Number);
	if (!audiostream)
		{
		cerr << "All audio streams in use" << endl;
		return AudioID(0, 0);
		}
	SDL::LockAudio();
	audiostream->set_volume(volume);
	audiostream->set_dir(dir);
	audiostream->set_repeat(repeat);
	audiostream->produce(sound_data,len);
	audiostream->end_production();
	AudioID id(audiostream, audiostream->get_seq());
	SDL::UnlockAudio();
	return id;
}

ProducerConsumerBuf	*Mixer::Create_Audio_Stream(uint32 type)
{
	ProducerConsumerBuf *buf = 0;
	SDL::LockAudio();
	stream_lock();
	int i;				// Find an inactive stream.
	for (i = 0; i < MAX_AUDIO_STREAMS && streams[i]->is_active(); i++)
		;
	if (i < MAX_AUDIO_STREAMS)
	{
		buf = streams[i];
		buf->init(type);
		SDL::PauseAudio(0);	// Enable filling.
	}
	stream_unlock();
	SDL::UnlockAudio();
	return buf;
}

void	Mixer::Destroy_Audio_Stream(uint32 type)
{
#ifdef DEBUG
	cerr << "Destroy_Audio_Stream:  " << type << endl;
#endif
	SDL::LockAudio();
	stream_lock();
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		if (streams[i]->get_type() == type)
			streams[i]->end_consumption();
	stream_unlock();
	SDL::UnlockAudio();
}

bool	Mixer::is_playing(uint32 type)
{
	stream_lock();
	bool result = false;
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		if (streams[i]->get_type() == type)
			{
			result = streams[i]->is_active();
			break;
			}
	stream_unlock();
	return result;
}

void Mixer::cancel_streams(void)
{
	stream_lock();
	SDL::LockAudio();
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		streams[i]->end_consumption();
	SDL_UnlockAudio();
	stream_unlock();
}

#endif // PENTAGRAM
