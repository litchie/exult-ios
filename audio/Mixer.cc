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

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

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

static const int Mixer_Sample_Magic_Number=0x55443322;

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


 /* The audio function callback takes the following parameters:
     stream:  A pointer to the audio buffer to be filled
     len:     The length (in bytes) of the audio buffer
 */
 
void fill_audio(void *udata, uint8 *stream, int len)
{
	Mixer *m = Audio::get_ptr()->mixer;
	if( m )
		m->fill_audio_func(udata,stream,len);
}

void	compress_audio_sample(uint8 *buf,int len)
{
#if 0
	uint8	*dbuf=new uint8[len*2];
	uint8	*source=buf;
	uint8	*dest=dbuf;
	while(len>0)
		{
		// Left channel
		*dest=(*source+*(source+2))/2;
		++dest;
		++source;
		// Right channel
		*dest=(*source+*(source+2))/2;
		++dest;
		source+=3;
		len-=2;
		}
	memcpy(buf,dbuf,len);
	delete [] dbuf;
#endif
}

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

#if 0 && !defined(MACOS)
	cout << "fill_audio_func: " << len << endl;
	// cout << "fill_audio_func(aux): " << auxilliary_audio << endl;
#endif
	if( len > buffer_length ) {
#if DEBUG && !defined(MACOS)
		cerr << "Audio callback length too big! (" << len << ">" 
		     << buffer_length << ")" << endl;
#endif
		return;		// This should never happen, but just to 
		                // keep on the safe side we check anyway...
	}
	stream_lock();
	int active_cnt=0;
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
	{
		ProducerConsumerBuf *buf = streams[i];
		if (!buf->is_active())
			continue;
		int	ret=0;
		size_t	sofar=0;
		memset(temp_buffer,silence,len);
		while((len-sofar))
		{
			ret=buf->consume((char*)temp_buffer+sofar,len-sofar);
			if(ret<=0)
				break;
			sofar+=ret;
		}
#if 0 && !defined(MACOS)
		cerr << "(" << active_cnt <<"/"<<audio_streams.size()<< ")" << 
						" Mixing auxilliary data " ;
		cerr << sofar << " of " << (*it)->size() << endl;
#endif
		if(len-sofar&&ret==-1)
		{
			// perror("consume");
			// delete the entry
			buf->end_consumption();
			continue;
		}
		compress_audio_sample(temp_buffer,len);
		SDL::MixAudio(stream, temp_buffer, len, SDL_MIX_MAXVOLUME);
		++active_cnt;
	}
	if (!active_cnt)		// Nothing found?
		SDL::PauseAudio(1);	// Stop asking.
	stream_unlock();
}

void	Mixer::play(uint8 *sound_data,uint32 len)
{
	ProducerConsumerBuf *audiostream=Create_Audio_Stream(
						Mixer_Sample_Magic_Number);
	if (!audiostream)
		{
		cerr << "All audio streams in use" << endl;
		return;
		}
	audiostream->produce(sound_data,len);
	audiostream->end_production();
}

#if 0
void	Mixer::set_auxilliary_audio(int fh)
{
	auxilliary_audio=fh;
#if DEBUG
	cout << "Auxilliary audio stream: " << auxilliary_audio << endl;
#endif
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
}
#endif

ProducerConsumerBuf	*Mixer::Create_Audio_Stream(uint32 id)
{
	ProducerConsumerBuf *buf = 0;
	SDL::PauseAudio(1);
	stream_lock();
#if DEBUG
	cerr << "Create_Audio_Stream()" << endl;
#endif
	int i;				// Find an inactive stream.
	for (i = 0; i < MAX_AUDIO_STREAMS && streams[i]->is_active(); i++)
		;
	if (i < MAX_AUDIO_STREAMS)
		{
		buf = streams[i];
		buf->init(id);
		cerr << "Create_Audio_Stream:  " << i << endl;
		}
	stream_unlock();
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
	return buf;
}

void	Mixer::Destroy_Audio_Stream(uint32 id)
{
	cerr << "Destroy_Audio_Stream:  " << id << endl;
	SDL::PauseAudio(1);
	stream_lock();
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		if (streams[i]->get_id() == id)
			{
			streams[i]->end_consumption();
			break;
			}
	stream_unlock();
	SDL::PauseAudio(0);
}

bool	Mixer::is_playing(uint32 id)
{
	stream_lock();
	bool result = false;
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		if (streams[i]->get_id() == id)
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
	for (int i = 0; i < MAX_AUDIO_STREAMS; i++)
		streams[i]->end_consumption();
	stream_unlock();
}

